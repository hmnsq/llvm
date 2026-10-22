#अन्तर्भाव "blake3_impl.h"

#अन्तर्भाव <immintrin.h>

#परिभाषय DEGREE 4

#परिभाषय _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu(ध्रुव uint8_t src[16]) {
  प्रत्यागम _mm_loadu_si128((ध्रुव __m128i *)src);
}

INLINE शून्य storeu(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE __m128i addv(__m128i a, __m128i b) { प्रत्यागम mm_सङ्कलय_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m128i xorv(__m128i a, __m128i b) { प्रत्यागम _mm_xor_si128(a, b); }

INLINE __m128i set1(uint32_t x) { प्रत्यागम _mm_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  प्रत्यागम _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16(__m128i x) {
  प्रत्यागम _mm_shuffle_epi8(
      x, mm_समुच्चयः_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
}

INLINE __m128i rot12(__m128i x) {
  प्रत्यागम xorv(_mm_srli_epi32(x, 12), _mm_slli_epi32(x, 32 - 12));
}

INLINE __m128i rot8(__m128i x) {
  प्रत्यागम _mm_shuffle_epi8(
      x, mm_समुच्चयः_epi8(12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1));
}

INLINE __m128i rot7(__m128i x) {
  प्रत्यागम xorv(_mm_srli_epi32(x, 7), _mm_slli_epi32(x, 32 - 7));
}

INLINE शून्य g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot16(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot12(*row1);
}

INLINE शून्य g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot8(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot7(*row1);
}

// Note the optimization here of leaving row1 as the unrotated row, rather than
// row0. All the message loads below are adjusted to compensate for this. See
// discussion at https://github.com/sneves/blake2-avx2/pull/4
INLINE शून्य diagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(2, 1, 0, 3));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(0, 3, 2, 1));
}

INLINE शून्य undiagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(0, 3, 2, 1));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(2, 1, 0, 3));
}

