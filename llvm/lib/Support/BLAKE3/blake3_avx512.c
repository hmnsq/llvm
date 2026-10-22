#ضمن "blake3_impl.h"

#ضمن <immintrin.h>

#تعریف_کړه _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu_128(ثابت uint8_t src[16]) {
  أرجع _mm_loadu_si128((ثابت __m128i *)src);
}

INLINE __m256i loadu_256(ثابت uint8_t src[32]) {
  أرجع _mm256_loadu_si256((ثابت __m256i *)src);
}

INLINE __m512i loadu_512(ثابت uint8_t src[64]) {
  أرجع _mm512_loadu_si512((ثابت __m512i *)src);
}

INLINE فراغ storeu_128(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE فراغ storeu_256(__m256i src, uint8_t dest[32]) {
  _mm256_storeu_si256((__m256i *)dest, src);
}

INLINE فراغ storeu_512(__m512i src, uint8_t dest[64]) {
  _mm512_storeu_si512((__m512i *)dest, src);
}

INLINE __m128i جمع_کړه_128(__m128i a, __m128i b) { أرجع mm_جمع_کړه_epi32(a, b); }

INLINE __m256i جمع_کړه_256(__m256i a, __m256i b) { أرجع mm256_جمع_کړه_epi32(a, b); }

INLINE __m512i جمع_کړه_512(__m512i a, __m512i b) { أرجع mm512_جمع_کړه_epi32(a, b); }

INLINE __m128i xor_128(__m128i a, __m128i b) { أرجع _mm_xor_si128(a, b); }

INLINE __m256i xor_256(__m256i a, __m256i b) { أرجع _mm256_xor_si256(a, b); }

INLINE __m512i xor_512(__m512i a, __m512i b) { أرجع _mm512_xor_si512(a, b); }

INLINE __m128i set1_128(uint32_t x) { أرجع _mm_set1_epi32((int32_t)x); }

INLINE __m256i set1_256(uint32_t x) { أرجع _mm256_set1_epi32((int32_t)x); }

INLINE __m512i set1_512(uint32_t x) { أرجع _mm512_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  أرجع _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16_128(__m128i x) { أرجع _mm_ror_epi32(x, 16); }

INLINE __m256i rot16_256(__m256i x) { أرجع _mm256_ror_epi32(x, 16); }

INLINE __m512i rot16_512(__m512i x) { أرجع _mm512_ror_epi32(x, 16); }

INLINE __m128i rot12_128(__m128i x) { أرجع _mm_ror_epi32(x, 12); }

INLINE __m256i rot12_256(__m256i x) { أرجع _mm256_ror_epi32(x, 12); }

INLINE __m512i rot12_512(__m512i x) { أرجع _mm512_ror_epi32(x, 12); }

INLINE __m128i rot8_128(__m128i x) { أرجع _mm_ror_epi32(x, 8); }

INLINE __m256i rot8_256(__m256i x) { أرجع _mm256_ror_epi32(x, 8); }

INLINE __m512i rot8_512(__m512i x) { أرجع _mm512_ror_epi32(x, 8); }

INLINE __m128i rot7_128(__m128i x) { أرجع _mm_ror_epi32(x, 7); }

INLINE __m256i rot7_256(__m256i x) { أرجع _mm256_ror_epi32(x, 7); }

INLINE __m512i rot7_512(__m512i x) { أرجع _mm512_ror_epi32(x, 7); }

/*
 * ----------------------------------------------------------------------------
 * compress_avx512
 * ----------------------------------------------------------------------------
 */

INLINE فراغ g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = جمع_کړه_128(جمع_کړه_128(*row0, m), *row1);
  *row3 = xor_128(*row3, *row0);
  *row3 = rot16_128(*row3);
  *row2 = جمع_کړه_128(*row2, *row3);
  *row1 = xor_128(*row1, *row2);
  *row1 = rot12_128(*row1);
}

INLINE فراغ g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = جمع_کړه_128(جمع_کړه_128(*row0, m), *row1);
  *row3 = xor_128(*row3, *row0);
  *row3 = rot8_128(*row3);
  *row2 = جمع_کړه_128(*row2, *row3);
  *row1 = xor_128(*row1, *row2);
  *row1 = rot7_128(*row1);
}

// Note the optimization here of leaving row1 as the unrotated row, rather than
// row0. All the message loads below are adjusted to compensate for this. See
// discussion at https://github.com/sneves/blake2-avx2/pull/4
INLINE فراغ diagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(2, 1, 0, 3));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(0, 3, 2, 1));
}

INLINE فراغ undiagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(0, 3, 2, 1));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(2, 1, 0, 3));
}

