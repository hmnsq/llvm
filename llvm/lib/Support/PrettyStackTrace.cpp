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

#அந்தர்பாவ "llvm/Support/PrettyStackTrace.h"
#அந்தர்பாவ "llvm-c/ErrorHandling.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/SaveAndRestore.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/Watchdog.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#endif

#அந்தர்பாவ <atomic>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdarg>
#அந்தர்பாவ <cstdio>
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <tuple>

#எனில் __has_include(<CrashReporterClient.h>)
#அந்தர்பாவ <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using நாமாகாஶ llvm;

நிலையான மாறிலி எழுத்து *Bug_Report_செய்தி =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#எனில் ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
நிலையான LLVM_THREAD_LOCAL Pretty_அடுக்கு_Trace_Entry *Pretty_அடுக்கு_Trace_Head = nullptr;

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
நிலையான நிலையற்ற std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
நிலையான LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

நாமாகாஶ llvm {
Pretty_அடுக்கு_Trace_Entry *Reverse_அடுக்கு_Trace(Pretty_அடுக்கு_Trace_Entry *Head) {
  Pretty_அடுக்கு_Trace_Entry *Prev = nullptr;
  போது (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  திரும்பு Prev;
}
} // namespace llvm

நிலையான வெறுமை Print_அடுக்கு(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_அடுக்கு_Trace_Entry *> Saved_அடுக்கு{Pretty_அடுக்கு_Trace_Head,
                                                     nullptr};
  Pretty_அடுக்கு_Trace_Entry *Reversed_அடுக்கு = Reverse_அடுக்கு_Trace(Saved_அடுக்கு.get());
  க்கு (மாறிலி Pretty_அடுக்கு_Trace_Entry *Entry = Reversed_அடுக்கு; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_அடுக்கு_Trace(Reversed_அடுக்கு);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
நிலையான வெறுமை Print_Cur_அடுக்கு_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  எனில் (!Pretty_அடுக்கு_Trace_Head) திரும்பு;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_அடுக்கு(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#எனில் defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
வெளி "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#இல்லையெனில்
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
கட்டமைப்பு crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#எனில் CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#அல்லதெனில் defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
வெளி "C" மாறிலி எழுத்து *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] நிலையான வெறுமை setCrashLogMessage(மாறிலி எழுத்து *செய்தி);
நிலையான வெறுமை setCrashLogMessage(மாறிலி எழுத்து *செய்தி) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (வெறுமை)CRSetCrashLogMessage(செய்தி);
#அல்லதெனில் HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = செய்தி;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[அளவு(CrashHandlerString)];
alignas(CrashHandlerString) நிலையான CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
நிலையான வெறுமை CrashHandler(வெறுமை *) {
  errs() << Bug_Report_செய்தி ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_அடுக்கு_Trace(errs());
#இல்லையெனில்
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
      *புதிய (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_அடுக்கு_Trace(Stream);
  }

  எனில் (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } இல்லையெனில் {
    setCrashLogMessage("No crash information.");
  }
#endif
}

நிலையான வெறுமை printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  எனில் (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    திரும்பு;
  }

  Print_Cur_அடுக்கு_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

வெறுமை llvm::set_Bug_Report_செய்தி(மாறிலி எழுத்து *செய்தி_2) {
  Bug_Report_செய்தி = செய்தி_2;
}

மாறிலி எழுத்து *llvm::get_Bug_Report_செய்தி() {
  திரும்பு Bug_Report_செய்தி;
}

Pretty_அடுக்கு_Trace_Entry::Pretty_அடுக்கு_Trace_Entry() {
#எனில் ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_அடுக்கு_Trace_Head;
  Pretty_அடுக்கு_Trace_Head = இது;
#endif
}

Pretty_அடுக்கு_Trace_Entry::~Pretty_அடுக்கு_Trace_Entry() {
#எனில் ENABLE_BACKTRACES
  ப்ரதிஜ்ஞா(Pretty_அடுக்கு_Trace_Head == இது &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_அடுக்கு_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

வெறுமை Pretty_அடுக்கு_Trace_String::print(raw_ostream &OS) மாறிலி { OS << Str << "\n"; }

Pretty_அடுக்கு_Trace_Format::Pretty_அடுக்கு_Trace_Format(மாறிலி எழுத்து *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  மாறிலி முழு SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  எனில் (SizeOrError < 0) {
    திரும்பு;
  }

  மாறிலி முழு Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

வெறுமை Pretty_அடுக்கு_Trace_Format::print(raw_ostream &OS) மாறிலி { OS << Str << "\n"; }

வெறுமை Pretty_அடுக்கு_Trace_Program::print(raw_ostream &OS) மாறிலி {
  OS << "Program arguments: ";
  // Print the argument list.
  க்கு (முழு I = 0; I < ArgC; ++I) {
    மாறிலி தருக்க HaveSpace = ::strchr(ArgV[I], ' ');
    எனில் (I)
      OS << ' ';
    எனில் (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    எனில் (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#எனில் ENABLE_BACKTRACES
நிலையான தருக்க RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  திரும்பு பொய்;
}
#endif

வெறுமை llvm::Enable_Pretty_அடுக்கு_Trace() {
#எனில் ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  நிலையான தருக்க HandlerRegistered = RegisterCrashPrinter();
  (வெறுமை)HandlerRegistered;
#endif
}

வெறுமை llvm::Enable_Pretty_அடுக்கு_Trace_On_Sig_Info_For_This_Thread(தருக்க ShouldEnable) {
#எனில் ENABLE_BACKTRACES
  எனில் (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    திரும்பு;
  }

  // The first time this is called, we register the SIGINFO handler.
  நிலையான தருக்க HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    திரும்பு பொய்;
  }();
  (வெறுமை)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

மாறிலி வெறுமை *llvm::Save_Pretty_அடுக்கு_State() {
#எனில் ENABLE_BACKTRACES
  திரும்பு Pretty_அடுக்கு_Trace_Head;
#இல்லையெனில்
  திரும்பு nullptr;
#endif
}

வெறுமை llvm::Restore_Pretty_அடுக்கு_State(மாறிலி வெறுமை *உச்சி) {
#எனில் ENABLE_BACKTRACES
  Pretty_அடுக்கு_Trace_Head =
      static_cast<Pretty_அடுக்கு_Trace_Entry *>(const_cast<வெறுமை *>(உச்சி));
#endif
}

வெறுமை LLVMEnable_Pretty_அடுக்கு_Trace() {
  Enable_Pretty_அடுக்கு_Trace();
}
