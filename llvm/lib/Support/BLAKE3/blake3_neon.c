#अन्तर्भाव "blake3_impl.h"

#यदि BLAKE3_USE_NEON

#अन्तर्भाव <arm_neon.h>

#यदि_परिभाषितम् __ARM_BIG_ENDIAN
#दोषः "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#यदि_अन्तः

INLINE uint32x4_t loadu_128(ध्रुव uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  प्रत्यागम vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE शून्य storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t सङ्कलय_128(uint32x4_t a, uint32x4_t b) {
  प्रत्यागम vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  प्रत्यागम veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { प्रत्यागम vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  प्रत्यागम vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  प्रत्यागम vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  प्रत्यागम vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#यदि defined(__clang__)
  प्रत्यागम vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#नोचेद्यदि defined(__GNUC__)
  स्थिर ध्रुव uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  प्रत्यागम vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#अन्यथा 
  प्रत्यागम vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#यदि_अन्तः
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  प्रत्यागम vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE शून्य आवर्तनम्_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = सङ्कलय_128(v[0], m[(size_t)सन्देशः_कालसारणी[r][0]]);
  v[1] = सङ्कलय_128(v[1], m[(size_t)सन्देशः_कालसारणी[r][2]]);
  v[2] = सङ्कलय_128(v[2], m[(size_t)सन्देशः_कालसारणी[r][4]]);
  v[3] = सङ्कलय_128(v[3], m[(size_t)सन्देशः_कालसारणी[r][6]]);
  v[0] = सङ्कलय_128(v[0], v[4]);
  v[1] = सङ्कलय_128(v[1], v[5]);
  v[2] = सङ्कलय_128(v[2], v[6]);
  v[3] = सङ्कलय_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = सङ्कलय_128(v[8], v[12]);
  v[9] = सङ्कलय_128(v[9], v[13]);
  v[10] = सङ्कलय_128(v[10], v[14]);
  v[11] = सङ्कलय_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = सङ्कलय_128(v[0], m[(size_t)सन्देशः_कालसारणी[r][1]]);
  v[1] = सङ्कलय_128(v[1], m[(size_t)सन्देशः_कालसारणी[r][3]]);
  v[2] = सङ्कलय_128(v[2], m[(size_t)सन्देशः_कालसारणी[r][5]]);
  v[3] = सङ्कलय_128(v[3], m[(size_t)सन्देशः_कालसारणी[r][7]]);
  v[0] = सङ्कलय_128(v[0], v[4]);
  v[1] = सङ्कलय_128(v[1], v[5]);
  v[2] = सङ्कलय_128(v[2], v[6]);
  v[3] = सङ्कलय_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = सङ्कलय_128(v[8], v[12]);
  v[9] = सङ्कलय_128(v[9], v[13]);
  v[10] = सङ्कलय_128(v[10], v[14]);
  v[11] = सङ्कलय_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = सङ्कलय_128(v[0], m[(size_t)सन्देशः_कालसारणी[r][8]]);
  v[1] = सङ्कलय_128(v[1], m[(size_t)सन्देशः_कालसारणी[r][10]]);
  v[2] = सङ्कलय_128(v[2], m[(size_t)सन्देशः_कालसारणी[r][12]]);
  v[3] = सङ्कलय_128(v[3], m[(size_t)सन्देशः_कालसारणी[r][14]]);
  v[0] = सङ्कलय_128(v[0], v[5]);
  v[1] = सङ्कलय_128(v[1], v[6]);
  v[2] = सङ्कलय_128(v[2], v[7]);
  v[3] = सङ्कलय_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = सङ्कलय_128(v[10], v[15]);
  v[11] = सङ्कलय_128(v[11], v[12]);
  v[8] = सङ्कलय_128(v[8], v[13]);
  v[9] = सङ्कलय_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = सङ्कलय_128(v[0], m[(size_t)सन्देशः_कालसारणी[r][9]]);
  v[1] = सङ्कलय_128(v[1], m[(size_t)सन्देशः_कालसारणी[r][11]]);
  v[2] = सङ्कलय_128(v[2], m[(size_t)सन्देशः_कालसारणी[r][13]]);
  v[3] = सङ्कलय_128(v[3], m[(size_t)सन्देशः_कालसारणी[r][15]]);
  v[0] = सङ्कलय_128(v[0], v[5]);
  v[1] = सङ्कलय_128(v[1], v[6]);
  v[2] = सङ्कलय_128(v[2], v[7]);
  v[3] = सङ्कलय_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = सङ्कलय_128(v[10], v[15]);
  v[11] = सङ्कलय_128(v[11], v[12]);
  v[8] = सङ्कलय_128(v[8], v[13]);
  v[9] = सङ्कलय_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE शून्य transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_नीचः_u32(rows01.val[0]), vget_नीचः_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_नीचः_u32(rows01.val[1]), vget_नीचः_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_उच्चः_u32(rows01.val[0]), vget_उच्चः_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_उच्चः_u32(rows01.val[1]), vget_उच्चः_u32(rows23.val[1]));
}

INLINE शून्य transpose_सन्देशः_vecs4(ध्रुव uint8_t *ध्रुव *inputs,
                                size_t खण्डम्_विस्थापनम्, uint32x4_t बहिर्गमनम्_2[16]) {
  बहिर्गमनम्_2[0] = loadu_128(&inputs[0][खण्डम्_विस्थापनम् + 0 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[1] = loadu_128(&inputs[1][खण्डम्_विस्थापनम् + 0 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[2] = loadu_128(&inputs[2][खण्डम्_विस्थापनम् + 0 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[3] = loadu_128(&inputs[3][खण्डम्_विस्थापनम् + 0 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[4] = loadu_128(&inputs[0][खण्डम्_विस्थापनम् + 1 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[5] = loadu_128(&inputs[1][खण्डम्_विस्थापनम् + 1 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[6] = loadu_128(&inputs[2][खण्डम्_विस्थापनम् + 1 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[7] = loadu_128(&inputs[3][खण्डम्_विस्थापनम् + 1 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[8] = loadu_128(&inputs[0][खण्डम्_विस्थापनम् + 2 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[9] = loadu_128(&inputs[1][खण्डम्_विस्थापनम् + 2 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[10] = loadu_128(&inputs[2][खण्डम्_विस्थापनम् + 2 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[11] = loadu_128(&inputs[3][खण्डम्_विस्थापनम् + 2 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[12] = loadu_128(&inputs[0][खण्डम्_विस्थापनम् + 3 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[13] = loadu_128(&inputs[1][खण्डम्_विस्थापनम् + 3 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[14] = loadu_128(&inputs[2][खण्डम्_विस्थापनम् + 3 * परिमाण(uint32x4_t)]);
  बहिर्गमनम्_2[15] = loadu_128(&inputs[3][खण्डम्_विस्थापनम् + 3 * परिमाण(uint32x4_t)]);
  transpose_vecs_128(&बहिर्गमनम्_2[0]);
  transpose_vecs_128(&बहिर्गमनम्_2[4]);
  transpose_vecs_128(&बहिर्गमनम्_2[8]);
  transpose_vecs_128(&बहिर्गमनम्_2[12]);
}

INLINE शून्य आरोपय_counters4(uint64_t गणक_2, bool increment_गणक,
                           uint32x4_t *बहिर्गमनम्_नीचः, uint32x4_t *बहिर्गमनम्_उच्चः) {
  uint64_t आवरणम्_2 = (increment_गणक ? ~0 : 0);
  *बहिर्गमनम्_नीचः = set4(
      गणक_नीचः(गणक_2 + (आवरणम्_2 & 0)), गणक_नीचः(गणक_2 + (आवरणम्_2 & 1)),
      गणक_नीचः(गणक_2 + (आवरणम्_2 & 2)), गणक_नीचः(गणक_2 + (आवरणम्_2 & 3)));
  *बहिर्गमनम्_उच्चः = set4(
      गणक_उच्चः(गणक_2 + (आवरणम्_2 & 0)), गणक_उच्चः(गणक_2 + (आवरणम्_2 & 1)),
      गणक_उच्चः(गणक_2 + (आवरणम्_2 & 2)), गणक_उच्चः(गणक_2 + (आवरणम्_2 & 3)));
}

स्थिर शून्य blake3_hash4_neon(ध्रुव uint8_t *ध्रुव *inputs, size_t खण्डानि,
                              ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                              bool increment_गणक, uint8_t flags,
                              uint8_t flags_start, uint8_t flags_अन्तः,
                              uint8_t *बहिर्गमनम्_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(कुञ्चिका_2[0]), set1_128(कुञ्चिका_2[1]), set1_128(कुञ्चिका_2[2]), set1_128(कुञ्चिका_2[3]),
      set1_128(कुञ्चिका_2[4]), set1_128(कुञ्चिका_2[5]), set1_128(कुञ्चिका_2[6]), set1_128(कुञ्चिका_2[7]),
  };
  uint32x4_t गणक_नीचः_vec, गणक_उच्चः_vec;
  आरोपय_counters4(गणक_2, increment_गणक, &गणक_नीचः_vec,
                 &गणक_उच्चः_vec);
  uint8_t खण्डम्_flags = flags | flags_start;

  प्रति (size_t खण्डम् = 0; खण्डम् < खण्डानि; खण्डम्++) {
    यदि (खण्डम् + 1 == खण्डानि) {
      खण्डम्_flags |= flags_अन्तः;
    }
    uint32x4_t खण्डम्_len_vec = set1_128(BLAKE3_खण्डम्_LEN);
    uint32x4_t खण्डम्_flags_vec = set1_128(खण्डम्_flags);
    uint32x4_t सन्देशः_vecs[16];
    transpose_सन्देशः_vecs4(inputs, खण्डम् * BLAKE3_खण्डम्_LEN, सन्देशः_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        गणक_नीचः_vec, गणक_उच्चः_vec, खण्डम्_len_vec,   खण्डम्_flags_vec,
    };
    आवर्तनम्_fn4(v, सन्देशः_vecs, 0);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 1);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 2);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 3);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 4);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 5);
    आवर्तनम्_fn4(v, सन्देशः_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    खण्डम्_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &बहिर्गमनम्_2[0 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[4], &बहिर्गमनम्_2[1 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[1], &बहिर्गमनम्_2[2 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[5], &बहिर्गमनम्_2[3 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[2], &बहिर्गमनम्_2[4 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[6], &बहिर्गमनम्_2[5 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[3], &बहिर्गमनम्_2[6 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[7], &बहिर्गमनम्_2[7 * परिमाण(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

शून्य blake3_compress_अन्तर्_place_portable(uint32_t cv[8],
                                       ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                                       uint8_t खण्डम्_len, uint64_t गणक_2,
                                       uint8_t flags);

INLINE शून्य hash_one_neon(ध्रुव uint8_t *निवेशः_2, size_t खण्डानि,
                          ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                          uint8_t flags, uint8_t flags_start, uint8_t flags_अन्तः,
                          uint8_t बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN]) {
  uint32_t cv[8];
  स्मृतिं_प्रतिलिख(cv, कुञ्चिका_2, BLAKE3_कुञ्चिका_LEN);
  uint8_t खण्डम्_flags = flags | flags_start;
  यावत् (खण्डानि > 0) {
    यदि (खण्डानि == 1) {
      खण्डम्_flags |= flags_अन्तः;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_अन्तर्_place_portable(cv, निवेशः_2, BLAKE3_खण्डम्_LEN, गणक_2,
                                      खण्डम्_flags);
    निवेशः_2 = &निवेशः_2[BLAKE3_खण्डम्_LEN];
    खण्डानि -= 1;
    खण्डम्_flags = flags;
  }
  स्मृतिं_प्रतिलिख(बहिर्गमनम्_2, cv, BLAKE3_बहिर्गमनम्_LEN);
}

शून्य blake3_hash_many_neon(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                           size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                           uint64_t गणक_2, bool increment_गणक,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  यावत् (अङ्कः_inputs >= 4) {
    blake3_hash4_neon(inputs, खण्डानि, कुञ्चिका_2, गणक_2, increment_गणक, flags,
                      flags_start, flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += 4;
    }
    inputs += 4;
    अङ्कः_inputs -= 4;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[4 * BLAKE3_बहिर्गमनम्_LEN];
  }
  यावत् (अङ्कः_inputs > 0) {
    hash_one_neon(inputs[0], खण्डानि, कुञ्चिका_2, गणक_2, flags, flags_start,
                  flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += 1;
    }
    inputs += 1;
    अङ्कः_inputs -= 1;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN];
  }
}

#यदि_अन्तः // BLAKE3_USE_NEON
