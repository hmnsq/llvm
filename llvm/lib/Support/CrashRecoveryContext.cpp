//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CrashRecoveryContext.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ExitCodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/thread.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/resource.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {

ਬਣਤਰ CrashRecoveryContextImpl;
ਸਥਿਰ LLVM_THREAD_LOCAL ਅਚਲ CrashRecoveryContextImpl *CurrentContext;

ਬਣਤਰ CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ਅਚਲ CrashRecoveryContextImpl *ਅਗਲਾ_2;

  CrashRecoveryContext *ਚੱਕਰੀ_ਜਾਂਚ;
  ::jmp_buf Jump_ਬਫਰ;
  ਅਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ Failed : 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ SwitchedThread : 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Valid_Jump_ਬਫਰ : 1;

ਜਨਤਕ:
  CrashRecoveryContextImpl(CrashRecoveryContext *ਚੱਕਰੀ_ਜਾਂਚ) noexcept
      : ਚੱਕਰੀ_ਜਾਂਚ(ਚੱਕਰੀ_ਜਾਂਚ), Failed(ਝੂਠ), SwitchedThread(ਝੂਠ), Valid_Jump_ਬਫਰ(ਝੂਠ) {
    ਅਗਲਾ_2 = CurrentContext;
    CurrentContext = ਇਹ;
  }
  ~CrashRecoveryContextImpl() {
    ਜੇ (!SwitchedThread)
      CurrentContext = ਅਗਲਾ_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  ਖਾਲੀ ਸੈੱਟ_Switched_Thread() {
#ਜੇ defined(LLVM_ENABLE_ਧਾਗੇ) && LLVM_ENABLE_ਧਾਗੇ != 0
    SwitchedThread = ਸੱਚ;
#ਜੇ_ਅੰਤ
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  ਖਾਲੀ HandleCrash(ਪੂਰਨ Ret_ਕੋਡ, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = ਅਗਲਾ_2;

    ਪ੍ਰਤਿਜ੍ਞਾ(!Failed && "Crash recovery context already failed!");
    Failed = ਸੱਚ;

    ਜੇ (ਚੱਕਰੀ_ਜਾਂਚ->Dump_ਢੇਰ_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    ਚੱਕਰੀ_ਜਾਂਚ->Ret_ਕੋਡ = Ret_ਕੋਡ;

    // Jump back to the RunSafely we were called under.
    ਜੇ (Valid_Jump_ਬਫਰ)
      longjmp(Jump_ਬਫਰ, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ਸਥਿਰ std::mutex CrashRecoveryContextMutex;
  ਵਾਪਸ CrashRecoveryContextMutex;
}

ਸਥਿਰ ਤਾਰਕਿਕ gCrashRecoveryEnabled = ਝੂਠ;

ਸਥਿਰ LLVM_THREAD_LOCAL ਅਚਲ CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ਸਥਿਰ ਖਾਲੀ
installExceptionOrSignalHandlers(ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling);
ਸਥਿਰ ਖਾਲੀ uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = ਮੂਲ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = ਸਿਰ;
  ਅਚਲ CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ਇਹ;
  ਜਦੋਂ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->ਅਗਲਾ;
    tmp->cleanupFired = ਸੱਚ;
    tmp->recoverResources();
    ਮਿਟਾਓ tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  ਮਿਟਾਓ CRCI;
}

ਤਾਰਕਿਕ CrashRecoveryContext::isRecoveringFromCrash() {
  ਵਾਪਸ IsRecoveringFromCrash != ਖਾਲੀ_ਸੂਚਕ;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ਜੇ (!gCrashRecoveryEnabled)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਅਚਲ CrashRecoveryContextImpl *CRCI = CurrentContext;
  ਜੇ (!CRCI)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਵਾਪਸ CRCI->ਚੱਕਰੀ_ਜਾਂਚ;
}

ਖਾਲੀ CrashRecoveryContext::Enable(ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling) {
  std::ਤਾਲਾ_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ਜੇ (gCrashRecoveryEnabled)
    ਵਾਪਸ;
  gCrashRecoveryEnabled = ਸੱਚ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

ਖਾਲੀ CrashRecoveryContext::Disable() {
  std::ਤਾਲਾ_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ਜੇ (!gCrashRecoveryEnabled)
    ਵਾਪਸ;
  gCrashRecoveryEnabled = ਝੂਠ;
  uninstallExceptionOrSignalHandlers();
}

ਖਾਲੀ CrashRecoveryContext::ਰਜਿਸਟਰ_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  ਜੇ (!cleanup)
    ਵਾਪਸ;
  ਜੇ (ਸਿਰ)
    ਸਿਰ->prev = cleanup;
  cleanup->ਅਗਲਾ = ਸਿਰ;
  ਸਿਰ = cleanup;
}

ਖਾਲੀ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ਜੇ (!cleanup)
    ਵਾਪਸ;
  ਜੇ (cleanup == ਸਿਰ) {
    ਸਿਰ = cleanup->ਅਗਲਾ;
    ਜੇ (ਸਿਰ)
      ਸਿਰ->prev = ਖਾਲੀ_ਸੂਚਕ;
  }
  ਨਹੀਂਤਾਂ {
    cleanup->prev->ਅਗਲਾ = cleanup->ਅਗਲਾ;
    ਜੇ (cleanup->ਅਗਲਾ)
      cleanup->ਅਗਲਾ->prev = cleanup->prev;
  }
  ਮਿਟਾਓ cleanup;
}

#ਜੇ defined(_MSC_VER)

#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

ਸਥਿਰ ਖਾਲੀ
installExceptionOrSignalHandlers(ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling) {}
ਸਥਿਰ ਖਾਲੀ uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
ਸਥਿਰ ਪੂਰਨ ExceptionFilter(EXCEPTION_ਸੂਚਕ_ਸਾਰੇ *Except) {
  // Lookup the current thread local recovery object.
  ਅਚਲ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ਜੇ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ਵਾਪਸ EXCEPTION_CONTINUE_ਖੋਜ;
  }

  ਪੂਰਨ Ret_ਕੋਡ = (ਪੂਰਨ)Except->Exception_ਰਿਕਾਰਡ->Exception_ਕੋਡ;
  ਜੇ ((Ret_ਕੋਡ & 0xF0000000) == 0xE0000000)
    Ret_ਕੋਡ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_ਕੋਡ, reinterpret_cast<uintptr_t>(Except));

  ਵਾਪਸ EXCEPTION_EXECUTE_HANDLER;
}

