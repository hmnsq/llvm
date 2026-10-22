#ضمن "blake3_impl.h"

#ضمن <immintrin.h>

#وصف_ڪريو DEGREE 4

#وصف_ڪريو _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu(ثابت uint8_t src[16]) {
  أرجع _mm_loadu_si128((ثابت __m128i *)src);
}

INLINE فراغ storeu(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE __m128i addv(__m128i a, __m128i b) { أرجع mm_جوڙيو_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m128i xorv(__m128i a, __m128i b) { أرجع _mm_xor_si128(a, b); }

INLINE __m128i set1(uint32_t x) { أرجع _mm_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  أرجع _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16(__m128i x) {
  أرجع _mm_shufflehi_epi16(_mm_shufflelo_epi16(x, 0xB1), 0xB1);
}

INLINE __m128i rot12(__m128i x) {
  أرجع xorv(_mm_srli_epi32(x, 12), _mm_slli_epi32(x, 32 - 12));
}

INLINE __m128i rot8(__m128i x) {
  أرجع xorv(_mm_srli_epi32(x, 8), _mm_slli_epi32(x, 32 - 8));
}

INLINE __m128i rot7(__m128i x) {
  أرجع xorv(_mm_srli_epi32(x, 7), _mm_slli_epi32(x, 32 - 7));
}

INLINE فراغ g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot16(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot12(*row1);
}

INLINE فراغ g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
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

INLINE __m128i blend_epi16(__m128i a, __m128i b, ثابت int16_t imm8) {
  ثابت __m128i بٽ = mm_سيٽ_epi16(0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01);
  __m128i نقاب_2 = _mm_set1_epi16(imm8);
  نقاب_2 = _mm_and_si128(نقاب_2, بٽ);
  نقاب_2 = _mm_cmpeq_epi16(نقاب_2, بٽ);
  أرجع _mm_or_si128(_mm_and_si128(نقاب_2, b), _mm_andnot_si128(نقاب_2, a));
}

INLINE فراغ compress_pre(__m128i صفون[4], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                         uint8_t بلاڪ_len, uint64_t counter, uint8_t flags) {
  صفون[0] = loadu((uint8_t *)&cv[0]);
  صفون[1] = loadu((uint8_t *)&cv[4]);
  صفون[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  صفون[3] = set4(counter_هيٺيون(counter), counter_مٿاهون(counter),
                 (uint32_t)بلاڪ_len, (uint32_t)flags);

  __m128i m0 = loadu(&بلاڪ[حجم(__m128i) * 0]);
  __m128i m1 = loadu(&بلاڪ[حجم(__m128i) * 1]);
  __m128i m2 = loadu(&بلاڪ[حجم(__m128i) * 2]);
  __m128i m3 = loadu(&بلاڪ[حجم(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t1);
  diagonalize(&صفون[0], &صفون[2], &صفون[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&صفون[0], &صفون[1], &صفون[2], &صفون[3], t3);
  undiagonalize(&صفون[0], &صفون[2], &صفون[3]);
}

فراغ blake3_compress_اندريون_رکو_sse2(uint32_t cv[8],
                                   ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                                   uint8_t بلاڪ_len, uint64_t counter,
                                   uint8_t flags) {
  __m128i صفون[4];
  compress_pre(صفون, cv, بلاڪ, بلاڪ_len, counter, flags);
  storeu(xorv(صفون[0], صفون[2]), (uint8_t *)&cv[0]);
  storeu(xorv(صفون[1], صفون[3]), (uint8_t *)&cv[4]);
}

فراغ blake3_compress_xof_sse2(ثابت uint32_t cv[8],
                              ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                              uint8_t بلاڪ_len, uint64_t counter,
                              uint8_t flags, uint8_t ٻاهريون_2[64]) {
  __m128i صفون[4];
  compress_pre(صفون, cv, بلاڪ, بلاڪ_len, counter, flags);
  storeu(xorv(صفون[0], صفون[2]), &ٻاهريون_2[0]);
  storeu(xorv(صفون[1], صفون[3]), &ٻاهريون_2[16]);
  storeu(xorv(صفون[2], loadu((uint8_t *)&cv[0])), &ٻاهريون_2[32]);
  storeu(xorv(صفون[3], loadu((uint8_t *)&cv[4])), &ٻاهريون_2[48]);
}

INLINE فراغ ڦيرو_fn(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)نياپو_وقت_جدول[r][0]]);
  v[1] = addv(v[1], m[(size_t)نياپو_وقت_جدول[r][2]]);
  v[2] = addv(v[2], m[(size_t)نياپو_وقت_جدول[r][4]]);
  v[3] = addv(v[3], m[(size_t)نياپو_وقت_جدول[r][6]]);
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
  v[0] = addv(v[0], m[(size_t)نياپو_وقت_جدول[r][1]]);
  v[1] = addv(v[1], m[(size_t)نياپو_وقت_جدول[r][3]]);
  v[2] = addv(v[2], m[(size_t)نياپو_وقت_جدول[r][5]]);
  v[3] = addv(v[3], m[(size_t)نياپو_وقت_جدول[r][7]]);
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

  v[0] = addv(v[0], m[(size_t)نياپو_وقت_جدول[r][8]]);
  v[1] = addv(v[1], m[(size_t)نياپو_وقت_جدول[r][10]]);
  v[2] = addv(v[2], m[(size_t)نياپو_وقت_جدول[r][12]]);
  v[3] = addv(v[3], m[(size_t)نياپو_وقت_جدول[r][14]]);
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
  v[0] = addv(v[0], m[(size_t)نياپو_وقت_جدول[r][9]]);
  v[1] = addv(v[1], m[(size_t)نياپو_وقت_جدول[r][11]]);
  v[2] = addv(v[2], m[(size_t)نياپو_وقت_جدول[r][13]]);
  v[3] = addv(v[3], m[(size_t)نياپو_وقت_جدول[r][15]]);
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

INLINE فراغ transpose_vecs(__m128i vecs[DEGREE]) {
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

INLINE فراغ transpose_نياپو_vecs(ثابت uint8_t *ثابت *inputs,
                               size_t بلاڪ_وٿي, __m128i ٻاهريون_2[16]) {
  ٻاهريون_2[0] = loadu(&inputs[0][بلاڪ_وٿي + 0 * حجم(__m128i)]);
  ٻاهريون_2[1] = loadu(&inputs[1][بلاڪ_وٿي + 0 * حجم(__m128i)]);
  ٻاهريون_2[2] = loadu(&inputs[2][بلاڪ_وٿي + 0 * حجم(__m128i)]);
  ٻاهريون_2[3] = loadu(&inputs[3][بلاڪ_وٿي + 0 * حجم(__m128i)]);
  ٻاهريون_2[4] = loadu(&inputs[0][بلاڪ_وٿي + 1 * حجم(__m128i)]);
  ٻاهريون_2[5] = loadu(&inputs[1][بلاڪ_وٿي + 1 * حجم(__m128i)]);
  ٻاهريون_2[6] = loadu(&inputs[2][بلاڪ_وٿي + 1 * حجم(__m128i)]);
  ٻاهريون_2[7] = loadu(&inputs[3][بلاڪ_وٿي + 1 * حجم(__m128i)]);
  ٻاهريون_2[8] = loadu(&inputs[0][بلاڪ_وٿي + 2 * حجم(__m128i)]);
  ٻاهريون_2[9] = loadu(&inputs[1][بلاڪ_وٿي + 2 * حجم(__m128i)]);
  ٻاهريون_2[10] = loadu(&inputs[2][بلاڪ_وٿي + 2 * حجم(__m128i)]);
  ٻاهريون_2[11] = loadu(&inputs[3][بلاڪ_وٿي + 2 * حجم(__m128i)]);
  ٻاهريون_2[12] = loadu(&inputs[0][بلاڪ_وٿي + 3 * حجم(__m128i)]);
  ٻاهريون_2[13] = loadu(&inputs[1][بلاڪ_وٿي + 3 * حجم(__m128i)]);
  ٻاهريون_2[14] = loadu(&inputs[2][بلاڪ_وٿي + 3 * حجم(__m128i)]);
  ٻاهريون_2[15] = loadu(&inputs[3][بلاڪ_وٿي + 3 * حجم(__m128i)]);
  لكل (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((ثابت فراغ *)&inputs[i][بلاڪ_وٿي + 256], _MM_HINT_T0);
  }
  transpose_vecs(&ٻاهريون_2[0]);
  transpose_vecs(&ٻاهريون_2[4]);
  transpose_vecs(&ٻاهريون_2[8]);
  transpose_vecs(&ٻاهريون_2[12]);
}

INLINE فراغ ڀريو_counters(uint64_t counter, bool increment_counter,
                          __m128i *ٻاهريون_lo, __m128i *ٻاهريون_hi) {
  ثابت __m128i نقاب_2 = _mm_set1_epi32(-(int32_t)increment_counter);
  ثابت __m128i add0 = mm_سيٽ_epi32(3, 2, 1, 0);
  ثابت __m128i add1 = _mm_and_si128(نقاب_2, add0);
  __m128i l = mm_جوڙيو_epi32(_mm_set1_epi32((int32_t)counter), add1);
  __m128i حاصل = _mm_cmpgt_epi32(_mm_xor_si128(add1, _mm_set1_epi32(0x80000000)), 
                                  _mm_xor_si128(   l, _mm_set1_epi32(0x80000000)));
  __m128i h = _mm_sub_epi32(_mm_set1_epi32((int32_t)(counter >> 32)), حاصل);
  *ٻاهريون_lo = l;
  *ٻاهريون_hi = h;
}

ساكن
فراغ blake3_hash4_sse2(ثابت uint8_t *ثابت *inputs, size_t بلاڪون,
                       ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                       bool increment_counter, uint8_t flags,
                       uint8_t flags_شروعات, uint8_t flags_پڇاڙي, uint8_t *ٻاهريون_2) {
  __m128i h_vecs[8] = {
      set1(ڪنجي_2[0]), set1(ڪنجي_2[1]), set1(ڪنجي_2[2]), set1(ڪنجي_2[3]),
      set1(ڪنجي_2[4]), set1(ڪنجي_2[5]), set1(ڪنجي_2[6]), set1(ڪنجي_2[7]),
  };
  __m128i counter_هيٺيون_vec, counter_مٿاهون_vec;
  ڀريو_counters(counter, increment_counter, &counter_هيٺيون_vec,
                &counter_مٿاهون_vec);
  uint8_t بلاڪ_flags = flags | flags_شروعات;

  لكل (size_t بلاڪ = 0; بلاڪ < بلاڪون; بلاڪ++) {
    إذا (بلاڪ + 1 == بلاڪون) {
      بلاڪ_flags |= flags_پڇاڙي;
    }
    __m128i بلاڪ_len_vec = set1(BLAKE3_بلاڪ_LEN);
    __m128i بلاڪ_flags_vec = set1(بلاڪ_flags);
    __m128i نياپو_vecs[16];
    transpose_نياپو_vecs(inputs, بلاڪ * BLAKE3_بلاڪ_LEN, نياپو_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        counter_هيٺيون_vec, counter_مٿاهون_vec, بلاڪ_len_vec, بلاڪ_flags_vec,
    };
    ڦيرو_fn(v, نياپو_vecs, 0);
    ڦيرو_fn(v, نياپو_vecs, 1);
    ڦيرو_fn(v, نياپو_vecs, 2);
    ڦيرو_fn(v, نياپو_vecs, 3);
    ڦيرو_fn(v, نياپو_vecs, 4);
    ڦيرو_fn(v, نياپو_vecs, 5);
    ڦيرو_fn(v, نياپو_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    بلاڪ_flags = flags;
  }

  transpose_vecs(&h_vecs[0]);
  transpose_vecs(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu(h_vecs[0], &ٻاهريون_2[0 * حجم(__m128i)]);
  storeu(h_vecs[4], &ٻاهريون_2[1 * حجم(__m128i)]);
  storeu(h_vecs[1], &ٻاهريون_2[2 * حجم(__m128i)]);
  storeu(h_vecs[5], &ٻاهريون_2[3 * حجم(__m128i)]);
  storeu(h_vecs[2], &ٻاهريون_2[4 * حجم(__m128i)]);
  storeu(h_vecs[6], &ٻاهريون_2[5 * حجم(__m128i)]);
  storeu(h_vecs[3], &ٻاهريون_2[6 * حجم(__m128i)]);
  storeu(h_vecs[7], &ٻاهريون_2[7 * حجم(__m128i)]);
}

INLINE فراغ hash_one_sse2(ثابت uint8_t *داخلا_2, size_t بلاڪون,
                          ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_شروعات,
                          uint8_t flags_پڇاڙي, uint8_t ٻاهريون_2[BLAKE3_ٻاهريون_LEN]) {
  uint32_t cv[8];
  يادگيري_نقل(cv, ڪنجي_2, BLAKE3_ڪنجي_LEN);
  uint8_t بلاڪ_flags = flags | flags_شروعات;
  طالما (بلاڪون > 0) {
    إذا (بلاڪون == 1) {
      بلاڪ_flags |= flags_پڇاڙي;
    }
    blake3_compress_اندريون_رکو_sse2(cv, داخلا_2, BLAKE3_بلاڪ_LEN, counter,
                                  بلاڪ_flags);
    داخلا_2 = &داخلا_2[BLAKE3_بلاڪ_LEN];
    بلاڪون -= 1;
    بلاڪ_flags = flags;
  }
  يادگيري_نقل(ٻاهريون_2, cv, BLAKE3_ٻاهريون_LEN);
}

فراغ blake3_hash_many_sse2(ثابت uint8_t *ثابت *inputs, size_t انگ_inputs,
                           size_t بلاڪون, ثابت uint32_t ڪنجي_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروعات,
                           uint8_t flags_پڇاڙي, uint8_t *ٻاهريون_2) {
  طالما (انگ_inputs >= DEGREE) {
    blake3_hash4_sse2(inputs, بلاڪون, ڪنجي_2, counter, increment_counter, flags,
                      flags_شروعات, flags_پڇاڙي, ٻاهريون_2);
    إذا (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    انگ_inputs -= DEGREE;
    ٻاهريون_2 = &ٻاهريون_2[DEGREE * BLAKE3_ٻاهريون_LEN];
  }
  طالما (انگ_inputs > 0) {
    hash_one_sse2(inputs[0], بلاڪون, ڪنجي_2, counter, flags, flags_شروعات,
                  flags_پڇاڙي, ٻاهريون_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    انگ_inputs -= 1;
    ٻاهريون_2 = &ٻاهريون_2[BLAKE3_ٻاهريون_LEN];
  }
}
