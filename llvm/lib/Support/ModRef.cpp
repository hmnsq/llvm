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
  case ModRefInfo::NoModRef:
    OS << "NoModRef";
    break;
  case ModRefInfo::Ref:
    OS << "Ref";
    break;
  case ModRefInfo::Mod:
    OS << "Mod";
    break;
  case ModRefInfo::ModRef:
    OS << "ModRef";
    break;
  }
  вернуть OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    switch (Loc) {
    case IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      break;
    case IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      break;
    case IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      break;
    case IRMemLocation::Other:
      OS << "Other: ";
      break;
    case IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      break;
    case IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      break;
    }
    OS << ME.getModRef(Loc);
  });
  вернуть OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  если (capturesNothing(CC)) {
    OS << "none";
    вернуть OS;
  }

  ListSeparator LS;
  если (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  иначе если (capturesAddress(CC))
    OS << LS << "address";
  если (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  если (capturesFullProvenance(CC))
    OS << LS << "provenance";

  вернуть OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  если (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  если (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  вернуть OS;
}
