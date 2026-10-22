//===- lib/Support/ErrorHandling.cpp - Callbacks for errors ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an API used to indicate fatal error conditions.  Non-fatal
// errors (most of them) should be handled through LLVMContext.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm-c/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errno.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/WindowsError.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <new>

#ਜੇ defined(HAVE_UNISTD_H)
# include <unistd.h>
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ fatal_error_handler_t ErrorHandler = ਖਾਲੀ_ਸੂਚਕ;
ਸਥਿਰ ਖਾਲੀ *Error_Handler_User_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;

ਸਥਿਰ fatal_error_handler_t BadAllocErrorHandler = ਖਾਲੀ_ਸੂਚਕ;
ਸਥਿਰ ਖਾਲੀ *Bad_Alloc_Error_Handler_User_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;

#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
// Mutexes to synchronize installing error handlers and calling error handlers.
// Do not use ManagedStatic, or that may allocate memory while attempting to
// report an OOM.
//
// This usage of std::mutex has to be conditionalized behind ifdefs because
// of this script:
//   compiler-rt/lib/sanitizer_common/symbolizer/scripts/build_symbolizer.sh
// That script attempts to statically link the LLVM symbolizer library with the
// STL and hide all of its symbols with 'opt -internalize'. To reduce size, it
// cuts out the threading portions of the hermetic copy of libc++ that it
// builds. We can remove these ifdefs if that script goes away.
ਸਥਿਰ std::mutex ErrorHandlerMutex;
ਸਥਿਰ std::mutex BadAllocErrorHandlerMutex;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਤਾਰਕਿਕ write_retry(ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *buf, size_t ਗਿਣਤੀ_2) {
  ਜਦੋਂ (ਗਿਣਤੀ_2 > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::ਲਿਖ, fd, buf, ਗਿਣਤੀ_2);
    ਜੇ (written <= 0)
      ਵਾਪਸ ਝੂਠ;
    buf += written;
    ਗਿਣਤੀ_2 -= written;
  }
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       ਖਾਲੀ *user_ਅੰਕੜੇ) {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(ErrorHandlerMutex);
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
}

ਖਾਲੀ llvm::remove_fatal_error_handler() {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(ErrorHandlerMutex);
#ਜੇ_ਅੰਤ
  ErrorHandler = ਖਾਲੀ_ਸੂਚਕ;
  Error_Handler_User_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;
}

ਖਾਲੀ llvm::report_fatal_error(ਅਚਲ ਅੱਖਰ *Reason, ਤਾਰਕਿਕ GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

ਖਾਲੀ llvm::report_fatal_error(StringRef Reason, ਤਾਰਕਿਕ GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

ਖਾਲੀ llvm::report_fatal_error(ਅਚਲ Twine &Reason, ਤਾਰਕਿਕ GenCrashDiag) {
  llvm::fatal_error_handler_t handler = ਖਾਲੀ_ਸੂਚਕ;
  ਖਾਲੀ* handler_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
    std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(ErrorHandlerMutex);
#ਜੇ_ਅੰਤ
    handler = ErrorHandler;
    handler_ਅੰਕੜੇ = Error_Handler_User_ਅੰਕੜੇ;
  }

  ਜੇ (handler) {
    handler(handler_ਅੰਕੜੇ, Reason.str().c_str(), GenCrashDiag);
  } ਨਹੀਂਤਾਂ {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_ਸਦਿਸ਼<ਅੱਖਰ, 64> ਬਫਰ;
    raw_svector_ostream OS(ਬਫਰ);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef ਸੁਨੇਹਾ_Str = OS.str();
    write_retry(2, ਸੁਨੇਹਾ_Str.ਅੰਕੜੇ(), ਸੁਨੇਹਾ_Str.ਮਾਪ_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::ਲੜੀਵਾਰ_ਰੁਕਾਵਟ_Handlers();

  ਜੇ (GenCrashDiag)
    abort();
  ਨਹੀਂਤਾਂ
    ਬਾਹਰ_ਨਿਕਲੋ(1);
}

ਖਾਲੀ llvm::report_Fatal_ਅੰਦਰੂਨੀ_Error(ਅਚਲ ਅੱਖਰ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਸੱਚ);
}
ਖਾਲੀ llvm::report_Fatal_ਅੰਦਰੂਨੀ_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਸੱਚ);
}
ਖਾਲੀ llvm::report_Fatal_ਅੰਦਰੂਨੀ_Error(ਅਚਲ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਸੱਚ);
}
ਖਾਲੀ llvm::reportFatalUsageError(ਅਚਲ ਅੱਖਰ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਝੂਠ);
}
ਖਾਲੀ llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਝੂਠ);
}
ਖਾਲੀ llvm::reportFatalUsageError(ਅਚਲ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ਝੂਠ);
}

ਖਾਲੀ llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           ਖਾਲੀ *user_ਅੰਕੜੇ) {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(BadAllocErrorHandlerMutex);
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
}

ਖਾਲੀ llvm::remove_bad_alloc_error_handler() {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(BadAllocErrorHandlerMutex);
#ਜੇ_ਅੰਤ
  BadAllocErrorHandler = ਖਾਲੀ_ਸੂਚਕ;
  Bad_Alloc_Error_Handler_User_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;
}

