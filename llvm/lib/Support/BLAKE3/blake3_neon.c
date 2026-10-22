#شامل "blake3_impl.h"

#اگر BLAKE3_USE_NEON

#شامل <arm_neon.h>

#اگر_تعریف_شدہ __ARM_BIG_ENDIAN
#غلطی "This implementation only supports little-endian ARM."
// It might be that all we need for big-endian support here is to get the loads
// and stores right, but step zero would be finding a way to test it in CI.
#اگر_ختم

INLINE uint32x4_t loadu_128(ثابت uint8_t src[16]) {
  // vld1q_u32 has alignment requirements. Don't use it.
  واپس vreinterpretq_u32_u8(vld1q_u8(src));
}

INLINE خالی storeu_128(uint32x4_t src, uint8_t dest[16]) {
  // vst1q_u32 has alignment requirements. Don't use it.
  vst1q_u8(dest, vreinterpretq_u8_u32(src));
}

INLINE uint32x4_t جوڑو_128(uint32x4_t a, uint32x4_t b) {
  واپس vaddq_u32(a, b);
}

INLINE uint32x4_t xor_128(uint32x4_t a, uint32x4_t b) {
  واپس veorq_u32(a, b);
}

INLINE uint32x4_t set1_128(uint32_t x) { واپس vld1q_dup_u32(&x); }

INLINE uint32x4_t set4(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t array[4] = {a, b, c, d};
  واپس vld1q_u32(array);
}

INLINE uint32x4_t rot16_128(uint32x4_t x) {
  // The straightforward implementation would be two shifts and an or, but that's
  // slower on microarchitectures we've tested. See
  // https://github.com/BLAKE3-team/BLAKE3/pull/319.
  // return vorrq_u32(vshrq_n_u32(x, 16), vshlq_n_u32(x, 32 - 16));
  واپس vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(x)));
}

INLINE uint32x4_t rot12_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 12), vshlq_n_u32(x, 32 - 12));
  واپس vsriq_n_u32(vshlq_n_u32(x, 32-12), x, 12);
}

INLINE uint32x4_t rot8_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 8), vshlq_n_u32(x, 32 - 8));
#اگر defined(__clang__)
  واپس vreinterpretq_u32_u8(__builtin_shufflevector(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), 1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12));
#وگرنہ_اگر defined(__GNUC__)
  ساکن ثابت uint8x16_t r8 = {1,2,3,0,5,6,7,4,9,10,11,8,13,14,15,12};
  واپس vreinterpretq_u32_u8(__builtin_shuffle(vreinterpretq_u8_u32(x), vreinterpretq_u8_u32(x), r8));
#ورنہ 
  واپس vsriq_n_u32(vshlq_n_u32(x, 32-8), x, 8);
#اگر_ختم
}

INLINE uint32x4_t rot7_128(uint32x4_t x) {
  // See comment in rot16_128.
  // return vorrq_u32(vshrq_n_u32(x, 7), vshlq_n_u32(x, 32 - 7));
  واپس vsriq_n_u32(vshlq_n_u32(x, 32-7), x, 7);
}

// TODO: compress_neon

// TODO: hash2_neon

/*
 * ----------------------------------------------------------------------------
 * hash4_neon
 * ----------------------------------------------------------------------------
 */

