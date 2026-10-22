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

#اگر_تعریف_شده __APPLE__
#ضمن "llvm/ADT/SmallString.h"
#پایان_اگر

#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdarg>
#ضمن <cstdio>
#ضمن <cstring>
#ضمن <tuple>

#إذا __has_include(<CrashReporterClient.h>)
#ضمن <CrashReporterClient.h>
#تعریف_کن LLVM_HAVE_CRASHREPORTERCLIENT_H
#پایان_اگر

استفاده نطاق llvm;

ساكن ثابت حرف *Bug_Report_پیام =
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
ساكن LLVM_THREAD_LOCAL Pretty_پشته_Trace_مدخل *Pretty_پشته_Trace_سر = اشاره_تهی;

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
ساكن متطاير std::atomic<بیعلامت> GlobalSigInfoGenerationCounter = 1;
ساكن LLVM_THREAD_LOCAL بیعلامت ThreadLocalSigInfoGenerationCounter = 0;

نطاق llvm {
Pretty_پشته_Trace_مدخل *معکوس_پشته_Trace(Pretty_پشته_Trace_مدخل *سر_2) {
  Pretty_پشته_Trace_مدخل *Prev = اشاره_تهی;
  طالما (سر_2)
    std::tie(Prev, سر_2, سر_2->بعدی_مدخل) =
        std::make_tuple(سر_2, سر_2->بعدی_مدخل, Prev);
  أرجع Prev;
}
} // namespace llvm

ساكن فراغ Print_پشته(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  بیعلامت ID = 0;
  SaveAndRestore<Pretty_پشته_Trace_مدخل *> Saved_پشته{Pretty_پشته_Trace_سر,
                                                     اشاره_تهی};
  Pretty_پشته_Trace_مدخل *Reversed_پشته = معکوس_پشته_Trace(Saved_پشته.get());
  لكل (ثابت Pretty_پشته_Trace_مدخل *مدخل = Reversed_پشته; مدخل;
       مدخل = مدخل->get_بعدی_مدخل()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    مدخل->print(OS);
  }
  llvm::معکوس_پشته_Trace(Reversed_پشته);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساكن فراغ Print_Cur_پشته_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  إذا (!Pretty_پشته_Trace_سر) أرجع;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_پشته(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#إذا defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجي "C" {
#اگر_تعریف_شده CRASHREPORTER_ANNOTATIONS_INITIALIZER
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
#پایان_اگر // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#پایان_اگر // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وإلاإذا defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجي "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#پایان_اگر

[[maybe_استفادهنشده]] ساكن فراغ مجموعه_Crash_Log_پیام(ثابت حرف *پیام);
ساكن فراغ مجموعه_Crash_Log_پیام(ثابت حرف *پیام) {
#اگر_تعریف_شده LLVM_HAVE_CRASHREPORTERCLIENT_H
  (فراغ)CRSet_Crash_Log_پیام(پیام);
#وإلاإذا HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = پیام;
#پایان_اگر
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#اگر_تعریف_شده __APPLE__
استفاده CrashHandlerString = SmallString<2048>;
استفاده CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساكن CrashHandlerStringStorage
    crashHandlerStringStorage;
#پایان_اگر

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساكن فراغ CrashHandler(فراغ *) {
  errs() << Bug_Report_پیام ;

#اگر_تعریف_نشده __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_پشته_Trace(errs());
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
  خودکار &crashHandlerString =
      *جديد (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  مجموعه_Crash_Log_پیام(crashHandlerString.c_str());

  {
    raw_svector_ostream جریان(crashHandlerString);
    Print_Cur_پشته_Trace(جریان);
  }

  إذا (!crashHandlerString.خالی()) {
    مجموعه_Crash_Log_پیام(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } وإلا {
    مجموعه_Crash_Log_پیام("No crash information.");
  }
#پایان_اگر
}

ساكن فراغ printForSigInfoIfNeeded() {
  بیعلامت CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.بارگیری(std::memory_order_relaxed);
  إذا (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    أرجع;
  }

  Print_Cur_پشته_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#پایان_اگر // ENABLE_BACKTRACES

فراغ llvm::مجموعه_Bug_Report_پیام(ثابت حرف *پیام_4) {
  Bug_Report_پیام = پیام_4;
}

ثابت حرف *llvm::get_Bug_Report_پیام() {
  أرجع Bug_Report_پیام;
}

Pretty_پشته_Trace_مدخل::Pretty_پشته_Trace_مدخل() {
#إذا ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  بعدی_مدخل = Pretty_پشته_Trace_سر;
  Pretty_پشته_Trace_سر = هذا;
#پایان_اگر
}

Pretty_پشته_Trace_مدخل::~Pretty_پشته_Trace_مدخل() {
#إذا ENABLE_BACKTRACES
  أكد(Pretty_پشته_Trace_سر == هذا &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_پشته_Trace_سر = بعدی_مدخل;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#پایان_اگر
}

فراغ Pretty_پشته_Trace_String::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_پشته_Trace_Format::Pretty_پشته_Trace_Format(ثابت حرف *Format, ...) {
  va_list AP;
  va_شروع(AP, Format);
  ثابت صحيح اندازه_Or_Error = vsnprintf(اشاره_تهی, 0, Format, AP);
  va_انتها(AP);
  إذا (اندازه_Or_Error < 0) {
    أرجع;
  }

  ثابت صحيح اندازه = اندازه_Or_Error + 1; // '\0'
  Str.resize(اندازه);
  va_شروع(AP, Format);
  vsnprintf(Str.داده(), اندازه, Format, AP);
  va_انتها(AP);
}

فراغ Pretty_پشته_Trace_Format::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

فراغ Pretty_پشته_Trace_برنامه::print(raw_ostream &OS) ثابت {
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
ساكن منطقي ثبات_Crash_Printer() {
  sys::جمع_کن_Signal_Handler(CrashHandler, اشاره_تهی);
  أرجع خطأ;
}
#پایان_اگر

فراغ llvm::Enable_Pretty_پشته_Trace() {
#إذا ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساكن منطقي HandlerRegistered = ثبات_Crash_Printer();
  (فراغ)HandlerRegistered;
#پایان_اگر
}

فراغ llvm::Enable_Pretty_پشته_Trace_On_Sig_Info_For_This_Thread(منطقي ShouldEnable) {
#إذا ENABLE_BACKTRACES
  إذا (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    أرجع;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساكن منطقي HandlerRegistered = []{
    sys::مجموعه_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_جمع_کن(1, std::memory_order_relaxed);
    });
    أرجع خطأ;
  }();
  (فراغ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.بارگیری(std::memory_order_relaxed);
#پایان_اگر
}

ثابت فراغ *llvm::Save_Pretty_پشته_State() {
#إذا ENABLE_BACKTRACES
  أرجع Pretty_پشته_Trace_سر;
#وإلا
  أرجع اشاره_تهی;
#پایان_اگر
}

فراغ llvm::Restore_Pretty_پشته_State(ثابت فراغ *بالاترین) {
#إذا ENABLE_BACKTRACES
  Pretty_پشته_Trace_سر =
      static_cast<Pretty_پشته_Trace_مدخل *>(const_cast<فراغ *>(بالاترین));
#پایان_اگر
}

فراغ LLVMEnable_Pretty_پشته_Trace() {
  Enable_Pretty_پشته_Trace();
}
