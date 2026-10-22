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

#अगर_अघोषित LLVM_LIB_SUPPORT_UNIX_UNIX_H
#घोषणा LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Chrono.h"
#समावेश "llvm/Support/Errno.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश <assert.h>
#समावेश <cerrno>
#समावेश <cstdio>
#समावेश <cstdlib>
#समावेश <cstring>
#समावेश <string>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>

#अगर_घोषित HAVE_UNISTD_H
#समावेश <unistd.h>
#पूर्ण_अगर

#समावेश <sys/time.h>
#समावेश <time.h>

#समावेश <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
अटल अंतरभूत bool Make_Err_संदेश(
  std::string* Err_संदेश, स्थिर std::string& prefix, पूर्णांक errnum = -1) {
  अगर (!Err_संदेश)
    वापस true;
  अगर (errnum == -1)
    errnum = त्रुटि_सं;
  *Err_संदेश = prefix + ": " + llvm::sys::StrError(errnum);
  वापस true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] अटल अंतरभूत व्योम ReportErrnumFatal(स्थिर अक्षर *संदेश_2, पूर्णांक errnum) {
  std::string Err_संदेश;
  Make_Err_संदेश(&Err_संदेश, संदेश_2, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_संदेश));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
अंतरभूत std::chrono::microseconds toDuration(स्थिर काष्ठा timeval &TV) {
  वापस std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
अंतरभूत काष्ठा timespec toTimeSpec(TimePoint<> TP) {
  using namespace std::chrono;

  काष्ठा timespec RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_nsec = (TP.time_since_epoch() % seconds(1)).गिनती();
  वापस RetVal;
}

/// Convert a time point to struct timeval.
अंतरभूत काष्ठा timeval toTimeVal(TimePoint<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  काष्ठा timeval RetVal;
  RetVal.tv_sec = toTimeT(TP);
  RetVal.tv_usec = (TP.time_since_epoch() % seconds(1)).गिनती();
  वापस RetVal;
}

} // namespace sys
} // namespace llvm

#पूर्ण_अगर
