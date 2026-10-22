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

#अन्तर्भाव "llvm/Support/ModRef.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"

using नामाकाश llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  विकल्प (MR) {
  पक्ष ModRefInfo::NoModRef:
    OS << "NoModRef";
    भङ्ग;
  पक्ष ModRefInfo::Ref:
    OS << "Ref";
    भङ्ग;
  पक्ष ModRefInfo::Mod:
    OS << "Mod";
    भङ्ग;
  पक्ष ModRefInfo::ModRef:
    OS << "ModRef";
    भङ्ग;
  }
  प्रत्यागम OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    विकल्प (Loc) {
    पक्ष IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      भङ्ग;
    पक्ष IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      भङ्ग;
    पक्ष IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      भङ्ग;
    पक्ष IRMemLocation::Other:
      OS << "Other: ";
      भङ्ग;
    पक्ष IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      भङ्ग;
    पक्ष IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      भङ्ग;
    }
    OS << ME.getModRef(Loc);
  });
  प्रत्यागम OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  जइ (capturesNothing(CC)) {
    OS << "none";
    प्रत्यागम OS;
  }

  ListSeparator LS;
  जइ (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  अण्णहा जइ (capturesAddress(CC))
    OS << LS << "address";
  जइ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  जइ (capturesFullProvenance(CC))
    OS << LS << "provenance";

  प्रत्यागम OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  जइ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  जइ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  प्रत्यागम OS;
}
