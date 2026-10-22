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

#అన్తర్భావ "llvm/Support/MD5.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Endian.h"
#అన్తర్భావ <cstdint>
#అన్తర్భావ <cstring>

// The basic MD5 functions.

// F and G are optimized compared to their RFC 1321 definitions for
// architectures that lack an AND-NOT instruction, just like in Colin Plumb's
// implementation.
#నిర్వచించు F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#నిర్వచించు G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#నిర్వచించు H(x, y, z) ((x) ^ (y) ^ (z))
#నిర్వచించు I(x, y, z) ((y) ^ ((x) | ~(z)))

// The MD5 transformation for all four rounds.
#నిర్వచించు అడుగు(f, a, b, c, d, x, t, s)                                           \
  (a) += f((b), (c), (d)) + (x) + (t);                                         \
  (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));                   \
  (a) += (b);

// SET reads 4 input bytes in little-endian byte order and stores them
// in a properly aligned word in host byte order.
#నిర్వచించు సమితి_3(n)                                                                 \
  (అంతర్గత_State.ఖండం[(n)] = (MD5_u32plus)ptr[(n)*4] |                        \
                              ((MD5_u32plus)ptr[(n)*4 + 1] << 8) |             \
                              ((MD5_u32plus)ptr[(n)*4 + 2] << 16) |            \
                              ((MD5_u32plus)ptr[(n)*4 + 3] << 24))
#నిర్వచించు GET(n) (అంతర్గత_State.ఖండం[(n)])

ఉపయోగించి నామాకాశ llvm;

