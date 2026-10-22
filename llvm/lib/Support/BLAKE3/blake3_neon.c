#अन्तर्भाव "blake3_impl.h"

#यदि BLAKE3_USE_NEON

#अन्तर्भाव <arm_neon.h>

#ifdef __ARM_BIG_ENDIAN
#error "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#endif

INLINE uint32x4_t loadu_128(ध्रुव uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  प्रत्यागम vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE शून्य storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t add_128(uint32x4_t a, uint32x4_t b) {
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
#endif
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

INLINE शून्य round_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = add_128(v[0], m[(size_t)संदेश_SCHEDULE[r][0]]);
  v[1] = add_128(v[1], m[(size_t)संदेश_SCHEDULE[r][2]]);
  v[2] = add_128(v[2], m[(size_t)संदेश_SCHEDULE[r][4]]);
  v[3] = add_128(v[3], m[(size_t)संदेश_SCHEDULE[r][6]]);
  v[0] = add_128(v[0], v[4]);
  v[1] = add_128(v[1], v[5]);
  v[2] = add_128(v[2], v[6]);
  v[3] = add_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = add_128(v[8], v[12]);
  v[9] = add_128(v[9], v[13]);
  v[10] = add_128(v[10], v[14]);
  v[11] = add_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = add_128(v[0], m[(size_t)संदेश_SCHEDULE[r][1]]);
  v[1] = add_128(v[1], m[(size_t)संदेश_SCHEDULE[r][3]]);
  v[2] = add_128(v[2], m[(size_t)संदेश_SCHEDULE[r][5]]);
  v[3] = add_128(v[3], m[(size_t)संदेश_SCHEDULE[r][7]]);
  v[0] = add_128(v[0], v[4]);
  v[1] = add_128(v[1], v[5]);
  v[2] = add_128(v[2], v[6]);
  v[3] = add_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = add_128(v[8], v[12]);
  v[9] = add_128(v[9], v[13]);
  v[10] = add_128(v[10], v[14]);
  v[11] = add_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = add_128(v[0], m[(size_t)संदेश_SCHEDULE[r][8]]);
  v[1] = add_128(v[1], m[(size_t)संदेश_SCHEDULE[r][10]]);
  v[2] = add_128(v[2], m[(size_t)संदेश_SCHEDULE[r][12]]);
  v[3] = add_128(v[3], m[(size_t)संदेश_SCHEDULE[r][14]]);
  v[0] = add_128(v[0], v[5]);
  v[1] = add_128(v[1], v[6]);
  v[2] = add_128(v[2], v[7]);
  v[3] = add_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = add_128(v[10], v[15]);
  v[11] = add_128(v[11], v[12]);
  v[8] = add_128(v[8], v[13]);
  v[9] = add_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = add_128(v[0], m[(size_t)संदेश_SCHEDULE[r][9]]);
  v[1] = add_128(v[1], m[(size_t)संदेश_SCHEDULE[r][11]]);
  v[2] = add_128(v[2], m[(size_t)संदेश_SCHEDULE[r][13]]);
  v[3] = add_128(v[3], m[(size_t)संदेश_SCHEDULE[r][15]]);
  v[0] = add_128(v[0], v[5]);
  v[1] = add_128(v[1], v[6]);
  v[2] = add_128(v[2], v[7]);
  v[3] = add_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = add_128(v[10], v[15]);
  v[11] = add_128(v[11], v[12]);
  v[8] = add_128(v[8], v[13]);
  v[9] = add_128(v[9], v[14]);
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
      vcombine_u32(vget_निचला_u32(rows01.val[0]), vget_निचला_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_निचला_u32(rows01.val[1]), vget_निचला_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_ऊपरी_u32(rows01.val[0]), vget_ऊपरी_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_ऊपरी_u32(rows01.val[1]), vget_ऊपरी_u32(rows23.val[1]));
}

INLINE शून्य transpose_संदेश_vecs4(ध्रुव uint8_t *ध्रुव *inputs,
                                size_t block_offset, uint32x4_t out[16]) {
  out[0] = loadu_128(&inputs[0][block_offset + 0 * परिमाण(uint32x4_t)]);
  out[1] = loadu_128(&inputs[1][block_offset + 0 * परिमाण(uint32x4_t)]);
  out[2] = loadu_128(&inputs[2][block_offset + 0 * परिमाण(uint32x4_t)]);
  out[3] = loadu_128(&inputs[3][block_offset + 0 * परिमाण(uint32x4_t)]);
  out[4] = loadu_128(&inputs[0][block_offset + 1 * परिमाण(uint32x4_t)]);
  out[5] = loadu_128(&inputs[1][block_offset + 1 * परिमाण(uint32x4_t)]);
  out[6] = loadu_128(&inputs[2][block_offset + 1 * परिमाण(uint32x4_t)]);
  out[7] = loadu_128(&inputs[3][block_offset + 1 * परिमाण(uint32x4_t)]);
  out[8] = loadu_128(&inputs[0][block_offset + 2 * परिमाण(uint32x4_t)]);
  out[9] = loadu_128(&inputs[1][block_offset + 2 * परिमाण(uint32x4_t)]);
  out[10] = loadu_128(&inputs[2][block_offset + 2 * परिमाण(uint32x4_t)]);
  out[11] = loadu_128(&inputs[3][block_offset + 2 * परिमाण(uint32x4_t)]);
  out[12] = loadu_128(&inputs[0][block_offset + 3 * परिमाण(uint32x4_t)]);
  out[13] = loadu_128(&inputs[1][block_offset + 3 * परिमाण(uint32x4_t)]);
  out[14] = loadu_128(&inputs[2][block_offset + 3 * परिमाण(uint32x4_t)]);
  out[15] = loadu_128(&inputs[3][block_offset + 3 * परिमाण(uint32x4_t)]);
  transpose_vecs_128(&out[0]);
  transpose_vecs_128(&out[4]);
  transpose_vecs_128(&out[8]);
  transpose_vecs_128(&out[12]);
}

INLINE शून्य load_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *out_निचला, uint32x4_t *out_ऊपरी) {
  uint64_t mask = (increment_counter ? ~0 : 0);
  *out_निचला = set4(
      counter_निचला(counter + (mask & 0)), counter_निचला(counter + (mask & 1)),
      counter_निचला(counter + (mask & 2)), counter_निचला(counter + (mask & 3)));
  *out_ऊपरी = set4(
      counter_ऊपरी(counter + (mask & 0)), counter_ऊपरी(counter + (mask & 1)),
      counter_ऊपरी(counter + (mask & 2)), counter_ऊपरी(counter + (mask & 3)));
}

