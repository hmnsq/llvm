#ضمن "blake3_impl.h"
#ضمن <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  أرجع (w >> c) | (w << (32 - c));
}

INLINE فراغ g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE فراغ پړاو_fn(uint32_t state[16], ثابت uint32_t *پیغام, size_t پړاو) {
  // Select the message schedule based on the round.
  ثابت uint8_t *مهالوېش = پیغام_مهالوېش[پړاو];

  // Mix the columns.
  g(state, 0, 4, 8, 12, پیغام[مهالوېش[0]], پیغام[مهالوېش[1]]);
  g(state, 1, 5, 9, 13, پیغام[مهالوېش[2]], پیغام[مهالوېش[3]]);
  g(state, 2, 6, 10, 14, پیغام[مهالوېش[4]], پیغام[مهالوېش[5]]);
  g(state, 3, 7, 11, 15, پیغام[مهالوېش[6]], پیغام[مهالوېش[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, پیغام[مهالوېش[8]], پیغام[مهالوېش[9]]);
  g(state, 1, 6, 11, 12, پیغام[مهالوېش[10]], پیغام[مهالوېش[11]]);
  g(state, 2, 7, 8, 13, پیغام[مهالوېش[12]], پیغام[مهالوېش[13]]);
  g(state, 3, 4, 9, 14, پیغام[مهالوېش[14]], پیغام[مهالوېش[15]]);
}

INLINE فراغ compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t counter, uint8_t flags) {
  uint32_t بلاک_ټکي[16];
  بلاک_ټکي[0] = load32(بلاک + 4 * 0);
  بلاک_ټکي[1] = load32(بلاک + 4 * 1);
  بلاک_ټکي[2] = load32(بلاک + 4 * 2);
  بلاک_ټکي[3] = load32(بلاک + 4 * 3);
  بلاک_ټکي[4] = load32(بلاک + 4 * 4);
  بلاک_ټکي[5] = load32(بلاک + 4 * 5);
  بلاک_ټکي[6] = load32(بلاک + 4 * 6);
  بلاک_ټکي[7] = load32(بلاک + 4 * 7);
  بلاک_ټکي[8] = load32(بلاک + 4 * 8);
  بلاک_ټکي[9] = load32(بلاک + 4 * 9);
  بلاک_ټکي[10] = load32(بلاک + 4 * 10);
  بلاک_ټکي[11] = load32(بلاک + 4 * 11);
  بلاک_ټکي[12] = load32(بلاک + 4 * 12);
  بلاک_ټکي[13] = load32(بلاک + 4 * 13);
  بلاک_ټکي[14] = load32(بلاک + 4 * 14);
  بلاک_ټکي[15] = load32(بلاک + 4 * 15);

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
  state[12] = counter_ټیټ(counter);
  state[13] = counter_لوړ(counter);
  state[14] = (uint32_t)بلاک_len;
  state[15] = (uint32_t)flags;

  پړاو_fn(state, &بلاک_ټکي[0], 0);
  پړاو_fn(state, &بلاک_ټکي[0], 1);
  پړاو_fn(state, &بلاک_ټکي[0], 2);
  پړاو_fn(state, &بلاک_ټکي[0], 3);
  پړاو_fn(state, &بلاک_ټکي[0], 4);
  پړاو_fn(state, &بلاک_ټکي[0], 5);
  پړاو_fn(state, &بلاک_ټکي[0], 6);
}

فراغ blake3_compress_دننه_ځای_پرځای_کړه_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                       uint8_t بلاک_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, بلاک, بلاک_len, counter, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

فراغ blake3_compress_xof_portable(ثابت uint32_t cv[8],
                                  ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                  uint8_t بلاک_len, uint64_t counter,
                                  uint8_t flags, uint8_t بهر_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, بلاک, بلاک_len, counter, flags);

  store32(&بهر_2[0 * 4], state[0] ^ state[8]);
  store32(&بهر_2[1 * 4], state[1] ^ state[9]);
  store32(&بهر_2[2 * 4], state[2] ^ state[10]);
  store32(&بهر_2[3 * 4], state[3] ^ state[11]);
  store32(&بهر_2[4 * 4], state[4] ^ state[12]);
  store32(&بهر_2[5 * 4], state[5] ^ state[13]);
  store32(&بهر_2[6 * 4], state[6] ^ state[14]);
  store32(&بهر_2[7 * 4], state[7] ^ state[15]);
  store32(&بهر_2[8 * 4], state[8] ^ cv[0]);
  store32(&بهر_2[9 * 4], state[9] ^ cv[1]);
  store32(&بهر_2[10 * 4], state[10] ^ cv[2]);
  store32(&بهر_2[11 * 4], state[11] ^ cv[3]);
  store32(&بهر_2[12 * 4], state[12] ^ cv[4]);
  store32(&بهر_2[13 * 4], state[13] ^ cv[5]);
  store32(&بهر_2[14 * 4], state[14] ^ cv[6]);
  store32(&بهر_2[15 * 4], state[15] ^ cv[7]);
}

INLINE فراغ hash_one_portable(ثابت uint8_t *ننوت_2, size_t بلاکونه,
                              ثابت uint32_t کلی_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_پیل,
                              uint8_t flags_پای_ټکی, uint8_t بهر_2[BLAKE3_بهر_LEN]) {
  uint32_t cv[8];
  حافظه_نقل(cv, کلی_2, BLAKE3_کلی_LEN);
  uint8_t بلاک_flags = flags | flags_پیل;
  طالما (بلاکونه > 0) {
    إذا (بلاکونه == 1) {
      بلاک_flags |= flags_پای_ټکی;
    }
    blake3_compress_دننه_ځای_پرځای_کړه_portable(cv, ننوت_2, BLAKE3_بلاک_LEN, counter,
                                      بلاک_flags);
    ننوت_2 = &ننوت_2[BLAKE3_بلاک_LEN];
    بلاکونه -= 1;
    بلاک_flags = flags;
  }
  store_cv_ټکي(بهر_2, cv);
}

فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                               size_t بلاکونه, ثابت uint32_t کلی_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_پیل,
                               uint8_t flags_پای_ټکی, uint8_t *بهر_2) {
  طالما (عدد_inputs > 0) {
    hash_one_portable(inputs[0], بلاکونه, کلی_2, counter, flags, flags_پیل,
                      flags_پای_ټکی, بهر_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    عدد_inputs -= 1;
    بهر_2 = &بهر_2[BLAKE3_بهر_LEN];
  }
}
