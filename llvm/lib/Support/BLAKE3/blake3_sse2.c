#அந்தர்பாவ "blake3_impl.h"

#அந்தர்பாவ <immintrin.h>

#define DEGREE 4

#define _mm_shuffle_ps2(a, b, c)                                               \
  (_mm_castps_si128(                                                           \
      _mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), (c))))

INLINE __m128i loadu(மாறிலி uint8_t src[16]) {
  திரும்பு _mm_loadu_si128((மாறிலி __m128i *)src);
}

INLINE வெறுமை storeu(__m128i src, uint8_t dest[16]) {
  _mm_storeu_si128((__m128i *)dest, src);
}

INLINE __m128i addv(__m128i a, __m128i b) { திரும்பு _mm_add_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m128i xorv(__m128i a, __m128i b) { திரும்பு _mm_xor_si128(a, b); }

INLINE __m128i set1(uint32_t x) { திரும்பு _mm_set1_epi32((int32_t)x); }

INLINE __m128i set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  திரும்பு _mm_setr_epi32((int32_t)a, (int32_t)b, (int32_t)c, (int32_t)d);
}

INLINE __m128i rot16(__m128i x) {
  திரும்பு _mm_shufflehi_epi16(_mm_shufflelo_epi16(x, 0xB1), 0xB1);
}

INLINE __m128i rot12(__m128i x) {
  திரும்பு xorv(_mm_srli_epi32(x, 12), _mm_slli_epi32(x, 32 - 12));
}

INLINE __m128i rot8(__m128i x) {
  திரும்பு xorv(_mm_srli_epi32(x, 8), _mm_slli_epi32(x, 32 - 8));
}

INLINE __m128i rot7(__m128i x) {
  திரும்பு xorv(_mm_srli_epi32(x, 7), _mm_slli_epi32(x, 32 - 7));
}

INLINE வெறுமை g1(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
               __m128i m) {
  *row0 = addv(addv(*row0, m), *row1);
  *row3 = xorv(*row3, *row0);
  *row3 = rot16(*row3);
  *row2 = addv(*row2, *row3);
  *row1 = xorv(*row1, *row2);
  *row1 = rot12(*row1);
}

INLINE வெறுமை g2(__m128i *row0, __m128i *row1, __m128i *row2, __m128i *row3,
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
INLINE வெறுமை diagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(2, 1, 0, 3));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(0, 3, 2, 1));
}

INLINE வெறுமை undiagonalize(__m128i *row0, __m128i *row2, __m128i *row3) {
  *row0 = _mm_shuffle_epi32(*row0, _MM_SHUFFLE(0, 3, 2, 1));
  *row3 = _mm_shuffle_epi32(*row3, _MM_SHUFFLE(1, 0, 3, 2));
  *row2 = _mm_shuffle_epi32(*row2, _MM_SHUFFLE(2, 1, 0, 3));
}

INLINE __m128i blend_epi16(__m128i a, __m128i b, மாறிலி int16_t imm8) {
  மாறிலி __m128i bits = _mm_set_epi16(0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01);
  __m128i mask = _mm_set1_epi16(imm8);
  mask = _mm_and_si128(mask, bits);
  mask = _mm_cmpeq_epi16(mask, bits);
  திரும்பு _mm_or_si128(_mm_and_si128(mask, b), _mm_andnot_si128(mask, a));
}

