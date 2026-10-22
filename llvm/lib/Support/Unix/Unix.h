//===- llvm/Support/Unix/Unix.h - Common Unix Include File -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines things specific to Unix implementations.
//
//===----------------------------------------------------------------------===//

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LLVM_LIB_SUPPORT_UNIX_UNIX_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LLVM_LIB_SUPPORT_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Chrono.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errno.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cerrno>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdio>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UNISTD_H
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

# include <fcntl.h>

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
ਸਥਿਰ ਸਤਰਅੰਦਰ bool Make_Err_ਸੁਨੇਹਾ(
  std::string* Err_ਸੁਨੇਹਾ, ਅਚਲ std::string& ਅਗੇਤਰ_2, ਪੂਰਨ errnum = -1) {
  ਜੇ (!Err_ਸੁਨੇਹਾ)
    ਵਾਪਸ true;
  ਜੇ (errnum == -1)
    errnum = errno;
  *Err_ਸੁਨੇਹਾ = ਅਗੇਤਰ_2 + ": " + llvm::sys::StrError(errnum);
  ਵਾਪਸ true;
}

// Include StrError(errnum) in a fatal error message.
[[noreturn]] ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ReportErrnumFatal(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_4, ਪੂਰਨ errnum) {
  std::string Err_ਸੁਨੇਹਾ;
  Make_Err_ਸੁਨੇਹਾ(&Err_ਸੁਨੇਹਾ, ਸੁਨੇਹਾ_4, errnum);
  llvm::report_fatal_error(llvm::Twine(Err_ਸੁਨੇਹਾ));
}

namespace llvm {
namespace sys {

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
ਸਤਰਅੰਦਰ std::chrono::microseconds ਮੰਜ਼ਿਲ_Duration(ਅਚਲ ਬਣਤਰ timeval &TV) {
  ਵਾਪਸ std::chrono::seconds(TV.tv_sec) +
         std::chrono::microseconds(TV.tv_usec);
}

/// Convert a time point to struct timespec.
ਸਤਰਅੰਦਰ ਬਣਤਰ timespec ਮੰਜ਼ਿਲ_ਸਮਾਂ_Spec(ਸਮਾਂ_ਬਿੰਦੂ<> TP) {
  using namespace std::chrono;

  ਬਣਤਰ timespec RetVal;
  RetVal.tv_sec = ਮੰਜ਼ਿਲ_ਸਮਾਂ_T(TP);
  RetVal.tv_nsec = (TP.ਸਮਾਂ_since_epoch() % seconds(1)).ਗਿਣਤੀ_2();
  ਵਾਪਸ RetVal;
}

/// Convert a time point to struct timeval.
ਸਤਰਅੰਦਰ ਬਣਤਰ timeval ਮੰਜ਼ਿਲ_ਸਮਾਂ_Val(ਸਮਾਂ_ਬਿੰਦੂ<std::chrono::microseconds> TP) {
  using namespace std::chrono;

  ਬਣਤਰ timeval RetVal;
  RetVal.tv_sec = ਮੰਜ਼ਿਲ_ਸਮਾਂ_T(TP);
  RetVal.tv_usec = (TP.ਸਮਾਂ_since_epoch() % seconds(1)).ਗਿਣਤੀ_2();
  ਵਾਪਸ RetVal;
}

} // namespace sys
} // namespace llvm

#ਜੇ_ਅੰਤ
