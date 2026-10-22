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

#അന്തര്ഭാവ "llvm/Support/PrettyStackTrace.h"
#അന്തര്ഭാവ "llvm-c/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/SaveAndRestore.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/Watchdog.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#endif

#അന്തര്ഭാവ <atomic>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstdarg>
#അന്തര്ഭാവ <cstdio>
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <tuple>

#എങ്കിൽ __has_include(<CrashReporterClient.h>)
#അന്തര്ഭാവ <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using നാമാകാശ llvm;

സ്ഥിതം സ്ഥിരം അക്ഷരം *Bug_Report_സന്ദേശം =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#എങ്കിൽ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
സ്ഥിതം LLVM_THREAD_LOCAL Pretty_അട്ടി_Trace_Entry *Pretty_അട്ടി_Trace_Head = nullptr;

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
സ്ഥിതം അസ്ഥിര std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
സ്ഥിതം LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

നാമാകാശ llvm {
Pretty_അട്ടി_Trace_Entry *Reverse_അട്ടി_Trace(Pretty_അട്ടി_Trace_Entry *Head) {
  Pretty_അട്ടി_Trace_Entry *Prev = nullptr;
  ആകുമ്പോൾ (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  തിരികെ Prev;
}
} // namespace llvm

സ്ഥിതം ശൂന്യം Print_അട്ടി(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_അട്ടി_Trace_Entry *> Saved_അട്ടി{Pretty_അട്ടി_Trace_Head,
                                                     nullptr};
  Pretty_അട്ടി_Trace_Entry *Reversed_അട്ടി = Reverse_അട്ടി_Trace(Saved_അട്ടി.get());
  വേണ്ടി (സ്ഥിരം Pretty_അട്ടി_Trace_Entry *Entry = Reversed_അട്ടി; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_അട്ടി_Trace(Reversed_അട്ടി);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
സ്ഥിതം ശൂന്യം Print_Cur_അട്ടി_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  എങ്കിൽ (!Pretty_അട്ടി_Trace_Head) തിരികെ;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_അട്ടി(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#എങ്കിൽ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
ബാഹ്യ "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#അല്ലെങ്കിൽ
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ഘടന crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#എങ്കിൽ CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#അല്ലെങ്കിലെങ്കിൽ defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
ബാഹ്യ "C" സ്ഥിരം അക്ഷരം *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] സ്ഥിതം ശൂന്യം setCrashLogMessage(സ്ഥിരം അക്ഷരം *സന്ദേശം);
സ്ഥിതം ശൂന്യം setCrashLogMessage(സ്ഥിരം അക്ഷരം *സന്ദേശം) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (ശൂന്യം)CRSetCrashLogMessage(സന്ദേശം);
#അല്ലെങ്കിലെങ്കിൽ HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = സന്ദേശം;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[വലുപ്പം(CrashHandlerString)];
alignas(CrashHandlerString) സ്ഥിതം CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
സ്ഥിതം ശൂന്യം CrashHandler(ശൂന്യം *) {
  errs() << Bug_Report_സന്ദേശം ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_അട്ടി_Trace(errs());
#അല്ലെങ്കിൽ
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
      *പുതിയ (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_അട്ടി_Trace(Stream);
  }

  എങ്കിൽ (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } അല്ലെങ്കിൽ {
    setCrashLogMessage("No crash information.");
  }
#endif
}

സ്ഥിതം ശൂന്യം printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  എങ്കിൽ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    തിരികെ;
  }

  Print_Cur_അട്ടി_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

ശൂന്യം llvm::set_Bug_Report_സന്ദേശം(സ്ഥിരം അക്ഷരം *സന്ദേശം_2) {
  Bug_Report_സന്ദേശം = സന്ദേശം_2;
}

സ്ഥിരം അക്ഷരം *llvm::get_Bug_Report_സന്ദേശം() {
  തിരികെ Bug_Report_സന്ദേശം;
}

Pretty_അട്ടി_Trace_Entry::Pretty_അട്ടി_Trace_Entry() {
#എങ്കിൽ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_അട്ടി_Trace_Head;
  Pretty_അട്ടി_Trace_Head = ഇത്;
#endif
}

Pretty_അട്ടി_Trace_Entry::~Pretty_അട്ടി_Trace_Entry() {
#എങ്കിൽ ENABLE_BACKTRACES
  പ്രതിജ്ഞാ(Pretty_അട്ടി_Trace_Head == ഇത് &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_അട്ടി_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

ശൂന്യം Pretty_അട്ടി_Trace_String::print(raw_ostream &OS) സ്ഥിരം { OS << Str << "\n"; }

Pretty_അട്ടി_Trace_Format::Pretty_അട്ടി_Trace_Format(സ്ഥിരം അക്ഷരം *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  സ്ഥിരം പൂർണ്ണസംഖ്യ SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  എങ്കിൽ (SizeOrError < 0) {
    തിരികെ;
  }

  സ്ഥിരം പൂർണ്ണസംഖ്യ Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

ശൂന്യം Pretty_അട്ടി_Trace_Format::print(raw_ostream &OS) സ്ഥിരം { OS << Str << "\n"; }

ശൂന്യം Pretty_അട്ടി_Trace_Program::print(raw_ostream &OS) സ്ഥിരം {
  OS << "Program arguments: ";
  // Print the argument list.
  വേണ്ടി (പൂർണ്ണസംഖ്യ I = 0; I < ArgC; ++I) {
    സ്ഥിരം തർക്കം HaveSpace = ::strchr(ArgV[I], ' ');
    എങ്കിൽ (I)
      OS << ' ';
    എങ്കിൽ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    എങ്കിൽ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#എങ്കിൽ ENABLE_BACKTRACES
സ്ഥിതം തർക്കം RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  തിരികെ അസത്യം;
}
#endif

ശൂന്യം llvm::Enable_Pretty_അട്ടി_Trace() {
#എങ്കിൽ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  സ്ഥിതം തർക്കം HandlerRegistered = RegisterCrashPrinter();
  (ശൂന്യം)HandlerRegistered;
#endif
}

ശൂന്യം llvm::Enable_Pretty_അട്ടി_Trace_On_Sig_Info_For_This_Thread(തർക്കം ShouldEnable) {
#എങ്കിൽ ENABLE_BACKTRACES
  എങ്കിൽ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    തിരികെ;
  }

  // The first time this is called, we register the SIGINFO handler.
  സ്ഥിതം തർക്കം HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    തിരികെ അസത്യം;
  }();
  (ശൂന്യം)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

സ്ഥിരം ശൂന്യം *llvm::Save_Pretty_അട്ടി_State() {
#എങ്കിൽ ENABLE_BACKTRACES
  തിരികെ Pretty_അട്ടി_Trace_Head;
#അല്ലെങ്കിൽ
  തിരികെ nullptr;
#endif
}

ശൂന്യം llvm::Restore_Pretty_അട്ടി_State(സ്ഥിരം ശൂന്യം *മുകൾ) {
#എങ്കിൽ ENABLE_BACKTRACES
  Pretty_അട്ടി_Trace_Head =
      static_cast<Pretty_അട്ടി_Trace_Entry *>(const_cast<ശൂന്യം *>(മുകൾ));
#endif
}

ശൂന്യം LLVMEnable_Pretty_അട്ടി_Trace() {
  Enable_Pretty_അട്ടി_Trace();
}
