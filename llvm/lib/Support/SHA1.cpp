//====- SHA1.cpp - Private copy of the SHA1 implementation ---*- C++ -* ======//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This code is taken from public domain
// (http://oauth.googlecode.com/svn/code/c/liboauth/src/sha1.c and
// http://cvsweb.netbsd.org/bsdweb.cgi/src/common/lib/libc/hash/sha1/sha1.c?rev=1.6)
// and modified by wrapping it in a C++ interface for LLVM,
// and removing unnecessary code.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/SHA1.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Endian.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"
#ಅನ್ತರ್ಭಾವ <string.h>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t rol(uint32_t ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು_2) {
  ಹಿಂತಿರುಗಿ (ಸಂಖ್ಯೆ << ಬಿಟ್ಟುಗಳು_2) | (ಸಂಖ್ಯೆ >> (32 - ಬಿಟ್ಟುಗಳು_2));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t blk0(uint32_t *Buf, ಪೂರ್ಣಾಂಕ I) { ಹಿಂತಿರುಗಿ Buf[I]; }

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t blk(uint32_t *Buf, ಪೂರ್ಣಾಂಕ I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  ಹಿಂತಿರುಗಿ Buf[I & 15];
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ಪೂರ್ಣಾಂಕ I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ಪೂರ್ಣಾಂಕ I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ಪೂರ್ಣಾಂಕ I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ಪೂರ್ಣಾಂಕ I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ಪೂರ್ಣಾಂಕ I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/*ಸಂಕೇತ*/
#ವ್ಯಾಖ್ಯಾನಿಸು SHA1_K0 0x5a827999
#ವ್ಯಾಖ್ಯಾನಿಸು SHA1_K20 0x6ed9eba1
#ವ್ಯಾಖ್ಯಾನಿಸು SHA1_K40 0x8f1bbcdc
#ವ್ಯಾಖ್ಯಾನಿಸು SHA1_K60 0xca62c1d6

#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಜ_0 0x67452301
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಜ_1 0xefcdab89
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಜ_2_2 0x98badcfe
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಜ_3 0x10325476
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಜ_4 0xc3d2e1f0

ಶೂನ್ಯ SHA1::init() {
  ಆಂತರಿಕ_State.State[0] = ಬೀಜ_0;
  ಆಂತರಿಕ_State.State[1] = ಬೀಜ_1;
  ಆಂತರಿಕ_State.State[2] = ಬೀಜ_2_2;
  ಆಂತರಿಕ_State.State[3] = ಬೀಜ_3;
  ಆಂತರಿಕ_State.State[4] = ಬೀಜ_4;
  ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ = 0;
  ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ = 0;
}

ಶೂನ್ಯ SHA1::hash_ಖಂಡ() {
  uint32_t A = ಆಂತರಿಕ_State.State[0];
  uint32_t B = ಆಂತರಿಕ_State.State[1];
  uint32_t C = ಆಂತರಿಕ_State.State[2];
  uint32_t D = ಆಂತರಿಕ_State.State[3];
  uint32_t E = ಆಂತರಿಕ_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(E, A, B, C, D, 1, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(D, E, A, B, C, 2, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(C, D, E, A, B, 3, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(B, C, D, E, A, 4, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(A, B, C, D, E, 5, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(E, A, B, C, D, 6, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(D, E, A, B, C, 7, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(C, D, E, A, B, 8, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(B, C, D, E, A, 9, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(A, B, C, D, E, 10, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(E, A, B, C, D, 11, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(D, E, A, B, C, 12, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(C, D, E, A, B, 13, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(B, C, D, E, A, 14, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r0(A, B, C, D, E, 15, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r1(E, A, B, C, D, 16, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r1(D, E, A, B, C, 17, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r1(C, D, E, A, B, 18, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r1(B, C, D, E, A, 19, ಆಂತರಿಕ_State.ಬಫರ್.L);

  r2(A, B, C, D, E, 20, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(E, A, B, C, D, 21, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(D, E, A, B, C, 22, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(C, D, E, A, B, 23, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(B, C, D, E, A, 24, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(A, B, C, D, E, 25, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(E, A, B, C, D, 26, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(D, E, A, B, C, 27, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(C, D, E, A, B, 28, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(B, C, D, E, A, 29, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(A, B, C, D, E, 30, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(E, A, B, C, D, 31, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(D, E, A, B, C, 32, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(C, D, E, A, B, 33, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(B, C, D, E, A, 34, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(A, B, C, D, E, 35, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(E, A, B, C, D, 36, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(D, E, A, B, C, 37, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(C, D, E, A, B, 38, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r2(B, C, D, E, A, 39, ಆಂತರಿಕ_State.ಬಫರ್.L);

  r3(A, B, C, D, E, 40, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(E, A, B, C, D, 41, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(D, E, A, B, C, 42, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(C, D, E, A, B, 43, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(B, C, D, E, A, 44, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(A, B, C, D, E, 45, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(E, A, B, C, D, 46, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(D, E, A, B, C, 47, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(C, D, E, A, B, 48, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(B, C, D, E, A, 49, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(A, B, C, D, E, 50, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(E, A, B, C, D, 51, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(D, E, A, B, C, 52, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(C, D, E, A, B, 53, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(B, C, D, E, A, 54, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(A, B, C, D, E, 55, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(E, A, B, C, D, 56, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(D, E, A, B, C, 57, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(C, D, E, A, B, 58, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r3(B, C, D, E, A, 59, ಆಂತರಿಕ_State.ಬಫರ್.L);

  r4(A, B, C, D, E, 60, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(E, A, B, C, D, 61, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(D, E, A, B, C, 62, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(C, D, E, A, B, 63, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(B, C, D, E, A, 64, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(A, B, C, D, E, 65, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(E, A, B, C, D, 66, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(D, E, A, B, C, 67, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(C, D, E, A, B, 68, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(B, C, D, E, A, 69, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(A, B, C, D, E, 70, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(E, A, B, C, D, 71, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(D, E, A, B, C, 72, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(C, D, E, A, B, 73, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(B, C, D, E, A, 74, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(A, B, C, D, E, 75, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(E, A, B, C, D, 76, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(D, E, A, B, C, 77, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(C, D, E, A, B, 78, ಆಂತರಿಕ_State.ಬಫರ್.L);
  r4(B, C, D, E, A, 79, ಆಂತರಿಕ_State.ಬಫರ್.L);

  ಆಂತರಿಕ_State.State[0] += A;
  ಆಂತರಿಕ_State.State[1] += B;
  ಆಂತರಿಕ_State.State[2] += C;
  ಆಂತರಿಕ_State.State[3] += D;
  ಆಂತರಿಕ_State.State[4] += E;
}

ಶೂನ್ಯ SHA1::ಕೂಡಿಸು_Uncounted(uint8_t ದತ್ತಾಂಶ_2) {
  ಆದರೆ constexpr (sys::IsBigEndianHost)
    ಆಂತರಿಕ_State.ಬಫರ್.C[ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ] = ದತ್ತಾಂಶ_2;
  ಇಲ್ಲದಿದ್ದರೆ
    ಆಂತರಿಕ_State.ಬಫರ್.C[ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ ^ 3] = ದತ್ತಾಂಶ_2;

  ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ++;
  ಆದರೆ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ == ಖಂಡ_ಉದ್ದಳತೆ) {
    hash_ಖಂಡ();
    ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ = 0;
  }
}

ಶೂನ್ಯ SHA1::writebyte(uint8_t ದತ್ತಾಂಶ_2) {
  ++ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ;
  ಕೂಡಿಸು_Uncounted(ದತ್ತಾಂಶ_2);
}

ಶೂನ್ಯ SHA1::update(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) {
  ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ += ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3();

  // Finish the current block.
  ಆದರೆ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ > 0) {
    ಸ್ಥಿರಾಂಕ size_t Remainder = std::min<size_t>(
        ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3(), ಖಂಡ_ಉದ್ದಳತೆ - ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ);
    ಗಾಗಿ (size_t I = 0; I < Remainder; ++I)
      ಕೂಡಿಸು_Uncounted(ದತ್ತಾಂಶ_2[I]);
    ದತ್ತಾಂಶ_2 = ದತ್ತಾಂಶ_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  ಆಗ (ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3() >= ಖಂಡ_ಉದ್ದಳತೆ) {
    ಪ್ರತಿಜ್ಞಾ(ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ == 0);
    static_assert(ಖಂಡ_ಉದ್ದಳತೆ % 4 == 0);
    constexpr size_t ಖಂಡ_ಉದ್ದಳತೆ_32 = ಖಂಡ_ಉದ್ದಳತೆ / 4;
    ಗಾಗಿ (size_t I = 0; I < ಖಂಡ_ಉದ್ದಳತೆ_32; ++I)
      ಆಂತರಿಕ_State.ಬಫರ್.L[I] = support::endian::read32be(&ದತ್ತಾಂಶ_2[I * 4]);
    hash_ಖಂಡ();
    ದತ್ತಾಂಶ_2 = ದತ್ತಾಂಶ_2.drop_front(ಖಂಡ_ಉದ್ದಳತೆ);
  }

  // Finish the remainder.
  ಗಾಗಿ (uint8_t C : ದತ್ತಾಂಶ_2)
    ಕೂಡಿಸು_Uncounted(C);
}

ಶೂನ್ಯ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<ಅಕ್ಷರ *>(Str.ದತ್ತಾಂಶ()), Str.ಗಾತ್ರ_3()));
}

ಶೂನ್ಯ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  ಕೂಡಿಸು_Uncounted(0x80);
  ಆಗ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ != 56)
    ಕೂಡಿಸು_Uncounted(0x00);

  // Append length in the last 8 bytes
  ಕೂಡಿಸು_Uncounted(0); // We're only using 32 bit lengths
  ಕೂಡಿಸು_Uncounted(0); // But SHA-1 supports 64 bit lengths
  ಕೂಡಿಸು_Uncounted(0); // So zero pad the top bits
  ಕೂಡಿಸು_Uncounted(ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ >> 29); // Shifting to multiply by 8
  ಕೂಡಿಸು_Uncounted(ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ >>
               21); // as SHA-1 supports bitstreams as well as
  ಕೂಡಿಸು_Uncounted(ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ >> 13); // byte.
  ಕೂಡಿಸು_Uncounted(ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ >> 5);
  ಕೂಡಿಸು_Uncounted(ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ << 3);
}

ಶೂನ್ಯ SHA1::final(std::array<uint32_t, HASH_ಉದ್ದಳತೆ / 4> &Hash_ಫಲಿತಾಂಶ) {
  // Pad to complete the last block
  pad();

  ಆದರೆ constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < 5; i++) {
      Hash_ಫಲಿತಾಂಶ[i] = ಆಂತರಿಕ_State.State[i];
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Swap byte order back
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < 5; i++) {
      Hash_ಫಲಿತಾಂಶ[i] = llvm::byteswap(ಆಂತರಿಕ_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  ಒಕ್ಕೂಟ {
    std::array<uint32_t, HASH_ಉದ್ದಳತೆ / 4> Hash_ಫಲಿತಾಂಶ;
    std::array<uint8_t, HASH_ಉದ್ದಳತೆ> Return_ಫಲಿತಾಂಶ;
  };
  static_assert(ಗಾತ್ರ(Hash_ಫಲಿತಾಂಶ) == ಗಾತ್ರ(Return_ಫಲಿತಾಂಶ));
  final(Hash_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ Return_ಫಲಿತಾಂಶ;
}

std::array<uint8_t, 20> SHA1::ಫಲಿತಾಂಶ_2() {
  ಸ್ವಯಂಚಾಲಿತ State_ಗಮ್ಯ_Restore = ಆಂತರಿಕ_State;

  ಸ್ವಯಂಚಾಲಿತ Hash = final();

  // Restore the state
  ಆಂತರಿಕ_State = State_ಗಮ್ಯ_Restore;

  // Return pointer to hash (20 characters)
  ಹಿಂತಿರುಗಿ Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) {
  SHA1 Hash;
  Hash.update(ದತ್ತಾಂಶ_2);
  ಹಿಂತಿರುಗಿ Hash.final();
}
