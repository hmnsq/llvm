//===-- DebugOptions.h - Global Command line opt for libSupport  *- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the entry point to initialize the options registered on the
// command line for libSupport, this is internal to libSupport.
//
//===----------------------------------------------------------------------===//

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LLVM_SUPPORT_DEBUGOPTIONS_H
#ವ್ಯಾಖ್ಯಾನಿಸು LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
ಶೂನ್ಯ initDebugCounterOptions();
ಶೂನ್ಯ init_ನಕ್ಷೆ_Writer_Options();
ಶೂನ್ಯ init_ಸಂಕೇತಗಳು_Options();
ಶೂನ್ಯ initStatisticOptions();
ಶೂನ್ಯ initTimerOptions();
ಶೂನ್ಯ initWithColorOptions();
ಶೂನ್ಯ initDebugOptions();
ಶೂನ್ಯ init_Random_ಬೀಜ_Options();

} // namespace llvm

#ಆದರೆ_ಅಂತ್ಯ // LLVM_SUPPORT_DEBUGOPTIONS_H
