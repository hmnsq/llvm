#شامل "blake3_impl.h"
#شامل <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  واپس (w >> c) | (w << (32 - c));
}

INLINE خالی g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
              uint32_t x, uint32_t y) {
  state[a] = state[a] + state[b] + x;
  state[d] = rotr32(state[d] ^ state[a], 16);
  state[c] = state[c] + state[d];
  state[b] = rotr32(state[b] ^ state[c], 12);
  state[a] = state[a] + state[b] + y;
  state[d] = rotr32(state[d] ^ state[a], 8);
  state[c] = state[c] + state[d];
  state[b] = rotr32(state[b] ^ state[c], 7);
}

INLINE خالی باری_fn(uint32_t state[16], ثابت uint32_t *پیغام, size_t باری) {
  // Select the message schedule based on the round.
  ثابت uint8_t *جدول_وقت = پیغام_جدول_وقت[باری];

  // Mix the columns.
  g(state, 0, 4, 8, 12, پیغام[جدول_وقت[0]], پیغام[جدول_وقت[1]]);
  g(state, 1, 5, 9, 13, پیغام[جدول_وقت[2]], پیغام[جدول_وقت[3]]);
  g(state, 2, 6, 10, 14, پیغام[جدول_وقت[4]], پیغام[جدول_وقت[5]]);
  g(state, 3, 7, 11, 15, پیغام[جدول_وقت[6]], پیغام[جدول_وقت[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, پیغام[جدول_وقت[8]], پیغام[جدول_وقت[9]]);
  g(state, 1, 6, 11, 12, پیغام[جدول_وقت[10]], پیغام[جدول_وقت[11]]);
  g(state, 2, 7, 8, 13, پیغام[جدول_وقت[12]], پیغام[جدول_وقت[13]]);
  g(state, 3, 4, 9, 14, پیغام[جدول_وقت[14]], پیغام[جدول_وقت[15]]);
}

INLINE خالی compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t شمار_3, uint8_t flags) {
  uint32_t بلاک_الفاظ[16];
  بلاک_الفاظ[0] = load32(بلاک + 4 * 0);
  بلاک_الفاظ[1] = load32(بلاک + 4 * 1);
  بلاک_الفاظ[2] = load32(بلاک + 4 * 2);
  بلاک_الفاظ[3] = load32(بلاک + 4 * 3);
  بلاک_الفاظ[4] = load32(بلاک + 4 * 4);
  بلاک_الفاظ[5] = load32(بلاک + 4 * 5);
  بلاک_الفاظ[6] = load32(بلاک + 4 * 6);
  بلاک_الفاظ[7] = load32(بلاک + 4 * 7);
  بلاک_الفاظ[8] = load32(بلاک + 4 * 8);
  بلاک_الفاظ[9] = load32(بلاک + 4 * 9);
  بلاک_الفاظ[10] = load32(بلاک + 4 * 10);
  بلاک_الفاظ[11] = load32(بلاک + 4 * 11);
  بلاک_الفاظ[12] = load32(بلاک + 4 * 12);
  بلاک_الفاظ[13] = load32(بلاک + 4 * 13);
  بلاک_الفاظ[14] = load32(بلاک + 4 * 14);
  بلاک_الفاظ[15] = load32(بلاک + 4 * 15);

  state[0] = cv[0];
  state[1] = cv[1];
  state[2] = cv[2];
  state[3] = cv[3];
  state[4] = cv[4];
  state[5] = cv[5];
  state[6] = cv[6];
  state[7] = cv[7];
  state[8] = IV[0];
  state[9] = IV[1];
  state[10] = IV[2];
  state[11] = IV[3];
  state[12] = شمار_کم(شمار_3);
  state[13] = شمار_بلند(شمار_3);
  state[14] = (uint32_t)بلاک_len;
  state[15] = (uint32_t)flags;

  باری_fn(state, &بلاک_الفاظ[0], 0);
  باری_fn(state, &بلاک_الفاظ[0], 1);
  باری_fn(state, &بلاک_الفاظ[0], 2);
  باری_fn(state, &بلاک_الفاظ[0], 3);
  باری_fn(state, &بلاک_الفاظ[0], 4);
  باری_fn(state, &بلاک_الفاظ[0], 5);
  باری_fn(state, &بلاک_الفاظ[0], 6);
}

خالی blake3_compress_اندر_رکھو_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                       uint8_t بلاک_len, uint64_t شمار_3,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, بلاک, بلاک_len, شمار_3, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

خالی blake3_compress_xof_portable(ثابت uint32_t cv[8],
                                  ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                  uint8_t بلاک_len, uint64_t شمار_3,
                                  uint8_t flags, uint8_t برآمد_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, بلاک, بلاک_len, شمار_3, flags);

  store32(&برآمد_2[0 * 4], state[0] ^ state[8]);
  store32(&برآمد_2[1 * 4], state[1] ^ state[9]);
  store32(&برآمد_2[2 * 4], state[2] ^ state[10]);
  store32(&برآمد_2[3 * 4], state[3] ^ state[11]);
  store32(&برآمد_2[4 * 4], state[4] ^ state[12]);
  store32(&برآمد_2[5 * 4], state[5] ^ state[13]);
  store32(&برآمد_2[6 * 4], state[6] ^ state[14]);
  store32(&برآمد_2[7 * 4], state[7] ^ state[15]);
  store32(&برآمد_2[8 * 4], state[8] ^ cv[0]);
  store32(&برآمد_2[9 * 4], state[9] ^ cv[1]);
  store32(&برآمد_2[10 * 4], state[10] ^ cv[2]);
  store32(&برآمد_2[11 * 4], state[11] ^ cv[3]);
  store32(&برآمد_2[12 * 4], state[12] ^ cv[4]);
  store32(&برآمد_2[13 * 4], state[13] ^ cv[5]);
  store32(&برآمد_2[14 * 4], state[14] ^ cv[6]);
  store32(&برآمد_2[15 * 4], state[15] ^ cv[7]);
}