#ਜੇ defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#ਜੇ_ਅੰਤ
ਤਾਰਕਿਕ CrashRecoveryContext::ਲੜੀਵਾਰ_Safely(function_ref<ਖਾਲੀ()> Fn) {
  ਜੇ (!gCrashRecoveryEnabled) {
    Fn();
    ਵਾਪਸ ਸੱਚ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(!Impl && "Crash recovery context already initialized!");
  Impl = ਨਵਾਂ CrashRecoveryContextImpl(ਇਹ);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

#ਨਹੀਂਤਾਂ // !_MSC_VER

#ਜੇ defined(_WIN32)
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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"

ਸਥਿਰ LONG CALLBACK ExceptionHandler(PEXCEPTION_ਸੂਚਕ_ਸਾਰੇ ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  ਚੋਣ (ExceptionInfo->Exception_ਰਿਕਾਰਡ->Exception_ਕੋਡ)
  {
  ਹਾਲਤ DBG_PRINTEXCEPTION_C:
  ਹਾਲਤ DbgPrintExceptionWideC:
  ਹਾਲਤ 0x406D1388:  // set debugger thread name
    ਵਾਪਸ EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ਅਚਲ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ਜੇ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ਵਾਪਸ EXCEPTION_CONTINUE_ਖੋਜ;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  ਪੂਰਨ Ret_ਕੋਡ = (ਪੂਰਨ)ExceptionInfo->Exception_ਰਿਕਾਰਡ->Exception_ਕੋਡ;
  ਜੇ ((Ret_ਕੋਡ & 0xF0000000) == 0xE0000000)
    Ret_ਕੋਡ &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_ਕੋਡ, reinterpret_cast<uintptr_t>(ExceptionInfo));

  // Note that we don't actually get here because HandleCrash calls
  // longjmp, which means the HandleCrash function never returns.
  llvm_unreachable("Handled the crash, should have longjmp'ed out of here");
}

// Because the Enable and Disable calls are static, it means that
// there may not actually be an Impl available, or even a current
// CrashRecoveryContext at all.  So we make use of a thread-local
// exception table.  The handles contained in here will either be
// non-NULL, valid VEH handles, or NULL.
ਸਥਿਰ LLVM_THREAD_LOCAL ਅਚਲ ਖਾਲੀ* sCurrentExceptionHandle;

ਸਥਿਰ ਖਾਲੀ
installExceptionOrSignalHandlers(ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::ਜੋੜੋ_Vectored_Exception_Handler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ਸਥਿਰ ਖਾਲੀ uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ਜੇ (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ਨਿਰਾ;
  }
}

#ਨਹੀਂਤਾਂ // !_WIN32

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

#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>

ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਸੰਕੇਤ_2[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸੰਕੇਤ = std::ਮਾਪ_2(ਸੰਕੇਤ_2);
ਸਥਿਰ ਬਣਤਰ sigaction PrevActions[ਅੰਕ_ਸੰਕੇਤ];

ਸਥਿਰ ਖਾਲੀ CrashRecoverySignalHandler(ਪੂਰਨ Signal) {
  // Lookup the current thread local recovery object.
  ਅਚਲ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ਜੇ (!CRCI) {
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
    gCrashRecoveryEnabled = ਝੂਠ;
    uninstallExceptionOrSignalHandlers();
    ਉਠਾਓ(Signal);

    // The signal will be thrown once the signal mask is restored.
    ਵਾਪਸ;
  }

  // Unblock the signal we received.
  sigset_t Sig_ਨਕਾਬ;
  sigemptyset(&Sig_ਨਕਾਬ);
  sigaddset(&Sig_ਨਕਾਬ, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_ਨਕਾਬ, ਖਾਲੀ_ਸੂਚਕ);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  ਪੂਰਨ Ret_ਕੋਡ = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ਜੇ (Signal == SIGPIPE)
    Ret_ਕੋਡ = EX_IOERR;

  ਜੇ (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_ਕੋਡ, Signal);
}

ਸਥਿਰ ਖਾਲੀ
installExceptionOrSignalHandlers(ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ਬਣਤਰ sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_ਨਕਾਬ);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਅੰਕ_ਸੰਕੇਤ; ++i) {
    ਜੇ (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ਬਣਤਰ sigaction act;
      ਜੇ (sigaction(ਸੰਕੇਤ_2[i], ਨਿਰਾ, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(ਸੰਕੇਤ_2[i], &Handler, &PrevActions[i]);
    } ਨਹੀਂਤਾਂ {
      sigaction(ਸੰਕੇਤ_2[i], &Handler, &PrevActions[i]);
    }
  }
}

ਸਥਿਰ ਖਾਲੀ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਅੰਕ_ਸੰਕੇਤ; ++i)
    sigaction(ਸੰਕੇਤ_2[i], &PrevActions[i], ਖਾਲੀ_ਸੂਚਕ);
}

#ਜੇ_ਅੰਤ // !_WIN32

ਤਾਰਕਿਕ CrashRecoveryContext::ਲੜੀਵਾਰ_Safely(function_ref<ਖਾਲੀ()> Fn) {
  // If crash recovery is disabled, do nothing.
  ਜੇ (gCrashRecoveryEnabled) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = ਨਵਾਂ CrashRecoveryContextImpl(ਇਹ);
    Impl = CRCI;

    CRCI->Valid_Jump_ਬਫਰ = ਸੱਚ;
    ਜੇ (setjmp(CRCI->Jump_ਬਫਰ) != 0) {
      ਵਾਪਸ ਝੂਠ;
    }
  }

  Fn();
  ਵਾਪਸ ਸੱਚ;
}

