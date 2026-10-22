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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SHA1.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SwapByteOrder.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t rol(uint32_t ਸੰਖਿਆ, ਪੂਰਨ ਬਿੱਟ_2) {
  ਵਾਪਸ (ਸੰਖਿਆ << ਬਿੱਟ_2) | (ਸੰਖਿਆ >> (32 - ਬਿੱਟ_2));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t blk0(uint32_t *Buf, ਪੂਰਨ I) { ਵਾਪਸ Buf[I]; }

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t blk(uint32_t *Buf, ਪੂਰਨ I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  ਵਾਪਸ Buf[I & 15];
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ਪੂਰਨ I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ਪੂਰਨ I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ਪੂਰਨ I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ਪੂਰਨ I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, ਪੂਰਨ I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/*ਕੋਡ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA1_K0 0x5a827999
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA1_K20 0x6ed9eba1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA1_K40 0x8f1bbcdc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA1_K60 0xca62c1d6

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬੀਜ_0 0x67452301
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬੀਜ_1 0xefcdab89
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬੀਜ_2_2 0x98badcfe
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬੀਜ_3 0x10325476
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬੀਜ_4 0xc3d2e1f0

ਖਾਲੀ SHA1::init() {
  ਅੰਦਰੂਨੀ_State.State[0] = ਬੀਜ_0;
  ਅੰਦਰੂਨੀ_State.State[1] = ਬੀਜ_1;
  ਅੰਦਰੂਨੀ_State.State[2] = ਬੀਜ_2_2;
  ਅੰਦਰੂਨੀ_State.State[3] = ਬੀਜ_3;
  ਅੰਦਰੂਨੀ_State.State[4] = ਬੀਜ_4;
  ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ = 0;
  ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ = 0;
}

ਖਾਲੀ SHA1::hash_ਖੰਡ() {
  uint32_t A = ਅੰਦਰੂਨੀ_State.State[0];
  uint32_t B = ਅੰਦਰੂਨੀ_State.State[1];
  uint32_t C = ਅੰਦਰੂਨੀ_State.State[2];
  uint32_t D = ਅੰਦਰੂਨੀ_State.State[3];
  uint32_t E = ਅੰਦਰੂਨੀ_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(E, A, B, C, D, 1, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(D, E, A, B, C, 2, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(C, D, E, A, B, 3, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(B, C, D, E, A, 4, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(A, B, C, D, E, 5, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(E, A, B, C, D, 6, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(D, E, A, B, C, 7, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(C, D, E, A, B, 8, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(B, C, D, E, A, 9, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(A, B, C, D, E, 10, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(E, A, B, C, D, 11, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(D, E, A, B, C, 12, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(C, D, E, A, B, 13, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(B, C, D, E, A, 14, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r0(A, B, C, D, E, 15, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r1(E, A, B, C, D, 16, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r1(D, E, A, B, C, 17, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r1(C, D, E, A, B, 18, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r1(B, C, D, E, A, 19, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);

  r2(A, B, C, D, E, 20, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(E, A, B, C, D, 21, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(D, E, A, B, C, 22, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(C, D, E, A, B, 23, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(B, C, D, E, A, 24, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(A, B, C, D, E, 25, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(E, A, B, C, D, 26, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(D, E, A, B, C, 27, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(C, D, E, A, B, 28, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(B, C, D, E, A, 29, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(A, B, C, D, E, 30, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(E, A, B, C, D, 31, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(D, E, A, B, C, 32, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(C, D, E, A, B, 33, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(B, C, D, E, A, 34, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(A, B, C, D, E, 35, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(E, A, B, C, D, 36, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(D, E, A, B, C, 37, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(C, D, E, A, B, 38, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r2(B, C, D, E, A, 39, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);

  r3(A, B, C, D, E, 40, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(E, A, B, C, D, 41, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(D, E, A, B, C, 42, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(C, D, E, A, B, 43, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(B, C, D, E, A, 44, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(A, B, C, D, E, 45, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(E, A, B, C, D, 46, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(D, E, A, B, C, 47, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(C, D, E, A, B, 48, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(B, C, D, E, A, 49, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(A, B, C, D, E, 50, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(E, A, B, C, D, 51, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(D, E, A, B, C, 52, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(C, D, E, A, B, 53, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(B, C, D, E, A, 54, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(A, B, C, D, E, 55, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(E, A, B, C, D, 56, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(D, E, A, B, C, 57, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(C, D, E, A, B, 58, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r3(B, C, D, E, A, 59, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);

  r4(A, B, C, D, E, 60, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(E, A, B, C, D, 61, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(D, E, A, B, C, 62, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(C, D, E, A, B, 63, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(B, C, D, E, A, 64, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(A, B, C, D, E, 65, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(E, A, B, C, D, 66, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(D, E, A, B, C, 67, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(C, D, E, A, B, 68, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(B, C, D, E, A, 69, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(A, B, C, D, E, 70, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(E, A, B, C, D, 71, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(D, E, A, B, C, 72, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(C, D, E, A, B, 73, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(B, C, D, E, A, 74, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(A, B, C, D, E, 75, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(E, A, B, C, D, 76, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(D, E, A, B, C, 77, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(C, D, E, A, B, 78, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);
  r4(B, C, D, E, A, 79, ਅੰਦਰੂਨੀ_State.ਬਫਰ.L);

  ਅੰਦਰੂਨੀ_State.State[0] += A;
  ਅੰਦਰੂਨੀ_State.State[1] += B;
  ਅੰਦਰੂਨੀ_State.State[2] += C;
  ਅੰਦਰੂਨੀ_State.State[3] += D;
  ਅੰਦਰੂਨੀ_State.State[4] += E;
}

ਖਾਲੀ SHA1::ਜੋੜੋ_Uncounted(uint8_t ਅੰਕੜੇ_2) {
  ਜੇ constexpr (sys::IsBigEndianHost)
    ਅੰਦਰੂਨੀ_State.ਬਫਰ.C[ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ] = ਅੰਕੜੇ_2;
  ਨਹੀਂਤਾਂ
    ਅੰਦਰੂਨੀ_State.ਬਫਰ.C[ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ ^ 3] = ਅੰਕੜੇ_2;

  ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ++;
  ਜੇ (ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ == ਖੰਡ_ਲੰਮਾਈ) {
    hash_ਖੰਡ();
    ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ = 0;
  }
}

ਖਾਲੀ SHA1::writebyte(uint8_t ਅੰਕੜੇ_2) {
  ++ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ;
  ਜੋੜੋ_Uncounted(ਅੰਕੜੇ_2);
}

ਖਾਲੀ SHA1::update(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ += ਅੰਕੜੇ_2.ਮਾਪ_2();

  // Finish the current block.
  ਜੇ (ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ > 0) {
    ਅਚਲ size_t Remainder = std::min<size_t>(
        ਅੰਕੜੇ_2.ਮਾਪ_2(), ਖੰਡ_ਲੰਮਾਈ - ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ);
    ਲਈ (size_t I = 0; I < Remainder; ++I)
      ਜੋੜੋ_Uncounted(ਅੰਕੜੇ_2[I]);
    ਅੰਕੜੇ_2 = ਅੰਕੜੇ_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  ਜਦੋਂ (ਅੰਕੜੇ_2.ਮਾਪ_2() >= ਖੰਡ_ਲੰਮਾਈ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ == 0);
    static_assert(ਖੰਡ_ਲੰਮਾਈ % 4 == 0);
    constexpr size_t ਖੰਡ_ਲੰਮਾਈ_32 = ਖੰਡ_ਲੰਮਾਈ / 4;
    ਲਈ (size_t I = 0; I < ਖੰਡ_ਲੰਮਾਈ_32; ++I)
      ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[I] = support::endian::read32be(&ਅੰਕੜੇ_2[I * 4]);
    hash_ਖੰਡ();
    ਅੰਕੜੇ_2 = ਅੰਕੜੇ_2.drop_front(ਖੰਡ_ਲੰਮਾਈ);
  }

  // Finish the remainder.
  ਲਈ (uint8_t C : ਅੰਕੜੇ_2)
    ਜੋੜੋ_Uncounted(C);
}

ਖਾਲੀ SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<ਅੱਖਰ *>(Str.ਅੰਕੜੇ()), Str.ਮਾਪ_2()));
}

ਖਾਲੀ SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  ਜੋੜੋ_Uncounted(0x80);
  ਜਦੋਂ (ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ != 56)
    ਜੋੜੋ_Uncounted(0x00);

  // Append length in the last 8 bytes
  ਜੋੜੋ_Uncounted(0); // We're only using 32 bit lengths
  ਜੋੜੋ_Uncounted(0); // But SHA-1 supports 64 bit lengths
  ਜੋੜੋ_Uncounted(0); // So zero pad the top bits
  ਜੋੜੋ_Uncounted(ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ >> 29); // Shifting to multiply by 8
  ਜੋੜੋ_Uncounted(ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ >>
               21); // as SHA-1 supports bitstreams as well as
  ਜੋੜੋ_Uncounted(ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ >> 13); // byte.
  ਜੋੜੋ_Uncounted(ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ >> 5);
  ਜੋੜੋ_Uncounted(ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ << 3);
}

ਖਾਲੀ SHA1::final(std::array<uint32_t, HASH_ਲੰਮਾਈ / 4> &Hash_ਨਤੀਜਾ) {
  // Pad to complete the last block
  pad();

  ਜੇ constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    ਲਈ (ਪੂਰਨ i = 0; i < 5; i++) {
      Hash_ਨਤੀਜਾ[i] = ਅੰਦਰੂਨੀ_State.State[i];
    }
  } ਨਹੀਂਤਾਂ {
    // Swap byte order back
    ਲਈ (ਪੂਰਨ i = 0; i < 5; i++) {
      Hash_ਨਤੀਜਾ[i] = llvm::byteswap(ਅੰਦਰੂਨੀ_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  ਸੰਘ {
    std::array<uint32_t, HASH_ਲੰਮਾਈ / 4> Hash_ਨਤੀਜਾ;
    std::array<uint8_t, HASH_ਲੰਮਾਈ> Return_ਨਤੀਜਾ;
  };
  static_assert(ਆਕਾਰ(Hash_ਨਤੀਜਾ) == ਆਕਾਰ(Return_ਨਤੀਜਾ));
  final(Hash_ਨਤੀਜਾ);
  ਵਾਪਸ Return_ਨਤੀਜਾ;
}

std::array<uint8_t, 20> SHA1::ਨਤੀਜਾ_2() {
  ਸਵੈਚਾਲਿਤ State_ਮੰਜ਼ਿਲ_Restore = ਅੰਦਰੂਨੀ_State;

  ਸਵੈਚਾਲਿਤ Hash = final();

  // Restore the state
  ਅੰਦਰੂਨੀ_State = State_ਮੰਜ਼ਿਲ_Restore;

  // Return pointer to hash (20 characters)
  ਵਾਪਸ Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  SHA1 Hash;
  Hash.update(ਅੰਕੜੇ_2);
  ਵਾਪਸ Hash.final();
}
