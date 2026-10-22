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
#اگر_تعریف_شده __APPLE__
#ضمن <sys/resource.h>
#پایان_اگر

استفاده نطاق llvm;

نطاق {

بنية CrashRecoveryContextImpl;
ساكن LLVM_THREAD_LOCAL ثابت CrashRecoveryContextImpl *CurrentContext;

بنية CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ثابت CrashRecoveryContextImpl *بعدی_2;

  CrashRecoveryContext *بررسی_چرخهای;
  ::jmp_buf Jump_میانگیر;
  متطاير بیعلامت Failed : 1;
  بیعلامت SwitchedThread : 1;
  بیعلامت Valid_Jump_میانگیر : 1;

عام:
  CrashRecoveryContextImpl(CrashRecoveryContext *بررسی_چرخهای) noexcept
      : بررسی_چرخهای(بررسی_چرخهای), Failed(خطأ), SwitchedThread(خطأ), Valid_Jump_میانگیر(خطأ) {
    بعدی_2 = CurrentContext;
    CurrentContext = هذا;
  }
  ~CrashRecoveryContextImpl() {
    إذا (!SwitchedThread)
      CurrentContext = بعدی_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  فراغ مجموعه_Switched_Thread() {
#إذا defined(LLVM_ENABLE_نخها) && LLVM_ENABLE_نخها != 0
    SwitchedThread = درست;
#پایان_اگر
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  فراغ HandleCrash(صحيح Ret_کد, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = بعدی_2;

    أكد(!Failed && "Crash recovery context already failed!");
    Failed = درست;

    إذا (بررسی_چرخهای->Dump_پشته_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    بررسی_چرخهای->Ret_کد = Ret_کد;

    // Jump back to the RunSafely we were called under.
    إذا (Valid_Jump_میانگیر)
      longjmp(Jump_میانگیر, 1);

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
    i = tmp->بعدی;
    tmp->cleanupFired = درست;
    tmp->recoverResources();
    حذف tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  حذف CRCI;
}

منطقي CrashRecoveryContext::isRecoveringFromCrash() {
  أرجع IsRecoveringFromCrash != اشاره_تهی;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  إذا (!gCrashRecoveryEnabled)
    أرجع اشاره_تهی;

  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;
  إذا (!CRCI)
    أرجع اشاره_تهی;

  أرجع CRCI->بررسی_چرخهای;
}

فراغ CrashRecoveryContext::Enable(منطقي NeedsPOSIXUtilitySignalHandling) {
  std::قفل_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  إذا (gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = درست;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

فراغ CrashRecoveryContext::Disable() {
  std::قفل_guard<std::mutex> L(getCrashRecoveryContextMutex());
  إذا (!gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = خطأ;
  uninstallExceptionOrSignalHandlers();
}

فراغ CrashRecoveryContext::ثبات_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  إذا (!cleanup)
    أرجع;
  إذا (سر)
    سر->prev = cleanup;
  cleanup->بعدی = سر;
  سر = cleanup;
}

فراغ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  إذا (!cleanup)
    أرجع;
  إذا (cleanup == سر) {
    سر = cleanup->بعدی;
    إذا (سر)
      سر->prev = اشاره_تهی;
  }
  وإلا {
    cleanup->prev->بعدی = cleanup->بعدی;
    إذا (cleanup->بعدی)
      cleanup->بعدی->prev = cleanup->prev;
  }
  حذف cleanup;
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
ساكن صحيح ExceptionFilter(EXCEPTION_اشارهگرها *Except) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  إذا (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    أرجع EXCEPTION_CONTINUE_جستجو;
  }

  صحيح Ret_کد = (صحيح)Except->Exception_رکورد->Exception_کد;
  إذا ((Ret_کد & 0xF0000000) == 0xE0000000)
    Ret_کد &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_کد, reinterpret_cast<uintptr_t>(Except));

  أرجع EXCEPTION_EXECUTE_HANDLER;
}

#إذا defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#پایان_اگر
منطقي CrashRecoveryContext::پیاپی_Safely(function_ref<فراغ()> Fn) {
  إذا (!gCrashRecoveryEnabled) {
    Fn();
    أرجع درست;
  }
  أكد(!Impl && "Crash recovery context already initialized!");
  Impl = جديد CrashRecoveryContextImpl(هذا);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    أرجع خطأ;
  }
  أرجع درست;
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

ساكن LONG CALLBACK ExceptionHandler(PEXCEPTION_اشارهگرها ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  بدل (ExceptionInfo->Exception_رکورد->Exception_کد)
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
    أرجع EXCEPTION_CONTINUE_جستجو;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  صحيح Ret_کد = (صحيح)ExceptionInfo->Exception_رکورد->Exception_کد;
  إذا ((Ret_کد & 0xF0000000) == 0xE0000000)
    Ret_کد &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_کد, reinterpret_cast<uintptr_t>(ExceptionInfo));

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
  PVOID handle = ::جمع_کن_Vectored_Exception_Handler(1, ExceptionHandler);
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

ساكن ثابت صحيح سیگنالها[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ساكن ثابت بیعلامت رقم_سیگنالها = std::اندازه_2(سیگنالها);
ساكن بنية sigaction PrevActions[رقم_سیگنالها];

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
  sigset_t Sig_ماسک;
  sigemptyset(&Sig_ماسک);
  sigaddset(&Sig_ماسک, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_ماسک, اشاره_تهی);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  صحيح Ret_کد = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  إذا (Signal == SIGPIPE)
    Ret_کد = EX_IOERR;

  إذا (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_کد, Signal);
}

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  بنية sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_ماسک);

  لكل (بیعلامت i = 0; i != رقم_سیگنالها; ++i) {
    إذا (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      بنية sigaction act;
      إذا (sigaction(سیگنالها[i], عدم, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(سیگنالها[i], &Handler, &PrevActions[i]);
    } وإلا {
      sigaction(سیگنالها[i], &Handler, &PrevActions[i]);
    }
  }
}

ساكن فراغ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  لكل (بیعلامت i = 0; i != رقم_سیگنالها; ++i)
    sigaction(سیگنالها[i], &PrevActions[i], اشاره_تهی);
}

#پایان_اگر // !_WIN32

منطقي CrashRecoveryContext::پیاپی_Safely(function_ref<فراغ()> Fn) {
  // If crash recovery is disabled, do nothing.
  إذا (gCrashRecoveryEnabled) {
    أكد(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = جديد CrashRecoveryContextImpl(هذا);
    Impl = CRCI;

    CRCI->Valid_Jump_میانگیر = درست;
    إذا (setjmp(CRCI->Jump_میانگیر) != 0) {
      أرجع خطأ;
    }
  }

  Fn();
  أرجع درست;
}

#پایان_اگر // !_MSC_VER

[[noreturn]] فراغ CrashRecoveryContext::HandleExit(صحيح Ret_کد) {
#إذا defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_کد, 0, 0, عدم);
#وإلا
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  أكد(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_کد, 0 /*no sig num*/);
#پایان_اگر
  llvm_unreachable("Most likely setjmp wasn't called!");
}

منطقي CrashRecoveryContext::isCrash(صحيح Ret_کد) {
#إذا defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  بیعلامت کد = ((بیعلامت)Ret_کد & 0xF0000000) >> 28;
  إذا (کد != 0xC && کد != 8)
    أرجع خطأ;
#وإلا
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  إذا (Ret_کد <= 128)
    أرجع خطأ;
#پایان_اگر
  أرجع درست;
}

منطقي CrashRecoveryContext::throwIfCrash(صحيح Ret_کد) {
  إذا (!isCrash(Ret_کد))
    أرجع خطأ;
#إذا defined(_WIN32)
  ::RaiseException(Ret_کد, 0, 0, عدم);
#وإلا
  llvm::sys::unregisterHandlers();
  ارفع(Ret_کد - 128);
#پایان_اگر
  أرجع درست;
}

// FIXME: Portability.
ساكن فراغ مجموعه_Thread_Background_اولویت() {
#اگر_تعریف_شده __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#پایان_اگر
}

ساكن منطقي has_Thread_Background_اولویت() {
#اگر_تعریف_شده __APPLE__
  أرجع getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#وإلا
  أرجع خطأ;
#پایان_اگر
}

نطاق {
بنية پیاپی_Safely_On_Thread_Info {
  function_ref<فراغ()> Fn;
  CrashRecoveryContext *بررسی_چرخهای;
  منطقي Use_Background_اولویت;
  منطقي نتیجه;
};
} // namespace

ساكن فراغ پیاپی_Safely_On_Thread_Dispatch(فراغ *User_داده) {
  پیاپی_Safely_On_Thread_Info *Info =
    reinterpret_cast<پیاپی_Safely_On_Thread_Info*>(User_داده);

  إذا (Info->Use_Background_اولویت)
    مجموعه_Thread_Background_اولویت();

  Info->نتیجه = Info->بررسی_چرخهای->پیاپی_Safely(Info->Fn);
}
منطقي CrashRecoveryContext::پیاپی_Safely_On_Thread(function_ref<فراغ()> Fn,
                                             بیعلامت Requested_پشته_اندازه) {
  منطقي Use_Background_اولویت = has_Thread_Background_اولویت();
  پیاپی_Safely_On_Thread_Info Info = { Fn, هذا, Use_Background_اولویت, خطأ };
  llvm::thread Thread(Requested_پشته_اندازه == 0
                          ? std::nullopt
                          : std::optional<بیعلامت>(Requested_پشته_اندازه),
                      پیاپی_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  إذا (CrashRecoveryContextImpl *بررسی_چرخهای = (CrashRecoveryContextImpl *)Impl)
    بررسی_چرخهای->مجموعه_Switched_Thread();
  أرجع Info.نتیجه;
}

منطقي CrashRecoveryContext::پیاپی_Safely_On_New_پشته(function_ref<فراغ()> Fn,
                                               بیعلامت Requested_پشته_اندازه) {
  أرجع پیاپی_Safely_On_Thread(Fn, Requested_پشته_اندازه);
}
