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

#అన్తర్భావ "llvm/Support/Errno.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ <cstring>
#అన్తర్భావ <errno.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

నామాకాశ llvm {
నామాకాశ sys {

std::string StrError() {
  తిరిగి StrError(errno);
}

std::string StrError(పూర్ణాంకం errnum) {
  std::string str;
  ఒకవేళ (errnum == 0)
    తిరిగి str;
#ఒకవేళ defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
  స్థిరరాశి పూర్ణాంకం MaxErrStrLen = 2000;
  అక్షరం బఫరు_2[MaxErrStrLen];
  బఫరు_2[0] = '\0';
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_STRERROR_R
  // strerror_r is thread-safe.
#ఒకవేళ defined(__GLIBC__) && defined(_GNU_SOURCE)
  // glibc defines its own incompatible version of strerror_r
  // which may not use the buffer supplied.
  str = strerror_r(errnum, బఫరు_2, MaxErrStrLen - 1);
#లేకపోతే
  strerror_r(errnum, బఫరు_2, MaxErrStrLen - 1);
  str = బఫరు_2;
#అయితే_ముగింపు
#లేదావేళ HAVE_DECL_STRERROR_S // "Windows Secure API"
  strerror_s(బఫరు_2, MaxErrStrLen - 1, errnum);
  str = బఫరు_2;
#లేకపోతే
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  str = strerror(errnum);
#అయితే_ముగింపు
  తిరిగి str;
}

}  // namespace sys
}  // namespace llvm