INLINE خالی hash_one_portable(ثابت uint8_t *داخلہ_2, size_t بلاکیں,
                              ثابت uint32_t کلید_2[8], uint64_t شمار_3,
                              uint8_t flags, uint8_t flags_آغاز,
                              uint8_t flags_انتہا, uint8_t برآمد_2[BLAKE3_برآمد_LEN]) {
  uint32_t cv[8];
  حافظہ_نقل(cv, کلید_2, BLAKE3_کلید_LEN);
  uint8_t بلاک_flags = flags | flags_آغاز;
  جب (بلاکیں > 0) {
    اگر (بلاکیں == 1) {
      بلاک_flags |= flags_انتہا;
    }
    blake3_compress_اندر_رکھو_portable(cv, داخلہ_2, BLAKE3_بلاک_LEN, شمار_3,
                                      بلاک_flags);
    داخلہ_2 = &داخلہ_2[BLAKE3_بلاک_LEN];
    بلاکیں -= 1;
    بلاک_flags = flags;
  }
  store_cv_الفاظ(برآمد_2, cv);
}

خالی blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                               size_t بلاکیں, ثابت uint32_t کلید_2[8],
                               uint64_t شمار_3, bool increment_شمار,
                               uint8_t flags, uint8_t flags_آغاز,
                               uint8_t flags_انتہا, uint8_t *برآمد_2) {
  جب (عدد_inputs > 0) {
    hash_one_portable(inputs[0], بلاکیں, کلید_2, شمار_3, flags, flags_آغاز,
                      flags_انتہا, برآمد_2);
    اگر (increment_شمار) {
      شمار_3 += 1;
    }
    inputs += 1;
    عدد_inputs -= 1;
    برآمد_2 = &برآمد_2[BLAKE3_برآمد_LEN];
  }
}
