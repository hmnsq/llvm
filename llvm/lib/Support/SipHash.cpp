//===--- SipHash.cpp - An ABI-stable string hash --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements an ABI-stable string hash based on SipHash, used to
//  compute ptrauth discriminators.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/SipHash.h"
#ಅನ್ತರ್ಭಾವ "siphash/SipHash.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Endian.h"
#ಅನ್ತರ್ಭಾವ <cstdint>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ support;

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "llvm-siphash"

ಶೂನ್ಯ llvm::getSipHash_2_4_64(ArrayRef<uint8_t> ಒಳ_2, ಸ್ಥಿರಾಂಕ uint8_t (&K)[16],
                             uint8_t (&ಹೊರಹೋಗು)[8]) {
  siphash<2, 4>(ಒಳ_2.ದತ್ತಾಂಶ(), ಒಳ_2.ಗಾತ್ರ_3(), K, ಹೊರಹೋಗು);
}

ಶೂನ್ಯ llvm::getSipHash_2_4_128(ArrayRef<uint8_t> ಒಳ_2, ಸ್ಥಿರಾಂಕ uint8_t (&K)[16],
                              uint8_t (&ಹೊರಹೋಗು)[16]) {
  siphash<2, 4>(ಒಳ_2.ದತ್ತಾಂಶ(), ಒಳ_2.ಗಾತ್ರ_3(), K, ಹೊರಹೋಗು);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_ಬೈಟುಗಳು[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_ಬೈಟುಗಳು);
  ಹಿಂತಿರುಗಿ endian::read64le(Raw_Hash_ಬೈಟುಗಳು);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::get_ಸೂಚಕ_Auth_Stable_Sip_Hash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/ಸುಳ್ಳು, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  ಹಿಂತಿರುಗಿ Discriminator;
}
