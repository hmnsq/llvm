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

#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Chrono.h"
#അന്തര്ഭാവ "llvm/Support/Errno.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <cerrno>
#അന്തര്ഭാവ <cstdio>
#അന്തര്ഭാവ <cstdlib>
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>

#ifdef HAVE_UNISTD_H
#അന്തര്ഭാവ <unistd.h>
#endif

#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <time.h>

#അന്തര്ഭാവ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
സ്ഥിതം വരിയിൽ bool Make_Err_സന്ദേശം(
  std::string* Err_സന്ദേശം, സ്ഥിരം std::string& prefix, പൂർണ്ണസംഖ്യ errnum = -1) {
  എങ്കിൽ (!Err_സന്ദേശം)
    തിരികെ true;
  എങ്കിൽ (errnum == -1)
    errnum = errno;
  *Err_സന്ദേശം = prefix + ": " + llvm::sys::StrError(errnum);
  തിരികെ true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] സ്ഥിതം വരിയിൽ ശൂന്യം ReportErrnumFatal(സ്ഥിരം അക്ഷരം *സന്ദേശം_2, പൂർണ്ണസംഖ്യ errnum) {
  std::string Err_സന്ദേശം;
  Make_Err_സന്ദേശം(&Err_സന്ദേശം, സന്ദേശം_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_സന്ദേശം));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
വരിയിൽ std::chrono::microseconds toDuration(സ്ഥിരം ഘടന timeval &TV) {
  തിരികെ std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
വരിയിൽ ഘടന timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  ഘടന timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).എണ്ണം();
  തിരികെ RetVal;
}

/// Convert a time point to struct timeval.
വരിയിൽ ഘടന timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ഘടന timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).എണ്ണം();
  തിരികെ RetVal;
}

} // namespace sys
} // namespace llvm

#endif
