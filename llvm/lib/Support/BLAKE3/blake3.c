/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *llvm_blake3_ಆವೃತ್ತಿ(ಶೂನ್ಯ) { ಹಿಂತಿರುಗಿ BLAKE3_ಆವೃತ್ತಿ_STRING; }

INLINE ಶೂನ್ಯ chunk_state_init(blake3_chunk_state *self, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                             uint8_t flags) {
  ಸ್ಮರಣೆ_ನಕಲು(self->cv, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  self->chunk_counter = 0;
  memset(self->buf, 0, BLAKE3_ಖಂಡ_LEN);
  self->buf_len = 0;
  self->ಖಂಡಗಳು_compressed = 0;
  self->flags = flags;
}

INLINE ಶೂನ್ಯ chunk_state_reset(blake3_chunk_state *self, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                              uint64_t chunk_counter) {
  ಸ್ಮರಣೆ_ನಕಲು(self->cv, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  self->chunk_counter = chunk_counter;
  self->ಖಂಡಗಳು_compressed = 0;
  memset(self->buf, 0, BLAKE3_ಖಂಡ_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(ಸ್ಥಿರಾಂಕ blake3_chunk_state *self) {
  ಹಿಂತಿರುಗಿ (BLAKE3_ಖಂಡ_LEN * (size_t)self->ಖಂಡಗಳು_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಒಳಹರಿವು_len) {
  size_t take = BLAKE3_ಖಂಡ_LEN - ((size_t)self->buf_len);
  ಆದರೆ (take > ಒಳಹರಿವು_len) {
    take = ಒಳಹರಿವು_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  ಸ್ಮರಣೆ_ನಕಲು(dest, ಒಳಹರಿವು_2, take);
  self->buf_len += (uint8_t)take;
  ಹಿಂತಿರುಗಿ take;
}

INLINE uint8_t chunk_state_maybe_ಆರಂಭ_flag(ಸ್ಥಿರಾಂಕ blake3_chunk_state *self) {
  ಆದರೆ (self->ಖಂಡಗಳು_compressed == 0) {
    ಹಿಂತಿರುಗಿ CHUNK_ಆರಂಭ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ 0;
  }
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  uint32_t ಒಳಹರಿವು_cv[8];
  uint64_t counter;
  uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN];
  uint8_t ಖಂಡ_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(ಸ್ಥಿರಾಂಕ uint32_t ಒಳಹರಿವು_cv[8],
                            ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                            uint8_t ಖಂಡ_len, uint64_t counter,
                            uint8_t flags) {
  output_t ret;
  ಸ್ಮರಣೆ_ನಕಲು(ret.ಒಳಹರಿವು_cv, ಒಳಹರಿವು_cv, 32);
  ಸ್ಮರಣೆ_ನಕಲು(ret.ಖಂಡ, ಖಂಡ, BLAKE3_ಖಂಡ_LEN);
  ret.ಖಂಡ_len = ಖಂಡ_len;
  ret.counter = counter;
  ret.flags = flags;
  ಹಿಂತಿರುಗಿ ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE ಶೂನ್ಯ output_chaining_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ output_t *self, uint8_t cv[32]) {
  uint32_t cv_ಪದಗಳು[8];
  ಸ್ಮರಣೆ_ನಕಲು(cv_ಪದಗಳು, self->ಒಳಹರಿವು_cv, 32);
  blake3_compress_ಒಳ_ಇರಿಸು(cv_ಪದಗಳು, self->ಖಂಡ, self->ಖಂಡ_len,
                           self->counter, self->flags);
  store_cv_ಪದಗಳು(cv, cv_ಪದಗಳು);
}

INLINE ಶೂನ್ಯ output_ಬೇರು_ಬೈಟುಗಳು(ಸ್ಥಿರಾಂಕ output_t *self, uint64_t seek, uint8_t *ಹೊರಹೋಗು_2,
                              size_t ಹೊರಹೋಗು_len) {
  ಆದರೆ (ಹೊರಹೋಗು_len == 0) {
      ಹಿಂತಿರುಗಿ;
  }
  uint64_t output_ಖಂಡ_counter = seek / 64;
  size_t ಸ್ಥಾನಾಂತರ_within_ಖಂಡ = seek % 64;
  uint8_t wide_buf[64];
  ಆದರೆ(ಸ್ಥಾನಾಂತರ_within_ಖಂಡ) {
    blake3_compress_xof(self->ಒಳಹರಿವು_cv, self->ಖಂಡ, self->ಖಂಡ_len, output_ಖಂಡ_counter, self->flags | ಬೇರು, wide_buf);
    ಸ್ಥಿರಾಂಕ size_t ಲಭ್ಯ_ಬೈಟುಗಳು = 64 - ಸ್ಥಾನಾಂತರ_within_ಖಂಡ;
    ಸ್ಥಿರಾಂಕ size_t ಬೈಟುಗಳು = ಹೊರಹೋಗು_len > ಲಭ್ಯ_ಬೈಟುಗಳು ? ಲಭ್ಯ_ಬೈಟುಗಳು : ಹೊರಹೋಗು_len;
    ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, wide_buf + ಸ್ಥಾನಾಂತರ_within_ಖಂಡ, ಬೈಟುಗಳು);
    ಹೊರಹೋಗು_2 += ಬೈಟುಗಳು;
    ಹೊರಹೋಗು_len -= ಬೈಟುಗಳು;
    output_ಖಂಡ_counter += 1;
  }
  ಆದರೆ(ಹೊರಹೋಗು_len / 64) {
    blake3_xof_many(self->ಒಳಹರಿವು_cv, self->ಖಂಡ, self->ಖಂಡ_len, output_ಖಂಡ_counter, self->flags | ಬೇರು, ಹೊರಹೋಗು_2, ಹೊರಹೋಗು_len / 64);
  }
  output_ಖಂಡ_counter += ಹೊರಹೋಗು_len / 64;
  ಹೊರಹೋಗು_2 += ಹೊರಹೋಗು_len & -64;
  ಹೊರಹೋಗು_len -= ಹೊರಹೋಗು_len & -64;
  ಆದರೆ(ಹೊರಹೋಗು_len) {
    blake3_compress_xof(self->ಒಳಹರಿವು_cv, self->ಖಂಡ, self->ಖಂಡ_len, output_ಖಂಡ_counter, self->flags | ಬೇರು, wide_buf);
    ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, wide_buf, ಹೊರಹೋಗು_len);
  }
}

INLINE ಶೂನ್ಯ chunk_state_update(blake3_chunk_state *self, ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2,
                               size_t ಒಳಹರಿವು_len) {
  ಆದರೆ (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, ಒಳಹರಿವು_2, ಒಳಹರಿವು_len);
    ಒಳಹರಿವು_2 += take;
    ಒಳಹರಿವು_len -= take;
    ಆದರೆ (ಒಳಹರಿವು_len > 0) {
      blake3_compress_ಒಳ_ಇರಿಸು(
          self->cv, self->buf, BLAKE3_ಖಂಡ_LEN, self->chunk_counter,
          self->flags | chunk_state_maybe_ಆರಂಭ_flag(self));
      self->ಖಂಡಗಳು_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_ಖಂಡ_LEN);
    }
  }

  ಆಗ (ಒಳಹರಿವು_len > BLAKE3_ಖಂಡ_LEN) {
    blake3_compress_ಒಳ_ಇರಿಸು(self->cv, ಒಳಹರಿವು_2, BLAKE3_ಖಂಡ_LEN,
                             self->chunk_counter,
                             self->flags | chunk_state_maybe_ಆರಂಭ_flag(self));
    self->ಖಂಡಗಳು_compressed += 1;
    ಒಳಹರಿವು_2 += BLAKE3_ಖಂಡ_LEN;
    ಒಳಹರಿವು_len -= BLAKE3_ಖಂಡ_LEN;
  }

  chunk_state_fill_buf(self, ಒಳಹರಿವು_2, ಒಳಹರಿವು_len);
}

INLINE output_t chunk_state_output(ಸ್ಥಿರಾಂಕ blake3_chunk_state *self) {
  uint8_t ಖಂಡ_flags =
      self->flags | chunk_state_maybe_ಆರಂಭ_flag(self) | CHUNK_ಕೊನೆ;
  ಹಿಂತಿರುಗಿ make_output(self->cv, self->buf, self->buf_len, self->chunk_counter,
                     ಖಂಡ_flags);
}

INLINE output_t ಪೋಷಕ_output(ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                              ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint8_t flags) {
  ಹಿಂತಿರುಗಿ make_output(ಕೀಲಿ_2, ಖಂಡ, BLAKE3_ಖಂಡ_LEN, 0, flags | ಪೋಷಕ);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t ಎಡ_subtree_len(size_t ಒಳಹರಿವು_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t ತುಂಬಿದ_chunks = (ಒಳಹರಿವು_len - 1) / BLAKE3_CHUNK_LEN;
  ಹಿಂತಿರುಗಿ ಸುತ್ತು_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_power_ದ_2(ತುಂಬಿದ_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಒಳಹರಿವು_len,
                                       ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t *ಹೊರಹೋಗು_2) {
#ಆದರೆ defined(BLAKE3_TESTING)
  ಪ್ರತಿಜ್ಞಾ(0 < ಒಳಹರಿವು_len);
  ಪ್ರತಿಜ್ಞಾ(ಒಳಹರಿವು_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ಥಿರಾಂಕ uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t ಒಳಹರಿವು_position = 0;
  size_t chunks_array_len = 0;
  ಆಗ (ಒಳಹರಿವು_len - ಒಳಹರಿವು_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &ಒಳಹರಿವು_2[ಒಳಹರಿವು_position];
    ಒಳಹರಿವು_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_ಖಂಡ_LEN, ಕೀಲಿ_2, chunk_counter,
                   true, flags, CHUNK_ಆರಂಭ, CHUNK_ಕೊನೆ, ಹೊರಹೋಗು_2);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  ಆದರೆ (ಒಳಹರಿವು_len > ಒಳಹರಿವು_position) {
    uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, ಕೀಲಿ_2, flags);
    chunk_state.chunk_counter = counter;
    chunk_state_update(&chunk_state, &ಒಳಹರಿವು_2[ಒಳಹರಿವು_position],
                       ಒಳಹರಿವು_len - ಒಳಹರಿವು_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_ಮೌಲ್ಯ(&output, &ಹೊರಹೋಗು_2[chunks_array_len * BLAKE3_ಹೊರಹೋಗು_LEN]);
    ಹಿಂತಿರುಗಿ chunks_array_len + 1;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(ಸ್ಥಿರಾಂಕ uint8_t *ಮಗು_chaining_ಮೌಲ್ಯಗಳು,
                                        size_t ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು,
                                        ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint8_t flags,
                                        uint8_t *ಹೊರಹೋಗು_2) {
#ಆದರೆ defined(BLAKE3_TESTING)
  ಪ್ರತಿಜ್ಞಾ(2 <= ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು);
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು <= 2 * MAX_SIMD_DEGREE_OR_2);
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ಥಿರಾಂಕ uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  ಆಗ (ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &ಮಗು_chaining_ಮೌಲ್ಯಗಳು[2 * parents_array_len * BLAKE3_ಹೊರಹೋಗು_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, ಕೀಲಿ_2,
                   0, // Parents always use counter 0.
                   false, flags | ಪೋಷಕ,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   ಹೊರಹೋಗು_2);

  // If there's an odd child left over, it becomes an output.
  ಆದರೆ (ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು > 2 * parents_array_len) {
    ಸ್ಮರಣೆ_ನಕಲು(&ಹೊರಹೋಗು_2[parents_array_len * BLAKE3_ಹೊರಹೋಗು_LEN],
           &ಮಗು_chaining_ಮೌಲ್ಯಗಳು[2 * parents_array_len * BLAKE3_ಹೊರಹೋಗು_LEN],
           BLAKE3_ಹೊರಹೋಗು_LEN);
    ಹಿಂತಿರುಗಿ parents_array_len + 1;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ parents_array_len;
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
size_t blake3_compress_subtree_wide(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಒಳಹರಿವು_len,
                                    ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t *ಹೊರಹೋಗು_2, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  ಆದರೆ (ಒಳಹರಿವು_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    ಹಿಂತಿರುಗಿ compress_chunks_parallel(ಒಳಹರಿವು_2, ಒಳಹರಿವು_len, ಕೀಲಿ_2, chunk_counter, flags,
                                    ಹೊರಹೋಗು_2);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t ಎಡ_ಒಳಹರಿವು_len = ಎಡ_subtree_len(ಒಳಹರಿವು_len);
  size_t ಬಲ_ಒಳಹರಿವು_len = ಒಳಹರಿವು_len - ಎಡ_ಒಳಹರಿವು_len;
  ಸ್ಥಿರಾಂಕ uint8_t *ಬಲ_ಒಳಹರಿವು = &ಒಳಹರಿವು_2[ಎಡ_ಒಳಹರಿವು_len];
  uint64_t ಬಲ_chunk_counter =
      chunk_counter + (uint64_t)(ಎಡ_ಒಳಹರಿವು_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_ಹೊರಹೋಗು_LEN];
  size_t degree = blake3_simd_degree();
  ಆದರೆ (ಎಡ_ಒಳಹರಿವು_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *ಬಲ_cvs = &cv_array[degree * BLAKE3_ಹೊರಹೋಗು_LEN];

  // Recurse!
  size_t ಎಡ_n = -1;
  size_t ಬಲ_n = -1;

#ಆದರೆ defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      ಕೀಲಿ_2, flags, use_tbb,
      // left-hand side
      ಒಳಹರಿವು_2, ಎಡ_ಒಳಹರಿವು_len, chunk_counter, cv_array, &ಎಡ_n,
      // right-hand side
      ಬಲ_ಒಳಹರಿವು, ಬಲ_ಒಳಹರಿವು_len, ಬಲ_chunk_counter, ಬಲ_cvs, &ಬಲ_n);
#ಇಲ್ಲದಿದ್ದರೆ
  ಎಡ_n = blake3_compress_subtree_wide(
      ಒಳಹರಿವು_2, ಎಡ_ಒಳಹರಿವು_len, ಕೀಲಿ_2, chunk_counter, flags, cv_array, use_tbb);
  ಬಲ_n = blake3_compress_subtree_wide(ಬಲ_ಒಳಹರಿವು, ಬಲ_ಒಳಹರಿವು_len, ಕೀಲಿ_2,
                                         ಬಲ_chunk_counter, flags, ಬಲ_cvs,
                                         use_tbb);
#ಆದರೆ_ಅಂತ್ಯ // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  ಆದರೆ (ಎಡ_n == 1) {
    ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, cv_array, 2 * BLAKE3_ಹೊರಹೋಗು_LEN);
    ಹಿಂತಿರುಗಿ 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು = ಎಡ_n + ಬಲ_n;
  ಹಿಂತಿರುಗಿ compress_parents_parallel(cv_array, ಅಂಕೆ_chaining_ಮೌಲ್ಯಗಳು, ಕೀಲಿ_2, flags,
                                   ಹೊರಹೋಗು_2);
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
INLINE ಶೂನ್ಯ
compress_subtree_ಗಮ್ಯ_ಪೋಷಕ_ಗಂಟು(ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_2, size_t ಒಳಹರಿವು_len,
                                ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t ಹೊರಹೋಗು_2[2 * BLAKE3_ಹೊರಹೋಗು_LEN],
                                bool use_tbb) {
#ಆದರೆ defined(BLAKE3_TESTING)
  ಪ್ರತಿಜ್ಞಾ(ಒಳಹರಿವು_len > BLAKE3_CHUNK_LEN);
#ಆದರೆ_ಅಂತ್ಯ

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_ಹೊರಹೋಗು_LEN];
  size_t ಅಂಕೆ_cvs = blake3_compress_subtree_wide(ಒಳಹರಿವು_2, ಒಳಹರಿವು_len, ಕೀಲಿ_2,
                                                chunk_counter, flags, cv_array, use_tbb);
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#ಆದರೆ MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t ಹೊರಹೋಗು_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_ಹೊರಹೋಗು_LEN / 2];
  ಆಗ (ಅಂಕೆ_cvs > 2) {
    ಅಂಕೆ_cvs =
        compress_parents_parallel(cv_array, ಅಂಕೆ_cvs, ಕೀಲಿ_2, flags, ಹೊರಹೋಗು_array);
    ಸ್ಮರಣೆ_ನಕಲು(cv_array, ಹೊರಹೋಗು_array, ಅಂಕೆ_cvs * BLAKE3_ಹೊರಹೋಗು_LEN);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_2, cv_array, 2 * BLAKE3_ಹೊರಹೋಗು_LEN);
}

INLINE ಶೂನ್ಯ hasher_init_ತಳ(blake3_hasher *self, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8],
                             uint8_t flags) {
  ಸ್ಮರಣೆ_ನಕಲು(self->ಕೀಲಿ_2, ಕೀಲಿ_2, BLAKE3_ಕೀಲಿ_LEN);
  chunk_state_init(&self->chunk, ಕೀಲಿ_2, flags);
  self->cv_ರಾಶಿ_len = 0;
}

ಶೂನ್ಯ llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_ತಳ(self, IV, 0); }

ಶೂನ್ಯ llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              ಸ್ಥಿರಾಂಕ uint8_t ಕೀಲಿ_2[BLAKE3_ಕೀಲಿ_LEN]) {
  uint32_t ಕೀಲಿ_ಪದಗಳು[8];
  ತುಂಬು_ಕೀಲಿ_ಪದಗಳು(ಕೀಲಿ_2, ಕೀಲಿ_ಪದಗಳು);
  hasher_init_ತಳ(self, ಕೀಲಿ_ಪದಗಳು, KEYED_HASH);
}

ಶೂನ್ಯ llvm_blake3_hasher_init_derive_ಕೀಲಿ_raw(blake3_hasher *self, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_ತಳ(&context_hasher, IV, DERIVE_ಕೀಲಿ_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_ಕೀಲಿ[BLAKE3_ಕೀಲಿ_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_ಕೀಲಿ, BLAKE3_ಕೀಲಿ_LEN);
  uint32_t context_ಕೀಲಿ_ಪದಗಳು[8];
  ತುಂಬು_ಕೀಲಿ_ಪದಗಳು(context_ಕೀಲಿ, context_ಕೀಲಿ_ಪದಗಳು);
  hasher_init_ತಳ(self, context_ಕೀಲಿ_ಪದಗಳು, DERIVE_ಕೀಲಿ_MATERIAL);
}

ಶೂನ್ಯ llvm_blake3_hasher_init_derive_ಕೀಲಿ(blake3_hasher *self, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *context) {
  llvm_blake3_hasher_init_derive_ಕೀಲಿ_raw(self, context, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(context));
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
INLINE ಶೂನ್ಯ hasher_ವಿಲೀನ_cv_ರಾಶಿ(blake3_hasher *self, uint64_t ಒಟ್ಟು_len) {
  size_t post_ವಿಲೀನ_ರಾಶಿ_len = (size_t)popcnt(ಒಟ್ಟು_len);
  ಆಗ (self->cv_ರಾಶಿ_len > post_ವಿಲೀನ_ರಾಶಿ_len) {
    uint8_t *ಪೋಷಕ_ಗಂಟು =
        &self->cv_ರಾಶಿ[(self->cv_ರಾಶಿ_len - 2) * BLAKE3_ಹೊರಹೋಗು_LEN];
    output_t output = ಪೋಷಕ_output(ಪೋಷಕ_ಗಂಟು, self->ಕೀಲಿ_2, self->chunk.flags);
    output_chaining_ಮೌಲ್ಯ(&output, ಪೋಷಕ_ಗಂಟು);
    self->cv_ರಾಶಿ_len -= 1;
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
INLINE ಶೂನ್ಯ hasher_ತಳ್ಳು_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_ಹೊರಹೋಗು_LEN],
                           uint64_t chunk_counter) {
  hasher_ವಿಲೀನ_cv_ರಾಶಿ(self, chunk_counter);
  ಸ್ಮರಣೆ_ನಕಲು(&self->cv_ರಾಶಿ[self->cv_ರಾಶಿ_len * BLAKE3_ಹೊರಹೋಗು_LEN], new_cv,
         BLAKE3_ಹೊರಹೋಗು_LEN);
  self->cv_ರಾಶಿ_len += 1;
}

INLINE ಶೂನ್ಯ blake3_hasher_update_ತಳ(blake3_hasher *self, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಒಳಹರಿವು_2,
                                      size_t ಒಳಹರಿವು_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  ಆದರೆ (ಒಳಹರಿವು_len == 0) {
    ಹಿಂತಿರುಗಿ;
  }

  ಸ್ಥಿರಾಂಕ uint8_t *ಒಳಹರಿವು_ಬೈಟುಗಳು = (ಸ್ಥಿರಾಂಕ uint8_t *)ಒಳಹರಿವು_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  ಆದರೆ (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    ಆದರೆ (take > ಒಳಹರಿವು_len) {
      take = ಒಳಹರಿವು_len;
    }
    chunk_state_update(&self->chunk, ಒಳಹರಿವು_ಬೈಟುಗಳು, take);
    ಒಳಹರಿವು_ಬೈಟುಗಳು += take;
    ಒಳಹರಿವು_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    ಆದರೆ (ಒಳಹರಿವು_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_ಮೌಲ್ಯ(&output, chunk_cv);
      hasher_ತಳ್ಳು_cv(self, chunk_cv, self->chunk.chunk_counter);
      chunk_state_reset(&self->chunk, self->ಕೀಲಿ_2, self->chunk.chunk_counter + 1);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಹಿಂತಿರುಗಿ;
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
  ಆಗ (ಒಳಹರಿವು_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = ಸುತ್ತು_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_power_ದ_2(ಒಳಹರಿವು_len);
    uint64_t ಎಣಿಕೆ_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
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
    ಆಗ ((((uint64_t)(subtree_len - 1)) & ಎಣಿಕೆ_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    ಆದರೆ (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->ಕೀಲಿ_2, self->chunk.flags);
      chunk_state.chunk_counter = self->chunk.chunk_counter;
      chunk_state_update(&chunk_state, ಒಳಹರಿವು_ಬೈಟುಗಳು, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_ಹೊರಹೋಗು_LEN];
      output_chaining_ಮೌಲ್ಯ(&output, cv);
      hasher_ತಳ್ಳು_cv(self, cv, chunk_state.chunk_counter);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_ಹೊರಹೋಗು_LEN];
      compress_subtree_ಗಮ್ಯ_ಪೋಷಕ_ಗಂಟು(ಒಳಹರಿವು_ಬೈಟುಗಳು, subtree_len, self->ಕೀಲಿ_2,
                                      self->chunk.chunk_counter,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_ತಳ್ಳು_cv(self, cv_pair, self->chunk.chunk_counter);
      hasher_ತಳ್ಳು_cv(self, &cv_pair[BLAKE3_ಹೊರಹೋಗು_LEN],
                     self->chunk.chunk_counter + (subtree_chunks / 2));
    }
    self->chunk.chunk_counter += subtree_chunks;
    ಒಳಹರಿವು_ಬೈಟುಗಳು += subtree_len;
    ಒಳಹರಿವು_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  ಆದರೆ (ಒಳಹರಿವು_len > 0) {
    chunk_state_update(&self->chunk, ಒಳಹರಿವು_ಬೈಟುಗಳು, ಒಳಹರಿವು_len);
    hasher_ವಿಲೀನ_cv_ರಾಶಿ(self, self->chunk.chunk_counter);
  }
}

ಶೂನ್ಯ llvm_blake3_hasher_update(blake3_hasher *self, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಒಳಹರಿವು_2,
                          size_t ಒಳಹರಿವು_len) {
  bool use_tbb = false;
  blake3_hasher_update_ತಳ(self, ಒಳಹರಿವು_2, ಒಳಹರಿವು_len, use_tbb);
}

#ಆದರೆ defined(BLAKE3_USE_TBB)
ಶೂನ್ಯ blake3_hasher_update_tbb(blake3_hasher *self, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಒಳಹರಿವು_2,
                              size_t ಒಳಹರಿವು_len) {
  bool use_tbb = true;
  blake3_hasher_update_ತಳ(self, ಒಳಹರಿವು_2, ಒಳಹರಿವು_len, use_tbb);
}
#ಆದರೆ_ಅಂತ್ಯ // BLAKE3_USE_TBB

ಶೂನ್ಯ llvm_blake3_hasher_finalize(ಸ್ಥಿರಾಂಕ blake3_hasher *self, uint8_t *ಹೊರಹೋಗು_2,
                            size_t ಹೊರಹೋಗು_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, ಹೊರಹೋಗು_2, ಹೊರಹೋಗು_len);
#ಆದರೆ LLVM_ಸ್ಮರಣೆ_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(ಹೊರಹೋಗು_2, ಹೊರಹೋಗು_len);
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ llvm_blake3_hasher_finalize_seek(ಸ್ಥಿರಾಂಕ blake3_hasher *self, uint64_t seek,
                                 uint8_t *ಹೊರಹೋಗು_2, size_t ಹೊರಹೋಗು_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  ಆದರೆ (ಹೊರಹೋಗು_len == 0) {
    ಹಿಂತಿರುಗಿ;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  ಆದರೆ (self->cv_ರಾಶಿ_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_ಬೇರು_ಬೈಟುಗಳು(&output, seek, ಹೊರಹೋಗು_2, ಹೊರಹೋಗು_len);
    ಹಿಂತಿರುಗಿ;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_ಉಳಿದ;
  ಆದರೆ (chunk_state_len(&self->chunk) > 0) {
    cvs_ಉಳಿದ = self->cv_ರಾಶಿ_len;
    output = chunk_state_output(&self->chunk);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // There are always at least 2 CVs in the stack in this case.
    cvs_ಉಳಿದ = self->cv_ರಾಶಿ_len - 2;
    output = ಪೋಷಕ_output(&self->cv_ರಾಶಿ[cvs_ಉಳಿದ * 32], self->ಕೀಲಿ_2,
                           self->chunk.flags);
  }
  ಆಗ (cvs_ಉಳಿದ > 0) {
    cvs_ಉಳಿದ -= 1;
    uint8_t ಪೋಷಕ_ಖಂಡ[BLAKE3_ಖಂಡ_LEN];
    ಸ್ಮರಣೆ_ನಕಲು(ಪೋಷಕ_ಖಂಡ, &self->cv_ರಾಶಿ[cvs_ಉಳಿದ * 32], 32);
    output_chaining_ಮೌಲ್ಯ(&output, &ಪೋಷಕ_ಖಂಡ[32]);
    output = ಪೋಷಕ_output(ಪೋಷಕ_ಖಂಡ, self->ಕೀಲಿ_2, self->chunk.flags);
  }
  output_ಬೇರು_ಬೈಟುಗಳು(&output, seek, ಹೊರಹೋಗು_2, ಹೊರಹೋಗು_len);
}

ಶೂನ್ಯ llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->ಕೀಲಿ_2, 0);
  self->cv_ರಾಶಿ_len = 0;
}
