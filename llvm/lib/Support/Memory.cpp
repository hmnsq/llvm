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

#அந்தர்பாவ "llvm/Support/Memory.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#அந்தர்பாவ "Unix/Memory.inc"
#endif
#ifdef _WIN32
#அந்தர்பாவ "Windows/Memory.inc"
#endif

#ifndef NDEBUG

நாமாகாஶ llvm {
நாமாகாஶ sys {

raw_ostream &operator<<(raw_ostream &OS, மாறிலி Memory::ProtectionFlags &PF) {
  ப்ரதிஜ்ஞா((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  திரும்பு OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, மாறிலி MemoryBlock &MB) {
  திரும்பு OS << "[ " << MB.base() << " .. "
            << (வெறுமை *)((எழுத்து *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
