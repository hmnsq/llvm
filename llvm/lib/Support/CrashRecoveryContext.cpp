//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/CrashRecoveryContext.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ExitCodes.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/thread.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <mutex>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>
#ifdef __APPLE__
#ଅନ୍ତର୍ଭାଵ <sys/resource.h>
#endif

using ନାମାକାଶ llvm;

ନାମାକାଶ {

ସଂରଚନା CrashRecoveryContextImpl;
ସ୍ଥିର LLVM_THREAD_LOCAL ଅଚଳ CrashRecoveryContextImpl *CurrentContext;

ସଂରଚନା CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ଅଚଳ CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  ଅସ୍ଥିର unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

ସର୍ବସାଧାରଣ:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(ମିଥ୍ୟା), SwitchedThread(ମିଥ୍ୟା), ValidJumpBuffer(ମିଥ୍ୟା) {
    Next = CurrentContext;
    CurrentContext = ଏହି;
  }
  ~CrashRecoveryContextImpl() {
    ଯଦି (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  ଶୂନ୍ୟ setSwitchedThread() {
#ଯଦି defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = ସତ୍ୟ;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  ଶୂନ୍ୟ HandleCrash(ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    ପ୍ରତିଜ୍ଞା(!Failed && "Crash recovery context already failed!");
    Failed = ସତ୍ୟ;

    ଯଦି (CRC->Dump_ଗଦା_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    ଯଦି (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  ସ୍ଥିର std::mutex CrashRecoveryContextMutex;
  ଫେରନ୍ତୁ CrashRecoveryContextMutex;
}

ସ୍ଥିର ତାର୍କିକ gCrashRecoveryEnabled = ମିଥ୍ୟା;

ସ୍ଥିର LLVM_THREAD_LOCAL ଅଚଳ CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

ସ୍ଥିର ଶୂନ୍ୟ
installExceptionOrSignalHandlers(ତାର୍କିକ NeedsPOSIXUtilitySignalHandling);
ସ୍ଥିର ଶୂନ୍ୟ uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  ଅଚଳ CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ଏହି;
  ଯେତେବେଳେ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = ସତ୍ୟ;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

ତାର୍କିକ CrashRecoveryContext::isRecoveringFromCrash() {
  ଫେରନ୍ତୁ IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ଯଦି (!gCrashRecoveryEnabled)
    ଫେରନ୍ତୁ nullptr;

  ଅଚଳ CrashRecoveryContextImpl *CRCI = CurrentContext;
  ଯଦି (!CRCI)
    ଫେରନ୍ତୁ nullptr;

  ଫେରନ୍ତୁ CRCI->CRC;
}

ଶୂନ୍ୟ CrashRecoveryContext::Enable(ତାର୍କିକ NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ଯଦି (gCrashRecoveryEnabled)
    ଫେରନ୍ତୁ;
  gCrashRecoveryEnabled = ସତ୍ୟ;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

ଶୂନ୍ୟ CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ଯଦି (!gCrashRecoveryEnabled)
    ଫେରନ୍ତୁ;
  gCrashRecoveryEnabled = ମିଥ୍ୟା;
  uninstallExceptionOrSignalHandlers();
}

ଶୂନ୍ୟ CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  ଯଦି (!cleanup)
    ଫେରନ୍ତୁ;
  ଯଦି (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

ଶୂନ୍ୟ
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ଯଦି (!cleanup)
    ଫେରନ୍ତୁ;
  ଯଦି (cleanup == head) {
    head = cleanup->next;
    ଯଦି (head)
      head->prev = nullptr;
  }
  ଅନ୍ୟଥା {
    cleanup->prev->next = cleanup->next;
    ଯଦି (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#ଯଦି defined(_MSC_VER)

#ଅନ୍ତର୍ଭାଵ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

ସ୍ଥିର ଶୂନ୍ୟ
installExceptionOrSignalHandlers(ତାର୍କିକ NeedsPOSIXUtilitySignalHandling) {}
ସ୍ଥିର ଶୂନ୍ୟ uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  ଅଚଳ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ଯଦି (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ଫେରନ୍ତୁ EXCEPTION_CONTINUE_ସନ୍ଧାନ;
  }

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)Except->ExceptionRecord->ExceptionCode;
  ଯଦି ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  ଫେରନ୍ତୁ EXCEPTION_EXECUTE_HANDLER;
}

#ଯଦି defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
ତାର୍କିକ CrashRecoveryContext::RunSafely(function_ref<ଶୂନ୍ୟ()> Fn) {
  ଯଦି (!gCrashRecoveryEnabled) {
    Fn();
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ପ୍ରତିଜ୍ଞା(!Impl && "Crash recovery context already initialized!");
  Impl = ନୂତନ CrashRecoveryContextImpl(ଏହି);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

#ଅନ୍ୟଥା // !_MSC_VER

#ଯଦି defined(_WIN32)
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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Windows/WindowsSupport.h"

ସ୍ଥିର LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  ବାଛ (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  ସ୍ଥିତି DBG_PRINTEXCEPTION_C:
  ସ୍ଥିତି DbgPrintExceptionWideC:
  ସ୍ଥିତି 0x406D1388:  // set debugger thread name
    ଫେରନ୍ତୁ EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ଅଚଳ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ଯଦି (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ଫେରନ୍ତୁ EXCEPTION_CONTINUE_ସନ୍ଧାନ;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ExceptionInfo->ExceptionRecord->ExceptionCode;
  ଯଦି ((RetCode & 0xF0000000) == 0xE0000000)
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
ସ୍ଥିର LLVM_THREAD_LOCAL ଅଚଳ ଶୂନ୍ୟ* sCurrentExceptionHandle;

ସ୍ଥିର ଶୂନ୍ୟ
installExceptionOrSignalHandlers(ତାର୍କିକ NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

ସ୍ଥିର ଶୂନ୍ୟ uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ଯଦି (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ଖାଲି;
  }
}

#ଅନ୍ୟଥା // !_WIN32

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

#ଅନ୍ତର୍ଭାଵ <signal.h>

ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
ସ୍ଥିର ଅଚଳ unsigned ଅଙ୍କ_Signals = std::size(Signals);
ସ୍ଥିର ସଂରଚନା sigaction PrevActions[ଅଙ୍କ_Signals];

ସ୍ଥିର ଶୂନ୍ୟ CrashRecoverySignalHandler(ପୂର୍ଣ୍ଣସଂଖ୍ୟା Signal) {
  // Lookup the current thread local recovery object.
  ଅଚଳ CrashRecoveryContextImpl *CRCI = CurrentContext;

  ଯଦି (!CRCI) {
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
    gCrashRecoveryEnabled = ମିଥ୍ୟା;
    uninstallExceptionOrSignalHandlers();
    ଉଠାଅ(Signal);

    // The signal will be thrown once the signal mask is restored.
    ଫେରନ୍ତୁ;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ଯଦି (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  ଯଦି (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

ସ୍ଥିର ଶୂନ୍ୟ
installExceptionOrSignalHandlers(ତାର୍କିକ NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  ସଂରଚନା sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  ପାଇଁ (unsigned i = 0; i != ଅଙ୍କ_Signals; ++i) {
    ଯଦି (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      ସଂରଚନା sigaction act;
      ଯଦି (sigaction(Signals[i], ଖାଲି, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } ଅନ୍ୟଥା {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

ସ୍ଥିର ଶୂନ୍ୟ uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  ପାଇଁ (unsigned i = 0; i != ଅଙ୍କ_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

ତାର୍କିକ CrashRecoveryContext::RunSafely(function_ref<ଶୂନ୍ୟ()> Fn) {
  // If crash recovery is disabled, do nothing.
  ଯଦି (gCrashRecoveryEnabled) {
    ପ୍ରତିଜ୍ଞା(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = ନୂତନ CrashRecoveryContextImpl(ଏହି);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = ସତ୍ୟ;
    ଯଦି (setjmp(CRCI->JumpBuffer) != 0) {
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  }

  Fn();
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

#endif // !_MSC_VER

[[noreturn]] ଶୂନ୍ୟ CrashRecoveryContext::HandleExit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode) {
#ଯଦି defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, ଖାଲି);
#ଅନ୍ୟଥା
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ପ୍ରତିଜ୍ଞା(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

ତାର୍କିକ CrashRecoveryContext::isCrash(ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode) {
#ଯଦି defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  ଯଦି (Code != 0xC && Code != 8)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
#ଅନ୍ୟଥା
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ଯଦି (RetCode <= 128)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
#endif
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ CrashRecoveryContext::throwIfCrash(ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode) {
  ଯଦି (!isCrash(RetCode))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
#ଯଦି defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, ଖାଲି);
#ଅନ୍ୟଥା
  llvm::sys::unregisterHandlers();
  ଉଠାଅ(RetCode - 128);
#endif
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

// FIXME: Portability.
ସ୍ଥିର ଶୂନ୍ୟ setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

ସ୍ଥିର ତାର୍କିକ hasThreadBackgroundPriority() {
#ifdef __APPLE__
  ଫେରନ୍ତୁ getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
#endif
}

ନାମାକାଶ {
ସଂରଚନା RunSafelyOnThreadInfo {
  function_ref<ଶୂନ୍ୟ()> Fn;
  CrashRecoveryContext *CRC;
  ତାର୍କିକ UseBackgroundPriority;
  ତାର୍କିକ Result;
};
} // namespace

ସ୍ଥିର ଶୂନ୍ୟ RunSafelyOnThread_Dispatch(ଶୂନ୍ୟ *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  ଯଦି (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
ତାର୍କିକ CrashRecoveryContext::RunSafelyOnThread(function_ref<ଶୂନ୍ୟ()> Fn,
                                             unsigned Requested_ଗଦା_Size) {
  ତାର୍କିକ UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, ଏହି, UseBackgroundPriority, ମିଥ୍ୟା };
  llvm::thread Thread(Requested_ଗଦା_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_ଗଦା_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  ଯଦି (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  ଫେରନ୍ତୁ Info.Result;
}

ତାର୍କିକ CrashRecoveryContext::Run_Safely_On_New_ଗଦା(function_ref<ଶୂନ୍ୟ()> Fn,
                                               unsigned Requested_ଗଦା_Size) {
  ଫେରନ୍ତୁ RunSafelyOnThread(Fn, Requested_ଗଦା_Size);
}
