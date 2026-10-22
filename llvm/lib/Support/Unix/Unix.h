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

#اگر_تعریف_نشده LLVM_LIB_SUPPORT_UNIX_UNIX_H
#تعریف_کن LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Chrono.h"
#ضمن "llvm/Support/Errno.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن <assert.h>
#ضمن <cerrno>
#ضمن <cstdio>
#ضمن <cstdlib>
#ضمن <cstring>
#ضمن <string>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>

#اگر_تعریف_شده HAVE_UNISTD_H
#ضمن <unistd.h>
#پایان_اگر

#ضمن <sys/time.h>
#ضمن <time.h>

#ضمن <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
ساكن مضمن bool Make_Err_پیام(
  std::string* Err_پیام, ثابت std::string& پیشوند_2, صحيح errnum = -1) {
  إذا (!Err_پیام)
    أرجع true;
  إذا (errnum == -1)
    errnum = errno;
  *Err_پیام = پیشوند_2 + ": " + llvm::sys::StrError(errnum);
  أرجع true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ساكن مضمن فراغ ReportErrnumFatal(ثابت حرف *پیام_4, صحيح errnum) {
  std::string Err_پیام;
  Make_Err_پیام(&Err_پیام, پیام_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_پیام));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
مضمن std::chrono::microseconds مقصد_Duration(ثابت بنية timeval &TV) {
  أرجع std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
مضمن بنية timespec مقصد_زمان_Spec(زمان_نقطه<> TP) {
  using namespace std::chrono;

  بنية timespec RetVal;
  RetVal.tv_sec = مقصد_زمان_T(TP);
  RetVal.tv_nsec = (TP.زمان_since_epoch() % seconds(1)).شمار();
  أرجع RetVal;
}

/// Convert a time point to struct timeval.
مضمن بنية timeval مقصد_زمان_Val(زمان_نقطه<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  بنية timeval RetVal;
  RetVal.tv_sec = مقصد_زمان_T(TP);
  RetVal.tv_usec = (TP.زمان_since_epoch() % seconds(1)).شمار();
  أرجع RetVal;
}

} // namespace sys
} // namespace llvm

#پایان_اگر
