//====- SHA256.cpp - SHA256 implementation ---*- C++ -* ======//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/*
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 *
 *   The implementation is based on nacl's sha256 implementation [0] and LLVM's
 *  pre-exsiting SHA1 code [1].
 *
 *   [0] https://hyperelliptic.org/nacl/nacl-20110221.tar.bz2 (public domain
 *       code)
 *   [1] llvm/lib/Support/SHA1.{h,cpp}
 */
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/SHA256.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Endian.h"
#समावेश "llvm/Support/SwapByteOrder.h"
#समावेश <string.h>

namespace llvm {

#घोषणा SHR(x, c) ((x) >> (c))
#घोषणा ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#घोषणा CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#घोषणा MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#घोषणा SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#घोषणा SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#घोषणा SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#घोषणा SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#घोषणा F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  करो {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } जबतक (0);

व्योम SHA256::init() {
  InternalState.State[0] = 0x6A09E667;
  InternalState.State[1] = 0xBB67AE85;
  InternalState.State[2] = 0x3C6EF372;
  InternalState.State[3] = 0xA54FF53A;
  InternalState.State[4] = 0x510E527F;
  InternalState.State[5] = 0x9B05688C;
  InternalState.State[6] = 0x1F83D9AB;
  InternalState.State[7] = 0x5BE0CD19;
  InternalState.Byte_गिनती = 0;
  InternalState.बफर_Offset = 0;
}

व्योम SHA256::hashBlock() {
  uint32_t A = InternalState.State[0];
  uint32_t B = InternalState.State[1];
  uint32_t C = InternalState.State[2];
  uint32_t D = InternalState.State[3];
  uint32_t E = InternalState.State[4];
  uint32_t F = InternalState.State[5];
  uint32_t G = InternalState.State[6];
  uint32_t H = InternalState.State[7];

  uint32_t W00 = InternalState.बफर.L[0];
  uint32_t W01 = InternalState.बफर.L[1];
  uint32_t W02 = InternalState.बफर.L[2];
  uint32_t W03 = InternalState.बफर.L[3];
  uint32_t W04 = InternalState.बफर.L[4];
  uint32_t W05 = InternalState.बफर.L[5];
  uint32_t W06 = InternalState.बफर.L[6];
  uint32_t W07 = InternalState.बफर.L[7];
  uint32_t W08 = InternalState.बफर.L[8];
  uint32_t W09 = InternalState.बफर.L[9];
  uint32_t W10 = InternalState.बफर.L[10];
  uint32_t W11 = InternalState.बफर.L[11];
  uint32_t W12 = InternalState.बफर.L[12];
  uint32_t W13 = InternalState.बफर.L[13];
  uint32_t W14 = InternalState.बफर.L[14];
  uint32_t W15 = InternalState.बफर.L[15];

  F_EXPAND(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
  F_EXPAND(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
  F_EXPAND(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
  F_EXPAND(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
  F_EXPAND(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
  F_EXPAND(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
  F_EXPAND(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
  F_EXPAND(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
  F_EXPAND(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
  F_EXPAND(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
  F_EXPAND(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
  F_EXPAND(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
  F_EXPAND(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
  F_EXPAND(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
  F_EXPAND(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
  F_EXPAND(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);

  F_EXPAND(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
  F_EXPAND(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
  F_EXPAND(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
  F_EXPAND(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
  F_EXPAND(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
  F_EXPAND(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
  F_EXPAND(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
  F_EXPAND(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
  F_EXPAND(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
  F_EXPAND(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
  F_EXPAND(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
  F_EXPAND(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
  F_EXPAND(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
  F_EXPAND(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
  F_EXPAND(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
  F_EXPAND(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);

  F_EXPAND(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
  F_EXPAND(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
  F_EXPAND(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
  F_EXPAND(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
  F_EXPAND(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
  F_EXPAND(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
  F_EXPAND(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
  F_EXPAND(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
  F_EXPAND(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
  F_EXPAND(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
  F_EXPAND(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
  F_EXPAND(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
  F_EXPAND(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
  F_EXPAND(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
  F_EXPAND(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
  F_EXPAND(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);

  F_EXPAND(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
  F_EXPAND(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
  F_EXPAND(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
  F_EXPAND(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
  F_EXPAND(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
  F_EXPAND(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
  F_EXPAND(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
  F_EXPAND(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
  F_EXPAND(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
  F_EXPAND(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
  F_EXPAND(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
  F_EXPAND(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
  F_EXPAND(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
  F_EXPAND(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
  F_EXPAND(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
  F_EXPAND(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

  InternalState.State[0] += A;
  InternalState.State[1] += B;
  InternalState.State[2] += C;
  InternalState.State[3] += D;
  InternalState.State[4] += E;
  InternalState.State[5] += F;
  InternalState.State[6] += G;
  InternalState.State[7] += H;
}

व्योम SHA256::addUncounted(uint8_t तथ्य_2) {
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

व्योम SHA256::writebyte(uint8_t तथ्य_2) {
  ++InternalState.Byte_गिनती;
  addUncounted(तथ्य_2);
}

व्योम SHA256::update(ArrayRef<uint8_t> तथ्य_2) {
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

व्योम SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<अक्षर *>(Str.तथ्य()), Str.आकार_2()));
}

व्योम SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  जबतक (InternalState.बफर_Offset != 56)
    addUncounted(0x00);

  uint64_t len = InternalState.Byte_गिनती << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  addUncounted(len >> 56);
  addUncounted(len >> 48);
  addUncounted(len >> 40);
  addUncounted(len >> 32);
  addUncounted(len >> 24);
  addUncounted(len >> 16);
  addUncounted(len >> 8);
  addUncounted(len);
}

व्योम SHA256::final(std::array<uint32_t, HASH_लंबाई / 4> &Hash_परिणाम) {
  // Pad to complete the last block
  pad();

  अगर constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    क्रम (पूर्णांक i = 0; i < 8; i++) {
      Hash_परिणाम[i] = InternalState.State[i];
    }
  } अन्यथा {
    // Swap byte order back
    क्रम (पूर्णांक i = 0; i < 8; i++) {
      Hash_परिणाम[i] = llvm::byteswap(InternalState.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  जोड़ {
    std::array<uint32_t, HASH_लंबाई / 4> Hash_परिणाम;
    std::array<uint8_t, HASH_लंबाई> Return_परिणाम;
  };
  static_assert(माप(Hash_परिणाम) == माप(Return_परिणाम));
  final(Hash_परिणाम);
  वापस Return_परिणाम;
}

std::array<uint8_t, 32> SHA256::परिणाम_2() {
  स्वतः StateToRestore = InternalState;

  स्वतः Hash = final();

  // Restore the state
  InternalState = StateToRestore;

  // Return pointer to hash (32 characters)
  वापस Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> तथ्य_2) {
  SHA256 Hash;
  Hash.update(तथ्य_2);
  वापस Hash.final();
}

} // namespace llvm
