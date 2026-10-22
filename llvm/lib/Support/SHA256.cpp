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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SHA256.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SwapByteOrder.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

ਨਾਮਾਕਾਸ਼ llvm {

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHR(x, c) ((x) >> (c))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  ਕਰੋ {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } ਜਦੋਂ (0);

ਖਾਲੀ SHA256::init() {
  ਅੰਦਰੂਨੀ_State.State[0] = 0x6A09E667;
  ਅੰਦਰੂਨੀ_State.State[1] = 0xBB67AE85;
  ਅੰਦਰੂਨੀ_State.State[2] = 0x3C6EF372;
  ਅੰਦਰੂਨੀ_State.State[3] = 0xA54FF53A;
  ਅੰਦਰੂਨੀ_State.State[4] = 0x510E527F;
  ਅੰਦਰੂਨੀ_State.State[5] = 0x9B05688C;
  ਅੰਦਰੂਨੀ_State.State[6] = 0x1F83D9AB;
  ਅੰਦਰੂਨੀ_State.State[7] = 0x5BE0CD19;
  ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ = 0;
  ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ = 0;
}

ਖਾਲੀ SHA256::hash_ਖੰਡ() {
  uint32_t A = ਅੰਦਰੂਨੀ_State.State[0];
  uint32_t B = ਅੰਦਰੂਨੀ_State.State[1];
  uint32_t C = ਅੰਦਰੂਨੀ_State.State[2];
  uint32_t D = ਅੰਦਰੂਨੀ_State.State[3];
  uint32_t E = ਅੰਦਰੂਨੀ_State.State[4];
  uint32_t F = ਅੰਦਰੂਨੀ_State.State[5];
  uint32_t G = ਅੰਦਰੂਨੀ_State.State[6];
  uint32_t H = ਅੰਦਰੂਨੀ_State.State[7];

  uint32_t W00 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[0];
  uint32_t W01 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[1];
  uint32_t W02 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[2];
  uint32_t W03 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[3];
  uint32_t W04 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[4];
  uint32_t W05 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[5];
  uint32_t W06 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[6];
  uint32_t W07 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[7];
  uint32_t W08 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[8];
  uint32_t W09 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[9];
  uint32_t W10 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[10];
  uint32_t W11 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[11];
  uint32_t W12 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[12];
  uint32_t W13 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[13];
  uint32_t W14 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[14];
  uint32_t W15 = ਅੰਦਰੂਨੀ_State.ਬਫਰ.L[15];

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

  ਅੰਦਰੂਨੀ_State.State[0] += A;
  ਅੰਦਰੂਨੀ_State.State[1] += B;
  ਅੰਦਰੂਨੀ_State.State[2] += C;
  ਅੰਦਰੂਨੀ_State.State[3] += D;
  ਅੰਦਰੂਨੀ_State.State[4] += E;
  ਅੰਦਰੂਨੀ_State.State[5] += F;
  ਅੰਦਰੂਨੀ_State.State[6] += G;
  ਅੰਦਰੂਨੀ_State.State[7] += H;
}

ਖਾਲੀ SHA256::ਜੋੜੋ_Uncounted(uint8_t ਅੰਕੜੇ_2) {
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

ਖਾਲੀ SHA256::writebyte(uint8_t ਅੰਕੜੇ_2) {
  ++ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ;
  ਜੋੜੋ_Uncounted(ਅੰਕੜੇ_2);
}

ਖਾਲੀ SHA256::update(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
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

ਖਾਲੀ SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<ਅੱਖਰ *>(Str.ਅੰਕੜੇ()), Str.ਮਾਪ_2()));
}

ਖਾਲੀ SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  ਜੋੜੋ_Uncounted(0x80);
  ਜਦੋਂ (ਅੰਦਰੂਨੀ_State.ਬਫਰ_ਵਿੱਥ != 56)
    ਜੋੜੋ_Uncounted(0x00);

  uint64_t len = ਅੰਦਰੂਨੀ_State.Byte_ਗਿਣਤੀ << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  ਜੋੜੋ_Uncounted(len >> 56);
  ਜੋੜੋ_Uncounted(len >> 48);
  ਜੋੜੋ_Uncounted(len >> 40);
  ਜੋੜੋ_Uncounted(len >> 32);
  ਜੋੜੋ_Uncounted(len >> 24);
  ਜੋੜੋ_Uncounted(len >> 16);
  ਜੋੜੋ_Uncounted(len >> 8);
  ਜੋੜੋ_Uncounted(len);
}

ਖਾਲੀ SHA256::final(std::array<uint32_t, HASH_ਲੰਮਾਈ / 4> &Hash_ਨਤੀਜਾ) {
  // Pad to complete the last block
  pad();

  ਜੇ constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    ਲਈ (ਪੂਰਨ i = 0; i < 8; i++) {
      Hash_ਨਤੀਜਾ[i] = ਅੰਦਰੂਨੀ_State.State[i];
    }
  } ਨਹੀਂਤਾਂ {
    // Swap byte order back
    ਲਈ (ਪੂਰਨ i = 0; i < 8; i++) {
      Hash_ਨਤੀਜਾ[i] = llvm::byteswap(ਅੰਦਰੂਨੀ_State.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  ਸੰਘ {
    std::array<uint32_t, HASH_ਲੰਮਾਈ / 4> Hash_ਨਤੀਜਾ;
    std::array<uint8_t, HASH_ਲੰਮਾਈ> Return_ਨਤੀਜਾ;
  };
  static_assert(ਆਕਾਰ(Hash_ਨਤੀਜਾ) == ਆਕਾਰ(Return_ਨਤੀਜਾ));
  final(Hash_ਨਤੀਜਾ);
  ਵਾਪਸ Return_ਨਤੀਜਾ;
}

std::array<uint8_t, 32> SHA256::ਨਤੀਜਾ_2() {
  ਸਵੈਚਾਲਿਤ State_ਮੰਜ਼ਿਲ_Restore = ਅੰਦਰੂਨੀ_State;

  ਸਵੈਚਾਲਿਤ Hash = final();

  // Restore the state
  ਅੰਦਰੂਨੀ_State = State_ਮੰਜ਼ਿਲ_Restore;

  // Return pointer to hash (32 characters)
  ਵਾਪਸ Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  SHA256 Hash;
  Hash.update(ਅੰਕੜੇ_2);
  ਵਾਪਸ Hash.final();
}

} // namespace llvm
