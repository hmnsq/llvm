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

#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm-c/ErrorHandling.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/Errno.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/Threading.h"
#అన్తర్భావ "llvm/Support/WindowsError.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdlib>
#అన్తర్భావ <mutex>
#అన్తర్భావ <new>

#ఒకవేళ defined(HAVE_UNISTD_H)
# include <unistd.h>
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

స్థిర fatal_error_handler_t ErrorHandler = శూన్యసూచిక;
స్థిర శూన్య *Error_Handler_User_దత్తాంశం = శూన్యసూచిక;

స్థిర fatal_error_handler_t BadAllocErrorHandler = శూన్యసూచిక;
స్థిర శూన్య *Bad_Alloc_Error_Handler_User_దత్తాంశం = శూన్యసూచిక;

#ఒకవేళ LLVM_ENABLE_దారాలు == 1
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
స్థిర std::mutex ErrorHandlerMutex;
స్థిర std::mutex BadAllocErrorHandlerMutex;
#అయితే_ముగింపు

స్థిర తార్కిక write_retry(పూర్ణాంకం fd, స్థిరరాశి అక్షరం *buf, size_t లెక్క) {
  అయితే (లెక్క > 0) {
    ssize_t written = sys::RetryAfterSignal(-1, ::లిఖ, fd, buf, లెక్క);
    ఒకవేళ (written <= 0)
      తిరిగి అబద్ధం;
    buf += written;
    లెక్క -= written;
  }
  తిరిగి నిజం;
}

శూన్య llvm::install_fatal_error_handler(fatal_error_handler_t handler,
                                       శూన్య *user_దత్తాంశం) {
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
  std::తాళం_guard<std::mutex> తాళం_3(ErrorHandlerMutex);
#అయితే_ముగింపు
  ప్రతిజ్ఞా(!ErrorHandler && "Error handler already registered!\n");
  ErrorHandler = handler;
  Error_Handler_User_దత్తాంశం = user_దత్తాంశం;
}

శూన్య llvm::remove_fatal_error_handler() {
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
  std::తాళం_guard<std::mutex> తాళం_3(ErrorHandlerMutex);
#అయితే_ముగింపు
  ErrorHandler = శూన్యసూచిక;
  Error_Handler_User_దత్తాంశం = శూన్యసూచిక;
}

శూన్య llvm::report_fatal_error(స్థిరరాశి అక్షరం *Reason, తార్కిక GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

శూన్య llvm::report_fatal_error(StringRef Reason, తార్కిక GenCrashDiag) {
  report_fatal_error(Twine(Reason), GenCrashDiag);
}

శూన్య llvm::report_fatal_error(స్థిరరాశి Twine &Reason, తార్కిక GenCrashDiag) {
  llvm::fatal_error_handler_t handler = శూన్యసూచిక;
  శూన్య* handler_దత్తాంశం = శూన్యసూచిక;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
    std::తాళం_guard<std::mutex> తాళం_3(ErrorHandlerMutex);
#అయితే_ముగింపు
    handler = ErrorHandler;
    handler_దత్తాంశం = Error_Handler_User_దత్తాంశం;
  }

  ఒకవేళ (handler) {
    handler(handler_దత్తాంశం, Reason.str().c_str(), GenCrashDiag);
  } లేకపోతే {
    // Blast the result out to stderr.  We don't try hard to make sure this
    // succeeds (e.g. handling EINTR) and we can't use errs() here because
    // raw ostreams can call report_fatal_error.
    Small_సదిశ<అక్షరం, 64> బఫరు;
    raw_svector_ostream OS(బఫరు);
    OS << "LLVM ERROR: " << Reason << "\n";
    StringRef సందేశం_Str = OS.str();
    write_retry(2, సందేశం_Str.దత్తాంశం(), సందేశం_Str.కొలత_2());
  }

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  sys::నిరంతరం_అంతరాయం_Handlers();

  ఒకవేళ (GenCrashDiag)
    abort();
  లేకపోతే
    నిష్క్రమించు(1);
}

