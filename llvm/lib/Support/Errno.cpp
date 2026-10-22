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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errno.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {

std::string StrError() {
  ਵਾਪਸ StrError(errno);
}

std::string StrError(ਪੂਰਨ errnum) {
  std::string str;
  ਜੇ (errnum == 0)
    ਵਾਪਸ str;
#ਜੇ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ਅਚਲ ਪੂਰਨ MaxErrStrLen = 2000;
  ਅੱਖਰ ਬਫਰ_2[MaxErrStrLen];
  ਬਫਰ_2[0] = '\0';
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_STRERROR_R
  // strerror_r is thread-safe.
#ਜੇ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, ਬਫਰ_2, MaxErrStrLen - 1);
#ਨਹੀਂਤਾਂ
  strerror_r(errnum, ਬਫਰ_2, MaxErrStrLen - 1);
  str = ਬਫਰ_2;
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(ਬਫਰ_2, MaxErrStrLen - 1, errnum);
  str = ਬਫਰ_2;
#ਨਹੀਂਤਾਂ
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ str;
}

}  // namespace sys
}  // namespace llvm
