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

#شامل "llvm/Support/SHA256.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Endian.h"
#شامل "llvm/Support/SwapByteOrder.h"
#شامل <string.h>

نامخلا llvm {

#تعریف_کرو SHR(x, c) ((x) >> (c))
#تعریف_کرو ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#تعریف_کرو CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#تعریف_کرو MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#تعریف_کرو SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#تعریف_کرو SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#تعریف_کرو SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#تعریف_کرو SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#تعریف_کرو F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  کرو {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } جب (0);

خالی SHA256::init() {
  اندرونی_State.State[0] = 0x6A09E667;
  اندرونی_State.State[1] = 0xBB67AE85;
  اندرونی_State.State[2] = 0x3C6EF372;
  اندرونی_State.State[3] = 0xA54FF53A;
  اندرونی_State.State[4] = 0x510E527F;
  اندرونی_State.State[5] = 0x9B05688C;
  اندرونی_State.State[6] = 0x1F83D9AB;
  اندرونی_State.State[7] = 0x5BE0CD19;
  اندرونی_State.Byte_گنتی = 0;
  اندرونی_State.بفر_انحراف = 0;
}

خالی SHA256::hash_بلاک() {
  uint32_t A = اندرونی_State.State[0];
  uint32_t B = اندرونی_State.State[1];
  uint32_t C = اندرونی_State.State[2];
  uint32_t D = اندرونی_State.State[3];
  uint32_t E = اندرونی_State.State[4];
  uint32_t F = اندرونی_State.State[5];
  uint32_t G = اندرونی_State.State[6];
  uint32_t H = اندرونی_State.State[7];

  uint32_t W00 = اندرونی_State.بفر.L[0];
  uint32_t W01 = اندرونی_State.بفر.L[1];
  uint32_t W02 = اندرونی_State.بفر.L[2];
  uint32_t W03 = اندرونی_State.بفر.L[3];
  uint32_t W04 = اندرونی_State.بفر.L[4];
  uint32_t W05 = اندرونی_State.بفر.L[5];
  uint32_t W06 = اندرونی_State.بفر.L[6];
  uint32_t W07 = اندرونی_State.بفر.L[7];
  uint32_t W08 = اندرونی_State.بفر.L[8];
  uint32_t W09 = اندرونی_State.بفر.L[9];
  uint32_t W10 = اندرونی_State.بفر.L[10];
  uint32_t W11 = اندرونی_State.بفر.L[11];
  uint32_t W12 = اندرونی_State.بفر.L[12];
  uint32_t W13 = اندرونی_State.بفر.L[13];
  uint32_t W14 = اندرونی_State.بفر.L[14];
  uint32_t W15 = اندرونی_State.بفر.L[15];

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

  اندرونی_State.State[0] += A;
  اندرونی_State.State[1] += B;
  اندرونی_State.State[2] += C;
  اندرونی_State.State[3] += D;
  اندرونی_State.State[4] += E;
  اندرونی_State.State[5] += F;
  اندرونی_State.State[6] += G;
  اندرونی_State.State[7] += H;
}

خالی SHA256::جوڑو_Uncounted(uint8_t ڈیٹا_2) {
  اگر constexpr (sys::IsBigEndianHost)
    اندرونی_State.بفر.C[اندرونی_State.بفر_انحراف] = ڈیٹا_2;
  ورنہ
    اندرونی_State.بفر.C[اندرونی_State.بفر_انحراف ^ 3] = ڈیٹا_2;

  اندرونی_State.بفر_انحراف++;
  اگر (اندرونی_State.بفر_انحراف == بلاک_لمبائی) {
    hash_بلاک();
    اندرونی_State.بفر_انحراف = 0;
  }
}

خالی SHA256::writebyte(uint8_t ڈیٹا_2) {
  ++اندرونی_State.Byte_گنتی;
  جوڑو_Uncounted(ڈیٹا_2);
}

خالی SHA256::update(ArrayRef<uint8_t> ڈیٹا_2) {
  اندرونی_State.Byte_گنتی += ڈیٹا_2.سائز_2();

  // Finish the current block.
  اگر (اندرونی_State.بفر_انحراف > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        ڈیٹا_2.سائز_2(), بلاک_لمبائی - اندرونی_State.بفر_انحراف);
    برائے (size_t I = 0; I < Remainder; ++I)
      جوڑو_Uncounted(ڈیٹا_2[I]);
    ڈیٹا_2 = ڈیٹا_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  جب (ڈیٹا_2.سائز_2() >= بلاک_لمبائی) {
    أكد(اندرونی_State.بفر_انحراف == 0);
    static_assert(بلاک_لمبائی % 4 == 0);
    constexpr size_t بلاک_لمبائی_32 = بلاک_لمبائی / 4;
    برائے (size_t I = 0; I < بلاک_لمبائی_32; ++I)
      اندرونی_State.بفر.L[I] = support::endian::read32be(&ڈیٹا_2[I * 4]);
    hash_بلاک();
    ڈیٹا_2 = ڈیٹا_2.drop_front(بلاک_لمبائی);
  }

  // Finish the remainder.
  برائے (uint8_t C : ڈیٹا_2)
    جوڑو_Uncounted(C);
}

خالی SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<حرف *>(Str.ڈیٹا()), Str.سائز_2()));
}

خالی SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  جوڑو_Uncounted(0x80);
  جب (اندرونی_State.بفر_انحراف != 56)
    جوڑو_Uncounted(0x00);

  uint64_t len = اندرونی_State.Byte_گنتی << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  جوڑو_Uncounted(len >> 56);
  جوڑو_Uncounted(len >> 48);
  جوڑو_Uncounted(len >> 40);
  جوڑو_Uncounted(len >> 32);
  جوڑو_Uncounted(len >> 24);
  جوڑو_Uncounted(len >> 16);
  جوڑو_Uncounted(len >> 8);
  جوڑو_Uncounted(len);
}

خالی SHA256::final(std::array<uint32_t, HASH_لمبائی / 4> &Hash_نتیجہ) {
  // Pad to complete the last block
  pad();

  اگر constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    برائے (صحیح i = 0; i < 8; i++) {
      Hash_نتیجہ[i] = اندرونی_State.State[i];
    }
  } ورنہ {
    // Swap byte order back
    برائے (صحیح i = 0; i < 8; i++) {
      Hash_نتیجہ[i] = llvm::byteswap(اندرونی_State.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  اتحاد {
    std::array<uint32_t, HASH_لمبائی / 4> Hash_نتیجہ;
    std::array<uint8_t, HASH_لمبائی> Return_نتیجہ;
  };
  static_assert(حجم(Hash_نتیجہ) == حجم(Return_نتیجہ));
  final(Hash_نتیجہ);
  واپس Return_نتیجہ;
}

std::array<uint8_t, 32> SHA256::نتیجہ_2() {
  خودکار State_مقصد_Restore = اندرونی_State;

  خودکار Hash = final();

  // Restore the state
  اندرونی_State = State_مقصد_Restore;

  // Return pointer to hash (32 characters)
  واپس Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> ڈیٹا_2) {
  SHA256 Hash;
  Hash.update(ڈیٹا_2);
  واپس Hash.final();
}

} // namespace llvm
