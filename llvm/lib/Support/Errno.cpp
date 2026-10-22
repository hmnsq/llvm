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

#अन्तर्भाव "llvm/Support/Errno.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव <cstring>
#अन्तर्भाव <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

नामाकाश llvm {
नामाकाश sys {

std::string StrError() {
  प्रत्यागम StrError(errno);
}

std::string StrError(पूर्णाङ्क errnum) {
  std::string str;
  यदि (errnum == 0)
    प्रत्यागम str;
#यदि defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ध्रुव पूर्णाङ्क MaxErrStrLen = 2000;
  वर्ण बफर_2[MaxErrStrLen];
  बफर_2[0] = '\0';
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_STRERROR_R
  // strerror_r is thread-safe.
#यदि defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, बफर_2, MaxErrStrLen - 1);
#अन्यथा
  strerror_r(errnum, बफर_2, MaxErrStrLen - 1);
  str = बफर_2;
#यदि_अन्तः
#नोचेद्यदि HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(बफर_2, MaxErrStrLen - 1, errnum);
  str = बफर_2;
#अन्यथा
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#यदि_अन्तः
  प्रत्यागम str;
}

}  // namespace sys
}  // namespace llvm
