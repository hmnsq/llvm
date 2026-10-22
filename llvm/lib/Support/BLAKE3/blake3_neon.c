#ضمن "blake3_impl.h"

#إذا BLAKE3_USE_NEON

#ضمن <arm_neon.h>

#اگر_تعریف_شده __ARM_BIG_ENDIAN
#خطا "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#پایان_اگر

INLINE uint32x4_t loadu_128(ثابت uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  أرجع vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE فراغ storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t جمع_کن_128(uint32x4_t a, uint32x4_t b) {
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
#پایان_اگر
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

INLINE فراغ دور_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = جمع_کن_128(v[0], m[(size_t)پیام_زمانبندی[r][0]]);
  v[1] = جمع_کن_128(v[1], m[(size_t)پیام_زمانبندی[r][2]]);
  v[2] = جمع_کن_128(v[2], m[(size_t)پیام_زمانبندی[r][4]]);
  v[3] = جمع_کن_128(v[3], m[(size_t)پیام_زمانبندی[r][6]]);
  v[0] = جمع_کن_128(v[0], v[4]);
  v[1] = جمع_کن_128(v[1], v[5]);
  v[2] = جمع_کن_128(v[2], v[6]);
  v[3] = جمع_کن_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = جمع_کن_128(v[8], v[12]);
  v[9] = جمع_کن_128(v[9], v[13]);
  v[10] = جمع_کن_128(v[10], v[14]);
  v[11] = جمع_کن_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = جمع_کن_128(v[0], m[(size_t)پیام_زمانبندی[r][1]]);
  v[1] = جمع_کن_128(v[1], m[(size_t)پیام_زمانبندی[r][3]]);
  v[2] = جمع_کن_128(v[2], m[(size_t)پیام_زمانبندی[r][5]]);
  v[3] = جمع_کن_128(v[3], m[(size_t)پیام_زمانبندی[r][7]]);
  v[0] = جمع_کن_128(v[0], v[4]);
  v[1] = جمع_کن_128(v[1], v[5]);
  v[2] = جمع_کن_128(v[2], v[6]);
  v[3] = جمع_کن_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = جمع_کن_128(v[8], v[12]);
  v[9] = جمع_کن_128(v[9], v[13]);
  v[10] = جمع_کن_128(v[10], v[14]);
  v[11] = جمع_کن_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = جمع_کن_128(v[0], m[(size_t)پیام_زمانبندی[r][8]]);
  v[1] = جمع_کن_128(v[1], m[(size_t)پیام_زمانبندی[r][10]]);
  v[2] = جمع_کن_128(v[2], m[(size_t)پیام_زمانبندی[r][12]]);
  v[3] = جمع_کن_128(v[3], m[(size_t)پیام_زمانبندی[r][14]]);
  v[0] = جمع_کن_128(v[0], v[5]);
  v[1] = جمع_کن_128(v[1], v[6]);
  v[2] = جمع_کن_128(v[2], v[7]);
  v[3] = جمع_کن_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = جمع_کن_128(v[10], v[15]);
  v[11] = جمع_کن_128(v[11], v[12]);
  v[8] = جمع_کن_128(v[8], v[13]);
  v[9] = جمع_کن_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = جمع_کن_128(v[0], m[(size_t)پیام_زمانبندی[r][9]]);
  v[1] = جمع_کن_128(v[1], m[(size_t)پیام_زمانبندی[r][11]]);
  v[2] = جمع_کن_128(v[2], m[(size_t)پیام_زمانبندی[r][13]]);
  v[3] = جمع_کن_128(v[3], m[(size_t)پیام_زمانبندی[r][15]]);
  v[0] = جمع_کن_128(v[0], v[5]);
  v[1] = جمع_کن_128(v[1], v[6]);
  v[2] = جمع_کن_128(v[2], v[7]);
  v[3] = جمع_کن_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = جمع_کن_128(v[10], v[15]);
  v[11] = جمع_کن_128(v[11], v[12]);
  v[8] = جمع_کن_128(v[8], v[13]);
  v[9] = جمع_کن_128(v[9], v[14]);
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
      vcombine_u32(vget_کم_u32(rows01.val[0]), vget_کم_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_کم_u32(rows01.val[1]), vget_کم_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_بالا_u32(rows01.val[0]), vget_بالا_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_بالا_u32(rows01.val[1]), vget_بالا_u32(rows23.val[1]));
}

INLINE فراغ transpose_پیام_vecs4(ثابت uint8_t *ثابت *inputs,
                                size_t بلوک_جابه_جایی, uint32x4_t بیرون_2[16]) {
  بیرون_2[0] = loadu_128(&inputs[0][بلوک_جابه_جایی + 0 * حجم(uint32x4_t)]);
  بیرون_2[1] = loadu_128(&inputs[1][بلوک_جابه_جایی + 0 * حجم(uint32x4_t)]);
  بیرون_2[2] = loadu_128(&inputs[2][بلوک_جابه_جایی + 0 * حجم(uint32x4_t)]);
  بیرون_2[3] = loadu_128(&inputs[3][بلوک_جابه_جایی + 0 * حجم(uint32x4_t)]);
  بیرون_2[4] = loadu_128(&inputs[0][بلوک_جابه_جایی + 1 * حجم(uint32x4_t)]);
  بیرون_2[5] = loadu_128(&inputs[1][بلوک_جابه_جایی + 1 * حجم(uint32x4_t)]);
  بیرون_2[6] = loadu_128(&inputs[2][بلوک_جابه_جایی + 1 * حجم(uint32x4_t)]);
  بیرون_2[7] = loadu_128(&inputs[3][بلوک_جابه_جایی + 1 * حجم(uint32x4_t)]);
  بیرون_2[8] = loadu_128(&inputs[0][بلوک_جابه_جایی + 2 * حجم(uint32x4_t)]);
  بیرون_2[9] = loadu_128(&inputs[1][بلوک_جابه_جایی + 2 * حجم(uint32x4_t)]);
  بیرون_2[10] = loadu_128(&inputs[2][بلوک_جابه_جایی + 2 * حجم(uint32x4_t)]);
  بیرون_2[11] = loadu_128(&inputs[3][بلوک_جابه_جایی + 2 * حجم(uint32x4_t)]);
  بیرون_2[12] = loadu_128(&inputs[0][بلوک_جابه_جایی + 3 * حجم(uint32x4_t)]);
  بیرون_2[13] = loadu_128(&inputs[1][بلوک_جابه_جایی + 3 * حجم(uint32x4_t)]);
  بیرون_2[14] = loadu_128(&inputs[2][بلوک_جابه_جایی + 3 * حجم(uint32x4_t)]);
  بیرون_2[15] = loadu_128(&inputs[3][بلوک_جابه_جایی + 3 * حجم(uint32x4_t)]);
  transpose_vecs_128(&بیرون_2[0]);
  transpose_vecs_128(&بیرون_2[4]);
  transpose_vecs_128(&بیرون_2[8]);
  transpose_vecs_128(&بیرون_2[12]);
}

INLINE فراغ بارگیری_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *بیرون_کم, uint32x4_t *بیرون_بالا) {
  uint64_t ماسک_2 = (increment_counter ? ~0 : 0);
  *بیرون_کم = set4(
      counter_کم(counter + (ماسک_2 & 0)), counter_کم(counter + (ماسک_2 & 1)),
      counter_کم(counter + (ماسک_2 & 2)), counter_کم(counter + (ماسک_2 & 3)));
  *بیرون_بالا = set4(
      counter_بالا(counter + (ماسک_2 & 0)), counter_بالا(counter + (ماسک_2 & 1)),
      counter_بالا(counter + (ماسک_2 & 2)), counter_بالا(counter + (ماسک_2 & 3)));
}

