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

#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Chrono.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errno.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <cerrno>
#ଅନ୍ତର୍ଭାଵ <cstdio>
#ଅନ୍ତର୍ଭାଵ <cstdlib>
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>

#ifdef HAVE_UNISTD_H
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#endif

#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <time.h>

#ଅନ୍ତର୍ଭାଵ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool Make_Err_ସନ୍ଦେଶ(
  std::string* Err_ସନ୍ଦେଶ, ଅଚଳ std::string& prefix, ପୂର୍ଣ୍ଣସଂଖ୍ୟା errnum = -1) {
  ଯଦି (!Err_ସନ୍ଦେଶ)
    ଫେରନ୍ତୁ true;
  ଯଦି (errnum == -1)
    errnum = errno;
  *Err_ସନ୍ଦେଶ = prefix + ": " + llvm::sys::StrError(errnum);
  ଫେରନ୍ତୁ true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ ReportErrnumFatal(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା errnum) {
  std::string Err_ସନ୍ଦେଶ;
  Make_Err_ସନ୍ଦେଶ(&Err_ସନ୍ଦେଶ, ସନ୍ଦେଶ_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_ସନ୍ଦେଶ));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
ଧାଡ଼ିଭିତରେ std::chrono::microseconds toDuration(ଅଚଳ ସଂରଚନା timeval &TV) {
  ଫେରନ୍ତୁ std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
ଧାଡ଼ିଭିତରେ ସଂରଚନା timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  ସଂରଚନା timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).ଗଣନା_2();
  ଫେରନ୍ତୁ RetVal;
}

/// Convert a time point to struct timeval.
ଧାଡ଼ିଭିତରେ ସଂରଚନା timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ସଂରଚନା timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).ଗଣନା_2();
  ଫେରନ୍ତୁ RetVal;
}

} // namespace sys
} // namespace llvm

#endif
