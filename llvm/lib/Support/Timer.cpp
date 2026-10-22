//===-- Timer.cpp - Interval Timing Support -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file Interval Timing implementation.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Timer.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/Statistic.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signposts.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ತರ್ಭಾವ <optional>

#ಯದಿ HAVE_UNISTD_H
#ಅನ್ತರ್ಭಾವ <unistd.h>
#endif

#ifdef HAVE_PROC_PID_RUSAGE
#ಅನ್ತರ್ಭಾವ <libproc.h>
#endif

using ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
ನಾಮಾಕಾಶ {
ವರ್ಗ Name2PairMap;
}

ಸ್ಥಿರ std::string &libSupportInfoOutputFilename();
ಸ್ಥಿರ ತಾರ್ಕಿಕ trackSpace();
ಸ್ಥಿರ ತಾರ್ಕಿಕ sortTimers();
[[maybe_unused]]
ಸ್ಥಿರ SignpostEmitter &signposts();
ಸ್ಥಿರ sys::SmartMutex<ಸತ್ಯ> &timerLock();
ಸ್ಥಿರ TimerGroup &defaultTimerGroup();
ಸ್ಥಿರ Name2PairMap &namedGroupedTimers();
ಸ್ಥಿರ ತಾರ್ಕಿಕ isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  ಧ್ರುವ std::string &OutputFilename = libSupportInfoOutputFilename();
  ಯದಿ (OutputFilename.empty())
    ಪ್ರತ್ಯಾಗಮ std::make_unique<raw_fd_ostream>(2, ಅಸತ್ಯ); // stderr.
  ಯದಿ (OutputFilename == "-")
    ಪ್ರತ್ಯಾಗಮ std::make_unique<raw_fd_ostream>(1, ಅಸತ್ಯ); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_code EC;
  auto Result = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::OF_Append | sys::fs::OF_TextWithCRLF);
  ಯದಿ (!EC)
    ಪ್ರತ್ಯಾಗಮ Result;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  ಪ್ರತ್ಯಾಗಮ std::make_unique<raw_fd_ostream>(2, ಅಸತ್ಯ); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, defaultTimerGroup());
}

ಶೂನ್ಯ Timer::init(StringRef TimerName, StringRef TimerDescription,
                 TimerGroup &tg) {
  ಪ್ರತಿಜ್ಞಾ(!TG && "Timer already initialized");
  Name.assign(TimerName.begin(), TimerName.end());
  Description.assign(TimerDescription.begin(), TimerDescription.end());
  Running = Triggered = ಅಸತ್ಯ;
  TG = &tg;
  TG->addTimer(*ಏತತ್);
}

