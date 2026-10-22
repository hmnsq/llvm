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

#ifdef HAVE_UNISTD_H
#ضمن <unistd.h>
#endif

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
ساكن مضمن bool MakeErrMsg(
  std::string* ErrMsg, ثابت std::string& prefix, صحيح errnum = -1) {
  ܐܢ (!ErrMsg)
    ܦܢܐ true;
  ܐܢ (errnum == -1)
    errnum = errno;
  *ErrMsg = prefix + ": " + llvm::sys::StrError(errnum);
  ܦܢܐ true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ساكن مضمن فراغ ReportErrnumFatal(ثابت حرف *Msg, صحيح errnum) {
  std::string ErrMsg;
  MakeErrMsg(&ErrMsg, Msg, errnum);
  llvm::report_fatal_error(llvm::Twine(ErrMsg));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
مضمن std::chrono::microseconds toDuration(ثابت بنية timeval &TV) {
  ܦܢܐ std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
مضمن بنية timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  بنية timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).count();
  ܦܢܐ RetVal;
}

/// Convert a time point to struct timeval.
مضمن بنية timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  بنية timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).count();
  ܦܢܐ RetVal;
}

} // namespace sys
} // namespace llvm

#endif
