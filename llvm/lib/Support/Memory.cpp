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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಆದರೆ_ಅಂತ್ಯ // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Memory.inc"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Memory.inc"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {

raw_ostream &ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_2::ProtectionFlags &PF) {
  ಪ್ರತಿಜ್ಞಾ((PF & ~(ಸ್ಮರಣೆ_2::MF_READ | ಸ್ಮರಣೆ_2::MF_WRITE | ಸ್ಮರಣೆ_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  ಹಿಂತಿರುಗಿ OS << (PF & ಸ್ಮರಣೆ_2::MF_READ ? 'R' : '-')
            << (PF & ಸ್ಮರಣೆ_2::MF_WRITE ? 'W' : '-')
            << (PF & ಸ್ಮರಣೆ_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಖಂಡ &MB) {
  ಹಿಂತಿರುಗಿ OS << "[ " << MB.ತಳ_2() << " .. "
            << (ಶೂನ್ಯ *)((ಅಕ್ಷರ *)MB.ತಳ_2() + MB.allocated_ಗಾತ್ರ()) << " ] ("
            << MB.allocated_ಗಾತ್ರ() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#ಆದರೆ_ಅಂತ್ಯ // ifndef NDEBUG
