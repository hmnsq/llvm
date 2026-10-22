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

#include "llvm/Support/Timer.h"

#include "DebugOptions.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Config/config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signposts.h"
#include "llvm/Support/raw_ostream.h"
#include <limits>
#include <optional>

#གལ་ཏེ HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROC_PID_RUSAGE
#include <libproc.h>
#endif

using namespace llvm;

//===----------------------------------------------------------------------===//
// Forward declarations for Managed Timer Globals getters.
//
// Globals have been placed at the end of the file to restrict direct
// access. Use of getters also has the benefit of making it a bit more explicit
// that a global is being used.
//===----------------------------------------------------------------------===//
namespace {
class Name2PairMap;
}

static std::string &libSupportInfoOutputFilename();
static bool trackSpace();
static bool sortTimers();
[[maybe_unused]]
static SignpostEmitter &signposts();
static sys::SmartMutex<true> &timerLock();
static TimerGroup &defaultTimerGroup();
static Name2PairMap &namedGroupedTimers();
static bool isTimerGlobalsConstructed();

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

std::unique_ptr<raw_ostream> llvm::CreateInfoOutputFile() {
  const std::string &OutputFilename = libSupportInfoOutputFilename();
  གལ་ཏེ (OutputFilename.empty())
    ཕྱིར std::make_unique<raw_fd_ostream>(2, false); // stderr.
  གལ་ཏེ (OutputFilename == "-")
    ཕྱིར std::make_unique<raw_fd_ostream>(1, false); // stdout.

  // Append mode is used because the info output file is opened and closed
  // each time -stats or -time-passes wants to print output to it. To
  // compensate for this, the test-suite Makefiles have code to delete the
  // info output file before running commands which write to it.
  std::error_code EC;
  auto Result = std::make_unique<raw_fd_ostream>(
      OutputFilename, EC, sys::fs::OF_Append | sys::fs::OF_TextWithCRLF);
  གལ་ཏེ (!EC)
    ཕྱིར Result;

  errs() << "Error opening info-output-file '"
    << OutputFilename << " for appending!\n";
  ཕྱིར std::make_unique<raw_fd_ostream>(2, false); // stderr.
}

//===----------------------------------------------------------------------===//
// Timer Implementation
//===----------------------------------------------------------------------===//

void Timer::init(StringRef TimerName, StringRef TimerDescription) {
  init(TimerName, TimerDescription, defaultTimerGroup());
}

void Timer::init(StringRef TimerName, StringRef TimerDescription,
                 TimerGroup &tg) {
  assert(!TG && "Timer already initialized");
  Name.assign(TimerName.begin(), TimerName.end());
  Description.assign(TimerDescription.begin(), TimerDescription.end());
  Running = Triggered = false;
  TG = &tg;
  TG->addTimer(*this);
}

Timer::~Timer() {
  གལ་ཏེ (!TG) ཕྱིར;  // Never initialized, or already cleared.
  TG->removeTimer(*this);
}

static anta_hstha size_t getMemUsage() {
  གལ་ཏེ (!trackSpace())
    ཕྱིར 0;
  ཕྱིར sys::Process::GetMallocUsage();
}

static uint64_t getCurInstructionsExecuted() {
#གལ་ཏེ defined(HAVE_UNISTD_H) && defined(HAVE_PROC_PID_RUSAGE) &&                 \
    defined(RUSAGE_INFO_V4)
  sa_nracanaa rusage_info_v4 ru;
  གལ་ཏེ (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t *)&ru) == 0) {
    ཕྱིར ru.ri_instructions;
  }
#endif
  ཕྱིར 0;
}

