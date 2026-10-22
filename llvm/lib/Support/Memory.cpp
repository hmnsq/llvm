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

#অন্তর্ভাব "llvm/Support/Memory.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"

#ifndef NDEBUG
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাব "Unix/Memory.inc"
#endif
#ifdef _WIN32
#অন্তর্ভাব "Windows/Memory.inc"
#endif

#ifndef NDEBUG

নামাকাশ llvm {
নামাকাশ sys {

raw_ostream &operator<<(raw_ostream &OS, ধ্রুবক Memory::ProtectionFlags &PF) {
  প্রতিজ্ঞা((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  ফেরত OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, ধ্রুবক MemoryBlock &MB) {
  ফেরত OS << "[ " << MB.base() << " .. "
            << (শূন্য *)((অক্ষর *)MB.base() + MB.allocated_আকার()) << " ] ("
            << MB.allocated_আকার() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#endif // ifndef NDEBUG
