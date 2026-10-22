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

#నిర్వచితం_కాకపోతే LLVM_SUPPORT_DEBUGOPTIONS_H
#నిర్వచించు LLVM_SUPPORT_DEBUGOPTIONS_H

namespace llvm {

// These are invoked internally before parsing command line options.
// This enables lazy-initialization of all the globals in libSupport, instead
// of eagerly loading everything on program startup.
శూన్య initDebugCounterOptions();
శూన్య init_లేఖాచిత్రం_Writer_Options();
శూన్య init_సంకేతాలు_Options();
శూన్య initStatisticOptions();
శూన్య initTimerOptions();
శూన్య initWithColorOptions();
శూన్య initDebugOptions();
శూన్య init_Random_విత్తనం_Options();

} // namespace llvm

#అయితే_ముగింపు // LLVM_SUPPORT_DEBUGOPTIONS_H
