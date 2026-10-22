//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/PGOOptions.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string ਸਮ੍ਰਿਤੀ_Profile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       ਤਾਰਕਿਕ DebugInfoForProfiling, ਤਾਰਕਿਕ PseudoProbeForProfiling,
                       ਤਾਰਕਿਕ AtomicCounterUpdate)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), ਸਮ੍ਰਿਤੀ_Profile(ਸਮ੍ਰਿਤੀ_Profile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      AtomicCounterUpdate(AtomicCounterUpdate) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->CSAction == NoCSAction ||
         (ਇਹ->Action != IRInstr && ਇਹ->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->CSAction != CSIRInstr || !ਇਹ->CSProfileGenFile.ਖਾਲੀ_ਥਾਵਾਂ());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->CSAction != CSIRUse || ਇਹ->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->ਸਮ੍ਰਿਤੀ_Profile.ਖਾਲੀ_ਥਾਵਾਂ() || ਇਹ->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->Action != NoAction || ਇਹ->CSAction != NoCSAction ||
         !ਇਹ->ਸਮ੍ਰਿਤੀ_Profile.ਖਾਲੀ_ਥਾਵਾਂ() || ਇਹ->DebugInfoForProfiling ||
         ਇਹ->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(ਅਚਲ PGOOptions &) = ਮੂਲ;

PGOOptions &PGOOptions::ਸੰਚਾਲਕ=(ਅਚਲ PGOOptions &O) = ਮੂਲ;

PGOOptions::~PGOOptions() = ਮੂਲ;
