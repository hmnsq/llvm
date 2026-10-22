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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Memory.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Memory.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Memory.inc"
#endif

#ifndef NDEBUG

ନାମାକାଶ llvm {
ନାମାକାଶ sys {

raw_ostream &operator<<(raw_ostream &OS, ଅଚଳ Memory::ProtectionFlags &PF) {
  ପ୍ରତିଜ୍ଞା((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  ଫେରନ୍ତୁ OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, ଅଚଳ MemoryBlock &MB) {
  ଫେରନ୍ତୁ OS << "[ " << MB.base() << " .. "
            << (ଶୂନ୍ୟ *)((ଅକ୍ଷର *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
