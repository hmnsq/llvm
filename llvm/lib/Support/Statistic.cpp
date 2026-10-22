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
استعمال نطاق llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ساكن منطقي EnableStats;
ساكن منطقي StatsAsJSON;
ساكن منطقي Enabled;
ساكن منطقي PrintOnExit;

فراغ llvm::initStatisticOptions() {
  ساكن cl::opt<منطقي, سچ> رجسٽر_Enable_Stats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ساكن cl::opt<منطقي, سچ> رجسٽر_Stats_As_Json{
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
  std::ويڪٽر<TrackingStatistic *> Stats;

  دوست فراغ llvm::PrintStatistics();
  دوست فراغ llvm::PrintStatistics(raw_ostream &OS);
  دوست فراغ llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  فراغ ترتيب_ڏيو();
عام:
  استعمال const_iterator = std::ويڪٽر<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  فراغ جوڙيو_Statistic(TrackingStatistic *S) { Stats.ڌڪو_پٺ(S); }

  const_iterator begin() ثابت { أرجع Stats.begin(); }
  const_iterator پڇاڙي() ثابت { أرجع Stats.پڇاڙي(); }
  iterator_range<const_iterator> statistics() ثابت {
    أرجع {begin(), پڇاڙي()};
  }

  فراغ reset();
};
} // end anonymous namespace

ساكن ManagedStatic<StatisticInfo> StatInfo;
ساكن ManagedStatic<sys::SmartMutex<سچ> > Stat_تالو;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
فراغ TrackingStatistic::رجسٽر_Statistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  إذا (!Initialized.ڀريو(std::memory_order_relaxed)) {
    sys::SmartMutex<سچ> &تالو_2 = *Stat_تالو;
    StatisticInfo &SI = *StatInfo;
    sys::Smart_Scoped_تالو<سچ> Writer(تالو_2);
    // Check Initialized again after acquiring the lock.
    إذا (Initialized.ڀريو(std::memory_order_relaxed))
      أرجع;
    إذا (EnableStats || Enabled)
      SI.جوڙيو_Statistic(هذا);

    // Remember we have been registered.
    Initialized.store(سچ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  Timer_گروهه::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  إذا (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

فراغ llvm::EnableStatistics(منطقي DoPrintOnExit) {
  Enabled = سچ;
  PrintOnExit = DoPrintOnExit;
}

منطقي llvm::AreStatisticsEnabled() { أرجع Enabled || EnableStats; }

فراغ StatisticInfo::ترتيب_ڏيو() {
  llvm::stable_ترتيب_ڏيو(
      Stats, [](ثابت TrackingStatistic *LHS, ثابت TrackingStatistic *RHS) {
        إذا (صحيح Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          أرجع Cmp < 0;

        إذا (صحيح Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          أرجع Cmp < 0;

        أرجع std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

فراغ StatisticInfo::reset() {
  sys::Smart_Scoped_تالو<سچ> Writer(*Stat_تالو);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  لكل (خودڪار *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = خطأ;
    Stat->قدر = 0;
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
  بنانشان MaxDebugTypeLen = 0, MaxValLen = 0;
  لكل (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (بنانشان)utostr(Stat->get_قدر()).سائيز_2());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (بنانشان)std::لڙي_ڊيگهه(Stat->getDebugType()));
  }

  Stats.ترتيب_ڏيو();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  لكل (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_قدر(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

فراغ llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::Smart_Scoped_تالو<سچ> Reader(*Stat_تالو);
  StatisticInfo &Stats = *StatInfo;

  Stats.ترتيب_ڏيو();

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
       << Stat->get_قدر();
    delim = ",\n";
  }
  // Print timers.
  Timer_گروهه::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

فراغ llvm::PrintStatistics() {
#إذا LLVM_ENABLE_STATS
  sys::Smart_Scoped_تالو<سچ> Reader(*Stat_تالو);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  إذا (Stats.Stats.خالي_جايون()) أرجع;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> ٻاهريون_وهڪرو = CreateInfoOutputFile();
  إذا (StatsAsJSON)
    PrintStatisticsJSON(*ٻاهريون_وهڪرو);
  وإلا
    PrintStatistics(*ٻاهريون_وهڪرو);

#وإلا
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  إذا (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> ٻاهريون_وهڪرو = CreateInfoOutputFile();
    (*ٻاهريون_وهڪرو) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#جيڪڏهن_پورو
}

std::ويڪٽر<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::Smart_Scoped_تالو<سچ> Reader(*Stat_تالو);
  std::ويڪٽر<std::pair<StringRef, uint64_t>> ReturnStats;

  لكل (ثابت خودڪار &Stat : StatInfo->statistics())
    ReturnStats.emplace_پٺ(Stat->getName(), Stat->get_قدر());
  أرجع ReturnStats;
}

فراغ llvm::ResetStatistics() {
  StatInfo->reset();
}
