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

INLINE فراغ دور_fn(uint32_t state[16], ثابت uint32_t *پیام, size_t دور) {
  // Select the message schedule based on the round.
  ثابت uint8_t *زمانبندی = پیام_زمانبندی[دور];

  // Mix the columns.
  g(state, 0, 4, 8, 12, پیام[زمانبندی[0]], پیام[زمانبندی[1]]);
  g(state, 1, 5, 9, 13, پیام[زمانبندی[2]], پیام[زمانبندی[3]]);
  g(state, 2, 6, 10, 14, پیام[زمانبندی[4]], پیام[زمانبندی[5]]);
  g(state, 3, 7, 11, 15, پیام[زمانبندی[6]], پیام[زمانبندی[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, پیام[زمانبندی[8]], پیام[زمانبندی[9]]);
  g(state, 1, 6, 11, 12, پیام[زمانبندی[10]], پیام[زمانبندی[11]]);
  g(state, 2, 7, 8, 13, پیام[زمانبندی[12]], پیام[زمانبندی[13]]);
  g(state, 3, 4, 9, 14, پیام[زمانبندی[14]], پیام[زمانبندی[15]]);
}

INLINE فراغ compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                         uint8_t بلوک_len, uint64_t counter, uint8_t flags) {
  uint32_t بلوک_واژگان[16];
  بلوک_واژگان[0] = load32(بلوک + 4 * 0);
  بلوک_واژگان[1] = load32(بلوک + 4 * 1);
  بلوک_واژگان[2] = load32(بلوک + 4 * 2);
  بلوک_واژگان[3] = load32(بلوک + 4 * 3);
  بلوک_واژگان[4] = load32(بلوک + 4 * 4);
  بلوک_واژگان[5] = load32(بلوک + 4 * 5);
  بلوک_واژگان[6] = load32(بلوک + 4 * 6);
  بلوک_واژگان[7] = load32(بلوک + 4 * 7);
  بلوک_واژگان[8] = load32(بلوک + 4 * 8);
  بلوک_واژگان[9] = load32(بلوک + 4 * 9);
  بلوک_واژگان[10] = load32(بلوک + 4 * 10);
  بلوک_واژگان[11] = load32(بلوک + 4 * 11);
  بلوک_واژگان[12] = load32(بلوک + 4 * 12);
  بلوک_واژگان[13] = load32(بلوک + 4 * 13);
  بلوک_واژگان[14] = load32(بلوک + 4 * 14);
  بلوک_واژگان[15] = load32(بلوک + 4 * 15);

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
  state[12] = counter_کم(counter);
  state[13] = counter_بالا(counter);
  state[14] = (uint32_t)بلوک_len;
  state[15] = (uint32_t)flags;

  دور_fn(state, &بلوک_واژگان[0], 0);
  دور_fn(state, &بلوک_واژگان[0], 1);
  دور_fn(state, &بلوک_واژگان[0], 2);
  دور_fn(state, &بلوک_واژگان[0], 3);
  دور_fn(state, &بلوک_واژگان[0], 4);
  دور_fn(state, &بلوک_واژگان[0], 5);
  دور_fn(state, &بلوک_واژگان[0], 6);
}

فراغ blake3_compress_درون_قراربده_portable(uint32_t cv[8],
                                       ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                                       uint8_t بلوک_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, بلوک, بلوک_len, counter, flags);
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
                                  ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                                  uint8_t بلوک_len, uint64_t counter,
                                  uint8_t flags, uint8_t بیرون_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, بلوک, بلوک_len, counter, flags);

  store32(&بیرون_2[0 * 4], state[0] ^ state[8]);
  store32(&بیرون_2[1 * 4], state[1] ^ state[9]);
  store32(&بیرون_2[2 * 4], state[2] ^ state[10]);
  store32(&بیرون_2[3 * 4], state[3] ^ state[11]);
  store32(&بیرون_2[4 * 4], state[4] ^ state[12]);
  store32(&بیرون_2[5 * 4], state[5] ^ state[13]);
  store32(&بیرون_2[6 * 4], state[6] ^ state[14]);
  store32(&بیرون_2[7 * 4], state[7] ^ state[15]);
  store32(&بیرون_2[8 * 4], state[8] ^ cv[0]);
  store32(&بیرون_2[9 * 4], state[9] ^ cv[1]);
  store32(&بیرون_2[10 * 4], state[10] ^ cv[2]);
  store32(&بیرون_2[11 * 4], state[11] ^ cv[3]);
  store32(&بیرون_2[12 * 4], state[12] ^ cv[4]);
  store32(&بیرون_2[13 * 4], state[13] ^ cv[5]);
  store32(&بیرون_2[14 * 4], state[14] ^ cv[6]);
  store32(&بیرون_2[15 * 4], state[15] ^ cv[7]);
}

INLINE فراغ hash_one_portable(ثابت uint8_t *درون_داد_2, size_t بلوکها,
                              ثابت uint32_t کلید_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_شروع,
                              uint8_t flags_انتها, uint8_t بیرون_2[BLAKE3_بیرون_LEN]) {
  uint32_t cv[8];
  کپی_حافظه(cv, کلید_2, BLAKE3_کلید_LEN);
  uint8_t بلوک_flags = flags | flags_شروع;
  طالما (بلوکها > 0) {
    إذا (بلوکها == 1) {
      بلوک_flags |= flags_انتها;
    }
    blake3_compress_درون_قراربده_portable(cv, درون_داد_2, BLAKE3_بلوک_LEN, counter,
                                      بلوک_flags);
    درون_داد_2 = &درون_داد_2[BLAKE3_بلوک_LEN];
    بلوکها -= 1;
    بلوک_flags = flags;
  }
  store_cv_واژگان(بیرون_2, cv);
}

فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t رقم_inputs,
                               size_t بلوکها, ثابت uint32_t کلید_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_شروع,
                               uint8_t flags_انتها, uint8_t *بیرون_2) {
  طالما (رقم_inputs > 0) {
    hash_one_portable(inputs[0], بلوکها, کلید_2, counter, flags, flags_شروع,
                      flags_انتها, بیرون_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    رقم_inputs -= 1;
    بیرون_2 = &بیرون_2[BLAKE3_بیرون_LEN];
  }
}
