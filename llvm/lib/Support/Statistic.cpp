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

#अन्तर्भाव "llvm/ADT/Statistic.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव "llvm/Support/Timer.h"
#अन्तर्भाव "llvm/Support/YAMLTraits.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cstring>
using नामाकाश llvm;

/// -stats - Command line option to cause transformations to emit stats about
/// what they did.
///
स्थिर तार्किक EnableStats;
स्थिर तार्किक StatsAsJSON;
स्थिर तार्किक Enabled;
स्थिर तार्किक PrintOnExit;

शून्य llvm::initStatisticOptions() {
  स्थिर cl::opt<तार्किक, सत्य> registerEnableStats{
      "stats",
      cl::desc(
          "Enable statistics output from program (available with Asserts)"),
      cl::location(EnableStats), cl::Hidden};
  स्थिर cl::opt<तार्किक, सत्य> registerStatsAsJson{
      "stats-json", cl::desc("Display statistics as json data"),
      cl::location(StatsAsJSON), cl::Hidden};
}

नामाकाश {
/// This class is used in a ManagedStatic so that it is created on demand (when
/// the first statistic is bumped) and destroyed only when llvm_shutdown is
/// called. We print statistics from the destructor.
/// This class is also used to look up statistic values from applications that
/// use LLVM.
वर्ग StatisticInfo {
  std::vector<TrackingStatistic *> Stats;

  मित्र शून्य llvm::PrintStatistics();
  मित्र शून्य llvm::PrintStatistics(raw_ostream &OS);
  मित्र शून्य llvm::PrintStatisticsJSON(raw_ostream &OS);

  /// Sort statistics by debugtype,name,description.
  शून्य sort();
सार्वजनिक:
  using const_iterator = std::vector<TrackingStatistic *>::const_iterator;

  StatisticInfo();
  ~StatisticInfo();

  शून्य addStatistic(TrackingStatistic *S) { Stats.धकेलो_back(S); }

  const_iterator begin() ध्रुव { प्रत्यागम Stats.begin(); }
  const_iterator end() ध्रुव { प्रत्यागम Stats.end(); }
  iterator_range<const_iterator> statistics() ध्रुव {
    प्रत्यागम {begin(), end()};
  }

  शून्य reset();
};
} // end anonymous namespace

स्थिर ManagedStatic<StatisticInfo> StatInfo;
स्थिर ManagedStatic<sys::SmartMutex<सत्य> > StatLock;

/// RegisterStatistic - The first time a statistic is bumped, this method is
/// called.
शून्य TrackingStatistic::RegisterStatistic() {
  // If stats are enabled, inform StatInfo that this statistic should be
  // printed.
  // llvm_shutdown calls destructors while holding the ManagedStatic mutex.
  // These destructors end up calling PrintStatistics, which takes StatLock.
  // Since dereferencing StatInfo and StatLock can require taking the
  // ManagedStatic mutex, doing so with StatLock held would lead to a lock
  // order inversion. To avoid that, we dereference the ManagedStatics first,
  // and only take StatLock afterwards.
  यदि (!Initialized.load(std::memory_order_relaxed)) {
    sys::SmartMutex<सत्य> &Lock = *StatLock;
    StatisticInfo &SI = *StatInfo;
    sys::SmartScopedLock<सत्य> Writer(Lock);
    // Check Initialized again after acquiring the lock.
    यदि (Initialized.load(std::memory_order_relaxed))
      प्रत्यागम;
    यदि (EnableStats || Enabled)
      SI.addStatistic(एतत्);

    // Remember we have been registered.
    Initialized.store(सत्य, std::memory_order_release);
  }
}

StatisticInfo::StatisticInfo() {
  // Ensure that necessary timer global objects are created first so they are
  // destructed after us.
  TimerGroup::constructForStatistics();
}

// Print information when destroyed, iff command line option is specified.
StatisticInfo::~StatisticInfo() {
  यदि (EnableStats || PrintOnExit)
    llvm::PrintStatistics();
}

शून्य llvm::EnableStatistics(तार्किक DoPrintOnExit) {
  Enabled = सत्य;
  PrintOnExit = DoPrintOnExit;
}

तार्किक llvm::AreStatisticsEnabled() { प्रत्यागम Enabled || EnableStats; }

शून्य StatisticInfo::sort() {
  llvm::stable_sort(
      Stats, [](ध्रुव TrackingStatistic *LHS, ध्रुव TrackingStatistic *RHS) {
        यदि (पूर्णाङ्क Cmp = std::म_भेद(LHS->getDebugType(), RHS->getDebugType()))
          प्रत्यागम Cmp < 0;

        यदि (पूर्णाङ्क Cmp = std::म_भेद(LHS->getName(), RHS->getName()))
          प्रत्यागम Cmp < 0;

        प्रत्यागम std::म_भेद(LHS->getDesc(), RHS->getDesc()) < 0;
      });
}

