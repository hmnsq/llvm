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

#ضمن "llvm/Support/Valgrind.h"
#ضمن <stddef.h>

#ܐܢ __has_include(<valgrind/valgrind.h>)
#ضمن <valgrind/valgrind.h>

منطقي llvm::sys::RunningOnValgrind() {
  ܦܢܐ RUNNING_ON_VALGRIND;
}

فراغ llvm::sys::ValgrindDiscardTranslations(ثابت فراغ *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#ܐܚܪܢܐ

منطقي llvm::sys::RunningOnValgrind() {
  ܦܢܐ ܕܓܠܐ;
}

فراغ llvm::sys::ValgrindDiscardTranslations(ثابت فراغ *Addr, size_t Len) {
}

#endif
