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

#अन्तर्भाव "llvm/Support/Timer.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm/ADT/Statistic.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signposts.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <limits>
#अन्तर्भाव <optional>

#यदि HAVE_UNISTD_H
#अन्तर्भाव <unistd.h>
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_PROC_PID_RUSAGE
#अन्तर्भाव <libproc.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
नामाकाश {
वर्ग Name2PairMap;
}

स्थिर std::string &libSupportInfoOutputFilename();
स्थिर तार्किक trackSpace();
स्थिर तार्किक क्रमय_Timers();
[[maybe_अप्रयुक्तम्]]
स्थिर SignpostEmitter &signposts();
स्थिर sys::SmartMutex<सत्य> &timer_तालकम्();
स्थिर Timer_गणः &default_Timer_गणः();
स्थिर Name2PairMap &namedGroupedTimers();
स्थिर तार्किक isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  ध्रुव std::string &OutputFilename = libSupportInfoOutputFilename();
  यदि (OutputFilename.रिक्तानि())
    प्रत्यागम std::make_unique<raw_fd_ostream>(2, असत्य); // stderr.
  यदि (OutputFilename == "-")
    प्रत्यागम std::make_unique<raw_fd_ostream>(1, असत्य); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_संहिता EC;
  स्वचालित परिणाम = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::सम्बन्धि_Append | sys::fs::सम्बन्धि_Text_With_CRLF);
  यदि (!EC)
    प्रत्यागम परिणाम;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  प्रत्यागम std::make_unique<raw_fd_ostream>(2, असत्य); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

शून्य Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, default_Timer_गणः());
}

शून्य Timer::init(StringRef TimerName, StringRef TimerDescription,
                 Timer_गणः &tg) {
  प्रतिज्ञा(!TG && "Timer already initialized");
  Name.नियोजय(TimerName.begin(), TimerName.अन्तः());
  Description.नियोजय(TimerDescription.begin(), TimerDescription.अन्तः());
  Running = Triggered = असत्य;
  TG = &tg;
  TG->सङ्कलय_Timer(*एतत्);
}

Timer::~Timer() {
  यदि (!TG) प्रत्यागम;  // Never initialized, or already cleared.
  TG->removeTimer(*एतत्);
}

स्थिर अन्तःस्थ size_t getMemUsage() {
  यदि (!trackSpace())
    प्रत्यागम 0;
  प्रत्यागम sys::Process::GetMallocUsage();
}

स्थिर uint64_t getCurInstructionsExecuted() {
#यदि defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  संरचना rusage_info_v4 ru;
  यदि (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    प्रत्यागम ru.ri_instructions;
  }
#यदि_अन्तः
  प्रत्यागम 0;
}

Time_लेखः Time_लेखः::getCurrentTime(तार्किक Start) {
  उपयुज्य Seconds = std::chrono::duration<द्विगुण, std::अनुपातः<1>>;
  Time_लेखः परिणाम;
  sys::TimePoint<> now;
  std::chrono::nanoseconds user, sys;

  यदि (Start) {
    परिणाम.Mem_प्रयुक्तम् = getMemUsage();
    परिणाम.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::GetTimeUsage(now, user, sys);
  } अन्यथा {
    sys::Process::GetTimeUsage(now, user, sys);
    परिणाम.InstructionsExecuted = getCurInstructionsExecuted();
    परिणाम.Mem_प्रयुक्तम् = getMemUsage();
  }

  परिणाम.WallTime = Seconds(now.time_since_epoch()).गणनम्();
  परिणाम.UserTime = Seconds(user).गणनम्();
  परिणाम.SystemTime = Seconds(sys).गणनम्();
  प्रत्यागम परिणाम;
}

शून्य Timer::startTimer() {
  प्रतिज्ञा(!Running && "Cannot start a running timer");
  Running = Triggered = सत्य;
#यदि LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().startInterval(एतत्, getName());
#यदि_अन्तः
  StartTime = Time_लेखः::getCurrentTime(सत्य);
}

शून्य Timer::stopTimer() {
  प्रतिज्ञा(Running && "Cannot stop a paused timer");
  Running = असत्य;
  Time += Time_लेखः::getCurrentTime(असत्य);
  Time -= StartTime;
#यदि LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().अन्तः_Interval(एतत्, getName());
#यदि_अन्तः
}

