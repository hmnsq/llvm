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

#समावेश "llvm/Support/Valgrind.h"
#समावेश <stddef.h>

#अगर __has_include(<valgrind/valgrind.h>)
#समावेश <valgrind/valgrind.h>

bool llvm::sys::RunningOnValgrind() {
  वापस RUNNING_ON_VALGRIND;
}

व्योम llvm::sys::ValgrindDiscardTranslations(स्थिर व्योम *Addr, माप_प्रकार Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#अन्यथा

bool llvm::sys::RunningOnValgrind() {
  वापस false;
}

व्योम llvm::sys::ValgrindDiscardTranslations(स्थिर व्योम *Addr, माप_प्रकार Len) {
}

#पूर्ण_अगर
