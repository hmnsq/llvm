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

#அந்தர்பாவ "llvm/Support/Valgrind.h"
#அந்தர்பாவ <stddef.h>

#எனில் __has_include(<valgrind/valgrind.h>)
#அந்தர்பாவ <valgrind/valgrind.h>

தருக்க llvm::sys::RunningOnValgrind() {
  திரும்பு RUNNING_ON_VALGRIND;
}

வெறுமை llvm::sys::ValgrindDiscardTranslations(மாறிலி வெறுமை *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#இல்லையெனில்

தருக்க llvm::sys::RunningOnValgrind() {
  திரும்பு பொய்;
}

வெறுமை llvm::sys::ValgrindDiscardTranslations(மாறிலி வெறுமை *Addr, size_t Len) {
}

#endif