శూన్య llvm::report_Fatal_అంతర్గత_Error(స్థిరరాశి అక్షరం *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/నిజం);
}
శూన్య llvm::report_Fatal_అంతర్గత_Error(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/నిజం);
}
శూన్య llvm::report_Fatal_అంతర్గత_Error(స్థిరరాశి Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/నిజం);
}
శూన్య llvm::reportFatalUsageError(స్థిరరాశి అక్షరం *reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/అబద్ధం);
}
శూన్య llvm::reportFatalUsageError(StringRef reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/అబద్ధం);
}
శూన్య llvm::reportFatalUsageError(స్థిరరాశి Twine &reason) {
  report_fatal_error(reason, /*GenCrashDiag=*/అబద్ధం);
}

శూన్య llvm::install_bad_alloc_error_handler(fatal_error_handler_t handler,
                                           శూన్య *user_దత్తాంశం) {
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
  std::తాళం_guard<std::mutex> తాళం_3(BadAllocErrorHandlerMutex);
#అయితే_ముగింపు
  ప్రతిజ్ఞా(!BadAllocErrorHandler &&
         "Bad alloc error handler already registered!\n");
  BadAllocErrorHandler = handler;
  Bad_Alloc_Error_Handler_User_దత్తాంశం = user_దత్తాంశం;
}

శూన్య llvm::remove_bad_alloc_error_handler() {
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
  std::తాళం_guard<std::mutex> తాళం_3(BadAllocErrorHandlerMutex);
#అయితే_ముగింపు
  BadAllocErrorHandler = శూన్యసూచిక;
  Bad_Alloc_Error_Handler_User_దత్తాంశం = శూన్యసూచిక;
}

శూన్య llvm::report_bad_alloc_error(స్థిరరాశి అక్షరం *Reason, తార్కిక GenCrashDiag) {
  fatal_error_handler_t Handler = శూన్యసూచిక;
  శూన్య *Handler_దత్తాంశం = శూన్యసూచిక;
  {
    // Only acquire the mutex while reading the handler, so as not to invoke a
    // user-supplied callback under a lock.
#ఒకవేళ LLVM_ENABLE_దారాలు == 1
    std::తాళం_guard<std::mutex> తాళం_3(BadAllocErrorHandlerMutex);
#అయితే_ముగింపు
    Handler = BadAllocErrorHandler;
    Handler_దత్తాంశం = Bad_Alloc_Error_Handler_User_దత్తాంశం;
  }

  ఒకవేళ (Handler) {
    Handler(Handler_దత్తాంశం, Reason, GenCrashDiag);
    llvm_unreachable("bad alloc handler should not return");
  }

#నిర్వచితమైతే LLVM_ENABLE_EXCEPTIONS
  // If exceptions are enabled, make OOM in malloc look like OOM in new.
  విసురు std::bad_alloc();
#లేకపోతే
  // Don't call the normal error handler. It may allocate memory. Directly write
  // an OOM to stderr and abort.
  స్థిరరాశి అక్షరం *OOMMessage = "LLVM ERROR: out of memory\n";
  స్థిరరాశి అక్షరం *Newline = "\n";
  write_retry(2, OOMMessage, అక్షరశ్రేణి_పొడవు(OOMMessage));
  write_retry(2, Reason, అక్షరశ్రేణి_పొడవు(Reason));
  write_retry(2, Newline, అక్షరశ్రేణి_పొడవు(Newline));
  abort();
#అయితే_ముగింపు
}

#నిర్వచితమైతే LLVM_ENABLE_EXCEPTIONS
// Do not set custom new handler if exceptions are enabled. In this case OOM
// errors are handled by throwing 'std::bad_alloc'.
శూన్య llvm::install_బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler() {
}
#లేకపోతే
// Causes crash on allocation failure. It is called prior to the handler set by
// 'install_bad_alloc_error_handler'.
స్థిర శూన్య బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler() {
  llvm::report_bad_alloc_error("Allocation failed");
}

