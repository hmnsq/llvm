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

#অন্তর্ভাব "llvm/Support/ModRef.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"

using নামাকাশ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  নির্বাচন (MR) {
  ক্ষেত্র ModRefInfo::NoModRef:
    OS << "NoModRef";
    বিরতি;
  ক্ষেত্র ModRefInfo::Ref:
    OS << "Ref";
    বিরতি;
  ক্ষেত্র ModRefInfo::Mod:
    OS << "Mod";
    বিরতি;
  ক্ষেত্র ModRefInfo::ModRef:
    OS << "ModRef";
    বিরতি;
  }
  ফেরত OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    নির্বাচন (Loc) {
    ক্ষেত্র IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      বিরতি;
    ক্ষেত্র IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      বিরতি;
    ক্ষেত্র IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      বিরতি;
    ক্ষেত্র IRMemLocation::Other:
      OS << "Other: ";
      বিরতি;
    ক্ষেত্র IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      বিরতি;
    ক্ষেত্র IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      বিরতি;
    }
    OS << ME.getModRef(Loc);
  });
  ফেরত OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  যদি (capturesNothing(CC)) {
    OS << "none";
    ফেরত OS;
  }

  ListSeparator LS;
  যদি (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  অন্যথা যদি (capturesAddress(CC))
    OS << LS << "address";
  যদি (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  যদি (capturesFullProvenance(CC))
    OS << LS << "provenance";

  ফেরত OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  যদি (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  যদি (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  ফেরত OS;
}
