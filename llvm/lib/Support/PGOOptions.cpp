//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/PGOOptions.h"

using नामाकाश llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       तार्किक DebugInfoForProfiling, तार्किक PseudoProbeForProfiling,
                       तार्किक AtomicCounterUpdate)
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
  प्रतिज्ञा(एतत्->CSAction == NoCSAction ||
         (एतत्->Action != IRInstr && एतत्->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  प्रतिज्ञा(एतत्->CSAction != CSIRInstr || !एतत्->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  प्रतिज्ञा(एतत्->CSAction != CSIRUse || एतत्->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  प्रतिज्ञा(एतत्->MemoryProfile.empty() || एतत्->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  प्रतिज्ञा(एतत्->Action != NoAction || एतत्->CSAction != NoCSAction ||
         !एतत्->MemoryProfile.empty() || एतत्->DebugInfoForProfiling ||
         एतत्->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(अचर PGOOptions &) = स्वभाव;

PGOOptions &PGOOptions::चालक=(अचर PGOOptions &O) = स्वभाव;

PGOOptions::~PGOOptions() = स्वभाव;
