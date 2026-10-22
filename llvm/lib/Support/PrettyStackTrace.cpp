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

#समावेश "llvm/Support/PrettyStackTrace.h"
#समावेश "llvm-c/ErrorHandling.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/SaveAndRestore.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/Watchdog.h"
#समावेश "llvm/Support/raw_ostream.h"

#अगर_घोषित __APPLE__
#समावेश "llvm/ADT/SmallString.h"
#पूर्ण_अगर

#समावेश <atomic>
#समावेश <cassert>
#समावेश <cstdarg>
#समावेश <cstdio>
#समावेश <cstring>
#समावेश <tuple>

#अगर __has_include(<CrashReporterClient.h>)
#समावेश <CrashReporterClient.h>
#घोषणा LLVM_HAVE_CRASHREPORTERCLIENT_H
#पूर्ण_अगर

using namespace llvm;

अटल स्थिर अक्षर *Bug_Report_संदेश =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#अगर ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
अटल LLVM_THREAD_LOCAL Pretty_ढेर_Trace_Entry *Pretty_ढेर_Trace_शीर्ष = nullptr;

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
अटल अस्थिर std::atomic<अचिन्हित> Global_Sig_Info_Generation_गणक = 1;
अटल LLVM_THREAD_LOCAL अचिन्हित Thread_Local_Sig_Info_Generation_गणक = 0;

namespace llvm {
Pretty_ढेर_Trace_Entry *Reverse_ढेर_Trace(Pretty_ढेर_Trace_Entry *शीर्ष_2) {
  Pretty_ढेर_Trace_Entry *पूर्व_2 = nullptr;
  जबतक (शीर्ष_2)
    std::tie(पूर्व_2, शीर्ष_2, शीर्ष_2->अग्र_Entry) =
        std::make_tuple(शीर्ष_2, शीर्ष_2->अग्र_Entry, पूर्व_2);
  वापस पूर्व_2;
}
} // namespace llvm

अटल व्योम Print_ढेर(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  अचिन्हित ID = 0;
  SaveAndRestore<Pretty_ढेर_Trace_Entry *> Saved_ढेर{Pretty_ढेर_Trace_शीर्ष,
                                                     nullptr};
  Pretty_ढेर_Trace_Entry *Reversed_ढेर = Reverse_ढेर_Trace(Saved_ढेर.लो());
  क्रम (स्थिर Pretty_ढेर_Trace_Entry *Entry = Reversed_ढेर; Entry;
       Entry = Entry->get_अग्र_Entry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->print(OS);
  }
  llvm::Reverse_ढेर_Trace(Reversed_ढेर);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
अटल व्योम Print_Cur_ढेर_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  अगर (!Pretty_ढेर_Trace_शीर्ष) वापस;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ढेर(OS);
  OS.सफाई();
}

