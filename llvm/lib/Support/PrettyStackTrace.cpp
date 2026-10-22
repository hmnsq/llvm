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

#ضمن "llvm/Support/PrettyStackTrace.h"
#ضمن "llvm-c/ErrorHandling.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/SaveAndRestore.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Watchdog.h"
#ضمن "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#ضمن "llvm/ADT/SmallString.h"
#endif

#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdarg>
#ضمن <cstdio>
#ضمن <cstring>
#ضمن <tuple>

#אם __has_include(<CrashReporterClient.h>)
#ضمن <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using نطاق llvm;

ساكن ثابت حرف *BugReportMsg =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#אם ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ساكن LLVM_THREAD_LOCAL PrettyStackTraceEntry *PrettyStackTraceHead = nullptr;

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
ساكن متطاير std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
ساكن LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

نطاق llvm {
PrettyStackTraceEntry *ReverseStackTrace(PrettyStackTraceEntry *Head) {
  PrettyStackTraceEntry *Prev = nullptr;
  כלעוד (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  החזר Prev;
}
} // namespace llvm

ساكن فراغ PrintStack(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<PrettyStackTraceEntry *> SavedStack{PrettyStackTraceHead,
                                                     nullptr};
  PrettyStackTraceEntry *ReversedStack = ReverseStackTrace(SavedStack.get());
  עבור (ثابت PrettyStackTraceEntry *Entry = ReversedStack; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::ReverseStackTrace(ReversedStack);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساكن فراغ PrintCurStackTrace(raw_ostream &OS) {
  // Don't print an empty trace.
  אם (!PrettyStackTraceHead) החזר;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  PrintStack(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#אם defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجي "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#אחרת
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
بنية crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#אם CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وإلاإذا defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجي "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] ساكن فراغ setCrashLogMessage(ثابت حرف *msg);
ساكن فراغ setCrashLogMessage(ثابت حرف *msg) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (فراغ)CRSetCrashLogMessage(msg);
#وإلاإذا HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = msg;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساكن CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساكن فراغ CrashHandler(فراغ *) {
  errs() << BugReportMsg ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  PrintCurStackTrace(errs());
#אחרת
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
      *جديد (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    PrintCurStackTrace(Stream);
  }

  אם (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } אחרת {
    setCrashLogMessage("No crash information.");
  }
#endif
}

ساكن فراغ printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  אם (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    החזר;
  }

  PrintCurStackTrace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

فراغ llvm::setBugReportMsg(ثابت حرف *Msg) {
  BugReportMsg = Msg;
}

ثابت حرف *llvm::getBugReportMsg() {
  החזר BugReportMsg;
}

PrettyStackTraceEntry::PrettyStackTraceEntry() {
#אם ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = PrettyStackTraceHead;
  PrettyStackTraceHead = هذا;
#endif
}

PrettyStackTraceEntry::~PrettyStackTraceEntry() {
#אם ENABLE_BACKTRACES
  أكد(PrettyStackTraceHead == هذا &&
         "Pretty stack trace entry destruction is out of order");
  PrettyStackTraceHead = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

فراغ PrettyStackTraceString::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

PrettyStackTraceFormat::PrettyStackTraceFormat(ثابت حرف *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ثابت صحيح SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  אם (SizeOrError < 0) {
    החזר;
  }

  ثابت صحيح Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

فراغ PrettyStackTraceFormat::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

فراغ PrettyStackTraceProgram::print(raw_ostream &OS) ثابت {
  OS << "Program arguments: ";
  // Print the argument list.
  עבור (صحيح I = 0; I < ArgC; ++I) {
    ثابت منطقي HaveSpace = ::strchr(ArgV[I], ' ');
    אם (I)
      OS << ' ';
    אם (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    אם (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#אם ENABLE_BACKTRACES
ساكن منطقي RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  החזר שקר;
}
#endif

فراغ llvm::EnablePrettyStackTrace() {
#אם ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساكن منطقي HandlerRegistered = RegisterCrashPrinter();
  (فراغ)HandlerRegistered;
#endif
}

فراغ llvm::EnablePrettyStackTraceOnSigInfoForThisThread(منطقي ShouldEnable) {
#אם ENABLE_BACKTRACES
  אם (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    החזר;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساكن منطقي HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    החזר שקר;
  }();
  (فراغ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ثابت فراغ *llvm::SavePrettyStackState() {
#אם ENABLE_BACKTRACES
  החזר PrettyStackTraceHead;
#אחרת
  החזר nullptr;
#endif
}

فراغ llvm::RestorePrettyStackState(ثابت فراغ *Top) {
#אם ENABLE_BACKTRACES
  PrettyStackTraceHead =
      static_cast<PrettyStackTraceEntry *>(const_cast<فراغ *>(Top));
#endif
}

فراغ LLVMEnablePrettyStackTrace() {
  EnablePrettyStackTrace();
}
