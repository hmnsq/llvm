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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MD5.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>

// The basic MD5 functions.

// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ H(x, y, z) ((x) ^ (y) ^ (z))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I(x, y, z) ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕਦਮ(f, a, b, c, d, x, t, s)                                           \
  (a) += f((b), (c), (d)) + (x) + (t);                                         \
  (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));                   \
  (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_3(n)                                                                 \
  (ਅੰਦਰੂਨੀ_State.ਖੰਡ[(n)] = (MD5_u32plus)ptr[(n)*4] |                        \
                              ((MD5_u32plus)ptr[(n)*4 + 1] << 8) |             \
                              ((MD5_u32plus)ptr[(n)*4 + 2] << 16) |            \
                              ((MD5_u32plus)ptr[(n)*4 + 3] << 24))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET(n) (ਅੰਦਰੂਨੀ_State.ਖੰਡ[(n)])

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// This processes one or more 64-byte data blocks, but does NOT update
///the bit counters.  There are no alignment requirements.
ਅਚਲ uint8_t *MD5::body(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  ਅਚਲ uint8_t *ptr;
  MD5_u32plus a, b, c, d;
  MD5_u32plus saved_a, saved_b, saved_c, saved_d;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ = ਅੰਕੜੇ_2.ਮਾਪ_2();

  ptr = ਅੰਕੜੇ_2.ਅੰਕੜੇ();

  a = ਅੰਦਰੂਨੀ_State.a;
  b = ਅੰਦਰੂਨੀ_State.b;
  c = ਅੰਦਰੂਨੀ_State.c;
  d = ਅੰਦਰੂਨੀ_State.d;

  ਕਰੋ {
    saved_a = a;
    saved_b = b;
    saved_c = c;
    saved_d = d;

    // Round 1
    ਕਦਮ(F, a, b, c, d, ਸੈੱਟ_3(0), 0xd76aa478, 7)
    ਕਦਮ(F, d, a, b, c, ਸੈੱਟ_3(1), 0xe8c7b756, 12)
    ਕਦਮ(F, c, d, a, b, ਸੈੱਟ_3(2), 0x242070db, 17)
    ਕਦਮ(F, b, c, d, a, ਸੈੱਟ_3(3), 0xc1bdceee, 22)
    ਕਦਮ(F, a, b, c, d, ਸੈੱਟ_3(4), 0xf57c0faf, 7)
    ਕਦਮ(F, d, a, b, c, ਸੈੱਟ_3(5), 0x4787c62a, 12)
    ਕਦਮ(F, c, d, a, b, ਸੈੱਟ_3(6), 0xa8304613, 17)
    ਕਦਮ(F, b, c, d, a, ਸੈੱਟ_3(7), 0xfd469501, 22)
    ਕਦਮ(F, a, b, c, d, ਸੈੱਟ_3(8), 0x698098d8, 7)
    ਕਦਮ(F, d, a, b, c, ਸੈੱਟ_3(9), 0x8b44f7af, 12)
    ਕਦਮ(F, c, d, a, b, ਸੈੱਟ_3(10), 0xffff5bb1, 17)
    ਕਦਮ(F, b, c, d, a, ਸੈੱਟ_3(11), 0x895cd7be, 22)
    ਕਦਮ(F, a, b, c, d, ਸੈੱਟ_3(12), 0x6b901122, 7)
    ਕਦਮ(F, d, a, b, c, ਸੈੱਟ_3(13), 0xfd987193, 12)
    ਕਦਮ(F, c, d, a, b, ਸੈੱਟ_3(14), 0xa679438e, 17)
    ਕਦਮ(F, b, c, d, a, ਸੈੱਟ_3(15), 0x49b40821, 22)

    // Round 2
    ਕਦਮ(G, a, b, c, d, GET(1), 0xf61e2562, 5)
    ਕਦਮ(G, d, a, b, c, GET(6), 0xc040b340, 9)
    ਕਦਮ(G, c, d, a, b, GET(11), 0x265e5a51, 14)
    ਕਦਮ(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
    ਕਦਮ(G, a, b, c, d, GET(5), 0xd62f105d, 5)
    ਕਦਮ(G, d, a, b, c, GET(10), 0x02441453, 9)
    ਕਦਮ(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
    ਕਦਮ(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
    ਕਦਮ(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
    ਕਦਮ(G, d, a, b, c, GET(14), 0xc33707d6, 9)
    ਕਦਮ(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
    ਕਦਮ(G, b, c, d, a, GET(8), 0x455a14ed, 20)
    ਕਦਮ(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
    ਕਦਮ(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
    ਕਦਮ(G, c, d, a, b, GET(7), 0x676f02d9, 14)
    ਕਦਮ(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

    // Round 3
    ਕਦਮ(H, a, b, c, d, GET(5), 0xfffa3942, 4)
    ਕਦਮ(H, d, a, b, c, GET(8), 0x8771f681, 11)
    ਕਦਮ(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
    ਕਦਮ(H, b, c, d, a, GET(14), 0xfde5380c, 23)
    ਕਦਮ(H, a, b, c, d, GET(1), 0xa4beea44, 4)
    ਕਦਮ(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
    ਕਦਮ(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
    ਕਦਮ(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
    ਕਦਮ(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
    ਕਦਮ(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
    ਕਦਮ(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
    ਕਦਮ(H, b, c, d, a, GET(6), 0x04881d05, 23)
    ਕਦਮ(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
    ਕਦਮ(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
    ਕਦਮ(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
    ਕਦਮ(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

    // Round 4
    ਕਦਮ(I, a, b, c, d, GET(0), 0xf4292244, 6)
    ਕਦਮ(I, d, a, b, c, GET(7), 0x432aff97, 10)
    ਕਦਮ(I, c, d, a, b, GET(14), 0xab9423a7, 15)
    ਕਦਮ(I, b, c, d, a, GET(5), 0xfc93a039, 21)
    ਕਦਮ(I, a, b, c, d, GET(12), 0x655b59c3, 6)
    ਕਦਮ(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
    ਕਦਮ(I, c, d, a, b, GET(10), 0xffeff47d, 15)
    ਕਦਮ(I, b, c, d, a, GET(1), 0x85845dd1, 21)
    ਕਦਮ(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
    ਕਦਮ(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
    ਕਦਮ(I, c, d, a, b, GET(6), 0xa3014314, 15)
    ਕਦਮ(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
    ਕਦਮ(I, a, b, c, d, GET(4), 0xf7537e82, 6)
    ਕਦਮ(I, d, a, b, c, GET(11), 0xbd3af235, 10)
    ਕਦਮ(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
    ਕਦਮ(I, b, c, d, a, GET(9), 0xeb86d391, 21)

    a += saved_a;
    b += saved_b;
    c += saved_c;
    d += saved_d;

    ptr += 64;
  } ਜਦੋਂ (ਮਾਪ -= 64);

  ਅੰਦਰੂਨੀ_State.a = a;
  ਅੰਦਰੂਨੀ_State.b = b;
  ਅੰਦਰੂਨੀ_State.c = c;
  ਅੰਦਰੂਨੀ_State.d = d;

  ਵਾਪਸ ptr;
}

MD5::MD5() = ਮੂਲ;

/// Incrementally add the bytes in \p Data to the hash.
ਖਾਲੀ MD5::update(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  MD5_u32plus saved_lo;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਵਰਤਿਆ, ਮੁਕਤ;
  ਅਚਲ uint8_t *Ptr = ਅੰਕੜੇ_2.ਅੰਕੜੇ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ = ਅੰਕੜੇ_2.ਮਾਪ_2();

  saved_lo = ਅੰਦਰੂਨੀ_State.lo;
  ਜੇ ((ਅੰਦਰੂਨੀ_State.lo = (saved_lo + ਮਾਪ) & 0x1fffffff) < saved_lo)
    ਅੰਦਰੂਨੀ_State.hi++;
  ਅੰਦਰੂਨੀ_State.hi += ਮਾਪ >> 29;

  ਵਰਤਿਆ = saved_lo & 0x3f;

  ਜੇ (ਵਰਤਿਆ) {
    ਮੁਕਤ = 64 - ਵਰਤਿਆ;

    ਜੇ (ਮਾਪ < ਮੁਕਤ) {
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[ਵਰਤਿਆ], Ptr, ਮਾਪ);
      ਵਾਪਸ;
    }

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[ਵਰਤਿਆ], Ptr, ਮੁਕਤ);
    Ptr = Ptr + ਮੁਕਤ;
    ਮਾਪ -= ਮੁਕਤ;
    body(ArrayRef(ਅੰਦਰੂਨੀ_State.ਬਫਰ_2, 64));
  }

  ਜੇ (ਮਾਪ >= 64) {
    Ptr = body(ArrayRef(Ptr, ਮਾਪ & ~(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)0x3f));
    ਮਾਪ &= 0x3f;
  }

  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਦਰੂਨੀ_State.ਬਫਰ_2, Ptr, ਮਾਪ);
}

/// Add the bytes in the StringRef \p Str to the hash.
// Note that this isn't a string and so this won't include any trailing NULL
// bytes.
ਖਾਲੀ MD5::update(StringRef Str) {
  ArrayRef<uint8_t> SVal((ਅਚਲ uint8_t *)Str.ਅੰਕੜੇ(), Str.ਮਾਪ_2());
  update(SVal);
}

/// Finish the hash and place the resulting hash into \p result.
/// \param Result is assumed to be a minimum of 16-bytes in size.
ਖਾਲੀ MD5::final(MD5_ਨਤੀਜਾ &ਨਤੀਜਾ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਵਰਤਿਆ, ਮੁਕਤ;

  ਵਰਤਿਆ = ਅੰਦਰੂਨੀ_State.lo & 0x3f;

  ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[ਵਰਤਿਆ++] = 0x80;

  ਮੁਕਤ = 64 - ਵਰਤਿਆ;

  ਜੇ (ਮੁਕਤ < 8) {
    memset(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[ਵਰਤਿਆ], 0, ਮੁਕਤ);
    body(ArrayRef(ਅੰਦਰੂਨੀ_State.ਬਫਰ_2, 64));
    ਵਰਤਿਆ = 0;
    ਮੁਕਤ = 64;
  }

  memset(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[ਵਰਤਿਆ], 0, ਮੁਕਤ - 8);

  ਅੰਦਰੂਨੀ_State.lo <<= 3;
  support::endian::write32le(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[56], ਅੰਦਰੂਨੀ_State.lo);
  support::endian::write32le(&ਅੰਦਰੂਨੀ_State.ਬਫਰ_2[60], ਅੰਦਰੂਨੀ_State.hi);

  body(ArrayRef(ਅੰਦਰੂਨੀ_State.ਬਫਰ_2, 64));

  support::endian::write32le(&ਨਤੀਜਾ[0], ਅੰਦਰੂਨੀ_State.a);
  support::endian::write32le(&ਨਤੀਜਾ[4], ਅੰਦਰੂਨੀ_State.b);
  support::endian::write32le(&ਨਤੀਜਾ[8], ਅੰਦਰੂਨੀ_State.c);
  support::endian::write32le(&ਨਤੀਜਾ[12], ਅੰਦਰੂਨੀ_State.d);
}

MD5::MD5_ਨਤੀਜਾ MD5::final() {
  MD5_ਨਤੀਜਾ ਨਤੀਜਾ;
  final(ਨਤੀਜਾ);
  ਵਾਪਸ ਨਤੀਜਾ;
}

MD5::MD5_ਨਤੀਜਾ MD5::ਨਤੀਜਾ_2() {
  ਸਵੈਚਾਲਿਤ State_ਮੰਜ਼ਿਲ_Restore = ਅੰਦਰੂਨੀ_State;

  ਸਵੈਚਾਲਿਤ Hash = final();

  // Restore the state
  ਅੰਦਰੂਨੀ_State = State_ਮੰਜ਼ਿਲ_Restore;

  ਵਾਪਸ Hash;
}

SmallString<32> MD5::MD5_ਨਤੀਜਾ::digest() ਅਚਲ {
  SmallString<32> Str;
  ਮੰਜ਼ਿਲ_Hex(*ਇਹ, /*LowerCase*/ ਸੱਚ, Str);
  ਵਾਪਸ Str;
}

ਖਾਲੀ MD5::stringify_ਨਤੀਜਾ(MD5_ਨਤੀਜਾ &ਨਤੀਜਾ, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str) {
  ਮੰਜ਼ਿਲ_Hex(ਨਤੀਜਾ, /*LowerCase*/ ਸੱਚ, Str);
}

MD5::MD5_ਨਤੀਜਾ MD5::hash(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  MD5 Hash;
  Hash.update(ਅੰਕੜੇ_2);
  MD5::MD5_ਨਤੀਜਾ Res;
  Hash.final(Res);

  ਵਾਪਸ Res;
}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ F
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ G
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ H
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ I
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਕਦਮ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸੈੱਟ_3
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ GET
