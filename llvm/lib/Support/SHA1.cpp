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

#అన్తర్భావ "llvm/Support/SHA1.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Endian.h"
#అన్తర్భావ "llvm/Support/SwapByteOrder.h"
#అన్తర్భావ <string.h>

ఉపయోగించి నామాకాశ llvm;

స్థిర అంతఃవరుస uint32_t rol(uint32_t సంఖ్య, పూర్ణాంకం బిట్లు_2) {
  తిరిగి (సంఖ్య << బిట్లు_2) | (సంఖ్య >> (32 - బిట్లు_2));
}

స్థిర అంతఃవరుస uint32_t blk0(uint32_t *Buf, పూర్ణాంకం I) { తిరిగి Buf[I]; }

స్థిర అంతఃవరుస uint32_t blk(uint32_t *Buf, పూర్ణాంకం I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  తిరిగి Buf[I & 15];
}

స్థిర అంతఃవరుస శూన్య r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, పూర్ణాంకం I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

స్థిర అంతఃవరుస శూన్య r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, పూర్ణాంకం I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

స్థిర అంతఃవరుస శూన్య r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, పూర్ణాంకం I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

స్థిర అంతఃవరుస శూన్య r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, పూర్ణాంకం I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

స్థిర అంతఃవరుస శూన్య r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, పూర్ణాంకం I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/*సంకేతం*/
#నిర్వచించు SHA1_K0 0x5a827999
#నిర్వచించు SHA1_K20 0x6ed9eba1
#నిర్వచించు SHA1_K40 0x8f1bbcdc
#నిర్వచించు SHA1_K60 0xca62c1d6

#నిర్వచించు విత్తనం_0 0x67452301
#నిర్వచించు విత్తనం_1 0xefcdab89
#నిర్వచించు విత్తనం_2_2 0x98badcfe
#నిర్వచించు విత్తనం_3 0x10325476
#నిర్వచించు విత్తనం_4 0xc3d2e1f0

శూన్య SHA1::init() {
  అంతర్గత_State.State[0] = విత్తనం_0;
  అంతర్గత_State.State[1] = విత్తనం_1;
  అంతర్గత_State.State[2] = విత్తనం_2_2;
  అంతర్గత_State.State[3] = విత్తనం_3;
  అంతర్గత_State.State[4] = విత్తనం_4;
  అంతర్గత_State.Byte_లెక్క = 0;
  అంతర్గత_State.బఫరు_స్థానభ్రంశం = 0;
}

