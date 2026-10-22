#ਅਨ੍ਤਰ੍ਭਾਵ "blake3_impl.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <immintrin.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEGREE 4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu(ਅਚਲ uint8_t src[16]) {
  ਵਾਪਸ _mm_loadu_si128((ਅਚਲ __m128i *)src);
}

INLINE ਖਾਲੀ storeu(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE __m128i addv(__m128i a, __m128i b) { ਵਾਪਸ mm_ਜੋੜੋ_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m128i xorv(__m128i a, __m128i b) { ਵਾਪਸ _mm_xor_si128(a, b); }

INLINE __m128i set1(uint32_t x) { ਵਾਪਸ _mm_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  ਵਾਪਸ _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16(__m128i x) {
  ਵਾਪਸ _mm_shufflehi_epi16(_mm_shufflelo_epi16(x, 0xB1), 0xB1);
}

INLINE __m128i rot12(__m128i x) {
  ਵਾਪਸ xorv(_mm_srli_epi32(x, 12), _mm_slli_epi32(x, 32 - 12));
}

INLINE __m128i rot8(__m128i x) {
  ਵਾਪਸ xorv(_mm_srli_epi32(x, 8), _mm_slli_epi32(x, 32 - 8));
}

INLINE __m128i rot7(__m128i x) {
  ਵਾਪਸ xorv(_mm_srli_epi32(x, 7), _mm_slli_epi32(x, 32 - 7));
}

INLINE ਖਾਲੀ g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot16(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot12(*row1);
}

INLINE ਖਾਲੀ g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
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
INLINE ਖਾਲੀ diagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(2, 1, 0, 3));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(0, 3, 2, 1));
}

INLINE ਖਾਲੀ undiagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(0, 3, 2, 1));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(2, 1, 0, 3));
}

INLINE __m128i blend_epi16(__m128i a, __m128i b, ਅਚਲ int16_t imm8) {
  ਅਚਲ __m128i ਬਿੱਟ = mm_ਸੈੱਟ_epi16(0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01);
  __m128i ਨਕਾਬ_2 = _mm_set1_epi16(imm8);
  ਨਕਾਬ_2 = _mm_and_si128(ਨਕਾਬ_2, ਬਿੱਟ);
  ਨਕਾਬ_2 = _mm_cmpeq_epi16(ਨਕਾਬ_2, ਬਿੱਟ);
  ਵਾਪਸ _mm_or_si128(_mm_and_si128(ਨਕਾਬ_2, b), _mm_andnot_si128(ਨਕਾਬ_2, a));
}

INLINE ਖਾਲੀ compress_pre(__m128i ਪੰਕਤੀਆਂ[4], ਅਚਲ uint32_t cv[8],
                         ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                         uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags) {
  ਪੰਕਤੀਆਂ[0] = loadu((uint8_t *)&cv[0]);
  ਪੰਕਤੀਆਂ[1] = loadu((uint8_t *)&cv[4]);
  ਪੰਕਤੀਆਂ[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  ਪੰਕਤੀਆਂ[3] = set4(counter_ਨੀਵਾਂ(counter), counter_ਉੱਚਾ(counter),
                 (uint32_t)ਖੰਡ_len, (uint32_t)flags);

  __m128i m0 = loadu(&ਖੰਡ[ਆਕਾਰ(__m128i) * 0]);
  __m128i m1 = loadu(&ਖੰਡ[ਆਕਾਰ(__m128i) * 1]);
  __m128i m2 = loadu(&ਖੰਡ[ਆਕਾਰ(__m128i) * 2]);
  __m128i m3 = loadu(&ਖੰਡ[ਆਕਾਰ(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t1);
  diagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[1], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3], t3);
  undiagonalize(&ਪੰਕਤੀਆਂ[0], &ਪੰਕਤੀਆਂ[2], &ਪੰਕਤੀਆਂ[3]);
}

ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse2(uint32_t cv[8],
                                   ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                   uint8_t ਖੰਡ_len, uint64_t counter,
                                   uint8_t flags) {
  __m128i ਪੰਕਤੀਆਂ[4];
  compress_pre(ਪੰਕਤੀਆਂ, cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
  storeu(xorv(ਪੰਕਤੀਆਂ[0], ਪੰਕਤੀਆਂ[2]), (uint8_t *)&cv[0]);
  storeu(xorv(ਪੰਕਤੀਆਂ[1], ਪੰਕਤੀਆਂ[3]), (uint8_t *)&cv[4]);
}

ਖਾਲੀ blake3_compress_xof_sse2(ਅਚਲ uint32_t cv[8],
                              ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                              uint8_t ਖੰਡ_len, uint64_t counter,
                              uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]) {
  __m128i ਪੰਕਤੀਆਂ[4];
  compress_pre(ਪੰਕਤੀਆਂ, cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
  storeu(xorv(ਪੰਕਤੀਆਂ[0], ਪੰਕਤੀਆਂ[2]), &ਬਾਹਰਲਾ_2[0]);
  storeu(xorv(ਪੰਕਤੀਆਂ[1], ਪੰਕਤੀਆਂ[3]), &ਬਾਹਰਲਾ_2[16]);
  storeu(xorv(ਪੰਕਤੀਆਂ[2], loadu((uint8_t *)&cv[0])), &ਬਾਹਰਲਾ_2[32]);
  storeu(xorv(ਪੰਕਤੀਆਂ[3], loadu((uint8_t *)&cv[4])), &ਬਾਹਰਲਾ_2[48]);
}

INLINE ਖਾਲੀ ਗੇੜਾ_fn(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][0]]);
  v[1] = addv(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][2]]);
  v[2] = addv(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][4]]);
  v[3] = addv(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][6]]);
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
  v[0] = addv(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][1]]);
  v[1] = addv(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][3]]);
  v[2] = addv(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][5]]);
  v[3] = addv(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][7]]);
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

  v[0] = addv(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][8]]);
  v[1] = addv(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][10]]);
  v[2] = addv(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][12]]);
  v[3] = addv(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][14]]);
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
  v[0] = addv(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][9]]);
  v[1] = addv(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][11]]);
  v[2] = addv(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][13]]);
  v[3] = addv(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][15]]);
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

