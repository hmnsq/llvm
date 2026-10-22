/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "blake3_impl.h"

స్థిరరాశి అక్షరం *llvm_blake3_సంస్కరణ(శూన్య) { తిరిగి BLAKE3_సంస్కరణ_STRING; }

INLINE శూన్య chunk_state_init(blake3_chunk_state *self, స్థిరరాశి uint32_t కీలకం_2[8],
                             uint8_t flags) {
  జ్ఞాపకం_నకలు(self->cv, కీలకం_2, BLAKE3_కీలకం_LEN);
  self->chunk_counter = 0;
  memset(self->buf, 0, BLAKE3_ఖండం_LEN);
  self->buf_len = 0;
  self->ఖండాలు_compressed = 0;
  self->flags = flags;
}

INLINE శూన్య chunk_state_reset(blake3_chunk_state *self, స్థిరరాశి uint32_t కీలకం_2[8],
                              uint64_t chunk_counter) {
  జ్ఞాపకం_నకలు(self->cv, కీలకం_2, BLAKE3_కీలకం_LEN);
  self->chunk_counter = chunk_counter;
  self->ఖండాలు_compressed = 0;
  memset(self->buf, 0, BLAKE3_ఖండం_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(స్థిరరాశి blake3_chunk_state *self) {
  తిరిగి (BLAKE3_ఖండం_LEN * (size_t)self->ఖండాలు_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   స్థిరరాశి uint8_t *ఆదానం_2, size_t ఆదానం_len) {
  size_t take = BLAKE3_ఖండం_LEN - ((size_t)self->buf_len);
  ఒకవేళ (take > ఆదానం_len) {
    take = ఆదానం_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  జ్ఞాపకం_నకలు(dest, ఆదానం_2, take);
  self->buf_len += (uint8_t)take;
  తిరిగి take;
}

INLINE uint8_t chunk_state_maybe_ప్రారంభం_flag(స్థిరరాశి blake3_chunk_state *self) {
  ఒకవేళ (self->ఖండాలు_compressed == 0) {
    తిరిగి CHUNK_ప్రారంభం;
  } లేకపోతే {
    తిరిగి 0;
  }
}

రకనిర్వచనం నిర్మాణం {
  uint32_t ఆదానం_cv[8];
  uint64_t counter;
  uint8_t ఖండం[BLAKE3_ఖండం_LEN];
  uint8_t ఖండం_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(స్థిరరాశి uint32_t ఆదానం_cv[8],
                            స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                            uint8_t ఖండం_len, uint64_t counter,
                            uint8_t flags) {
  output_t ret;
  జ్ఞాపకం_నకలు(ret.ఆదానం_cv, ఆదానం_cv, 32);
  జ్ఞాపకం_నకలు(ret.ఖండం, ఖండం, BLAKE3_ఖండం_LEN);
  ret.ఖండం_len = ఖండం_len;
  ret.counter = counter;
  ret.flags = flags;
  తిరిగి ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE శూన్య output_chaining_విలువ(స్థిరరాశి output_t *self, uint8_t cv[32]) {
  uint32_t cv_పదాలు[8];
  జ్ఞాపకం_నకలు(cv_పదాలు, self->ఆదానం_cv, 32);
  blake3_compress_లోపల_ఉంచు(cv_పదాలు, self->ఖండం, self->ఖండం_len,
                           self->counter, self->flags);
  store_cv_పదాలు(cv, cv_పదాలు);
}

INLINE శూన్య output_మూలం_బైట్లు(స్థిరరాశి output_t *self, uint64_t seek, uint8_t *బహిర్గతం_2,
                              size_t బహిర్గతం_len) {
  ఒకవేళ (బహిర్గతం_len == 0) {
      తిరిగి;
  }
  uint64_t output_ఖండం_counter = seek / 64;
  size_t స్థానభ్రంశం_within_ఖండం = seek % 64;
  uint8_t wide_buf[64];
  ఒకవేళ(స్థానభ్రంశం_within_ఖండం) {
    blake3_compress_xof(self->ఆదానం_cv, self->ఖండం, self->ఖండం_len, output_ఖండం_counter, self->flags | మూలం, wide_buf);
    స్థిరరాశి size_t అందుబాటు_బైట్లు = 64 - స్థానభ్రంశం_within_ఖండం;
    స్థిరరాశి size_t బైట్లు = బహిర్గతం_len > అందుబాటు_బైట్లు ? అందుబాటు_బైట్లు : బహిర్గతం_len;
    జ్ఞాపకం_నకలు(బహిర్గతం_2, wide_buf + స్థానభ్రంశం_within_ఖండం, బైట్లు);
    బహిర్గతం_2 += బైట్లు;
    బహిర్గతం_len -= బైట్లు;
    output_ఖండం_counter += 1;
  }
  ఒకవేళ(బహిర్గతం_len / 64) {
    blake3_xof_many(self->ఆదానం_cv, self->ఖండం, self->ఖండం_len, output_ఖండం_counter, self->flags | మూలం, బహిర్గతం_2, బహిర్గతం_len / 64);
  }
  output_ఖండం_counter += బహిర్గతం_len / 64;
  బహిర్గతం_2 += బహిర్గతం_len & -64;
  బహిర్గతం_len -= బహిర్గతం_len & -64;
  ఒకవేళ(బహిర్గతం_len) {
    blake3_compress_xof(self->ఆదానం_cv, self->ఖండం, self->ఖండం_len, output_ఖండం_counter, self->flags | మూలం, wide_buf);
    జ్ఞాపకం_నకలు(బహిర్గతం_2, wide_buf, బహిర్గతం_len);
  }
}

INLINE శూన్య chunk_state_update(blake3_chunk_state *self, స్థిరరాశి uint8_t *ఆదానం_2,
                               size_t ఆదానం_len) {
  ఒకవేళ (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, ఆదానం_2, ఆదానం_len);
    ఆదానం_2 += take;
    ఆదానం_len -= take;
    ఒకవేళ (ఆదానం_len > 0) {
      blake3_compress_లోపల_ఉంచు(
          self->cv, self->buf, BLAKE3_ఖండం_LEN, self->chunk_counter,
          self->flags | chunk_state_maybe_ప్రారంభం_flag(self));
      self->ఖండాలు_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_ఖండం_LEN);
    }
  }

  అయితే (ఆదానం_len > BLAKE3_ఖండం_LEN) {
    blake3_compress_లోపల_ఉంచు(self->cv, ఆదానం_2, BLAKE3_ఖండం_LEN,
                             self->chunk_counter,
                             self->flags | chunk_state_maybe_ప్రారంభం_flag(self));
    self->ఖండాలు_compressed += 1;
    ఆదానం_2 += BLAKE3_ఖండం_LEN;
    ఆదానం_len -= BLAKE3_ఖండం_LEN;
  }

  chunk_state_fill_buf(self, ఆదానం_2, ఆదానం_len);
}

INLINE output_t chunk_state_output(స్థిరరాశి blake3_chunk_state *self) {
  uint8_t ఖండం_flags =
      self->flags | chunk_state_maybe_ప్రారంభం_flag(self) | CHUNK_చివర;
  తిరిగి make_output(self->cv, self->buf, self->buf_len, self->chunk_counter,
                     ఖండం_flags);
}

INLINE output_t మాతృ_output(స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                              స్థిరరాశి uint32_t కీలకం_2[8], uint8_t flags) {
  తిరిగి make_output(కీలకం_2, ఖండం, BLAKE3_ఖండం_LEN, 0, flags | మాతృ);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t ఎడమ_subtree_len(size_t ఆదానం_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t నిండిన_chunks = (ఆదానం_len - 1) / BLAKE3_CHUNK_LEN;
  తిరిగి విడత_కిందికి_గమ్యం_power_యొక్క_2(నిండిన_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(స్థిరరాశి uint8_t *ఆదానం_2, size_t ఆదానం_len,
                                       స్థిరరాశి uint32_t కీలకం_2[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t *బహిర్గతం_2) {
#ఒకవేళ defined(BLAKE3_TESTING)
  ప్రతిజ్ఞా(0 < ఆదానం_len);
  ప్రతిజ్ఞా(ఆదానం_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#అయితే_ముగింపు

  స్థిరరాశి uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t ఆదానం_position = 0;
  size_t chunks_array_len = 0;
  అయితే (ఆదానం_len - ఆదానం_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &ఆదానం_2[ఆదానం_position];
    ఆదానం_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_ఖండం_LEN, కీలకం_2, chunk_counter,
                   true, flags, CHUNK_ప్రారంభం, CHUNK_చివర, బహిర్గతం_2);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  ఒకవేళ (ఆదానం_len > ఆదానం_position) {
    uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, కీలకం_2, flags);
    chunk_state.chunk_counter = counter;
    chunk_state_update(&chunk_state, &ఆదానం_2[ఆదానం_position],
                       ఆదానం_len - ఆదానం_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_విలువ(&output, &బహిర్గతం_2[chunks_array_len * BLAKE3_బహిర్గతం_LEN]);
    తిరిగి chunks_array_len + 1;
  } లేకపోతే {
    తిరిగి chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(స్థిరరాశి uint8_t *బిడ్డ_chaining_విలువలు,
                                        size_t అంకె_chaining_విలువలు,
                                        స్థిరరాశి uint32_t కీలకం_2[8], uint8_t flags,
                                        uint8_t *బహిర్గతం_2) {
#ఒకవేళ defined(BLAKE3_TESTING)
  ప్రతిజ్ఞా(2 <= అంకె_chaining_విలువలు);
  ప్రతిజ్ఞా(అంకె_chaining_విలువలు <= 2 * MAX_SIMD_DEGREE_OR_2);
#అయితే_ముగింపు

  స్థిరరాశి uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  అయితే (అంకె_chaining_విలువలు - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &బిడ్డ_chaining_విలువలు[2 * parents_array_len * BLAKE3_బహిర్గతం_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, కీలకం_2,
                   0, // Parents always use counter 0.
                   false, flags | మాతృ,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   బహిర్గతం_2);

  // If there's an odd child left over, it becomes an output.
  ఒకవేళ (అంకె_chaining_విలువలు > 2 * parents_array_len) {
    జ్ఞాపకం_నకలు(&బహిర్గతం_2[parents_array_len * BLAKE3_బహిర్గతం_LEN],
           &బిడ్డ_chaining_విలువలు[2 * parents_array_len * BLAKE3_బహిర్గతం_LEN],
           BLAKE3_బహిర్గతం_LEN);
    తిరిగి parents_array_len + 1;
  } లేకపోతే {
    తిరిగి parents_array_len;
  }
}

// The wide helper function returns (writes out) an array of chaining values
// and returns the length of that array. The number of chaining values returned
// is the dynamically detected SIMD degree, at most MAX_SIMD_DEGREE. Or fewer,
// if the input is shorter than that many chunks. The reason for maintaining a
// wide array of chaining values going back up the tree, is to allow the
// implementation to hash as many parents in parallel as possible.
//
// As a special case when the SIMD degree is 1, this function will still return
// at least 2 outputs. This guarantees that this function doesn't perform the
// root compression. (If it did, it would use the wrong flags, and also we
// wouldn't be able to implement extendable output.) Note that this function is
// not used when the whole input is only 1 chunk long; that's a different
// codepath.
//
// Why not just have the caller split the input on the first update(), instead
// of implementing this special rule? Because we don't want to limit SIMD or
// multi-threading parallelism for that update().
size_t blake3_compress_subtree_wide(స్థిరరాశి uint8_t *ఆదానం_2, size_t ఆదానం_len,
                                    స్థిరరాశి uint32_t కీలకం_2[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t *బహిర్గతం_2, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  ఒకవేళ (ఆదానం_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    తిరిగి compress_chunks_parallel(ఆదానం_2, ఆదానం_len, కీలకం_2, chunk_counter, flags,
                                    బహిర్గతం_2);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t ఎడమ_ఆదానం_len = ఎడమ_subtree_len(ఆదానం_len);
  size_t కుడి_ఆదానం_len = ఆదానం_len - ఎడమ_ఆదానం_len;
  స్థిరరాశి uint8_t *కుడి_ఆదానం = &ఆదానం_2[ఎడమ_ఆదానం_len];
  uint64_t కుడి_chunk_counter =
      chunk_counter + (uint64_t)(ఎడమ_ఆదానం_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_బహిర్గతం_LEN];
  size_t degree = blake3_simd_degree();
  ఒకవేళ (ఎడమ_ఆదానం_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *కుడి_cvs = &cv_array[degree * BLAKE3_బహిర్గతం_LEN];

  // Recurse!
  size_t ఎడమ_n = -1;
  size_t కుడి_n = -1;

#ఒకవేళ defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      కీలకం_2, flags, use_tbb,
      // left-hand side
      ఆదానం_2, ఎడమ_ఆదానం_len, chunk_counter, cv_array, &ఎడమ_n,
      // right-hand side
      కుడి_ఆదానం, కుడి_ఆదానం_len, కుడి_chunk_counter, కుడి_cvs, &కుడి_n);
#లేకపోతే
  ఎడమ_n = blake3_compress_subtree_wide(
      ఆదానం_2, ఎడమ_ఆదానం_len, కీలకం_2, chunk_counter, flags, cv_array, use_tbb);
  కుడి_n = blake3_compress_subtree_wide(కుడి_ఆదానం, కుడి_ఆదానం_len, కీలకం_2,
                                         కుడి_chunk_counter, flags, కుడి_cvs,
                                         use_tbb);
#అయితే_ముగింపు // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  ఒకవేళ (ఎడమ_n == 1) {
    జ్ఞాపకం_నకలు(బహిర్గతం_2, cv_array, 2 * BLAKE3_బహిర్గతం_LEN);
    తిరిగి 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t అంకె_chaining_విలువలు = ఎడమ_n + కుడి_n;
  తిరిగి compress_parents_parallel(cv_array, అంకె_chaining_విలువలు, కీలకం_2, flags,
                                   బహిర్గతం_2);
}

// Hash a subtree with compress_subtree_wide(), and then condense the resulting
// list of chaining values down to a single parent node. Don't compress that
// last parent node, however. Instead, return its message bytes (the
// concatenated chaining values of its children). This is necessary when the
// first call to update() supplies a complete subtree, because the topmost
// parent node of that subtree could end up being the root. It's also necessary
// for extended output in the general case.
//
// As with compress_subtree_wide(), this function is not used on inputs of 1
// chunk or less. That's a different codepath.
INLINE శూన్య
compress_subtree_గమ్యం_మాతృ_కణుపు(స్థిరరాశి uint8_t *ఆదానం_2, size_t ఆదానం_len,
                                స్థిరరాశి uint32_t కీలకం_2[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t బహిర్గతం_2[2 * BLAKE3_బహిర్గతం_LEN],
                                bool use_tbb) {
#ఒకవేళ defined(BLAKE3_TESTING)
  ప్రతిజ్ఞా(ఆదానం_len > BLAKE3_CHUNK_LEN);
#అయితే_ముగింపు

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_బహిర్గతం_LEN];
  size_t అంకె_cvs = blake3_compress_subtree_wide(ఆదానం_2, ఆదానం_len, కీలకం_2,
                                                chunk_counter, flags, cv_array, use_tbb);
  ప్రతిజ్ఞా(అంకె_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#ఒకవేళ MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t బహిర్గతం_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_బహిర్గతం_LEN / 2];
  అయితే (అంకె_cvs > 2) {
    అంకె_cvs =
        compress_parents_parallel(cv_array, అంకె_cvs, కీలకం_2, flags, బహిర్గతం_array);
    జ్ఞాపకం_నకలు(cv_array, బహిర్గతం_array, అంకె_cvs * BLAKE3_బహిర్గతం_LEN);
  }
#అయితే_ముగింపు
  జ్ఞాపకం_నకలు(బహిర్గతం_2, cv_array, 2 * BLAKE3_బహిర్గతం_LEN);
}

INLINE శూన్య hasher_init_పునాది(blake3_hasher *self, స్థిరరాశి uint32_t కీలకం_2[8],
                             uint8_t flags) {
  జ్ఞాపకం_నకలు(self->కీలకం_2, కీలకం_2, BLAKE3_కీలకం_LEN);
  chunk_state_init(&self->chunk, కీలకం_2, flags);
  self->cv_దొంతర_len = 0;
}

శూన్య llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_పునాది(self, IV, 0); }

శూన్య llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              స్థిరరాశి uint8_t కీలకం_2[BLAKE3_కీలకం_LEN]) {
  uint32_t కీలకం_పదాలు[8];
  ఎక్కించు_కీలకం_పదాలు(కీలకం_2, కీలకం_పదాలు);
  hasher_init_పునాది(self, కీలకం_పదాలు, KEYED_HASH);
}

శూన్య llvm_blake3_hasher_init_derive_కీలకం_raw(blake3_hasher *self, స్థిరరాశి శూన్య *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_పునాది(&context_hasher, IV, DERIVE_కీలకం_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_కీలకం[BLAKE3_కీలకం_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_కీలకం, BLAKE3_కీలకం_LEN);
  uint32_t context_కీలకం_పదాలు[8];
  ఎక్కించు_కీలకం_పదాలు(context_కీలకం, context_కీలకం_పదాలు);
  hasher_init_పునాది(self, context_కీలకం_పదాలు, DERIVE_కీలకం_MATERIAL);
}

శూన్య llvm_blake3_hasher_init_derive_కీలకం(blake3_hasher *self, స్థిరరాశి అక్షరం *context) {
  llvm_blake3_hasher_init_derive_కీలకం_raw(self, context, అక్షరశ్రేణి_పొడవు(context));
}

// As described in hasher_push_cv() below, we do "lazy merging", delaying
// merges until right before the next CV is about to be added. This is
// different from the reference implementation. Another difference is that we
// aren't always merging 1 chunk at a time. Instead, each CV might represent
// any power-of-two number of chunks, as long as the smaller-above-larger stack
// order is maintained. Instead of the "count the trailing 0-bits" algorithm
// described in the spec, we use a "count the total number of 1-bits" variant
// that doesn't require us to retain the subtree size of the CV on top of the
// stack. The principle is the same: each CV that should remain in the stack is
// represented by a 1-bit in the total number of chunks (or bytes) so far.
INLINE శూన్య hasher_విలీనం_cv_దొంతర(blake3_hasher *self, uint64_t మొత్తం_len) {
  size_t post_విలీనం_దొంతర_len = (size_t)popcnt(మొత్తం_len);
  అయితే (self->cv_దొంతర_len > post_విలీనం_దొంతర_len) {
    uint8_t *మాతృ_కణుపు =
        &self->cv_దొంతర[(self->cv_దొంతర_len - 2) * BLAKE3_బహిర్గతం_LEN];
    output_t output = మాతృ_output(మాతృ_కణుపు, self->కీలకం_2, self->chunk.flags);
    output_chaining_విలువ(&output, మాతృ_కణుపు);
    self->cv_దొంతర_len -= 1;
  }
}

// In reference_impl.rs, we merge the new CV with existing CVs from the stack
// before pushing it. We can do that because we know more input is coming, so
// we know none of the merges are root.
//
// This setting is different. We want to feed as much input as possible to
// compress_subtree_wide(), without setting aside anything for the chunk_state.
// If the user gives us 64 KiB, we want to parallelize over all 64 KiB at once
// as a single subtree, if at all possible.
//
// This leads to two problems:
// 1) This 64 KiB input might be the only call that ever gets made to update.
//    In this case, the root node of the 64 KiB subtree would be the root node
//    of the whole tree, and it would need to be ROOT finalized. We can't
//    compress it until we know.
// 2) This 64 KiB input might complete a larger tree, whose root node is
//    similarly going to be the root of the whole tree. For example, maybe
//    we have 196 KiB (that is, 128 + 64) hashed so far. We can't compress the
//    node at the root of the 256 KiB subtree until we know how to finalize it.
//
// The second problem is solved with "lazy merging". That is, when we're about
// to add a CV to the stack, we don't merge it with anything first, as the
// reference impl does. Instead we do merges using the *previous* CV that was
// added, which is sitting on top of the stack, and we put the new CV
// (unmerged) on top of the stack afterwards. This guarantees that we never
// merge the root node until finalize().
//
// Solving the first problem requires an additional tool,
// compress_subtree_to_parent_node(). That function always returns the top
// *two* chaining values of the subtree it's compressing. We then do lazy
// merging with each of them separately, so that the second CV will always
// remain unmerged. (That also helps us support extendable output when we're
// hashing an input all-at-once.)
INLINE శూన్య hasher_నెట్టు_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_బహిర్గతం_LEN],
                           uint64_t chunk_counter) {
  hasher_విలీనం_cv_దొంతర(self, chunk_counter);
  జ్ఞాపకం_నకలు(&self->cv_దొంతర[self->cv_దొంతర_len * BLAKE3_బహిర్గతం_LEN], new_cv,
         BLAKE3_బహిర్గతం_LEN);
  self->cv_దొంతర_len += 1;
}

INLINE శూన్య blake3_hasher_update_పునాది(blake3_hasher *self, స్థిరరాశి శూన్య *ఆదానం_2,
                                      size_t ఆదానం_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  ఒకవేళ (ఆదానం_len == 0) {
    తిరిగి;
  }

  స్థిరరాశి uint8_t *ఆదానం_బైట్లు = (స్థిరరాశి uint8_t *)ఆదానం_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  ఒకవేళ (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    ఒకవేళ (take > ఆదానం_len) {
      take = ఆదానం_len;
    }
    chunk_state_update(&self->chunk, ఆదానం_బైట్లు, take);
    ఆదానం_బైట్లు += take;
    ఆదానం_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    ఒకవేళ (ఆదానం_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_విలువ(&output, chunk_cv);
      hasher_నెట్టు_cv(self, chunk_cv, self->chunk.chunk_counter);
      chunk_state_reset(&self->chunk, self->కీలకం_2, self->chunk.chunk_counter + 1);
    } లేకపోతే {
      తిరిగి;
    }
  }

  // Now the chunk_state is clear, and we have more input. If there's more than
  // a single chunk (so, definitely not the root chunk), hash the largest whole
  // subtree we can, with the full benefits of SIMD (and maybe in the future,
  // multi-threading) parallelism. Two restrictions:
  // - The subtree has to be a power-of-2 number of chunks. Only subtrees along
  //   the right edge can be incomplete, and we don't know where the right edge
  //   is going to be until we get to finalize().
  // - The subtree must evenly divide the total number of chunks up until this
  //   point (if total is not 0). If the current incomplete subtree is only
  //   waiting for 1 more chunk, we can't hash a subtree of 4 chunks. We have
  //   to complete the current subtree first.
  // Because we might need to break up the input to form powers of 2, or to
  // evenly divide what we already have, this part runs in a loop.
  అయితే (ఆదానం_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = విడత_కిందికి_గమ్యం_power_యొక్క_2(ఆదానం_len);
    uint64_t లెక్క_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
    // Shrink the subtree_len until it evenly divides the count so far. We know
    // that subtree_len itself is a power of 2, so we can use a bitmasking
    // trick instead of an actual remainder operation. (Note that if the caller
    // consistently passes power-of-2 inputs of the same size, as is hopefully
    // typical, this loop condition will always fail, and subtree_len will
    // always be the full length of the input.)
    //
    // An aside: We don't have to shrink subtree_len quite this much. For
    // example, if count_so_far is 1, we could pass 2 chunks to
    // compress_subtree_to_parent_node. Since we'll get 2 CVs back, we'll still
    // get the right answer in the end, and we might get to use 2-way SIMD
    // parallelism. The problem with this optimization, is that it gets us
    // stuck always hashing 2 chunks. The total number of chunks will remain
    // odd, and we'll never graduate to higher degrees of parallelism. See
    // https://github.com/BLAKE3-team/BLAKE3/issues/69.
    అయితే ((((uint64_t)(subtree_len - 1)) & లెక్క_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    ఒకవేళ (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->కీలకం_2, self->chunk.flags);
      chunk_state.chunk_counter = self->chunk.chunk_counter;
      chunk_state_update(&chunk_state, ఆదానం_బైట్లు, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_బహిర్గతం_LEN];
      output_chaining_విలువ(&output, cv);
      hasher_నెట్టు_cv(self, cv, chunk_state.chunk_counter);
    } లేకపోతే {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_బహిర్గతం_LEN];
      compress_subtree_గమ్యం_మాతృ_కణుపు(ఆదానం_బైట్లు, subtree_len, self->కీలకం_2,
                                      self->chunk.chunk_counter,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_నెట్టు_cv(self, cv_pair, self->chunk.chunk_counter);
      hasher_నెట్టు_cv(self, &cv_pair[BLAKE3_బహిర్గతం_LEN],
                     self->chunk.chunk_counter + (subtree_chunks / 2));
    }
    self->chunk.chunk_counter += subtree_chunks;
    ఆదానం_బైట్లు += subtree_len;
    ఆదానం_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  ఒకవేళ (ఆదానం_len > 0) {
    chunk_state_update(&self->chunk, ఆదానం_బైట్లు, ఆదానం_len);
    hasher_విలీనం_cv_దొంతర(self, self->chunk.chunk_counter);
  }
}

శూన్య llvm_blake3_hasher_update(blake3_hasher *self, స్థిరరాశి శూన్య *ఆదానం_2,
                          size_t ఆదానం_len) {
  bool use_tbb = false;
  blake3_hasher_update_పునాది(self, ఆదానం_2, ఆదానం_len, use_tbb);
}

#ఒకవేళ defined(BLAKE3_USE_TBB)
శూన్య blake3_hasher_update_tbb(blake3_hasher *self, స్థిరరాశి శూన్య *ఆదానం_2,
                              size_t ఆదానం_len) {
  bool use_tbb = true;
  blake3_hasher_update_పునాది(self, ఆదానం_2, ఆదానం_len, use_tbb);
}
#అయితే_ముగింపు // BLAKE3_USE_TBB

శూన్య llvm_blake3_hasher_finalize(స్థిరరాశి blake3_hasher *self, uint8_t *బహిర్గతం_2,
                            size_t బహిర్గతం_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, బహిర్గతం_2, బహిర్గతం_len);
#ఒకవేళ LLVM_జ్ఞాపకం_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(బహిర్గతం_2, బహిర్గతం_len);
#అయితే_ముగింపు
}

శూన్య llvm_blake3_hasher_finalize_seek(స్థిరరాశి blake3_hasher *self, uint64_t seek,
                                 uint8_t *బహిర్గతం_2, size_t బహిర్గతం_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  ఒకవేళ (బహిర్గతం_len == 0) {
    తిరిగి;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  ఒకవేళ (self->cv_దొంతర_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_మూలం_బైట్లు(&output, seek, బహిర్గతం_2, బహిర్గతం_len);
    తిరిగి;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_మిగిలిన;
  ఒకవేళ (chunk_state_len(&self->chunk) > 0) {
    cvs_మిగిలిన = self->cv_దొంతర_len;
    output = chunk_state_output(&self->chunk);
  } లేకపోతే {
    // There are always at least 2 CVs in the stack in this case.
    cvs_మిగిలిన = self->cv_దొంతర_len - 2;
    output = మాతృ_output(&self->cv_దొంతర[cvs_మిగిలిన * 32], self->కీలకం_2,
                           self->chunk.flags);
  }
  అయితే (cvs_మిగిలిన > 0) {
    cvs_మిగిలిన -= 1;
    uint8_t మాతృ_ఖండం[BLAKE3_ఖండం_LEN];
    జ్ఞాపకం_నకలు(మాతృ_ఖండం, &self->cv_దొంతర[cvs_మిగిలిన * 32], 32);
    output_chaining_విలువ(&output, &మాతృ_ఖండం[32]);
    output = మాతృ_output(మాతృ_ఖండం, self->కీలకం_2, self->chunk.flags);
  }
  output_మూలం_బైట్లు(&output, seek, బహిర్గతం_2, బహిర్గతం_len);
}

శూన్య llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->కీలకం_2, 0);
  self->cv_దొంతర_len = 0;
}
