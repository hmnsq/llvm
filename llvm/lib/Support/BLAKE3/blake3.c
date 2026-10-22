/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "blake3_impl.h"

ਅਚਲ ਅੱਖਰ *llvm_blake3_ਸੰਸਕਰਣ(ਖਾਲੀ) { ਵਾਪਸ BLAKE3_ਸੰਸਕਰਣ_STRING; }

INLINE ਖਾਲੀ chunk_state_init(blake3_chunk_state *self, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                             uint8_t flags) {
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(self->cv, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  self->chunk_counter = 0;
  memset(self->buf, 0, BLAKE3_ਖੰਡ_LEN);
  self->buf_len = 0;
  self->ਖੰਡਾਂ_compressed = 0;
  self->flags = flags;
}

INLINE ਖਾਲੀ chunk_state_reset(blake3_chunk_state *self, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                              uint64_t chunk_counter) {
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(self->cv, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  self->chunk_counter = chunk_counter;
  self->ਖੰਡਾਂ_compressed = 0;
  memset(self->buf, 0, BLAKE3_ਖੰਡ_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(ਅਚਲ blake3_chunk_state *self) {
  ਵਾਪਸ (BLAKE3_ਖੰਡ_LEN * (size_t)self->ਖੰਡਾਂ_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਨਿਵੇਸ਼_len) {
  size_t take = BLAKE3_ਖੰਡ_LEN - ((size_t)self->buf_len);
  ਜੇ (take > ਨਿਵੇਸ਼_len) {
    take = ਨਿਵੇਸ਼_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, ਨਿਵੇਸ਼_2, take);
  self->buf_len += (uint8_t)take;
  ਵਾਪਸ take;
}

INLINE uint8_t chunk_state_maybe_ਸ਼ੁਰੂ_flag(ਅਚਲ blake3_chunk_state *self) {
  ਜੇ (self->ਖੰਡਾਂ_compressed == 0) {
    ਵਾਪਸ CHUNK_ਸ਼ੁਰੂ;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ 0;
  }
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  uint32_t ਨਿਵੇਸ਼_cv[8];
  uint64_t counter;
  uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN];
  uint8_t ਖੰਡ_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(ਅਚਲ uint32_t ਨਿਵੇਸ਼_cv[8],
                            ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                            uint8_t ਖੰਡ_len, uint64_t counter,
                            uint8_t flags) {
  output_t ret;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ret.ਨਿਵੇਸ਼_cv, ਨਿਵੇਸ਼_cv, 32);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ret.ਖੰਡ, ਖੰਡ, BLAKE3_ਖੰਡ_LEN);
  ret.ਖੰਡ_len = ਖੰਡ_len;
  ret.counter = counter;
  ret.flags = flags;
  ਵਾਪਸ ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE ਖਾਲੀ output_chaining_ਮੁੱਲ(ਅਚਲ output_t *self, uint8_t cv[32]) {
  uint32_t cv_ਸ਼ਬਦ[8];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(cv_ਸ਼ਬਦ, self->ਨਿਵੇਸ਼_cv, 32);
  blake3_compress_ਵਿੱਚ_ਰੱਖੋ(cv_ਸ਼ਬਦ, self->ਖੰਡ, self->ਖੰਡ_len,
                           self->counter, self->flags);
  store_cv_ਸ਼ਬਦ(cv, cv_ਸ਼ਬਦ);
}

INLINE ਖਾਲੀ output_ਜੜ੍ਹ_ਬਾਈਟਾਂ(ਅਚਲ output_t *self, uint64_t seek, uint8_t *ਬਾਹਰਲਾ_2,
                              size_t ਬਾਹਰਲਾ_len) {
  ਜੇ (ਬਾਹਰਲਾ_len == 0) {
      ਵਾਪਸ;
  }
  uint64_t output_ਖੰਡ_counter = seek / 64;
  size_t ਵਿੱਥ_within_ਖੰਡ = seek % 64;
  uint8_t wide_buf[64];
  ਜੇ(ਵਿੱਥ_within_ਖੰਡ) {
    blake3_compress_xof(self->ਨਿਵੇਸ਼_cv, self->ਖੰਡ, self->ਖੰਡ_len, output_ਖੰਡ_counter, self->flags | ਜੜ੍ਹ, wide_buf);
    ਅਚਲ size_t ਉਪਲਬਧ_ਬਾਈਟਾਂ = 64 - ਵਿੱਥ_within_ਖੰਡ;
    ਅਚਲ size_t ਬਾਈਟਾਂ = ਬਾਹਰਲਾ_len > ਉਪਲਬਧ_ਬਾਈਟਾਂ ? ਉਪਲਬਧ_ਬਾਈਟਾਂ : ਬਾਹਰਲਾ_len;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, wide_buf + ਵਿੱਥ_within_ਖੰਡ, ਬਾਈਟਾਂ);
    ਬਾਹਰਲਾ_2 += ਬਾਈਟਾਂ;
    ਬਾਹਰਲਾ_len -= ਬਾਈਟਾਂ;
    output_ਖੰਡ_counter += 1;
  }
  ਜੇ(ਬਾਹਰਲਾ_len / 64) {
    blake3_xof_many(self->ਨਿਵੇਸ਼_cv, self->ਖੰਡ, self->ਖੰਡ_len, output_ਖੰਡ_counter, self->flags | ਜੜ੍ਹ, ਬਾਹਰਲਾ_2, ਬਾਹਰਲਾ_len / 64);
  }
  output_ਖੰਡ_counter += ਬਾਹਰਲਾ_len / 64;
  ਬਾਹਰਲਾ_2 += ਬਾਹਰਲਾ_len & -64;
  ਬਾਹਰਲਾ_len -= ਬਾਹਰਲਾ_len & -64;
  ਜੇ(ਬਾਹਰਲਾ_len) {
    blake3_compress_xof(self->ਨਿਵੇਸ਼_cv, self->ਖੰਡ, self->ਖੰਡ_len, output_ਖੰਡ_counter, self->flags | ਜੜ੍ਹ, wide_buf);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, wide_buf, ਬਾਹਰਲਾ_len);
  }
}

INLINE ਖਾਲੀ chunk_state_update(blake3_chunk_state *self, ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2,
                               size_t ਨਿਵੇਸ਼_len) {
  ਜੇ (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len);
    ਨਿਵੇਸ਼_2 += take;
    ਨਿਵੇਸ਼_len -= take;
    ਜੇ (ਨਿਵੇਸ਼_len > 0) {
      blake3_compress_ਵਿੱਚ_ਰੱਖੋ(
          self->cv, self->buf, BLAKE3_ਖੰਡ_LEN, self->chunk_counter,
          self->flags | chunk_state_maybe_ਸ਼ੁਰੂ_flag(self));
      self->ਖੰਡਾਂ_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_ਖੰਡ_LEN);
    }
  }

  ਜਦੋਂ (ਨਿਵੇਸ਼_len > BLAKE3_ਖੰਡ_LEN) {
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ(self->cv, ਨਿਵੇਸ਼_2, BLAKE3_ਖੰਡ_LEN,
                             self->chunk_counter,
                             self->flags | chunk_state_maybe_ਸ਼ੁਰੂ_flag(self));
    self->ਖੰਡਾਂ_compressed += 1;
    ਨਿਵੇਸ਼_2 += BLAKE3_ਖੰਡ_LEN;
    ਨਿਵੇਸ਼_len -= BLAKE3_ਖੰਡ_LEN;
  }

  chunk_state_fill_buf(self, ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len);
}

