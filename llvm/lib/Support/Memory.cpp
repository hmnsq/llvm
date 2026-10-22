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

#यदि_अपरिभाषितम् NDEBUG
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#यदि_अन्तः // ifndef NDEBUG

// Include the platform-specific parts of this class.
#यदि_परिभाषितम् LLVM_ON_UNIX
#अन्तर्भाव "Unix/Memory.inc"
#यदि_अन्तः
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "Windows/Memory.inc"
#यदि_अन्तः

#यदि_अपरिभाषितम् NDEBUG

नामाकाश llvm {
नामाकाश sys {

raw_ostream &संक्रियकः<<(raw_ostream &OS, ध्रुव स्मृतिः_2::ProtectionFlags &PF) {
  प्रतिज्ञा((PF & ~(स्मृतिः_2::MF_READ | स्मृतिः_2::MF_WRITE | स्मृतिः_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  प्रत्यागम OS << (PF & स्मृतिः_2::MF_READ ? 'R' : '-')
            << (PF & स्मृतिः_2::MF_WRITE ? 'W' : '-')
            << (PF & स्मृतिः_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &संक्रियकः<<(raw_ostream &OS, ध्रुव स्मृतिः_खण्डम् &MB) {
  प्रत्यागम OS << "[ " << MB.आधारः_2() << " .. "
            << (शून्य *)((वर्ण *)MB.आधारः_2() + MB.allocated_आकार()) << " ] ("
            << MB.allocated_आकार() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#यदि_अन्तः // ifndef NDEBUG
