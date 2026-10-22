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

#ضمن "llvm/Support/SHA1.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Endian.h"
#ضمن "llvm/Support/SwapByteOrder.h"
#ضمن <string.h>

استعمال نطاق llvm;

ساكن مضمن uint32_t rol(uint32_t عدد, صحيح بٽ_2) {
  أرجع (عدد << بٽ_2) | (عدد >> (32 - بٽ_2));
}

ساكن مضمن uint32_t blk0(uint32_t *Buf, صحيح I) { أرجع Buf[I]; }

ساكن مضمن uint32_t blk(uint32_t *Buf, صحيح I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  أرجع Buf[I & 15];
}

ساكن مضمن فراغ r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحيح I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ساكن مضمن فراغ r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحيح I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ساكن مضمن فراغ r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحيح I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

ساكن مضمن فراغ r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحيح I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

ساكن مضمن فراغ r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحيح I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/*ڪوڊ*/
#وصف_ڪريو SHA1_K0 0x5a827999
#وصف_ڪريو SHA1_K20 0x6ed9eba1
#وصف_ڪريو SHA1_K40 0x8f1bbcdc
#وصف_ڪريو SHA1_K60 0xca62c1d6

#وصف_ڪريو ٻج_0 0x67452301
#وصف_ڪريو ٻج_1 0xefcdab89
#وصف_ڪريو ٻج_2_2 0x98badcfe
#وصف_ڪريو ٻج_3 0x10325476
#وصف_ڪريو ٻج_4 0xc3d2e1f0

فراغ SHA1::init() {
  اندروني_State.State[0] = ٻج_0;
  اندروني_State.State[1] = ٻج_1;
  اندروني_State.State[2] = ٻج_2_2;
  اندروني_State.State[3] = ٻج_3;
  اندروني_State.State[4] = ٻج_4;
  اندروني_State.Byte_ڳڻپ = 0;
  اندروني_State.بفر_وٿي = 0;
}

