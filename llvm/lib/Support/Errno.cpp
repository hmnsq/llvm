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

#समावेश "llvm/Support/Errno.h"
#समावेश "llvm/Config/config.h"
#समावेश <cstring>
#समावेश <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

namespace llvm {
namespace sys {

std::string StrError() {
  वापस StrError(त्रुटि_सं);
}

std::string StrError(पूर्णांक errnum) {
  std::string str;
  अगर (errnum == 0)
    वापस str;
#अगर defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  स्थिर पूर्णांक MaxErrStrLen = 2000;
  अक्षर बफर_2[MaxErrStrLen];
  बफर_2[0] = '\0';
#पूर्ण_अगर

#अगर_घोषित HAVE_STRERROR_R
  // strerror_r is thread-safe.
#अगर defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, बफर_2, MaxErrStrLen - 1);
#अन्यथा
  strerror_r(errnum, बफर_2, MaxErrStrLen - 1);
  str = बफर_2;
#पूर्ण_अगर
#या_अगर HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(बफर_2, MaxErrStrLen - 1, errnum);
  str = बफर_2;
#अन्यथा
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = म_त्रुटि(errnum);
#पूर्ण_अगर
  वापस str;
}

}  // namespace sys
}  // namespace llvm
