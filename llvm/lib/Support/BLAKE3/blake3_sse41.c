#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

#ಅನ್ತರ್ಭಾವ <immintrin.h>

#ವ್ಯಾಖ್ಯಾನಿಸು DEGREE 4

#ವ್ಯಾಖ್ಯಾನಿಸು _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu(ಸ್ಥಿರಾಂಕ uint8_t src[16]) {
  ಹಿಂತಿರುಗಿ _mm_loadu_si128((ಸ್ಥಿರಾಂಕ __m128i *)src);
}

INLINE ಶೂನ್ಯ storeu(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE __m128i addv(__m128i a, __m128i b) { ಹಿಂತಿರುಗಿ mm_ಕೂಡಿಸು_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m128i xorv(__m128i a, __m128i b) { ಹಿಂತಿರುಗಿ _mm_xor_si128(a, b); }

INLINE __m128i set1(uint32_t x) { ಹಿಂತಿರುಗಿ _mm_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  ಹಿಂತಿರುಗಿ _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16(__m128i x) {
  ಹಿಂತಿರುಗಿ _mm_shuffle_epi8(
      x, mm_ಗಣ_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
}

INLINE __m128i rot12(__m128i x) {
  ಹಿಂತಿರುಗಿ xorv(_mm_srli_epi32(x, 12), _mm_slli_epi32(x, 32 - 12));
}

INLINE __m128i rot8(__m128i x) {
  ಹಿಂತಿರುಗಿ _mm_shuffle_epi8(
      x, mm_ಗಣ_epi8(12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1));
}

INLINE __m128i rot7(__m128i x) {
  ಹಿಂತಿರುಗಿ xorv(_mm_srli_epi32(x, 7), _mm_slli_epi32(x, 32 - 7));
}

INLINE ಶೂನ್ಯ g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot16(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot12(*row1);
}

INLINE ಶೂನ್ಯ g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
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
INLINE ಶೂನ್ಯ diagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(2, 1, 0, 3));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(0, 3, 2, 1));
}

INLINE ಶೂನ್ಯ undiagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(0, 3, 2, 1));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(2, 1, 0, 3));
}

INLINE ಶೂನ್ಯ compress_pre(__m128i ಅಡ್ಡಸಾಲುಗಳು[4], ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                         ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                         uint8_t ಖಂಡ_len, uint64_t counter, uint8_t flags) {
  ಅಡ್ಡಸಾಲುಗಳು[0] = loadu((uint8_t *)&cv[0]);
  ಅಡ್ಡಸಾಲುಗಳು[1] = loadu((uint8_t *)&cv[4]);
  ಅಡ್ಡಸಾಲುಗಳು[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  ಅಡ್ಡಸಾಲುಗಳು[3] = set4(counter_ಕೆಳಗಿನ(counter), counter_ಮೇಲಿನ(counter),
                 (uint32_t)ಖಂಡ_len, (uint32_t)flags);

  __m128i m0 = loadu(&ಖಂಡ[ಗಾತ್ರ(__m128i) * 0]);
  __m128i m1 = loadu(&ಖಂಡ[ಗಾತ್ರ(__m128i) * 1]);
  __m128i m2 = loadu(&ಖಂಡ[ಗಾತ್ರ(__m128i) * 2]);
  __m128i m3 = loadu(&ಖಂಡ[ಗಾತ್ರ(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = _mm_blend_epi16(tt, t1, 0xCC);
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t1);
  diagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = _mm_blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[1], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3], t3);
  undiagonalize(&ಅಡ್ಡಸಾಲುಗಳು[0], &ಅಡ್ಡಸಾಲುಗಳು[2], &ಅಡ್ಡಸಾಲುಗಳು[3]);
}

ಶೂನ್ಯ blake3_compress_ಒಳ_ಇರಿಸು_sse41(uint32_t cv[8],
                                    ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                                    uint8_t ಖಂಡ_len, uint64_t counter,
                                    uint8_t flags) {
  __m128i ಅಡ್ಡಸಾಲುಗಳು[4];
  compress_pre(ಅಡ್ಡಸಾಲುಗಳು, cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[0], ಅಡ್ಡಸಾಲುಗಳು[2]), (uint8_t *)&cv[0]);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[1], ಅಡ್ಡಸಾಲುಗಳು[3]), (uint8_t *)&cv[4]);
}

