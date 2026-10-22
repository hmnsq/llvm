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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm-c/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errno.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/WindowsError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdlib>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <new>

#ಆದರೆ defined(HAVE_UNISTD_H)
# include <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ fatal_error_handler_t ErrorHandler = ಶೂನ್ಯಸೂಚಕ;
ಸ್ಥಿರ ಶೂನ್ಯ *Error_Handler_User_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;

ಸ್ಥಿರ fatal_error_handler_t BadAllocErrorHandler = ಶೂನ್ಯಸೂಚಕ;
ಸ್ಥಿರ ಶೂನ್ಯ *Bad_Alloc_Error_Handler_User_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;

#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
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
ಸ್ಥಿರ std::mutex ErrorHandlerMutex;
ಸ್ಥಿರ std::mutex BadAllocErrorHandlerMutex;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ತಾರ್ಕಿಕ write_retry(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf, size_t ಎಣಿಕೆ_2) {
  ಆಗ (ಎಣಿಕೆ_2 > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::ಲಿಖ, fd, buf, ಎಣಿಕೆ_2);
    ಆದರೆ (written <= 0)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    buf += written;
    ಎಣಿಕೆ_2 -= written;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       ಶೂನ್ಯ *user_ದತ್ತಾಂಶ) {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
  std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(ErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
}

ಶೂನ್ಯ llvm::remove_fatal_error_handler() {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
  std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(ErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
  ErrorHandler = ಶೂನ್ಯಸೂಚಕ;
  Error_Handler_User_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;
}

ಶೂನ್ಯ llvm::report_fatal_error(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Reason, ತಾರ್ಕಿಕ GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

ಶೂನ್ಯ llvm::report_fatal_error(StringRef Reason, ತಾರ್ಕಿಕ GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

ಶೂನ್ಯ llvm::report_fatal_error(ಸ್ಥಿರಾಂಕ Twine &Reason, ತಾರ್ಕಿಕ GenCrashDiag) {
  llvm::fatal_error_handler_t handler = ಶೂನ್ಯಸೂಚಕ;
  ಶೂನ್ಯ* handler_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
    std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(ErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
    handler = ErrorHandler;
    handler_ದತ್ತಾಂಶ = Error_Handler_User_ದತ್ತಾಂಶ;
  }

  ಆದರೆ (handler) {
    handler(handler_ದತ್ತಾಂಶ, Reason.str().c_str(), GenCrashDiag);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_ಸದಿಶ<ಅಕ್ಷರ, 64> ಬಫರ್;
    raw_svector_ostream OS(ಬಫರ್);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef ಸಂದೇಶ_Str = OS.str();
    write_retry(2, ಸಂದೇಶ_Str.ದತ್ತಾಂಶ(), ಸಂದೇಶ_Str.ಗಾತ್ರ_3());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::ಸತತ_ಅಡಚಣೆ_Handlers();

  ಆದರೆ (GenCrashDiag)
    abort();
  ಇಲ್ಲದಿದ್ದರೆ
    ನಿರ್ಗಮಿಸು(1);
}

ಶೂನ್ಯ llvm::report_Fatal_ಆಂತರಿಕ_Error(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ನಿಜ);
}
ಶೂನ್ಯ llvm::report_Fatal_ಆಂತರಿಕ_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ನಿಜ);
}
ಶೂನ್ಯ llvm::report_Fatal_ಆಂತರಿಕ_Error(ಸ್ಥಿರಾಂಕ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ನಿಜ);
}
ಶೂನ್ಯ llvm::reportFatalUsageError(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ಸುಳ್ಳು);
}
ಶೂನ್ಯ llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ಸುಳ್ಳು);
}
ಶೂನ್ಯ llvm::reportFatalUsageError(ಸ್ಥಿರಾಂಕ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ಸುಳ್ಳು);
}