INLINE வெறுமை compress_pre(__m128i rows[4], மாறிலி uint32_t cv[8],
                         மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags) {
  rows[0] = loadu((uint8_t *)&cv[0]);
  rows[1] = loadu((uint8_t *)&cv[4]);
  rows[2] = set4(IV[0], IV[1], IV[2], IV[3]);
  rows[3] = set4(counter_கீழ்(counter), counter_உயர்வு(counter),
                 (uint32_t)block_len, (uint32_t)flags);

  __m128i m0 = loadu(&block[அளவு(__m128i) * 0]);
  __m128i m1 = loadu(&block[அளவு(__m128i) * 1]);
  __m128i m2 = loadu(&block[அளவு(__m128i) * 2]);
  __m128i m3 = loadu(&block[அளவு(__m128i) * 3]);

  __m128i t0, t1, t2, t3, tt;

  // Round 1. The first round permutes the message words from the original
  // input order, into the groups that get mixed in parallel.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(2, 0, 2, 0)); //  6  4  2  0
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 3, 1)); //  7  5  3  1
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(2, 0, 2, 0)); // 14 12 10  8
  t2 = _mm_shuffle_epi32(t2, _MM_SHUFFLE(2, 1, 0, 3));   // 12 10  8 14
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 1, 3, 1)); // 15 13 11  9
  t3 = _mm_shuffle_epi32(t3, _MM_SHUFFLE(2, 1, 0, 3));   // 13 11  9 15
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 2. This round and all following rounds apply a fixed permutation
  // to the message words from the round before.
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 3
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 4
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 5
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 6
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
  m0 = t0;
  m1 = t1;
  m2 = t2;
  m3 = t3;

  // Round 7
  t0 = _mm_shuffle_ps2(m0, m1, _MM_SHUFFLE(3, 1, 1, 2));
  t0 = _mm_shuffle_epi32(t0, _MM_SHUFFLE(0, 3, 2, 1));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t0);
  t1 = _mm_shuffle_ps2(m2, m3, _MM_SHUFFLE(3, 3, 2, 2));
  tt = _mm_shuffle_epi32(m0, _MM_SHUFFLE(0, 0, 3, 3));
  t1 = blend_epi16(tt, t1, 0xCC);
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t1);
  diagonalize(&rows[0], &rows[2], &rows[3]);
  t2 = _mm_unpacklo_epi64(m3, m1);
  tt = blend_epi16(t2, m2, 0xC0);
  t2 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(1, 3, 2, 0));
  g1(&rows[0], &rows[1], &rows[2], &rows[3], t2);
  t3 = _mm_unpackhi_epi32(m1, m3);
  tt = _mm_unpacklo_epi32(m2, t3);
  t3 = _mm_shuffle_epi32(tt, _MM_SHUFFLE(0, 1, 3, 2));
  g2(&rows[0], &rows[1], &rows[2], &rows[3], t3);
  undiagonalize(&rows[0], &rows[2], &rows[3]);
}

வெறுமை blake3_compress_in_place_sse2(uint32_t cv[8],
                                   மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags) {
  __m128i rows[4];
  compress_pre(rows, cv, block, block_len, counter, flags);
  storeu(xorv(rows[0], rows[2]), (uint8_t *)&cv[0]);
  storeu(xorv(rows[1], rows[3]), (uint8_t *)&cv[4]);
}

வெறுமை blake3_compress_xof_sse2(மாறிலி uint32_t cv[8],
                              மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]) {
  __m128i rows[4];
  compress_pre(rows, cv, block, block_len, counter, flags);
  storeu(xorv(rows[0], rows[2]), &out[0]);
  storeu(xorv(rows[1], rows[3]), &out[16]);
  storeu(xorv(rows[2], loadu((uint8_t *)&cv[0])), &out[32]);
  storeu(xorv(rows[3], loadu((uint8_t *)&cv[4])), &out[48]);
}

