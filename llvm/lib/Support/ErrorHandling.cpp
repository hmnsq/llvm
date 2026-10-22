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

#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm-c/ErrorHandling.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/Errno.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/Threading.h"
#شامل "llvm/Support/WindowsError.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cstdlib>
#شامل <mutex>
#شامل <new>

#اگر defined(HAVE_UNISTD_H)
# include <unistd.h>
#اگر_ختم
#اگر defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#اگر_ختم

استعمال نامخلا llvm;

ساکن fatal_error_handler_t ErrorHandler = خالی_اشاریہ;
ساکن خالی *Error_Handler_User_ڈیٹا = خالی_اشاریہ;

ساکن fatal_error_handler_t BadAllocErrorHandler = خالی_اشاریہ;
ساکن خالی *Bad_Alloc_Error_Handler_User_ڈیٹا = خالی_اشاریہ;

#اگر LLVM_ENABLE_دھاگے == 1
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
ساکن std::mutex ErrorHandlerMutex;
ساکن std::mutex BadAllocErrorHandlerMutex;
#اگر_ختم

ساکن منطقی write_retry(صحیح fd, ثابت حرف *buf, size_t گنتی) {
  جب (گنتی > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::اكتب, fd, buf, گنتی);
    اگر (written <= 0)
      واپس جھوٹ;
    buf += written;
    گنتی -= written;
  }
  واپس سچ;
}

خالی llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       خالی *user_ڈیٹا) {
#اگر LLVM_ENABLE_دھاگے == 1
  std::تالا_guard<std::mutex> تالا_3(ErrorHandlerMutex);
#اگر_ختم
  أكد(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_ڈیٹا = user_ڈیٹا;
}

خالی llvm::remove_fatal_error_handler() {
#اگر LLVM_ENABLE_دھاگے == 1
  std::تالا_guard<std::mutex> تالا_3(ErrorHandlerMutex);
#اگر_ختم
  ErrorHandler = خالی_اشاریہ;
  Error_Handler_User_ڈیٹا = خالی_اشاریہ;
}

خالی llvm::report_fatal_error(ثابت حرف *Reason, منطقی GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

خالی llvm::report_fatal_error(StringRef Reason, منطقی GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

خالی llvm::report_fatal_error(ثابت Twine &Reason, منطقی GenCrashDiag) {
  llvm::fatal_error_handler_t handler = خالی_اشاریہ;
  خالی* handler_ڈیٹا = خالی_اشاریہ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#اگر LLVM_ENABLE_دھاگے == 1
    std::تالا_guard<std::mutex> تالا_3(ErrorHandlerMutex);
#اگر_ختم
    handler = ErrorHandler;
    handler_ڈیٹا = Error_Handler_User_ڈیٹا;
  }

  اگر (handler) {
    handler(handler_ڈیٹا, Reason.str().c_str(), GenCrashDiag);
  } ورنہ {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_سمتیہ<حرف, 64> بفر;
    raw_svector_ostream OS(بفر);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef پیغام_Str = OS.str();
    write_retry(2, پیغام_Str.ڈیٹا(), پیغام_Str.سائز_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::تسلسل_مداخلت_Handlers();

  اگر (GenCrashDiag)
    abort();
  ورنہ
    باہر_نکلو(1);
}

خالی llvm::report_Fatal_اندرونی_Error(ثابت حرف *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
خالی llvm::report_Fatal_اندرونی_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
خالی llvm::report_Fatal_اندرونی_Error(ثابت Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
خالی llvm::reportFatalUsageError(ثابت حرف *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/جھوٹ);
}
خالی llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/جھوٹ);
}
خالی llvm::reportFatalUsageError(ثابت Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/جھوٹ);
}

خالی llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           خالی *user_ڈیٹا) {
#اگر LLVM_ENABLE_دھاگے == 1
  std::تالا_guard<std::mutex> تالا_3(BadAllocErrorHandlerMutex);
#اگر_ختم
  أكد(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_ڈیٹا = user_ڈیٹا;
}

خالی llvm::remove_bad_alloc_error_handler() {
#اگر LLVM_ENABLE_دھاگے == 1
  std::تالا_guard<std::mutex> تالا_3(BadAllocErrorHandlerMutex);
#اگر_ختم
  BadAllocErrorHandler = خالی_اشاریہ;
  Bad_Alloc_Error_Handler_User_ڈیٹا = خالی_اشاریہ;
}

خالی llvm::report_bad_alloc_error(ثابت حرف *Reason, منطقی GenCrashDiag) {
  fatal_error_handler_t Handler = خالی_اشاریہ;
  خالی *Handler_ڈیٹا = خالی_اشاریہ;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#اگر LLVM_ENABLE_دھاگے == 1
    std::تالا_guard<std::mutex> تالا_3(BadAllocErrorHandlerMutex);
#اگر_ختم
    Handler = BadAllocErrorHandler;
    Handler_ڈیٹا = Bad_Alloc_Error_Handler_User_ڈیٹا;
  }

  اگر (Handler) {
    Handler(Handler_ڈیٹا, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#اگر_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  پھینکو std::bad_alloc();
#ورنہ
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ثابت حرف *OOMMessage = "LLVM ERROR: out of memory\n";
  ثابت حرف *Newline = "\n";
  write_retry(2, OOMMessage, لڑی_لمبائی(OOMMessage));
  write_retry(2, Reason, لڑی_لمبائی(Reason));
  write_retry(2, Newline, لڑی_لمبائی(Newline));
  abort();
#اگر_ختم
}

#اگر_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
خالی llvm::install_برآمد_کا_حافظہ_new_handler() {
}
#ورنہ
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
ساکن خالی برآمد_کا_حافظہ_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
خالی llvm::install_برآمد_کا_حافظہ_new_handler() {
  std::new_handler old = std::سیٹ_new_handler(برآمد_کا_حافظہ_new_handler);
  (خالی)old;
  أكد((old == خالی_اشاریہ || old == برآمد_کا_حافظہ_new_handler) &&
         "new-handler already installed");
}
#اگر_ختم

خالی llvm::llvm_unreachable_اندرونی(ثابت حرف *پیغام, ثابت حرف *file,
                                     غیرعلامتی سطر_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  اگر (پیغام)
    dbgs() << پیغام << "\n";
  dbgs() << "UNREACHABLE executed";
  اگر (file)
    dbgs() << " at " << file << ":" << سطر_2;
  dbgs() << "!\n";
  abort();
#اگر_تعریف_شدہ LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#اگر_ختم
}

ساکن خالی bindingsErrorHandler(خالی *user_ڈیٹا, ثابت حرف *reason,
                                 منطقی gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_ڈیٹا);
  handler(reason);
}

خالی LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<خالی *>(Handler));
}

