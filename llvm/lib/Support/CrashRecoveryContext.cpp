//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/CrashRecoveryContext.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/ExitCodes.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/thread.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <mutex>
#అన్తర్భావ <setjmp.h>
#నిర్వచితమైతే __APPLE__
#అన్తర్భావ <sys/resource.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

నామాకాశ {

నిర్మాణం CrashRecoveryContextImpl;
స్థిర LLVM_THREAD_LOCAL స్థిరరాశి CrashRecoveryContextImpl *CurrentContext;

నిర్మాణం CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  స్థిరరాశి CrashRecoveryContextImpl *తదుపరి_2;

  CrashRecoveryContext *చక్రీయ_తనిఖీ;
  ::jmp_buf Jump_బఫరు;
  అస్థిర గుర్తులేని Failed : 1;
  గుర్తులేని SwitchedThread : 1;
  గుర్తులేని Valid_Jump_బఫరు : 1;

బహిరంగ:
  CrashRecoveryContextImpl(CrashRecoveryContext *చక్రీయ_తనిఖీ) noexcept
      : చక్రీయ_తనిఖీ(చక్రీయ_తనిఖీ), Failed(అబద్ధం), SwitchedThread(అబద్ధం), Valid_Jump_బఫరు(అబద్ధం) {
    తదుపరి_2 = CurrentContext;
    CurrentContext = ఇది;
  }
  ~CrashRecoveryContextImpl() {
    ఒకవేళ (!SwitchedThread)
      CurrentContext = తదుపరి_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  శూన్య సమితి_Switched_Thread() {
#ఒకవేళ defined(LLVM_ENABLE_దారాలు) && LLVM_ENABLE_దారాలు != 0
    SwitchedThread = నిజం;
#అయితే_ముగింపు
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  శూన్య HandleCrash(పూర్ణాంకం Ret_సంకేతం, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = తదుపరి_2;

    ప్రతిజ్ఞా(!Failed && "Crash recovery context already failed!");
    Failed = నిజం;

    ఒకవేళ (చక్రీయ_తనిఖీ->Dump_దొంతర_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    చక్రీయ_తనిఖీ->Ret_సంకేతం = Ret_సంకేతం;

    // Jump back to the RunSafely we were called under.
    ఒకవేళ (Valid_Jump_బఫరు)
      longjmp(Jump_బఫరు, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  స్థిర std::mutex CrashRecoveryContextMutex;
  తిరిగి CrashRecoveryContextMutex;
}

స్థిర తార్కిక gCrashRecoveryEnabled = అబద్ధం;

స్థిర LLVM_THREAD_LOCAL స్థిరరాశి CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

స్థిర శూన్య
installExceptionOrSignalHandlers(తార్కిక NeedsPOSIXUtilitySignalHandling);
స్థిర శూన్య uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = అప్రమేయ;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = తల;
  స్థిరరాశి CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = ఇది;
  అయితే (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->తదుపరి;
    tmp->cleanupFired = నిజం;
    tmp->recoverResources();
    తొలగించు tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  తొలగించు CRCI;
}

తార్కిక CrashRecoveryContext::isRecoveringFromCrash() {
  తిరిగి IsRecoveringFromCrash != శూన్యసూచిక;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ఒకవేళ (!gCrashRecoveryEnabled)
    తిరిగి శూన్యసూచిక;

  స్థిరరాశి CrashRecoveryContextImpl *CRCI = CurrentContext;
  ఒకవేళ (!CRCI)
    తిరిగి శూన్యసూచిక;

  తిరిగి CRCI->చక్రీయ_తనిఖీ;
}

శూన్య CrashRecoveryContext::Enable(తార్కిక NeedsPOSIXUtilitySignalHandling) {
  std::తాళం_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ఒకవేళ (gCrashRecoveryEnabled)
    తిరిగి;
  gCrashRecoveryEnabled = నిజం;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

శూన్య CrashRecoveryContext::Disable() {
  std::తాళం_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ఒకవేళ (!gCrashRecoveryEnabled)
    తిరిగి;
  gCrashRecoveryEnabled = అబద్ధం;
  uninstallExceptionOrSignalHandlers();
}

శూన్య CrashRecoveryContext::రిజిస్టరు_Cleanup(CrashRecoveryContextCleanup *cleanup)
{
  ఒకవేళ (!cleanup)
    తిరిగి;
  ఒకవేళ (తల)
    తల->prev = cleanup;
  cleanup->తదుపరి = తల;
  తల = cleanup;
}

శూన్య
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ఒకవేళ (!cleanup)
    తిరిగి;
  ఒకవేళ (cleanup == తల) {
    తల = cleanup->తదుపరి;
    ఒకవేళ (తల)
      తల->prev = శూన్యసూచిక;
  }
  లేకపోతే {
    cleanup->prev->తదుపరి = cleanup->తదుపరి;
    ఒకవేళ (cleanup->తదుపరి)
      cleanup->తదుపరి->prev = cleanup->prev;
  }
  తొలగించు cleanup;
}

#ఒకవేళ defined(_MSC_VER)

#అన్తర్భావ <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

స్థిర శూన్య
installExceptionOrSignalHandlers(తార్కిక NeedsPOSIXUtilitySignalHandling) {}
స్థిర శూన్య uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
స్థిర పూర్ణాంకం ExceptionFilter(EXCEPTION_సూచికలు *Except) {
  // Lookup the current thread local recovery object.
  స్థిరరాశి CrashRecoveryContextImpl *CRCI = CurrentContext;

  ఒకవేళ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    తిరిగి EXCEPTION_CONTINUE_శోధన;
  }

  పూర్ణాంకం Ret_సంకేతం = (పూర్ణాంకం)Except->Exception_రికార్డు->Exception_సంకేతం;
  ఒకవేళ ((Ret_సంకేతం & 0xF0000000) == 0xE0000000)
    Ret_సంకేతం &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_సంకేతం, reinterpret_cast<uintptr_t>(Except));

  తిరిగి EXCEPTION_EXECUTE_HANDLER;
}

#ఒకవేళ defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#అయితే_ముగింపు
తార్కిక CrashRecoveryContext::నిరంతరం_Safely(function_ref<శూన్య()> Fn) {
  ఒకవేళ (!gCrashRecoveryEnabled) {
    Fn();
    తిరిగి నిజం;
  }
  ప్రతిజ్ఞా(!Impl && "Crash recovery context already initialized!");
  Impl = కొత్త CrashRecoveryContextImpl(ఇది);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

#లేకపోతే // !_MSC_VER

#ఒకవేళ defined(_WIN32)
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

#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"

స్థిర LONG CALLBACK ExceptionHandler(PEXCEPTION_సూచికలు ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  ఎంపిక (ExceptionInfo->Exception_రికార్డు->Exception_సంకేతం)
  {
  స్థితి DBG_PRINTEXCEPTION_C:
  స్థితి DbgPrintExceptionWideC:
  స్థితి 0x406D1388:  // set debugger thread name
    తిరిగి EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  స్థిరరాశి CrashRecoveryContextImpl *CRCI = CurrentContext;

  ఒకవేళ (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    తిరిగి EXCEPTION_CONTINUE_శోధన;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  పూర్ణాంకం Ret_సంకేతం = (పూర్ణాంకం)ExceptionInfo->Exception_రికార్డు->Exception_సంకేతం;
  ఒకవేళ ((Ret_సంకేతం & 0xF0000000) == 0xE0000000)
    Ret_సంకేతం &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      Ret_సంకేతం, reinterpret_cast<uintptr_t>(ExceptionInfo));

  // Note that we don't actually get here because HandleCrash calls
  // longjmp, which means the HandleCrash function never returns.
  llvm_unreachable("Handled the crash, should have longjmp'ed out of here");
}

// Because the Enable and Disable calls are static, it means that
// there may not actually be an Impl available, or even a current
// CrashRecoveryContext at all.  So we make use of a thread-local
// exception table.  The handles contained in here will either be
// non-NULL, valid VEH handles, or NULL.
స్థిర LLVM_THREAD_LOCAL స్థిరరాశి శూన్య* sCurrentExceptionHandle;

స్థిర శూన్య
installExceptionOrSignalHandlers(తార్కిక NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::కూడు_Vectored_Exception_Handler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

స్థిర శూన్య uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ఒకవేళ (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = ఖాళీ;
  }
}

#లేకపోతే // !_WIN32

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

#అన్తర్భావ <signal.h>

స్థిర స్థిరరాశి పూర్ణాంకం సంకేతాలు[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
స్థిర స్థిరరాశి గుర్తులేని అంకె_సంకేతాలు = std::కొలత_2(సంకేతాలు);
స్థిర నిర్మాణం sigaction PrevActions[అంకె_సంకేతాలు];

స్థిర శూన్య CrashRecoverySignalHandler(పూర్ణాంకం Signal) {
  // Lookup the current thread local recovery object.
  స్థిరరాశి CrashRecoveryContextImpl *CRCI = CurrentContext;

  ఒకవేళ (!CRCI) {
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
    gCrashRecoveryEnabled = అబద్ధం;
    uninstallExceptionOrSignalHandlers();
    లేపు(Signal);

    // The signal will be thrown once the signal mask is restored.
    తిరిగి;
  }

  // Unblock the signal we received.
  sigset_t Sig_ముసుగు;
  sigemptyset(&Sig_ముసుగు);
  sigaddset(&Sig_ముసుగు, Signal);
  sigprocmask(SIG_UNBLOCK, &Sig_ముసుగు, శూన్యసూచిక);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  పూర్ణాంకం Ret_సంకేతం = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ఒకవేళ (Signal == SIGPIPE)
    Ret_సంకేతం = EX_IOERR;

  ఒకవేళ (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(Ret_సంకేతం, Signal);
}

స్థిర శూన్య
installExceptionOrSignalHandlers(తార్కిక NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  నిర్మాణం sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_ముసుగు);

  కొరకు (గుర్తులేని i = 0; i != అంకె_సంకేతాలు; ++i) {
    ఒకవేళ (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      నిర్మాణం sigaction act;
      ఒకవేళ (sigaction(సంకేతాలు[i], ఖాళీ, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(సంకేతాలు[i], &Handler, &PrevActions[i]);
    } లేకపోతే {
      sigaction(సంకేతాలు[i], &Handler, &PrevActions[i]);
    }
  }
}

స్థిర శూన్య uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  కొరకు (గుర్తులేని i = 0; i != అంకె_సంకేతాలు; ++i)
    sigaction(సంకేతాలు[i], &PrevActions[i], శూన్యసూచిక);
}

#అయితే_ముగింపు // !_WIN32

తార్కిక CrashRecoveryContext::నిరంతరం_Safely(function_ref<శూన్య()> Fn) {
  // If crash recovery is disabled, do nothing.
  ఒకవేళ (gCrashRecoveryEnabled) {
    ప్రతిజ్ఞా(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = కొత్త CrashRecoveryContextImpl(ఇది);
    Impl = CRCI;

    CRCI->Valid_Jump_బఫరు = నిజం;
    ఒకవేళ (setjmp(CRCI->Jump_బఫరు) != 0) {
      తిరిగి అబద్ధం;
    }
  }

  Fn();
  తిరిగి నిజం;
}

#అయితే_ముగింపు // !_MSC_VER

[[noreturn]] శూన్య CrashRecoveryContext::HandleExit(పూర్ణాంకం Ret_సంకేతం) {
#ఒకవేళ defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | Ret_సంకేతం, 0, 0, ఖాళీ);
#లేకపోతే
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  ప్రతిజ్ఞా(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(Ret_సంకేతం, 0 /*no sig num*/);
#అయితే_ముగింపు
  llvm_unreachable("Most likely setjmp wasn't called!");
}

తార్కిక CrashRecoveryContext::isCrash(పూర్ణాంకం Ret_సంకేతం) {
#ఒకవేళ defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  గుర్తులేని సంకేతం_2 = ((గుర్తులేని)Ret_సంకేతం & 0xF0000000) >> 28;
  ఒకవేళ (సంకేతం_2 != 0xC && సంకేతం_2 != 8)
    తిరిగి అబద్ధం;
#లేకపోతే
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ఒకవేళ (Ret_సంకేతం <= 128)
    తిరిగి అబద్ధం;
#అయితే_ముగింపు
  తిరిగి నిజం;
}

తార్కిక CrashRecoveryContext::throwIfCrash(పూర్ణాంకం Ret_సంకేతం) {
  ఒకవేళ (!isCrash(Ret_సంకేతం))
    తిరిగి అబద్ధం;
#ఒకవేళ defined(_WIN32)
  ::RaiseException(Ret_సంకేతం, 0, 0, ఖాళీ);
#లేకపోతే
  llvm::sys::unregisterHandlers();
  లేపు(Ret_సంకేతం - 128);
#అయితే_ముగింపు
  తిరిగి నిజం;
}

// FIXME: Portability.
స్థిర శూన్య సమితి_Thread_Background_ప్రాధాన్యత() {
#నిర్వచితమైతే __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#అయితే_ముగింపు
}

స్థిర తార్కిక has_Thread_Background_ప్రాధాన్యత() {
#నిర్వచితమైతే __APPLE__
  తిరిగి getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#లేకపోతే
  తిరిగి అబద్ధం;
#అయితే_ముగింపు
}

నామాకాశ {
నిర్మాణం నిరంతరం_Safely_On_Thread_Info {
  function_ref<శూన్య()> Fn;
  CrashRecoveryContext *చక్రీయ_తనిఖీ;
  తార్కిక Use_Background_ప్రాధాన్యత;
  తార్కిక ఫలితం;
};
} // namespace

స్థిర శూన్య నిరంతరం_Safely_On_Thread_Dispatch(శూన్య *User_దత్తాంశం) {
  నిరంతరం_Safely_On_Thread_Info *Info =
    reinterpret_cast<నిరంతరం_Safely_On_Thread_Info*>(User_దత్తాంశం);

  ఒకవేళ (Info->Use_Background_ప్రాధాన్యత)
    సమితి_Thread_Background_ప్రాధాన్యత();

  Info->ఫలితం = Info->చక్రీయ_తనిఖీ->నిరంతరం_Safely(Info->Fn);
}
తార్కిక CrashRecoveryContext::నిరంతరం_Safely_On_Thread(function_ref<శూన్య()> Fn,
                                             గుర్తులేని Requested_దొంతర_కొలత) {
  తార్కిక Use_Background_ప్రాధాన్యత = has_Thread_Background_ప్రాధాన్యత();
  నిరంతరం_Safely_On_Thread_Info Info = { Fn, ఇది, Use_Background_ప్రాధాన్యత, అబద్ధం };
  llvm::thread Thread(Requested_దొంతర_కొలత == 0
                          ? std::nullopt
                          : std::optional<గుర్తులేని>(Requested_దొంతర_కొలత),
                      నిరంతరం_Safely_On_Thread_Dispatch, &Info);
  Thread.join();

  ఒకవేళ (CrashRecoveryContextImpl *చక్రీయ_తనిఖీ = (CrashRecoveryContextImpl *)Impl)
    చక్రీయ_తనిఖీ->సమితి_Switched_Thread();
  తిరిగి Info.ఫలితం;
}

తార్కిక CrashRecoveryContext::నిరంతరం_Safely_On_New_దొంతర(function_ref<శూన్య()> Fn,
                                               గుర్తులేని Requested_దొంతర_కొలత) {
  తిరిగి నిరంతరం_Safely_On_Thread(Fn, Requested_దొంతర_కొలత);
}
