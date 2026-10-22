//===--- CrashRecoveryContext.cpp - Crash Recovery ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ExitCodes.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/thread.h"
#include <cassert>
#include <mutex>
#include <setjmp.h>
#ifdef __APPLE__
#include <sys/resource.h>
#endif

using namespace llvm;

namespace {

sa_nracanaa CrashRecoveryContextImpl;
static LLVM_THREAD_LOCAL const CrashRecoveryContextImpl *CurrentContext;

sa_nracanaa CrashRecoveryContextImpl {
  // When threads are disabled, this links up all active
  // CrashRecoveryContextImpls.  When threads are enabled there's one thread
  // per CrashRecoveryContext and CurrentContext is a thread-local, so only one
  // CrashRecoveryContextImpl is active per thread and this is always null.
  const CrashRecoveryContextImpl *Next;

  CrashRecoveryContext *CRC;
  ::jmp_buf JumpBuffer;
  ca_ncala unsigned Failed : 1;
  unsigned SwitchedThread : 1;
  unsigned ValidJumpBuffer : 1;

public:
  CrashRecoveryContextImpl(CrashRecoveryContext *CRC) noexcept
      : CRC(CRC), Failed(false), SwitchedThread(false), ValidJumpBuffer(false) {
    Next = CurrentContext;
    CurrentContext = this;
  }
  ~CrashRecoveryContextImpl() {
    ถ้า (!SwitchedThread)
      CurrentContext = Next;
  }

  /// Called when the separate crash-recovery thread was finished, to
  /// indicate that we don't need to clear the thread-local CurrentContext.
  void setSwitchedThread() {
#ถ้า defined(LLVM_ENABLE_THREADS) && LLVM_ENABLE_THREADS != 0
    SwitchedThread = true;
#endif
  }

  // If the function ran by the CrashRecoveryContext crashes or fails, then
  // 'RetCode' represents the returned error code, as if it was returned by a
  // process. 'Context' represents the signal type on Unix; on Windows, it is
  // the ExceptionContext.
  void HandleCrash(จำนวนเต็ม RetCode, uintptr_t Context) {
    // Eliminate the current context entry, to avoid re-entering in case the
    // cleanup code crashes.
    CurrentContext = Next;

    assert(!Failed && "Crash recovery context already failed!");
    Failed = true;

    ถ้า (CRC->DumpStackAndCleanupOnFailure)
      sys::CleanupOnSignal(Context);

    CRC->RetCode = RetCode;

    // Jump back to the RunSafely we were called under.
    ถ้า (ValidJumpBuffer)
      longjmp(JumpBuffer, 1);

    // Otherwise let the caller decide of the outcome of the crash. Currently
    // this occurs when using SEH on Windows with MSVC or clang-cl.
  }
};

std::mutex &getCrashRecoveryContextMutex() {
  static std::mutex CrashRecoveryContextMutex;
  คืน CrashRecoveryContextMutex;
}

static bool gCrashRecoveryEnabled = false;

static LLVM_THREAD_LOCAL const CrashRecoveryContext *IsRecoveringFromCrash;

} // namespace

static void
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling);
static void uninstallExceptionOrSignalHandlers();

CrashRecoveryContextCleanup::~CrashRecoveryContextCleanup() = default;

CrashRecoveryContext::CrashRecoveryContext() {
  // On Windows, if abort() was previously triggered (and caught by a previous
  // CrashRecoveryContext) the Windows CRT removes our installed signal handler,
  // so we need to install it again.
  sys::DisableSystemDialogsOnCrash();
}

CrashRecoveryContext::~CrashRecoveryContext() {
  // Reclaim registered resources.
  CrashRecoveryContextCleanup *i = head;
  const CrashRecoveryContext *PC = IsRecoveringFromCrash;
  IsRecoveringFromCrash = this;
  ขณะ (i) {
    CrashRecoveryContextCleanup *tmp = i;
    i = tmp->next;
    tmp->cleanupFired = true;
    tmp->recoverResources();
    delete tmp;
  }
  IsRecoveringFromCrash = PC;

  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *) Impl;
  delete CRCI;
}

bool CrashRecoveryContext::isRecoveringFromCrash() {
  คืน IsRecoveringFromCrash != nullptr;
}

