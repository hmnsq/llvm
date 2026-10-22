#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

#ಆದರೆ BLAKE3_USE_NEON

#ಅನ್ತರ್ಭಾವ <arm_neon.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ARM_BIG_ENDIAN
#ದೋಷ "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#ಆದರೆ_ಅಂತ್ಯ

INLINE uint32x4_t loadu_128(ಸ್ಥಿರಾಂಕ uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  ಹಿಂತಿರುಗಿ vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE ಶೂನ್ಯ storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t ಕೂಡಿಸು_128(uint32x4_t a, uint32x4_t b) {
  ಹಿಂತಿರುಗಿ vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  ಹಿಂತಿರುಗಿ veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { ಹಿಂತಿರುಗಿ vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  ಹಿಂತಿರುಗಿ vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  ಹಿಂತಿರುಗಿ vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  ಹಿಂತಿರುಗಿ vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#ಆದರೆ defined(__clang__)
  ಹಿಂತಿರುಗಿ vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#ಅಥವಾಆದರೆ defined(__GNUC__)
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  ಹಿಂತಿರುಗಿ vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#ಇಲ್ಲದಿದ್ದರೆ 
  ಹಿಂತಿರುಗಿ vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#ಆದರೆ_ಅಂತ್ಯ
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  ಹಿಂತಿರುಗಿ vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE ಶೂನ್ಯ ಸುತ್ತು_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = ಕೂಡಿಸು_128(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][0]]);
  v[1] = ಕೂಡಿಸು_128(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][2]]);
  v[2] = ಕೂಡಿಸು_128(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][4]]);
  v[3] = ಕೂಡಿಸು_128(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][6]]);
  v[0] = ಕೂಡಿಸು_128(v[0], v[4]);
  v[1] = ಕೂಡಿಸು_128(v[1], v[5]);
  v[2] = ಕೂಡಿಸು_128(v[2], v[6]);
  v[3] = ಕೂಡಿಸು_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = ಕೂಡಿಸು_128(v[8], v[12]);
  v[9] = ಕೂಡಿಸು_128(v[9], v[13]);
  v[10] = ಕೂಡಿಸು_128(v[10], v[14]);
  v[11] = ಕೂಡಿಸು_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = ಕೂಡಿಸು_128(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][1]]);
  v[1] = ಕೂಡಿಸು_128(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][3]]);
  v[2] = ಕೂಡಿಸು_128(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][5]]);
  v[3] = ಕೂಡಿಸು_128(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][7]]);
  v[0] = ಕೂಡಿಸು_128(v[0], v[4]);
  v[1] = ಕೂಡಿಸು_128(v[1], v[5]);
  v[2] = ಕೂಡಿಸು_128(v[2], v[6]);
  v[3] = ಕೂಡಿಸು_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = ಕೂಡಿಸು_128(v[8], v[12]);
  v[9] = ಕೂಡಿಸು_128(v[9], v[13]);
  v[10] = ಕೂಡಿಸು_128(v[10], v[14]);
  v[11] = ಕೂಡಿಸು_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = ಕೂಡಿಸು_128(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][8]]);
  v[1] = ಕೂಡಿಸು_128(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][10]]);
  v[2] = ಕೂಡಿಸು_128(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][12]]);
  v[3] = ಕೂಡಿಸು_128(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][14]]);
  v[0] = ಕೂಡಿಸು_128(v[0], v[5]);
  v[1] = ಕೂಡಿಸು_128(v[1], v[6]);
  v[2] = ಕೂಡಿಸು_128(v[2], v[7]);
  v[3] = ಕೂಡಿಸು_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = ಕೂಡಿಸು_128(v[10], v[15]);
  v[11] = ಕೂಡಿಸು_128(v[11], v[12]);
  v[8] = ಕೂಡಿಸು_128(v[8], v[13]);
  v[9] = ಕೂಡಿಸು_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = ಕೂಡಿಸು_128(v[0], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][9]]);
  v[1] = ಕೂಡಿಸು_128(v[1], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][11]]);
  v[2] = ಕೂಡಿಸು_128(v[2], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][13]]);
  v[3] = ಕೂಡಿಸು_128(v[3], m[(size_t)ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[r][15]]);
  v[0] = ಕೂಡಿಸು_128(v[0], v[5]);
  v[1] = ಕೂಡಿಸು_128(v[1], v[6]);
  v[2] = ಕೂಡಿಸು_128(v[2], v[7]);
  v[3] = ಕೂಡಿಸು_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = ಕೂಡಿಸು_128(v[10], v[15]);
  v[11] = ಕೂಡಿಸು_128(v[11], v[12]);
  v[8] = ಕೂಡಿಸು_128(v[8], v[13]);
  v[9] = ಕೂಡಿಸು_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE ಶೂನ್ಯ transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_ಕೆಳಗಿನ_u32(rows01.val[0]), vget_ಕೆಳಗಿನ_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_ಕೆಳಗಿನ_u32(rows01.val[1]), vget_ಕೆಳಗಿನ_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_ಮೇಲಿನ_u32(rows01.val[0]), vget_ಮೇಲಿನ_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_ಮೇಲಿನ_u32(rows01.val[1]), vget_ಮೇಲಿನ_u32(rows23.val[1]));
}

