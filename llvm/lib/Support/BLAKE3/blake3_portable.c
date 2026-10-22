#அந்தர்பாவ "blake3_impl.h"
#அந்தர்பாவ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  திரும்பு (w >> c) | (w << (32 - c));
}

INLINE வெறுமை g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE வெறுமை round_fn(uint32_t state[16], மாறிலி uint32_t *செய்தி, size_t round) {
  // Select the message schedule based on the round.
  மாறிலி uint8_t *schedule = செய்தி_SCHEDULE[round];

  // Mix the columns.
  g(state, 0, 4, 8, 12, செய்தி[schedule[0]], செய்தி[schedule[1]]);
  g(state, 1, 5, 9, 13, செய்தி[schedule[2]], செய்தி[schedule[3]]);
  g(state, 2, 6, 10, 14, செய்தி[schedule[4]], செய்தி[schedule[5]]);
  g(state, 3, 7, 11, 15, செய்தி[schedule[6]], செய்தி[schedule[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, செய்தி[schedule[8]], செய்தி[schedule[9]]);
  g(state, 1, 6, 11, 12, செய்தி[schedule[10]], செய்தி[schedule[11]]);
  g(state, 2, 7, 8, 13, செய்தி[schedule[12]], செய்தி[schedule[13]]);
  g(state, 3, 4, 9, 14, செய்தி[schedule[14]], செய்தி[schedule[15]]);
}

INLINE வெறுமை compress_pre(uint32_t state[16], மாறிலி uint32_t cv[8],
                         மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags) {
  uint32_t block_சொற்கள்[16];
  block_சொற்கள்[0] = load32(block + 4 * 0);
  block_சொற்கள்[1] = load32(block + 4 * 1);
  block_சொற்கள்[2] = load32(block + 4 * 2);
  block_சொற்கள்[3] = load32(block + 4 * 3);
  block_சொற்கள்[4] = load32(block + 4 * 4);
  block_சொற்கள்[5] = load32(block + 4 * 5);
  block_சொற்கள்[6] = load32(block + 4 * 6);
  block_சொற்கள்[7] = load32(block + 4 * 7);
  block_சொற்கள்[8] = load32(block + 4 * 8);
  block_சொற்கள்[9] = load32(block + 4 * 9);
  block_சொற்கள்[10] = load32(block + 4 * 10);
  block_சொற்கள்[11] = load32(block + 4 * 11);
  block_சொற்கள்[12] = load32(block + 4 * 12);
  block_சொற்கள்[13] = load32(block + 4 * 13);
  block_சொற்கள்[14] = load32(block + 4 * 14);
  block_சொற்கள்[15] = load32(block + 4 * 15);

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
  state[12] = counter_கீழ்(counter);
  state[13] = counter_உயர்வு(counter);
  state[14] = (uint32_t)block_len;
  state[15] = (uint32_t)flags;

  round_fn(state, &block_சொற்கள்[0], 0);
  round_fn(state, &block_சொற்கள்[0], 1);
  round_fn(state, &block_சொற்கள்[0], 2);
  round_fn(state, &block_சொற்கள்[0], 3);
  round_fn(state, &block_சொற்கள்[0], 4);
  round_fn(state, &block_சொற்கள்[0], 5);
  round_fn(state, &block_சொற்கள்[0], 6);
}

வெறுமை blake3_compress_in_place_portable(uint32_t cv[8],
                                       மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
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

வெறுமை blake3_compress_xof_portable(மாறிலி uint32_t cv[8],
                                  மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
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

INLINE வெறுமை hash_one_portable(மாறிலி uint8_t *உள்ளீடு_2, size_t blocks,
                              மாறிலி uint32_t சாவி_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, சாவி_2, BLAKE3_சாவி_LEN);
  uint8_t block_flags = flags | flags_start;
  போது (blocks > 0) {
    எனில் (blocks == 1) {
      block_flags |= flags_end;
    }
    blake3_compress_in_place_portable(cv, உள்ளீடு_2, BLAKE3_BLOCK_LEN, counter,
                                      block_flags);
    உள்ளீடு_2 = &உள்ளீடு_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  store_cv_சொற்கள்(out, cv);
}

வெறுமை blake3_hash_many_portable(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                               size_t blocks, மாறிலி uint32_t சாவி_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out) {
  போது (இலக்கம்_inputs > 0) {
    hash_one_portable(inputs[0], blocks, சாவி_2, counter, flags, flags_start,
                      flags_end, out);
    எனில் (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    இலக்கம்_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}
