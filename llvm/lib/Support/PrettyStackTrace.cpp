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

#অন্তর্ভাব "llvm/Support/PrettyStackTrace.h"
#অন্তর্ভাব "llvm-c/ErrorHandling.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/SaveAndRestore.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/Watchdog.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#endif

#অন্তর্ভাব <atomic>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdarg>
#অন্তর্ভাব <cstdio>
#অন্তর্ভাব <cstring>
#অন্তর্ভাব <tuple>

#যদি __has_include(<CrashReporterClient.h>)
#অন্তর্ভাব <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using নামাকাশ llvm;

স্থির ধ্রুব বর্ণ *Bug_Report_বার্তা =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#যদি ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
স্থির LLVM_THREAD_LOCAL Pretty_স্তূপ_Trace_Entry *Pretty_স্তূপ_Trace_Head = nullptr;

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
স্থির চঞ্চল std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
স্থির LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

নামাকাশ llvm {
Pretty_স্তূপ_Trace_Entry *Reverse_স্তূপ_Trace(Pretty_স্তূপ_Trace_Entry *Head) {
  Pretty_স্তূপ_Trace_Entry *Prev = nullptr;
  যাবত্ (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  প্রত্যাগম Prev;
}
} // namespace llvm

স্থির শূন্য Print_স্তূপ(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_স্তূপ_Trace_Entry *> Saved_স্তূপ{Pretty_স্তূপ_Trace_Head,
                                                     nullptr};
  Pretty_স্তূপ_Trace_Entry *Reversed_স্তূপ = Reverse_স্তূপ_Trace(Saved_স্তূপ.get());
  প্রতি (ধ্রুব Pretty_স্তূপ_Trace_Entry *Entry = Reversed_স্তূপ; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_স্তূপ_Trace(Reversed_স্তূপ);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
স্থির শূন্য Print_Cur_স্তূপ_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  যদি (!Pretty_স্তূপ_Trace_Head) প্রত্যাগম;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_স্তূপ(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#যদি defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
বাহ্য "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#অন্যথা
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
সংরচনা crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#যদি CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#নোচেদ্যদি defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
বাহ্য "C" ধ্রুব বর্ণ *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] স্থির শূন্য setCrashLogMessage(ধ্রুব বর্ণ *বার্তা);
স্থির শূন্য setCrashLogMessage(ধ্রুব বর্ণ *বার্তা) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (শূন্য)CRSetCrashLogMessage(বার্তা);
#নোচেদ্যদি HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = বার্তা;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[পরিমাণ(CrashHandlerString)];
alignas(CrashHandlerString) স্থির CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
স্থির শূন্য CrashHandler(শূন্য *) {
  errs() << Bug_Report_বার্তা ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_স্তূপ_Trace(errs());
#অন্যথা
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
      *নূতন (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_স্তূপ_Trace(Stream);
  }

  যদি (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } অন্যথা {
    setCrashLogMessage("No crash information.");
  }
#endif
}

স্থির শূন্য printForSigInfoIfNeeded() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  যদি (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    প্রত্যাগম;
  }

  Print_Cur_স্তূপ_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

শূন্য llvm::set_Bug_Report_বার্তা(ধ্রুব বর্ণ *বার্তা_2) {
  Bug_Report_বার্তা = বার্তা_2;
}

ধ্রুব বর্ণ *llvm::get_Bug_Report_বার্তা() {
  প্রত্যাগম Bug_Report_বার্তা;
}

Pretty_স্তূপ_Trace_Entry::Pretty_স্তূপ_Trace_Entry() {
#যদি ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  NextEntry = Pretty_স্তূপ_Trace_Head;
  Pretty_স্তূপ_Trace_Head = এতত্;
#endif
}

Pretty_স্তূপ_Trace_Entry::~Pretty_স্তূপ_Trace_Entry() {
#যদি ENABLE_BACKTRACES
  প্রতিজ্ঞা(Pretty_স্তূপ_Trace_Head == এতত্ &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_স্তূপ_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

শূন্য Pretty_স্তূপ_Trace_String::print(raw_ostream &OS) ধ্রুব { OS << Str << "\n"; }

Pretty_স্তূপ_Trace_Format::Pretty_স্তূপ_Trace_Format(ধ্রুব বর্ণ *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ধ্রুব পূর্ণাঙ্ক SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  যদি (SizeOrError < 0) {
    প্রত্যাগম;
  }

  ধ্রুব পূর্ণাঙ্ক Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

শূন্য Pretty_স্তূপ_Trace_Format::print(raw_ostream &OS) ধ্রুব { OS << Str << "\n"; }

শূন্য Pretty_স্তূপ_Trace_Program::print(raw_ostream &OS) ধ্রুব {
  OS << "Program arguments: ";
  // Print the argument list.
  প্রতি (পূর্ণাঙ্ক I = 0; I < ArgC; ++I) {
    ধ্রুব তার্কিক HaveSpace = ::strchr(ArgV[I], ' ');
    যদি (I)
      OS << ' ';
    যদি (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    যদি (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#যদি ENABLE_BACKTRACES
স্থির তার্কিক RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  প্রত্যাগম অসত্য;
}
#endif

শূন্য llvm::Enable_Pretty_স্তূপ_Trace() {
#যদি ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  স্থির তার্কিক HandlerRegistered = RegisterCrashPrinter();
  (শূন্য)HandlerRegistered;
#endif
}

শূন্য llvm::Enable_Pretty_স্তূপ_Trace_On_Sig_Info_For_This_Thread(তার্কিক ShouldEnable) {
#যদি ENABLE_BACKTRACES
  যদি (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    প্রত্যাগম;
  }

  // The first time this is called, we register the SIGINFO handler.
  স্থির তার্কিক HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    প্রত্যাগম অসত্য;
  }();
  (শূন্য)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ধ্রুব শূন্য *llvm::Save_Pretty_স্তূপ_State() {
#যদি ENABLE_BACKTRACES
  প্রত্যাগম Pretty_স্তূপ_Trace_Head;
#অন্যথা
  প্রত্যাগম nullptr;
#endif
}

শূন্য llvm::Restore_Pretty_স্তূপ_State(ধ্রুব শূন্য *শীর্ষ) {
#যদি ENABLE_BACKTRACES
  Pretty_স্তূপ_Trace_Head =
      static_cast<Pretty_স্তূপ_Trace_Entry *>(const_cast<শূন্য *>(শীর্ষ));
#endif
}

শূন্য LLVMEnable_Pretty_স্তূপ_Trace() {
  Enable_Pretty_স্তূপ_Trace();
}
