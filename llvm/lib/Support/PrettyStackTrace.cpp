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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdarg>
#ಅನ್ತರ್ಭಾವ <cstdio>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <tuple>

#ಆದರೆ __has_include(<CrashReporterClient.h>)
#ಅನ್ತರ್ಭಾವ <CrashReporterClient.h>
#ವ್ಯಾಖ್ಯಾನಿಸು LLVM_HAVE_CRASHREPORTERCLIENT_H
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Bug_Report_ಸಂದೇಶ =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#ಆದರೆ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ಸ್ಥಿರ LLVM_THREAD_LOCAL Pretty_ರಾಶಿ_Trace_ನಮೂದು *Pretty_ರಾಶಿ_Trace_ತಲೆ = ಶೂನ್ಯಸೂಚಕ;

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
ಸ್ಥಿರ ಅಸ್ಥಿರ std::atomic<ಚಿಹ್ನೆರಹಿತ> GlobalSigInfoGenerationCounter = 1;
ಸ್ಥಿರ LLVM_THREAD_LOCAL ಚಿಹ್ನೆರಹಿತ ThreadLocalSigInfoGenerationCounter = 0;

ನಾಮಾಕಾಶ llvm {
Pretty_ರಾಶಿ_Trace_ನಮೂದು *ಹಿಮ್ಮುಖ_ರಾಶಿ_Trace(Pretty_ರಾಶಿ_Trace_ನಮೂದು *ತಲೆ_2) {
  Pretty_ರಾಶಿ_Trace_ನಮೂದು *Prev = ಶೂನ್ಯಸೂಚಕ;
  ಆಗ (ತಲೆ_2)
    std::tie(Prev, ತಲೆ_2, ತಲೆ_2->ಮುಂದಿನ_ನಮೂದು) =
        std::make_tuple(ತಲೆ_2, ತಲೆ_2->ಮುಂದಿನ_ನಮೂದು, Prev);
  ಹಿಂತಿರುಗಿ Prev;
}
} // namespace llvm

