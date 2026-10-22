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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Valgrind.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਜੇ __has_include(<valgrind/valgrind.h>)
#ਅਨ੍ਤਰ੍ਭਾਵ <valgrind/valgrind.h>

ਤਾਰਕਿਕ llvm::sys::RunningOnValgrind() {
  ਵਾਪਸ RUNNING_ON_VALGRIND;
}

ਖਾਲੀ llvm::sys::ValgrindDiscardTranslations(ਅਚਲ ਖਾਲੀ *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#ਨਹੀਂਤਾਂ

ਤਾਰਕਿਕ llvm::sys::RunningOnValgrind() {
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ llvm::sys::ValgrindDiscardTranslations(ਅਚਲ ਖਾਲੀ *Addr, size_t Len) {
}

#ਜੇ_ਅੰਤ
