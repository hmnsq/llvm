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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

ರಚನೆ CrashRecoveryContextImpl;
ಸ್ಥಿರ LLVM_THREAD_LOCAL ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *CurrentContext;

ರಚನೆ CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *ಮುಂದಿನ_2;

  CrashRecoveryContext *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
  ::jmp_buf Jump_ಬಫರ್;
  ಅಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ Failed : 1;
  ಚಿಹ್ನೆರಹಿತ SwitchedThread : 1;
  ಚಿಹ್ನೆರಹಿತ Valid_Jump_ಬಫರ್ : 1;

ಸಾರ್ವಜನಿಕ:
  CrashRecoveryContextImpl(CrashRecoveryContext *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) noexcept
      : ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ), Failed(ಸುಳ್ಳು), SwitchedThread(ಸುಳ್ಳು), Valid_Jump_ಬಫರ್(ಸುಳ್ಳು) {
    ಮುಂದಿನ_2 = CurrentContext;
    CurrentContext = ಇದು;
  }
  ~CrashRecoveryContextImpl() {
    ಆದರೆ (!SwitchedThread)
      CurrentContext = ಮುಂದಿನ_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  ಶೂನ್ಯ ಗಣ_Switched_Thread() {
#ಆದರೆ defined(LLVM_ENABLE_ಎಳೆಗಳು) && LLVM_ENABLE_ಎಳೆಗಳು != 0
    SwitchedThread = ನಿಜ;
#ಆದರೆ_ಅಂತ್ಯ
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  ಶೂನ್ಯ HandleCrash(ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = ಮುಂದಿನ_2;

    ಪ್ರತಿಜ್ಞಾ(!Failed && "Crash recovery context already failed!");
    Failed = ನಿಜ;

    ಆದರೆ (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ->Dump_ರಾಶಿ_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ->Ret_ಸಂಕೇತ = Ret_ಸಂಕೇತ;

    // Jump back to the RunSafely we were called under.
    ಆದರೆ (Valid_Jump_ಬಫರ್)
      longjmp(Jump_ಬಫರ್, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ಸ್ಥಿರ std::mutex CrashRecoveryContextMutex;
  ಹಿಂತಿರುಗಿ CrashRecoveryContextMutex;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ gCrashRecoveryEnabled = ಸುಳ್ಳು;

ಸ್ಥಿರ LLVM_THREAD_LOCAL ಸ್ಥಿರಾಂಕ CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling);
ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = ಪೂರ್ವನಿಯೋಜಿತ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = ತಲೆ;
  ಸ್ಥಿರಾಂಕ CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ಇದು;
  ಆಗ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->ಮುಂದಿನ;
    tmp->cleanupFired = ನಿಜ;
    tmp->recoverResources();
    ಅಳಿಸು tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  ಅಳಿಸು CRCI;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::isRecoveringFromCrash() {
  ಹಿಂತಿರುಗಿ IsRecoveringFromCrash != ಶೂನ್ಯಸೂಚಕ;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ಆದರೆ (!gCrashRecoveryEnabled)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *CRCI = CurrentContext;
  ಆದರೆ (!CRCI)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಹಿಂತಿರುಗಿ CRCI->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
}

ಶೂನ್ಯ CrashRecoveryContext::Enable(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  std::ಬೀಗ_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ಆದರೆ (gCrashRecoveryEnabled)
    ಹಿಂತಿರುಗಿ;
  gCrashRecoveryEnabled = ನಿಜ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

ಶೂನ್ಯ CrashRecoveryContext::Disable() {
  std::ಬೀಗ_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ಆದರೆ (!gCrashRecoveryEnabled)
    ಹಿಂತಿರುಗಿ;
  gCrashRecoveryEnabled = ಸುಳ್ಳು;
  uninstallExceptionOrSignalHandlers();
}

ಶೂನ್ಯ CrashRecoveryContext::ರಿಜಿಸ್ಟರ್_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  ಆದರೆ (!cleanup)
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (ತಲೆ)
    ತಲೆ->prev = cleanup;
  cleanup->ಮುಂದಿನ = ತಲೆ;
  ತಲೆ = cleanup;
}

ಶೂನ್ಯ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ಆದರೆ (!cleanup)
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (cleanup == ತಲೆ) {
    ತಲೆ = cleanup->ಮುಂದಿನ;
    ಆದರೆ (ತಲೆ)
      ತಲೆ->prev = ಶೂನ್ಯಸೂಚಕ;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    cleanup->prev->ಮುಂದಿನ = cleanup->ಮುಂದಿನ;
    ಆದರೆ (cleanup->ಮುಂದಿನ)
      cleanup->ಮುಂದಿನ->prev = cleanup->prev;
  }
  ಅಳಿಸು cleanup;
}

#ಆದರೆ defined(_MSC_VER)

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ExceptionFilter(EXCEPTION_ಸೂಚಕಗಳು *Except) {
  // Lookup the current thread local recovery object.
  ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಆದರೆ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ಹಿಂತಿರುಗಿ EXCEPTION_CONTINUE_ಹುಡುಕು;
  }

  ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ = (ಪೂರ್ಣಾಂಕ)Except->Exception_ದಾಖಲೆ->Exception_ಸಂಕೇತ;
  ಆದರೆ ((Ret_ಸಂಕೇತ & 0xF0000000) == 0xE0000000)
    Ret_ಸಂಕೇತ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_ಸಂಕೇತ, reinterpret_cast<uintptr_t>(Except));

  ಹಿಂತಿರುಗಿ EXCEPTION_EXECUTE_HANDLER;
}

#ಆದರೆ defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#ಆದರೆ_ಅಂತ್ಯ
ತಾರ್ಕಿಕ CrashRecoveryContext::ಸತತ_Safely(function_ref<ಶೂನ್ಯ()> Fn) {
  ಆದರೆ (!gCrashRecoveryEnabled) {
    Fn();
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಪ್ರತಿಜ್ಞಾ(!Impl && "Crash recovery context already initialized!");
  Impl = ಹೊಸ CrashRecoveryContextImpl(ಇದು);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

#ಇಲ್ಲದಿದ್ದರೆ // !_MSC_VER

#ಆದರೆ defined(_WIN32)
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

ಸ್ಥಿರ LONG CALLBACK ExceptionHandler(PEXCEPTION_ಸೂಚಕಗಳು ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  ಆಯ್ಕೆ (ExceptionInfo->Exception_ದಾಖಲೆ->Exception_ಸಂಕೇತ)
  {
  ಸ್ಥಿತಿ DBG_PRINTEXCEPTION_C:
  ಸ್ಥಿತಿ DbgPrintExceptionWideC:
  ಸ್ಥಿತಿ 0x406D1388:  // set debugger thread name
    ಹಿಂತಿರುಗಿ EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಆದರೆ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ಹಿಂತಿರುಗಿ EXCEPTION_CONTINUE_ಹುಡುಕು;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ = (ಪೂರ್ಣಾಂಕ)ExceptionInfo->Exception_ದಾಖಲೆ->Exception_ಸಂಕೇತ;
  ಆದರೆ ((Ret_ಸಂಕೇತ & 0xF0000000) == 0xE0000000)
    Ret_ಸಂಕೇತ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_ಸಂಕೇತ, reinterpret_cast<uintptr_t>(ExceptionInfo));

  // Note that we don't actually get here because HandleCrash calls
  // longjmp, which means the HandleCrash function never returns.
  llvm_unreachable("Handled the crash, should have longjmp'ed out of here");
}

// Because the Enable and Disable calls are static, it means that
// there may not actually be an Impl available, or even a current
// CrashRecoveryContext at all.  So we make use of a thread-local
// exception table.  The handles contained in here will either be
// non-NULL, valid VEH handles, or NULL.
ಸ್ಥಿರ LLVM_THREAD_LOCAL ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* sCurrentExceptionHandle;

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::ಕೂಡಿಸು_Vectored_Exception_Handler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ಆದರೆ (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ಖಾಲಿ;
  }
}

#ಇಲ್ಲದಿದ್ದರೆ // !_WIN32

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

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಸಂಕೇತಗಳು[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಸಂಕೇತಗಳು = std::ಗಾತ್ರ_3(ಸಂಕೇತಗಳು);
ಸ್ಥಿರ ರಚನೆ sigaction PrevActions[ಅಂಕೆ_ಸಂಕೇತಗಳು];

ಸ್ಥಿರ ಶೂನ್ಯ CrashRecoverySignalHandler(ಪೂರ್ಣಾಂಕ Signal) {
  // Lookup the current thread local recovery object.
  ಸ್ಥಿರಾಂಕ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ಆದರೆ (!CRCI) {
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
    gCrashRecoveryEnabled = ಸುಳ್ಳು;
    uninstallExceptionOrSignalHandlers();
    ಎತ್ತು(Signal);

    // The signal will be thrown once the signal mask is restored.
    ಹಿಂತಿರುಗಿ;
  }

  // Unblock the signal we received.
  sigset_t Sig_ಮುಖವಾಡ;
  sigemptyset(&Sig_ಮುಖವಾಡ);
  sigaddset(&Sig_ಮುಖವಾಡ, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_ಮುಖವಾಡ, ಶೂನ್ಯಸೂಚಕ);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ಆದರೆ (Signal == SIGPIPE)
    Ret_ಸಂಕೇತ = EX_IOERR;

  ಆದರೆ (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_ಸಂಕೇತ, Signal);
}

ಸ್ಥಿರ ಶೂನ್ಯ
installExceptionOrSignalHandlers(ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ರಚನೆ sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_ಮುಖವಾಡ);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಅಂಕೆ_ಸಂಕೇತಗಳು; ++i) {
    ಆದರೆ (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ರಚನೆ sigaction act;
      ಆದರೆ (sigaction(ಸಂಕೇತಗಳು[i], ಖಾಲಿ, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(ಸಂಕೇತಗಳು[i], &Handler, &PrevActions[i]);
    } ಇಲ್ಲದಿದ್ದರೆ {
      sigaction(ಸಂಕೇತಗಳು[i], &Handler, &PrevActions[i]);
    }
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಅಂಕೆ_ಸಂಕೇತಗಳು; ++i)
    sigaction(ಸಂಕೇತಗಳು[i], &PrevActions[i], ಶೂನ್ಯಸೂಚಕ);
}

#ಆದರೆ_ಅಂತ್ಯ // !_WIN32

ತಾರ್ಕಿಕ CrashRecoveryContext::ಸತತ_Safely(function_ref<ಶೂನ್ಯ()> Fn) {
  // If crash recovery is disabled, do nothing.
  ಆದರೆ (gCrashRecoveryEnabled) {
    ಪ್ರತಿಜ್ಞಾ(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = ಹೊಸ CrashRecoveryContextImpl(ಇದು);
    Impl = CRCI;

    CRCI->Valid_Jump_ಬಫರ್ = ನಿಜ;
    ಆದರೆ (setjmp(CRCI->Jump_ಬಫರ್) != 0) {
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  }

  Fn();
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

#ಆದರೆ_ಅಂತ್ಯ // !_MSC_VER

[[noreturn]] ಶೂನ್ಯ CrashRecoveryContext::HandleExit(ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ) {
#ಆದರೆ defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_ಸಂಕೇತ, 0, 0, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ಪ್ರತಿಜ್ಞಾ(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_ಸಂಕೇತ, 0 /*no sig num*/);
#ಆದರೆ_ಅಂತ್ಯ
  llvm_unreachable("Most likely setjmp wasn't called!");
}

ತಾರ್ಕಿಕ CrashRecoveryContext::isCrash(ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ) {
#ಆದರೆ defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  ಚಿಹ್ನೆರಹಿತ ಸಂಕೇತ_2 = ((ಚಿಹ್ನೆರಹಿತ)Ret_ಸಂಕೇತ & 0xF0000000) >> 28;
  ಆದರೆ (ಸಂಕೇತ_2 != 0xC && ಸಂಕೇತ_2 != 8)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಇಲ್ಲದಿದ್ದರೆ
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ಆದರೆ (Ret_ಸಂಕೇತ <= 128)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::throwIfCrash(ಪೂರ್ಣಾಂಕ Ret_ಸಂಕೇತ) {
  ಆದರೆ (!isCrash(Ret_ಸಂಕೇತ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಆದರೆ defined(_WIN32)
  ::RaiseException(Ret_ಸಂಕೇತ, 0, 0, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
  llvm::sys::unregisterHandlers();
  ಎತ್ತು(Ret_ಸಂಕೇತ - 128);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

// FIXME: Portability.
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Thread_Background_ಆದ್ಯತೆ() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ has_Thread_Background_ಆದ್ಯತೆ() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  ಹಿಂತಿರುಗಿ getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ
}

ನಾಮಾಕಾಶ {
ರಚನೆ ಸತತ_Safely_On_Thread_Info {
  function_ref<ಶೂನ್ಯ()> Fn;
  CrashRecoveryContext *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
  ತಾರ್ಕಿಕ Use_Background_ಆದ್ಯತೆ;
  ತಾರ್ಕಿಕ ಫಲಿತಾಂಶ;
};
} // namespace

ಸ್ಥಿರ ಶೂನ್ಯ ಸತತ_Safely_On_Thread_Dispatch(ಶೂನ್ಯ *User_ದತ್ತಾಂಶ) {
  ಸತತ_Safely_On_Thread_Info *Info =
    reinterpret_cast<ಸತತ_Safely_On_Thread_Info*>(User_ದತ್ತಾಂಶ);

  ಆದರೆ (Info->Use_Background_ಆದ್ಯತೆ)
    ಗಣ_Thread_Background_ಆದ್ಯತೆ();

  Info->ಫಲಿತಾಂಶ = Info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ->ಸತತ_Safely(Info->Fn);
}
ತಾರ್ಕಿಕ CrashRecoveryContext::ಸತತ_Safely_On_Thread(function_ref<ಶೂನ್ಯ()> Fn,
                                             ಚಿಹ್ನೆರಹಿತ Requested_ರಾಶಿ_ಗಾತ್ರ) {
  ತಾರ್ಕಿಕ Use_Background_ಆದ್ಯತೆ = has_Thread_Background_ಆದ್ಯತೆ();
  ಸತತ_Safely_On_Thread_Info Info = { Fn, ಇದು, Use_Background_ಆದ್ಯತೆ, ಸುಳ್ಳು };
  llvm::thread Thread(Requested_ರಾಶಿ_ಗಾತ್ರ == 0
                          ? std::nullopt
                          : std::optional<ಚಿಹ್ನೆರಹಿತ>(Requested_ರಾಶಿ_ಗಾತ್ರ),
                      ಸತತ_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  ಆದರೆ (CrashRecoveryContextImpl *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (CrashRecoveryContextImpl *)Impl)
    ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ->ಗಣ_Switched_Thread();
  ಹಿಂತಿರುಗಿ Info.ಫಲಿತಾಂಶ;
}

ತಾರ್ಕಿಕ CrashRecoveryContext::ಸತತ_Safely_On_New_ರಾಶಿ(function_ref<ಶೂನ್ಯ()> Fn,
                                               ಚಿಹ್ನೆರಹಿತ Requested_ರಾಶಿ_ಗಾತ್ರ) {
  ಹಿಂತಿರುಗಿ ಸತತ_Safely_On_Thread(Fn, Requested_ರಾಶಿ_ಗಾತ್ರ);
}
