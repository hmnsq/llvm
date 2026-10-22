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

#جيڪڏهن_وصف_ٿيل __APPLE__
#ضمن "llvm/ADT/SmallString.h"
#جيڪڏهن_پورو

#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdarg>
#ضمن <cstdio>
#ضمن <cstring>
#ضمن <tuple>

#إذا __has_include(<CrashReporterClient.h>)
#ضمن <CrashReporterClient.h>
#وصف_ڪريو LLVM_HAVE_CRASHREPORTERCLIENT_H
#جيڪڏهن_پورو

استعمال نطاق llvm;

ساكن ثابت حرف *Bug_Report_نياپو =
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
ساكن LLVM_THREAD_LOCAL Pretty_ڍير_Trace_داخلا *Pretty_ڍير_Trace_سر = خالي_اشارو;

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
ساكن متطاير std::atomic<بنانشان> GlobalSigInfoGenerationCounter = 1;
ساكن LLVM_THREAD_LOCAL بنانشان ThreadLocalSigInfoGenerationCounter = 0;

نطاق llvm {
Pretty_ڍير_Trace_داخلا *ابتو_ڍير_Trace(Pretty_ڍير_Trace_داخلا *سر_2) {
  Pretty_ڍير_Trace_داخلا *Prev = خالي_اشارو;
  طالما (سر_2)
    std::tie(Prev, سر_2, سر_2->ايندڙ_داخلا) =
        std::make_tuple(سر_2, سر_2->ايندڙ_داخلا, Prev);
  أرجع Prev;
}
} // namespace llvm

