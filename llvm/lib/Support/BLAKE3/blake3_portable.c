#ଅନ୍ତର୍ଭାଵ "blake3_impl.h"
#ଅନ୍ତର୍ଭାଵ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  ଫେରନ୍ତୁ (w >> c) | (w << (32 - c));
}

INLINE ଶୂନ୍ୟ g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE ଶୂନ୍ୟ round_fn(uint32_t state[16], ଅଚଳ uint32_t *ସନ୍ଦେଶ, size_t round) {
  // Select the message schedule based on the round.
  ଅଚଳ uint8_t *schedule = ସନ୍ଦେଶ_SCHEDULE[round];

  // Mix the columns.
  g(state, 0, 4, 8, 12, ସନ୍ଦେଶ[schedule[0]], ସନ୍ଦେଶ[schedule[1]]);
  g(state, 1, 5, 9, 13, ସନ୍ଦେଶ[schedule[2]], ସନ୍ଦେଶ[schedule[3]]);
  g(state, 2, 6, 10, 14, ସନ୍ଦେଶ[schedule[4]], ସନ୍ଦେଶ[schedule[5]]);
  g(state, 3, 7, 11, 15, ସନ୍ଦେଶ[schedule[6]], ସନ୍ଦେଶ[schedule[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, ସନ୍ଦେଶ[schedule[8]], ସନ୍ଦେଶ[schedule[9]]);
  g(state, 1, 6, 11, 12, ସନ୍ଦେଶ[schedule[10]], ସନ୍ଦେଶ[schedule[11]]);
  g(state, 2, 7, 8, 13, ସନ୍ଦେଶ[schedule[12]], ସନ୍ଦେଶ[schedule[13]]);
  g(state, 3, 4, 9, 14, ସନ୍ଦେଶ[schedule[14]], ସନ୍ଦେଶ[schedule[15]]);
}

INLINE ଶୂନ୍ୟ compress_pre(uint32_t state[16], ଅଚଳ uint32_t cv[8],
                         ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags) {
  uint32_t block_ଶବ୍ଦଗୁଡ଼ିକ[16];
  block_ଶବ୍ଦଗୁଡ଼ିକ[0] = load32(block + 4 * 0);
  block_ଶବ୍ଦଗୁଡ଼ିକ[1] = load32(block + 4 * 1);
  block_ଶବ୍ଦଗୁଡ଼ିକ[2] = load32(block + 4 * 2);
  block_ଶବ୍ଦଗୁଡ଼ିକ[3] = load32(block + 4 * 3);
  block_ଶବ୍ଦଗୁଡ଼ିକ[4] = load32(block + 4 * 4);
  block_ଶବ୍ଦଗୁଡ଼ିକ[5] = load32(block + 4 * 5);
  block_ଶବ୍ଦଗୁଡ଼ିକ[6] = load32(block + 4 * 6);
  block_ଶବ୍ଦଗୁଡ଼ିକ[7] = load32(block + 4 * 7);
  block_ଶବ୍ଦଗୁଡ଼ିକ[8] = load32(block + 4 * 8);
  block_ଶବ୍ଦଗୁଡ଼ିକ[9] = load32(block + 4 * 9);
  block_ଶବ୍ଦଗୁଡ଼ିକ[10] = load32(block + 4 * 10);
  block_ଶବ୍ଦଗୁଡ଼ିକ[11] = load32(block + 4 * 11);
  block_ଶବ୍ଦଗୁଡ଼ିକ[12] = load32(block + 4 * 12);
  block_ଶବ୍ଦଗୁଡ଼ିକ[13] = load32(block + 4 * 13);
  block_ଶବ୍ଦଗୁଡ଼ିକ[14] = load32(block + 4 * 14);
  block_ଶବ୍ଦଗୁଡ଼ିକ[15] = load32(block + 4 * 15);

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
  state[12] = counter_ତଳ(counter);
  state[13] = counter_ଉପର(counter);
  state[14] = (uint32_t)block_len;
  state[15] = (uint32_t)flags;

  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 0);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 1);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 2);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 3);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 4);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 5);
  round_fn(state, &block_ଶବ୍ଦଗୁଡ଼ିକ[0], 6);
}

ଶୂନ୍ୟ blake3_compress_in_place_portable(uint32_t cv[8],
                                       ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
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

ଶୂନ୍ୟ blake3_compress_xof_portable(ଅଚଳ uint32_t cv[8],
                                  ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
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

INLINE ଶୂନ୍ୟ hash_one_portable(ଅଚଳ uint8_t *ନିବେଶ_2, size_t blocks,
                              ଅଚଳ uint32_t ଚାବି_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, ଚାବି_2, BLAKE3_ଚାବି_LEN);
  uint8_t block_flags = flags | flags_start;
  ଯେତେବେଳେ (blocks > 0) {
    ଯଦି (blocks == 1) {
      block_flags |= flags_end;
    }
    blake3_compress_in_place_portable(cv, ନିବେଶ_2, BLAKE3_BLOCK_LEN, counter,
                                      block_flags);
    ନିବେଶ_2 = &ନିବେଶ_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  store_cv_ଶବ୍ଦଗୁଡ଼ିକ(out, cv);
}

ଶୂନ୍ୟ blake3_hash_many_portable(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                               size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out) {
  ଯେତେବେଳେ (ଅଙ୍କ_inputs > 0) {
    hash_one_portable(inputs[0], blocks, ଚାବି_2, counter, flags, flags_start,
                      flags_end, out);
    ଯଦି (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ଅଙ୍କ_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}
