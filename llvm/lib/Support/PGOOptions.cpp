//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/PGOOptions.h"

using فضاینام llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       منطقی DebugInfoForProfiling, منطقی PseudoProbeForProfiling,
                       منطقی AtomicCounterUpdate)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), MemoryProfile(MemoryProfile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      AtomicCounterUpdate(AtomicCounterUpdate) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  أكد(این->CSAction == NoCSAction ||
         (این->Action != IRInstr && این->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  أكد(این->CSAction != CSIRInstr || !این->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  أكد(این->CSAction != CSIRUse || این->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  أكد(این->MemoryProfile.empty() || این->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  أكد(این->Action != NoAction || این->CSAction != NoCSAction ||
         !این->MemoryProfile.empty() || این->DebugInfoForProfiling ||
         این->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(ثابت PGOOptions &) = پیشفرض;

PGOOptions &PGOOptions::operator=(ثابت PGOOptions &O) = پیشفرض;

PGOOptions::~PGOOptions() = پیشفرض;