INLINE शून्य compress_pre(__m128i पङ्क्तिकाः[4], ध्रुव uint32_t cv[8],
                         ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                         uint8_t खण्डम्_len, uint64_t गणक_2, uint8_t flags) {
  पङ्क्तिकाः[0] = loadu((uint8_t *)&cv[0]);
  पङ्क्तिकाः[1] = loadu((uint8_t *)&cv[4]);
  पङ्क्तिकाः[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  पङ्क्तिकाः[3] = set4(गणक_नीचः(गणक_2), गणक_उच्चः(गणक_2),
                 (uint32_t)खण्डम्_len, (uint32_t)flags);

  __m128i m0 = loadu(&खण्डम्[परिमाण(__m128i) * 0]);
  __m128i m1 = loadu(&खण्डम्[परिमाण(__m128i) * 1]);
  __m128i m2 = loadu(&खण्डम्[परिमाण(__m128i) * 2]);
  __m128i m3 = loadu(&खण्डम्[परिमाण(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t1);
  diagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&पङ्क्तिकाः[0], &पङ्क्तिकाः[1], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3], t3);
  undiagonalize(&पङ्क्तिकाः[0], &पङ्क्तिकाः[2], &पङ्क्तिकाः[3]);
}

शून्य blake3_compress_अन्तर्_place_sse41(uint32_t cv[8],
                                    ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                                    uint8_t खण्डम्_len, uint64_t गणक_2,
                                    uint8_t flags) {
  __m128i पङ्क्तिकाः[4];
  compress_pre(पङ्क्तिकाः, cv, खण्डम्, खण्डम्_len, गणक_2, flags);
  storeu(xorv(पङ्क्तिकाः[0], पङ्क्तिकाः[2]), (uint8_t *)&cv[0]);
  storeu(xorv(पङ्क्तिकाः[1], पङ्क्तिकाः[3]), (uint8_t *)&cv[4]);
}

शून्य blake3_compress_xof_sse41(ध्रुव uint32_t cv[8],
                               ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                               uint8_t खण्डम्_len, uint64_t गणक_2,
                               uint8_t flags, uint8_t बहिर्गमनम्_2[64]) {
  __m128i पङ्क्तिकाः[4];
  compress_pre(पङ्क्तिकाः, cv, खण्डम्, खण्डम्_len, गणक_2, flags);
  storeu(xorv(पङ्क्तिकाः[0], पङ्क्तिकाः[2]), &बहिर्गमनम्_2[0]);
  storeu(xorv(पङ्क्तिकाः[1], पङ्क्तिकाः[3]), &बहिर्गमनम्_2[16]);
  storeu(xorv(पङ्क्तिकाः[2], loadu((uint8_t *)&cv[0])), &बहिर्गमनम्_2[32]);
  storeu(xorv(पङ्क्तिकाः[3], loadu((uint8_t *)&cv[4])), &बहिर्गमनम्_2[48]);
}

INLINE शून्य आवर्तनम्_fn(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)सन्देशः_कालसारणी[r][0]]);
  v[1] = addv(v[1], m[(size_t)सन्देशः_कालसारणी[r][2]]);
  v[2] = addv(v[2], m[(size_t)सन्देशः_कालसारणी[r][4]]);
  v[3] = addv(v[3], m[(size_t)सन्देशः_कालसारणी[r][6]]);
  v[0] = addv(v[0], v[4]);
  v[1] = addv(v[1], v[5]);
  v[2] = addv(v[2], v[6]);
  v[3] = addv(v[3], v[7]);
  v[12] = xorv(v[12], v[0]);
  v[13] = xorv(v[13], v[1]);
  v[14] = xorv(v[14], v[2]);
  v[15] = xorv(v[15], v[3]);
  v[12] = rot16(v[12]);
  v[13] = rot16(v[13]);
  v[14] = rot16(v[14]);
  v[15] = rot16(v[15]);
  v[8] = addv(v[8], v[12]);
  v[9] = addv(v[9], v[13]);
  v[10] = addv(v[10], v[14]);
  v[11] = addv(v[11], v[15]);
  v[4] = xorv(v[4], v[8]);
  v[5] = xorv(v[5], v[9]);
  v[6] = xorv(v[6], v[10]);
  v[7] = xorv(v[7], v[11]);
  v[4] = rot12(v[4]);
  v[5] = rot12(v[5]);
  v[6] = rot12(v[6]);
  v[7] = rot12(v[7]);
  v[0] = addv(v[0], m[(size_t)सन्देशः_कालसारणी[r][1]]);
  v[1] = addv(v[1], m[(size_t)सन्देशः_कालसारणी[r][3]]);
  v[2] = addv(v[2], m[(size_t)सन्देशः_कालसारणी[r][5]]);
  v[3] = addv(v[3], m[(size_t)सन्देशः_कालसारणी[r][7]]);
  v[0] = addv(v[0], v[4]);
  v[1] = addv(v[1], v[5]);
  v[2] = addv(v[2], v[6]);
  v[3] = addv(v[3], v[7]);
  v[12] = xorv(v[12], v[0]);
  v[13] = xorv(v[13], v[1]);
  v[14] = xorv(v[14], v[2]);
  v[15] = xorv(v[15], v[3]);
  v[12] = rot8(v[12]);
  v[13] = rot8(v[13]);
  v[14] = rot8(v[14]);
  v[15] = rot8(v[15]);
  v[8] = addv(v[8], v[12]);
  v[9] = addv(v[9], v[13]);
  v[10] = addv(v[10], v[14]);
  v[11] = addv(v[11], v[15]);
  v[4] = xorv(v[4], v[8]);
  v[5] = xorv(v[5], v[9]);
  v[6] = xorv(v[6], v[10]);
  v[7] = xorv(v[7], v[11]);
  v[4] = rot7(v[4]);
  v[5] = rot7(v[5]);
  v[6] = rot7(v[6]);
  v[7] = rot7(v[7]);

  v[0] = addv(v[0], m[(size_t)सन्देशः_कालसारणी[r][8]]);
  v[1] = addv(v[1], m[(size_t)सन्देशः_कालसारणी[r][10]]);
  v[2] = addv(v[2], m[(size_t)सन्देशः_कालसारणी[r][12]]);
  v[3] = addv(v[3], m[(size_t)सन्देशः_कालसारणी[r][14]]);
  v[0] = addv(v[0], v[5]);
  v[1] = addv(v[1], v[6]);
  v[2] = addv(v[2], v[7]);
  v[3] = addv(v[3], v[4]);
  v[15] = xorv(v[15], v[0]);
  v[12] = xorv(v[12], v[1]);
  v[13] = xorv(v[13], v[2]);
  v[14] = xorv(v[14], v[3]);
  v[15] = rot16(v[15]);
  v[12] = rot16(v[12]);
  v[13] = rot16(v[13]);
  v[14] = rot16(v[14]);
  v[10] = addv(v[10], v[15]);
  v[11] = addv(v[11], v[12]);
  v[8] = addv(v[8], v[13]);
  v[9] = addv(v[9], v[14]);
  v[5] = xorv(v[5], v[10]);
  v[6] = xorv(v[6], v[11]);
  v[7] = xorv(v[7], v[8]);
  v[4] = xorv(v[4], v[9]);
  v[5] = rot12(v[5]);
  v[6] = rot12(v[6]);
  v[7] = rot12(v[7]);
  v[4] = rot12(v[4]);
  v[0] = addv(v[0], m[(size_t)सन्देशः_कालसारणी[r][9]]);
  v[1] = addv(v[1], m[(size_t)सन्देशः_कालसारणी[r][11]]);
  v[2] = addv(v[2], m[(size_t)सन्देशः_कालसारणी[r][13]]);
  v[3] = addv(v[3], m[(size_t)सन्देशः_कालसारणी[r][15]]);
  v[0] = addv(v[0], v[5]);
  v[1] = addv(v[1], v[6]);
  v[2] = addv(v[2], v[7]);
  v[3] = addv(v[3], v[4]);
  v[15] = xorv(v[15], v[0]);
  v[12] = xorv(v[12], v[1]);
  v[13] = xorv(v[13], v[2]);
  v[14] = xorv(v[14], v[3]);
  v[15] = rot8(v[15]);
  v[12] = rot8(v[12]);
  v[13] = rot8(v[13]);
  v[14] = rot8(v[14]);
  v[10] = addv(v[10], v[15]);
  v[11] = addv(v[11], v[12]);
  v[8] = addv(v[8], v[13]);
  v[9] = addv(v[9], v[14]);
  v[5] = xorv(v[5], v[10]);
  v[6] = xorv(v[6], v[11]);
  v[7] = xorv(v[7], v[8]);
  v[4] = xorv(v[4], v[9]);
  v[5] = rot7(v[5]);
  v[6] = rot7(v[6]);
  v[7] = rot7(v[7]);
  v[4] = rot7(v[4]);
}

