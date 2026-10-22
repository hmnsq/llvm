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

ورتوں نطاق llvm;

ساكن مضمن uint32_t rol(uint32_t نمبر, صحيح بٹ_2) {
  أرجع (نمبر << بٹ_2) | (نمبر >> (32 - بٹ_2));
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

/*کوڈ*/
#تعریف_کرو SHA1_K0 0x5a827999
#تعریف_کرو SHA1_K20 0x6ed9eba1
#تعریف_کرو SHA1_K40 0x8f1bbcdc
#تعریف_کرو SHA1_K60 0xca62c1d6

#تعریف_کرو بیج_0 0x67452301
#تعریف_کرو بیج_1 0xefcdab89
#تعریف_کرو بیج_2_2 0x98badcfe
#تعریف_کرو بیج_3 0x10325476
#تعریف_کرو بیج_4 0xc3d2e1f0

فراغ SHA1::init() {
  اندرونی_State.State[0] = بیج_0;
  اندرونی_State.State[1] = بیج_1;
  اندرونی_State.State[2] = بیج_2_2;
  اندرونی_State.State[3] = بیج_3;
  اندرونی_State.State[4] = بیج_4;
  اندرونی_State.Byte_گنتی = 0;
  اندرونی_State.بفر_وتھ = 0;
}

فراغ SHA1::hash_بلاک() {
  uint32_t A = اندرونی_State.State[0];
  uint32_t B = اندرونی_State.State[1];
  uint32_t C = اندرونی_State.State[2];
  uint32_t D = اندرونی_State.State[3];
  uint32_t E = اندرونی_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, اندرونی_State.بفر.L);
  r0(E, A, B, C, D, 1, اندرونی_State.بفر.L);
  r0(D, E, A, B, C, 2, اندرونی_State.بفر.L);
  r0(C, D, E, A, B, 3, اندرونی_State.بفر.L);
  r0(B, C, D, E, A, 4, اندرونی_State.بفر.L);
  r0(A, B, C, D, E, 5, اندرونی_State.بفر.L);
  r0(E, A, B, C, D, 6, اندرونی_State.بفر.L);
  r0(D, E, A, B, C, 7, اندرونی_State.بفر.L);
  r0(C, D, E, A, B, 8, اندرونی_State.بفر.L);
  r0(B, C, D, E, A, 9, اندرونی_State.بفر.L);
  r0(A, B, C, D, E, 10, اندرونی_State.بفر.L);
  r0(E, A, B, C, D, 11, اندرونی_State.بفر.L);
  r0(D, E, A, B, C, 12, اندرونی_State.بفر.L);
  r0(C, D, E, A, B, 13, اندرونی_State.بفر.L);
  r0(B, C, D, E, A, 14, اندرونی_State.بفر.L);
  r0(A, B, C, D, E, 15, اندرونی_State.بفر.L);
  r1(E, A, B, C, D, 16, اندرونی_State.بفر.L);
  r1(D, E, A, B, C, 17, اندرونی_State.بفر.L);
  r1(C, D, E, A, B, 18, اندرونی_State.بفر.L);
  r1(B, C, D, E, A, 19, اندرونی_State.بفر.L);

  r2(A, B, C, D, E, 20, اندرونی_State.بفر.L);
  r2(E, A, B, C, D, 21, اندرونی_State.بفر.L);
  r2(D, E, A, B, C, 22, اندرونی_State.بفر.L);
  r2(C, D, E, A, B, 23, اندرونی_State.بفر.L);
  r2(B, C, D, E, A, 24, اندرونی_State.بفر.L);
  r2(A, B, C, D, E, 25, اندرونی_State.بفر.L);
  r2(E, A, B, C, D, 26, اندرونی_State.بفر.L);
  r2(D, E, A, B, C, 27, اندرونی_State.بفر.L);
  r2(C, D, E, A, B, 28, اندرونی_State.بفر.L);
  r2(B, C, D, E, A, 29, اندرونی_State.بفر.L);
  r2(A, B, C, D, E, 30, اندرونی_State.بفر.L);
  r2(E, A, B, C, D, 31, اندرونی_State.بفر.L);
  r2(D, E, A, B, C, 32, اندرونی_State.بفر.L);
  r2(C, D, E, A, B, 33, اندرونی_State.بفر.L);
  r2(B, C, D, E, A, 34, اندرونی_State.بفر.L);
  r2(A, B, C, D, E, 35, اندرونی_State.بفر.L);
  r2(E, A, B, C, D, 36, اندرونی_State.بفر.L);
  r2(D, E, A, B, C, 37, اندرونی_State.بفر.L);
  r2(C, D, E, A, B, 38, اندرونی_State.بفر.L);
  r2(B, C, D, E, A, 39, اندرونی_State.بفر.L);

  r3(A, B, C, D, E, 40, اندرونی_State.بفر.L);
  r3(E, A, B, C, D, 41, اندرونی_State.بفر.L);
  r3(D, E, A, B, C, 42, اندرونی_State.بفر.L);
  r3(C, D, E, A, B, 43, اندرونی_State.بفر.L);
  r3(B, C, D, E, A, 44, اندرونی_State.بفر.L);
  r3(A, B, C, D, E, 45, اندرونی_State.بفر.L);
  r3(E, A, B, C, D, 46, اندرونی_State.بفر.L);
  r3(D, E, A, B, C, 47, اندرونی_State.بفر.L);
  r3(C, D, E, A, B, 48, اندرونی_State.بفر.L);
  r3(B, C, D, E, A, 49, اندرونی_State.بفر.L);
  r3(A, B, C, D, E, 50, اندرونی_State.بفر.L);
  r3(E, A, B, C, D, 51, اندرونی_State.بفر.L);
  r3(D, E, A, B, C, 52, اندرونی_State.بفر.L);
  r3(C, D, E, A, B, 53, اندرونی_State.بفر.L);
  r3(B, C, D, E, A, 54, اندرونی_State.بفر.L);
  r3(A, B, C, D, E, 55, اندرونی_State.بفر.L);
  r3(E, A, B, C, D, 56, اندرونی_State.بفر.L);
  r3(D, E, A, B, C, 57, اندرونی_State.بفر.L);
  r3(C, D, E, A, B, 58, اندرونی_State.بفر.L);
  r3(B, C, D, E, A, 59, اندرونی_State.بفر.L);

  r4(A, B, C, D, E, 60, اندرونی_State.بفر.L);
  r4(E, A, B, C, D, 61, اندرونی_State.بفر.L);
  r4(D, E, A, B, C, 62, اندرونی_State.بفر.L);
  r4(C, D, E, A, B, 63, اندرونی_State.بفر.L);
  r4(B, C, D, E, A, 64, اندرونی_State.بفر.L);
  r4(A, B, C, D, E, 65, اندرونی_State.بفر.L);
  r4(E, A, B, C, D, 66, اندرونی_State.بفر.L);
  r4(D, E, A, B, C, 67, اندرونی_State.بفر.L);
  r4(C, D, E, A, B, 68, اندرونی_State.بفر.L);
  r4(B, C, D, E, A, 69, اندرونی_State.بفر.L);
  r4(A, B, C, D, E, 70, اندرونی_State.بفر.L);
  r4(E, A, B, C, D, 71, اندرونی_State.بفر.L);
  r4(D, E, A, B, C, 72, اندرونی_State.بفر.L);
  r4(C, D, E, A, B, 73, اندرونی_State.بفر.L);
  r4(B, C, D, E, A, 74, اندرونی_State.بفر.L);
  r4(A, B, C, D, E, 75, اندرونی_State.بفر.L);
  r4(E, A, B, C, D, 76, اندرونی_State.بفر.L);
  r4(D, E, A, B, C, 77, اندرونی_State.بفر.L);
  r4(C, D, E, A, B, 78, اندرونی_State.بفر.L);
  r4(B, C, D, E, A, 79, اندرونی_State.بفر.L);

  اندرونی_State.State[0] += A;
  اندرونی_State.State[1] += B;
  اندرونی_State.State[2] += C;
  اندرونی_State.State[3] += D;
  اندرونی_State.State[4] += E;
}

