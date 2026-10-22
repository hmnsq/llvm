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

#समावेश "llvm/Support/SHA1.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Endian.h"
#समावेश "llvm/Support/SwapByteOrder.h"
#समावेश <string.h>

using namespace llvm;

अटल अंतरभूत uint32_t rol(uint32_t संख्या, पूर्णांक Bits) {
  वापस (संख्या << Bits) | (संख्या >> (32 - Bits));
}

अटल अंतरभूत uint32_t blk0(uint32_t *Buf, पूर्णांक I) { वापस Buf[I]; }

अटल अंतरभूत uint32_t blk(uint32_t *Buf, पूर्णांक I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  वापस Buf[I & 15];
}

अटल अंतरभूत व्योम r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णांक I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

अटल अंतरभूत व्योम r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णांक I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

अटल अंतरभूत व्योम r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णांक I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

अटल अंतरभूत व्योम r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णांक I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

अटल अंतरभूत व्योम r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णांक I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/* code */
#घोषणा SHA1_K0 0x5a827999
#घोषणा SHA1_K20 0x6ed9eba1
#घोषणा SHA1_K40 0x8f1bbcdc
#घोषणा SHA1_K60 0xca62c1d6

#घोषणा SEED_0 0x67452301
#घोषणा SEED_1 0xefcdab89
#घोषणा SEED_2 0x98badcfe
#घोषणा SEED_3 0x10325476
#घोषणा SEED_4 0xc3d2e1f0

व्योम SHA1::init() {
  InternalState.State[0] = SEED_0;
  InternalState.State[1] = SEED_1;
  InternalState.State[2] = SEED_2;
  InternalState.State[3] = SEED_3;
  InternalState.State[4] = SEED_4;
  InternalState.Byte_गिनती = 0;
  InternalState.बफर_Offset = 0;
}

