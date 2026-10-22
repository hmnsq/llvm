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

#ضمن "llvm/Support/Errno.h"
#ضمن "llvm/Config/config.h"
#ضمن <cstring>
#ضمن <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

نطاق llvm {
نطاق sys {

std::string StrError() {
  أرجع StrError(errno);
}

std::string StrError(صحيح errnum) {
  std::string str;
  إذا (errnum == 0)
    أرجع str;
#إذا defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ثابت صحيح MaxErrStrLen = 2000;
  حرف میانگیر_2[MaxErrStrLen];
  میانگیر_2[0] = '\0';
#پایان_اگر

#اگر_تعریف_شده HAVE_STRERROR_R
  // strerror_r is thread-safe.
#إذا defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, میانگیر_2, MaxErrStrLen - 1);
#وإلا
  strerror_r(errnum, میانگیر_2, MaxErrStrLen - 1);
  str = میانگیر_2;
#پایان_اگر
#وإلاإذا HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(میانگیر_2, MaxErrStrLen - 1, errnum);
  str = میانگیر_2;
#وإلا
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#پایان_اگر
  أرجع str;
}

}  // namespace sys
}  // namespace llvm
