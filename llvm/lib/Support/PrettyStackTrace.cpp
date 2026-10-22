//===- PrettyStackTrace.cpp - Pretty Crash Handling -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines some helpful functions for dealing with the possibility of
// Unix signals occurring while your program is running.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/PrettyStackTrace.h"
#شامل "llvm-c/ErrorHandling.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/SaveAndRestore.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/Watchdog.h"
#شامل "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#شامل "llvm/ADT/SmallString.h"
#endif

#شامل <atomic>
#شامل <cassert>
#شامل <cstdarg>
#شامل <cstdio>
#شامل <cstring>
#شامل <tuple>

#اگر __has_include(<CrashReporterClient.h>)
#شامل <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using فضاینام llvm;

ایستا ثابت نویسه *Bug_Report_پیام =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#اگر ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ایستا LLVM_THREAD_LOCAL Pretty_پشته_Trace_Entry *Pretty_پشته_Trace_Head = nullptr;

// The use of 'volatile' here is to ensure that any particular thread always
// reloads the value of the counter. The 'std::atomic' allows us to specify that
// this variable is accessed in an unsychronized way (it's not actually
// synchronizing). This does technically mean that the value may not appear to
// be the same across threads running simultaneously on different CPUs, but in
// practice the worst that will happen is that we won't print a stack trace when
// we could have.
//
// This is initialized to 1 because 0 is used as a sentinel for "not enabled on
// the current thread". If the user happens to overflow an 'unsigned' with
// SIGINFO requests, it's possible that some threads will stop responding to it,
// but the program won't crash.
ایستا فرار std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
ایستا LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

فضاینام llvm {
Pretty_پشته_Trace_Entry *Reverse_پشته_Trace(Pretty_پشته_Trace_Entry *Head) {
  Pretty_پشته_Trace_Entry *Prev = nullptr;
  وقتی (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  بازگشت Prev;
}
} // namespace llvm

ایستا تهی Print_پشته(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_پشته_Trace_Entry *> Saved_پشته{Pretty_پشته_Trace_Head,
                                                     nullptr};
  Pretty_پشته_Trace_Entry *Reversed_پشته = Reverse_پشته_Trace(Saved_پشته.get());
  برای (ثابت Pretty_پشته_Trace_Entry *Entry = Reversed_پشته; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_پشته_Trace(Reversed_پشته);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ایستا تهی Print_Cur_پشته_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  اگر (!Pretty_پشته_Trace_Head) بازگشت;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_پشته(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#اگر defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
بیرونی "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#وگرنه
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ساختار crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#اگر CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وگرنهاگر defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
بیرونی "C" ثابت نویسه *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] ایستا تهی setCrashLogMessage(ثابت نویسه *پیام);
ایستا تهی setCrashLogMessage(ثابت نویسه *پیام) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (تهی)CRSetCrashLogMessage(پیام);
#وگرنهاگر HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = پیام;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[اندازه(CrashHandlerString)];
alignas(CrashHandlerString) ایستا CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ایستا تهی CrashHandler(تهی *) {
  errs() << Bug_Report_پیام ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_پشته_Trace(errs());
#وگرنه
  // Emit the crash stack trace to a SmallString, put it where the system crash
  // handling will find it, and also send it to stderr.
  //
  // The SmallString is fairly large in the hope that we don't allocate (we're
  // handling a fatal signal, something is already pretty wrong, allocation
  // might not work). Further, we don't use a magic static in case that's also
  // borked. We leak any allocation that does occur because the program is about
  // to die anyways. This is technically racy if we were handling two fatal
  // signals, however if we're in that situation a race is the least of our
  // worries.
  auto &crashHandlerString =
      *جدید (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_پشته_Trace(Stream);
  }

  اگر (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } وگرنه {
    setCrashLogMessage("No crash information.");
  }
#endif
}

ایستا تهی printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  اگر (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    بازگشت;
  }

  Print_Cur_پشته_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

تهی llvm::set_Bug_Report_پیام(ثابت نویسه *پیام_2) {
  Bug_Report_پیام = پیام_2;
}

ثابت نویسه *llvm::get_Bug_Report_پیام() {
  بازگشت Bug_Report_پیام;
}

Pretty_پشته_Trace_Entry::Pretty_پشته_Trace_Entry() {
#اگر ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_پشته_Trace_Head;
  Pretty_پشته_Trace_Head = این;
#endif
}

Pretty_پشته_Trace_Entry::~Pretty_پشته_Trace_Entry() {
#اگر ENABLE_BACKTRACES
  أكد(Pretty_پشته_Trace_Head == این &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_پشته_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

تهی Pretty_پشته_Trace_String::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_پشته_Trace_Format::Pretty_پشته_Trace_Format(ثابت نویسه *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ثابت صحیح SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  اگر (SizeOrError < 0) {
    بازگشت;
  }

  ثابت صحیح Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

تهی Pretty_پشته_Trace_Format::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

تهی Pretty_پشته_Trace_Program::print(raw_ostream &OS) ثابت {
  OS << "Program arguments: ";
  // Print the argument list.
  برای (صحیح I = 0; I < ArgC; ++I) {
    ثابت منطقی HaveSpace = ::strchr(ArgV[I], ' ');
    اگر (I)
      OS << ' ';
    اگر (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    اگر (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#اگر ENABLE_BACKTRACES
ایستا منطقی RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  بازگشت نادرست;
}
#endif

تهی llvm::Enable_Pretty_پشته_Trace() {
#اگر ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ایستا منطقی HandlerRegistered = RegisterCrashPrinter();
  (تهی)HandlerRegistered;
#endif
}

تهی llvm::Enable_Pretty_پشته_Trace_On_Sig_Info_For_This_Thread(منطقی ShouldEnable) {
#اگر ENABLE_BACKTRACES
  اگر (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    بازگشت;
  }

  // The first time this is called, we register the SIGINFO handler.
  ایستا منطقی HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    بازگشت نادرست;
  }();
  (تهی)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ثابت تهی *llvm::Save_Pretty_پشته_State() {
#اگر ENABLE_BACKTRACES
  بازگشت Pretty_پشته_Trace_Head;
#وگرنه
  بازگشت nullptr;
#endif
}

تهی llvm::Restore_Pretty_پشته_State(ثابت تهی *بالاترین) {
#اگر ENABLE_BACKTRACES
  Pretty_پشته_Trace_Head =
      static_cast<Pretty_پشته_Trace_Entry *>(const_cast<تهی *>(بالاترین));
#endif
}

تهی LLVMEnable_Pretty_پشته_Trace() {
  Enable_Pretty_پشته_Trace();
}