Timer::~Timer() {
  ಯದಿ (!TG) ಪ್ರತ್ಯಾಗಮ;  // Never initialized, or already cleared.
  TG->removeTimer(*ಏತತ್);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ size_t getMemUsage() {
  ಯದಿ (!trackSpace())
    ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sys::Process::GetMallocUsage();
}

ಸ್ಥಿರ uint64_t getCurInstructionsExecuted() {
#ಯದಿ defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  ಸಂರಚನಾ rusage_info_v4 ru;
  ಯದಿ (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    ಪ್ರತ್ಯಾಗಮ ru.ri_instructions;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ 0;
}

TimeRecord TimeRecord::getCurrentTime(ತಾರ್ಕಿಕ Start) {
  using Seconds = std::chrono::duration<ದ್ವಿಗುಣ, std::ratio<1>>;
  TimeRecord Result;
  sys::TimePoint<> now;
  std::chrono::nanoseconds user, sys;

  ಯದಿ (Start) {
    Result.MemUsed = getMemUsage();
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::GetTimeUsage(now, user, sys);
  } ಅನ್ಯಥಾ {
    sys::Process::GetTimeUsage(now, user, sys);
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    Result.MemUsed = getMemUsage();
  }

  Result.WallTime = Seconds(now.time_since_epoch()).ಎಣಿಕೆ();
  Result.UserTime = Seconds(user).ಎಣಿಕೆ();
  Result.SystemTime = Seconds(sys).ಎಣಿಕೆ();
  ಪ್ರತ್ಯಾಗಮ Result;
}

ಶೂನ್ಯ Timer::startTimer() {
  ಪ್ರತಿಜ್ಞಾ(!Running && "Cannot start a running timer");
  Running = Triggered = ಸತ್ಯ;
#ಯದಿ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().startInterval(ಏತತ್, getName());
#endif
  StartTime = TimeRecord::getCurrentTime(ಸತ್ಯ);
}

ಶೂನ್ಯ Timer::stopTimer() {
  ಪ್ರತಿಜ್ಞಾ(Running && "Cannot stop a paused timer");
  Running = ಅಸತ್ಯ;
  Time += TimeRecord::getCurrentTime(ಅಸತ್ಯ);
  Time -= StartTime;
#ಯದಿ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().endInterval(ಏತತ್, getName());
#endif
}

ಶೂನ್ಯ Timer::clear() {
  Running = Triggered = ಅಸತ್ಯ;
  Time = StartTime = TimeRecord();
}

ಶೂನ್ಯ Timer::yieldTo(Timer &O) {
  stopTimer();
  O.startTimer();
}

ಸ್ಥಿರ ಶೂನ್ಯ printVal(ದ್ವಿಗುಣ Val, ದ್ವಿಗುಣ Total, raw_ostream &OS) {
  ಯದಿ (Total < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  ಅನ್ಯಥಾ
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/Total);
}

ಶೂನ್ಯ TimeRecord::print(ಧ್ರುವ TimeRecord &Total, raw_ostream &OS) ಧ್ರುವ {
  ಯದಿ (Total.getUserTime())
    printVal(getUserTime(), Total.getUserTime(), OS);
  ಯದಿ (Total.getSystemTime())
    printVal(getSystemTime(), Total.getSystemTime(), OS);
  ಯದಿ (Total.getProcessTime())
    printVal(getProcessTime(), Total.getProcessTime(), OS);
  printVal(getWallTime(), Total.getWallTime(), OS);

  OS << "  ";

  ಯದಿ (Total.getMemUsed())
    OS << format("%9" PRId64 "  ", (int64_t)getMemUsed());
  ಯದಿ (Total.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

ನಾಮಾಕಾಶ {

using Name2TimerMap = StringMap<Timer>;

ವರ್ಗ Name2PairMap {
  StringMap<std::pair<TimerGroup*, Name2TimerMap> > Map;
ಸಾರ್ವಜನಿಕ:
  ~Name2PairMap() {
    ಪ್ರತಿ (StringMap<std::pair<TimerGroup*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.end(); I != E; ++I)
      delete I->second.first;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef GroupName,
             StringRef GroupDescription) {
    sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());

    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry =
        getGroupEntry(GroupName, GroupDescription);
    Timer &T = GroupEntry.second[Name];
    ಯದಿ (!T.isInitialized())
      T.init(Name, Description, *GroupEntry.first);
    ಪ್ರತ್ಯಾಗಮ T;
  }

  TimerGroup &getTimerGroup(StringRef GroupName, StringRef GroupDescription) {
    sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
    ಪ್ರತ್ಯಾಗಮ *getGroupEntry(GroupName, GroupDescription).first;
  }

ಗುಪ್ತ:
  std::pair<TimerGroup *, Name2TimerMap> &
  getGroupEntry(StringRef GroupName, StringRef GroupDescription) {
    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry = Map[GroupName];
    ಯದಿ (!GroupEntry.first)
      GroupEntry.first =
          ನೂತನ TimerGroup(GroupName, GroupDescription, /*PrintOnExit=*/ಸತ್ಯ);

    ಪ್ರತ್ಯಾಗಮ GroupEntry;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef GroupName,
                                   StringRef GroupDescription, ತಾರ್ಕಿಕ Enabled)
    : TimeRegion(!Enabled
                     ? nullptr
                     : &namedGroupedTimers().get(Name, Description, GroupName,
                                                 GroupDescription)) {}

TimerGroup &NamedRegionTimer::getNamedTimerGroup(StringRef GroupName,
                                                 StringRef GroupDescription) {
  ಪ್ರತ್ಯಾಗಮ namedGroupedTimers().getTimerGroup(GroupName, GroupDescription);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
ಸ್ಥಿರ TimerGroup *TimerGroupList = nullptr;

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       sys::SmartMutex<ಸತ್ಯ> &lock, ತಾರ್ಕಿಕ PrintOnExit)
    : Name(Name.begin(), Name.end()),
      Description(Description.begin(), Description.end()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::SmartScopedLock<ಸತ್ಯ> L(lock);
  ಯದಿ (TimerGroupList)
    TimerGroupList->Prev = &Next;
  Next = TimerGroupList;
  Prev = &TimerGroupList;
  TimerGroupList = ಏತತ್;
}

TimerGroup::TimerGroup(StringRef Name, StringRef Description, ತಾರ್ಕಿಕ PrintOnExit)
    : TimerGroup(Name, Description, timerLock(), PrintOnExit) {}

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       ಧ್ರುವ StringMap<TimeRecord> &Records, ತಾರ್ಕಿಕ PrintOnExit)
    : TimerGroup(Name, Description, PrintOnExit) {
  TimersToPrint.reserve(Records.size());
  ಪ್ರತಿ (ಧ್ರುವ auto &P : Records)
    TimersToPrint.emplace_back(P.get_ಮೌಲ್ಯ(), std::string(P.get_ಕೀಲಿ()),
                               std::string(P.get_ಕೀಲಿ()));
  ಪ್ರತಿಜ್ಞಾ(TimersToPrint.size() == Records.size() && "Size mismatch");
}

TimerGroup::~TimerGroup() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  ಯಾವತ್ (FirstTimer)
    removeTimer(*FirstTimer);

  ಯದಿ (!TimersToPrint.empty() && PrintOnExit) {
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    PrintQueuedTimers(*OutStream);
  }

  auto unlink = [&]() {
    *Prev = Next;
    ಯದಿ (Next)
      Next->Prev = Prev;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  ಯದಿ (!isTimerGlobalsConstructed()) {
    unlink();
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Remove the group from the TimerGroupList.
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
  unlink();
}

ಶೂನ್ಯ TimerGroup::removeTimer(Timer &T) {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());

  // If the timer was started, move its data to TimersToPrint.
  ಯದಿ (T.hasTriggered())
    TimersToPrint.emplace_back(T.Time, T.Name, T.Description);

  T.TG = nullptr;

  // Unlink the timer from our list.
  *T.Prev = T.Next;
  ಯದಿ (T.Next)
    T.Next->Prev = T.Prev;
}

ಶೂನ್ಯ TimerGroup::addTimer(Timer &T) {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());

  // Add the timer to our list.
  ಯದಿ (FirstTimer)
    FirstTimer->Prev = &T.Next;
  T.Next = FirstTimer;
  T.Prev = &FirstTimer;
  FirstTimer = &T;
}

ಶೂನ್ಯ TimerGroup::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  ಯದಿ (sortTimers())
    llvm::sort(TimersToPrint);

  TimeRecord Total;
  ಪ್ರತಿ (ಧ್ರುವ PrintRecord &Record : TimersToPrint)
    Total += Record.Time;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  unsigned Padding = (80-Description.length())/2;
  ಯದಿ (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  ಯದಿ (ಏತತ್ != &defaultTimerGroup())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 Total.getProcessTime(), Total.getWallTime());
  OS << '\n';

  ಯದಿ (Total.getUserTime())
    OS << "   ---User Time---";
  ಯದಿ (Total.getSystemTime())
    OS << "   --System Time--";
  ಯದಿ (Total.getProcessTime())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  ಯದಿ (Total.getMemUsed())
    OS << "  ---Mem---";
  ಯದಿ (Total.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  ಪ್ರತಿ (ಧ್ರುವ PrintRecord &Record : llvm::reverse(TimersToPrint)) {
    Record.Time.print(Total, OS);
    OS << Record.Description << '\n';
  }

  Total.print(Total, OS);
  OS << "Total\n\n";
  OS.flush();

  TimersToPrint.clear();
}

ಶೂನ್ಯ TimerGroup::prepareToPrintList(ತಾರ್ಕಿಕ ResetTime) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  ಪ್ರತಿ (Timer *T = FirstTimer; T; T = T->Next) {
    ಯದಿ (!T->hasTriggered()) ಅನುವರ್ತ;
    ತಾರ್ಕಿಕ WasRunning = T->isRunning();
    ಯದಿ (WasRunning)
      T->stopTimer();

    TimersToPrint.emplace_back(T->Time, T->Name, T->Description);

    ಯದಿ (ResetTime)
      T->clear();

    ಯದಿ (WasRunning)
      T->startTimer();
  }
}

ಶೂನ್ಯ TimerGroup::print(raw_ostream &OS, ತಾರ್ಕಿಕ ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
    prepareToPrintList(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  ಯದಿ (!TimersToPrint.empty())
    PrintQueuedTimers(OS);
}

ಶೂನ್ಯ TimerGroup::clear() {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
  ಪ್ರತಿ (Timer *T = FirstTimer; T; T = T->Next)
    T->clear();
}

ಶೂನ್ಯ TimerGroup::printAll(raw_ostream &OS) {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());

  ಪ್ರತಿ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->print(OS);
}

ಶೂನ್ಯ TimerGroup::clearAll() {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
  ಪ್ರತಿ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->clear();
}

ಶೂನ್ಯ TimerGroup::printJSONValue(raw_ostream &OS, ಧ್ರುವ PrintRecord &R,
                                ಧ್ರುವ ವರ್ಣ *suffix, ದ್ವಿಗುಣ ಮೌಲ್ಯ) {
  constexpr auto max_digits10 = std::numeric_limits<ದ್ವಿಗುಣ>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, ಮೌಲ್ಯ);
}

ಧ್ರುವ ವರ್ಣ *TimerGroup::printJSONValues(raw_ostream &OS, ಧ್ರುವ ವರ್ಣ *delim) {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());

  prepareToPrintList(ಅಸತ್ಯ);
  ಪ್ರತಿ (ಧ್ರುವ PrintRecord &R : TimersToPrint) {
    OS << delim;
    delim = ",\n";

    ಧ್ರುವ TimeRecord &T = R.Time;
    printJSONValue(OS, R, ".wall", T.getWallTime());
    OS << delim;
    printJSONValue(OS, R, ".user", T.getUserTime());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.getSystemTime());
    ಯದಿ (T.getMemUsed()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.getMemUsed());
    }
    ಯದಿ (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  TimersToPrint.clear();
  ಪ್ರತ್ಯಾಗಮ delim;
}

