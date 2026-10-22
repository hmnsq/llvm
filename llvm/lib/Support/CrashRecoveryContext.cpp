//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/CrashRecoveryContext.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/ExitCodes.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/thread.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <mutex>
#અન્તર્ભાવ <setjmp.h>
#ifdef __APPLE__
#અન્તર્ભાવ <sys/resource.h>
#endif

using નામાકાશ llvm;

નામાકાશ {

સંરચના CrashRecoveryContextImpl;
સ્થિર LLVM_THREAD_LOCAL અચલ CrashRecoveryContextImpl *CurrentContext;

સંરચના CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  અચલ CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  અસ્થિર unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

જાહેર:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(ખોટું), SwitchedThread(ખોટું), ValidJumpBuffer(ખોટું) {
    Next = CurrentContext;
    CurrentContext = આ;
  }
  ~CrashRecoveryContextImpl() {
    જો (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  શૂન્ય setSwitchedThread() {
#જો defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = સાચું;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  શૂન્ય HandleCrash(પૂર્ણાંક RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    પ્રતિજ્ઞા(!Failed && "Crash recovery context already failed!");
    Failed = સાચું;

    જો (CRC->Dump_ઢગલો_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    જો (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  સ્થિર std::mutex CrashRecoveryContextMutex;
  પાછા CrashRecoveryContextMutex;
}

સ્થિર તાર્કિક gCrashRecoveryEnabled = ખોટું;

સ્થિર LLVM_THREAD_LOCAL અચલ CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

સ્થિર શૂન્ય
installExceptionOrSignalHandlers(તાર્કિક NeedsPOSIXUtilitySignalHandling);
સ્થિર શૂન્ય uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = મૂળભૂત;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  અચલ CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = આ;
  જ્યારે (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = સાચું;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

તાર્કિક CrashRecoveryContext::isRecoveringFromCrash() {
  પાછા IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  જો (!gCrashRecoveryEnabled)
    પાછા nullptr;

  અચલ CrashRecoveryContextImpl *CRCI = CurrentContext;
  જો (!CRCI)
    પાછા nullptr;

  પાછા CRCI->CRC;
}

શૂન્ય CrashRecoveryContext::Enable(તાર્કિક NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  જો (gCrashRecoveryEnabled)
    પાછા;
  gCrashRecoveryEnabled = સાચું;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

શૂન્ય CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  જો (!gCrashRecoveryEnabled)
    પાછા;
  gCrashRecoveryEnabled = ખોટું;
  uninstallExceptionOrSignalHandlers();
}

શૂન્ય CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  જો (!cleanup)
    પાછા;
  જો (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

શૂન્ય
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  જો (!cleanup)
    પાછા;
  જો (cleanup == head) {
    head = cleanup->next;
    જો (head)
      head->prev = nullptr;
  }
  નહીંતર {
    cleanup->prev->next = cleanup->next;
    જો (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#જો defined(_MSC_VER)

#અન્તર્ભાવ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

સ્થિર શૂન્ય
installExceptionOrSignalHandlers(તાર્કિક NeedsPOSIXUtilitySignalHandling) {}
સ્થિર શૂન્ય uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
સ્થિર પૂર્ણાંક ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  અચલ CrashRecoveryContextImpl *CRCI = CurrentContext;

  જો (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    પાછા EXCEPTION_CONTINUE_શોધ;
  }

  પૂર્ણાંક RetCode = (પૂર્ણાંક)Except->ExceptionRecord->ExceptionCode;
  જો ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  પાછા EXCEPTION_EXECUTE_HANDLER;
}

#જો defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
તાર્કિક CrashRecoveryContext::RunSafely(function_ref<શૂન્ય()> Fn) {
  જો (!gCrashRecoveryEnabled) {
    Fn();
    પાછા સાચું;
  }
  પ્રતિજ્ઞા(!Impl && "Crash recovery context already initialized!");
  Impl = નવું CrashRecoveryContextImpl(આ);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    પાછા ખોટું;
  }
  પાછા સાચું;
}

#નહીંતર // !_MSC_VER

#જો defined(_WIN32)
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

#અન્તર્ભાવ "llvm/Support/Windows/WindowsSupport.h"

સ્થિર LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  પસંદગી (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  સ્થિતિ DBG_PRINTEXCEPTION_C:
  સ્થિતિ DbgPrintExceptionWideC:
  સ્થિતિ 0x406D1388:  // set debugger thread name
    પાછા EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  અચલ CrashRecoveryContextImpl *CRCI = CurrentContext;

  જો (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    પાછા EXCEPTION_CONTINUE_શોધ;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  પૂર્ણાંક RetCode = (પૂર્ણાંક)ExceptionInfo->ExceptionRecord->ExceptionCode;
  જો ((RetCode & 0xF0000000) == 0xE0000000)
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
સ્થિર LLVM_THREAD_LOCAL અચલ શૂન્ય* sCurrentExceptionHandle;

સ્થિર શૂન્ય
installExceptionOrSignalHandlers(તાર્કિક NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

સ્થિર શૂન્ય uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  જો (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ખાલી;
  }
}

#નહીંતર // !_WIN32

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

#અન્તર્ભાવ <signal.h>

સ્થિર અચલ પૂર્ણાંક Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
સ્થિર અચલ unsigned અંક_Signals = std::size(Signals);
સ્થિર સંરચના sigaction PrevActions[અંક_Signals];

સ્થિર શૂન્ય CrashRecoverySignalHandler(પૂર્ણાંક Signal) {
  // Lookup the current thread local recovery object.
  અચલ CrashRecoveryContextImpl *CRCI = CurrentContext;

  જો (!CRCI) {
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
    gCrashRecoveryEnabled = ખોટું;
    uninstallExceptionOrSignalHandlers();
    ઉઠાવો(Signal);

    // The signal will be thrown once the signal mask is restored.
    પાછા;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  પૂર્ણાંક RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  જો (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  જો (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

સ્થિર શૂન્ય
installExceptionOrSignalHandlers(તાર્કિક NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  સંરચના sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  માટે (unsigned i = 0; i != અંક_Signals; ++i) {
    જો (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      સંરચના sigaction act;
      જો (sigaction(Signals[i], ખાલી, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } નહીંતર {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

સ્થિર શૂન્ય uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  માટે (unsigned i = 0; i != અંક_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

તાર્કિક CrashRecoveryContext::RunSafely(function_ref<શૂન્ય()> Fn) {
  // If crash recovery is disabled, do nothing.
  જો (gCrashRecoveryEnabled) {
    પ્રતિજ્ઞા(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = નવું CrashRecoveryContextImpl(આ);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = સાચું;
    જો (setjmp(CRCI->JumpBuffer) != 0) {
      પાછા ખોટું;
    }
  }

  Fn();
  પાછા સાચું;
}

#endif // !_MSC_VER

[[noreturn]] શૂન્ય CrashRecoveryContext::HandleExit(પૂર્ણાંક RetCode) {
#જો defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, ખાલી);
#નહીંતર
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  પ્રતિજ્ઞા(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

તાર્કિક CrashRecoveryContext::isCrash(પૂર્ણાંક RetCode) {
#જો defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  જો (Code != 0xC && Code != 8)
    પાછા ખોટું;
#નહીંતર
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  જો (RetCode <= 128)
    પાછા ખોટું;
#endif
  પાછા સાચું;
}

તાર્કિક CrashRecoveryContext::throwIfCrash(પૂર્ણાંક RetCode) {
  જો (!isCrash(RetCode))
    પાછા ખોટું;
#જો defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, ખાલી);
#નહીંતર
  llvm::sys::unregisterHandlers();
  ઉઠાવો(RetCode - 128);
#endif
  પાછા સાચું;
}

// FIXME: Portability.
સ્થિર શૂન્ય setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

સ્થિર તાર્કિક hasThreadBackgroundPriority() {
#ifdef __APPLE__
  પાછા getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#નહીંતર
  પાછા ખોટું;
#endif
}

નામાકાશ {
સંરચના RunSafelyOnThreadInfo {
  function_ref<શૂન્ય()> Fn;
  CrashRecoveryContext *CRC;
  તાર્કિક UseBackgroundPriority;
  તાર્કિક Result;
};
} // namespace

સ્થિર શૂન્ય RunSafelyOnThread_Dispatch(શૂન્ય *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  જો (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
તાર્કિક CrashRecoveryContext::RunSafelyOnThread(function_ref<શૂન્ય()> Fn,
                                             unsigned Requested_ઢગલો_Size) {
  તાર્કિક UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, આ, UseBackgroundPriority, ખોટું };
  llvm::thread Thread(Requested_ઢગલો_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_ઢગલો_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  જો (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  પાછા Info.Result;
}

તાર્કિક CrashRecoveryContext::Run_Safely_On_New_ઢગલો(function_ref<શૂન્ય()> Fn,
                                               unsigned Requested_ઢગલો_Size) {
  પાછા RunSafelyOnThread(Fn, Requested_ઢગલો_Size);
}
