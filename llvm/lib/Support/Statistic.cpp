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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Statistic.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Mutex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Timer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLTraits.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cstring>
using ନାମାକାଶ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ସ୍ଥିର ତାର୍କିକ EnableStats;
ସ୍ଥିର ତାର୍କିକ StatsAsJSON;
ସ୍ଥିର ତାର୍କିକ Enabled;
ସ୍ଥିର ତାର୍କିକ PrintOnExit;

ଶୂନ୍ୟ llvm::initStatisticOptions() {
  ସ୍ଥିର cl::opt<ତାର୍କିକ, ସତ୍ୟ> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ସ୍ଥିର cl::opt<ତାର୍କିକ, ସତ୍ୟ> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

ନାମାକାଶ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
ଶ୍ରେଣୀ StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend ଶୂନ୍ୟ llvm::PrintStatistics();
  friend ଶୂନ୍ୟ llvm::PrintStatistics(raw_ostream &OS);
  friend ଶୂନ୍ୟ llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  ଶୂନ୍ୟ sort();
ସର୍ବସାଧାରଣ:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  ଶୂନ୍ୟ addStatistic(TrackingStatistic *S) { Stats.ଠେଲ_back(S); }

  const_iterator begin() ଅଚଳ { ଫେରନ୍ତୁ Stats.begin(); }
  const_iterator end() ଅଚଳ { ଫେରନ୍ତୁ Stats.end(); }
  iterator_range<const_iterator> statistics() ଅଚଳ {
    ଫେରନ୍ତୁ {begin(), end()};
  }

  ଶୂନ୍ୟ reset();
};
} // end anonymous namespace

ସ୍ଥିର ManagedStatic<StatisticInfo> StatInfo;
ସ୍ଥିର ManagedStatic<sys::SmartMutex<ସତ୍ୟ> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
ଶୂନ୍ୟ TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  ଯଦି (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<ସତ୍ୟ> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<ସତ୍ୟ> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    ଯଦି (Initialized.load(std::memory_order_relaxed))
      ଫେରନ୍ତୁ;
    ଯଦି (EnableStats || Enabled)
      SI.addStatistic(ଏହି);

    // Remember we have been registered.
    Initialized.store(ସତ୍ୟ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  ଯଦି (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

ଶୂନ୍ୟ llvm::EnableStatistics(ତାର୍କିକ DoPrintOnExit) {
  Enabled = ସତ୍ୟ;
  PrintOnExit = DoPrintOnExit;
}

ତାର୍କିକ llvm::AreStatisticsEnabled() { ଫେରନ୍ତୁ Enabled || EnableStats; }

ଶୂନ୍ୟ StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](ଅଚଳ TrackingStatistic *LHS, ଅଚଳ TrackingStatistic *RHS) {
        ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          ଫେରନ୍ତୁ Cmp < 0;

        ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          ଫେରନ୍ତୁ Cmp < 0;

        ଫେରନ୍ତୁ std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

ଶୂନ୍ୟ StatisticInfo::reset() {
  sys::SmartScopedLock<ସତ୍ୟ> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  ପାଇଁ (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = ମିଥ୍ୟା;
    Stat->ମୂଲ୍ୟ = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

ଶୂନ୍ୟ llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  ପାଇଁ (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->get_ମୂଲ୍ୟ()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  ପାଇଁ (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_ମୂଲ୍ୟ(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

ଶୂନ୍ୟ llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<ସତ୍ୟ> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  ଅଚଳ ଅକ୍ଷର *delim = "";
  ପାଇଁ (ଅଚଳ TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    ପ୍ରତିଜ୍ଞା(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    ପ୍ରତିଜ୍ଞା(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_ମୂଲ୍ୟ();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

ଶୂନ୍ୟ llvm::PrintStatistics() {
#ଯଦି LLVM_ENABLE_STATS
  sys::SmartScopedLock<ସତ୍ୟ> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  ଯଦି (Stats.Stats.empty()) ଫେରନ୍ତୁ;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  ଯଦି (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  ଅନ୍ୟଥା
    PrintStatistics(*OutStream);

#ଅନ୍ୟଥା
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  ଯଦି (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<ସତ୍ୟ> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  ପାଇଁ (ଅଚଳ auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_ମୂଲ୍ୟ());
  ଫେରନ୍ତୁ ReturnStats;
}

ଶୂନ୍ୟ llvm::ResetStatistics() {
  StatInfo->reset();
}
