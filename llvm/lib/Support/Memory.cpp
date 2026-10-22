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

#അന്തര്ഭാവ "llvm/Support/Memory.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/Memory.inc"
#endif
#ifdef _WIN32
#അന്തര്ഭാവ "Windows/Memory.inc"
#endif

#ifndef NDEBUG

നാമാകാശ llvm {
നാമാകാശ sys {

raw_ostream &operator<<(raw_ostream &OS, സ്ഥിരം Memory::ProtectionFlags &PF) {
  പ്രതിജ്ഞാ((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  തിരികെ OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, സ്ഥിരം MemoryBlock &MB) {
  തിരികെ OS << "[ " << MB.base() << " .. "
            << (ശൂന്യം *)((അക്ഷരം *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
