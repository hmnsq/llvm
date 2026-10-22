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

کارول نطاق llvm;

ساكن مضمن uint32_t rol(uint32_t شمېره, صحيح بټونه_2) {
  أرجع (شمېره << بټونه_2) | (شمېره >> (32 - بټونه_2));
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

/*کوډ*/
#تعریف_کړه SHA1_K0 0x5a827999
#تعریف_کړه SHA1_K20 0x6ed9eba1
#تعریف_کړه SHA1_K40 0x8f1bbcdc
#تعریف_کړه SHA1_K60 0xca62c1d6

#تعریف_کړه تخم_0 0x67452301
#تعریف_کړه تخم_1 0xefcdab89
#تعریف_کړه تخم_2_2 0x98badcfe
#تعریف_کړه تخم_3 0x10325476
#تعریف_کړه تخم_4 0xc3d2e1f0

فراغ SHA1::init() {
  دننی_State.State[0] = تخم_0;
  دننی_State.State[1] = تخم_1;
  دننی_State.State[2] = تخم_2_2;
  دننی_State.State[3] = تخم_3;
  دننی_State.State[4] = تخم_4;
  دننی_State.Byte_شمېر = 0;
  دننی_State.بفر_دننه_واټن = 0;
}

فراغ SHA1::hash_بلاک() {
  uint32_t A = دننی_State.State[0];
  uint32_t B = دننی_State.State[1];
  uint32_t C = دننی_State.State[2];
  uint32_t D = دننی_State.State[3];
  uint32_t E = دننی_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, دننی_State.بفر.L);
  r0(E, A, B, C, D, 1, دننی_State.بفر.L);
  r0(D, E, A, B, C, 2, دننی_State.بفر.L);
  r0(C, D, E, A, B, 3, دننی_State.بفر.L);
  r0(B, C, D, E, A, 4, دننی_State.بفر.L);
  r0(A, B, C, D, E, 5, دننی_State.بفر.L);
  r0(E, A, B, C, D, 6, دننی_State.بفر.L);
  r0(D, E, A, B, C, 7, دننی_State.بفر.L);
  r0(C, D, E, A, B, 8, دننی_State.بفر.L);
  r0(B, C, D, E, A, 9, دننی_State.بفر.L);
  r0(A, B, C, D, E, 10, دننی_State.بفر.L);
  r0(E, A, B, C, D, 11, دننی_State.بفر.L);
  r0(D, E, A, B, C, 12, دننی_State.بفر.L);
  r0(C, D, E, A, B, 13, دننی_State.بفر.L);
  r0(B, C, D, E, A, 14, دننی_State.بفر.L);
  r0(A, B, C, D, E, 15, دننی_State.بفر.L);
  r1(E, A, B, C, D, 16, دننی_State.بفر.L);
  r1(D, E, A, B, C, 17, دننی_State.بفر.L);
  r1(C, D, E, A, B, 18, دننی_State.بفر.L);
  r1(B, C, D, E, A, 19, دننی_State.بفر.L);

  r2(A, B, C, D, E, 20, دننی_State.بفر.L);
  r2(E, A, B, C, D, 21, دننی_State.بفر.L);
  r2(D, E, A, B, C, 22, دننی_State.بفر.L);
  r2(C, D, E, A, B, 23, دننی_State.بفر.L);
  r2(B, C, D, E, A, 24, دننی_State.بفر.L);
  r2(A, B, C, D, E, 25, دننی_State.بفر.L);
  r2(E, A, B, C, D, 26, دننی_State.بفر.L);
  r2(D, E, A, B, C, 27, دننی_State.بفر.L);
  r2(C, D, E, A, B, 28, دننی_State.بفر.L);
  r2(B, C, D, E, A, 29, دننی_State.بفر.L);
  r2(A, B, C, D, E, 30, دننی_State.بفر.L);
  r2(E, A, B, C, D, 31, دننی_State.بفر.L);
  r2(D, E, A, B, C, 32, دننی_State.بفر.L);
  r2(C, D, E, A, B, 33, دننی_State.بفر.L);
  r2(B, C, D, E, A, 34, دننی_State.بفر.L);
  r2(A, B, C, D, E, 35, دننی_State.بفر.L);
  r2(E, A, B, C, D, 36, دننی_State.بفر.L);
  r2(D, E, A, B, C, 37, دننی_State.بفر.L);
  r2(C, D, E, A, B, 38, دننی_State.بفر.L);
  r2(B, C, D, E, A, 39, دننی_State.بفر.L);

  r3(A, B, C, D, E, 40, دننی_State.بفر.L);
  r3(E, A, B, C, D, 41, دننی_State.بفر.L);
  r3(D, E, A, B, C, 42, دننی_State.بفر.L);
  r3(C, D, E, A, B, 43, دننی_State.بفر.L);
  r3(B, C, D, E, A, 44, دننی_State.بفر.L);
  r3(A, B, C, D, E, 45, دننی_State.بفر.L);
  r3(E, A, B, C, D, 46, دننی_State.بفر.L);
  r3(D, E, A, B, C, 47, دننی_State.بفر.L);
  r3(C, D, E, A, B, 48, دننی_State.بفر.L);
  r3(B, C, D, E, A, 49, دننی_State.بفر.L);
  r3(A, B, C, D, E, 50, دننی_State.بفر.L);
  r3(E, A, B, C, D, 51, دننی_State.بفر.L);
  r3(D, E, A, B, C, 52, دننی_State.بفر.L);
  r3(C, D, E, A, B, 53, دننی_State.بفر.L);
  r3(B, C, D, E, A, 54, دننی_State.بفر.L);
  r3(A, B, C, D, E, 55, دننی_State.بفر.L);
  r3(E, A, B, C, D, 56, دننی_State.بفر.L);
  r3(D, E, A, B, C, 57, دننی_State.بفر.L);
  r3(C, D, E, A, B, 58, دننی_State.بفر.L);
  r3(B, C, D, E, A, 59, دننی_State.بفر.L);

  r4(A, B, C, D, E, 60, دننی_State.بفر.L);
  r4(E, A, B, C, D, 61, دننی_State.بفر.L);
  r4(D, E, A, B, C, 62, دننی_State.بفر.L);
  r4(C, D, E, A, B, 63, دننی_State.بفر.L);
  r4(B, C, D, E, A, 64, دننی_State.بفر.L);
  r4(A, B, C, D, E, 65, دننی_State.بفر.L);
  r4(E, A, B, C, D, 66, دننی_State.بفر.L);
  r4(D, E, A, B, C, 67, دننی_State.بفر.L);
  r4(C, D, E, A, B, 68, دننی_State.بفر.L);
  r4(B, C, D, E, A, 69, دننی_State.بفر.L);
  r4(A, B, C, D, E, 70, دننی_State.بفر.L);
  r4(E, A, B, C, D, 71, دننی_State.بفر.L);
  r4(D, E, A, B, C, 72, دننی_State.بفر.L);
  r4(C, D, E, A, B, 73, دننی_State.بفر.L);
  r4(B, C, D, E, A, 74, دننی_State.بفر.L);
  r4(A, B, C, D, E, 75, دننی_State.بفر.L);
  r4(E, A, B, C, D, 76, دننی_State.بفر.L);
  r4(D, E, A, B, C, 77, دننی_State.بفر.L);
  r4(C, D, E, A, B, 78, دننی_State.بفر.L);
  r4(B, C, D, E, A, 79, دننی_State.بفر.L);

  دننی_State.State[0] += A;
  دننی_State.State[1] += B;
  دننی_State.State[2] += C;
  دننی_State.State[3] += D;
  دننی_State.State[4] += E;
}

