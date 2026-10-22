//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ARMWinEH.h"

नामाकाश llvm {
नामाकाश ARM {
नामाकाश WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(स्थिरांक RuntimeFunction &RF,
                                                तार्किक Prologue) {
  uint8_t अंक_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  जर (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } अन्यथा {
    // If Ret != 0, we pop into Lr and return later
    जर (RF.Ret() != ReturnType::RT_काढा)
      GPRMask |= (LinkRegister << 14);
    अन्यथा जर (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  जर (RegistersVFP)
    VFPMask |= (((1 << ((अंक_Registers + 1) % 8)) - 1) << 8);
  अन्यथा
    GPRMask |= (((1 << (अंक_Registers + 1)) - 1) << 4);

  जर ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.चळत_Adjust() & 0x3) + 1)) - 1)
                << (~RF.चळत_Adjust() & 0x3));

  परत {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

