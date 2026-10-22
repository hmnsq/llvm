#అన్తర్భావ "blake3_impl.h"

#అన్తర్భావ <immintrin.h>

#నిర్వచించు DEGREE 8

INLINE __m256i loadu(స్థిరరాశి uint8_t src[32]) {
  తిరిగి _mm256_loadu_si256((స్థిరరాశి __m256i *)src);
}

INLINE శూన్య storeu(__m256i src, uint8_t dest[16]) {
  _mm256_storeu_si256((__m256i *)dest, src);
}

INLINE __m256i addv(__m256i a, __m256i b) { తిరిగి mm256_కూడు_epi32(a, b); }

// Note that clang-format doesn't like the name "xor" for some reason.
INLINE __m256i xorv(__m256i a, __m256i b) { తిరిగి _mm256_xor_si256(a, b); }

INLINE __m256i set1(uint32_t x) { తిరిగి _mm256_set1_epi32((int32_t)x); }

INLINE __m256i rot16(__m256i x) {
  తిరిగి _mm256_shuffle_epi8(
      x, mm256_సమితి_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2,
                         13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
}

INLINE __m256i rot12(__m256i x) {
  తిరిగి _mm256_or_si256(_mm256_srli_epi32(x, 12), _mm256_slli_epi32(x, 32 - 12));
}

INLINE __m256i rot8(__m256i x) {
  తిరిగి _mm256_shuffle_epi8(
      x, mm256_సమితి_epi8(12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1,
                         12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1));
}

INLINE __m256i rot7(__m256i x) {
  తిరిగి _mm256_or_si256(_mm256_srli_epi32(x, 7), _mm256_slli_epi32(x, 32 - 7));
}

INLINE శూన్య విడత_fn(__m256i v[16], __m256i m[16], size_t r) {
  v[0] = addv(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][0]]);
  v[1] = addv(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][2]]);
  v[2] = addv(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][4]]);
  v[3] = addv(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][6]]);
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
  v[0] = addv(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][1]]);
  v[1] = addv(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][3]]);
  v[2] = addv(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][5]]);
  v[3] = addv(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][7]]);
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

  v[0] = addv(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][8]]);
  v[1] = addv(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][10]]);
  v[2] = addv(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][12]]);
  v[3] = addv(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][14]]);
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
  v[0] = addv(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][9]]);
  v[1] = addv(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][11]]);
  v[2] = addv(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][13]]);
  v[3] = addv(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][15]]);
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

