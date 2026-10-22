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

#include "llvm/Support/ModRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"

using namespace llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  switch (MR) {
  pak_sa ModRefInfo::NoModRef:
    OS << "NoModRef";
    bha_nga;
  pak_sa ModRefInfo::Ref:
    OS << "Ref";
    bha_nga;
  pak_sa ModRefInfo::Mod:
    OS << "Mod";
    bha_nga;
  pak_sa ModRefInfo::ModRef:
    OS << "ModRef";
    bha_nga;
  }
  ᬯᬮᬶ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    switch (Loc) {
    pak_sa IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      bha_nga;
    pak_sa IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      bha_nga;
    pak_sa IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      bha_nga;
    pak_sa IRMemLocation::Other:
      OS << "Other: ";
      bha_nga;
    pak_sa IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      bha_nga;
    pak_sa IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      bha_nga;
    }
    OS << ME.getModRef(Loc);
  });
  ᬯᬮᬶ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  ᬬᬾᬦ᭄ (capturesNothing(CC)) {
    OS << "none";
    ᬯᬮᬶ OS;
  }

  ListSeparator LS;
  ᬬᬾᬦ᭄ (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  ᬮᬶᬬ ᬬᬾᬦ᭄ (capturesAddress(CC))
    OS << LS << "address";
  ᬬᬾᬦ᭄ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ᬬᬾᬦ᭄ (capturesFullProvenance(CC))
    OS << LS << "provenance";

  ᬯᬮᬶ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ᬬᬾᬦ᭄ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ᬬᬾᬦ᭄ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ᬯᬮᬶ OS;
}