فراغ SHA1::hash_بلاڪ() {
  uint32_t A = اندروني_State.State[0];
  uint32_t B = اندروني_State.State[1];
  uint32_t C = اندروني_State.State[2];
  uint32_t D = اندروني_State.State[3];
  uint32_t E = اندروني_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, اندروني_State.بفر.L);
  r0(E, A, B, C, D, 1, اندروني_State.بفر.L);
  r0(D, E, A, B, C, 2, اندروني_State.بفر.L);
  r0(C, D, E, A, B, 3, اندروني_State.بفر.L);
  r0(B, C, D, E, A, 4, اندروني_State.بفر.L);
  r0(A, B, C, D, E, 5, اندروني_State.بفر.L);
  r0(E, A, B, C, D, 6, اندروني_State.بفر.L);
  r0(D, E, A, B, C, 7, اندروني_State.بفر.L);
  r0(C, D, E, A, B, 8, اندروني_State.بفر.L);
  r0(B, C, D, E, A, 9, اندروني_State.بفر.L);
  r0(A, B, C, D, E, 10, اندروني_State.بفر.L);
  r0(E, A, B, C, D, 11, اندروني_State.بفر.L);
  r0(D, E, A, B, C, 12, اندروني_State.بفر.L);
  r0(C, D, E, A, B, 13, اندروني_State.بفر.L);
  r0(B, C, D, E, A, 14, اندروني_State.بفر.L);
  r0(A, B, C, D, E, 15, اندروني_State.بفر.L);
  r1(E, A, B, C, D, 16, اندروني_State.بفر.L);
  r1(D, E, A, B, C, 17, اندروني_State.بفر.L);
  r1(C, D, E, A, B, 18, اندروني_State.بفر.L);
  r1(B, C, D, E, A, 19, اندروني_State.بفر.L);

  r2(A, B, C, D, E, 20, اندروني_State.بفر.L);
  r2(E, A, B, C, D, 21, اندروني_State.بفر.L);
  r2(D, E, A, B, C, 22, اندروني_State.بفر.L);
  r2(C, D, E, A, B, 23, اندروني_State.بفر.L);
  r2(B, C, D, E, A, 24, اندروني_State.بفر.L);
  r2(A, B, C, D, E, 25, اندروني_State.بفر.L);
  r2(E, A, B, C, D, 26, اندروني_State.بفر.L);
  r2(D, E, A, B, C, 27, اندروني_State.بفر.L);
  r2(C, D, E, A, B, 28, اندروني_State.بفر.L);
  r2(B, C, D, E, A, 29, اندروني_State.بفر.L);
  r2(A, B, C, D, E, 30, اندروني_State.بفر.L);
  r2(E, A, B, C, D, 31, اندروني_State.بفر.L);
  r2(D, E, A, B, C, 32, اندروني_State.بفر.L);
  r2(C, D, E, A, B, 33, اندروني_State.بفر.L);
  r2(B, C, D, E, A, 34, اندروني_State.بفر.L);
  r2(A, B, C, D, E, 35, اندروني_State.بفر.L);
  r2(E, A, B, C, D, 36, اندروني_State.بفر.L);
  r2(D, E, A, B, C, 37, اندروني_State.بفر.L);
  r2(C, D, E, A, B, 38, اندروني_State.بفر.L);
  r2(B, C, D, E, A, 39, اندروني_State.بفر.L);

  r3(A, B, C, D, E, 40, اندروني_State.بفر.L);
  r3(E, A, B, C, D, 41, اندروني_State.بفر.L);
  r3(D, E, A, B, C, 42, اندروني_State.بفر.L);
  r3(C, D, E, A, B, 43, اندروني_State.بفر.L);
  r3(B, C, D, E, A, 44, اندروني_State.بفر.L);
  r3(A, B, C, D, E, 45, اندروني_State.بفر.L);
  r3(E, A, B, C, D, 46, اندروني_State.بفر.L);
  r3(D, E, A, B, C, 47, اندروني_State.بفر.L);
  r3(C, D, E, A, B, 48, اندروني_State.بفر.L);
  r3(B, C, D, E, A, 49, اندروني_State.بفر.L);
  r3(A, B, C, D, E, 50, اندروني_State.بفر.L);
  r3(E, A, B, C, D, 51, اندروني_State.بفر.L);
  r3(D, E, A, B, C, 52, اندروني_State.بفر.L);
  r3(C, D, E, A, B, 53, اندروني_State.بفر.L);
  r3(B, C, D, E, A, 54, اندروني_State.بفر.L);
  r3(A, B, C, D, E, 55, اندروني_State.بفر.L);
  r3(E, A, B, C, D, 56, اندروني_State.بفر.L);
  r3(D, E, A, B, C, 57, اندروني_State.بفر.L);
  r3(C, D, E, A, B, 58, اندروني_State.بفر.L);
  r3(B, C, D, E, A, 59, اندروني_State.بفر.L);

  r4(A, B, C, D, E, 60, اندروني_State.بفر.L);
  r4(E, A, B, C, D, 61, اندروني_State.بفر.L);
  r4(D, E, A, B, C, 62, اندروني_State.بفر.L);
  r4(C, D, E, A, B, 63, اندروني_State.بفر.L);
  r4(B, C, D, E, A, 64, اندروني_State.بفر.L);
  r4(A, B, C, D, E, 65, اندروني_State.بفر.L);
  r4(E, A, B, C, D, 66, اندروني_State.بفر.L);
  r4(D, E, A, B, C, 67, اندروني_State.بفر.L);
  r4(C, D, E, A, B, 68, اندروني_State.بفر.L);
  r4(B, C, D, E, A, 69, اندروني_State.بفر.L);
  r4(A, B, C, D, E, 70, اندروني_State.بفر.L);
  r4(E, A, B, C, D, 71, اندروني_State.بفر.L);
  r4(D, E, A, B, C, 72, اندروني_State.بفر.L);
  r4(C, D, E, A, B, 73, اندروني_State.بفر.L);
  r4(B, C, D, E, A, 74, اندروني_State.بفر.L);
  r4(A, B, C, D, E, 75, اندروني_State.بفر.L);
  r4(E, A, B, C, D, 76, اندروني_State.بفر.L);
  r4(D, E, A, B, C, 77, اندروني_State.بفر.L);
  r4(C, D, E, A, B, 78, اندروني_State.بفر.L);
  r4(B, C, D, E, A, 79, اندروني_State.بفر.L);

  اندروني_State.State[0] += A;
  اندروني_State.State[1] += B;
  اندروني_State.State[2] += C;
  اندروني_State.State[3] += D;
  اندروني_State.State[4] += E;
}