INLINE ਖਾਲੀ transpose_vecs(__m128i vecs[DEGREE]) {
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

INLINE ਖਾਲੀ transpose_ਸੁਨੇਹਾ_vecs(ਅਚਲ uint8_t *ਅਚਲ *inputs,
                               size_t ਖੰਡ_ਵਿੱਥ, __m128i ਬਾਹਰਲਾ_2[16]) {
  ਬਾਹਰਲਾ_2[0] = loadu(&inputs[0][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[1] = loadu(&inputs[1][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[2] = loadu(&inputs[2][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[3] = loadu(&inputs[3][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[4] = loadu(&inputs[0][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[5] = loadu(&inputs[1][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[6] = loadu(&inputs[2][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[7] = loadu(&inputs[3][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[8] = loadu(&inputs[0][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[9] = loadu(&inputs[1][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[10] = loadu(&inputs[2][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[11] = loadu(&inputs[3][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[12] = loadu(&inputs[0][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[13] = loadu(&inputs[1][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[14] = loadu(&inputs[2][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(__m128i)]);
  ਬਾਹਰਲਾ_2[15] = loadu(&inputs[3][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(__m128i)]);
  ਲਈ (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((ਅਚਲ ਖਾਲੀ *)&inputs[i][ਖੰਡ_ਵਿੱਥ + 256], _MM_HINT_T0);
  }
  transpose_vecs(&ਬਾਹਰਲਾ_2[0]);
  transpose_vecs(&ਬਾਹਰਲਾ_2[4]);
  transpose_vecs(&ਬਾਹਰਲਾ_2[8]);
  transpose_vecs(&ਬਾਹਰਲਾ_2[12]);
}

INLINE ਖਾਲੀ ਲੱਦੋ_counters(uint64_t counter, bool increment_counter,
                          __m128i *ਬਾਹਰਲਾ_lo, __m128i *ਬਾਹਰਲਾ_hi) {
  ਅਚਲ __m128i ਨਕਾਬ_2 = _mm_set1_epi32(-(int32_t)increment_counter);
  ਅਚਲ __m128i add0 = mm_ਸੈੱਟ_epi32(3, 2, 1, 0);
  ਅਚਲ __m128i add1 = _mm_and_si128(ਨਕਾਬ_2, add0);
  __m128i l = mm_ਜੋੜੋ_epi32(_mm_set1_epi32((int32_t)counter), add1);
  __m128i ਹਾਸਲ = _mm_cmpgt_epi32(_mm_xor_si128(add1, _mm_set1_epi32(0x80000000)), 
                                  _mm_xor_si128(   l, _mm_set1_epi32(0x80000000)));
  __m128i h = _mm_sub_epi32(_mm_set1_epi32((int32_t)(counter >> 32)), ਹਾਸਲ);
  *ਬਾਹਰਲਾ_lo = l;
  *ਬਾਹਰਲਾ_hi = h;
}

ਸਥਿਰ
ਖਾਲੀ blake3_hash4_sse2(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਖੰਡਾਂ,
                       ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                       bool increment_counter, uint8_t flags,
                       uint8_t flags_ਸ਼ੁਰੂ, uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2) {
  __m128i h_vecs[8] = {
      set1(ਕੁੰਜੀ_2[0]), set1(ਕੁੰਜੀ_2[1]), set1(ਕੁੰਜੀ_2[2]), set1(ਕੁੰਜੀ_2[3]),
      set1(ਕੁੰਜੀ_2[4]), set1(ਕੁੰਜੀ_2[5]), set1(ਕੁੰਜੀ_2[6]), set1(ਕੁੰਜੀ_2[7]),
  };
  __m128i counter_ਨੀਵਾਂ_vec, counter_ਉੱਚਾ_vec;
  ਲੱਦੋ_counters(counter, increment_counter, &counter_ਨੀਵਾਂ_vec,
                &counter_ਉੱਚਾ_vec);
  uint8_t ਖੰਡ_flags = flags | flags_ਸ਼ੁਰੂ;

  ਲਈ (size_t ਖੰਡ = 0; ਖੰਡ < ਖੰਡਾਂ; ਖੰਡ++) {
    ਜੇ (ਖੰਡ + 1 == ਖੰਡਾਂ) {
      ਖੰਡ_flags |= flags_ਸਿਰਾ;
    }
    __m128i ਖੰਡ_len_vec = set1(BLAKE3_ਖੰਡ_LEN);
    __m128i ਖੰਡ_flags_vec = set1(ਖੰਡ_flags);
    __m128i ਸੁਨੇਹਾ_vecs[16];
    transpose_ਸੁਨੇਹਾ_vecs(inputs, ਖੰਡ * BLAKE3_ਖੰਡ_LEN, ਸੁਨੇਹਾ_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        counter_ਨੀਵਾਂ_vec, counter_ਉੱਚਾ_vec, ਖੰਡ_len_vec, ਖੰਡ_flags_vec,
    };
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 0);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 1);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 2);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 3);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 4);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 5);
    ਗੇੜਾ_fn(v, ਸੁਨੇਹਾ_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    ਖੰਡ_flags = flags;
  }

  transpose_vecs(&h_vecs[0]);
  transpose_vecs(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu(h_vecs[0], &ਬਾਹਰਲਾ_2[0 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[4], &ਬਾਹਰਲਾ_2[1 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[1], &ਬਾਹਰਲਾ_2[2 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[5], &ਬਾਹਰਲਾ_2[3 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[2], &ਬਾਹਰਲਾ_2[4 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[6], &ਬਾਹਰਲਾ_2[5 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[3], &ਬਾਹਰਲਾ_2[6 * ਆਕਾਰ(__m128i)]);
  storeu(h_vecs[7], &ਬਾਹਰਲਾ_2[7 * ਆਕਾਰ(__m128i)]);
}

INLINE ਖਾਲੀ hash_one_sse2(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਖੰਡਾਂ,
                          ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                          uint8_t flags_ਸਿਰਾ, uint8_t ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN]) {
  uint32_t cv[8];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(cv, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  uint8_t ਖੰਡ_flags = flags | flags_ਸ਼ੁਰੂ;
  ਜਦੋਂ (ਖੰਡਾਂ > 0) {
    ਜੇ (ਖੰਡਾਂ == 1) {
      ਖੰਡ_flags |= flags_ਸਿਰਾ;
    }
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse2(cv, ਨਿਵੇਸ਼_2, BLAKE3_ਖੰਡ_LEN, counter,
                                  ਖੰਡ_flags);
    ਨਿਵੇਸ਼_2 = &ਨਿਵੇਸ਼_2[BLAKE3_ਖੰਡ_LEN];
    ਖੰਡਾਂ -= 1;
    ਖੰਡ_flags = flags;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, cv, BLAKE3_ਬਾਹਰਲਾ_LEN);
}

ਖਾਲੀ blake3_hash_many_sse2(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                           size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                           uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2) {
  ਜਦੋਂ (ਅੰਕ_inputs >= DEGREE) {
    blake3_hash4_sse2(inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter, increment_counter, flags,
                      flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
    ਜੇ (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    ਅੰਕ_inputs -= DEGREE;
    ਬਾਹਰਲਾ_2 = &ਬਾਹਰਲਾ_2[DEGREE * BLAKE3_ਬਾਹਰਲਾ_LEN];
  }
  ਜਦੋਂ (ਅੰਕ_inputs > 0) {
    hash_one_sse2(inputs[0], ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter, flags, flags_ਸ਼ੁਰੂ,
                  flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
    ਜੇ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ਅੰਕ_inputs -= 1;
    ਬਾਹਰਲਾ_2 = &ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN];
  }
}
