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

#অন্তর্ভাব "llvm/Support/SHA1.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Endian.h"
#অন্তর্ভাব "llvm/Support/SwapByteOrder.h"
#অন্তর্ভাব <string.h>

using নামাকাশ llvm;

স্থির অন্তঃরেখ uint32_t rol(uint32_t সংখ্যা, পূর্ণসংখ্যা Bits) {
  ফেরত (সংখ্যা << Bits) | (সংখ্যা >> (32 - Bits));
}

স্থির অন্তঃরেখ uint32_t blk0(uint32_t *Buf, পূর্ণসংখ্যা I) { ফেরত Buf[I]; }

স্থির অন্তঃরেখ uint32_t blk(uint32_t *Buf, পূর্ণসংখ্যা I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  ফেরত Buf[I & 15];
}

স্থির অন্তঃরেখ শূন্য r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, পূর্ণসংখ্যা I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

স্থির অন্তঃরেখ শূন্য r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, পূর্ণসংখ্যা I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

স্থির অন্তঃরেখ শূন্য r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, পূর্ণসংখ্যা I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

স্থির অন্তঃরেখ শূন্য r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, পূর্ণসংখ্যা I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

স্থির অন্তঃরেখ শূন্য r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, পূর্ণসংখ্যা I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/* code */
#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

#define SEED_0 0x67452301
#define SEED_1 0xefcdab89
#define SEED_2 0x98badcfe
#define SEED_3 0x10325476
#define SEED_4 0xc3d2e1f0

শূন্য SHA1::init() {
  InternalState.State[0] = SEED_0;
  InternalState.State[1] = SEED_1;
  InternalState.State[2] = SEED_2;
  InternalState.State[3] = SEED_3;
  InternalState.State[4] = SEED_4;
  InternalState.Byte_গুনতি = 0;
  InternalState.বাফার_Offset = 0;
}