فراغ SHA1::جوڙيو_Uncounted(uint8_t معلومات_2) {
  إذا constexpr (sys::IsBigEndianHost)
    اندروني_State.بفر.C[اندروني_State.بفر_وٿي] = معلومات_2;
  وإلا
    اندروني_State.بفر.C[اندروني_State.بفر_وٿي ^ 3] = معلومات_2;

  اندروني_State.بفر_وٿي++;
  إذا (اندروني_State.بفر_وٿي == بلاڪ_ڊيگهائي) {
    hash_بلاڪ();
    اندروني_State.بفر_وٿي = 0;
  }
}

فراغ SHA1::writebyte(uint8_t معلومات_2) {
  ++اندروني_State.Byte_ڳڻپ;
  جوڙيو_Uncounted(معلومات_2);
}

فراغ SHA1::update(ArrayRef<uint8_t> معلومات_2) {
  اندروني_State.Byte_ڳڻپ += معلومات_2.سائيز_2();

  // Finish the current block.
  إذا (اندروني_State.بفر_وٿي > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        معلومات_2.سائيز_2(), بلاڪ_ڊيگهائي - اندروني_State.بفر_وٿي);
    لكل (size_t I = 0; I < Remainder; ++I)
      جوڙيو_Uncounted(معلومات_2[I]);
    معلومات_2 = معلومات_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  طالما (معلومات_2.سائيز_2() >= بلاڪ_ڊيگهائي) {
    أكد(اندروني_State.بفر_وٿي == 0);
    static_assert(بلاڪ_ڊيگهائي % 4 == 0);
    constexpr size_t بلاڪ_ڊيگهائي_32 = بلاڪ_ڊيگهائي / 4;
    لكل (size_t I = 0; I < بلاڪ_ڊيگهائي_32; ++I)
      اندروني_State.بفر.L[I] = support::endian::read32be(&معلومات_2[I * 4]);
    hash_بلاڪ();
    معلومات_2 = معلومات_2.drop_front(بلاڪ_ڊيگهائي);
  }

  // Finish the remainder.
  لكل (uint8_t C : معلومات_2)
    جوڙيو_Uncounted(C);
}

فراغ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.معلومات()), Str.سائيز_2()));
}

فراغ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جوڙيو_Uncounted(0x80);
  طالما (اندروني_State.بفر_وٿي != 56)
    جوڙيو_Uncounted(0x00);

  // Append length in the last 8 bytes
  جوڙيو_Uncounted(0); // We're only using 32 bit lengths
  جوڙيو_Uncounted(0); // But SHA-1 supports 64 bit lengths
  جوڙيو_Uncounted(0); // So zero pad the top bits
  جوڙيو_Uncounted(اندروني_State.Byte_ڳڻپ >> 29); // Shifting to multiply by 8
  جوڙيو_Uncounted(اندروني_State.Byte_ڳڻپ >>
               21); // as SHA-1 supports bitstreams as well as
  جوڙيو_Uncounted(اندروني_State.Byte_ڳڻپ >> 13); // byte.
  جوڙيو_Uncounted(اندروني_State.Byte_ڳڻپ >> 5);
  جوڙيو_Uncounted(اندروني_State.Byte_ڳڻپ << 3);
}

فراغ SHA1::final(std::array<uint32_t, HASH_ڊيگهائي / 4> &Hash_نتيجو) {
  // Pad to complete the last block
  pad();

  إذا constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتيجو[i] = اندروني_State.State[i];
    }
  } وإلا {
    // Swap byte order back
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتيجو[i] = llvm::byteswap(اندروني_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  اتحاد {
    std::array<uint32_t, HASH_ڊيگهائي / 4> Hash_نتيجو;
    std::array<uint8_t, HASH_ڊيگهائي> Return_نتيجو;
  };
  static_assert(حجم(Hash_نتيجو) == حجم(Return_نتيجو));
  final(Hash_نتيجو);
  أرجع Return_نتيجو;
}

std::array<uint8_t, 20> SHA1::نتيجو_2() {
  خودڪار State_منزل_Restore = اندروني_State;

  خودڪار Hash = final();

  // Restore the state
  اندروني_State = State_منزل_Restore;

  // Return pointer to hash (20 characters)
  أرجع Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> معلومات_2) {
  SHA1 Hash;
  Hash.update(معلومات_2);
  أرجع Hash.final();
}
