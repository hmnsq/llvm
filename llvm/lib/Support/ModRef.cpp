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

#అన్తర్భావ "llvm/Support/ModRef.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"

ఉపయోగించి నామాకాశ llvm;

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, ModRefInfo MR) {
  ఎంపిక (MR) {
  స్థితి ModRefInfo::NoModRef:
    OS << "NoModRef";
    ఆపు;
  స్థితి ModRefInfo::Ref:
    OS << "Ref";
    ఆపు;
  స్థితి ModRefInfo::Mod:
    OS << "Mod";
    ఆపు;
  స్థితి ModRefInfo::ModRef:
    OS << "ModRef";
    ఆపు;
  }
  తిరిగి OS;
}

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, జ్ఞాపకం_Effects ME) {
  interleaveComma(జ్ఞాపకం_Effects::locations(), OS, [&](IRMemLocation Loc) {
    ఎంపిక (Loc) {
    స్థితి IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      ఆపు;
    స్థితి IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      ఆపు;
    స్థితి IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      ఆపు;
    స్థితి IRMemLocation::Other:
      OS << "Other: ";
      ఆపు;
    స్థితి IRMemLocation::లక్ష్యం_Mem0:
      OS << "TargetMem0: ";
      ఆపు;
    స్థితి IRMemLocation::లక్ష్యం_Mem1:
      OS << "TargetMem1: ";
      ఆపు;
    }
    OS << ME.getModRef(Loc);
  });
  తిరిగి OS;
}

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, CaptureComponents CC) {
  ఒకవేళ (capturesNothing(CC)) {
    OS << "none";
    తిరిగి OS;
  }

  ListSeparator LS;
  ఒకవేళ (captures_చిరునామా_Is_Null_Only(CC))
    OS << LS << "address_is_null";
  లేకపోతే ఒకవేళ (captures_చిరునామా(CC))
    OS << LS << "address";
  ఒకవేళ (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  ఒకవేళ (captures_నిండిన_Provenance(CC))
    OS << LS << "provenance";

  తిరిగి OS;
}

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  ఒకవేళ (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  ఒకవేళ (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  తిరిగి OS;
}
