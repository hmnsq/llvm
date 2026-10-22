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

#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm-c/ErrorHandling.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/Errno.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Threading.h"
#ضمن "llvm/Support/WindowsError.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cstdlib>
#ضمن <mutex>
#ضمن <new>

#إذا defined(HAVE_UNISTD_H)
# include <unistd.h>
#جے_انت
#إذا defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#جے_انت

ورتوں نطاق llvm;

ساكن fatal_error_handler_t ErrorHandler = خالی_سوچک;
ساكن فراغ *Error_Handler_User_انکڑے = خالی_سوچک;

ساكن fatal_error_handler_t BadAllocErrorHandler = خالی_سوچک;
ساكن فراغ *Bad_Alloc_Error_Handler_User_انکڑے = خالی_سوچک;

#إذا LLVM_ENABLE_دھاگے == 1
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
ساكن std::mutex ErrorHandlerMutex;
ساكن std::mutex BadAllocErrorHandlerMutex;
#جے_انت

ساكن منطقي write_retry(صحيح fd, ثابت حرف *buf, size_t گنتی) {
  طالما (گنتی > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::اكتب, fd, buf, گنتی);
    إذا (written <= 0)
      أرجع خطأ;
    buf += written;
    گنتی -= written;
  }
  أرجع سچ;
}

فراغ llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       فراغ *user_انکڑے) {
#إذا LLVM_ENABLE_دھاگے == 1
  std::جندرا_guard<std::mutex> جندرا_2(ErrorHandlerMutex);
#جے_انت
  أكد(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_انکڑے = user_انکڑے;
}

فراغ llvm::remove_fatal_error_handler() {
#إذا LLVM_ENABLE_دھاگے == 1
  std::جندرا_guard<std::mutex> جندرا_2(ErrorHandlerMutex);
#جے_انت
  ErrorHandler = خالی_سوچک;
  Error_Handler_User_انکڑے = خالی_سوچک;
}

فراغ llvm::report_fatal_error(ثابت حرف *Reason, منطقي GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

فراغ llvm::report_fatal_error(StringRef Reason, منطقي GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

فراغ llvm::report_fatal_error(ثابت Twine &Reason, منطقي GenCrashDiag) {
  llvm::fatal_error_handler_t handler = خالی_سوچک;
  فراغ* handler_انکڑے = خالی_سوچک;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#إذا LLVM_ENABLE_دھاگے == 1
    std::جندرا_guard<std::mutex> جندرا_2(ErrorHandlerMutex);
#جے_انت
    handler = ErrorHandler;
    handler_انکڑے = Error_Handler_User_انکڑے;
  }

  إذا (handler) {
    handler(handler_انکڑے, Reason.str().c_str(), GenCrashDiag);
  } وإلا {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_سمتیہ<حرف, 64> بفر;
    raw_svector_ostream OS(بفر);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef سنیہا_Str = OS.str();
    write_retry(2, سنیہا_Str.انکڑے(), سنیہا_Str.ناپ_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::لگاتار_رکاوٹ_Handlers();

  إذا (GenCrashDiag)
    abort();
  وإلا
    باہر_نکلو(1);
}

فراغ llvm::report_Fatal_اندرونی_Error(ثابت حرف *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
فراغ llvm::report_Fatal_اندرونی_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
فراغ llvm::report_Fatal_اندرونی_Error(ثابت Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/سچ);
}
فراغ llvm::reportFatalUsageError(ثابت حرف *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/خطأ);
}
فراغ llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/خطأ);
}
فراغ llvm::reportFatalUsageError(ثابت Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/خطأ);
}

فراغ llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           فراغ *user_انکڑے) {
#إذا LLVM_ENABLE_دھاگے == 1
  std::جندرا_guard<std::mutex> جندرا_2(BadAllocErrorHandlerMutex);
#جے_انت
  أكد(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_انکڑے = user_انکڑے;
}

فراغ llvm::remove_bad_alloc_error_handler() {
#إذا LLVM_ENABLE_دھاگے == 1
  std::جندرا_guard<std::mutex> جندرا_2(BadAllocErrorHandlerMutex);
#جے_انت
  BadAllocErrorHandler = خالی_سوچک;
  Bad_Alloc_Error_Handler_User_انکڑے = خالی_سوچک;
}

فراغ llvm::report_bad_alloc_error(ثابت حرف *Reason, منطقي GenCrashDiag) {
  fatal_error_handler_t Handler = خالی_سوچک;
  فراغ *Handler_انکڑے = خالی_سوچک;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#إذا LLVM_ENABLE_دھاگے == 1
    std::جندرا_guard<std::mutex> جندرا_2(BadAllocErrorHandlerMutex);
#جے_انت
    Handler = BadAllocErrorHandler;
    Handler_انکڑے = Bad_Alloc_Error_Handler_User_انکڑے;
  }

  إذا (Handler) {
    Handler(Handler_انکڑے, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#جے_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  ارم std::bad_alloc();
#وإلا
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ثابت حرف *OOMMessage = "LLVM ERROR: out of memory\n";
  ثابت حرف *Newline = "\n";
  write_retry(2, OOMMessage, لڑی_لمبائی(OOMMessage));
  write_retry(2, Reason, لڑی_لمبائی(Reason));
  write_retry(2, Newline, لڑی_لمبائی(Newline));
  abort();
#جے_انت
}

#جے_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
فراغ llvm::install_باہرلا_دا_حافظہ_new_handler() {
}
#وإلا
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
ساكن فراغ باہرلا_دا_حافظہ_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
فراغ llvm::install_باہرلا_دا_حافظہ_new_handler() {
  std::new_handler old = std::سیٹ_new_handler(باہرلا_دا_حافظہ_new_handler);
  (فراغ)old;
  أكد((old == خالی_سوچک || old == باہرلا_دا_حافظہ_new_handler) &&
         "new-handler already installed");
}
#جے_انت

فراغ llvm::llvm_unreachable_اندرونی(ثابت حرف *پیغام, ثابت حرف *file,
                                     بنانشانی سطر_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  إذا (پیغام)
    dbgs() << پیغام << "\n";
  dbgs() << "UNREACHABLE executed";
  إذا (file)
    dbgs() << " at " << file << ":" << سطر_2;
  dbgs() << "!\n";
  abort();
#جے_تعریف_شدہ LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#جے_انت
}

ساكن فراغ bindingsErrorHandler(فراغ *user_انکڑے, ثابت حرف *reason,
                                 منطقي gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_انکڑے);
  handler(reason);
}

فراغ LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<فراغ *>(Handler));
}