#ਜੇ_ਅੰਤ // !_MSC_VER

[[noreturn]] ਖਾਲੀ CrashRecoveryContext::HandleExit(ਪੂਰਨ Ret_ਕੋਡ) {
#ਜੇ defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_ਕੋਡ, 0, 0, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ਪ੍ਰਤਿਜ੍ਞਾ(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_ਕੋਡ, 0 /*no sig num*/);
#ਜੇ_ਅੰਤ
  llvm_unreachable("Most likely setjmp wasn't called!");
}

ਤਾਰਕਿਕ CrashRecoveryContext::isCrash(ਪੂਰਨ Ret_ਕੋਡ) {
#ਜੇ defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੋਡ = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)Ret_ਕੋਡ & 0xF0000000) >> 28;
  ਜੇ (ਕੋਡ != 0xC && ਕੋਡ != 8)
    ਵਾਪਸ ਝੂਠ;
#ਨਹੀਂਤਾਂ
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ਜੇ (Ret_ਕੋਡ <= 128)
    ਵਾਪਸ ਝੂਠ;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ CrashRecoveryContext::throwIfCrash(ਪੂਰਨ Ret_ਕੋਡ) {
  ਜੇ (!isCrash(Ret_ਕੋਡ))
    ਵਾਪਸ ਝੂਠ;
#ਜੇ defined(_WIN32)
  ::RaiseException(Ret_ਕੋਡ, 0, 0, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
  llvm::sys::unregisterHandlers();
  ਉਠਾਓ(Ret_ਕੋਡ - 128);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ ਸੱਚ;
}

// FIXME: Portability.
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Thread_Background_ਤਰਜੀਹ() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਤਾਰਕਿਕ has_Thread_Background_ਤਰਜੀਹ() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
  ਵਾਪਸ getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ਝੂਠ;
#ਜੇ_ਅੰਤ
}

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ ਲੜੀਵਾਰ_Safely_On_Thread_Info {
  function_ref<ਖਾਲੀ()> Fn;
  CrashRecoveryContext *ਚੱਕਰੀ_ਜਾਂਚ;
  ਤਾਰਕਿਕ Use_Background_ਤਰਜੀਹ;
  ਤਾਰਕਿਕ ਨਤੀਜਾ;
};
} // namespace

