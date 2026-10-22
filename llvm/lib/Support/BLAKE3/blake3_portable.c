#ਅਨ੍ਤਰ੍ਭਾਵ "blake3_impl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  ਵਾਪਸ (w >> c) | (w << (32 - c));
}

INLINE ਖਾਲੀ g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE ਖਾਲੀ ਗੇੜਾ_fn(uint32_t state[16], ਅਚਲ uint32_t *ਸੁਨੇਹਾ, size_t ਗੇੜਾ) {
  // Select the message schedule based on the round.
  ਅਚਲ uint8_t *ਸਮਾਂ_ਸਾਰਣੀ = ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[ਗੇੜਾ];

  // Mix the columns.
  g(state, 0, 4, 8, 12, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[0]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[1]]);
  g(state, 1, 5, 9, 13, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[2]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[3]]);
  g(state, 2, 6, 10, 14, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[4]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[5]]);
  g(state, 3, 7, 11, 15, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[6]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[8]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[9]]);
  g(state, 1, 6, 11, 12, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[10]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[11]]);
  g(state, 2, 7, 8, 13, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[12]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[13]]);
  g(state, 3, 4, 9, 14, ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[14]], ਸੁਨੇਹਾ[ਸਮਾਂ_ਸਾਰਣੀ[15]]);
}

INLINE ਖਾਲੀ compress_pre(uint32_t state[16], ਅਚਲ uint32_t cv[8],
                         ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                         uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags) {
  uint32_t ਖੰਡ_ਸ਼ਬਦ[16];
  ਖੰਡ_ਸ਼ਬਦ[0] = load32(ਖੰਡ + 4 * 0);
  ਖੰਡ_ਸ਼ਬਦ[1] = load32(ਖੰਡ + 4 * 1);
  ਖੰਡ_ਸ਼ਬਦ[2] = load32(ਖੰਡ + 4 * 2);
  ਖੰਡ_ਸ਼ਬਦ[3] = load32(ਖੰਡ + 4 * 3);
  ਖੰਡ_ਸ਼ਬਦ[4] = load32(ਖੰਡ + 4 * 4);
  ਖੰਡ_ਸ਼ਬਦ[5] = load32(ਖੰਡ + 4 * 5);
  ਖੰਡ_ਸ਼ਬਦ[6] = load32(ਖੰਡ + 4 * 6);
  ਖੰਡ_ਸ਼ਬਦ[7] = load32(ਖੰਡ + 4 * 7);
  ਖੰਡ_ਸ਼ਬਦ[8] = load32(ਖੰਡ + 4 * 8);
  ਖੰਡ_ਸ਼ਬਦ[9] = load32(ਖੰਡ + 4 * 9);
  ਖੰਡ_ਸ਼ਬਦ[10] = load32(ਖੰਡ + 4 * 10);
  ਖੰਡ_ਸ਼ਬਦ[11] = load32(ਖੰਡ + 4 * 11);
  ਖੰਡ_ਸ਼ਬਦ[12] = load32(ਖੰਡ + 4 * 12);
  ਖੰਡ_ਸ਼ਬਦ[13] = load32(ਖੰਡ + 4 * 13);
  ਖੰਡ_ਸ਼ਬਦ[14] = load32(ਖੰਡ + 4 * 14);
  ਖੰਡ_ਸ਼ਬਦ[15] = load32(ਖੰਡ + 4 * 15);

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
  state[12] = counter_ਨੀਵਾਂ(counter);
  state[13] = counter_ਉੱਚਾ(counter);
  state[14] = (uint32_t)ਖੰਡ_len;
  state[15] = (uint32_t)flags;

  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 0);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 1);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 2);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 3);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 4);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 5);
  ਗੇੜਾ_fn(state, &ਖੰਡ_ਸ਼ਬਦ[0], 6);
}

ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(uint32_t cv[8],
                                       ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                       uint8_t ਖੰਡ_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

ਖਾਲੀ blake3_compress_xof_portable(ਅਚਲ uint32_t cv[8],
                                  ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                  uint8_t ਖੰਡ_len, uint64_t counter,
                                  uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, ਖੰਡ, ਖੰਡ_len, counter, flags);

  store32(&ਬਾਹਰਲਾ_2[0 * 4], state[0] ^ state[8]);
  store32(&ਬਾਹਰਲਾ_2[1 * 4], state[1] ^ state[9]);
  store32(&ਬਾਹਰਲਾ_2[2 * 4], state[2] ^ state[10]);
  store32(&ਬਾਹਰਲਾ_2[3 * 4], state[3] ^ state[11]);
  store32(&ਬਾਹਰਲਾ_2[4 * 4], state[4] ^ state[12]);
  store32(&ਬਾਹਰਲਾ_2[5 * 4], state[5] ^ state[13]);
  store32(&ਬਾਹਰਲਾ_2[6 * 4], state[6] ^ state[14]);
  store32(&ਬਾਹਰਲਾ_2[7 * 4], state[7] ^ state[15]);
  store32(&ਬਾਹਰਲਾ_2[8 * 4], state[8] ^ cv[0]);
  store32(&ਬਾਹਰਲਾ_2[9 * 4], state[9] ^ cv[1]);
  store32(&ਬਾਹਰਲਾ_2[10 * 4], state[10] ^ cv[2]);
  store32(&ਬਾਹਰਲਾ_2[11 * 4], state[11] ^ cv[3]);
  store32(&ਬਾਹਰਲਾ_2[12 * 4], state[12] ^ cv[4]);
  store32(&ਬਾਹਰਲਾ_2[13 * 4], state[13] ^ cv[5]);
  store32(&ਬਾਹਰਲਾ_2[14 * 4], state[14] ^ cv[6]);
  store32(&ਬਾਹਰਲਾ_2[15 * 4], state[15] ^ cv[7]);
}

INLINE ਖਾਲੀ hash_one_portable(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਖੰਡਾਂ,
                              ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                              uint8_t flags_ਸਿਰਾ, uint8_t ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN]) {
  uint32_t cv[8];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(cv, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  uint8_t ਖੰਡ_flags = flags | flags_ਸ਼ੁਰੂ;
  ਜਦੋਂ (ਖੰਡਾਂ > 0) {
    ਜੇ (ਖੰਡਾਂ == 1) {
      ਖੰਡ_flags |= flags_ਸਿਰਾ;
    }
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(cv, ਨਿਵੇਸ਼_2, BLAKE3_ਖੰਡ_LEN, counter,
                                      ਖੰਡ_flags);
    ਨਿਵੇਸ਼_2 = &ਨਿਵੇਸ਼_2[BLAKE3_ਖੰਡ_LEN];
    ਖੰਡਾਂ -= 1;
    ਖੰਡ_flags = flags;
  }
  store_cv_ਸ਼ਬਦ(ਬਾਹਰਲਾ_2, cv);
}

ਖਾਲੀ blake3_hash_many_portable(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                               size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                               uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2) {
  ਜਦੋਂ (ਅੰਕ_inputs > 0) {
    hash_one_portable(inputs[0], ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter, flags, flags_ਸ਼ੁਰੂ,
                      flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
    ਜੇ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ਅੰਕ_inputs -= 1;
    ਬਾਹਰਲਾ_2 = &ਬਾਹਰਲਾ_2[BLAKE3_ਬਾਹਰਲਾ_LEN];
  }
}
