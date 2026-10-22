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
std::pair<uint16_t, uint32_t> Saved_पञ्जिकाकोशः_आवरणम्(ध्रुव RuntimeFunction &RF,
                                                तार्किक Prologue) {
  uint8_t अङ्कः_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_पञ्जिकाकोशः = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  यदि (Prologue) {
    GPRMask |= (Link_पञ्जिकाकोशः << 14);
  } अन्यथा {
    // If Ret != 0, we pop into Lr and return later
    यदि (RF.Ret() != ReturnType::RT_निष्कासय)
      GPRMask |= (Link_पञ्जिकाकोशः << 14);
    अन्यथा यदि (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_पञ्जिकाकोशः << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  यदि (RegistersVFP)
    VFPMask |= (((1 << ((अङ्कः_Registers + 1) % 8)) - 1) << 8);
  अन्यथा
    GPRMask |= (((1 << (अङ्कः_Registers + 1)) - 1) << 4);

  यदि ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.राशिः_Adjust() & 0x3) + 1)) - 1)
                << (~RF.राशिः_Adjust() & 0x3));

  प्रत्यागम {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