فراغ SHA1::جمع_کړه_Uncounted(uint8_t معلومات_2) {
  إذا constexpr (sys::IsBigEndianHost)
    دننی_State.بفر.C[دننی_State.بفر_دننه_واټن] = معلومات_2;
  وإلا
    دننی_State.بفر.C[دننی_State.بفر_دننه_واټن ^ 3] = معلومات_2;

  دننی_State.بفر_دننه_واټن++;
  إذا (دننی_State.بفر_دننه_واټن == بلاک_اوږدتیا) {
    hash_بلاک();
    دننی_State.بفر_دننه_واټن = 0;
  }
}

فراغ SHA1::writebyte(uint8_t معلومات_2) {
  ++دننی_State.Byte_شمېر;
  جمع_کړه_Uncounted(معلومات_2);
}

فراغ SHA1::update(ArrayRef<uint8_t> معلومات_2) {
  دننی_State.Byte_شمېر += معلومات_2.اندازه_2();

  // Finish the current block.
  إذا (دننی_State.بفر_دننه_واټن > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        معلومات_2.اندازه_2(), بلاک_اوږدتیا - دننی_State.بفر_دننه_واټن);
    لكل (size_t I = 0; I < Remainder; ++I)
      جمع_کړه_Uncounted(معلومات_2[I]);
    معلومات_2 = معلومات_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  طالما (معلومات_2.اندازه_2() >= بلاک_اوږدتیا) {
    أكد(دننی_State.بفر_دننه_واټن == 0);
    static_assert(بلاک_اوږدتیا % 4 == 0);
    constexpr size_t بلاک_اوږدتیا_32 = بلاک_اوږدتیا / 4;
    لكل (size_t I = 0; I < بلاک_اوږدتیا_32; ++I)
      دننی_State.بفر.L[I] = support::endian::read32be(&معلومات_2[I * 4]);
    hash_بلاک();
    معلومات_2 = معلومات_2.drop_front(بلاک_اوږدتیا);
  }

  // Finish the remainder.
  لكل (uint8_t C : معلومات_2)
    جمع_کړه_Uncounted(C);
}