व्योम SHA1::hashBlock() {
  uint32_t A = InternalState.State[0];
  uint32_t B = InternalState.State[1];
  uint32_t C = InternalState.State[2];
  uint32_t D = InternalState.State[3];
  uint32_t E = InternalState.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, InternalState.बफर.L);
  r0(E, A, B, C, D, 1, InternalState.बफर.L);
  r0(D, E, A, B, C, 2, InternalState.बफर.L);
  r0(C, D, E, A, B, 3, InternalState.बफर.L);
  r0(B, C, D, E, A, 4, InternalState.बफर.L);
  r0(A, B, C, D, E, 5, InternalState.बफर.L);
  r0(E, A, B, C, D, 6, InternalState.बफर.L);
  r0(D, E, A, B, C, 7, InternalState.बफर.L);
  r0(C, D, E, A, B, 8, InternalState.बफर.L);
  r0(B, C, D, E, A, 9, InternalState.बफर.L);
  r0(A, B, C, D, E, 10, InternalState.बफर.L);
  r0(E, A, B, C, D, 11, InternalState.बफर.L);
  r0(D, E, A, B, C, 12, InternalState.बफर.L);
  r0(C, D, E, A, B, 13, InternalState.बफर.L);
  r0(B, C, D, E, A, 14, InternalState.बफर.L);
  r0(A, B, C, D, E, 15, InternalState.बफर.L);
  r1(E, A, B, C, D, 16, InternalState.बफर.L);
  r1(D, E, A, B, C, 17, InternalState.बफर.L);
  r1(C, D, E, A, B, 18, InternalState.बफर.L);
  r1(B, C, D, E, A, 19, InternalState.बफर.L);

  r2(A, B, C, D, E, 20, InternalState.बफर.L);
  r2(E, A, B, C, D, 21, InternalState.बफर.L);
  r2(D, E, A, B, C, 22, InternalState.बफर.L);
  r2(C, D, E, A, B, 23, InternalState.बफर.L);
  r2(B, C, D, E, A, 24, InternalState.बफर.L);
  r2(A, B, C, D, E, 25, InternalState.बफर.L);
  r2(E, A, B, C, D, 26, InternalState.बफर.L);
  r2(D, E, A, B, C, 27, InternalState.बफर.L);
  r2(C, D, E, A, B, 28, InternalState.बफर.L);
  r2(B, C, D, E, A, 29, InternalState.बफर.L);
  r2(A, B, C, D, E, 30, InternalState.बफर.L);
  r2(E, A, B, C, D, 31, InternalState.बफर.L);
  r2(D, E, A, B, C, 32, InternalState.बफर.L);
  r2(C, D, E, A, B, 33, InternalState.बफर.L);
  r2(B, C, D, E, A, 34, InternalState.बफर.L);
  r2(A, B, C, D, E, 35, InternalState.बफर.L);
  r2(E, A, B, C, D, 36, InternalState.बफर.L);
  r2(D, E, A, B, C, 37, InternalState.बफर.L);
  r2(C, D, E, A, B, 38, InternalState.बफर.L);
  r2(B, C, D, E, A, 39, InternalState.बफर.L);

  r3(A, B, C, D, E, 40, InternalState.बफर.L);
  r3(E, A, B, C, D, 41, InternalState.बफर.L);
  r3(D, E, A, B, C, 42, InternalState.बफर.L);
  r3(C, D, E, A, B, 43, InternalState.बफर.L);
  r3(B, C, D, E, A, 44, InternalState.बफर.L);
  r3(A, B, C, D, E, 45, InternalState.बफर.L);
  r3(E, A, B, C, D, 46, InternalState.बफर.L);
  r3(D, E, A, B, C, 47, InternalState.बफर.L);
  r3(C, D, E, A, B, 48, InternalState.बफर.L);
  r3(B, C, D, E, A, 49, InternalState.बफर.L);
  r3(A, B, C, D, E, 50, InternalState.बफर.L);
  r3(E, A, B, C, D, 51, InternalState.बफर.L);
  r3(D, E, A, B, C, 52, InternalState.बफर.L);
  r3(C, D, E, A, B, 53, InternalState.बफर.L);
  r3(B, C, D, E, A, 54, InternalState.बफर.L);
  r3(A, B, C, D, E, 55, InternalState.बफर.L);
  r3(E, A, B, C, D, 56, InternalState.बफर.L);
  r3(D, E, A, B, C, 57, InternalState.बफर.L);
  r3(C, D, E, A, B, 58, InternalState.बफर.L);
  r3(B, C, D, E, A, 59, InternalState.बफर.L);

  r4(A, B, C, D, E, 60, InternalState.बफर.L);
  r4(E, A, B, C, D, 61, InternalState.बफर.L);
  r4(D, E, A, B, C, 62, InternalState.बफर.L);
  r4(C, D, E, A, B, 63, InternalState.बफर.L);
  r4(B, C, D, E, A, 64, InternalState.बफर.L);
  r4(A, B, C, D, E, 65, InternalState.बफर.L);
  r4(E, A, B, C, D, 66, InternalState.बफर.L);
  r4(D, E, A, B, C, 67, InternalState.बफर.L);
  r4(C, D, E, A, B, 68, InternalState.बफर.L);
  r4(B, C, D, E, A, 69, InternalState.बफर.L);
  r4(A, B, C, D, E, 70, InternalState.बफर.L);
  r4(E, A, B, C, D, 71, InternalState.बफर.L);
  r4(D, E, A, B, C, 72, InternalState.बफर.L);
  r4(C, D, E, A, B, 73, InternalState.बफर.L);
  r4(B, C, D, E, A, 74, InternalState.बफर.L);
  r4(A, B, C, D, E, 75, InternalState.बफर.L);
  r4(E, A, B, C, D, 76, InternalState.बफर.L);
  r4(D, E, A, B, C, 77, InternalState.बफर.L);
  r4(C, D, E, A, B, 78, InternalState.बफर.L);
  r4(B, C, D, E, A, 79, InternalState.बफर.L);

  InternalState.State[0] += A;
  InternalState.State[1] += B;
  InternalState.State[2] += C;
  InternalState.State[3] += D;
  InternalState.State[4] += E;
}