ಧ್ರುವ ವರ್ಣ *TimerGroup::printAllJSONValues(raw_ostream &OS, ಧ್ರುವ ವರ್ಣ *delim) {
  sys::SmartScopedLock<ಸತ್ಯ> L(timerLock());
  ಪ್ರತಿ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    delim = TG->printJSONValues(OS, delim);
  ಪ್ರತ್ಯಾಗಮ delim;
}

//===----------------------------------------------------------------------===//
// Timer Globals
//
// Previously, these were independent ManagedStatics. This led to bugs because
// there are dependencies between the globals, but no reliable mechanism to
// control relative lifetimes.
//
// Placing the globals within one class instance lets us control the lifetimes
// of the various data members and ensure that no global uses another that has
// been deleted.
//
// Globals fall into two categories. First are simple data types and
// command-line options. These are cheap to construct and/or required early
// during launch. They are created when the ManagedTimerGlobals singleton is
// constructed. Second are types that are more expensive to construct or not
// needed until later during compilation. These are lazily constructed in order
// to reduce launch time.
//===----------------------------------------------------------------------===//
ವರ್ಗ llvm::TimerGlobals {
ಸಾರ್ವಜನಿಕ:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, ಸತ್ಯ> InfoOutputFilename{
      "info-output-file", cl::ಮೌಲ್ಯ_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<ತಾರ್ಕಿಕ> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<ತಾರ್ಕಿಕ> SortTimers{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(ಸತ್ಯ), cl::Hidden};

  sys::SmartMutex<ಸತ್ಯ> TimerLock;
  TimerGroup DefaultTimerGroup{"misc", "Miscellaneous Ungrouped Timers",
                               TimerLock, /*PrintOnExit=*/ಸತ್ಯ};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [ಏತತ್]() { NamedGroupedTimersPtr.emplace(); });
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  }
};

