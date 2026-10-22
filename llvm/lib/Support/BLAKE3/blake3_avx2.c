#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

#ಅನ್ತರ್ಭಾವ <immintrin.h>

#ವ್ಯಾಖ್ಯಾನಿಸು DEGREE 8

INLINE __m256i loadu(ಸ್ಥಿರಾಂಕ uint8_t src[32]) {
  ಹಿಂತಿರುಗಿ _mm256_loadu_si256((ಸ್ಥಿರಾಂಕ __m256i *)src);
}

INLINE ಶೂನ್ಯ storeu(__m256i src, uint8_t dest[16]) {
  _mm256_storeu_si256((__m256i *)dest, src);
}

INLINE __m256i addv(__m256i a, __m256i b) { ಹಿಂತಿರುಗಿ mm256_ಕೂಡಿಸು_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m256i xorv(__m256i a, __m256i b) { ಹಿಂತಿರುಗಿ _mm256_xor_si256(a, b); }

INLINE __m256i set1(uint32_t x) { ಹಿಂತಿರುಗಿ _mm256_set1_epi32((int32_t)x); }

INLINE __m256i rot16(__m256i x) {
  ಹಿಂತಿರುಗಿ _mm256_shuffle_epi8(
      x, mm256_ಗಣ_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2,
                         13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
}

INLINE __m256i rot12(__m256i x) {
  ಹಿಂತಿರುಗಿ _mm256_or_si256(_mm256_srli_epi32(x, 12), _mm256_slli_epi32(x, 32 - 12));
}

INLINE __m256i rot8(__m256i x) {
  ಹಿಂತಿರುಗಿ _mm256_shuffle_epi8(
      x, mm256_ಗಣ_epi8(12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1,
                         12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1));
}

INLINE __m256i rot7(__m256i x) {
  ಹಿಂತಿರುಗಿ _mm256_or_si256(_mm256_srli_epi32(x, 7), _mm256_slli_epi32(x, 32 - 7));
}

INLINE ಶೂನ್ಯ ಸುತ್ತು_fn(__m256i v[16], __m256i m[16], size_t r) {
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

INLINE ಶೂನ್ಯ transpose_vecs(__m256i vecs[DEGREE]) {
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

INLINE ಶೂನ್ಯ transpose_ಸಂದೇಶ_vecs(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs,
                               size_t ಖಂಡ_ಸ್ಥಾನಾಂತರ, __m256i ಹೊರಹೋಗು_2[16]) {
  ಹೊರಹೋಗು_2[0] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[1] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[2] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[3] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[4] = loadu(&inputs[4][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[5] = loadu(&inputs[5][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[6] = loadu(&inputs[6][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[7] = loadu(&inputs[7][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[8] = loadu(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[9] = loadu(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[10] = loadu(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[11] = loadu(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[12] = loadu(&inputs[4][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[13] = loadu(&inputs[5][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[14] = loadu(&inputs[6][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಹೊರಹೋಗು_2[15] = loadu(&inputs[7][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(__m256i)]);
  ಗಾಗಿ (size_t i = 0; i < 8; ++i) {
    _mm_prefetch((ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)&inputs[i][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 256], _MM_HINT_T0);
  }
  transpose_vecs(&ಹೊರಹೋಗು_2[0]);
  transpose_vecs(&ಹೊರಹೋಗು_2[8]);
}

INLINE ಶೂನ್ಯ ತುಂಬು_counters(uint64_t counter, bool increment_counter,
                          __m256i *ಹೊರಹೋಗು_lo, __m256i *ಹೊರಹೋಗು_hi) {
  ಸ್ಥಿರಾಂಕ __m256i ಮುಖವಾಡ_2 = _mm256_set1_epi32(-(int32_t)increment_counter);
  ಸ್ಥಿರಾಂಕ __m256i add0 = mm256_ಗಣ_epi32(7, 6, 5, 4, 3, 2, 1, 0);
  ಸ್ಥಿರಾಂಕ __m256i add1 = _mm256_and_si256(ಮುಖವಾಡ_2, add0);
  __m256i l = mm256_ಕೂಡಿಸು_epi32(_mm256_set1_epi32((int32_t)counter), add1);
  __m256i ವಾಹಕ = _mm256_cmpgt_epi32(_mm256_xor_si256(add1, _mm256_set1_epi32(0x80000000)), 
                                     _mm256_xor_si256(   l, _mm256_set1_epi32(0x80000000)));
  __m256i h = _mm256_sub_epi32(_mm256_set1_epi32((int32_t)(counter >> 32)), ವಾಹಕ);
  *ಹೊರಹೋಗು_lo = l;
  *ಹೊರಹೋಗು_hi = h;
}

ಸ್ಥಿರ
ಶೂನ್ಯ blake3_hash8_avx2(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಖಂಡಗಳು,
                       ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                       bool increment_counter, uint8_t flags,
                       uint8_t flags_ಆರಂಭ, uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  __m256i h_vecs[8] = {
      set1(ಕೀಲಿ_2[0]), set1(ಕೀಲಿ_2[1]), set1(ಕೀಲಿ_2[2]), set1(ಕೀಲಿ_2[3]),
      set1(ಕೀಲಿ_2[4]), set1(ಕೀಲಿ_2[5]), set1(ಕೀಲಿ_2[6]), set1(ಕೀಲಿ_2[7]),
  };
  __m256i counter_ಕೆಳಗಿನ_vec, counter_ಮೇಲಿನ_vec;
  ತುಂಬು_counters(counter, increment_counter, &counter_ಕೆಳಗಿನ_vec,
                &counter_ಮೇಲಿನ_vec);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;

  ಗಾಗಿ (size_t ಖಂಡ = 0; ಖಂಡ < ಖಂಡಗಳು; ಖಂಡ++) {
    ಆದರೆ (ಖಂಡ + 1 == ಖಂಡಗಳು) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    __m256i ಖಂಡ_len_vec = set1(BLAKE3_ಖಂಡ_LEN);
    __m256i ಖಂಡ_flags_vec = set1(ಖಂಡ_flags);
    __m256i ಸಂದೇಶ_vecs[16];
    transpose_ಸಂದೇಶ_vecs(inputs, ಖಂಡ * BLAKE3_ಖಂಡ_LEN, ಸಂದೇಶ_vecs);

    __m256i v[16] = {
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

  transpose_vecs(h_vecs);
  storeu(h_vecs[0], &ಹೊರಹೋಗು_2[0 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[1], &ಹೊರಹೋಗು_2[1 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[2], &ಹೊರಹೋಗು_2[2 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[3], &ಹೊರಹೋಗು_2[3 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[4], &ಹೊರಹೋಗು_2[4 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[5], &ಹೊರಹೋಗು_2[5 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[6], &ಹೊರಹೋಗು_2[6 * ಗಾತ್ರ(__m256i)]);
  storeu(h_vecs[7], &ಹೊರಹೋಗು_2[7 * ಗಾತ್ರ(__m256i)]);
}

#ಆದರೆ !defined(BLAKE3_NO_SSE41)
ಶೂನ್ಯ blake3_hash_many_sse41(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                            size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_ಆರಂಭ,
                            uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2);
#ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ blake3_hash_many_portable(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                               size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ಆರಂಭ,
                               uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2);
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ blake3_hash_many_avx2(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                           size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ಆರಂಭ,
                           uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  ಆಗ (ಅಂಕೆ_inputs >= DEGREE) {
    blake3_hash8_avx2(inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter, increment_counter, flags,
                      flags_ಆರಂಭ, flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    ಅಂಕೆ_inputs -= DEGREE;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[DEGREE * BLAKE3_ಹೊರಹೋಗು_LEN];
  }
#ಆದರೆ !defined(BLAKE3_NO_SSE41)
  blake3_hash_many_sse41(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                         increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
#ಇಲ್ಲದಿದ್ದರೆ
  blake3_hash_many_portable(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                            increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                            ಹೊರಹೋಗು_2);
#ಆದರೆ_ಅಂತ್ಯ
}
