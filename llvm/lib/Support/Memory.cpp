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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Memory.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Memory.inc"
#endif
#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Memory.inc"
#endif

#ifndef NDEBUG

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {

raw_ostream &operator<<(raw_ostream &OS, ಧ್ರುವ Memory::ProtectionFlags &PF) {
  ಪ್ರತಿಜ್ಞಾ((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  ಪ್ರತ್ಯಾಗಮ OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, ಧ್ರುವ MemoryBlock &MB) {
  ಪ್ರತ್ಯಾಗಮ OS << "[ " << MB.base() << " .. "
            << (ಶೂನ್ಯ *)((ವರ್ಣ *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