INLINE வெறுமை round_fn(__m128i v[16], __m128i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)செய்தி_SCHEDULE[r][0]]);
  v[1] = addv(v[1], m[(size_t)செய்தி_SCHEDULE[r][2]]);
  v[2] = addv(v[2], m[(size_t)செய்தி_SCHEDULE[r][4]]);
  v[3] = addv(v[3], m[(size_t)செய்தி_SCHEDULE[r][6]]);
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
  v[0] = addv(v[0], m[(size_t)செய்தி_SCHEDULE[r][1]]);
  v[1] = addv(v[1], m[(size_t)செய்தி_SCHEDULE[r][3]]);
  v[2] = addv(v[2], m[(size_t)செய்தி_SCHEDULE[r][5]]);
  v[3] = addv(v[3], m[(size_t)செய்தி_SCHEDULE[r][7]]);
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

  v[0] = addv(v[0], m[(size_t)செய்தி_SCHEDULE[r][8]]);
  v[1] = addv(v[1], m[(size_t)செய்தி_SCHEDULE[r][10]]);
  v[2] = addv(v[2], m[(size_t)செய்தி_SCHEDULE[r][12]]);
  v[3] = addv(v[3], m[(size_t)செய்தி_SCHEDULE[r][14]]);
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
  v[0] = addv(v[0], m[(size_t)செய்தி_SCHEDULE[r][9]]);
  v[1] = addv(v[1], m[(size_t)செய்தி_SCHEDULE[r][11]]);
  v[2] = addv(v[2], m[(size_t)செய்தி_SCHEDULE[r][13]]);
  v[3] = addv(v[3], m[(size_t)செய்தி_SCHEDULE[r][15]]);
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

