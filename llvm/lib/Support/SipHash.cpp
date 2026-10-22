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

#అన్తర్భావ "llvm/Support/SipHash.h"
#అన్తర్భావ "siphash/SipHash.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Endian.h"
#అన్తర్భావ <cstdint>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ support;

#నిర్వచించు DEBUG_TYPE "llvm-siphash"

శూన్య llvm::getSipHash_2_4_64(ArrayRef<uint8_t> లోపల_2, స్థిరరాశి uint8_t (&K)[16],
                             uint8_t (&బహిర్గతం)[8]) {
  siphash<2, 4>(లోపల_2.దత్తాంశం(), లోపల_2.కొలత_2(), K, బహిర్గతం);
}

శూన్య llvm::getSipHash_2_4_128(ArrayRef<uint8_t> లోపల_2, స్థిరరాశి uint8_t (&K)[16],
                              uint8_t (&బహిర్గతం)[16]) {
  siphash<2, 4>(లోపల_2.దత్తాంశం(), లోపల_2.కొలత_2(), K, బహిర్గతం);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  స్థిర స్థిరరాశి uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_బైట్లు[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_బైట్లు);
  తిరిగి endian::read64le(Raw_Hash_బైట్లు);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::get_సూచిక_Auth_Stable_Sip_Hash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/అబద్ధం, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  తిరిగి Discriminator;
}
