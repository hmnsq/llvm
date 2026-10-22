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

#include "llvm/Support/Valgrind.h"
#include <stddef.h>

#만약 __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>

bool llvm::sys::RunningOnValgrind() {
  반환 RUNNING_ON_VALGRIND;
}

void llvm::sys::ValgrindDiscardTranslations(const void *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#아니면

bool llvm::sys::RunningOnValgrind() {
  반환 false;
}

void llvm::sys::ValgrindDiscardTranslations(const void *Addr, size_t Len) {
}

#endif
