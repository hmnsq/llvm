//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ARMWinEH.h"

నామాకాశ llvm {
నామాకాశ ARM {
నామాకాశ WinEH {
std::pair<uint16_t, uint32_t> Saved_రిజిస్టరు_ముసుగు(స్థిరరాశి RuntimeFunction &RF,
                                                తార్కిక Prologue) {
  uint8_t అంకె_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_రిజిస్టరు = RF.L();
  uint8_t Chained_చట్రం = RF.C();

  uint16_t GPRMask = (Chained_చట్రం << 11);
  uint32_t VFPMask = 0;

  ఒకవేళ (Prologue) {
    GPRMask |= (Link_రిజిస్టరు << 14);
  } లేకపోతే {
    // If Ret != 0, we pop into Lr and return later
    ఒకవేళ (RF.Ret() != ReturnType::RT_తీయి)
      GPRMask |= (Link_రిజిస్టరు << 14);
    లేకపోతే ఒకవేళ (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_రిజిస్టరు << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  ఒకవేళ (RegistersVFP)
    VFPMask |= (((1 << ((అంకె_Registers + 1) % 8)) - 1) << 8);
  లేకపోతే
    GPRMask |= (((1 << (అంకె_Registers + 1)) - 1) << 4);

  ఒకవేళ ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.దొంతర_Adjust() & 0x3) + 1)) - 1)
                << (~RF.దొంతర_Adjust() & 0x3));

  తిరిగి {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