CrashRecoveryContext *CrashRecoveryContext::GetCurrent() {
  ถ้า (!gCrashRecoveryEnabled)
    คืน nullptr;

  const CrashRecoveryContextImpl *CRCI = CurrentContext;
  ถ้า (!CRCI)
    คืน nullptr;

  คืน CRCI->CRC;
}

void CrashRecoveryContext::Enable(bool NeedsPOSIXUtilitySignalHandling) {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  // FIXME: Shouldn't this be a refcount or something?
  ถ้า (gCrashRecoveryEnabled)
    คืน;
  gCrashRecoveryEnabled = true;
  installExceptionOrSignalHandlers(NeedsPOSIXUtilitySignalHandling);
}

void CrashRecoveryContext::Disable() {
  std::lock_guard<std::mutex> L(getCrashRecoveryContextMutex());
  ถ้า (!gCrashRecoveryEnabled)
    คืน;
  gCrashRecoveryEnabled = false;
  uninstallExceptionOrSignalHandlers();
}

void CrashRecoveryContext::registerCleanup(CrashRecoveryContextCleanup *cleanup)
{
  ถ้า (!cleanup)
    คืน;
  ถ้า (head)
    head->prev = cleanup;
  cleanup->next = head;
  head = cleanup;
}

void
CrashRecoveryContext::unregisterCleanup(CrashRecoveryContextCleanup *cleanup) {
  ถ้า (!cleanup)
    คืน;
  ถ้า (cleanup == head) {
    head = cleanup->next;
    ถ้า (head)
      head->prev = nullptr;
  }
  มิฉะนั้น {
    cleanup->prev->next = cleanup->next;
    ถ้า (cleanup->next)
      cleanup->next->prev = cleanup->prev;
  }
  delete cleanup;
}

#ถ้า defined(_MSC_VER)

#include <windows.h> // for GetExceptionInformation

// If _MSC_VER is defined, we must have SEH. Use it if it's available. It's way
// better than VEH. Vectored exception handling catches all exceptions happening
// on the thread with installed exception handlers, so it can interfere with
// internal exception handling of other libraries on that thread. SEH works
// exactly as you would expect normal exception handling to work: it only
// catches exceptions if they would bubble out from the stack frame with __try /
// __except.

static void
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {}
static void uninstallExceptionOrSignalHandlers() {}

// We need this function because the call to GetExceptionInformation() can only
// occur inside the __except evaluation block
static จำนวนเต็ม ExceptionFilter(_EXCEPTION_POINTERS *Except) {
  // Lookup the current thread local recovery object.
  const CrashRecoveryContextImpl *CRCI = CurrentContext;

  ถ้า (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    คืน EXCEPTION_CONTINUE_SEARCH;
  }

  จำนวนเต็ม RetCode = (จำนวนเต็ม)Except->ExceptionRecord->ExceptionCode;
  ถ้า ((RetCode & 0xF0000000) == 0xE0000000)
    RetCode &= ~0xF0000000; // this crash was generated by sys::Process::Exit

  // Handle the crash
  const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(
      RetCode, reinterpret_cast<uintptr_t>(Except));

  คืน EXCEPTION_EXECUTE_HANDLER;
}

#ถ้า defined(__clang__) && defined(_M_IX86)
// Work around PR44697.
__attribute__((optnone))
#endif
bool CrashRecoveryContext::RunSafely(function_ref<void()> Fn) {
  ถ้า (!gCrashRecoveryEnabled) {
    Fn();
    คืน true;
  }
  assert(!Impl && "Crash recovery context already initialized!");
  Impl = new CrashRecoveryContextImpl(this);
  __try {
    Fn();
  } __except (ExceptionFilter(GetExceptionInformation())) {
    คืน false;
  }
  คืน true;
}

#มิฉะนั้น // !_MSC_VER

#ถ้า defined(_WIN32)
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

#include "llvm/Support/Windows/WindowsSupport.h"

static LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  // DBG_PRINTEXCEPTION_WIDE_C is not properly defined on all supported
  // compilers and platforms, so we define it manually.
  constexpr ULONG DbgPrintExceptionWideC = 0x4001000AL;
  switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  pak_sa DBG_PRINTEXCEPTION_C:
  pak_sa DbgPrintExceptionWideC:
  pak_sa 0x406D1388:  // set debugger thread name
    คืน EXCEPTION_CONTINUE_EXECUTION;
  }

  // Lookup the current thread local recovery object.
  const CrashRecoveryContextImpl *CRCI = CurrentContext;

  ถ้า (!CRCI) {
    // Something has gone horribly wrong, so let's just tell everyone
    // to keep searching
    CrashRecoveryContext::Disable();
    คืน EXCEPTION_CONTINUE_SEARCH;
  }

  // TODO: We can capture the stack backtrace here and store it on the
  // implementation if we so choose.

  จำนวนเต็ม RetCode = (จำนวนเต็ม)ExceptionInfo->ExceptionRecord->ExceptionCode;
  ถ้า ((RetCode & 0xF0000000) == 0xE0000000)
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
static LLVM_THREAD_LOCAL const void* sCurrentExceptionHandle;

static void
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {
  // We can set up vectored exception handling now.  We will install our
  // handler as the front of the list, though there's no assurances that
  // it will remain at the front (another call could install itself before
  // our handler).  This 1) isn't likely, and 2) shouldn't cause problems.
  PVOID handle = ::AddVectoredExceptionHandler(1, ExceptionHandler);
  sCurrentExceptionHandle = handle;
}

static void uninstallExceptionOrSignalHandlers() {
  PVOID currentHandle = const_cast<PVOID>(sCurrentExceptionHandle);
  ถ้า (currentHandle) {
    // Now we can remove the vectored exception handler from the chain
    ::RemoveVectoredExceptionHandler(currentHandle);

    // Reset the handle in our thread-local set.
    sCurrentExceptionHandle = NULL;
  }
}

#มิฉะนั้น // !_WIN32

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

#include <signal.h>

static const จำนวนเต็ม Signals[] =
    { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP };
static const unsigned NumSignals = std::size(Signals);
static sa_nracanaa sigaction PrevActions[NumSignals];

static void CrashRecoverySignalHandler(จำนวนเต็ม Signal) {
  // Lookup the current thread local recovery object.
  const CrashRecoveryContextImpl *CRCI = CurrentContext;

  ถ้า (!CRCI) {
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
    raise(Signal);

    // The signal will be thrown once the signal mask is restored.
    คืน;
  }

  // Unblock the signal we received.
  sigset_t SigMask;
  sigemptyset(&SigMask);
  sigaddset(&SigMask, Signal);
  sigprocmask(SIG_UNBLOCK, &SigMask, nullptr);

  // Return the same error code as if the program crashed, as mentioned in the
  // section "Exit Status for Commands":
  // https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html
  จำนวนเต็ม RetCode = 128 + Signal;

  // Don't consider a broken pipe as a crash (see clang/lib/Driver/Driver.cpp)
  ถ้า (Signal == SIGPIPE)
    RetCode = EX_IOERR;

  ถ้า (CRCI)
    const_cast<CrashRecoveryContextImpl *>(CRCI)->HandleCrash(RetCode, Signal);
}

static void
installExceptionOrSignalHandlers(bool NeedsPOSIXUtilitySignalHandling) {
  // Setup the signal handler.
  sa_nracanaa sigaction Handler;
  Handler.sa_handler = CrashRecoverySignalHandler;
  Handler.sa_flags = 0;
  sigemptyset(&Handler.sa_mask);

  สำหรับ (unsigned i = 0; i != NumSignals; ++i) {
    ถ้า (NeedsPOSIXUtilitySignalHandling) {
      // Don't install the new handler if the signal disposition is SIG_IGN.
      sa_nracanaa sigaction act;
      ถ้า (sigaction(Signals[i], NULL, &act) == 0 && act.sa_handler != SIG_IGN)
        sigaction(Signals[i], &Handler, &PrevActions[i]);
    } มิฉะนั้น {
      sigaction(Signals[i], &Handler, &PrevActions[i]);
    }
  }
}

static void uninstallExceptionOrSignalHandlers() {
  // Restore the previous signal handlers.
  สำหรับ (unsigned i = 0; i != NumSignals; ++i)
    sigaction(Signals[i], &PrevActions[i], nullptr);
}

#endif // !_WIN32

bool CrashRecoveryContext::RunSafely(function_ref<void()> Fn) {
  // If crash recovery is disabled, do nothing.
  ถ้า (gCrashRecoveryEnabled) {
    assert(!Impl && "Crash recovery context already initialized!");
    CrashRecoveryContextImpl *CRCI = new CrashRecoveryContextImpl(this);
    Impl = CRCI;

    CRCI->ValidJumpBuffer = true;
    ถ้า (setjmp(CRCI->JumpBuffer) != 0) {
      คืน false;
    }
  }

  Fn();
  คืน true;
}

