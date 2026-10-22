//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ARMWinEH.h"

નામાકાશ llvm {
નામાકાશ ARM {
નામાકાશ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(અચલ RuntimeFunction &RF,
                                                તાર્કિક Prologue) {
  uint8_t અંક_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  જો (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } નહીંતર {
    // If Ret != 0, we pop into Lr and return later
    જો (RF.Ret() != ReturnType::RT_કાઢો)
      GPRMask |= (LinkRegister << 14);
    નહીંતર જો (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  જો (RegistersVFP)
    VFPMask |= (((1 << ((અંક_Registers + 1) % 8)) - 1) << 8);
  નહીંતર
    GPRMask |= (((1 << (અંક_Registers + 1)) - 1) << 4);

  જો ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ઢગલો_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ઢગલો_Adjust() & 0x3));

  પાછા {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

