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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Timer.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Statistic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signposts.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <limits>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

#ਜੇ HAVE_UNISTD_H
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PROC_PID_RUSAGE
#ਅਨ੍ਤਰ੍ਭਾਵ <libproc.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
ਨਾਮਾਕਾਸ਼ {
ਵਰਗ Name2PairMap;
}

ਸਥਿਰ std::string &libSupportInfoOutputFilename();
ਸਥਿਰ ਤਾਰਕਿਕ trackSpace();
ਸਥਿਰ ਤਾਰਕਿਕ ਛਾਂਟੋ_Timers();
[[maybe_ਅਣਵਰਤਿਆ]]
ਸਥਿਰ SignpostEmitter &signposts();
ਸਥਿਰ sys::SmartMutex<ਸੱਚ> &timer_ਤਾਲਾ();
ਸਥਿਰ Timer_ਸਮੂਹ &default_Timer_ਸਮੂਹ();
ਸਥਿਰ Name2PairMap &namedGroupedTimers();
ਸਥਿਰ ਤਾਰਕਿਕ isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  ਅਚਲ std::string &OutputFilename = libSupportInfoOutputFilename();
  ਜੇ (OutputFilename.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ std::make_unique<raw_fd_ostream>(2, ਝੂਠ); // stderr.
  ਜੇ (OutputFilename == "-")
    ਵਾਪਸ std::make_unique<raw_fd_ostream>(1, ਝੂਠ); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_ਕੋਡ EC;
  ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::ਦਾ_Append | sys::fs::ਦਾ_ਪਾਠ_With_CRLF);
  ਜੇ (!EC)
    ਵਾਪਸ ਨਤੀਜਾ;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  ਵਾਪਸ std::make_unique<raw_fd_ostream>(2, ਝੂਠ); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

ਖਾਲੀ Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, default_Timer_ਸਮੂਹ());
}

ਖਾਲੀ Timer::init(StringRef TimerName, StringRef TimerDescription,
                 Timer_ਸਮੂਹ &tg) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!TG && "Timer already initialized");
  Name.ਸੌਂਪੋ(TimerName.begin(), TimerName.ਸਿਰਾ());
  Description.ਸੌਂਪੋ(TimerDescription.begin(), TimerDescription.ਸਿਰਾ());
  Running = Triggered = ਝੂਠ;
  TG = &tg;
  TG->ਜੋੜੋ_Timer(*ਇਹ);
}

