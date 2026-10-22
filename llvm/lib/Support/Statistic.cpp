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

#அந்தர்பாவ "llvm/ADT/Statistic.h"

#அந்தர்பாவ "DebugOptions.h"

#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/CommandLine.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/Mutex.h"
#அந்தர்பாவ "llvm/Support/Timer.h"
#அந்தர்பாவ "llvm/Support/YAMLTraits.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cstring>
using நாமாகாஶ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
நிலையான தருக்க EnableStats;
நிலையான தருக்க StatsAsJSON;
நிலையான தருக்க Enabled;
நிலையான தருக்க PrintOnExit;

வெறுமை llvm::initStatisticOptions() {
  நிலையான cl::opt<தருக்க, உண்மை> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  நிலையான cl::opt<தருக்க, உண்மை> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

நாமாகாஶ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
வகுப்பு StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend வெறுமை llvm::PrintStatistics();
  friend வெறுமை llvm::PrintStatistics(raw_ostream &OS);
  friend வெறுமை llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  வெறுமை sort();
பொது:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  வெறுமை addStatistic(TrackingStatistic *S) { Stats.தள்ளு_back(S); }

  const_iterator begin() மாறிலி { திரும்பு Stats.begin(); }
  const_iterator end() மாறிலி { திரும்பு Stats.end(); }
  iterator_range<const_iterator> statistics() மாறிலி {
    திரும்பு {begin(), end()};
  }

  வெறுமை reset();
};
} // end anonymous namespace

நிலையான ManagedStatic<StatisticInfo> StatInfo;
நிலையான ManagedStatic<sys::SmartMutex<உண்மை> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
வெறுமை TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  எனில் (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<உண்மை> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<உண்மை> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    எனில் (Initialized.load(std::memory_order_relaxed))
      திரும்பு;
    எனில் (EnableStats || Enabled)
      SI.addStatistic(இது);

    // Remember we have been registered.
    Initialized.store(உண்மை, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  எனில் (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

வெறுமை llvm::EnableStatistics(தருக்க DoPrintOnExit) {
  Enabled = உண்மை;
  PrintOnExit = DoPrintOnExit;
}

தருக்க llvm::AreStatisticsEnabled() { திரும்பு Enabled || EnableStats; }

வெறுமை StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](மாறிலி TrackingStatistic *LHS, மாறிலி TrackingStatistic *RHS) {
        எனில் (முழு Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          திரும்பு Cmp < 0;

        எனில் (முழு Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          திரும்பு Cmp < 0;

        திரும்பு std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

வெறுமை StatisticInfo::reset() {
  sys::SmartScopedLock<உண்மை> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  க்கு (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = பொய்;
    Stat->மதிப்பு = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

வெறுமை llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  க்கு (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->get_மதிப்பு()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  க்கு (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_மதிப்பு(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

வெறுமை llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<உண்மை> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  மாறிலி எழுத்து *delim = "";
  க்கு (மாறிலி TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    ப்ரதிஜ்ஞா(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    ப்ரதிஜ்ஞா(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_மதிப்பு();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

வெறுமை llvm::PrintStatistics() {
#எனில் LLVM_ENABLE_STATS
  sys::SmartScopedLock<உண்மை> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  எனில் (Stats.Stats.empty()) திரும்பு;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  எனில் (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  இல்லையெனில்
    PrintStatistics(*OutStream);

#இல்லையெனில்
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  எனில் (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<உண்மை> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  க்கு (மாறிலி auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_மதிப்பு());
  திரும்பு ReturnStats;
}

வெறுமை llvm::ResetStatistics() {
  StatInfo->reset();
}
