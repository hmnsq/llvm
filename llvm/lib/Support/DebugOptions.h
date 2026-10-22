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

#अगर_अघोषित LLVM_SUPPORT_DEBUGOPTIONS_H
#घोषणा LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
व्योम init_Debug_गणक_Options();
व्योम initGraphWriterOptions();
व्योम initSignalsOptions();
व्योम initStatisticOptions();
व्योम initTimerOptions();
व्योम initWithColorOptions();
व्योम initDebugOptions();
व्योम initRandomSeedOptions();

} // namespace llvm

#पूर्ण_अगर // LLVM_SUPPORT_DEBUGOPTIONS_H
