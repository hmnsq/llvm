//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/PGOOptions.h"

استفاده نطاق llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string حافظه_Profile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       منطقي DebugInfoForProfiling, منطقي PseudoProbeForProfiling,
                       منطقي AtomicCounterUpdate)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), حافظه_Profile(حافظه_Profile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      AtomicCounterUpdate(AtomicCounterUpdate) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  أكد(هذا->CSAction == NoCSAction ||
         (هذا->Action != IRInstr && هذا->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  أكد(هذا->CSAction != CSIRInstr || !هذا->CSProfileGenFile.خالی());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  أكد(هذا->CSAction != CSIRUse || هذا->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  أكد(هذا->حافظه_Profile.خالی() || هذا->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  أكد(هذا->Action != NoAction || هذا->CSAction != NoCSAction ||
         !هذا->حافظه_Profile.خالی() || هذا->DebugInfoForProfiling ||
         هذا->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(ثابت PGOOptions &) = افتراضي;

PGOOptions &PGOOptions::عملگر=(ثابت PGOOptions &O) = افتراضي;

PGOOptions::~PGOOptions() = افتراضي;