ಶೂನ್ಯ llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           ಶೂನ್ಯ *user_ದತ್ತಾಂಶ) {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
  std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(BadAllocErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
}

ಶೂನ್ಯ llvm::remove_bad_alloc_error_handler() {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
  std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(BadAllocErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
  BadAllocErrorHandler = ಶೂನ್ಯಸೂಚಕ;
  Bad_Alloc_Error_Handler_User_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;
}

ಶೂನ್ಯ llvm::report_bad_alloc_error(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Reason, ತಾರ್ಕಿಕ GenCrashDiag) {
  fatal_error_handler_t Handler = ಶೂನ್ಯಸೂಚಕ;
  ಶೂನ್ಯ *Handler_ದತ್ತಾಂಶ = ಶೂನ್ಯಸೂಚಕ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 1
    std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(BadAllocErrorHandlerMutex);
#ಆದರೆ_ಅಂತ್ಯ
    Handler = BadAllocErrorHandler;
    Handler_ದತ್ತಾಂಶ = Bad_Alloc_Error_Handler_User_ದತ್ತಾಂಶ;
  }

  ಆದರೆ (Handler) {
    Handler(Handler_ದತ್ತಾಂಶ, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  ಎಸೆ std::bad_alloc();
#ಇಲ್ಲದಿದ್ದರೆ
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *OOMMessage = "LLVM ERROR: out of memory\n";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Newline = "\n";
  write_retry(2, OOMMessage, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(OOMMessage));
  write_retry(2, Reason, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(Reason));
  write_retry(2, Newline, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(Newline));
  abort();
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
ಶೂನ್ಯ llvm::install_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler() {
}
#ಇಲ್ಲದಿದ್ದರೆ
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
ಸ್ಥಿರ ಶೂನ್ಯ ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
ಶೂನ್ಯ llvm::install_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler() {
  std::new_handler old = std::ಗಣ_new_handler(ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler);
  (ಶೂನ್ಯ)old;
  ಪ್ರತಿಜ್ಞಾ((old == ಶೂನ್ಯಸೂಚಕ || old == ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler) &&
         "new-handler already installed");
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ llvm::llvm_unreachable_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file,
                                     ಚಿಹ್ನೆರಹಿತ ಸಾಲು_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  ಆದರೆ (ಸಂದೇಶ)
    dbgs() << ಸಂದೇಶ << "\n";
  dbgs() << "UNREACHABLE executed";
  ಆದರೆ (file)
    dbgs() << " at " << file << ":" << ಸಾಲು_2;
  dbgs() << "!\n";
  abort();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ bindingsErrorHandler(ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *reason,
                                 ತಾರ್ಕಿಕ gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_ದತ್ತಾಂಶ);
  handler(reason);
}

ಶೂನ್ಯ LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<ಶೂನ್ಯ *>(Handler));
}

ಶೂನ್ಯ LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32

#ವ್ಯಾಖ್ಯಾನಿಸು WIN32_NO_ಸ್ಥಿತಿ
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು WIN32_NO_ಸ್ಥಿತಿ
#ಅನ್ತರ್ಭಾವ <ntstatus.h>
#ಅನ್ತರ್ಭಾವ <winerror.h>

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
ಬಾಹ್ಯ "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_ಕೊನೆಯ_Nt_ಸ್ಥಿತಿ();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_ಸಂಕೇತ llvm::map_ಕೊನೆಯ_Windows_Error() {
  ಚಿಹ್ನೆರಹಿತ EV = ::Get_ಕೊನೆಯ_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  ಆದರೆ (EV == ERROR_ACCESS_DENIED) {
    llvm::errc ಸಂಕೇತ_3 = Rtl_Get_ಕೊನೆಯ_Nt_ಸ್ಥಿತಿ() == ಸ್ಥಿತಿ_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(ಸಂಕೇತ_3);
  }
  ಹಿಂತಿರುಗಿ mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#ವ್ಯಾಖ್ಯಾನಿಸು MAP_ERR_ಗಮ್ಯ_COND(x, y)                                                  \
  ಸ್ಥಿತಿ x:                                                                      \
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(errc::y)

std::error_ಸಂಕೇತ llvm::mapWindowsError(ಚಿಹ್ನೆರಹಿತ EV) {
  ಆಯ್ಕೆ (EV) {
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_ಬಫರ್_ಅತಿಪ್ರವಾಹ, filename_too_long);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_DEVICE_ಒಳ_USE, device_or_resource_busy);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_DIR_NOT_ಖಾಲಿಗಳು, directory_not_ಖಾಲಿಗಳು);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_DIRECTORY, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_DISK_ತುಂಬಿದ, no_space_on_device);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_FILE_NOT_ಸಿಕ್ಕಿತು, no_such_file_or_directory);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_HANDLE_DISK_ತುಂಬಿದ, no_space_on_device);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_HANDLE, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_NAME, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_INVALID_PARAMETER, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_ಬೀಗ_VIOLATION, no_ಬೀಗ_ಲಭ್ಯ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_LOCKED, no_ಬೀಗ_ಲಭ್ಯ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_NEGATIVE_SEEK, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_NOT_ENOUGH_ಸ್ಮರಣೆ, not_enough_ಸ್ಮರಣೆ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_OUTOFMEMORY, not_enough_ಸ್ಮರಣೆ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_ಮಾರ್ಗ_NOT_ಸಿಕ್ಕಿತು, no_such_file_or_directory);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_REPARSE_ಟ್ಯಾಗ್_INVALID, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_SEEK, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_ಗಮ್ಯ_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEACCES, permission_denied);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEFAULT, bad_ವಿಳಾಸ);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEINTR, interrupted);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEINVAL, invalid_ವಾದ);
    MAP_ERR_ಗಮ್ಯ_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_ಗಮ್ಯ_COND(WSAENAMETOOLONG, filename_too_long);
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(EV, std::system_category());
  }
}

#ಆದರೆ_ಅಂತ್ಯ
