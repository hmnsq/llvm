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

#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Chrono.h"
#அந்தர்பாவ "llvm/Support/Errno.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <cerrno>
#அந்தர்பாவ <cstdio>
#அந்தர்பாவ <cstdlib>
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <string>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>

#ifdef HAVE_UNISTD_H
#அந்தர்பாவ <unistd.h>
#endif

#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <time.h>

#அந்தர்பாவ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
நிலையான உள்வரி bool Make_Err_செய்தி(
  std::string* Err_செய்தி, மாறிலி std::string& prefix, முழு errnum = -1) {
  எனில் (!Err_செய்தி)
    திரும்பு true;
  எனில் (errnum == -1)
    errnum = errno;
  *Err_செய்தி = prefix + ": " + llvm::sys::StrError(errnum);
  திரும்பு true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] நிலையான உள்வரி வெறுமை ReportErrnumFatal(மாறிலி எழுத்து *செய்தி_2, முழு errnum) {
  std::string Err_செய்தி;
  Make_Err_செய்தி(&Err_செய்தி, செய்தி_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_செய்தி));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
உள்வரி std::chrono::microseconds toDuration(மாறிலி கட்டமைப்பு timeval &TV) {
  திரும்பு std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
உள்வரி கட்டமைப்பு timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  கட்டமைப்பு timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).மொத்தம்();
  திரும்பு RetVal;
}

/// Convert a time point to struct timeval.
உள்வரி கட்டமைப்பு timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  கட்டமைப்பு timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).மொத்தம்();
  திரும்பு RetVal;
}

} // namespace sys
} // namespace llvm

#endif