ਖਾਲੀ llvm::report_bad_alloc_error(ਅਚਲ ਅੱਖਰ *Reason, ਤਾਰਕਿਕ GenCrashDiag) {
  fatal_error_handler_t Handler = ਖਾਲੀ_ਸੂਚਕ;
  ਖਾਲੀ *Handler_ਅੰਕੜੇ = ਖਾਲੀ_ਸੂਚਕ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 1
    std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(BadAllocErrorHandlerMutex);
#ਜੇ_ਅੰਤ
    Handler = BadAllocErrorHandler;
    Handler_ਅੰਕੜੇ = Bad_Alloc_Error_Handler_User_ਅੰਕੜੇ;
  }

  ਜੇ (Handler) {
    Handler(Handler_ਅੰਕੜੇ, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  ਸੁੱਟੋ std::bad_alloc();
#ਨਹੀਂਤਾਂ
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ਅਚਲ ਅੱਖਰ *OOMMessage = "LLVM ERROR: out of memory\n";
  ਅਚਲ ਅੱਖਰ *Newline = "\n";
  write_retry(2, OOMMessage, ਲੜੀ_ਲੰਬਾਈ(OOMMessage));
  write_retry(2, Reason, ਲੜੀ_ਲੰਬਾਈ(Reason));
  write_retry(2, Newline, ਲੜੀ_ਲੰਬਾਈ(Newline));
  abort();
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
ਖਾਲੀ llvm::install_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler() {
}
#ਨਹੀਂਤਾਂ
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
ਸਥਿਰ ਖਾਲੀ ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
ਖਾਲੀ llvm::install_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler() {
  std::new_handler old = std::ਸੈੱਟ_new_handler(ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler);
  (ਖਾਲੀ)old;
  ਪ੍ਰਤਿਜ੍ਞਾ((old == ਖਾਲੀ_ਸੂਚਕ || old == ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler) &&
         "new-handler already installed");
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ llvm::llvm_unreachable_ਅੰਦਰੂਨੀ(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਅਚਲ ਅੱਖਰ *file,
                                     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  ਜੇ (ਸੁਨੇਹਾ)
    dbgs() << ਸੁਨੇਹਾ << "\n";
  dbgs() << "UNREACHABLE executed";
  ਜੇ (file)
    dbgs() << " at " << file << ":" << ਸਤਰ_2;
  dbgs() << "!\n";
  abort();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ bindingsErrorHandler(ਖਾਲੀ *user_ਅੰਕੜੇ, ਅਚਲ ਅੱਖਰ *reason,
                                 ਤਾਰਕਿਕ gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_ਅੰਕੜੇ);
  handler(reason);
}

ਖਾਲੀ LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<ਖਾਲੀ *>(Handler));
}

ਖਾਲੀ LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32_NO_ਸਥਿਤੀ
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ WIN32_NO_ਸਥਿਤੀ
#ਅਨ੍ਤਰ੍ਭਾਵ <ntstatus.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <winerror.h>

// This is equivalent to NtCurrentTeb()->LastStatusValue, but the public
// _TEB definition does not expose the LastStatusValue field directly.
// Avoid offsetting into this structure by calling RtlGetLastNtStatus
// from ntdll.dll.
//
// The return of this function will roughly match that of
// GetLastError, but this lower level API disambiguates some cases
// that GetLastError does not.
//
// For more information, see:
// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/pebteb/teb/index.htm
// https://github.com/llvm/llvm-project/issues/89137
ਬਾਹਰੀ "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_ਆਖਰੀ_Nt_ਸਥਿਤੀ();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_ਕੋਡ llvm::map_ਆਖਰੀ_Windows_Error() {
  ਬਿਨਾਨਿਸ਼ਾਨੀ EV = ::Get_ਆਖਰੀ_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  ਜੇ (EV == ERROR_ACCESS_DENIED) {
    llvm::errc ਕੋਡ_2 = Rtl_Get_ਆਖਰੀ_Nt_ਸਥਿਤੀ() == ਸਥਿਤੀ_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    ਵਾਪਸ make_error_ਕੋਡ(ਕੋਡ_2);
  }
  ਵਾਪਸ mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAP_ERR_ਮੰਜ਼ਿਲ_COND(x, y)                                                  \
  ਹਾਲਤ x:                                                                      \
    ਵਾਪਸ make_error_ਕੋਡ(errc::y)

std::error_ਕੋਡ llvm::mapWindowsError(ਬਿਨਾਨਿਸ਼ਾਨੀ EV) {
  ਚੋਣ (EV) {
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_ਬਫਰ_ਉਛਾਲ, filename_too_long);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_DEVICE_ਵਿੱਚ_USE, device_or_resource_busy);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_DIR_NOT_ਖਾਲੀ_ਥਾਵਾਂ, directory_not_ਖਾਲੀ_ਥਾਵਾਂ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_DIRECTORY, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_DISK_ਭਰਿਆ, no_space_on_device);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_FILE_NOT_ਮਿਲਿਆ, no_such_file_or_directory);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_HANDLE_DISK_ਭਰਿਆ, no_space_on_device);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_HANDLE, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_NAME, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_INVALID_PARAMETER, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_ਤਾਲਾ_VIOLATION, no_ਤਾਲਾ_ਉਪਲਬਧ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_LOCKED, no_ਤਾਲਾ_ਉਪਲਬਧ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_NEGATIVE_SEEK, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_NOT_ENOUGH_ਸਮ੍ਰਿਤੀ, not_enough_ਸਮ੍ਰਿਤੀ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_OUTOFMEMORY, not_enough_ਸਮ੍ਰਿਤੀ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_ਰਸਤਾ_NOT_ਮਿਲਿਆ, no_such_file_or_directory);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_REPARSE_ਟੈਗ_INVALID, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_SEEK, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEACCES, permission_denied);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEFAULT, bad_ਪਤਾ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEINTR, interrupted);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEINVAL, invalid_ਦਲੀਲ);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_ਮੰਜ਼ਿਲ_COND(WSAENAMETOOLONG, filename_too_long);
  ਮੂਲ:
    ਵਾਪਸ std::error_ਕੋਡ(EV, std::system_category());
  }
}

#ਜੇ_ਅੰਤ
