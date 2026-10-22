//===- Errno.cpp - errno support --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the errno wrappers.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Errno.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {

std::string StrError() {
  ಪ್ರತ್ಯಾಗಮ StrError(errno);
}

std::string StrError(ಪೂರ್ಣಾಙ್ಕ errnum) {
  std::string str;
  ಯದಿ (errnum == 0)
    ಪ್ರತ್ಯಾಗಮ str;
#ಯದಿ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ MaxErrStrLen = 2000;
  ವರ್ಣ buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#ಯದಿ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#ಅನ್ಯಥಾ
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#ನೋಚೇದ್ಯದಿ HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#ಅನ್ಯಥಾ
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  ಪ್ರತ್ಯಾಗಮ str;
}

}  // namespace sys
}  // namespace llvm
