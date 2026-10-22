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

INLINE فراغ ڦيرو_fn(uint32_t state[16], ثابت uint32_t *نياپو, size_t ڦيرو) {
  // Select the message schedule based on the round.
  ثابت uint8_t *وقت_جدول = نياپو_وقت_جدول[ڦيرو];

  // Mix the columns.
  g(state, 0, 4, 8, 12, نياپو[وقت_جدول[0]], نياپو[وقت_جدول[1]]);
  g(state, 1, 5, 9, 13, نياپو[وقت_جدول[2]], نياپو[وقت_جدول[3]]);
  g(state, 2, 6, 10, 14, نياپو[وقت_جدول[4]], نياپو[وقت_جدول[5]]);
  g(state, 3, 7, 11, 15, نياپو[وقت_جدول[6]], نياپو[وقت_جدول[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, نياپو[وقت_جدول[8]], نياپو[وقت_جدول[9]]);
  g(state, 1, 6, 11, 12, نياپو[وقت_جدول[10]], نياپو[وقت_جدول[11]]);
  g(state, 2, 7, 8, 13, نياپو[وقت_جدول[12]], نياپو[وقت_جدول[13]]);
  g(state, 3, 4, 9, 14, نياپو[وقت_جدول[14]], نياپو[وقت_جدول[15]]);
}

INLINE فراغ compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                         uint8_t بلاڪ_len, uint64_t counter, uint8_t flags) {
  uint32_t بلاڪ_لفظ[16];
  بلاڪ_لفظ[0] = load32(بلاڪ + 4 * 0);
  بلاڪ_لفظ[1] = load32(بلاڪ + 4 * 1);
  بلاڪ_لفظ[2] = load32(بلاڪ + 4 * 2);
  بلاڪ_لفظ[3] = load32(بلاڪ + 4 * 3);
  بلاڪ_لفظ[4] = load32(بلاڪ + 4 * 4);
  بلاڪ_لفظ[5] = load32(بلاڪ + 4 * 5);
  بلاڪ_لفظ[6] = load32(بلاڪ + 4 * 6);
  بلاڪ_لفظ[7] = load32(بلاڪ + 4 * 7);
  بلاڪ_لفظ[8] = load32(بلاڪ + 4 * 8);
  بلاڪ_لفظ[9] = load32(بلاڪ + 4 * 9);
  بلاڪ_لفظ[10] = load32(بلاڪ + 4 * 10);
  بلاڪ_لفظ[11] = load32(بلاڪ + 4 * 11);
  بلاڪ_لفظ[12] = load32(بلاڪ + 4 * 12);
  بلاڪ_لفظ[13] = load32(بلاڪ + 4 * 13);
  بلاڪ_لفظ[14] = load32(بلاڪ + 4 * 14);
  بلاڪ_لفظ[15] = load32(بلاڪ + 4 * 15);

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
  state[12] = counter_هيٺيون(counter);
  state[13] = counter_مٿاهون(counter);
  state[14] = (uint32_t)بلاڪ_len;
  state[15] = (uint32_t)flags;

  ڦيرو_fn(state, &بلاڪ_لفظ[0], 0);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 1);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 2);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 3);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 4);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 5);
  ڦيرو_fn(state, &بلاڪ_لفظ[0], 6);
}

فراغ blake3_compress_اندريون_رکو_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                                       uint8_t بلاڪ_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, بلاڪ, بلاڪ_len, counter, flags);
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
                                  ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                                  uint8_t بلاڪ_len, uint64_t counter,
                                  uint8_t flags, uint8_t ٻاهريون_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, بلاڪ, بلاڪ_len, counter, flags);

  store32(&ٻاهريون_2[0 * 4], state[0] ^ state[8]);
  store32(&ٻاهريون_2[1 * 4], state[1] ^ state[9]);
  store32(&ٻاهريون_2[2 * 4], state[2] ^ state[10]);
  store32(&ٻاهريون_2[3 * 4], state[3] ^ state[11]);
  store32(&ٻاهريون_2[4 * 4], state[4] ^ state[12]);
  store32(&ٻاهريون_2[5 * 4], state[5] ^ state[13]);
  store32(&ٻاهريون_2[6 * 4], state[6] ^ state[14]);
  store32(&ٻاهريون_2[7 * 4], state[7] ^ state[15]);
  store32(&ٻاهريون_2[8 * 4], state[8] ^ cv[0]);
  store32(&ٻاهريون_2[9 * 4], state[9] ^ cv[1]);
  store32(&ٻاهريون_2[10 * 4], state[10] ^ cv[2]);
  store32(&ٻاهريون_2[11 * 4], state[11] ^ cv[3]);
  store32(&ٻاهريون_2[12 * 4], state[12] ^ cv[4]);
  store32(&ٻاهريون_2[13 * 4], state[13] ^ cv[5]);
  store32(&ٻاهريون_2[14 * 4], state[14] ^ cv[6]);
  store32(&ٻاهريون_2[15 * 4], state[15] ^ cv[7]);
}

INLINE فراغ hash_one_portable(ثابت uint8_t *داخلا_2, size_t بلاڪون,
                              ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_شروعات,
                              uint8_t flags_پڇاڙي, uint8_t ٻاهريون_2[BLAKE3_ٻاهريون_LEN]) {
  uint32_t cv[8];
  يادگيري_نقل(cv, ڪنجي_2, BLAKE3_ڪنجي_LEN);
  uint8_t بلاڪ_flags = flags | flags_شروعات;
  طالما (بلاڪون > 0) {
    إذا (بلاڪون == 1) {
      بلاڪ_flags |= flags_پڇاڙي;
    }
    blake3_compress_اندريون_رکو_portable(cv, داخلا_2, BLAKE3_بلاڪ_LEN, counter,
                                      بلاڪ_flags);
    داخلا_2 = &داخلا_2[BLAKE3_بلاڪ_LEN];
    بلاڪون -= 1;
    بلاڪ_flags = flags;
  }
  store_cv_لفظ(ٻاهريون_2, cv);
}

فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t انگ_inputs,
                               size_t بلاڪون, ثابت uint32_t ڪنجي_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_شروعات,
                               uint8_t flags_پڇاڙي, uint8_t *ٻاهريون_2) {
  طالما (انگ_inputs > 0) {
    hash_one_portable(inputs[0], بلاڪون, ڪنجي_2, counter, flags, flags_شروعات,
                      flags_پڇاڙي, ٻاهريون_2);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    انگ_inputs -= 1;
    ٻاهريون_2 = &ٻاهريون_2[BLAKE3_ٻاهريون_LEN];
  }
}
