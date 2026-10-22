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

#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm-c/ErrorHandling.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/Errno.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/Threading.h"
#অন্তর্ভাব "llvm/Support/WindowsError.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdlib>
#অন্তর্ভাব <mutex>
#অন্তর্ভাব <new>

#যদি defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#যদি defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#endif

using নামাকাশ llvm;

স্থির fatal_error_handler_t ErrorHandler = nullptr;
স্থির শূন্য *ErrorHandlerUserData = nullptr;

স্থির fatal_error_handler_t BadAllocErrorHandler = nullptr;
স্থির শূন্য *BadAllocErrorHandlerUserData = nullptr;

#যদি LLVM_ENABLE_THREADS == 1
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
স্থির std::mutex ErrorHandlerMutex;
স্থির std::mutex BadAllocErrorHandlerMutex;
#endif

স্থির তার্কিক write_retry(পূর্ণাঙ্ক fd, ধ্রুব বর্ণ *buf, size_t গুনতি) {
  যাবত্ (গুনতি > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::লিখ, fd, buf, গুনতি);
    যদি (written <= 0)
      প্রত্যাগম অসত্য;
    buf += written;
    গুনতি -= written;
  }
  প্রত্যাগম সত্য;
}

শূন্য llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       শূন্য *user_data) {
#যদি LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  প্রতিজ্ঞা(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  ErrorHandlerUserData = user_data;
}

শূন্য llvm::remove_fatal_error_handler() {
#যদি LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  ErrorHandler = nullptr;
  ErrorHandlerUserData = nullptr;
}

শূন্য llvm::report_fatal_error(ধ্রুব বর্ণ *Reason, তার্কিক GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

শূন্য llvm::report_fatal_error(StringRef Reason, তার্কিক GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

শূন্য llvm::report_fatal_error(ধ্রুব Twine &Reason, তার্কিক GenCrashDiag) {
  llvm::fatal_error_handler_t handler = nullptr;
  শূন্য* handlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#যদি LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
    handler = ErrorHandler;
    handlerData = ErrorHandlerUserData;
  }

  যদি (handler) {
    handler(handlerData, Reason.str().c_str(), GenCrashDiag);
  } অন্যথা {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    SmallVector<বর্ণ, 64> Buffer;
    raw_svector_ostream OS(Buffer);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef MessageStr = OS.str();
    write_retry(2, MessageStr.data(), MessageStr.size());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::RunInterruptHandlers();

  যদি (GenCrashDiag)
    abort();
  অন্যথা
    exit(1);
}

শূন্য llvm::reportFatalInternalError(ধ্রুব বর্ণ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/সত্য);
}
শূন্য llvm::reportFatalInternalError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/সত্য);
}
শূন্য llvm::reportFatalInternalError(ধ্রুব Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/সত্য);
}
শূন্য llvm::reportFatalUsageError(ধ্রুব বর্ণ *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/অসত্য);
}
শূন্য llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/অসত্য);
}
শূন্য llvm::reportFatalUsageError(ধ্রুব Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/অসত্য);
}

শূন্য llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           শূন্য *user_data) {
#যদি LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  প্রতিজ্ঞা(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  BadAllocErrorHandlerUserData = user_data;
}

শূন্য llvm::remove_bad_alloc_error_handler() {
#যদি LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  BadAllocErrorHandler = nullptr;
  BadAllocErrorHandlerUserData = nullptr;
}

শূন্য llvm::report_bad_alloc_error(ধ্রুব বর্ণ *Reason, তার্কিক GenCrashDiag) {
  fatal_error_handler_t Handler = nullptr;
  শূন্য *HandlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#যদি LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
    Handler = BadAllocErrorHandler;
    HandlerData = BadAllocErrorHandlerUserData;
  }

  যদি (Handler) {
    Handler(HandlerData, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#ifdef LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  ক্ষিপ std::bad_alloc();
#অন্যথা
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ধ্রুব বর্ণ *OOMMessage = "LLVM ERROR: out of memory\n";
  ধ্রুব বর্ণ *Newline = "\n";
  write_retry(2, OOMMessage, strlen(OOMMessage));
  write_retry(2, Reason, strlen(Reason));
  write_retry(2, Newline, strlen(Newline));
  abort();
#endif
}

#ifdef LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
শূন্য llvm::install_out_of_memory_new_handler() {
}
#অন্যথা
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
স্থির শূন্য out_of_memory_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
শূন্য llvm::install_out_of_memory_new_handler() {
  std::new_handler old = std::set_new_handler(out_of_memory_new_handler);
  (শূন্য)old;
  প্রতিজ্ঞা((old == nullptr || old == out_of_memory_new_handler) &&
         "new-handler already installed");
}
#endif

শূন্য llvm::llvm_unreachable_internal(ধ্রুব বর্ণ *বার্তা, ধ্রুব বর্ণ *file,
                                     unsigned সারি_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  যদি (বার্তা)
    dbgs() << বার্তা << "\n";
  dbgs() << "UNREACHABLE executed";
  যদি (file)
    dbgs() << " at " << file << ":" << সারি_2;
  dbgs() << "!\n";
  abort();
#ifdef LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#endif
}

স্থির শূন্য bindingsErrorHandler(শূন্য *user_data, ধ্রুব বর্ণ *reason,
                                 তার্কিক gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_data);
  handler(reason);
}

শূন্য LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<শূন্য *>(Handler));
}

শূন্য LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#ifdef _WIN32

#define WIN32_NO_STATUS
#অন্তর্ভাব "llvm/Support/Windows/WindowsSupport.h"
#undef WIN32_NO_STATUS
#অন্তর্ভাব <ntstatus.h>
#অন্তর্ভাব <winerror.h>

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
বাহ্য "C" NTSYSAPI NTSTATUS NTAPI RtlGetLastNtStatus();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_code llvm::mapLastWindowsError() {
  unsigned EV = ::GetLastError();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  যদি (EV == ERROR_ACCESS_DENIED) {
    llvm::errc code = RtlGetLastNtStatus() == STATUS_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    প্রত্যাগম make_error_code(code);
  }
  প্রত্যাগম mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#define MAP_ERR_TO_COND(x, y)                                                  \
  পক্ষ x:                                                                      \
    প্রত্যাগম make_error_code(errc::y)

std::error_code llvm::mapWindowsError(unsigned EV) {
  বিকল্প (EV) {
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
  স্বভাব:
    প্রত্যাগম std::error_code(EV, std::system_category());
  }
}

#endif
