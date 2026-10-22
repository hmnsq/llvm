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

#ifndef NDEBUG
#ضمن "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ضمن "Unix/Memory.inc"
#endif
#ifdef _WIN32
#ضمن "Windows/Memory.inc"
#endif

#ifndef NDEBUG

نطاق llvm {
نطاق sys {

raw_ostream &operator<<(raw_ostream &OS, ثابت Memory::ProtectionFlags &PF) {
  أكد((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  החזר OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, ثابت MemoryBlock &MB) {
  החזר OS << "[ " << MB.base() << " .. "
            << (فراغ *)((حرف *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
