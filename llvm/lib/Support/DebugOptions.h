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

#اگر_غیر_تعریف_شدہ LLVM_SUPPORT_DEBUGOPTIONS_H
#تعریف_کرو LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
خالی init_Debug_شمار_Options();
خالی init_گراف_Writer_Options();
خالی init_اشارات_Options();
خالی initStatisticOptions();
خالی initTimerOptions();
خالی initWithColorOptions();
خالی initDebugOptions();
خالی init_Random_بیج_Options();

} // namespace llvm

#اگر_ختم // LLVM_SUPPORT_DEBUGOPTIONS_H
