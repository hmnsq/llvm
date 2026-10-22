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
#ifdef __APPLE__
#ضمن <sys/resource.h>
#endif

using نطاق llvm;

نطاق {

بنية CrashRecoveryContextImpl;
ساكن LLVM_THREAD_LOCAL ثابت CrashRecoveryContextImpl *CurrentContext;

بنية CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ثابت CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  متطاير unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

عام:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(خطأ), SwitchedThread(خطأ), ValidJumpBuffer(خطأ) {
    Next = CurrentContext;
    CurrentContext = هذا;
  }
  ~CrashRecoveryContextImpl() {
    إذا (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  فراغ setSwitchedThread() {
#إذا defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = true;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  فراغ HandleCrash(صحيح RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    أكد(!Failed && "Crash recovery context already failed!");
    Failed = true;

    إذا (CRC->Dump_ڈھیر_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    إذا (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

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
  CrashRecoveryContextCleanup *i = head;
  ثابت CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = هذا;
  طالما (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = true;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

منطقي CrashRecoveryContext::isRecoveringFromCrash() {
  أرجع IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  إذا (!gCrashRecoveryEnabled)
    أرجع nullptr;

  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;
  إذا (!CRCI)
    أرجع nullptr;

  أرجع CRCI->CRC;
}

فراغ CrashRecoveryContext::Enable(منطقي NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  إذا (gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = true;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

فراغ CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  إذا (!gCrashRecoveryEnabled)
    أرجع;
  gCrashRecoveryEnabled = خطأ;
  uninstallExceptionOrSignalHandlers();
}

فراغ CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  إذا (!cleanup)
    أرجع;
  إذا (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

فراغ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  إذا (!cleanup)
    أرجع;
  إذا (cleanup == head) {
    head = cleanup->next;
    إذا (head)
      head->prev = nullptr;
  }
  وإلا {
    cleanup->prev->next = cleanup->next;
    إذا (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
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
ساكن صحيح ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  ثابت CrashRecoveryContextImpl *CRCI = CurrentContext;

  إذا (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    أرجع EXCEPTION_CONTINUE_تلاش;
  }

  صحيح RetCode = (صحيح)Except->ExceptionRecord->ExceptionCode;
  إذا ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  أرجع EXCEPTION_EXECUTE_HANDLER;
}

#إذا defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
منطقي CrashRecoveryContext::RunSafely(function_ref<فراغ()> Fn) {
  إذا (!gCrashRecoveryEnabled) {
    Fn();
    أرجع true;
  }
  أكد(!Impl && "Crash recovery context already initialized!");
  Impl = جديد CrashRecoveryContextImpl(هذا);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    أرجع خطأ;
  }
  أرجع true;
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

ساكن LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  بدل (ExceptionInfo->ExceptionRecord->ExceptionCode)
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

  صحيح RetCode = (صحيح)ExceptionInfo->ExceptionRecord->ExceptionCode;
  إذا ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(ExceptionInfo));

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
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
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

ساكن ثابت صحيح Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ساكن ثابت unsigned عدد_Signals = std::size(Signals);
ساكن بنية sigaction PrevActions[عدد_Signals];

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
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  صحيح RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  إذا (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  إذا (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

ساكن فراغ
installExceptionOrSignalHandlers(منطقي NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  بنية sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  لكل (unsigned i = 0; i != عدد_Signals; ++i) {
    إذا (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      بنية sigaction act;
      إذا (sigaction(Signals[i], عدم, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } وإلا {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

ساكن فراغ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  لكل (unsigned i = 0; i != عدد_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

منطقي CrashRecoveryContext::RunSafely(function_ref<فراغ()> Fn) {
  // If crash recovery is disabled, do nothing.
  إذا (gCrashRecoveryEnabled) {
    أكد(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = جديد CrashRecoveryContextImpl(هذا);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = true;
    إذا (setjmp(CRCI->JumpBuffer) != 0) {
      أرجع خطأ;
    }
  }

  Fn();
  أرجع true;
}

#endif // !_MSC_VER

[[noreturn]] فراغ CrashRecoveryContext::HandleExit(صحيح RetCode) {
#إذا defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, عدم);
#وإلا
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  أكد(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

منطقي CrashRecoveryContext::isCrash(صحيح RetCode) {
#إذا defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  إذا (Code != 0xC && Code != 8)
    أرجع خطأ;
#وإلا
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  إذا (RetCode <= 128)
    أرجع خطأ;
#endif
  أرجع true;
}

منطقي CrashRecoveryContext::throwIfCrash(صحيح RetCode) {
  إذا (!isCrash(RetCode))
    أرجع خطأ;
#إذا defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, عدم);
#وإلا
  llvm::sys::unregisterHandlers();
  ارفع(RetCode - 128);
#endif
  أرجع true;
}

// FIXME: Portability.
ساكن فراغ setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

ساكن منطقي hasThreadBackgroundPriority() {
#ifdef __APPLE__
  أرجع getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#وإلا
  أرجع خطأ;
#endif
}

نطاق {
بنية RunSafelyOnThreadInfo {
  function_ref<فراغ()> Fn;
  CrashRecoveryContext *CRC;
  منطقي UseBackgroundPriority;
  منطقي Result;
};
} // namespace

ساكن فراغ RunSafelyOnThread_Dispatch(فراغ *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  إذا (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
منطقي CrashRecoveryContext::RunSafelyOnThread(function_ref<فراغ()> Fn,
                                             unsigned Requested_ڈھیر_Size) {
  منطقي UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, هذا, UseBackgroundPriority, خطأ };
  llvm::thread Thread(Requested_ڈھیر_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_ڈھیر_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  إذا (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  أرجع Info.Result;
}

منطقي CrashRecoveryContext::Run_Safely_On_New_ڈھیر(function_ref<فراغ()> Fn,
                                               unsigned Requested_ڈھیر_Size) {
  أرجع RunSafelyOnThread(Fn, Requested_ڈھیر_Size);
}
