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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/PrettyStackTrace.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm-c/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SaveAndRestore.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Watchdog.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdarg>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdio>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <tuple>

#ਜੇ __has_include(<CrashReporterClient.h>)
#ਅਨ੍ਤਰ੍ਭਾਵ <CrashReporterClient.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LLVM_HAVE_CRASHREPORTERCLIENT_H
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *Bug_Report_ਸੁਨੇਹਾ =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#ਜੇ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ਸਥਿਰ LLVM_THREAD_LOCAL Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *Pretty_ਢੇਰ_Trace_ਸਿਰ = ਖਾਲੀ_ਸੂਚਕ;

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
ਸਥਿਰ ਅਸਥਿਰ std::atomic<ਬਿਨਾਨਿਸ਼ਾਨੀ> GlobalSigInfoGenerationCounter = 1;
ਸਥਿਰ LLVM_THREAD_LOCAL ਬਿਨਾਨਿਸ਼ਾਨੀ ThreadLocalSigInfoGenerationCounter = 0;

ਨਾਮਾਕਾਸ਼ llvm {
Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *ਉਲਟ_ਢੇਰ_Trace(Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *ਸਿਰ_2) {
  Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *Prev = ਖਾਲੀ_ਸੂਚਕ;
  ਜਦੋਂ (ਸਿਰ_2)
    std::tie(Prev, ਸਿਰ_2, ਸਿਰ_2->ਅਗਲਾ_ਇੰਦਰਾਜ) =
        std::make_tuple(ਸਿਰ_2, ਸਿਰ_2->ਅਗਲਾ_ਇੰਦਰਾਜ, Prev);
  ਵਾਪਸ Prev;
}
} // namespace llvm

