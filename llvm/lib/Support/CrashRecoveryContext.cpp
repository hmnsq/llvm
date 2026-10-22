//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/CrashRecoveryContext.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/ExitCodes.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/thread.h"
#شامل <cassert>
#شامل <mutex>
#شامل <setjmp.h>
#اگر_تعریف_شدہ __APPLE__
#شامل <sys/resource.h>
#اگر_ختم

استعمال نامخلا llvm;

نامخلا {

ساخت CrashRecoveryContextImpl;
ساکن LLVM_THREAD_LOCAL ثابت CrashRecoveryContextImpl *CurrentContext;

ساخت CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ثابت CrashRecoveryContextImpl *اگلا_2;

  CrashRecoveryContext *چکری_جانچ;
  ::jmp_buf Jump_بفر;
  متلون غیرعلامتی Failed : 1;
  غیرعلامتی SwitchedThread : 1;
  غیرعلامتی Valid_Jump_بفر : 1;

عوامی:
  CrashRecoveryContextImpl(CrashRecoveryContext *چکری_جانچ) noexcept
      : چکری_جانچ(چکری_جانچ), Failed(جھوٹ), SwitchedThread(جھوٹ), Valid_Jump_بفر(جھوٹ) {
    اگلا_2 = CurrentContext;
    CurrentContext = یہ;
  }
  ~CrashRecoveryContextImpl() {
    اگر (!SwitchedThread)
      CurrentContext = اگلا_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  خالی سیٹ_Switched_Thread() {
#اگر defined(LLVM_ENABLE_دھاگے) && LLVM_ENABLE_دھاگے != 0
    SwitchedThread = سچ;
#اگر_ختم
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  خالی HandleCrash(صحیح Ret_رمز, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = اگلا_2;

    أكد(!Failed && "Crash recovery context already failed!");
    Failed = سچ;

    اگر (چکری_جانچ->Dump_ڈھیر_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    چکری_جانچ->Ret_رمز = Ret_رمز;

    // Jump back to the RunSafely we were called under.
    اگر (Valid_Jump_بفر)
      longjmp(Jump_بفر, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ساکن std::mutex CrashRecoveryContextMutex;
  واپس CrashRecoveryContextMutex;
}

ساکن منطقی gCrashRecoveryEnabled = جھوٹ;

ساکن LLVM_THREAD_LOCAL ثابت CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ساکن خالی
installExceptionOrSignalHandlers(منطقی NeedsPOSIXUtilitySignalHandling);
ساکن خالی uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = طےشدہ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = سر;
  ثابت CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = یہ;
  جب (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->اگلا;
    tmp->cleanupFired = سچ;
    tmp->recoverResources();
    حذف tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  حذف CRCI;
}

منطقی CrashRecoveryContext::isRecoveringFromCrash() {
  واپس IsRecoveringFromCrash != خالی_اشاریہ;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  اگر (!gCrashRecoveryEnabled)
    واپس خالی_اشاریہ;

  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;
  اگر (!CRCI)
    واپس خالی_اشاریہ;

  واپس CRCI->چکری_جانچ;
}

خالی CrashRecoveryContext::Enable(منطقی NeedsPOSIXUtilitySignalHandling) {
  std::تالا_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  اگر (gCrashRecoveryEnabled)
    واپس;
  gCrashRecoveryEnabled = سچ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

خالی CrashRecoveryContext::Disable() {
  std::تالا_guard<std::mutex> L(getCrashRecoveryContextMutex());
  اگر (!gCrashRecoveryEnabled)
    واپس;
  gCrashRecoveryEnabled = جھوٹ;
  uninstallExceptionOrSignalHandlers();
}

خالی CrashRecoveryContext::رجسٹر_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  اگر (!cleanup)
    واپس;
  اگر (سر)
    سر->پچھلا = cleanup;
  cleanup->اگلا = سر;
  سر = cleanup;
}

خالی
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  اگر (!cleanup)
    واپس;
  اگر (cleanup == سر) {
    سر = cleanup->اگلا;
    اگر (سر)
      سر->پچھلا = خالی_اشاریہ;
  }
  ورنہ {
    cleanup->پچھلا->اگلا = cleanup->اگلا;
    اگر (cleanup->اگلا)
      cleanup->اگلا->پچھلا = cleanup->پچھلا;
  }
  حذف cleanup;
}

#اگر defined(_MSC_VER)

#شامل <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

ساکن خالی
installExceptionOrSignalHandlers(منطقی NeedsPOSIXUtilitySignalHandling) {}
ساکن خالی uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
ساکن صحیح ExceptionFilter(EXCEPTION_اشارے *Except) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  اگر (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    واپس EXCEPTION_CONTINUE_تلاش;
  }

  صحیح Ret_رمز = (صحیح)Except->Exception_ریکارڈ->Exception_رمز;
  اگر ((Ret_رمز & 0xF0000000) == 0xE0000000)
    Ret_رمز &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_رمز, reinterpret_cast<uintptr_t>(Except));

  واپس EXCEPTION_EXECUTE_HANDLER;
}

#اگر defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#اگر_ختم
منطقی CrashRecoveryContext::تسلسل_Safely(function_ref<خالی()> Fn) {
  اگر (!gCrashRecoveryEnabled) {
    Fn();
    واپس سچ;
  }
  أكد(!Impl && "Crash recovery context already initialized!");
  Impl = نیا CrashRecoveryContextImpl(یہ);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    واپس جھوٹ;
  }
  واپس سچ;
}

#ورنہ // !_MSC_VER

#اگر defined(_WIN32)
// This is a non-MSVC compiler, probably mingw gcc or clang without
// -fms-extensions. Use vectored exception handling (VEH).
//
// On Windows, we can make use of vectored exception handling to catch most
// crashing situations.  Note that this does mean we will be alerted of
// exceptions *before* structured exception handling has the opportunity to
// catch it. Unfortunately, this causes problems in practice with other code
// running on threads with LLVM crash recovery contexts, so we would like to
// eventually move away from VEH.
//
// Vectored works on a per-thread basis, which is an advantage over
// SetUnhandledExceptionFilter. SetUnhandledExceptionFilter also doesn't have
// any native support for chaining exception handlers, but VEH allows more than
// one.
//
// The vectored exception handler functionality was added in Windows
// XP, so if support for older versions of Windows is required,
// it will have to be added.

#شامل "llvm/Support/Windows/WindowsSupport.h"

ساکن LONG CALLBACK ExceptionHandler(PEXCEPTION_اشارے ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  انتخاب (ExceptionInfo->Exception_ریکارڈ->Exception_رمز)
  {
  حالت DBG_PRINTEXCEPTION_C:
  حالت DbgPrintExceptionWideC:
  حالت 0x406D1388:  // set debugger thread name
    واپس EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  اگر (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    واپس EXCEPTION_CONTINUE_تلاش;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  صحیح Ret_رمز = (صحیح)ExceptionInfo->Exception_ریکارڈ->Exception_رمز;
  اگر ((Ret_رمز & 0xF0000000) == 0xE0000000)
    Ret_رمز &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_رمز, reinterpret_cast<uintptr_t>(ExceptionInfo));

  // Note that we don't actually get here because HandleCrash calls
  // longjmp, which means the HandleCrash function never returns.
  llvm_unreachable("Handled the crash, should have longjmp'ed out of here");
}

// Because the Enable and Disable calls are static, it means that
// there may not actually be an Impl available, or even a current
// CrashRecoveryContext at all.  So we make use of a thread-local
// exception table.  The handles contained in here will either be
// non-NULL, valid VEH handles, or NULL.
ساکن LLVM_THREAD_LOCAL ثابت خالی* sCurrentExceptionHandle;

ساکن خالی
installExceptionOrSignalHandlers(منطقی NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::جوڑو_Vectored_Exception_Handler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ساکن خالی uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  اگر (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = تہی;
  }
}

#ورنہ // !_WIN32

// Generic POSIX implementation.
//
// This implementation relies on synchronous signals being delivered to the
// current thread. We use a thread local object to keep track of the active
// crash recovery context, and install signal handlers to invoke HandleCrash on
// the active object.
//
// This implementation does not attempt to chain signal handlers in any
// reliable fashion -- if we get a signal outside of a crash recovery context we
// simply disable crash recovery and raise the signal again.

#شامل <signal.h>

ساکن ثابت صحیح اشارات[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ساکن ثابت غیرعلامتی عدد_اشارات = std::سائز_2(اشارات);
ساکن ساخت sigaction پچھلا_Actions[عدد_اشارات];

ساکن خالی CrashRecoverySignalHandler(صحیح Signal) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  اگر (!CRCI) {
    // We didn't find a crash recovery context -- this means either we got a
    // signal on a thread we didn't expect it on, the application got a signal
    // outside of a crash recovery context, or something else went horribly
    // wrong.
    //
    // Disable crash recovery and raise the signal again. The assumption here is
    // that the enclosing application will terminate soon, and we won't want to
    // attempt crash recovery again.
    //
    // Uninstall directly rather than through Disable(): a signal handler must
    // not take a lock, and a lock with static storage duration may already be
    // destroyed by the time a signal is delivered during exit(). Racing with
    // another thread here doesn't matter.
    gCrashRecoveryEnabled = جھوٹ;
    uninstallExceptionOrSignalHandlers();
    اٹھاؤ(Signal);

    // The signal will be thrown once the signal mask is restored.
    واپس;
  }

  // Unblock the signal we received.
  sigset_t Sig_نقاب;
  sigemptyset(&Sig_نقاب);
  sigaddset(&Sig_نقاب, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_نقاب, خالی_اشاریہ);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  صحیح Ret_رمز = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  اگر (Signal == SIGPIPE)
    Ret_رمز = EX_IOERR;

  اگر (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_رمز, Signal);
}

ساکن خالی
installExceptionOrSignalHandlers(منطقی NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ساخت sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_نقاب);

  برائے (غیرعلامتی i = 0; i != عدد_اشارات; ++i) {
    اگر (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ساخت sigaction act;
      اگر (sigaction(اشارات[i], تہی, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(اشارات[i], &Handler, &پچھلا_Actions[i]);
    } ورنہ {
      sigaction(اشارات[i], &Handler, &پچھلا_Actions[i]);
    }
  }
}

ساکن خالی uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  برائے (غیرعلامتی i = 0; i != عدد_اشارات; ++i)
    sigaction(اشارات[i], &پچھلا_Actions[i], خالی_اشاریہ);
}

#اگر_ختم // !_WIN32

منطقی CrashRecoveryContext::تسلسل_Safely(function_ref<خالی()> Fn) {
  // If crash recovery is disabled, do nothing.
  اگر (gCrashRecoveryEnabled) {
    أكد(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = نیا CrashRecoveryContextImpl(یہ);
    Impl = CRCI;

    CRCI->Valid_Jump_بفر = سچ;
    اگر (setjmp(CRCI->Jump_بفر) != 0) {
      واپس جھوٹ;
    }
  }

  Fn();
  واپس سچ;
}

#اگر_ختم // !_MSC_VER

[[noreturn]] خالی CrashRecoveryContext::HandleExit(صحیح Ret_رمز) {
#اگر defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_رمز, 0, 0, تہی);
#ورنہ
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  أكد(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_رمز, 0 /*no sig num*/);
#اگر_ختم
  llvm_unreachable("Most likely setjmp wasn't called!");
}

منطقی CrashRecoveryContext::isCrash(صحیح Ret_رمز) {
#اگر defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  غیرعلامتی رمز = ((غیرعلامتی)Ret_رمز & 0xF0000000) >> 28;
  اگر (رمز != 0xC && رمز != 8)
    واپس جھوٹ;
#ورنہ
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  اگر (Ret_رمز <= 128)
    واپس جھوٹ;
#اگر_ختم
  واپس سچ;
}

منطقی CrashRecoveryContext::throwIfCrash(صحیح Ret_رمز) {
  اگر (!isCrash(Ret_رمز))
    واپس جھوٹ;
#اگر defined(_WIN32)
  ::RaiseException(Ret_رمز, 0, 0, تہی);
#ورنہ
  llvm::sys::unregisterHandlers();
  اٹھاؤ(Ret_رمز - 128);
#اگر_ختم
  واپس سچ;
}

// FIXME: Portability.
ساکن خالی سیٹ_Thread_Background_ترجیح() {
#اگر_تعریف_شدہ __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#اگر_ختم
}

ساکن منطقی has_Thread_Background_ترجیح() {
#اگر_تعریف_شدہ __APPLE__
  واپس getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#ورنہ
  واپس جھوٹ;
#اگر_ختم
}

نامخلا {
ساخت تسلسل_Safely_On_Thread_Info {
  function_ref<خالی()> Fn;
  CrashRecoveryContext *چکری_جانچ;
  منطقی Use_Background_ترجیح;
  منطقی نتیجہ;
};
} // namespace

ساکن خالی تسلسل_Safely_On_Thread_Dispatch(خالی *User_ڈیٹا) {
  تسلسل_Safely_On_Thread_Info *Info =
    reinterpret_cast<تسلسل_Safely_On_Thread_Info*>(User_ڈیٹا);

  اگر (Info->Use_Background_ترجیح)
    سیٹ_Thread_Background_ترجیح();

  Info->نتیجہ = Info->چکری_جانچ->تسلسل_Safely(Info->Fn);
}
منطقی CrashRecoveryContext::تسلسل_Safely_On_Thread(function_ref<خالی()> Fn,
                                             غیرعلامتی Requested_ڈھیر_سائز) {
  منطقی Use_Background_ترجیح = has_Thread_Background_ترجیح();
  تسلسل_Safely_On_Thread_Info Info = { Fn, یہ, Use_Background_ترجیح, جھوٹ };
  llvm::thread Thread(Requested_ڈھیر_سائز == 0
                          ? std::nullopt
                          : std::optional<غیرعلامتی>(Requested_ڈھیر_سائز),
                      تسلسل_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  اگر (CrashRecoveryContextImpl *چکری_جانچ = (CrashRecoveryContextImpl *)Impl)
    چکری_جانچ->سیٹ_Switched_Thread();
  واپس Info.نتیجہ;
}

منطقی CrashRecoveryContext::تسلسل_Safely_On_New_ڈھیر(function_ref<خالی()> Fn,
                                               غیرعلامتی Requested_ڈھیر_سائز) {
  واپس تسلسل_Safely_On_Thread(Fn, Requested_ڈھیر_سائز);
}
