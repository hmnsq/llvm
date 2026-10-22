//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/CrashRecoveryContext.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/ExitCodes.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/thread.h"
#ضمن <cassert>
#ضمن <mutex>
#ضمن <setjmp.h>
#جے_تعریف_شدہ __APPLE__
#ضمن <sys/resource.h>
#جے_انت

ورتوں نطاق llvm;

نطاق {

بنية CrashRecoveryContextImpl;
ساكن LLVM_THREAD_LOCAL ثابت CrashRecoveryContextImpl *CurrentContext;

بنية CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ثابت CrashRecoveryContextImpl *اگلا_2;

  CrashRecoveryContext *چکری_جانچ;
  ::jmp_buf Jump_بفر;
  متطاير بنانشانی Failed : 1;
  بنانشانی SwitchedThread : 1;
  بنانشانی Valid_Jump_بفر : 1;

عام:
  CrashRecoveryContextImpl(CrashRecoveryContext *چکری_جانچ) noexcept
      : چکری_جانچ(چکری_جانچ), Failed(خطأ), SwitchedThread(خطأ), Valid_Jump_بفر(خطأ) {
    اگلا_2 = CurrentContext;
    CurrentContext = هذا;
  }
  ~CrashRecoveryContextImpl() {
    إذا (!SwitchedThread)
      CurrentContext = اگلا_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  فراغ سیٹ_Switched_Thread() {
#إذا defined(LLVM_ENABLE_دھاگے) && LLVM_ENABLE_دھاگے != 0
    SwitchedThread = سچ;
#جے_انت
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  فراغ HandleCrash(صحيح Ret_کوڈ, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = اگلا_2;

    أكد(!Failed && "Crash recovery context already failed!");
    Failed = سچ;

    إذا (چکری_جانچ->Dump_ڈھیر_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    چکری_جانچ->Ret_کوڈ = Ret_کوڈ;

    // Jump back to the RunSafely we were called under.
    إذا (Valid_Jump_بفر)
      longjmp(Jump_بفر, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ساكن std::mutex CrashRecoveryContextMutex;
  أرجع CrashRecoveryContextMutex;
}

ساكن منطقي gCrashRecoveryEnabled = خطأ;

ساكن LLVM_THREAD_LOCAL ثابت CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling);
ساكن فراغ uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = افتراضي;

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
  IsRecoveringFromCrash = هذا;
  طالما (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->اگلا;
    tmp->cleanupFired = سچ;
    tmp->recoverResources();
    مٹاؤ tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  مٹاؤ CRCI;
}

منطقي CrashRecoveryContext::isRecoveringFromCrash() {
  أرجع IsRecoveringFromCrash != خالی_سوچک;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  إذا (!gCrashRecoveryEnabled)
    أرجع خالی_سوچک;

  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;
  إذا (!CRCI)
    أرجع خالی_سوچک;

  أرجع CRCI->چکری_جانچ;
}

فراغ CrashRecoveryContext::Enable(منطقي NeedsPOSIXUtilitySignalHandling) {
  std::جندرا_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  إذا (gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = سچ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

فراغ CrashRecoveryContext::Disable() {
  std::جندرا_guard<std::mutex> L(getCrashRecoveryContextMutex());
  إذا (!gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = خطأ;
  uninstallExceptionOrSignalHandlers();
}

فراغ CrashRecoveryContext::رجسٹر_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  إذا (!cleanup)
    أرجع;
  إذا (سر)
    سر->prev = cleanup;
  cleanup->اگلا = سر;
  سر = cleanup;
}

فراغ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  إذا (!cleanup)
    أرجع;
  إذا (cleanup == سر) {
    سر = cleanup->اگلا;
    إذا (سر)
      سر->prev = خالی_سوچک;
  }
  وإلا {
    cleanup->prev->اگلا = cleanup->اگلا;
    إذا (cleanup->اگلا)
      cleanup->اگلا->prev = cleanup->prev;
  }
  مٹاؤ cleanup;
}

#إذا defined(_MSC_VER)

#ضمن <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling) {}
ساكن فراغ uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
ساكن صحيح ExceptionFilter(EXCEPTION_سوچکاں *Except) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  إذا (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    أرجع EXCEPTION_CONTINUE_تلاش;
  }

  صحيح Ret_کوڈ = (صحيح)Except->Exception_ریکارڈ->Exception_کوڈ;
  إذا ((Ret_کوڈ & 0xF0000000) == 0xE0000000)
    Ret_کوڈ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_کوڈ, reinterpret_cast<uintptr_t>(Except));

  أرجع EXCEPTION_EXECUTE_HANDLER;
}

#إذا defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#جے_انت
منطقي CrashRecoveryContext::لگاتار_Safely(function_ref<فراغ()> Fn) {
  إذا (!gCrashRecoveryEnabled) {
    Fn();
    أرجع سچ;
  }
  أكد(!Impl && "Crash recovery context already initialized!");
  Impl = جديد CrashRecoveryContextImpl(هذا);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    أرجع خطأ;
  }
  أرجع سچ;
}

#وإلا // !_MSC_VER

#إذا defined(_WIN32)
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

#ضمن "llvm/Support/Windows/WindowsSupport.h"

ساكن LONG CALLBACK ExceptionHandler(PEXCEPTION_سوچکاں ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  بدل (ExceptionInfo->Exception_ریکارڈ->Exception_کوڈ)
  {
  حالة DBG_PRINTEXCEPTION_C:
  حالة DbgPrintExceptionWideC:
  حالة 0x406D1388:  // set debugger thread name
    أرجع EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  إذا (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    أرجع EXCEPTION_CONTINUE_تلاش;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  صحيح Ret_کوڈ = (صحيح)ExceptionInfo->Exception_ریکارڈ->Exception_کوڈ;
  إذا ((Ret_کوڈ & 0xF0000000) == 0xE0000000)
    Ret_کوڈ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_کوڈ, reinterpret_cast<uintptr_t>(ExceptionInfo));

  // Note that we don't actually get here because HandleCrash calls
  // longjmp, which means the HandleCrash function never returns.
  llvm_unreachable("Handled the crash, should have longjmp'ed out of here");
}

// Because the Enable and Disable calls are static, it means that
// there may not actually be an Impl available, or even a current
// CrashRecoveryContext at all.  So we make use of a thread-local
// exception table.  The handles contained in here will either be
// non-NULL, valid VEH handles, or NULL.
ساكن LLVM_THREAD_LOCAL ثابت فراغ* sCurrentExceptionHandle;

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::جوڑو_Vectored_Exception_Handler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ساكن فراغ uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  إذا (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = عدم;
  }
}

#وإلا // !_WIN32

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

#ضمن <signal.h>

ساكن ثابت صحيح اشارے[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ساكن ثابت بنانشانی عدد_اشارے = std::ناپ_2(اشارے);
ساكن بنية sigaction PrevActions[عدد_اشارے];

ساكن فراغ CrashRecoverySignalHandler(صحيح Signal) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  إذا (!CRCI) {
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
    gCrashRecoveryEnabled = خطأ;
    uninstallExceptionOrSignalHandlers();
    ارفع(Signal);

    // The signal will be thrown once the signal mask is restored.
    أرجع;
  }

  // Unblock the signal we received.
  sigset_t Sig_نقاب;
  sigemptyset(&Sig_نقاب);
  sigaddset(&Sig_نقاب, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_نقاب, خالی_سوچک);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  صحيح Ret_کوڈ = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  إذا (Signal == SIGPIPE)
    Ret_کوڈ = EX_IOERR;

  إذا (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_کوڈ, Signal);
}

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  بنية sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_نقاب);

  لكل (بنانشانی i = 0; i != عدد_اشارے; ++i) {
    إذا (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      بنية sigaction act;
      إذا (sigaction(اشارے[i], عدم, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(اشارے[i], &Handler, &PrevActions[i]);
    } وإلا {
      sigaction(اشارے[i], &Handler, &PrevActions[i]);
    }
  }
}

ساكن فراغ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  لكل (بنانشانی i = 0; i != عدد_اشارے; ++i)
    sigaction(اشارے[i], &PrevActions[i], خالی_سوچک);
}

#جے_انت // !_WIN32

منطقي CrashRecoveryContext::لگاتار_Safely(function_ref<فراغ()> Fn) {
  // If crash recovery is disabled, do nothing.
  إذا (gCrashRecoveryEnabled) {
    أكد(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = جديد CrashRecoveryContextImpl(هذا);
    Impl = CRCI;

    CRCI->Valid_Jump_بفر = سچ;
    إذا (setjmp(CRCI->Jump_بفر) != 0) {
      أرجع خطأ;
    }
  }

  Fn();
  أرجع سچ;
}

#جے_انت // !_MSC_VER

[[noreturn]] فراغ CrashRecoveryContext::HandleExit(صحيح Ret_کوڈ) {
#إذا defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_کوڈ, 0, 0, عدم);
#وإلا
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  أكد(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_کوڈ, 0 /*no sig num*/);
#جے_انت
  llvm_unreachable("Most likely setjmp wasn't called!");
}

منطقي CrashRecoveryContext::isCrash(صحيح Ret_کوڈ) {
#إذا defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  بنانشانی کوڈ = ((بنانشانی)Ret_کوڈ & 0xF0000000) >> 28;
  إذا (کوڈ != 0xC && کوڈ != 8)
    أرجع خطأ;
#وإلا
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  إذا (Ret_کوڈ <= 128)
    أرجع خطأ;
#جے_انت
  أرجع سچ;
}

منطقي CrashRecoveryContext::throwIfCrash(صحيح Ret_کوڈ) {
  إذا (!isCrash(Ret_کوڈ))
    أرجع خطأ;
#إذا defined(_WIN32)
  ::RaiseException(Ret_کوڈ, 0, 0, عدم);
#وإلا
  llvm::sys::unregisterHandlers();
  ارفع(Ret_کوڈ - 128);
#جے_انت
  أرجع سچ;
}

// FIXME: Portability.
ساكن فراغ سیٹ_Thread_Background_ترجیح() {
#جے_تعریف_شدہ __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#جے_انت
}

ساكن منطقي has_Thread_Background_ترجیح() {
#جے_تعریف_شدہ __APPLE__
  أرجع getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#وإلا
  أرجع خطأ;
#جے_انت
}

نطاق {
بنية لگاتار_Safely_On_Thread_Info {
  function_ref<فراغ()> Fn;
  CrashRecoveryContext *چکری_جانچ;
  منطقي Use_Background_ترجیح;
  منطقي نتیجہ;
};
} // namespace

ساكن فراغ لگاتار_Safely_On_Thread_Dispatch(فراغ *User_انکڑے) {
  لگاتار_Safely_On_Thread_Info *Info =
    reinterpret_cast<لگاتار_Safely_On_Thread_Info*>(User_انکڑے);

  إذا (Info->Use_Background_ترجیح)
    سیٹ_Thread_Background_ترجیح();

  Info->نتیجہ = Info->چکری_جانچ->لگاتار_Safely(Info->Fn);
}
منطقي CrashRecoveryContext::لگاتار_Safely_On_Thread(function_ref<فراغ()> Fn,
                                             بنانشانی Requested_ڈھیر_ناپ) {
  منطقي Use_Background_ترجیح = has_Thread_Background_ترجیح();
  لگاتار_Safely_On_Thread_Info Info = { Fn, هذا, Use_Background_ترجیح, خطأ };
  llvm::thread Thread(Requested_ڈھیر_ناپ == 0
                          ? std::nullopt
                          : std::optional<بنانشانی>(Requested_ڈھیر_ناپ),
                      لگاتار_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  إذا (CrashRecoveryContextImpl *چکری_جانچ = (CrashRecoveryContextImpl *)Impl)
    چکری_جانچ->سیٹ_Switched_Thread();
  أرجع Info.نتیجہ;
}

منطقي CrashRecoveryContext::لگاتار_Safely_On_New_ڈھیر(function_ref<فراغ()> Fn,
                                               بنانشانی Requested_ڈھیر_ناپ) {
  أرجع لگاتار_Safely_On_Thread(Fn, Requested_ڈھیر_ناپ);
}
