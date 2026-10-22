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

#ifndef LLVM_LIB_SUPPORT_UNIX_UNIX_H
#define LLVM_LIB_SUPPORT_UNIX_UNIX_H

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

#ifdef HAVE_UNISTD_H
#شامل <unistd.h>
#endif

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
ایستا درونخطی bool Make_Err_پیام(
  std::string* Err_پیام, ثابت std::string& prefix, صحیح errnum = -1) {
  اگر (!Err_پیام)
    بازگشت true;
  اگر (errnum == -1)
    errnum = errno;
  *Err_پیام = prefix + ": " + llvm::sys::StrError(errnum);
  بازگشت true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ایستا درونخطی تهی ReportErrnumFatal(ثابت نویسه *پیام_2, صحیح errnum) {
  std::string Err_پیام;
  Make_Err_پیام(&Err_پیام, پیام_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_پیام));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
درونخطی std::chrono::microseconds toDuration(ثابت ساختار timeval &TV) {
  بازگشت std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
درونخطی ساختار timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  ساختار timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).شمار();
  بازگشت RetVal;
}

/// Convert a time point to struct timeval.
درونخطی ساختار timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ساختار timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).شمار();
  بازگشت RetVal;
}

} // namespace sys
} // namespace llvm

#endif
