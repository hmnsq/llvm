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

#ifdef __APPLE__
#ضمن "llvm/ADT/SmallString.h"
#endif

#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdarg>
#ضمن <cstdio>
#ضمن <cstring>
#ضمن <tuple>

#إذا __has_include(<CrashReporterClient.h>)
#ضمن <CrashReporterClient.h>
#define LLVM_HAVE_CRASHREPORTERCLIENT_H
#endif

using نطاق llvm;

ساكن ثابت حرف *Bug_Report_رسالة =
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
ساكن LLVM_THREAD_LOCAL Pretty_مكدس_Trace_Entry *Pretty_مكدس_Trace_Head = nullptr;

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
ساكن متطاير std::atomic<unsigned> GlobalSigInfoGenerationCounter = 1;
ساكن LLVM_THREAD_LOCAL unsigned ThreadLocalSigInfoGenerationCounter = 0;

نطاق llvm {
Pretty_مكدس_Trace_Entry *Reverse_مكدس_Trace(Pretty_مكدس_Trace_Entry *Head) {
  Pretty_مكدس_Trace_Entry *Prev = nullptr;
  طالما (Head)
    std::tie(Prev, Head, Head->NextEntry) =
        std::make_tuple(Head, Head->NextEntry, Prev);
  أرجع Prev;
}
} // namespace llvm

ساكن فراغ اطبع_مكدس(raw_ostream &OS) {
  // Print out the stack in reverse order. To avoid recursion (which is likely
  // to fail if we crashed due to stack overflow), we do an up-front pass to
  // reverse the stack, then print it, then reverse it again.
  unsigned ID = 0;
  SaveAndRestore<Pretty_مكدس_Trace_Entry *> Saved_مكدس{Pretty_مكدس_Trace_Head,
                                                     nullptr};
  Pretty_مكدس_Trace_Entry *Reversed_مكدس = Reverse_مكدس_Trace(Saved_مكدس.get());
  لكل (ثابت Pretty_مكدس_Trace_Entry *Entry = Reversed_مكدس; Entry;
       Entry = Entry->getNextEntry()) {
    OS << ID++ << ".\t";
    sys::Watchdog W(5);
    Entry->اطبع(OS);
  }
  llvm::Reverse_مكدس_Trace(Reversed_مكدس);
}

/// Print the current stack trace to the specified stream.
///
/// Marked NOINLINE so it can be called from debuggers.
LLVM_ATTRIBUTE_NOINLINE
ساكن فراغ اطبع_Cur_مكدس_Trace(raw_ostream &OS) {
  // Don't print an empty trace.
  إذا (!Pretty_مكدس_Trace_Head) أرجع;

  // If there are pretty stack frames registered, walk and emit them.
  OS << "Stack dump:\n";

  اطبع_مكدس(OS);
  OS.flush();
}

// Integrate with crash reporter libraries.
#إذا defined(__APPLE__) && defined(LLVM_HAVE_CRASHREPORTERCLIENT_H)
//  If any clients of llvm try to link to libCrashReporterClient.a themselves,
//  only one crash info struct will be used.
خارجي "C" {
#ifdef CRASHREPORTER_ANNOTATIONS_INITIALIZER
// Should be available in CRASHREPORTER_ANNOTATIONS_VERSION > 5
CRASHREPORTER_ANNOTATIONS_INITIALIZER()
#وإلا
// Older CrashReporter annotations layouts
CRASH_REPORTER_CLIENT_HIDDEN
بنية crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) = {
        CRASHREPORTER_ANNOTATIONS_VERSION,
        0,
        0,
        0,
        0,
        0,
        0,
#إذا CRASHREPORTER_ANNOTATIONS_VERSION > 4
        0
#endif // CRASHREPORTER_ANNOTATIONS_VERSION > 4
};
#endif // CRASHREPORTER_ANNOTATIONS_INITIALIZER
} // extern "C"
#وإلاإذا defined(__APPLE__) && HAVE_CRASHREPORTER_INFO
خارجي "C" ثابت حرف *__crashreporter_info__
    __attribute__((visibility("hidden"))) = 0;
asm(".desc ___crashreporter_info__, 0x10");
#endif

[[maybe_unused]] ساكن فراغ setCrashLogMessage(ثابت حرف *رسالة);
ساكن فراغ setCrashLogMessage(ثابت حرف *رسالة) {
#ifdef LLVM_HAVE_CRASHREPORTERCLIENT_H
  (فراغ)CRSetCrashLogMessage(رسالة);
#وإلاإذا HAVE_CRASHREPORTER_INFO
  __crashreporter_info__ = رسالة;
#endif
  // Don't reorder subsequent operations: whatever comes after might crash and
  // we want the system crash handling to see the message we just set.
  std::atomic_signal_fence(std::memory_order_seq_cst);
}

#ifdef __APPLE__
using CrashHandlerString = SmallString<2048>;
using CrashHandlerStringStorage = std::byte[حجم(CrashHandlerString)];
alignas(CrashHandlerString) ساكن CrashHandlerStringStorage
    crashHandlerStringStorage;
#endif

