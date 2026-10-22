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

#ಅನ್ತರ್ಭಾವ "llvm/Support/SHA256.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Endian.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"
#ಅನ್ತರ್ಭಾವ <string.h>

ನಾಮಾಕಾಶ llvm {

#ವ್ಯಾಖ್ಯಾನಿಸು SHR(x, c) ((x) >> (c))
#ವ್ಯಾಖ್ಯಾನಿಸು ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#ವ್ಯಾಖ್ಯಾನಿಸು CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#ವ್ಯಾಖ್ಯಾನಿಸು MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#ವ್ಯಾಖ್ಯಾನಿಸು SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#ವ್ಯಾಖ್ಯಾನಿಸು SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#ವ್ಯಾಖ್ಯಾನಿಸು SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#ವ್ಯಾಖ್ಯಾನಿಸು SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#ವ್ಯಾಖ್ಯಾನಿಸು F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  ಮಾಡು {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } ಆಗ (0);

ಶೂನ್ಯ SHA256::init() {
  ಆಂತರಿಕ_State.State[0] = 0x6A09E667;
  ಆಂತರಿಕ_State.State[1] = 0xBB67AE85;
  ಆಂತರಿಕ_State.State[2] = 0x3C6EF372;
  ಆಂತರಿಕ_State.State[3] = 0xA54FF53A;
  ಆಂತರಿಕ_State.State[4] = 0x510E527F;
  ಆಂತರಿಕ_State.State[5] = 0x9B05688C;
  ಆಂತರಿಕ_State.State[6] = 0x1F83D9AB;
  ಆಂತರಿಕ_State.State[7] = 0x5BE0CD19;
  ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ = 0;
  ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ = 0;
}

ಶೂನ್ಯ SHA256::hash_ಖಂಡ() {
  uint32_t A = ಆಂತರಿಕ_State.State[0];
  uint32_t B = ಆಂತರಿಕ_State.State[1];
  uint32_t C = ಆಂತರಿಕ_State.State[2];
  uint32_t D = ಆಂತರಿಕ_State.State[3];
  uint32_t E = ಆಂತರಿಕ_State.State[4];
  uint32_t F = ಆಂತರಿಕ_State.State[5];
  uint32_t G = ಆಂತರಿಕ_State.State[6];
  uint32_t H = ಆಂತರಿಕ_State.State[7];

  uint32_t W00 = ಆಂತರಿಕ_State.ಬಫರ್.L[0];
  uint32_t W01 = ಆಂತರಿಕ_State.ಬಫರ್.L[1];
  uint32_t W02 = ಆಂತರಿಕ_State.ಬಫರ್.L[2];
  uint32_t W03 = ಆಂತರಿಕ_State.ಬಫರ್.L[3];
  uint32_t W04 = ಆಂತರಿಕ_State.ಬಫರ್.L[4];
  uint32_t W05 = ಆಂತರಿಕ_State.ಬಫರ್.L[5];
  uint32_t W06 = ಆಂತರಿಕ_State.ಬಫರ್.L[6];
  uint32_t W07 = ಆಂತರಿಕ_State.ಬಫರ್.L[7];
  uint32_t W08 = ಆಂತರಿಕ_State.ಬಫರ್.L[8];
  uint32_t W09 = ಆಂತರಿಕ_State.ಬಫರ್.L[9];
  uint32_t W10 = ಆಂತರಿಕ_State.ಬಫರ್.L[10];
  uint32_t W11 = ಆಂತರಿಕ_State.ಬಫರ್.L[11];
  uint32_t W12 = ಆಂತರಿಕ_State.ಬಫರ್.L[12];
  uint32_t W13 = ಆಂತರಿಕ_State.ಬಫರ್.L[13];
  uint32_t W14 = ಆಂತರಿಕ_State.ಬಫರ್.L[14];
  uint32_t W15 = ಆಂತರಿಕ_State.ಬಫರ್.L[15];

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

  ಆಂತರಿಕ_State.State[0] += A;
  ಆಂತರಿಕ_State.State[1] += B;
  ಆಂತರಿಕ_State.State[2] += C;
  ಆಂತರಿಕ_State.State[3] += D;
  ಆಂತರಿಕ_State.State[4] += E;
  ಆಂತರಿಕ_State.State[5] += F;
  ಆಂತರಿಕ_State.State[6] += G;
  ಆಂತರಿಕ_State.State[7] += H;
}

ಶೂನ್ಯ SHA256::ಕೂಡಿಸು_Uncounted(uint8_t ದತ್ತಾಂಶ_2) {
  ಆದರೆ constexpr (sys::IsBigEndianHost)
    ಆಂತರಿಕ_State.ಬಫರ್.C[ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ] = ದತ್ತಾಂಶ_2;
  ಇಲ್ಲದಿದ್ದರೆ
    ಆಂತರಿಕ_State.ಬಫರ್.C[ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ ^ 3] = ದತ್ತಾಂಶ_2;

  ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ++;
  ಆದರೆ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ == ಖಂಡ_ಉದ್ದಳತೆ) {
    hash_ಖಂಡ();
    ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ = 0;
  }
}

