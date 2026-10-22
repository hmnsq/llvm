//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/CrashRecoveryContext.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/ExitCodes.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/thread.h"
#समावेश <cassert>
#समावेश <mutex>
#समावेश <setjmp.h>
#अगर_घोषित __APPLE__
#समावेश <sys/resource.h>
#पूर्ण_अगर

using namespace llvm;

namespace {

काष्ठा CrashRecoveryContextImpl;
अटल LLVM_THREAD_LOCAL स्थिर CrashRecoveryContextImpl *CurrentContext;

काष्ठा CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  स्थिर CrashRecoveryContextImpl *अग्र_2;

  CrashRecoveryContext *CRC;
  ::लाँघ_बफ Jump_बफर;
  अस्थिर अचिन्हित Failed : 1;
  अचिन्हित SwitchedThread : 1;
  अचिन्हित Valid_Jump_बफर : 1;

खुला:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(false), SwitchedThread(false), Valid_Jump_बफर(false) {
    अग्र_2 = CurrentContext;
    CurrentContext = यह;
  }
  ~CrashRecoveryContextImpl() {
    अगर (!SwitchedThread)
      CurrentContext = अग्र_2;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  व्योम setSwitchedThread() {
#अगर defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = true;
#पूर्ण_अगर
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  व्योम HandleCrash(पूर्णांक RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = अग्र_2;

    निश्चित(!Failed && "Crash recovery context already failed!");
    Failed = true;

    अगर (CRC->Dump_ढेर_And_Cleanup_On_Failure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    अगर (Valid_Jump_बफर)
      दीर्घ_लाँघ(Jump_बफर, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  अटल std::mutex CrashRecoveryContextMutex;
  वापस CrashRecoveryContextMutex;
}

अटल bool gCrashRecoveryEnabled = false;

अटल LLVM_THREAD_LOCAL स्थिर CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

अटल व्योम
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling);
अटल व्योम uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = शेष;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = शीर्ष;
  स्थिर CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = यह;
  जबतक (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->अग्र;
    tmp->cleanupFired = true;
    tmp->recoverResources();
    मिटाओ tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  मिटाओ CRCI;
}

bool CrashRecoveryContext::isRecoveringFromCrash() {
  वापस IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  अगर (!gCrashRecoveryEnabled)
    वापस nullptr;

  स्थिर CrashRecoveryContextImpl *CRCI = CurrentContext;
  अगर (!CRCI)
    वापस nullptr;

  वापस CRCI->CRC;
}

व्योम CrashRecoveryContext::Enable(bool NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  अगर (gCrashRecoveryEnabled)
    वापस;
  gCrashRecoveryEnabled = true;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

व्योम CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  अगर (!gCrashRecoveryEnabled)
    वापस;
  gCrashRecoveryEnabled = false;
  uninstallExceptionOrSignalHandlers();
}

व्योम CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  अगर (!cleanup)
    वापस;
  अगर (शीर्ष)
    शीर्ष->पूर्व = cleanup;
  cleanup->अग्र = शीर्ष;
  शीर्ष = cleanup;
}

व्योम
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  अगर (!cleanup)
    वापस;
  अगर (cleanup == शीर्ष) {
    शीर्ष = cleanup->अग्र;
    अगर (शीर्ष)
      शीर्ष->पूर्व = nullptr;
  }
  अन्यथा {
    cleanup->पूर्व->अग्र = cleanup->अग्र;
    अगर (cleanup->अग्र)
      cleanup->अग्र->पूर्व = cleanup->पूर्व;
  }
  मिटाओ cleanup;
}

#अगर defined(_MSC_VER)

#समावेश <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

अटल व्योम
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {}
अटल व्योम uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
अटल पूर्णांक ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  स्थिर CrashRecoveryContextImpl *CRCI = CurrentContext;

  अगर (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    वापस EXCEPTION_CONTINUE_खोज;
  }

  पूर्णांक RetCode = (पूर्णांक)Except->ExceptionRecord->ExceptionCode;
  अगर ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  वापस EXCEPTION_EXECUTE_HANDLER;
}

#अगर defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#पूर्ण_अगर
bool CrashRecoveryContext::RunSafely(function_ref<व्योम()> Fn) {
  अगर (!gCrashRecoveryEnabled) {
    Fn();
    वापस true;
  }
  निश्चित(!Impl && "Crash recovery context already initialized!");
  Impl = नया CrashRecoveryContextImpl(यह);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    वापस false;
  }
  वापस true;
}

#अन्यथा // !_MSC_VER

#अगर defined(_WIN32)
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

#समावेश "llvm/Support/Windows/WindowsSupport.h"

