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

#اگر_تعریف_نشده LLVM_SUPPORT_DEBUGOPTIONS_H
#تعریف_کن LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
فراغ initDebugCounterOptions();
فراغ init_گراف_Writer_Options();
فراغ init_سیگنالها_Options();
فراغ initStatisticOptions();
فراغ initTimerOptions();
فراغ initWithColorOptions();
فراغ initDebugOptions();
فراغ init_Random_دانه_Options();

} // namespace llvm

#پایان_اگر // LLVM_SUPPORT_DEBUGOPTIONS_H
