//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ARMWinEH.h"

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ ARM {
ਨਾਮਾਕਾਸ਼ WinEH {
std::pair<uint16_t, uint32_t> Saved_ਰਜਿਸਟਰ_ਨਕਾਬ(ਅਚਲ RuntimeFunction &RF,
                                                ਤਾਰਕਿਕ Prologue) {
  uint8_t ਅੰਕ_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_ਰਜਿਸਟਰ = RF.L();
  uint8_t Chained_ਚੌਖਟਾ = RF.C();

  uint16_t GPRMask = (Chained_ਚੌਖਟਾ << 11);
  uint32_t VFPMask = 0;

  ਜੇ (Prologue) {
    GPRMask |= (Link_ਰਜਿਸਟਰ << 14);
  } ਨਹੀਂਤਾਂ {
    // If Ret != 0, we pop into Lr and return later
    ਜੇ (RF.Ret() != ReturnType::RT_ਕੱਢੋ)
      GPRMask |= (Link_ਰਜਿਸਟਰ << 14);
    ਨਹੀਂਤਾਂ ਜੇ (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_ਰਜਿਸਟਰ << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  ਜੇ (RegistersVFP)
    VFPMask |= (((1 << ((ਅੰਕ_Registers + 1) % 8)) - 1) << 8);
  ਨਹੀਂਤਾਂ
    GPRMask |= (((1 << (ਅੰਕ_Registers + 1)) - 1) << 4);

  ਜੇ ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ਢੇਰ_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ਢੇਰ_Adjust() & 0x3));

  ਵਾਪਸ {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

