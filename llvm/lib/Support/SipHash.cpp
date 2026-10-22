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

#ضمن "llvm/Support/SipHash.h"
#ضمن "siphash/SipHash.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Endian.h"
#ضمن <cstdint>

کارول نطاق llvm;
کارول نطاق support;

#تعریف_کړه DEBUG_TYPE "llvm-siphash"

فراغ llvm::getSipHash_2_4_64(ArrayRef<uint8_t> دننه_2, ثابت uint8_t (&K)[16],
                             uint8_t (&بهر)[8]) {
  siphash<2, 4>(دننه_2.معلومات(), دننه_2.اندازه_2(), K, بهر);
}

فراغ llvm::getSipHash_2_4_128(ArrayRef<uint8_t> دننه_2, ثابت uint8_t (&K)[16],
                              uint8_t (&بهر)[16]) {
  siphash<2, 4>(دننه_2.معلومات(), دننه_2.اندازه_2(), K, بهر);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  ساكن ثابت uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_بایټونه[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_بایټونه);
  أرجع endian::read64le(Raw_Hash_بایټونه);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::get_اشاره_ګر_Auth_Stable_Sip_Hash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/خطأ, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  أرجع Discriminator;
}