// Installs new handler that causes crash on allocation failure. It is called by
// InitLLVM.
శూన్య llvm::install_బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler() {
  std::new_handler old = std::సమితి_new_handler(బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler);
  (శూన్య)old;
  ప్రతిజ్ఞా((old == శూన్యసూచిక || old == బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler) &&
         "new-handler already installed");
}
#అయితే_ముగింపు

శూన్య llvm::llvm_unreachable_అంతర్గత(స్థిరరాశి అక్షరం *సందేశం, స్థిరరాశి అక్షరం *file,
                                     గుర్తులేని పంక్తి_2) {
  // This code intentionally doesn't call the ErrorHandler callback, because
  // llvm_unreachable is intended to be used to indicate "impossible"
  // situations, and not legitimate runtime errors.
  ఒకవేళ (సందేశం)
    dbgs() << సందేశం << "\n";
  dbgs() << "UNREACHABLE executed";
  ఒకవేళ (file)
    dbgs() << " at " << file << ":" << పంక్తి_2;
  dbgs() << "!\n";
  abort();
#నిర్వచితమైతే LLVM_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  LLVM_BUILTIN_UNREACHABLE;
#అయితే_ముగింపు
}

స్థిర శూన్య bindingsErrorHandler(శూన్య *user_దత్తాంశం, స్థిరరాశి అక్షరం *reason,
                                 తార్కిక gen_crash_diag) {
  LLVMFatalErrorHandler handler =
      LLVM_EXTENSION reinterpret_cast<LLVMFatalErrorHandler>(user_దత్తాంశం);
  handler(reason);
}

శూన్య LLVMInstallFatalErrorHandler(LLVMFatalErrorHandler Handler) {
  install_fatal_error_handler(bindingsErrorHandler,
                              LLVM_EXTENSION reinterpret_cast<శూన్య *>(Handler));
}

శూన్య LLVMResetFatalErrorHandler() {
  remove_fatal_error_handler();
}

#నిర్వచితమైతే _WIN32

#నిర్వచించు WIN32_NO_స్థితి
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
#నిర్వచనం_రద్దు WIN32_NO_స్థితి
#అన్తర్భావ <ntstatus.h>
#అన్తర్భావ <winerror.h>

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
బాహ్య "C" NTSYSAPI NTSTATUS NTAPI Rtl_Get_చివరి_Nt_స్థితి();

// This function obtains the last error code and maps it. It may call
// RtlGetLastNtStatus, which is a lower level API that can return a
// more specific error code than GetLastError.
std::error_సంకేతం llvm::map_చివరి_Windows_Error() {
  గుర్తులేని EV = ::Get_చివరి_Error();
  // The mapping of NTSTATUS to Win32 error loses some information; special
  // case the generic ERROR_ACCESS_DENIED code to check the underlying
  // NTSTATUS and potentially return a more accurate error code.
  ఒకవేళ (EV == ERROR_ACCESS_DENIED) {
    llvm::errc సంకేతం_3 = Rtl_Get_చివరి_Nt_స్థితి() == స్థితి_DELETE_PENDING
                          ? errc::delete_pending
                          : errc::permission_denied;
    తిరిగి make_error_సంకేతం(సంకేతం_3);
  }
  తిరిగి mapWindowsError(EV);
}

// I'd rather not double the line count of the following.
#నిర్వచించు MAP_ERR_గమ్యం_COND(x, y)                                                  \
  స్థితి x:                                                                      \
    తిరిగి make_error_సంకేతం(errc::y)

