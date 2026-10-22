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

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
#ضمن "llvm/Support/raw_ostream.h"
#جيڪڏهن_پورو // ifndef NDEBUG

// Include the platform-specific parts of this class.
#جيڪڏهن_وصف_ٿيل LLVM_ON_UNIX
#ضمن "Unix/Memory.inc"
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "Windows/Memory.inc"
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل NDEBUG

نطاق llvm {
نطاق sys {

raw_ostream &عامل<<(raw_ostream &OS, ثابت يادگيري_2::ProtectionFlags &PF) {
  أكد((PF & ~(يادگيري_2::MF_READ | يادگيري_2::MF_WRITE | يادگيري_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  أرجع OS << (PF & يادگيري_2::MF_READ ? 'R' : '-')
            << (PF & يادگيري_2::MF_WRITE ? 'W' : '-')
            << (PF & يادگيري_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &عامل<<(raw_ostream &OS, ثابت يادگيري_بلاڪ &MB) {
  أرجع OS << "[ " << MB.بنياد_2() << " .. "
            << (فراغ *)((حرف *)MB.بنياد_2() + MB.allocated_سائيز()) << " ] ("
            << MB.allocated_سائيز() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#جيڪڏهن_پورو // ifndef NDEBUG
