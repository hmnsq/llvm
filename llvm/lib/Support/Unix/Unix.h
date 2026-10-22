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

#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Chrono.h"
#અન્તર્ભાવ "llvm/Support/Errno.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <cerrno>
#અન્તર્ભાવ <cstdio>
#અન્તર્ભાવ <cstdlib>
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>

#ifdef HAVE_UNISTD_H
#અન્તર્ભાવ <unistd.h>
#endif

#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <time.h>

#અન્તર્ભાવ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
સ્થિર અંતઃરેખ bool Make_Err_સંદેશ(
  std::string* Err_સંદેશ, અચલ std::string& prefix, પૂર્ણાંક errnum = -1) {
  જો (!Err_સંદેશ)
    પાછા true;
  જો (errnum == -1)
    errnum = errno;
  *Err_સંદેશ = prefix + ": " + llvm::sys::StrError(errnum);
  પાછા true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] સ્થિર અંતઃરેખ શૂન્ય ReportErrnumFatal(અચલ અક્ષર *સંદેશ_2, પૂર્ણાંક errnum) {
  std::string Err_સંદેશ;
  Make_Err_સંદેશ(&Err_સંદેશ, સંદેશ_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_સંદેશ));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
અંતઃરેખ std::chrono::microseconds toDuration(અચલ સંરચના timeval &TV) {
  પાછા std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
અંતઃરેખ સંરચના timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  સંરચના timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).ગણતરી();
  પાછા RetVal;
}

/// Convert a time point to struct timeval.
અંતઃરેખ સંરચના timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  સંરચના timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).ગણતરી();
  પાછા RetVal;
}

} // namespace sys
} // namespace llvm

#endif