فراغ SHA1::جوڑو_Uncounted(uint8_t انکڑے_2) {
  إذا constexpr (sys::IsBigEndianHost)
    اندرونی_State.بفر.C[اندرونی_State.بفر_وتھ] = انکڑے_2;
  وإلا
    اندرونی_State.بفر.C[اندرونی_State.بفر_وتھ ^ 3] = انکڑے_2;

  اندرونی_State.بفر_وتھ++;
  إذا (اندرونی_State.بفر_وتھ == بلاک_لمائی) {
    hash_بلاک();
    اندرونی_State.بفر_وتھ = 0;
  }
}

فراغ SHA1::writebyte(uint8_t انکڑے_2) {
  ++اندرونی_State.Byte_گنتی;
  جوڑو_Uncounted(انکڑے_2);
}

فراغ SHA1::update(ArrayRef<uint8_t> انکڑے_2) {
  اندرونی_State.Byte_گنتی += انکڑے_2.ناپ_2();

  // Finish the current block.
  إذا (اندرونی_State.بفر_وتھ > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        انکڑے_2.ناپ_2(), بلاک_لمائی - اندرونی_State.بفر_وتھ);
    لكل (size_t I = 0; I < Remainder; ++I)
      جوڑو_Uncounted(انکڑے_2[I]);
    انکڑے_2 = انکڑے_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  طالما (انکڑے_2.ناپ_2() >= بلاک_لمائی) {
    أكد(اندرونی_State.بفر_وتھ == 0);
    static_assert(بلاک_لمائی % 4 == 0);
    constexpr size_t بلاک_لمائی_32 = بلاک_لمائی / 4;
    لكل (size_t I = 0; I < بلاک_لمائی_32; ++I)
      اندرونی_State.بفر.L[I] = support::endian::read32be(&انکڑے_2[I * 4]);
    hash_بلاک();
    انکڑے_2 = انکڑے_2.drop_front(بلاک_لمائی);
  }

  // Finish the remainder.
  لكل (uint8_t C : انکڑے_2)
    جوڑو_Uncounted(C);
}

