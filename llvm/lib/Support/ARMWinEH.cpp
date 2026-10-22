//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ARMWinEH.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ ARM {
ನಾಮಾಕಾಶ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(ಧ್ರುವ RuntimeFunction &RF,
                                                ತಾರ್ಕಿಕ Prologue) {
  uint8_t ಅಂಕೆ_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  ಯದಿ (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } ಅನ್ಯಥಾ {
    // If Ret != 0, we pop into Lr and return later
    ಯದಿ (RF.Ret() != ReturnType::RT_ತೆಗೆ)
      GPRMask |= (LinkRegister << 14);
    ಅನ್ಯಥಾ ಯದಿ (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  ಯದಿ (RegistersVFP)
    VFPMask |= (((1 << ((ಅಂಕೆ_Registers + 1) % 8)) - 1) << 8);
  ಅನ್ಯಥಾ
    GPRMask |= (((1 << (ಅಂಕೆ_Registers + 1)) - 1) << 4);

  ಯದಿ ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ರಾಶಿ_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ರಾಶಿ_Adjust() & 0x3));

  ಪ್ರತ್ಯಾಗಮ {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

