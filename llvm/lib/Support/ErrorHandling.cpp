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

#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm-c/ErrorHandling.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/Errno.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/Threading.h"
#அந்தர்பாவ "llvm/Support/WindowsError.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdlib>
#அந்தர்பாவ <mutex>
#அந்தர்பாவ <new>

#எனில் defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#எனில் defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#endif

using நாமாகாஶ llvm;

நிலையான fatal_error_handler_t ErrorHandler = nullptr;
நிலையான வெறுமை *ErrorHandlerUserData = nullptr;

நிலையான fatal_error_handler_t BadAllocErrorHandler = nullptr;
நிலையான வெறுமை *BadAllocErrorHandlerUserData = nullptr;

#எனில் LLVM_ENABLE_THREADS == 1
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
நிலையான std::mutex ErrorHandlerMutex;
நிலையான std::mutex BadAllocErrorHandlerMutex;
#endif

நிலையான தருக்க write_retry(முழு fd, மாறிலி எழுத்து *buf, size_t மொத்தம்) {
  போது (மொத்தம் > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::லிக, fd, buf, மொத்தம்);
    எனில் (written <= 0)
      திரும்பு பொய்;
    buf += written;
    மொத்தம் -= written;
  }
  திரும்பு உண்மை;
}

வெறுமை llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       வெறுமை *user_data) {
#எனில் LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  ப்ரதிஜ்ஞா(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  ErrorHandlerUserData = user_data;
}

வெறுமை llvm::remove_fatal_error_handler() {
#எனில் LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  ErrorHandler = nullptr;
  ErrorHandlerUserData = nullptr;
}

வெறுமை llvm::report_fatal_error(மாறிலி எழுத்து *Reason, தருக்க GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

வெறுமை llvm::report_fatal_error(StringRef Reason, தருக்க GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

வெறுமை llvm::report_fatal_error(மாறிலி Twine &Reason, தருக்க GenCrashDiag) {
  llvm::fatal_error_handler_t handler = nullptr;
  வெறுமை* handlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#எனில் LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
    handler = ErrorHandler;
    handlerData = ErrorHandlerUserData;
  }

  எனில் (handler) {
    handler(handlerData, Reason.str().c_str(), GenCrashDiag);
  } இல்லையெனில் {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    SmallVector<எழுத்து, 64> Buffer;
    raw_svector_ostream OS(Buffer);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef MessageStr = OS.str();
    write_retry(2, MessageStr.data(), MessageStr.size());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::RunInterruptHandlers();

  எனில் (GenCrashDiag)
    abort();
  இல்லையெனில்
    exit(1);
}

வெறுமை llvm::reportFatalInternalError(மாறிலி எழுத்து *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/உண்மை);
}
வெறுமை llvm::reportFatalInternalError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/உண்மை);
}
வெறுமை llvm::reportFatalInternalError(மாறிலி Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/உண்மை);
}
வெறுமை llvm::reportFatalUsageError(மாறிலி எழுத்து *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/பொய்);
}
வெறுமை llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/பொய்);
}
வெறுமை llvm::reportFatalUsageError(மாறிலி Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/பொய்);
}

வெறுமை llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           வெறுமை *user_data) {
#எனில் LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  ப்ரதிஜ்ஞா(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  BadAllocErrorHandlerUserData = user_data;
}

வெறுமை llvm::remove_bad_alloc_error_handler() {
#எனில் LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  BadAllocErrorHandler = nullptr;
  BadAllocErrorHandlerUserData = nullptr;
}

வெறுமை llvm::report_bad_alloc_error(மாறிலி எழுத்து *Reason, தருக்க GenCrashDiag) {
  fatal_error_handler_t Handler = nullptr;
  வெறுமை *HandlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#எனில் LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
    Handler = BadAllocErrorHandler;
    HandlerData = BadAllocErrorHandlerUserData;
  }

  எனில் (Handler) {
    Handler(HandlerData, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#ifdef LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  எறி std::bad_alloc();
#இல்லையெனில்
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  மாறிலி எழுத்து *OOMMessage = "LLVM ERROR: out of memory\n";
  மாறிலி எழுத்து *Newline = "\n";
  write_retry(2, OOMMessage, strlen(OOMMessage));
  write_retry(2, Reason, strlen(Reason));
  write_retry(2, Newline, strlen(Newline));
  abort();
#endif
}

#ifdef LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
வெறுமை llvm::install_out_of_memory_new_handler() {
}
#இல்லையெனில்
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
நிலையான வெறுமை out_of_memory_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
வெறுமை llvm::install_out_of_memory_new_handler() {
  std::new_handler old = std::set_new_handler(out_of_memory_new_handler);
  (வெறுமை)old;
  ப்ரதிஜ்ஞா((old == nullptr || old == out_of_memory_new_handler) &&
         "new-handler already installed");
}
#endif

வெறுமை llvm::llvm_unreachable_internal(மாறிலி எழுத்து *செய்தி, மாறிலி எழுத்து *file,
                                     unsigned வரி_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  எனில் (செய்தி)
    dbgs() << செய்தி << "\n";
  dbgs() << "UNREACHABLE executed";
  எனில் (file)
    dbgs() << " at " << file << ":" << வரி_2;
  dbgs() << "!\n";
  abort();
#ifdef LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#endif
}

நிலையான வெறுமை bindingsErrorHandler(வெறுமை *user_data, மாறிலி எழுத்து *reason,
                                 தருக்க gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_data);
  handler(reason);
}