/// This callback is run if a fatal signal is delivered to the process, it
/// prints the pretty stack trace.
ساكن فراغ CrashHandler(فراغ *) {
  errs() << Bug_Report_رسالة ;

#ifndef __APPLE__
  // On non-apple systems, just emit the crash stack trace to stderr.
  اطبع_Cur_مكدس_Trace(errs());
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
  auto &crashHandlerString =
      *جديد (&crashHandlerStringStorage) CrashHandlerString;

  // If we crash while trying to print the stack trace, we still want the system
  // crash handling to have some partial information. That'll work out as long
  // as the SmallString doesn't allocate. If it does allocate then the system
  // crash handling will see some garbage because the inline buffer now contains
  // a pointer.
  setCrashLogMessage(crashHandlerString.c_str());

  {
    raw_svector_ostream Stream(crashHandlerString);
    اطبع_Cur_مكدس_Trace(Stream);
  }

  إذا (!crashHandlerString.empty()) {
    setCrashLogMessage(crashHandlerString.c_str());
    errs() << crashHandlerString.str();
  } وإلا {
    setCrashLogMessage("No crash information.");
  }
#endif
}

ساكن فراغ اطبع_For_Sig_Info_If_Needed() {
  unsigned CurrentSigInfoGeneration =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
  إذا (ThreadLocalSigInfoGenerationCounter == 0 ||
      ThreadLocalSigInfoGenerationCounter == CurrentSigInfoGeneration) {
    أرجع;
  }

  اطبع_Cur_مكدس_Trace(errs());
  ThreadLocalSigInfoGenerationCounter = CurrentSigInfoGeneration;
}

#endif // ENABLE_BACKTRACES

فراغ llvm::set_Bug_Report_رسالة(ثابت حرف *رسالة_2) {
  Bug_Report_رسالة = رسالة_2;
}

ثابت حرف *llvm::get_Bug_Report_رسالة() {
  أرجع Bug_Report_رسالة;
}

Pretty_مكدس_Trace_Entry::Pretty_مكدس_Trace_Entry() {
#إذا ENABLE_BACKTRACES
  // Handle SIGINFO first, because we haven't finished constructing yet.
  اطبع_For_Sig_Info_If_Needed();
  // Link ourselves.
  NextEntry = Pretty_مكدس_Trace_Head;
  Pretty_مكدس_Trace_Head = هذا;
#endif
}

Pretty_مكدس_Trace_Entry::~Pretty_مكدس_Trace_Entry() {
#إذا ENABLE_BACKTRACES
  أكد(Pretty_مكدس_Trace_Head == هذا &&
         "Pretty stack trace entry destruction is out of order");
  Pretty_مكدس_Trace_Head = NextEntry;
  // Handle SIGINFO first, because we already started destructing.
  اطبع_For_Sig_Info_If_Needed();
#endif
}

فراغ Pretty_مكدس_Trace_String::اطبع(raw_ostream &OS) ثابت { OS << Str << "\n"; }

Pretty_مكدس_Trace_Format::Pretty_مكدس_Trace_Format(ثابت حرف *Format, ...) {
  va_list AP;
  va_start(AP, Format);
  ثابت صحيح SizeOrError = vsnprintf(nullptr, 0, Format, AP);
  va_end(AP);
  إذا (SizeOrError < 0) {
    أرجع;
  }

  ثابت صحيح Size = SizeOrError + 1; // '\0'
  Str.resize(Size);
  va_start(AP, Format);
  vsnprintf(Str.data(), Size, Format, AP);
  va_end(AP);
}

فراغ Pretty_مكدس_Trace_Format::اطبع(raw_ostream &OS) ثابت { OS << Str << "\n"; }

فراغ Pretty_مكدس_Trace_Program::اطبع(raw_ostream &OS) ثابت {
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
ساكن منطقي RegisterCrashPrinter() {
  sys::AddSignalHandler(CrashHandler, nullptr);
  أرجع خطأ;
}
#endif

فراغ llvm::Enable_Pretty_مكدس_Trace() {
#إذا ENABLE_BACKTRACES
  // The first time this is called, we register the crash printer.
  ساكن منطقي HandlerRegistered = RegisterCrashPrinter();
  (فراغ)HandlerRegistered;
#endif
}

فراغ llvm::Enable_Pretty_مكدس_Trace_On_Sig_Info_For_This_Thread(منطقي ShouldEnable) {
#إذا ENABLE_BACKTRACES
  إذا (!ShouldEnable) {
    ThreadLocalSigInfoGenerationCounter = 0;
    أرجع;
  }

  // The first time this is called, we register the SIGINFO handler.
  ساكن منطقي HandlerRegistered = []{
    sys::SetInfoSignalFunction([]{
      GlobalSigInfoGenerationCounter.fetch_add(1, std::memory_order_relaxed);
    });
    أرجع خطأ;
  }();
  (فراغ)HandlerRegistered;

  // Next, enable it for the current thread.
  ThreadLocalSigInfoGenerationCounter =
      GlobalSigInfoGenerationCounter.load(std::memory_order_relaxed);
#endif
}

ثابت فراغ *llvm::Save_Pretty_مكدس_State() {
#إذا ENABLE_BACKTRACES
  أرجع Pretty_مكدس_Trace_Head;
#وإلا
  أرجع nullptr;
#endif
}

فراغ llvm::Restore_Pretty_مكدس_State(ثابت فراغ *قمة) {
#إذا ENABLE_BACKTRACES
  Pretty_مكدس_Trace_Head =
      static_cast<Pretty_مكدس_Trace_Entry *>(const_cast<فراغ *>(قمة));
#endif
}

فراغ LLVMEnable_Pretty_مكدس_Trace() {
  Enable_Pretty_مكدس_Trace();
}
