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

#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm-c/ErrorHandling.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/Errno.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव "llvm/Support/WindowsError.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <mutex>
#अन्तर्भाव <new>

#यदि defined(HAVE_UNISTD_H)
# include <unistd.h>
#यदि_अन्तः
#यदि defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

स्थिर fatal_error_handler_t ErrorHandler = शून्यसूचकः;
स्थिर शून्य *Error_Handler_User_दत्त = शून्यसूचकः;

स्थिर fatal_error_handler_t BadAllocErrorHandler = शून्यसूचकः;
स्थिर शून्य *Bad_Alloc_Error_Handler_User_दत्त = शून्यसूचकः;

#यदि LLVM_ENABLE_तन्तवः == 1
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
स्थिर std::mutex ErrorHandlerMutex;
स्थिर std::mutex BadAllocErrorHandlerMutex;
#यदि_अन्तः

स्थिर तार्किक write_retry(पूर्णाङ्क fd, ध्रुव वर्ण *buf, size_t गणनम्) {
  यावत् (गणनम् > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::लिख, fd, buf, गणनम्);
    यदि (written <= 0)
      प्रत्यागम असत्य;
    buf += written;
    गणनम् -= written;
  }
  प्रत्यागम सत्य;
}

शून्य llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       शून्य *user_दत्त) {
#यदि LLVM_ENABLE_तन्तवः == 1
  std::तालकम्_guard<std::mutex> तालकम्_2(ErrorHandlerMutex);
#यदि_अन्तः
  प्रतिज्ञा(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_दत्त = user_दत्त;
}

शून्य llvm::remove_fatal_error_handler() {
#यदि LLVM_ENABLE_तन्तवः == 1
  std::तालकम्_guard<std::mutex> तालकम्_2(ErrorHandlerMutex);
#यदि_अन्तः
  ErrorHandler = शून्यसूचकः;
  Error_Handler_User_दत्त = शून्यसूचकः;
}

शून्य llvm::report_fatal_error(ध्रुव वर्ण *Reason, तार्किक GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

शून्य llvm::report_fatal_error(StringRef Reason, तार्किक GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

शून्य llvm::report_fatal_error(ध्रुव Twine &Reason, तार्किक GenCrashDiag) {
  llvm::fatal_error_handler_t handler = शून्यसूचकः;
  शून्य* handler_दत्त = शून्यसूचकः;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#यदि LLVM_ENABLE_तन्तवः == 1
    std::तालकम्_guard<std::mutex> तालकम्_2(ErrorHandlerMutex);
#यदि_अन्तः
    handler = ErrorHandler;
    handler_दत्त = Error_Handler_User_दत्त;
  }

  यदि (handler) {
    handler(handler_दत्त, Reason.str().c_str(), GenCrashDiag);
  } अन्यथा {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_सदिशम्<वर्ण, 64> बफर;
    raw_svector_ostream OS(बफर);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef सन्देशः_Str = OS.str();
    write_retry(2, सन्देशः_Str.दत्त(), सन्देशः_Str.आकार_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::सततम्_व्यवधानम्_Handlers();

  यदि (GenCrashDiag)
    abort();
  अन्यथा
    निर्गच्छ(1);
}

शून्य llvm::report_Fatal_आन्तरिकम्_Error(ध्रुव वर्ण *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/सत्य);
}
शून्य llvm::report_Fatal_आन्तरिकम्_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/सत्य);
}
शून्य llvm::report_Fatal_आन्तरिकम्_Error(ध्रुव Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/सत्य);
}
शून्य llvm::reportFatalUsageError(ध्रुव वर्ण *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/असत्य);
}
शून्य llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/असत्य);
}
शून्य llvm::reportFatalUsageError(ध्रुव Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/असत्य);
}

