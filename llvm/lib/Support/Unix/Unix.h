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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LLVM_LIB_SUPPORT_UNIX_UNIX_H
#ವ್ಯಾಖ್ಯಾನಿಸು LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Chrono.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errno.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <cerrno>
#ಅನ್ತರ್ಭಾವ <cstdio>
#ಅನ್ತರ್ಭಾವ <cstdlib>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_UNISTD_H
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <time.h>

#ಅನ್ತರ್ಭಾವ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool Make_Err_ಸಂದೇಶ(
  std::string* Err_ಸಂದೇಶ, ಸ್ಥಿರಾಂಕ std::string& ಪೂರ್ವಪ್ರತ್ಯಯ_2, ಪೂರ್ಣಾಂಕ errnum = -1) {
  ಆದರೆ (!Err_ಸಂದೇಶ)
    ಹಿಂತಿರುಗಿ true;
  ಆದರೆ (errnum == -1)
    errnum = errno;
  *Err_ಸಂದೇಶ = ಪೂರ್ವಪ್ರತ್ಯಯ_2 + ": " + llvm::sys::StrError(errnum);
  ಹಿಂತಿರುಗಿ true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ReportErrnumFatal(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_4, ಪೂರ್ಣಾಂಕ errnum) {
  std::string Err_ಸಂದೇಶ;
  Make_Err_ಸಂದೇಶ(&Err_ಸಂದೇಶ, ಸಂದೇಶ_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_ಸಂದೇಶ));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
ಸಾಲೊಳಗೆ std::chrono::microseconds ಗಮ್ಯ_Duration(ಸ್ಥಿರಾಂಕ ರಚನೆ timeval &TV) {
  ಹಿಂತಿರುಗಿ std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
ಸಾಲೊಳಗೆ ರಚನೆ timespec ಗಮ್ಯ_ಸಮಯ_Spec(ಸಮಯ_ಬಿಂದು<> TP) {
  using namespace std::chrono;

  ರಚನೆ timespec RetVal;
  RetVal.tv_sec = ಗಮ್ಯ_ಸಮಯ_T(TP);
  RetVal.tv_nsec = (TP.ಸಮಯ_since_epoch() % seconds(1)).ಎಣಿಕೆ_2();
  ಹಿಂತಿರುಗಿ RetVal;
}

/// Convert a time point to struct timeval.
ಸಾಲೊಳಗೆ ರಚನೆ timeval ಗಮ್ಯ_ಸಮಯ_Val(ಸಮಯ_ಬಿಂದು<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ರಚನೆ timeval RetVal;
  RetVal.tv_sec = ಗಮ್ಯ_ಸಮಯ_T(TP);
  RetVal.tv_usec = (TP.ಸಮಯ_since_epoch() % seconds(1)).ಎಣಿಕೆ_2();
  ಹಿಂತಿರುಗಿ RetVal;
}

} // namespace sys
} // namespace llvm

#ಆದರೆ_ಅಂತ್ಯ