INLINE வெறுமை transpose_vecs(__m128i vecs[DEGREE]) {
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

INLINE வெறுமை transpose_செய்தி_vecs(மாறிலி uint8_t *மாறிலி *inputs,
                               size_t block_offset, __m128i out[16]) {
  out[0] = loadu(&inputs[0][block_offset + 0 * அளவு(__m128i)]);
  out[1] = loadu(&inputs[1][block_offset + 0 * அளவு(__m128i)]);
  out[2] = loadu(&inputs[2][block_offset + 0 * அளவு(__m128i)]);
  out[3] = loadu(&inputs[3][block_offset + 0 * அளவு(__m128i)]);
  out[4] = loadu(&inputs[0][block_offset + 1 * அளவு(__m128i)]);
  out[5] = loadu(&inputs[1][block_offset + 1 * அளவு(__m128i)]);
  out[6] = loadu(&inputs[2][block_offset + 1 * அளவு(__m128i)]);
  out[7] = loadu(&inputs[3][block_offset + 1 * அளவு(__m128i)]);
  out[8] = loadu(&inputs[0][block_offset + 2 * அளவு(__m128i)]);
  out[9] = loadu(&inputs[1][block_offset + 2 * அளவு(__m128i)]);
  out[10] = loadu(&inputs[2][block_offset + 2 * அளவு(__m128i)]);
  out[11] = loadu(&inputs[3][block_offset + 2 * அளவு(__m128i)]);
  out[12] = loadu(&inputs[0][block_offset + 3 * அளவு(__m128i)]);
  out[13] = loadu(&inputs[1][block_offset + 3 * அளவு(__m128i)]);
  out[14] = loadu(&inputs[2][block_offset + 3 * அளவு(__m128i)]);
  out[15] = loadu(&inputs[3][block_offset + 3 * அளவு(__m128i)]);
  க்கு (size_t i = 0; i < 4; ++i) {
    _mm_prefetch((மாறிலி வெறுமை *)&inputs[i][block_offset + 256], _MM_HINT_T0);
  }
  transpose_vecs(&out[0]);
  transpose_vecs(&out[4]);
  transpose_vecs(&out[8]);
  transpose_vecs(&out[12]);
}

INLINE வெறுமை load_counters(uint64_t counter, bool increment_counter,
                          __m128i *out_lo, __m128i *out_hi) {
  மாறிலி __m128i mask = _mm_set1_epi32(-(int32_t)increment_counter);
  மாறிலி __m128i add0 = _mm_set_epi32(3, 2, 1, 0);
  மாறிலி __m128i add1 = _mm_and_si128(mask, add0);
  __m128i l = _mm_add_epi32(_mm_set1_epi32((int32_t)counter), add1);
  __m128i carry = _mm_cmpgt_epi32(_mm_xor_si128(add1, _mm_set1_epi32(0x80000000)), 
                                  _mm_xor_si128(   l, _mm_set1_epi32(0x80000000)));
  __m128i h = _mm_sub_epi32(_mm_set1_epi32((int32_t)(counter >> 32)), carry);
  *out_lo = l;
  *out_hi = h;
}

நிலையான
வெறுமை blake3_hash4_sse2(மாறிலி uint8_t *மாறிலி *inputs, size_t blocks,
                       மாறிலி uint32_t சாவி_2[8], uint64_t counter,
                       bool increment_counter, uint8_t flags,
                       uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
  __m128i h_vecs[8] = {
      set1(சாவி_2[0]), set1(சாவி_2[1]), set1(சாவி_2[2]), set1(சாவி_2[3]),
      set1(சாவி_2[4]), set1(சாவி_2[5]), set1(சாவி_2[6]), set1(சாவி_2[7]),
  };
  __m128i counter_கீழ்_vec, counter_உயர்வு_vec;
  load_counters(counter, increment_counter, &counter_கீழ்_vec,
                &counter_உயர்வு_vec);
  uint8_t block_flags = flags | flags_start;

  க்கு (size_t block = 0; block < blocks; block++) {
    எனில் (block + 1 == blocks) {
      block_flags |= flags_end;
    }
    __m128i block_len_vec = set1(BLAKE3_BLOCK_LEN);
    __m128i block_flags_vec = set1(block_flags);
    __m128i செய்தி_vecs[16];
    transpose_செய்தி_vecs(inputs, block * BLAKE3_BLOCK_LEN, செய்தி_vecs);

    __m128i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        counter_கீழ்_vec, counter_உயர்வு_vec, block_len_vec, block_flags_vec,
    };
    round_fn(v, செய்தி_vecs, 0);
    round_fn(v, செய்தி_vecs, 1);
    round_fn(v, செய்தி_vecs, 2);
    round_fn(v, செய்தி_vecs, 3);
    round_fn(v, செய்தி_vecs, 4);
    round_fn(v, செய்தி_vecs, 5);
    round_fn(v, செய்தி_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    block_flags = flags;
  }

  transpose_vecs(&h_vecs[0]);
  transpose_vecs(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu(h_vecs[0], &out[0 * அளவு(__m128i)]);
  storeu(h_vecs[4], &out[1 * அளவு(__m128i)]);
  storeu(h_vecs[1], &out[2 * அளவு(__m128i)]);
  storeu(h_vecs[5], &out[3 * அளவு(__m128i)]);
  storeu(h_vecs[2], &out[4 * அளவு(__m128i)]);
  storeu(h_vecs[6], &out[5 * அளவு(__m128i)]);
  storeu(h_vecs[3], &out[6 * அளவு(__m128i)]);
  storeu(h_vecs[7], &out[7 * அளவு(__m128i)]);
}

INLINE வெறுமை hash_one_sse2(மாறிலி uint8_t *உள்ளீடு_2, size_t blocks,
                          மாறிலி uint32_t சாவி_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_start,
                          uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, சாவி_2, BLAKE3_சாவி_LEN);
  uint8_t block_flags = flags | flags_start;
  போது (blocks > 0) {
    எனில் (blocks == 1) {
      block_flags |= flags_end;
    }
    blake3_compress_in_place_sse2(cv, உள்ளீடு_2, BLAKE3_BLOCK_LEN, counter,
                                  block_flags);
    உள்ளீடு_2 = &உள்ளீடு_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  memcpy(out, cv, BLAKE3_OUT_LEN);
}

வெறுமை blake3_hash_many_sse2(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                           size_t blocks, மாறிலி uint32_t சாவி_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out) {
  போது (இலக்கம்_inputs >= DEGREE) {
    blake3_hash4_sse2(inputs, blocks, சாவி_2, counter, increment_counter, flags,
                      flags_start, flags_end, out);
    எனில் (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    இலக்கம்_inputs -= DEGREE;
    out = &out[DEGREE * BLAKE3_OUT_LEN];
  }
  போது (இலக்கம்_inputs > 0) {
    hash_one_sse2(inputs[0], blocks, சாவி_2, counter, flags, flags_start,
                  flags_end, out);
    எனில் (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    இலக்கம்_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}
