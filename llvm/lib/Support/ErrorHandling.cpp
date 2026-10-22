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

#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm-c/ErrorHandling.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/Errno.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/Threading.h"
#समावेश "llvm/Support/WindowsError.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>
#समावेश <cstdlib>
#समावेश <mutex>
#समावेश <new>

#अगर defined(HAVE_UNISTD_H)
# include <unistd.h>
#पूर्ण_अगर
#अगर defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#पूर्ण_अगर

using namespace llvm;

अटल fatal_error_handler_t ErrorHandler = nullptr;
अटल व्योम *Error_Handler_User_तथ्य = nullptr;

अटल fatal_error_handler_t BadAllocErrorHandler = nullptr;
अटल व्योम *Bad_Alloc_Error_Handler_User_तथ्य = nullptr;

#अगर LLVM_ENABLE_THREADS == 1
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
अटल std::mutex ErrorHandlerMutex;
अटल std::mutex BadAllocErrorHandlerMutex;
#पूर्ण_अगर

अटल bool write_retry(पूर्णांक fd, स्थिर अक्षर *buf, माप_प्रकार गिनती) {
  जबतक (गिनती > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::ग_लिखो, fd, buf, गिनती);
    अगर (written <= 0)
      वापस false;
    buf += written;
    गिनती -= written;
  }
  वापस true;
}

व्योम llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       व्योम *user_तथ्य) {
#अगर LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#पूर्ण_अगर
  निश्चित(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_तथ्य = user_तथ्य;
}

व्योम llvm::remove_fatal_error_handler() {
#अगर LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#पूर्ण_अगर
  ErrorHandler = nullptr;
  Error_Handler_User_तथ्य = nullptr;
}

व्योम llvm::report_fatal_error(स्थिर अक्षर *Reason, bool GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

व्योम llvm::report_fatal_error(StringRef Reason, bool GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

व्योम llvm::report_fatal_error(स्थिर Twine &Reason, bool GenCrashDiag) {
  llvm::fatal_error_handler_t handler = nullptr;
  व्योम* handler_तथ्य = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#अगर LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(ErrorHandlerMutex);
#पूर्ण_अगर
    handler = ErrorHandler;
    handler_तथ्य = Error_Handler_User_तथ्य;
  }

  अगर (handler) {
    handler(handler_तथ्य, Reason.str().c_str(), GenCrashDiag);
  } अन्यथा {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    SmallVector<अक्षर, 64> बफर;
    raw_svector_ostream OS(बफर);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef MessageStr = OS.str();
    write_retry(2, MessageStr.तथ्य(), MessageStr.आकार_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::RunInterruptHandlers();

  अगर (GenCrashDiag)
    पात();
  अन्यथा
    निकास(1);
}

व्योम llvm::reportFatalInternalError(स्थिर अक्षर *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/true);
}
व्योम llvm::reportFatalInternalError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/true);
}
व्योम llvm::reportFatalInternalError(स्थिर Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/true);
}
व्योम llvm::reportFatalUsageError(स्थिर अक्षर *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/false);
}
व्योम llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/false);
}
व्योम llvm::reportFatalUsageError(स्थिर Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/false);
}

व्योम llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           व्योम *user_तथ्य) {
#अगर LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#पूर्ण_अगर
  निश्चित(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_तथ्य = user_तथ्य;
}

व्योम llvm::remove_bad_alloc_error_handler() {
#अगर LLVM_ENABLE_THREADS == 1
  std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#पूर्ण_अगर
  BadAllocErrorHandler = nullptr;
  Bad_Alloc_Error_Handler_User_तथ्य = nullptr;
}

व्योम llvm::report_bad_alloc_error(स्थिर अक्षर *Reason, bool GenCrashDiag) {
  fatal_error_handler_t Handler = nullptr;
  व्योम *Handler_तथ्य = nullptr;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#अगर LLVM_ENABLE_THREADS == 1
    std::lock_guard<std::mutex> Lock(BadAllocErrorHandlerMutex);
#पूर्ण_अगर
    Handler = BadAllocErrorHandler;
    Handler_तथ्य = Bad_Alloc_Error_Handler_User_तथ्य;
  }

  अगर (Handler) {
    Handler(Handler_तथ्य, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#अगर_घोषित LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  फेंको std::bad_alloc();
#अन्यथा
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  स्थिर अक्षर *OOMMessage = "LLVM ERROR: out of memory\n";
  स्थिर अक्षर *Newline = "\n";
  write_retry(2, OOMMessage, म_माप(OOMMessage));
  write_retry(2, Reason, म_माप(Reason));
  write_retry(2, Newline, म_माप(Newline));
  पात();
#पूर्ण_अगर
}

#अगर_घोषित LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
व्योम llvm::install_out_of_memory_new_handler() {
}
#अन्यथा
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
अटल व्योम out_of_memory_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
व्योम llvm::install_out_of_memory_new_handler() {
  std::new_handler old = std::set_new_handler(out_of_memory_new_handler);
  (व्योम)old;
  निश्चित((old == nullptr || old == out_of_memory_new_handler) &&
         "new-handler already installed");
}
#पूर्ण_अगर

व्योम llvm::llvm_unreachable_internal(स्थिर अक्षर *संदेश, स्थिर अक्षर *file,
                                     अचिन्हित पंक्ति_3) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  अगर (संदेश)
    dbgs() << संदेश << "\n";
  dbgs() << "UNREACHABLE executed";
  अगर (file)
    dbgs() << " at " << file << ":" << पंक्ति_3;
  dbgs() << "!\n";
  पात();
#अगर_घोषित LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#पूर्ण_अगर
}

अटल व्योम bindingsErrorHandler(व्योम *user_तथ्य, स्थिर अक्षर *reason,
                                 bool gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_तथ्य);
  handler(reason);
}

व्योम LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<व्योम *>(Handler));
}

व्योम LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#अगर_घोषित _WIN32

#घोषणा WIN32_NO_STATUS
#समावेश "llvm/Support/Windows/WindowsSupport.h"
#अघोषित WIN32_NO_STATUS
#समावेश <ntstatus.h>
#समावेश <winerror.h>

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
बाह्य "C" NTSYSAPI NTSTATUS NTAPI RtlGetLastNtStatus();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_code llvm::mapLastWindowsError() {
  अचिन्हित EV = ::GetLastError();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  अगर (EV == ERROR_ACCESS_DENIED) {
    llvm::errc code = RtlGetLastNtStatus() == STATUS_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    वापस make_error_code(code);
  }
  वापस mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#घोषणा MAP_ERR_TO_COND(x, y)                                                  \
  हाल x:                                                                      \
    वापस make_error_code(errc::y)

std::error_code llvm::mapWindowsError(अचिन्हित EV) {
  चयन (EV) {
    MAP_ERR_TO_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_TO_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_TO_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_TO_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_TO_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_TO_COND(ERROR_बफर_OVERFLOW, filename_too_long);
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
  शेष:
    वापस std::error_code(EV, std::system_category());
  }
}

#पूर्ण_अगर
