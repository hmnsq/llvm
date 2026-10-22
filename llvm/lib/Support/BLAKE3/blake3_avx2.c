#अन्तर्भाव "blake3_impl.h"

#अन्तर्भाव <immintrin.h>

#परिभाषय DEGREE 8

INLINE __m256i loadu(ध्रुव uint8_t src[32]) {
  प्रत्यागम _mm256_loadu_si256((ध्रुव __m256i *)src);
}

INLINE शून्य storeu(__m256i src, uint8_t dest[16]) {
  _mm256_storeu_si256((__m256i *)dest, src);
}

INLINE __m256i addv(__m256i a, __m256i b) { प्रत्यागम mm256_सङ्कलय_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m256i xorv(__m256i a, __m256i b) { प्रत्यागम _mm256_xor_si256(a, b); }

INLINE __m256i set1(uint32_t x) { प्रत्यागम _mm256_set1_epi32((int32_t)x); }

INLINE __m256i rot16(__m256i x) {
  प्रत्यागम _mm256_shuffle_epi8(
      x, mm256_समुच्चयः_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2,
                         13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
}

INLINE __m256i rot12(__m256i x) {
  प्रत्यागम _mm256_or_si256(_mm256_srli_epi32(x, 12), _mm256_slli_epi32(x, 32 - 12));
}

INLINE __m256i rot8(__m256i x) {
  प्रत्यागम _mm256_shuffle_epi8(
      x, mm256_समुच्चयः_epi8(12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1,
                         12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1));
}

INLINE __m256i rot7(__m256i x) {
  प्रत्यागम _mm256_or_si256(_mm256_srli_epi32(x, 7), _mm256_slli_epi32(x, 32 - 7));
}

INLINE शून्य आवर्तनम्_fn(__m256i v[16], __m256i m[16], size_t r) {
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

INLINE शून्य transpose_vecs(__m256i vecs[DEGREE]) {
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

INLINE शून्य transpose_सन्देशः_vecs(ध्रुव uint8_t *ध्रुव *inputs,
                               size_t खण्डम्_विस्थापनम्, __m256i बहिर्गमनम्_2[16]) {
  बहिर्गमनम्_2[0] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[1] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[2] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[3] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[4] = loadu(&inputs[4][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[5] = loadu(&inputs[5][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[6] = loadu(&inputs[6][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[7] = loadu(&inputs[7][खण्डम्_विस्थापनम् + 0 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[8] = loadu(&inputs[0][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[9] = loadu(&inputs[1][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[10] = loadu(&inputs[2][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[11] = loadu(&inputs[3][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[12] = loadu(&inputs[4][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[13] = loadu(&inputs[5][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[14] = loadu(&inputs[6][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  बहिर्गमनम्_2[15] = loadu(&inputs[7][खण्डम्_विस्थापनम् + 1 * परिमाण(__m256i)]);
  प्रति (size_t i = 0; i < 8; ++i) {
    _mm_prefetch((ध्रुव शून्य *)&inputs[i][खण्डम्_विस्थापनम् + 256], _MM_HINT_T0);
  }
  transpose_vecs(&बहिर्गमनम्_2[0]);
  transpose_vecs(&बहिर्गमनम्_2[8]);
}

INLINE शून्य आरोपय_counters(uint64_t गणक_2, bool increment_गणक,
                          __m256i *बहिर्गमनम्_lo, __m256i *बहिर्गमनम्_hi) {
  ध्रुव __m256i आवरणम्_2 = _mm256_set1_epi32(-(int32_t)increment_गणक);
  ध्रुव __m256i add0 = mm256_समुच्चयः_epi32(7, 6, 5, 4, 3, 2, 1, 0);
  ध्रुव __m256i add1 = _mm256_and_si256(आवरणम्_2, add0);
  __m256i l = mm256_सङ्कलय_epi32(_mm256_set1_epi32((int32_t)गणक_2), add1);
  __m256i carry = _mm256_cmpgt_epi32(_mm256_xor_si256(add1, _mm256_set1_epi32(0x80000000)), 
                                     _mm256_xor_si256(   l, _mm256_set1_epi32(0x80000000)));
  __m256i h = _mm256_sub_epi32(_mm256_set1_epi32((int32_t)(गणक_2 >> 32)), carry);
  *बहिर्गमनम्_lo = l;
  *बहिर्गमनम्_hi = h;
}

स्थिर
शून्य blake3_hash8_avx2(ध्रुव uint8_t *ध्रुव *inputs, size_t खण्डानि,
                       ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                       bool increment_गणक, uint8_t flags,
                       uint8_t flags_start, uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  __m256i h_vecs[8] = {
      set1(कुञ्चिका_2[0]), set1(कुञ्चिका_2[1]), set1(कुञ्चिका_2[2]), set1(कुञ्चिका_2[3]),
      set1(कुञ्चिका_2[4]), set1(कुञ्चिका_2[5]), set1(कुञ्चिका_2[6]), set1(कुञ्चिका_2[7]),
  };
  __m256i गणक_नीचः_vec, गणक_उच्चः_vec;
  आरोपय_counters(गणक_2, increment_गणक, &गणक_नीचः_vec,
                &गणक_उच्चः_vec);
  uint8_t खण्डम्_flags = flags | flags_start;

  प्रति (size_t खण्डम् = 0; खण्डम् < खण्डानि; खण्डम्++) {
    यदि (खण्डम् + 1 == खण्डानि) {
      खण्डम्_flags |= flags_अन्तः;
    }
    __m256i खण्डम्_len_vec = set1(BLAKE3_खण्डम्_LEN);
    __m256i खण्डम्_flags_vec = set1(खण्डम्_flags);
    __m256i सन्देशः_vecs[16];
    transpose_सन्देशः_vecs(inputs, खण्डम् * BLAKE3_खण्डम्_LEN, सन्देशः_vecs);

    __m256i v[16] = {
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

  transpose_vecs(h_vecs);
  storeu(h_vecs[0], &बहिर्गमनम्_2[0 * परिमाण(__m256i)]);
  storeu(h_vecs[1], &बहिर्गमनम्_2[1 * परिमाण(__m256i)]);
  storeu(h_vecs[2], &बहिर्गमनम्_2[2 * परिमाण(__m256i)]);
  storeu(h_vecs[3], &बहिर्गमनम्_2[3 * परिमाण(__m256i)]);
  storeu(h_vecs[4], &बहिर्गमनम्_2[4 * परिमाण(__m256i)]);
  storeu(h_vecs[5], &बहिर्गमनम्_2[5 * परिमाण(__m256i)]);
  storeu(h_vecs[6], &बहिर्गमनम्_2[6 * परिमाण(__m256i)]);
  storeu(h_vecs[7], &बहिर्गमनम्_2[7 * परिमाण(__m256i)]);
}

#यदि !defined(BLAKE3_NO_SSE41)
शून्य blake3_hash_many_sse41(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                            size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                            uint64_t गणक_2, bool increment_गणक,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2);
#अन्यथा
शून्य blake3_hash_many_portable(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                               size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                               uint64_t गणक_2, bool increment_गणक,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2);
#यदि_अन्तः

शून्य blake3_hash_many_avx2(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                           size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                           uint64_t गणक_2, bool increment_गणक,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  यावत् (अङ्कः_inputs >= DEGREE) {
    blake3_hash8_avx2(inputs, खण्डानि, कुञ्चिका_2, गणक_2, increment_गणक, flags,
                      flags_start, flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += DEGREE;
    }
    inputs += DEGREE;
    अङ्कः_inputs -= DEGREE;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[DEGREE * BLAKE3_बहिर्गमनम्_LEN];
  }
#यदि !defined(BLAKE3_NO_SSE41)
  blake3_hash_many_sse41(inputs, अङ्कः_inputs, खण्डानि, कुञ्चिका_2, गणक_2,
                         increment_गणक, flags, flags_start, flags_अन्तः, बहिर्गमनम्_2);
#अन्यथा
  blake3_hash_many_portable(inputs, अङ्कः_inputs, खण्डानि, कुञ्चिका_2, गणक_2,
                            increment_गणक, flags, flags_start, flags_अन्तः,
                            बहिर्गमनम्_2);
#यदि_अन्तः
}
