//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/PGOOptions.h"

استعمال نامخلا llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string حافظہ_Profile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       منطقی DebugInfoForProfiling, منطقی PseudoProbeForProfiling,
                       منطقی Atomic_شمار_Update)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), حافظہ_Profile(حافظہ_Profile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      Atomic_شمار_Update(Atomic_شمار_Update) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  أكد(یہ->CSAction == NoCSAction ||
         (یہ->Action != IRInstr && یہ->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  أكد(یہ->CSAction != CSIRInstr || !یہ->CSProfileGenFile.خالی_خانے());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  أكد(یہ->CSAction != CSIRUse || یہ->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  أكد(یہ->حافظہ_Profile.خالی_خانے() || یہ->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  أكد(یہ->Action != NoAction || یہ->CSAction != NoCSAction ||
         !یہ->حافظہ_Profile.خالی_خانے() || یہ->DebugInfoForProfiling ||
         یہ->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(ثابت PGOOptions &) = طےشدہ;

PGOOptions &PGOOptions::عامل=(ثابت PGOOptions &O) = طےشدہ;

PGOOptions::~PGOOptions() = طےشدہ;
