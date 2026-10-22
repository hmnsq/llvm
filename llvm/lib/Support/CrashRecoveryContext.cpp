//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/CrashRecoveryContext.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ExitCodes.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/thread.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <setjmp.h>
#ifdef __APPLE__
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#endif

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

ಸಂರಚನಾ CrashRecoveryContextImpl;
ಸ್ಥಿರ LLVM_THREAD_LOCAL ಧ್ರುವ CrashRecoveryContextImpl *CurrentContext;

ಸಂರಚನಾ CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ಧ್ರುವ CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  ಚಞ್ಚಲ unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

ಸಾರ್ವಜನಿಕ:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(ಅಸತ್ಯ), SwitchedThread(ಅಸತ್ಯ), ValidJumpBuffer(ಅಸತ್ಯ) {
    Next = CurrentContext;
    CurrentContext = ಏತತ್;
  }
  ~CrashRecoveryContextImpl() {
    ಯದಿ (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  ಶೂನ್ಯ setSwitchedThread() {
#ಯದಿ defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = ಸತ್ಯ;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  ಶೂನ್ಯ HandleCrash(ಪೂರ್ಣಾಙ್ಕ RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    ಪ್ರತಿಜ್ಞಾ(!Failed && "Crash recovery context already failed!");
    Failed = ಸತ್ಯ;

    ಯದಿ (CRC->Dump_ರಾಶಿ_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    ಯದಿ (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ಸ್ಥಿರ std::mutex CrashRecoveryContextMutex;
  ಪ್ರತ್ಯಾಗಮ CrashRecoveryContextMutex;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ gCrashRecoveryEnabled = ಅಸತ್ಯ;

ಸ್ಥಿರ LLVM_THREAD_LOCAL ಧ್ರುವ CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling);
ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = ಸ್ವಭಾವ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  ಧ್ರುವ CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ಏತತ್;
  ಯಾವತ್ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = ಸತ್ಯ;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::isRecoveringFromCrash() {
  ಪ್ರತ್ಯಾಗಮ IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ಯದಿ (!gCrashRecoveryEnabled)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಧ್ರುವ CrashRecoveryContextImpl *CRCI = CurrentContext;
  ಯದಿ (!CRCI)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಪ್ರತ್ಯಾಗಮ CRCI->CRC;
}

ಶೂನ್ಯ CrashRecoveryContext::Enable(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ಯದಿ (gCrashRecoveryEnabled)
    ಪ್ರತ್ಯಾಗಮ;
  gCrashRecoveryEnabled = ಸತ್ಯ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

ಶೂನ್ಯ CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ಯದಿ (!gCrashRecoveryEnabled)
    ಪ್ರತ್ಯಾಗಮ;
  gCrashRecoveryEnabled = ಅಸತ್ಯ;
  uninstallExceptionOrSignalHandlers();
}

ಶೂನ್ಯ CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  ಯದಿ (!cleanup)
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

ಶೂನ್ಯ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ಯದಿ (!cleanup)
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (cleanup == head) {
    head = cleanup->next;
    ಯದಿ (head)
      head->prev = nullptr;
  }
  ಅನ್ಯಥಾ {
    cleanup->prev->next = cleanup->next;
    ಯದಿ (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#ಯದಿ defined(_MSC_VER)

#ಅನ್ತರ್ಭಾವ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {}
ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  ಧ್ರುವ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಯದಿ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ಪ್ರತ್ಯಾಗಮ EXCEPTION_CONTINUE_ಹುಡುಕು;
  }

  ಪೂರ್ಣಾಙ್ಕ RetCode = (ಪೂರ್ಣಾಙ್ಕ)Except->ExceptionRecord->ExceptionCode;
  ಯದಿ ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  ಪ್ರತ್ಯಾಗಮ EXCEPTION_EXECUTE_HANDLER;
}

#ಯದಿ defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
ತಾರ್ಕಿಕ CrashRecoveryContext::RunSafely(function_ref<ಶೂನ್ಯ()> Fn) {
  ಯದಿ (!gCrashRecoveryEnabled) {
    Fn();
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತಿಜ್ಞಾ(!Impl && "Crash recovery context already initialized!");
  Impl = ನೂತನ CrashRecoveryContextImpl(ಏತತ್);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

#ಅನ್ಯಥಾ // !_MSC_VER

#ಯದಿ defined(_WIN32)
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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"

ಸ್ಥಿರ LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  ವಿಕಲ್ಪ (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  ಪಕ್ಷ DBG_PRINTEXCEPTION_C:
  ಪಕ್ಷ DbgPrintExceptionWideC:
  ಪಕ್ಷ 0x406D1388:  // set debugger thread name
    ಪ್ರತ್ಯಾಗಮ EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ಧ್ರುವ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಯದಿ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ಪ್ರತ್ಯಾಗಮ EXCEPTION_CONTINUE_ಹುಡುಕು;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  ಪೂರ್ಣಾಙ್ಕ RetCode = (ಪೂರ್ಣಾಙ್ಕ)ExceptionInfo->ExceptionRecord->ExceptionCode;
  ಯದಿ ((RetCode & 0xF0000000) == 0xE0000000)
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
ಸ್ಥಿರ LLVM_THREAD_LOCAL ಧ್ರುವ ಶೂನ್ಯ* sCurrentExceptionHandle;

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ಯದಿ (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ರಿಕ್ತ;
  }
}

#ಅನ್ಯಥಾ // !_WIN32

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

#ಅನ್ತರ್ಭಾವ <signal.h>

ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ಸ್ಥಿರ ಧ್ರುವ unsigned ಅಂಕೆ_Signals = std::size(Signals);
ಸ್ಥಿರ ಸಂರಚನಾ sigaction PrevActions[ಅಂಕೆ_Signals];

ಸ್ಥಿರ ಶೂನ್ಯ CrashRecoverySignalHandler(ಪೂರ್ಣಾಙ್ಕ Signal) {
  // Lookup the current thread local recovery object.
  ಧ್ರುವ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಯದಿ (!CRCI) {
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
    gCrashRecoveryEnabled = ಅಸತ್ಯ;
    uninstallExceptionOrSignalHandlers();
    ಉತ್ಥಾಪಯ(Signal);

    // The signal will be thrown once the signal mask is restored.
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  ಪೂರ್ಣಾಙ್ಕ RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ಯದಿ (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  ಯದಿ (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ಸಂರಚನಾ sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  ಪ್ರತಿ (unsigned i = 0; i != ಅಂಕೆ_Signals; ++i) {
    ಯದಿ (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ಸಂರಚನಾ sigaction act;
      ಯದಿ (sigaction(Signals[i], ರಿಕ್ತ, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } ಅನ್ಯಥಾ {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  ಪ್ರತಿ (unsigned i = 0; i != ಅಂಕೆ_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

ತಾರ್ಕಿಕ CrashRecoveryContext::RunSafely(function_ref<ಶೂನ್ಯ()> Fn) {
  // If crash recovery is disabled, do nothing.
  ಯದಿ (gCrashRecoveryEnabled) {
    ಪ್ರತಿಜ್ಞಾ(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = ನೂತನ CrashRecoveryContextImpl(ಏತತ್);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = ಸತ್ಯ;
    ಯದಿ (setjmp(CRCI->JumpBuffer) != 0) {
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  }

  Fn();
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

#endif // !_MSC_VER

[[noreturn]] ಶೂನ್ಯ CrashRecoveryContext::HandleExit(ಪೂರ್ಣಾಙ್ಕ RetCode) {
#ಯದಿ defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, ರಿಕ್ತ);
#ಅನ್ಯಥಾ
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ಪ್ರತಿಜ್ಞಾ(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

ತಾರ್ಕಿಕ CrashRecoveryContext::isCrash(ಪೂರ್ಣಾಙ್ಕ RetCode) {
#ಯದಿ defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  ಯದಿ (Code != 0xC && Code != 8)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#ಅನ್ಯಥಾ
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ಯದಿ (RetCode <= 128)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#endif
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::throwIfCrash(ಪೂರ್ಣಾಙ್ಕ RetCode) {
  ಯದಿ (!isCrash(RetCode))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#ಯದಿ defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, ರಿಕ್ತ);
#ಅನ್ಯಥಾ
  llvm::sys::unregisterHandlers();
  ಉತ್ಥಾಪಯ(RetCode - 128);
#endif
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

// FIXME: Portability.
ಸ್ಥಿರ ಶೂನ್ಯ setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ hasThreadBackgroundPriority() {
#ifdef __APPLE__
  ಪ್ರತ್ಯಾಗಮ getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#endif
}

ನಾಮಾಕಾಶ {
ಸಂರಚನಾ RunSafelyOnThreadInfo {
  function_ref<ಶೂನ್ಯ()> Fn;
  CrashRecoveryContext *CRC;
  ತಾರ್ಕಿಕ UseBackgroundPriority;
  ತಾರ್ಕಿಕ Result;
};
} // namespace

ಸ್ಥಿರ ಶೂನ್ಯ RunSafelyOnThread_Dispatch(ಶೂನ್ಯ *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  ಯದಿ (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
ತಾರ್ಕಿಕ CrashRecoveryContext::RunSafelyOnThread(function_ref<ಶೂನ್ಯ()> Fn,
                                             unsigned Requested_ರಾಶಿ_Size) {
  ತಾರ್ಕಿಕ UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, ಏತತ್, UseBackgroundPriority, ಅಸತ್ಯ };
  llvm::thread Thread(Requested_ರಾಶಿ_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_ರಾಶಿ_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  ಯದಿ (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  ಪ್ರತ್ಯಾಗಮ Info.Result;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::Run_Safely_On_New_ರಾಶಿ(function_ref<ಶೂನ್ಯ()> Fn,
                                               unsigned Requested_ರಾಶಿ_Size) {
  ಪ್ರತ್ಯಾಗಮ RunSafelyOnThread(Fn, Requested_ರಾಶಿ_Size);
}
