//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/ARMWinEH.h"

নামাকাশ llvm {
নামাকাশ ARM {
নামাকাশ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(ধ্রুৱ RuntimeFunction &RF,
                                                তার্কিক Prologue) {
  uint8_t অঙ্ক_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  যদি (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } অন্যথা {
    // If Ret != 0, we pop into Lr and return later
    যদি (RF.Ret() != ReturnType::RT_বেৰ_কৰো)
      GPRMask |= (LinkRegister << 14);
    অন্যথা যদি (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  যদি (RegistersVFP)
    VFPMask |= (((1 << ((অঙ্ক_Registers + 1) % 8)) - 1) << 8);
  অন্যথা
    GPRMask |= (((1 << (অঙ্ক_Registers + 1)) - 1) << 4);

  যদি ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.স্তূপ_Adjust() & 0x3) + 1)) - 1)
                << (~RF.স্তূপ_Adjust() & 0x3));

  ঘূৰাই {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

