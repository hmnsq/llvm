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
std::pair<uint16_t, uint32_t> Saved_ರಿಜಿಸ್ಟರ್_ಮುಖವಾಡ(ಸ್ಥಿರಾಂಕ RuntimeFunction &RF,
                                                ತಾರ್ಕಿಕ Prologue) {
  uint8_t ಅಂಕೆ_Registers = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t Link_ರಿಜಿಸ್ಟರ್ = RF.L();
  uint8_t Chained_ಚೌಕಟ್ಟು = RF.C();

  uint16_t GPRMask = (Chained_ಚೌಕಟ್ಟು << 11);
  uint32_t VFPMask = 0;

  ಆದರೆ (Prologue) {
    GPRMask |= (Link_ರಿಜಿಸ್ಟರ್ << 14);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // If Ret != 0, we pop into Lr and return later
    ಆದರೆ (RF.Ret() != ReturnType::RT_ತೆಗೆ)
      GPRMask |= (Link_ರಿಜಿಸ್ಟರ್ << 14);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (Link_ರಿಜಿಸ್ಟರ್ << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  ಆದರೆ (RegistersVFP)
    VFPMask |= (((1 << ((ಅಂಕೆ_Registers + 1) % 8)) - 1) << 8);
  ಇಲ್ಲದಿದ್ದರೆ
    GPRMask |= (((1 << (ಅಂಕೆ_Registers + 1)) - 1) << 4);

  ಆದರೆ ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.ರಾಶಿ_Adjust() & 0x3) + 1)) - 1)
                << (~RF.ರಾಶಿ_Adjust() & 0x3));

  ಹಿಂತಿರುಗಿ {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace llvm

