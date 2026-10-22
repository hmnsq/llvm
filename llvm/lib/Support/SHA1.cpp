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

استفاده نطاق llvm;

ساكن مضمن uint32_t rol(uint32_t عدد, صحيح بیتها_2) {
  أرجع (عدد << بیتها_2) | (عدد >> (32 - بیتها_2));
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

/*کد*/
#تعریف_کن SHA1_K0 0x5a827999
#تعریف_کن SHA1_K20 0x6ed9eba1
#تعریف_کن SHA1_K40 0x8f1bbcdc
#تعریف_کن SHA1_K60 0xca62c1d6

#تعریف_کن دانه_0 0x67452301
#تعریف_کن دانه_1 0xefcdab89
#تعریف_کن دانه_2_2 0x98badcfe
#تعریف_کن دانه_3 0x10325476
#تعریف_کن دانه_4 0xc3d2e1f0

فراغ SHA1::init() {
  داخلی_State.State[0] = دانه_0;
  داخلی_State.State[1] = دانه_1;
  داخلی_State.State[2] = دانه_2_2;
  داخلی_State.State[3] = دانه_3;
  داخلی_State.State[4] = دانه_4;
  داخلی_State.Byte_شمار = 0;
  داخلی_State.میانگیر_جابه_جایی = 0;
}

فراغ SHA1::hash_بلوک() {
  uint32_t A = داخلی_State.State[0];
  uint32_t B = داخلی_State.State[1];
  uint32_t C = داخلی_State.State[2];
  uint32_t D = داخلی_State.State[3];
  uint32_t E = داخلی_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, داخلی_State.میانگیر.L);
  r0(E, A, B, C, D, 1, داخلی_State.میانگیر.L);
  r0(D, E, A, B, C, 2, داخلی_State.میانگیر.L);
  r0(C, D, E, A, B, 3, داخلی_State.میانگیر.L);
  r0(B, C, D, E, A, 4, داخلی_State.میانگیر.L);
  r0(A, B, C, D, E, 5, داخلی_State.میانگیر.L);
  r0(E, A, B, C, D, 6, داخلی_State.میانگیر.L);
  r0(D, E, A, B, C, 7, داخلی_State.میانگیر.L);
  r0(C, D, E, A, B, 8, داخلی_State.میانگیر.L);
  r0(B, C, D, E, A, 9, داخلی_State.میانگیر.L);
  r0(A, B, C, D, E, 10, داخلی_State.میانگیر.L);
  r0(E, A, B, C, D, 11, داخلی_State.میانگیر.L);
  r0(D, E, A, B, C, 12, داخلی_State.میانگیر.L);
  r0(C, D, E, A, B, 13, داخلی_State.میانگیر.L);
  r0(B, C, D, E, A, 14, داخلی_State.میانگیر.L);
  r0(A, B, C, D, E, 15, داخلی_State.میانگیر.L);
  r1(E, A, B, C, D, 16, داخلی_State.میانگیر.L);
  r1(D, E, A, B, C, 17, داخلی_State.میانگیر.L);
  r1(C, D, E, A, B, 18, داخلی_State.میانگیر.L);
  r1(B, C, D, E, A, 19, داخلی_State.میانگیر.L);

  r2(A, B, C, D, E, 20, داخلی_State.میانگیر.L);
  r2(E, A, B, C, D, 21, داخلی_State.میانگیر.L);
  r2(D, E, A, B, C, 22, داخلی_State.میانگیر.L);
  r2(C, D, E, A, B, 23, داخلی_State.میانگیر.L);
  r2(B, C, D, E, A, 24, داخلی_State.میانگیر.L);
  r2(A, B, C, D, E, 25, داخلی_State.میانگیر.L);
  r2(E, A, B, C, D, 26, داخلی_State.میانگیر.L);
  r2(D, E, A, B, C, 27, داخلی_State.میانگیر.L);
  r2(C, D, E, A, B, 28, داخلی_State.میانگیر.L);
  r2(B, C, D, E, A, 29, داخلی_State.میانگیر.L);
  r2(A, B, C, D, E, 30, داخلی_State.میانگیر.L);
  r2(E, A, B, C, D, 31, داخلی_State.میانگیر.L);
  r2(D, E, A, B, C, 32, داخلی_State.میانگیر.L);
  r2(C, D, E, A, B, 33, داخلی_State.میانگیر.L);
  r2(B, C, D, E, A, 34, داخلی_State.میانگیر.L);
  r2(A, B, C, D, E, 35, داخلی_State.میانگیر.L);
  r2(E, A, B, C, D, 36, داخلی_State.میانگیر.L);
  r2(D, E, A, B, C, 37, داخلی_State.میانگیر.L);
  r2(C, D, E, A, B, 38, داخلی_State.میانگیر.L);
  r2(B, C, D, E, A, 39, داخلی_State.میانگیر.L);

  r3(A, B, C, D, E, 40, داخلی_State.میانگیر.L);
  r3(E, A, B, C, D, 41, داخلی_State.میانگیر.L);
  r3(D, E, A, B, C, 42, داخلی_State.میانگیر.L);
  r3(C, D, E, A, B, 43, داخلی_State.میانگیر.L);
  r3(B, C, D, E, A, 44, داخلی_State.میانگیر.L);
  r3(A, B, C, D, E, 45, داخلی_State.میانگیر.L);
  r3(E, A, B, C, D, 46, داخلی_State.میانگیر.L);
  r3(D, E, A, B, C, 47, داخلی_State.میانگیر.L);
  r3(C, D, E, A, B, 48, داخلی_State.میانگیر.L);
  r3(B, C, D, E, A, 49, داخلی_State.میانگیر.L);
  r3(A, B, C, D, E, 50, داخلی_State.میانگیر.L);
  r3(E, A, B, C, D, 51, داخلی_State.میانگیر.L);
  r3(D, E, A, B, C, 52, داخلی_State.میانگیر.L);
  r3(C, D, E, A, B, 53, داخلی_State.میانگیر.L);
  r3(B, C, D, E, A, 54, داخلی_State.میانگیر.L);
  r3(A, B, C, D, E, 55, داخلی_State.میانگیر.L);
  r3(E, A, B, C, D, 56, داخلی_State.میانگیر.L);
  r3(D, E, A, B, C, 57, داخلی_State.میانگیر.L);
  r3(C, D, E, A, B, 58, داخلی_State.میانگیر.L);
  r3(B, C, D, E, A, 59, داخلی_State.میانگیر.L);

  r4(A, B, C, D, E, 60, داخلی_State.میانگیر.L);
  r4(E, A, B, C, D, 61, داخلی_State.میانگیر.L);
  r4(D, E, A, B, C, 62, داخلی_State.میانگیر.L);
  r4(C, D, E, A, B, 63, داخلی_State.میانگیر.L);
  r4(B, C, D, E, A, 64, داخلی_State.میانگیر.L);
  r4(A, B, C, D, E, 65, داخلی_State.میانگیر.L);
  r4(E, A, B, C, D, 66, داخلی_State.میانگیر.L);
  r4(D, E, A, B, C, 67, داخلی_State.میانگیر.L);
  r4(C, D, E, A, B, 68, داخلی_State.میانگیر.L);
  r4(B, C, D, E, A, 69, داخلی_State.میانگیر.L);
  r4(A, B, C, D, E, 70, داخلی_State.میانگیر.L);
  r4(E, A, B, C, D, 71, داخلی_State.میانگیر.L);
  r4(D, E, A, B, C, 72, داخلی_State.میانگیر.L);
  r4(C, D, E, A, B, 73, داخلی_State.میانگیر.L);
  r4(B, C, D, E, A, 74, داخلی_State.میانگیر.L);
  r4(A, B, C, D, E, 75, داخلی_State.میانگیر.L);
  r4(E, A, B, C, D, 76, داخلی_State.میانگیر.L);
  r4(D, E, A, B, C, 77, داخلی_State.میانگیر.L);
  r4(C, D, E, A, B, 78, داخلی_State.میانگیر.L);
  r4(B, C, D, E, A, 79, داخلی_State.میانگیر.L);

  داخلی_State.State[0] += A;
  داخلی_State.State[1] += B;
  داخلی_State.State[2] += C;
  داخلی_State.State[3] += D;
  داخلی_State.State[4] += E;
}