Timer::~Timer() {
  ਜੇ (!TG) ਵਾਪਸ;  // Never initialized, or already cleared.
  TG->removeTimer(*ਇਹ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ size_t getMemUsage() {
  ਜੇ (!trackSpace())
    ਵਾਪਸ 0;
  ਵਾਪਸ sys::Process::GetMallocUsage();
}

ਸਥਿਰ uint64_t getCurInstructionsExecuted() {
#ਜੇ defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  ਬਣਤਰ rusage_info_v4 ru;
  ਜੇ (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    ਵਾਪਸ ru.ri_instructions;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 0;
}

ਸਮਾਂ_ਰਿਕਾਰਡ ਸਮਾਂ_ਰਿਕਾਰਡ::get_Current_ਸਮਾਂ(ਤਾਰਕਿਕ ਸ਼ੁਰੂ) {
  ਵਰਤੋਂ Seconds = std::chrono::duration<ਦੁੱਗਣਾ, std::ਅਨੁਪਾਤ<1>>;
  ਸਮਾਂ_ਰਿਕਾਰਡ ਨਤੀਜਾ;
  sys::ਸਮਾਂ_ਬਿੰਦੂ<> now;
  std::chrono::nanoseconds user, sys;

  ਜੇ (ਸ਼ੁਰੂ) {
    ਨਤੀਜਾ.Mem_ਵਰਤਿਆ = getMemUsage();
    ਨਤੀਜਾ.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::Get_ਸਮਾਂ_Usage(now, user, sys);
  } ਨਹੀਂਤਾਂ {
    sys::Process::Get_ਸਮਾਂ_Usage(now, user, sys);
    ਨਤੀਜਾ.InstructionsExecuted = getCurInstructionsExecuted();
    ਨਤੀਜਾ.Mem_ਵਰਤਿਆ = getMemUsage();
  }

  ਨਤੀਜਾ.Wall_ਸਮਾਂ = Seconds(now.ਸਮਾਂ_since_epoch()).ਗਿਣਤੀ_2();
  ਨਤੀਜਾ.User_ਸਮਾਂ = Seconds(user).ਗਿਣਤੀ_2();
  ਨਤੀਜਾ.System_ਸਮਾਂ = Seconds(sys).ਗਿਣਤੀ_2();
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ Timer::ਸ਼ੁਰੂ_Timer() {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Running && "Cannot start a running timer");
  Running = Triggered = ਸੱਚ;
#ਜੇ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().ਸ਼ੁਰੂ_Interval(ਇਹ, getName());
#ਜੇ_ਅੰਤ
  ਸ਼ੁਰੂ_ਸਮਾਂ = ਸਮਾਂ_ਰਿਕਾਰਡ::get_Current_ਸਮਾਂ(ਸੱਚ);
}

ਖਾਲੀ Timer::stopTimer() {
  ਪ੍ਰਤਿਜ੍ਞਾ(Running && "Cannot stop a paused timer");
  Running = ਝੂਠ;
  ਸਮਾਂ += ਸਮਾਂ_ਰਿਕਾਰਡ::get_Current_ਸਮਾਂ(ਝੂਠ);
  ਸਮਾਂ -= ਸ਼ੁਰੂ_ਸਮਾਂ;
#ਜੇ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().ਸਿਰਾ_Interval(ਇਹ, getName());
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ Timer::clear() {
  Running = Triggered = ਝੂਠ;
  ਸਮਾਂ = ਸ਼ੁਰੂ_ਸਮਾਂ = ਸਮਾਂ_ਰਿਕਾਰਡ();
}

ਖਾਲੀ Timer::yield_ਮੰਜ਼ਿਲ(Timer &O) {
  stopTimer();
  O.ਸ਼ੁਰੂ_Timer();
}

ਸਥਿਰ ਖਾਲੀ printVal(ਦੁੱਗਣਾ Val, ਦੁੱਗਣਾ ਕੁੱਲ, raw_ostream &OS) {
  ਜੇ (ਕੁੱਲ < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  ਨਹੀਂਤਾਂ
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/ਕੁੱਲ);
}

ਖਾਲੀ ਸਮਾਂ_ਰਿਕਾਰਡ::print(ਅਚਲ ਸਮਾਂ_ਰਿਕਾਰਡ &ਕੁੱਲ, raw_ostream &OS) ਅਚਲ {
  ਜੇ (ਕੁੱਲ.get_User_ਸਮਾਂ())
    printVal(get_User_ਸਮਾਂ(), ਕੁੱਲ.get_User_ਸਮਾਂ(), OS);
  ਜੇ (ਕੁੱਲ.get_System_ਸਮਾਂ())
    printVal(get_System_ਸਮਾਂ(), ਕੁੱਲ.get_System_ਸਮਾਂ(), OS);
  ਜੇ (ਕੁੱਲ.get_Process_ਸਮਾਂ())
    printVal(get_Process_ਸਮਾਂ(), ਕੁੱਲ.get_Process_ਸਮਾਂ(), OS);
  printVal(get_Wall_ਸਮਾਂ(), ਕੁੱਲ.get_Wall_ਸਮਾਂ(), OS);

  OS << "  ";

  ਜੇ (ਕੁੱਲ.get_Mem_ਵਰਤਿਆ())
    OS << format("%9" PRId64 "  ", (int64_t)get_Mem_ਵਰਤਿਆ());
  ਜੇ (ਕੁੱਲ.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

ਨਾਮਾਕਾਸ਼ {

ਵਰਤੋਂ Name2TimerMap = StringMap<Timer>;

ਵਰਗ Name2PairMap {
  StringMap<std::pair<Timer_ਸਮੂਹ*, Name2TimerMap> > Map;
ਜਨਤਕ:
  ~Name2PairMap() {
    ਲਈ (StringMap<std::pair<Timer_ਸਮੂਹ*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.ਸਿਰਾ(); I != E; ++I)
      ਮਿਟਾਓ I->ਦੂਜਾ_2.ਪਹਿਲਾ_2;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef ਸਮੂਹ_Name,
             StringRef ਸਮੂਹ_Description) {
    sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());

    std::pair<Timer_ਸਮੂਹ *, Name2TimerMap> &ਸਮੂਹ_ਇੰਦਰਾਜ =
        get_ਸਮੂਹ_ਇੰਦਰਾਜ(ਸਮੂਹ_Name, ਸਮੂਹ_Description);
    Timer &T = ਸਮੂਹ_ਇੰਦਰਾਜ.ਦੂਜਾ_2[Name];
    ਜੇ (!T.isInitialized())
      T.init(Name, Description, *ਸਮੂਹ_ਇੰਦਰਾਜ.ਪਹਿਲਾ_2);
    ਵਾਪਸ T;
  }

  Timer_ਸਮੂਹ &get_Timer_ਸਮੂਹ(StringRef ਸਮੂਹ_Name, StringRef ਸਮੂਹ_Description) {
    sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
    ਵਾਪਸ *get_ਸਮੂਹ_ਇੰਦਰਾਜ(ਸਮੂਹ_Name, ਸਮੂਹ_Description).ਪਹਿਲਾ_2;
  }

ਨਿਜੀ:
  std::pair<Timer_ਸਮੂਹ *, Name2TimerMap> &
  get_ਸਮੂਹ_ਇੰਦਰਾਜ(StringRef ਸਮੂਹ_Name, StringRef ਸਮੂਹ_Description) {
    std::pair<Timer_ਸਮੂਹ *, Name2TimerMap> &ਸਮੂਹ_ਇੰਦਰਾਜ = Map[ਸਮੂਹ_Name];
    ਜੇ (!ਸਮੂਹ_ਇੰਦਰਾਜ.ਪਹਿਲਾ_2)
      ਸਮੂਹ_ਇੰਦਰਾਜ.ਪਹਿਲਾ_2 =
          ਨਵਾਂ Timer_ਸਮੂਹ(ਸਮੂਹ_Name, ਸਮੂਹ_Description, /*PrintOnExit=*/ਸੱਚ);

    ਵਾਪਸ ਸਮੂਹ_ਇੰਦਰਾਜ;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef ਸਮੂਹ_Name,
                                   StringRef ਸਮੂਹ_Description, ਤਾਰਕਿਕ Enabled)
    : ਸਮਾਂ_Region(!Enabled
                     ? ਖਾਲੀ_ਸੂਚਕ
                     : &namedGroupedTimers().get(Name, Description, ਸਮੂਹ_Name,
                                                 ਸਮੂਹ_Description)) {}

Timer_ਸਮੂਹ &NamedRegionTimer::get_Named_Timer_ਸਮੂਹ(StringRef ਸਮੂਹ_Name,
                                                 StringRef ਸਮੂਹ_Description) {
  ਵਾਪਸ namedGroupedTimers().get_Timer_ਸਮੂਹ(ਸਮੂਹ_Name, ਸਮੂਹ_Description);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
ਸਥਿਰ Timer_ਸਮੂਹ *Timer_ਸਮੂਹ_List = ਖਾਲੀ_ਸੂਚਕ;

Timer_ਸਮੂਹ::Timer_ਸਮੂਹ(StringRef Name, StringRef Description,
                       sys::SmartMutex<ਸੱਚ> &ਤਾਲਾ, ਤਾਰਕਿਕ PrintOnExit)
    : Name(Name.begin(), Name.ਸਿਰਾ()),
      Description(Description.begin(), Description.ਸਿਰਾ()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(ਤਾਲਾ);
  ਜੇ (Timer_ਸਮੂਹ_List)
    Timer_ਸਮੂਹ_List->Prev = &ਅਗਲਾ_2;
  ਅਗਲਾ_2 = Timer_ਸਮੂਹ_List;
  Prev = &Timer_ਸਮੂਹ_List;
  Timer_ਸਮੂਹ_List = ਇਹ;
}

Timer_ਸਮੂਹ::Timer_ਸਮੂਹ(StringRef Name, StringRef Description, ਤਾਰਕਿਕ PrintOnExit)
    : Timer_ਸਮੂਹ(Name, Description, timer_ਤਾਲਾ(), PrintOnExit) {}

Timer_ਸਮੂਹ::Timer_ਸਮੂਹ(StringRef Name, StringRef Description,
                       ਅਚਲ StringMap<ਸਮਾਂ_ਰਿਕਾਰਡ> &ਰਿਕਾਰਡਾਂ, ਤਾਰਕਿਕ PrintOnExit)
    : Timer_ਸਮੂਹ(Name, Description, PrintOnExit) {
  Timers_ਮੰਜ਼ਿਲ_Print.reserve(ਰਿਕਾਰਡਾਂ.ਮਾਪ_2());
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &P : ਰਿਕਾਰਡਾਂ)
    Timers_ਮੰਜ਼ਿਲ_Print.emplace_ਪਿੱਠ(P.get_ਮੁੱਲ(), std::string(P.get_ਕੁੰਜੀ()),
                               std::string(P.get_ਕੁੰਜੀ()));
  ਪ੍ਰਤਿਜ੍ਞਾ(Timers_ਮੰਜ਼ਿਲ_Print.ਮਾਪ_2() == ਰਿਕਾਰਡਾਂ.ਮਾਪ_2() && "Size mismatch");
}

Timer_ਸਮੂਹ::~Timer_ਸਮੂਹ() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  ਜਦੋਂ (ਪਹਿਲਾ_Timer)
    removeTimer(*ਪਹਿਲਾ_Timer);

  ਜੇ (!Timers_ਮੰਜ਼ਿਲ_Print.ਖਾਲੀ_ਥਾਵਾਂ() && PrintOnExit) {
    std::unique_ptr<raw_ostream> ਬਾਹਰਲਾ_ਧਾਰਾ = CreateInfoOutputFile();
    PrintQueuedTimers(*ਬਾਹਰਲਾ_ਧਾਰਾ);
  }

  ਸਵੈਚਾਲਿਤ unlink = [&]() {
    *Prev = ਅਗਲਾ_2;
    ਜੇ (ਅਗਲਾ_2)
      ਅਗਲਾ_2->Prev = Prev;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  ਜੇ (!isTimerGlobalsConstructed()) {
    unlink();
    ਵਾਪਸ;
  }

  // Remove the group from the TimerGroupList.
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
  unlink();
}

ਖਾਲੀ Timer_ਸਮੂਹ::removeTimer(Timer &T) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());

  // If the timer was started, move its data to TimersToPrint.
  ਜੇ (T.hasTriggered())
    Timers_ਮੰਜ਼ਿਲ_Print.emplace_ਪਿੱਠ(T.ਸਮਾਂ, T.Name, T.Description);

  T.TG = ਖਾਲੀ_ਸੂਚਕ;

  // Unlink the timer from our list.
  *T.Prev = T.ਅਗਲਾ_2;
  ਜੇ (T.ਅਗਲਾ_2)
    T.ਅਗਲਾ_2->Prev = T.Prev;
}

ਖਾਲੀ Timer_ਸਮੂਹ::ਜੋੜੋ_Timer(Timer &T) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());

  // Add the timer to our list.
  ਜੇ (ਪਹਿਲਾ_Timer)
    ਪਹਿਲਾ_Timer->Prev = &T.ਅਗਲਾ_2;
  T.ਅਗਲਾ_2 = ਪਹਿਲਾ_Timer;
  T.Prev = &ਪਹਿਲਾ_Timer;
  ਪਹਿਲਾ_Timer = &T;
}

ਖਾਲੀ Timer_ਸਮੂਹ::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  ਜੇ (ਛਾਂਟੋ_Timers())
    llvm::ਛਾਂਟੋ(Timers_ਮੰਜ਼ਿਲ_Print);

  ਸਮਾਂ_ਰਿਕਾਰਡ ਕੁੱਲ;
  ਲਈ (ਅਚਲ Print_ਰਿਕਾਰਡ &ਰਿਕਾਰਡ : Timers_ਮੰਜ਼ਿਲ_Print)
    ਕੁੱਲ += ਰਿਕਾਰਡ.ਸਮਾਂ;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Padding = (80-Description.ਲੰਮਾਈ_2())/2;
  ਜੇ (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  ਜੇ (ਇਹ != &default_Timer_ਸਮੂਹ())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 ਕੁੱਲ.get_Process_ਸਮਾਂ(), ਕੁੱਲ.get_Wall_ਸਮਾਂ());
  OS << '\n';

  ਜੇ (ਕੁੱਲ.get_User_ਸਮਾਂ())
    OS << "   ---User Time---";
  ਜੇ (ਕੁੱਲ.get_System_ਸਮਾਂ())
    OS << "   --System Time--";
  ਜੇ (ਕੁੱਲ.get_Process_ਸਮਾਂ())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  ਜੇ (ਕੁੱਲ.get_Mem_ਵਰਤਿਆ())
    OS << "  ---Mem---";
  ਜੇ (ਕੁੱਲ.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  ਲਈ (ਅਚਲ Print_ਰਿਕਾਰਡ &ਰਿਕਾਰਡ : llvm::ਉਲਟ(Timers_ਮੰਜ਼ਿਲ_Print)) {
    ਰਿਕਾਰਡ.ਸਮਾਂ.print(ਕੁੱਲ, OS);
    OS << ਰਿਕਾਰਡ.Description << '\n';
  }

  ਕੁੱਲ.print(ਕੁੱਲ, OS);
  OS << "Total\n\n";
  OS.flush();

  Timers_ਮੰਜ਼ਿਲ_Print.clear();
}

ਖਾਲੀ Timer_ਸਮੂਹ::prepare_ਮੰਜ਼ਿਲ_Print_List(ਤਾਰਕਿਕ Reset_ਸਮਾਂ) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  ਲਈ (Timer *T = ਪਹਿਲਾ_Timer; T; T = T->ਅਗਲਾ_2) {
    ਜੇ (!T->hasTriggered()) ਜਾਰੀ;
    ਤਾਰਕਿਕ WasRunning = T->isRunning();
    ਜੇ (WasRunning)
      T->stopTimer();

    Timers_ਮੰਜ਼ਿਲ_Print.emplace_ਪਿੱਠ(T->ਸਮਾਂ, T->Name, T->Description);

    ਜੇ (Reset_ਸਮਾਂ)
      T->clear();

    ਜੇ (WasRunning)
      T->ਸ਼ੁਰੂ_Timer();
  }
}

ਖਾਲੀ Timer_ਸਮੂਹ::print(raw_ostream &OS, ਤਾਰਕਿਕ ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
    prepare_ਮੰਜ਼ਿਲ_Print_List(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  ਜੇ (!Timers_ਮੰਜ਼ਿਲ_Print.ਖਾਲੀ_ਥਾਵਾਂ())
    PrintQueuedTimers(OS);
}

ਖਾਲੀ Timer_ਸਮੂਹ::clear() {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
  ਲਈ (Timer *T = ਪਹਿਲਾ_Timer; T; T = T->ਅਗਲਾ_2)
    T->clear();
}

ਖਾਲੀ Timer_ਸਮੂਹ::printAll(raw_ostream &OS) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());

  ਲਈ (Timer_ਸਮੂਹ *TG = Timer_ਸਮੂਹ_List; TG; TG = TG->ਅਗਲਾ_2)
    TG->print(OS);
}

ਖਾਲੀ Timer_ਸਮੂਹ::clearAll() {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
  ਲਈ (Timer_ਸਮੂਹ *TG = Timer_ਸਮੂਹ_List; TG; TG = TG->ਅਗਲਾ_2)
    TG->clear();
}

ਖਾਲੀ Timer_ਸਮੂਹ::printJSONValue(raw_ostream &OS, ਅਚਲ Print_ਰਿਕਾਰਡ &R,
                                ਅਚਲ ਅੱਖਰ *suffix, ਦੁੱਗਣਾ ਮੁੱਲ) {
  constexpr ਸਵੈਚਾਲਿਤ max_digits10 = std::numeric_limits<ਦੁੱਗਣਾ>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, ਮੁੱਲ);
}