शून्य StatisticInfo::reset() {
  sys::SmartScopedLock<सत्य> Writer(*StatLock);

  // Tell each statistic that it isn't registered so it has to register
  // again. We're holding the lock so it won't be able to do so until we're
  // finished. Once we've forced it to re-register (after we return), then zero
  // the value.
  प्रति (स्वतः *Stat : Stats) {
    // Value updates to a statistic that complete before this statement in the
    // iteration for that statistic will be lost as intended.
    Stat->Initialized = असत्य;
    Stat->मूल्य = 0;
  }

  // Clear the registration list and release the lock once we're done. Any
  // pending updates from other threads will safely take effect after we return.
  // That might not be what the user wants if they're measuring a compilation
  // but it's their responsibility to prevent concurrent compilations to make
  // a single compilation measurable.
  Stats.clear();
}

शून्य llvm::PrintStatistics(raw_ostream &OS) {
  StatisticInfo &Stats = *StatInfo;

  // Figure out how long the biggest Value and Name fields are.
  अचिन्हित MaxDebugTypeLen = 0, MaxValLen = 0;
  प्रति (TrackingStatistic *Stat : Stats.Stats) {
    MaxValLen = std::max(MaxValLen, (अचिन्हित)utostr(Stat->get_मूल्य()).size());
    MaxDebugTypeLen =
        std::max(MaxDebugTypeLen, (अचिन्हित)std::म_माप(Stat->getDebugType()));
  }

  Stats.sort();

  // Print out the statistics header...
  OS << "===" << std::string(73, '-') << "===\n"
     << "                          ... Statistics Collected ...\n"
     << "===" << std::string(73, '-') << "===\n\n";

  // Print all of the statistics.
  प्रति (TrackingStatistic *Stat : Stats.Stats)
    OS << format("%*" PRIu64 " %-*s - %s\n", MaxValLen, Stat->get_मूल्य(),
                 MaxDebugTypeLen, Stat->getDebugType(), Stat->getDesc());

  OS << '\n';  // Flush the output stream.
  OS.flush();
}

शून्य llvm::PrintStatisticsJSON(raw_ostream &OS) {
  sys::SmartScopedLock<सत्य> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  Stats.sort();

  // Print all of the statistics.
  OS << "{\n";
  ध्रुव वर्ण *delim = "";
  प्रति (ध्रुव TrackingStatistic *Stat : Stats.Stats) {
    OS << delim;
    प्रतिज्ञा(yaml::needsQuotes(Stat->getDebugType()) == yaml::QuotingType::None &&
           "Statistic group/type name is simple.");
    प्रतिज्ञा(yaml::needsQuotes(Stat->getName()) == yaml::QuotingType::None &&
           "Statistic name is simple");
    OS << "\t\"" << Stat->getDebugType() << '.' << Stat->getName() << "\": "
       << Stat->get_मूल्य();
    delim = ",\n";
  }
  // Print timers.
  TimerGroup::printAllJSONValues(OS, delim);

  OS << "\n}\n";
  OS.flush();
}

शून्य llvm::PrintStatistics() {
#यदि LLVM_ENABLE_STATS
  sys::SmartScopedLock<सत्य> Reader(*StatLock);
  StatisticInfo &Stats = *StatInfo;

  // Statistics not enabled?
  यदि (Stats.Stats.empty()) प्रत्यागम;

  // Get the stream to write to.
  std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
  यदि (StatsAsJSON)
    PrintStatisticsJSON(*OutStream);
  अन्यथा
    PrintStatistics(*OutStream);

#अन्यथा
  // Check if the -stats option is set instead of checking
  // !Stats.Stats.empty().  In release builds, Statistics operators
  // do nothing, so stats are never Registered.
  यदि (EnableStats) {
    // Get the stream to write to.
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    (*OutStream) << "Statistics are disabled.  "
                 << "Build with asserts or with -DLLVM_FORCE_ENABLE_STATS\n";
  }
#endif
}

std::vector<std::pair<StringRef, uint64_t>> llvm::GetStatistics() {
  sys::SmartScopedLock<सत्य> Reader(*StatLock);
  std::vector<std::pair<StringRef, uint64_t>> ReturnStats;

  प्रति (ध्रुव स्वतः &Stat : StatInfo->statistics())
    ReturnStats.emplace_back(Stat->getName(), Stat->get_मूल्य());
  प्रत्यागम ReturnStats;
}

शून्य llvm::ResetStatistics() {
  StatInfo->reset();
}
