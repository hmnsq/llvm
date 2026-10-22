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

#अन्तर्भाव "llvm/Support/Valgrind.h"
#अन्तर्भाव <stddef.h>

#जइ __has_include(<valgrind/valgrind.h>)
#अन्तर्भाव <valgrind/valgrind.h>

तार्किक llvm::sys::RunningOnValgrind() {
  प्रत्यागम RUNNING_ON_VALGRIND;
}

शून्य llvm::sys::ValgrindDiscardTranslations(ध्रुव शून्य *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#अण्णहा

तार्किक llvm::sys::RunningOnValgrind() {
  प्रत्यागम मिच्छ;
}

शून्य llvm::sys::ValgrindDiscardTranslations(ध्रुव शून्य *Addr, size_t Len) {
}

#endif