ساكن فراغ blake3_hash4_neon(ثابت uint8_t *ثابت *inputs, size_t بلوکها,
                              ثابت uint32_t کلید_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_شروع, uint8_t flags_انتها,
                              uint8_t *بیرون_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(کلید_2[0]), set1_128(کلید_2[1]), set1_128(کلید_2[2]), set1_128(کلید_2[3]),
      set1_128(کلید_2[4]), set1_128(کلید_2[5]), set1_128(کلید_2[6]), set1_128(کلید_2[7]),
  };
  uint32x4_t counter_کم_vec, counter_بالا_vec;
  بارگیری_counters4(counter, increment_counter, &counter_کم_vec,
                 &counter_بالا_vec);
  uint8_t بلوک_flags = flags | flags_شروع;

  لكل (size_t بلوک = 0; بلوک < بلوکها; بلوک++) {
    إذا (بلوک + 1 == بلوکها) {
      بلوک_flags |= flags_انتها;
    }
    uint32x4_t بلوک_len_vec = set1_128(BLAKE3_بلوک_LEN);
    uint32x4_t بلوک_flags_vec = set1_128(بلوک_flags);
    uint32x4_t پیام_vecs[16];
    transpose_پیام_vecs4(inputs, بلوک * BLAKE3_بلوک_LEN, پیام_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_کم_vec, counter_بالا_vec, بلوک_len_vec,   بلوک_flags_vec,
    };
    دور_fn4(v, پیام_vecs, 0);
    دور_fn4(v, پیام_vecs, 1);
    دور_fn4(v, پیام_vecs, 2);
    دور_fn4(v, پیام_vecs, 3);
    دور_fn4(v, پیام_vecs, 4);
    دور_fn4(v, پیام_vecs, 5);
    دور_fn4(v, پیام_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    بلوک_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &بیرون_2[0 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[4], &بیرون_2[1 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[1], &بیرون_2[2 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[5], &بیرون_2[3 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[2], &بیرون_2[4 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[6], &بیرون_2[5 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[3], &بیرون_2[6 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[7], &بیرون_2[7 * حجم(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

فراغ blake3_compress_درون_قراربده_portable(uint32_t cv[8],
                                       ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                                       uint8_t بلوک_len, uint64_t counter,
                                       uint8_t flags);

INLINE فراغ hash_one_neon(ثابت uint8_t *درون_داد_2, size_t بلوکها,
                          ثابت uint32_t کلید_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_شروع, uint8_t flags_انتها,
                          uint8_t بیرون_2[BLAKE3_بیرون_LEN]) {
  uint32_t cv[8];
  کپی_حافظه(cv, کلید_2, BLAKE3_کلید_LEN);
  uint8_t بلوک_flags = flags | flags_شروع;
  طالما (بلوکها > 0) {
    إذا (بلوکها == 1) {
      بلوک_flags |= flags_انتها;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_درون_قراربده_portable(cv, درون_داد_2, BLAKE3_بلوک_LEN, counter,
                                      بلوک_flags);
    درون_داد_2 = &درون_داد_2[BLAKE3_بلوک_LEN];
    بلوکها -= 1;
    بلوک_flags = flags;
  }
  کپی_حافظه(بیرون_2, cv, BLAKE3_بیرون_LEN);
}

فراغ blake3_hash_many_neon(ثابت uint8_t *ثابت *inputs, size_t رقم_inputs,
                           size_t بلوکها, ثابت uint32_t کلید_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروع,
                           uint8_t flags_انتها, uint8_t *بیرون_2) {
  طالما (رقم_inputs >= 4) {
    blake3_hash4_neon(inputs, بلوکها, کلید_2, counter, increment_counter, flags,
                      flags_شروع, flags_انتها, بیرون_2);
    إذا (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    رقم_inputs -= 4;
    بیرون_2 = &بیرون_2[4 * BLAKE3_بیرون_LEN];
  }
  طالما (رقم_inputs > 0) {
    hash_one_neon(inputs[0], بلوکها, کلید_2, counter, flags, flags_شروع,
                  flags_انتها, بیرون_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    رقم_inputs -= 1;
    بیرون_2 = &بیرون_2[BLAKE3_بیرون_LEN];
  }
}

#پایان_اگر // BLAKE3_USE_NEON