std::error_సంకేతం llvm::mapWindowsError(గుర్తులేని EV) {
  ఎంపిక (EV) {
    MAP_ERR_గమ్యం_COND(ERROR_ACCESS_DENIED, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_ALREADY_EXISTS, file_exists);
    MAP_ERR_గమ్యం_COND(ERROR_BAD_NETPATH, no_such_file_or_directory);
    MAP_ERR_గమ్యం_COND(ERROR_BAD_PATHNAME, no_such_file_or_directory);
    MAP_ERR_గమ్యం_COND(ERROR_BAD_UNIT, no_such_device);
    MAP_ERR_గమ్యం_COND(ERROR_BROKEN_PIPE, broken_pipe);
    MAP_ERR_గమ్యం_COND(ERROR_బఫరు_అధికప్రవాహం, filename_too_long);
    MAP_ERR_గమ్యం_COND(ERROR_BUSY, device_or_resource_busy);
    MAP_ERR_గమ్యం_COND(ERROR_BUSY_DRIVE, device_or_resource_busy);
    MAP_ERR_గమ్యం_COND(ERROR_CANNOT_MAKE, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_CANTOPEN, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_CANTREAD, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_CANTWRITE, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_CURRENT_DIRECTORY, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_DEV_NOT_EXIST, no_such_device);
    MAP_ERR_గమ్యం_COND(ERROR_DEVICE_లోపల_USE, device_or_resource_busy);
    MAP_ERR_గమ్యం_COND(ERROR_DIR_NOT_ఖాళీలు, directory_not_ఖాళీలు);
    MAP_ERR_గమ్యం_COND(ERROR_DIRECTORY, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_DISK_నిండిన, no_space_on_device);
    MAP_ERR_గమ్యం_COND(ERROR_FILE_EXISTS, file_exists);
    MAP_ERR_గమ్యం_COND(ERROR_FILE_NOT_దొరికింది, no_such_file_or_directory);
    MAP_ERR_గమ్యం_COND(ERROR_HANDLE_DISK_నిండిన, no_space_on_device);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_ACCESS, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_DRIVE, no_such_device);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_FUNCTION, function_not_supported);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_HANDLE, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_NAME, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_INVALID_PARAMETER, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_తాళం_VIOLATION, no_తాళం_అందుబాటు);
    MAP_ERR_గమ్యం_COND(ERROR_LOCKED, no_తాళం_అందుబాటు);
    MAP_ERR_గమ్యం_COND(ERROR_NEGATIVE_SEEK, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_NOACCESS, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_NOT_ENOUGH_జ్ఞాపకం, not_enough_జ్ఞాపకం);
    MAP_ERR_గమ్యం_COND(ERROR_NOT_READY, resource_unavailable_try_again);
    MAP_ERR_గమ్యం_COND(ERROR_NOT_SUPPORTED, not_supported);
    MAP_ERR_గమ్యం_COND(ERROR_OPEN_FAILED, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_OPEN_FILES, device_or_resource_busy);
    MAP_ERR_గమ్యం_COND(ERROR_OUTOFMEMORY, not_enough_జ్ఞాపకం);
    MAP_ERR_గమ్యం_COND(ERROR_మార్గం_NOT_దొరికింది, no_such_file_or_directory);
    MAP_ERR_గమ్యం_COND(ERROR_READ_FAULT, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_REPARSE_ట్యాగ్_INVALID, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(ERROR_RETRY, resource_unavailable_try_again);
    MAP_ERR_గమ్యం_COND(ERROR_SEEK, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_SHARING_VIOLATION, permission_denied);
    MAP_ERR_గమ్యం_COND(ERROR_TOO_MANY_OPEN_FILES, too_many_files_open);
    MAP_ERR_గమ్యం_COND(ERROR_WRITE_FAULT, io_error);
    MAP_ERR_గమ్యం_COND(ERROR_WRITE_PROTECT, permission_denied);
    MAP_ERR_గమ్యం_COND(WSAEACCES, permission_denied);
    MAP_ERR_గమ్యం_COND(WSAEBADF, bad_file_descriptor);
    MAP_ERR_గమ్యం_COND(WSAEFAULT, bad_చిరునామా);
    MAP_ERR_గమ్యం_COND(WSAEINTR, interrupted);
    MAP_ERR_గమ్యం_COND(WSAEINVAL, invalid_వాదం);
    MAP_ERR_గమ్యం_COND(WSAEMFILE, too_many_files_open);
    MAP_ERR_గమ్యం_COND(WSAENAMETOOLONG, filename_too_long);
  అప్రమేయ:
    తిరిగి std::error_సంకేతం(EV, std::system_category());
  }
}

#అయితే_ముగింపు
