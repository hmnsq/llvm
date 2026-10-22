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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ModRefInfo MR) {
  ಆಯ್ಕೆ (MR) {
  ಸ್ಥಿತಿ ModRefInfo::NoModRef:
    OS << "NoModRef";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ModRefInfo::Ref:
    OS << "Ref";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ModRefInfo::Mod:
    OS << "Mod";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ModRefInfo::ModRef:
    OS << "ModRef";
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ OS;
}

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಮರಣೆ_Effects ME) {
  interleaveComma(ಸ್ಮರಣೆ_Effects::locations(), OS, [&](IRMemLocation Loc) {
    ಆಯ್ಕೆ (Loc) {
    ಸ್ಥಿತಿ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IRMemLocation::Other:
      OS << "Other: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IRMemLocation::ಗುರಿ_Mem0:
      OS << "TargetMem0: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IRMemLocation::ಗುರಿ_Mem1:
      OS << "TargetMem1: ";
      ನಿಲ್ಲಿಸು;
    }
    OS << ME.getModRef(Loc);
  });
  ಹಿಂತಿರುಗಿ OS;
}

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, CaptureComponents CC) {
  ಆದರೆ (capturesNothing(CC)) {
    OS << "none";
    ಹಿಂತಿರುಗಿ OS;
  }

  ListSeparator LS;
  ಆದರೆ (captures_ವಿಳಾಸ_Is_Null_Only(CC))
    OS << LS << "address_is_null";
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (captures_ವಿಳಾಸ(CC))
    OS << LS << "address";
  ಆದರೆ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ಆದರೆ (captures_ತುಂಬಿದ_Provenance(CC))
    OS << LS << "provenance";

  ಹಿಂತಿರುಗಿ OS;
}

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ಆದರೆ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ಆದರೆ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ಹಿಂತಿರುಗಿ OS;
}
