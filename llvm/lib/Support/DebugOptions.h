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

#که_نه_تعریف_شوی LLVM_SUPPORT_DEBUGOPTIONS_H
#تعریف_کړه LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
فراغ initDebugCounterOptions();
فراغ init_ګراف_Writer_Options();
فراغ init_سیګنالونه_Options();
فراغ initStatisticOptions();
فراغ initTimerOptions();
فراغ initWithColorOptions();
فراغ initDebugOptions();
فراغ init_Random_تخم_Options();

} // namespace llvm

#د_که_پای // LLVM_SUPPORT_DEBUGOPTIONS_H
