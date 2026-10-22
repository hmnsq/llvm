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

#अन्तर्भाव "llvm/Support/SHA256.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"
#अन्तर्भाव <string.h>

नामाकाश llvm {

#परिभाषय SHR(x, c) ((x) >> (c))
#परिभाषय ROTR(x, n) (((x) >> n) | ((x) << (32 - (n))))

#परिभाषय CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#परिभाषय MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#परिभाषय SIGMA_0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#परिभाषय SIGMA_1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#परिभाषय SIGMA_2(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#परिभाषय SIGMA_3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))

#परिभाषय F_EXPAND(A, B, C, D, E, F, G, H, M1, M2, M3, M4, k)                    \
  कुरु {                                                                         \
    H += SIGMA_1(E) + CH(E, F, G) + M1 + k;                                    \
    D += H;                                                                    \
    H += SIGMA_0(A) + MAJ(A, B, C);                                            \
    M1 += SIGMA_2(M2) + M3 + SIGMA_3(M4);                                      \
  } यावत् (0);

शून्य SHA256::init() {
  आन्तरिकम्_State.State[0] = 0x6A09E667;
  आन्तरिकम्_State.State[1] = 0xBB67AE85;
  आन्तरिकम्_State.State[2] = 0x3C6EF372;
  आन्तरिकम्_State.State[3] = 0xA54FF53A;
  आन्तरिकम्_State.State[4] = 0x510E527F;
  आन्तरिकम्_State.State[5] = 0x9B05688C;
  आन्तरिकम्_State.State[6] = 0x1F83D9AB;
  आन्तरिकम्_State.State[7] = 0x5BE0CD19;
  आन्तरिकम्_State.Byte_गणनम् = 0;
  आन्तरिकम्_State.बफर_विस्थापनम् = 0;
}

शून्य SHA256::hash_खण्डम्() {
  uint32_t A = आन्तरिकम्_State.State[0];
  uint32_t B = आन्तरिकम्_State.State[1];
  uint32_t C = आन्तरिकम्_State.State[2];
  uint32_t D = आन्तरिकम्_State.State[3];
  uint32_t E = आन्तरिकम्_State.State[4];
  uint32_t F = आन्तरिकम्_State.State[5];
  uint32_t G = आन्तरिकम्_State.State[6];
  uint32_t H = आन्तरिकम्_State.State[7];

  uint32_t W00 = आन्तरिकम्_State.बफर.L[0];
  uint32_t W01 = आन्तरिकम्_State.बफर.L[1];
  uint32_t W02 = आन्तरिकम्_State.बफर.L[2];
  uint32_t W03 = आन्तरिकम्_State.बफर.L[3];
  uint32_t W04 = आन्तरिकम्_State.बफर.L[4];
  uint32_t W05 = आन्तरिकम्_State.बफर.L[5];
  uint32_t W06 = आन्तरिकम्_State.बफर.L[6];
  uint32_t W07 = आन्तरिकम्_State.बफर.L[7];
  uint32_t W08 = आन्तरिकम्_State.बफर.L[8];
  uint32_t W09 = आन्तरिकम्_State.बफर.L[9];
  uint32_t W10 = आन्तरिकम्_State.बफर.L[10];
  uint32_t W11 = आन्तरिकम्_State.बफर.L[11];
  uint32_t W12 = आन्तरिकम्_State.बफर.L[12];
  uint32_t W13 = आन्तरिकम्_State.बफर.L[13];
  uint32_t W14 = आन्तरिकम्_State.बफर.L[14];
  uint32_t W15 = आन्तरिकम्_State.बफर.L[15];

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

  आन्तरिकम्_State.State[0] += A;
  आन्तरिकम्_State.State[1] += B;
  आन्तरिकम्_State.State[2] += C;
  आन्तरिकम्_State.State[3] += D;
  आन्तरिकम्_State.State[4] += E;
  आन्तरिकम्_State.State[5] += F;
  आन्तरिकम्_State.State[6] += G;
  आन्तरिकम्_State.State[7] += H;
}

शून्य SHA256::सङ्कलय_Uncounted(uint8_t दत्त_2) {
  यदि constexpr (sys::IsBigEndianHost)
    आन्तरिकम्_State.बफर.C[आन्तरिकम्_State.बफर_विस्थापनम्] = दत्त_2;
  अन्यथा
    आन्तरिकम्_State.बफर.C[आन्तरिकम्_State.बफर_विस्थापनम् ^ 3] = दत्त_2;

  आन्तरिकम्_State.बफर_विस्थापनम्++;
  यदि (आन्तरिकम्_State.बफर_विस्थापनम् == खण्डम्_दीर्घता) {
    hash_खण्डम्();
    आन्तरिकम्_State.बफर_विस्थापनम् = 0;
  }
}

शून्य SHA256::writebyte(uint8_t दत्त_2) {
  ++आन्तरिकम्_State.Byte_गणनम्;
  सङ्कलय_Uncounted(दत्त_2);
}

शून्य SHA256::update(ArrayRef<uint8_t> दत्त_2) {
  आन्तरिकम्_State.Byte_गणनम् += दत्त_2.आकार_2();

  // Finish the current block.
  यदि (आन्तरिकम्_State.बफर_विस्थापनम् > 0) {
    ध्रुव size_t Remainder = std::min<size_t>(
        दत्त_2.आकार_2(), खण्डम्_दीर्घता - आन्तरिकम्_State.बफर_विस्थापनम्);
    प्रति (size_t I = 0; I < Remainder; ++I)
      सङ्कलय_Uncounted(दत्त_2[I]);
    दत्त_2 = दत्त_2.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  यावत् (दत्त_2.आकार_2() >= खण्डम्_दीर्घता) {
    प्रतिज्ञा(आन्तरिकम्_State.बफर_विस्थापनम् == 0);
    static_assert(खण्डम्_दीर्घता % 4 == 0);
    constexpr size_t खण्डम्_दीर्घता_32 = खण्डम्_दीर्घता / 4;
    प्रति (size_t I = 0; I < खण्डम्_दीर्घता_32; ++I)
      आन्तरिकम्_State.बफर.L[I] = support::endian::read32be(&दत्त_2[I * 4]);
    hash_खण्डम्();
    दत्त_2 = दत्त_2.drop_front(खण्डम्_दीर्घता);
  }

  // Finish the remainder.
  प्रति (uint8_t C : दत्त_2)
    सङ्कलय_Uncounted(C);
}

शून्य SHA256::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<वर्ण *>(Str.दत्त()), Str.आकार_2()));
}