فراغ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.انکڑے()), Str.ناپ_2()));
}

فراغ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جوڑو_Uncounted(0x80);
  طالما (اندرونی_State.بفر_وتھ != 56)
    جوڑو_Uncounted(0x00);

  // Append length in the last 8 bytes
  جوڑو_Uncounted(0); // We're only using 32 bit lengths
  جوڑو_Uncounted(0); // But SHA-1 supports 64 bit lengths
  جوڑو_Uncounted(0); // So zero pad the top bits
  جوڑو_Uncounted(اندرونی_State.Byte_گنتی >> 29); // Shifting to multiply by 8
  جوڑو_Uncounted(اندرونی_State.Byte_گنتی >>
               21); // as SHA-1 supports bitstreams as well as
  جوڑو_Uncounted(اندرونی_State.Byte_گنتی >> 13); // byte.
  جوڑو_Uncounted(اندرونی_State.Byte_گنتی >> 5);
  جوڑو_Uncounted(اندرونی_State.Byte_گنتی << 3);
}

فراغ SHA1::final(std::array<uint32_t, HASH_لمائی / 4> &Hash_نتیجہ) {
  // Pad to complete the last block
  pad();

  إذا constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتیجہ[i] = اندرونی_State.State[i];
    }
  } وإلا {
    // Swap byte order back
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتیجہ[i] = llvm::byteswap(اندرونی_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  اتحاد {
    std::array<uint32_t, HASH_لمائی / 4> Hash_نتیجہ;
    std::array<uint8_t, HASH_لمائی> Return_نتیجہ;
  };
  static_assert(حجم(Hash_نتیجہ) == حجم(Return_نتیجہ));
  final(Hash_نتیجہ);
  أرجع Return_نتیجہ;
}

std::array<uint8_t, 20> SHA1::نتیجہ_2() {
  خودکار State_منزل_Restore = اندرونی_State;

  خودکار Hash = final();

  // Restore the state
  اندرونی_State = State_منزل_Restore;

  // Return pointer to hash (20 characters)
  أرجع Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> انکڑے_2) {
  SHA1 Hash;
  Hash.update(انکڑے_2);
  أرجع Hash.final();
}
