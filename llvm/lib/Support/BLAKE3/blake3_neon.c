#ضمن "blake3_impl.h"

#إذا BLAKE3_USE_NEON

#ضمن <arm_neon.h>

#جيڪڏهن_وصف_ٿيل __ARM_BIG_ENDIAN
#غلطي "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#جيڪڏهن_پورو

INLINE uint32x4_t loadu_128(ثابت uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  أرجع vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE فراغ storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t جوڙيو_128(uint32x4_t a, uint32x4_t b) {
  أرجع vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  أرجع veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { أرجع vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  أرجع vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  أرجع vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  أرجع vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#إذا defined(__clang__)
  أرجع vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#وإلاإذا defined(__GNUC__)
  ساكن ثابت uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  أرجع vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#وإلا 
  أرجع vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#جيڪڏهن_پورو
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  أرجع vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE فراغ ڦيرو_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = جوڙيو_128(v[0], m[(size_t)نياپو_وقت_جدول[r][0]]);
  v[1] = جوڙيو_128(v[1], m[(size_t)نياپو_وقت_جدول[r][2]]);
  v[2] = جوڙيو_128(v[2], m[(size_t)نياپو_وقت_جدول[r][4]]);
  v[3] = جوڙيو_128(v[3], m[(size_t)نياپو_وقت_جدول[r][6]]);
  v[0] = جوڙيو_128(v[0], v[4]);
  v[1] = جوڙيو_128(v[1], v[5]);
  v[2] = جوڙيو_128(v[2], v[6]);
  v[3] = جوڙيو_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = جوڙيو_128(v[8], v[12]);
  v[9] = جوڙيو_128(v[9], v[13]);
  v[10] = جوڙيو_128(v[10], v[14]);
  v[11] = جوڙيو_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = جوڙيو_128(v[0], m[(size_t)نياپو_وقت_جدول[r][1]]);
  v[1] = جوڙيو_128(v[1], m[(size_t)نياپو_وقت_جدول[r][3]]);
  v[2] = جوڙيو_128(v[2], m[(size_t)نياپو_وقت_جدول[r][5]]);
  v[3] = جوڙيو_128(v[3], m[(size_t)نياپو_وقت_جدول[r][7]]);
  v[0] = جوڙيو_128(v[0], v[4]);
  v[1] = جوڙيو_128(v[1], v[5]);
  v[2] = جوڙيو_128(v[2], v[6]);
  v[3] = جوڙيو_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = جوڙيو_128(v[8], v[12]);
  v[9] = جوڙيو_128(v[9], v[13]);
  v[10] = جوڙيو_128(v[10], v[14]);
  v[11] = جوڙيو_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = جوڙيو_128(v[0], m[(size_t)نياپو_وقت_جدول[r][8]]);
  v[1] = جوڙيو_128(v[1], m[(size_t)نياپو_وقت_جدول[r][10]]);
  v[2] = جوڙيو_128(v[2], m[(size_t)نياپو_وقت_جدول[r][12]]);
  v[3] = جوڙيو_128(v[3], m[(size_t)نياپو_وقت_جدول[r][14]]);
  v[0] = جوڙيو_128(v[0], v[5]);
  v[1] = جوڙيو_128(v[1], v[6]);
  v[2] = جوڙيو_128(v[2], v[7]);
  v[3] = جوڙيو_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = جوڙيو_128(v[10], v[15]);
  v[11] = جوڙيو_128(v[11], v[12]);
  v[8] = جوڙيو_128(v[8], v[13]);
  v[9] = جوڙيو_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = جوڙيو_128(v[0], m[(size_t)نياپو_وقت_جدول[r][9]]);
  v[1] = جوڙيو_128(v[1], m[(size_t)نياپو_وقت_جدول[r][11]]);
  v[2] = جوڙيو_128(v[2], m[(size_t)نياپو_وقت_جدول[r][13]]);
  v[3] = جوڙيو_128(v[3], m[(size_t)نياپو_وقت_جدول[r][15]]);
  v[0] = جوڙيو_128(v[0], v[5]);
  v[1] = جوڙيو_128(v[1], v[6]);
  v[2] = جوڙيو_128(v[2], v[7]);
  v[3] = جوڙيو_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = جوڙيو_128(v[10], v[15]);
  v[11] = جوڙيو_128(v[11], v[12]);
  v[8] = جوڙيو_128(v[8], v[13]);
  v[9] = جوڙيو_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE فراغ transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_هيٺيون_u32(rows01.val[0]), vget_هيٺيون_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_هيٺيون_u32(rows01.val[1]), vget_هيٺيون_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_مٿاهون_u32(rows01.val[0]), vget_مٿاهون_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_مٿاهون_u32(rows01.val[1]), vget_مٿاهون_u32(rows23.val[1]));
}

INLINE فراغ transpose_نياپو_vecs4(ثابت uint8_t *ثابت *inputs,
                                size_t بلاڪ_وٿي, uint32x4_t ٻاهريون_2[16]) {
  ٻاهريون_2[0] = loadu_128(&inputs[0][بلاڪ_وٿي + 0 * حجم(uint32x4_t)]);
  ٻاهريون_2[1] = loadu_128(&inputs[1][بلاڪ_وٿي + 0 * حجم(uint32x4_t)]);
  ٻاهريون_2[2] = loadu_128(&inputs[2][بلاڪ_وٿي + 0 * حجم(uint32x4_t)]);
  ٻاهريون_2[3] = loadu_128(&inputs[3][بلاڪ_وٿي + 0 * حجم(uint32x4_t)]);
  ٻاهريون_2[4] = loadu_128(&inputs[0][بلاڪ_وٿي + 1 * حجم(uint32x4_t)]);
  ٻاهريون_2[5] = loadu_128(&inputs[1][بلاڪ_وٿي + 1 * حجم(uint32x4_t)]);
  ٻاهريون_2[6] = loadu_128(&inputs[2][بلاڪ_وٿي + 1 * حجم(uint32x4_t)]);
  ٻاهريون_2[7] = loadu_128(&inputs[3][بلاڪ_وٿي + 1 * حجم(uint32x4_t)]);
  ٻاهريون_2[8] = loadu_128(&inputs[0][بلاڪ_وٿي + 2 * حجم(uint32x4_t)]);
  ٻاهريون_2[9] = loadu_128(&inputs[1][بلاڪ_وٿي + 2 * حجم(uint32x4_t)]);
  ٻاهريون_2[10] = loadu_128(&inputs[2][بلاڪ_وٿي + 2 * حجم(uint32x4_t)]);
  ٻاهريون_2[11] = loadu_128(&inputs[3][بلاڪ_وٿي + 2 * حجم(uint32x4_t)]);
  ٻاهريون_2[12] = loadu_128(&inputs[0][بلاڪ_وٿي + 3 * حجم(uint32x4_t)]);
  ٻاهريون_2[13] = loadu_128(&inputs[1][بلاڪ_وٿي + 3 * حجم(uint32x4_t)]);
  ٻاهريون_2[14] = loadu_128(&inputs[2][بلاڪ_وٿي + 3 * حجم(uint32x4_t)]);
  ٻاهريون_2[15] = loadu_128(&inputs[3][بلاڪ_وٿي + 3 * حجم(uint32x4_t)]);
  transpose_vecs_128(&ٻاهريون_2[0]);
  transpose_vecs_128(&ٻاهريون_2[4]);
  transpose_vecs_128(&ٻاهريون_2[8]);
  transpose_vecs_128(&ٻاهريون_2[12]);
}

INLINE فراغ ڀريو_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *ٻاهريون_هيٺيون, uint32x4_t *ٻاهريون_مٿاهون) {
  uint64_t نقاب_2 = (increment_counter ? ~0 : 0);
  *ٻاهريون_هيٺيون = set4(
      counter_هيٺيون(counter + (نقاب_2 & 0)), counter_هيٺيون(counter + (نقاب_2 & 1)),
      counter_هيٺيون(counter + (نقاب_2 & 2)), counter_هيٺيون(counter + (نقاب_2 & 3)));
  *ٻاهريون_مٿاهون = set4(
      counter_مٿاهون(counter + (نقاب_2 & 0)), counter_مٿاهون(counter + (نقاب_2 & 1)),
      counter_مٿاهون(counter + (نقاب_2 & 2)), counter_مٿاهون(counter + (نقاب_2 & 3)));
}

