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

#અન્તર્ભાવ "llvm/Support/PrettyStackTrace.h"
#અન્તર્ભાવ "llvm-c/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/SaveAndRestore.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/Watchdog.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#endif

#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdarg>
#અન્તર્ભાવ <cstdio>
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <tuple>

#જો __has_include(<CrashReporterClient.h>)
#અન્તર્ભાવ <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using નામાકાશ llvm;

સ્થિર અચલ અક્ષર *Bug_Report_સંદેશ =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#જો ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
સ્થિર LLVM_THREAD_LOCAL Pretty_ઢગલો_Trace_Entry *Pretty_ઢગલો_Trace_Head = nullptr;

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
સ્થિર અસ્થિર std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
સ્થિર LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

નામાકાશ llvm {
Pretty_ઢગલો_Trace_Entry *Reverse_ઢગલો_Trace(Pretty_ઢગલો_Trace_Entry *Head) {
  Pretty_ઢગલો_Trace_Entry *Prev = nullptr;
  જ્યારે (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  પાછા Prev;
}
} // namespace llvm

સ્થિર શૂન્ય Print_ઢગલો(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_ઢગલો_Trace_Entry *> Saved_ઢગલો{Pretty_ઢગલો_Trace_Head,
                                                     nullptr};
  Pretty_ઢગલો_Trace_Entry *Reversed_ઢગલો = Reverse_ઢગલો_Trace(Saved_ઢગલો.get());
  માટે (અચલ Pretty_ઢગલો_Trace_Entry *Entry = Reversed_ઢગલો; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_ઢગલો_Trace(Reversed_ઢગલો);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
સ્થિર શૂન્ય Print_Cur_ઢગલો_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  જો (!Pretty_ઢગલો_Trace_Head) પાછા;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ઢગલો(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#જો defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
બાહ્ય "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#નહીંતર
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
સંરચના crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#જો CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#અથવાજો defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
બાહ્ય "C" અચલ અક્ષર *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] સ્થિર શૂન્ય setCrashLogMessage(અચલ અક્ષર *સંદેશ);
સ્થિર શૂન્ય setCrashLogMessage(અચલ અક્ષર *સંદેશ) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (શૂન્ય)CRSetCrashLogMessage(સંદેશ);
#અથવાજો HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = સંદેશ;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[કદ(CrashHandlerString)];
alignas(CrashHandlerString) સ્થિર CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
સ્થિર શૂન્ય CrashHandler(શૂન્ય *) {
  errs() << Bug_Report_સંદેશ ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ઢગલો_Trace(errs());
#નહીંતર
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
      *નવું (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_ઢગલો_Trace(Stream);
  }

  જો (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } નહીંતર {
    setCrashLogMessage("No crash information.");
  }
#endif
}

સ્થિર શૂન્ય printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  જો (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    પાછા;
  }

  Print_Cur_ઢગલો_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

શૂન્ય llvm::set_Bug_Report_સંદેશ(અચલ અક્ષર *સંદેશ_2) {
  Bug_Report_સંદેશ = સંદેશ_2;
}

અચલ અક્ષર *llvm::get_Bug_Report_સંદેશ() {
  પાછા Bug_Report_સંદેશ;
}

Pretty_ઢગલો_Trace_Entry::Pretty_ઢગલો_Trace_Entry() {
#જો ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_ઢગલો_Trace_Head;
  Pretty_ઢગલો_Trace_Head = આ;
#endif
}

Pretty_ઢગલો_Trace_Entry::~Pretty_ઢગલો_Trace_Entry() {
#જો ENABLE_BACKTRACES
  પ્રતિજ્ઞા(Pretty_ઢગલો_Trace_Head == આ &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ઢગલો_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

શૂન્ય Pretty_ઢગલો_Trace_String::print(raw_ostream &OS) અચલ { OS << Str << "\n"; }

Pretty_ઢગલો_Trace_Format::Pretty_ઢગલો_Trace_Format(અચલ અક્ષર *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  અચલ પૂર્ણાંક SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  જો (SizeOrError < 0) {
    પાછા;
  }

  અચલ પૂર્ણાંક Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

શૂન્ય Pretty_ઢગલો_Trace_Format::print(raw_ostream &OS) અચલ { OS << Str << "\n"; }

શૂન્ય Pretty_ઢગલો_Trace_Program::print(raw_ostream &OS) અચલ {
  OS << "Program arguments: ";
  // Print the argument list.
  માટે (પૂર્ણાંક I = 0; I < ArgC; ++I) {
    અચલ તાર્કિક HaveSpace = ::strchr(ArgV[I], ' ');
    જો (I)
      OS << ' ';
    જો (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    જો (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#જો ENABLE_BACKTRACES
સ્થિર તાર્કિક RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  પાછા ખોટું;
}
#endif

શૂન્ય llvm::Enable_Pretty_ઢગલો_Trace() {
#જો ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  સ્થિર તાર્કિક HandlerRegistered = RegisterCrashPrinter();
  (શૂન્ય)HandlerRegistered;
#endif
}

શૂન્ય llvm::Enable_Pretty_ઢગલો_Trace_On_Sig_Info_For_This_Thread(તાર્કિક ShouldEnable) {
#જો ENABLE_BACKTRACES
  જો (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    પાછા;
  }

  // The first time this is called, we register the SIGINFO handler.
  સ્થિર તાર્કિક HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    પાછા ખોટું;
  }();
  (શૂન્ય)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

અચલ શૂન્ય *llvm::Save_Pretty_ઢગલો_State() {
#જો ENABLE_BACKTRACES
  પાછા Pretty_ઢગલો_Trace_Head;
#નહીંતર
  પાછા nullptr;
#endif
}

શૂન્ય llvm::Restore_Pretty_ઢગલો_State(અચલ શૂન્ય *ટોચ) {
#જો ENABLE_BACKTRACES
  Pretty_ઢગલો_Trace_Head =
      static_cast<Pretty_ઢગલો_Trace_Entry *>(const_cast<શૂન્ય *>(ટોચ));
#endif
}

શૂન્ય LLVMEnable_Pretty_ઢગલો_Trace() {
  Enable_Pretty_ઢગલો_Trace();
}
