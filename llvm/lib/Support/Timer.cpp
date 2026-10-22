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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Timer.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Statistic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Mutex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signposts.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <limits>
#ଅନ୍ତର୍ଭାଵ <optional>

#ଯଦି HAVE_UNISTD_H
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#endif

#ifdef HAVE_PROC_PID_RUSAGE
#ଅନ୍ତର୍ଭାଵ <libproc.h>
#endif

using ନାମାକାଶ llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
ନାମାକାଶ {
ଶ୍ରେଣୀ Name2PairMap;
}

ସ୍ଥିର std::string &libSupportInfoOutputFilename();
ସ୍ଥିର ତାର୍କିକ trackSpace();
ସ୍ଥିର ତାର୍କିକ sortTimers();
[[maybe_unused]]
ସ୍ଥିର SignpostEmitter &signposts();
ସ୍ଥିର sys::SmartMutex<ସତ୍ୟ> &timerLock();
ସ୍ଥିର TimerGroup &defaultTimerGroup();
ସ୍ଥିର Name2PairMap &namedGroupedTimers();
ସ୍ଥିର ତାର୍କିକ isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  ଅଚଳ std::string &OutputFilename = libSupportInfoOutputFilename();
  ଯଦି (OutputFilename.empty())
    ଫେରନ୍ତୁ std::make_unique<raw_fd_ostream>(2, ମିଥ୍ୟା); // stderr.
  ଯଦି (OutputFilename == "-")
    ଫେରନ୍ତୁ std::make_unique<raw_fd_ostream>(1, ମିଥ୍ୟା); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_code EC;
  auto Result = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::OF_Append | sys::fs::OF_TextWithCRLF);
  ଯଦି (!EC)
    ଫେରନ୍ତୁ Result;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  ଫେରନ୍ତୁ std::make_unique<raw_fd_ostream>(2, ମିଥ୍ୟା); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

ଶୂନ୍ୟ Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, defaultTimerGroup());
}

ଶୂନ୍ୟ Timer::init(StringRef TimerName, StringRef TimerDescription,
                 TimerGroup &tg) {
  ପ୍ରତିଜ୍ଞା(!TG && "Timer already initialized");
  Name.assign(TimerName.begin(), TimerName.end());
  Description.assign(TimerDescription.begin(), TimerDescription.end());
  Running = Triggered = ମିଥ୍ୟା;
  TG = &tg;
  TG->addTimer(*ଏହି);
}