INLINE فراغ compress_pre(__m128i لیکې[4], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t counter, uint8_t flags) {
  لیکې[0] = loadu_128((uint8_t *)&cv[0]);
  لیکې[1] = loadu_128((uint8_t *)&cv[4]);
  لیکې[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  لیکې[3] = set4(counter_ټیټ(counter), counter_لوړ(counter),
                 (uint32_t)بلاک_len, (uint32_t)flags);

  __m128i m0 = loadu_128(&بلاک[حجم(__m128i) * 0]);
  __m128i m1 = loadu_128(&بلاک[حجم(__m128i) * 1]);
  __m128i m2 = loadu_128(&بلاک[حجم(__m128i) * 2]);
  __m128i m3 = loadu_128(&بلاک[حجم(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t1);
  diagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&لیکې[0], &لیکې[1], &لیکې[2], &لیکې[3], t3);
  undiagonalize(&لیکې[0], &لیکې[2], &لیکې[3]);
}

فراغ blake3_compress_xof_avx512(ثابت uint32_t cv[8],
                                ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                uint8_t بلاک_len, uint64_t counter,
                                uint8_t flags, uint8_t بهر_2[64]) {
  __m128i لیکې[4];
  compress_pre(لیکې, cv, بلاک, بلاک_len, counter, flags);
  storeu_128(xor_128(لیکې[0], لیکې[2]), &بهر_2[0]);
  storeu_128(xor_128(لیکې[1], لیکې[3]), &بهر_2[16]);
  storeu_128(xor_128(لیکې[2], loadu_128((uint8_t *)&cv[0])), &بهر_2[32]);
  storeu_128(xor_128(لیکې[3], loadu_128((uint8_t *)&cv[4])), &بهر_2[48]);
}

فراغ blake3_compress_دننه_ځای_پرځای_کړه_avx512(uint32_t cv[8],
                                     ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                     uint8_t بلاک_len, uint64_t counter,
                                     uint8_t flags) {
  __m128i لیکې[4];
  compress_pre(لیکې, cv, بلاک, بلاک_len, counter, flags);
  storeu_128(xor_128(لیکې[0], لیکې[2]), (uint8_t *)&cv[0]);
  storeu_128(xor_128(لیکې[1], لیکې[3]), (uint8_t *)&cv[4]);
}

/*
 * ----------------------------------------------------------------------------
 * hash4_avx512
 * ----------------------------------------------------------------------------
 */

INLINE فراغ پړاو_fn4(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = جمع_کړه_128(v[0], m[(size_t)پیغام_مهالوېش[r][0]]);
  v[1] = جمع_کړه_128(v[1], m[(size_t)پیغام_مهالوېش[r][2]]);
  v[2] = جمع_کړه_128(v[2], m[(size_t)پیغام_مهالوېش[r][4]]);
  v[3] = جمع_کړه_128(v[3], m[(size_t)پیغام_مهالوېش[r][6]]);
  v[0] = جمع_کړه_128(v[0], v[4]);
  v[1] = جمع_کړه_128(v[1], v[5]);
  v[2] = جمع_کړه_128(v[2], v[6]);
  v[3] = جمع_کړه_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = جمع_کړه_128(v[8], v[12]);
  v[9] = جمع_کړه_128(v[9], v[13]);
  v[10] = جمع_کړه_128(v[10], v[14]);
  v[11] = جمع_کړه_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = جمع_کړه_128(v[0], m[(size_t)پیغام_مهالوېش[r][1]]);
  v[1] = جمع_کړه_128(v[1], m[(size_t)پیغام_مهالوېش[r][3]]);
  v[2] = جمع_کړه_128(v[2], m[(size_t)پیغام_مهالوېش[r][5]]);
  v[3] = جمع_کړه_128(v[3], m[(size_t)پیغام_مهالوېش[r][7]]);
  v[0] = جمع_کړه_128(v[0], v[4]);
  v[1] = جمع_کړه_128(v[1], v[5]);
  v[2] = جمع_کړه_128(v[2], v[6]);
  v[3] = جمع_کړه_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = جمع_کړه_128(v[8], v[12]);
  v[9] = جمع_کړه_128(v[9], v[13]);
  v[10] = جمع_کړه_128(v[10], v[14]);
  v[11] = جمع_کړه_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = جمع_کړه_128(v[0], m[(size_t)پیغام_مهالوېش[r][8]]);
  v[1] = جمع_کړه_128(v[1], m[(size_t)پیغام_مهالوېش[r][10]]);
  v[2] = جمع_کړه_128(v[2], m[(size_t)پیغام_مهالوېش[r][12]]);
  v[3] = جمع_کړه_128(v[3], m[(size_t)پیغام_مهالوېش[r][14]]);
  v[0] = جمع_کړه_128(v[0], v[5]);
  v[1] = جمع_کړه_128(v[1], v[6]);
  v[2] = جمع_کړه_128(v[2], v[7]);
  v[3] = جمع_کړه_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = جمع_کړه_128(v[10], v[15]);
  v[11] = جمع_کړه_128(v[11], v[12]);
  v[8] = جمع_کړه_128(v[8], v[13]);
  v[9] = جمع_کړه_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = جمع_کړه_128(v[0], m[(size_t)پیغام_مهالوېش[r][9]]);
  v[1] = جمع_کړه_128(v[1], m[(size_t)پیغام_مهالوېش[r][11]]);
  v[2] = جمع_کړه_128(v[2], m[(size_t)پیغام_مهالوېش[r][13]]);
  v[3] = جمع_کړه_128(v[3], m[(size_t)پیغام_مهالوېش[r][15]]);
  v[0] = جمع_کړه_128(v[0], v[5]);
  v[1] = جمع_کړه_128(v[1], v[6]);
  v[2] = جمع_کړه_128(v[2], v[7]);
  v[3] = جمع_کړه_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = جمع_کړه_128(v[10], v[15]);
  v[11] = جمع_کړه_128(v[11], v[12]);
  v[8] = جمع_کړه_128(v[8], v[13]);
  v[9] = جمع_کړه_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE فراغ transpose_vecs_128(__m128i vecs[4]) {
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

INLINE فراغ transpose_پیغام_vecs4(ثابت uint8_t *ثابت *inputs,
                                size_t بلاک_دننه_واټن, __m128i بهر_2[16]) {
  بهر_2[0] = loadu_128(&inputs[0][بلاک_دننه_واټن + 0 * حجم(__m128i)]);
  بهر_2[1] = loadu_128(&inputs[1][بلاک_دننه_واټن + 0 * حجم(__m128i)]);
  بهر_2[2] = loadu_128(&inputs[2][بلاک_دننه_واټن + 0 * حجم(__m128i)]);
  بهر_2[3] = loadu_128(&inputs[3][بلاک_دننه_واټن + 0 * حجم(__m128i)]);
  بهر_2[4] = loadu_128(&inputs[0][بلاک_دننه_واټن + 1 * حجم(__m128i)]);
  بهر_2[5] = loadu_128(&inputs[1][بلاک_دننه_واټن + 1 * حجم(__m128i)]);
  بهر_2[6] = loadu_128(&inputs[2][بلاک_دننه_واټن + 1 * حجم(__m128i)]);
  بهر_2[7] = loadu_128(&inputs[3][بلاک_دننه_واټن + 1 * حجم(__m128i)]);
  بهر_2[8] = loadu_128(&inputs[0][بلاک_دننه_واټن + 2 * حجم(__m128i)]);
  بهر_2[9] = loadu_128(&inputs[1][بلاک_دننه_واټن + 2 * حجم(__m128i)]);
  بهر_2[10] = loadu_128(&inputs[2][بلاک_دننه_واټن + 2 * حجم(__m128i)]);
  بهر_2[11] = loadu_128(&inputs[3][بلاک_دننه_واټن + 2 * حجم(__m128i)]);
  بهر_2[12] = loadu_128(&inputs[0][بلاک_دننه_واټن + 3 * حجم(__m128i)]);
  بهر_2[13] = loadu_128(&inputs[1][بلاک_دننه_واټن + 3 * حجم(__m128i)]);
  بهر_2[14] = loadu_128(&inputs[2][بلاک_دننه_واټن + 3 * حجم(__m128i)]);
  بهر_2[15] = loadu_128(&inputs[3][بلاک_دننه_واټن + 3 * حجم(__m128i)]);
  لكل (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((ثابت فراغ *)&inputs[i][بلاک_دننه_واټن + 256], _MM_HINT_T0);
  }
  transpose_vecs_128(&بهر_2[0]);
  transpose_vecs_128(&بهر_2[4]);
  transpose_vecs_128(&بهر_2[8]);
  transpose_vecs_128(&بهر_2[12]);
}

INLINE فراغ بارول_counters4(uint64_t counter, bool increment_counter,
                           __m128i *بهر_lo, __m128i *بهر_hi) {
  uint64_t ماسک_2 = (increment_counter ? ~0 : 0);
  __m256i ماسک_vec = _mm256_set1_epi64x(ماسک_2);
  __m256i deltas = _mm256_setr_epi64x(0, 1, 2, 3);
  deltas = _mm256_and_si256(ماسک_vec, deltas);
  __m256i counters =
      mm256_جمع_کړه_epi64(_mm256_set1_epi64x((int64_t)counter), deltas);
  *بهر_lo = _mm256_cvtepi64_epi32(counters);
  *بهر_hi = _mm256_cvtepi64_epi32(_mm256_srli_epi64(counters, 32));
}

ساكن
فراغ blake3_hash4_avx512(ثابت uint8_t *ثابت *inputs, size_t بلاکونه,
                         ثابت uint32_t کلی_2[8], uint64_t counter,
                         bool increment_counter, uint8_t flags,
                         uint8_t flags_پیل, uint8_t flags_پای_ټکی, uint8_t *بهر_2) {
  __m128i h_vecs[8] = {
      set1_128(کلی_2[0]), set1_128(کلی_2[1]), set1_128(کلی_2[2]), set1_128(کلی_2[3]),
      set1_128(کلی_2[4]), set1_128(کلی_2[5]), set1_128(کلی_2[6]), set1_128(کلی_2[7]),
  };
  __m128i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters4(counter, increment_counter, &counter_ټیټ_vec,
                 &counter_لوړ_vec);
  uint8_t بلاک_flags = flags | flags_پیل;

  لكل (size_t بلاک = 0; بلاک < بلاکونه; بلاک++) {
    إذا (بلاک + 1 == بلاکونه) {
      بلاک_flags |= flags_پای_ټکی;
    }
    __m128i بلاک_len_vec = set1_128(BLAKE3_بلاک_LEN);
    __m128i بلاک_flags_vec = set1_128(بلاک_flags);
    __m128i پیغام_vecs[16];
    transpose_پیغام_vecs4(inputs, بلاک * BLAKE3_بلاک_LEN, پیغام_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
    };
    پړاو_fn4(v, پیغام_vecs, 0);
    پړاو_fn4(v, پیغام_vecs, 1);
    پړاو_fn4(v, پیغام_vecs, 2);
    پړاو_fn4(v, پیغام_vecs, 3);
    پړاو_fn4(v, پیغام_vecs, 4);
    پړاو_fn4(v, پیغام_vecs, 5);
    پړاو_fn4(v, پیغام_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    بلاک_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &بهر_2[0 * حجم(__m128i)]);
  storeu_128(h_vecs[4], &بهر_2[1 * حجم(__m128i)]);
  storeu_128(h_vecs[1], &بهر_2[2 * حجم(__m128i)]);
  storeu_128(h_vecs[5], &بهر_2[3 * حجم(__m128i)]);
  storeu_128(h_vecs[2], &بهر_2[4 * حجم(__m128i)]);
  storeu_128(h_vecs[6], &بهر_2[5 * حجم(__m128i)]);
  storeu_128(h_vecs[3], &بهر_2[6 * حجم(__m128i)]);
  storeu_128(h_vecs[7], &بهر_2[7 * حجم(__m128i)]);
}

ساكن
فراغ blake3_xof4_avx512(ثابت uint32_t cv[8],
                        ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                        uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                        uint8_t بهر_2[4 * 64]) {
  __m128i h_vecs[8] = {
      set1_128(cv[0]), set1_128(cv[1]), set1_128(cv[2]), set1_128(cv[3]),
      set1_128(cv[4]), set1_128(cv[5]), set1_128(cv[6]), set1_128(cv[7]),
  };
  uint32_t بلاک_ټکي[16];
  بارول_بلاک_ټکي(بلاک, بلاک_ټکي);
  __m128i پیغام_vecs[16];
  لكل (size_t i = 0; i < 16; i++) {
      پیغام_vecs[i] = set1_128(بلاک_ټکي[i]);
  }
  __m128i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters4(counter, true, &counter_ټیټ_vec, &counter_لوړ_vec);
  __m128i بلاک_len_vec = set1_128(بلاک_len);
  __m128i بلاک_flags_vec = set1_128(flags);
  __m128i v[16] = {
      h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
      h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
      set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
      counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
  };
  پړاو_fn4(v, پیغام_vecs, 0);
  پړاو_fn4(v, پیغام_vecs, 1);
  پړاو_fn4(v, پیغام_vecs, 2);
  پړاو_fn4(v, پیغام_vecs, 3);
  پړاو_fn4(v, پیغام_vecs, 4);
  پړاو_fn4(v, پیغام_vecs, 5);
  پړاو_fn4(v, پیغام_vecs, 6);
  لكل (size_t i = 0; i < 8; i++) {
      v[i] = xor_128(v[i], v[i+8]);
      v[i+8] = xor_128(v[i+8], h_vecs[i]);
  }
  transpose_vecs_128(&v[0]);
  transpose_vecs_128(&v[4]);
  transpose_vecs_128(&v[8]);
  transpose_vecs_128(&v[12]);
  لكل (size_t i = 0; i < 4; i++) {
      storeu_128(v[i+ 0], &بهر_2[(4*i+0) * حجم(__m128i)]);
      storeu_128(v[i+ 4], &بهر_2[(4*i+1) * حجم(__m128i)]);
      storeu_128(v[i+ 8], &بهر_2[(4*i+2) * حجم(__m128i)]);
      storeu_128(v[i+12], &بهر_2[(4*i+3) * حجم(__m128i)]);
  }
}

/*
 * ----------------------------------------------------------------------------
 * hash8_avx512
 * ----------------------------------------------------------------------------
 */

INLINE فراغ پړاو_fn8(__m256i v[16], __m256i m[16], size_t r) {
  v[0] = جمع_کړه_256(v[0], m[(size_t)پیغام_مهالوېش[r][0]]);
  v[1] = جمع_کړه_256(v[1], m[(size_t)پیغام_مهالوېش[r][2]]);
  v[2] = جمع_کړه_256(v[2], m[(size_t)پیغام_مهالوېش[r][4]]);
  v[3] = جمع_کړه_256(v[3], m[(size_t)پیغام_مهالوېش[r][6]]);
  v[0] = جمع_کړه_256(v[0], v[4]);
  v[1] = جمع_کړه_256(v[1], v[5]);
  v[2] = جمع_کړه_256(v[2], v[6]);
  v[3] = جمع_کړه_256(v[3], v[7]);
  v[12] = xor_256(v[12], v[0]);
  v[13] = xor_256(v[13], v[1]);
  v[14] = xor_256(v[14], v[2]);
  v[15] = xor_256(v[15], v[3]);
  v[12] = rot16_256(v[12]);
  v[13] = rot16_256(v[13]);
  v[14] = rot16_256(v[14]);
  v[15] = rot16_256(v[15]);
  v[8] = جمع_کړه_256(v[8], v[12]);
  v[9] = جمع_کړه_256(v[9], v[13]);
  v[10] = جمع_کړه_256(v[10], v[14]);
  v[11] = جمع_کړه_256(v[11], v[15]);
  v[4] = xor_256(v[4], v[8]);
  v[5] = xor_256(v[5], v[9]);
  v[6] = xor_256(v[6], v[10]);
  v[7] = xor_256(v[7], v[11]);
  v[4] = rot12_256(v[4]);
  v[5] = rot12_256(v[5]);
  v[6] = rot12_256(v[6]);
  v[7] = rot12_256(v[7]);
  v[0] = جمع_کړه_256(v[0], m[(size_t)پیغام_مهالوېش[r][1]]);
  v[1] = جمع_کړه_256(v[1], m[(size_t)پیغام_مهالوېش[r][3]]);
  v[2] = جمع_کړه_256(v[2], m[(size_t)پیغام_مهالوېش[r][5]]);
  v[3] = جمع_کړه_256(v[3], m[(size_t)پیغام_مهالوېش[r][7]]);
  v[0] = جمع_کړه_256(v[0], v[4]);
  v[1] = جمع_کړه_256(v[1], v[5]);
  v[2] = جمع_کړه_256(v[2], v[6]);
  v[3] = جمع_کړه_256(v[3], v[7]);
  v[12] = xor_256(v[12], v[0]);
  v[13] = xor_256(v[13], v[1]);
  v[14] = xor_256(v[14], v[2]);
  v[15] = xor_256(v[15], v[3]);
  v[12] = rot8_256(v[12]);
  v[13] = rot8_256(v[13]);
  v[14] = rot8_256(v[14]);
  v[15] = rot8_256(v[15]);
  v[8] = جمع_کړه_256(v[8], v[12]);
  v[9] = جمع_کړه_256(v[9], v[13]);
  v[10] = جمع_کړه_256(v[10], v[14]);
  v[11] = جمع_کړه_256(v[11], v[15]);
  v[4] = xor_256(v[4], v[8]);
  v[5] = xor_256(v[5], v[9]);
  v[6] = xor_256(v[6], v[10]);
  v[7] = xor_256(v[7], v[11]);
  v[4] = rot7_256(v[4]);
  v[5] = rot7_256(v[5]);
  v[6] = rot7_256(v[6]);
  v[7] = rot7_256(v[7]);

  v[0] = جمع_کړه_256(v[0], m[(size_t)پیغام_مهالوېش[r][8]]);
  v[1] = جمع_کړه_256(v[1], m[(size_t)پیغام_مهالوېش[r][10]]);
  v[2] = جمع_کړه_256(v[2], m[(size_t)پیغام_مهالوېش[r][12]]);
  v[3] = جمع_کړه_256(v[3], m[(size_t)پیغام_مهالوېش[r][14]]);
  v[0] = جمع_کړه_256(v[0], v[5]);
  v[1] = جمع_کړه_256(v[1], v[6]);
  v[2] = جمع_کړه_256(v[2], v[7]);
  v[3] = جمع_کړه_256(v[3], v[4]);
  v[15] = xor_256(v[15], v[0]);
  v[12] = xor_256(v[12], v[1]);
  v[13] = xor_256(v[13], v[2]);
  v[14] = xor_256(v[14], v[3]);
  v[15] = rot16_256(v[15]);
  v[12] = rot16_256(v[12]);
  v[13] = rot16_256(v[13]);
  v[14] = rot16_256(v[14]);
  v[10] = جمع_کړه_256(v[10], v[15]);
  v[11] = جمع_کړه_256(v[11], v[12]);
  v[8] = جمع_کړه_256(v[8], v[13]);
  v[9] = جمع_کړه_256(v[9], v[14]);
  v[5] = xor_256(v[5], v[10]);
  v[6] = xor_256(v[6], v[11]);
  v[7] = xor_256(v[7], v[8]);
  v[4] = xor_256(v[4], v[9]);
  v[5] = rot12_256(v[5]);
  v[6] = rot12_256(v[6]);
  v[7] = rot12_256(v[7]);
  v[4] = rot12_256(v[4]);
  v[0] = جمع_کړه_256(v[0], m[(size_t)پیغام_مهالوېش[r][9]]);
  v[1] = جمع_کړه_256(v[1], m[(size_t)پیغام_مهالوېش[r][11]]);
  v[2] = جمع_کړه_256(v[2], m[(size_t)پیغام_مهالوېش[r][13]]);
  v[3] = جمع_کړه_256(v[3], m[(size_t)پیغام_مهالوېش[r][15]]);
  v[0] = جمع_کړه_256(v[0], v[5]);
  v[1] = جمع_کړه_256(v[1], v[6]);
  v[2] = جمع_کړه_256(v[2], v[7]);
  v[3] = جمع_کړه_256(v[3], v[4]);
  v[15] = xor_256(v[15], v[0]);
  v[12] = xor_256(v[12], v[1]);
  v[13] = xor_256(v[13], v[2]);
  v[14] = xor_256(v[14], v[3]);
  v[15] = rot8_256(v[15]);
  v[12] = rot8_256(v[12]);
  v[13] = rot8_256(v[13]);
  v[14] = rot8_256(v[14]);
  v[10] = جمع_کړه_256(v[10], v[15]);
  v[11] = جمع_کړه_256(v[11], v[12]);
  v[8] = جمع_کړه_256(v[8], v[13]);
  v[9] = جمع_کړه_256(v[9], v[14]);
  v[5] = xor_256(v[5], v[10]);
  v[6] = xor_256(v[6], v[11]);
  v[7] = xor_256(v[7], v[8]);
  v[4] = xor_256(v[4], v[9]);
  v[5] = rot7_256(v[5]);
  v[6] = rot7_256(v[6]);
  v[7] = rot7_256(v[7]);
  v[4] = rot7_256(v[4]);
}

INLINE فراغ transpose_vecs_256(__m256i vecs[8]) {
  // Interleave 32-bit lanes. The low unpack is lanes 00/11/44/55, and the high
  // is 22/33/66/77.
  __m256i ab_0145 = _mm256_unpacklo_epi32(vecs[0], vecs[1]);
  __m256i ab_2367 = _mm256_unpackhi_epi32(vecs[0], vecs[1]);
  __m256i cd_0145 = _mm256_unpacklo_epi32(vecs[2], vecs[3]);
  __m256i cd_2367 = _mm256_unpackhi_epi32(vecs[2], vecs[3]);
  __m256i ef_0145 = _mm256_unpacklo_epi32(vecs[4], vecs[5]);
  __m256i ef_2367 = _mm256_unpackhi_epi32(vecs[4], vecs[5]);
  __m256i gh_0145 = _mm256_unpacklo_epi32(vecs[6], vecs[7]);
  __m256i gh_2367 = _mm256_unpackhi_epi32(vecs[6], vecs[7]);

  // Interleave 64-bit lanes. The low unpack is lanes 00/22 and the high is
  // 11/33.
  __m256i abcd_04 = _mm256_unpacklo_epi64(ab_0145, cd_0145);
  __m256i abcd_15 = _mm256_unpackhi_epi64(ab_0145, cd_0145);
  __m256i abcd_26 = _mm256_unpacklo_epi64(ab_2367, cd_2367);
  __m256i abcd_37 = _mm256_unpackhi_epi64(ab_2367, cd_2367);
  __m256i efgh_04 = _mm256_unpacklo_epi64(ef_0145, gh_0145);
  __m256i efgh_15 = _mm256_unpackhi_epi64(ef_0145, gh_0145);
  __m256i efgh_26 = _mm256_unpacklo_epi64(ef_2367, gh_2367);
  __m256i efgh_37 = _mm256_unpackhi_epi64(ef_2367, gh_2367);

  // Interleave 128-bit lanes.
  vecs[0] = _mm256_permute2x128_si256(abcd_04, efgh_04, 0x20);
  vecs[1] = _mm256_permute2x128_si256(abcd_15, efgh_15, 0x20);
  vecs[2] = _mm256_permute2x128_si256(abcd_26, efgh_26, 0x20);
  vecs[3] = _mm256_permute2x128_si256(abcd_37, efgh_37, 0x20);
  vecs[4] = _mm256_permute2x128_si256(abcd_04, efgh_04, 0x31);
  vecs[5] = _mm256_permute2x128_si256(abcd_15, efgh_15, 0x31);
  vecs[6] = _mm256_permute2x128_si256(abcd_26, efgh_26, 0x31);
  vecs[7] = _mm256_permute2x128_si256(abcd_37, efgh_37, 0x31);
}

INLINE فراغ transpose_پیغام_vecs8(ثابت uint8_t *ثابت *inputs,
                                size_t بلاک_دننه_واټن, __m256i بهر_2[16]) {
  بهر_2[0] = loadu_256(&inputs[0][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[1] = loadu_256(&inputs[1][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[2] = loadu_256(&inputs[2][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[3] = loadu_256(&inputs[3][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[4] = loadu_256(&inputs[4][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[5] = loadu_256(&inputs[5][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[6] = loadu_256(&inputs[6][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[7] = loadu_256(&inputs[7][بلاک_دننه_واټن + 0 * حجم(__m256i)]);
  بهر_2[8] = loadu_256(&inputs[0][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[9] = loadu_256(&inputs[1][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[10] = loadu_256(&inputs[2][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[11] = loadu_256(&inputs[3][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[12] = loadu_256(&inputs[4][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[13] = loadu_256(&inputs[5][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[14] = loadu_256(&inputs[6][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  بهر_2[15] = loadu_256(&inputs[7][بلاک_دننه_واټن + 1 * حجم(__m256i)]);
  لكل (size_t i = 0; i < 8; ++i) {
    _mm_prefetch((ثابت فراغ *)&inputs[i][بلاک_دننه_واټن + 256], _MM_HINT_T0);
  }
  transpose_vecs_256(&بهر_2[0]);
  transpose_vecs_256(&بهر_2[8]);
}

INLINE فراغ بارول_counters8(uint64_t counter, bool increment_counter,
                           __m256i *بهر_lo, __m256i *بهر_hi) {
  uint64_t ماسک_2 = (increment_counter ? ~0 : 0);
  __m512i ماسک_vec = _mm512_set1_epi64(ماسک_2);
  __m512i deltas = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
  deltas = _mm512_and_si512(ماسک_vec, deltas);
  __m512i counters =
      mm512_جمع_کړه_epi64(_mm512_set1_epi64((int64_t)counter), deltas);
  *بهر_lo = _mm512_cvtepi64_epi32(counters);
  *بهر_hi = _mm512_cvtepi64_epi32(_mm512_srli_epi64(counters, 32));
}

ساكن
فراغ blake3_hash8_avx512(ثابت uint8_t *ثابت *inputs, size_t بلاکونه,
                         ثابت uint32_t کلی_2[8], uint64_t counter,
                         bool increment_counter, uint8_t flags,
                         uint8_t flags_پیل, uint8_t flags_پای_ټکی, uint8_t *بهر_2) {
  __m256i h_vecs[8] = {
      set1_256(کلی_2[0]), set1_256(کلی_2[1]), set1_256(کلی_2[2]), set1_256(کلی_2[3]),
      set1_256(کلی_2[4]), set1_256(کلی_2[5]), set1_256(کلی_2[6]), set1_256(کلی_2[7]),
  };
  __m256i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters8(counter, increment_counter, &counter_ټیټ_vec,
                 &counter_لوړ_vec);
  uint8_t بلاک_flags = flags | flags_پیل;

  لكل (size_t بلاک = 0; بلاک < بلاکونه; بلاک++) {
    إذا (بلاک + 1 == بلاکونه) {
      بلاک_flags |= flags_پای_ټکی;
    }
    __m256i بلاک_len_vec = set1_256(BLAKE3_بلاک_LEN);
    __m256i بلاک_flags_vec = set1_256(بلاک_flags);
    __m256i پیغام_vecs[16];
    transpose_پیغام_vecs8(inputs, بلاک * BLAKE3_بلاک_LEN, پیغام_vecs);

    __m256i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_256(IV[0]), set1_256(IV[1]),  set1_256(IV[2]), set1_256(IV[3]),
        counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
    };
    پړاو_fn8(v, پیغام_vecs, 0);
    پړاو_fn8(v, پیغام_vecs, 1);
    پړاو_fn8(v, پیغام_vecs, 2);
    پړاو_fn8(v, پیغام_vecs, 3);
    پړاو_fn8(v, پیغام_vecs, 4);
    پړاو_fn8(v, پیغام_vecs, 5);
    پړاو_fn8(v, پیغام_vecs, 6);
    h_vecs[0] = xor_256(v[0], v[8]);
    h_vecs[1] = xor_256(v[1], v[9]);
    h_vecs[2] = xor_256(v[2], v[10]);
    h_vecs[3] = xor_256(v[3], v[11]);
    h_vecs[4] = xor_256(v[4], v[12]);
    h_vecs[5] = xor_256(v[5], v[13]);
    h_vecs[6] = xor_256(v[6], v[14]);
    h_vecs[7] = xor_256(v[7], v[15]);

    بلاک_flags = flags;
  }

  transpose_vecs_256(h_vecs);
  storeu_256(h_vecs[0], &بهر_2[0 * حجم(__m256i)]);
  storeu_256(h_vecs[1], &بهر_2[1 * حجم(__m256i)]);
  storeu_256(h_vecs[2], &بهر_2[2 * حجم(__m256i)]);
  storeu_256(h_vecs[3], &بهر_2[3 * حجم(__m256i)]);
  storeu_256(h_vecs[4], &بهر_2[4 * حجم(__m256i)]);
  storeu_256(h_vecs[5], &بهر_2[5 * حجم(__m256i)]);
  storeu_256(h_vecs[6], &بهر_2[6 * حجم(__m256i)]);
  storeu_256(h_vecs[7], &بهر_2[7 * حجم(__m256i)]);
}

ساكن
فراغ blake3_xof8_avx512(ثابت uint32_t cv[8],
                        ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                        uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                        uint8_t بهر_2[8 * 64]) {
  __m256i h_vecs[8] = {
      set1_256(cv[0]), set1_256(cv[1]), set1_256(cv[2]), set1_256(cv[3]),
      set1_256(cv[4]), set1_256(cv[5]), set1_256(cv[6]), set1_256(cv[7]),
  };
  uint32_t بلاک_ټکي[16];
  بارول_بلاک_ټکي(بلاک, بلاک_ټکي);
  __m256i پیغام_vecs[16];
  لكل (size_t i = 0; i < 16; i++) {
      پیغام_vecs[i] = set1_256(بلاک_ټکي[i]);
  }
  __m256i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters8(counter, true, &counter_ټیټ_vec, &counter_لوړ_vec);
  __m256i بلاک_len_vec = set1_256(بلاک_len);
  __m256i بلاک_flags_vec = set1_256(flags);
  __m256i v[16] = {
      h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
      h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
      set1_256(IV[0]), set1_256(IV[1]),  set1_256(IV[2]), set1_256(IV[3]),
      counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
  };
  پړاو_fn8(v, پیغام_vecs, 0);
  پړاو_fn8(v, پیغام_vecs, 1);
  پړاو_fn8(v, پیغام_vecs, 2);
  پړاو_fn8(v, پیغام_vecs, 3);
  پړاو_fn8(v, پیغام_vecs, 4);
  پړاو_fn8(v, پیغام_vecs, 5);
  پړاو_fn8(v, پیغام_vecs, 6);
  لكل (size_t i = 0; i < 8; i++) {
      v[i] = xor_256(v[i], v[i+8]);
      v[i+8] = xor_256(v[i+8], h_vecs[i]);
  }
  transpose_vecs_256(&v[0]);
  transpose_vecs_256(&v[8]);
  لكل (size_t i = 0; i < 8; i++) {
      storeu_256(v[i+0], &بهر_2[(2*i+0) * حجم(__m256i)]);
      storeu_256(v[i+8], &بهر_2[(2*i+1) * حجم(__m256i)]);
  }
}

/*
 * ----------------------------------------------------------------------------
 * hash16_avx512
 * ----------------------------------------------------------------------------
 */

INLINE فراغ پړاو_fn16(__m512i v[16], __m512i m[16], size_t r) {
  v[0] = جمع_کړه_512(v[0], m[(size_t)پیغام_مهالوېش[r][0]]);
  v[1] = جمع_کړه_512(v[1], m[(size_t)پیغام_مهالوېش[r][2]]);
  v[2] = جمع_کړه_512(v[2], m[(size_t)پیغام_مهالوېش[r][4]]);
  v[3] = جمع_کړه_512(v[3], m[(size_t)پیغام_مهالوېش[r][6]]);
  v[0] = جمع_کړه_512(v[0], v[4]);
  v[1] = جمع_کړه_512(v[1], v[5]);
  v[2] = جمع_کړه_512(v[2], v[6]);
  v[3] = جمع_کړه_512(v[3], v[7]);
  v[12] = xor_512(v[12], v[0]);
  v[13] = xor_512(v[13], v[1]);
  v[14] = xor_512(v[14], v[2]);
  v[15] = xor_512(v[15], v[3]);
  v[12] = rot16_512(v[12]);
  v[13] = rot16_512(v[13]);
  v[14] = rot16_512(v[14]);
  v[15] = rot16_512(v[15]);
  v[8] = جمع_کړه_512(v[8], v[12]);
  v[9] = جمع_کړه_512(v[9], v[13]);
  v[10] = جمع_کړه_512(v[10], v[14]);
  v[11] = جمع_کړه_512(v[11], v[15]);
  v[4] = xor_512(v[4], v[8]);
  v[5] = xor_512(v[5], v[9]);
  v[6] = xor_512(v[6], v[10]);
  v[7] = xor_512(v[7], v[11]);
  v[4] = rot12_512(v[4]);
  v[5] = rot12_512(v[5]);
  v[6] = rot12_512(v[6]);
  v[7] = rot12_512(v[7]);
  v[0] = جمع_کړه_512(v[0], m[(size_t)پیغام_مهالوېش[r][1]]);
  v[1] = جمع_کړه_512(v[1], m[(size_t)پیغام_مهالوېش[r][3]]);
  v[2] = جمع_کړه_512(v[2], m[(size_t)پیغام_مهالوېش[r][5]]);
  v[3] = جمع_کړه_512(v[3], m[(size_t)پیغام_مهالوېش[r][7]]);
  v[0] = جمع_کړه_512(v[0], v[4]);
  v[1] = جمع_کړه_512(v[1], v[5]);
  v[2] = جمع_کړه_512(v[2], v[6]);
  v[3] = جمع_کړه_512(v[3], v[7]);
  v[12] = xor_512(v[12], v[0]);
  v[13] = xor_512(v[13], v[1]);
  v[14] = xor_512(v[14], v[2]);
  v[15] = xor_512(v[15], v[3]);
  v[12] = rot8_512(v[12]);
  v[13] = rot8_512(v[13]);
  v[14] = rot8_512(v[14]);
  v[15] = rot8_512(v[15]);
  v[8] = جمع_کړه_512(v[8], v[12]);
  v[9] = جمع_کړه_512(v[9], v[13]);
  v[10] = جمع_کړه_512(v[10], v[14]);
  v[11] = جمع_کړه_512(v[11], v[15]);
  v[4] = xor_512(v[4], v[8]);
  v[5] = xor_512(v[5], v[9]);
  v[6] = xor_512(v[6], v[10]);
  v[7] = xor_512(v[7], v[11]);
  v[4] = rot7_512(v[4]);
  v[5] = rot7_512(v[5]);
  v[6] = rot7_512(v[6]);
  v[7] = rot7_512(v[7]);

  v[0] = جمع_کړه_512(v[0], m[(size_t)پیغام_مهالوېش[r][8]]);
  v[1] = جمع_کړه_512(v[1], m[(size_t)پیغام_مهالوېش[r][10]]);
  v[2] = جمع_کړه_512(v[2], m[(size_t)پیغام_مهالوېش[r][12]]);
  v[3] = جمع_کړه_512(v[3], m[(size_t)پیغام_مهالوېش[r][14]]);
  v[0] = جمع_کړه_512(v[0], v[5]);
  v[1] = جمع_کړه_512(v[1], v[6]);
  v[2] = جمع_کړه_512(v[2], v[7]);
  v[3] = جمع_کړه_512(v[3], v[4]);
  v[15] = xor_512(v[15], v[0]);
  v[12] = xor_512(v[12], v[1]);
  v[13] = xor_512(v[13], v[2]);
  v[14] = xor_512(v[14], v[3]);
  v[15] = rot16_512(v[15]);
  v[12] = rot16_512(v[12]);
  v[13] = rot16_512(v[13]);
  v[14] = rot16_512(v[14]);
  v[10] = جمع_کړه_512(v[10], v[15]);
  v[11] = جمع_کړه_512(v[11], v[12]);
  v[8] = جمع_کړه_512(v[8], v[13]);
  v[9] = جمع_کړه_512(v[9], v[14]);
  v[5] = xor_512(v[5], v[10]);
  v[6] = xor_512(v[6], v[11]);
  v[7] = xor_512(v[7], v[8]);
  v[4] = xor_512(v[4], v[9]);
  v[5] = rot12_512(v[5]);
  v[6] = rot12_512(v[6]);
  v[7] = rot12_512(v[7]);
  v[4] = rot12_512(v[4]);
  v[0] = جمع_کړه_512(v[0], m[(size_t)پیغام_مهالوېش[r][9]]);
  v[1] = جمع_کړه_512(v[1], m[(size_t)پیغام_مهالوېش[r][11]]);
  v[2] = جمع_کړه_512(v[2], m[(size_t)پیغام_مهالوېش[r][13]]);
  v[3] = جمع_کړه_512(v[3], m[(size_t)پیغام_مهالوېش[r][15]]);
  v[0] = جمع_کړه_512(v[0], v[5]);
  v[1] = جمع_کړه_512(v[1], v[6]);
  v[2] = جمع_کړه_512(v[2], v[7]);
  v[3] = جمع_کړه_512(v[3], v[4]);
  v[15] = xor_512(v[15], v[0]);
  v[12] = xor_512(v[12], v[1]);
  v[13] = xor_512(v[13], v[2]);
  v[14] = xor_512(v[14], v[3]);
  v[15] = rot8_512(v[15]);
  v[12] = rot8_512(v[12]);
  v[13] = rot8_512(v[13]);
  v[14] = rot8_512(v[14]);
  v[10] = جمع_کړه_512(v[10], v[15]);
  v[11] = جمع_کړه_512(v[11], v[12]);
  v[8] = جمع_کړه_512(v[8], v[13]);
  v[9] = جمع_کړه_512(v[9], v[14]);
  v[5] = xor_512(v[5], v[10]);
  v[6] = xor_512(v[6], v[11]);
  v[7] = xor_512(v[7], v[8]);
  v[4] = xor_512(v[4], v[9]);
  v[5] = rot7_512(v[5]);
  v[6] = rot7_512(v[6]);
  v[7] = rot7_512(v[7]);
  v[4] = rot7_512(v[4]);
}

// 0b10001000, or lanes a0/a2/b0/b2 in little-endian order
#تعریف_کړه LO_IMM8 0x88

INLINE __m512i unpack_lo_128(__m512i a, __m512i b) {
  أرجع _mm512_shuffle_i32x4(a, b, LO_IMM8);
}

// 0b11011101, or lanes a1/a3/b1/b3 in little-endian order
#تعریف_کړه HI_IMM8 0xdd

INLINE __m512i unpack_hi_128(__m512i a, __m512i b) {
  أرجع _mm512_shuffle_i32x4(a, b, HI_IMM8);
}

INLINE فراغ transpose_vecs_512(__m512i vecs[16]) {
  // Interleave 32-bit lanes. The _0 unpack is lanes
  // 0/0/1/1/4/4/5/5/8/8/9/9/12/12/13/13, and the _2 unpack is lanes
  // 2/2/3/3/6/6/7/7/10/10/11/11/14/14/15/15.
  __m512i ab_0 = _mm512_unpacklo_epi32(vecs[0], vecs[1]);
  __m512i ab_2 = _mm512_unpackhi_epi32(vecs[0], vecs[1]);
  __m512i cd_0 = _mm512_unpacklo_epi32(vecs[2], vecs[3]);
  __m512i cd_2 = _mm512_unpackhi_epi32(vecs[2], vecs[3]);
  __m512i ef_0 = _mm512_unpacklo_epi32(vecs[4], vecs[5]);
  __m512i ef_2 = _mm512_unpackhi_epi32(vecs[4], vecs[5]);
  __m512i gh_0 = _mm512_unpacklo_epi32(vecs[6], vecs[7]);
  __m512i gh_2 = _mm512_unpackhi_epi32(vecs[6], vecs[7]);
  __m512i ij_0 = _mm512_unpacklo_epi32(vecs[8], vecs[9]);
  __m512i ij_2 = _mm512_unpackhi_epi32(vecs[8], vecs[9]);
  __m512i kl_0 = _mm512_unpacklo_epi32(vecs[10], vecs[11]);
  __m512i kl_2 = _mm512_unpackhi_epi32(vecs[10], vecs[11]);
  __m512i mn_0 = _mm512_unpacklo_epi32(vecs[12], vecs[13]);
  __m512i mn_2 = _mm512_unpackhi_epi32(vecs[12], vecs[13]);
  __m512i op_0 = _mm512_unpacklo_epi32(vecs[14], vecs[15]);
  __m512i op_2 = _mm512_unpackhi_epi32(vecs[14], vecs[15]);

  // Interleave 64-bit lanes. The _0 unpack is lanes
  // 0/0/0/0/4/4/4/4/8/8/8/8/12/12/12/12, the _1 unpack is lanes
  // 1/1/1/1/5/5/5/5/9/9/9/9/13/13/13/13, the _2 unpack is lanes
  // 2/2/2/2/6/6/6/6/10/10/10/10/14/14/14/14, and the _3 unpack is lanes
  // 3/3/3/3/7/7/7/7/11/11/11/11/15/15/15/15.
  __m512i abcd_0 = _mm512_unpacklo_epi64(ab_0, cd_0);
  __m512i abcd_1 = _mm512_unpackhi_epi64(ab_0, cd_0);
  __m512i abcd_2 = _mm512_unpacklo_epi64(ab_2, cd_2);
  __m512i abcd_3 = _mm512_unpackhi_epi64(ab_2, cd_2);
  __m512i efgh_0 = _mm512_unpacklo_epi64(ef_0, gh_0);
  __m512i efgh_1 = _mm512_unpackhi_epi64(ef_0, gh_0);
  __m512i efgh_2 = _mm512_unpacklo_epi64(ef_2, gh_2);
  __m512i efgh_3 = _mm512_unpackhi_epi64(ef_2, gh_2);
  __m512i ijkl_0 = _mm512_unpacklo_epi64(ij_0, kl_0);
  __m512i ijkl_1 = _mm512_unpackhi_epi64(ij_0, kl_0);
  __m512i ijkl_2 = _mm512_unpacklo_epi64(ij_2, kl_2);
  __m512i ijkl_3 = _mm512_unpackhi_epi64(ij_2, kl_2);
  __m512i mnop_0 = _mm512_unpacklo_epi64(mn_0, op_0);
  __m512i mnop_1 = _mm512_unpackhi_epi64(mn_0, op_0);
  __m512i mnop_2 = _mm512_unpacklo_epi64(mn_2, op_2);
  __m512i mnop_3 = _mm512_unpackhi_epi64(mn_2, op_2);

  // Interleave 128-bit lanes. The _0 unpack is
  // 0/0/0/0/8/8/8/8/0/0/0/0/8/8/8/8, the _1 unpack is
  // 1/1/1/1/9/9/9/9/1/1/1/1/9/9/9/9, and so on.
  __m512i abcdefgh_0 = unpack_lo_128(abcd_0, efgh_0);
  __m512i abcdefgh_1 = unpack_lo_128(abcd_1, efgh_1);
  __m512i abcdefgh_2 = unpack_lo_128(abcd_2, efgh_2);
  __m512i abcdefgh_3 = unpack_lo_128(abcd_3, efgh_3);
  __m512i abcdefgh_4 = unpack_hi_128(abcd_0, efgh_0);
  __m512i abcdefgh_5 = unpack_hi_128(abcd_1, efgh_1);
  __m512i abcdefgh_6 = unpack_hi_128(abcd_2, efgh_2);
  __m512i abcdefgh_7 = unpack_hi_128(abcd_3, efgh_3);
  __m512i ijklmnop_0 = unpack_lo_128(ijkl_0, mnop_0);
  __m512i ijklmnop_1 = unpack_lo_128(ijkl_1, mnop_1);
  __m512i ijklmnop_2 = unpack_lo_128(ijkl_2, mnop_2);
  __m512i ijklmnop_3 = unpack_lo_128(ijkl_3, mnop_3);
  __m512i ijklmnop_4 = unpack_hi_128(ijkl_0, mnop_0);
  __m512i ijklmnop_5 = unpack_hi_128(ijkl_1, mnop_1);
  __m512i ijklmnop_6 = unpack_hi_128(ijkl_2, mnop_2);
  __m512i ijklmnop_7 = unpack_hi_128(ijkl_3, mnop_3);

  // Interleave 128-bit lanes again for the final outputs.
  vecs[0] = unpack_lo_128(abcdefgh_0, ijklmnop_0);
  vecs[1] = unpack_lo_128(abcdefgh_1, ijklmnop_1);
  vecs[2] = unpack_lo_128(abcdefgh_2, ijklmnop_2);
  vecs[3] = unpack_lo_128(abcdefgh_3, ijklmnop_3);
  vecs[4] = unpack_lo_128(abcdefgh_4, ijklmnop_4);
  vecs[5] = unpack_lo_128(abcdefgh_5, ijklmnop_5);
  vecs[6] = unpack_lo_128(abcdefgh_6, ijklmnop_6);
  vecs[7] = unpack_lo_128(abcdefgh_7, ijklmnop_7);
  vecs[8] = unpack_hi_128(abcdefgh_0, ijklmnop_0);
  vecs[9] = unpack_hi_128(abcdefgh_1, ijklmnop_1);
  vecs[10] = unpack_hi_128(abcdefgh_2, ijklmnop_2);
  vecs[11] = unpack_hi_128(abcdefgh_3, ijklmnop_3);
  vecs[12] = unpack_hi_128(abcdefgh_4, ijklmnop_4);
  vecs[13] = unpack_hi_128(abcdefgh_5, ijklmnop_5);
  vecs[14] = unpack_hi_128(abcdefgh_6, ijklmnop_6);
  vecs[15] = unpack_hi_128(abcdefgh_7, ijklmnop_7);
}

INLINE فراغ transpose_پیغام_vecs16(ثابت uint8_t *ثابت *inputs,
                                 size_t بلاک_دننه_واټن, __m512i بهر_2[16]) {
  بهر_2[0] = loadu_512(&inputs[0][بلاک_دننه_واټن]);
  بهر_2[1] = loadu_512(&inputs[1][بلاک_دننه_واټن]);
  بهر_2[2] = loadu_512(&inputs[2][بلاک_دننه_واټن]);
  بهر_2[3] = loadu_512(&inputs[3][بلاک_دننه_واټن]);
  بهر_2[4] = loadu_512(&inputs[4][بلاک_دننه_واټن]);
  بهر_2[5] = loadu_512(&inputs[5][بلاک_دننه_واټن]);
  بهر_2[6] = loadu_512(&inputs[6][بلاک_دننه_واټن]);
  بهر_2[7] = loadu_512(&inputs[7][بلاک_دننه_واټن]);
  بهر_2[8] = loadu_512(&inputs[8][بلاک_دننه_واټن]);
  بهر_2[9] = loadu_512(&inputs[9][بلاک_دننه_واټن]);
  بهر_2[10] = loadu_512(&inputs[10][بلاک_دننه_واټن]);
  بهر_2[11] = loadu_512(&inputs[11][بلاک_دننه_واټن]);
  بهر_2[12] = loadu_512(&inputs[12][بلاک_دننه_واټن]);
  بهر_2[13] = loadu_512(&inputs[13][بلاک_دننه_واټن]);
  بهر_2[14] = loadu_512(&inputs[14][بلاک_دننه_واټن]);
  بهر_2[15] = loadu_512(&inputs[15][بلاک_دننه_واټن]);
  لكل (size_t i = 0; i < 16; ++i) {
    _mm_prefetch((ثابت فراغ *)&inputs[i][بلاک_دننه_واټن + 256], _MM_HINT_T0);
  }
  transpose_vecs_512(بهر_2);
}

INLINE فراغ بارول_counters16(uint64_t counter, bool increment_counter,
                            __m512i *بهر_lo, __m512i *بهر_hi) {
  ثابت __m512i ماسک_2 = _mm512_set1_epi32(-(int32_t)increment_counter);
  ثابت __m512i deltas = mm512_سېټ_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  ثابت __m512i masked_deltas = _mm512_and_si512(deltas, ماسک_2);
  ثابت __m512i ټیټ_ټکي = mm512_جمع_کړه_epi32(
    _mm512_set1_epi32((int32_t)counter),
    masked_deltas);
  // The carry bit is 1 if the high bit of the word was 1 before addition and is
  // 0 after.
  // NOTE: It would be a bit more natural to use _mm512_cmp_epu32_mask to
  // compute the carry bits here, and originally we did, but that intrinsic is
  // broken under GCC 5.4. See https://github.com/BLAKE3-team/BLAKE3/issues/271.
  ثابت __m512i carries = _mm512_srli_epi32(
    _mm512_andnot_si512(
        ټیټ_ټکي, // 0 after (gets inverted by andnot)
        _mm512_set1_epi32((int32_t)counter)), // and 1 before
    31);
  ثابت __m512i لوړ_ټکي = mm512_جمع_کړه_epi32(
    _mm512_set1_epi32((int32_t)(counter >> 32)),
    carries);
  *بهر_lo = ټیټ_ټکي;
  *بهر_hi = لوړ_ټکي;
}

ساكن
فراغ blake3_hash16_avx512(ثابت uint8_t *ثابت *inputs, size_t بلاکونه,
                          ثابت uint32_t کلی_2[8], uint64_t counter,
                          bool increment_counter, uint8_t flags,
                          uint8_t flags_پیل, uint8_t flags_پای_ټکی,
                          uint8_t *بهر_2) {
  __m512i h_vecs[8] = {
      set1_512(کلی_2[0]), set1_512(کلی_2[1]), set1_512(کلی_2[2]), set1_512(کلی_2[3]),
      set1_512(کلی_2[4]), set1_512(کلی_2[5]), set1_512(کلی_2[6]), set1_512(کلی_2[7]),
  };
  __m512i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters16(counter, increment_counter, &counter_ټیټ_vec,
                  &counter_لوړ_vec);
  uint8_t بلاک_flags = flags | flags_پیل;

  لكل (size_t بلاک = 0; بلاک < بلاکونه; بلاک++) {
    إذا (بلاک + 1 == بلاکونه) {
      بلاک_flags |= flags_پای_ټکی;
    }
    __m512i بلاک_len_vec = set1_512(BLAKE3_بلاک_LEN);
    __m512i بلاک_flags_vec = set1_512(بلاک_flags);
    __m512i پیغام_vecs[16];
    transpose_پیغام_vecs16(inputs, بلاک * BLAKE3_بلاک_LEN, پیغام_vecs);

    __m512i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_512(IV[0]), set1_512(IV[1]),  set1_512(IV[2]), set1_512(IV[3]),
        counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
    };
    پړاو_fn16(v, پیغام_vecs, 0);
    پړاو_fn16(v, پیغام_vecs, 1);
    پړاو_fn16(v, پیغام_vecs, 2);
    پړاو_fn16(v, پیغام_vecs, 3);
    پړاو_fn16(v, پیغام_vecs, 4);
    پړاو_fn16(v, پیغام_vecs, 5);
    پړاو_fn16(v, پیغام_vecs, 6);
    h_vecs[0] = xor_512(v[0], v[8]);
    h_vecs[1] = xor_512(v[1], v[9]);
    h_vecs[2] = xor_512(v[2], v[10]);
    h_vecs[3] = xor_512(v[3], v[11]);
    h_vecs[4] = xor_512(v[4], v[12]);
    h_vecs[5] = xor_512(v[5], v[13]);
    h_vecs[6] = xor_512(v[6], v[14]);
    h_vecs[7] = xor_512(v[7], v[15]);

    بلاک_flags = flags;
  }

  // transpose_vecs_512 operates on a 16x16 matrix of words, but we only have 8
  // state vectors. Pad the matrix with zeros. After transposition, store the
  // lower half of each vector.
  __m512i padded[16] = {
      h_vecs[0],   h_vecs[1],   h_vecs[2],   h_vecs[3],
      h_vecs[4],   h_vecs[5],   h_vecs[6],   h_vecs[7],
      set1_512(0), set1_512(0), set1_512(0), set1_512(0),
      set1_512(0), set1_512(0), set1_512(0), set1_512(0),
  };
  transpose_vecs_512(padded);
  mm256_ماسک_storeu_epi32(&بهر_2[0 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[0]));
  mm256_ماسک_storeu_epi32(&بهر_2[1 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[1]));
  mm256_ماسک_storeu_epi32(&بهر_2[2 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[2]));
  mm256_ماسک_storeu_epi32(&بهر_2[3 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[3]));
  mm256_ماسک_storeu_epi32(&بهر_2[4 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[4]));
  mm256_ماسک_storeu_epi32(&بهر_2[5 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[5]));
  mm256_ماسک_storeu_epi32(&بهر_2[6 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[6]));
  mm256_ماسک_storeu_epi32(&بهر_2[7 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[7]));
  mm256_ماسک_storeu_epi32(&بهر_2[8 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[8]));
  mm256_ماسک_storeu_epi32(&بهر_2[9 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[9]));
  mm256_ماسک_storeu_epi32(&بهر_2[10 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[10]));
  mm256_ماسک_storeu_epi32(&بهر_2[11 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[11]));
  mm256_ماسک_storeu_epi32(&بهر_2[12 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[12]));
  mm256_ماسک_storeu_epi32(&بهر_2[13 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[13]));
  mm256_ماسک_storeu_epi32(&بهر_2[14 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[14]));
  mm256_ماسک_storeu_epi32(&بهر_2[15 * حجم(__m256i)], (__mmask8)-1, _mm512_castsi512_si256(padded[15]));
}

ساكن
فراغ blake3_xof16_avx512(ثابت uint32_t cv[8],
                        ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                        uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                        uint8_t بهر_2[16 * 64]) {
  __m512i h_vecs[8] = {
      set1_512(cv[0]), set1_512(cv[1]), set1_512(cv[2]), set1_512(cv[3]),
      set1_512(cv[4]), set1_512(cv[5]), set1_512(cv[6]), set1_512(cv[7]),
  };
  uint32_t بلاک_ټکي[16];
  بارول_بلاک_ټکي(بلاک, بلاک_ټکي);
  __m512i پیغام_vecs[16];
  لكل (size_t i = 0; i < 16; i++) {
      پیغام_vecs[i] = set1_512(بلاک_ټکي[i]);
  }
  __m512i counter_ټیټ_vec, counter_لوړ_vec;
  بارول_counters16(counter, true, &counter_ټیټ_vec, &counter_لوړ_vec);
  __m512i بلاک_len_vec = set1_512(بلاک_len);
  __m512i بلاک_flags_vec = set1_512(flags);
  __m512i v[16] = {
      h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
      h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
      set1_512(IV[0]), set1_512(IV[1]),  set1_512(IV[2]), set1_512(IV[3]),
      counter_ټیټ_vec, counter_لوړ_vec, بلاک_len_vec,   بلاک_flags_vec,
  };
  پړاو_fn16(v, پیغام_vecs, 0);
  پړاو_fn16(v, پیغام_vecs, 1);
  پړاو_fn16(v, پیغام_vecs, 2);
  پړاو_fn16(v, پیغام_vecs, 3);
  پړاو_fn16(v, پیغام_vecs, 4);
  پړاو_fn16(v, پیغام_vecs, 5);
  پړاو_fn16(v, پیغام_vecs, 6);
  لكل (size_t i = 0; i < 8; i++) {
      v[i] = xor_512(v[i], v[i+8]);
      v[i+8] = xor_512(v[i+8], h_vecs[i]);
  }
  transpose_vecs_512(&v[0]);
  لكل (size_t i = 0; i < 16; i++) {
      storeu_512(v[i], &بهر_2[i * حجم(__m512i)]);
  }
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_avx512
 * ----------------------------------------------------------------------------
 */

INLINE فراغ hash_one_avx512(ثابت uint8_t *ننوت_2, size_t بلاکونه,
                            ثابت uint32_t کلی_2[8], uint64_t counter,
                            uint8_t flags, uint8_t flags_پیل,
                            uint8_t flags_پای_ټکی, uint8_t بهر_2[BLAKE3_بهر_LEN]) {
  uint32_t cv[8];
  حافظه_نقل(cv, کلی_2, BLAKE3_کلی_LEN);
  uint8_t بلاک_flags = flags | flags_پیل;
  طالما (بلاکونه > 0) {
    إذا (بلاکونه == 1) {
      بلاک_flags |= flags_پای_ټکی;
    }
    blake3_compress_دننه_ځای_پرځای_کړه_avx512(cv, ننوت_2, BLAKE3_بلاک_LEN, counter,
                                    بلاک_flags);
    ننوت_2 = &ننوت_2[BLAKE3_بلاک_LEN];
    بلاکونه -= 1;
    بلاک_flags = flags;
  }
  حافظه_نقل(بهر_2, cv, BLAKE3_بهر_LEN);
}

فراغ blake3_hash_many_avx512(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                             size_t بلاکونه, ثابت uint32_t کلی_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_پیل,
                             uint8_t flags_پای_ټکی, uint8_t *بهر_2) {
  طالما (عدد_inputs >= 16) {
    blake3_hash16_avx512(inputs, بلاکونه, کلی_2, counter, increment_counter, flags,
                         flags_پیل, flags_پای_ټکی, بهر_2);
    إذا (increment_counter) {
      counter += 16;
    }
    inputs += 16;
    عدد_inputs -= 16;
    بهر_2 = &بهر_2[16 * BLAKE3_بهر_LEN];
  }
  طالما (عدد_inputs >= 8) {
    blake3_hash8_avx512(inputs, بلاکونه, کلی_2, counter, increment_counter, flags,
                        flags_پیل, flags_پای_ټکی, بهر_2);
    إذا (increment_counter) {
      counter += 8;
    }
    inputs += 8;
    عدد_inputs -= 8;
    بهر_2 = &بهر_2[8 * BLAKE3_بهر_LEN];
  }
  طالما (عدد_inputs >= 4) {
    blake3_hash4_avx512(inputs, بلاکونه, کلی_2, counter, increment_counter, flags,
                        flags_پیل, flags_پای_ټکی, بهر_2);
    إذا (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    عدد_inputs -= 4;
    بهر_2 = &بهر_2[4 * BLAKE3_بهر_LEN];
  }
  طالما (عدد_inputs > 0) {
    hash_one_avx512(inputs[0], بلاکونه, کلی_2, counter, flags, flags_پیل,
                    flags_پای_ټکی, بهر_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    عدد_inputs -= 1;
    بهر_2 = &بهر_2[BLAKE3_بهر_LEN];
  }
}

فراغ blake3_xof_many_avx512(ثابت uint32_t cv[8],
                            ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                            uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                            uint8_t* بهر_2, size_t outblocks) {
  طالما (outblocks >= 16) {
    blake3_xof16_avx512(cv, بلاک, بلاک_len, counter, flags, بهر_2);
    counter += 16;
    outblocks -= 16;
    بهر_2 += 16 * BLAKE3_بلاک_LEN;
  }
  طالما (outblocks >= 8) {
    blake3_xof8_avx512(cv, بلاک, بلاک_len, counter, flags, بهر_2);
    counter += 8;
    outblocks -= 8;
    بهر_2 += 8 * BLAKE3_بلاک_LEN;
  }
  طالما (outblocks >= 4) {
    blake3_xof4_avx512(cv, بلاک, بلاک_len, counter, flags, بهر_2);
    counter += 4;
    outblocks -= 4;
    بهر_2 += 4 * BLAKE3_بلاک_LEN;
  }
  طالما (outblocks > 0) {
    blake3_compress_xof_avx512(cv, بلاک, بلاک_len, counter, flags, بهر_2);
    counter += 1;
    outblocks -= 1;
    بهر_2 += BLAKE3_بلاک_LEN;
  }
}
