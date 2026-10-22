//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/PGOOptions.h"

using namespace llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       bool DebugInfoForProfiling, bool PseudoProbeForProfiling,
                       bool Atomic_गणक_Update)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), MemoryProfile(MemoryProfile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      Atomic_गणक_Update(Atomic_गणक_Update) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  निश्चित(यह->CSAction == NoCSAction ||
         (यह->Action != IRInstr && यह->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  निश्चित(यह->CSAction != CSIRInstr || !यह->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  निश्चित(यह->CSAction != CSIRUse || यह->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  निश्चित(यह->MemoryProfile.empty() || यह->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  निश्चित(यह->Action != NoAction || यह->CSAction != NoCSAction ||
         !यह->MemoryProfile.empty() || यह->DebugInfoForProfiling ||
         यह->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(स्थिर PGOOptions &) = शेष;

PGOOptions &PGOOptions::चालक=(स्थिर PGOOptions &O) = शेष;

PGOOptions::~PGOOptions() = शेष;