Timer::~Timer() {
  ଯଦି (!TG) ଫେରନ୍ତୁ;  // Never initialized, or already cleared.
  TG->removeTimer(*ଏହି);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ size_t getMemUsage() {
  ଯଦି (!trackSpace())
    ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sys::Process::GetMallocUsage();
}

ସ୍ଥିର uint64_t getCurInstructionsExecuted() {
#ଯଦି defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  ସଂରଚନା rusage_info_v4 ru;
  ଯଦି (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    ଫେରନ୍ତୁ ru.ri_instructions;
  }
#endif
  ଫେରନ୍ତୁ 0;
}

TimeRecord TimeRecord::getCurrentTime(ତାର୍କିକ Start) {
  using Seconds = std::chrono::duration<ଦ୍ୱିଗୁଣ, std::ratio<1>>;
  TimeRecord Result;
  sys::TimePoint<> now;
  std::chrono::nanoseconds user, sys;

  ଯଦି (Start) {
    Result.MemUsed = getMemUsage();
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::GetTimeUsage(now, user, sys);
  } ଅନ୍ୟଥା {
    sys::Process::GetTimeUsage(now, user, sys);
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    Result.MemUsed = getMemUsage();
  }

  Result.WallTime = Seconds(now.time_since_epoch()).ଗଣନା_2();
  Result.UserTime = Seconds(user).ଗଣନା_2();
  Result.SystemTime = Seconds(sys).ଗଣନା_2();
  ଫେରନ୍ତୁ Result;
}

ଶୂନ୍ୟ Timer::startTimer() {
  ପ୍ରତିଜ୍ଞା(!Running && "Cannot start a running timer");
  Running = Triggered = ସତ୍ୟ;
#ଯଦି LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().startInterval(ଏହି, getName());
#endif
  StartTime = TimeRecord::getCurrentTime(ସତ୍ୟ);
}

ଶୂନ୍ୟ Timer::stopTimer() {
  ପ୍ରତିଜ୍ଞା(Running && "Cannot stop a paused timer");
  Running = ମିଥ୍ୟା;
  Time += TimeRecord::getCurrentTime(ମିଥ୍ୟା);
  Time -= StartTime;
#ଯଦି LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().endInterval(ଏହି, getName());
#endif
}

ଶୂନ୍ୟ Timer::clear() {
  Running = Triggered = ମିଥ୍ୟା;
  Time = StartTime = TimeRecord();
}

ଶୂନ୍ୟ Timer::yieldTo(Timer &O) {
  stopTimer();
  O.startTimer();
}

ସ୍ଥିର ଶୂନ୍ୟ printVal(ଦ୍ୱିଗୁଣ Val, ଦ୍ୱିଗୁଣ Total, raw_ostream &OS) {
  ଯଦି (Total < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  ଅନ୍ୟଥା
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/Total);
}

ଶୂନ୍ୟ TimeRecord::print(ଅଚଳ TimeRecord &Total, raw_ostream &OS) ଅଚଳ {
  ଯଦି (Total.getUserTime())
    printVal(getUserTime(), Total.getUserTime(), OS);
  ଯଦି (Total.getSystemTime())
    printVal(getSystemTime(), Total.getSystemTime(), OS);
  ଯଦି (Total.getProcessTime())
    printVal(getProcessTime(), Total.getProcessTime(), OS);
  printVal(getWallTime(), Total.getWallTime(), OS);

  OS << "  ";

  ଯଦି (Total.getMemUsed())
    OS << format("%9" PRId64 "  ", (int64_t)getMemUsed());
  ଯଦି (Total.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

ନାମାକାଶ {

using Name2TimerMap = StringMap<Timer>;

ଶ୍ରେଣୀ Name2PairMap {
  StringMap<std::pair<TimerGroup*, Name2TimerMap> > Map;
ସର୍ବସାଧାରଣ:
  ~Name2PairMap() {
    ପାଇଁ (StringMap<std::pair<TimerGroup*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.end(); I != E; ++I)
      delete I->second.first;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef GroupName,
             StringRef GroupDescription) {
    sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());

    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry =
        getGroupEntry(GroupName, GroupDescription);
    Timer &T = GroupEntry.second[Name];
    ଯଦି (!T.isInitialized())
      T.init(Name, Description, *GroupEntry.first);
    ଫେରନ୍ତୁ T;
  }

  TimerGroup &getTimerGroup(StringRef GroupName, StringRef GroupDescription) {
    sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
    ଫେରନ୍ତୁ *getGroupEntry(GroupName, GroupDescription).first;
  }

ବ୍ୟକ୍ତିଗତ:
  std::pair<TimerGroup *, Name2TimerMap> &
  getGroupEntry(StringRef GroupName, StringRef GroupDescription) {
    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry = Map[GroupName];
    ଯଦି (!GroupEntry.first)
      GroupEntry.first =
          ନୂତନ TimerGroup(GroupName, GroupDescription, /*PrintOnExit=*/ସତ୍ୟ);

    ଫେରନ୍ତୁ GroupEntry;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef GroupName,
                                   StringRef GroupDescription, ତାର୍କିକ Enabled)
    : TimeRegion(!Enabled
                     ? nullptr
                     : &namedGroupedTimers().get(Name, Description, GroupName,
                                                 GroupDescription)) {}

TimerGroup &NamedRegionTimer::getNamedTimerGroup(StringRef GroupName,
                                                 StringRef GroupDescription) {
  ଫେରନ୍ତୁ namedGroupedTimers().getTimerGroup(GroupName, GroupDescription);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
ସ୍ଥିର TimerGroup *TimerGroupList = nullptr;

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       sys::SmartMutex<ସତ୍ୟ> &lock, ତାର୍କିକ PrintOnExit)
    : Name(Name.begin(), Name.end()),
      Description(Description.begin(), Description.end()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::SmartScopedLock<ସତ୍ୟ> L(lock);
  ଯଦି (TimerGroupList)
    TimerGroupList->Prev = &Next;
  Next = TimerGroupList;
  Prev = &TimerGroupList;
  TimerGroupList = ଏହି;
}

TimerGroup::TimerGroup(StringRef Name, StringRef Description, ତାର୍କିକ PrintOnExit)
    : TimerGroup(Name, Description, timerLock(), PrintOnExit) {}

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       ଅଚଳ StringMap<TimeRecord> &Records, ତାର୍କିକ PrintOnExit)
    : TimerGroup(Name, Description, PrintOnExit) {
  TimersToPrint.reserve(Records.size());
  ପାଇଁ (ଅଚଳ auto &P : Records)
    TimersToPrint.emplace_back(P.get_ମୂଲ୍ୟ(), std::string(P.get_ଚାବି()),
                               std::string(P.get_ଚାବି()));
  ପ୍ରତିଜ୍ଞା(TimersToPrint.size() == Records.size() && "Size mismatch");
}

TimerGroup::~TimerGroup() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  ଯେତେବେଳେ (FirstTimer)
    removeTimer(*FirstTimer);

  ଯଦି (!TimersToPrint.empty() && PrintOnExit) {
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    PrintQueuedTimers(*OutStream);
  }

  auto unlink = [&]() {
    *Prev = Next;
    ଯଦି (Next)
      Next->Prev = Prev;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  ଯଦି (!isTimerGlobalsConstructed()) {
    unlink();
    ଫେରନ୍ତୁ;
  }

  // Remove the group from the TimerGroupList.
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
  unlink();
}

ଶୂନ୍ୟ TimerGroup::removeTimer(Timer &T) {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());

  // If the timer was started, move its data to TimersToPrint.
  ଯଦି (T.hasTriggered())
    TimersToPrint.emplace_back(T.Time, T.Name, T.Description);

  T.TG = nullptr;

  // Unlink the timer from our list.
  *T.Prev = T.Next;
  ଯଦି (T.Next)
    T.Next->Prev = T.Prev;
}

ଶୂନ୍ୟ TimerGroup::addTimer(Timer &T) {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());

  // Add the timer to our list.
  ଯଦି (FirstTimer)
    FirstTimer->Prev = &T.Next;
  T.Next = FirstTimer;
  T.Prev = &FirstTimer;
  FirstTimer = &T;
}

ଶୂନ୍ୟ TimerGroup::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  ଯଦି (sortTimers())
    llvm::sort(TimersToPrint);

  TimeRecord Total;
  ପାଇଁ (ଅଚଳ PrintRecord &Record : TimersToPrint)
    Total += Record.Time;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  unsigned Padding = (80-Description.length())/2;
  ଯଦି (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  ଯଦି (ଏହି != &defaultTimerGroup())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 Total.getProcessTime(), Total.getWallTime());
  OS << '\n';

  ଯଦି (Total.getUserTime())
    OS << "   ---User Time---";
  ଯଦି (Total.getSystemTime())
    OS << "   --System Time--";
  ଯଦି (Total.getProcessTime())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  ଯଦି (Total.getMemUsed())
    OS << "  ---Mem---";
  ଯଦି (Total.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  ପାଇଁ (ଅଚଳ PrintRecord &Record : llvm::reverse(TimersToPrint)) {
    Record.Time.print(Total, OS);
    OS << Record.Description << '\n';
  }

  Total.print(Total, OS);
  OS << "Total\n\n";
  OS.flush();

  TimersToPrint.clear();
}

ଶୂନ୍ୟ TimerGroup::prepareToPrintList(ତାର୍କିକ ResetTime) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  ପାଇଁ (Timer *T = FirstTimer; T; T = T->Next) {
    ଯଦି (!T->hasTriggered()) ଜାରି;
    ତାର୍କିକ WasRunning = T->isRunning();
    ଯଦି (WasRunning)
      T->stopTimer();

    TimersToPrint.emplace_back(T->Time, T->Name, T->Description);

    ଯଦି (ResetTime)
      T->clear();

    ଯଦି (WasRunning)
      T->startTimer();
  }
}

