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

#अन्तर्भाव "llvm/Support/Memory.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"

#ifndef न_संशोधन
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Memory.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Memory.inc"
#endif

#ifndef न_संशोधन

नामाकाश llvm {
नामाकाश sys {

raw_ostream &चालक<<(raw_ostream &OS, स्थिरांक Memory::ProtectionFlags &PF) {
  प्रतिज्ञा((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  परत OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &चालक<<(raw_ostream &OS, स्थिरांक MemoryBlock &MB) {
  परत OS << "[ " << MB.base() << " .. "
            << (शून्य *)((अक्षर *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
