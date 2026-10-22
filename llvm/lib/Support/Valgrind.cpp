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

#అన్తర్భావ "llvm/Support/Valgrind.h"
#అన్తర్భావ <stddef.h>

#ఒకవేళ __has_include(<valgrind/valgrind.h>)
#అన్తర్భావ <valgrind/valgrind.h>

తార్కిక llvm::sys::RunningOnValgrind() {
  తిరిగి RUNNING_ON_VALGRIND;
}

శూన్య llvm::sys::ValgrindDiscardTranslations(స్థిరరాశి శూన్య *Addr, size_t Len) {
  VALGRIND_DISCARD_TRANSLATIONS(Addr, Len);
}

#లేకపోతే

తార్కిక llvm::sys::RunningOnValgrind() {
  తిరిగి అబద్ధం;
}

శూన్య llvm::sys::ValgrindDiscardTranslations(స్థిరరాశి శూన్య *Addr, size_t Len) {
}

#అయితే_ముగింపు
