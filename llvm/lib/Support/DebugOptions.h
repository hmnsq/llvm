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

#ifndef LLVM_SUPPORT_DEBUGOPTIONS_H
#define LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
શૂન્ય initDebugCounterOptions();
શૂન્ય initGraphWriterOptions();
શૂન્ય initSignalsOptions();
શૂન્ય initStatisticOptions();
શૂન્ય initTimerOptions();
શૂન્ય initWithColorOptions();
શૂન્ય initDebugOptions();
શૂન્ય initRandomSeedOptions();

} // namespace llvm

#endif // LLVM_SUPPORT_DEBUGOPTIONS_H
