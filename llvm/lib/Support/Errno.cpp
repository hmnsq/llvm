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

#അന്തര്ഭാവ "llvm/Support/Errno.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

നാമാകാശ llvm {
നാമാകാശ sys {

std::string StrError() {
  തിരികെ StrError(errno);
}

std::string StrError(പൂർണ്ണസംഖ്യ errnum) {
  std::string str;
  എങ്കിൽ (errnum == 0)
    തിരികെ str;
#എങ്കിൽ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  സ്ഥിരം പൂർണ്ണസംഖ്യ MaxErrStrLen = 2000;
  അക്ഷരം buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#എങ്കിൽ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#അല്ലെങ്കിൽ
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#അല്ലെങ്കിലെങ്കിൽ HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#അല്ലെങ്കിൽ
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  തിരികെ str;
}

}  // namespace sys
}  // namespace llvm
