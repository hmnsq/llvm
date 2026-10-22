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

#ضمن "llvm/Support/PrettyStackTrace.h"
#ضمن "llvm-c/ErrorHandling.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/SaveAndRestore.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Watchdog.h"
#ضمن "llvm/Support/raw_ostream.h"

#که_تعریف_شوی __APPLE__
#ضمن "llvm/ADT/SmallString.h"
#د_که_پای

#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdarg>
#ضمن <cstdio>
#ضمن <cstring>
#ضمن <tuple>

#إذا __has_include(<CrashReporterClient.h>)
#ضمن <CrashReporterClient.h>
#تعریف_کړه LLVM_HAVE_CRASHREPORTERCLIENT_H
#د_که_پای

کارول نطاق llvm;

ساكن ثابت حرف *Bug_Report_پیغام =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#إذا ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ساكن LLVM_THREAD_LOCAL Pretty_ډېری_Trace_ثبت *Pretty_ډېری_Trace_سر = تش_اشاره;

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
ساكن متطاير std::atomic<بینښه> GlobalSigInfoGenerationCounter = 1;
ساكن LLVM_THREAD_LOCAL بینښه ThreadLocalSigInfoGenerationCounter = 0;

نطاق llvm {
Pretty_ډېری_Trace_ثبت *سرچپه_ډېری_Trace(Pretty_ډېری_Trace_ثبت *سر_2) {
  Pretty_ډېری_Trace_ثبت *Prev = تش_اشاره;
  طالما (سر_2)
    std::tie(Prev, سر_2, سر_2->بل_ثبت) =
        std::make_tuple(سر_2, سر_2->بل_ثبت, Prev);
  أرجع Prev;
}
} // namespace llvm

