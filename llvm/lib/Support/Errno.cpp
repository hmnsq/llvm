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
  ಹಿಂತಿರುಗಿ StrError(errno);
}

std::string StrError(ಪೂರ್ಣಾಂಕ errnum) {
  std::string str;
  ಆದರೆ (errnum == 0)
    ಹಿಂತಿರುಗಿ str;
#ಆದರೆ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ MaxErrStrLen = 2000;
  ಅಕ್ಷರ ಬಫರ್_2[MaxErrStrLen];
  ಬಫರ್_2[0] = '\0';
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_STRERROR_R
  // strerror_r is thread-safe.
#ಆದರೆ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, ಬಫರ್_2, MaxErrStrLen - 1);
#ಇಲ್ಲದಿದ್ದರೆ
  strerror_r(errnum, ಬಫರ್_2, MaxErrStrLen - 1);
  str = ಬಫರ್_2;
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(ಬಫರ್_2, MaxErrStrLen - 1, errnum);
  str = ಬಫರ್_2;
#ಇಲ್ಲದಿದ್ದರೆ
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ str;
}

}  // namespace sys
}  // namespace llvm
