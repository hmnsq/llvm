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
  ফেরত StrError(errno);
}

std::string StrError(পূর্ণসংখ্যা errnum) {
  std::string str;
  যদি (errnum == 0)
    ফেরত str;
#যদি defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ধ্রুবক পূর্ণসংখ্যা MaxErrStrLen = 2000;
  অক্ষর বাফার_2[MaxErrStrLen];
  বাফার_2[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#যদি defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, বাফার_2, MaxErrStrLen - 1);
#অন্যথা
  strerror_r(errnum, বাফার_2, MaxErrStrLen - 1);
  str = বাফার_2;
#endif
#নতুবা_যদি HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(বাফার_2, MaxErrStrLen - 1, errnum);
  str = বাফার_2;
#অন্যথা
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  ফেরত str;
}

}  // namespace sys
}  // namespace llvm