/// This processes one or more 64-byte data blocks, but does NOT update
///the bit counters.  There are no alignment requirements.
స్థిరరాశి uint8_t *MD5::body(ArrayRef<uint8_t> దత్తాంశం_2) {
  స్థిరరాశి uint8_t *ptr;
  MD5_u32plus a, b, c, d;
  MD5_u32plus saved_a, saved_b, saved_c, saved_d;
  గుర్తులేని దీర్ఘ కొలత = దత్తాంశం_2.కొలత_2();

  ptr = దత్తాంశం_2.దత్తాంశం();

  a = అంతర్గత_State.a;
  b = అంతర్గత_State.b;
  c = అంతర్గత_State.c;
  d = అంతర్గత_State.d;

  చేయి {
    saved_a = a;
    saved_b = b;
    saved_c = c;
    saved_d = d;

    // Round 1
    అడుగు(F, a, b, c, d, సమితి_3(0), 0xd76aa478, 7)
    అడుగు(F, d, a, b, c, సమితి_3(1), 0xe8c7b756, 12)
    అడుగు(F, c, d, a, b, సమితి_3(2), 0x242070db, 17)
    అడుగు(F, b, c, d, a, సమితి_3(3), 0xc1bdceee, 22)
    అడుగు(F, a, b, c, d, సమితి_3(4), 0xf57c0faf, 7)
    అడుగు(F, d, a, b, c, సమితి_3(5), 0x4787c62a, 12)
    అడుగు(F, c, d, a, b, సమితి_3(6), 0xa8304613, 17)
    అడుగు(F, b, c, d, a, సమితి_3(7), 0xfd469501, 22)
    అడుగు(F, a, b, c, d, సమితి_3(8), 0x698098d8, 7)
    అడుగు(F, d, a, b, c, సమితి_3(9), 0x8b44f7af, 12)
    అడుగు(F, c, d, a, b, సమితి_3(10), 0xffff5bb1, 17)
    అడుగు(F, b, c, d, a, సమితి_3(11), 0x895cd7be, 22)
    అడుగు(F, a, b, c, d, సమితి_3(12), 0x6b901122, 7)
    అడుగు(F, d, a, b, c, సమితి_3(13), 0xfd987193, 12)
    అడుగు(F, c, d, a, b, సమితి_3(14), 0xa679438e, 17)
    అడుగు(F, b, c, d, a, సమితి_3(15), 0x49b40821, 22)

    // Round 2
    అడుగు(G, a, b, c, d, GET(1), 0xf61e2562, 5)
    అడుగు(G, d, a, b, c, GET(6), 0xc040b340, 9)
    అడుగు(G, c, d, a, b, GET(11), 0x265e5a51, 14)
    అడుగు(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
    అడుగు(G, a, b, c, d, GET(5), 0xd62f105d, 5)
    అడుగు(G, d, a, b, c, GET(10), 0x02441453, 9)
    అడుగు(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
    అడుగు(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
    అడుగు(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
    అడుగు(G, d, a, b, c, GET(14), 0xc33707d6, 9)
    అడుగు(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
    అడుగు(G, b, c, d, a, GET(8), 0x455a14ed, 20)
    అడుగు(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
    అడుగు(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
    అడుగు(G, c, d, a, b, GET(7), 0x676f02d9, 14)
    అడుగు(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

    // Round 3
    అడుగు(H, a, b, c, d, GET(5), 0xfffa3942, 4)
    అడుగు(H, d, a, b, c, GET(8), 0x8771f681, 11)
    అడుగు(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
    అడుగు(H, b, c, d, a, GET(14), 0xfde5380c, 23)
    అడుగు(H, a, b, c, d, GET(1), 0xa4beea44, 4)
    అడుగు(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
    అడుగు(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
    అడుగు(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
    అడుగు(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
    అడుగు(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
    అడుగు(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
    అడుగు(H, b, c, d, a, GET(6), 0x04881d05, 23)
    అడుగు(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
    అడుగు(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
    అడుగు(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
    అడుగు(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

    // Round 4
    అడుగు(I, a, b, c, d, GET(0), 0xf4292244, 6)
    అడుగు(I, d, a, b, c, GET(7), 0x432aff97, 10)
    అడుగు(I, c, d, a, b, GET(14), 0xab9423a7, 15)
    అడుగు(I, b, c, d, a, GET(5), 0xfc93a039, 21)
    అడుగు(I, a, b, c, d, GET(12), 0x655b59c3, 6)
    అడుగు(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
    అడుగు(I, c, d, a, b, GET(10), 0xffeff47d, 15)
    అడుగు(I, b, c, d, a, GET(1), 0x85845dd1, 21)
    అడుగు(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
    అడుగు(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
    అడుగు(I, c, d, a, b, GET(6), 0xa3014314, 15)
    అడుగు(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
    అడుగు(I, a, b, c, d, GET(4), 0xf7537e82, 6)
    అడుగు(I, d, a, b, c, GET(11), 0xbd3af235, 10)
    అడుగు(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
    అడుగు(I, b, c, d, a, GET(9), 0xeb86d391, 21)

    a += saved_a;
    b += saved_b;
    c += saved_c;
    d += saved_d;

    ptr += 64;
  } అయితే (కొలత -= 64);

  అంతర్గత_State.a = a;
  అంతర్గత_State.b = b;
  అంతర్గత_State.c = c;
  అంతర్గత_State.d = d;

  తిరిగి ptr;
}

MD5::MD5() = అప్రమేయ;

/// Incrementally add the bytes in \p Data to the hash.
శూన్య MD5::update(ArrayRef<uint8_t> దత్తాంశం_2) {
  MD5_u32plus saved_lo;
  గుర్తులేని దీర్ఘ వాడిన, విడుదల;
  స్థిరరాశి uint8_t *Ptr = దత్తాంశం_2.దత్తాంశం();
  గుర్తులేని దీర్ఘ కొలత = దత్తాంశం_2.కొలత_2();

  saved_lo = అంతర్గత_State.lo;
  ఒకవేళ ((అంతర్గత_State.lo = (saved_lo + కొలత) & 0x1fffffff) < saved_lo)
    అంతర్గత_State.hi++;
  అంతర్గత_State.hi += కొలత >> 29;

  వాడిన = saved_lo & 0x3f;

  ఒకవేళ (వాడిన) {
    విడుదల = 64 - వాడిన;

    ఒకవేళ (కొలత < విడుదల) {
      జ్ఞాపకం_నకలు(&అంతర్గత_State.బఫరు_2[వాడిన], Ptr, కొలత);
      తిరిగి;
    }

    జ్ఞాపకం_నకలు(&అంతర్గత_State.బఫరు_2[వాడిన], Ptr, విడుదల);
    Ptr = Ptr + విడుదల;
    కొలత -= విడుదల;
    body(ArrayRef(అంతర్గత_State.బఫరు_2, 64));
  }

  ఒకవేళ (కొలత >= 64) {
    Ptr = body(ArrayRef(Ptr, కొలత & ~(గుర్తులేని దీర్ఘ)0x3f));
    కొలత &= 0x3f;
  }

  జ్ఞాపకం_నకలు(అంతర్గత_State.బఫరు_2, Ptr, కొలత);
}

/// Add the bytes in the StringRef \p Str to the hash.
// Note that this isn't a string and so this won't include any trailing NULL
// bytes.
శూన్య MD5::update(StringRef Str) {
  ArrayRef<uint8_t> SVal((స్థిరరాశి uint8_t *)Str.దత్తాంశం(), Str.కొలత_2());
  update(SVal);
}

/// Finish the hash and place the resulting hash into \p result.
/// \param Result is assumed to be a minimum of 16-bytes in size.
శూన్య MD5::final(MD5_ఫలితం &ఫలితం) {
  గుర్తులేని దీర్ఘ వాడిన, విడుదల;

  వాడిన = అంతర్గత_State.lo & 0x3f;

  అంతర్గత_State.బఫరు_2[వాడిన++] = 0x80;

  విడుదల = 64 - వాడిన;

  ఒకవేళ (విడుదల < 8) {
    memset(&అంతర్గత_State.బఫరు_2[వాడిన], 0, విడుదల);
    body(ArrayRef(అంతర్గత_State.బఫరు_2, 64));
    వాడిన = 0;
    విడుదల = 64;
  }

  memset(&అంతర్గత_State.బఫరు_2[వాడిన], 0, విడుదల - 8);

  అంతర్గత_State.lo <<= 3;
  support::endian::write32le(&అంతర్గత_State.బఫరు_2[56], అంతర్గత_State.lo);
  support::endian::write32le(&అంతర్గత_State.బఫరు_2[60], అంతర్గత_State.hi);

  body(ArrayRef(అంతర్గత_State.బఫరు_2, 64));

  support::endian::write32le(&ఫలితం[0], అంతర్గత_State.a);
  support::endian::write32le(&ఫలితం[4], అంతర్గత_State.b);
  support::endian::write32le(&ఫలితం[8], అంతర్గత_State.c);
  support::endian::write32le(&ఫలితం[12], అంతర్గత_State.d);
}

MD5::MD5_ఫలితం MD5::final() {
  MD5_ఫలితం ఫలితం;
  final(ఫలితం);
  తిరిగి ఫలితం;
}

MD5::MD5_ఫలితం MD5::ఫలితం_2() {
  స్వయంచాలక State_గమ్యం_Restore = అంతర్గత_State;

  స్వయంచాలక Hash = final();

  // Restore the state
  అంతర్గత_State = State_గమ్యం_Restore;

  తిరిగి Hash;
}

SmallString<32> MD5::MD5_ఫలితం::digest() స్థిరరాశి {
  SmallString<32> Str;
  గమ్యం_Hex(*ఇది, /*LowerCase*/ నిజం, Str);
  తిరిగి Str;
}

శూన్య MD5::stringify_ఫలితం(MD5_ఫలితం &ఫలితం, Small_సదిశ_Impl<అక్షరం> &Str) {
  గమ్యం_Hex(ఫలితం, /*LowerCase*/ నిజం, Str);
}

MD5::MD5_ఫలితం MD5::hash(ArrayRef<uint8_t> దత్తాంశం_2) {
  MD5 Hash;
  Hash.update(దత్తాంశం_2);
  MD5::MD5_ఫలితం Res;
  Hash.final(Res);

  తిరిగి Res;
}

#నిర్వచనం_రద్దు F
#నిర్వచనం_రద్దు G
#నిర్వచనం_రద్దు H
#నిర్వచనం_రద్దు I
#నిర్వచనం_రద్దు అడుగు
#నిర్వచనం_రద్దు సమితి_3
#నిర్వచనం_రద్దు GET
