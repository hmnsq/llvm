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
  বিকল্প (MR) {
  পক্ষ ModRefInfo::NoModRef:
    OS << "NoModRef";
    ভঙ্গ;
  পক্ষ ModRefInfo::Ref:
    OS << "Ref";
    ভঙ্গ;
  পক্ষ ModRefInfo::Mod:
    OS << "Mod";
    ভঙ্গ;
  পক্ষ ModRefInfo::ModRef:
    OS << "ModRef";
    ভঙ্গ;
  }
  প্রত্যাগম OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    বিকল্প (Loc) {
    পক্ষ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ভঙ্গ;
    পক্ষ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ভঙ্গ;
    পক্ষ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ভঙ্গ;
    পক্ষ IRMemLocation::Other:
      OS << "Other: ";
      ভঙ্গ;
    পক্ষ IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      ভঙ্গ;
    পক্ষ IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      ভঙ্গ;
    }
    OS << ME.getModRef(Loc);
  });
  প্রত্যাগম OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  যদি (capturesNothing(CC)) {
    OS << "none";
    প্রত্যাগম OS;
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

  প্রত্যাগম OS;
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
  প্রত্যাগম OS;
}
