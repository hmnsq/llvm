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

#अन्तर्भाव "llvm/Support/MD5.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव <cstdint>
#अन्तर्भाव <cstring>

// The basic MD5 functions.

// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#परिभाषय F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#परिभाषय G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#परिभाषय H(x, y, z) ((x) ^ (y) ^ (z))
#परिभाषय I(x, y, z) ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#परिभाषय चरणः(f, a, b, c, d, x, t, s)                                           \
  (a) += f((b), (c), (d)) + (x) + (t);                                         \
  (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));                   \
  (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
#परिभाषय समुच्चयः_3(n)                                                                 \
  (आन्तरिकम्_State.खण्डम्[(n)] = (MD5_u32plus)ptr[(n)*4] |                        \
                              ((MD5_u32plus)ptr[(n)*4 + 1] << 8) |             \
                              ((MD5_u32plus)ptr[(n)*4 + 2] << 16) |            \
                              ((MD5_u32plus)ptr[(n)*4 + 3] << 24))
#परिभाषय GET(n) (आन्तरिकम्_State.खण्डम्[(n)])

उपयुज्य नामाकाश llvm;

/// This processes one or more 64-byte data blocks, but does NOT update
///the bit counters.  There are no alignment requirements.
ध्रुव uint8_t *MD5::body(ArrayRef<uint8_t> दत्त_2) {
  ध्रुव uint8_t *ptr;
  MD5_u32plus a, b, c, d;
  MD5_u32plus saved_a, saved_b, saved_c, saved_d;
  अचिह्नित दीर्घ आकार = दत्त_2.आकार_2();

  ptr = दत्त_2.दत्त();

  a = आन्तरिकम्_State.a;
  b = आन्तरिकम्_State.b;
  c = आन्तरिकम्_State.c;
  d = आन्तरिकम्_State.d;

  कुरु {
    saved_a = a;
    saved_b = b;
    saved_c = c;
    saved_d = d;

    // Round 1
    चरणः(F, a, b, c, d, समुच्चयः_3(0), 0xd76aa478, 7)
    चरणः(F, d, a, b, c, समुच्चयः_3(1), 0xe8c7b756, 12)
    चरणः(F, c, d, a, b, समुच्चयः_3(2), 0x242070db, 17)
    चरणः(F, b, c, d, a, समुच्चयः_3(3), 0xc1bdceee, 22)
    चरणः(F, a, b, c, d, समुच्चयः_3(4), 0xf57c0faf, 7)
    चरणः(F, d, a, b, c, समुच्चयः_3(5), 0x4787c62a, 12)
    चरणः(F, c, d, a, b, समुच्चयः_3(6), 0xa8304613, 17)
    चरणः(F, b, c, d, a, समुच्चयः_3(7), 0xfd469501, 22)
    चरणः(F, a, b, c, d, समुच्चयः_3(8), 0x698098d8, 7)
    चरणः(F, d, a, b, c, समुच्चयः_3(9), 0x8b44f7af, 12)
    चरणः(F, c, d, a, b, समुच्चयः_3(10), 0xffff5bb1, 17)
    चरणः(F, b, c, d, a, समुच्चयः_3(11), 0x895cd7be, 22)
    चरणः(F, a, b, c, d, समुच्चयः_3(12), 0x6b901122, 7)
    चरणः(F, d, a, b, c, समुच्चयः_3(13), 0xfd987193, 12)
    चरणः(F, c, d, a, b, समुच्चयः_3(14), 0xa679438e, 17)
    चरणः(F, b, c, d, a, समुच्चयः_3(15), 0x49b40821, 22)

    // Round 2
    चरणः(G, a, b, c, d, GET(1), 0xf61e2562, 5)
    चरणः(G, d, a, b, c, GET(6), 0xc040b340, 9)
    चरणः(G, c, d, a, b, GET(11), 0x265e5a51, 14)
    चरणः(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
    चरणः(G, a, b, c, d, GET(5), 0xd62f105d, 5)
    चरणः(G, d, a, b, c, GET(10), 0x02441453, 9)
    चरणः(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
    चरणः(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
    चरणः(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
    चरणः(G, d, a, b, c, GET(14), 0xc33707d6, 9)
    चरणः(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
    चरणः(G, b, c, d, a, GET(8), 0x455a14ed, 20)
    चरणः(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
    चरणः(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
    चरणः(G, c, d, a, b, GET(7), 0x676f02d9, 14)
    चरणः(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

    // Round 3
    चरणः(H, a, b, c, d, GET(5), 0xfffa3942, 4)
    चरणः(H, d, a, b, c, GET(8), 0x8771f681, 11)
    चरणः(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
    चरणः(H, b, c, d, a, GET(14), 0xfde5380c, 23)
    चरणः(H, a, b, c, d, GET(1), 0xa4beea44, 4)
    चरणः(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
    चरणः(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
    चरणः(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
    चरणः(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
    चरणः(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
    चरणः(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
    चरणः(H, b, c, d, a, GET(6), 0x04881d05, 23)
    चरणः(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
    चरणः(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
    चरणः(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
    चरणः(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

    // Round 4
    चरणः(I, a, b, c, d, GET(0), 0xf4292244, 6)
    चरणः(I, d, a, b, c, GET(7), 0x432aff97, 10)
    चरणः(I, c, d, a, b, GET(14), 0xab9423a7, 15)
    चरणः(I, b, c, d, a, GET(5), 0xfc93a039, 21)
    चरणः(I, a, b, c, d, GET(12), 0x655b59c3, 6)
    चरणः(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
    चरणः(I, c, d, a, b, GET(10), 0xffeff47d, 15)
    चरणः(I, b, c, d, a, GET(1), 0x85845dd1, 21)
    चरणः(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
    चरणः(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
    चरणः(I, c, d, a, b, GET(6), 0xa3014314, 15)
    चरणः(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
    चरणः(I, a, b, c, d, GET(4), 0xf7537e82, 6)
    चरणः(I, d, a, b, c, GET(11), 0xbd3af235, 10)
    चरणः(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
    चरणः(I, b, c, d, a, GET(9), 0xeb86d391, 21)

    a += saved_a;
    b += saved_b;
    c += saved_c;
    d += saved_d;

    ptr += 64;
  } यावत् (आकार -= 64);

  आन्तरिकम्_State.a = a;
  आन्तरिकम्_State.b = b;
  आन्तरिकम्_State.c = c;
  आन्तरिकम्_State.d = d;

  प्रत्यागम ptr;
}

MD5::MD5() = स्वभाव;

/// Incrementally add the bytes in \p Data to the hash.
शून्य MD5::update(ArrayRef<uint8_t> दत्त_2) {
  MD5_u32plus saved_lo;
  अचिह्नित दीर्घ प्रयुक्तम्, मुक्त;
  ध्रुव uint8_t *Ptr = दत्त_2.दत्त();
  अचिह्नित दीर्घ आकार = दत्त_2.आकार_2();

  saved_lo = आन्तरिकम्_State.lo;
  यदि ((आन्तरिकम्_State.lo = (saved_lo + आकार) & 0x1fffffff) < saved_lo)
    आन्तरिकम्_State.hi++;
  आन्तरिकम्_State.hi += आकार >> 29;

  प्रयुक्तम् = saved_lo & 0x3f;

  यदि (प्रयुक्तम्) {
    मुक्त = 64 - प्रयुक्तम्;

    यदि (आकार < मुक्त) {
      स्मृतिं_प्रतिलिख(&आन्तरिकम्_State.बफर_2[प्रयुक्तम्], Ptr, आकार);
      प्रत्यागम;
    }

    स्मृतिं_प्रतिलिख(&आन्तरिकम्_State.बफर_2[प्रयुक्तम्], Ptr, मुक्त);
    Ptr = Ptr + मुक्त;
    आकार -= मुक्त;
    body(ArrayRef(आन्तरिकम्_State.बफर_2, 64));
  }

  यदि (आकार >= 64) {
    Ptr = body(ArrayRef(Ptr, आकार & ~(अचिह्नित दीर्घ)0x3f));
    आकार &= 0x3f;
  }

  स्मृतिं_प्रतिलिख(आन्तरिकम्_State.बफर_2, Ptr, आकार);
}

/// Add the bytes in the StringRef \p Str to the hash.
// Note that this isn't a string and so this won't include any trailing NULL
// bytes.
शून्य MD5::update(StringRef Str) {
  ArrayRef<uint8_t> SVal((ध्रुव uint8_t *)Str.दत्त(), Str.आकार_2());
  update(SVal);
}

/// Finish the hash and place the resulting hash into \p result.
/// \param Result is assumed to be a minimum of 16-bytes in size.
शून्य MD5::final(MD5_परिणाम &परिणाम) {
  अचिह्नित दीर्घ प्रयुक्तम्, मुक्त;

  प्रयुक्तम् = आन्तरिकम्_State.lo & 0x3f;

  आन्तरिकम्_State.बफर_2[प्रयुक्तम्++] = 0x80;

  मुक्त = 64 - प्रयुक्तम्;

  यदि (मुक्त < 8) {
    स्मृतिं_पूरय(&आन्तरिकम्_State.बफर_2[प्रयुक्तम्], 0, मुक्त);
    body(ArrayRef(आन्तरिकम्_State.बफर_2, 64));
    प्रयुक्तम् = 0;
    मुक्त = 64;
  }

  स्मृतिं_पूरय(&आन्तरिकम्_State.बफर_2[प्रयुक्तम्], 0, मुक्त - 8);

  आन्तरिकम्_State.lo <<= 3;
  support::endian::write32le(&आन्तरिकम्_State.बफर_2[56], आन्तरिकम्_State.lo);
  support::endian::write32le(&आन्तरिकम्_State.बफर_2[60], आन्तरिकम्_State.hi);

  body(ArrayRef(आन्तरिकम्_State.बफर_2, 64));

  support::endian::write32le(&परिणाम[0], आन्तरिकम्_State.a);
  support::endian::write32le(&परिणाम[4], आन्तरिकम्_State.b);
  support::endian::write32le(&परिणाम[8], आन्तरिकम्_State.c);
  support::endian::write32le(&परिणाम[12], आन्तरिकम्_State.d);
}

MD5::MD5_परिणाम MD5::final() {
  MD5_परिणाम परिणाम;
  final(परिणाम);
  प्रत्यागम परिणाम;
}

MD5::MD5_परिणाम MD5::परिणाम_2() {
  स्वचालित State_गन्तव्यम्_Restore = आन्तरिकम्_State;

  स्वचालित Hash = final();

  // Restore the state
  आन्तरिकम्_State = State_गन्तव्यम्_Restore;

  प्रत्यागम Hash;
}

SmallString<32> MD5::MD5_परिणाम::digest() ध्रुव {
  SmallString<32> Str;
  गन्तव्यम्_Hex(*एतत्, /*LowerCase*/ सत्य, Str);
  प्रत्यागम Str;
}

शून्य MD5::stringify_परिणाम(MD5_परिणाम &परिणाम, Small_सदिशम्_Impl<वर्ण> &Str) {
  गन्तव्यम्_Hex(परिणाम, /*LowerCase*/ सत्य, Str);
}

MD5::MD5_परिणाम MD5::hash(ArrayRef<uint8_t> दत्त_2) {
  MD5 Hash;
  Hash.update(दत्त_2);
  MD5::MD5_परिणाम Res;
  Hash.final(Res);

  प्रत्यागम Res;
}

#अपरिभाषय F
#अपरिभाषय G
#अपरिभाषय H
#अपरिभाषय I
#अपरिभाषय चरणः
#अपरिभाषय समुच्चयः_3
#अपरिभाषय GET
