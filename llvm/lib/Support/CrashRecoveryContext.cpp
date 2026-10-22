//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/CrashRecoveryContext.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/ExitCodes.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/thread.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <mutex>
#অন্তর্ভাব <setjmp.h>
#ifdef __APPLE__
#অন্তর্ভাব <sys/resource.h>
#endif

using নামাকাশ llvm;

নামাকাশ {

কাঠামো CrashRecoveryContextImpl;
স্থির LLVM_THREAD_LOCAL ধ্রুবক CrashRecoveryContextImpl *CurrentContext;

কাঠামো CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ধ্রুবক CrashRecoveryContextImpl *পরবর্তী_2;

  CrashRecoveryContext *CRC;
  ::jmp_buf Jump_বাফার;
  অস্থির unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned Valid_Jump_বাফার : 1;

সর্বজনীন:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(মিথ্যা), SwitchedThread(মিথ্যা), Valid_Jump_বাফার(মিথ্যা) {
    পরবর্তী_2 = CurrentContext;
    CurrentContext = এই;
  }
  ~CrashRecoveryContextImpl() {
    যদি (!SwitchedThread)
      CurrentContext = পরবর্তী_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  শূন্য setSwitchedThread() {
#যদি defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = সত্য;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  শূন্য HandleCrash(পূর্ণসংখ্যা RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = পরবর্তী_2;

    প্রতিজ্ঞা(!Failed && "Crash recovery context already failed!");
    Failed = সত্য;

    যদি (CRC->Dump_স্তূপ_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    যদি (Valid_Jump_বাফার)
      longjmp(Jump_বাফার, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  স্থির std::mutex CrashRecoveryContextMutex;
  ফেরত CrashRecoveryContextMutex;
}

স্থির যৌক্তিক gCrashRecoveryEnabled = মিথ্যা;

স্থির LLVM_THREAD_LOCAL ধ্রুবক CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

স্থির শূন্য
installExceptionOrSignalHandlers(যৌক্তিক NeedsPOSIXUtilitySignalHandling);
স্থির শূন্য uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = পূর্বনির্ধারিত;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = শীর্ষ;
  ধ্রুবক CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = এই;
  যখন (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->পরবর্তী;
    tmp->cleanupFired = সত্য;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

যৌক্তিক CrashRecoveryContext::isRecoveringFromCrash() {
  ফেরত IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  যদি (!gCrashRecoveryEnabled)
    ফেরত nullptr;

  ধ্রুবক CrashRecoveryContextImpl *CRCI = CurrentContext;
  যদি (!CRCI)
    ফেরত nullptr;

  ফেরত CRCI->CRC;
}

শূন্য CrashRecoveryContext::Enable(যৌক্তিক NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  যদি (gCrashRecoveryEnabled)
    ফেরত;
  gCrashRecoveryEnabled = সত্য;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

শূন্য CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  যদি (!gCrashRecoveryEnabled)
    ফেরত;
  gCrashRecoveryEnabled = মিথ্যা;
  uninstallExceptionOrSignalHandlers();
}

শূন্য CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  যদি (!cleanup)
    ফেরত;
  যদি (শীর্ষ)
    শীর্ষ->পূর্ব = cleanup;
  cleanup->পরবর্তী = শীর্ষ;
  শীর্ষ = cleanup;
}

শূন্য
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  যদি (!cleanup)
    ফেরত;
  যদি (cleanup == শীর্ষ) {
    শীর্ষ = cleanup->পরবর্তী;
    যদি (শীর্ষ)
      শীর্ষ->পূর্ব = nullptr;
  }
  অন্যথা {
    cleanup->পূর্ব->পরবর্তী = cleanup->পরবর্তী;
    যদি (cleanup->পরবর্তী)
      cleanup->পরবর্তী->পূর্ব = cleanup->পূর্ব;
  }
  delete cleanup;
}

#যদি defined(_MSC_VER)

#অন্তর্ভাব <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

স্থির শূন্য
installExceptionOrSignalHandlers(যৌক্তিক NeedsPOSIXUtilitySignalHandling) {}
স্থির শূন্য uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
স্থির পূর্ণসংখ্যা ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  ধ্রুবক CrashRecoveryContextImpl *CRCI = CurrentContext;

  যদি (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ফেরত EXCEPTION_CONTINUE_অনুসন্ধান;
  }

  পূর্ণসংখ্যা RetCode = (পূর্ণসংখ্যা)Except->ExceptionRecord->ExceptionCode;
  যদি ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  ফেরত EXCEPTION_EXECUTE_HANDLER;
}

#যদি defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
যৌক্তিক CrashRecoveryContext::RunSafely(function_ref<শূন্য()> Fn) {
  যদি (!gCrashRecoveryEnabled) {
    Fn();
    ফেরত সত্য;
  }
  প্রতিজ্ঞা(!Impl && "Crash recovery context already initialized!");
  Impl = নতুন CrashRecoveryContextImpl(এই);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}

#অন্যথা // !_MSC_VER

#যদি defined(_WIN32)
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

#অন্তর্ভাব "llvm/Support/Windows/WindowsSupport.h"

স্থির LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  নির্বাচন (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  ক্ষেত্র DBG_PRINTEXCEPTION_C:
  ক্ষেত্র DbgPrintExceptionWideC:
  ক্ষেত্র 0x406D1388:  // set debugger thread name
    ফেরত EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ধ্রুবক CrashRecoveryContextImpl *CRCI = CurrentContext;

  যদি (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    ফেরত EXCEPTION_CONTINUE_অনুসন্ধান;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  পূর্ণসংখ্যা RetCode = (পূর্ণসংখ্যা)ExceptionInfo->ExceptionRecord->ExceptionCode;
  যদি ((RetCode & 0xF0000000) == 0xE0000000)
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
স্থির LLVM_THREAD_LOCAL ধ্রুবক শূন্য* sCurrentExceptionHandle;

স্থির শূন্য
installExceptionOrSignalHandlers(যৌক্তিক NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

স্থির শূন্য uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  যদি (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ফাঁকা;
  }
}

#অন্যথা // !_WIN32

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

#অন্তর্ভাব <signal.h>

স্থির ধ্রুবক পূর্ণসংখ্যা Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
স্থির ধ্রুবক unsigned অঙ্ক_Signals = std::আকার_3(Signals);
স্থির কাঠামো sigaction পূর্ব_Actions[অঙ্ক_Signals];

স্থির শূন্য CrashRecoverySignalHandler(পূর্ণসংখ্যা Signal) {
  // Lookup the current thread local recovery object.
  ধ্রুবক CrashRecoveryContextImpl *CRCI = CurrentContext;

  যদি (!CRCI) {
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
    gCrashRecoveryEnabled = মিথ্যা;
    uninstallExceptionOrSignalHandlers();
    তোলো(Signal);

    // The signal will be thrown once the signal mask is restored.
    ফেরত;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  পূর্ণসংখ্যা RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  যদি (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  যদি (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

স্থির শূন্য
installExceptionOrSignalHandlers(যৌক্তিক NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  কাঠামো sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  জন্য (unsigned i = 0; i != অঙ্ক_Signals; ++i) {
    যদি (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      কাঠামো sigaction act;
      যদি (sigaction(Signals[i], ফাঁকা, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &পূর্ব_Actions[i]);
    } অন্যথা {
      sigaction(Signals[i], &Handler, &পূর্ব_Actions[i]);
    }
  }
}

স্থির শূন্য uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  জন্য (unsigned i = 0; i != অঙ্ক_Signals; ++i)
    sigaction(Signals[i], &পূর্ব_Actions[i], nullptr);
}

#endif // !_WIN32

যৌক্তিক CrashRecoveryContext::RunSafely(function_ref<শূন্য()> Fn) {
  // If crash recovery is disabled, do nothing.
  যদি (gCrashRecoveryEnabled) {
    প্রতিজ্ঞা(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = নতুন CrashRecoveryContextImpl(এই);
    Impl = CRCI;

    CRCI->Valid_Jump_বাফার = সত্য;
    যদি (setjmp(CRCI->Jump_বাফার) != 0) {
      ফেরত মিথ্যা;
    }
  }

  Fn();
  ফেরত সত্য;
}

#endif // !_MSC_VER

[[noreturn]] শূন্য CrashRecoveryContext::HandleExit(পূর্ণসংখ্যা RetCode) {
#যদি defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, ফাঁকা);
#অন্যথা
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  প্রতিজ্ঞা(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

যৌক্তিক CrashRecoveryContext::isCrash(পূর্ণসংখ্যা RetCode) {
#যদি defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  যদি (Code != 0xC && Code != 8)
    ফেরত মিথ্যা;
#অন্যথা
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  যদি (RetCode <= 128)
    ফেরত মিথ্যা;
#endif
  ফেরত সত্য;
}

যৌক্তিক CrashRecoveryContext::throwIfCrash(পূর্ণসংখ্যা RetCode) {
  যদি (!isCrash(RetCode))
    ফেরত মিথ্যা;
#যদি defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, ফাঁকা);
#অন্যথা
  llvm::sys::unregisterHandlers();
  তোলো(RetCode - 128);
#endif
  ফেরত সত্য;
}

// FIXME: Portability.
স্থির শূন্য setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

স্থির যৌক্তিক hasThreadBackgroundPriority() {
#ifdef __APPLE__
  ফেরত getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#অন্যথা
  ফেরত মিথ্যা;
#endif
}

নামাকাশ {
কাঠামো RunSafelyOnThreadInfo {
  function_ref<শূন্য()> Fn;
  CrashRecoveryContext *CRC;
  যৌক্তিক UseBackgroundPriority;
  যৌক্তিক ফলাফল;
};
} // namespace

স্থির শূন্য RunSafelyOnThread_Dispatch(শূন্য *User_তথ্য) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(User_তথ্য);

  যদি (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->ফলাফল = Info->CRC->RunSafely(Info->Fn);
}
যৌক্তিক CrashRecoveryContext::RunSafelyOnThread(function_ref<শূন্য()> Fn,
                                             unsigned Requested_স্তূপ_আকার) {
  যৌক্তিক UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, এই, UseBackgroundPriority, মিথ্যা };
  llvm::thread Thread(Requested_স্তূপ_আকার == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_স্তূপ_আকার),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  যদি (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  ফেরত Info.ফলাফল;
}

যৌক্তিক CrashRecoveryContext::Run_Safely_On_New_স্তূপ(function_ref<শূন্য()> Fn,
                                               unsigned Requested_স্তূপ_আকার) {
  ফেরত RunSafelyOnThread(Fn, Requested_স্তূপ_আকার);
}