शून्य Timer::clear() {
  Running = Triggered = असत्य;
  Time = StartTime = Time_लेखः();
}

शून्य Timer::yield_गन्तव्यम्(Timer &O) {
  stopTimer();
  O.startTimer();
}

स्थिर शून्य printVal(द्विगुण Val, द्विगुण योग_2, raw_ostream &OS) {
  यदि (योग_2 < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  अन्यथा
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/योग_2);
}

शून्य Time_लेखः::print(ध्रुव Time_लेखः &योग_2, raw_ostream &OS) ध्रुव {
  यदि (योग_2.getUserTime())
    printVal(getUserTime(), योग_2.getUserTime(), OS);
  यदि (योग_2.getSystemTime())
    printVal(getSystemTime(), योग_2.getSystemTime(), OS);
  यदि (योग_2.getProcessTime())
    printVal(getProcessTime(), योग_2.getProcessTime(), OS);
  printVal(getWallTime(), योग_2.getWallTime(), OS);

  OS << "  ";

  यदि (योग_2.get_Mem_प्रयुक्तम्())
    OS << format("%9" PRId64 "  ", (int64_t)get_Mem_प्रयुक्तम्());
  यदि (योग_2.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

नामाकाश {

उपयुज्य Name2TimerMap = StringMap<Timer>;

वर्ग Name2PairMap {
  StringMap<std::pair<Timer_गणः*, Name2TimerMap> > Map;
सार्वजनिक:
  ~Name2PairMap() {
    प्रति (StringMap<std::pair<Timer_गणः*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.अन्तः(); I != E; ++I)
      विलोपय I->second.first;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef गणः_Name,
             StringRef गणः_Description) {
    sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());

    std::pair<Timer_गणः *, Name2TimerMap> &गणः_प्रविष्टिः =
        get_गणः_प्रविष्टिः(गणः_Name, गणः_Description);
    Timer &T = गणः_प्रविष्टिः.second[Name];
    यदि (!T.isInitialized())
      T.init(Name, Description, *गणः_प्रविष्टिः.first);
    प्रत्यागम T;
  }

  Timer_गणः &get_Timer_गणः(StringRef गणः_Name, StringRef गणः_Description) {
    sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
    प्रत्यागम *get_गणः_प्रविष्टिः(गणः_Name, गणः_Description).first;
  }

गुप्त:
  std::pair<Timer_गणः *, Name2TimerMap> &
  get_गणः_प्रविष्टिः(StringRef गणः_Name, StringRef गणः_Description) {
    std::pair<Timer_गणः *, Name2TimerMap> &गणः_प्रविष्टिः = Map[गणः_Name];
    यदि (!गणः_प्रविष्टिः.first)
      गणः_प्रविष्टिः.first =
          नूतन Timer_गणः(गणः_Name, गणः_Description, /*PrintOnExit=*/सत्य);

    प्रत्यागम गणः_प्रविष्टिः;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef गणः_Name,
                                   StringRef गणः_Description, तार्किक Enabled)
    : TimeRegion(!Enabled
                     ? शून्यसूचकः
                     : &namedGroupedTimers().get(Name, Description, गणः_Name,
                                                 गणः_Description)) {}

Timer_गणः &NamedRegionTimer::get_Named_Timer_गणः(StringRef गणः_Name,
                                                 StringRef गणः_Description) {
  प्रत्यागम namedGroupedTimers().get_Timer_गणः(गणः_Name, गणः_Description);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
स्थिर Timer_गणः *Timer_गणः_List = शून्यसूचकः;

Timer_गणः::Timer_गणः(StringRef Name, StringRef Description,
                       sys::SmartMutex<सत्य> &तालकम्, तार्किक PrintOnExit)
    : Name(Name.begin(), Name.अन्तः()),
      Description(Description.begin(), Description.अन्तः()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::Smart_Scoped_तालकम्<सत्य> L(तालकम्);
  यदि (Timer_गणः_List)
    Timer_गणः_List->पूर्व_2 = &अग्र_2;
  अग्र_2 = Timer_गणः_List;
  पूर्व_2 = &Timer_गणः_List;
  Timer_गणः_List = एतत्;
}

Timer_गणः::Timer_गणः(StringRef Name, StringRef Description, तार्किक PrintOnExit)
    : Timer_गणः(Name, Description, timer_तालकम्(), PrintOnExit) {}

Timer_गणः::Timer_गणः(StringRef Name, StringRef Description,
                       ध्रुव StringMap<Time_लेखः> &लेखाः, तार्किक PrintOnExit)
    : Timer_गणः(Name, Description, PrintOnExit) {
  Timers_गन्तव्यम्_Print.reserve(लेखाः.आकार_2());
  प्रति (ध्रुव स्वचालित &P : लेखाः)
    Timers_गन्तव्यम्_Print.emplace_पृष्ठभागः(P.get_मानम्(), std::string(P.get_कुञ्चिका()),
                               std::string(P.get_कुञ्चिका()));
  प्रतिज्ञा(Timers_गन्तव्यम्_Print.आकार_2() == लेखाः.आकार_2() && "Size mismatch");
}

Timer_गणः::~Timer_गणः() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  यावत् (FirstTimer)
    removeTimer(*FirstTimer);

  यदि (!Timers_गन्तव्यम्_Print.रिक्तानि() && PrintOnExit) {
    std::unique_ptr<raw_ostream> बहिर्गमनम्_प्रवाहः = CreateInfoOutputFile();
    PrintQueuedTimers(*बहिर्गमनम्_प्रवाहः);
  }

  स्वचालित unlink = [&]() {
    *पूर्व_2 = अग्र_2;
    यदि (अग्र_2)
      अग्र_2->पूर्व_2 = पूर्व_2;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  यदि (!isTimerGlobalsConstructed()) {
    unlink();
    प्रत्यागम;
  }

  // Remove the group from the TimerGroupList.
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
  unlink();
}

शून्य Timer_गणः::removeTimer(Timer &T) {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());

  // If the timer was started, move its data to TimersToPrint.
  यदि (T.hasTriggered())
    Timers_गन्तव्यम्_Print.emplace_पृष्ठभागः(T.Time, T.Name, T.Description);

  T.TG = शून्यसूचकः;

  // Unlink the timer from our list.
  *T.पूर्व_2 = T.अग्र_2;
  यदि (T.अग्र_2)
    T.अग्र_2->पूर्व_2 = T.पूर्व_2;
}

शून्य Timer_गणः::सङ्कलय_Timer(Timer &T) {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());

  // Add the timer to our list.
  यदि (FirstTimer)
    FirstTimer->पूर्व_2 = &T.अग्र_2;
  T.अग्र_2 = FirstTimer;
  T.पूर्व_2 = &FirstTimer;
  FirstTimer = &T;
}

शून्य Timer_गणः::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  यदि (क्रमय_Timers())
    llvm::क्रमय(Timers_गन्तव्यम्_Print);

  Time_लेखः योग_2;
  प्रति (ध्रुव Print_लेखः &लेखः : Timers_गन्तव्यम्_Print)
    योग_2 += लेखः.Time;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  अचिह्नित Padding = (80-Description.दीर्घता_2())/2;
  यदि (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  यदि (एतत् != &default_Timer_गणः())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 योग_2.getProcessTime(), योग_2.getWallTime());
  OS << '\n';

  यदि (योग_2.getUserTime())
    OS << "   ---User Time---";
  यदि (योग_2.getSystemTime())
    OS << "   --System Time--";
  यदि (योग_2.getProcessTime())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  यदि (योग_2.get_Mem_प्रयुक्तम्())
    OS << "  ---Mem---";
  यदि (योग_2.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  प्रति (ध्रुव Print_लेखः &लेखः : llvm::reverse(Timers_गन्तव्यम्_Print)) {
    लेखः.Time.print(योग_2, OS);
    OS << लेखः.Description << '\n';
  }

  योग_2.print(योग_2, OS);
  OS << "Total\n\n";
  OS.flush();

  Timers_गन्तव्यम्_Print.clear();
}

शून्य Timer_गणः::prepare_गन्तव्यम्_Print_List(तार्किक ResetTime) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  प्रति (Timer *T = FirstTimer; T; T = T->अग्र_2) {
    यदि (!T->hasTriggered()) अनुवर्त;
    तार्किक WasRunning = T->isRunning();
    यदि (WasRunning)
      T->stopTimer();

    Timers_गन्तव्यम्_Print.emplace_पृष्ठभागः(T->Time, T->Name, T->Description);

    यदि (ResetTime)
      T->clear();

    यदि (WasRunning)
      T->startTimer();
  }
}

शून्य Timer_गणः::print(raw_ostream &OS, तार्किक ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
    prepare_गन्तव्यम्_Print_List(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  यदि (!Timers_गन्तव्यम्_Print.रिक्तानि())
    PrintQueuedTimers(OS);
}

शून्य Timer_गणः::clear() {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
  प्रति (Timer *T = FirstTimer; T; T = T->अग्र_2)
    T->clear();
}

शून्य Timer_गणः::printAll(raw_ostream &OS) {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());

  प्रति (Timer_गणः *TG = Timer_गणः_List; TG; TG = TG->अग्र_2)
    TG->print(OS);
}

शून्य Timer_गणः::clearAll() {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
  प्रति (Timer_गणः *TG = Timer_गणः_List; TG; TG = TG->अग्र_2)
    TG->clear();
}

शून्य Timer_गणः::printJSONValue(raw_ostream &OS, ध्रुव Print_लेखः &R,
                                ध्रुव वर्ण *suffix, द्विगुण मानम्) {
  constexpr स्वचालित max_digits10 = std::numeric_limits<द्विगुण>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, मानम्);
}

