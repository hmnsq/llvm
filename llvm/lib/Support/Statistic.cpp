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

#અન્તર્ભાવ "llvm/ADT/Statistic.h"

#અન્તર્ભાવ "DebugOptions.h"

#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/CommandLine.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/Mutex.h"
#અન્તર્ભાવ "llvm/Support/Timer.h"
#અન્તર્ભાવ "llvm/Support/YAMLTraits.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cstring>
using નામાકાશ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
સ્થિર તાર્કિક EnableStats;
સ્થિર તાર્કિક StatsAsJSON;
સ્થિર તાર્કિક Enabled;
સ્થિર તાર્કિક PrintOnExit;

શૂન્ય llvm::initStatisticOptions() {
  સ્થિર cl::opt<તાર્કિક, સાચું> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  સ્થિર cl::opt<તાર્કિક, સાચું> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

નામાકાશ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
વર્ગ StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend શૂન્ય llvm::PrintStatistics();
  friend શૂન્ય llvm::PrintStatistics(raw_ostream &OS);
  friend શૂન્ય llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  શૂન્ય sort();
જાહેર:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  શૂન્ય addStatistic(TrackingStatistic *S) { Stats.ધકેલો_back(S); }

  const_iterator begin() અચલ { પાછા Stats.begin(); }
  const_iterator end() અચલ { પાછા Stats.end(); }
  iterator_range<const_iterator> statistics() અચલ {
    પાછા {begin(), end()};
  }

  શૂન્ય reset();
};
} // end anonymous namespace

સ્થિર ManagedStatic<StatisticInfo> StatInfo;
સ્થિર ManagedStatic<sys::SmartMutex<સાચું> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
શૂન્ય TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  જો (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<સાચું> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<સાચું> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    જો (Initialized.load(std::memory_order_relaxed))
      પાછા;
    જો (EnableStats || Enabled)
      SI.addStatistic(આ);

    // Remember we have been registered.
    Initialized.store(સાચું, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  જો (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

શૂન્ય llvm::EnableStatistics(તાર્કિક DoPrintOnExit) {
  Enabled = સાચું;
  PrintOnExit = DoPrintOnExit;
}

તાર્કિક llvm::AreStatisticsEnabled() { પાછા Enabled || EnableStats; }

શૂન્ય StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](અચલ TrackingStatistic *LHS, અચલ TrackingStatistic *RHS) {
        જો (પૂર્ણાંક Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          પાછા Cmp < 0;

        જો (પૂર્ણાંક Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          પાછા Cmp < 0;

        પાછા std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

શૂન્ય StatisticInfo::reset() {
  sys::SmartScopedLock<સાચું> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  માટે (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = ખોટું;
    Stat->કિંમત = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

શૂન્ય llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  માટે (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->get_કિંમત()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  માટે (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_કિંમત(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

શૂન્ય llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<સાચું> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  અચલ અક્ષર *delim = "";
  માટે (અચલ TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    પ્રતિજ્ઞા(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    પ્રતિજ્ઞા(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_કિંમત();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

શૂન્ય llvm::PrintStatistics() {
#જો LLVM_ENABLE_STATS
  sys::SmartScopedLock<સાચું> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  જો (Stats.Stats.empty()) પાછા;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  જો (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  નહીંતર
    PrintStatistics(*OutStream);

#નહીંતર
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  જો (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<સાચું> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  માટે (અચલ auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_કિંમત());
  પાછા ReturnStats;
}

શૂન્ય llvm::ResetStatistics() {
  StatInfo->reset();
}
