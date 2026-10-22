//===-- Statistic.cpp - Easy way to expose stats information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the 'Statistic' class, which is designed to be an easy
// way to expose various success metrics from passes.  These statistics are
// printed at the end of a run, when the -stats command line option is enabled
// on the command line.
//
// This is useful for reporting information like the number of instructions
// simplified, optimized or removed by various transformations, like this:
//
// static Statistic NumInstEliminated("GCSE", "Number of instructions killed");
//
// Later, in the code: ++NumInstEliminated;
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/Statistic.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/Timer.h"
#ضمن "llvm/Support/YAMLTraits.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <algorithm>
#ضمن <cstring>
using نطاق llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ساكن منطقي EnableStats;
ساكن منطقي StatsAsJSON;
ساكن منطقي Enabled;
ساكن منطقي PrintOnExit;

فراغ llvm::initStatisticOptions() {
  ساكن cl::opt<منطقي, ܫܪܪܐ> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ساكن cl::opt<منطقي, ܫܪܪܐ> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

نطاق {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
صنف StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend فراغ llvm::PrintStatistics();
  friend فراغ llvm::PrintStatistics(raw_ostream &OS);
  friend فراغ llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  فراغ sort();
عام:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  فراغ addStatistic(TrackingStatistic *S) { Stats.push_back(S); }

  const_iterator begin() ثابت { ܦܢܐ Stats.begin(); }
  const_iterator end() ثابت { ܦܢܐ Stats.end(); }
  iterator_range<const_iterator> statistics() ثابت {
    ܦܢܐ {begin(), end()};
  }

  فراغ reset();
};
} // end anonymous namespace

ساكن ManagedStatic<StatisticInfo> StatInfo;
ساكن ManagedStatic<sys::SmartMutex<ܫܪܪܐ> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
فراغ TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  ܐܢ (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<ܫܪܪܐ> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<ܫܪܪܐ> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    ܐܢ (Initialized.load(std::memory_order_relaxed))
      ܦܢܐ;
    ܐܢ (EnableStats || Enabled)
      SI.addStatistic(هذا);

    // Remember we have been registered.
    Initialized.store(ܫܪܪܐ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  ܐܢ (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

فراغ llvm::EnableStatistics(منطقي DoPrintOnExit) {
  Enabled = ܫܪܪܐ;
  PrintOnExit = DoPrintOnExit;
}

منطقي llvm::AreStatisticsEnabled() { ܦܢܐ Enabled || EnableStats; }

فراغ StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](ثابت TrackingStatistic *LHS, ثابت TrackingStatistic *RHS) {
        ܐܢ (صحيح Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          ܦܢܐ Cmp < 0;

        ܐܢ (صحيح Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          ܦܢܐ Cmp < 0;

        ܦܢܐ std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

فراغ StatisticInfo::reset() {
  sys::SmartScopedLock<ܫܪܪܐ> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  لكل (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = ܕܓܠܐ;
    Stat->Value = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

فراغ llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  لكل (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->getValue()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  لكل (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->getValue(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

فراغ llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<ܫܪܪܐ> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  ثابت حرف *delim = "";
  لكل (ثابت TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    أكد(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    أكد(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->getValue();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

فراغ llvm::PrintStatistics() {
#ܐܢ LLVM_ENABLE_STATS
  sys::SmartScopedLock<ܫܪܪܐ> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  ܐܢ (Stats.Stats.empty()) ܦܢܐ;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  ܐܢ (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  ܐܚܪܢܐ
    PrintStatistics(*OutStream);

#ܐܚܪܢܐ
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  ܐܢ (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<ܫܪܪܐ> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  لكل (ثابت auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->getValue());
  ܦܢܐ ReturnStats;
}

فراغ llvm::ResetStatistics() {
  StatInfo->reset();
}
