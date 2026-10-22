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

#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Chrono.h"
#अन्तर्भाव "llvm/Support/Errno.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव <cerrno>
#अन्तर्भाव <cstdio>
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <cstring>
#अन्तर्भाव <string>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>

#ifdef HAVE_UNISTD_H
#अन्तर्भाव <unistd.h>
#endif

#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <time.h>

#अन्तर्भाव <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
स्थिर ओळीत bool Make_Err_संदेश(
  std::string* Err_संदेश, स्थिरांक std::string& prefix, पूर्णांक errnum = -1) {
  जर (!Err_संदेश)
    परत true;
  जर (errnum == -1)
    errnum = त्रुटि_सं;
  *Err_संदेश = prefix + ": " + llvm::sys::StrError(errnum);
  परत true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] स्थिर ओळीत शून्य ReportErrnumFatal(स्थिरांक अक्षर *संदेश_2, पूर्णांक errnum) {
  std::string Err_संदेश;
  Make_Err_संदेश(&Err_संदेश, संदेश_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_संदेश));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
ओळीत std::chrono::microseconds toDuration(स्थिरांक संरचना timeval &TV) {
  परत std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
ओळीत संरचना timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  संरचना timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).मोजणी();
  परत RetVal;
}

/// Convert a time point to struct timeval.
ओळीत संरचना timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  संरचना timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).मोजणी();
  परत RetVal;
}

} // namespace sys
} // namespace llvm

#endif
