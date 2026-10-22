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

#அந்தர்பாவ "llvm/Support/ModRef.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"

using நாமாகாஶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  தேர்வு (MR) {
  நிலை ModRefInfo::NoModRef:
    OS << "NoModRef";
    நிறுத்து;
  நிலை ModRefInfo::Ref:
    OS << "Ref";
    நிறுத்து;
  நிலை ModRefInfo::Mod:
    OS << "Mod";
    நிறுத்து;
  நிலை ModRefInfo::ModRef:
    OS << "ModRef";
    நிறுத்து;
  }
  திரும்பு OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    தேர்வு (Loc) {
    நிலை IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      நிறுத்து;
    நிலை IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      நிறுத்து;
    நிலை IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      நிறுத்து;
    நிலை IRMemLocation::Other:
      OS << "Other: ";
      நிறுத்து;
    நிலை IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      நிறுத்து;
    நிலை IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      நிறுத்து;
    }
    OS << ME.getModRef(Loc);
  });
  திரும்பு OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  எனில் (capturesNothing(CC)) {
    OS << "none";
    திரும்பு OS;
  }

  ListSeparator LS;
  எனில் (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  இல்லையெனில் எனில் (capturesAddress(CC))
    OS << LS << "address";
  எனில் (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  எனில் (capturesFullProvenance(CC))
    OS << LS << "provenance";

  திரும்பு OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  எனில் (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  எனில் (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  திரும்பு OS;
}