ध्रुव वर्ण *Timer_गणः::printJSONValues(raw_ostream &OS, ध्रुव वर्ण *delim) {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());

  prepare_गन्तव्यम्_Print_List(असत्य);
  प्रति (ध्रुव Print_लेखः &R : Timers_गन्तव्यम्_Print) {
    OS << delim;
    delim = ",\n";

    ध्रुव Time_लेखः &T = R.Time;
    printJSONValue(OS, R, ".wall", T.getWallTime());
    OS << delim;
    printJSONValue(OS, R, ".user", T.getUserTime());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.getSystemTime());
    यदि (T.get_Mem_प्रयुक्तम्()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.get_Mem_प्रयुक्तम्());
    }
    यदि (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  Timers_गन्तव्यम्_Print.clear();
  प्रत्यागम delim;
}

ध्रुव वर्ण *Timer_गणः::printAllJSONValues(raw_ostream &OS, ध्रुव वर्ण *delim) {
  sys::Smart_Scoped_तालकम्<सत्य> L(timer_तालकम्());
  प्रति (Timer_गणः *TG = Timer_गणः_List; TG; TG = TG->अग्र_2)
    delim = TG->printJSONValues(OS, delim);
  प्रत्यागम delim;
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
वर्ग llvm::TimerGlobals {
सार्वजनिक:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, सत्य> InfoOutputFilename{
      "info-output-file", cl::मानम्_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<तार्किक> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<तार्किक> क्रमय_Timers_2{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(सत्य), cl::Hidden};

  sys::SmartMutex<सत्य> Timer_तालकम्;
  Timer_गणः Default_Timer_गणः{"misc", "Miscellaneous Ungrouped Timers",
                               Timer_तालकम्, /*PrintOnExit=*/सत्य};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [एतत्]() { NamedGroupedTimersPtr.emplace(); });
    प्रत्यागम *एतत्;
  }
};

