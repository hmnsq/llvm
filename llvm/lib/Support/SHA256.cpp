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

#ضمن "llvm/Support/SHA256.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Endian.h"
#ضمن "llvm/Support/SwapByteOrder.h"
#ضمن <string.h>

نطاق llvm {

#وصف_ڪريو SHR(x, c) ((x) >> (c))
#وصف_ڪريو ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#وصف_ڪريو CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#وصف_ڪريو MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#وصف_ڪريو SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#وصف_ڪريو SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#وصف_ڪريو SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#وصف_ڪريو SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#وصف_ڪريو F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  افعل {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } طالما (0);

فراغ SHA256::init() {
  اندروني_State.State[0] = 0x6A09E667;
  اندروني_State.State[1] = 0xBB67AE85;
  اندروني_State.State[2] = 0x3C6EF372;
  اندروني_State.State[3] = 0xA54FF53A;
  اندروني_State.State[4] = 0x510E527F;
  اندروني_State.State[5] = 0x9B05688C;
  اندروني_State.State[6] = 0x1F83D9AB;
  اندروني_State.State[7] = 0x5BE0CD19;
  اندروني_State.Byte_ڳڻپ = 0;
  اندروني_State.بفر_وٿي = 0;
}

فراغ SHA256::hash_بلاڪ() {
  uint32_t A = اندروني_State.State[0];
  uint32_t B = اندروني_State.State[1];
  uint32_t C = اندروني_State.State[2];
  uint32_t D = اندروني_State.State[3];
  uint32_t E = اندروني_State.State[4];
  uint32_t F = اندروني_State.State[5];
  uint32_t G = اندروني_State.State[6];
  uint32_t H = اندروني_State.State[7];

  uint32_t W00 = اندروني_State.بفر.L[0];
  uint32_t W01 = اندروني_State.بفر.L[1];
  uint32_t W02 = اندروني_State.بفر.L[2];
  uint32_t W03 = اندروني_State.بفر.L[3];
  uint32_t W04 = اندروني_State.بفر.L[4];
  uint32_t W05 = اندروني_State.بفر.L[5];
  uint32_t W06 = اندروني_State.بفر.L[6];
  uint32_t W07 = اندروني_State.بفر.L[7];
  uint32_t W08 = اندروني_State.بفر.L[8];
  uint32_t W09 = اندروني_State.بفر.L[9];
  uint32_t W10 = اندروني_State.بفر.L[10];
  uint32_t W11 = اندروني_State.بفر.L[11];
  uint32_t W12 = اندروني_State.بفر.L[12];
  uint32_t W13 = اندروني_State.بفر.L[13];
  uint32_t W14 = اندروني_State.بفر.L[14];
  uint32_t W15 = اندروني_State.بفر.L[15];

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

  اندروني_State.State[0] += A;
  اندروني_State.State[1] += B;
  اندروني_State.State[2] += C;
  اندروني_State.State[3] += D;
  اندروني_State.State[4] += E;
  اندروني_State.State[5] += F;
  اندروني_State.State[6] += G;
  اندروني_State.State[7] += H;
}

فراغ SHA256::جوڙيو_Uncounted(uint8_t معلومات_2) {
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

فراغ SHA256::writebyte(uint8_t معلومات_2) {
  ++اندروني_State.Byte_ڳڻپ;
  جوڙيو_Uncounted(معلومات_2);
}

فراغ SHA256::update(ArrayRef<uint8_t> معلومات_2) {
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

فراغ SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.معلومات()), Str.سائيز_2()));
}

فراغ SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جوڙيو_Uncounted(0x80);
  طالما (اندروني_State.بفر_وٿي != 56)
    جوڙيو_Uncounted(0x00);

  uint64_t len = اندروني_State.Byte_ڳڻپ << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  جوڙيو_Uncounted(len >> 56);
  جوڙيو_Uncounted(len >> 48);
  جوڙيو_Uncounted(len >> 40);
  جوڙيو_Uncounted(len >> 32);
  جوڙيو_Uncounted(len >> 24);
  جوڙيو_Uncounted(len >> 16);
  جوڙيو_Uncounted(len >> 8);
  جوڙيو_Uncounted(len);
}

فراغ SHA256::final(std::array<uint32_t, HASH_ڊيگهائي / 4> &Hash_نتيجو) {
  // Pad to complete the last block
  pad();

  إذا constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    لكل (صحيح i = 0; i < 8; i++) {
      Hash_نتيجو[i] = اندروني_State.State[i];
    }
  } وإلا {
    // Swap byte order back
    لكل (صحيح i = 0; i < 8; i++) {
      Hash_نتيجو[i] = llvm::byteswap(اندروني_State.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  اتحاد {
    std::array<uint32_t, HASH_ڊيگهائي / 4> Hash_نتيجو;
    std::array<uint8_t, HASH_ڊيگهائي> Return_نتيجو;
  };
  static_assert(حجم(Hash_نتيجو) == حجم(Return_نتيجو));
  final(Hash_نتيجو);
  أرجع Return_نتيجو;
}

std::array<uint8_t, 32> SHA256::نتيجو_2() {
  خودڪار State_منزل_Restore = اندروني_State;

  خودڪار Hash = final();

  // Restore the state
  اندروني_State = State_منزل_Restore;

  // Return pointer to hash (32 characters)
  أرجع Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> معلومات_2) {
  SHA256 Hash;
  Hash.update(معلومات_2);
  أرجع Hash.final();
}

} // namespace llvm
