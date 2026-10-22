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

INLINE فراغ گیڑا_fn(uint32_t state[16], ثابت uint32_t *پیغام, size_t گیڑا) {
  // Select the message schedule based on the round.
  ثابت uint8_t *ویلا_سارنی = پیغام_ویلا_سارنی[گیڑا];

  // Mix the columns.
  g(state, 0, 4, 8, 12, پیغام[ویلا_سارنی[0]], پیغام[ویلا_سارنی[1]]);
  g(state, 1, 5, 9, 13, پیغام[ویلا_سارنی[2]], پیغام[ویلا_سارنی[3]]);
  g(state, 2, 6, 10, 14, پیغام[ویلا_سارنی[4]], پیغام[ویلا_سارنی[5]]);
  g(state, 3, 7, 11, 15, پیغام[ویلا_سارنی[6]], پیغام[ویلا_سارنی[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, پیغام[ویلا_سارنی[8]], پیغام[ویلا_سارنی[9]]);
  g(state, 1, 6, 11, 12, پیغام[ویلا_سارنی[10]], پیغام[ویلا_سارنی[11]]);
  g(state, 2, 7, 8, 13, پیغام[ویلا_سارنی[12]], پیغام[ویلا_سارنی[13]]);
  g(state, 3, 4, 9, 14, پیغام[ویلا_سارنی[14]], پیغام[ویلا_سارنی[15]]);
}

INLINE فراغ compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t counter, uint8_t flags) {
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
  state[12] = counter_کم(counter);
  state[13] = counter_بلند(counter);
  state[14] = (uint32_t)بلاک_len;
  state[15] = (uint32_t)flags;

  گیڑا_fn(state, &بلاک_الفاظ[0], 0);
  گیڑا_fn(state, &بلاک_الفاظ[0], 1);
  گیڑا_fn(state, &بلاک_الفاظ[0], 2);
  گیڑا_fn(state, &بلاک_الفاظ[0], 3);
  گیڑا_fn(state, &بلاک_الفاظ[0], 4);
  گیڑا_fn(state, &بلاک_الفاظ[0], 5);
  گیڑا_fn(state, &بلاک_الفاظ[0], 6);
}

فراغ blake3_compress_وچ_رکھو_portable(uint32_t cv[8],
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
                                  uint8_t flags, uint8_t باہرلا_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, بلاک, بلاک_len, counter, flags);

  store32(&باہرلا_2[0 * 4], state[0] ^ state[8]);
  store32(&باہرلا_2[1 * 4], state[1] ^ state[9]);
  store32(&باہرلا_2[2 * 4], state[2] ^ state[10]);
  store32(&باہرلا_2[3 * 4], state[3] ^ state[11]);
  store32(&باہرلا_2[4 * 4], state[4] ^ state[12]);
  store32(&باہرلا_2[5 * 4], state[5] ^ state[13]);
  store32(&باہرلا_2[6 * 4], state[6] ^ state[14]);
  store32(&باہرلا_2[7 * 4], state[7] ^ state[15]);
  store32(&باہرلا_2[8 * 4], state[8] ^ cv[0]);
  store32(&باہرلا_2[9 * 4], state[9] ^ cv[1]);
  store32(&باہرلا_2[10 * 4], state[10] ^ cv[2]);
  store32(&باہرلا_2[11 * 4], state[11] ^ cv[3]);
  store32(&باہرلا_2[12 * 4], state[12] ^ cv[4]);
  store32(&باہرلا_2[13 * 4], state[13] ^ cv[5]);
  store32(&باہرلا_2[14 * 4], state[14] ^ cv[6]);
  store32(&باہرلا_2[15 * 4], state[15] ^ cv[7]);
}

INLINE فراغ hash_one_portable(ثابت uint8_t *داخلہ_2, size_t بلاکاں,
                              ثابت uint32_t کلید_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_شروع,
                              uint8_t flags_سرا, uint8_t باہرلا_2[BLAKE3_باہرلا_LEN]) {
  uint32_t cv[8];
  حافظہ_نقل(cv, کلید_2, BLAKE3_کلید_LEN);
  uint8_t بلاک_flags = flags | flags_شروع;
  طالما (بلاکاں > 0) {
    إذا (بلاکاں == 1) {
      بلاک_flags |= flags_سرا;
    }
    blake3_compress_وچ_رکھو_portable(cv, داخلہ_2, BLAKE3_بلاک_LEN, counter,
                                      بلاک_flags);
    داخلہ_2 = &داخلہ_2[BLAKE3_بلاک_LEN];
    بلاکاں -= 1;
    بلاک_flags = flags;
  }
  store_cv_الفاظ(باہرلا_2, cv);
}

فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                               size_t بلاکاں, ثابت uint32_t کلید_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_شروع,
                               uint8_t flags_سرا, uint8_t *باہرلا_2) {
  طالما (عدد_inputs > 0) {
    hash_one_portable(inputs[0], بلاکاں, کلید_2, counter, flags, flags_شروع,
                      flags_سرا, باہرلا_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    عدد_inputs -= 1;
    باہرلا_2 = &باہرلا_2[BLAKE3_باہرلا_LEN];
  }
}