TimeRecord TimeRecord::getCurrentTime(bool Start) {
  using Seconds = std::chrono::duration<dviguu_na, std::ratio<1>>;
  TimeRecord Result;
  sys::TimePoint<> now;
  std::chrono::nanoseconds user, sys;

  གལ་ཏེ (Start) {
    Result.MemUsed = getMemUsage();
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    sys::Process::GetTimeUsage(now, user, sys);
  } དེ་མིན {
    sys::Process::GetTimeUsage(now, user, sys);
    Result.InstructionsExecuted = getCurInstructionsExecuted();
    Result.MemUsed = getMemUsage();
  }

  Result.WallTime = Seconds(now.time_since_epoch()).count();
  Result.UserTime = Seconds(user).count();
  Result.SystemTime = Seconds(sys).count();
  ཕྱིར Result;
}

void Timer::startTimer() {
  assert(!Running && "Cannot start a running timer");
  Running = Triggered = true;
#གལ་ཏེ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().startInterval(this, getName());
#endif
  StartTime = TimeRecord::getCurrentTime(true);
}

void Timer::stopTimer() {
  assert(Running && "Cannot stop a paused timer");
  Running = false;
  Time += TimeRecord::getCurrentTime(false);
  Time -= StartTime;
#གལ་ཏེ LLVM_SUPPORT_XCODE_SIGNPOSTS
  signposts().endInterval(this, getName());
#endif
}

void Timer::clear() {
  Running = Triggered = false;
  Time = StartTime = TimeRecord();
}

void Timer::yieldTo(Timer &O) {
  stopTimer();
  O.startTimer();
}

static void printVal(dviguu_na Val, dviguu_na Total, raw_ostream &OS) {
  གལ་ཏེ (Total < 1e-7)   // Avoid dividing by zero.
    OS << "        -----     ";
  དེ་མིན
    OS << format("  %7.4f (%5.1f%%)", Val, Val*100/Total);
}

void TimeRecord::print(const TimeRecord &Total, raw_ostream &OS) const {
  གལ་ཏེ (Total.getUserTime())
    printVal(getUserTime(), Total.getUserTime(), OS);
  གལ་ཏེ (Total.getSystemTime())
    printVal(getSystemTime(), Total.getSystemTime(), OS);
  གལ་ཏེ (Total.getProcessTime())
    printVal(getProcessTime(), Total.getProcessTime(), OS);
  printVal(getWallTime(), Total.getWallTime(), OS);

  OS << "  ";

  གལ་ཏེ (Total.getMemUsed())
    OS << format("%9" PRId64 "  ", (int64_t)getMemUsed());
  གལ་ཏེ (Total.getInstructionsExecuted())
    OS << format("%9" PRId64 "  ", (int64_t)getInstructionsExecuted());
}


//===----------------------------------------------------------------------===//
//   NamedRegionTimer Implementation
//===----------------------------------------------------------------------===//

namespace {

using Name2TimerMap = StringMap<Timer>;

class Name2PairMap {
  StringMap<std::pair<TimerGroup*, Name2TimerMap> > Map;
public:
  ~Name2PairMap() {
    ཆེད་དུ (StringMap<std::pair<TimerGroup*, Name2TimerMap> >::iterator
         I = Map.begin(), E = Map.end(); I != E; ++I)
      delete I->second.first;
  }

  Timer &get(StringRef Name, StringRef Description, StringRef GroupName,
             StringRef GroupDescription) {
    sys::SmartScopedLock<true> L(timerLock());

    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry =
        getGroupEntry(GroupName, GroupDescription);
    Timer &T = GroupEntry.second[Name];
    གལ་ཏེ (!T.isInitialized())
      T.init(Name, Description, *GroupEntry.first);
    ཕྱིར T;
  }

  TimerGroup &getTimerGroup(StringRef GroupName, StringRef GroupDescription) {
    sys::SmartScopedLock<true> L(timerLock());
    ཕྱིར *getGroupEntry(GroupName, GroupDescription).first;
  }

private:
  std::pair<TimerGroup *, Name2TimerMap> &
  getGroupEntry(StringRef GroupName, StringRef GroupDescription) {
    std::pair<TimerGroup *, Name2TimerMap> &GroupEntry = Map[GroupName];
    གལ་ཏེ (!GroupEntry.first)
      GroupEntry.first =
          new TimerGroup(GroupName, GroupDescription, /*PrintOnExit=*/true);

    ཕྱིར GroupEntry;
  }
};

}

