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

#ضمن "llvm/Support/MD5.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Endian.h"
#ضمن <cstdint>
#ضمن <cstring>

// The basic MD5 functions.

// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#تعریف_کن F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#تعریف_کن G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#تعریف_کن H(x, y, z) ((x) ^ (y) ^ (z))
#تعریف_کن I(x, y, z) ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#تعریف_کن گام(f, a, b, c, d, x, t, s)                                           \
  (a) += f((b), (c), (d)) + (x) + (t);                                         \
  (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));                   \
  (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
#تعریف_کن مجموعه_3(n)                                                                 \
  (داخلی_State.بلوک[(n)] = (MD5_u32plus)ptr[(n)*4] |                        \
                              ((MD5_u32plus)ptr[(n)*4 + 1] << 8) |             \
                              ((MD5_u32plus)ptr[(n)*4 + 2] << 16) |            \
                              ((MD5_u32plus)ptr[(n)*4 + 3] << 24))
#تعریف_کن GET(n) (داخلی_State.بلوک[(n)])

استفاده نطاق llvm;

/// This processes one or more 64-byte data blocks, but does NOT update
///the bit counters.  There are no alignment requirements.
ثابت uint8_t *MD5::body(ArrayRef<uint8_t> داده_2) {
  ثابت uint8_t *ptr;
  MD5_u32plus a, b, c, d;
  MD5_u32plus saved_a, saved_b, saved_c, saved_d;
  بیعلامت دراز اندازه = داده_2.اندازه_2();

  ptr = داده_2.داده();

  a = داخلی_State.a;
  b = داخلی_State.b;
  c = داخلی_State.c;
  d = داخلی_State.d;

  افعل {
    saved_a = a;
    saved_b = b;
    saved_c = c;
    saved_d = d;

    // Round 1
    گام(F, a, b, c, d, مجموعه_3(0), 0xd76aa478, 7)
    گام(F, d, a, b, c, مجموعه_3(1), 0xe8c7b756, 12)
    گام(F, c, d, a, b, مجموعه_3(2), 0x242070db, 17)
    گام(F, b, c, d, a, مجموعه_3(3), 0xc1bdceee, 22)
    گام(F, a, b, c, d, مجموعه_3(4), 0xf57c0faf, 7)
    گام(F, d, a, b, c, مجموعه_3(5), 0x4787c62a, 12)
    گام(F, c, d, a, b, مجموعه_3(6), 0xa8304613, 17)
    گام(F, b, c, d, a, مجموعه_3(7), 0xfd469501, 22)
    گام(F, a, b, c, d, مجموعه_3(8), 0x698098d8, 7)
    گام(F, d, a, b, c, مجموعه_3(9), 0x8b44f7af, 12)
    گام(F, c, d, a, b, مجموعه_3(10), 0xffff5bb1, 17)
    گام(F, b, c, d, a, مجموعه_3(11), 0x895cd7be, 22)
    گام(F, a, b, c, d, مجموعه_3(12), 0x6b901122, 7)
    گام(F, d, a, b, c, مجموعه_3(13), 0xfd987193, 12)
    گام(F, c, d, a, b, مجموعه_3(14), 0xa679438e, 17)
    گام(F, b, c, d, a, مجموعه_3(15), 0x49b40821, 22)

    // Round 2
    گام(G, a, b, c, d, GET(1), 0xf61e2562, 5)
    گام(G, d, a, b, c, GET(6), 0xc040b340, 9)
    گام(G, c, d, a, b, GET(11), 0x265e5a51, 14)
    گام(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
    گام(G, a, b, c, d, GET(5), 0xd62f105d, 5)
    گام(G, d, a, b, c, GET(10), 0x02441453, 9)
    گام(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
    گام(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
    گام(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
    گام(G, d, a, b, c, GET(14), 0xc33707d6, 9)
    گام(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
    گام(G, b, c, d, a, GET(8), 0x455a14ed, 20)
    گام(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
    گام(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
    گام(G, c, d, a, b, GET(7), 0x676f02d9, 14)
    گام(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

    // Round 3
    گام(H, a, b, c, d, GET(5), 0xfffa3942, 4)
    گام(H, d, a, b, c, GET(8), 0x8771f681, 11)
    گام(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
    گام(H, b, c, d, a, GET(14), 0xfde5380c, 23)
    گام(H, a, b, c, d, GET(1), 0xa4beea44, 4)
    گام(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
    گام(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
    گام(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
    گام(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
    گام(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
    گام(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
    گام(H, b, c, d, a, GET(6), 0x04881d05, 23)
    گام(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
    گام(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
    گام(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
    گام(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

    // Round 4
    گام(I, a, b, c, d, GET(0), 0xf4292244, 6)
    گام(I, d, a, b, c, GET(7), 0x432aff97, 10)
    گام(I, c, d, a, b, GET(14), 0xab9423a7, 15)
    گام(I, b, c, d, a, GET(5), 0xfc93a039, 21)
    گام(I, a, b, c, d, GET(12), 0x655b59c3, 6)
    گام(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
    گام(I, c, d, a, b, GET(10), 0xffeff47d, 15)
    گام(I, b, c, d, a, GET(1), 0x85845dd1, 21)
    گام(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
    گام(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
    گام(I, c, d, a, b, GET(6), 0xa3014314, 15)
    گام(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
    گام(I, a, b, c, d, GET(4), 0xf7537e82, 6)
    گام(I, d, a, b, c, GET(11), 0xbd3af235, 10)
    گام(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
    گام(I, b, c, d, a, GET(9), 0xeb86d391, 21)

    a += saved_a;
    b += saved_b;
    c += saved_c;
    d += saved_d;

    ptr += 64;
  } طالما (اندازه -= 64);

  داخلی_State.a = a;
  داخلی_State.b = b;
  داخلی_State.c = c;
  داخلی_State.d = d;

  أرجع ptr;
}

MD5::MD5() = افتراضي;

/// Incrementally add the bytes in \p Data to the hash.
فراغ MD5::update(ArrayRef<uint8_t> داده_2) {
  MD5_u32plus saved_lo;
  بیعلامت دراز استفاده_شده, حرر;
  ثابت uint8_t *Ptr = داده_2.داده();
  بیعلامت دراز اندازه = داده_2.اندازه_2();

  saved_lo = داخلی_State.lo;
  إذا ((داخلی_State.lo = (saved_lo + اندازه) & 0x1fffffff) < saved_lo)
    داخلی_State.hi++;
  داخلی_State.hi += اندازه >> 29;

  استفاده_شده = saved_lo & 0x3f;

  إذا (استفاده_شده) {
    حرر = 64 - استفاده_شده;

    إذا (اندازه < حرر) {
      کپی_حافظه(&داخلی_State.میانگیر_2[استفاده_شده], Ptr, اندازه);
      أرجع;
    }

    کپی_حافظه(&داخلی_State.میانگیر_2[استفاده_شده], Ptr, حرر);
    Ptr = Ptr + حرر;
    اندازه -= حرر;
    body(ArrayRef(داخلی_State.میانگیر_2, 64));
  }

  إذا (اندازه >= 64) {
    Ptr = body(ArrayRef(Ptr, اندازه & ~(بیعلامت دراز)0x3f));
    اندازه &= 0x3f;
  }

  کپی_حافظه(داخلی_State.میانگیر_2, Ptr, اندازه);
}

/// Add the bytes in the StringRef \p Str to the hash.
// Note that this isn't a string and so this won't include any trailing NULL
// bytes.
فراغ MD5::update(StringRef Str) {
  ArrayRef<uint8_t> SVal((ثابت uint8_t *)Str.داده(), Str.اندازه_2());
  update(SVal);
}

/// Finish the hash and place the resulting hash into \p result.
/// \param Result is assumed to be a minimum of 16-bytes in size.
فراغ MD5::final(MD5_نتیجه &نتیجه) {
  بیعلامت دراز استفاده_شده, حرر;

  استفاده_شده = داخلی_State.lo & 0x3f;

  داخلی_State.میانگیر_2[استفاده_شده++] = 0x80;

  حرر = 64 - استفاده_شده;

  إذا (حرر < 8) {
    memset(&داخلی_State.میانگیر_2[استفاده_شده], 0, حرر);
    body(ArrayRef(داخلی_State.میانگیر_2, 64));
    استفاده_شده = 0;
    حرر = 64;
  }

  memset(&داخلی_State.میانگیر_2[استفاده_شده], 0, حرر - 8);

  داخلی_State.lo <<= 3;
  support::endian::write32le(&داخلی_State.میانگیر_2[56], داخلی_State.lo);
  support::endian::write32le(&داخلی_State.میانگیر_2[60], داخلی_State.hi);

  body(ArrayRef(داخلی_State.میانگیر_2, 64));

  support::endian::write32le(&نتیجه[0], داخلی_State.a);
  support::endian::write32le(&نتیجه[4], داخلی_State.b);
  support::endian::write32le(&نتیجه[8], داخلی_State.c);
  support::endian::write32le(&نتیجه[12], داخلی_State.d);
}

MD5::MD5_نتیجه MD5::final() {
  MD5_نتیجه نتیجه;
  final(نتیجه);
  أرجع نتیجه;
}

MD5::MD5_نتیجه MD5::نتیجه_2() {
  خودکار State_مقصد_Restore = داخلی_State;

  خودکار Hash = final();

  // Restore the state
  داخلی_State = State_مقصد_Restore;

  أرجع Hash;
}

SmallString<32> MD5::MD5_نتیجه::digest() ثابت {
  SmallString<32> Str;
  مقصد_Hex(*هذا, /*LowerCase*/ درست, Str);
  أرجع Str;
}

فراغ MD5::stringify_نتیجه(MD5_نتیجه &نتیجه, Small_وکتور_Impl<حرف> &Str) {
  مقصد_Hex(نتیجه, /*LowerCase*/ درست, Str);
}

MD5::MD5_نتیجه MD5::hash(ArrayRef<uint8_t> داده_2) {
  MD5 Hash;
  Hash.update(داده_2);
  MD5::MD5_نتیجه Res;
  Hash.final(Res);

  أرجع Res;
}

#لغو_تعریف F
#لغو_تعریف G
#لغو_تعریف H
#لغو_تعریف I
#لغو_تعریف گام
#لغو_تعریف مجموعه_3
#لغو_تعریف GET
