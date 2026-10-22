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

#अन्तर्भाव "llvm/Support/SipHash.h"
#अन्तर्भाव "siphash/SipHash.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव <cstdint>

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश support;

#परिभाषय DEBUG_TYPE "llvm-siphash"

शून्य llvm::getSipHash_2_4_64(ArrayRef<uint8_t> अन्तर्_2, ध्रुव uint8_t (&K)[16],
                             uint8_t (&बहिर्गमनम्)[8]) {
  siphash<2, 4>(अन्तर्_2.दत्त(), अन्तर्_2.आकार_2(), K, बहिर्गमनम्);
}

शून्य llvm::getSipHash_2_4_128(ArrayRef<uint8_t> अन्तर्_2, ध्रुव uint8_t (&K)[16],
                              uint8_t (&बहिर्गमनम्)[16]) {
  siphash<2, 4>(अन्तर्_2.दत्त(), अन्तर्_2.आकार_2(), K, बहिर्गमनम्);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t llvm::getStableSipHash(StringRef Str) {
  स्थिर ध्रुव uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t Raw_Hash_अष्टकानि[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, Raw_Hash_अष्टकानि);
  प्रत्यागम endian::read64le(Raw_Hash_अष्टकानि);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t llvm::getPointerAuthStableSipHash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/असत्य, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  प्रत्यागम Discriminator;
}
