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

#ضمن "llvm/Support/ModRef.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringExtras.h"

using نطاق llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  بدل (MR) {
  حالة ModRefInfo::NoModRef:
    OS << "NoModRef";
    اكسر;
  حالة ModRefInfo::Ref:
    OS << "Ref";
    اكسر;
  حالة ModRefInfo::Mod:
    OS << "Mod";
    اكسر;
  حالة ModRefInfo::ModRef:
    OS << "ModRef";
    اكسر;
  }
  أرجع OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    بدل (Loc) {
    حالة IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      اكسر;
    حالة IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      اكسر;
    حالة IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      اكسر;
    حالة IRMemLocation::Other:
      OS << "Other: ";
      اكسر;
    حالة IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      اكسر;
    حالة IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      اكسر;
    }
    OS << ME.getModRef(Loc);
  });
  أرجع OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  إذا (capturesNothing(CC)) {
    OS << "none";
    أرجع OS;
  }

  ListSeparator LS;
  إذا (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  وإلا إذا (capturesAddress(CC))
    OS << LS << "address";
  إذا (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  إذا (capturesFullProvenance(CC))
    OS << LS << "provenance";

  أرجع OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  إذا (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  إذا (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  أرجع OS;
}
