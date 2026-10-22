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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ModRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"

using ನಾಮಾಕಾಶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  ವಿಕಲ್ಪ (MR) {
  ಪಕ್ಷ ModRefInfo::NoModRef:
    OS << "NoModRef";
    ಭಙ್ಗ;
  ಪಕ್ಷ ModRefInfo::Ref:
    OS << "Ref";
    ಭಙ್ಗ;
  ಪಕ್ಷ ModRefInfo::Mod:
    OS << "Mod";
    ಭಙ್ಗ;
  ಪಕ್ಷ ModRefInfo::ModRef:
    OS << "ModRef";
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    ವಿಕಲ್ಪ (Loc) {
    ಪಕ್ಷ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ IRMemLocation::Other:
      OS << "Other: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      ಭಙ್ಗ;
    }
    OS << ME.getModRef(Loc);
  });
  ಪ್ರತ್ಯಾಗಮ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  ಯದಿ (capturesNothing(CC)) {
    OS << "none";
    ಪ್ರತ್ಯಾಗಮ OS;
  }

  ListSeparator LS;
  ಯದಿ (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  ಅನ್ಯಥಾ ಯದಿ (capturesAddress(CC))
    OS << LS << "address";
  ಯದಿ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ಯದಿ (capturesFullProvenance(CC))
    OS << LS << "provenance";

  ಪ್ರತ್ಯಾಗಮ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ಯದಿ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ಯದಿ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ಪ್ರತ್ಯಾಗಮ OS;
}
