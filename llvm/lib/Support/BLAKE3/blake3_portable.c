#અન્તર્ભાવ "blake3_impl.h"
#અન્તર્ભાવ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  પાછા (w >> c) | (w << (32 - c));
}

INLINE શૂન્ય g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE શૂન્ય round_fn(uint32_t state[16], અચલ uint32_t *સંદેશ, size_t round) {
  // Select the message schedule based on the round.
  અચલ uint8_t *schedule = સંદેશ_SCHEDULE[round];

  // Mix the columns.
  g(state, 0, 4, 8, 12, સંદેશ[schedule[0]], સંદેશ[schedule[1]]);
  g(state, 1, 5, 9, 13, સંદેશ[schedule[2]], સંદેશ[schedule[3]]);
  g(state, 2, 6, 10, 14, સંદેશ[schedule[4]], સંદેશ[schedule[5]]);
  g(state, 3, 7, 11, 15, સંદેશ[schedule[6]], સંદેશ[schedule[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, સંદેશ[schedule[8]], સંદેશ[schedule[9]]);
  g(state, 1, 6, 11, 12, સંદેશ[schedule[10]], સંદેશ[schedule[11]]);
  g(state, 2, 7, 8, 13, સંદેશ[schedule[12]], સંદેશ[schedule[13]]);
  g(state, 3, 4, 9, 14, સંદેશ[schedule[14]], સંદેશ[schedule[15]]);
}

INLINE શૂન્ય compress_pre(uint32_t state[16], અચલ uint32_t cv[8],
                         અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags) {
  uint32_t block_શબ્દો[16];
  block_શબ્દો[0] = load32(block + 4 * 0);
  block_શબ્દો[1] = load32(block + 4 * 1);
  block_શબ્દો[2] = load32(block + 4 * 2);
  block_શબ્દો[3] = load32(block + 4 * 3);
  block_શબ્દો[4] = load32(block + 4 * 4);
  block_શબ્દો[5] = load32(block + 4 * 5);
  block_શબ્દો[6] = load32(block + 4 * 6);
  block_શબ્દો[7] = load32(block + 4 * 7);
  block_શબ્દો[8] = load32(block + 4 * 8);
  block_શબ્દો[9] = load32(block + 4 * 9);
  block_શબ્દો[10] = load32(block + 4 * 10);
  block_શબ્દો[11] = load32(block + 4 * 11);
  block_શબ્દો[12] = load32(block + 4 * 12);
  block_શબ્દો[13] = load32(block + 4 * 13);
  block_શબ્દો[14] = load32(block + 4 * 14);
  block_શબ્દો[15] = load32(block + 4 * 15);

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
  state[12] = counter_નીચલું(counter);
  state[13] = counter_ઉપલું(counter);
  state[14] = (uint32_t)block_len;
  state[15] = (uint32_t)flags;

  round_fn(state, &block_શબ્દો[0], 0);
  round_fn(state, &block_શબ્દો[0], 1);
  round_fn(state, &block_શબ્દો[0], 2);
  round_fn(state, &block_શબ્દો[0], 3);
  round_fn(state, &block_શબ્દો[0], 4);
  round_fn(state, &block_શબ્દો[0], 5);
  round_fn(state, &block_શબ્દો[0], 6);
}

શૂન્ય blake3_compress_in_place_portable(uint32_t cv[8],
                                       અચલ uint8_t block[BLAKE3_BLOCK_LEN],
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

શૂન્ય blake3_compress_xof_portable(અચલ uint32_t cv[8],
                                  અચલ uint8_t block[BLAKE3_BLOCK_LEN],
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

INLINE શૂન્ય hash_one_portable(અચલ uint8_t *આગત_2, size_t blocks,
                              અચલ uint32_t ચાવી_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN]) {
  uint32_t cv[8];
  memcpy(cv, ચાવી_2, BLAKE3_ચાવી_LEN);
  uint8_t block_flags = flags | flags_start;
  જ્યારે (blocks > 0) {
    જો (blocks == 1) {
      block_flags |= flags_end;
    }
    blake3_compress_in_place_portable(cv, આગત_2, BLAKE3_BLOCK_LEN, counter,
                                      block_flags);
    આગત_2 = &આગત_2[BLAKE3_BLOCK_LEN];
    blocks -= 1;
    block_flags = flags;
  }
  store_cv_શબ્દો(out, cv);
}

શૂન્ય blake3_hash_many_portable(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                               size_t blocks, અચલ uint32_t ચાવી_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out) {
  જ્યારે (અંક_inputs > 0) {
    hash_one_portable(inputs[0], blocks, ચાવી_2, counter, flags, flags_start,
                      flags_end, out);
    જો (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    અંક_inputs -= 1;
    out = &out[BLAKE3_OUT_LEN];
  }
}
