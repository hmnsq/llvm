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

#अन्तर्भाव "llvm/Support/SHA1.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"
#अन्तर्भाव <string.h>

उपयुज्य नामाकाश llvm;

स्थिर अन्तःस्थ uint32_t rol(uint32_t सङ्ख्या, पूर्णाङ्क द्वयङ्काः_2) {
  प्रत्यागम (सङ्ख्या << द्वयङ्काः_2) | (सङ्ख्या >> (32 - द्वयङ्काः_2));
}

स्थिर अन्तःस्थ uint32_t blk0(uint32_t *Buf, पूर्णाङ्क I) { प्रत्यागम Buf[I]; }

स्थिर अन्तःस्थ uint32_t blk(uint32_t *Buf, पूर्णाङ्क I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  प्रत्यागम Buf[I & 15];
}

स्थिर अन्तःस्थ शून्य r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णाङ्क I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

स्थिर अन्तःस्थ शून्य r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णाङ्क I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

स्थिर अन्तःस्थ शून्य r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णाङ्क I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

स्थिर अन्तःस्थ शून्य r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णाङ्क I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

स्थिर अन्तःस्थ शून्य r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, पूर्णाङ्क I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/*संहिता*/
#परिभाषय SHA1_K0 0x5a827999
#परिभाषय SHA1_K20 0x6ed9eba1
#परिभाषय SHA1_K40 0x8f1bbcdc
#परिभाषय SHA1_K60 0xca62c1d6

#परिभाषय बीजम्_0 0x67452301
#परिभाषय बीजम्_1 0xefcdab89
#परिभाषय बीजम्_2_2 0x98badcfe
#परिभाषय बीजम्_3 0x10325476
#परिभाषय बीजम्_4 0xc3d2e1f0

शून्य SHA1::init() {
  आन्तरिकम्_State.State[0] = बीजम्_0;
  आन्तरिकम्_State.State[1] = बीजम्_1;
  आन्तरिकम्_State.State[2] = बीजम्_2_2;
  आन्तरिकम्_State.State[3] = बीजम्_3;
  आन्तरिकम्_State.State[4] = बीजम्_4;
  आन्तरिकम्_State.Byte_गणनम् = 0;
  आन्तरिकम्_State.बफर_विस्थापनम् = 0;
}