शून्य llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           शून्य *user_दत्त) {
#यदि LLVM_ENABLE_तन्तवः == 1
  std::तालकम्_guard<std::mutex> तालकम्_2(BadAllocErrorHandlerMutex);
#यदि_अन्तः
  प्रतिज्ञा(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_दत्त = user_दत्त;
}

शून्य llvm::remove_bad_alloc_error_handler() {
#यदि LLVM_ENABLE_तन्तवः == 1
  std::तालकम्_guard<std::mutex> तालकम्_2(BadAllocErrorHandlerMutex);
#यदि_अन्तः
  BadAllocErrorHandler = शून्यसूचकः;
  Bad_Alloc_Error_Handler_User_दत्त = शून्यसूचकः;
}

शून्य llvm::report_bad_alloc_error(ध्रुव वर्ण *Reason, तार्किक GenCrashDiag) {
  fatal_error_handler_t Handler = शून्यसूचकः;
  शून्य *Handler_दत्त = शून्यसूचकः;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#यदि LLVM_ENABLE_तन्तवः == 1
    std::तालकम्_guard<std::mutex> तालकम्_2(BadAllocErrorHandlerMutex);
#यदि_अन्तः
    Handler = BadAllocErrorHandler;
    Handler_दत्त = Bad_Alloc_Error_Handler_User_दत्त;
  }

  यदि (Handler) {
    Handler(Handler_दत्त, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#यदि_परिभाषितम् LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  क्षिप std::bad_alloc();
#अन्यथा
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  ध्रुव वर्ण *OOMMessage = "LLVM ERROR: out of memory\n";
  ध्रुव वर्ण *Newline = "\n";
  write_retry(2, OOMMessage, वाक्यदैर्घ्यम्(OOMMessage));
  write_retry(2, Reason, वाक्यदैर्घ्यम्(Reason));
  write_retry(2, Newline, वाक्यदैर्घ्यम्(Newline));
  abort();
#यदि_अन्तः
}

#यदि_परिभाषितम् LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
शून्य llvm::install_बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler() {
}
#अन्यथा
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
स्थिर शून्य बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
शून्य llvm::install_बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler() {
  std::new_handler old = std::समुच्चयः_new_handler(बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler);
  (शून्य)old;
  प्रतिज्ञा((old == शून्यसूचकः || old == बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler) &&
         "new-handler already installed");
}
#यदि_अन्तः

शून्य llvm::llvm_unreachable_आन्तरिकम्(ध्रुव वर्ण *सन्देशः, ध्रुव वर्ण *file,
                                     अचिह्नित पङ्क्तिः_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  यदि (सन्देशः)
    dbgs() << सन्देशः << "\n";
  dbgs() << "UNREACHABLE executed";
  यदि (file)
    dbgs() << " at " << file << ":" << पङ्क्तिः_2;
  dbgs() << "!\n";
  abort();
#यदि_परिभाषितम् LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#यदि_अन्तः
}

स्थिर शून्य bindingsErrorHandler(शून्य *user_दत्त, ध्रुव वर्ण *reason,
                                 तार्किक gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_दत्त);
  handler(reason);
}

शून्य LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<शून्य *>(Handler));
}

शून्य LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#यदि_परिभाषितम् _WIN32

#परिभाषय WIN32_NO_स्थितिः
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#अपरिभाषय WIN32_NO_स्थितिः
#अन्तर्भाव <ntstatus.h>
#अन्तर्भाव <winerror.h>

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
बाह्य "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_चरमः_Nt_स्थितिः();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_संहिता llvm::map_चरमः_Windows_Error() {
  अचिह्नित EV = ::Get_चरमः_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  यदि (EV == ERROR_ACCESS_DENIED) {
    llvm::errc संहिता_2 = Rtl_Get_चरमः_Nt_स्थितिः() == स्थितिः_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    प्रत्यागम make_error_संहिता(संहिता_2);
  }
  प्रत्यागम mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#परिभाषय MAP_ERR_गन्तव्यम्_COND(x, y)                                                  \
  पक्ष x:                                                                      \
    प्रत्यागम make_error_संहिता(errc::y)

std::error_संहिता llvm::mapWindowsError(अचिह्नित EV) {
  विकल्प (EV) {
    MAP_ERR_गन्तव्यम्_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_गन्तव्यम्_COND(ERROR_बफर_अतिप्रवाहः, filename_too_long);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_गन्तव्यम्_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_गन्तव्यम्_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_गन्तव्यम्_COND(ERROR_DEVICE_अन्तर्_USE, device_or_resource_busy);
    MAP_ERR_गन्तव्यम्_COND(ERROR_DIR_NOT_रिक्तानि, directory_not_रिक्तानि);
    MAP_ERR_गन्तव्यम्_COND(ERROR_DIRECTORY, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_DISK_पूर्णम्, no_space_on_device);
    MAP_ERR_गन्तव्यम्_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_गन्तव्यम्_COND(ERROR_FILE_NOT_प्राप्तम्, no_such_file_or_directory);
    MAP_ERR_गन्तव्यम्_COND(ERROR_HANDLE_DISK_पूर्णम्, no_space_on_device);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_HANDLE, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_NAME, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_INVALID_PARAMETER, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_तालकम्_VIOLATION, no_तालकम्_उपलब्धम्);
    MAP_ERR_गन्तव्यम्_COND(ERROR_LOCKED, no_तालकम्_उपलब्धम्);
    MAP_ERR_गन्तव्यम्_COND(ERROR_NEGATIVE_SEEK, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_NOT_ENOUGH_स्मृतिः, not_enough_स्मृतिः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_गन्तव्यम्_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_गन्तव्यम्_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_गन्तव्यम्_COND(ERROR_OUTOFMEMORY, not_enough_स्मृतिः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_मार्गः_NOT_प्राप्तम्, no_such_file_or_directory);
    MAP_ERR_गन्तव्यम्_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_REPARSE_चिह्नकम्_INVALID, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_गन्तव्यम्_COND(ERROR_SEEK, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_गन्तव्यम्_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_गन्तव्यम्_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(WSAEACCES, permission_denied);
    MAP_ERR_गन्तव्यम्_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_गन्तव्यम्_COND(WSAEFAULT, bad_स्थानाङ्कः);
    MAP_ERR_गन्तव्यम्_COND(WSAEINTR, interrupted);
    MAP_ERR_गन्तव्यम्_COND(WSAEINVAL, invalid_तर्कः);
    MAP_ERR_गन्तव्यम्_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_गन्तव्यम्_COND(WSAENAMETOOLONG, filename_too_long);
  स्वभाव:
    प्रत्यागम std::error_संहिता(EV, std::system_category());
  }
}

#यदि_अन्तः