خالی LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#اگر_تعریف_شدہ _WIN32

#تعریف_کرو WIN32_NO_حالت
#شامل "llvm/Support/Windows/WindowsSupport.h"
#تعریف_منسوخ WIN32_NO_حالت
#شامل <ntstatus.h>
#شامل <winerror.h>

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
خارجی "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_آخری_Nt_حالت();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_رمز llvm::map_آخری_Windows_Error() {
  غیرعلامتی EV = ::Get_آخری_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  اگر (EV == ERROR_ACCESS_DENIED) {
    llvm::errc رمز_2 = Rtl_Get_آخری_Nt_حالت() == حالت_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    واپس make_error_رمز(رمز_2);
  }
  واپس mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#تعریف_کرو MAP_ERR_مقصد_COND(x, y)                                                  \
  حالت x:                                                                      \
    واپس make_error_رمز(errc::y)

std::error_رمز llvm::mapWindowsError(غیرعلامتی EV) {
  انتخاب (EV) {
    MAP_ERR_مقصد_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_مقصد_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_مقصد_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_مقصد_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_مقصد_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_مقصد_COND(ERROR_بفر_حد_سے_تجاوز, filename_too_long);
    MAP_ERR_مقصد_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_مقصد_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_مقصد_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_مقصد_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_مقصد_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_مقصد_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_مقصد_COND(ERROR_DEVICE_اندر_USE, device_or_resource_busy);
    MAP_ERR_مقصد_COND(ERROR_DIR_NOT_خالی_خانے, directory_not_خالی_خانے);
    MAP_ERR_مقصد_COND(ERROR_DIRECTORY, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_DISK_بھرا, no_space_on_device);
    MAP_ERR_مقصد_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_مقصد_COND(ERROR_FILE_NOT_ملا, no_such_file_or_directory);
    MAP_ERR_مقصد_COND(ERROR_HANDLE_DISK_بھرا, no_space_on_device);
    MAP_ERR_مقصد_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_مقصد_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_مقصد_COND(ERROR_INVALID_HANDLE, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_INVALID_NAME, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_INVALID_PARAMETER, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_تالا_VIOLATION, no_تالا_دستیاب);
    MAP_ERR_مقصد_COND(ERROR_LOCKED, no_تالا_دستیاب);
    MAP_ERR_مقصد_COND(ERROR_NEGATIVE_SEEK, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_NOT_ENOUGH_حافظہ, not_enough_حافظہ);
    MAP_ERR_مقصد_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_مقصد_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_مقصد_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_مقصد_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_مقصد_COND(ERROR_OUTOFMEMORY, not_enough_حافظہ);
    MAP_ERR_مقصد_COND(ERROR_راستہ_NOT_ملا, no_such_file_or_directory);
    MAP_ERR_مقصد_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_مقصد_COND(ERROR_REPARSE_لیبل_INVALID, invalid_دلیل);
    MAP_ERR_مقصد_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_مقصد_COND(ERROR_SEEK, io_error);
    MAP_ERR_مقصد_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_مقصد_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_مقصد_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_مقصد_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_مقصد_COND(WSAEACCES, permission_denied);
    MAP_ERR_مقصد_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_مقصد_COND(WSAEFAULT, bad_پتہ);
    MAP_ERR_مقصد_COND(WSAEINTR, interrupted);
    MAP_ERR_مقصد_COND(WSAEINVAL, invalid_دلیل);
    MAP_ERR_مقصد_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_مقصد_COND(WSAENAMETOOLONG, filename_too_long);
  طےشدہ:
    واپس std::error_رمز(EV, std::system_category());
  }
}

#اگر_ختم
