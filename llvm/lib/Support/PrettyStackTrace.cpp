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

#यदि_परिभाषितम् __APPLE__
#अन्तर्भाव "llvm/ADT/SmallString.h"
#यदि_अन्तः

#अन्तर्भाव <atomic>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdarg>
#अन्तर्भाव <cstdio>
#अन्तर्भाव <cstring>
#अन्तर्भाव <tuple>

#यदि __has_include(<CrashReporterClient.h>)
#अन्तर्भाव <CrashReporterClient.h>
#परिभाषय LLVM_HAVE_CRASHREPORTERCLIENT_H
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

स्थिर ध्रुव वर्ण *Bug_Report_सन्देशः =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#यदि ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
स्थिर LLVM_THREAD_LOCAL Pretty_राशिः_Trace_प्रविष्टिः *Pretty_राशिः_Trace_शीर्ष = शून्यसूचकः;

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
स्थिर चञ्चल std::atomic<अचिह्नित> Global_Sig_Info_Generation_गणक = 1;
स्थिर LLVM_THREAD_LOCAL अचिह्नित Thread_Local_Sig_Info_Generation_गणक = 0;

नामाकाश llvm {
Pretty_राशिः_Trace_प्रविष्टिः *Reverse_राशिः_Trace(Pretty_राशिः_Trace_प्रविष्टिः *शीर्ष_2) {
  Pretty_राशिः_Trace_प्रविष्टिः *पूर्व_2 = शून्यसूचकः;
  यावत् (शीर्ष_2)
    std::tie(पूर्व_2, शीर्ष_2, शीर्ष_2->अग्र_प्रविष्टिः) =
        std::make_tuple(शीर्ष_2, शीर्ष_2->अग्र_प्रविष्टिः, पूर्व_2);
  प्रत्यागम पूर्व_2;
}
} // namespace llvm

