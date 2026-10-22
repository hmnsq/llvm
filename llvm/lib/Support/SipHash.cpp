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

#شامل "llvm/Support/SipHash.h"
#شامل "siphash/SipHash.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Endian.h"
#شامل <cstdint>

استعمال نامخلا llvm;
استعمال نامخلا support;

#تعریف_کرو DEBUG_TYPE "llvm-siphash"

خالی llvm::getSipHash_2_4_64(ArrayRef<uint8_t> اندر_2, ثابت uint8_t (&K)[16],
                             uint8_t (&برآمد)[8]) {
  siphash<2, 4>(اندر_2.ڈیٹا(), اندر_2.سائز_2(), K, برآمد);
}

خالی llvm::getSipHash_2_4_128(ArrayRef<uint8_t> اندر_2, ثابت uint8_t (&K)[16],
                              uint8_t (&برآمد)[16]) {
  siphash<2, 4>(اندر_2.ڈیٹا(), اندر_2.سائز_2(), K, برآمد);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  ساکن ثابت uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_بائٹیں[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_بائٹیں);
  واپس endian::read64le(Raw_Hash_بائٹیں);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::get_اشارہ_گر_Auth_Stable_Sip_Hash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/جھوٹ, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  واپس Discriminator;
}
