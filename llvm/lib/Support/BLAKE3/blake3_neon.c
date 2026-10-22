#అన్తర్భావ "blake3_impl.h"

#ఒకవేళ BLAKE3_USE_NEON

#అన్తర్భావ <arm_neon.h>

#నిర్వచితమైతే __ARM_BIG_ENDIAN
#దోషం "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#అయితే_ముగింపు

INLINE uint32x4_t loadu_128(స్థిరరాశి uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  తిరిగి vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE శూన్య storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t కూడు_128(uint32x4_t a, uint32x4_t b) {
  తిరిగి vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  తిరిగి veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { తిరిగి vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  తిరిగి vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  తిరిగి vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  తిరిగి vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#ఒకవేళ defined(__clang__)
  తిరిగి vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#లేదావేళ defined(__GNUC__)
  స్థిర స్థిరరాశి uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  తిరిగి vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#లేకపోతే 
  తిరిగి vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#అయితే_ముగింపు
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  తిరిగి vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE శూన్య విడత_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = కూడు_128(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][0]]);
  v[1] = కూడు_128(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][2]]);
  v[2] = కూడు_128(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][4]]);
  v[3] = కూడు_128(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][6]]);
  v[0] = కూడు_128(v[0], v[4]);
  v[1] = కూడు_128(v[1], v[5]);
  v[2] = కూడు_128(v[2], v[6]);
  v[3] = కూడు_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = కూడు_128(v[8], v[12]);
  v[9] = కూడు_128(v[9], v[13]);
  v[10] = కూడు_128(v[10], v[14]);
  v[11] = కూడు_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = కూడు_128(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][1]]);
  v[1] = కూడు_128(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][3]]);
  v[2] = కూడు_128(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][5]]);
  v[3] = కూడు_128(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][7]]);
  v[0] = కూడు_128(v[0], v[4]);
  v[1] = కూడు_128(v[1], v[5]);
  v[2] = కూడు_128(v[2], v[6]);
  v[3] = కూడు_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = కూడు_128(v[8], v[12]);
  v[9] = కూడు_128(v[9], v[13]);
  v[10] = కూడు_128(v[10], v[14]);
  v[11] = కూడు_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = కూడు_128(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][8]]);
  v[1] = కూడు_128(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][10]]);
  v[2] = కూడు_128(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][12]]);
  v[3] = కూడు_128(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][14]]);
  v[0] = కూడు_128(v[0], v[5]);
  v[1] = కూడు_128(v[1], v[6]);
  v[2] = కూడు_128(v[2], v[7]);
  v[3] = కూడు_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = కూడు_128(v[10], v[15]);
  v[11] = కూడు_128(v[11], v[12]);
  v[8] = కూడు_128(v[8], v[13]);
  v[9] = కూడు_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = కూడు_128(v[0], m[(size_t)సందేశం_కాలపట్టిక[r][9]]);
  v[1] = కూడు_128(v[1], m[(size_t)సందేశం_కాలపట్టిక[r][11]]);
  v[2] = కూడు_128(v[2], m[(size_t)సందేశం_కాలపట్టిక[r][13]]);
  v[3] = కూడు_128(v[3], m[(size_t)సందేశం_కాలపట్టిక[r][15]]);
  v[0] = కూడు_128(v[0], v[5]);
  v[1] = కూడు_128(v[1], v[6]);
  v[2] = కూడు_128(v[2], v[7]);
  v[3] = కూడు_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = కూడు_128(v[10], v[15]);
  v[11] = కూడు_128(v[11], v[12]);
  v[8] = కూడు_128(v[8], v[13]);
  v[9] = కూడు_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE శూన్య transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_దిగువ_u32(rows01.val[0]), vget_దిగువ_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_దిగువ_u32(rows01.val[1]), vget_దిగువ_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_ఎగువ_u32(rows01.val[0]), vget_ఎగువ_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_ఎగువ_u32(rows01.val[1]), vget_ఎగువ_u32(rows23.val[1]));
}

