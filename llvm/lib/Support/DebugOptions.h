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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LLVM_SUPPORT_DEBUGOPTIONS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
ਖਾਲੀ initDebugCounterOptions();
ਖਾਲੀ init_ਗ੍ਰਾਫ_Writer_Options();
ਖਾਲੀ init_ਸੰਕੇਤ_Options();
ਖਾਲੀ initStatisticOptions();
ਖਾਲੀ initTimerOptions();
ਖਾਲੀ initWithColorOptions();
ਖਾਲੀ initDebugOptions();
ਖਾਲੀ init_Random_ਬੀਜ_Options();

} // namespace llvm

#ਜੇ_ਅੰਤ // LLVM_SUPPORT_DEBUGOPTIONS_H
