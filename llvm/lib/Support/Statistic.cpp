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

#شامل "llvm/ADT/Statistic.h"

#شامل "DebugOptions.h"

#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/Mutex.h"
#شامل "llvm/Support/Timer.h"
#شامل "llvm/Support/YAMLTraits.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <algorithm>
#شامل <cstring>
using فضاینام llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ایستا منطقی EnableStats;
ایستا منطقی StatsAsJSON;
ایستا منطقی Enabled;
ایستا منطقی PrintOnExit;

تهی llvm::initStatisticOptions() {
  ایستا cl::opt<منطقی, درست> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ایستا cl::opt<منطقی, درست> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

فضاینام {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
رده StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend تهی llvm::PrintStatistics();
  friend تهی llvm::PrintStatistics(raw_ostream &OS);
  friend تهی llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  تهی sort();
عمومی:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  تهی addStatistic(TrackingStatistic *S) { Stats.افزودن_back(S); }

  const_iterator begin() ثابت { بازگشت Stats.begin(); }
  const_iterator end() ثابت { بازگشت Stats.end(); }
  iterator_range<const_iterator> statistics() ثابت {
    بازگشت {begin(), end()};
  }

  تهی reset();
};
} // end anonymous namespace

ایستا ManagedStatic<StatisticInfo> StatInfo;
ایستا ManagedStatic<sys::SmartMutex<درست> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
تهی TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  اگر (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<درست> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<درست> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    اگر (Initialized.load(std::memory_order_relaxed))
      بازگشت;
    اگر (EnableStats || Enabled)
      SI.addStatistic(این);

    // Remember we have been registered.
    Initialized.store(درست, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  اگر (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

تهی llvm::EnableStatistics(منطقی DoPrintOnExit) {
  Enabled = درست;
  PrintOnExit = DoPrintOnExit;
}

منطقی llvm::AreStatisticsEnabled() { بازگشت Enabled || EnableStats; }

تهی StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](ثابت TrackingStatistic *LHS, ثابت TrackingStatistic *RHS) {
        اگر (صحیح Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          بازگشت Cmp < 0;

        اگر (صحیح Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          بازگشت Cmp < 0;

        بازگشت std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

تهی StatisticInfo::reset() {
  sys::SmartScopedLock<درست> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  برای (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = نادرست;
    Stat->مقدار = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

تهی llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  برای (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->get_مقدار()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  برای (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_مقدار(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

تهی llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<درست> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  ثابت نویسه *delim = "";
  برای (ثابت TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    أكد(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    أكد(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_مقدار();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

تهی llvm::PrintStatistics() {
#اگر LLVM_ENABLE_STATS
  sys::SmartScopedLock<درست> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  اگر (Stats.Stats.empty()) بازگشت;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  اگر (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  وگرنه
    PrintStatistics(*OutStream);

#وگرنه
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  اگر (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<درست> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  برای (ثابت auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_مقدار());
  بازگشت ReturnStats;
}

تهی llvm::ResetStatistics() {
  StatInfo->reset();
}
