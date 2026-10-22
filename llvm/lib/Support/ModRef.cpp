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

#അന്തര്ഭാവ "llvm/Support/ModRef.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"

using നാമാകാശ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  തിരഞ്ഞെടുപ്പ് (MR) {
  അവസ്ഥ ModRefInfo::NoModRef:
    OS << "NoModRef";
    നിർത്തുക;
  അവസ്ഥ ModRefInfo::Ref:
    OS << "Ref";
    നിർത്തുക;
  അവസ്ഥ ModRefInfo::Mod:
    OS << "Mod";
    നിർത്തുക;
  അവസ്ഥ ModRefInfo::ModRef:
    OS << "ModRef";
    നിർത്തുക;
  }
  തിരികെ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    തിരഞ്ഞെടുപ്പ് (Loc) {
    അവസ്ഥ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      നിർത്തുക;
    അവസ്ഥ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      നിർത്തുക;
    അവസ്ഥ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      നിർത്തുക;
    അവസ്ഥ IRMemLocation::Other:
      OS << "Other: ";
      നിർത്തുക;
    അവസ്ഥ IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      നിർത്തുക;
    അവസ്ഥ IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      നിർത്തുക;
    }
    OS << ME.getModRef(Loc);
  });
  തിരികെ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  എങ്കിൽ (capturesNothing(CC)) {
    OS << "none";
    തിരികെ OS;
  }

  ListSeparator LS;
  എങ്കിൽ (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  അല്ലെങ്കിൽ എങ്കിൽ (capturesAddress(CC))
    OS << LS << "address";
  എങ്കിൽ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  എങ്കിൽ (capturesFullProvenance(CC))
    OS << LS << "provenance";

  തിരികെ OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  എങ്കിൽ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  എങ്കിൽ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  തിരികെ OS;
}
