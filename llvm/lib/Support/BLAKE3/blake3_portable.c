#अन्तर्भाव "blake3_impl.h"
#अन्तर्भाव <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  प्रत्यागम (w >> c) | (w << (32 - c));
}

INLINE शून्य g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE शून्य आवर्तनम्_fn(uint32_t state[16], ध्रुव uint32_t *सन्देशः, size_t आवर्तनम्) {
  // Select the message schedule based on the round.
  ध्रुव uint8_t *कालसारणी = सन्देशः_कालसारणी[आवर्तनम्];

  // Mix the columns.
  g(state, 0, 4, 8, 12, सन्देशः[कालसारणी[0]], सन्देशः[कालसारणी[1]]);
  g(state, 1, 5, 9, 13, सन्देशः[कालसारणी[2]], सन्देशः[कालसारणी[3]]);
  g(state, 2, 6, 10, 14, सन्देशः[कालसारणी[4]], सन्देशः[कालसारणी[5]]);
  g(state, 3, 7, 11, 15, सन्देशः[कालसारणी[6]], सन्देशः[कालसारणी[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, सन्देशः[कालसारणी[8]], सन्देशः[कालसारणी[9]]);
  g(state, 1, 6, 11, 12, सन्देशः[कालसारणी[10]], सन्देशः[कालसारणी[11]]);
  g(state, 2, 7, 8, 13, सन्देशः[कालसारणी[12]], सन्देशः[कालसारणी[13]]);
  g(state, 3, 4, 9, 14, सन्देशः[कालसारणी[14]], सन्देशः[कालसारणी[15]]);
}

INLINE शून्य compress_pre(uint32_t state[16], ध्रुव uint32_t cv[8],
                         ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                         uint8_t खण्डम्_len, uint64_t गणक_2, uint8_t flags) {
  uint32_t खण्डम्_शब्दाः[16];
  खण्डम्_शब्दाः[0] = load32(खण्डम् + 4 * 0);
  खण्डम्_शब्दाः[1] = load32(खण्डम् + 4 * 1);
  खण्डम्_शब्दाः[2] = load32(खण्डम् + 4 * 2);
  खण्डम्_शब्दाः[3] = load32(खण्डम् + 4 * 3);
  खण्डम्_शब्दाः[4] = load32(खण्डम् + 4 * 4);
  खण्डम्_शब्दाः[5] = load32(खण्डम् + 4 * 5);
  खण्डम्_शब्दाः[6] = load32(खण्डम् + 4 * 6);
  खण्डम्_शब्दाः[7] = load32(खण्डम् + 4 * 7);
  खण्डम्_शब्दाः[8] = load32(खण्डम् + 4 * 8);
  खण्डम्_शब्दाः[9] = load32(खण्डम् + 4 * 9);
  खण्डम्_शब्दाः[10] = load32(खण्डम् + 4 * 10);
  खण्डम्_शब्दाः[11] = load32(खण्डम् + 4 * 11);
  खण्डम्_शब्दाः[12] = load32(खण्डम् + 4 * 12);
  खण्डम्_शब्दाः[13] = load32(खण्डम् + 4 * 13);
  खण्डम्_शब्दाः[14] = load32(खण्डम् + 4 * 14);
  खण्डम्_शब्दाः[15] = load32(खण्डम् + 4 * 15);

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
  state[12] = गणक_नीचः(गणक_2);
  state[13] = गणक_उच्चः(गणक_2);
  state[14] = (uint32_t)खण्डम्_len;
  state[15] = (uint32_t)flags;

  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 0);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 1);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 2);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 3);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 4);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 5);
  आवर्तनम्_fn(state, &खण्डम्_शब्दाः[0], 6);
}

शून्य blake3_compress_अन्तर्_place_portable(uint32_t cv[8],
                                       ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                                       uint8_t खण्डम्_len, uint64_t गणक_2,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, खण्डम्, खण्डम्_len, गणक_2, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

शून्य blake3_compress_xof_portable(ध्रुव uint32_t cv[8],
                                  ध्रुव uint8_t खण्डम्[BLAKE3_खण्डम्_LEN],
                                  uint8_t खण्डम्_len, uint64_t गणक_2,
                                  uint8_t flags, uint8_t बहिर्गमनम्_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, खण्डम्, खण्डम्_len, गणक_2, flags);

  store32(&बहिर्गमनम्_2[0 * 4], state[0] ^ state[8]);
  store32(&बहिर्गमनम्_2[1 * 4], state[1] ^ state[9]);
  store32(&बहिर्गमनम्_2[2 * 4], state[2] ^ state[10]);
  store32(&बहिर्गमनम्_2[3 * 4], state[3] ^ state[11]);
  store32(&बहिर्गमनम्_2[4 * 4], state[4] ^ state[12]);
  store32(&बहिर्गमनम्_2[5 * 4], state[5] ^ state[13]);
  store32(&बहिर्गमनम्_2[6 * 4], state[6] ^ state[14]);
  store32(&बहिर्गमनम्_2[7 * 4], state[7] ^ state[15]);
  store32(&बहिर्गमनम्_2[8 * 4], state[8] ^ cv[0]);
  store32(&बहिर्गमनम्_2[9 * 4], state[9] ^ cv[1]);
  store32(&बहिर्गमनम्_2[10 * 4], state[10] ^ cv[2]);
  store32(&बहिर्गमनम्_2[11 * 4], state[11] ^ cv[3]);
  store32(&बहिर्गमनम्_2[12 * 4], state[12] ^ cv[4]);
  store32(&बहिर्गमनम्_2[13 * 4], state[13] ^ cv[5]);
  store32(&बहिर्गमनम्_2[14 * 4], state[14] ^ cv[6]);
  store32(&बहिर्गमनम्_2[15 * 4], state[15] ^ cv[7]);
}

INLINE शून्य hash_one_portable(ध्रुव uint8_t *निवेशः_2, size_t खण्डानि,
                              ध्रुव uint32_t कुञ्चिका_2[8], uint64_t गणक_2,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_अन्तः, uint8_t बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN]) {
  uint32_t cv[8];
  स्मृतिं_प्रतिलिख(cv, कुञ्चिका_2, BLAKE3_कुञ्चिका_LEN);
  uint8_t खण्डम्_flags = flags | flags_start;
  यावत् (खण्डानि > 0) {
    यदि (खण्डानि == 1) {
      खण्डम्_flags |= flags_अन्तः;
    }
    blake3_compress_अन्तर्_place_portable(cv, निवेशः_2, BLAKE3_खण्डम्_LEN, गणक_2,
                                      खण्डम्_flags);
    निवेशः_2 = &निवेशः_2[BLAKE3_खण्डम्_LEN];
    खण्डानि -= 1;
    खण्डम्_flags = flags;
  }
  store_cv_शब्दाः(बहिर्गमनम्_2, cv);
}

शून्य blake3_hash_many_portable(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                               size_t खण्डानि, ध्रुव uint32_t कुञ्चिका_2[8],
                               uint64_t गणक_2, bool increment_गणक,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_अन्तः, uint8_t *बहिर्गमनम्_2) {
  यावत् (अङ्कः_inputs > 0) {
    hash_one_portable(inputs[0], खण्डानि, कुञ्चिका_2, गणक_2, flags, flags_start,
                      flags_अन्तः, बहिर्गमनम्_2);
    यदि (increment_गणक) {
      गणक_2 += 1;
    }
    inputs += 1;
    अङ्कः_inputs -= 1;
    बहिर्गमनम्_2 = &बहिर्गमनम्_2[BLAKE3_बहिर्गमनम्_LEN];
  }
}
