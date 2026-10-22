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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/PrettyStackTrace.h"
#ଅନ୍ତର୍ଭାଵ "llvm-c/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SaveAndRestore.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Watchdog.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#endif

#ଅନ୍ତର୍ଭାଵ <atomic>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstdarg>
#ଅନ୍ତର୍ଭାଵ <cstdio>
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <tuple>

#ଯଦି __has_include(<CrashReporterClient.h>)
#ଅନ୍ତର୍ଭାଵ <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using ନାମାକାଶ llvm;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *Bug_Report_ସନ୍ଦେଶ =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#ଯଦି ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ସ୍ଥିର LLVM_THREAD_LOCAL Pretty_ଗଦା_Trace_Entry *Pretty_ଗଦା_Trace_Head = nullptr;

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
ସ୍ଥିର ଅସ୍ଥିର std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
ସ୍ଥିର LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

ନାମାକାଶ llvm {
Pretty_ଗଦା_Trace_Entry *Reverse_ଗଦା_Trace(Pretty_ଗଦା_Trace_Entry *Head) {
  Pretty_ଗଦା_Trace_Entry *Prev = nullptr;
  ଯେତେବେଳେ (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  ଫେରନ୍ତୁ Prev;
}
} // namespace llvm

ସ୍ଥିର ଶୂନ୍ୟ Print_ଗଦା(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_ଗଦା_Trace_Entry *> Saved_ଗଦା{Pretty_ଗଦା_Trace_Head,
                                                     nullptr};
  Pretty_ଗଦା_Trace_Entry *Reversed_ଗଦା = Reverse_ଗଦା_Trace(Saved_ଗଦା.get());
  ପାଇଁ (ଅଚଳ Pretty_ଗଦା_Trace_Entry *Entry = Reversed_ଗଦା; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_ଗଦା_Trace(Reversed_ଗଦା);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ସ୍ଥିର ଶୂନ୍ୟ Print_Cur_ଗଦା_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  ଯଦି (!Pretty_ଗଦା_Trace_Head) ଫେରନ୍ତୁ;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ଗଦା(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#ଯଦି defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
ବାହ୍ୟ "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#ଅନ୍ୟଥା
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ସଂରଚନା crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#ଯଦି CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#ନଚେତଯଦି defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
ବାହ୍ୟ "C" ଅଚଳ ଅକ୍ଷର *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] ସ୍ଥିର ଶୂନ୍ୟ setCrashLogMessage(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ);
ସ୍ଥିର ଶୂନ୍ୟ setCrashLogMessage(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (ଶୂନ୍ୟ)CRSetCrashLogMessage(ସନ୍ଦେଶ);
#ନଚେତଯଦି HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = ସନ୍ଦେଶ;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[ଆକାର(CrashHandlerString)];
alignas(CrashHandlerString) ସ୍ଥିର CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ସ୍ଥିର ଶୂନ୍ୟ CrashHandler(ଶୂନ୍ୟ *) {
  errs() << Bug_Report_ସନ୍ଦେଶ ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ଗଦା_Trace(errs());
#ଅନ୍ୟଥା
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
      *ନୂତନ (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_ଗଦା_Trace(Stream);
  }

  ଯଦି (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } ଅନ୍ୟଥା {
    setCrashLogMessage("No crash information.");
  }
#endif
}

ସ୍ଥିର ଶୂନ୍ୟ printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  ଯଦି (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    ଫେରନ୍ତୁ;
  }

  Print_Cur_ଗଦା_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

ଶୂନ୍ୟ llvm::set_Bug_Report_ସନ୍ଦେଶ(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ_2) {
  Bug_Report_ସନ୍ଦେଶ = ସନ୍ଦେଶ_2;
}

ଅଚଳ ଅକ୍ଷର *llvm::get_Bug_Report_ସନ୍ଦେଶ() {
  ଫେରନ୍ତୁ Bug_Report_ସନ୍ଦେଶ;
}

Pretty_ଗଦା_Trace_Entry::Pretty_ଗଦା_Trace_Entry() {
#ଯଦି ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_ଗଦା_Trace_Head;
  Pretty_ଗଦା_Trace_Head = ଏହି;
#endif
}

Pretty_ଗଦା_Trace_Entry::~Pretty_ଗଦା_Trace_Entry() {
#ଯଦି ENABLE_BACKTRACES
  ପ୍ରତିଜ୍ଞା(Pretty_ଗଦା_Trace_Head == ଏହି &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ଗଦା_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

ଶୂନ୍ୟ Pretty_ଗଦା_Trace_String::print(raw_ostream &OS) ଅଚଳ { OS << Str << "\n"; }

Pretty_ଗଦା_Trace_Format::Pretty_ଗଦା_Trace_Format(ଅଚଳ ଅକ୍ଷର *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  ଯଦି (SizeOrError < 0) {
    ଫେରନ୍ତୁ;
  }

  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

ଶୂନ୍ୟ Pretty_ଗଦା_Trace_Format::print(raw_ostream &OS) ଅଚଳ { OS << Str << "\n"; }

ଶୂନ୍ୟ Pretty_ଗଦା_Trace_Program::print(raw_ostream &OS) ଅଚଳ {
  OS << "Program arguments: ";
  // Print the argument list.
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା I = 0; I < ArgC; ++I) {
    ଅଚଳ ତାର୍କିକ HaveSpace = ::strchr(ArgV[I], ' ');
    ଯଦି (I)
      OS << ' ';
    ଯଦି (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    ଯଦି (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#ଯଦି ENABLE_BACKTRACES
ସ୍ଥିର ତାର୍କିକ RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}
#endif

ଶୂନ୍ୟ llvm::Enable_Pretty_ଗଦା_Trace() {
#ଯଦି ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ସ୍ଥିର ତାର୍କିକ HandlerRegistered = RegisterCrashPrinter();
  (ଶୂନ୍ୟ)HandlerRegistered;
#endif
}

ଶୂନ୍ୟ llvm::Enable_Pretty_ଗଦା_Trace_On_Sig_Info_For_This_Thread(ତାର୍କିକ ShouldEnable) {
#ଯଦି ENABLE_BACKTRACES
  ଯଦି (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    ଫେରନ୍ତୁ;
  }

  // The first time this is called, we register the SIGINFO handler.
  ସ୍ଥିର ତାର୍କିକ HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }();
  (ଶୂନ୍ୟ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ଅଚଳ ଶୂନ୍ୟ *llvm::Save_Pretty_ଗଦା_State() {
#ଯଦି ENABLE_BACKTRACES
  ଫେରନ୍ତୁ Pretty_ଗଦା_Trace_Head;
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ nullptr;
#endif
}

ଶୂନ୍ୟ llvm::Restore_Pretty_ଗଦା_State(ଅଚଳ ଶୂନ୍ୟ *ଶୀର୍ଷ) {
#ଯଦି ENABLE_BACKTRACES
  Pretty_ଗଦା_Trace_Head =
      static_cast<Pretty_ଗଦା_Trace_Entry *>(const_cast<ଶୂନ୍ୟ *>(ଶୀର୍ଷ));
#endif
}

ଶୂନ୍ୟ LLVMEnable_Pretty_ଗଦା_Trace() {
  Enable_Pretty_ଗଦା_Trace();
}