ଶୂନ୍ୟ TimerGroup::print(raw_ostream &OS, ତାର୍କିକ ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
    prepareToPrintList(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  ଯଦି (!TimersToPrint.empty())
    PrintQueuedTimers(OS);
}

ଶୂନ୍ୟ TimerGroup::clear() {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
  ପାଇଁ (Timer *T = FirstTimer; T; T = T->Next)
    T->clear();
}

ଶୂନ୍ୟ TimerGroup::printAll(raw_ostream &OS) {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());

  ପାଇଁ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->print(OS);
}

ଶୂନ୍ୟ TimerGroup::clearAll() {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
  ପାଇଁ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->clear();
}

ଶୂନ୍ୟ TimerGroup::printJSONValue(raw_ostream &OS, ଅଚଳ PrintRecord &R,
                                ଅଚଳ ଅକ୍ଷର *suffix, ଦ୍ୱିଗୁଣ ମୂଲ୍ୟ) {
  constexpr auto max_digits10 = std::numeric_limits<ଦ୍ୱିଗୁଣ>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, ମୂଲ୍ୟ);
}

ଅଚଳ ଅକ୍ଷର *TimerGroup::printJSONValues(raw_ostream &OS, ଅଚଳ ଅକ୍ଷର *delim) {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());

  prepareToPrintList(ମିଥ୍ୟା);
  ପାଇଁ (ଅଚଳ PrintRecord &R : TimersToPrint) {
    OS << delim;
    delim = ",\n";

    ଅଚଳ TimeRecord &T = R.Time;
    printJSONValue(OS, R, ".wall", T.getWallTime());
    OS << delim;
    printJSONValue(OS, R, ".user", T.getUserTime());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.getSystemTime());
    ଯଦି (T.getMemUsed()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.getMemUsed());
    }
    ଯଦି (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  TimersToPrint.clear();
  ଫେରନ୍ତୁ delim;
}