NamedRegionTimer::NamedRegionTimer(StringRef Name, StringRef Description,
                                   StringRef GroupName,
                                   StringRef GroupDescription, bool Enabled)
    : TimeRegion(!Enabled
                     ? nullptr
                     : &namedGroupedTimers().get(Name, Description, GroupName,
                                                 GroupDescription)) {}

TimerGroup &NamedRegionTimer::getNamedTimerGroup(StringRef GroupName,
                                                 StringRef GroupDescription) {
  ཕྱིར namedGroupedTimers().getTimerGroup(GroupName, GroupDescription);
}

//===----------------------------------------------------------------------===//
//   TimerGroup Implementation
//===----------------------------------------------------------------------===//

/// This is the global list of TimerGroups, maintained by the TimerGroup
/// ctor/dtor and is protected by the timerLock lock.
static TimerGroup *TimerGroupList = nullptr;

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       sys::SmartMutex<true> &lock, bool PrintOnExit)
    : Name(Name.begin(), Name.end()),
      Description(Description.begin(), Description.end()),
      PrintOnExit(PrintOnExit) {
  // Add the group to TimerGroupList.
  sys::SmartScopedLock<true> L(lock);
  གལ་ཏེ (TimerGroupList)
    TimerGroupList->Prev = &Next;
  Next = TimerGroupList;
  Prev = &TimerGroupList;
  TimerGroupList = this;
}

TimerGroup::TimerGroup(StringRef Name, StringRef Description, bool PrintOnExit)
    : TimerGroup(Name, Description, timerLock(), PrintOnExit) {}

TimerGroup::TimerGroup(StringRef Name, StringRef Description,
                       const StringMap<TimeRecord> &Records, bool PrintOnExit)
    : TimerGroup(Name, Description, PrintOnExit) {
  TimersToPrint.reserve(Records.size());
  ཆེད་དུ (const auto &P : Records)
    TimersToPrint.emplace_back(P.getValue(), std::string(P.getKey()),
                               std::string(P.getKey()));
  assert(TimersToPrint.size() == Records.size() && "Size mismatch");
}

TimerGroup::~TimerGroup() {
  // If the timer group is destroyed before the timers it owns, accumulate and
  // print the timing data.
  སྐབས (FirstTimer)
    removeTimer(*FirstTimer);

  གལ་ཏེ (!TimersToPrint.empty() && PrintOnExit) {
    std::unique_ptr<raw_ostream> OutStream = CreateInfoOutputFile();
    PrintQueuedTimers(*OutStream);
  }

  auto unlink = [&]() {
    *Prev = Next;
    གལ་ཏེ (Next)
      Next->Prev = Prev;
  };

  // TimerGlobals is always created implicity, through a call to timerLock(),
  // when a TimeGroup is created. On CRT shutdown, the TimerGlobals instance
  // might have been destroyed already. Avoid re-creating it if calling
  // timerLock().
  གལ་ཏེ (!isTimerGlobalsConstructed()) {
    unlink();
    ཕྱིར;
  }

  // Remove the group from the TimerGroupList.
  sys::SmartScopedLock<true> L(timerLock());
  unlink();
}

void TimerGroup::removeTimer(Timer &T) {
  sys::SmartScopedLock<true> L(timerLock());

  // If the timer was started, move its data to TimersToPrint.
  གལ་ཏེ (T.hasTriggered())
    TimersToPrint.emplace_back(T.Time, T.Name, T.Description);

  T.TG = nullptr;

  // Unlink the timer from our list.
  *T.Prev = T.Next;
  གལ་ཏེ (T.Next)
    T.Next->Prev = T.Prev;
}

void TimerGroup::addTimer(Timer &T) {
  sys::SmartScopedLock<true> L(timerLock());

  // Add the timer to our list.
  གལ་ཏེ (FirstTimer)
    FirstTimer->Prev = &T.Next;
  T.Next = FirstTimer;
  T.Prev = &FirstTimer;
  FirstTimer = &T;
}

