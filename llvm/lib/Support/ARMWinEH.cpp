//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ARMWinEH.h"

நாமாகாஶ llvm {
நாமாகாஶ ARM {
நாமாகாஶ WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(மாறிலி RuntimeFunction &RF,
                                                தருக்க Prologue) {
  uint8_t இலக்கம்_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  எனில் (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } இல்லையெனில் {
    // If Ret != 0, we pop into Lr and return later
    எனில் (RF.Ret() != ReturnType::RT_எடு)
      GPRMask |= (LinkRegister << 14);
    இல்லையெனில் எனில் (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  எனில் (RegistersVFP)
    VFPMask |= (((1 << ((இலக்கம்_Registers + 1) % 8)) - 1) << 8);
  இல்லையெனில்
    GPRMask |= (((1 << (இலக்கம்_Registers + 1)) - 1) << 4);

  எனில் ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.அடுக்கு_Adjust() & 0x3) + 1)) - 1)
                << (~RF.அடுக்கு_Adjust() & 0x3));

  திரும்பு {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

