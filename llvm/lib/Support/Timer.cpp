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

#అన్తర్భావ "llvm/Support/Timer.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm/ADT/Statistic.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/Mutex.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Signposts.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <limits>
#అన్తర్భావ <optional>

#ఒకవేళ HAVE_UNISTD_H
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_PROC_PID_RUSAGE
#అన్తర్భావ <libproc.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
నామాకాశ {
తరగతి Name2PairMap;
}

స్థిర std::string &libSupportInfoOutputFilename();
స్థిర తార్కిక trackSpace();
స్థిర తార్కిక క్రమీకరించు_Timers();
[[maybe_వాడని]]
స్థిర SignpostEmitter &signposts();
స్థిర sys::SmartMutex<నిజం> &timer_తాళం();
స్థిర Timer_సమూహం &default_Timer_సమూహం();
స్థిర Name2PairMap &namedGroupedTimers();
స్థిర తార్కిక isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  స్థిరరాశి std::string &OutputFilename = libSupportInfoOutputFilename();
  ఒకవేళ (OutputFilename.ఖాళీలు())
    తిరిగి std::make_unique<raw_fd_ostream>(2, అబద్ధం); // stderr.
  ఒకవేళ (OutputFilename == "-")
    తిరిగి std::make_unique<raw_fd_ostream>(1, అబద్ధం); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_సంకేతం EC;
  స్వయంచాలక ఫలితం = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::యొక్క_Append | sys::fs::యొక్క_పాఠ్యం_With_CRLF);
  ఒకవేళ (!EC)
    తిరిగి ఫలితం;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  తిరిగి std::make_unique<raw_fd_ostream>(2, అబద్ధం); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

శూన్య Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, default_Timer_సమూహం());
}

శూన్య Timer::init(StringRef TimerName, StringRef TimerDescription,
                 Timer_సమూహం &tg) {
  ప్రతిజ్ఞా(!TG && "Timer already initialized");
  Name.కేటాయించు(TimerName.begin(), TimerName.చివర());
  Description.కేటాయించు(TimerDescription.begin(), TimerDescription.చివర());
  Running = Triggered = అబద్ధం;
  TG = &tg;
  TG->కూడు_Timer(*ఇది);
}

Timer::~Timer() {
  ఒకవేళ (!TG) తిరిగి;  // Never initialized, or already cleared.
  TG->removeTimer(*ఇది);
}

స్థిర అంతఃవరుస size_t getMemUsage() {
  ఒకవేళ (!trackSpace())
    తిరిగి 0;
  తిరిగి sys::Process::GetMallocUsage();
}

స్థిర uint64_t getCurInstructionsExecuted() {
#ఒకవేళ defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  నిర్మాణం rusage_info_v4 ru;
  ఒకవేళ (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    తిరిగి ru.ri_instructions;
  }
#అయితే_ముగింపు
  తిరిగి 0;
}

సమయం_రికార్డు సమయం_రికార్డు::get_Current_సమయం(తార్కిక ప్రారంభం) {
  ఉపయోగించి Seconds = std::chrono::duration<ద్విగుణ, std::నిష్పత్తి<1>>;
  సమయం_రికార్డు ఫలితం;
  sys::సమయం_బిందువు<> now;
  std::chrono::nanoseconds user, sys;

  ఒకవేళ (ప్రారంభం) {
    ఫలితం.Mem_వాడిన = getMemUsage();
    ఫలితం.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::Get_సమయం_Usage(now, user, sys);
  } లేకపోతే {
    sys::Process::Get_సమయం_Usage(now, user, sys);
    ఫలితం.InstructionsExecuted = getCurInstructionsExecuted();
    ఫలితం.Mem_వాడిన = getMemUsage();
  }

  ఫలితం.Wall_సమయం = Seconds(now.సమయం_since_epoch()).లెక్క();
  ఫలితం.User_సమయం = Seconds(user).లెక్క();
  ఫలితం.System_సమయం = Seconds(sys).లెక్క();
  తిరిగి ఫలితం;
}

శూన్య Timer::ప్రారంభం_Timer() {
  ప్రతిజ్ఞా(!Running && "Cannot start a running timer");
  Running = Triggered = నిజం;
#ఒకవేళ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().ప్రారంభం_Interval(ఇది, getName());
#అయితే_ముగింపు
  ప్రారంభం_సమయం = సమయం_రికార్డు::get_Current_సమయం(నిజం);
}

శూన్య Timer::stopTimer() {
  ప్రతిజ్ఞా(Running && "Cannot stop a paused timer");
  Running = అబద్ధం;
  సమయం += సమయం_రికార్డు::get_Current_సమయం(అబద్ధం);
  సమయం -= ప్రారంభం_సమయం;
#ఒకవేళ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().చివర_Interval(ఇది, getName());
#అయితే_ముగింపు
}