ಶೂನ್ಯ blake3_compress_xof_sse41(ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                               ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                               uint8_t ಖಂಡ_len, uint64_t counter,
                               uint8_t flags, uint8_t ಹೊರಹೋಗು_2[64]) {
  __m128i ಅಡ್ಡಸಾಲುಗಳು[4];
  compress_pre(ಅಡ್ಡಸಾಲುಗಳು, cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[0], ಅಡ್ಡಸಾಲುಗಳು[2]), &ಹೊರಹೋಗು_2[0]);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[1], ಅಡ್ಡಸಾಲುಗಳು[3]), &ಹೊರಹೋಗು_2[16]);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[2], loadu((uint8_t *)&cv[0])), &ಹೊರಹೋಗು_2[32]);
  storeu(xorv(ಅಡ್ಡಸಾಲುಗಳು[3], loadu((uint8_t *)&cv[4])), &ಹೊರಹೋಗು_2[48]);
}

INLINE ಶೂನ್ಯ ಸುತ್ತು_fn(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][0]]);
  v[1] = addv(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][2]]);
  v[2] = addv(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][4]]);
  v[3] = addv(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][6]]);
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
  v[0] = addv(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][1]]);
  v[1] = addv(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][3]]);
  v[2] = addv(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][5]]);
  v[3] = addv(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][7]]);
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

  v[0] = addv(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][8]]);
  v[1] = addv(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][10]]);
  v[2] = addv(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][12]]);
  v[3] = addv(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][14]]);
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
  v[0] = addv(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][9]]);
  v[1] = addv(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][11]]);
  v[2] = addv(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][13]]);
  v[3] = addv(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][15]]);
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

INLINE ಶೂನ್ಯ transpose_vecs(__m128i vecs[DEGREE]) {
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

INLINE ಶೂನ್ಯ transpose_ಸಂದೇಶ_vecs(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs,
                               size_t ಖಂಡ_ಸ್ಥಾನಾಂತರ, __m128i ಹೊರಹೋಗು_2[16]) {
  ಹೊರಹೋಗು_2[0] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[1] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[2] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[3] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[4] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[5] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[6] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[7] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[8] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[9] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[10] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[11] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[12] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[13] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[14] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(__m128i)]);
  ಹೊರಹೋಗು_2[15] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(__m128i)]);
  ಗಾಗಿ (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)&inputs[i][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 256], _MM_HINT_T0);
  }
  transpose_vecs(&ಹೊರಹೋಗು_2[0]);
  transpose_vecs(&ಹೊರಹೋಗು_2[4]);
  transpose_vecs(&ಹೊರಹೋಗು_2[8]);
  transpose_vecs(&ಹೊರಹೋಗು_2[12]);
}

INLINE ಶೂನ್ಯ ತುಂಬು_counters(uint64_t counter, bool increment_counter,
                          __m128i *ಹೊರಹೋಗು_lo, __m128i *ಹೊರಹೋಗು_hi) {
  ಸ್ಥಿರಾಂಕ __m128i ಮುಖವಾಡ_2 = _mm_set1_epi32(-(int32_t)increment_counter);
  ಸ್ಥಿರಾಂಕ __m128i add0 = mm_ಗಣ_epi32(3, 2, 1, 0);
  ಸ್ಥಿರಾಂಕ __m128i add1 = _mm_and_si128(ಮುಖವಾಡ_2, add0);
  __m128i l = mm_ಕೂಡಿಸು_epi32(_mm_set1_epi32((int32_t)counter), add1);
  __m128i ವಾಹಕ = _mm_cmpgt_epi32(_mm_xor_si128(add1, _mm_set1_epi32(0x80000000)), 
                                  _mm_xor_si128(   l, _mm_set1_epi32(0x80000000)));
  __m128i h = _mm_sub_epi32(_mm_set1_epi32((int32_t)(counter >> 32)), ವಾಹಕ);
  *ಹೊರಹೋಗು_lo = l;
  *ಹೊರಹೋಗು_hi = h;
}

