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

#شامل "llvm/Support/Memory.h"
#شامل "llvm/Config/llvm-config.h"

#اگر_غیر_تعریف_شدہ NDEBUG
#شامل "llvm/Support/raw_ostream.h"
#اگر_ختم // ifndef NDEBUG

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/Memory.inc"
#اگر_ختم
#اگر_تعریف_شدہ _WIN32
#شامل "Windows/Memory.inc"
#اگر_ختم

#اگر_غیر_تعریف_شدہ NDEBUG

نامخلا llvm {
نامخلا sys {

raw_ostream &عامل<<(raw_ostream &OS, ثابت حافظہ_2::ProtectionFlags &PF) {
  أكد((PF & ~(حافظہ_2::MF_READ | حافظہ_2::MF_WRITE | حافظہ_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  واپس OS << (PF & حافظہ_2::MF_READ ? 'R' : '-')
            << (PF & حافظہ_2::MF_WRITE ? 'W' : '-')
            << (PF & حافظہ_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &عامل<<(raw_ostream &OS, ثابت حافظہ_بلاک &MB) {
  واپس OS << "[ " << MB.بنیاد_2() << " .. "
            << (خالی *)((حرف *)MB.بنیاد_2() + MB.allocated_سائز()) << " ] ("
            << MB.allocated_سائز() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#اگر_ختم // ifndef NDEBUG
