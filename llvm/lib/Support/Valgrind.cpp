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

#અન્તર્ભાવ "llvm/Support/Valgrind.h"
#અન્તર્ભાવ <stddef.h>

#જો __has_include(<valgrind/valgrind.h>)
#અન્તર્ભાવ <valgrind/valgrind.h>

તાર્કિક llvm::sys::RunningOnValgrind() {
  પાછા RUNNING_ON_VALGRIND;
}

શૂન્ય llvm::sys::ValgrindDiscardTranslations(અચલ શૂન્ય *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#નહીંતર

તાર્કિક llvm::sys::RunningOnValgrind() {
  પાછા ખોટું;
}

શૂન્ય llvm::sys::ValgrindDiscardTranslations(અચલ શૂન્ય *Addr, size_t Len) {
}

#endif
