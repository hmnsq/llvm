//===- llvm/Support/Unix/Unix.h - Common Unix Include File -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines things specific to Unix implementations.
//
//===----------------------------------------------------------------------===//

#اگر_غیر_تعریف_شدہ LLVM_LIB_SUPPORT_UNIX_UNIX_H
#تعریف_کرو LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Chrono.h"
#شامل "llvm/Support/Errno.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل <assert.h>
#شامل <cerrno>
#شامل <cstdio>
#شامل <cstdlib>
#شامل <cstring>
#شامل <string>
#شامل <sys/types.h>
#شامل <sys/wait.h>

#اگر_تعریف_شدہ HAVE_UNISTD_H
#شامل <unistd.h>
#اگر_ختم

#شامل <sys/time.h>
#شامل <time.h>

#شامل <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
ساکن اندرونی bool Make_Err_پیغام(
  std::string* Err_پیغام, ثابت std::string& سابقہ_2, صحیح errnum = -1) {
  اگر (!Err_پیغام)
    واپس true;
  اگر (errnum == -1)
    errnum = errno;
  *Err_پیغام = سابقہ_2 + ": " + llvm::sys::StrError(errnum);
  واپس true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ساکن اندرونی خالی ReportErrnumFatal(ثابت حرف *پیغام_4, صحیح errnum) {
  std::string Err_پیغام;
  Make_Err_پیغام(&Err_پیغام, پیغام_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_پیغام));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
اندرونی std::chrono::microseconds مقصد_Duration(ثابت ساخت timeval &TV) {
  واپس std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
اندرونی ساخت timespec مقصد_وقت_Spec(وقت_نقطہ<> TP) {
  using namespace std::chrono;

  ساخت timespec RetVal;
  RetVal.tv_sec = مقصد_وقت_T(TP);
  RetVal.tv_nsec = (TP.وقت_since_epoch() % seconds(1)).گنتی();
  واپس RetVal;
}

/// Convert a time point to struct timeval.
اندرونی ساخت timeval مقصد_وقت_Val(وقت_نقطہ<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ساخت timeval RetVal;
  RetVal.tv_sec = مقصد_وقت_T(TP);
  RetVal.tv_usec = (TP.وقت_since_epoch() % seconds(1)).گنتی();
  واپس RetVal;
}

} // namespace sys
} // namespace llvm

#اگر_ختم