शून्य SHA1::hash_खण्डम्() {
  uint32_t A = आन्तरिकम्_State.State[0];
  uint32_t B = आन्तरिकम्_State.State[1];
  uint32_t C = आन्तरिकम्_State.State[2];
  uint32_t D = आन्तरिकम्_State.State[3];
  uint32_t E = आन्तरिकम्_State.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, आन्तरिकम्_State.बफर.L);
  r0(E, A, B, C, D, 1, आन्तरिकम्_State.बफर.L);
  r0(D, E, A, B, C, 2, आन्तरिकम्_State.बफर.L);
  r0(C, D, E, A, B, 3, आन्तरिकम्_State.बफर.L);
  r0(B, C, D, E, A, 4, आन्तरिकम्_State.बफर.L);
  r0(A, B, C, D, E, 5, आन्तरिकम्_State.बफर.L);
  r0(E, A, B, C, D, 6, आन्तरिकम्_State.बफर.L);
  r0(D, E, A, B, C, 7, आन्तरिकम्_State.बफर.L);
  r0(C, D, E, A, B, 8, आन्तरिकम्_State.बफर.L);
  r0(B, C, D, E, A, 9, आन्तरिकम्_State.बफर.L);
  r0(A, B, C, D, E, 10, आन्तरिकम्_State.बफर.L);
  r0(E, A, B, C, D, 11, आन्तरिकम्_State.बफर.L);
  r0(D, E, A, B, C, 12, आन्तरिकम्_State.बफर.L);
  r0(C, D, E, A, B, 13, आन्तरिकम्_State.बफर.L);
  r0(B, C, D, E, A, 14, आन्तरिकम्_State.बफर.L);
  r0(A, B, C, D, E, 15, आन्तरिकम्_State.बफर.L);
  r1(E, A, B, C, D, 16, आन्तरिकम्_State.बफर.L);
  r1(D, E, A, B, C, 17, आन्तरिकम्_State.बफर.L);
  r1(C, D, E, A, B, 18, आन्तरिकम्_State.बफर.L);
  r1(B, C, D, E, A, 19, आन्तरिकम्_State.बफर.L);

  r2(A, B, C, D, E, 20, आन्तरिकम्_State.बफर.L);
  r2(E, A, B, C, D, 21, आन्तरिकम्_State.बफर.L);
  r2(D, E, A, B, C, 22, आन्तरिकम्_State.बफर.L);
  r2(C, D, E, A, B, 23, आन्तरिकम्_State.बफर.L);
  r2(B, C, D, E, A, 24, आन्तरिकम्_State.बफर.L);
  r2(A, B, C, D, E, 25, आन्तरिकम्_State.बफर.L);
  r2(E, A, B, C, D, 26, आन्तरिकम्_State.बफर.L);
  r2(D, E, A, B, C, 27, आन्तरिकम्_State.बफर.L);
  r2(C, D, E, A, B, 28, आन्तरिकम्_State.बफर.L);
  r2(B, C, D, E, A, 29, आन्तरिकम्_State.बफर.L);
  r2(A, B, C, D, E, 30, आन्तरिकम्_State.बफर.L);
  r2(E, A, B, C, D, 31, आन्तरिकम्_State.बफर.L);
  r2(D, E, A, B, C, 32, आन्तरिकम्_State.बफर.L);
  r2(C, D, E, A, B, 33, आन्तरिकम्_State.बफर.L);
  r2(B, C, D, E, A, 34, आन्तरिकम्_State.बफर.L);
  r2(A, B, C, D, E, 35, आन्तरिकम्_State.बफर.L);
  r2(E, A, B, C, D, 36, आन्तरिकम्_State.बफर.L);
  r2(D, E, A, B, C, 37, आन्तरिकम्_State.बफर.L);
  r2(C, D, E, A, B, 38, आन्तरिकम्_State.बफर.L);
  r2(B, C, D, E, A, 39, आन्तरिकम्_State.बफर.L);

  r3(A, B, C, D, E, 40, आन्तरिकम्_State.बफर.L);
  r3(E, A, B, C, D, 41, आन्तरिकम्_State.बफर.L);
  r3(D, E, A, B, C, 42, आन्तरिकम्_State.बफर.L);
  r3(C, D, E, A, B, 43, आन्तरिकम्_State.बफर.L);
  r3(B, C, D, E, A, 44, आन्तरिकम्_State.बफर.L);
  r3(A, B, C, D, E, 45, आन्तरिकम्_State.बफर.L);
  r3(E, A, B, C, D, 46, आन्तरिकम्_State.बफर.L);
  r3(D, E, A, B, C, 47, आन्तरिकम्_State.बफर.L);
  r3(C, D, E, A, B, 48, आन्तरिकम्_State.बफर.L);
  r3(B, C, D, E, A, 49, आन्तरिकम्_State.बफर.L);
  r3(A, B, C, D, E, 50, आन्तरिकम्_State.बफर.L);
  r3(E, A, B, C, D, 51, आन्तरिकम्_State.बफर.L);
  r3(D, E, A, B, C, 52, आन्तरिकम्_State.बफर.L);
  r3(C, D, E, A, B, 53, आन्तरिकम्_State.बफर.L);
  r3(B, C, D, E, A, 54, आन्तरिकम्_State.बफर.L);
  r3(A, B, C, D, E, 55, आन्तरिकम्_State.बफर.L);
  r3(E, A, B, C, D, 56, आन्तरिकम्_State.बफर.L);
  r3(D, E, A, B, C, 57, आन्तरिकम्_State.बफर.L);
  r3(C, D, E, A, B, 58, आन्तरिकम्_State.बफर.L);
  r3(B, C, D, E, A, 59, आन्तरिकम्_State.बफर.L);

  r4(A, B, C, D, E, 60, आन्तरिकम्_State.बफर.L);
  r4(E, A, B, C, D, 61, आन्तरिकम्_State.बफर.L);
  r4(D, E, A, B, C, 62, आन्तरिकम्_State.बफर.L);
  r4(C, D, E, A, B, 63, आन्तरिकम्_State.बफर.L);
  r4(B, C, D, E, A, 64, आन्तरिकम्_State.बफर.L);
  r4(A, B, C, D, E, 65, आन्तरिकम्_State.बफर.L);
  r4(E, A, B, C, D, 66, आन्तरिकम्_State.बफर.L);
  r4(D, E, A, B, C, 67, आन्तरिकम्_State.बफर.L);
  r4(C, D, E, A, B, 68, आन्तरिकम्_State.बफर.L);
  r4(B, C, D, E, A, 69, आन्तरिकम्_State.बफर.L);
  r4(A, B, C, D, E, 70, आन्तरिकम्_State.बफर.L);
  r4(E, A, B, C, D, 71, आन्तरिकम्_State.बफर.L);
  r4(D, E, A, B, C, 72, आन्तरिकम्_State.बफर.L);
  r4(C, D, E, A, B, 73, आन्तरिकम्_State.बफर.L);
  r4(B, C, D, E, A, 74, आन्तरिकम्_State.बफर.L);
  r4(A, B, C, D, E, 75, आन्तरिकम्_State.बफर.L);
  r4(E, A, B, C, D, 76, आन्तरिकम्_State.बफर.L);
  r4(D, E, A, B, C, 77, आन्तरिकम्_State.बफर.L);
  r4(C, D, E, A, B, 78, आन्तरिकम्_State.बफर.L);
  r4(B, C, D, E, A, 79, आन्तरिकम्_State.बफर.L);

  आन्तरिकम्_State.State[0] += A;
  आन्तरिकम्_State.State[1] += B;
  आन्तरिकम्_State.State[2] += C;
  आन्तरिकम्_State.State[3] += D;
  आन्तरिकम्_State.State[4] += E;
}

