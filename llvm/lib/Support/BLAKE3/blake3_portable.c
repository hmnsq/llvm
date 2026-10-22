#అన్తర్భావ "blake3_impl.h"
#అన్తర్భావ <string.h>

INLINE uint32_t rotr32(uint32_t w, uint32_t c) {
  తిరిగి (w >> c) | (w << (32 - c));
}

INLINE శూన్య g(uint32_t *state, size_t a, size_t b, size_t c, size_t d,
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

INLINE శూన్య విడత_fn(uint32_t state[16], స్థిరరాశి uint32_t *సందేశం, size_t విడత) {
  // Select the message schedule based on the round.
  స్థిరరాశి uint8_t *కాలపట్టిక = సందేశం_కాలపట్టిక[విడత];

  // Mix the columns.
  g(state, 0, 4, 8, 12, సందేశం[కాలపట్టిక[0]], సందేశం[కాలపట్టిక[1]]);
  g(state, 1, 5, 9, 13, సందేశం[కాలపట్టిక[2]], సందేశం[కాలపట్టిక[3]]);
  g(state, 2, 6, 10, 14, సందేశం[కాలపట్టిక[4]], సందేశం[కాలపట్టిక[5]]);
  g(state, 3, 7, 11, 15, సందేశం[కాలపట్టిక[6]], సందేశం[కాలపట్టిక[7]]);

  // Mix the rows.
  g(state, 0, 5, 10, 15, సందేశం[కాలపట్టిక[8]], సందేశం[కాలపట్టిక[9]]);
  g(state, 1, 6, 11, 12, సందేశం[కాలపట్టిక[10]], సందేశం[కాలపట్టిక[11]]);
  g(state, 2, 7, 8, 13, సందేశం[కాలపట్టిక[12]], సందేశం[కాలపట్టిక[13]]);
  g(state, 3, 4, 9, 14, సందేశం[కాలపట్టిక[14]], సందేశం[కాలపట్టిక[15]]);
}

INLINE శూన్య compress_pre(uint32_t state[16], స్థిరరాశి uint32_t cv[8],
                         స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                         uint8_t ఖండం_len, uint64_t counter, uint8_t flags) {
  uint32_t ఖండం_పదాలు[16];
  ఖండం_పదాలు[0] = load32(ఖండం + 4 * 0);
  ఖండం_పదాలు[1] = load32(ఖండం + 4 * 1);
  ఖండం_పదాలు[2] = load32(ఖండం + 4 * 2);
  ఖండం_పదాలు[3] = load32(ఖండం + 4 * 3);
  ఖండం_పదాలు[4] = load32(ఖండం + 4 * 4);
  ఖండం_పదాలు[5] = load32(ఖండం + 4 * 5);
  ఖండం_పదాలు[6] = load32(ఖండం + 4 * 6);
  ఖండం_పదాలు[7] = load32(ఖండం + 4 * 7);
  ఖండం_పదాలు[8] = load32(ఖండం + 4 * 8);
  ఖండం_పదాలు[9] = load32(ఖండం + 4 * 9);
  ఖండం_పదాలు[10] = load32(ఖండం + 4 * 10);
  ఖండం_పదాలు[11] = load32(ఖండం + 4 * 11);
  ఖండం_పదాలు[12] = load32(ఖండం + 4 * 12);
  ఖండం_పదాలు[13] = load32(ఖండం + 4 * 13);
  ఖండం_పదాలు[14] = load32(ఖండం + 4 * 14);
  ఖండం_పదాలు[15] = load32(ఖండం + 4 * 15);

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
  state[12] = counter_దిగువ(counter);
  state[13] = counter_ఎగువ(counter);
  state[14] = (uint32_t)ఖండం_len;
  state[15] = (uint32_t)flags;

  విడత_fn(state, &ఖండం_పదాలు[0], 0);
  విడత_fn(state, &ఖండం_పదాలు[0], 1);
  విడత_fn(state, &ఖండం_పదాలు[0], 2);
  విడత_fn(state, &ఖండం_పదాలు[0], 3);
  విడత_fn(state, &ఖండం_పదాలు[0], 4);
  విడత_fn(state, &ఖండం_పదాలు[0], 5);
  విడత_fn(state, &ఖండం_పదాలు[0], 6);
}

శూన్య blake3_compress_లోపల_ఉంచు_portable(uint32_t cv[8],
                                       స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                                       uint8_t ఖండం_len, uint64_t counter,
                                       uint8_t flags) {
  uint32_t state[16];
  compress_pre(state, cv, ఖండం, ఖండం_len, counter, flags);
  cv[0] = state[0] ^ state[8];
  cv[1] = state[1] ^ state[9];
  cv[2] = state[2] ^ state[10];
  cv[3] = state[3] ^ state[11];
  cv[4] = state[4] ^ state[12];
  cv[5] = state[5] ^ state[13];
  cv[6] = state[6] ^ state[14];
  cv[7] = state[7] ^ state[15];
}

శూన్య blake3_compress_xof_portable(స్థిరరాశి uint32_t cv[8],
                                  స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                                  uint8_t ఖండం_len, uint64_t counter,
                                  uint8_t flags, uint8_t బహిర్గతం_2[64]) {
  uint32_t state[16];
  compress_pre(state, cv, ఖండం, ఖండం_len, counter, flags);

  store32(&బహిర్గతం_2[0 * 4], state[0] ^ state[8]);
  store32(&బహిర్గతం_2[1 * 4], state[1] ^ state[9]);
  store32(&బహిర్గతం_2[2 * 4], state[2] ^ state[10]);
  store32(&బహిర్గతం_2[3 * 4], state[3] ^ state[11]);
  store32(&బహిర్గతం_2[4 * 4], state[4] ^ state[12]);
  store32(&బహిర్గతం_2[5 * 4], state[5] ^ state[13]);
  store32(&బహిర్గతం_2[6 * 4], state[6] ^ state[14]);
  store32(&బహిర్గతం_2[7 * 4], state[7] ^ state[15]);
  store32(&బహిర్గతం_2[8 * 4], state[8] ^ cv[0]);
  store32(&బహిర్గతం_2[9 * 4], state[9] ^ cv[1]);
  store32(&బహిర్గతం_2[10 * 4], state[10] ^ cv[2]);
  store32(&బహిర్గతం_2[11 * 4], state[11] ^ cv[3]);
  store32(&బహిర్గతం_2[12 * 4], state[12] ^ cv[4]);
  store32(&బహిర్గతం_2[13 * 4], state[13] ^ cv[5]);
  store32(&బహిర్గతం_2[14 * 4], state[14] ^ cv[6]);
  store32(&బహిర్గతం_2[15 * 4], state[15] ^ cv[7]);
}

INLINE శూన్య hash_one_portable(స్థిరరాశి uint8_t *ఆదానం_2, size_t ఖండాలు,
                              స్థిరరాశి uint32_t కీలకం_2[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_ప్రారంభం,
                              uint8_t flags_చివర, uint8_t బహిర్గతం_2[BLAKE3_బహిర్గతం_LEN]) {
  uint32_t cv[8];
  జ్ఞాపకం_నకలు(cv, కీలకం_2, BLAKE3_కీలకం_LEN);
  uint8_t ఖండం_flags = flags | flags_ప్రారంభం;
  అయితే (ఖండాలు > 0) {
    ఒకవేళ (ఖండాలు == 1) {
      ఖండం_flags |= flags_చివర;
    }
    blake3_compress_లోపల_ఉంచు_portable(cv, ఆదానం_2, BLAKE3_ఖండం_LEN, counter,
                                      ఖండం_flags);
    ఆదానం_2 = &ఆదానం_2[BLAKE3_ఖండం_LEN];
    ఖండాలు -= 1;
    ఖండం_flags = flags;
  }
  store_cv_పదాలు(బహిర్గతం_2, cv);
}

శూన్య blake3_hash_many_portable(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                               size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ప్రారంభం,
                               uint8_t flags_చివర, uint8_t *బహిర్గతం_2) {
  అయితే (అంకె_inputs > 0) {
    hash_one_portable(inputs[0], ఖండాలు, కీలకం_2, counter, flags, flags_ప్రారంభం,
                      flags_చివర, బహిర్గతం_2);
    ఒకవేళ (increment_counter) {
      counter += 1;
    }
    inputs += 1;
    అంకె_inputs -= 1;
    బహిర్గతం_2 = &బహిర్గతం_2[BLAKE3_బహిర్గతం_LEN];
  }
}
