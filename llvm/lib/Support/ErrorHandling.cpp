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

#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm-c/ErrorHandling.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/Errno.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/Threading.h"
#અન્તર્ભાવ "llvm/Support/WindowsError.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdlib>
#અન્તર્ભાવ <mutex>
#અન્તર્ભાવ <new>

#જો defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#જો defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#endif

using નામાકાશ llvm;

સ્થિર fatal_error_handler_t ErrorHandler = nullptr;
સ્થિર શૂન્ય *ErrorHandlerUserData = nullptr;

સ્થિર fatal_error_handler_t BadAllocErrorHandler = nullptr;
સ્થિર શૂન્ય *BadAllocErrorHandlerUserData = nullptr;

#જો LLVM_ENABLE_THREADS == 1
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
સ્થિર std::mutex ErrorHandlerMutex;
સ્થિર std::mutex BadAllocErrorHandlerMutex;
#endif

સ્થિર તાર્કિક write_retry(પૂર્ણાંક fd, અચલ અક્ષર *buf, size_t ગણતરી) {
  જ્યારે (ગણતરી > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::લિખ, fd, buf, ગણતરી);
    જો (written <= 0)
      પાછા ખોટું;
    buf += written;
    ગણતરી -= written;
  }
  પાછા સાચું;
}

શૂન્ય llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       શૂન્ય *user_data) {
#જો LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  પ્રતિજ્ઞા(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  ErrorHandlerUserData = user_data;
}

શૂન્ય llvm::remove_fatal_error_handler() {
#જો LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
  ErrorHandler = nullptr;
  ErrorHandlerUserData = nullptr;
}

શૂન્ય llvm::report_fatal_error(અચલ અક્ષર *Reason, તાર્કિક GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

શૂન્ય llvm::report_fatal_error(StringRef Reason, તાર્કિક GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

શૂન્ય llvm::report_fatal_error(અચલ Twine &Reason, તાર્કિક GenCrashDiag) {
  llvm::fatal_error_handler_t handler = nullptr;
  શૂન્ય* handlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#જો LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#endif
    handler = ErrorHandler;
    handlerData = ErrorHandlerUserData;
  }

  જો (handler) {
    handler(handlerData, Reason.str().c_str(), GenCrashDiag);
  } નહીંતર {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    SmallVector<અક્ષર, 64> Buffer;
    raw_svector_ostream OS(Buffer);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef MessageStr = OS.str();
    write_retry(2, MessageStr.data(), MessageStr.size());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::RunInterruptHandlers();

  જો (GenCrashDiag)
    abort();
  નહીંતર
    exit(1);
}

શૂન્ય llvm::reportFatalInternalError(અચલ અક્ષર *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/સાચું);
}
શૂન્ય llvm::reportFatalInternalError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/સાચું);
}
શૂન્ય llvm::reportFatalInternalError(અચલ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/સાચું);
}
શૂન્ય llvm::reportFatalUsageError(અચલ અક્ષર *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ખોટું);
}
શૂન્ય llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ખોટું);
}
શૂન્ય llvm::reportFatalUsageError(અચલ Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/ખોટું);
}

શૂન્ય llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           શૂન્ય *user_data) {
#જો LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  પ્રતિજ્ઞા(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  BadAllocErrorHandlerUserData = user_data;
}

શૂન્ય llvm::remove_bad_alloc_error_handler() {
#જો LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
  BadAllocErrorHandler = nullptr;
  BadAllocErrorHandlerUserData = nullptr;
}

શૂન્ય llvm::report_bad_alloc_error(અચલ અક્ષર *Reason, તાર્કિક GenCrashDiag) {
  fatal_error_handler_t Handler = nullptr;
  શૂન્ય *HandlerData = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#જો LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#endif
    Handler = BadAllocErrorHandler;
    HandlerData = BadAllocErrorHandlerUserData;
  }

  જો (Handler) {
    Handler(HandlerData, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#ifdef LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  ફેંકો std::bad_alloc();
#નહીંતર
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  અચલ અક્ષર *OOMMessage = "LLVM ERROR: out of memory\n";
  અચલ અક્ષર *Newline = "\n";
  write_retry(2, OOMMessage, strlen(OOMMessage));
  write_retry(2, Reason, strlen(Reason));
  write_retry(2, Newline, strlen(Newline));
  abort();
#endif
}

#ifdef LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
શૂન્ય llvm::install_out_of_memory_new_handler() {
}
#નહીંતર
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
સ્થિર શૂન્ય out_of_memory_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
શૂન્ય llvm::install_out_of_memory_new_handler() {
  std::new_handler old = std::set_new_handler(out_of_memory_new_handler);
  (શૂન્ય)old;
  પ્રતિજ્ઞા((old == nullptr || old == out_of_memory_new_handler) &&
         "new-handler already installed");
}
#endif

શૂન્ય llvm::llvm_unreachable_internal(અચલ અક્ષર *સંદેશ, અચલ અક્ષર *file,
                                     unsigned લીટી_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  જો (સંદેશ)
    dbgs() << સંદેશ << "\n";
  dbgs() << "UNREACHABLE executed";
  જો (file)
    dbgs() << " at " << file << ":" << લીટી_2;
  dbgs() << "!\n";
  abort();
#ifdef LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#endif
}

સ્થિર શૂન્ય bindingsErrorHandler(શૂન્ય *user_data, અચલ અક્ષર *reason,
                                 તાર્કિક gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_data);
  handler(reason);
}

શૂન્ય LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<શૂન્ય *>(Handler));
}

શૂન્ય LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#ifdef _WIN32

#define WIN32_NO_STATUS
#અન્તર્ભાવ "llvm/Support/Windows/WindowsSupport.h"
#undef WIN32_NO_STATUS
#અન્તર્ભાવ <ntstatus.h>
#અન્તર્ભાવ <winerror.h>

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
બાહ્ય "C" NTSYSAPI NTSTATUS NTAPI RtlGetLastNtStatus();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_code llvm::mapLastWindowsError() {
  unsigned EV = ::GetLastError();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  જો (EV == ERROR_ACCESS_DENIED) {
    llvm::errc code = RtlGetLastNtStatus() == STATUS_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    પાછા make_error_code(code);
  }
  પાછા mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#define MAP_ERR_TO_COND(x, y)                                                  \
  સ્થિતિ x:                                                                      \
    પાછા make_error_code(errc::y)

std::error_code llvm::mapWindowsError(unsigned EV) {
  પસંદગી (EV) {
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
  મૂળભૂત:
    પાછા std::error_code(EV, std::system_category());
  }
}

#endif