ಸ್ಥಿರ ಶೂನ್ಯ Print_ರಾಶಿ(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  ಚಿಹ್ನೆರಹಿತ ID = 0;
  SaveAndRestore<Pretty_ರಾಶಿ_Trace_ನಮೂದು *> Saved_ರಾಶಿ{Pretty_ರಾಶಿ_Trace_ತಲೆ,
                                                     ಶೂನ್ಯಸೂಚಕ};
  Pretty_ರಾಶಿ_Trace_ನಮೂದು *Reversed_ರಾಶಿ = ಹಿಮ್ಮುಖ_ರಾಶಿ_Trace(Saved_ರಾಶಿ.get());
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ Pretty_ರಾಶಿ_Trace_ನಮೂದು *ನಮೂದು = Reversed_ರಾಶಿ; ನಮೂದು;
       ನಮೂದು = ನಮೂದು->get_ಮುಂದಿನ_ನಮೂದು()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    ನಮೂದು->print(OS);
  }
  llvm::ಹಿಮ್ಮುಖ_ರಾಶಿ_Trace(Reversed_ರಾಶಿ);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ಸ್ಥಿರ ಶೂನ್ಯ Print_Cur_ರಾಶಿ_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  ಆದರೆ (!Pretty_ರಾಶಿ_Trace_ತಲೆ) ಹಿಂತಿರುಗಿ;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ರಾಶಿ(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#ಆದರೆ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
ಬಾಹ್ಯ "C" {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#ಇಲ್ಲದಿದ್ದರೆ
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ರಚನೆ crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_ಆವೃತ್ತಿ,
        0,
        0,
        0,
        0,
        0,
        0,
#ಆದರೆ CRASHREPORTER_ANNOTATIONS_ಆವೃತ್ತಿ > 4
        0
#ಆದರೆ_ಅಂತ್ಯ // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#ಆದರೆ_ಅಂತ್ಯ // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#ಅಥವಾಆದರೆ defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
ಬಾಹ್ಯ "C" ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#ಆದರೆ_ಅಂತ್ಯ

[[maybe_ಬಳಸದ]] ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Crash_Log_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Crash_Log_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_HAVE_CRASHREPORTERCLIENT_H
  (ಶೂನ್ಯ)CRSet_Crash_Log_ಸಂದೇಶ(ಸಂದೇಶ);
#ಅಥವಾಆದರೆ HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = ಸಂದೇಶ;
#ಆದರೆ_ಅಂತ್ಯ
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
ಬಳಸಿ CrashHandlerString = SmallString<2048>;
ಬಳಸಿ CrashHandlerStringStorage = std::byte[ಗಾತ್ರ(CrashHandlerString)];
alignas(CrashHandlerString) ಸ್ಥಿರ CrashHandlerStringStorage
    crashHandlerStringStorage;
#ಆದರೆ_ಅಂತ್ಯ

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ಸ್ಥಿರ ಶೂನ್ಯ CrashHandler(ಶೂನ್ಯ *) {
  errs() << Bug_Report_ಸಂದೇಶ ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ರಾಶಿ_Trace(errs());
#ಇಲ್ಲದಿದ್ದರೆ
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
  ಸ್ವಯಂಚಾಲಿತ &crashHandlerString =
      *ಹೊಸ (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  ಗಣ_Crash_Log_ಸಂದೇಶ(crashHandlerString.c_str());

  {
    raw_svector_ostream ಹರಿವು(crashHandlerString);
    Print_Cur_ರಾಶಿ_Trace(ಹರಿವು);
  }

  ಆದರೆ (!crashHandlerString.ಖಾಲಿಗಳು()) {
    ಗಣ_Crash_Log_ಸಂದೇಶ(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಣ_Crash_Log_ಸಂದೇಶ("No crash information.");
  }
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ printForSigInfoIfNeeded() {
  ಚಿಹ್ನೆರಹಿತ CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.ತುಂಬು(std::memory_order_relaxed);
  ಆದರೆ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    ಹಿಂತಿರುಗಿ;
  }

  Print_Cur_ರಾಶಿ_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#ಆದರೆ_ಅಂತ್ಯ // ENABLE_BACKTRACES

ಶೂನ್ಯ llvm::ಗಣ_Bug_Report_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_4) {
  Bug_Report_ಸಂದೇಶ = ಸಂದೇಶ_4;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *llvm::get_Bug_Report_ಸಂದೇಶ() {
  ಹಿಂತಿರುಗಿ Bug_Report_ಸಂದೇಶ;
}

Pretty_ರಾಶಿ_Trace_ನಮೂದು::Pretty_ರಾಶಿ_Trace_ನಮೂದು() {
#ಆದರೆ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  ಮುಂದಿನ_ನಮೂದು = Pretty_ರಾಶಿ_Trace_ತಲೆ;
  Pretty_ರಾಶಿ_Trace_ತಲೆ = ಇದು;
#ಆದರೆ_ಅಂತ್ಯ
}

Pretty_ರಾಶಿ_Trace_ನಮೂದು::~Pretty_ರಾಶಿ_Trace_ನಮೂದು() {
#ಆದರೆ ENABLE_BACKTRACES
  ಪ್ರತಿಜ್ಞಾ(Pretty_ರಾಶಿ_Trace_ತಲೆ == ಇದು &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ರಾಶಿ_Trace_ತಲೆ = ಮುಂದಿನ_ನಮೂದು;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_String::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ { OS << Str << "\n"; }

Pretty_ರಾಶಿ_Trace_Format::Pretty_ರಾಶಿ_Trace_Format(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Format, ...) {
  va_list AP;
  va_ಆರಂಭ(AP, Format);
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_Or_Error = vsnprintf(ಶೂನ್ಯಸೂಚಕ, 0, Format, AP);
  va_ಕೊನೆ(AP);
  ಆದರೆ (ಗಾತ್ರ_Or_Error < 0) {
    ಹಿಂತಿರುಗಿ;
  }

  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ_Or_Error + 1; // '\0'
  Str.resize(ಗಾತ್ರ_2);
  va_ಆರಂಭ(AP, Format);
  vsnprintf(Str.ದತ್ತಾಂಶ(), ಗಾತ್ರ_2, Format, AP);
  va_ಕೊನೆ(AP);
}

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_Format::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ { OS << Str << "\n"; }

ಶೂನ್ಯ Pretty_ರಾಶಿ_Trace_ಕಾರ್ಯಕ್ರಮ::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << "Program arguments: ";
  // Print the argument list.
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ I = 0; I < ArgC; ++I) {
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ HaveSpace = ::strchr(ArgV[I], ' ');
    ಆದರೆ (I)
      OS << ' ';
    ಆದರೆ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    ಆದರೆ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#ಆದರೆ ENABLE_BACKTRACES
ಸ್ಥಿರ ತಾರ್ಕಿಕ ರಿಜಿಸ್ಟರ್_Crash_Printer() {
  sys::ಕೂಡಿಸು_Signal_Handler(CrashHandler, ಶೂನ್ಯಸೂಚಕ);
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ llvm::Enable_Pretty_ರಾಶಿ_Trace() {
#ಆದರೆ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ಸ್ಥಿರ ತಾರ್ಕಿಕ HandlerRegistered = ರಿಜಿಸ್ಟರ್_Crash_Printer();
  (ಶೂನ್ಯ)HandlerRegistered;
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ llvm::Enable_Pretty_ರಾಶಿ_Trace_On_Sig_Info_For_This_Thread(ತಾರ್ಕಿಕ ShouldEnable) {
#ಆದರೆ ENABLE_BACKTRACES
  ಆದರೆ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    ಹಿಂತಿರುಗಿ;
  }

  // The first time this is called, we register the SIGINFO handler.
  ಸ್ಥಿರ ತಾರ್ಕಿಕ HandlerRegistered = []{
    sys::ಗಣ_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_ಕೂಡಿಸು(1, std::memory_order_relaxed);
    });
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }();
  (ಶೂನ್ಯ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.ತುಂಬು(std::memory_order_relaxed);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *llvm::Save_Pretty_ರಾಶಿ_State() {
#ಆದರೆ ENABLE_BACKTRACES
  ಹಿಂತಿರುಗಿ Pretty_ರಾಶಿ_Trace_ತಲೆ;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ llvm::Restore_Pretty_ರಾಶಿ_State(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ತುದಿ) {
#ಆದರೆ ENABLE_BACKTRACES
  Pretty_ರಾಶಿ_Trace_ತಲೆ =
      static_cast<Pretty_ರಾಶಿ_Trace_ನಮೂದು *>(const_cast<ಶೂನ್ಯ *>(ತುದಿ));
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ LLVMEnable_Pretty_ರಾಶಿ_Trace() {
  Enable_Pretty_ರಾಶಿ_Trace();
}
