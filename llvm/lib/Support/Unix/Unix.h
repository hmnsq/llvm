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

#यदि_अपरिभाषितम् LLVM_LIB_SUPPORT_UNIX_UNIX_H
#परिभाषय LLVM_LIB_SUPPORT_UNIX_UNIX_H

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

#यदि_परिभाषितम् HAVE_UNISTD_H
#अन्तर्भाव <unistd.h>
#यदि_अन्तः

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
स्थिर अन्तःस्थ bool Make_Err_सन्देशः(
  std::string* Err_सन्देशः, ध्रुव std::string& उपसर्गः_2, पूर्णाङ्क errnum = -1) {
  यदि (!Err_सन्देशः)
    प्रत्यागम true;
  यदि (errnum == -1)
    errnum = errno;
  *Err_सन्देशः = उपसर्गः_2 + ": " + llvm::sys::StrError(errnum);
  प्रत्यागम true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] स्थिर अन्तःस्थ शून्य ReportErrnumFatal(ध्रुव वर्ण *सन्देशः_4, पूर्णाङ्क errnum) {
  std::string Err_सन्देशः;
  Make_Err_सन्देशः(&Err_सन्देशः, सन्देशः_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_सन्देशः));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
अन्तःस्थ std::chrono::microseconds गन्तव्यम्_Duration(ध्रुव संरचना timeval &TV) {
  प्रत्यागम std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
अन्तःस्थ संरचना timespec गन्तव्यम्_Time_Spec(TimePoint<> TP) {
  using namespace std::chrono;

  संरचना timespec RetVal;
  RetVal.tv_sec = गन्तव्यम्_Time_T(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).गणनम्();
  प्रत्यागम RetVal;
}

/// Convert a time point to struct timeval.
अन्तःस्थ संरचना timeval गन्तव्यम्_Time_Val(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  संरचना timeval RetVal;
  RetVal.tv_sec = गन्तव्यम्_Time_T(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).गणनम्();
  प्रत्यागम RetVal;
}

} // namespace sys
} // namespace llvm

#यदि_अन्तः
