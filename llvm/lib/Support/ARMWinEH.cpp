//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ARMWinEH.h"

نامخلا llvm {
نامخلا ARM {
نامخلا WinEH {
std::pair<uint16_t, uint32_t> Saved_رجسٹر_نقاب(ثابت RuntimeFunction &RF,
                                                منطقی Prologue) {
  uint8_t عدد_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_رجسٹر = RF.L();
  uint8_t Chained_چوکھٹا = RF.C();

  uint16_t GPRMask = (Chained_چوکھٹا << 11);
  uint32_t VFPMask = 0;

  اگر (Prologue) {
    GPRMask |= (Link_رجسٹر << 14);
  } ورنہ {
    // If Ret != 0, we pop into Lr and return later
    اگر (RF.Ret() != ReturnType::RT_نکالو)
      GPRMask |= (Link_رجسٹر << 14);
    ورنہ اگر (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_رجسٹر << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  اگر (RegistersVFP)
    VFPMask |= (((1 << ((عدد_Registers + 1) % 8)) - 1) << 8);
  ورنہ
    GPRMask |= (((1 << (عدد_Registers + 1)) - 1) << 4);

  اگر ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ڈھیر_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ڈھیر_Adjust() & 0x3));

  واپس {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