শূন্য SHA1::hashBlock() {
  uint32_t A = InternalState.State[0];
  uint32_t B = InternalState.State[1];
  uint32_t C = InternalState.State[2];
  uint32_t D = InternalState.State[3];
  uint32_t E = InternalState.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, InternalState.বাফার.L);
  r0(E, A, B, C, D, 1, InternalState.বাফার.L);
  r0(D, E, A, B, C, 2, InternalState.বাফার.L);
  r0(C, D, E, A, B, 3, InternalState.বাফার.L);
  r0(B, C, D, E, A, 4, InternalState.বাফার.L);
  r0(A, B, C, D, E, 5, InternalState.বাফার.L);
  r0(E, A, B, C, D, 6, InternalState.বাফার.L);
  r0(D, E, A, B, C, 7, InternalState.বাফার.L);
  r0(C, D, E, A, B, 8, InternalState.বাফার.L);
  r0(B, C, D, E, A, 9, InternalState.বাফার.L);
  r0(A, B, C, D, E, 10, InternalState.বাফার.L);
  r0(E, A, B, C, D, 11, InternalState.বাফার.L);
  r0(D, E, A, B, C, 12, InternalState.বাফার.L);
  r0(C, D, E, A, B, 13, InternalState.বাফার.L);
  r0(B, C, D, E, A, 14, InternalState.বাফার.L);
  r0(A, B, C, D, E, 15, InternalState.বাফার.L);
  r1(E, A, B, C, D, 16, InternalState.বাফার.L);
  r1(D, E, A, B, C, 17, InternalState.বাফার.L);
  r1(C, D, E, A, B, 18, InternalState.বাফার.L);
  r1(B, C, D, E, A, 19, InternalState.বাফার.L);

  r2(A, B, C, D, E, 20, InternalState.বাফার.L);
  r2(E, A, B, C, D, 21, InternalState.বাফার.L);
  r2(D, E, A, B, C, 22, InternalState.বাফার.L);
  r2(C, D, E, A, B, 23, InternalState.বাফার.L);
  r2(B, C, D, E, A, 24, InternalState.বাফার.L);
  r2(A, B, C, D, E, 25, InternalState.বাফার.L);
  r2(E, A, B, C, D, 26, InternalState.বাফার.L);
  r2(D, E, A, B, C, 27, InternalState.বাফার.L);
  r2(C, D, E, A, B, 28, InternalState.বাফার.L);
  r2(B, C, D, E, A, 29, InternalState.বাফার.L);
  r2(A, B, C, D, E, 30, InternalState.বাফার.L);
  r2(E, A, B, C, D, 31, InternalState.বাফার.L);
  r2(D, E, A, B, C, 32, InternalState.বাফার.L);
  r2(C, D, E, A, B, 33, InternalState.বাফার.L);
  r2(B, C, D, E, A, 34, InternalState.বাফার.L);
  r2(A, B, C, D, E, 35, InternalState.বাফার.L);
  r2(E, A, B, C, D, 36, InternalState.বাফার.L);
  r2(D, E, A, B, C, 37, InternalState.বাফার.L);
  r2(C, D, E, A, B, 38, InternalState.বাফার.L);
  r2(B, C, D, E, A, 39, InternalState.বাফার.L);

  r3(A, B, C, D, E, 40, InternalState.বাফার.L);
  r3(E, A, B, C, D, 41, InternalState.বাফার.L);
  r3(D, E, A, B, C, 42, InternalState.বাফার.L);
  r3(C, D, E, A, B, 43, InternalState.বাফার.L);
  r3(B, C, D, E, A, 44, InternalState.বাফার.L);
  r3(A, B, C, D, E, 45, InternalState.বাফার.L);
  r3(E, A, B, C, D, 46, InternalState.বাফার.L);
  r3(D, E, A, B, C, 47, InternalState.বাফার.L);
  r3(C, D, E, A, B, 48, InternalState.বাফার.L);
  r3(B, C, D, E, A, 49, InternalState.বাফার.L);
  r3(A, B, C, D, E, 50, InternalState.বাফার.L);
  r3(E, A, B, C, D, 51, InternalState.বাফার.L);
  r3(D, E, A, B, C, 52, InternalState.বাফার.L);
  r3(C, D, E, A, B, 53, InternalState.বাফার.L);
  r3(B, C, D, E, A, 54, InternalState.বাফার.L);
  r3(A, B, C, D, E, 55, InternalState.বাফার.L);
  r3(E, A, B, C, D, 56, InternalState.বাফার.L);
  r3(D, E, A, B, C, 57, InternalState.বাফার.L);
  r3(C, D, E, A, B, 58, InternalState.বাফার.L);
  r3(B, C, D, E, A, 59, InternalState.বাফার.L);

  r4(A, B, C, D, E, 60, InternalState.বাফার.L);
  r4(E, A, B, C, D, 61, InternalState.বাফার.L);
  r4(D, E, A, B, C, 62, InternalState.বাফার.L);
  r4(C, D, E, A, B, 63, InternalState.বাফার.L);
  r4(B, C, D, E, A, 64, InternalState.বাফার.L);
  r4(A, B, C, D, E, 65, InternalState.বাফার.L);
  r4(E, A, B, C, D, 66, InternalState.বাফার.L);
  r4(D, E, A, B, C, 67, InternalState.বাফার.L);
  r4(C, D, E, A, B, 68, InternalState.বাফার.L);
  r4(B, C, D, E, A, 69, InternalState.বাফার.L);
  r4(A, B, C, D, E, 70, InternalState.বাফার.L);
  r4(E, A, B, C, D, 71, InternalState.বাফার.L);
  r4(D, E, A, B, C, 72, InternalState.বাফার.L);
  r4(C, D, E, A, B, 73, InternalState.বাফার.L);
  r4(B, C, D, E, A, 74, InternalState.বাফার.L);
  r4(A, B, C, D, E, 75, InternalState.বাফার.L);
  r4(E, A, B, C, D, 76, InternalState.বাফার.L);
  r4(D, E, A, B, C, 77, InternalState.বাফার.L);
  r4(C, D, E, A, B, 78, InternalState.বাফার.L);
  r4(B, C, D, E, A, 79, InternalState.বাফার.L);

  InternalState.State[0] += A;
  InternalState.State[1] += B;
  InternalState.State[2] += C;
  InternalState.State[3] += D;
  InternalState.State[4] += E;
}

