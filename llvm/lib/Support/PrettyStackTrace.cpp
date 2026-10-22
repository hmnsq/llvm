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

#అన్తర్భావ "llvm/Support/PrettyStackTrace.h"
#అన్తర్భావ "llvm-c/ErrorHandling.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/SaveAndRestore.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/Watchdog.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

#నిర్వచితమైతే __APPLE__
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అయితే_ముగింపు

#అన్తర్భావ <atomic>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdarg>
#అన్తర్భావ <cstdio>
#అన్తర్భావ <cstring>
#అన్తర్భావ <tuple>

#ఒకవేళ __has_include(<CrashReporterClient.h>)
#అన్తర్భావ <CrashReporterClient.h>
#నిర్వచించు LLVM_HAVE_CRASHREPORTERCLIENT_H
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

స్థిర స్థిరరాశి అక్షరం *Bug_Report_సందేశం =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#ఒకవేళ ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
స్థిర LLVM_THREAD_LOCAL Pretty_దొంతర_Trace_నమోదు *Pretty_దొంతర_Trace_తల = శూన్యసూచిక;

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
స్థిర అస్థిర std::atomic<గుర్తులేని> GlobalSigInfoGenerationCounter = 1;
స్థిర LLVM_THREAD_LOCAL గుర్తులేని ThreadLocalSigInfoGenerationCounter = 0;

నామాకాశ llvm {
Pretty_దొంతర_Trace_నమోదు *తిరగేయి_దొంతర_Trace(Pretty_దొంతర_Trace_నమోదు *తల_2) {
  Pretty_దొంతర_Trace_నమోదు *Prev = శూన్యసూచిక;
  అయితే (తల_2)
    std::tie(Prev, తల_2, తల_2->తదుపరి_నమోదు) =
        std::make_tuple(తల_2, తల_2->తదుపరి_నమోదు, Prev);
  తిరిగి Prev;
}
} // namespace llvm

