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

স্থির ধ্রুবক অক্ষর *Bug_Report_বার্তা =
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
স্থির LLVM_THREAD_LOCAL Pretty_স্তূপ_Trace_Entry *Pretty_স্তূপ_Trace_শীর্ষ = nullptr;

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
স্থির অস্থির std::atomic<unsigned> Global_Sig_Info_Generation_গণক = 1;
স্থির LLVM_THREAD_LOCAL unsigned Thread_Local_Sig_Info_Generation_গণক = 0;

নামাকাশ llvm {
Pretty_স্তূপ_Trace_Entry *Reverse_স্তূপ_Trace(Pretty_স্তূপ_Trace_Entry *শীর্ষ_2) {
  Pretty_স্তূপ_Trace_Entry *পূর্ব_2 = nullptr;
  যখন (শীর্ষ_2)
    std::tie(পূর্ব_2, শীর্ষ_2, শীর্ষ_2->পরবর্তী_Entry) =
        std::make_tuple(শীর্ষ_2, শীর্ষ_2->পরবর্তী_Entry, পূর্ব_2);
  ফেরত পূর্ব_2;
}
} // namespace llvm

স্থির শূন্য Print_স্তূপ(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_স্তূপ_Trace_Entry *> Saved_স্তূপ{Pretty_স্তূপ_Trace_শীর্ষ,
                                                     nullptr};
  Pretty_স্তূপ_Trace_Entry *Reversed_স্তূপ = Reverse_স্তূপ_Trace(Saved_স্তূপ.get());
  জন্য (ধ্রুবক Pretty_স্তূপ_Trace_Entry *Entry = Reversed_স্তূপ; Entry;
       Entry = Entry->get_পরবর্তী_Entry()) {
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
  যদি (!Pretty_স্তূপ_Trace_শীর্ষ) ফেরত;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_স্তূপ(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#যদি defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
বাহ্যিক "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#অন্যথা
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
কাঠামো crashreporter_annotations_t gCRAnnotations
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
#নতুবা_যদি defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
বাহ্যিক "C" ধ্রুবক অক্ষর *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] স্থির শূন্য setCrashLogMessage(ধ্রুবক অক্ষর *বার্তা);
স্থির শূন্য setCrashLogMessage(ধ্রুবক অক্ষর *বার্তা) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (শূন্য)CRSetCrashLogMessage(বার্তা);
#নতুবা_যদি HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = বার্তা;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[আকার(CrashHandlerString)];
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
      *নতুন (&crashHandlerStringStorage) CrashHandlerString;

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
      Global_Sig_Info_Generation_গণক.load(std::memory_order_relaxed);
  যদি (Thread_Local_Sig_Info_Generation_গণক == 0 ||
      Thread_Local_Sig_Info_Generation_গণক == CurrentSigInfoGeneration) {
    ফেরত;
  }

  Print_Cur_স্তূপ_Trace(errs());
  Thread_Local_Sig_Info_Generation_গণক = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

শূন্য llvm::set_Bug_Report_বার্তা(ধ্রুবক অক্ষর *বার্তা_2) {
  Bug_Report_বার্তা = বার্তা_2;
}

ধ্রুবক অক্ষর *llvm::get_Bug_Report_বার্তা() {
  ফেরত Bug_Report_বার্তা;
}

Pretty_স্তূপ_Trace_Entry::Pretty_স্তূপ_Trace_Entry() {
#যদি ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  পরবর্তী_Entry = Pretty_স্তূপ_Trace_শীর্ষ;
  Pretty_স্তূপ_Trace_শীর্ষ = এই;
#endif
}

Pretty_স্তূপ_Trace_Entry::~Pretty_স্তূপ_Trace_Entry() {
#যদি ENABLE_BACKTRACES
  প্রতিজ্ঞা(Pretty_স্তূপ_Trace_শীর্ষ == এই &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_স্তূপ_Trace_শীর্ষ = পরবর্তী_Entry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#endif
}

শূন্য Pretty_স্তূপ_Trace_String::print(raw_ostream &OS) ধ্রুবক { OS << Str << "\n"; }

Pretty_স্তূপ_Trace_Format::Pretty_স্তূপ_Trace_Format(ধ্রুবক অক্ষর *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ধ্রুবক পূর্ণসংখ্যা আকার_Or_Error = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  যদি (আকার_Or_Error < 0) {
    ফেরত;
  }

  ধ্রুবক পূর্ণসংখ্যা আকার_2 = আকার_Or_Error + 1; // '\0'
  Str.resize(আকার_2);
  va_start(AP, Format);
  vsnprintf(Str.তথ্য(), আকার_2, Format, AP);
  va_end(AP);
}

শূন্য Pretty_স্তূপ_Trace_Format::print(raw_ostream &OS) ধ্রুবক { OS << Str << "\n"; }

শূন্য Pretty_স্তূপ_Trace_Program::print(raw_ostream &OS) ধ্রুবক {
  OS << "Program arguments: ";
  // Print the argument list.
  জন্য (পূর্ণসংখ্যা I = 0; I < ArgC; ++I) {
    ধ্রুবক যৌক্তিক HaveSpace = ::strchr(ArgV[I], ' ');
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
স্থির যৌক্তিক RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  ফেরত মিথ্যা;
}
#endif

শূন্য llvm::Enable_Pretty_স্তূপ_Trace() {
#যদি ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  স্থির যৌক্তিক HandlerRegistered = RegisterCrashPrinter();
  (শূন্য)HandlerRegistered;
#endif
}

শূন্য llvm::Enable_Pretty_স্তূপ_Trace_On_Sig_Info_For_This_Thread(যৌক্তিক ShouldEnable) {
#যদি ENABLE_BACKTRACES
  যদি (!ShouldEnable) {
    Thread_Local_Sig_Info_Generation_গণক = 0;
    ফেরত;
  }

  // The first time this is called, we register the SIGINFO handler.
  স্থির যৌক্তিক HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      Global_Sig_Info_Generation_গণক.fetch_add(1, std::memory_order_relaxed);
    });
    ফেরত মিথ্যা;
  }();
  (শূন্য)HandlerRegistered;

  // Next, enable it for the current thread.
  Thread_Local_Sig_Info_Generation_গণক =
      Global_Sig_Info_Generation_গণক.load(std::memory_order_relaxed);
#endif
}

ধ্রুবক শূন্য *llvm::Save_Pretty_স্তূপ_State() {
#যদি ENABLE_BACKTRACES
  ফেরত Pretty_স্তূপ_Trace_শীর্ষ;
#অন্যথা
  ফেরত nullptr;
#endif
}

শূন্য llvm::Restore_Pretty_স্তূপ_State(ধ্রুবক শূন্য *শীর্ষ_3) {
#যদি ENABLE_BACKTRACES
  Pretty_স্তূপ_Trace_শীর্ষ =
      static_cast<Pretty_স্তূপ_Trace_Entry *>(const_cast<শূন্য *>(শীর্ষ_3));
#endif
}

শূন্য LLVMEnable_Pretty_স্তূপ_Trace() {
  Enable_Pretty_স্তূপ_Trace();
}