अटल LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  चयन (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  हाल DBG_PRINTEXCEPTION_C:
  हाल DbgPrintExceptionWideC:
  हाल 0x406D1388:  // set debugger thread name
    वापस EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  स्थिर CrashRecoveryContextImpl *CRCI = CurrentContext;

  अगर (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    वापस EXCEPTION_CONTINUE_खोज;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  पूर्णांक RetCode = (पूर्णांक)ExceptionInfo->ExceptionRecord->ExceptionCode;
  अगर ((RetCode & 0xF0000000) == 0xE0000000)
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
अटल LLVM_THREAD_LOCAL स्थिर व्योम* sCurrentExceptionHandle;

अटल व्योम
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

अटल व्योम uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  अगर (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = शून्य;
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

#समावेश <signal.h>

अटल स्थिर पूर्णांक Signals[] =
    { SIGABRT, SIGBUS, संक_भ_त्रुटि, संक_अवैध, संक_अंश, SIGTRAP };
अटल स्थिर अचिन्हित अंक_Signals = std::आकार_2(Signals);
अटल काष्ठा sigaction पूर्व_Actions[अंक_Signals];

अटल व्योम CrashRecoverySignalHandler(पूर्णांक Signal) {
  // Lookup the current thread local recovery object.
  स्थिर CrashRecoveryContextImpl *CRCI = CurrentContext;

  अगर (!CRCI) {
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
    gCrashRecoveryEnabled = false;
    uninstallExceptionOrSignalHandlers();
    उठाओ(Signal);

    // The signal will be thrown once the signal mask is restored.
    वापस;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  पूर्णांक RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  अगर (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  अगर (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

अटल व्योम
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  काष्ठा sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  क्रम (अचिन्हित i = 0; i != अंक_Signals; ++i) {
    अगर (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      काष्ठा sigaction act;
      अगर (sigaction(Signals[i], शून्य, &act) == 0 && act.sa_handler != संक_छोड़ो)
        sigaction(Signals[i], &Handler, &पूर्व_Actions[i]);
    } अन्यथा {
      sigaction(Signals[i], &Handler, &पूर्व_Actions[i]);
    }
  }
}

अटल व्योम uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  क्रम (अचिन्हित i = 0; i != अंक_Signals; ++i)
    sigaction(Signals[i], &पूर्व_Actions[i], nullptr);
}

#पूर्ण_अगर // !_WIN32

bool CrashRecoveryContext::RunSafely(function_ref<व्योम()> Fn) {
  // If crash recovery is disabled, do nothing.
  अगर (gCrashRecoveryEnabled) {
    निश्चित(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = नया CrashRecoveryContextImpl(यह);
    Impl = CRCI;

    CRCI->Valid_Jump_बफर = true;
    अगर (बनाओ_लाँघ(CRCI->Jump_बफर) != 0) {
      वापस false;
    }
  }

  Fn();
  वापस true;
}

#पूर्ण_अगर // !_MSC_VER

[[noreturn]] व्योम CrashRecoveryContext::HandleExit(पूर्णांक RetCode) {
#अगर defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, शून्य);
#अन्यथा
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  निश्चित(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#पूर्ण_अगर
  llvm_unreachable("Most likely setjmp wasn't called!");
}

bool CrashRecoveryContext::isCrash(पूर्णांक RetCode) {
#अगर defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  अचिन्हित Code = ((अचिन्हित)RetCode & 0xF0000000) >> 28;
  अगर (Code != 0xC && Code != 8)
    वापस false;
#अन्यथा
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  अगर (RetCode <= 128)
    वापस false;
#पूर्ण_अगर
  वापस true;
}

bool CrashRecoveryContext::throwIfCrash(पूर्णांक RetCode) {
  अगर (!isCrash(RetCode))
    वापस false;
#अगर defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, शून्य);
#अन्यथा
  llvm::sys::unregisterHandlers();
  उठाओ(RetCode - 128);
#पूर्ण_अगर
  वापस true;
}

// FIXME: Portability.
अटल व्योम setThreadBackgroundPriority() {
#अगर_घोषित __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#पूर्ण_अगर
}

अटल bool hasThreadBackgroundPriority() {
#अगर_घोषित __APPLE__
  वापस getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#अन्यथा
  वापस false;
#पूर्ण_अगर
}

namespace {
काष्ठा RunSafelyOnThreadInfo {
  function_ref<व्योम()> Fn;
  CrashRecoveryContext *CRC;
  bool UseBackgroundPriority;
  bool परिणाम;
};
} // namespace

अटल व्योम RunSafelyOnThread_Dispatch(व्योम *User_तथ्य) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(User_तथ्य);

  अगर (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->परिणाम = Info->CRC->RunSafely(Info->Fn);
}
bool CrashRecoveryContext::RunSafelyOnThread(function_ref<व्योम()> Fn,
                                             अचिन्हित Requested_ढेर_आकार) {
  bool UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, यह, UseBackgroundPriority, false };
  llvm::thread Thread(Requested_ढेर_आकार == 0
                          ? std::nullopt
                          : std::optional<अचिन्हित>(Requested_ढेर_आकार),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  अगर (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  वापस Info.परिणाम;
}

bool CrashRecoveryContext::Run_Safely_On_New_ढेर(function_ref<व्योम()> Fn,
                                               अचिन्हित Requested_ढेर_आकार) {
  वापस RunSafelyOnThread(Fn, Requested_ढेर_आकार);
}