శూన్య Timer::clear() {
  Running = Triggered = అబద్ధం;
  సమయం = ప్రారంభం_సమయం = సమయం_రికార్డు();
}

శూన్య Timer::yield_గమ్యం(Timer &O) {
  stopTimer();
  O.ప్రారంభం_Timer();
}

స్థిర శూన్య printVal(ద్విగుణ Val, ద్విగుణ మొత్తం, raw_ostream &OS) {
  ఒకవేళ (మొత్తం < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  లేకపోతే
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/మొత్తం);
}

శూన్య సమయం_రికార్డు::print(స్థిరరాశి సమయం_రికార్డు &మొత్తం, raw_ostream &OS) స్థిరరాశి {
  ఒకవేళ (మొత్తం.get_User_సమయం())
    printVal(get_User_సమయం(), మొత్తం.get_User_సమయం(), OS);
  ఒకవేళ (మొత్తం.get_System_సమయం())
    printVal(get_System_సమయం(), మొత్తం.get_System_సమయం(), OS);
  ఒకవేళ (మొత్తం.get_Process_సమయం())
    printVal(get_Process_సమయం(), మొత్తం.get_Process_సమయం(), OS);
  printVal(get_Wall_సమయం(), మొత్తం.get_Wall_సమయం(), OS);

  OS << "  ";

  ఒకవేళ (మొత్తం.get_Mem_వాడిన())
    OS << format("%9" PRId64 "  ", (int64_t)get_Mem_వాడిన());
  ఒకవేళ (మొత్తం.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

నామాకాశ {

ఉపయోగించి Name2TimerMap = StringMap<Timer>;

తరగతి Name2PairMap {
  StringMap<std::pair<Timer_సమూహం*, Name2TimerMap> > Map;
బహిరంగ:
  ~Name2PairMap() {
    కొరకు (StringMap<std::pair<Timer_సమూహం*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.చివర(); I != E; ++I)
      తొలగించు I->రెండవ_2.మొదటి_2;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef సమూహం_Name,
             StringRef సమూహం_Description) {
    sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());

    std::pair<Timer_సమూహం *, Name2TimerMap> &సమూహం_నమోదు =
        get_సమూహం_నమోదు(సమూహం_Name, సమూహం_Description);
    Timer &T = సమూహం_నమోదు.రెండవ_2[Name];
    ఒకవేళ (!T.isInitialized())
      T.init(Name, Description, *సమూహం_నమోదు.మొదటి_2);
    తిరిగి T;
  }

  Timer_సమూహం &get_Timer_సమూహం(StringRef సమూహం_Name, StringRef సమూహం_Description) {
    sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
    తిరిగి *get_సమూహం_నమోదు(సమూహం_Name, సమూహం_Description).మొదటి_2;
  }

అంతరంగ:
  std::pair<Timer_సమూహం *, Name2TimerMap> &
  get_సమూహం_నమోదు(StringRef సమూహం_Name, StringRef సమూహం_Description) {
    std::pair<Timer_సమూహం *, Name2TimerMap> &సమూహం_నమోదు = Map[సమూహం_Name];
    ఒకవేళ (!సమూహం_నమోదు.మొదటి_2)
      సమూహం_నమోదు.మొదటి_2 =
          కొత్త Timer_సమూహం(సమూహం_Name, సమూహం_Description, /*PrintOnExit=*/నిజం);

    తిరిగి సమూహం_నమోదు;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef సమూహం_Name,
                                   StringRef సమూహం_Description, తార్కిక Enabled)
    : సమయం_Region(!Enabled
                     ? శూన్యసూచిక
                     : &namedGroupedTimers().get(Name, Description, సమూహం_Name,
                                                 సమూహం_Description)) {}

Timer_సమూహం &NamedRegionTimer::get_Named_Timer_సమూహం(StringRef సమూహం_Name,
                                                 StringRef సమూహం_Description) {
  తిరిగి namedGroupedTimers().get_Timer_సమూహం(సమూహం_Name, సమూహం_Description);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
స్థిర Timer_సమూహం *Timer_సమూహం_List = శూన్యసూచిక;

Timer_సమూహం::Timer_సమూహం(StringRef Name, StringRef Description,
                       sys::SmartMutex<నిజం> &తాళం_2, తార్కిక PrintOnExit)
    : Name(Name.begin(), Name.చివర()),
      Description(Description.begin(), Description.చివర()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::Smart_Scoped_తాళం<నిజం> L(తాళం_2);
  ఒకవేళ (Timer_సమూహం_List)
    Timer_సమూహం_List->Prev = &తదుపరి_2;
  తదుపరి_2 = Timer_సమూహం_List;
  Prev = &Timer_సమూహం_List;
  Timer_సమూహం_List = ఇది;
}

Timer_సమూహం::Timer_సమూహం(StringRef Name, StringRef Description, తార్కిక PrintOnExit)
    : Timer_సమూహం(Name, Description, timer_తాళం(), PrintOnExit) {}

Timer_సమూహం::Timer_సమూహం(StringRef Name, StringRef Description,
                       స్థిరరాశి StringMap<సమయం_రికార్డు> &రికార్డులు, తార్కిక PrintOnExit)
    : Timer_సమూహం(Name, Description, PrintOnExit) {
  Timers_గమ్యం_Print.reserve(రికార్డులు.కొలత_2());
  కొరకు (స్థిరరాశి స్వయంచాలక &P : రికార్డులు)
    Timers_గమ్యం_Print.emplace_వెనుక(P.get_విలువ(), std::string(P.get_కీలకం()),
                               std::string(P.get_కీలకం()));
  ప్రతిజ్ఞా(Timers_గమ్యం_Print.కొలత_2() == రికార్డులు.కొలత_2() && "Size mismatch");
}

Timer_సమూహం::~Timer_సమూహం() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  అయితే (మొదటి_Timer)
    removeTimer(*మొదటి_Timer);

  ఒకవేళ (!Timers_గమ్యం_Print.ఖాళీలు() && PrintOnExit) {
    std::unique_ptr<raw_ostream> బహిర్గతం_ప్రవాహం = CreateInfoOutputFile();
    PrintQueuedTimers(*బహిర్గతం_ప్రవాహం);
  }

  స్వయంచాలక unlink = [&]() {
    *Prev = తదుపరి_2;
    ఒకవేళ (తదుపరి_2)
      తదుపరి_2->Prev = Prev;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  ఒకవేళ (!isTimerGlobalsConstructed()) {
    unlink();
    తిరిగి;
  }

  // Remove the group from the TimerGroupList.
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
  unlink();
}

శూన్య Timer_సమూహం::removeTimer(Timer &T) {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());

  // If the timer was started, move its data to TimersToPrint.
  ఒకవేళ (T.hasTriggered())
    Timers_గమ్యం_Print.emplace_వెనుక(T.సమయం, T.Name, T.Description);

  T.TG = శూన్యసూచిక;

  // Unlink the timer from our list.
  *T.Prev = T.తదుపరి_2;
  ఒకవేళ (T.తదుపరి_2)
    T.తదుపరి_2->Prev = T.Prev;
}

శూన్య Timer_సమూహం::కూడు_Timer(Timer &T) {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());

  // Add the timer to our list.
  ఒకవేళ (మొదటి_Timer)
    మొదటి_Timer->Prev = &T.తదుపరి_2;
  T.తదుపరి_2 = మొదటి_Timer;
  T.Prev = &మొదటి_Timer;
  మొదటి_Timer = &T;
}

శూన్య Timer_సమూహం::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  ఒకవేళ (క్రమీకరించు_Timers())
    llvm::క్రమీకరించు(Timers_గమ్యం_Print);

  సమయం_రికార్డు మొత్తం;
  కొరకు (స్థిరరాశి Print_రికార్డు &రికార్డు : Timers_గమ్యం_Print)
    మొత్తం += రికార్డు.సమయం;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  గుర్తులేని Padding = (80-Description.నిడివి_2())/2;
  ఒకవేళ (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  ఒకవేళ (ఇది != &default_Timer_సమూహం())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 మొత్తం.get_Process_సమయం(), మొత్తం.get_Wall_సమయం());
  OS << '\n';

  ఒకవేళ (మొత్తం.get_User_సమయం())
    OS << "   ---User Time---";
  ఒకవేళ (మొత్తం.get_System_సమయం())
    OS << "   --System Time--";
  ఒకవేళ (మొత్తం.get_Process_సమయం())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  ఒకవేళ (మొత్తం.get_Mem_వాడిన())
    OS << "  ---Mem---";
  ఒకవేళ (మొత్తం.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  కొరకు (స్థిరరాశి Print_రికార్డు &రికార్డు : llvm::తిరగేయి(Timers_గమ్యం_Print)) {
    రికార్డు.సమయం.print(మొత్తం, OS);
    OS << రికార్డు.Description << '\n';
  }

  మొత్తం.print(మొత్తం, OS);
  OS << "Total\n\n";
  OS.flush();

  Timers_గమ్యం_Print.clear();
}

శూన్య Timer_సమూహం::prepare_గమ్యం_Print_List(తార్కిక Reset_సమయం) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  కొరకు (Timer *T = మొదటి_Timer; T; T = T->తదుపరి_2) {
    ఒకవేళ (!T->hasTriggered()) కొనసాగు;
    తార్కిక WasRunning = T->isRunning();
    ఒకవేళ (WasRunning)
      T->stopTimer();

    Timers_గమ్యం_Print.emplace_వెనుక(T->సమయం, T->Name, T->Description);

    ఒకవేళ (Reset_సమయం)
      T->clear();

    ఒకవేళ (WasRunning)
      T->ప్రారంభం_Timer();
  }
}

శూన్య Timer_సమూహం::print(raw_ostream &OS, తార్కిక ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
    prepare_గమ్యం_Print_List(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  ఒకవేళ (!Timers_గమ్యం_Print.ఖాళీలు())
    PrintQueuedTimers(OS);
}

శూన్య Timer_సమూహం::clear() {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
  కొరకు (Timer *T = మొదటి_Timer; T; T = T->తదుపరి_2)
    T->clear();
}

శూన్య Timer_సమూహం::printAll(raw_ostream &OS) {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());

  కొరకు (Timer_సమూహం *TG = Timer_సమూహం_List; TG; TG = TG->తదుపరి_2)
    TG->print(OS);
}

శూన్య Timer_సమూహం::clearAll() {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
  కొరకు (Timer_సమూహం *TG = Timer_సమూహం_List; TG; TG = TG->తదుపరి_2)
    TG->clear();
}

శూన్య Timer_సమూహం::printJSONValue(raw_ostream &OS, స్థిరరాశి Print_రికార్డు &R,
                                స్థిరరాశి అక్షరం *suffix, ద్విగుణ విలువ) {
  constexpr స్వయంచాలక max_digits10 = std::numeric_limits<ద్విగుణ>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, విలువ);
}

