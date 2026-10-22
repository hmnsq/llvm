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

#અન્તર્ભાવ "llvm/Support/ModRef.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"

using નામાકાશ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ModRefInfo MR) {
  પસંદગી (MR) {
  સ્થિતિ ModRefInfo::NoModRef:
    OS << "NoModRef";
    તોડો;
  સ્થિતિ ModRefInfo::Ref:
    OS << "Ref";
    તોડો;
  સ્થિતિ ModRefInfo::Mod:
    OS << "Mod";
    તોડો;
  સ્થિતિ ModRefInfo::ModRef:
    OS << "ModRef";
    તોડો;
  }
  પાછા OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    પસંદગી (Loc) {
    સ્થિતિ IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      તોડો;
    સ્થિતિ IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      તોડો;
    સ્થિતિ IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      તોડો;
    સ્થિતિ IRMemLocation::Other:
      OS << "Other: ";
      તોડો;
    સ્થિતિ IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      તોડો;
    સ્થિતિ IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      તોડો;
    }
    OS << ME.getModRef(Loc);
  });
  પાછા OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureComponents CC) {
  જો (capturesNothing(CC)) {
    OS << "none";
    પાછા OS;
  }

  ListSeparator LS;
  જો (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  નહીંતર જો (capturesAddress(CC))
    OS << LS << "address";
  જો (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  જો (capturesFullProvenance(CC))
    OS << LS << "provenance";

  પાછા OS;
}

raw_ostream &llvm::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  જો (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  જો (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  પાછા OS;
}
