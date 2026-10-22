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

#నిర్వచితం_కాకపోతే LLVM_LIB_SUPPORT_UNIX_UNIX_H
#నిర్వచించు LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Chrono.h"
#అన్తర్భావ "llvm/Support/Errno.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ <assert.h>
#అన్తర్భావ <cerrno>
#అన్తర్భావ <cstdio>
#అన్తర్భావ <cstdlib>
#అన్తర్భావ <cstring>
#అన్తర్భావ <string>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>

#నిర్వచితమైతే HAVE_UNISTD_H
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు

#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <time.h>

#అన్తర్భావ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
స్థిర అంతఃవరుస bool Make_Err_సందేశం(
  std::string* Err_సందేశం, స్థిరరాశి std::string& ఉపసర్గ_2, పూర్ణాంకం errnum = -1) {
  ఒకవేళ (!Err_సందేశం)
    తిరిగి true;
  ఒకవేళ (errnum == -1)
    errnum = errno;
  *Err_సందేశం = ఉపసర్గ_2 + ": " + llvm::sys::StrError(errnum);
  తిరిగి true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] స్థిర అంతఃవరుస శూన్య ReportErrnumFatal(స్థిరరాశి అక్షరం *సందేశం_4, పూర్ణాంకం errnum) {
  std::string Err_సందేశం;
  Make_Err_సందేశం(&Err_సందేశం, సందేశం_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_సందేశం));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
అంతఃవరుస std::chrono::microseconds గమ్యం_Duration(స్థిరరాశి నిర్మాణం timeval &TV) {
  తిరిగి std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
అంతఃవరుస నిర్మాణం timespec గమ్యం_సమయం_Spec(సమయం_బిందువు<> TP) {
  using namespace std::chrono;

  నిర్మాణం timespec RetVal;
  RetVal.tv_sec = గమ్యం_సమయం_T(TP);
  RetVal.tv_nsec = (TP.సమయం_since_epoch() % seconds(1)).లెక్క();
  తిరిగి RetVal;
}

/// Convert a time point to struct timeval.
అంతఃవరుస నిర్మాణం timeval గమ్యం_సమయం_Val(సమయం_బిందువు<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  నిర్మాణం timeval RetVal;
  RetVal.tv_sec = గమ్యం_సమయం_T(TP);
  RetVal.tv_usec = (TP.సమయం_since_epoch() % seconds(1)).లెక్క();
  తిరిగి RetVal;
}

} // namespace sys
} // namespace llvm

#అయితే_ముగింపు
