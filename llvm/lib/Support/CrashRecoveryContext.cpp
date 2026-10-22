//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CrashRecoveryContext.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/ExitCodes.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/thread.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <mutex>
#अन्तर्भाव <setjmp.h>
#ifdef __APPLE__
#अन्तर्भाव <sys/resource.h>
#endif

using नामाकाश llvm;

नामाकाश {

संरचना CrashRecoveryContextImpl;
स्थिर LLVM_THREAD_LOCAL ध्रुव CrashRecoveryContextImpl *CurrentContext;

संरचना CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  ध्रुव CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  चञ्चल unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

सार्वजनिक:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(असत्य), SwitchedThread(असत्य), ValidJumpBuffer(असत्य) {
    Next = CurrentContext;
    CurrentContext = एतत्;
  }
  ~CrashRecoveryContextImpl() {
    यदि (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  शून्य setSwitchedThread() {
#यदि defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = सत्य;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  शून्य HandleCrash(पूर्णाङ्क RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    प्रतिज्ञा(!Failed && "Crash recovery context already failed!");
    Failed = सत्य;

    यदि (CRC->Dump_ढेर_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    यदि (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  स्थिर std::mutex CrashRecoveryContextMutex;
  प्रत्यागम CrashRecoveryContextMutex;
}

स्थिर तार्किक gCrashRecoveryEnabled = असत्य;

स्थिर LLVM_THREAD_LOCAL ध्रुव CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

स्थिर शून्य
installExceptionOrSignalHandlers(तार्किक NeedsPOSIXUtilitySignalHandling);
स्थिर शून्य uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = स्वभाव;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  ध्रुव CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = एतत्;
  यावत् (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = सत्य;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

तार्किक CrashRecoveryContext::isRecoveringFromCrash() {
  प्रत्यागम IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  यदि (!gCrashRecoveryEnabled)
    प्रत्यागम nullptr;

  ध्रुव CrashRecoveryContextImpl *CRCI = CurrentContext;
  यदि (!CRCI)
    प्रत्यागम nullptr;

  प्रत्यागम CRCI->CRC;
}

शून्य CrashRecoveryContext::Enable(तार्किक NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  यदि (gCrashRecoveryEnabled)
    प्रत्यागम;
  gCrashRecoveryEnabled = सत्य;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

शून्य CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  यदि (!gCrashRecoveryEnabled)
    प्रत्यागम;
  gCrashRecoveryEnabled = असत्य;
  uninstallExceptionOrSignalHandlers();
}

शून्य CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  यदि (!cleanup)
    प्रत्यागम;
  यदि (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

शून्य
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  यदि (!cleanup)
    प्रत्यागम;
  यदि (cleanup == head) {
    head = cleanup->next;
    यदि (head)
      head->prev = nullptr;
  }
  अन्यथा {
    cleanup->prev->next = cleanup->next;
    यदि (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#यदि defined(_MSC_VER)

#अन्तर्भाव <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

स्थिर शून्य
installExceptionOrSignalHandlers(तार्किक NeedsPOSIXUtilitySignalHandling) {}
स्थिर शून्य uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
स्थिर पूर्णाङ्क ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  ध्रुव CrashRecoveryContextImpl *CRCI = CurrentContext;

  यदि (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    प्रत्यागम EXCEPTION_CONTINUE_खोज;
  }

  पूर्णाङ्क RetCode = (पूर्णाङ्क)Except->ExceptionRecord->ExceptionCode;
  यदि ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  प्रत्यागम EXCEPTION_EXECUTE_HANDLER;
}

#यदि defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
तार्किक CrashRecoveryContext::RunSafely(function_ref<शून्य()> Fn) {
  यदि (!gCrashRecoveryEnabled) {
    Fn();
    प्रत्यागम सत्य;
  }
  प्रतिज्ञा(!Impl && "Crash recovery context already initialized!");
  Impl = नूतन CrashRecoveryContextImpl(एतत्);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

#अन्यथा // !_MSC_VER

#यदि defined(_WIN32)
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

#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"

स्थिर LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  विकल्प (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  पक्ष DBG_PRINTEXCEPTION_C:
  पक्ष DbgPrintExceptionWideC:
  पक्ष 0x406D1388:  // set debugger thread name
    प्रत्यागम EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  ध्रुव CrashRecoveryContextImpl *CRCI = CurrentContext;

  यदि (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    प्रत्यागम EXCEPTION_CONTINUE_खोज;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  पूर्णाङ्क RetCode = (पूर्णाङ्क)ExceptionInfo->ExceptionRecord->ExceptionCode;
  यदि ((RetCode & 0xF0000000) == 0xE0000000)
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
स्थिर LLVM_THREAD_LOCAL ध्रुव शून्य* sCurrentExceptionHandle;

स्थिर शून्य
installExceptionOrSignalHandlers(तार्किक NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

स्थिर शून्य uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  यदि (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = रिक्त;
  }
}

#अन्यथा // !_WIN32

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

#अन्तर्भाव <signal.h>

स्थिर ध्रुव पूर्णाङ्क Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
स्थिर ध्रुव unsigned अंक_Signals = std::size(Signals);
स्थिर संरचना sigaction PrevActions[अंक_Signals];

स्थिर शून्य CrashRecoverySignalHandler(पूर्णाङ्क Signal) {
  // Lookup the current thread local recovery object.
  ध्रुव CrashRecoveryContextImpl *CRCI = CurrentContext;

  यदि (!CRCI) {
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
    gCrashRecoveryEnabled = असत्य;
    uninstallExceptionOrSignalHandlers();
    उत्थापय(Signal);

    // The signal will be thrown once the signal mask is restored.
    प्रत्यागम;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  पूर्णाङ्क RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  यदि (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  यदि (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

स्थिर शून्य
installExceptionOrSignalHandlers(तार्किक NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  संरचना sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  प्रति (unsigned i = 0; i != अंक_Signals; ++i) {
    यदि (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      संरचना sigaction act;
      यदि (sigaction(Signals[i], रिक्त, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } अन्यथा {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

स्थिर शून्य uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  प्रति (unsigned i = 0; i != अंक_Signals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

तार्किक CrashRecoveryContext::RunSafely(function_ref<शून्य()> Fn) {
  // If crash recovery is disabled, do nothing.
  यदि (gCrashRecoveryEnabled) {
    प्रतिज्ञा(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = नूतन CrashRecoveryContextImpl(एतत्);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = सत्य;
    यदि (setjmp(CRCI->JumpBuffer) != 0) {
      प्रत्यागम असत्य;
    }
  }

  Fn();
  प्रत्यागम सत्य;
}

#endif // !_MSC_VER

[[noreturn]] शून्य CrashRecoveryContext::HandleExit(पूर्णाङ्क RetCode) {
#यदि defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, रिक्त);
#अन्यथा
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  प्रतिज्ञा(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

तार्किक CrashRecoveryContext::isCrash(पूर्णाङ्क RetCode) {
#यदि defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  यदि (Code != 0xC && Code != 8)
    प्रत्यागम असत्य;
#अन्यथा
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  यदि (RetCode <= 128)
    प्रत्यागम असत्य;
#endif
  प्रत्यागम सत्य;
}

तार्किक CrashRecoveryContext::throwIfCrash(पूर्णाङ्क RetCode) {
  यदि (!isCrash(RetCode))
    प्रत्यागम असत्य;
#यदि defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, रिक्त);
#अन्यथा
  llvm::sys::unregisterHandlers();
  उत्थापय(RetCode - 128);
#endif
  प्रत्यागम सत्य;
}

// FIXME: Portability.
स्थिर शून्य setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

स्थिर तार्किक hasThreadBackgroundPriority() {
#ifdef __APPLE__
  प्रत्यागम getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#अन्यथा
  प्रत्यागम असत्य;
#endif
}

नामाकाश {
संरचना RunSafelyOnThreadInfo {
  function_ref<शून्य()> Fn;
  CrashRecoveryContext *CRC;
  तार्किक UseBackgroundPriority;
  तार्किक Result;
};
} // namespace

स्थिर शून्य RunSafelyOnThread_Dispatch(शून्य *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  यदि (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
तार्किक CrashRecoveryContext::RunSafelyOnThread(function_ref<शून्य()> Fn,
                                             unsigned Requested_ढेर_Size) {
  तार्किक UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, एतत्, UseBackgroundPriority, असत्य };
  llvm::thread Thread(Requested_ढेर_Size == 0
                          ? std::nullopt
                          : std::optional<unsigned>(Requested_ढेर_Size),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  यदि (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  प्रत्यागम Info.Result;
}

तार्किक CrashRecoveryContext::Run_Safely_On_New_ढेर(function_ref<शून्य()> Fn,
                                               unsigned Requested_ढेर_Size) {
  प्रत्यागम RunSafelyOnThread(Fn, Requested_ढेर_Size);
}