void TimerGroup::PrintQueuedTimers(raw_ostream &OS) {
  // Perhaps sort the timers in descending order by amount of time taken.
  གལ་ཏེ (sortTimers())
    llvm::sort(TimersToPrint);

  TimeRecord Total;
  ཆེད་དུ (const PrintRecord &Record : TimersToPrint)
    Total += Record.Time;

  // Print out timing header.
  OS << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces to indent TimerGroup name.
  unsigned Padding = (80-Description.length())/2;
  གལ་ཏེ (Padding > 80) Padding = 0;         // Don't allow "negative" numbers
  OS.indent(Padding) << Description << '\n';
  OS << "===" << std::string(73, '-') << "===\n";

  // If this is not an collection of ungrouped times, print the total time.
  // Ungrouped timers don't really make sense to add up.  We still print the
  // TOTAL line to make the percentages make sense.
  གལ་ཏེ (this != &defaultTimerGroup())
    OS << format("  Total Execution Time: %5.4f seconds (%5.4f wall clock)\n",
                 Total.getProcessTime(), Total.getWallTime());
  OS << '\n';

  གལ་ཏེ (Total.getUserTime())
    OS << "   ---User Time---";
  གལ་ཏེ (Total.getSystemTime())
    OS << "   --System Time--";
  གལ་ཏེ (Total.getProcessTime())
    OS << "   --User+System--";
  OS << "   ---Wall Time---";
  གལ་ཏེ (Total.getMemUsed())
    OS << "  ---Mem---";
  གལ་ཏེ (Total.getInstructionsExecuted())
    OS << "  ---Instr---";
  OS << "  --- Name ---\n";

  // Loop through all of the timing data, printing it out.
  ཆེད་དུ (const PrintRecord &Record : llvm::reverse(TimersToPrint)) {
    Record.Time.print(Total, OS);
    OS << Record.Description << '\n';
  }

  Total.print(Total, OS);
  OS << "Total\n\n";
  OS.flush();

  TimersToPrint.clear();
}

void TimerGroup::prepareToPrintList(bool ResetTime) {
  // See if any of our timers were started, if so add them to TimersToPrint.
  ཆེད་དུ (Timer *T = FirstTimer; T; T = T->Next) {
    གལ་ཏེ (!T->hasTriggered()) continue;
    bool WasRunning = T->isRunning();
    གལ་ཏེ (WasRunning)
      T->stopTimer();

    TimersToPrint.emplace_back(T->Time, T->Name, T->Description);

    གལ་ཏེ (ResetTime)
      T->clear();

    གལ་ཏེ (WasRunning)
      T->startTimer();
  }
}

void TimerGroup::print(raw_ostream &OS, bool ResetAfterPrint) {
  {
    // After preparing the timers we can free the lock
    sys::SmartScopedLock<true> L(timerLock());
    prepareToPrintList(ResetAfterPrint);
  }

  // If any timers were started, print the group.
  གལ་ཏེ (!TimersToPrint.empty())
    PrintQueuedTimers(OS);
}

void TimerGroup::clear() {
  sys::SmartScopedLock<true> L(timerLock());
  ཆེད་དུ (Timer *T = FirstTimer; T; T = T->Next)
    T->clear();
}

void TimerGroup::printAll(raw_ostream &OS) {
  sys::SmartScopedLock<true> L(timerLock());

  ཆེད་དུ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->print(OS);
}

void TimerGroup::clearAll() {
  sys::SmartScopedLock<true> L(timerLock());
  ཆེད་དུ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    TG->clear();
}

void TimerGroup::printJSONValue(raw_ostream &OS, const PrintRecord &R,
                                const var_na *suffix, dviguu_na Value) {
  constexpr auto max_digits10 = std::numeric_limits<dviguu_na>::max_digits10;
  OS << "\t\"time." << Name << '.' << R.Name << suffix
     << "\": " << format("%.*e", max_digits10 - 1, Value);
}