ಸ್ಥಿರ
ಶೂನ್ಯ blake3_hash4_sse41(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಖಂಡಗಳು,
                        ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                        bool increment_counter, uint8_t flags,
                        uint8_t flags_ಆರಂಭ, uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  __m128i h_vecs[8] = {
      set1(ಕೀಲಿ_2[0]), set1(ಕೀಲಿ_2[1]), set1(ಕೀಲಿ_2[2]), set1(ಕೀಲಿ_2[3]),
      set1(ಕೀಲಿ_2[4]), set1(ಕೀಲಿ_2[5]), set1(ಕೀಲಿ_2[6]), set1(ಕೀಲಿ_2[7]),
  };
  __m128i counter_ಕೆಳಗಿನ_vec, counter_ಮೇಲಿನ_vec;
  ತುಂಬು_counters(counter, increment_counter, &counter_ಕೆಳಗಿನ_vec,
                &counter_ಮೇಲಿನ_vec);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;

  ಗಾಗಿ (size_t ಖಂಡ = 0; ಖಂಡ < ಖಂಡಗಳು; ಖಂಡ++) {
    ಆದರೆ (ಖಂಡ + 1 == ಖಂಡಗಳು) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    __m128i ಖಂಡ_len_vec = set1(BLAKE3_ಖಂಡ_LEN);
    __m128i ಖಂಡ_flags_vec = set1(ಖಂಡ_flags);
    __m128i ಸಂದೇಶ_vecs[16];
    transpose_ಸಂದೇಶ_vecs(inputs, ಖಂಡ * BLAKE3_ಖಂಡ_LEN, ಸಂದೇಶ_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        counter_ಕೆಳಗಿನ_vec, counter_ಮೇಲಿನ_vec, ಖಂಡ_len_vec, ಖಂಡ_flags_vec,
    };
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 0);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 1);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 2);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 3);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 4);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 5);
    ಸುತ್ತು_fn(v, ಸಂದೇಶ_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    ಖಂಡ_flags = flags;
  }

  transpose_vecs(&h_vecs[0]);
  transpose_vecs(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu(h_vecs[0], &ಹೊರಹೋಗು_2[0 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[4], &ಹೊರಹೋಗು_2[1 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[1], &ಹೊರಹೋಗು_2[2 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[5], &ಹೊರಹೋಗು_2[3 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[2], &ಹೊರಹೋಗು_2[4 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[6], &ಹೊರಹೋಗು_2[5 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[3], &ಹೊರಹೋಗು_2[6 * ಗಾತ್ರ(__m128i)]);
  storeu(h_vecs[7], &ಹೊರಹೋಗು_2[7 * ಗಾತ್ರ(__m128i)]);
}

INLINE ಶೂನ್ಯ hash_one_sse41(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಖಂಡಗಳು,
                           ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                           uint8_t flags, uint8_t flags_ಆರಂಭ,
                           uint8_t flags_ಕೊನೆ, uint8_t ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN]) {
  uint32_t cv[8];
  ಸ್ಮರಣೆ_ನಕಲು(cv, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;
  ಆಗ (ಖಂಡಗಳು > 0) {
    ಆದರೆ (ಖಂಡಗಳು == 1) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    blake3_compress_ಒಳ_ಇರಿಸು_sse41(cv, ಒಳಹರಿವು_2, BLAKE3_ಖಂಡ_LEN, counter,
                                   ಖಂಡ_flags);
    ಒಳಹರಿವು_2 = &ಒಳಹರಿವು_2[BLAKE3_ಖಂಡ_LEN];
    ಖಂಡಗಳು -= 1;
    ಖಂಡ_flags = flags;
  }
  ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, cv, BLAKE3_ಹೊರಹೋಗು_LEN);
}

ಶೂನ್ಯ blake3_hash_many_sse41(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                            size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_ಆರಂಭ,
                            uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  ಆಗ (ಅಂಕೆ_inputs >= DEGREE) {
    blake3_hash4_sse41(inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter, increment_counter, flags,
                       flags_ಆರಂಭ, flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    ಅಂಕೆ_inputs -= DEGREE;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[DEGREE * BLAKE3_ಹೊರಹೋಗು_LEN];
  }
  ಆಗ (ಅಂಕೆ_inputs > 0) {
    hash_one_sse41(inputs[0], ಖಂಡಗಳು, ಕೀಲಿ_2, counter, flags, flags_ಆರಂಭ,
                   flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ಅಂಕೆ_inputs -= 1;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN];
  }
}
