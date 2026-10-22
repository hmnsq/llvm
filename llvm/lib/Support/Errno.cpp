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

فضاینام llvm {
فضاینام sys {

std::string StrError() {
  بازگشت StrError(errno);
}

std::string StrError(صحیح errnum) {
  std::string str;
  اگر (errnum == 0)
    بازگشت str;
#اگر defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ثابت صحیح MaxErrStrLen = 2000;
  نویسه buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#اگر defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#وگرنه
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#وگرنهاگر HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#وگرنه
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  بازگشت str;
}

}  // namespace sys
}  // namespace llvm
