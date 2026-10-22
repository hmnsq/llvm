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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Valgrind.h"
#ଅନ୍ତର୍ଭାଵ <stddef.h>

#ଯଦି __has_include(<valgrind/valgrind.h>)
#ଅନ୍ତର୍ଭାଵ <valgrind/valgrind.h>

ତାର୍କିକ llvm::sys::RunningOnValgrind() {
  ଫେରନ୍ତୁ RUNNING_ON_VALGRIND;
}

ଶୂନ୍ୟ llvm::sys::ValgrindDiscardTranslations(ଅଚଳ ଶୂନ୍ୟ *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#ଅନ୍ୟଥା

ତାର୍କିକ llvm::sys::RunningOnValgrind() {
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ llvm::sys::ValgrindDiscardTranslations(ଅଚଳ ଶୂନ୍ୟ *Addr, size_t Len) {
}

#endif
