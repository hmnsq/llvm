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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errno.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ନାମାକାଶ llvm {
ନାମାକାଶ sys {

std::string StrError() {
  ଫେରନ୍ତୁ StrError(errno);
}

std::string StrError(ପୂର୍ଣ୍ଣସଂଖ୍ୟା errnum) {
  std::string str;
  ଯଦି (errnum == 0)
    ଫେରନ୍ତୁ str;
#ଯଦି defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxErrStrLen = 2000;
  ଅକ୍ଷର buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#ଯଦି defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#ଅନ୍ୟଥା
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#ନଚେତଯଦି HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#ଅନ୍ୟଥା
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  ଫେରନ୍ତୁ str;
}

}  // namespace sys
}  // namespace llvm