فراغ LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#جے_تعریف_شدہ _WIN32

#تعریف_کرو WIN32_NO_حالت
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#تعریف_رد WIN32_NO_حالت
#ضمن <ntstatus.h>
#ضمن <winerror.h>

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
خارجي "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_آخری_Nt_حالت();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_کوڈ llvm::map_آخری_Windows_Error() {
  بنانشانی EV = ::Get_آخری_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  إذا (EV == ERROR_ACCESS_DENIED) {
    llvm::errc کوڈ_2 = Rtl_Get_آخری_Nt_حالت() == حالت_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    أرجع make_error_کوڈ(کوڈ_2);
  }
  أرجع mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#تعریف_کرو MAP_ERR_منزل_COND(x, y)                                                  \
  حالة x:                                                                      \
    أرجع make_error_کوڈ(errc::y)

std::error_کوڈ llvm::mapWindowsError(بنانشانی EV) {
  بدل (EV) {
    MAP_ERR_منزل_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_منزل_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_منزل_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_منزل_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_منزل_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_منزل_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_منزل_COND(ERROR_بفر_حد_توں_ودھ, filename_too_long);
    MAP_ERR_منزل_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_منزل_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_منزل_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_منزل_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_منزل_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_منزل_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_منزل_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_منزل_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_منزل_COND(ERROR_DEVICE_وچ_USE, device_or_resource_busy);
    MAP_ERR_منزل_COND(ERROR_DIR_NOT_خالی_تھاواں, directory_not_خالی_تھاواں);
    MAP_ERR_منزل_COND(ERROR_DIRECTORY, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_DISK_بھریا, no_space_on_device);
    MAP_ERR_منزل_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_منزل_COND(ERROR_FILE_NOT_لبھیا, no_such_file_or_directory);
    MAP_ERR_منزل_COND(ERROR_HANDLE_DISK_بھریا, no_space_on_device);
    MAP_ERR_منزل_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_منزل_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_منزل_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_منزل_COND(ERROR_INVALID_HANDLE, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_INVALID_NAME, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_INVALID_PARAMETER, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_جندرا_VIOLATION, no_جندرا_دستیاب);
    MAP_ERR_منزل_COND(ERROR_LOCKED, no_جندرا_دستیاب);
    MAP_ERR_منزل_COND(ERROR_NEGATIVE_SEEK, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_منزل_COND(ERROR_NOT_ENOUGH_حافظہ, not_enough_حافظہ);
    MAP_ERR_منزل_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_منزل_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_منزل_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_منزل_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_منزل_COND(ERROR_OUTOFMEMORY, not_enough_حافظہ);
    MAP_ERR_منزل_COND(ERROR_رستہ_NOT_لبھیا, no_such_file_or_directory);
    MAP_ERR_منزل_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_منزل_COND(ERROR_REPARSE_ٹیگ_INVALID, invalid_دلیل);
    MAP_ERR_منزل_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_منزل_COND(ERROR_SEEK, io_error);
    MAP_ERR_منزل_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_منزل_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_منزل_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_منزل_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_منزل_COND(WSAEACCES, permission_denied);
    MAP_ERR_منزل_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_منزل_COND(WSAEFAULT, bad_پتہ);
    MAP_ERR_منزل_COND(WSAEINTR, interrupted);
    MAP_ERR_منزل_COND(WSAEINVAL, invalid_دلیل);
    MAP_ERR_منزل_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_منزل_COND(WSAENAMETOOLONG, filename_too_long);
  افتراضي:
    أرجع std::error_کوڈ(EV, std::system_category());
  }
}

#جے_انت
