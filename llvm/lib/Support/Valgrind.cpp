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

#അന്തര്ഭാവ "llvm/Support/Valgrind.h"
#അന്തര്ഭാവ <stddef.h>

#എങ്കിൽ __has_include(<valgrind/valgrind.h>)
#അന്തര്ഭാവ <valgrind/valgrind.h>

തർക്കം llvm::sys::RunningOnValgrind() {
  തിരികെ RUNNING_ON_VALGRIND;
}

ശൂന്യം llvm::sys::ValgrindDiscardTranslations(സ്ഥിരം ശൂന്യം *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#അല്ലെങ്കിൽ

തർക്കം llvm::sys::RunningOnValgrind() {
  തിരികെ അസത്യം;
}

ശൂന്യം llvm::sys::ValgrindDiscardTranslations(സ്ഥിരം ശൂന്യം *Addr, size_t Len) {
}

#endif
