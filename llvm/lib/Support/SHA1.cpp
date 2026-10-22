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

#شامل "llvm/Support/SHA1.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Endian.h"
#شامل "llvm/Support/SwapByteOrder.h"
#شامل <string.h>

using فضاینام llvm;

ایستا درونخطی uint32_t rol(uint32_t عدد, صحیح Bits) {
  بازگشت (عدد << Bits) | (عدد >> (32 - Bits));
}

ایستا درونخطی uint32_t blk0(uint32_t *Buf, صحیح I) { بازگشت Buf[I]; }

ایستا درونخطی uint32_t blk(uint32_t *Buf, صحیح I) {
  Buf[I & 15] = rol(Buf[(I + 13) & 15] ^ Buf[(I + 8) & 15] ^ Buf[(I + 2) & 15] ^
                        Buf[I & 15],
                    1);
  بازگشت Buf[I & 15];
}

ایستا درونخطی تهی r0(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحیح I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk0(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ایستا درونخطی تهی r1(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحیح I, uint32_t *Buf) {
  E += ((B & (C ^ D)) ^ D) + blk(Buf, I) + 0x5A827999 + rol(A, 5);
  B = rol(B, 30);
}

ایستا درونخطی تهی r2(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحیح I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0x6ED9EBA1 + rol(A, 5);
  B = rol(B, 30);
}

ایستا درونخطی تهی r3(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحیح I, uint32_t *Buf) {
  E += (((B | C) & D) | (B & C)) + blk(Buf, I) + 0x8F1BBCDC + rol(A, 5);
  B = rol(B, 30);
}

ایستا درونخطی تهی r4(uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D,
                      uint32_t &E, صحیح I, uint32_t *Buf) {
  E += (B ^ C ^ D) + blk(Buf, I) + 0xCA62C1D6 + rol(A, 5);
  B = rol(B, 30);
}

/* code */
#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

#define SEED_0 0x67452301
#define SEED_1 0xefcdab89
#define SEED_2 0x98badcfe
#define SEED_3 0x10325476
#define SEED_4 0xc3d2e1f0

تهی SHA1::init() {
  InternalState.State[0] = SEED_0;
  InternalState.State[1] = SEED_1;
  InternalState.State[2] = SEED_2;
  InternalState.State[3] = SEED_3;
  InternalState.State[4] = SEED_4;
  InternalState.Byte_شمار = 0;
  InternalState.BufferOffset = 0;
}

تهی SHA1::hashBlock() {
  uint32_t A = InternalState.State[0];
  uint32_t B = InternalState.State[1];
  uint32_t C = InternalState.State[2];
  uint32_t D = InternalState.State[3];
  uint32_t E = InternalState.State[4];

  // 4 rounds of 20 operations each. Loop unrolled.
  r0(A, B, C, D, E, 0, InternalState.Buffer.L);
  r0(E, A, B, C, D, 1, InternalState.Buffer.L);
  r0(D, E, A, B, C, 2, InternalState.Buffer.L);
  r0(C, D, E, A, B, 3, InternalState.Buffer.L);
  r0(B, C, D, E, A, 4, InternalState.Buffer.L);
  r0(A, B, C, D, E, 5, InternalState.Buffer.L);
  r0(E, A, B, C, D, 6, InternalState.Buffer.L);
  r0(D, E, A, B, C, 7, InternalState.Buffer.L);
  r0(C, D, E, A, B, 8, InternalState.Buffer.L);
  r0(B, C, D, E, A, 9, InternalState.Buffer.L);
  r0(A, B, C, D, E, 10, InternalState.Buffer.L);
  r0(E, A, B, C, D, 11, InternalState.Buffer.L);
  r0(D, E, A, B, C, 12, InternalState.Buffer.L);
  r0(C, D, E, A, B, 13, InternalState.Buffer.L);
  r0(B, C, D, E, A, 14, InternalState.Buffer.L);
  r0(A, B, C, D, E, 15, InternalState.Buffer.L);
  r1(E, A, B, C, D, 16, InternalState.Buffer.L);
  r1(D, E, A, B, C, 17, InternalState.Buffer.L);
  r1(C, D, E, A, B, 18, InternalState.Buffer.L);
  r1(B, C, D, E, A, 19, InternalState.Buffer.L);

  r2(A, B, C, D, E, 20, InternalState.Buffer.L);
  r2(E, A, B, C, D, 21, InternalState.Buffer.L);
  r2(D, E, A, B, C, 22, InternalState.Buffer.L);
  r2(C, D, E, A, B, 23, InternalState.Buffer.L);
  r2(B, C, D, E, A, 24, InternalState.Buffer.L);
  r2(A, B, C, D, E, 25, InternalState.Buffer.L);
  r2(E, A, B, C, D, 26, InternalState.Buffer.L);
  r2(D, E, A, B, C, 27, InternalState.Buffer.L);
  r2(C, D, E, A, B, 28, InternalState.Buffer.L);
  r2(B, C, D, E, A, 29, InternalState.Buffer.L);
  r2(A, B, C, D, E, 30, InternalState.Buffer.L);
  r2(E, A, B, C, D, 31, InternalState.Buffer.L);
  r2(D, E, A, B, C, 32, InternalState.Buffer.L);
  r2(C, D, E, A, B, 33, InternalState.Buffer.L);
  r2(B, C, D, E, A, 34, InternalState.Buffer.L);
  r2(A, B, C, D, E, 35, InternalState.Buffer.L);
  r2(E, A, B, C, D, 36, InternalState.Buffer.L);
  r2(D, E, A, B, C, 37, InternalState.Buffer.L);
  r2(C, D, E, A, B, 38, InternalState.Buffer.L);
  r2(B, C, D, E, A, 39, InternalState.Buffer.L);

  r3(A, B, C, D, E, 40, InternalState.Buffer.L);
  r3(E, A, B, C, D, 41, InternalState.Buffer.L);
  r3(D, E, A, B, C, 42, InternalState.Buffer.L);
  r3(C, D, E, A, B, 43, InternalState.Buffer.L);
  r3(B, C, D, E, A, 44, InternalState.Buffer.L);
  r3(A, B, C, D, E, 45, InternalState.Buffer.L);
  r3(E, A, B, C, D, 46, InternalState.Buffer.L);
  r3(D, E, A, B, C, 47, InternalState.Buffer.L);
  r3(C, D, E, A, B, 48, InternalState.Buffer.L);
  r3(B, C, D, E, A, 49, InternalState.Buffer.L);
  r3(A, B, C, D, E, 50, InternalState.Buffer.L);
  r3(E, A, B, C, D, 51, InternalState.Buffer.L);
  r3(D, E, A, B, C, 52, InternalState.Buffer.L);
  r3(C, D, E, A, B, 53, InternalState.Buffer.L);
  r3(B, C, D, E, A, 54, InternalState.Buffer.L);
  r3(A, B, C, D, E, 55, InternalState.Buffer.L);
  r3(E, A, B, C, D, 56, InternalState.Buffer.L);
  r3(D, E, A, B, C, 57, InternalState.Buffer.L);
  r3(C, D, E, A, B, 58, InternalState.Buffer.L);
  r3(B, C, D, E, A, 59, InternalState.Buffer.L);

  r4(A, B, C, D, E, 60, InternalState.Buffer.L);
  r4(E, A, B, C, D, 61, InternalState.Buffer.L);
  r4(D, E, A, B, C, 62, InternalState.Buffer.L);
  r4(C, D, E, A, B, 63, InternalState.Buffer.L);
  r4(B, C, D, E, A, 64, InternalState.Buffer.L);
  r4(A, B, C, D, E, 65, InternalState.Buffer.L);
  r4(E, A, B, C, D, 66, InternalState.Buffer.L);
  r4(D, E, A, B, C, 67, InternalState.Buffer.L);
  r4(C, D, E, A, B, 68, InternalState.Buffer.L);
  r4(B, C, D, E, A, 69, InternalState.Buffer.L);
  r4(A, B, C, D, E, 70, InternalState.Buffer.L);
  r4(E, A, B, C, D, 71, InternalState.Buffer.L);
  r4(D, E, A, B, C, 72, InternalState.Buffer.L);
  r4(C, D, E, A, B, 73, InternalState.Buffer.L);
  r4(B, C, D, E, A, 74, InternalState.Buffer.L);
  r4(A, B, C, D, E, 75, InternalState.Buffer.L);
  r4(E, A, B, C, D, 76, InternalState.Buffer.L);
  r4(D, E, A, B, C, 77, InternalState.Buffer.L);
  r4(C, D, E, A, B, 78, InternalState.Buffer.L);
  r4(B, C, D, E, A, 79, InternalState.Buffer.L);

  InternalState.State[0] += A;
  InternalState.State[1] += B;
  InternalState.State[2] += C;
  InternalState.State[3] += D;
  InternalState.State[4] += E;
}

تهی SHA1::addUncounted(uint8_t Data) {
  اگر constexpr (sys::IsBigEndianHost)
    InternalState.Buffer.C[InternalState.BufferOffset] = Data;
  وگرنه
    InternalState.Buffer.C[InternalState.BufferOffset ^ 3] = Data;

  InternalState.BufferOffset++;
  اگر (InternalState.BufferOffset == BLOCK_LENGTH) {
    hashBlock();
    InternalState.BufferOffset = 0;
  }
}

تهی SHA1::writebyte(uint8_t Data) {
  ++InternalState.Byte_شمار;
  addUncounted(Data);
}

تهی SHA1::update(ArrayRef<uint8_t> Data) {
  InternalState.Byte_شمار += Data.size();

  // Finish the current block.
  اگر (InternalState.BufferOffset > 0) {
    ثابت size_t Remainder = std::min<size_t>(
        Data.size(), BLOCK_LENGTH - InternalState.BufferOffset);
    برای (size_t I = 0; I < Remainder; ++I)
      addUncounted(Data[I]);
    Data = Data.drop_front(Remainder);
  }

  // Fast buffer filling for large inputs.
  وقتی (Data.size() >= BLOCK_LENGTH) {
    أكد(InternalState.BufferOffset == 0);
    static_assert(BLOCK_LENGTH % 4 == 0);
    constexpr size_t BLOCK_LENGTH_32 = BLOCK_LENGTH / 4;
    برای (size_t I = 0; I < BLOCK_LENGTH_32; ++I)
      InternalState.Buffer.L[I] = support::endian::read32be(&Data[I * 4]);
    hashBlock();
    Data = Data.drop_front(BLOCK_LENGTH);
  }

  // Finish the remainder.
  برای (uint8_t C : Data)
    addUncounted(C);
}

تهی SHA1::update(StringRef Str) {
  update(
      ArrayRef<uint8_t>((uint8_t *)const_cast<نویسه *>(Str.data()), Str.size()));
}

تهی SHA1::pad() {
  // Implement SHA-1 padding (fips180-2 5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  addUncounted(0x80);
  وقتی (InternalState.BufferOffset != 56)
    addUncounted(0x00);

  // Append length in the last 8 bytes
  addUncounted(0); // We're only using 32 bit lengths
  addUncounted(0); // But SHA-1 supports 64 bit lengths
  addUncounted(0); // So zero pad the top bits
  addUncounted(InternalState.Byte_شمار >> 29); // Shifting to multiply by 8
  addUncounted(InternalState.Byte_شمار >>
               21); // as SHA-1 supports bitstreams as well as
  addUncounted(InternalState.Byte_شمار >> 13); // byte.
  addUncounted(InternalState.Byte_شمار >> 5);
  addUncounted(InternalState.Byte_شمار << 3);
}

تهی SHA1::final(std::array<uint32_t, HASH_LENGTH / 4> &HashResult) {
  // Pad to complete the last block
  pad();

  اگر constexpr (sys::IsBigEndianHost) {
    // Just copy the current state
    برای (صحیح i = 0; i < 5; i++) {
      HashResult[i] = InternalState.State[i];
    }
  } وگرنه {
    // Swap byte order back
    برای (صحیح i = 0; i < 5; i++) {
      HashResult[i] = llvm::byteswap(InternalState.State[i]);
    }
  }
}

std::array<uint8_t, 20> SHA1::final() {
  اجتماع {
    std::array<uint32_t, HASH_LENGTH / 4> HashResult;
    std::array<uint8_t, HASH_LENGTH> ReturnResult;
  };
  static_assert(اندازه(HashResult) == اندازه(ReturnResult));
  final(HashResult);
  بازگشت ReturnResult;
}

std::array<uint8_t, 20> SHA1::result() {
  auto StateToRestore = InternalState;

  auto Hash = final();

  // Restore the state
  InternalState = StateToRestore;

  // Return pointer to hash (20 characters)
  بازگشت Hash;
}

std::array<uint8_t, 20> SHA1::hash(ArrayRef<uint8_t> Data) {
  SHA1 Hash;
  Hash.update(Data);
  بازگشت Hash.final();
}