ساكن فراغ Print_ډېری(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  بینښه ID = 0;
  SaveAndRestore<Pretty_ډېری_Trace_ثبت *> Saved_ډېری{Pretty_ډېری_Trace_سر,
                                                     تش_اشاره};
  Pretty_ډېری_Trace_ثبت *Reversed_ډېری = سرچپه_ډېری_Trace(Saved_ډېری.get());
  لكل (ثابت Pretty_ډېری_Trace_ثبت *ثبت = Reversed_ډېری; ثبت;
       ثبت = ثبت->get_بل_ثبت()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    ثبت->print(OS);
  }
  llvm::سرچپه_ډېری_Trace(Reversed_ډېری);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساكن فراغ Print_Cur_ډېری_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  إذا (!Pretty_ډېری_Trace_سر) أرجع;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ډېری(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#إذا defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجي "C" {
#که_تعریف_شوی CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#وإلا
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
بنية crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_نسخه,
        0,
        0,
        0,
        0,
        0,
        0,
#إذا CRASHREPORTER_ANNOTATIONS_نسخه > 4
        0
#د_که_پای // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#د_که_پای // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وإلاإذا defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجي "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#د_که_پای

[[maybe_ناکارول_شوی]] ساكن فراغ سېټ_Crash_Log_پیغام(ثابت حرف *پیغام);
ساكن فراغ سېټ_Crash_Log_پیغام(ثابت حرف *پیغام) {
#که_تعریف_شوی LLVM_HAVE_CRASHREPORTERCLIENT_H
  (فراغ)CRSet_Crash_Log_پیغام(پیغام);
#وإلاإذا HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = پیغام;
#د_که_پای
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#که_تعریف_شوی __APPLE__
کارول CrashHandlerString = SmallString<2048>;
کارول CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساكن CrashHandlerStringStorage
    crashHandlerStringStorage;
#د_که_پای

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساكن فراغ CrashHandler(فراغ *) {
  errs() << Bug_Report_پیغام ;

#که_نه_تعریف_شوی __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ډېری_Trace(errs());
#وإلا
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
  خپلکاری &crashHandlerString =
      *جديد (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  سېټ_Crash_Log_پیغام(crashHandlerString.c_str());

  {
    raw_svector_ostream بهیر(crashHandlerString);
    Print_Cur_ډېری_Trace(بهیر);
  }

  إذا (!crashHandlerString.تشه()) {
    سېټ_Crash_Log_پیغام(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } وإلا {
    سېټ_Crash_Log_پیغام("No crash information.");
  }
#د_که_پای
}

ساكن فراغ printForSigInfoIfNeeded() {
  بینښه CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.بارول(std::memory_order_relaxed);
  إذا (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    أرجع;
  }

  Print_Cur_ډېری_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#د_که_پای // ENABLE_BACKTRACES

فراغ llvm::سېټ_Bug_Report_پیغام(ثابت حرف *پیغام_4) {
  Bug_Report_پیغام = پیغام_4;
}

ثابت حرف *llvm::get_Bug_Report_پیغام() {
  أرجع Bug_Report_پیغام;
}

Pretty_ډېری_Trace_ثبت::Pretty_ډېری_Trace_ثبت() {
#إذا ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  بل_ثبت = Pretty_ډېری_Trace_سر;
  Pretty_ډېری_Trace_سر = هذا;
#د_که_پای
}

Pretty_ډېری_Trace_ثبت::~Pretty_ډېری_Trace_ثبت() {
#إذا ENABLE_BACKTRACES
  أكد(Pretty_ډېری_Trace_سر == هذا &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ډېری_Trace_سر = بل_ثبت;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#د_که_پای
}

فراغ Pretty_ډېری_Trace_String::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_ډېری_Trace_Format::Pretty_ډېری_Trace_Format(ثابت حرف *Format, ...) {
  va_list AP;
  va_پیل(AP, Format);
  ثابت صحيح اندازه_Or_Error = vsnprintf(تش_اشاره, 0, Format, AP);
  va_پای_ټکی(AP);
  إذا (اندازه_Or_Error < 0) {
    أرجع;
  }

  ثابت صحيح اندازه = اندازه_Or_Error + 1; // '\0'
  Str.resize(اندازه);
  va_پیل(AP, Format);
  vsnprintf(Str.معلومات(), اندازه, Format, AP);
  va_پای_ټکی(AP);
}

فراغ Pretty_ډېری_Trace_Format::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

فراغ Pretty_ډېری_Trace_پروګرام::print(raw_ostream &OS) ثابت {
  OS << "Program arguments: ";
  // Print the argument list.
  لكل (صحيح I = 0; I < ArgC; ++I) {
    ثابت منطقي HaveSpace = ::strchr(ArgV[I], ' ');
    إذا (I)
      OS << ' ';
    إذا (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    إذا (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#إذا ENABLE_BACKTRACES
ساكن منطقي ثبتوونکی_Crash_Printer() {
  sys::جمع_کړه_Signal_Handler(CrashHandler, تش_اشاره);
  أرجع خطأ;
}
#د_که_پای

فراغ llvm::Enable_Pretty_ډېری_Trace() {
#إذا ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساكن منطقي HandlerRegistered = ثبتوونکی_Crash_Printer();
  (فراغ)HandlerRegistered;
#د_که_پای
}

فراغ llvm::Enable_Pretty_ډېری_Trace_On_Sig_Info_For_This_Thread(منطقي ShouldEnable) {
#إذا ENABLE_BACKTRACES
  إذا (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    أرجع;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساكن منطقي HandlerRegistered = []{
    sys::سېټ_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_جمع_کړه(1, std::memory_order_relaxed);
    });
    أرجع خطأ;
  }();
  (فراغ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.بارول(std::memory_order_relaxed);
#د_که_پای
}

ثابت فراغ *llvm::Save_Pretty_ډېری_State() {
#إذا ENABLE_BACKTRACES
  أرجع Pretty_ډېری_Trace_سر;
#وإلا
  أرجع تش_اشاره;
#د_که_پای
}

فراغ llvm::Restore_Pretty_ډېری_State(ثابت فراغ *سر_3) {
#إذا ENABLE_BACKTRACES
  Pretty_ډېری_Trace_سر =
      static_cast<Pretty_ډېری_Trace_ثبت *>(const_cast<فراغ *>(سر_3));
#د_که_پای
}

فراغ LLVMEnable_Pretty_ډېری_Trace() {
  Enable_Pretty_ډېری_Trace();
}