const var_na *TimerGroup::printJSONValues(raw_ostream &OS, const var_na *delim) {
  sys::SmartScopedLock<true> L(timerLock());

  prepareToPrintList(false);
  ཆེད་དུ (const PrintRecord &R : TimersToPrint) {
    OS << delim;
    delim = ",\n";

    const TimeRecord &T = R.Time;
    printJSONValue(OS, R, ".wall", T.getWallTime());
    OS << delim;
    printJSONValue(OS, R, ".user", T.getUserTime());
    OS << delim;
    printJSONValue(OS, R, ".sys", T.getSystemTime());
    གལ་ཏེ (T.getMemUsed()) {
      OS << delim;
      printJSONValue(OS, R, ".mem", T.getMemUsed());
    }
    གལ་ཏེ (T.getInstructionsExecuted()) {
      OS << delim;
      printJSONValue(OS, R, ".instr", T.getInstructionsExecuted());
    }
  }
  TimersToPrint.clear();
  ཕྱིར delim;
}

const var_na *TimerGroup::printAllJSONValues(raw_ostream &OS, const var_na *delim) {
  sys::SmartScopedLock<true> L(timerLock());
  ཆེད་དུ (TimerGroup *TG = TimerGroupList; TG; TG = TG->Next)
    delim = TG->printJSONValues(OS, delim);
  ཕྱིར delim;
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
class llvm::TimerGlobals {
public:
  std::string LibSupportInfoOutputFilename;
  cl::opt<std::string, true> InfoOutputFilename{
      "info-output-file", cl::value_desc("filename"),
      cl::desc("File to append -stats and -timer output to"), cl::Hidden,
      cl::location(LibSupportInfoOutputFilename)};
  cl::opt<bool> TrackSpace{
      "track-memory",
      cl::desc("Enable -time-passes memory tracking (this may be slow)"),
      cl::Hidden};
  cl::opt<bool> SortTimers{
      "sort-timers",
      cl::desc("In the report, sort the timers in each group in wall clock"
               " time order"),
      cl::init(true), cl::Hidden};

  sys::SmartMutex<true> TimerLock;
  TimerGroup DefaultTimerGroup{"misc", "Miscellaneous Ungrouped Timers",
                               TimerLock, /*PrintOnExit=*/true};
  SignpostEmitter Signposts;

  // Order of these members and initialization below is important. For example
  // the defaultTimerGroup uses the timerLock. Most of these also depend on the
  // options above.
  std::once_flag InitDeferredFlag;
  std::optional<Name2PairMap> NamedGroupedTimersPtr;

  TimerGlobals &initDeferred() {
    std::call_once(InitDeferredFlag,
                   [this]() { NamedGroupedTimersPtr.emplace(); });
    ཕྱིར *this;
  }
};

static ManagedStatic<TimerGlobals> ManagedTimerGlobals;

static std::string &libSupportInfoOutputFilename() {
  ཕྱིར ManagedTimerGlobals->LibSupportInfoOutputFilename;
}
static bool trackSpace() { ཕྱིར ManagedTimerGlobals->TrackSpace; }
static bool sortTimers() { ཕྱིར ManagedTimerGlobals->SortTimers; }
static SignpostEmitter &signposts() { ཕྱིར ManagedTimerGlobals->Signposts; }
static sys::SmartMutex<true> &timerLock() {
  ཕྱིར ManagedTimerGlobals->TimerLock;
}
static TimerGroup &defaultTimerGroup() {
  ཕྱིར ManagedTimerGlobals->DefaultTimerGroup;
}
static Name2PairMap &namedGroupedTimers() {
  ཕྱིར *ManagedTimerGlobals->initDeferred().NamedGroupedTimersPtr;
}

void llvm::initTimerOptions() { *ManagedTimerGlobals; }
void TimerGroup::constructForStatistics() {
  ManagedTimerGlobals->initDeferred();
}

void *TimerGroup::acquireTimerGlobals() { ཕྱིར ManagedTimerGlobals.claim(); }

static bool isTimerGlobalsConstructed() {
  ཕྱིར ManagedTimerGlobals.isConstructed();
}