स्थिर शून्य Print_राशिः(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  अचिह्नित ID = 0;
  SaveAndRestore<Pretty_राशिः_Trace_प्रविष्टिः *> Saved_राशिः{Pretty_राशिः_Trace_शीर्ष,
                                                     शून्यसूचकः};
  Pretty_राशिः_Trace_प्रविष्टिः *Reversed_राशिः = Reverse_राशिः_Trace(Saved_राशिः.get());
  प्रति (ध्रुव Pretty_राशिः_Trace_प्रविष्टिः *प्रविष्टिः = Reversed_राशिः; प्रविष्टिः;
       प्रविष्टिः = प्रविष्टिः->get_अग्र_प्रविष्टिः()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    प्रविष्टिः->print(OS);
  }
  llvm::Reverse_राशिः_Trace(Reversed_राशिः);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
स्थिर शून्य Print_Cur_राशिः_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  यदि (!Pretty_राशिः_Trace_शीर्ष) प्रत्यागम;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_राशिः(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#यदि defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
बाह्य "C" {
#यदि_परिभाषितम् CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#अन्यथा
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
संरचना crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_संस्करणम्,
        0,
        0,
        0,
        0,
        0,
        0,
#यदि CRASHREPORTER_ANNOTATIONS_संस्करणम् > 4
        0
#यदि_अन्तः // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#यदि_अन्तः // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#नोचेद्यदि defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
बाह्य "C" ध्रुव वर्ण *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#यदि_अन्तः

[[maybe_अप्रयुक्तम्]] स्थिर शून्य समुच्चयः_Crash_Log_सन्देशः(ध्रुव वर्ण *सन्देशः);
स्थिर शून्य समुच्चयः_Crash_Log_सन्देशः(ध्रुव वर्ण *सन्देशः) {
#यदि_परिभाषितम् LLVM_HAVE_CRASHREPORTERCLIENT_H
  (शून्य)CRSet_Crash_Log_सन्देशः(सन्देशः);
#नोचेद्यदि HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = सन्देशः;
#यदि_अन्तः
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#यदि_परिभाषितम् __APPLE__
उपयुज्य CrashHandlerString = SmallString<2048>;
उपयुज्य CrashHandlerStringStorage = std::byte[परिमाण(CrashHandlerString)];
alignas(CrashHandlerString) स्थिर CrashHandlerStringStorage
    crashHandlerStringStorage;
#यदि_अन्तः

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
स्थिर शून्य CrashHandler(शून्य *) {
  errs() << Bug_Report_सन्देशः ;

#यदि_अपरिभाषितम् __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_राशिः_Trace(errs());
#अन्यथा
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
  स्वचालित &crashHandlerString =
      *नूतन (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  समुच्चयः_Crash_Log_सन्देशः(crashHandlerString.c_str());

  {
    raw_svector_ostream प्रवाहः(crashHandlerString);
    Print_Cur_राशिः_Trace(प्रवाहः);
  }

  यदि (!crashHandlerString.रिक्तानि()) {
    समुच्चयः_Crash_Log_सन्देशः(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } अन्यथा {
    समुच्चयः_Crash_Log_सन्देशः("No crash information.");
  }
#यदि_अन्तः
}

स्थिर शून्य printForSigInfoIfNeeded() {
  अचिह्नित CurrentSigInfoGeneration =
      Global_Sig_Info_Generation_गणक.आरोपय(std::memory_order_relaxed);
  यदि (Thread_Local_Sig_Info_Generation_गणक == 0 ||
      Thread_Local_Sig_Info_Generation_गणक == CurrentSigInfoGeneration) {
    प्रत्यागम;
  }

  Print_Cur_राशिः_Trace(errs());
  Thread_Local_Sig_Info_Generation_गणक = CurrentSigInfoGeneration;
}

#यदि_अन्तः // ENABLE_BACKTRACES

शून्य llvm::समुच्चयः_Bug_Report_सन्देशः(ध्रुव वर्ण *सन्देशः_4) {
  Bug_Report_सन्देशः = सन्देशः_4;
}

ध्रुव वर्ण *llvm::get_Bug_Report_सन्देशः() {
  प्रत्यागम Bug_Report_सन्देशः;
}

Pretty_राशिः_Trace_प्रविष्टिः::Pretty_राशिः_Trace_प्रविष्टिः() {
#यदि ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  अग्र_प्रविष्टिः = Pretty_राशिः_Trace_शीर्ष;
  Pretty_राशिः_Trace_शीर्ष = एतत्;
#यदि_अन्तः
}

Pretty_राशिः_Trace_प्रविष्टिः::~Pretty_राशिः_Trace_प्रविष्टिः() {
#यदि ENABLE_BACKTRACES
  प्रतिज्ञा(Pretty_राशिः_Trace_शीर्ष == एतत् &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_राशिः_Trace_शीर्ष = अग्र_प्रविष्टिः;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#यदि_अन्तः
}

शून्य Pretty_राशिः_Trace_String::print(raw_ostream &OS) ध्रुव { OS << Str << "\n"; }

Pretty_राशिः_Trace_Format::Pretty_राशिः_Trace_Format(ध्रुव वर्ण *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ध्रुव पूर्णाङ्क आकार_Or_Error = vsnprintf(शून्यसूचकः, 0, Format, AP);
  va_अन्तः(AP);
  यदि (आकार_Or_Error < 0) {
    प्रत्यागम;
  }

  ध्रुव पूर्णाङ्क आकार = आकार_Or_Error + 1; // '\0'
  Str.resize(आकार);
  va_start(AP, Format);
  vsnprintf(Str.दत्त(), आकार, Format, AP);
  va_अन्तः(AP);
}

शून्य Pretty_राशिः_Trace_Format::print(raw_ostream &OS) ध्रुव { OS << Str << "\n"; }

शून्य Pretty_राशिः_Trace_क्रमादेशः::print(raw_ostream &OS) ध्रुव {
  OS << "Program arguments: ";
  // Print the argument list.
  प्रति (पूर्णाङ्क I = 0; I < ArgC; ++I) {
    ध्रुव तार्किक HaveSpace = ::strchr(ArgV[I], ' ');
    यदि (I)
      OS << ' ';
    यदि (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    यदि (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#यदि ENABLE_BACKTRACES
स्थिर तार्किक पञ्जिकाकोशः_Crash_Printer() {
  sys::सङ्कलय_Signal_Handler(CrashHandler, शून्यसूचकः);
  प्रत्यागम असत्य;
}
#यदि_अन्तः

शून्य llvm::Enable_Pretty_राशिः_Trace() {
#यदि ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  स्थिर तार्किक HandlerRegistered = पञ्जिकाकोशः_Crash_Printer();
  (शून्य)HandlerRegistered;
#यदि_अन्तः
}

शून्य llvm::Enable_Pretty_राशिः_Trace_On_Sig_Info_For_This_Thread(तार्किक ShouldEnable) {
#यदि ENABLE_BACKTRACES
  यदि (!ShouldEnable) {
    Thread_Local_Sig_Info_Generation_गणक = 0;
    प्रत्यागम;
  }

  // The first time this is called, we register the SIGINFO handler.
  स्थिर तार्किक HandlerRegistered = []{
    sys::समुच्चयः_Info_Signal_Function([]{
      Global_Sig_Info_Generation_गणक.fetch_सङ्कलय(1, std::memory_order_relaxed);
    });
    प्रत्यागम असत्य;
  }();
  (शून्य)HandlerRegistered;

  // Next, enable it for the current thread.
  Thread_Local_Sig_Info_Generation_गणक =
      Global_Sig_Info_Generation_गणक.आरोपय(std::memory_order_relaxed);
#यदि_अन्तः
}

ध्रुव शून्य *llvm::Save_Pretty_राशिः_State() {
#यदि ENABLE_BACKTRACES
  प्रत्यागम Pretty_राशिः_Trace_शीर्ष;
#अन्यथा
  प्रत्यागम शून्यसूचकः;
#यदि_अन्तः
}

शून्य llvm::Restore_Pretty_राशिः_State(ध्रुव शून्य *शिखरम्) {
#यदि ENABLE_BACKTRACES
  Pretty_राशिः_Trace_शीर्ष =
      static_cast<Pretty_राशिः_Trace_प्रविष्टिः *>(const_cast<शून्य *>(शिखरम्));
#यदि_अन्तः
}

शून्य LLVMEnable_Pretty_राशिः_Trace() {
  Enable_Pretty_राशिः_Trace();
}
