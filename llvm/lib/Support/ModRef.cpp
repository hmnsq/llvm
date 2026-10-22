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

raw_ostream &llvm::चालक<<(raw_ostream &OS, ModRefInfo MR) {
  निवड (MR) {
  स्थिती ModRefInfo::NoModRef:
    OS << "NoModRef";
    थांबा;
  स्थिती ModRefInfo::Ref:
    OS << "Ref";
    थांबा;
  स्थिती ModRefInfo::Mod:
    OS << "Mod";
    थांबा;
  स्थिती ModRefInfo::ModRef:
    OS << "ModRef";
    थांबा;
  }
  परत OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    निवड (Loc) {
    स्थिती IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      थांबा;
    स्थिती IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      थांबा;
    स्थिती IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      थांबा;
    स्थिती IRMemLocation::Other:
      OS << "Other: ";
      थांबा;
    स्थिती IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      थांबा;
    स्थिती IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      थांबा;
    }
    OS << ME.getModRef(Loc);
  });
  परत OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, CaptureComponents CC) {
  जर (capturesNothing(CC)) {
    OS << "none";
    परत OS;
  }

  ListSeparator LS;
  जर (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  अन्यथा जर (capturesAddress(CC))
    OS << LS << "address";
  जर (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  जर (capturesFullProvenance(CC))
    OS << LS << "provenance";

  परत OS;
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  जर (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  जर (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  परत OS;
}
