#ਅਨ੍ਤਰ੍ਭਾਵ "blake3_impl.h"

#ਜੇ BLAKE3_USE_NEON

#ਅਨ੍ਤਰ੍ਭਾਵ <arm_neon.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ARM_BIG_ENDIAN
#ਗਲਤੀ "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#ਜੇ_ਅੰਤ

INLINE uint32x4_t loadu_128(ਅਚਲ uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  ਵਾਪਸ vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE ਖਾਲੀ storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t ਜੋੜੋ_128(uint32x4_t a, uint32x4_t b) {
  ਵਾਪਸ vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  ਵਾਪਸ veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { ਵਾਪਸ vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  ਵਾਪਸ vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  ਵਾਪਸ vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  ਵਾਪਸ vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#ਜੇ defined(__clang__)
  ਵਾਪਸ vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#ਜਾਂਜੇ defined(__GNUC__)
  ਸਥਿਰ ਅਚਲ uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  ਵਾਪਸ vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#ਨਹੀਂਤਾਂ 
  ਵਾਪਸ vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#ਜੇ_ਅੰਤ
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  ਵਾਪਸ vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE ਖਾਲੀ ਗੇੜਾ_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = ਜੋੜੋ_128(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][0]]);
  v[1] = ਜੋੜੋ_128(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][2]]);
  v[2] = ਜੋੜੋ_128(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][4]]);
  v[3] = ਜੋੜੋ_128(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][6]]);
  v[0] = ਜੋੜੋ_128(v[0], v[4]);
  v[1] = ਜੋੜੋ_128(v[1], v[5]);
  v[2] = ਜੋੜੋ_128(v[2], v[6]);
  v[3] = ਜੋੜੋ_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = ਜੋੜੋ_128(v[8], v[12]);
  v[9] = ਜੋੜੋ_128(v[9], v[13]);
  v[10] = ਜੋੜੋ_128(v[10], v[14]);
  v[11] = ਜੋੜੋ_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = ਜੋੜੋ_128(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][1]]);
  v[1] = ਜੋੜੋ_128(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][3]]);
  v[2] = ਜੋੜੋ_128(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][5]]);
  v[3] = ਜੋੜੋ_128(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][7]]);
  v[0] = ਜੋੜੋ_128(v[0], v[4]);
  v[1] = ਜੋੜੋ_128(v[1], v[5]);
  v[2] = ਜੋੜੋ_128(v[2], v[6]);
  v[3] = ਜੋੜੋ_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = ਜੋੜੋ_128(v[8], v[12]);
  v[9] = ਜੋੜੋ_128(v[9], v[13]);
  v[10] = ਜੋੜੋ_128(v[10], v[14]);
  v[11] = ਜੋੜੋ_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = ਜੋੜੋ_128(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][8]]);
  v[1] = ਜੋੜੋ_128(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][10]]);
  v[2] = ਜੋੜੋ_128(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][12]]);
  v[3] = ਜੋੜੋ_128(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][14]]);
  v[0] = ਜੋੜੋ_128(v[0], v[5]);
  v[1] = ਜੋੜੋ_128(v[1], v[6]);
  v[2] = ਜੋੜੋ_128(v[2], v[7]);
  v[3] = ਜੋੜੋ_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = ਜੋੜੋ_128(v[10], v[15]);
  v[11] = ਜੋੜੋ_128(v[11], v[12]);
  v[8] = ਜੋੜੋ_128(v[8], v[13]);
  v[9] = ਜੋੜੋ_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = ਜੋੜੋ_128(v[0], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][9]]);
  v[1] = ਜੋੜੋ_128(v[1], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][11]]);
  v[2] = ਜੋੜੋ_128(v[2], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][13]]);
  v[3] = ਜੋੜੋ_128(v[3], m[(size_t)ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[r][15]]);
  v[0] = ਜੋੜੋ_128(v[0], v[5]);
  v[1] = ਜੋੜੋ_128(v[1], v[6]);
  v[2] = ਜੋੜੋ_128(v[2], v[7]);
  v[3] = ਜੋੜੋ_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = ਜੋੜੋ_128(v[10], v[15]);
  v[11] = ਜੋੜੋ_128(v[11], v[12]);
  v[8] = ਜੋੜੋ_128(v[8], v[13]);
  v[9] = ਜੋੜੋ_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE ਖਾਲੀ transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_ਨੀਵਾਂ_u32(rows01.val[0]), vget_ਨੀਵਾਂ_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_ਨੀਵਾਂ_u32(rows01.val[1]), vget_ਨੀਵਾਂ_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_ਉੱਚਾ_u32(rows01.val[0]), vget_ਉੱਚਾ_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_ਉੱਚਾ_u32(rows01.val[1]), vget_ਉੱਚਾ_u32(rows23.val[1]));
}

