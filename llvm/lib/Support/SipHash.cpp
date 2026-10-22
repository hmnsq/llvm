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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SipHash.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "siphash/SipHash.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ support;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "llvm-siphash"

ਖਾਲੀ llvm::getSipHash_2_4_64(ArrayRef<uint8_t> ਵਿੱਚ_3, ਅਚਲ uint8_t (&K)[16],
                             uint8_t (&ਬਾਹਰਲਾ)[8]) {
  siphash<2, 4>(ਵਿੱਚ_3.ਅੰਕੜੇ(), ਵਿੱਚ_3.ਮਾਪ_2(), K, ਬਾਹਰਲਾ);
}

ਖਾਲੀ llvm::getSipHash_2_4_128(ArrayRef<uint8_t> ਵਿੱਚ_3, ਅਚਲ uint8_t (&K)[16],
                              uint8_t (&ਬਾਹਰਲਾ)[16]) {
  siphash<2, 4>(ਵਿੱਚ_3.ਅੰਕੜੇ(), ਵਿੱਚ_3.ਮਾਪ_2(), K, ਬਾਹਰਲਾ);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  ਸਥਿਰ ਅਚਲ uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_ਬਾਈਟਾਂ[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_ਬਾਈਟਾਂ);
  ਵਾਪਸ endian::read64le(Raw_Hash_ਬਾਈਟਾਂ);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::get_ਸੂਚਕ_Auth_Stable_Sip_Hash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/ਝੂਠ, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  ਵਾਪਸ Discriminator;
}