शून्य SHA1::सङ्कलय_Uncounted(uint8_t दत्त_2) {
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

शून्य SHA1::writebyte(uint8_t दत्त_2) {
  ++आन्तरिकम्_State.Byte_गणनम्;
  सङ्कलय_Uncounted(दत्त_2);
}

शून्य SHA1::update(ArrayRef<uint8_t> दत्त_2) {
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

शून्य SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<वर्ण *>(Str.दत्त()), Str.आकार_2()));
}

शून्य SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  सङ्कलय_Uncounted(0x80);
  यावत् (आन्तरिकम्_State.बफर_विस्थापनम् != 56)
    सङ्कलय_Uncounted(0x00);

  // Append length in the last 8 bytes
  सङ्कलय_Uncounted(0); // We're only using 32 bit lengths
  सङ्कलय_Uncounted(0); // But SHA-1 supports 64 bit lengths
  सङ्कलय_Uncounted(0); // So zero pad the top bits
  सङ्कलय_Uncounted(आन्तरिकम्_State.Byte_गणनम् >> 29); // Shifting to multiply by 8
  सङ्कलय_Uncounted(आन्तरिकम्_State.Byte_गणनम् >>
               21); // as SHA-1 supports bitstreams as well as
  सङ्कलय_Uncounted(आन्तरिकम्_State.Byte_गणनम् >> 13); // byte.
  सङ्कलय_Uncounted(आन्तरिकम्_State.Byte_गणनम् >> 5);
  सङ्कलय_Uncounted(आन्तरिकम्_State.Byte_गणनम् << 3);
}

शून्य SHA1::final(std::array<uint32_t, HASH_दीर्घता / 4> &Hash_परिणाम) {
  // Pad to complete the last block
  pad();

  यदि constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    प्रति (पूर्णाङ्क i = 0; i < 5; i++) {
      Hash_परिणाम[i] = आन्तरिकम्_State.State[i];
    }
  } अन्यथा {
    // Swap byte order back
    प्रति (पूर्णाङ्क i = 0; i < 5; i++) {
      Hash_परिणाम[i] = llvm::byteswap(आन्तरिकम्_State.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  सङ्घ {
    std::array<uint32_t, HASH_दीर्घता / 4> Hash_परिणाम;
    std::array<uint8_t, HASH_दीर्घता> Return_परिणाम;
  };
  static_assert(परिमाण(Hash_परिणाम) == परिमाण(Return_परिणाम));
  final(Hash_परिणाम);
  प्रत्यागम Return_परिणाम;
}

std::array<uint8_t, 20> SHA1::परिणाम_2() {
  स्वचालित State_गन्तव्यम्_Restore = आन्तरिकम्_State;

  स्वचालित Hash = final();

  // Restore the state
  आन्तरिकम्_State = State_गन्तव्यम्_Restore;

  // Return pointer to hash (20 characters)
  प्रत्यागम Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> दत्त_2) {
  SHA1 Hash;
  Hash.update(दत्त_2);
  प्रत्यागम Hash.final();
}