INLINE ਖਾਲੀ transpose_ਸੁਨੇਹਾ_vecs4(ਅਚਲ uint8_t *ਅਚਲ *inputs,
                                size_t ਖੰਡ_ਵਿੱਥ, uint32x4_t ਬਾਹਰਲਾ_2[16]) {
  ਬਾਹਰਲਾ_2[0] = loadu_128(&inputs[0][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[1] = loadu_128(&inputs[1][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[2] = loadu_128(&inputs[2][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[3] = loadu_128(&inputs[3][ਖੰਡ_ਵਿੱਥ + 0 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[4] = loadu_128(&inputs[0][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[5] = loadu_128(&inputs[1][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[6] = loadu_128(&inputs[2][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[7] = loadu_128(&inputs[3][ਖੰਡ_ਵਿੱਥ + 1 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[8] = loadu_128(&inputs[0][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[9] = loadu_128(&inputs[1][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[10] = loadu_128(&inputs[2][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[11] = loadu_128(&inputs[3][ਖੰਡ_ਵਿੱਥ + 2 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[12] = loadu_128(&inputs[0][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[13] = loadu_128(&inputs[1][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[14] = loadu_128(&inputs[2][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(uint32x4_t)]);
  ਬਾਹਰਲਾ_2[15] = loadu_128(&inputs[3][ਖੰਡ_ਵਿੱਥ + 3 * ਆਕਾਰ(uint32x4_t)]);
  transpose_vecs_128(&ਬਾਹਰਲਾ_2[0]);
  transpose_vecs_128(&ਬਾਹਰਲਾ_2[4]);
  transpose_vecs_128(&ਬਾਹਰਲਾ_2[8]);
  transpose_vecs_128(&ਬਾਹਰਲਾ_2[12]);
}

INLINE ਖਾਲੀ ਲੱਦੋ_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *ਬਾਹਰਲਾ_ਨੀਵਾਂ, uint32x4_t *ਬਾਹਰਲਾ_ਉੱਚਾ) {
  uint64_t ਨਕਾਬ_2 = (increment_counter ? ~0 : 0);
  *ਬਾਹਰਲਾ_ਨੀਵਾਂ = set4(
      counter_ਨੀਵਾਂ(counter + (ਨਕਾਬ_2 & 0)), counter_ਨੀਵਾਂ(counter + (ਨਕਾਬ_2 & 1)),
      counter_ਨੀਵਾਂ(counter + (ਨਕਾਬ_2 & 2)), counter_ਨੀਵਾਂ(counter + (ਨਕਾਬ_2 & 3)));
  *ਬਾਹਰਲਾ_ਉੱਚਾ = set4(
      counter_ਉੱਚਾ(counter + (ਨਕਾਬ_2 & 0)), counter_ਉੱਚਾ(counter + (ਨਕਾਬ_2 & 1)),
      counter_ਉੱਚਾ(counter + (ਨਕਾਬ_2 & 2)), counter_ਉੱਚਾ(counter + (ਨਕਾਬ_2 & 3)));
}

ਸਥਿਰ ਖਾਲੀ blake3_hash4_neon(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਖੰਡਾਂ,
                              ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_ਸ਼ੁਰੂ, uint8_t flags_ਸਿਰਾ,
                              uint8_t *ਬਾਹਰਲਾ_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(ਕੁੰਜੀ_2[0]), set1_128(ਕੁੰਜੀ_2[1]), set1_128(ਕੁੰਜੀ_2[2]), set1_128(ਕੁੰਜੀ_2[3]),
      set1_128(ਕੁੰਜੀ_2[4]), set1_128(ਕੁੰਜੀ_2[5]), set1_128(ਕੁੰਜੀ_2[6]), set1_128(ਕੁੰਜੀ_2[7]),
  };
  uint32x4_t counter_ਨੀਵਾਂ_vec, counter_ਉੱਚਾ_vec;
  ਲੱਦੋ_counters4(counter, increment_counter, &counter_ਨੀਵਾਂ_vec,
                 &counter_ਉੱਚਾ_vec);
  uint8_t ਖੰਡ_flags = flags | flags_ਸ਼ੁਰੂ;

  ਲਈ (size_t ਖੰਡ = 0; ਖੰਡ < ਖੰਡਾਂ; ਖੰਡ++) {
    ਜੇ (ਖੰਡ + 1 == ਖੰਡਾਂ) {
      ਖੰਡ_flags |= flags_ਸਿਰਾ;
    }
    uint32x4_t ਖੰਡ_len_vec = set1_128(BLAKE3_ਖੰਡ_LEN);
    uint32x4_t ਖੰਡ_flags_vec = set1_128(ਖੰਡ_flags);
    uint32x4_t ਸੁਨੇਹਾ_vecs[16];
    transpose_ਸੁਨੇਹਾ_vecs4(inputs, ਖੰਡ * BLAKE3_ਖੰਡ_LEN, ਸੁਨੇਹਾ_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_ਨੀਵਾਂ_vec, counter_ਉੱਚਾ_vec, ਖੰਡ_len_vec,   ਖੰਡ_flags_vec,
    };
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 0);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 1);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 2);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 3);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 4);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 5);
    ਗੇੜਾ_fn4(v, ਸੁਨੇਹਾ_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    ਖੰਡ_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &ਬਾਹਰਲਾ_2[0 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[4], &ਬਾਹਰਲਾ_2[1 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[1], &ਬਾਹਰਲਾ_2[2 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[5], &ਬਾਹਰਲਾ_2[3 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[2], &ਬਾਹਰਲਾ_2[4 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[6], &ਬਾਹਰਲਾ_2[5 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[3], &ਬਾਹਰਲਾ_2[6 * ਆਕਾਰ(uint32x4_t)]);
  storeu_128(h_vecs[7], &ਬਾਹਰਲਾ_2[7 * ਆਕਾਰ(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(uint32_t cv[8],
                                       ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                       uint8_t ਖੰਡ_len, uint64_t counter,
                                       uint8_t flags);

INLINE ਖਾਲੀ hash_one_neon(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਖੰਡਾਂ,
                          ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ, uint8_t flags_ਸਿਰਾ,
                          uint8_t ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN]) {
  uint32_t cv[8];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(cv, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  uint8_t ਖੰਡ_flags = flags | flags_ਸ਼ੁਰੂ;
  ਜਦੋਂ (ਖੰਡਾਂ > 0) {
    ਜੇ (ਖੰਡਾਂ == 1) {
      ਖੰਡ_flags |= flags_ਸਿਰਾ;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(cv, ਨਿਵੇਸ਼_2, BLAKE3_ਖੰਡ_LEN, counter,
                                      ਖੰਡ_flags);
    ਨਿਵੇਸ਼_2 = &ਨਿਵੇਸ਼_2[BLAKE3_ਖੰਡ_LEN];
    ਖੰਡਾਂ -= 1;
    ਖੰਡ_flags = flags;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, cv, BLAKE3_ਬਾਹਰਲਾ_LEN);
}

ਖਾਲੀ blake3_hash_many_neon(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                           size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                           uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2) {
  ਜਦੋਂ (ਅੰਕ_inputs >= 4) {
    blake3_hash4_neon(inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter, increment_counter, flags,
                      flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
    ਜੇ (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    ਅੰਕ_inputs -= 4;
    ਬਾਹਰਲਾ_2 = &ਬਾਹਰਲਾ_2[4 * BLAKE3_ਬਾਹਰਲਾ_LEN];
  }
  ਜਦੋਂ (ਅੰਕ_inputs > 0) {
    hash_one_neon(inputs[0], ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter, flags, flags_ਸ਼ੁਰੂ,
                  flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
    ਜੇ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ਅੰਕ_inputs -= 1;
    ਬਾਹਰਲਾ_2 = &ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN];
  }
}

#ਜੇ_ਅੰਤ // BLAKE3_USE_NEON
