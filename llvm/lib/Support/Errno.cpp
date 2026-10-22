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
  𐡄𐡍 (errnum == 0)
    أرجع str;
#𐡄𐡍 defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  ثابت صحيح MaxErrStrLen = 2000;
  حرف buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#𐡄𐡍 defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#وإلا
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#وإلاإذا HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#وإلا
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  أرجع str;
}

}  // namespace sys
}  // namespace llvm