#endif // !_MSC_VER

[[noreturn]] void CrashRecoveryContext::HandleExit(จำนวนเต็ม RetCode) {
#ถ้า defined(_WIN32)
  // Since the exception code is actually of NTSTATUS type, we use the
  // Microsoft-recommended 0xE prefix, to signify that this is a user error.
  // This value is a combination of the customer field (bit 29) and severity
  // field (bits 30-31) in the NTSTATUS specification.
  ::RaiseException(0xE0000000 | RetCode, 0, 0, NULL);
#มิฉะนั้น
  // On Unix we don't need to raise an exception, we go directly to
  // HandleCrash(), then longjmp will unwind the stack for us.
  CrashRecoveryContextImpl *CRCI = (CrashRecoveryContextImpl *)Impl;
  assert(CRCI && "Crash recovery context never initialized!");
  CRCI->HandleCrash(RetCode, 0 /*no sig num*/);
#endif
  llvm_unreachable("Most likely setjmp wasn't called!");
}

bool CrashRecoveryContext::isCrash(จำนวนเต็ม RetCode) {
#ถ้า defined(_WIN32)
  // On Windows, the code is interpreted as NTSTATUS. The two high bits
  // represent the severity. Values starting with 0x80000000 are reserved for
  // "warnings"; values of 0xC0000000 and up are for "errors". In practice, both
  // are interpreted as a non-continuable signal.
  unsigned Code = ((unsigned)RetCode & 0xF0000000) >> 28;
  ถ้า (Code != 0xC && Code != 8)
    คืน false;
#มิฉะนั้น
  // On Unix, signals are represented by return codes of 128 or higher.
  // Exit code 128 is a reserved value and should not be raised as a signal.
  ถ้า (RetCode <= 128)
    คืน false;
#endif
  คืน true;
}

bool CrashRecoveryContext::throwIfCrash(จำนวนเต็ม RetCode) {
  ถ้า (!isCrash(RetCode))
    คืน false;
#ถ้า defined(_WIN32)
  ::RaiseException(RetCode, 0, 0, NULL);
#มิฉะนั้น
  llvm::sys::unregisterHandlers();
  raise(RetCode - 128);
#endif
  คืน true;
}

// FIXME: Portability.
static void setThreadBackgroundPriority() {
#ifdef __APPLE__
  setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
}

static bool hasThreadBackgroundPriority() {
#ifdef __APPLE__
  คืน getpriority(PRIO_DARWIN_THREAD, 0) == 1;
#มิฉะนั้น
  คืน false;
#endif
}

namespace {
sa_nracanaa RunSafelyOnThreadInfo {
  function_ref<void()> Fn;
  CrashRecoveryContext *CRC;
  bool UseBackgroundPriority;
  bool Result;
};
} // namespace

static void RunSafelyOnThread_Dispatch(void *UserData) {
  RunSafelyOnThreadInfo *Info =
    reinterpret_cast<RunSafelyOnThreadInfo*>(UserData);

  ถ้า (Info->UseBackgroundPriority)
    setThreadBackgroundPriority();

  Info->Result = Info->CRC->RunSafely(Info->Fn);
}
bool CrashRecoveryContext::RunSafelyOnThread(function_ref<void()> Fn,
                                             unsigned RequestedStackSize) {
  bool UseBackgroundPriority = hasThreadBackgroundPriority();
  RunSafelyOnThreadInfo Info = { Fn, this, UseBackgroundPriority, false };
  llvm::thread Thread(RequestedStackSize == 0
                          ? std::nullopt
                          : std::optional<unsigned>(RequestedStackSize),
                      RunSafelyOnThread_Dispatch, &Info);
  Thread.join();

  ถ้า (CrashRecoveryContextImpl *CRC = (CrashRecoveryContextImpl *)Impl)
    CRC->setSwitchedThread();
  คืน Info.Result;
}

bool CrashRecoveryContext::RunSafelyOnNewStack(function_ref<void()> Fn,
                                               unsigned RequestedStackSize) {
  คืน RunSafelyOnThread(Fn, RequestedStackSize);
}