INLINE शून्य transpose_vecs(__m128i vecs[DEGREE]) {
  // Interleave 32-bit lanes. The low unpack is lanes 00/11 and the high is
  // 22/33. Note that this doesn't split the vector into two lanes, as the
  // AVX2 counterparts do.
  __m128i ab_01 = _mm_unpacklo_epi32(vecs[0], vecs[1]);
  __m128i ab_23 = _mm_unpackhi_epi32(vecs[0], vecs[1]);
  __m128i cd_01 = _mm_unpacklo_epi32(vecs[2], vecs[3]);
  __m128i cd_23 = _mm_unpackhi_epi32(vecs[2], vecs[3]);

  // Interleave 64-bit lanes.
  __m128i abcd_0 = _mm_unpacklo_epi64(ab_01, cd_01);
  __m128i abcd_1 = _mm_unpackhi_epi64(ab_01, cd_01);
  __m128i abcd_2 = _mm_unpacklo_epi64(ab_23, cd_23);
  __m128i abcd_3 = _mm_unpackhi_epi64(ab_23, cd_23);

  vecs[0] = abcd_0;
  vecs[1] = abcd_1;
  vecs[2] = abcd_2;
  vecs[3] = abcd_3;
}

INLINE शून्य transpose_सन्देशः_vecs(ध्रुव uint8_t *ध्रुव *inputs,
                               size_t खण्डम्_विस्थापनम्, __m128i बहिर्गमनम्_2[16]) {
  बहिर्गमनम्_2[0] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 0 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[1] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 0 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[2] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 0 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[3] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 0 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[4] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 1 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[5] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 1 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[6] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 1 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[7] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 1 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[8] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 2 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[9] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 2 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[10] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 2 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[11] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 2 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[12] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 3 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[13] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 3 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[14] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 3 * परिमाण(__m128i)]);
  बहिर्गमनम्_2[15] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 3 * परिमाण(__m128i)]);
  प्रति (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((ध्रुव शून्य *)&inputs[i][खण्डम्_विस्थापनम् + 256], _MM_HINT_T0);
  }
  transpose_vecs(&बहिर्गमनम्_2[0]);
  transpose_vecs(&बहिर्गमनम्_2[4]);
  transpose_vecs(&बहिर्गमनम्_2[8]);
  transpose_vecs(&बहिर्गमनम्_2[12]);
}

