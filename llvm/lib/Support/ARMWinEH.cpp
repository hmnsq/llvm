//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ARMWinEH.h"

ନାମାକାଶ llvm {
ନାମାକାଶ ARM {
ନାମାକାଶ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(ଅଚଳ RuntimeFunction &RF,
                                                ତାର୍କିକ Prologue) {
  uint8_t ଅଙ୍କ_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  ଯଦି (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } ଅନ୍ୟଥା {
    // If Ret != 0, we pop into Lr and return later
    ଯଦି (RF.Ret() != ReturnType::RT_କାଢ଼)
      GPRMask |= (LinkRegister << 14);
    ଅନ୍ୟଥା ଯଦି (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  ଯଦି (RegistersVFP)
    VFPMask |= (((1 << ((ଅଙ୍କ_Registers + 1) % 8)) - 1) << 8);
  ଅନ୍ୟଥା
    GPRMask |= (((1 << (ଅଙ୍କ_Registers + 1)) - 1) << 4);

  ଯଦି ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ଗଦା_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ଗଦା_Adjust() & 0x3));

  ଫେରନ୍ତୁ {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

