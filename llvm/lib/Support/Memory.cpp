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

#جے_بنا_تعریف NDEBUG
#ضمن "llvm/Support/raw_ostream.h"
#جے_انت // ifndef NDEBUG

// Include the platform-specific parts of this class.
#جے_تعریف_شدہ LLVM_ON_UNIX
#ضمن "Unix/Memory.inc"
#جے_انت
#جے_تعریف_شدہ _WIN32
#ضمن "Windows/Memory.inc"
#جے_انت

#جے_بنا_تعریف NDEBUG

نطاق llvm {
نطاق sys {

raw_ostream &عامل<<(raw_ostream &OS, ثابت حافظہ_2::ProtectionFlags &PF) {
  أكد((PF & ~(حافظہ_2::MF_READ | حافظہ_2::MF_WRITE | حافظہ_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  أرجع OS << (PF & حافظہ_2::MF_READ ? 'R' : '-')
            << (PF & حافظہ_2::MF_WRITE ? 'W' : '-')
            << (PF & حافظہ_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &عامل<<(raw_ostream &OS, ثابت حافظہ_بلاک &MB) {
  أرجع OS << "[ " << MB.ادھار_2() << " .. "
            << (فراغ *)((حرف *)MB.ادھار_2() + MB.allocated_ناپ()) << " ] ("
            << MB.allocated_ناپ() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#جے_انت // ifndef NDEBUG