// Integrate with crash reporter libraries.
#अगर defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
बाह्य "C" {
#अगर_घोषित CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#अन्यथा
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
काष्ठा crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#अगर CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#पूर्ण_अगर // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#पूर्ण_अगर // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#या_अगर defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
बाह्य "C" स्थिर अक्षर *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
यंत्र(".desc ___crashreporter_info__, 0x10");
#पूर्ण_अगर

[[maybe_unused]] अटल व्योम setCrashLogMessage(स्थिर अक्षर *संदेश);
अटल व्योम setCrashLogMessage(स्थिर अक्षर *संदेश) {
#अगर_घोषित LLVM_HAVE_CRASHREPORTERCLIENT_H
  (व्योम)CRSetCrashLogMessage(संदेश);
#या_अगर HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = संदेश;
#पूर्ण_अगर
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#अगर_घोषित __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[माप(CrashHandlerString)];
alignas(CrashHandlerString) अटल CrashHandlerStringStorage
    crashHandlerStringStorage;
#पूर्ण_अगर

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
अटल व्योम CrashHandler(व्योम *) {
  errs() << Bug_Report_संदेश ;

#अगर_अघोषित __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ढेर_Trace(errs());
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
  स्वतः &crashHandlerString =
      *नया (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    Print_Cur_ढेर_Trace(Stream);
  }

  अगर (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } अन्यथा {
    setCrashLogMessage("No crash information.");
  }
#पूर्ण_अगर
}

अटल व्योम printForSigInfoIfNeeded() {
  अचिन्हित CurrentSigInfoGeneration =
      Global_Sig_Info_Generation_गणक.load(std::memory_order_relaxed);
  अगर (Thread_Local_Sig_Info_Generation_गणक == 0 ||
      Thread_Local_Sig_Info_Generation_गणक == CurrentSigInfoGeneration) {
    वापस;
  }

  Print_Cur_ढेर_Trace(errs());
  Thread_Local_Sig_Info_Generation_गणक = CurrentSigInfoGeneration;
}

#पूर्ण_अगर // ENABLE_BACKTRACES

व्योम llvm::set_Bug_Report_संदेश(स्थिर अक्षर *संदेश_2) {
  Bug_Report_संदेश = संदेश_2;
}

स्थिर अक्षर *llvm::get_Bug_Report_संदेश() {
  वापस Bug_Report_संदेश;
}

Pretty_ढेर_Trace_Entry::Pretty_ढेर_Trace_Entry() {
#अगर ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  अग्र_Entry = Pretty_ढेर_Trace_शीर्ष;
  Pretty_ढेर_Trace_शीर्ष = यह;
#पूर्ण_अगर
}

Pretty_ढेर_Trace_Entry::~Pretty_ढेर_Trace_Entry() {
#अगर ENABLE_BACKTRACES
  निश्चित(Pretty_ढेर_Trace_शीर्ष == यह &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ढेर_Trace_शीर्ष = अग्र_Entry;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#पूर्ण_अगर
}

व्योम Pretty_ढेर_Trace_String::print(raw_ostream &OS) स्थिर { OS << Str << "\n"; }

Pretty_ढेर_Trace_Format::Pretty_ढेर_Trace_Format(स्थिर अक्षर *Format, ...) {
  बहु_सूची AP;
  बहु_शुरू(AP, Format);
  स्थिर पूर्णांक आकार_Or_Error = vsnprintf(nullptr, 0, Format, AP);
  बहु_पूर्ण(AP);
  अगर (आकार_Or_Error < 0) {
    वापस;
  }

  स्थिर पूर्णांक आकार = आकार_Or_Error + 1; // '\0'
  Str.resize(आकार);
  बहु_शुरू(AP, Format);
  vsnprintf(Str.तथ्य(), आकार, Format, AP);
  बहु_पूर्ण(AP);
}

व्योम Pretty_ढेर_Trace_Format::print(raw_ostream &OS) स्थिर { OS << Str << "\n"; }

व्योम Pretty_ढेर_Trace_Program::print(raw_ostream &OS) स्थिर {
  OS << "Program arguments: ";
  // Print the argument list.
  क्रम (पूर्णांक I = 0; I < ArgC; ++I) {
    स्थिर bool HaveSpace = ::म_अक्षर(ArgV[I], ' ');
    अगर (I)
      OS << ' ';
    अगर (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    अगर (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#अगर ENABLE_BACKTRACES
अटल bool RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  वापस false;
}
#पूर्ण_अगर

व्योम llvm::Enable_Pretty_ढेर_Trace() {
#अगर ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  अटल bool HandlerRegistered = RegisterCrashPrinter();
  (व्योम)HandlerRegistered;
#पूर्ण_अगर
}

व्योम llvm::Enable_Pretty_ढेर_Trace_On_Sig_Info_For_This_Thread(bool ShouldEnable) {
#अगर ENABLE_BACKTRACES
  अगर (!ShouldEnable) {
    Thread_Local_Sig_Info_Generation_गणक = 0;
    वापस;
  }

  // The first time this is called, we register the SIGINFO handler.
  अटल bool HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      Global_Sig_Info_Generation_गणक.fetch_add(1, std::memory_order_relaxed);
    });
    वापस false;
  }();
  (व्योम)HandlerRegistered;

  // Next, enable it for the current thread.
  Thread_Local_Sig_Info_Generation_गणक =
      Global_Sig_Info_Generation_गणक.load(std::memory_order_relaxed);
#पूर्ण_अगर
}

स्थिर व्योम *llvm::Save_Pretty_ढेर_State() {
#अगर ENABLE_BACKTRACES
  वापस Pretty_ढेर_Trace_शीर्ष;
#अन्यथा
  वापस nullptr;
#पूर्ण_अगर
}

व्योम llvm::Restore_Pretty_ढेर_State(स्थिर व्योम *शीर्ष_3) {
#अगर ENABLE_BACKTRACES
  Pretty_ढेर_Trace_शीर्ष =
      static_cast<Pretty_ढेर_Trace_Entry *>(const_cast<व्योम *>(शीर्ष_3));
#पूर्ण_अगर
}

व्योम LLVMEnable_Pretty_ढेर_Trace() {
  Enable_Pretty_ढेर_Trace();
}
