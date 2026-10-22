//===-- Valgrind.cpp - Implement Valgrind communication ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Defines Valgrind communication methods if valgrind.h is available. If we
//  have valgrind.h but valgrind isn't running, its macros are no-ops.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Valgrind.h"
#شامل <stddef.h>

#اگر __has_include(<valgrind/valgrind.h>)
#شامل <valgrind/valgrind.h>

منطقی llvm::sys::RunningOnValgrind() {
  بازگشت RUNNING_ON_VALGRIND;
}

تهی llvm::sys::ValgrindDiscardTranslations(ثابت تهی *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#وگرنه

منطقی llvm::sys::RunningOnValgrind() {
  بازگشت نادرست;
}

تهی llvm::sys::ValgrindDiscardTranslations(ثابت تهی *Addr, size_t Len) {
}

#endif