ಸ್ಥಿರ ManagedStatic<TimerGlobals> ManagedTimerGlobals;

ಸ್ಥಿರ std::string &libSupportInfoOutputFilename() {
  ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
ಸ್ಥಿರ ತಾರ್ಕಿಕ trackSpace() { ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->TrackSpace; }
ಸ್ಥಿರ ತಾರ್ಕಿಕ sortTimers() { ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->SortTimers; }
ಸ್ಥಿರ SignpostEmitter &signposts() { ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->Signposts; }
ಸ್ಥಿರ sys::SmartMutex<ಸತ್ಯ> &timerLock() {
  ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->TimerLock;
}
ಸ್ಥಿರ TimerGroup &defaultTimerGroup() {
  ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals->DefaultTimerGroup;
}
ಸ್ಥಿರ Name2PairMap &namedGroupedTimers() {
  ಪ್ರತ್ಯಾಗಮ *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

ಶೂನ್ಯ llvm::initTimerOptions() { *ManagedTimerGlobals; }
ಶೂನ್ಯ TimerGroup::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

ಶೂನ್ಯ *TimerGroup::acquireTimerGlobals() { ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals.claim(); }

ಸ್ಥಿರ ತಾರ್ಕಿಕ isTimerGlobalsConstructed() {
  ಪ್ರತ್ಯಾಗಮ ManagedTimerGlobals.isConstructed();
}
