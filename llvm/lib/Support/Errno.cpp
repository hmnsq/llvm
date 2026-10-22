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

#شامل "llvm/Support/Errno.h"
#شامل "llvm/Config/config.h"
#شامل <cstring>
#شامل <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

نامخلا llvm {
نامخلا sys {

std::string StrError() {
  واپس StrError(errno);
}

std::string StrError(صحیح errnum) {
  std::string str;
  اگر (errnum == 0)
    واپس str;
#اگر defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ثابت صحیح MaxErrStrLen = 2000;
  حرف بفر_2[MaxErrStrLen];
  بفر_2[0] = '\0';
#اگر_ختم

#اگر_تعریف_شدہ HAVE_STRERROR_R
  // strerror_r is thread-safe.
#اگر defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, بفر_2, MaxErrStrLen - 1);
#ورنہ
  strerror_r(errnum, بفر_2, MaxErrStrLen - 1);
  str = بفر_2;
#اگر_ختم
#وگرنہ_اگر HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(بفر_2, MaxErrStrLen - 1, errnum);
  str = بفر_2;
#ورنہ
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#اگر_ختم
  واپس str;
}

}  // namespace sys
}  // namespace llvm
