//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/CrashRecoveryContext.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/ExitCodes.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/thread.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <mutex>
#അന്തര്ഭാവ <setjmp.h>
#ifdef __APPLE__
#അന്തര്ഭാവ <sys/resource.h>
#endif

using നാമാകാശ llvm;

നാമാകാശ {

ഘടന CrashRecoveryContextImpl;
സ്ഥിതം LLVM_THREAD_LOCAL സ്ഥിരം CrashRecoveryContextImpl *CurrentContext;

ഘടന CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  സ്ഥിരം CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  അസ്ഥിര unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

പൊതു:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(അസത്യം), SwitchedThread(അസത്യം), ValidJumpBuffer(അസത്യം) {
    Next = CurrentContext;
    CurrentContext = ഇത്;
  }
  ~CrashRecoveryContextImpl() {
    എങ്കിൽ (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  ശൂന്യം setSwitchedThread() {
#എങ്കിൽ defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = സത്യം;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  ശൂന്യം HandleCrash(പൂർണ്ണസംഖ്യ RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    പ്രതിജ്ഞാ(!Failed && "Crash recovery context already failed!");
    Failed = സത്യം;

    എങ്കിൽ (CRC->Dump_അട്ടി_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    എങ്കിൽ (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  സ്ഥിതം std::mutex CrashRecoveryContextMutex;
  തിരികെ CrashRecoveryContextMutex;
}

സ്ഥിതം തർക്കം gCrashRecoveryEnabled = അസത്യം;

സ്ഥിതം LLVM_THREAD_LOCAL സ്ഥിരം CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

സ്ഥിതം ശൂന്യം
installExceptionOrSignalHandlers(തർക്കം NeedsPOSIXUtilitySignalHandling);
സ്ഥിതം ശൂന്യം uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = സ്ഥിരസ്ഥിതി;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  സ്ഥിരം CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ഇത്;
  ആകുമ്പോൾ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = സത്യം;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

തർക്കം CrashRecoveryContext::isRecoveringFromCrash() {
  തിരികെ IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  എങ്കിൽ (!gCrashRecoveryEnabled)
    തിരികെ nullptr;

  സ്ഥിരം CrashRecoveryContextImpl *CRCI = CurrentContext;
  എങ്കിൽ (!CRCI)
    തിരികെ nullptr;

  തിരികെ CRCI->CRC;
}

ശൂന്യം CrashRecoveryContext::Enable(തർക്കം NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  എങ്കിൽ (gCrashRecoveryEnabled)
    തിരികെ;
  gCrashRecoveryEnabled = സത്യം;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

ശൂന്യം CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  എങ്കിൽ (!gCrashRecoveryEnabled)
    തിരികെ;
  gCrashRecoveryEnabled = അസത്യം;
  uninstallExceptionOrSignalHandlers();
}

ശൂന്യം CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  എങ്കിൽ (!cleanup)
    തിരികെ;
  എങ്കിൽ (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

ശൂന്യം
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  എങ്കിൽ (!cleanup)
    തിരികെ;
  എങ്കിൽ (cleanup == head) {
    head = cleanup->next;
    എങ്കിൽ (head)
      head->prev = nullptr;
  }
  അല്ലെങ്കിൽ {
    cleanup->prev->next = cleanup->next;
    എങ്കിൽ (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#എങ്കിൽ defined(_MSC_VER)

#അന്തര്ഭാവ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

സ്ഥിതം ശൂന്യം
installExceptionOrSignalHandlers(തർക്കം NeedsPOSIXUtilitySignalHandling) {}
സ്ഥിതം ശൂന്യം uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
സ്ഥിതം പൂർണ്ണസംഖ്യ ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  സ്ഥിരം CrashRecoveryContextImpl *CRCI = CurrentContext;

  എങ്കിൽ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    തിരികെ EXCEPTION_CONTINUE_തിരയൽ;
  }

  പൂർണ്ണസംഖ്യ RetCode = (പൂർണ്ണസംഖ്യ)Except->ExceptionRecord->ExceptionCode;
  എങ്കിൽ ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  തിരികെ EXCEPTION_EXECUTE_HANDLER;
}

#എങ്കിൽ defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
തർക്കം CrashRecoveryContext::RunSafely(function_ref<ശൂന്യം()> Fn) {
  എങ്കിൽ (!gCrashRecoveryEnabled) {
    Fn();
    തിരികെ സത്യം;
  }
  പ്രതിജ്ഞാ(!Impl && "Crash recovery context already initialized!");
  Impl = പുതിയ CrashRecoveryContextImpl(ഇത്);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

#അല്ലെങ്കിൽ // !_MSC_VER

#എങ്കിൽ defined(_WIN32)
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

#അന്തര്ഭാവ "llvm/Support/Windows/WindowsSupport.h"

സ്ഥിതം LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  തിരഞ്ഞെടുപ്പ് (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  അവസ്ഥ DBG_PRINTEXCEPTION_C:
  അവസ്ഥ DbgPrintExceptionWideC:
  അവസ്ഥ 0x406D1388:  // set debugger thread name
    തിരികെ EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  സ്ഥിരം CrashRecoveryContextImpl *CRCI = CurrentContext;

  എങ്കിൽ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    തിരികെ EXCEPTION_CONTINUE_തിരയൽ;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  പൂർണ്ണസംഖ്യ RetCode = (പൂർണ്ണസംഖ്യ)ExceptionInfo->ExceptionRecord->ExceptionCode;
  എങ്കിൽ ((RetCode & 0xF0000000) == 0xE0000000)
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
സ്ഥിതം LLVM_THREAD_LOCAL സ്ഥിരം ശൂന്യം* sCurrentExceptionHandle;

സ്ഥിതം ശൂന്യം
installExceptionOrSignalHandlers(തർക്കം NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

സ്ഥിതം ശൂന്യം uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  എങ്കിൽ (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ശൂന്യ;
  }
}

#അല്ലെങ്കിൽ // !_WIN32

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

#അന്തര്ഭാവ <signal.h>

സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
സ്ഥിതം സ്ഥിരം unsigned അക്കം_Signals = std::size(Signals);
സ്ഥിതം ഘടന sigaction PrevActions[അക്കം_Signals];

സ്ഥിതം ശൂന്യം CrashRecoverySignalHandler(പൂർണ്ണസംഖ്യ Signal) {
  // Lookup the current thread local recovery object.
  സ്ഥിരം CrashRecoveryContextImpl *CRCI = CurrentContext;

  എങ്കിൽ (!CRCI) {
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
    gCrashRecoveryEnabled = അസത്യം;
    uninstallExceptionOrSignalHandlers();
    ഉയർത്തുക(Signal);

    // The signal will be thrown once the signal mask is restored.
    തിരികെ;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  പൂർണ്ണസംഖ്യ RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  എങ്കിൽ (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  എങ്കിൽ (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

സ്ഥിതം ശൂന്യം
installExceptionOrSignalHandlers(തർക്കം NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ഘടന sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  വേണ്ടി (unsigned i = 0; i != അക്കം_Signals; ++i) {
    എങ്കിൽ (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ഘടന sigaction act;
      എങ്കിൽ (sigaction(Signals[i], ശൂന്യ, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } അല്ലെങ്കിൽ {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

സ്ഥിതം ശൂന്യം uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  വേണ്ടി (unsigned i = 0; i != അക്കം_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

തർക്കം CrashRecoveryContext::RunSafely(function_ref<ശൂന്യം()> Fn) {
  // If crash recovery is disabled, do nothing.
  എങ്കിൽ (gCrashRecoveryEnabled) {
    പ്രതിജ്ഞാ(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = പുതിയ CrashRecoveryContextImpl(ഇത്);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = സത്യം;
    എങ്കിൽ (setjmp(CRCI->JumpBuffer) != 0) {
      തിരികെ അസത്യം;
    }
  }

  Fn();
  തിരികെ സത്യം;
}

#endif // !_MSC_VER

[[noreturn]] ശൂന്യം CrashRecoveryContext::HandleExit(പൂർണ്ണസംഖ്യ RetCode) {
#എങ്കിൽ defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, ശൂന്യ);
#അല്ലെങ്കിൽ
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  പ്രതിജ്ഞാ(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

തർക്കം CrashRecoveryContext::isCrash(പൂർണ്ണസംഖ്യ RetCode) {
#എങ്കിൽ defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  എങ്കിൽ (Code != 0xC && Code != 8)
    തിരികെ അസത്യം;
#അല്ലെങ്കിൽ
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  എങ്കിൽ (RetCode <= 128)
    തിരികെ അസത്യം;
#endif
  തിരികെ സത്യം;
}

തർക്കം CrashRecoveryContext::throwIfCrash(പൂർണ്ണസംഖ്യ RetCode) {
  എങ്കിൽ (!isCrash(RetCode))
    തിരികെ അസത്യം;
#എങ്കിൽ defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, ശൂന്യ);
#അല്ലെങ്കിൽ
  llvm::sys::unregisterHandlers();
  ഉയർത്തുക(RetCode - 128);
#endif
  തിരികെ സത്യം;
}

// FIXME: Portability.
സ്ഥിതം ശൂന്യം setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

സ്ഥിതം തർക്കം hasThreadBackgroundPriority() {
#ifdef __APPLE__
  തിരികെ getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#അല്ലെങ്കിൽ
  തിരികെ അസത്യം;
#endif
}

നാമാകാശ {
ഘടന RunSafelyOnThreadInfo {
  function_ref<ശൂന്യം()> Fn;
  CrashRecoveryContext *CRC;
  തർക്കം UseBackgroundPriority;
  തർക്കം Result;
};
} // namespace

സ്ഥിതം ശൂന്യം RunSafelyOnThread_Dispatch(ശൂന്യം *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  എങ്കിൽ (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
തർക്കം CrashRecoveryContext::RunSafelyOnThread(function_ref<ശൂന്യം()> Fn,
                                             unsigned Requested_അട്ടി_Size) {
  തർക്കം UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, ഇത്, UseBackgroundPriority, അസത്യം };
  llvm::thread Thread(Requested_അട്ടി_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_അട്ടി_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  എങ്കിൽ (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  തിരികെ Info.Result;
}

തർക്കം CrashRecoveryContext::Run_Safely_On_New_അട്ടി(function_ref<ശൂന്യം()> Fn,
                                               unsigned Requested_അട്ടി_Size) {
  തിരികെ RunSafelyOnThread(Fn, Requested_അട്ടി_Size);
}