INLINE output_t chunk_state_output(ਅਚਲ blake3_chunk_state *self) {
  uint8_t ਖੰਡ_flags =
      self->flags | chunk_state_maybe_ਸ਼ੁਰੂ_flag(self) | CHUNK_ਸਿਰਾ;
  ਵਾਪਸ make_output(self->cv, self->buf, self->buf_len, self->chunk_counter,
                     ਖੰਡ_flags);
}

INLINE output_t ਮਾਪਾ_output(ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                              ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint8_t flags) {
  ਵਾਪਸ make_output(ਕੁੰਜੀ_2, ਖੰਡ, BLAKE3_ਖੰਡ_LEN, 0, flags | ਮਾਪਾ);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t ਖੱਬਾ_subtree_len(size_t ਨਿਵੇਸ਼_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t ਭਰਿਆ_chunks = (ਨਿਵੇਸ਼_len - 1) / BLAKE3_CHUNK_LEN;
  ਵਾਪਸ ਗੇੜਾ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_power_ਦਾ_2(ਭਰਿਆ_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਨਿਵੇਸ਼_len,
                                       ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t *ਬਾਹਰਲਾ_2) {
#ਜੇ defined(BLAKE3_TESTING)
  ਪ੍ਰਤਿਜ੍ਞਾ(0 < ਨਿਵੇਸ਼_len);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਨਿਵੇਸ਼_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#ਜੇ_ਅੰਤ

  ਅਚਲ uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t ਨਿਵੇਸ਼_position = 0;
  size_t chunks_array_len = 0;
  ਜਦੋਂ (ਨਿਵੇਸ਼_len - ਨਿਵੇਸ਼_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &ਨਿਵੇਸ਼_2[ਨਿਵੇਸ਼_position];
    ਨਿਵੇਸ਼_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_ਖੰਡ_LEN, ਕੁੰਜੀ_2, chunk_counter,
                   true, flags, CHUNK_ਸ਼ੁਰੂ, CHUNK_ਸਿਰਾ, ਬਾਹਰਲਾ_2);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  ਜੇ (ਨਿਵੇਸ਼_len > ਨਿਵੇਸ਼_position) {
    uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, ਕੁੰਜੀ_2, flags);
    chunk_state.chunk_counter = counter;
    chunk_state_update(&chunk_state, &ਨਿਵੇਸ਼_2[ਨਿਵੇਸ਼_position],
                       ਨਿਵੇਸ਼_len - ਨਿਵੇਸ਼_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_ਮੁੱਲ(&output, &ਬਾਹਰਲਾ_2[chunks_array_len * BLAKE3_ਬਾਹਰਲਾ_LEN]);
    ਵਾਪਸ chunks_array_len + 1;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(ਅਚਲ uint8_t *ਬੱਚਾ_chaining_ਮੁੱਲਾਂ,
                                        size_t ਅੰਕ_chaining_ਮੁੱਲਾਂ,
                                        ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint8_t flags,
                                        uint8_t *ਬਾਹਰਲਾ_2) {
#ਜੇ defined(BLAKE3_TESTING)
  ਪ੍ਰਤਿਜ੍ਞਾ(2 <= ਅੰਕ_chaining_ਮੁੱਲਾਂ);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_chaining_ਮੁੱਲਾਂ <= 2 * MAX_SIMD_DEGREE_OR_2);
#ਜੇ_ਅੰਤ

  ਅਚਲ uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  ਜਦੋਂ (ਅੰਕ_chaining_ਮੁੱਲਾਂ - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &ਬੱਚਾ_chaining_ਮੁੱਲਾਂ[2 * parents_array_len * BLAKE3_ਬਾਹਰਲਾ_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, ਕੁੰਜੀ_2,
                   0, // Parents always use counter 0.
                   false, flags | ਮਾਪਾ,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   ਬਾਹਰਲਾ_2);

  // If there's an odd child left over, it becomes an output.
  ਜੇ (ਅੰਕ_chaining_ਮੁੱਲਾਂ > 2 * parents_array_len) {
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਬਾਹਰਲਾ_2[parents_array_len * BLAKE3_ਬਾਹਰਲਾ_LEN],
           &ਬੱਚਾ_chaining_ਮੁੱਲਾਂ[2 * parents_array_len * BLAKE3_ਬਾਹਰਲਾ_LEN],
           BLAKE3_ਬਾਹਰਲਾ_LEN);
    ਵਾਪਸ parents_array_len + 1;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ parents_array_len;
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
size_t blake3_compress_subtree_wide(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਨਿਵੇਸ਼_len,
                                    ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t *ਬਾਹਰਲਾ_2, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  ਜੇ (ਨਿਵੇਸ਼_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    ਵਾਪਸ compress_chunks_parallel(ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len, ਕੁੰਜੀ_2, chunk_counter, flags,
                                    ਬਾਹਰਲਾ_2);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t ਖੱਬਾ_ਨਿਵੇਸ਼_len = ਖੱਬਾ_subtree_len(ਨਿਵੇਸ਼_len);
  size_t ਸੱਜਾ_ਨਿਵੇਸ਼_len = ਨਿਵੇਸ਼_len - ਖੱਬਾ_ਨਿਵੇਸ਼_len;
  ਅਚਲ uint8_t *ਸੱਜਾ_ਨਿਵੇਸ਼ = &ਨਿਵੇਸ਼_2[ਖੱਬਾ_ਨਿਵੇਸ਼_len];
  uint64_t ਸੱਜਾ_chunk_counter =
      chunk_counter + (uint64_t)(ਖੱਬਾ_ਨਿਵੇਸ਼_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_ਬਾਹਰਲਾ_LEN];
  size_t degree = blake3_simd_degree();
  ਜੇ (ਖੱਬਾ_ਨਿਵੇਸ਼_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *ਸੱਜਾ_cvs = &cv_array[degree * BLAKE3_ਬਾਹਰਲਾ_LEN];

  // Recurse!
  size_t ਖੱਬਾ_n = -1;
  size_t ਸੱਜਾ_n = -1;

#ਜੇ defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      ਕੁੰਜੀ_2, flags, use_tbb,
      // left-hand side
      ਨਿਵੇਸ਼_2, ਖੱਬਾ_ਨਿਵੇਸ਼_len, chunk_counter, cv_array, &ਖੱਬਾ_n,
      // right-hand side
      ਸੱਜਾ_ਨਿਵੇਸ਼, ਸੱਜਾ_ਨਿਵੇਸ਼_len, ਸੱਜਾ_chunk_counter, ਸੱਜਾ_cvs, &ਸੱਜਾ_n);
#ਨਹੀਂਤਾਂ
  ਖੱਬਾ_n = blake3_compress_subtree_wide(
      ਨਿਵੇਸ਼_2, ਖੱਬਾ_ਨਿਵੇਸ਼_len, ਕੁੰਜੀ_2, chunk_counter, flags, cv_array, use_tbb);
  ਸੱਜਾ_n = blake3_compress_subtree_wide(ਸੱਜਾ_ਨਿਵੇਸ਼, ਸੱਜਾ_ਨਿਵੇਸ਼_len, ਕੁੰਜੀ_2,
                                         ਸੱਜਾ_chunk_counter, flags, ਸੱਜਾ_cvs,
                                         use_tbb);
#ਜੇ_ਅੰਤ // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  ਜੇ (ਖੱਬਾ_n == 1) {
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, cv_array, 2 * BLAKE3_ਬਾਹਰਲਾ_LEN);
    ਵਾਪਸ 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t ਅੰਕ_chaining_ਮੁੱਲਾਂ = ਖੱਬਾ_n + ਸੱਜਾ_n;
  ਵਾਪਸ compress_parents_parallel(cv_array, ਅੰਕ_chaining_ਮੁੱਲਾਂ, ਕੁੰਜੀ_2, flags,
                                   ਬਾਹਰਲਾ_2);
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
INLINE ਖਾਲੀ
compress_subtree_ਮੰਜ਼ਿਲ_ਮਾਪਾ_ਗੰਢ(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਨਿਵੇਸ਼_len,
                                ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[2 * BLAKE3_ਬਾਹਰਲਾ_LEN],
                                bool use_tbb) {
#ਜੇ defined(BLAKE3_TESTING)
  ਪ੍ਰਤਿਜ੍ਞਾ(ਨਿਵੇਸ਼_len > BLAKE3_CHUNK_LEN);
#ਜੇ_ਅੰਤ

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_ਬਾਹਰਲਾ_LEN];
  size_t ਅੰਕ_cvs = blake3_compress_subtree_wide(ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len, ਕੁੰਜੀ_2,
                                                chunk_counter, flags, cv_array, use_tbb);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#ਜੇ MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t ਬਾਹਰਲਾ_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_ਬਾਹਰਲਾ_LEN / 2];
  ਜਦੋਂ (ਅੰਕ_cvs > 2) {
    ਅੰਕ_cvs =
        compress_parents_parallel(cv_array, ਅੰਕ_cvs, ਕੁੰਜੀ_2, flags, ਬਾਹਰਲਾ_array);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(cv_array, ਬਾਹਰਲਾ_array, ਅੰਕ_cvs * BLAKE3_ਬਾਹਰਲਾ_LEN);
  }
#ਜੇ_ਅੰਤ
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_2, cv_array, 2 * BLAKE3_ਬਾਹਰਲਾ_LEN);
}

INLINE ਖਾਲੀ hasher_init_ਅਧਾਰ(blake3_hasher *self, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                             uint8_t flags) {
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(self->ਕੁੰਜੀ_2, ਕੁੰਜੀ_2, BLAKE3_ਕੁੰਜੀ_LEN);
  chunk_state_init(&self->chunk, ਕੁੰਜੀ_2, flags);
  self->cv_ਢੇਰ_len = 0;
}

ਖਾਲੀ llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_ਅਧਾਰ(self, IV, 0); }

ਖਾਲੀ llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              ਅਚਲ uint8_t ਕੁੰਜੀ_2[BLAKE3_ਕੁੰਜੀ_LEN]) {
  uint32_t ਕੁੰਜੀ_ਸ਼ਬਦ[8];
  ਲੱਦੋ_ਕੁੰਜੀ_ਸ਼ਬਦ(ਕੁੰਜੀ_2, ਕੁੰਜੀ_ਸ਼ਬਦ);
  hasher_init_ਅਧਾਰ(self, ਕੁੰਜੀ_ਸ਼ਬਦ, KEYED_HASH);
}

ਖਾਲੀ llvm_blake3_hasher_init_derive_ਕੁੰਜੀ_raw(blake3_hasher *self, ਅਚਲ ਖਾਲੀ *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_ਅਧਾਰ(&context_hasher, IV, DERIVE_ਕੁੰਜੀ_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_ਕੁੰਜੀ[BLAKE3_ਕੁੰਜੀ_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_ਕੁੰਜੀ, BLAKE3_ਕੁੰਜੀ_LEN);
  uint32_t context_ਕੁੰਜੀ_ਸ਼ਬਦ[8];
  ਲੱਦੋ_ਕੁੰਜੀ_ਸ਼ਬਦ(context_ਕੁੰਜੀ, context_ਕੁੰਜੀ_ਸ਼ਬਦ);
  hasher_init_ਅਧਾਰ(self, context_ਕੁੰਜੀ_ਸ਼ਬਦ, DERIVE_ਕੁੰਜੀ_MATERIAL);
}

ਖਾਲੀ llvm_blake3_hasher_init_derive_ਕੁੰਜੀ(blake3_hasher *self, ਅਚਲ ਅੱਖਰ *context) {
  llvm_blake3_hasher_init_derive_ਕੁੰਜੀ_raw(self, context, ਲੜੀ_ਲੰਬਾਈ(context));
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
INLINE ਖਾਲੀ hasher_ਮਿਲਾਓ_cv_ਢੇਰ(blake3_hasher *self, uint64_t ਕੁੱਲ_len) {
  size_t post_ਮਿਲਾਓ_ਢੇਰ_len = (size_t)popcnt(ਕੁੱਲ_len);
  ਜਦੋਂ (self->cv_ਢੇਰ_len > post_ਮਿਲਾਓ_ਢੇਰ_len) {
    uint8_t *ਮਾਪਾ_ਗੰਢ =
        &self->cv_ਢੇਰ[(self->cv_ਢੇਰ_len - 2) * BLAKE3_ਬਾਹਰਲਾ_LEN];
    output_t output = ਮਾਪਾ_output(ਮਾਪਾ_ਗੰਢ, self->ਕੁੰਜੀ_2, self->chunk.flags);
    output_chaining_ਮੁੱਲ(&output, ਮਾਪਾ_ਗੰਢ);
    self->cv_ਢੇਰ_len -= 1;
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
INLINE ਖਾਲੀ hasher_ਧੱਕੋ_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_ਬਾਹਰਲਾ_LEN],
                           uint64_t chunk_counter) {
  hasher_ਮਿਲਾਓ_cv_ਢੇਰ(self, chunk_counter);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&self->cv_ਢੇਰ[self->cv_ਢੇਰ_len * BLAKE3_ਬਾਹਰਲਾ_LEN], new_cv,
         BLAKE3_ਬਾਹਰਲਾ_LEN);
  self->cv_ਢੇਰ_len += 1;
}

INLINE ਖਾਲੀ blake3_hasher_update_ਅਧਾਰ(blake3_hasher *self, ਅਚਲ ਖਾਲੀ *ਨਿਵੇਸ਼_2,
                                      size_t ਨਿਵੇਸ਼_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  ਜੇ (ਨਿਵੇਸ਼_len == 0) {
    ਵਾਪਸ;
  }

  ਅਚਲ uint8_t *ਨਿਵੇਸ਼_ਬਾਈਟਾਂ = (ਅਚਲ uint8_t *)ਨਿਵੇਸ਼_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  ਜੇ (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    ਜੇ (take > ਨਿਵੇਸ਼_len) {
      take = ਨਿਵੇਸ਼_len;
    }
    chunk_state_update(&self->chunk, ਨਿਵੇਸ਼_ਬਾਈਟਾਂ, take);
    ਨਿਵੇਸ਼_ਬਾਈਟਾਂ += take;
    ਨਿਵੇਸ਼_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    ਜੇ (ਨਿਵੇਸ਼_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_ਮੁੱਲ(&output, chunk_cv);
      hasher_ਧੱਕੋ_cv(self, chunk_cv, self->chunk.chunk_counter);
      chunk_state_reset(&self->chunk, self->ਕੁੰਜੀ_2, self->chunk.chunk_counter + 1);
    } ਨਹੀਂਤਾਂ {
      ਵਾਪਸ;
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
  ਜਦੋਂ (ਨਿਵੇਸ਼_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = ਗੇੜਾ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_power_ਦਾ_2(ਨਿਵੇਸ਼_len);
    uint64_t ਗਿਣਤੀ_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
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
    ਜਦੋਂ ((((uint64_t)(subtree_len - 1)) & ਗਿਣਤੀ_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    ਜੇ (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->ਕੁੰਜੀ_2, self->chunk.flags);
      chunk_state.chunk_counter = self->chunk.chunk_counter;
      chunk_state_update(&chunk_state, ਨਿਵੇਸ਼_ਬਾਈਟਾਂ, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_ਬਾਹਰਲਾ_LEN];
      output_chaining_ਮੁੱਲ(&output, cv);
      hasher_ਧੱਕੋ_cv(self, cv, chunk_state.chunk_counter);
    } ਨਹੀਂਤਾਂ {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_ਬਾਹਰਲਾ_LEN];
      compress_subtree_ਮੰਜ਼ਿਲ_ਮਾਪਾ_ਗੰਢ(ਨਿਵੇਸ਼_ਬਾਈਟਾਂ, subtree_len, self->ਕੁੰਜੀ_2,
                                      self->chunk.chunk_counter,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_ਧੱਕੋ_cv(self, cv_pair, self->chunk.chunk_counter);
      hasher_ਧੱਕੋ_cv(self, &cv_pair[BLAKE3_ਬਾਹਰਲਾ_LEN],
                     self->chunk.chunk_counter + (subtree_chunks / 2));
    }
    self->chunk.chunk_counter += subtree_chunks;
    ਨਿਵੇਸ਼_ਬਾਈਟਾਂ += subtree_len;
    ਨਿਵੇਸ਼_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  ਜੇ (ਨਿਵੇਸ਼_len > 0) {
    chunk_state_update(&self->chunk, ਨਿਵੇਸ਼_ਬਾਈਟਾਂ, ਨਿਵੇਸ਼_len);
    hasher_ਮਿਲਾਓ_cv_ਢੇਰ(self, self->chunk.chunk_counter);
  }
}

ਖਾਲੀ llvm_blake3_hasher_update(blake3_hasher *self, ਅਚਲ ਖਾਲੀ *ਨਿਵੇਸ਼_2,
                          size_t ਨਿਵੇਸ਼_len) {
  bool use_tbb = false;
  blake3_hasher_update_ਅਧਾਰ(self, ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len, use_tbb);
}

#ਜੇ defined(BLAKE3_USE_TBB)
ਖਾਲੀ blake3_hasher_update_tbb(blake3_hasher *self, ਅਚਲ ਖਾਲੀ *ਨਿਵੇਸ਼_2,
                              size_t ਨਿਵੇਸ਼_len) {
  bool use_tbb = true;
  blake3_hasher_update_ਅਧਾਰ(self, ਨਿਵੇਸ਼_2, ਨਿਵੇਸ਼_len, use_tbb);
}
#ਜੇ_ਅੰਤ // BLAKE3_USE_TBB

ਖਾਲੀ llvm_blake3_hasher_finalize(ਅਚਲ blake3_hasher *self, uint8_t *ਬਾਹਰਲਾ_2,
                            size_t ਬਾਹਰਲਾ_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, ਬਾਹਰਲਾ_2, ਬਾਹਰਲਾ_len);
#ਜੇ LLVM_ਸਮ੍ਰਿਤੀ_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(ਬਾਹਰਲਾ_2, ਬਾਹਰਲਾ_len);
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ llvm_blake3_hasher_finalize_seek(ਅਚਲ blake3_hasher *self, uint64_t seek,
                                 uint8_t *ਬਾਹਰਲਾ_2, size_t ਬਾਹਰਲਾ_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  ਜੇ (ਬਾਹਰਲਾ_len == 0) {
    ਵਾਪਸ;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  ਜੇ (self->cv_ਢੇਰ_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_ਜੜ੍ਹ_ਬਾਈਟਾਂ(&output, seek, ਬਾਹਰਲਾ_2, ਬਾਹਰਲਾ_len);
    ਵਾਪਸ;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_ਬਚਿਆ;
  ਜੇ (chunk_state_len(&self->chunk) > 0) {
    cvs_ਬਚਿਆ = self->cv_ਢੇਰ_len;
    output = chunk_state_output(&self->chunk);
  } ਨਹੀਂਤਾਂ {
    // There are always at least 2 CVs in the stack in this case.
    cvs_ਬਚਿਆ = self->cv_ਢੇਰ_len - 2;
    output = ਮਾਪਾ_output(&self->cv_ਢੇਰ[cvs_ਬਚਿਆ * 32], self->ਕੁੰਜੀ_2,
                           self->chunk.flags);
  }
  ਜਦੋਂ (cvs_ਬਚਿਆ > 0) {
    cvs_ਬਚਿਆ -= 1;
    uint8_t ਮਾਪਾ_ਖੰਡ[BLAKE3_ਖੰਡ_LEN];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਮਾਪਾ_ਖੰਡ, &self->cv_ਢੇਰ[cvs_ਬਚਿਆ * 32], 32);
    output_chaining_ਮੁੱਲ(&output, &ਮਾਪਾ_ਖੰਡ[32]);
    output = ਮਾਪਾ_output(ਮਾਪਾ_ਖੰਡ, self->ਕੁੰਜੀ_2, self->chunk.flags);
  }
  output_ਜੜ੍ਹ_ਬਾਈਟਾਂ(&output, seek, ਬਾਹਰਲਾ_2, ਬਾਹਰਲਾ_len);
}

ਖਾਲੀ llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->ਕੁੰਜੀ_2, 0);
  self->cv_ਢੇਰ_len = 0;
}
