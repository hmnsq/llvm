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
  返回 OS;
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
  返回 OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  如果 (capturesNothing(CC)) {
    OS << "none";
    返回 OS;
  }

  ListSeparator LS;
  如果 (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  否则 如果 (capturesAddress(CC))
    OS << LS << "address";
  如果 (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  如果 (capturesFullProvenance(CC))
    OS << LS << "provenance";

  返回 OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  如果 (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  如果 (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  返回 OS;
}