ਅਚਲ ਅੱਖਰ *Timer_ਸਮੂਹ::printJSONValues(raw_ostream &OS, ਅਚਲ ਅੱਖਰ *delim) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());

  prepare_ਮੰਜ਼ਿਲ_Print_List(ਝੂਠ);
  ਲਈ (ਅਚਲ Print_ਰਿਕਾਰਡ &R : Timers_ਮੰਜ਼ਿਲ_Print) {
    OS << delim;
    delim = ",\n";

    ਅਚਲ ਸਮਾਂ_ਰਿਕਾਰਡ &T = R.ਸਮਾਂ;
    printJSONValue(OS, R, ".wall", T.get_Wall_ਸਮਾਂ());
    OS << delim;
    printJSONValue(OS, R, ".user", T.get_User_ਸਮਾਂ());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.get_System_ਸਮਾਂ());
    ਜੇ (T.get_Mem_ਵਰਤਿਆ()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.get_Mem_ਵਰਤਿਆ());
    }
    ਜੇ (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  Timers_ਮੰਜ਼ਿਲ_Print.clear();
  ਵਾਪਸ delim;
}

ਅਚਲ ਅੱਖਰ *Timer_ਸਮੂਹ::printAllJSONValues(raw_ostream &OS, ਅਚਲ ਅੱਖਰ *delim) {
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> L(timer_ਤਾਲਾ());
  ਲਈ (Timer_ਸਮੂਹ *TG = Timer_ਸਮੂਹ_List; TG; TG = TG->ਅਗਲਾ_2)
    delim = TG->printJSONValues(OS, delim);
  ਵਾਪਸ delim;
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
ਵਰਗ llvm::TimerGlobals {
ਜਨਤਕ:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, ਸੱਚ> InfoOutputFilename{
      "info-output-file", cl::ਮੁੱਲ_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<ਤਾਰਕਿਕ> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<ਤਾਰਕਿਕ> ਛਾਂਟੋ_Timers_2{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(ਸੱਚ), cl::Hidden};

  sys::SmartMutex<ਸੱਚ> Timer_ਤਾਲਾ;
  Timer_ਸਮੂਹ Default_Timer_ਸਮੂਹ{"misc", "Miscellaneous Ungrouped Timers",
                               Timer_ਤਾਲਾ, /*PrintOnExit=*/ਸੱਚ};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [ਇਹ]() { NamedGroupedTimersPtr.emplace(); });
    ਵਾਪਸ *ਇਹ;
  }
};