ساكن فراغ blake3_hash4_neon(ثابت uint8_t *ثابت *inputs, size_t بلاڪون,
                              ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_شروعات, uint8_t flags_پڇاڙي,
                              uint8_t *ٻاهريون_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(ڪنجي_2[0]), set1_128(ڪنجي_2[1]), set1_128(ڪنجي_2[2]), set1_128(ڪنجي_2[3]),
      set1_128(ڪنجي_2[4]), set1_128(ڪنجي_2[5]), set1_128(ڪنجي_2[6]), set1_128(ڪنجي_2[7]),
  };
  uint32x4_t counter_هيٺيون_vec, counter_مٿاهون_vec;
  ڀريو_counters4(counter, increment_counter, &counter_هيٺيون_vec,
                 &counter_مٿاهون_vec);
  uint8_t بلاڪ_flags = flags | flags_شروعات;

  لكل (size_t بلاڪ = 0; بلاڪ < بلاڪون; بلاڪ++) {
    إذا (بلاڪ + 1 == بلاڪون) {
      بلاڪ_flags |= flags_پڇاڙي;
    }
    uint32x4_t بلاڪ_len_vec = set1_128(BLAKE3_بلاڪ_LEN);
    uint32x4_t بلاڪ_flags_vec = set1_128(بلاڪ_flags);
    uint32x4_t نياپو_vecs[16];
    transpose_نياپو_vecs4(inputs, بلاڪ * BLAKE3_بلاڪ_LEN, نياپو_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_هيٺيون_vec, counter_مٿاهون_vec, بلاڪ_len_vec,   بلاڪ_flags_vec,
    };
    ڦيرو_fn4(v, نياپو_vecs, 0);
    ڦيرو_fn4(v, نياپو_vecs, 1);
    ڦيرو_fn4(v, نياپو_vecs, 2);
    ڦيرو_fn4(v, نياپو_vecs, 3);
    ڦيرو_fn4(v, نياپو_vecs, 4);
    ڦيرو_fn4(v, نياپو_vecs, 5);
    ڦيرو_fn4(v, نياپو_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    بلاڪ_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &ٻاهريون_2[0 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[4], &ٻاهريون_2[1 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[1], &ٻاهريون_2[2 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[5], &ٻاهريون_2[3 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[2], &ٻاهريون_2[4 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[6], &ٻاهريون_2[5 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[3], &ٻاهريون_2[6 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[7], &ٻاهريون_2[7 * حجم(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

فراغ blake3_compress_اندريون_رکو_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                                       uint8_t بلاڪ_len, uint64_t counter,
                                       uint8_t flags);

INLINE فراغ hash_one_neon(ثابت uint8_t *داخلا_2, size_t بلاڪون,
                          ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_شروعات, uint8_t flags_پڇاڙي,
                          uint8_t ٻاهريون_2[BLAKE3_ٻاهريون_LEN]) {
  uint32_t cv[8];
  يادگيري_نقل(cv, ڪنجي_2, BLAKE3_ڪنجي_LEN);
  uint8_t بلاڪ_flags = flags | flags_شروعات;
  طالما (بلاڪون > 0) {
    إذا (بلاڪون == 1) {
      بلاڪ_flags |= flags_پڇاڙي;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_اندريون_رکو_portable(cv, داخلا_2, BLAKE3_بلاڪ_LEN, counter,
                                      بلاڪ_flags);
    داخلا_2 = &داخلا_2[BLAKE3_بلاڪ_LEN];
    بلاڪون -= 1;
    بلاڪ_flags = flags;
  }
  يادگيري_نقل(ٻاهريون_2, cv, BLAKE3_ٻاهريون_LEN);
}

فراغ blake3_hash_many_neon(ثابت uint8_t *ثابت *inputs, size_t انگ_inputs,
                           size_t بلاڪون, ثابت uint32_t ڪنجي_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروعات,
                           uint8_t flags_پڇاڙي, uint8_t *ٻاهريون_2) {
  طالما (انگ_inputs >= 4) {
    blake3_hash4_neon(inputs, بلاڪون, ڪنجي_2, counter, increment_counter, flags,
                      flags_شروعات, flags_پڇاڙي, ٻاهريون_2);
    إذا (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    انگ_inputs -= 4;
    ٻاهريون_2 = &ٻاهريون_2[4 * BLAKE3_ٻاهريون_LEN];
  }
  طالما (انگ_inputs > 0) {
    hash_one_neon(inputs[0], بلاڪون, ڪنجي_2, counter, flags, flags_شروعات,
                  flags_پڇاڙي, ٻاهريون_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    انگ_inputs -= 1;
    ٻاهريون_2 = &ٻاهريون_2[BLAKE3_ٻاهريون_LEN];
  }
}

#جيڪڏهن_پورو // BLAKE3_USE_NEON