శూన్య SHA1::hash_ఖండం() {
  uint32_t A = అంతర్గత_State.State[0];
  uint32_t B = అంతర్గత_State.State[1];
  uint32_t C = అంతర్గత_State.State[2];
  uint32_t D = అంతర్గత_State.State[3];
  uint32_t E = అంతర్గత_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, అంతర్గత_State.బఫరు.L);
  r0(E, A, B, C, D, 1, అంతర్గత_State.బఫరు.L);
  r0(D, E, A, B, C, 2, అంతర్గత_State.బఫరు.L);
  r0(C, D, E, A, B, 3, అంతర్గత_State.బఫరు.L);
  r0(B, C, D, E, A, 4, అంతర్గత_State.బఫరు.L);
  r0(A, B, C, D, E, 5, అంతర్గత_State.బఫరు.L);
  r0(E, A, B, C, D, 6, అంతర్గత_State.బఫరు.L);
  r0(D, E, A, B, C, 7, అంతర్గత_State.బఫరు.L);
  r0(C, D, E, A, B, 8, అంతర్గత_State.బఫరు.L);
  r0(B, C, D, E, A, 9, అంతర్గత_State.బఫరు.L);
  r0(A, B, C, D, E, 10, అంతర్గత_State.బఫరు.L);
  r0(E, A, B, C, D, 11, అంతర్గత_State.బఫరు.L);
  r0(D, E, A, B, C, 12, అంతర్గత_State.బఫరు.L);
  r0(C, D, E, A, B, 13, అంతర్గత_State.బఫరు.L);
  r0(B, C, D, E, A, 14, అంతర్గత_State.బఫరు.L);
  r0(A, B, C, D, E, 15, అంతర్గత_State.బఫరు.L);
  r1(E, A, B, C, D, 16, అంతర్గత_State.బఫరు.L);
  r1(D, E, A, B, C, 17, అంతర్గత_State.బఫరు.L);
  r1(C, D, E, A, B, 18, అంతర్గత_State.బఫరు.L);
  r1(B, C, D, E, A, 19, అంతర్గత_State.బఫరు.L);

  r2(A, B, C, D, E, 20, అంతర్గత_State.బఫరు.L);
  r2(E, A, B, C, D, 21, అంతర్గత_State.బఫరు.L);
  r2(D, E, A, B, C, 22, అంతర్గత_State.బఫరు.L);
  r2(C, D, E, A, B, 23, అంతర్గత_State.బఫరు.L);
  r2(B, C, D, E, A, 24, అంతర్గత_State.బఫరు.L);
  r2(A, B, C, D, E, 25, అంతర్గత_State.బఫరు.L);
  r2(E, A, B, C, D, 26, అంతర్గత_State.బఫరు.L);
  r2(D, E, A, B, C, 27, అంతర్గత_State.బఫరు.L);
  r2(C, D, E, A, B, 28, అంతర్గత_State.బఫరు.L);
  r2(B, C, D, E, A, 29, అంతర్గత_State.బఫరు.L);
  r2(A, B, C, D, E, 30, అంతర్గత_State.బఫరు.L);
  r2(E, A, B, C, D, 31, అంతర్గత_State.బఫరు.L);
  r2(D, E, A, B, C, 32, అంతర్గత_State.బఫరు.L);
  r2(C, D, E, A, B, 33, అంతర్గత_State.బఫరు.L);
  r2(B, C, D, E, A, 34, అంతర్గత_State.బఫరు.L);
  r2(A, B, C, D, E, 35, అంతర్గత_State.బఫరు.L);
  r2(E, A, B, C, D, 36, అంతర్గత_State.బఫరు.L);
  r2(D, E, A, B, C, 37, అంతర్గత_State.బఫరు.L);
  r2(C, D, E, A, B, 38, అంతర్గత_State.బఫరు.L);
  r2(B, C, D, E, A, 39, అంతర్గత_State.బఫరు.L);

  r3(A, B, C, D, E, 40, అంతర్గత_State.బఫరు.L);
  r3(E, A, B, C, D, 41, అంతర్గత_State.బఫరు.L);
  r3(D, E, A, B, C, 42, అంతర్గత_State.బఫరు.L);
  r3(C, D, E, A, B, 43, అంతర్గత_State.బఫరు.L);
  r3(B, C, D, E, A, 44, అంతర్గత_State.బఫరు.L);
  r3(A, B, C, D, E, 45, అంతర్గత_State.బఫరు.L);
  r3(E, A, B, C, D, 46, అంతర్గత_State.బఫరు.L);
  r3(D, E, A, B, C, 47, అంతర్గత_State.బఫరు.L);
  r3(C, D, E, A, B, 48, అంతర్గత_State.బఫరు.L);
  r3(B, C, D, E, A, 49, అంతర్గత_State.బఫరు.L);
  r3(A, B, C, D, E, 50, అంతర్గత_State.బఫరు.L);
  r3(E, A, B, C, D, 51, అంతర్గత_State.బఫరు.L);
  r3(D, E, A, B, C, 52, అంతర్గత_State.బఫరు.L);
  r3(C, D, E, A, B, 53, అంతర్గత_State.బఫరు.L);
  r3(B, C, D, E, A, 54, అంతర్గత_State.బఫరు.L);
  r3(A, B, C, D, E, 55, అంతర్గత_State.బఫరు.L);
  r3(E, A, B, C, D, 56, అంతర్గత_State.బఫరు.L);
  r3(D, E, A, B, C, 57, అంతర్గత_State.బఫరు.L);
  r3(C, D, E, A, B, 58, అంతర్గత_State.బఫరు.L);
  r3(B, C, D, E, A, 59, అంతర్గత_State.బఫరు.L);

  r4(A, B, C, D, E, 60, అంతర్గత_State.బఫరు.L);
  r4(E, A, B, C, D, 61, అంతర్గత_State.బఫరు.L);
  r4(D, E, A, B, C, 62, అంతర్గత_State.బఫరు.L);
  r4(C, D, E, A, B, 63, అంతర్గత_State.బఫరు.L);
  r4(B, C, D, E, A, 64, అంతర్గత_State.బఫరు.L);
  r4(A, B, C, D, E, 65, అంతర్గత_State.బఫరు.L);
  r4(E, A, B, C, D, 66, అంతర్గత_State.బఫరు.L);
  r4(D, E, A, B, C, 67, అంతర్గత_State.బఫరు.L);
  r4(C, D, E, A, B, 68, అంతర్గత_State.బఫరు.L);
  r4(B, C, D, E, A, 69, అంతర్గత_State.బఫరు.L);
  r4(A, B, C, D, E, 70, అంతర్గత_State.బఫరు.L);
  r4(E, A, B, C, D, 71, అంతర్గత_State.బఫరు.L);
  r4(D, E, A, B, C, 72, అంతర్గత_State.బఫరు.L);
  r4(C, D, E, A, B, 73, అంతర్గత_State.బఫరు.L);
  r4(B, C, D, E, A, 74, అంతర్గత_State.బఫరు.L);
  r4(A, B, C, D, E, 75, అంతర్గత_State.బఫరు.L);
  r4(E, A, B, C, D, 76, అంతర్గత_State.బఫరు.L);
  r4(D, E, A, B, C, 77, అంతర్గత_State.బఫరు.L);
  r4(C, D, E, A, B, 78, అంతర్గత_State.బఫరు.L);
  r4(B, C, D, E, A, 79, అంతర్గత_State.బఫరు.L);

  అంతర్గత_State.State[0] += A;
  అంతర్గత_State.State[1] += B;
  అంతర్గత_State.State[2] += C;
  అంతర్గత_State.State[3] += D;
  అంతర్గత_State.State[4] += E;
}

