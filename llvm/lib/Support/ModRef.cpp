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

#समावेश "llvm/Support/ModRef.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringExtras.h"

using namespace llvm;

raw_ostream &llvm::चालक<<(raw_ostream &OS, ModRefInfo MR) {
  चयन (MR) {
  हाल ModRefInfo::NoModRef:
    OS << "NoModRef";
    अवरोध;
  हाल ModRefInfo::Ref:
    OS << "Ref";
    अवरोध;
  हाल ModRefInfo::Mod:
    OS << "Mod";
    अवरोध;
  हाल ModRefInfo::ModRef:
    OS << "ModRef";
    अवरोध;
  }
  वापस OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    चयन (Loc) {
    हाल IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      अवरोध;
    हाल IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      अवरोध;
    हाल IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      अवरोध;
    हाल IRMemLocation::Other:
      OS << "Other: ";
      अवरोध;
    हाल IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      अवरोध;
    हाल IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      अवरोध;
    }
    OS << ME.getModRef(Loc);
  });
  वापस OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, CaptureComponents CC) {
  अगर (capturesNothing(CC)) {
    OS << "none";
    वापस OS;
  }

  ListSeparator LS;
  अगर (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  अन्यथा अगर (capturesAddress(CC))
    OS << LS << "address";
  अगर (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  अगर (capturesFullProvenance(CC))
    OS << LS << "provenance";

  वापस OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  अगर (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  अगर (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  वापस OS;
}
