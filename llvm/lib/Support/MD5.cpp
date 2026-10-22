/*
 * This code is derived from (original license follows):
 *
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * (This is a heavily cut-down "BSD license".)
 *
 * This differs from Colin Plumb's older public domain implementation in that
 * no exactly 32-bit integer data type is required (any 32-bit or wider
 * unsigned integer data type will do), there's no compile-time endianness
 * configuration, and the function prototypes match OpenSSL's.  No code from
 * Colin Plumb's implementation has been reused; this comment merely compares
 * the properties of the two independent implementations.
 *
 * The primary goals of this implementation are portability and ease of use.
 * It is meant to be fast, but not as fast as possible.  Some known
 * optimizations are not included to reduce source code size and avoid
 * compile-time configuration.
 */

#समावेश "llvm/Support/MD5.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Endian.h"
#समावेश <cstdint>
#समावेश <cstring>

// The basic MD5 functions.

// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#घोषणा F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#घोषणा G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#घोषणा H(x, y, z) ((x) ^ (y) ^ (z))
#घोषणा I(x, y, z) ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#घोषणा STEP(f, a, b, c, d, x, t, s)                                           \
  (a) += f((b), (c), (d)) + (x) + (t);                                         \
  (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));                   \
  (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
#घोषणा SET(n)                                                                 \
  (InternalState.block[(n)] = (MD5_u32plus)ptr[(n)*4] |                        \
                              ((MD5_u32plus)ptr[(n)*4 + 1] << 8) |             \
                              ((MD5_u32plus)ptr[(n)*4 + 2] << 16) |            \
                              ((MD5_u32plus)ptr[(n)*4 + 3] << 24))
#घोषणा GET(n) (InternalState.block[(n)])

using namespace llvm;

/// This processes one or more 64-byte data blocks, but does NOT update
///the bit counters.  There are no alignment requirements.
स्थिर uint8_t *MD5::body(ArrayRef<uint8_t> तथ्य_2) {
  स्थिर uint8_t *ptr;
  MD5_u32plus a, b, c, d;
  MD5_u32plus saved_a, saved_b, saved_c, saved_d;
  अचिन्हित दीर्घ आकार = तथ्य_2.आकार_2();

  ptr = तथ्य_2.तथ्य();

  a = InternalState.a;
  b = InternalState.b;
  c = InternalState.c;
  d = InternalState.d;

  करो {
    saved_a = a;
    saved_b = b;
    saved_c = c;
    saved_d = d;

    // Round 1
    STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
    STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
    STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
    STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
    STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
    STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
    STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
    STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
    STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
    STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
    STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
    STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
    STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
    STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
    STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
    STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

    // Round 2
    STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
    STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
    STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
    STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
    STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
    STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
    STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
    STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
    STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
    STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
    STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
    STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
    STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
    STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
    STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
    STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

    // Round 3
    STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
    STEP(H, d, a, b, c, GET(8), 0x8771f681, 11)
    STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
    STEP(H, b, c, d, a, GET(14), 0xfde5380c, 23)
    STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
    STEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
    STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
    STEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
    STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
    STEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
    STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
    STEP(H, b, c, d, a, GET(6), 0x04881d05, 23)
    STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
    STEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
    STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
    STEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

    // Round 4
    STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
    STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
    STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
    STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
    STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
    STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
    STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
    STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
    STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
    STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
    STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
    STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
    STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
    STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
    STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
    STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

    a += saved_a;
    b += saved_b;
    c += saved_c;
    d += saved_d;

    ptr += 64;
  } जबतक (आकार -= 64);

  InternalState.a = a;
  InternalState.b = b;
  InternalState.c = c;
  InternalState.d = d;

  वापस ptr;
}

MD5::MD5() = शेष;

/// Incrementally add the bytes in \p Data to the hash.
व्योम MD5::update(ArrayRef<uint8_t> तथ्य_2) {
  MD5_u32plus saved_lo;
  अचिन्हित दीर्घ used, मुक्त;
  स्थिर uint8_t *Ptr = तथ्य_2.तथ्य();
  अचिन्हित दीर्घ आकार = तथ्य_2.आकार_2();

  saved_lo = InternalState.lo;
  अगर ((InternalState.lo = (saved_lo + आकार) & 0x1fffffff) < saved_lo)
    InternalState.hi++;
  InternalState.hi += आकार >> 29;

  used = saved_lo & 0x3f;

  अगर (used) {
    मुक्त = 64 - used;

    अगर (आकार < मुक्त) {
      स_नकल(&InternalState.बफर_2[used], Ptr, आकार);
      वापस;
    }

    स_नकल(&InternalState.बफर_2[used], Ptr, मुक्त);
    Ptr = Ptr + मुक्त;
    आकार -= मुक्त;
    body(ArrayRef(InternalState.बफर_2, 64));
  }

  अगर (आकार >= 64) {
    Ptr = body(ArrayRef(Ptr, आकार & ~(अचिन्हित दीर्घ)0x3f));
    आकार &= 0x3f;
  }

  स_नकल(InternalState.बफर_2, Ptr, आकार);
}

/// Add the bytes in the StringRef \p Str to the hash.
// Note that this isn't a string and so this won't include any trailing NULL
// bytes.
व्योम MD5::update(StringRef Str) {
  ArrayRef<uint8_t> SVal((स्थिर uint8_t *)Str.तथ्य(), Str.आकार_2());
  update(SVal);
}

/// Finish the hash and place the resulting hash into \p result.
/// \param Result is assumed to be a minimum of 16-bytes in size.
व्योम MD5::final(MD5_परिणाम &परिणाम) {
  अचिन्हित दीर्घ used, मुक्त;

  used = InternalState.lo & 0x3f;

  InternalState.बफर_2[used++] = 0x80;

  मुक्त = 64 - used;

  अगर (मुक्त < 8) {
    स_रखो(&InternalState.बफर_2[used], 0, मुक्त);
    body(ArrayRef(InternalState.बफर_2, 64));
    used = 0;
    मुक्त = 64;
  }

  स_रखो(&InternalState.बफर_2[used], 0, मुक्त - 8);

  InternalState.lo <<= 3;
  support::endian::write32le(&InternalState.बफर_2[56], InternalState.lo);
  support::endian::write32le(&InternalState.बफर_2[60], InternalState.hi);

  body(ArrayRef(InternalState.बफर_2, 64));

  support::endian::write32le(&परिणाम[0], InternalState.a);
  support::endian::write32le(&परिणाम[4], InternalState.b);
  support::endian::write32le(&परिणाम[8], InternalState.c);
  support::endian::write32le(&परिणाम[12], InternalState.d);
}

MD5::MD5_परिणाम MD5::final() {
  MD5_परिणाम परिणाम;
  final(परिणाम);
  वापस परिणाम;
}

MD5::MD5_परिणाम MD5::परिणाम_2() {
  स्वतः StateToRestore = InternalState;

  स्वतः Hash = final();

  // Restore the state
  InternalState = StateToRestore;

  वापस Hash;
}

SmallString<32> MD5::MD5_परिणाम::digest() स्थिर {
  SmallString<32> Str;
  toHex(*यह, /*LowerCase*/ true, Str);
  वापस Str;
}

व्योम MD5::stringify_परिणाम(MD5_परिणाम &परिणाम, SmallVectorImpl<अक्षर> &Str) {
  toHex(परिणाम, /*LowerCase*/ true, Str);
}

MD5::MD5_परिणाम MD5::hash(ArrayRef<uint8_t> तथ्य_2) {
  MD5 Hash;
  Hash.update(तथ्य_2);
  MD5::MD5_परिणाम Res;
  Hash.final(Res);

  वापस Res;
}

#अघोषित F
#अघोषित G
#अघोषित H
#अघोषित I
#अघोषित STEP
#अघोषित SET
#अघोषित GET