ଅଚଳ ଅକ୍ଷର *TimerGroup::printAllJSONValues(raw_ostream &OS, ଅଚଳ ଅକ୍ଷର *delim) {
  sys::SmartScopedLock<ସତ୍ୟ> L(timerLock());
  ପାଇଁ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    delim = TG->printJSONValues(OS, delim);
  ଫେରନ୍ତୁ delim;
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
ଶ୍ରେଣୀ llvm::TimerGlobals {
ସର୍ବସାଧାରଣ:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, ସତ୍ୟ> InfoOutputFilename{
      "info-output-file", cl::ମୂଲ୍ୟ_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<ତାର୍କିକ> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<ତାର୍କିକ> SortTimers{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(ସତ୍ୟ), cl::Hidden};

  sys::SmartMutex<ସତ୍ୟ> TimerLock;
  TimerGroup DefaultTimerGroup{"misc", "Miscellaneous Ungrouped Timers",
                               TimerLock, /*PrintOnExit=*/ସତ୍ୟ};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [ଏହି]() { NamedGroupedTimersPtr.emplace(); });
    ଫେରନ୍ତୁ *ଏହି;
  }
};

ସ୍ଥିର ManagedStatic<TimerGlobals> ManagedTimerGlobals;

ସ୍ଥିର std::string &libSupportInfoOutputFilename() {
  ଫେରନ୍ତୁ ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
ସ୍ଥିର ତାର୍କିକ trackSpace() { ଫେରନ୍ତୁ ManagedTimerGlobals->TrackSpace; }
ସ୍ଥିର ତାର୍କିକ sortTimers() { ଫେରନ୍ତୁ ManagedTimerGlobals->SortTimers; }
ସ୍ଥିର SignpostEmitter &signposts() { ଫେରନ୍ତୁ ManagedTimerGlobals->Signposts; }
ସ୍ଥିର sys::SmartMutex<ସତ୍ୟ> &timerLock() {
  ଫେରନ୍ତୁ ManagedTimerGlobals->TimerLock;
}
ସ୍ଥିର TimerGroup &defaultTimerGroup() {
  ଫେରନ୍ତୁ ManagedTimerGlobals->DefaultTimerGroup;
}
ସ୍ଥିର Name2PairMap &namedGroupedTimers() {
  ଫେରନ୍ତୁ *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

ଶୂନ୍ୟ llvm::initTimerOptions() { *ManagedTimerGlobals; }
ଶୂନ୍ୟ TimerGroup::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

ଶୂନ୍ୟ *TimerGroup::acquireTimerGlobals() { ଫେରନ୍ତୁ ManagedTimerGlobals.claim(); }

ସ୍ଥିର ତାର୍କିକ isTimerGlobalsConstructed() {
  ଫେରନ୍ତୁ ManagedTimerGlobals.isConstructed();
}
