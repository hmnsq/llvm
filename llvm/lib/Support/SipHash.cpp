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

#அந்தர்பாவ "llvm/Support/SipHash.h"
#அந்தர்பாவ "siphash/SipHash.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Endian.h"
#அந்தர்பாவ <cstdint>

using நாமாகாஶ llvm;
using நாமாகாஶ support;

#define DEBUG_TYPE "llvm-siphash"

வெறுமை llvm::getSipHash_2_4_64(ArrayRef<uint8_t> In, மாறிலி uint8_t (&K)[16],
                             uint8_t (&Out)[8]) {
  siphash<2, 4>(In.data(), In.size(), K, Out);
}

வெறுமை llvm::getSipHash_2_4_128(ArrayRef<uint8_t> In, மாறிலி uint8_t (&K)[16],
                              uint8_t (&Out)[16]) {
  siphash<2, 4>(In.data(), In.size(), K, Out);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  நிலையான மாறிலி uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t RawHashBytes[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, RawHashBytes);
  திரும்பு endian::read64le(RawHashBytes);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::getPointerAuthStableSipHash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/பொய், /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  திரும்பு Discriminator;
}