INLINE शून्य आरोपय_counters(uint64_t गणक_2, bool increment_गणक,
                          __m128i *बहिर्गमनम्_lo, __m128i *बहिर्गमनम्_hi) {
  ध्रुव __m128i आवरणम्_2 = _mm_set1_epi32(-(int32_t)increment_गणक);
  ध्रुव __m128i add0 = mm_समुच्चयः_epi32(3, 2, 1, 0);
  ध्रुव __m128i add1 = _mm_and_si128(आवरणम्_2, add0);
  __m128i l = mm_सङ्कलय_epi32(_mm_set1_epi32((int32_t)गणक_2), add1);
  __m128i carry = _mm_cmpgt_epi32(_mm_xor_si128(add1, _mm_set1_epi32(0x80000000)), 
                                  _mm_xor_si128(   l, _mm_set1_epi32(0x80000000)));
  __m128i h = _mm_sub_epi32(_mm_set1_epi32((int32_t)(गणक_2 >> 32)), carry);
  *बहिर्गमनम्_lo = l;
  *बहिर्गमनम्_hi = h;
}

स्थिर
शून्य blake3_hash4_sse41(ध्रुव uint8_t *ध्रुव *inputs, size_t खण्डानि,
                        ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                        bool increment_गणक, uint8_t flags,
                        uint8_t flags_start, uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  __m128i h_vecs[8] = {
      set1(कुञ्चिका_2[0]), set1(कुञ्चिका_2[1]), set1(कुञ्चिका_2[2]), set1(कुञ्चिका_2[3]),
      set1(कुञ्चिका_2[4]), set1(कुञ्चिका_2[5]), set1(कुञ्चिका_2[6]), set1(कुञ्चिका_2[7]),
  };
  __m128i गणक_नीचः_vec, गणक_उच्चः_vec;
  आरोपय_counters(गणक_2, increment_गणक, &गणक_नीचः_vec,
                &गणक_उच्चः_vec);
  uint8_t खण्डम्_flags = flags | flags_start;

  प्रति (size_t खण्डम् = 0; खण्डम् < खण्डानि; खण्डम्++) {
    यदि (खण्डम् + 1 == खण्डानि) {
      खण्डम्_flags |= flags_अन्तः;
    }
    __m128i खण्डम्_len_vec = set1(BLAKE3_खण्डम्_LEN);
    __m128i खण्डम्_flags_vec = set1(खण्डम्_flags);
    __m128i सन्देशः_vecs[16];
    transpose_सन्देशः_vecs(inputs, खण्डम् * BLAKE3_खण्डम्_LEN, सन्देशः_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        गणक_नीचः_vec, गणक_उच्चः_vec, खण्डम्_len_vec, खण्डम्_flags_vec,
    };
    आवर्तनम्_fn(v, सन्देशः_vecs, 0);
    आवर्तनम्_fn(v, सन्देशः_vecs, 1);
    आवर्तनम्_fn(v, सन्देशः_vecs, 2);
    आवर्तनम्_fn(v, सन्देशः_vecs, 3);
    आवर्तनम्_fn(v, सन्देशः_vecs, 4);
    आवर्तनम्_fn(v, सन्देशः_vecs, 5);
    आवर्तनम्_fn(v, सन्देशः_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    खण्डम्_flags = flags;
  }

  transpose_vecs(&h_vecs[0]);
  transpose_vecs(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu(h_vecs[0], &बहिर्गमनम्_2[0 * परिमाण(__m128i)]);
  storeu(h_vecs[4], &बहिर्गमनम्_2[1 * परिमाण(__m128i)]);
  storeu(h_vecs[1], &बहिर्गमनम्_2[2 * परिमाण(__m128i)]);
  storeu(h_vecs[5], &बहिर्गमनम्_2[3 * परिमाण(__m128i)]);
  storeu(h_vecs[2], &बहिर्गमनम्_2[4 * परिमाण(__m128i)]);
  storeu(h_vecs[6], &बहिर्गमनम्_2[5 * परिमाण(__m128i)]);
  storeu(h_vecs[3], &बहिर्गमनम्_2[6 * परिमाण(__m128i)]);
  storeu(h_vecs[7], &बहिर्गमनम्_2[7 * परिमाण(__m128i)]);
}

INLINE शून्य hash_one_sse41(ध्रुव uint8_t *निवेशः_2, size_t खण्डानि,
                           ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_अन्तः, uint8_t बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN]) {
  uint32_t cv[8];
  स्मृतिं_प्रतिलिख(cv, कुञ्चिका_2, BLAKE3_कुञ्चिका_LEN);
  uint8_t खण्डम्_flags = flags | flags_start;
  यावत् (खण्डानि > 0) {
    यदि (खण्डानि == 1) {
      खण्डम्_flags |= flags_अन्तः;
    }
    blake3_compress_अन्तर्_place_sse41(cv, निवेशः_2, BLAKE3_खण्डम्_LEN, गणक_2,
                                   खण्डम्_flags);
    निवेशः_2 = &निवेशः_2[BLAKE3_खण्डम्_LEN];
    खण्डानि -= 1;
    खण्डम्_flags = flags;
  }
  स्मृतिं_प्रतिलिख(बहिर्गमनम्_2, cv, BLAKE3_बहिर्गमनम्_LEN);
}

शून्य blake3_hash_many_sse41(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                            size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                            uint64_t गणक_2, bool increment_गणक,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  यावत् (अङ्कः_inputs >= DEGREE) {
    blake3_hash4_sse41(inputs, खण्डानि, कुञ्चिका_2, गणक_2, increment_गणक, flags,
                       flags_start, flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += DEGREE;
    }
    inputs += DEGREE;
    अङ्कः_inputs -= DEGREE;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[DEGREE * BLAKE3_बहिर्गमनम्_LEN];
  }
  यावत् (अङ्कः_inputs > 0) {
    hash_one_sse41(inputs[0], खण्डानि, कुञ्चिका_2, गणक_2, flags, flags_start,
                   flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += 1;
    }
    inputs += 1;
    अङ्कः_inputs -= 1;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN];
  }
}