ਸਥਿਰ ManagedStatic<TimerGlobals> ManagedTimerGlobals;

ਸਥਿਰ std::string &libSupportInfoOutputFilename() {
  ਵਾਪਸ ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
ਸਥਿਰ ਤਾਰਕਿਕ trackSpace() { ਵਾਪਸ ManagedTimerGlobals->TrackSpace; }
ਸਥਿਰ ਤਾਰਕਿਕ ਛਾਂਟੋ_Timers() { ਵਾਪਸ ManagedTimerGlobals->ਛਾਂਟੋ_Timers_2; }
ਸਥਿਰ SignpostEmitter &signposts() { ਵਾਪਸ ManagedTimerGlobals->Signposts; }
ਸਥਿਰ sys::SmartMutex<ਸੱਚ> &timer_ਤਾਲਾ() {
  ਵਾਪਸ ManagedTimerGlobals->Timer_ਤਾਲਾ;
}
ਸਥਿਰ Timer_ਸਮੂਹ &default_Timer_ਸਮੂਹ() {
  ਵਾਪਸ ManagedTimerGlobals->Default_Timer_ਸਮੂਹ;
}
ਸਥਿਰ Name2PairMap &namedGroupedTimers() {
  ਵਾਪਸ *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

ਖਾਲੀ llvm::initTimerOptions() { *ManagedTimerGlobals; }
ਖਾਲੀ Timer_ਸਮੂਹ::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

ਖਾਲੀ *Timer_ਸਮੂਹ::acquireTimerGlobals() { ਵਾਪਸ ManagedTimerGlobals.claim(); }

ਸਥਿਰ ਤਾਰਕਿਕ isTimerGlobalsConstructed() {
  ਵਾਪਸ ManagedTimerGlobals.isConstructed();
}