INLINE శూన్య transpose_vecs(__m256i vecs[DEGREE]) {
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

INLINE శూన్య transpose_సందేశం_vecs(స్థిరరాశి uint8_t *స్థిరరాశి *inputs,
                               size_t ఖండం_స్థానభ్రంశం, __m256i బహిర్గతం_2[16]) {
  బహిర్గతం_2[0] = loadu(&inputs[0][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[1] = loadu(&inputs[1][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[2] = loadu(&inputs[2][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[3] = loadu(&inputs[3][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[4] = loadu(&inputs[4][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[5] = loadu(&inputs[5][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[6] = loadu(&inputs[6][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[7] = loadu(&inputs[7][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[8] = loadu(&inputs[0][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[9] = loadu(&inputs[1][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[10] = loadu(&inputs[2][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[11] = loadu(&inputs[3][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[12] = loadu(&inputs[4][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[13] = loadu(&inputs[5][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[14] = loadu(&inputs[6][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  బహిర్గతం_2[15] = loadu(&inputs[7][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(__m256i)]);
  కొరకు (size_t i = 0; i < 8; ++i) {
    _mm_prefetch((స్థిరరాశి శూన్య *)&inputs[i][ఖండం_స్థానభ్రంశం + 256], _MM_HINT_T0);
  }
  transpose_vecs(&బహిర్గతం_2[0]);
  transpose_vecs(&బహిర్గతం_2[8]);
}

INLINE శూన్య ఎక్కించు_counters(uint64_t counter, bool increment_counter,
                          __m256i *బహిర్గతం_lo, __m256i *బహిర్గతం_hi) {
  స్థిరరాశి __m256i ముసుగు_2 = _mm256_set1_epi32(-(int32_t)increment_counter);
  స్థిరరాశి __m256i add0 = mm256_సమితి_epi32(7, 6, 5, 4, 3, 2, 1, 0);
  స్థిరరాశి __m256i add1 = _mm256_and_si256(ముసుగు_2, add0);
  __m256i l = mm256_కూడు_epi32(_mm256_set1_epi32((int32_t)counter), add1);
  __m256i వాహకం = _mm256_cmpgt_epi32(_mm256_xor_si256(add1, _mm256_set1_epi32(0x80000000)), 
                                     _mm256_xor_si256(   l, _mm256_set1_epi32(0x80000000)));
  __m256i h = _mm256_sub_epi32(_mm256_set1_epi32((int32_t)(counter >> 32)), వాహకం);
  *బహిర్గతం_lo = l;
  *బహిర్గతం_hi = h;
}

స్థిర
శూన్య blake3_hash8_avx2(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t ఖండాలు,
                       స్థిరరాశి uint32_t కీలకం_2[8], uint64_t counter,
                       bool increment_counter, uint8_t flags,
                       uint8_t flags_ప్రారంభం, uint8_t flags_చివర, uint8_t *బహిర్గతం_2) {
  __m256i h_vecs[8] = {
      set1(కీలకం_2[0]), set1(కీలకం_2[1]), set1(కీలకం_2[2]), set1(కీలకం_2[3]),
      set1(కీలకం_2[4]), set1(కీలకం_2[5]), set1(కీలకం_2[6]), set1(కీలకం_2[7]),
  };
  __m256i counter_దిగువ_vec, counter_ఎగువ_vec;
  ఎక్కించు_counters(counter, increment_counter, &counter_దిగువ_vec,
                &counter_ఎగువ_vec);
  uint8_t ఖండం_flags = flags | flags_ప్రారంభం;

  కొరకు (size_t ఖండం = 0; ఖండం < ఖండాలు; ఖండం++) {
    ఒకవేళ (ఖండం + 1 == ఖండాలు) {
      ఖండం_flags |= flags_చివర;
    }
    __m256i ఖండం_len_vec = set1(BLAKE3_ఖండం_LEN);
    __m256i ఖండం_flags_vec = set1(ఖండం_flags);
    __m256i సందేశం_vecs[16];
    transpose_సందేశం_vecs(inputs, ఖండం * BLAKE3_ఖండం_LEN, సందేశం_vecs);

    __m256i v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],     h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],     h_vecs[7],
        set1(IV[0]),     set1(IV[1]),      set1(IV[2]),   set1(IV[3]),
        counter_దిగువ_vec, counter_ఎగువ_vec, ఖండం_len_vec, ఖండం_flags_vec,
    };
    విడత_fn(v, సందేశం_vecs, 0);
    విడత_fn(v, సందేశం_vecs, 1);
    విడత_fn(v, సందేశం_vecs, 2);
    విడత_fn(v, సందేశం_vecs, 3);
    విడత_fn(v, సందేశం_vecs, 4);
    విడత_fn(v, సందేశం_vecs, 5);
    విడత_fn(v, సందేశం_vecs, 6);
    h_vecs[0] = xorv(v[0], v[8]);
    h_vecs[1] = xorv(v[1], v[9]);
    h_vecs[2] = xorv(v[2], v[10]);
    h_vecs[3] = xorv(v[3], v[11]);
    h_vecs[4] = xorv(v[4], v[12]);
    h_vecs[5] = xorv(v[5], v[13]);
    h_vecs[6] = xorv(v[6], v[14]);
    h_vecs[7] = xorv(v[7], v[15]);

    ఖండం_flags = flags;
  }

  transpose_vecs(h_vecs);
  storeu(h_vecs[0], &బహిర్గతం_2[0 * పరిమాణం(__m256i)]);
  storeu(h_vecs[1], &బహిర్గతం_2[1 * పరిమాణం(__m256i)]);
  storeu(h_vecs[2], &బహిర్గతం_2[2 * పరిమాణం(__m256i)]);
  storeu(h_vecs[3], &బహిర్గతం_2[3 * పరిమాణం(__m256i)]);
  storeu(h_vecs[4], &బహిర్గతం_2[4 * పరిమాణం(__m256i)]);
  storeu(h_vecs[5], &బహిర్గతం_2[5 * పరిమాణం(__m256i)]);
  storeu(h_vecs[6], &బహిర్గతం_2[6 * పరిమాణం(__m256i)]);
  storeu(h_vecs[7], &బహిర్గతం_2[7 * పరిమాణం(__m256i)]);
}

#ఒకవేళ !defined(BLAKE3_NO_SSE41)
శూన్య blake3_hash_many_sse41(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                            size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_ప్రారంభం,
                            uint8_t flags_చివర, uint8_t *బహిర్గతం_2);
#లేకపోతే
శూన్య blake3_hash_many_portable(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                               size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ప్రారంభం,
                               uint8_t flags_చివర, uint8_t *బహిర్గతం_2);
#అయితే_ముగింపు

శూన్య blake3_hash_many_avx2(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                           size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ప్రారంభం,
                           uint8_t flags_చివర, uint8_t *బహిర్గతం_2) {
  అయితే (అంకె_inputs >= DEGREE) {
    blake3_hash8_avx2(inputs, ఖండాలు, కీలకం_2, counter, increment_counter, flags,
                      flags_ప్రారంభం, flags_చివర, బహిర్గతం_2);
    ఒకవేళ (increment_counter) {
      counter += DEGREE;
    }
    inputs += DEGREE;
    అంకె_inputs -= DEGREE;
    బహిర్గతం_2 = &బహిర్గతం_2[DEGREE * BLAKE3_బహిర్గతం_LEN];
  }
#ఒకవేళ !defined(BLAKE3_NO_SSE41)
  blake3_hash_many_sse41(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                         increment_counter, flags, flags_ప్రారంభం, flags_చివర, బహిర్గతం_2);
#లేకపోతే
  blake3_hash_many_portable(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                            increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                            బహిర్గతం_2);
#అయితే_ముగింపు
}