स्थिर ManagedStatic<TimerGlobals> ManagedTimerGlobals;

स्थिर std::string &libSupportInfoOutputFilename() {
  प्रत्यागम ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
स्थिर तार्किक trackSpace() { प्रत्यागम ManagedTimerGlobals->TrackSpace; }
स्थिर तार्किक क्रमय_Timers() { प्रत्यागम ManagedTimerGlobals->क्रमय_Timers_2; }
स्थिर SignpostEmitter &signposts() { प्रत्यागम ManagedTimerGlobals->Signposts; }
स्थिर sys::SmartMutex<सत्य> &timer_तालकम्() {
  प्रत्यागम ManagedTimerGlobals->Timer_तालकम्;
}
स्थिर Timer_गणः &default_Timer_गणः() {
  प्रत्यागम ManagedTimerGlobals->Default_Timer_गणः;
}
स्थिर Name2PairMap &namedGroupedTimers() {
  प्रत्यागम *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

शून्य llvm::initTimerOptions() { *ManagedTimerGlobals; }
शून्य Timer_गणः::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

शून्य *Timer_गणः::acquireTimerGlobals() { प्रत्यागम ManagedTimerGlobals.claim(); }

स्थिर तार्किक isTimerGlobalsConstructed() {
  प्रत्यागम ManagedTimerGlobals.isConstructed();
}
