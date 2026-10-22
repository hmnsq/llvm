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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ModRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"

using ନାମାକାଶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  ବାଛ (MR) {
  ସ୍ଥିତି ModRefInfo::NoModRef:
    OS << "NoModRef";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ModRefInfo::Ref:
    OS << "Ref";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ModRefInfo::Mod:
    OS << "Mod";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ModRefInfo::ModRef:
    OS << "ModRef";
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    ବାଛ (Loc) {
    ସ୍ଥିତି IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି IRMemLocation::Other:
      OS << "Other: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      ଭାଙ୍ଗ;
    }
    OS << ME.getModRef(Loc);
  });
  ଫେରନ୍ତୁ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  ଯଦି (capturesNothing(CC)) {
    OS << "none";
    ଫେରନ୍ତୁ OS;
  }

  ListSeparator LS;
  ଯଦି (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  ଅନ୍ୟଥା ଯଦି (capturesAddress(CC))
    OS << LS << "address";
  ଯଦି (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ଯଦି (capturesFullProvenance(CC))
    OS << LS << "provenance";

  ଫେରନ୍ତୁ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ଯଦି (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ଯଦି (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ଫେରନ୍ତୁ OS;
}