INLINE خالی باری_fn4(uint32x4_t v[16], uint32x4_t m[16], size_t r) {
  v[0] = جوڑو_128(v[0], m[(size_t)پیغام_جدول_وقت[r][0]]);
  v[1] = جوڑو_128(v[1], m[(size_t)پیغام_جدول_وقت[r][2]]);
  v[2] = جوڑو_128(v[2], m[(size_t)پیغام_جدول_وقت[r][4]]);
  v[3] = جوڑو_128(v[3], m[(size_t)پیغام_جدول_وقت[r][6]]);
  v[0] = جوڑو_128(v[0], v[4]);
  v[1] = جوڑو_128(v[1], v[5]);
  v[2] = جوڑو_128(v[2], v[6]);
  v[3] = جوڑو_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[15] = rot16_128(v[15]);
  v[8] = جوڑو_128(v[8], v[12]);
  v[9] = جوڑو_128(v[9], v[13]);
  v[10] = جوڑو_128(v[10], v[14]);
  v[11] = جوڑو_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot12_128(v[4]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[0] = جوڑو_128(v[0], m[(size_t)پیغام_جدول_وقت[r][1]]);
  v[1] = جوڑو_128(v[1], m[(size_t)پیغام_جدول_وقت[r][3]]);
  v[2] = جوڑو_128(v[2], m[(size_t)پیغام_جدول_وقت[r][5]]);
  v[3] = جوڑو_128(v[3], m[(size_t)پیغام_جدول_وقت[r][7]]);
  v[0] = جوڑو_128(v[0], v[4]);
  v[1] = جوڑو_128(v[1], v[5]);
  v[2] = جوڑو_128(v[2], v[6]);
  v[3] = جوڑو_128(v[3], v[7]);
  v[12] = xor_128(v[12], v[0]);
  v[13] = xor_128(v[13], v[1]);
  v[14] = xor_128(v[14], v[2]);
  v[15] = xor_128(v[15], v[3]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[15] = rot8_128(v[15]);
  v[8] = جوڑو_128(v[8], v[12]);
  v[9] = جوڑو_128(v[9], v[13]);
  v[10] = جوڑو_128(v[10], v[14]);
  v[11] = جوڑو_128(v[11], v[15]);
  v[4] = xor_128(v[4], v[8]);
  v[5] = xor_128(v[5], v[9]);
  v[6] = xor_128(v[6], v[10]);
  v[7] = xor_128(v[7], v[11]);
  v[4] = rot7_128(v[4]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);

  v[0] = جوڑو_128(v[0], m[(size_t)پیغام_جدول_وقت[r][8]]);
  v[1] = جوڑو_128(v[1], m[(size_t)پیغام_جدول_وقت[r][10]]);
  v[2] = جوڑو_128(v[2], m[(size_t)پیغام_جدول_وقت[r][12]]);
  v[3] = جوڑو_128(v[3], m[(size_t)پیغام_جدول_وقت[r][14]]);
  v[0] = جوڑو_128(v[0], v[5]);
  v[1] = جوڑو_128(v[1], v[6]);
  v[2] = جوڑو_128(v[2], v[7]);
  v[3] = جوڑو_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot16_128(v[15]);
  v[12] = rot16_128(v[12]);
  v[13] = rot16_128(v[13]);
  v[14] = rot16_128(v[14]);
  v[10] = جوڑو_128(v[10], v[15]);
  v[11] = جوڑو_128(v[11], v[12]);
  v[8] = جوڑو_128(v[8], v[13]);
  v[9] = جوڑو_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot12_128(v[5]);
  v[6] = rot12_128(v[6]);
  v[7] = rot12_128(v[7]);
  v[4] = rot12_128(v[4]);
  v[0] = جوڑو_128(v[0], m[(size_t)پیغام_جدول_وقت[r][9]]);
  v[1] = جوڑو_128(v[1], m[(size_t)پیغام_جدول_وقت[r][11]]);
  v[2] = جوڑو_128(v[2], m[(size_t)پیغام_جدول_وقت[r][13]]);
  v[3] = جوڑو_128(v[3], m[(size_t)پیغام_جدول_وقت[r][15]]);
  v[0] = جوڑو_128(v[0], v[5]);
  v[1] = جوڑو_128(v[1], v[6]);
  v[2] = جوڑو_128(v[2], v[7]);
  v[3] = جوڑو_128(v[3], v[4]);
  v[15] = xor_128(v[15], v[0]);
  v[12] = xor_128(v[12], v[1]);
  v[13] = xor_128(v[13], v[2]);
  v[14] = xor_128(v[14], v[3]);
  v[15] = rot8_128(v[15]);
  v[12] = rot8_128(v[12]);
  v[13] = rot8_128(v[13]);
  v[14] = rot8_128(v[14]);
  v[10] = جوڑو_128(v[10], v[15]);
  v[11] = جوڑو_128(v[11], v[12]);
  v[8] = جوڑو_128(v[8], v[13]);
  v[9] = جوڑو_128(v[9], v[14]);
  v[5] = xor_128(v[5], v[10]);
  v[6] = xor_128(v[6], v[11]);
  v[7] = xor_128(v[7], v[8]);
  v[4] = xor_128(v[4], v[9]);
  v[5] = rot7_128(v[5]);
  v[6] = rot7_128(v[6]);
  v[7] = rot7_128(v[7]);
  v[4] = rot7_128(v[4]);
}

INLINE خالی transpose_vecs_128(uint32x4_t vecs[4]) {
  // Individually transpose the four 2x2 sub-matrices in each corner.
  uint32x4x2_t rows01 = vtrnq_u32(vecs[0], vecs[1]);
  uint32x4x2_t rows23 = vtrnq_u32(vecs[2], vecs[3]);

  // Swap the top-right and bottom-left 2x2s (which just got transposed).
  vecs[0] =
      vcombine_u32(vget_کم_u32(rows01.val[0]), vget_کم_u32(rows23.val[0]));
  vecs[1] =
      vcombine_u32(vget_کم_u32(rows01.val[1]), vget_کم_u32(rows23.val[1]));
  vecs[2] =
      vcombine_u32(vget_بلند_u32(rows01.val[0]), vget_بلند_u32(rows23.val[0]));
  vecs[3] =
      vcombine_u32(vget_بلند_u32(rows01.val[1]), vget_بلند_u32(rows23.val[1]));
}

INLINE خالی transpose_پیغام_vecs4(ثابت uint8_t *ثابت *inputs,
                                size_t بلاک_انحراف, uint32x4_t برآمد_2[16]) {
  برآمد_2[0] = loadu_128(&inputs[0][بلاک_انحراف + 0 * حجم(uint32x4_t)]);
  برآمد_2[1] = loadu_128(&inputs[1][بلاک_انحراف + 0 * حجم(uint32x4_t)]);
  برآمد_2[2] = loadu_128(&inputs[2][بلاک_انحراف + 0 * حجم(uint32x4_t)]);
  برآمد_2[3] = loadu_128(&inputs[3][بلاک_انحراف + 0 * حجم(uint32x4_t)]);
  برآمد_2[4] = loadu_128(&inputs[0][بلاک_انحراف + 1 * حجم(uint32x4_t)]);
  برآمد_2[5] = loadu_128(&inputs[1][بلاک_انحراف + 1 * حجم(uint32x4_t)]);
  برآمد_2[6] = loadu_128(&inputs[2][بلاک_انحراف + 1 * حجم(uint32x4_t)]);
  برآمد_2[7] = loadu_128(&inputs[3][بلاک_انحراف + 1 * حجم(uint32x4_t)]);
  برآمد_2[8] = loadu_128(&inputs[0][بلاک_انحراف + 2 * حجم(uint32x4_t)]);
  برآمد_2[9] = loadu_128(&inputs[1][بلاک_انحراف + 2 * حجم(uint32x4_t)]);
  برآمد_2[10] = loadu_128(&inputs[2][بلاک_انحراف + 2 * حجم(uint32x4_t)]);
  برآمد_2[11] = loadu_128(&inputs[3][بلاک_انحراف + 2 * حجم(uint32x4_t)]);
  برآمد_2[12] = loadu_128(&inputs[0][بلاک_انحراف + 3 * حجم(uint32x4_t)]);
  برآمد_2[13] = loadu_128(&inputs[1][بلاک_انحراف + 3 * حجم(uint32x4_t)]);
  برآمد_2[14] = loadu_128(&inputs[2][بلاک_انحراف + 3 * حجم(uint32x4_t)]);
  برآمد_2[15] = loadu_128(&inputs[3][بلاک_انحراف + 3 * حجم(uint32x4_t)]);
  transpose_vecs_128(&برآمد_2[0]);
  transpose_vecs_128(&برآمد_2[4]);
  transpose_vecs_128(&برآمد_2[8]);
  transpose_vecs_128(&برآمد_2[12]);
}

INLINE خالی بھرو_counters4(uint64_t شمار_3, bool increment_شمار,
                           uint32x4_t *برآمد_کم, uint32x4_t *برآمد_بلند) {
  uint64_t نقاب_2 = (increment_شمار ? ~0 : 0);
  *برآمد_کم = set4(
      شمار_کم(شمار_3 + (نقاب_2 & 0)), شمار_کم(شمار_3 + (نقاب_2 & 1)),
      شمار_کم(شمار_3 + (نقاب_2 & 2)), شمار_کم(شمار_3 + (نقاب_2 & 3)));
  *برآمد_بلند = set4(
      شمار_بلند(شمار_3 + (نقاب_2 & 0)), شمار_بلند(شمار_3 + (نقاب_2 & 1)),
      شمار_بلند(شمار_3 + (نقاب_2 & 2)), شمار_بلند(شمار_3 + (نقاب_2 & 3)));
}

ساکن خالی blake3_hash4_neon(ثابت uint8_t *ثابت *inputs, size_t بلاکیں,
                              ثابت uint32_t کلید_2[8], uint64_t شمار_3,
                              bool increment_شمار, uint8_t flags,
                              uint8_t flags_آغاز, uint8_t flags_انتہا,
                              uint8_t *برآمد_2) {
  uint32x4_t h_vecs[8] = {
      set1_128(کلید_2[0]), set1_128(کلید_2[1]), set1_128(کلید_2[2]), set1_128(کلید_2[3]),
      set1_128(کلید_2[4]), set1_128(کلید_2[5]), set1_128(کلید_2[6]), set1_128(کلید_2[7]),
  };
  uint32x4_t شمار_کم_vec, شمار_بلند_vec;
  بھرو_counters4(شمار_3, increment_شمار, &شمار_کم_vec,
                 &شمار_بلند_vec);
  uint8_t بلاک_flags = flags | flags_آغاز;

  برائے (size_t بلاک = 0; بلاک < بلاکیں; بلاک++) {
    اگر (بلاک + 1 == بلاکیں) {
      بلاک_flags |= flags_انتہا;
    }
    uint32x4_t بلاک_len_vec = set1_128(BLAKE3_بلاک_LEN);
    uint32x4_t بلاک_flags_vec = set1_128(بلاک_flags);
    uint32x4_t پیغام_vecs[16];
    transpose_پیغام_vecs4(inputs, بلاک * BLAKE3_بلاک_LEN, پیغام_vecs);

    uint32x4_t v[16] = {
        h_vecs[0],       h_vecs[1],        h_vecs[2],       h_vecs[3],
        h_vecs[4],       h_vecs[5],        h_vecs[6],       h_vecs[7],
        set1_128(IV[0]), set1_128(IV[1]),  set1_128(IV[2]), set1_128(IV[3]),
        شمار_کم_vec, شمار_بلند_vec, بلاک_len_vec,   بلاک_flags_vec,
    };
    باری_fn4(v, پیغام_vecs, 0);
    باری_fn4(v, پیغام_vecs, 1);
    باری_fn4(v, پیغام_vecs, 2);
    باری_fn4(v, پیغام_vecs, 3);
    باری_fn4(v, پیغام_vecs, 4);
    باری_fn4(v, پیغام_vecs, 5);
    باری_fn4(v, پیغام_vecs, 6);
    h_vecs[0] = xor_128(v[0], v[8]);
    h_vecs[1] = xor_128(v[1], v[9]);
    h_vecs[2] = xor_128(v[2], v[10]);
    h_vecs[3] = xor_128(v[3], v[11]);
    h_vecs[4] = xor_128(v[4], v[12]);
    h_vecs[5] = xor_128(v[5], v[13]);
    h_vecs[6] = xor_128(v[6], v[14]);
    h_vecs[7] = xor_128(v[7], v[15]);

    بلاک_flags = flags;
  }

  transpose_vecs_128(&h_vecs[0]);
  transpose_vecs_128(&h_vecs[4]);
  // The first four vecs now contain the first half of each output, and the
  // second four vecs contain the second half of each output.
  storeu_128(h_vecs[0], &برآمد_2[0 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[4], &برآمد_2[1 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[1], &برآمد_2[2 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[5], &برآمد_2[3 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[2], &برآمد_2[4 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[6], &برآمد_2[5 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[3], &برآمد_2[6 * حجم(uint32x4_t)]);
  storeu_128(h_vecs[7], &برآمد_2[7 * حجم(uint32x4_t)]);
}

/*
 * ----------------------------------------------------------------------------
 * hash_many_neon
 * ----------------------------------------------------------------------------
 */

خالی blake3_compress_اندر_رکھو_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                       uint8_t بلاک_len, uint64_t شمار_3,
                                       uint8_t flags);

INLINE خالی hash_one_neon(ثابت uint8_t *داخلہ_2, size_t بلاکیں,
                          ثابت uint32_t کلید_2[8], uint64_t شمار_3,
                          uint8_t flags, uint8_t flags_آغاز, uint8_t flags_انتہا,
                          uint8_t برآمد_2[BLAKE3_برآمد_LEN]) {
  uint32_t cv[8];
  حافظہ_نقل(cv, کلید_2, BLAKE3_کلید_LEN);
  uint8_t بلاک_flags = flags | flags_آغاز;
  جب (بلاکیں > 0) {
    اگر (بلاکیں == 1) {
      بلاک_flags |= flags_انتہا;
    }
    // TODO: Implement compress_neon. However note that according to
    // https://github.com/BLAKE2/BLAKE2/commit/7965d3e6e1b4193438b8d3a656787587d2579227,
    // compress_neon might not be any faster than compress_portable.
    blake3_compress_اندر_رکھو_portable(cv, داخلہ_2, BLAKE3_بلاک_LEN, شمار_3,
                                      بلاک_flags);
    داخلہ_2 = &داخلہ_2[BLAKE3_بلاک_LEN];
    بلاکیں -= 1;
    بلاک_flags = flags;
  }
  حافظہ_نقل(برآمد_2, cv, BLAKE3_برآمد_LEN);
}

خالی blake3_hash_many_neon(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                           size_t بلاکیں, ثابت uint32_t کلید_2[8],
                           uint64_t شمار_3, bool increment_شمار,
                           uint8_t flags, uint8_t flags_آغاز,
                           uint8_t flags_انتہا, uint8_t *برآمد_2) {
  جب (عدد_inputs >= 4) {
    blake3_hash4_neon(inputs, بلاکیں, کلید_2, شمار_3, increment_شمار, flags,
                      flags_آغاز, flags_انتہا, برآمد_2);
    اگر (increment_شمار) {
      شمار_3 += 4;
    }
    inputs += 4;
    عدد_inputs -= 4;
    برآمد_2 = &برآمد_2[4 * BLAKE3_برآمد_LEN];
  }
  جب (عدد_inputs > 0) {
    hash_one_neon(inputs[0], بلاکیں, کلید_2, شمار_3, flags, flags_آغاز,
                  flags_انتہا, برآمد_2);
    اگر (increment_شمار) {
      شمار_3 += 1;
    }
    inputs += 1;
    عدد_inputs -= 1;
    برآمد_2 = &برآمد_2[BLAKE3_برآمد_LEN];
  }
}

#اگر_ختم // BLAKE3_USE_NEON
