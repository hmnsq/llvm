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

#समावेश "llvm/Support/Memory.h"
#समावेश "llvm/Config/llvm-config.h"

#अगर_अघोषित न_संशोधन
#समावेश "llvm/Support/raw_ostream.h"
#पूर्ण_अगर // ifndef NDEBUG

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Memory.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Memory.inc"
#पूर्ण_अगर

#अगर_अघोषित न_संशोधन

namespace llvm {
namespace sys {

raw_ostream &चालक<<(raw_ostream &OS, स्थिर Memory::ProtectionFlags &PF) {
  निश्चित((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  वापस OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &चालक<<(raw_ostream &OS, स्थिर MemoryBlock &MB) {
  वापस OS << "[ " << MB.base() << " .. "
            << (व्योम *)((अक्षर *)MB.base() + MB.allocated_आकार()) << " ] ("
            << MB.allocated_आकार() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#पूर्ण_अगर // ifndef NDEBUG
