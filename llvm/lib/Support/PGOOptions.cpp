//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/PGOOptions.h"

using નામાકાશ llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       તાર્કિક DebugInfoForProfiling, તાર્કિક PseudoProbeForProfiling,
                       તાર્કિક AtomicCounterUpdate)
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
  પ્રતિજ્ઞા(આ->CSAction == NoCSAction ||
         (આ->Action != IRInstr && આ->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  પ્રતિજ્ઞા(આ->CSAction != CSIRInstr || !આ->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  પ્રતિજ્ઞા(આ->CSAction != CSIRUse || આ->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  પ્રતિજ્ઞા(આ->MemoryProfile.empty() || આ->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  પ્રતિજ્ઞા(આ->Action != NoAction || આ->CSAction != NoCSAction ||
         !આ->MemoryProfile.empty() || આ->DebugInfoForProfiling ||
         આ->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(અચલ PGOOptions &) = મૂળભૂત;

PGOOptions &PGOOptions::operator=(અચલ PGOOptions &O) = મૂળભૂત;

PGOOptions::~PGOOptions() = મૂળભૂત;
