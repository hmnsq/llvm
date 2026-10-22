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

#অন্তর্ভাব "llvm/Support/Errno.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব <cstring>
#অন্তর্ভাব <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

নামাকাশ llvm {
নামাকাশ sys {

std::string StrError() {
  প্রত্যাগম StrError(errno);
}

std::string StrError(পূর্ণাঙ্ক errnum) {
  std::string str;
  যদি (errnum == 0)
    প্রত্যাগম str;
#যদি defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ধ্রুব পূর্ণাঙ্ক MaxErrStrLen = 2000;
  বর্ণ buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#যদি defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#অন্যথা
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#নোচেদ্যদি HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#অন্যথা
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  প্রত্যাগম str;
}

}  // namespace sys
}  // namespace llvm
