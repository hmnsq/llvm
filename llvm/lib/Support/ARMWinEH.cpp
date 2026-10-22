//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ARMWinEH.h"

നാമാകാശ llvm {
നാമാകാശ ARM {
നാമാകാശ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(സ്ഥിരം RuntimeFunction &RF,
                                                തർക്കം Prologue) {
  uint8_t അക്കം_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  എങ്കിൽ (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } അല്ലെങ്കിൽ {
    // If Ret != 0, we pop into Lr and return later
    എങ്കിൽ (RF.Ret() != ReturnType::RT_എടുക്കുക)
      GPRMask |= (LinkRegister << 14);
    അല്ലെങ്കിൽ എങ്കിൽ (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  എങ്കിൽ (RegistersVFP)
    VFPMask |= (((1 << ((അക്കം_Registers + 1) % 8)) - 1) << 8);
  അല്ലെങ്കിൽ
    GPRMask |= (((1 << (അക്കം_Registers + 1)) - 1) << 4);

  എങ്കിൽ ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.അട്ടി_Adjust() & 0x3) + 1)) - 1)
                << (~RF.അട്ടി_Adjust() & 0x3));

  തിരികെ {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

