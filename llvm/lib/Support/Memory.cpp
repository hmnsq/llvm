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

#અન્તર્ભાવ "llvm/Support/Memory.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#અન્તર્ભાવ "Unix/Memory.inc"
#endif
#ifdef _WIN32
#અન્તર્ભાવ "Windows/Memory.inc"
#endif

#ifndef NDEBUG

નામાકાશ llvm {
નામાકાશ sys {

raw_ostream &operator<<(raw_ostream &OS, અચલ Memory::ProtectionFlags &PF) {
  પ્રતિજ્ઞા((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  પાછા OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, અચલ MemoryBlock &MB) {
  પાછા OS << "[ " << MB.base() << " .. "
            << (શૂન્ય *)((અક્ષર *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
