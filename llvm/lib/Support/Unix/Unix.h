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

#جے_بنا_تعریف LLVM_LIB_SUPPORT_UNIX_UNIX_H
#تعریف_کرو LLVM_LIB_SUPPORT_UNIX_UNIX_H

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

#جے_تعریف_شدہ HAVE_UNISTD_H
#ضمن <unistd.h>
#جے_انت

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
ساكن مضمن bool Make_Err_پیغام(
  std::string* Err_پیغام, ثابت std::string& اگیتر_2, صحيح errnum = -1) {
  إذا (!Err_پیغام)
    أرجع true;
  إذا (errnum == -1)
    errnum = errno;
  *Err_پیغام = اگیتر_2 + ": " + llvm::sys::StrError(errnum);
  أرجع true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ساكن مضمن فراغ ReportErrnumFatal(ثابت حرف *پیغام_2, صحيح errnum) {
  std::string Err_پیغام;
  Make_Err_پیغام(&Err_پیغام, پیغام_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_پیغام));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
مضمن std::chrono::microseconds منزل_Duration(ثابت بنية timeval &TV) {
  أرجع std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
مضمن بنية timespec منزل_ویلا_Spec(ویلا_نقطہ<> TP) {
  using namespace std::chrono;

  بنية timespec RetVal;
  RetVal.tv_sec = منزل_ویلا_T(TP);
  RetVal.tv_nsec = (TP.ویلا_since_epoch() % seconds(1)).گنتی();
  أرجع RetVal;
}

/// Convert a time point to struct timeval.
مضمن بنية timeval منزل_ویلا_Val(ویلا_نقطہ<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  بنية timeval RetVal;
  RetVal.tv_sec = منزل_ویلا_T(TP);
  RetVal.tv_usec = (TP.ویلا_since_epoch() % seconds(1)).گنتی();
  أرجع RetVal;
}

} // namespace sys
} // namespace llvm

#جے_انت
