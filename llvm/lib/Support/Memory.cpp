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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Memory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਜੇ_ਅੰਤ // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Memory.inc"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Memory.inc"
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {

raw_ostream &ਸੰਚਾਲਕ<<(raw_ostream &OS, ਅਚਲ ਸਮ੍ਰਿਤੀ_2::ProtectionFlags &PF) {
  ਪ੍ਰਤਿਜ੍ਞਾ((PF & ~(ਸਮ੍ਰਿਤੀ_2::MF_READ | ਸਮ੍ਰਿਤੀ_2::MF_WRITE | ਸਮ੍ਰਿਤੀ_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  ਵਾਪਸ OS << (PF & ਸਮ੍ਰਿਤੀ_2::MF_READ ? 'R' : '-')
            << (PF & ਸਮ੍ਰਿਤੀ_2::MF_WRITE ? 'W' : '-')
            << (PF & ਸਮ੍ਰਿਤੀ_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &ਸੰਚਾਲਕ<<(raw_ostream &OS, ਅਚਲ ਸਮ੍ਰਿਤੀ_ਖੰਡ &MB) {
  ਵਾਪਸ OS << "[ " << MB.ਅਧਾਰ_2() << " .. "
            << (ਖਾਲੀ *)((ਅੱਖਰ *)MB.ਅਧਾਰ_2() + MB.allocated_ਮਾਪ()) << " ] ("
            << MB.allocated_ਮਾਪ() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#ਜੇ_ਅੰਤ // ifndef NDEBUG