ಶೂನ್ಯ SHA256::writebyte(uint8_t ದತ್ತಾಂಶ_2) {
  ++ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ;
  ಕೂಡಿಸು_Uncounted(ದತ್ತಾಂಶ_2);
}

ಶೂನ್ಯ SHA256::update(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) {
  ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ += ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3();

  // Finish the current block.
  ಆದರೆ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ > 0) {
    ಸ್ಥಿರಾಂಕ size_t Remainder = std::min<size_t>(
        ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3(), ಖಂಡ_ಉದ್ದಳತೆ - ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ);
    ಗಾಗಿ (size_t I = 0; I < Remainder; ++I)
      ಕೂಡಿಸು_Uncounted(ದತ್ತಾಂಶ_2[I]);
    ದತ್ತಾಂಶ_2 = ದತ್ತಾಂಶ_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  ಆಗ (ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3() >= ಖಂಡ_ಉದ್ದಳತೆ) {
    ಪ್ರತಿಜ್ಞಾ(ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ == 0);
    static_assert(ಖಂಡ_ಉದ್ದಳತೆ % 4 == 0);
    constexpr size_t ಖಂಡ_ಉದ್ದಳತೆ_32 = ಖಂಡ_ಉದ್ದಳತೆ / 4;
    ಗಾಗಿ (size_t I = 0; I < ಖಂಡ_ಉದ್ದಳತೆ_32; ++I)
      ಆಂತರಿಕ_State.ಬಫರ್.L[I] = support::endian::read32be(&ದತ್ತಾಂಶ_2[I * 4]);
    hash_ಖಂಡ();
    ದತ್ತಾಂಶ_2 = ದತ್ತಾಂಶ_2.drop_front(ಖಂಡ_ಉದ್ದಳತೆ);
  }

  // Finish the remainder.
  ಗಾಗಿ (uint8_t C : ದತ್ತಾಂಶ_2)
    ಕೂಡಿಸು_Uncounted(C);
}

ಶೂನ್ಯ SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<ಅಕ್ಷರ *>(Str.ದತ್ತಾಂಶ()), Str.ಗಾತ್ರ_3()));
}

ಶೂನ್ಯ SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  ಕೂಡಿಸು_Uncounted(0x80);
  ಆಗ (ಆಂತರಿಕ_State.ಬಫರ್_ಸ್ಥಾನಾಂತರ != 56)
    ಕೂಡಿಸು_Uncounted(0x00);

  uint64_t len = ಆಂತರಿಕ_State.Byte_ಎಣಿಕೆ << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  ಕೂಡಿಸು_Uncounted(len >> 56);
  ಕೂಡಿಸು_Uncounted(len >> 48);
  ಕೂಡಿಸು_Uncounted(len >> 40);
  ಕೂಡಿಸು_Uncounted(len >> 32);
  ಕೂಡಿಸು_Uncounted(len >> 24);
  ಕೂಡಿಸು_Uncounted(len >> 16);
  ಕೂಡಿಸು_Uncounted(len >> 8);
  ಕೂಡಿಸು_Uncounted(len);
}

ಶೂನ್ಯ SHA256::final(std::array<uint32_t, HASH_ಉದ್ದಳತೆ / 4> &Hash_ಫಲಿತಾಂಶ) {
  // Pad to complete the last block
  pad();

  ಆದರೆ constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < 8; i++) {
      Hash_ಫಲಿತಾಂಶ[i] = ಆಂತರಿಕ_State.State[i];
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Swap byte order back
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < 8; i++) {
      Hash_ಫಲಿತಾಂಶ[i] = llvm::byteswap(ಆಂತರಿಕ_State.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  ಒಕ್ಕೂಟ {
    std::array<uint32_t, HASH_ಉದ್ದಳತೆ / 4> Hash_ಫಲಿತಾಂಶ;
    std::array<uint8_t, HASH_ಉದ್ದಳತೆ> Return_ಫಲಿತಾಂಶ;
  };
  static_assert(ಗಾತ್ರ(Hash_ಫಲಿತಾಂಶ) == ಗಾತ್ರ(Return_ಫಲಿತಾಂಶ));
  final(Hash_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ Return_ಫಲಿತಾಂಶ;
}

std::array<uint8_t, 32> SHA256::ಫಲಿತಾಂಶ_2() {
  ಸ್ವಯಂಚಾಲಿತ State_ಗಮ್ಯ_Restore = ಆಂತರಿಕ_State;

  ಸ್ವಯಂಚಾಲಿತ Hash = final();

  // Restore the state
  ಆಂತರಿಕ_State = State_ಗಮ್ಯ_Restore;

  // Return pointer to hash (32 characters)
  ಹಿಂತಿರುಗಿ Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) {
  SHA256 Hash;
  Hash.update(ದತ್ತಾಂಶ_2);
  ಹಿಂತಿರುಗಿ Hash.final();
}

} // namespace llvm
