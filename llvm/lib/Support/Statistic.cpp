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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Statistic.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Timer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLTraits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ਸਥਿਰ ਤਾਰਕਿਕ EnableStats;
ਸਥਿਰ ਤਾਰਕਿਕ StatsAsJSON;
ਸਥਿਰ ਤਾਰਕਿਕ Enabled;
ਸਥਿਰ ਤਾਰਕਿਕ PrintOnExit;

ਖਾਲੀ llvm::initStatisticOptions() {
  ਸਥਿਰ cl::opt<ਤਾਰਕਿਕ, ਸੱਚ> ਰਜਿਸਟਰ_Enable_Stats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ਸਥਿਰ cl::opt<ਤਾਰਕਿਕ, ਸੱਚ> ਰਜਿਸਟਰ_Stats_As_Json{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

ਨਾਮਾਕਾਸ਼ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
ਵਰਗ StatisticInfo {
  std::ਸਦਿਸ਼<TrackingStatistic *> Stats;

  ਮਿੱਤਰ ਖਾਲੀ llvm::PrintStatistics();
  ਮਿੱਤਰ ਖਾਲੀ llvm::PrintStatistics(raw_ostream &OS);
  ਮਿੱਤਰ ਖਾਲੀ llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  ਖਾਲੀ ਛਾਂਟੋ();
ਜਨਤਕ:
  ਵਰਤੋਂ const_iterator = std::ਸਦਿਸ਼<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  ਖਾਲੀ ਜੋੜੋ_Statistic(TrackingStatistic *S) { Stats.ਧੱਕੋ_ਪਿੱਠ(S); }

  const_iterator begin() ਅਚਲ { ਵਾਪਸ Stats.begin(); }
  const_iterator ਸਿਰਾ() ਅਚਲ { ਵਾਪਸ Stats.ਸਿਰਾ(); }
  iterator_range<const_iterator> statistics() ਅਚਲ {
    ਵਾਪਸ {begin(), ਸਿਰਾ()};
  }

  ਖਾਲੀ reset();
};
} // end anonymous namespace

ਸਥਿਰ ManagedStatic<StatisticInfo> StatInfo;
ਸਥਿਰ ManagedStatic<sys::SmartMutex<ਸੱਚ> > Stat_ਤਾਲਾ;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
ਖਾਲੀ TrackingStatistic::ਰਜਿਸਟਰ_Statistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  ਜੇ (!Initialized.ਲੱਦੋ(std::memory_order_relaxed)) {
    sys::SmartMutex<ਸੱਚ> &ਤਾਲਾ_2 = *Stat_ਤਾਲਾ;
    StatisticInfo &SI = *StatInfo;
    sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> Writer(ਤਾਲਾ_2);
    // Check Initialized again after acquiring the lock.
    ਜੇ (Initialized.ਲੱਦੋ(std::memory_order_relaxed))
      ਵਾਪਸ;
    ਜੇ (EnableStats || Enabled)
      SI.ਜੋੜੋ_Statistic(ਇਹ);

    // Remember we have been registered.
    Initialized.store(ਸੱਚ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  Timer_ਸਮੂਹ::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  ਜੇ (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

ਖਾਲੀ llvm::EnableStatistics(ਤਾਰਕਿਕ DoPrintOnExit) {
  Enabled = ਸੱਚ;
  PrintOnExit = DoPrintOnExit;
}

ਤਾਰਕਿਕ llvm::AreStatisticsEnabled() { ਵਾਪਸ Enabled || EnableStats; }

ਖਾਲੀ StatisticInfo::ਛਾਂਟੋ() {
  llvm::stable_ਛਾਂਟੋ(
      Stats, [](ਅਚਲ TrackingStatistic *LHS, ਅਚਲ TrackingStatistic *RHS) {
        ਜੇ (ਪੂਰਨ Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          ਵਾਪਸ Cmp < 0;

        ਜੇ (ਪੂਰਨ Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          ਵਾਪਸ Cmp < 0;

        ਵਾਪਸ std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

ਖਾਲੀ StatisticInfo::reset() {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> Writer(*Stat_ਤਾਲਾ);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  ਲਈ (ਸਵੈਚਾਲਿਤ *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = ਝੂਠ;
    Stat->ਮੁੱਲ = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

ਖਾਲੀ llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  ਬਿਨਾਨਿਸ਼ਾਨੀ MaxDebugTypeLen = 0, MaxValLen = 0;
  ਲਈ (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (ਬਿਨਾਨਿਸ਼ਾਨੀ)utostr(Stat->get_ਮੁੱਲ()).ਮਾਪ_2());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (ਬਿਨਾਨਿਸ਼ਾਨੀ)std::ਲੜੀ_ਲੰਬਾਈ(Stat->getDebugType()));
  }

  Stats.ਛਾਂਟੋ();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  ਲਈ (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_ਮੁੱਲ(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

ਖਾਲੀ llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> Reader(*Stat_ਤਾਲਾ);
  StatisticInfo &Stats = *StatInfo;

  Stats.ਛਾਂਟੋ();

  // Print all of the statistics.
  OS << "{\n";
  ਅਚਲ ਅੱਖਰ *delim = "";
  ਲਈ (ਅਚਲ TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    ਪ੍ਰਤਿਜ੍ਞਾ(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    ਪ੍ਰਤਿਜ੍ਞਾ(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_ਮੁੱਲ();
    delim = ",\n";
  }
  // Print timers.
  Timer_ਸਮੂਹ::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

ਖਾਲੀ llvm::PrintStatistics() {
#ਜੇ LLVM_ENABLE_STATS
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> Reader(*Stat_ਤਾਲਾ);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  ਜੇ (Stats.Stats.ਖਾਲੀ_ਥਾਵਾਂ()) ਵਾਪਸ;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> ਬਾਹਰਲਾ_ਧਾਰਾ = CreateInfoOutputFile();
  ਜੇ (StatsAsJSON)
    PrintStatisticsJSON(*ਬਾਹਰਲਾ_ਧਾਰਾ);
  ਨਹੀਂਤਾਂ
    PrintStatistics(*ਬਾਹਰਲਾ_ਧਾਰਾ);

#ਨਹੀਂਤਾਂ
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  ਜੇ (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> ਬਾਹਰਲਾ_ਧਾਰਾ = CreateInfoOutputFile();
    (*ਬਾਹਰਲਾ_ਧਾਰਾ) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#ਜੇ_ਅੰਤ
}

std::ਸਦਿਸ਼<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> Reader(*Stat_ਤਾਲਾ);
  std::ਸਦਿਸ਼<std::pair<StringRef, uint64_t>> ReturnStats;

  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Stat : StatInfo->statistics())
    ReturnStats.emplace_ਪਿੱਠ(Stat->getName(), Stat->get_ਮੁੱਲ());
  ਵਾਪਸ ReturnStats;
}

ਖਾਲੀ llvm::ResetStatistics() {
  StatInfo->reset();
}
