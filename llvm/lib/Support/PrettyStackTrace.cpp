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

#अन्तर्भाव "llvm/Support/PrettyStackTrace.h"
#अन्तर्भाव "llvm-c/ErrorHandling.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/SaveAndRestore.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/Watchdog.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#अन्तर्भाव "llvm/ADT/SmallString.h"
#endif

#अन्तर्भाव <atomic>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdarg>
#अन्तर्भाव <cstdio>
#अन्तर्भाव <cstring>
#अन्तर्भाव <tuple>

#जइ __has_include(<CrashReporterClient.h>)
#अन्तर्भाव <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using नामाकाश llvm;

स्थिर ध्रुव वर्ण *Bug_Report_सन्देशः =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#जइ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
स्थिर LLVM_THREAD_LOCAL Pretty_राशिः_Trace_Entry *Pretty_राशिः_Trace_Head = nullptr;

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
स्थिर चञ्चल std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
स्थिर LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

नामाकाश llvm {
Pretty_राशिः_Trace_Entry *Reverse_राशिः_Trace(Pretty_राशिः_Trace_Entry *Head) {
  Pretty_राशिः_Trace_Entry *Prev = nullptr;
  जाव (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  प्रत्यागम Prev;
}
} // namespace llvm

स्थिर शून्य Print_राशिः(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_राशिः_Trace_Entry *> Saved_राशिः{Pretty_राशिः_Trace_Head,
                                                     nullptr};
  Pretty_राशिः_Trace_Entry *Reversed_राशिः = Reverse_राशिः_Trace(Saved_राशिः.get());
  प्रति (ध्रुव Pretty_राशिः_Trace_Entry *Entry = Reversed_राशिः; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_राशिः_Trace(Reversed_राशिः);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
स्थिर शून्य Print_Cur_राशिः_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  जइ (!Pretty_राशिः_Trace_Head) प्रत्यागम;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_राशिः(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#जइ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
बाह्य "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#अण्णहा
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
संरचना crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#जइ CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#नोचेद्यदि defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
बाह्य "C" ध्रुव वर्ण *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] स्थिर शून्य setCrashLogMessage(ध्रुव वर्ण *सन्देशः);
स्थिर शून्य setCrashLogMessage(ध्रुव वर्ण *सन्देशः) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (शून्य)CRSetCrashLogMessage(सन्देशः);
#नोचेद्यदि HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = सन्देशः;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[परिमाण(CrashHandlerString)];
alignas(CrashHandlerString) स्थिर CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
स्थिर शून्य CrashHandler(शून्य *) {
  errs() << Bug_Report_सन्देशः ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_राशिः_Trace(errs());
#अण्णहा
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
      *नूतन (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_राशिः_Trace(Stream);
  }

  जइ (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } अण्णहा {
    setCrashLogMessage("No crash information.");
  }
#endif
}

स्थिर शून्य printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  जइ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    प्रत्यागम;
  }

  Print_Cur_राशिः_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

शून्य llvm::set_Bug_Report_सन्देशः(ध्रुव वर्ण *सन्देशः_2) {
  Bug_Report_सन्देशः = सन्देशः_2;
}

ध्रुव वर्ण *llvm::get_Bug_Report_सन्देशः() {
  प्रत्यागम Bug_Report_सन्देशः;
}

Pretty_राशिः_Trace_Entry::Pretty_राशिः_Trace_Entry() {
#जइ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_राशिः_Trace_Head;
  Pretty_राशिः_Trace_Head = एतत्;
#endif
}

Pretty_राशिः_Trace_Entry::~Pretty_राशिः_Trace_Entry() {
#जइ ENABLE_BACKTRACES
  प्रतिज्ञा(Pretty_राशिः_Trace_Head == एतत् &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_राशिः_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

शून्य Pretty_राशिः_Trace_String::print(raw_ostream &OS) ध्रुव { OS << Str << "\n"; }

Pretty_राशिः_Trace_Format::Pretty_राशिः_Trace_Format(ध्रुव वर्ण *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ध्रुव पूर्णाङ्क SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  जइ (SizeOrError < 0) {
    प्रत्यागम;
  }

  ध्रुव पूर्णाङ्क Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

शून्य Pretty_राशिः_Trace_Format::print(raw_ostream &OS) ध्रुव { OS << Str << "\n"; }

शून्य Pretty_राशिः_Trace_Program::print(raw_ostream &OS) ध्रुव {
  OS << "Program arguments: ";
  // Print the argument list.
  प्रति (पूर्णाङ्क I = 0; I < ArgC; ++I) {
    ध्रुव तार्किक HaveSpace = ::strchr(ArgV[I], ' ');
    जइ (I)
      OS << ' ';
    जइ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    जइ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#जइ ENABLE_BACKTRACES
स्थिर तार्किक RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  प्रत्यागम मिच्छ;
}
#endif

शून्य llvm::Enable_Pretty_राशिः_Trace() {
#जइ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  स्थिर तार्किक HandlerRegistered = RegisterCrashPrinter();
  (शून्य)HandlerRegistered;
#endif
}

शून्य llvm::Enable_Pretty_राशिः_Trace_On_Sig_Info_For_This_Thread(तार्किक ShouldEnable) {
#जइ ENABLE_BACKTRACES
  जइ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    प्रत्यागम;
  }

  // The first time this is called, we register the SIGINFO handler.
  स्थिर तार्किक HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    प्रत्यागम मिच्छ;
  }();
  (शून्य)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ध्रुव शून्य *llvm::Save_Pretty_राशिः_State() {
#जइ ENABLE_BACKTRACES
  प्रत्यागम Pretty_राशिः_Trace_Head;
#अण्णहा
  प्रत्यागम nullptr;
#endif
}

शून्य llvm::Restore_Pretty_राशिः_State(ध्रुव शून्य *शिखरम्) {
#जइ ENABLE_BACKTRACES
  Pretty_राशिः_Trace_Head =
      static_cast<Pretty_राशिः_Trace_Entry *>(const_cast<शून्य *>(शिखरम्));
#endif
}

शून्य LLVMEnable_Pretty_राशिः_Trace() {
  Enable_Pretty_राशिः_Trace();
}
