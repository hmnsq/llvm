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

#include "llvm/Support/Errno.h"
#include "llvm/Config/config.h"
#include <cstring>
#include <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

namespace llvm {
namespace sys {

std::string StrError() {
  ပြန်ပေး StrError(errno);
}

std::string StrError(ကိန်းပြည့် errnum) {
  std::string str;
  အကယ်၍ (errnum == 0)
    ပြန်ပေး str;
#အကယ်၍ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  const ကိန်းပြည့် MaxErrStrLen = 2000;
  var_na buffer[MaxErrStrLen];
  buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
#အကယ်၍ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, buffer, MaxErrStrLen - 1);
#မဟုတ်ပါက
  strerror_r(errnum, buffer, MaxErrStrLen - 1);
  str = buffer;
#endif
#elif HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(buffer, MaxErrStrLen - 1, errnum);
  str = buffer;
#မဟုတ်ပါက
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#endif
  ပြန်ပေး str;
}

}  // namespace sys
}  // namespace llvm
