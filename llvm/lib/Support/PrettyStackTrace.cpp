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

#شامل "llvm/Support/PrettyStackTrace.h"
#شامل "llvm-c/ErrorHandling.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/SaveAndRestore.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/Watchdog.h"
#شامل "llvm/Support/raw_ostream.h"

#اگر_تعریف_شدہ __APPLE__
#شامل "llvm/ADT/SmallString.h"
#اگر_ختم

#شامل <atomic>
#شامل <cassert>
#شامل <cstdarg>
#شامل <cstdio>
#شامل <cstring>
#شامل <tuple>

#اگر __has_include(<CrashReporterClient.h>)
#شامل <CrashReporterClient.h>
#تعریف_کرو LLVM_HAVE_CRASHREPORTERCLIENT_H
#اگر_ختم

استعمال نامخلا llvm;

ساکن ثابت حرف *Bug_Report_پیغام =
    "PLEASE submit a bug report to " BUG_REPORT_URL
    " and include the crash backtrace and instructions to reproduce the bug.\n";

// If backtrace support is not enabled, compile out support for pretty stack
// traces.  This has the secondary effect of not requiring thread local storage
// when backtrace support is disabled.
#اگر ENABLE_BACKTRACES

// We need a thread local pointer to manage the stack of our stack trace
// objects, but we *really* cannot tolerate destructors running and do not want
// to pay any overhead of synchronizing. As a consequence, we use a raw
// thread-local variable.
ساکن LLVM_THREAD_LOCAL Pretty_ڈھیر_Trace_اندراج *Pretty_ڈھیر_Trace_سر = خالی_اشاریہ;

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
ساکن متلون std::atomic<غیرعلامتی> Global_Sig_Info_Generation_شمار = 1;
ساکن LLVM_THREAD_LOCAL غیرعلامتی Thread_Local_Sig_Info_Generation_شمار = 0;

نامخلا llvm {
Pretty_ڈھیر_Trace_اندراج *الٹ_ڈھیر_Trace(Pretty_ڈھیر_Trace_اندراج *سر_2) {
  Pretty_ڈھیر_Trace_اندراج *پچھلا_2 = خالی_اشاریہ;
  جب (سر_2)
    std::tie(پچھلا_2, سر_2, سر_2->اگلا_اندراج) =
        std::make_tuple(سر_2, سر_2->اگلا_اندراج, پچھلا_2);
  واپس پچھلا_2;
}
} // namespace llvm

