//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ARMWinEH.h"

نطاق llvm {
نطاق ARM {
نطاق WinEH {
std::pair<uint16_t, uint32_t> Saved_ثبتوونکی_ماسک(ثابت RuntimeFunction &RF,
                                                منطقي Prologue) {
  uint8_t عدد_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_ثبتوونکی = RF.L();
  uint8_t Chained_چوکاټ = RF.C();

  uint16_t GPRMask = (Chained_چوکاټ << 11);
  uint32_t VFPMask = 0;

  إذا (Prologue) {
    GPRMask |= (Link_ثبتوونکی << 14);
  } وإلا {
    // If Ret != 0, we pop into Lr and return later
    إذا (RF.Ret() != ReturnType::RT_راوباسه)
      GPRMask |= (Link_ثبتوونکی << 14);
    وإلا إذا (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_ثبتوونکی << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  إذا (RegistersVFP)
    VFPMask |= (((1 << ((عدد_Registers + 1) % 8)) - 1) << 8);
  وإلا
    GPRMask |= (((1 << (عدد_Registers + 1)) - 1) << 4);

  إذا ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ډېری_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ډېری_Adjust() & 0x3));

  أرجع {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