INLINE ಶೂನ್ಯ transpose_ಸಂದೇಶ_vecs4(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs,
                                size_t ಖಂಡ_ಸ್ಥಾನಾಂತರ, uint32x4_t ಹೊರಹೋಗು_2[16]) {
  ಹೊರಹೋಗು_2[0] = loadu_128(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[1] = loadu_128(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[2] = loadu_128(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[3] = loadu_128(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 0 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[4] = loadu_128(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[5] = loadu_128(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[6] = loadu_128(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[7] = loadu_128(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 1 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[8] = loadu_128(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[9] = loadu_128(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[10] = loadu_128(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[11] = loadu_128(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 2 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[12] = loadu_128(&inputs[0][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[13] = loadu_128(&inputs[1][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[14] = loadu_128(&inputs[2][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(uint32x4_t)]);
  ಹೊರಹೋಗು_2[15] = loadu_128(&inputs[3][ಖಂಡ_ಸ್ಥಾನಾಂತರ + 3 * ಗಾತ್ರ(uint32x4_t)]);
  transpose_vecs_128(&ಹೊರಹೋಗು_2[0]);
  transpose_vecs_128(&ಹೊರಹೋಗು_2[4]);
  transpose_vecs_128(&ಹೊರಹೋಗು_2[8]);
  transpose_vecs_128(&ಹೊರಹೋಗು_2[12]);
}

INLINE ಶೂನ್ಯ ತುಂಬು_counters4(uint64_t counter, bool increment_counter,
                           uint32x4_t *ಹೊರಹೋಗು_ಕೆಳಗಿನ, uint32x4_t *ಹೊರಹೋಗು_ಮೇಲಿನ) {
  uint64_t ಮುಖವಾಡ_2 = (increment_counter ? ~0 : 0);
  *ಹೊರಹೋಗು_ಕೆಳಗಿನ = set4(
      counter_ಕೆಳಗಿನ(counter + (ಮುಖವಾಡ_2 & 0)), counter_ಕೆಳಗಿನ(counter + (ಮುಖವಾಡ_2 & 1)),
      counter_ಕೆಳಗಿನ(counter + (ಮುಖವಾಡ_2 & 2)), counter_ಕೆಳಗಿನ(counter + (ಮುಖವಾಡ_2 & 3)));
  *ಹೊರಹೋಗು_ಮೇಲಿನ = set4(
      counter_ಮೇಲಿನ(counter + (ಮುಖವಾಡ_2 & 0)), counter_ಮೇಲಿನ(counter + (ಮುಖವಾಡ_2 & 1)),
      counter_ಮೇಲಿನ(counter + (ಮುಖವಾಡ_2 & 2)), counter_ಮೇಲಿನ(counter + (ಮುಖವಾಡ_2 & 3)));
}

ಸ್ಥಿರ ಶೂನ್ಯ blake3_hash4_neon(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಖಂಡಗಳು,
                              ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                              bool increment_counter, uint8_t flags,
                              uint8_t flags_ಆರಂಭ, uint8_t flags_ಕೊನೆ,
                              uint8_t *ಹೊರಹೋಗು_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(ಕೀಲಿ_2[0]), set1_128(ಕೀಲಿ_2[1]), set1_128(ಕೀಲಿ_2[2]), set1_128(ಕೀಲಿ_2[3]),
      set1_128(ಕೀಲಿ_2[4]), set1_128(ಕೀಲಿ_2[5]), set1_128(ಕೀಲಿ_2[6]), set1_128(ಕೀಲಿ_2[7]),
  };
  uint32x4_t counter_ಕೆಳಗಿನ_vec, counter_ಮೇಲಿನ_vec;
  ತುಂಬು_counters4(counter, increment_counter, &counter_ಕೆಳಗಿನ_vec,
                 &counter_ಮೇಲಿನ_vec);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;

  ಗಾಗಿ (size_t ಖಂಡ = 0; ಖಂಡ < ಖಂಡಗಳು; ಖಂಡ++) {
    ಆದರೆ (ಖಂಡ + 1 == ಖಂಡಗಳು) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    uint32x4_t ಖಂಡ_len_vec = set1_128(BLAKE3_ಖಂಡ_LEN);
    uint32x4_t ಖಂಡ_flags_vec = set1_128(ಖಂಡ_flags);
    uint32x4_t ಸಂದೇಶ_vecs[16];
    transpose_ಸಂದೇಶ_vecs4(inputs, ಖಂಡ * BLAKE3_ಖಂಡ_LEN, ಸಂದೇಶ_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        counter_ಕೆಳಗಿನ_vec, counter_ಮೇಲಿನ_vec, ಖಂಡ_len_vec,   ಖಂಡ_flags_vec,
    };
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 0);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 1);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 2);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 3);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 4);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 5);
    ಸುತ್ತು_fn4(v, ಸಂದೇಶ_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    ಖಂಡ_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &ಹೊರಹೋಗು_2[0 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[4], &ಹೊರಹೋಗು_2[1 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[1], &ಹೊರಹೋಗು_2[2 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[5], &ಹೊರಹೋಗು_2[3 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[2], &ಹೊರಹೋಗು_2[4 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[6], &ಹೊರಹೋಗು_2[5 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[3], &ಹೊರಹೋಗು_2[6 * ಗಾತ್ರ(uint32x4_t)]);
  storeu_128(h_vecs[7], &ಹೊರಹೋಗು_2[7 * ಗಾತ್ರ(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

ಶೂನ್ಯ blake3_compress_ಒಳ_ಇರಿಸು_portable(uint32_t cv[8],
                                       ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                                       uint8_t ಖಂಡ_len, uint64_t counter,
                                       uint8_t flags);

INLINE ಶೂನ್ಯ hash_one_neon(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಖಂಡಗಳು,
                          ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                          uint8_t flags, uint8_t flags_ಆರಂಭ, uint8_t flags_ಕೊನೆ,
                          uint8_t ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN]) {
  uint32_t cv[8];
  ಸ್ಮರಣೆ_ನಕಲು(cv, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;
  ಆಗ (ಖಂಡಗಳು > 0) {
    ಆದರೆ (ಖಂಡಗಳು == 1) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_ಒಳ_ಇರಿಸು_portable(cv, ಒಳಹರಿವು_2, BLAKE3_ಖಂಡ_LEN, counter,
                                      ಖಂಡ_flags);
    ಒಳಹರಿವು_2 = &ಒಳಹರಿವು_2[BLAKE3_ಖಂಡ_LEN];
    ಖಂಡಗಳು -= 1;
    ಖಂಡ_flags = flags;
  }
  ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, cv, BLAKE3_ಹೊರಹೋಗು_LEN);
}

ಶೂನ್ಯ blake3_hash_many_neon(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                           size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ಆರಂಭ,
                           uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  ಆಗ (ಅಂಕೆ_inputs >= 4) {
    blake3_hash4_neon(inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter, increment_counter, flags,
                      flags_ಆರಂಭ, flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += 4;
    }
    inputs += 4;
    ಅಂಕೆ_inputs -= 4;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[4 * BLAKE3_ಹೊರಹೋಗು_LEN];
  }
  ಆಗ (ಅಂಕೆ_inputs > 0) {
    hash_one_neon(inputs[0], ಖಂಡಗಳು, ಕೀಲಿ_2, counter, flags, flags_ಆರಂಭ,
                  flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ಅಂಕೆ_inputs -= 1;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN];
  }
}

#ಆದರೆ_ಅಂತ್ಯ // BLAKE3_USE_NEON