స్థిరరాశి అక్షరం *Timer_సమూహం::printJSONValues(raw_ostream &OS, స్థిరరాశి అక్షరం *delim) {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());

  prepare_గమ్యం_Print_List(అబద్ధం);
  కొరకు (స్థిరరాశి Print_రికార్డు &R : Timers_గమ్యం_Print) {
    OS << delim;
    delim = ",\n";

    స్థిరరాశి సమయం_రికార్డు &T = R.సమయం;
    printJSONValue(OS, R, ".wall", T.get_Wall_సమయం());
    OS << delim;
    printJSONValue(OS, R, ".user", T.get_User_సమయం());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.get_System_సమయం());
    ఒకవేళ (T.get_Mem_వాడిన()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.get_Mem_వాడిన());
    }
    ఒకవేళ (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  Timers_గమ్యం_Print.clear();
  తిరిగి delim;
}

స్థిరరాశి అక్షరం *Timer_సమూహం::printAllJSONValues(raw_ostream &OS, స్థిరరాశి అక్షరం *delim) {
  sys::Smart_Scoped_తాళం<నిజం> L(timer_తాళం());
  కొరకు (Timer_సమూహం *TG = Timer_సమూహం_List; TG; TG = TG->తదుపరి_2)
    delim = TG->printJSONValues(OS, delim);
  తిరిగి delim;
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
తరగతి llvm::TimerGlobals {
బహిరంగ:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, నిజం> InfoOutputFilename{
      "info-output-file", cl::విలువ_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<తార్కిక> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<తార్కిక> క్రమీకరించు_Timers_2{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(నిజం), cl::Hidden};

  sys::SmartMutex<నిజం> Timer_తాళం;
  Timer_సమూహం Default_Timer_సమూహం{"misc", "Miscellaneous Ungrouped Timers",
                               Timer_తాళం, /*PrintOnExit=*/నిజం};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [ఇది]() { NamedGroupedTimersPtr.emplace(); });
    తిరిగి *ఇది;
  }
};

