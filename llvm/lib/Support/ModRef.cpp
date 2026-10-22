//===--- ModRef.cpp - Memory effect modeling --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements ModRef and MemoryEffects misc functions.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ModRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, ModRefInfo MR) {
  ਚੋਣ (MR) {
  ਹਾਲਤ ModRefInfo::NoModRef:
    OS << "NoModRef";
    ਰੋਕੋ;
  ਹਾਲਤ ModRefInfo::Ref:
    OS << "Ref";
    ਰੋਕੋ;
  ਹਾਲਤ ModRefInfo::Mod:
    OS << "Mod";
    ਰੋਕੋ;
  ਹਾਲਤ ModRefInfo::ModRef:
    OS << "ModRef";
    ਰੋਕੋ;
  }
  ਵਾਪਸ OS;
}

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, ਸਮ੍ਰਿਤੀ_Effects ME) {
  interleaveComma(ਸਮ੍ਰਿਤੀ_Effects::locations(), OS, [&](IRMemLocation Loc) {
    ਚੋਣ (Loc) {
    ਹਾਲਤ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ਰੋਕੋ;
    ਹਾਲਤ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ਰੋਕੋ;
    ਹਾਲਤ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ਰੋਕੋ;
    ਹਾਲਤ IRMemLocation::Other:
      OS << "Other: ";
      ਰੋਕੋ;
    ਹਾਲਤ IRMemLocation::ਨਿਸ਼ਾਨਾ_Mem0:
      OS << "TargetMem0: ";
      ਰੋਕੋ;
    ਹਾਲਤ IRMemLocation::ਨਿਸ਼ਾਨਾ_Mem1:
      OS << "TargetMem1: ";
      ਰੋਕੋ;
    }
    OS << ME.getModRef(Loc);
  });
  ਵਾਪਸ OS;
}

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, CaptureComponents CC) {
  ਜੇ (capturesNothing(CC)) {
    OS << "none";
    ਵਾਪਸ OS;
  }

  ListSeparator LS;
  ਜੇ (captures_ਪਤਾ_Is_Null_Only(CC))
    OS << LS << "address_is_null";
  ਨਹੀਂਤਾਂ ਜੇ (captures_ਪਤਾ(CC))
    OS << LS << "address";
  ਜੇ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ਜੇ (captures_ਭਰਿਆ_Provenance(CC))
    OS << LS << "provenance";

  ਵਾਪਸ OS;
}

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ਜੇ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ਜੇ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ਵਾਪਸ OS;
}
