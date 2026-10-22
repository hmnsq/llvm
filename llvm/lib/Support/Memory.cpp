//===- Memory.cpp - Memory Handling Support ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines some helpful functions for allocating memory and dealing
// with memory mapped files
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Memory.h"
#ضمن "llvm/Config/llvm-config.h"

#اگر_تعریف_نشده NDEBUG
#ضمن "llvm/Support/raw_ostream.h"
#پایان_اگر // ifndef NDEBUG

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Memory.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Memory.inc"
#پایان_اگر

#اگر_تعریف_نشده NDEBUG

نطاق llvm {
نطاق sys {

raw_ostream &عملگر<<(raw_ostream &OS, ثابت حافظه_2::ProtectionFlags &PF) {
  أكد((PF & ~(حافظه_2::MF_READ | حافظه_2::MF_WRITE | حافظه_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  أرجع OS << (PF & حافظه_2::MF_READ ? 'R' : '-')
            << (PF & حافظه_2::MF_WRITE ? 'W' : '-')
            << (PF & حافظه_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &عملگر<<(raw_ostream &OS, ثابت حافظه_بلوک &MB) {
  أرجع OS << "[ " << MB.پایه_2() << " .. "
            << (فراغ *)((حرف *)MB.پایه_2() + MB.allocated_اندازه()) << " ] ("
            << MB.allocated_اندازه() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#پایان_اگر // ifndef NDEBUG
