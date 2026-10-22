//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/CrashRecoveryContext.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/ExitCodes.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/thread.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <mutex>
#அந்தர்பாவ <setjmp.h>
#ifdef __APPLE__
#அந்தர்பாவ <sys/resource.h>
#endif

using நாமாகாஶ llvm;

நாமாகாஶ {

கட்டமைப்பு CrashRecoveryContextImpl;
நிலையான LLVM_THREAD_LOCAL மாறிலி CrashRecoveryContextImpl *CurrentContext;

கட்டமைப்பு CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  மாறிலி CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  நிலையற்ற unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

பொது:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(பொய்), SwitchedThread(பொய்), ValidJumpBuffer(பொய்) {
    Next = CurrentContext;
    CurrentContext = இது;
  }
  ~CrashRecoveryContextImpl() {
    எனில் (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  வெறுமை setSwitchedThread() {
#எனில் defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = உண்மை;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  வெறுமை HandleCrash(முழு RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    ப்ரதிஜ்ஞா(!Failed && "Crash recovery context already failed!");
    Failed = உண்மை;

    எனில் (CRC->Dump_அடுக்கு_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    எனில் (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  நிலையான std::mutex CrashRecoveryContextMutex;
  திரும்பு CrashRecoveryContextMutex;
}

நிலையான தருக்க gCrashRecoveryEnabled = பொய்;

நிலையான LLVM_THREAD_LOCAL மாறிலி CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

நிலையான வெறுமை
installExceptionOrSignalHandlers(தருக்க NeedsPOSIXUtilitySignalHandling);
நிலையான வெறுமை uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = இயல்பு;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  மாறிலி CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = இது;
  போது (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = உண்மை;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

தருக்க CrashRecoveryContext::isRecoveringFromCrash() {
  திரும்பு IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  எனில் (!gCrashRecoveryEnabled)
    திரும்பு nullptr;

  மாறிலி CrashRecoveryContextImpl *CRCI = CurrentContext;
  எனில் (!CRCI)
    திரும்பு nullptr;

  திரும்பு CRCI->CRC;
}

வெறுமை CrashRecoveryContext::Enable(தருக்க NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  எனில் (gCrashRecoveryEnabled)
    திரும்பு;
  gCrashRecoveryEnabled = உண்மை;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

வெறுமை CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  எனில் (!gCrashRecoveryEnabled)
    திரும்பு;
  gCrashRecoveryEnabled = பொய்;
  uninstallExceptionOrSignalHandlers();
}

வெறுமை CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  எனில் (!cleanup)
    திரும்பு;
  எனில் (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

வெறுமை
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  எனில் (!cleanup)
    திரும்பு;
  எனில் (cleanup == head) {
    head = cleanup->next;
    எனில் (head)
      head->prev = nullptr;
  }
  இல்லையெனில் {
    cleanup->prev->next = cleanup->next;
    எனில் (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#எனில் defined(_MSC_VER)

#அந்தர்பாவ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

நிலையான வெறுமை
installExceptionOrSignalHandlers(தருக்க NeedsPOSIXUtilitySignalHandling) {}
நிலையான வெறுமை uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
நிலையான முழு ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  மாறிலி CrashRecoveryContextImpl *CRCI = CurrentContext;

  எனில் (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    திரும்பு EXCEPTION_CONTINUE_தேடல்;
  }

  முழு RetCode = (முழு)Except->ExceptionRecord->ExceptionCode;
  எனில் ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  திரும்பு EXCEPTION_EXECUTE_HANDLER;
}

#எனில் defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
தருக்க CrashRecoveryContext::RunSafely(function_ref<வெறுமை()> Fn) {
  எனில் (!gCrashRecoveryEnabled) {
    Fn();
    திரும்பு உண்மை;
  }
  ப்ரதிஜ்ஞா(!Impl && "Crash recovery context already initialized!");
  Impl = புதிய CrashRecoveryContextImpl(இது);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

#இல்லையெனில் // !_MSC_VER

#எனில் defined(_WIN32)
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

#அந்தர்பாவ "llvm/Support/Windows/WindowsSupport.h"

நிலையான LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  தேர்வு (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  நிலை DBG_PRINTEXCEPTION_C:
  நிலை DbgPrintExceptionWideC:
  நிலை 0x406D1388:  // set debugger thread name
    திரும்பு EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  மாறிலி CrashRecoveryContextImpl *CRCI = CurrentContext;

  எனில் (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    திரும்பு EXCEPTION_CONTINUE_தேடல்;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  முழு RetCode = (முழு)ExceptionInfo->ExceptionRecord->ExceptionCode;
  எனில் ((RetCode & 0xF0000000) == 0xE0000000)
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
நிலையான LLVM_THREAD_LOCAL மாறிலி வெறுமை* sCurrentExceptionHandle;

நிலையான வெறுமை
installExceptionOrSignalHandlers(தருக்க NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

நிலையான வெறுமை uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  எனில் (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = வெற்று;
  }
}

#இல்லையெனில் // !_WIN32

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

#அந்தர்பாவ <signal.h>

நிலையான மாறிலி முழு Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
நிலையான மாறிலி unsigned இலக்கம்_Signals = std::size(Signals);
நிலையான கட்டமைப்பு sigaction PrevActions[இலக்கம்_Signals];

நிலையான வெறுமை CrashRecoverySignalHandler(முழு Signal) {
  // Lookup the current thread local recovery object.
  மாறிலி CrashRecoveryContextImpl *CRCI = CurrentContext;

  எனில் (!CRCI) {
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
    gCrashRecoveryEnabled = பொய்;
    uninstallExceptionOrSignalHandlers();
    எழுப்பு(Signal);

    // The signal will be thrown once the signal mask is restored.
    திரும்பு;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  முழு RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  எனில் (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  எனில் (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

நிலையான வெறுமை
installExceptionOrSignalHandlers(தருக்க NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  கட்டமைப்பு sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  க்கு (unsigned i = 0; i != இலக்கம்_Signals; ++i) {
    எனில் (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      கட்டமைப்பு sigaction act;
      எனில் (sigaction(Signals[i], வெற்று, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } இல்லையெனில் {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

நிலையான வெறுமை uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  க்கு (unsigned i = 0; i != இலக்கம்_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

தருக்க CrashRecoveryContext::RunSafely(function_ref<வெறுமை()> Fn) {
  // If crash recovery is disabled, do nothing.
  எனில் (gCrashRecoveryEnabled) {
    ப்ரதிஜ்ஞா(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = புதிய CrashRecoveryContextImpl(இது);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = உண்மை;
    எனில் (setjmp(CRCI->JumpBuffer) != 0) {
      திரும்பு பொய்;
    }
  }

  Fn();
  திரும்பு உண்மை;
}

#endif // !_MSC_VER

[[noreturn]] வெறுமை CrashRecoveryContext::HandleExit(முழு RetCode) {
#எனில் defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, வெற்று);
#இல்லையெனில்
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ப்ரதிஜ்ஞா(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

தருக்க CrashRecoveryContext::isCrash(முழு RetCode) {
#எனில் defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  எனில் (Code != 0xC && Code != 8)
    திரும்பு பொய்;
#இல்லையெனில்
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  எனில் (RetCode <= 128)
    திரும்பு பொய்;
#endif
  திரும்பு உண்மை;
}

தருக்க CrashRecoveryContext::throwIfCrash(முழு RetCode) {
  எனில் (!isCrash(RetCode))
    திரும்பு பொய்;
#எனில் defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, வெற்று);
#இல்லையெனில்
  llvm::sys::unregisterHandlers();
  எழுப்பு(RetCode - 128);
#endif
  திரும்பு உண்மை;
}

// FIXME: Portability.
நிலையான வெறுமை setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

நிலையான தருக்க hasThreadBackgroundPriority() {
#ifdef __APPLE__
  திரும்பு getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#இல்லையெனில்
  திரும்பு பொய்;
#endif
}

நாமாகாஶ {
கட்டமைப்பு RunSafelyOnThreadInfo {
  function_ref<வெறுமை()> Fn;
  CrashRecoveryContext *CRC;
  தருக்க UseBackgroundPriority;
  தருக்க Result;
};
} // namespace

நிலையான வெறுமை RunSafelyOnThread_Dispatch(வெறுமை *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  எனில் (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
தருக்க CrashRecoveryContext::RunSafelyOnThread(function_ref<வெறுமை()> Fn,
                                             unsigned Requested_அடுக்கு_Size) {
  தருக்க UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, இது, UseBackgroundPriority, பொய் };
  llvm::thread Thread(Requested_அடுக்கு_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_அடுக்கு_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  எனில் (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  திரும்பு Info.Result;
}

தருக்க CrashRecoveryContext::Run_Safely_On_New_அடுக்கு(function_ref<வெறுமை()> Fn,
                                               unsigned Requested_அடுக்கு_Size) {
  திரும்பு RunSafelyOnThread(Fn, Requested_அடுக்கு_Size);
}
