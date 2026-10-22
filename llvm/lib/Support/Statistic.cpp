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

#അന്തര്ഭാവ "llvm/ADT/Statistic.h"

#അന്തര്ഭാവ "DebugOptions.h"

#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/CommandLine.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/Mutex.h"
#അന്തര്ഭാവ "llvm/Support/Timer.h"
#അന്തര്ഭാവ "llvm/Support/YAMLTraits.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cstring>
using നാമാകാശ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
സ്ഥിതം തർക്കം EnableStats;
സ്ഥിതം തർക്കം StatsAsJSON;
സ്ഥിതം തർക്കം Enabled;
സ്ഥിതം തർക്കം PrintOnExit;

ശൂന്യം llvm::initStatisticOptions() {
  സ്ഥിതം cl::opt<തർക്കം, സത്യം> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  സ്ഥിതം cl::opt<തർക്കം, സത്യം> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

നാമാകാശ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
വർഗ്ഗം StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  friend ശൂന്യം llvm::PrintStatistics();
  friend ശൂന്യം llvm::PrintStatistics(raw_ostream &OS);
  friend ശൂന്യം llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  ശൂന്യം sort();
പൊതു:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  ശൂന്യം addStatistic(TrackingStatistic *S) { Stats.തള്ളുക_back(S); }

  const_iterator begin() സ്ഥിരം { തിരികെ Stats.begin(); }
  const_iterator end() സ്ഥിരം { തിരികെ Stats.end(); }
  iterator_range<const_iterator> statistics() സ്ഥിരം {
    തിരികെ {begin(), end()};
  }

  ശൂന്യം reset();
};
} // end anonymous namespace

സ്ഥിതം ManagedStatic<StatisticInfo> StatInfo;
സ്ഥിതം ManagedStatic<sys::SmartMutex<സത്യം> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
ശൂന്യം TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  എങ്കിൽ (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<സത്യം> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<സത്യം> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    എങ്കിൽ (Initialized.load(std::memory_order_relaxed))
      തിരികെ;
    എങ്കിൽ (EnableStats || Enabled)
      SI.addStatistic(ഇത്);

    // Remember we have been registered.
    Initialized.store(സത്യം, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  എങ്കിൽ (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

ശൂന്യം llvm::EnableStatistics(തർക്കം DoPrintOnExit) {
  Enabled = സത്യം;
  PrintOnExit = DoPrintOnExit;
}

തർക്കം llvm::AreStatisticsEnabled() { തിരികെ Enabled || EnableStats; }

ശൂന്യം StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](സ്ഥിരം TrackingStatistic *LHS, സ്ഥിരം TrackingStatistic *RHS) {
        എങ്കിൽ (പൂർണ്ണസംഖ്യ Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          തിരികെ Cmp < 0;

        എങ്കിൽ (പൂർണ്ണസംഖ്യ Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          തിരികെ Cmp < 0;

        തിരികെ std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

ശൂന്യം StatisticInfo::reset() {
  sys::SmartScopedLock<സത്യം> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  വേണ്ടി (auto *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = അസത്യം;
    Stat->മൂല്യം = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

ശൂന്യം llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  unsigned MaxDebugTypeLen = 0, MaxValLen = 0;
  വേണ്ടി (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (unsigned)utostr(Stat->get_മൂല്യം()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (unsigned)std::strlen(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  വേണ്ടി (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_മൂല്യം(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

ശൂന്യം llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<സത്യം> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  സ്ഥിരം അക്ഷരം *delim = "";
  വേണ്ടി (സ്ഥിരം TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    പ്രതിജ്ഞാ(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    പ്രതിജ്ഞാ(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_മൂല്യം();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

ശൂന്യം llvm::PrintStatistics() {
#എങ്കിൽ LLVM_ENABLE_STATS
  sys::SmartScopedLock<സത്യം> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  എങ്കിൽ (Stats.Stats.empty()) തിരികെ;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  എങ്കിൽ (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  അല്ലെങ്കിൽ
    PrintStatistics(*OutStream);

#അല്ലെങ്കിൽ
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  എങ്കിൽ (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<സത്യം> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  വേണ്ടി (സ്ഥിരം auto &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_മൂല്യം());
  തിരികെ ReturnStats;
}

ശൂന്യം llvm::ResetStatistics() {
  StatInfo->reset();
}