शून्य SHA256::pad() {
  // Implement SHA-2 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  सङ्कलय_Uncounted(0x80);
  यावत् (आन्तरिकम्_State.बफर_विस्थापनम् != 56)
    सङ्कलय_Uncounted(0x00);

  uint64_t len = आन्तरिकम्_State.Byte_गणनम् << 3; // bit size

  // Append length in the last 8 bytes big edian encoded
  सङ्कलय_Uncounted(len >> 56);
  सङ्कलय_Uncounted(len >> 48);
  सङ्कलय_Uncounted(len >> 40);
  सङ्कलय_Uncounted(len >> 32);
  सङ्कलय_Uncounted(len >> 24);
  सङ्कलय_Uncounted(len >> 16);
  सङ्कलय_Uncounted(len >> 8);
  सङ्कलय_Uncounted(len);
}

शून्य SHA256::final(std::array<uint32_t, HASH_दीर्घता / 4> &Hash_परिणाम) {
  // Pad to complete the last block
  pad();

  यदि constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    प्रति (पूर्णाङ्क i = 0; i < 8; i++) {
      Hash_परिणाम[i] = आन्तरिकम्_State.State[i];
    }
  } अन्यथा {
    // Swap byte order back
    प्रति (पूर्णाङ्क i = 0; i < 8; i++) {
      Hash_परिणाम[i] = llvm::byteswap(आन्तरिकम्_State.State[i]);
    }
  }
}

std::array<uint8_t, 32> SHA256::final() {
  सङ्घ {
    std::array<uint32_t, HASH_दीर्घता / 4> Hash_परिणाम;
    std::array<uint8_t, HASH_दीर्घता> Return_परिणाम;
  };
  static_assert(परिमाण(Hash_परिणाम) == परिमाण(Return_परिणाम));
  final(Hash_परिणाम);
  प्रत्यागम Return_परिणाम;
}

std::array<uint8_t, 32> SHA256::परिणाम_2() {
  स्वचालित State_गन्तव्यम्_Restore = आन्तरिकम्_State;

  स्वचालित Hash = final();

  // Restore the state
  आन्तरिकम्_State = State_गन्तव्यम्_Restore;

  // Return pointer to hash (32 characters)
  प्रत्यागम Hash;
}

std::array<uint8_t, 32> SHA256::hash(ArrayRef<uint8_t> दत्त_2) {
  SHA256 Hash;
  Hash.update(दत्त_2);
  प्रत्यागम Hash.final();
}

} // namespace llvm
