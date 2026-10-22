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

#అన్తర్భావ "llvm/ADT/Statistic.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/Mutex.h"
#అన్తర్భావ "llvm/Support/Timer.h"
#అన్తర్భావ "llvm/Support/YAMLTraits.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cstring>
ఉపయోగించి నామాకాశ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
స్థిర తార్కిక EnableStats;
స్థిర తార్కిక StatsAsJSON;
స్థిర తార్కిక Enabled;
స్థిర తార్కిక PrintOnExit;

శూన్య llvm::initStatisticOptions() {
  స్థిర cl::opt<తార్కిక, నిజం> రిజిస్టరు_Enable_Stats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  స్థిర cl::opt<తార్కిక, నిజం> రిజిస్టరు_Stats_As_Json{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

నామాకాశ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
తరగతి StatisticInfo {
  std::సదిశ<TrackingStatistic *> Stats;

  మిత్ర శూన్య llvm::PrintStatistics();
  మిత్ర శూన్య llvm::PrintStatistics(raw_ostream &OS);
  మిత్ర శూన్య llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  శూన్య క్రమీకరించు();
బహిరంగ:
  ఉపయోగించి const_iterator = std::సదిశ<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  శూన్య కూడు_Statistic(TrackingStatistic *S) { Stats.నెట్టు_వెనుక(S); }

  const_iterator begin() స్థిరరాశి { తిరిగి Stats.begin(); }
  const_iterator చివర() స్థిరరాశి { తిరిగి Stats.చివర(); }
  iterator_range<const_iterator> statistics() స్థిరరాశి {
    తిరిగి {begin(), చివర()};
  }

  శూన్య reset();
};
} // end anonymous namespace

స్థిర ManagedStatic<StatisticInfo> StatInfo;
స్థిర ManagedStatic<sys::SmartMutex<నిజం> > Stat_తాళం;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
శూన్య TrackingStatistic::రిజిస్టరు_Statistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  ఒకవేళ (!Initialized.ఎక్కించు(std::memory_order_relaxed)) {
    sys::SmartMutex<నిజం> &తాళం_3 = *Stat_తాళం;
    StatisticInfo &SI = *StatInfo;
    sys::Smart_Scoped_తాళం<నిజం> Writer(తాళం_3);
    // Check Initialized again after acquiring the lock.
    ఒకవేళ (Initialized.ఎక్కించు(std::memory_order_relaxed))
      తిరిగి;
    ఒకవేళ (EnableStats || Enabled)
      SI.కూడు_Statistic(ఇది);

    // Remember we have been registered.
    Initialized.store(నిజం, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  Timer_సమూహం::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  ఒకవేళ (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

శూన్య llvm::EnableStatistics(తార్కిక DoPrintOnExit) {
  Enabled = నిజం;
  PrintOnExit = DoPrintOnExit;
}

తార్కిక llvm::AreStatisticsEnabled() { తిరిగి Enabled || EnableStats; }

శూన్య StatisticInfo::క్రమీకరించు() {
  llvm::stable_క్రమీకరించు(
      Stats, [](స్థిరరాశి TrackingStatistic *LHS, స్థిరరాశి TrackingStatistic *RHS) {
        ఒకవేళ (పూర్ణాంకం Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          తిరిగి Cmp < 0;

        ఒకవేళ (పూర్ణాంకం Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          తిరిగి Cmp < 0;

        తిరిగి std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

శూన్య StatisticInfo::reset() {
  sys::Smart_Scoped_తాళం<నిజం> Writer(*Stat_తాళం);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  కొరకు (స్వయంచాలక *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = అబద్ధం;
    Stat->విలువ = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

శూన్య llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  గుర్తులేని MaxDebugTypeLen = 0, MaxValLen = 0;
  కొరకు (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (గుర్తులేని)utostr(Stat->get_విలువ()).కొలత_2());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (గుర్తులేని)std::అక్షరశ్రేణి_పొడవు(Stat->getDebugType()));
  }

  Stats.క్రమీకరించు();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  కొరకు (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_విలువ(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

శూన్య llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::Smart_Scoped_తాళం<నిజం> Reader(*Stat_తాళం);
  StatisticInfo &Stats = *StatInfo;

  Stats.క్రమీకరించు();

  // Print all of the statistics.
  OS << "{\n";
  స్థిరరాశి అక్షరం *delim = "";
  కొరకు (స్థిరరాశి TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    ప్రతిజ్ఞా(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    ప్రతిజ్ఞా(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_విలువ();
    delim = ",\n";
  }
  // Print timers.
  Timer_సమూహం::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

శూన్య llvm::PrintStatistics() {
#ఒకవేళ LLVM_ENABLE_STATS
  sys::Smart_Scoped_తాళం<నిజం> Reader(*Stat_తాళం);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  ఒకవేళ (Stats.Stats.ఖాళీలు()) తిరిగి;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> బహిర్గతం_ప్రవాహం = CreateInfoOutputFile();
  ఒకవేళ (StatsAsJSON)
    PrintStatisticsJSON(*బహిర్గతం_ప్రవాహం);
  లేకపోతే
    PrintStatistics(*బహిర్గతం_ప్రవాహం);

#లేకపోతే
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  ఒకవేళ (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> బహిర్గతం_ప్రవాహం = CreateInfoOutputFile();
    (*బహిర్గతం_ప్రవాహం) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#అయితే_ముగింపు
}

std::సదిశ<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::Smart_Scoped_తాళం<నిజం> Reader(*Stat_తాళం);
  std::సదిశ<std::pair<StringRef, uint64_t>> ReturnStats;

  కొరకు (స్థిరరాశి స్వయంచాలక &Stat : StatInfo->statistics())
    ReturnStats.emplace_వెనుక(Stat->getName(), Stat->get_విలువ());
  తిరిగి ReturnStats;
}

శూన్య llvm::ResetStatistics() {
  StatInfo->reset();
}