শূন্য SHA1::addUncounted(uint8_t তথ্য_2) {
  যদি constexpr (sys::IsBigEndianHost)
    InternalState.বাফার.C[InternalState.বাফার_Offset] = তথ্য_2;
  অন্যথা
    InternalState.বাফার.C[InternalState.বাফার_Offset ^ 3] = তথ্য_2;

  InternalState.বাফার_Offset++;
  যদি (InternalState.বাফার_Offset == BLOCK_দৈর্ঘ্য) {
    hashBlock();
    InternalState.বাফার_Offset = 0;
  }
}

শূন্য SHA1::writebyte(uint8_t তথ্য_2) {
  ++InternalState.Byte_গুনতি;
  addUncounted(তথ্য_2);
}

শূন্য SHA1::update(ArrayRef<uint8_t> তথ্য_2) {
  InternalState.Byte_গুনতি += তথ্য_2.আকার_3();

  // Finish the current block.
  যদি (InternalState.বাফার_Offset > 0) {
    ধ্রুবক size_t Remainder = std::min<size_t>(
        তথ্য_2.আকার_3(), BLOCK_দৈর্ঘ্য - InternalState.বাফার_Offset);
    জন্য (size_t I = 0; I < Remainder; ++I)
      addUncounted(তথ্য_2[I]);
    তথ্য_2 = তথ্য_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  যখন (তথ্য_2.আকার_3() >= BLOCK_দৈর্ঘ্য) {
    প্রতিজ্ঞা(InternalState.বাফার_Offset == 0);
    static_assert(BLOCK_দৈর্ঘ্য % 4 == 0);
    constexpr size_t BLOCK_দৈর্ঘ্য_32 = BLOCK_দৈর্ঘ্য / 4;
    জন্য (size_t I = 0; I < BLOCK_দৈর্ঘ্য_32; ++I)
      InternalState.বাফার.L[I] = support::endian::read32be(&তথ্য_2[I * 4]);
    hashBlock();
    তথ্য_2 = তথ্য_2.drop_front(BLOCK_দৈর্ঘ্য);
  }

  // Finish the remainder.
  জন্য (uint8_t C : তথ্য_2)
    addUncounted(C);
}

শূন্য SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<অক্ষর *>(Str.তথ্য()), Str.আকার_3()));
}

শূন্য SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  যখন (InternalState.বাফার_Offset != 56)
    addUncounted(0x00);

  // Append length in the last 8 bytes
  addUncounted(0); // We're only using 32 bit lengths
  addUncounted(0); // But SHA-1 supports 64 bit lengths
  addUncounted(0); // So zero pad the top bits
  addUncounted(InternalState.Byte_গুনতি >> 29); // Shifting to multiply by 8
  addUncounted(InternalState.Byte_গুনতি >>
               21); // as SHA-1 supports bitstreams as well as
  addUncounted(InternalState.Byte_গুনতি >> 13); // byte.
  addUncounted(InternalState.Byte_গুনতি >> 5);
  addUncounted(InternalState.Byte_গুনতি << 3);
}

শূন্য SHA1::final(std::array<uint32_t, HASH_দৈর্ঘ্য / 4> &Hash_ফলাফল) {
  // Pad to complete the last block
  pad();

  যদি constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    জন্য (পূর্ণসংখ্যা i = 0; i < 5; i++) {
      Hash_ফলাফল[i] = InternalState.State[i];
    }
  } অন্যথা {
    // Swap byte order back
    জন্য (পূর্ণসংখ্যা i = 0; i < 5; i++) {
      Hash_ফলাফল[i] = llvm::byteswap(InternalState.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  সংঘ {
    std::array<uint32_t, HASH_দৈর্ঘ্য / 4> Hash_ফলাফল;
    std::array<uint8_t, HASH_দৈর্ঘ্য> Return_ফলাফল;
  };
  static_assert(আকার(Hash_ফলাফল) == আকার(Return_ফলাফল));
  final(Hash_ফলাফল);
  ফেরত Return_ফলাফল;
}

std::array<uint8_t, 20> SHA1::ফলাফল_2() {
  auto StateToRestore = InternalState;

  auto Hash = final();

  // Restore the state
  InternalState = StateToRestore;

  // Return pointer to hash (20 characters)
  ফেরত Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> তথ্য_2) {
  SHA1 Hash;
  Hash.update(তথ্য_2);
  ফেরত Hash.final();
}
