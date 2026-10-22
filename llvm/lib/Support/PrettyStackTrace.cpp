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

#ಅನ್ತರ್ಭಾವ "llvm/Support/PrettyStackTrace.h"
#ಅನ್ತರ್ಭಾವ "llvm-c/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SaveAndRestore.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Watchdog.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#endif

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdarg>
#ಅನ್ತರ್ಭಾವ <cstdio>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <tuple>

#ಯದಿ __has_include(<CrashReporterClient.h>)
#ಅನ್ತರ್ಭಾವ <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *Bug_Report_ಸಂದೇಶ =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#ಯದಿ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ಸ್ಥಿರ LLVM_THREAD_LOCAL Pretty_ರಾಶಿ_Trace_Entry *Pretty_ರಾಶಿ_Trace_Head = nullptr;

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
ಸ್ಥಿರ ಚಞ್ಚಲ std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
ಸ್ಥಿರ LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

ನಾಮಾಕಾಶ llvm {
Pretty_ರಾಶಿ_Trace_Entry *Reverse_ರಾಶಿ_Trace(Pretty_ರಾಶಿ_Trace_Entry *Head) {
  Pretty_ರಾಶಿ_Trace_Entry *Prev = nullptr;
  ಯಾವತ್ (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  ಪ್ರತ್ಯಾಗಮ Prev;
}
} // namespace llvm

ಸ್ಥಿರ ಶೂನ್ಯ Print_ರಾಶಿ(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_ರಾಶಿ_Trace_Entry *> Saved_ರಾಶಿ{Pretty_ರಾಶಿ_Trace_Head,
                                                     nullptr};
  Pretty_ರಾಶಿ_Trace_Entry *Reversed_ರಾಶಿ = Reverse_ರಾಶಿ_Trace(Saved_ರಾಶಿ.get());
  ಪ್ರತಿ (ಧ್ರುವ Pretty_ರಾಶಿ_Trace_Entry *Entry = Reversed_ರಾಶಿ; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_ರಾಶಿ_Trace(Reversed_ರಾಶಿ);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ಸ್ಥಿರ ಶೂನ್ಯ Print_Cur_ರಾಶಿ_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  ಯದಿ (!Pretty_ರಾಶಿ_Trace_Head) ಪ್ರತ್ಯಾಗಮ;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ರಾಶಿ(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#ಯದಿ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
ಬಾಹ್ಯ "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#ಅನ್ಯಥಾ
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ಸಂರಚನಾ crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#ಯದಿ CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#ನೋಚೇದ್ಯದಿ defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
ಬಾಹ್ಯ "C" ಧ್ರುವ ವರ್ಣ *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] ಸ್ಥಿರ ಶೂನ್ಯ setCrashLogMessage(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ setCrashLogMessage(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (ಶೂನ್ಯ)CRSetCrashLogMessage(ಸಂದೇಶ);
#ನೋಚೇದ್ಯದಿ HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = ಸಂದೇಶ;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[ಪರಿಮಾಣ(CrashHandlerString)];
alignas(CrashHandlerString) ಸ್ಥಿರ CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ಸ್ಥಿರ ಶೂನ್ಯ CrashHandler(ಶೂನ್ಯ *) {
  errs() << Bug_Report_ಸಂದೇಶ ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ರಾಶಿ_Trace(errs());
#ಅನ್ಯಥಾ
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
      *ನೂತನ (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_ರಾಶಿ_Trace(Stream);
  }

  ಯದಿ (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } ಅನ್ಯಥಾ {
    setCrashLogMessage("No crash information.");
  }
#endif
}

ಸ್ಥಿರ ಶೂನ್ಯ printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  ಯದಿ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    ಪ್ರತ್ಯಾಗಮ;
  }

  Print_Cur_ರಾಶಿ_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

ಶೂನ್ಯ llvm::set_Bug_Report_ಸಂದೇಶ(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ_2) {
  Bug_Report_ಸಂದೇಶ = ಸಂದೇಶ_2;
}

ಧ್ರುವ ವರ್ಣ *llvm::get_Bug_Report_ಸಂದೇಶ() {
  ಪ್ರತ್ಯಾಗಮ Bug_Report_ಸಂದೇಶ;
}

Pretty_ರಾಶಿ_Trace_Entry::Pretty_ರಾಶಿ_Trace_Entry() {
#ಯದಿ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_ರಾಶಿ_Trace_Head;
  Pretty_ರಾಶಿ_Trace_Head = ಏತತ್;
#endif
}

Pretty_ರಾಶಿ_Trace_Entry::~Pretty_ರಾಶಿ_Trace_Entry() {
#ಯದಿ ENABLE_BACKTRACES
  ಪ್ರತಿಜ್ಞಾ(Pretty_ರಾಶಿ_Trace_Head == ಏತತ್ &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ರಾಶಿ_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_String::print(raw_ostream &OS) ಧ್ರುವ { OS << Str << "\n"; }

Pretty_ರಾಶಿ_Trace_Format::Pretty_ರಾಶಿ_Trace_Format(ಧ್ರುವ ವರ್ಣ *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  ಯದಿ (SizeOrError < 0) {
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_Format::print(raw_ostream &OS) ಧ್ರುವ { OS << Str << "\n"; }

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_Program::print(raw_ostream &OS) ಧ್ರುವ {
  OS << "Program arguments: ";
  // Print the argument list.
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ I = 0; I < ArgC; ++I) {
    ಧ್ರುವ ತಾರ್ಕಿಕ HaveSpace = ::strchr(ArgV[I], ' ');
    ಯದಿ (I)
      OS << ' ';
    ಯದಿ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    ಯದಿ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#ಯದಿ ENABLE_BACKTRACES
ಸ್ಥಿರ ತಾರ್ಕಿಕ RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}
#endif

ಶೂನ್ಯ llvm::Enable_Pretty_ರಾಶಿ_Trace() {
#ಯದಿ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ಸ್ಥಿರ ತಾರ್ಕಿಕ HandlerRegistered = RegisterCrashPrinter();
  (ಶೂನ್ಯ)HandlerRegistered;
#endif
}

ಶೂನ್ಯ llvm::Enable_Pretty_ರಾಶಿ_Trace_On_Sig_Info_For_This_Thread(ತಾರ್ಕಿಕ ShouldEnable) {
#ಯದಿ ENABLE_BACKTRACES
  ಯದಿ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // The first time this is called, we register the SIGINFO handler.
  ಸ್ಥಿರ ತಾರ್ಕಿಕ HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }();
  (ಶೂನ್ಯ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ಧ್ರುವ ಶೂನ್ಯ *llvm::Save_Pretty_ರಾಶಿ_State() {
#ಯದಿ ENABLE_BACKTRACES
  ಪ್ರತ್ಯಾಗಮ Pretty_ರಾಶಿ_Trace_Head;
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ nullptr;
#endif
}

ಶೂನ್ಯ llvm::Restore_Pretty_ರಾಶಿ_State(ಧ್ರುವ ಶೂನ್ಯ *ತುದಿ) {
#ಯದಿ ENABLE_BACKTRACES
  Pretty_ರಾಶಿ_Trace_Head =
      static_cast<Pretty_ರಾಶಿ_Trace_Entry *>(const_cast<ಶೂನ್ಯ *>(ತುದಿ));
#endif
}

ಶೂನ್ಯ LLVMEnable_Pretty_ರಾಶಿ_Trace() {
  Enable_Pretty_ರಾಶಿ_Trace();
}