ਸਥਿਰ ਖਾਲੀ ਲੜੀਵਾਰ_Safely_On_Thread_Dispatch(ਖਾਲੀ *User_ਅੰਕੜੇ) {
  ਲੜੀਵਾਰ_Safely_On_Thread_Info *Info =
    reinterpret_cast<ਲੜੀਵਾਰ_Safely_On_Thread_Info*>(User_ਅੰਕੜੇ);

  ਜੇ (Info->Use_Background_ਤਰਜੀਹ)
    ਸੈੱਟ_Thread_Background_ਤਰਜੀਹ();

  Info->ਨਤੀਜਾ = Info->ਚੱਕਰੀ_ਜਾਂਚ->ਲੜੀਵਾਰ_Safely(Info->Fn);
}
ਤਾਰਕਿਕ CrashRecoveryContext::ਲੜੀਵਾਰ_Safely_On_Thread(function_ref<ਖਾਲੀ()> Fn,
                                             ਬਿਨਾਨਿਸ਼ਾਨੀ Requested_ਢੇਰ_ਮਾਪ) {
  ਤਾਰਕਿਕ Use_Background_ਤਰਜੀਹ = has_Thread_Background_ਤਰਜੀਹ();
  ਲੜੀਵਾਰ_Safely_On_Thread_Info Info = { Fn, ਇਹ, Use_Background_ਤਰਜੀਹ, ਝੂਠ };
  llvm::thread Thread(Requested_ਢੇਰ_ਮਾਪ == 0
                          ? std::nullopt
                          : std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>(Requested_ਢੇਰ_ਮਾਪ),
                      ਲੜੀਵਾਰ_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  ਜੇ (CrashRecoveryContextImpl *ਚੱਕਰੀ_ਜਾਂਚ = (CrashRecoveryContextImpl *)Impl)
    ਚੱਕਰੀ_ਜਾਂਚ->ਸੈੱਟ_Switched_Thread();
  ਵਾਪਸ Info.ਨਤੀਜਾ;
}

ਤਾਰਕਿਕ CrashRecoveryContext::ਲੜੀਵਾਰ_Safely_On_New_ਢੇਰ(function_ref<ਖਾਲੀ()> Fn,
                                               ਬਿਨਾਨਿਸ਼ਾਨੀ Requested_ਢੇਰ_ਮਾਪ) {
  ਵਾਪਸ ਲੜੀਵਾਰ_Safely_On_Thread(Fn, Requested_ਢੇਰ_ਮਾਪ);
}
