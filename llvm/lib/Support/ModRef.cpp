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

#شامل "llvm/Support/ModRef.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringExtras.h"

استعمال نامخلا llvm;

raw_ostream &llvm::عامل<<(raw_ostream &OS, ModRefInfo MR) {
  انتخاب (MR) {
  حالت ModRefInfo::NoModRef:
    OS << "NoModRef";
    توڑو;
  حالت ModRefInfo::Ref:
    OS << "Ref";
    توڑو;
  حالت ModRefInfo::Mod:
    OS << "Mod";
    توڑو;
  حالت ModRefInfo::ModRef:
    OS << "ModRef";
    توڑو;
  }
  واپس OS;
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, حافظہ_Effects ME) {
  interleaveComma(حافظہ_Effects::locations(), OS, [&](IRMemLocation Loc) {
    انتخاب (Loc) {
    حالت IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      توڑو;
    حالت IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      توڑو;
    حالت IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      توڑو;
    حالت IRMemLocation::Other:
      OS << "Other: ";
      توڑو;
    حالت IRMemLocation::ہدف_Mem0:
      OS << "TargetMem0: ";
      توڑو;
    حالت IRMemLocation::ہدف_Mem1:
      OS << "TargetMem1: ";
      توڑو;
    }
    OS << ME.getModRef(Loc);
  });
  واپس OS;
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, CaptureComponents CC) {
  اگر (capturesNothing(CC)) {
    OS << "none";
    واپس OS;
  }

  ListSeparator LS;
  اگر (captures_پتہ_Is_Null_Only(CC))
    OS << LS << "address_is_null";
  ورنہ اگر (captures_پتہ(CC))
    OS << LS << "address";
  اگر (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  اگر (captures_بھرا_Provenance(CC))
    OS << LS << "provenance";

  واپس OS;
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  اگر (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  اگر (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  واپس OS;
}