व्योम SHA1::addUncounted(uint8_t तथ्य_2) {
  अगर constexpr (sys::IsBigEndianHost)
    InternalState.बफर.C[InternalState.बफर_Offset] = तथ्य_2;
  अन्यथा
    InternalState.बफर.C[InternalState.बफर_Offset ^ 3] = तथ्य_2;

  InternalState.बफर_Offset++;
  अगर (InternalState.बफर_Offset == BLOCK_लंबाई) {
    hashBlock();
    InternalState.बफर_Offset = 0;
  }
}

व्योम SHA1::writebyte(uint8_t तथ्य_2) {
  ++InternalState.Byte_गिनती;
  addUncounted(तथ्य_2);
}

व्योम SHA1::update(ArrayRef<uint8_t> तथ्य_2) {
  InternalState.Byte_गिनती += तथ्य_2.आकार_2();

  // Finish the current block.
  अगर (InternalState.बफर_Offset > 0) {
    स्थिर माप_प्रकार Remainder = std::min<माप_प्रकार>(
        तथ्य_2.आकार_2(), BLOCK_लंबाई - InternalState.बफर_Offset);
    क्रम (माप_प्रकार I = 0; I < Remainder; ++I)
      addUncounted(तथ्य_2[I]);
    तथ्य_2 = तथ्य_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  जबतक (तथ्य_2.आकार_2() >= BLOCK_लंबाई) {
    निश्चित(InternalState.बफर_Offset == 0);
    static_assert(BLOCK_लंबाई % 4 == 0);
    constexpr माप_प्रकार BLOCK_लंबाई_32 = BLOCK_लंबाई / 4;
    क्रम (माप_प्रकार I = 0; I < BLOCK_लंबाई_32; ++I)
      InternalState.बफर.L[I] = support::endian::read32be(&तथ्य_2[I * 4]);
    hashBlock();
    तथ्य_2 = तथ्य_2.drop_front(BLOCK_लंबाई);
  }

  // Finish the remainder.
  क्रम (uint8_t C : तथ्य_2)
    addUncounted(C);
}

व्योम SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<अक्षर *>(Str.तथ्य()), Str.आकार_2()));
}

व्योम SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  जबतक (InternalState.बफर_Offset != 56)
    addUncounted(0x00);

  // Append length in the last 8 bytes
  addUncounted(0); // We're only using 32 bit lengths
  addUncounted(0); // But SHA-1 supports 64 bit lengths
  addUncounted(0); // So zero pad the top bits
  addUncounted(InternalState.Byte_गिनती >> 29); // Shifting to multiply by 8
  addUncounted(InternalState.Byte_गिनती >>
               21); // as SHA-1 supports bitstreams as well as
  addUncounted(InternalState.Byte_गिनती >> 13); // byte.
  addUncounted(InternalState.Byte_गिनती >> 5);
  addUncounted(InternalState.Byte_गिनती << 3);
}

व्योम SHA1::final(std::array<uint32_t, HASH_लंबाई / 4> &Hash_परिणाम) {
  // Pad to complete the last block
  pad();

  अगर constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    क्रम (पूर्णांक i = 0; i < 5; i++) {
      Hash_परिणाम[i] = InternalState.State[i];
    }
  } अन्यथा {
    // Swap byte order back
    क्रम (पूर्णांक i = 0; i < 5; i++) {
      Hash_परिणाम[i] = llvm::byteswap(InternalState.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  जोड़ {
    std::array<uint32_t, HASH_लंबाई / 4> Hash_परिणाम;
    std::array<uint8_t, HASH_लंबाई> Return_परिणाम;
  };
  static_assert(माप(Hash_परिणाम) == माप(Return_परिणाम));
  final(Hash_परिणाम);
  वापस Return_परिणाम;
}

std::array<uint8_t, 20> SHA1::परिणाम_2() {
  स्वतः StateToRestore = InternalState;

  स्वतः Hash = final();

  // Restore the state
  InternalState = StateToRestore;

  // Return pointer to hash (20 characters)
  वापस Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> तथ्य_2) {
  SHA1 Hash;
  Hash.update(तथ्य_2);
  वापस Hash.final();
}
