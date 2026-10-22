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

#जर __has_include(<valgrind/valgrind.h>)
#अन्तर्भाव <valgrind/valgrind.h>

तार्किक llvm::sys::RunningOnValgrind() {
  परत RUNNING_ON_VALGRIND;
}

शून्य llvm::sys::ValgrindDiscardTranslations(स्थिरांक शून्य *Addr, माप_प्रकार Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#अन्यथा

तार्किक llvm::sys::RunningOnValgrind() {
  परत असत्य;
}

शून्य llvm::sys::ValgrindDiscardTranslations(स्थिरांक शून्य *Addr, माप_प्रकार Len) {
}

#endif