ਸਥਿਰ ਖਾਲੀ Print_ਢੇਰ(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ID = 0;
  SaveAndRestore<Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *> Saved_ਢੇਰ{Pretty_ਢੇਰ_Trace_ਸਿਰ,
                                                     ਖਾਲੀ_ਸੂਚਕ};
  Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *Reversed_ਢੇਰ = ਉਲਟ_ਢੇਰ_Trace(Saved_ਢੇਰ.get());
  ਲਈ (ਅਚਲ Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = Reversed_ਢੇਰ; ਇੰਦਰਾਜ;
       ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->get_ਅਗਲਾ_ਇੰਦਰਾਜ()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    ਇੰਦਰਾਜ->print(OS);
  }
  llvm::ਉਲਟ_ਢੇਰ_Trace(Reversed_ਢੇਰ);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ਸਥਿਰ ਖਾਲੀ Print_Cur_ਢੇਰ_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  ਜੇ (!Pretty_ਢੇਰ_Trace_ਸਿਰ) ਵਾਪਸ;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ਢੇਰ(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#ਜੇ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
ਬਾਹਰੀ "C" {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#ਨਹੀਂਤਾਂ
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ਬਣਤਰ crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_ਸੰਸਕਰਣ,
        0,
        0,
        0,
        0,
        0,
        0,
#ਜੇ CRASHREPORTER_ANNOTATIONS_ਸੰਸਕਰਣ > 4
        0
#ਜੇ_ਅੰਤ // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#ਜੇ_ਅੰਤ // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#ਜਾਂਜੇ defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
ਬਾਹਰੀ "C" ਅਚਲ ਅੱਖਰ *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#ਜੇ_ਅੰਤ

[[maybe_ਅਣਵਰਤਿਆ]] ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Crash_Log_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Crash_Log_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_HAVE_CRASHREPORTERCLIENT_H
  (ਖਾਲੀ)CRSet_Crash_Log_ਸੁਨੇਹਾ(ਸੁਨੇਹਾ);
#ਜਾਂਜੇ HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = ਸੁਨੇਹਾ;
#ਜੇ_ਅੰਤ
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
ਵਰਤੋਂ CrashHandlerString = SmallString<2048>;
ਵਰਤੋਂ CrashHandlerStringStorage = std::byte[ਆਕਾਰ(CrashHandlerString)];
alignas(CrashHandlerString) ਸਥਿਰ CrashHandlerStringStorage
    crashHandlerStringStorage;
#ਜੇ_ਅੰਤ

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ਸਥਿਰ ਖਾਲੀ CrashHandler(ਖਾਲੀ *) {
  errs() << Bug_Report_ਸੁਨੇਹਾ ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ਢੇਰ_Trace(errs());
#ਨਹੀਂਤਾਂ
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
  ਸਵੈਚਾਲਿਤ &crashHandlerString =
      *ਨਵਾਂ (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  ਸੈੱਟ_Crash_Log_ਸੁਨੇਹਾ(crashHandlerString.c_str());

  {
    raw_svector_ostream ਧਾਰਾ(crashHandlerString);
    Print_Cur_ਢੇਰ_Trace(ਧਾਰਾ);
  }

  ਜੇ (!crashHandlerString.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਸੈੱਟ_Crash_Log_ਸੁਨੇਹਾ(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } ਨਹੀਂਤਾਂ {
    ਸੈੱਟ_Crash_Log_ਸੁਨੇਹਾ("No crash information.");
  }
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ printForSigInfoIfNeeded() {
  ਬਿਨਾਨਿਸ਼ਾਨੀ CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.ਲੱਦੋ(std::memory_order_relaxed);
  ਜੇ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    ਵਾਪਸ;
  }

  Print_Cur_ਢੇਰ_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#ਜੇ_ਅੰਤ // ENABLE_BACKTRACES

ਖਾਲੀ llvm::ਸੈੱਟ_Bug_Report_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_4) {
  Bug_Report_ਸੁਨੇਹਾ = ਸੁਨੇਹਾ_4;
}

ਅਚਲ ਅੱਖਰ *llvm::get_Bug_Report_ਸੁਨੇਹਾ() {
  ਵਾਪਸ Bug_Report_ਸੁਨੇਹਾ;
}

Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ::Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ() {
#ਜੇ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  ਅਗਲਾ_ਇੰਦਰਾਜ = Pretty_ਢੇਰ_Trace_ਸਿਰ;
  Pretty_ਢੇਰ_Trace_ਸਿਰ = ਇਹ;
#ਜੇ_ਅੰਤ
}

Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ::~Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ() {
#ਜੇ ENABLE_BACKTRACES
  ਪ੍ਰਤਿਜ੍ਞਾ(Pretty_ਢੇਰ_Trace_ਸਿਰ == ਇਹ &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ਢੇਰ_Trace_ਸਿਰ = ਅਗਲਾ_ਇੰਦਰਾਜ;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ Pretty_ਢੇਰ_Trace_String::print(raw_ostream &OS) ਅਚਲ { OS << Str << "\n"; }

Pretty_ਢੇਰ_Trace_Format::Pretty_ਢੇਰ_Trace_Format(ਅਚਲ ਅੱਖਰ *Format, ...) {
  va_list AP;
  va_ਸ਼ੁਰੂ(AP, Format);
  ਅਚਲ ਪੂਰਨ ਮਾਪ_Or_Error = vsnprintf(ਖਾਲੀ_ਸੂਚਕ, 0, Format, AP);
  va_ਸਿਰਾ(AP);
  ਜੇ (ਮਾਪ_Or_Error < 0) {
    ਵਾਪਸ;
  }

  ਅਚਲ ਪੂਰਨ ਮਾਪ = ਮਾਪ_Or_Error + 1; // '\0'
  Str.resize(ਮਾਪ);
  va_ਸ਼ੁਰੂ(AP, Format);
  vsnprintf(Str.ਅੰਕੜੇ(), ਮਾਪ, Format, AP);
  va_ਸਿਰਾ(AP);
}

ਖਾਲੀ Pretty_ਢੇਰ_Trace_Format::print(raw_ostream &OS) ਅਚਲ { OS << Str << "\n"; }

ਖਾਲੀ Pretty_ਢੇਰ_Trace_ਪ੍ਰੋਗਰਾਮ::print(raw_ostream &OS) ਅਚਲ {
  OS << "Program arguments: ";
  // Print the argument list.
  ਲਈ (ਪੂਰਨ I = 0; I < ArgC; ++I) {
    ਅਚਲ ਤਾਰਕਿਕ HaveSpace = ::strchr(ArgV[I], ' ');
    ਜੇ (I)
      OS << ' ';
    ਜੇ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    ਜੇ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#ਜੇ ENABLE_BACKTRACES
ਸਥਿਰ ਤਾਰਕਿਕ ਰਜਿਸਟਰ_Crash_Printer() {
  sys::ਜੋੜੋ_Signal_Handler(CrashHandler, ਖਾਲੀ_ਸੂਚਕ);
  ਵਾਪਸ ਝੂਠ;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ llvm::Enable_Pretty_ਢੇਰ_Trace() {
#ਜੇ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ਸਥਿਰ ਤਾਰਕਿਕ HandlerRegistered = ਰਜਿਸਟਰ_Crash_Printer();
  (ਖਾਲੀ)HandlerRegistered;
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ llvm::Enable_Pretty_ਢੇਰ_Trace_On_Sig_Info_For_This_Thread(ਤਾਰਕਿਕ ShouldEnable) {
#ਜੇ ENABLE_BACKTRACES
  ਜੇ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    ਵਾਪਸ;
  }

  // The first time this is called, we register the SIGINFO handler.
  ਸਥਿਰ ਤਾਰਕਿਕ HandlerRegistered = []{
    sys::ਸੈੱਟ_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_ਜੋੜੋ(1, std::memory_order_relaxed);
    });
    ਵਾਪਸ ਝੂਠ;
  }();
  (ਖਾਲੀ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.ਲੱਦੋ(std::memory_order_relaxed);
#ਜੇ_ਅੰਤ
}

ਅਚਲ ਖਾਲੀ *llvm::Save_Pretty_ਢੇਰ_State() {
#ਜੇ ENABLE_BACKTRACES
  ਵਾਪਸ Pretty_ਢੇਰ_Trace_ਸਿਰ;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ llvm::Restore_Pretty_ਢੇਰ_State(ਅਚਲ ਖਾਲੀ *ਸਿਖਰ) {
#ਜੇ ENABLE_BACKTRACES
  Pretty_ਢੇਰ_Trace_ਸਿਰ =
      static_cast<Pretty_ਢੇਰ_Trace_ਇੰਦਰਾਜ *>(const_cast<ਖਾਲੀ *>(ਸਿਖਰ));
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ LLVMEnable_Pretty_ਢੇਰ_Trace() {
  Enable_Pretty_ਢੇਰ_Trace();
}