స్థిర ManagedStatic<TimerGlobals> ManagedTimerGlobals;

స్థిర std::string &libSupportInfoOutputFilename() {
  తిరిగి ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
స్థిర తార్కిక trackSpace() { తిరిగి ManagedTimerGlobals->TrackSpace; }
స్థిర తార్కిక క్రమీకరించు_Timers() { తిరిగి ManagedTimerGlobals->క్రమీకరించు_Timers_2; }
స్థిర SignpostEmitter &signposts() { తిరిగి ManagedTimerGlobals->Signposts; }
స్థిర sys::SmartMutex<నిజం> &timer_తాళం() {
  తిరిగి ManagedTimerGlobals->Timer_తాళం;
}
స్థిర Timer_సమూహం &default_Timer_సమూహం() {
  తిరిగి ManagedTimerGlobals->Default_Timer_సమూహం;
}
స్థిర Name2PairMap &namedGroupedTimers() {
  తిరిగి *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

శూన్య llvm::initTimerOptions() { *ManagedTimerGlobals; }
శూన్య Timer_సమూహం::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

శూన్య *Timer_సమూహం::acquireTimerGlobals() { తిరిగి ManagedTimerGlobals.claim(); }

స్థిర తార్కిక isTimerGlobalsConstructed() {
  తిరిగి ManagedTimerGlobals.isConstructed();
}