வெறுமை LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<வெறுமை *>(Handler));
}

வெறுமை LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#ifdef _WIN32

#define WIN32_NO_STATUS
#அந்தர்பாவ "llvm/Support/Windows/WindowsSupport.h"
#undef WIN32_NO_STATUS
#அந்தர்பாவ <ntstatus.h>
#அந்தர்பாவ <winerror.h>

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
வெளி "C" NTSYSAPI NTSTATUS NTAPI RtlGetLastNtStatus();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_code llvm::mapLastWindowsError() {
  unsigned EV = ::GetLastError();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  எனில் (EV == ERROR_ACCESS_DENIED) {
    llvm::errc code = RtlGetLastNtStatus() == STATUS_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    திரும்பு make_error_code(code);
  }
  திரும்பு mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#define MAP_ERR_TO_COND(x, y)                                                  \
  நிலை x:                                                                      \
    திரும்பு make_error_code(errc::y)

std::error_code llvm::mapWindowsError(unsigned EV) {
  தேர்வு (EV) {
    MAP_ERR_TO_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_TO_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_TO_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_TO_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_TO_COND(ERROR_BUFFER_OVERFLOW, filename_too_long);
    MAP_ERR_TO_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_TO_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_TO_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_TO_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_TO_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_TO_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_TO_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_TO_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_TO_COND(ERROR_DEVICE_IN_USE, device_or_resource_busy);
    MAP_ERR_TO_COND(ERROR_DIR_NOT_EMPTY, directory_not_empty);
    MAP_ERR_TO_COND(ERROR_DIRECTORY, invalid_argument);
    MAP_ERR_TO_COND(ERROR_DISK_FULL, no_space_on_device);
    MAP_ERR_TO_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_TO_COND(ERROR_FILE_NOT_FOUND, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_HANDLE_DISK_FULL, no_space_on_device);
    MAP_ERR_TO_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_TO_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_TO_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_TO_COND(ERROR_INVALID_HANDLE, invalid_argument);
    MAP_ERR_TO_COND(ERROR_INVALID_NAME, invalid_argument);
    MAP_ERR_TO_COND(ERROR_INVALID_PARAMETER, invalid_argument);
    MAP_ERR_TO_COND(ERROR_LOCK_VIOLATION, no_lock_available);
    MAP_ERR_TO_COND(ERROR_LOCKED, no_lock_available);
    MAP_ERR_TO_COND(ERROR_NEGATIVE_SEEK, invalid_argument);
    MAP_ERR_TO_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_TO_COND(ERROR_NOT_ENOUGH_MEMORY, not_enough_memory);
    MAP_ERR_TO_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_TO_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_TO_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_TO_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_TO_COND(ERROR_OUTOFMEMORY, not_enough_memory);
    MAP_ERR_TO_COND(ERROR_PATH_NOT_FOUND, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_TO_COND(ERROR_REPARSE_TAG_INVALID, invalid_argument);
    MAP_ERR_TO_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_TO_COND(ERROR_SEEK, io_error);
    MAP_ERR_TO_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_TO_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_TO_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_TO_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_TO_COND(WSAEACCES, permission_denied);
    MAP_ERR_TO_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_TO_COND(WSAEFAULT, bad_address);
    MAP_ERR_TO_COND(WSAEINTR, interrupted);
    MAP_ERR_TO_COND(WSAEINVAL, invalid_argument);
    MAP_ERR_TO_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_TO_COND(WSAENAMETOOLONG, filename_too_long);
  இயல்பு:
    திரும்பு std::error_code(EV, std::system_category());
  }
}

#endif
