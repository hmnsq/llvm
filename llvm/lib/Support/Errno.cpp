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

#અન્તર્ભાવ "llvm/Support/Errno.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

નામાકાશ llvm {
નામાકાશ sys {

std::string StrError() {
  પાછા StrError(errno);
}

std::string StrError(પૂર્ણાંક errnum) {
  std::string str;
  જો (errnum == 0)
    પાછા str;
#જો defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  અચલ પૂર્ણાંક MaxErrStrLen = 2000;
  અક્ષર buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#જો defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#નહીંતર
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#અથવાજો HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#નહીંતર
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  પાછા str;
}

}  // namespace sys
}  // namespace llvm