فراغ SHA1::جمع_کن_Uncounted(uint8_t داده_2) {
  إذا constexpr (sys::IsBigEndianHost)
    داخلی_State.میانگیر.C[داخلی_State.میانگیر_جابه_جایی] = داده_2;
  وإلا
    داخلی_State.میانگیر.C[داخلی_State.میانگیر_جابه_جایی ^ 3] = داده_2;

  داخلی_State.میانگیر_جابه_جایی++;
  إذا (داخلی_State.میانگیر_جابه_جایی == بلوک_درازی) {
    hash_بلوک();
    داخلی_State.میانگیر_جابه_جایی = 0;
  }
}

فراغ SHA1::writebyte(uint8_t داده_2) {
  ++داخلی_State.Byte_شمار;
  جمع_کن_Uncounted(داده_2);
}

فراغ SHA1::update(ArrayRef<uint8_t> داده_2) {
  داخلی_State.Byte_شمار += داده_2.اندازه_2();

  // Finish the current block.
  إذا (داخلی_State.میانگیر_جابه_جایی > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        داده_2.اندازه_2(), بلوک_درازی - داخلی_State.میانگیر_جابه_جایی);
    لكل (size_t I = 0; I < Remainder; ++I)
      جمع_کن_Uncounted(داده_2[I]);
    داده_2 = داده_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  طالما (داده_2.اندازه_2() >= بلوک_درازی) {
    أكد(داخلی_State.میانگیر_جابه_جایی == 0);
    static_assert(بلوک_درازی % 4 == 0);
    constexpr size_t بلوک_درازی_32 = بلوک_درازی / 4;
    لكل (size_t I = 0; I < بلوک_درازی_32; ++I)
      داخلی_State.میانگیر.L[I] = support::endian::read32be(&داده_2[I * 4]);
    hash_بلوک();
    داده_2 = داده_2.drop_front(بلوک_درازی);
  }

  // Finish the remainder.
  لكل (uint8_t C : داده_2)
    جمع_کن_Uncounted(C);
}