ساكن فراغ Print_ڍير(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  بنانشان ID = 0;
  SaveAndRestore<Pretty_ڍير_Trace_داخلا *> Saved_ڍير{Pretty_ڍير_Trace_سر,
                                                     خالي_اشارو};
  Pretty_ڍير_Trace_داخلا *Reversed_ڍير = ابتو_ڍير_Trace(Saved_ڍير.get());
  لكل (ثابت Pretty_ڍير_Trace_داخلا *داخلا_3 = Reversed_ڍير; داخلا_3;
       داخلا_3 = داخلا_3->get_ايندڙ_داخلا()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    داخلا_3->print(OS);
  }
  llvm::ابتو_ڍير_Trace(Reversed_ڍير);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساكن فراغ Print_Cur_ڍير_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  إذا (!Pretty_ڍير_Trace_سر) أرجع;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  Print_ڍير(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#إذا defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجي "C" {
#جيڪڏهن_وصف_ٿيل CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#وإلا
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
بنية crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_نسخو,
        0,
        0,
        0,
        0,
        0,
        0,
#إذا CRASHREPORTER_ANNOTATIONS_نسخو > 4
        0
#جيڪڏهن_پورو // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#جيڪڏهن_پورو // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وإلاإذا defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجي "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#جيڪڏهن_پورو

[[maybe_اڻ_استعمال_ٿيل]] ساكن فراغ سيٽ_Crash_Log_پيغام(ثابت حرف *نياپو);
ساكن فراغ سيٽ_Crash_Log_پيغام(ثابت حرف *نياپو) {
#جيڪڏهن_وصف_ٿيل LLVM_HAVE_CRASHREPORTERCLIENT_H
  (فراغ)CRSet_Crash_Log_پيغام(نياپو);
#وإلاإذا HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = نياپو;
#جيڪڏهن_پورو
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#جيڪڏهن_وصف_ٿيل __APPLE__
استعمال CrashHandlerString = SmallString<2048>;
استعمال CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساكن CrashHandlerStringStorage
    crashHandlerStringStorage;
#جيڪڏهن_پورو

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساكن فراغ CrashHandler(فراغ *) {
  errs() << Bug_Report_نياپو ;

#جيڪڏهن_وصف_نه_ٿيل __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  Print_Cur_ڍير_Trace(errs());
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
  خودڪار &crashHandlerString =
      *جديد (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  سيٽ_Crash_Log_پيغام(crashHandlerString.c_str());

  {
    raw_svector_ostream وهڪرو(crashHandlerString);
    Print_Cur_ڍير_Trace(وهڪرو);
  }

  إذا (!crashHandlerString.خالي_جايون()) {
    سيٽ_Crash_Log_پيغام(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } وإلا {
    سيٽ_Crash_Log_پيغام("No crash information.");
  }
#جيڪڏهن_پورو
}

ساكن فراغ printForSigInfoIfNeeded() {
  بنانشان CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.ڀريو(std::memory_order_relaxed);
  إذا (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    أرجع;
  }

  Print_Cur_ڍير_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#جيڪڏهن_پورو // ENABLE_BACKTRACES

فراغ llvm::سيٽ_Bug_Report_نياپو(ثابت حرف *نياپو_2) {
  Bug_Report_نياپو = نياپو_2;
}

ثابت حرف *llvm::get_Bug_Report_نياپو() {
  أرجع Bug_Report_نياپو;
}

Pretty_ڍير_Trace_داخلا::Pretty_ڍير_Trace_داخلا() {
#إذا ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  printForSigInfoIfNeeded();
  // Link ourselves.
  ايندڙ_داخلا = Pretty_ڍير_Trace_سر;
  Pretty_ڍير_Trace_سر = هذا;
#جيڪڏهن_پورو
}

Pretty_ڍير_Trace_داخلا::~Pretty_ڍير_Trace_داخلا() {
#إذا ENABLE_BACKTRACES
  أكد(Pretty_ڍير_Trace_سر == هذا &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_ڍير_Trace_سر = ايندڙ_داخلا;
  // Handle SIGINFO first, because we already started destructing.
  printForSigInfoIfNeeded();
#جيڪڏهن_پورو
}

فراغ Pretty_ڍير_Trace_String::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_ڍير_Trace_Format::Pretty_ڍير_Trace_Format(ثابت حرف *Format, ...) {
  va_list AP;
  va_شروعات(AP, Format);
  ثابت صحيح سائيز_Or_Error = vsnprintf(خالي_اشارو, 0, Format, AP);
  va_پڇاڙي(AP);
  إذا (سائيز_Or_Error < 0) {
    أرجع;
  }

  ثابت صحيح سائيز = سائيز_Or_Error + 1; // '\0'
  Str.resize(سائيز);
  va_شروعات(AP, Format);
  vsnprintf(Str.معلومات(), سائيز, Format, AP);
  va_پڇاڙي(AP);
}

فراغ Pretty_ڍير_Trace_Format::print(raw_ostream &OS) ثابت { OS << Str << "\n"; }

فراغ Pretty_ڍير_Trace_پروگرام::print(raw_ostream &OS) ثابت {
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
ساكن منطقي رجسٽر_Crash_Printer() {
  sys::جوڙيو_Signal_Handler(CrashHandler, خالي_اشارو);
  أرجع خطأ;
}
#جيڪڏهن_پورو

فراغ llvm::Enable_Pretty_ڍير_Trace() {
#إذا ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساكن منطقي HandlerRegistered = رجسٽر_Crash_Printer();
  (فراغ)HandlerRegistered;
#جيڪڏهن_پورو
}

فراغ llvm::Enable_Pretty_ڍير_Trace_On_Sig_Info_For_This_Thread(منطقي ShouldEnable) {
#إذا ENABLE_BACKTRACES
  إذا (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    أرجع;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساكن منطقي HandlerRegistered = []{
    sys::سيٽ_Info_Signal_Function([]{
      GlobalSigInfoGenerationCounter.fetch_جوڙيو(1, std::memory_order_relaxed);
    });
    أرجع خطأ;
  }();
  (فراغ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.ڀريو(std::memory_order_relaxed);
#جيڪڏهن_پورو
}

ثابت فراغ *llvm::Save_Pretty_ڍير_State() {
#إذا ENABLE_BACKTRACES
  أرجع Pretty_ڍير_Trace_سر;
#وإلا
  أرجع خالي_اشارو;
#جيڪڏهن_پورو
}

فراغ llvm::Restore_Pretty_ڍير_State(ثابت فراغ *چوٽي) {
#إذا ENABLE_BACKTRACES
  Pretty_ڍير_Trace_سر =
      static_cast<Pretty_ڍير_Trace_داخلا *>(const_cast<فراغ *>(چوٽي));
#جيڪڏهن_پورو
}

فراغ LLVMEnable_Pretty_ڍير_Trace() {
  Enable_Pretty_ڍير_Trace();
}