स्थिर शून्य blake3_hash4_neon(ध्रुव uint8_t *ध्रुव *inputs, size_t blocks,
                              ध्रुव uint32_t कुंजी_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_start, uint8_t flags_end,
                              uint8_t *out) {
  uint32x4_t h_vecs[8] = {
      set1_128(कुंजी_2[0]), set1_128(कुंजी_2[1]), set1_128(कुंजी_2[2]), set1_128(कुंजी_2[3]),
      set1_128(कुंजी_2[4]), set1_128(कुंजी_2[5]), set1_128(कुंजी_2[6]), set1_128(कुंजी_2[7]),
  };
  uint32x4_t counter_निचला_vec, counter_ऊपरी_vec;
  load_counters4(counter, increment_counter, &counter_निचला_vec,
                 &counter_ऊपरी_vec);
  uint8_t block_flags = flags | flags_start;

  प्रति (size_t block = 0; block < blocks; block++) {
    यदि (block + 1 == blocks) {
      block_flags |= flags_end;
    }
    uint32x4_t block_len_vec = set1_128(BLAKE3_BLOCK_LEN);
    uint32x4_t block_flags_vec = set1_128(block_flags);
    uint32x4_t संदेश_vecs[16];
    transpose_संदेश_vecs4(inputs, block * BLAKE3_BLOCK_LEN, संदेश_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_निचला_vec, counter_ऊपरी_vec, block_len_vec,   block_flags_vec,
    };
    round_fn4(v, संदेश_vecs, 0);
    round_fn4(v, संदेश_vecs, 1);
    round_fn4(v, संदेश_vecs, 2);
    round_fn4(v, संदेश_vecs, 3);
    round_fn4(v, संदेश_vecs, 4);
    round_fn4(v, संदेश_vecs, 5);
    round_fn4(v, संदेश_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    block_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &out[0 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[4], &out[1 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[1], &out[2 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[5], &out[3 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[2], &out[4 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[6], &out[5 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[3], &out[6 * परिमाण(uint32x4_t)]);
  storeu_128(h_vecs[7], &out[7 * परिमाण(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

शून्य blake3_compress_in_place_portable(uint32_t cv[8],
                                       ध्रुव uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

INLINE शून्य hash_one_neon(ध्रुव uint8_t *निवेश_2, size_t blocks,
                          ध्रुव uint32_t कुंजी_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_start, uint8_t flags_end,
                          uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, कुंजी_2, BLAKE3_कुंजी_LEN);
  uint8_t block_flags = flags | flags_start;
  यावत् (blocks > 0) {
    यदि (blocks == 1) {
      block_flags |= flags_end;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_in_place_portable(cv, निवेश_2, BLAKE3_BLOCK_LEN, counter,
                                      block_flags);
    निवेश_2 = &निवेश_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  memcpy(out, cv, BLAKE3_OUT_LEN);
}

शून्य blake3_hash_many_neon(ध्रुव uint8_t *ध्रुव *inputs, size_t अंक_inputs,
                           size_t blocks, ध्रुव uint32_t कुंजी_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out) {
  यावत् (अंक_inputs >= 4) {
    blake3_hash4_neon(inputs, blocks, कुंजी_2, counter, increment_counter, flags,
                      flags_start, flags_end, out);
    यदि (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    अंक_inputs -= 4;
    out = &out[4 * BLAKE3_OUT_LEN];
  }
  यावत् (अंक_inputs > 0) {
    hash_one_neon(inputs[0], blocks, कुंजी_2, counter, flags, flags_start,
                  flags_end, out);
    यदि (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    अंक_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}

#endif // BLAKE3_USE_NEON
