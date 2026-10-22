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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/Statistic.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Timer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLTraits.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cstring>
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ಸ್ಥಿರ ತಾರ್ಕಿಕ EnableStats;
ಸ್ಥಿರ ತಾರ್ಕಿಕ StatsAsJSON;
ಸ್ಥಿರ ತಾರ್ಕಿಕ Enabled;
ಸ್ಥಿರ ತಾರ್ಕಿಕ PrintOnExit;

ಶೂನ್ಯ llvm::initStatisticOptions() {
  ಸ್ಥಿರ cl::opt<ತಾರ್ಕಿಕ, ನಿಜ> ರಿಜಿಸ್ಟರ್_Enable_Stats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ಸ್ಥಿರ cl::opt<ತಾರ್ಕಿಕ, ನಿಜ> ರಿಜಿಸ್ಟರ್_Stats_As_Json{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

ನಾಮಾಕಾಶ {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
ವರ್ಗ StatisticInfo {
  std::ಸದಿಶ<TrackingStatistic *> Stats;

  ಮಿತ್ರ ಶೂನ್ಯ llvm::PrintStatistics();
  ಮಿತ್ರ ಶೂನ್ಯ llvm::PrintStatistics(raw_ostream &OS);
  ಮಿತ್ರ ಶೂನ್ಯ llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  ಶೂನ್ಯ ವಿಂಗಡಿಸು();
ಸಾರ್ವಜನಿಕ:
  ಬಳಸಿ const_iterator = std::ಸದಿಶ<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  ಶೂನ್ಯ ಕೂಡಿಸು_Statistic(TrackingStatistic *S) { Stats.ತಳ್ಳು_ಹಿಂಭಾಗ(S); }

  const_iterator begin() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Stats.begin(); }
  const_iterator ಕೊನೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Stats.ಕೊನೆ(); }
  iterator_range<const_iterator> statistics() ಸ್ಥಿರಾಂಕ {
    ಹಿಂತಿರುಗಿ {begin(), ಕೊನೆ()};
  }

  ಶೂನ್ಯ reset();
};
} // end anonymous namespace

ಸ್ಥಿರ ManagedStatic<StatisticInfo> StatInfo;
ಸ್ಥಿರ ManagedStatic<sys::SmartMutex<ನಿಜ> > Stat_ಬೀಗ;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
ಶೂನ್ಯ TrackingStatistic::ರಿಜಿಸ್ಟರ್_Statistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  ಆದರೆ (!Initialized.ತುಂಬು(std::memory_order_relaxed)) {
    sys::SmartMutex<ನಿಜ> &ಬೀಗ_3 = *Stat_ಬೀಗ;
    StatisticInfo &SI = *StatInfo;
    sys::Smart_Scoped_ಬೀಗ<ನಿಜ> Writer(ಬೀಗ_3);
    // Check Initialized again after acquiring the lock.
    ಆದರೆ (Initialized.ತುಂಬು(std::memory_order_relaxed))
      ಹಿಂತಿರುಗಿ;
    ಆದರೆ (EnableStats || Enabled)
      SI.ಕೂಡಿಸು_Statistic(ಇದು);

    // Remember we have been registered.
    Initialized.store(ನಿಜ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  Timer_ಗುಂಪು::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  ಆದರೆ (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

ಶೂನ್ಯ llvm::EnableStatistics(ತಾರ್ಕಿಕ DoPrintOnExit) {
  Enabled = ನಿಜ;
  PrintOnExit = DoPrintOnExit;
}

ತಾರ್ಕಿಕ llvm::AreStatisticsEnabled() { ಹಿಂತಿರುಗಿ Enabled || EnableStats; }

ಶೂನ್ಯ StatisticInfo::ವಿಂಗಡಿಸು() {
  llvm::stable_ವಿಂಗಡಿಸು(
      Stats, [](ಸ್ಥಿರಾಂಕ TrackingStatistic *LHS, ಸ್ಥಿರಾಂಕ TrackingStatistic *RHS) {
        ಆದರೆ (ಪೂರ್ಣಾಂಕ Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          ಹಿಂತಿರುಗಿ Cmp < 0;

        ಆದರೆ (ಪೂರ್ಣಾಂಕ Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          ಹಿಂತಿರುಗಿ Cmp < 0;

        ಹಿಂತಿರುಗಿ std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

ಶೂನ್ಯ StatisticInfo::reset() {
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> Writer(*Stat_ಬೀಗ);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = ಸುಳ್ಳು;
    Stat->ಮೌಲ್ಯ = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

ಶೂನ್ಯ llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  ಚಿಹ್ನೆರಹಿತ MaxDebugTypeLen = 0, MaxValLen = 0;
  ಗಾಗಿ (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (ಚಿಹ್ನೆರಹಿತ)utostr(Stat->get_ಮೌಲ್ಯ()).ಗಾತ್ರ_3());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (ಚಿಹ್ನೆರಹಿತ)std::ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(Stat->getDebugType()));
  }

  Stats.ವಿಂಗಡಿಸು();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  ಗಾಗಿ (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_ಮೌಲ್ಯ(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

ಶೂನ್ಯ llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> Reader(*Stat_ಬೀಗ);
  StatisticInfo &Stats = *StatInfo;

  Stats.ವಿಂಗಡಿಸು();

  // Print all of the statistics.
  OS << "{\n";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *delim = "";
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    ಪ್ರತಿಜ್ಞಾ(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    ಪ್ರತಿಜ್ಞಾ(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_ಮೌಲ್ಯ();
    delim = ",\n";
  }
  // Print timers.
  Timer_ಗುಂಪು::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

ಶೂನ್ಯ llvm::PrintStatistics() {
#ಆದರೆ LLVM_ENABLE_STATS
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> Reader(*Stat_ಬೀಗ);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  ಆದರೆ (Stats.Stats.ಖಾಲಿಗಳು()) ಹಿಂತಿರುಗಿ;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> ಹೊರಹೋಗು_ಹರಿವು = CreateInfoOutputFile();
  ಆದರೆ (StatsAsJSON)
    PrintStatisticsJSON(*ಹೊರಹೋಗು_ಹರಿವು);
  ಇಲ್ಲದಿದ್ದರೆ
    PrintStatistics(*ಹೊರಹೋಗು_ಹರಿವು);

#ಇಲ್ಲದಿದ್ದರೆ
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  ಆದರೆ (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> ಹೊರಹೋಗು_ಹರಿವು = CreateInfoOutputFile();
    (*ಹೊರಹೋಗು_ಹರಿವು) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#ಆದರೆ_ಅಂತ್ಯ
}

std::ಸದಿಶ<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> Reader(*Stat_ಬೀಗ);
  std::ಸದಿಶ<std::pair<StringRef, uint64_t>> ReturnStats;

  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Stat : StatInfo->statistics())
    ReturnStats.emplace_ಹಿಂಭಾಗ(Stat->getName(), Stat->get_ಮೌಲ್ಯ());
  ಹಿಂತಿರುಗಿ ReturnStats;
}

ಶೂನ್ಯ llvm::ResetStatistics() {
  StatInfo->reset();
}
