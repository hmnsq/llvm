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

#অন্তর্ভাব "llvm/Support/Valgrind.h"
#অন্তর্ভাব <stddef.h>

#যদি __has_include(<valgrind/valgrind.h>)
#অন্তর্ভাব <valgrind/valgrind.h>

যৌক্তিক llvm::sys::RunningOnValgrind() {
  ফেরত RUNNING_ON_VALGRIND;
}

শূন্য llvm::sys::ValgrindDiscardTranslations(ধ্রুবক শূন্য *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#অন্যথা

যৌক্তিক llvm::sys::RunningOnValgrind() {
  ফেরত মিথ্যা;
}

শূন্য llvm::sys::ValgrindDiscardTranslations(ধ্রুবক শূন্য *Addr, size_t Len) {
}

#endif