فراغ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.معلومات()), Str.اندازه_2()));
}

فراغ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جمع_کړه_Uncounted(0x80);
  طالما (دننی_State.بفر_دننه_واټن != 56)
    جمع_کړه_Uncounted(0x00);

  // Append length in the last 8 bytes
  جمع_کړه_Uncounted(0); // We're only using 32 bit lengths
  جمع_کړه_Uncounted(0); // But SHA-1 supports 64 bit lengths
  جمع_کړه_Uncounted(0); // So zero pad the top bits
  جمع_کړه_Uncounted(دننی_State.Byte_شمېر >> 29); // Shifting to multiply by 8
  جمع_کړه_Uncounted(دننی_State.Byte_شمېر >>
               21); // as SHA-1 supports bitstreams as well as
  جمع_کړه_Uncounted(دننی_State.Byte_شمېر >> 13); // byte.
  جمع_کړه_Uncounted(دننی_State.Byte_شمېر >> 5);
  جمع_کړه_Uncounted(دننی_State.Byte_شمېر << 3);
}

فراغ SHA1::final(std::array<uint32_t, HASH_اوږدتیا / 4> &Hash_پایله) {
  // Pad to complete the last block
  pad();

  إذا constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_پایله[i] = دننی_State.State[i];
    }
  } وإلا {
    // Swap byte order back
    لكل (صحيح i = 0; i < 5; i++) {
      Hash_پایله[i] = llvm::byteswap(دننی_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  اتحاد {
    std::array<uint32_t, HASH_اوږدتیا / 4> Hash_پایله;
    std::array<uint8_t, HASH_اوږدتیا> Return_پایله;
  };
  static_assert(حجم(Hash_پایله) == حجم(Return_پایله));
  final(Hash_پایله);
  أرجع Return_پایله;
}

std::array<uint8_t, 20> SHA1::پایله_2() {
  خپلکاری State_منزل_Restore = دننی_State;

  خپلکاری Hash = final();

  // Restore the state
  دننی_State = State_منزل_Restore;

  // Return pointer to hash (20 characters)
  أرجع Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> معلومات_2) {
  SHA1 Hash;
  Hash.update(معلومات_2);
  أرجع Hash.final();
}
