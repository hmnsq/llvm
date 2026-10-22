#ಅನ್ತರ್ಭಾವ "blake3_impl.h"
#ಅನ್ತರ್ಭಾವ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  ಹಿಂತಿರುಗಿ (w >> c) | (w << (32 - c));
}

INLINE ಶೂನ್ಯ g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE ಶೂನ್ಯ ಸುತ್ತು_fn(uint32_t state[16], ಸ್ಥಿರಾಂಕ uint32_t *ಸಂದೇಶ, size_t ಸುತ್ತು) {
  // Select the message schedule based on the round.
  ಸ್ಥಿರಾಂಕ uint8_t *ವೇಳಾಪಟ್ಟಿ = ಸಂದೇಶ_ವೇಳಾಪಟ್ಟಿ[ಸುತ್ತು];

  // Mix the columns.
  g(state, 0, 4, 8, 12, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[0]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[1]]);
  g(state, 1, 5, 9, 13, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[2]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[3]]);
  g(state, 2, 6, 10, 14, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[4]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[5]]);
  g(state, 3, 7, 11, 15, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[6]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[8]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[9]]);
  g(state, 1, 6, 11, 12, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[10]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[11]]);
  g(state, 2, 7, 8, 13, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[12]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[13]]);
  g(state, 3, 4, 9, 14, ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[14]], ಸಂದೇಶ[ವೇಳಾಪಟ್ಟಿ[15]]);
}

INLINE ಶೂನ್ಯ compress_pre(uint32_t state[16], ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                         ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                         uint8_t ಖಂಡ_len, uint64_t counter, uint8_t flags) {
  uint32_t ಖಂಡ_ಪದಗಳು[16];
  ಖಂಡ_ಪದಗಳು[0] = load32(ಖಂಡ + 4 * 0);
  ಖಂಡ_ಪದಗಳು[1] = load32(ಖಂಡ + 4 * 1);
  ಖಂಡ_ಪದಗಳು[2] = load32(ಖಂಡ + 4 * 2);
  ಖಂಡ_ಪದಗಳು[3] = load32(ಖಂಡ + 4 * 3);
  ಖಂಡ_ಪದಗಳು[4] = load32(ಖಂಡ + 4 * 4);
  ಖಂಡ_ಪದಗಳು[5] = load32(ಖಂಡ + 4 * 5);
  ಖಂಡ_ಪದಗಳು[6] = load32(ಖಂಡ + 4 * 6);
  ಖಂಡ_ಪದಗಳು[7] = load32(ಖಂಡ + 4 * 7);
  ಖಂಡ_ಪದಗಳು[8] = load32(ಖಂಡ + 4 * 8);
  ಖಂಡ_ಪದಗಳು[9] = load32(ಖಂಡ + 4 * 9);
  ಖಂಡ_ಪದಗಳು[10] = load32(ಖಂಡ + 4 * 10);
  ಖಂಡ_ಪದಗಳು[11] = load32(ಖಂಡ + 4 * 11);
  ಖಂಡ_ಪದಗಳು[12] = load32(ಖಂಡ + 4 * 12);
  ಖಂಡ_ಪದಗಳು[13] = load32(ಖಂಡ + 4 * 13);
  ಖಂಡ_ಪದಗಳು[14] = load32(ಖಂಡ + 4 * 14);
  ಖಂಡ_ಪದಗಳು[15] = load32(ಖಂಡ + 4 * 15);

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
  state[12] = counter_ಕೆಳಗಿನ(counter);
  state[13] = counter_ಮೇಲಿನ(counter);
  state[14] = (uint32_t)ಖಂಡ_len;
  state[15] = (uint32_t)flags;

  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 0);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 1);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 2);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 3);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 4);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 5);
  ಸುತ್ತು_fn(state, &ಖಂಡ_ಪದಗಳು[0], 6);
}

ಶೂನ್ಯ blake3_compress_ಒಳ_ಇರಿಸು_portable(uint32_t cv[8],
                                       ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                                       uint8_t ಖಂಡ_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

ಶೂನ್ಯ blake3_compress_xof_portable(ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                                  ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                                  uint8_t ಖಂಡ_len, uint64_t counter,
                                  uint8_t flags, uint8_t ಹೊರಹೋಗು_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, ಖಂಡ, ಖಂಡ_len, counter, flags);

  store32(&ಹೊರಹೋಗು_2[0 * 4], state[0] ^ state[8]);
  store32(&ಹೊರಹೋಗು_2[1 * 4], state[1] ^ state[9]);
  store32(&ಹೊರಹೋಗು_2[2 * 4], state[2] ^ state[10]);
  store32(&ಹೊರಹೋಗು_2[3 * 4], state[3] ^ state[11]);
  store32(&ಹೊರಹೋಗು_2[4 * 4], state[4] ^ state[12]);
  store32(&ಹೊರಹೋಗು_2[5 * 4], state[5] ^ state[13]);
  store32(&ಹೊರಹೋಗು_2[6 * 4], state[6] ^ state[14]);
  store32(&ಹೊರಹೋಗು_2[7 * 4], state[7] ^ state[15]);
  store32(&ಹೊರಹೋಗು_2[8 * 4], state[8] ^ cv[0]);
  store32(&ಹೊರಹೋಗು_2[9 * 4], state[9] ^ cv[1]);
  store32(&ಹೊರಹೋಗು_2[10 * 4], state[10] ^ cv[2]);
  store32(&ಹೊರಹೋಗು_2[11 * 4], state[11] ^ cv[3]);
  store32(&ಹೊರಹೋಗು_2[12 * 4], state[12] ^ cv[4]);
  store32(&ಹೊರಹೋಗು_2[13 * 4], state[13] ^ cv[5]);
  store32(&ಹೊರಹೋಗು_2[14 * 4], state[14] ^ cv[6]);
  store32(&ಹೊರಹೋಗು_2[15 * 4], state[15] ^ cv[7]);
}

INLINE ಶೂನ್ಯ hash_one_portable(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಖಂಡಗಳು,
                              ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_ಆರಂಭ,
                              uint8_t flags_ಕೊನೆ, uint8_t ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN]) {
  uint32_t cv[8];
  ಸ್ಮರಣೆ_ನಕಲು(cv, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  uint8_t ಖಂಡ_flags = flags | flags_ಆರಂಭ;
  ಆಗ (ಖಂಡಗಳು > 0) {
    ಆದರೆ (ಖಂಡಗಳು == 1) {
      ಖಂಡ_flags |= flags_ಕೊನೆ;
    }
    blake3_compress_ಒಳ_ಇರಿಸು_portable(cv, ಒಳಹರಿವು_2, BLAKE3_ಖಂಡ_LEN, counter,
                                      ಖಂಡ_flags);
    ಒಳಹರಿವು_2 = &ಒಳಹರಿವು_2[BLAKE3_ಖಂಡ_LEN];
    ಖಂಡಗಳು -= 1;
    ಖಂಡ_flags = flags;
  }
  store_cv_ಪದಗಳು(ಹೊರಹೋಗು_2, cv);
}

ಶೂನ್ಯ blake3_hash_many_portable(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                               size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ಆರಂಭ,
                               uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
  ಆಗ (ಅಂಕೆ_inputs > 0) {
    hash_one_portable(inputs[0], ಖಂಡಗಳು, ಕೀಲಿ_2, counter, flags, flags_ಆರಂಭ,
                      flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
    ಆದರೆ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    ಅಂಕೆ_inputs -= 1;
    ಹೊರಹೋಗು_2 = &ಹೊರಹೋಗು_2[BLAKE3_ಹೊರಹೋಗು_LEN];
  }
}