శూన్య SHA1::కూడు_Uncounted(uint8_t దత్తాంశం_2) {
  ఒకవేళ constexpr (sys::IsBigEndianHost)
    అంతర్గత_State.బఫరు.C[అంతర్గత_State.బఫరు_స్థానభ్రంశం] = దత్తాంశం_2;
  లేకపోతే
    అంతర్గత_State.బఫరు.C[అంతర్గత_State.బఫరు_స్థానభ్రంశం ^ 3] = దత్తాంశం_2;

  అంతర్గత_State.బఫరు_స్థానభ్రంశం++;
  ఒకవేళ (అంతర్గత_State.బఫరు_స్థానభ్రంశం == ఖండం_నిడివి) {
    hash_ఖండం();
    అంతర్గత_State.బఫరు_స్థానభ్రంశం = 0;
  }
}

శూన్య SHA1::writebyte(uint8_t దత్తాంశం_2) {
  ++అంతర్గత_State.Byte_లెక్క;
  కూడు_Uncounted(దత్తాంశం_2);
}

శూన్య SHA1::update(ArrayRef<uint8_t> దత్తాంశం_2) {
  అంతర్గత_State.Byte_లెక్క += దత్తాంశం_2.కొలత_2();

  // Finish the current block.
  ఒకవేళ (అంతర్గత_State.బఫరు_స్థానభ్రంశం > 0) {
    స్థిరరాశి size_t Remainder = std::min<size_t>(
        దత్తాంశం_2.కొలత_2(), ఖండం_నిడివి - అంతర్గత_State.బఫరు_స్థానభ్రంశం);
    కొరకు (size_t I = 0; I < Remainder; ++I)
      కూడు_Uncounted(దత్తాంశం_2[I]);
    దత్తాంశం_2 = దత్తాంశం_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  అయితే (దత్తాంశం_2.కొలత_2() >= ఖండం_నిడివి) {
    ప్రతిజ్ఞా(అంతర్గత_State.బఫరు_స్థానభ్రంశం == 0);
    static_assert(ఖండం_నిడివి % 4 == 0);
    constexpr size_t ఖండం_నిడివి_32 = ఖండం_నిడివి / 4;
    కొరకు (size_t I = 0; I < ఖండం_నిడివి_32; ++I)
      అంతర్గత_State.బఫరు.L[I] = support::endian::read32be(&దత్తాంశం_2[I * 4]);
    hash_ఖండం();
    దత్తాంశం_2 = దత్తాంశం_2.drop_front(ఖండం_నిడివి);
  }

  // Finish the remainder.
  కొరకు (uint8_t C : దత్తాంశం_2)
    కూడు_Uncounted(C);
}

శూన్య SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<అక్షరం *>(Str.దత్తాంశం()), Str.కొలత_2()));
}

