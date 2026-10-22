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

INLINE فراغ round_fn(uint32_t state[16], ثابت uint32_t *پەیام, size_t round) {
  // Select the message schedule based on the round.
  ثابت uint8_t *schedule = پەیام_SCHEDULE[round];

  // Mix the columns.
  g(state, 0, 4, 8, 12, پەیام[schedule[0]], پەیام[schedule[1]]);
  g(state, 1, 5, 9, 13, پەیام[schedule[2]], پەیام[schedule[3]]);
  g(state, 2, 6, 10, 14, پەیام[schedule[4]], پەیام[schedule[5]]);
  g(state, 3, 7, 11, 15, پەیام[schedule[6]], پەیام[schedule[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, پەیام[schedule[8]], پەیام[schedule[9]]);
  g(state, 1, 6, 11, 12, پەیام[schedule[10]], پەیام[schedule[11]]);
  g(state, 2, 7, 8, 13, پەیام[schedule[12]], پەیام[schedule[13]]);
  g(state, 3, 4, 9, 14, پەیام[schedule[14]], پەیام[schedule[15]]);
}

INLINE فراغ compress_pre(uint32_t state[16], ثابت uint32_t cv[8],
                         ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags) {
  uint32_t block_وشەکان[16];
  block_وشەکان[0] = load32(block + 4 * 0);
  block_وشەکان[1] = load32(block + 4 * 1);
  block_وشەکان[2] = load32(block + 4 * 2);
  block_وشەکان[3] = load32(block + 4 * 3);
  block_وشەکان[4] = load32(block + 4 * 4);
  block_وشەکان[5] = load32(block + 4 * 5);
  block_وشەکان[6] = load32(block + 4 * 6);
  block_وشەکان[7] = load32(block + 4 * 7);
  block_وشەکان[8] = load32(block + 4 * 8);
  block_وشەکان[9] = load32(block + 4 * 9);
  block_وشەکان[10] = load32(block + 4 * 10);
  block_وشەکان[11] = load32(block + 4 * 11);
  block_وشەکان[12] = load32(block + 4 * 12);
  block_وشەکان[13] = load32(block + 4 * 13);
  block_وشەکان[14] = load32(block + 4 * 14);
  block_وشەکان[15] = load32(block + 4 * 15);

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
  state[12] = counter_نزم(counter);
  state[13] = counter_بەرز(counter);
  state[14] = (uint32_t)block_len;
  state[15] = (uint32_t)flags;

  round_fn(state, &block_وشەکان[0], 0);
  round_fn(state, &block_وشەکان[0], 1);
  round_fn(state, &block_وشەکان[0], 2);
  round_fn(state, &block_وشەکان[0], 3);
  round_fn(state, &block_وشەکان[0], 4);
  round_fn(state, &block_وشەکان[0], 5);
  round_fn(state, &block_وشەکان[0], 6);
}

فراغ blake3_compress_in_place_portable(uint32_t cv[8],
                                       ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, block, block_len, counter, flags);
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
                                  ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]) {
  uint32_t state[16];
  compress_pre(state, cv, block, block_len, counter, flags);

  store32(&out[0 * 4], state[0] ^ state[8]);
  store32(&out[1 * 4], state[1] ^ state[9]);
  store32(&out[2 * 4], state[2] ^ state[10]);
  store32(&out[3 * 4], state[3] ^ state[11]);
  store32(&out[4 * 4], state[4] ^ state[12]);
  store32(&out[5 * 4], state[5] ^ state[13]);
  store32(&out[6 * 4], state[6] ^ state[14]);
  store32(&out[7 * 4], state[7] ^ state[15]);
  store32(&out[8 * 4], state[8] ^ cv[0]);
  store32(&out[9 * 4], state[9] ^ cv[1]);
  store32(&out[10 * 4], state[10] ^ cv[2]);
  store32(&out[11 * 4], state[11] ^ cv[3]);
  store32(&out[12 * 4], state[12] ^ cv[4]);
  store32(&out[13 * 4], state[13] ^ cv[5]);
  store32(&out[14 * 4], state[14] ^ cv[6]);
  store32(&out[15 * 4], state[15] ^ cv[7]);
}

INLINE فراغ hash_one_portable(ثابت uint8_t *تێخستن_2, size_t blocks,
                              ثابت uint32_t کلیل_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, کلیل_2, BLAKE3_کلیل_LEN);
  uint8_t block_flags = flags | flags_start;
  طالما (blocks > 0) {
    إذا (blocks == 1) {
      block_flags |= flags_end;
    }
    blake3_compress_in_place_portable(cv, تێخستن_2, BLAKE3_BLOCK_LEN, counter,
                                      block_flags);
    تێخستن_2 = &تێخستن_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  store_cv_وشەکان(out, cv);
}

فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t ژمارە_inputs,
                               size_t blocks, ثابت uint32_t کلیل_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out) {
  طالما (ژمارە_inputs > 0) {
    hash_one_portable(inputs[0], blocks, کلیل_2, counter, flags, flags_start,
                      flags_end, out);
    إذا (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ژمارە_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}