ساکن خالی Print_ڈھیر(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  غیرعلامتی ID = 0;
  SaveAndRestore<Pretty_ڈھیر_Trace_اندراج *> Saved_ڈھیر{Pretty_ڈھیر_Trace_سر,
                                                     خالی_اشاریہ};
  Pretty_ڈھیر_Trace_اندراج *Reversed_ڈھیر = الٹ_ڈھیر_Trace(Saved_ڈھیر.get());
  برائے (ثابت Pretty_ڈھیر_Trace_اندراج *اندراج_2 = Reversed_ڈھیر; اندراج_2;
       اندراج_2 = اندراج_2->get_اگلا_اندراج()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    اندراج_2->print(OS);
  }
  llvm::الٹ_ڈھیر_Trace(Reversed_ڈھیر);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساکن خالی Print_Cur_ڈھیر_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  اگر (!Pretty_ڈھیر_Trace_سر) واپس;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ڈھیر(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#اگر defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجی "C" {
#اگر_تعریف_شدہ CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#ورنہ
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
ساخت crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_نسخہ,
        0,
        0,
        0,
        0,
        0,
        0,
#اگر CRASHREPORTER_ANNOTATIONS_نسخہ > 4
        0
#اگر_ختم // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#اگر_ختم // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وگرنہ_اگر defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجی "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#اگر_ختم

[[maybe_غیر_مستعمل]] ساکن خالی سیٹ_Crash_Log_پیغام(ثابت حرف *پیغام);
ساکن خالی سیٹ_Crash_Log_پیغام(ثابت حرف *پیغام) {
#اگر_تعریف_شدہ LLVM_HAVE_CRASHREPORTERCLIENT_H
  (خالی)CRSet_Crash_Log_پیغام(پیغام);
#وگرنہ_اگر HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = پیغام;
#اگر_ختم
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#اگر_تعریف_شدہ __APPLE__
استعمال CrashHandlerString = SmallString<2048>;
استعمال CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساکن CrashHandlerStringStorage
    crashHandlerStringStorage;
#اگر_ختم

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساکن خالی CrashHandler(خالی *) {
  errs() << Bug_Report_پیغام ;

#اگر_غیر_تعریف_شدہ __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ڈھیر_Trace(errs());
#ورنہ
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
      *نیا (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  سیٹ_Crash_Log_پیغام(crashHandlerString.c_str());

  {
    raw_svector_ostream دھارا(crashHandlerString);
    Print_Cur_ڈھیر_Trace(دھارا);
  }

  اگر (!crashHandlerString.خالی_خانے()) {
    سیٹ_Crash_Log_پیغام(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } ورنہ {
    سیٹ_Crash_Log_پیغام("No crash information.");
  }
#اگر_ختم
}

ساکن خالی printForSigInfoIfNeeded() {
  غیرعلامتی CurrentSigInfoGeneration =
      Global_Sig_Info_Generation_شمار.بھرو(std::memory_order_relaxed);
  اگر (Thread_Local_Sig_Info_Generation_شمار == 0 ||
      Thread_Local_Sig_Info_Generation_شمار == CurrentSigInfoGeneration) {
    واپس;
  }

  Print_Cur_ڈھیر_Trace(errs());
  Thread_Local_Sig_Info_Generation_شمار = CurrentSigInfoGeneration;
}

#اگر_ختم // ENABLE_BACKTRACES

خالی llvm::سیٹ_Bug_Report_پیغام(ثابت حرف *پیغام_4) {
  Bug_Report_پیغام = پیغام_4;
}

ثابت حرف *llvm::get_Bug_Report_پیغام() {
  واپس Bug_Report_پیغام;
}

Pretty_ڈھیر_Trace_اندراج::Pretty_ڈھیر_Trace_اندراج() {
#اگر ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  اگلا_اندراج = Pretty_ڈھیر_Trace_سر;
  Pretty_ڈھیر_Trace_سر = یہ;
#اگر_ختم
}

Pretty_ڈھیر_Trace_اندراج::~Pretty_ڈھیر_Trace_اندراج() {
#اگر ENABLE_BACKTRACES
  أكد(Pretty_ڈھیر_Trace_سر == یہ &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ڈھیر_Trace_سر = اگلا_اندراج;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#اگر_ختم
}

خالی Pretty_ڈھیر_Trace_String::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_ڈھیر_Trace_Format::Pretty_ڈھیر_Trace_Format(ثابت حرف *Format, ...) {
  va_list AP;
  va_آغاز(AP, Format);
  ثابت صحیح سائز_Or_Error = vsnprintf(خالی_اشاریہ, 0, Format, AP);
  va_انتہا(AP);
  اگر (سائز_Or_Error < 0) {
    واپس;
  }

  ثابت صحیح سائز = سائز_Or_Error + 1; // '\0'
  Str.resize(سائز);
  va_آغاز(AP, Format);
  vsnprintf(Str.ڈیٹا(), سائز, Format, AP);
  va_انتہا(AP);
}

خالی Pretty_ڈھیر_Trace_Format::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

خالی Pretty_ڈھیر_Trace_پروگرام::print(raw_ostream &OS) ثابت {
  OS << "Program arguments: ";
  // Print the argument list.
  برائے (صحیح I = 0; I < ArgC; ++I) {
    ثابت منطقی HaveSpace = ::strchr(ArgV[I], ' ');
    اگر (I)
      OS << ' ';
    اگر (HaveSpace)
      OS << '"';
    OS.write_escaped(ArgV[I]);
    اگر (HaveSpace)
      OS << '"';
  }
  OS << '\n';
}

#اگر ENABLE_BACKTRACES
ساکن منطقی رجسٹر_Crash_Printer() {
  sys::جوڑو_Signal_Handler(CrashHandler, خالی_اشاریہ);
  واپس جھوٹ;
}
#اگر_ختم

خالی llvm::Enable_Pretty_ڈھیر_Trace() {
#اگر ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساکن منطقی HandlerRegistered = رجسٹر_Crash_Printer();
  (خالی)HandlerRegistered;
#اگر_ختم
}

خالی llvm::Enable_Pretty_ڈھیر_Trace_On_Sig_Info_For_This_Thread(منطقی ShouldEnable) {
#اگر ENABLE_BACKTRACES
  اگر (!ShouldEnable) {
    Thread_Local_Sig_Info_Generation_شمار = 0;
    واپس;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساکن منطقی HandlerRegistered = []{
    sys::سیٹ_Info_Signal_Function([]{
      Global_Sig_Info_Generation_شمار.fetch_جوڑو(1, std::memory_order_relaxed);
    });
    واپس جھوٹ;
  }();
  (خالی)HandlerRegistered;

  // Next, enable it for the current thread.
  Thread_Local_Sig_Info_Generation_شمار =
      Global_Sig_Info_Generation_شمار.بھرو(std::memory_order_relaxed);
#اگر_ختم
}

ثابت خالی *llvm::Save_Pretty_ڈھیر_State() {
#اگر ENABLE_BACKTRACES
  واپس Pretty_ڈھیر_Trace_سر;
#ورنہ
  واپس خالی_اشاریہ;
#اگر_ختم
}

خالی llvm::Restore_Pretty_ڈھیر_State(ثابت خالی *چوٹی) {
#اگر ENABLE_BACKTRACES
  Pretty_ڈھیر_Trace_سر =
      static_cast<Pretty_ڈھیر_Trace_اندراج *>(const_cast<خالی *>(چوٹی));
#اگر_ختم
}

خالی LLVMEnable_Pretty_ڈھیر_Trace() {
  Enable_Pretty_ڈھیر_Trace();
}
