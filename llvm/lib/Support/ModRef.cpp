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

using فضاینام llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  انتخاب (MR) {
  حالت ModRefInfo::NoModRef:
    OS << "NoModRef";
    بشکن;
  حالت ModRefInfo::Ref:
    OS << "Ref";
    بشکن;
  حالت ModRefInfo::Mod:
    OS << "Mod";
    بشکن;
  حالت ModRefInfo::ModRef:
    OS << "ModRef";
    بشکن;
  }
  بازگشت OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    انتخاب (Loc) {
    حالت IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      بشکن;
    حالت IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      بشکن;
    حالت IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      بشکن;
    حالت IRMemLocation::Other:
      OS << "Other: ";
      بشکن;
    حالت IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      بشکن;
    حالت IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      بشکن;
    }
    OS << ME.getModRef(Loc);
  });
  بازگشت OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  اگر (capturesNothing(CC)) {
    OS << "none";
    بازگشت OS;
  }

  ListSeparator LS;
  اگر (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  وگرنه اگر (capturesAddress(CC))
    OS << LS << "address";
  اگر (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  اگر (capturesFullProvenance(CC))
    OS << LS << "provenance";

  بازگشت OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  اگر (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  اگر (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  بازگشت OS;
}