శూన్య SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  కూడు_Uncounted(0x80);
  అయితే (అంతర్గత_State.బఫరు_స్థానభ్రంశం != 56)
    కూడు_Uncounted(0x00);

  // Append length in the last 8 bytes
  కూడు_Uncounted(0); // We're only using 32 bit lengths
  కూడు_Uncounted(0); // But SHA-1 supports 64 bit lengths
  కూడు_Uncounted(0); // So zero pad the top bits
  కూడు_Uncounted(అంతర్గత_State.Byte_లెక్క >> 29); // Shifting to multiply by 8
  కూడు_Uncounted(అంతర్గత_State.Byte_లెక్క >>
               21); // as SHA-1 supports bitstreams as well as
  కూడు_Uncounted(అంతర్గత_State.Byte_లెక్క >> 13); // byte.
  కూడు_Uncounted(అంతర్గత_State.Byte_లెక్క >> 5);
  కూడు_Uncounted(అంతర్గత_State.Byte_లెక్క << 3);
}

శూన్య SHA1::final(std::array<uint32_t, HASH_నిడివి / 4> &Hash_ఫలితం) {
  // Pad to complete the last block
  pad();

  ఒకవేళ constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    కొరకు (పూర్ణాంకం i = 0; i < 5; i++) {
      Hash_ఫలితం[i] = అంతర్గత_State.State[i];
    }
  } లేకపోతే {
    // Swap byte order back
    కొరకు (పూర్ణాంకం i = 0; i < 5; i++) {
      Hash_ఫలితం[i] = llvm::byteswap(అంతర్గత_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  సంఘం {
    std::array<uint32_t, HASH_నిడివి / 4> Hash_ఫలితం;
    std::array<uint8_t, HASH_నిడివి> Return_ఫలితం;
  };
  static_assert(పరిమాణం(Hash_ఫలితం) == పరిమాణం(Return_ఫలితం));
  final(Hash_ఫలితం);
  తిరిగి Return_ఫలితం;
}

std::array<uint8_t, 20> SHA1::ఫలితం_2() {
  స్వయంచాలక State_గమ్యం_Restore = అంతర్గత_State;

  స్వయంచాలక Hash = final();

  // Restore the state
  అంతర్గత_State = State_గమ్యం_Restore;

  // Return pointer to hash (20 characters)
  తిరిగి Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> దత్తాంశం_2) {
  SHA1 Hash;
  Hash.update(దత్తాంశం_2);
  తిరిగి Hash.final();
}
