//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/PGOOptions.h"

using நாமாகாஶ llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       தருக்க DebugInfoForProfiling, தருக்க PseudoProbeForProfiling,
                       தருக்க AtomicCounterUpdate)
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
  ப்ரதிஜ்ஞா(இது->CSAction == NoCSAction ||
         (இது->Action != IRInstr && இது->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  ப்ரதிஜ்ஞா(இது->CSAction != CSIRInstr || !இது->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  ப்ரதிஜ்ஞா(இது->CSAction != CSIRUse || இது->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  ப்ரதிஜ்ஞா(இது->MemoryProfile.empty() || இது->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  ப்ரதிஜ்ஞா(இது->Action != NoAction || இது->CSAction != NoCSAction ||
         !இது->MemoryProfile.empty() || இது->DebugInfoForProfiling ||
         இது->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(மாறிலி PGOOptions &) = இயல்பு;

PGOOptions &PGOOptions::operator=(மாறிலி PGOOptions &O) = இயல்பு;

PGOOptions::~PGOOptions() = இயல்பு;
