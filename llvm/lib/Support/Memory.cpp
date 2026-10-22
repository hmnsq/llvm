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

#అన్తర్భావ "llvm/Support/Memory.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"

#నిర్వచితం_కాకపోతే NDEBUG
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అయితే_ముగింపు // ifndef NDEBUG

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/Memory.inc"
#అయితే_ముగింపు
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "Windows/Memory.inc"
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే NDEBUG

నామాకాశ llvm {
నామాకాశ sys {

raw_ostream &క్రియాకారకం<<(raw_ostream &OS, స్థిరరాశి జ్ఞాపకం_2::ProtectionFlags &PF) {
  ప్రతిజ్ఞా((PF & ~(జ్ఞాపకం_2::MF_READ | జ్ఞాపకం_2::MF_WRITE | జ్ఞాపకం_2::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  తిరిగి OS << (PF & జ్ఞాపకం_2::MF_READ ? 'R' : '-')
            << (PF & జ్ఞాపకం_2::MF_WRITE ? 'W' : '-')
            << (PF & జ్ఞాపకం_2::MF_EXEC ? 'X' : '-');
}

raw_ostream &క్రియాకారకం<<(raw_ostream &OS, స్థిరరాశి జ్ఞాపకం_ఖండం &MB) {
  తిరిగి OS << "[ " << MB.పునాది_2() << " .. "
            << (శూన్య *)((అక్షరం *)MB.పునాది_2() + MB.allocated_కొలత()) << " ] ("
            << MB.allocated_కొలత() << " bytes)";
}

} // end namespace sys
} // end namespace llvm

#అయితే_ముగింపు // ifndef NDEBUG