INLINE శూన్య transpose_సందేశం_vecs4(స్థిరరాశి uint8_t *స్థిరరాశి *inputs,
                                size_t ఖండం_స్థానభ్రంశం, uint32x4_t బహిర్గతం_2[16]) {
  బహిర్గతం_2[0] = loadu_128(&inputs[0][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[1] = loadu_128(&inputs[1][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[2] = loadu_128(&inputs[2][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[3] = loadu_128(&inputs[3][ఖండం_స్థానభ్రంశం + 0 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[4] = loadu_128(&inputs[0][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[5] = loadu_128(&inputs[1][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[6] = loadu_128(&inputs[2][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[7] = loadu_128(&inputs[3][ఖండం_స్థానభ్రంశం + 1 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[8] = loadu_128(&inputs[0][ఖండం_స్థానభ్రంశం + 2 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[9] = loadu_128(&inputs[1][ఖండం_స్థానభ్రంశం + 2 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[10] = loadu_128(&inputs[2][ఖండం_స్థానభ్రంశం + 2 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[11] = loadu_128(&inputs[3][ఖండం_స్థానభ్రంశం + 2 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[12] = loadu_128(&inputs[0][ఖండం_స్థానభ్రంశం + 3 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[13] = loadu_128(&inputs[1][ఖండం_స్థానభ్రంశం + 3 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[14] = loadu_128(&inputs[2][ఖండం_స్థానభ్రంశం + 3 * పరిమాణం(uint32x4_t)]);
  బహిర్గతం_2[15] = loadu_128(&inputs[3][ఖండం_స్థానభ్రంశం + 3 * పరిమాణం(uint32x4_t)]);
  transpose_vecs_128(&బహిర్గతం_2[0]);
  transpose_vecs_128(&బహిర్గతం_2[4]);
  transpose_vecs_128(&బహిర్గతం_2[8]);
  transpose_vecs_128(&బహిర్గతం_2[12]);
}

INLINE శూన్య ఎక్కించు_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *బహిర్గతం_దిగువ, uint32x4_t *బహిర్గతం_ఎగువ) {
  uint64_t ముసుగు_2 = (increment_counter ? ~0 : 0);
  *బహిర్గతం_దిగువ = set4(
      counter_దిగువ(counter + (ముసుగు_2 & 0)), counter_దిగువ(counter + (ముసుగు_2 & 1)),
      counter_దిగువ(counter + (ముసుగు_2 & 2)), counter_దిగువ(counter + (ముసుగు_2 & 3)));
  *బహిర్గతం_ఎగువ = set4(
      counter_ఎగువ(counter + (ముసుగు_2 & 0)), counter_ఎగువ(counter + (ముసుగు_2 & 1)),
      counter_ఎగువ(counter + (ముసుగు_2 & 2)), counter_ఎగువ(counter + (ముసుగు_2 & 3)));
}

స్థిర శూన్య blake3_hash4_neon(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t ఖండాలు,
                              స్థిరరాశి uint32_t కీలకం_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_ప్రారంభం, uint8_t flags_చివర,
                              uint8_t *బహిర్గతం_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(కీలకం_2[0]), set1_128(కీలకం_2[1]), set1_128(కీలకం_2[2]), set1_128(కీలకం_2[3]),
      set1_128(కీలకం_2[4]), set1_128(కీలకం_2[5]), set1_128(కీలకం_2[6]), set1_128(కీలకం_2[7]),
  };
  uint32x4_t counter_దిగువ_vec, counter_ఎగువ_vec;
  ఎక్కించు_counters4(counter, increment_counter, &counter_దిగువ_vec,
                 &counter_ఎగువ_vec);
  uint8_t ఖండం_flags = flags | flags_ప్రారంభం;

  కొరకు (size_t ఖండం = 0; ఖండం < ఖండాలు; ఖండం++) {
    ఒకవేళ (ఖండం + 1 == ఖండాలు) {
      ఖండం_flags |= flags_చివర;
    }
    uint32x4_t ఖండం_len_vec = set1_128(BLAKE3_ఖండం_LEN);
    uint32x4_t ఖండం_flags_vec = set1_128(ఖండం_flags);
    uint32x4_t సందేశం_vecs[16];
    transpose_సందేశం_vecs4(inputs, ఖండం * BLAKE3_ఖండం_LEN, సందేశం_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_దిగువ_vec, counter_ఎగువ_vec, ఖండం_len_vec,   ఖండం_flags_vec,
    };
    విడత_fn4(v, సందేశం_vecs, 0);
    విడత_fn4(v, సందేశం_vecs, 1);
    విడత_fn4(v, సందేశం_vecs, 2);
    విడత_fn4(v, సందేశం_vecs, 3);
    విడత_fn4(v, సందేశం_vecs, 4);
    విడత_fn4(v, సందేశం_vecs, 5);
    విడత_fn4(v, సందేశం_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    ఖండం_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &బహిర్గతం_2[0 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[4], &బహిర్గతం_2[1 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[1], &బహిర్గతం_2[2 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[5], &బహిర్గతం_2[3 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[2], &బహిర్గతం_2[4 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[6], &బహిర్గతం_2[5 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[3], &బహిర్గతం_2[6 * పరిమాణం(uint32x4_t)]);
  storeu_128(h_vecs[7], &బహిర్గతం_2[7 * పరిమాణం(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

శూన్య blake3_compress_లోపల_ఉంచు_portable(uint32_t cv[8],
                                       స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                                       uint8_t ఖండం_len, uint64_t counter,
                                       uint8_t flags);

INLINE శూన్య hash_one_neon(స్థిరరాశి uint8_t *ఆదానం_2, size_t ఖండాలు,
                          స్థిరరాశి uint32_t కీలకం_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_ప్రారంభం, uint8_t flags_చివర,
                          uint8_t బహిర్గతం_2[BLAKE3_బహిర్గతం_LEN]) {
  uint32_t cv[8];
  జ్ఞాపకం_నకలు(cv, కీలకం_2, BLAKE3_కీలకం_LEN);
  uint8_t ఖండం_flags = flags | flags_ప్రారంభం;
  అయితే (ఖండాలు > 0) {
    ఒకవేళ (ఖండాలు == 1) {
      ఖండం_flags |= flags_చివర;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_లోపల_ఉంచు_portable(cv, ఆదానం_2, BLAKE3_ఖండం_LEN, counter,
                                      ఖండం_flags);
    ఆదానం_2 = &ఆదానం_2[BLAKE3_ఖండం_LEN];
    ఖండాలు -= 1;
    ఖండం_flags = flags;
  }
  జ్ఞాపకం_నకలు(బహిర్గతం_2, cv, BLAKE3_బహిర్గతం_LEN);
}

శూన్య blake3_hash_many_neon(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                           size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ప్రారంభం,
                           uint8_t flags_చివర, uint8_t *బహిర్గతం_2) {
  అయితే (అంకె_inputs >= 4) {
    blake3_hash4_neon(inputs, ఖండాలు, కీలకం_2, counter, increment_counter, flags,
                      flags_ప్రారంభం, flags_చివర, బహిర్గతం_2);
    ఒకవేళ (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    అంకె_inputs -= 4;
    బహిర్గతం_2 = &బహిర్గతం_2[4 * BLAKE3_బహిర్గతం_LEN];
  }
  అయితే (అంకె_inputs > 0) {
    hash_one_neon(inputs[0], ఖండాలు, కీలకం_2, counter, flags, flags_ప్రారంభం,
                  flags_చివర, బహిర్గతం_2);
    ఒకవేళ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    అంకె_inputs -= 1;
    బహిర్గతం_2 = &బహిర్గతం_2[BLAKE3_బహిర్గతం_LEN];
  }
}

#అయితే_ముగింపు // BLAKE3_USE_NEON
