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

#समावेश "llvm/ADT/Statistic.h"

#समावेश "DebugOptions.h"

#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/CommandLine.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/Mutex.h"
#समावेश "llvm/Support/Timer.h"
#समावेश "llvm/Support/YAMLTraits.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <algorithm>
#समावेश <cstring>
using namespace llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
अटल bool EnableStats;
अटल bool StatsAsJSON;
अटल bool Enabled;
अटल bool PrintOnExit;

व्योम llvm::initStatisticOptions() {
  अटल cl::opt<bool, true> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  अटल cl::opt<bool, true> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

namespace {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
श्रेणी StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  मित्र व्योम llvm::PrintStatistics();
  मित्र व्योम llvm::PrintStatistics(raw_ostream &OS);
  मित्र व्योम llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  व्योम sort();
खुला:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  व्योम addStatistic(TrackingStatistic *S) { Stats.धकेलो_back(S); }

  const_iterator begin() स्थिर { वापस Stats.begin(); }
  const_iterator end() स्थिर { वापस Stats.end(); }
  iterator_range<const_iterator> statistics() स्थिर {
    वापस {begin(), end()};
  }

  व्योम reset();
};
} // end anonymous namespace

अटल ManagedStatic<StatisticInfo> StatInfo;
अटल ManagedStatic<sys::SmartMutex<true> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
व्योम TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  अगर (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<true> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<true> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    अगर (Initialized.load(std::memory_order_relaxed))
      वापस;
    अगर (EnableStats || Enabled)
      SI.addStatistic(यह);

    // Remember we have been registered.
    Initialized.store(true, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  अगर (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

व्योम llvm::EnableStatistics(bool DoPrintOnExit) {
  Enabled = true;
  PrintOnExit = DoPrintOnExit;
}

bool llvm::AreStatisticsEnabled() { वापस Enabled || EnableStats; }

व्योम StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](स्थिर TrackingStatistic *LHS, स्थिर TrackingStatistic *RHS) {
        अगर (पूर्णांक Cmp = std::म_भेद(LHS->getDebugType(), RHS->getDebugType()))
          वापस Cmp < 0;

        अगर (पूर्णांक Cmp = std::म_भेद(LHS->getName(), RHS->getName()))
          वापस Cmp < 0;

        वापस std::म_भेद(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

व्योम StatisticInfo::reset() {
  sys::SmartScopedLock<true> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  क्रम (स्वतः *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = false;
    Stat->मूल्य = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.सफा();
}

व्योम llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  अचिन्हित MaxDebugTypeLen = 0, MaxValLen = 0;
  क्रम (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (अचिन्हित)utostr(Stat->get_मूल्य()).आकार_2());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (अचिन्हित)std::म_माप(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  क्रम (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_मूल्य(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.सफाई();
}

व्योम llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<true> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  स्थिर अक्षर *delim = "";
  क्रम (स्थिर TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    निश्चित(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    निश्चित(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_मूल्य();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.सफाई();
}

व्योम llvm::PrintStatistics() {
#अगर LLVM_ENABLE_STATS
  sys::SmartScopedLock<true> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  अगर (Stats.Stats.empty()) वापस;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  अगर (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  अन्यथा
    PrintStatistics(*OutStream);

#अन्यथा
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  अगर (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#पूर्ण_अगर
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<true> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  क्रम (स्थिर स्वतः &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_मूल्य());
  वापस ReturnStats;
}

व्योम llvm::ResetStatistics() {
  StatInfo->reset();
}