فراغ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.داده()), Str.اندازه_2()));
}

فراغ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جمع_کن_Uncounted(0x80);
  طالما (داخلی_State.میانگیر_جابه_جایی != 56)
    جمع_کن_Uncounted(0x00);

  // Append length in the last 8 bytes
  جمع_کن_Uncounted(0); // We're only using 32 bit lengths
  جمع_کن_Uncounted(0); // But SHA-1 supports 64 bit lengths
  جمع_کن_Uncounted(0); // So zero pad the top bits
  جمع_کن_Uncounted(داخلی_State.Byte_شمار >> 29); // Shifting to multiply by 8
  جمع_کن_Uncounted(داخلی_State.Byte_شمار >>
               21); // as SHA-1 supports bitstreams as well as
  جمع_کن_Uncounted(داخلی_State.Byte_شمار >> 13); // byte.
  جمع_کن_Uncounted(داخلی_State.Byte_شمار >> 5);
  جمع_کن_Uncounted(داخلی_State.Byte_شمار << 3);
}

فراغ SHA1::final(std::array<uint32_t, HASH_درازی / 4> &Hash_نتیجه) {
  // Pad to complete the last block
  pad();

  إذا constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتیجه[i] = داخلی_State.State[i];
    }
  } وإلا {
    // Swap byte order back
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_نتیجه[i] = llvm::byteswap(داخلی_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  اتحاد {
    std::array<uint32_t, HASH_درازی / 4> Hash_نتیجه;
    std::array<uint8_t, HASH_درازی> Return_نتیجه;
  };
  static_assert(حجم(Hash_نتیجه) == حجم(Return_نتیجه));
  final(Hash_نتیجه);
  أرجع Return_نتیجه;
}

std::array<uint8_t, 20> SHA1::نتیجه_2() {
  خودکار State_مقصد_Restore = داخلی_State;

  خودکار Hash = final();

  // Restore the state
  داخلی_State = State_مقصد_Restore;

  // Return pointer to hash (20 characters)
  أرجع Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> داده_2) {
  SHA1 Hash;
  Hash.update(داده_2);
  أرجع Hash.final();
}