స్థిర శూన్య Print_దొంతర(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  గుర్తులేని ID = 0;
  SaveAndRestore<Pretty_దొంతర_Trace_నమోదు *> Saved_దొంతర{Pretty_దొంతర_Trace_తల,
                                                     శూన్యసూచిక};
  Pretty_దొంతర_Trace_నమోదు *Reversed_దొంతర = తిరగేయి_దొంతర_Trace(Saved_దొంతర.get());
  కొరకు (స్థిరరాశి Pretty_దొంతర_Trace_నమోదు *నమోదు_2 = Reversed_దొంతర; నమోదు_2;
       నమోదు_2 = నమోదు_2->get_తదుపరి_నమోదు()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    నమోదు_2->print(OS);
  }
  llvm::తిరగేయి_దొంతర_Trace(Reversed_దొంతర);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
స్థిర శూన్య Print_Cur_దొంతర_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  ఒకవేళ (!Pretty_దొంతర_Trace_తల) తిరిగి;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_దొంతర(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#ఒకవేళ defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
బాహ్య "C" {
#నిర్వచితమైతే CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#లేకపోతే
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
నిర్మాణం crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_సంస్కరణ,
        0,
        0,
        0,
        0,
        0,
        0,
#ఒకవేళ CRASHREPORTER_ANNOTATIONS_సంస్కరణ > 4
        0
#అయితే_ముగింపు // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#అయితే_ముగింపు // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#లేదావేళ defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
బాహ్య "C" స్థిరరాశి అక్షరం *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#అయితే_ముగింపు

[[maybe_వాడని]] స్థిర శూన్య సమితి_Crash_Log_సందేశం(స్థిరరాశి అక్షరం *సందేశం);
స్థిర శూన్య సమితి_Crash_Log_సందేశం(స్థిరరాశి అక్షరం *సందేశం) {
#నిర్వచితమైతే LLVM_HAVE_CRASHREPORTERCLIENT_H
  (శూన్య)CRSet_Crash_Log_సందేశం(సందేశం);
#లేదావేళ HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = సందేశం;
#అయితే_ముగింపు
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#నిర్వచితమైతే __APPLE__
ఉపయోగించి CrashHandlerString = SmallString<2048>;
ఉపయోగించి CrashHandlerStringStorage = std::byte[పరిమాణం(CrashHandlerString)];
alignas(CrashHandlerString) స్థిర CrashHandlerStringStorage
    crashHandlerStringStorage;
#అయితే_ముగింపు

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
స్థిర శూన్య CrashHandler(శూన్య *) {
  errs() << Bug_Report_సందేశం ;

#నిర్వచితం_కాకపోతే __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_దొంతర_Trace(errs());
#లేకపోతే
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
  స్వయంచాలక &crashHandlerString =
      *కొత్త (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  సమితి_Crash_Log_సందేశం(crashHandlerString.c_str());

  {
    raw_svector_ostream ప్రవాహం(crashHandlerString);
    Print_Cur_దొంతర_Trace(ప్రవాహం);
  }

  ఒకవేళ (!crashHandlerString.ఖాళీలు()) {
    సమితి_Crash_Log_సందేశం(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } లేకపోతే {
    సమితి_Crash_Log_సందేశం("No crash information.");
  }
#అయితే_ముగింపు
}

స్థిర శూన్య printForSigInfoIfNeeded() {
  గుర్తులేని CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.ఎక్కించు(std::memory_order_relaxed);
  ఒకవేళ (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    తిరిగి;
  }

  Print_Cur_దొంతర_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#అయితే_ముగింపు // ENABLE_BACKTRACES

శూన్య llvm::సమితి_Bug_Report_సందేశం(స్థిరరాశి అక్షరం *సందేశం_4) {
  Bug_Report_సందేశం = సందేశం_4;
}

స్థిరరాశి అక్షరం *llvm::get_Bug_Report_సందేశం() {
  తిరిగి Bug_Report_సందేశం;
}

Pretty_దొంతర_Trace_నమోదు::Pretty_దొంతర_Trace_నమోదు() {
#ఒకవేళ ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  తదుపరి_నమోదు = Pretty_దొంతర_Trace_తల;
  Pretty_దొంతర_Trace_తల = ఇది;
#అయితే_ముగింపు
}

Pretty_దొంతర_Trace_నమోదు::~Pretty_దొంతర_Trace_నమోదు() {
#ఒకవేళ ENABLE_BACKTRACES
  ప్రతిజ్ఞా(Pretty_దొంతర_Trace_తల == ఇది &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_దొంతర_Trace_తల = తదుపరి_నమోదు;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#అయితే_ముగింపు
}

శూన్య Pretty_దొంతర_Trace_String::print(raw_ostream &OS) స్థిరరాశి { OS << Str << "\n"; }

Pretty_దొంతర_Trace_Format::Pretty_దొంతర_Trace_Format(స్థిరరాశి అక్షరం *Format, ...) {
  va_list AP;
  va_ప్రారంభం(AP, Format);
  స్థిరరాశి పూర్ణాంకం కొలత_Or_Error = vsnprintf(శూన్యసూచిక, 0, Format, AP);
  va_చివర(AP);
  ఒకవేళ (కొలత_Or_Error < 0) {
    తిరిగి;
  }

  స్థిరరాశి పూర్ణాంకం కొలత = కొలత_Or_Error + 1; // '\0'
  Str.resize(కొలత);
  va_ప్రారంభం(AP, Format);
  vsnprintf(Str.దత్తాంశం(), కొలత, Format, AP);
  va_చివర(AP);
}

శూన్య Pretty_దొంతర_Trace_Format::print(raw_ostream &OS) స్థిరరాశి { OS << Str << "\n"; }

శూన్య Pretty_దొంతర_Trace_కార్యక్రమం::print(raw_ostream &OS) స్థిరరాశి {
  OS << "Program arguments: ";
  // Print the argument list.
  కొరకు (పూర్ణాంకం I = 0; I < ArgC; ++I) {
    స్థిరరాశి తార్కిక HaveSpace = ::strchr(ArgV[I], ' ');
    ఒకవేళ (I)
      OS << ' ';
    ఒకవేళ (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    ఒకవేళ (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#ఒకవేళ ENABLE_BACKTRACES
స్థిర తార్కిక రిజిస్టరు_Crash_Printer() {
  sys::కూడు_Signal_Handler(CrashHandler, శూన్యసూచిక);
  తిరిగి అబద్ధం;
}
#అయితే_ముగింపు

శూన్య llvm::Enable_Pretty_దొంతర_Trace() {
#ఒకవేళ ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  స్థిర తార్కిక HandlerRegistered = రిజిస్టరు_Crash_Printer();
  (శూన్య)HandlerRegistered;
#అయితే_ముగింపు
}

శూన్య llvm::Enable_Pretty_దొంతర_Trace_On_Sig_Info_For_This_Thread(తార్కిక ShouldEnable) {
#ఒకవేళ ENABLE_BACKTRACES
  ఒకవేళ (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    తిరిగి;
  }

  // The first time this is called, we register the SIGINFO handler.
  స్థిర తార్కిక HandlerRegistered = []{
    sys::సమితి_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_కూడు(1, std::memory_order_relaxed);
    });
    తిరిగి అబద్ధం;
  }();
  (శూన్య)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.ఎక్కించు(std::memory_order_relaxed);
#అయితే_ముగింపు
}

స్థిరరాశి శూన్య *llvm::Save_Pretty_దొంతర_State() {
#ఒకవేళ ENABLE_BACKTRACES
  తిరిగి Pretty_దొంతర_Trace_తల;
#లేకపోతే
  తిరిగి శూన్యసూచిక;
#అయితే_ముగింపు
}

శూన్య llvm::Restore_Pretty_దొంతర_State(స్థిరరాశి శూన్య *శిఖరం) {
#ఒకవేళ ENABLE_BACKTRACES
  Pretty_దొంతర_Trace_తల =
      static_cast<Pretty_దొంతర_Trace_నమోదు *>(const_cast<శూన్య *>(శిఖరం));
#అయితే_ముగింపు
}

శూన్య LLVMEnable_Pretty_దొంతర_Trace() {
  Enable_Pretty_దొంతర_Trace();
}
