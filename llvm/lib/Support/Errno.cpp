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

#அந்தர்பாவ "llvm/Support/Errno.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

நாமாகாஶ llvm {
நாமாகாஶ sys {

std::string StrError() {
  திரும்பு StrError(errno);
}

std::string StrError(முழு errnum) {
  std::string str;
  எனில் (errnum == 0)
    திரும்பு str;
#எனில் defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  மாறிலி முழு MaxErrStrLen = 2000;
  எழுத்து buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#எனில் defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#இல்லையெனில்
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#அல்லதெனில் HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#இல்லையெனில்
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  திரும்பு str;
}

}  // namespace sys
}  // namespace llvm
