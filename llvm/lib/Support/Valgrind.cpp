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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Valgrind.h"
#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಆದರೆ __has_include(<valgrind/valgrind.h>)
#ಅನ್ತರ್ಭಾವ <valgrind/valgrind.h>

ತಾರ್ಕಿಕ llvm::sys::RunningOnValgrind() {
  ಹಿಂತಿರುಗಿ RUNNING_ON_VALGRIND;
}

ಶೂನ್ಯ llvm::sys::ValgrindDiscardTranslations(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#ಇಲ್ಲದಿದ್ದರೆ

ತಾರ್ಕಿಕ llvm::sys::RunningOnValgrind() {
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ llvm::sys::ValgrindDiscardTranslations(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Addr, size_t Len) {
}

#ಆದರೆ_ಅಂತ್ಯ
