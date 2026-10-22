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
استعمال نامخلا llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
ساکن منطقی EnableStats;
ساکن منطقی StatsAsJSON;
ساکن منطقی Enabled;
ساکن منطقی PrintOnExit;

خالی llvm::initStatisticOptions() {
  ساکن cl::opt<منطقی, سچ> رجسٹر_Enable_Stats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  ساکن cl::opt<منطقی, سچ> رجسٹر_Stats_As_Json{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

نامخلا {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
جماعت StatisticInfo {
  std::سمتیہ<TrackingStatistic *> Stats;

  دوست خالی llvm::PrintStatistics();
  دوست خالی llvm::PrintStatistics(raw_ostream &OS);
  دوست خالی llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  خالی ترتیب_دو();
عوامی:
  استعمال const_iterator = std::سمتیہ<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  خالی جوڑو_Statistic(TrackingStatistic *S) { Stats.ڈالو_عقب(S); }

  const_iterator begin() ثابت { واپس Stats.begin(); }
  const_iterator انتہا() ثابت { واپس Stats.انتہا(); }
  iterator_range<const_iterator> statistics() ثابت {
    واپس {begin(), انتہا()};
  }

  خالی reset();
};
} // end anonymous namespace

ساکن ManagedStatic<StatisticInfo> StatInfo;
ساکن ManagedStatic<sys::SmartMutex<سچ> > Stat_تالا;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
خالی TrackingStatistic::رجسٹر_Statistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  اگر (!Initialized.بھرو(std::memory_order_relaxed)) {
    sys::SmartMutex<سچ> &تالا_3 = *Stat_تالا;
    StatisticInfo &SI = *StatInfo;
    sys::Smart_Scoped_تالا<سچ> Writer(تالا_3);
    // Check Initialized again after acquiring the lock.
    اگر (Initialized.بھرو(std::memory_order_relaxed))
      واپس;
    اگر (EnableStats || Enabled)
      SI.جوڑو_Statistic(یہ);

    // Remember we have been registered.
    Initialized.store(سچ, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  Timer_گروہ::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  اگر (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

خالی llvm::EnableStatistics(منطقی DoPrintOnExit) {
  Enabled = سچ;
  PrintOnExit = DoPrintOnExit;
}

منطقی llvm::AreStatisticsEnabled() { واپس Enabled || EnableStats; }

خالی StatisticInfo::ترتیب_دو() {
  llvm::stable_ترتیب_دو(
      Stats, [](ثابت TrackingStatistic *LHS, ثابت TrackingStatistic *RHS) {
        اگر (صحیح Cmp = std::strcmp(LHS->getDebugType(), RHS->getDebugType()))
          واپس Cmp < 0;

        اگر (صحیح Cmp = std::strcmp(LHS->getName(), RHS->getName()))
          واپس Cmp < 0;

        واپس std::strcmp(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

خالی StatisticInfo::reset() {
  sys::Smart_Scoped_تالا<سچ> Writer(*Stat_تالا);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  برائے (خودکار *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = جھوٹ;
    Stat->قدر = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

خالی llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  غیرعلامتی MaxDebugTypeLen = 0, MaxValLen = 0;
  برائے (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (غیرعلامتی)utostr(Stat->get_قدر()).سائز_2());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (غیرعلامتی)std::لڑی_لمبائی(Stat->getDebugType()));
  }

  Stats.ترتیب_دو();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  برائے (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_قدر(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

خالی llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::Smart_Scoped_تالا<سچ> Reader(*Stat_تالا);
  StatisticInfo &Stats = *StatInfo;

  Stats.ترتیب_دو();

  // Print all of the statistics.
  OS << "{\n";
  ثابت حرف *delim = "";
  برائے (ثابت TrackingStatistic *Stat : Stats.Stats) {
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
  Timer_گروہ::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

خالی llvm::PrintStatistics() {
#اگر LLVM_ENABLE_STATS
  sys::Smart_Scoped_تالا<سچ> Reader(*Stat_تالا);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  اگر (Stats.Stats.خالی_خانے()) واپس;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> برآمد_دھارا = CreateInfoOutputFile();
  اگر (StatsAsJSON)
    PrintStatisticsJSON(*برآمد_دھارا);
  ورنہ
    PrintStatistics(*برآمد_دھارا);

#ورنہ
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  اگر (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> برآمد_دھارا = CreateInfoOutputFile();
    (*برآمد_دھارا) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#اگر_ختم
}

std::سمتیہ<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::Smart_Scoped_تالا<سچ> Reader(*Stat_تالا);
  std::سمتیہ<std::pair<StringRef, uint64_t>> ReturnStats;

  برائے (ثابت خودکار &Stat : StatInfo->statistics())
    ReturnStats.emplace_عقب(Stat->getName(), Stat->get_قدر());
  واپس ReturnStats;
}

خالی llvm::ResetStatistics() {
  StatInfo->reset();
}
