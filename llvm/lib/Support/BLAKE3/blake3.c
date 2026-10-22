/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "blake3_impl.h"

സ്ഥിരം അക്ഷരം *llvm_blake3_version(ശൂന്യം) { തിരികെ BLAKE3_VERSION_STRING; }

INLINE ശൂന്യം chunk_state_init(blake3_chunk_state *self, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                             uint8_t flags) {
  memcpy(self->cv, താക്കോൽ_2, BLAKE3_താക്കോൽ_LEN);
  self->chunk_counter = 0;
  memset(self->buf, 0, BLAKE3_BLOCK_LEN);
  self->buf_len = 0;
  self->blocks_compressed = 0;
  self->flags = flags;
}

INLINE ശൂന്യം chunk_state_reset(blake3_chunk_state *self, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                              uint64_t chunk_counter) {
  memcpy(self->cv, താക്കോൽ_2, BLAKE3_താക്കോൽ_LEN);
  self->chunk_counter = chunk_counter;
  self->blocks_compressed = 0;
  memset(self->buf, 0, BLAKE3_BLOCK_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(സ്ഥിരം blake3_chunk_state *self) {
  തിരികെ (BLAKE3_BLOCK_LEN * (size_t)self->blocks_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   സ്ഥിരം uint8_t *നിവേശം_2, size_t നിവേശം_len) {
  size_t take = BLAKE3_BLOCK_LEN - ((size_t)self->buf_len);
  എങ്കിൽ (take > നിവേശം_len) {
    take = നിവേശം_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  memcpy(dest, നിവേശം_2, take);
  self->buf_len += (uint8_t)take;
  തിരികെ take;
}

INLINE uint8_t chunk_state_maybe_start_flag(സ്ഥിരം blake3_chunk_state *self) {
  എങ്കിൽ (self->blocks_compressed == 0) {
    തിരികെ CHUNK_START;
  } അല്ലെങ്കിൽ {
    തിരികെ 0;
  }
}

തരനിർവചനം ഘടന {
  uint32_t നിവേശം_cv[8];
  uint64_t counter;
  uint8_t block[BLAKE3_BLOCK_LEN];
  uint8_t block_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(സ്ഥിരം uint32_t നിവേശം_cv[8],
                            സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter,
                            uint8_t flags) {
  output_t ret;
  memcpy(ret.നിവേശം_cv, നിവേശം_cv, 32);
  memcpy(ret.block, block, BLAKE3_BLOCK_LEN);
  ret.block_len = block_len;
  ret.counter = counter;
  ret.flags = flags;
  തിരികെ ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE ശൂന്യം output_chaining_മൂല്യം(സ്ഥിരം output_t *self, uint8_t cv[32]) {
  uint32_t cv_വാക്കുകൾ[8];
  memcpy(cv_വാക്കുകൾ, self->നിവേശം_cv, 32);
  blake3_compress_in_place(cv_വാക്കുകൾ, self->block, self->block_len,
                           self->counter, self->flags);
  store_cv_വാക്കുകൾ(cv, cv_വാക്കുകൾ);
}

INLINE ശൂന്യം output_വേര്_bytes(സ്ഥിരം output_t *self, uint64_t seek, uint8_t *out,
                              size_t out_len) {
  എങ്കിൽ (out_len == 0) {
      തിരികെ;
  }
  uint64_t output_block_counter = seek / 64;
  size_t offset_within_block = seek % 64;
  uint8_t wide_buf[64];
  എങ്കിൽ(offset_within_block) {
    blake3_compress_xof(self->നിവേശം_cv, self->block, self->block_len, output_block_counter, self->flags | വേര്, wide_buf);
    സ്ഥിരം size_t available_bytes = 64 - offset_within_block;
    സ്ഥിരം size_t bytes = out_len > available_bytes ? available_bytes : out_len;
    memcpy(out, wide_buf + offset_within_block, bytes);
    out += bytes;
    out_len -= bytes;
    output_block_counter += 1;
  }
  എങ്കിൽ(out_len / 64) {
    blake3_xof_many(self->നിവേശം_cv, self->block, self->block_len, output_block_counter, self->flags | വേര്, out, out_len / 64);
  }
  output_block_counter += out_len / 64;
  out += out_len & -64;
  out_len -= out_len & -64;
  എങ്കിൽ(out_len) {
    blake3_compress_xof(self->നിവേശം_cv, self->block, self->block_len, output_block_counter, self->flags | വേര്, wide_buf);
    memcpy(out, wide_buf, out_len);
  }
}

INLINE ശൂന്യം chunk_state_update(blake3_chunk_state *self, സ്ഥിരം uint8_t *നിവേശം_2,
                               size_t നിവേശം_len) {
  എങ്കിൽ (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, നിവേശം_2, നിവേശം_len);
    നിവേശം_2 += take;
    നിവേശം_len -= take;
    എങ്കിൽ (നിവേശം_len > 0) {
      blake3_compress_in_place(
          self->cv, self->buf, BLAKE3_BLOCK_LEN, self->chunk_counter,
          self->flags | chunk_state_maybe_start_flag(self));
      self->blocks_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_BLOCK_LEN);
    }
  }

  ആകുമ്പോൾ (നിവേശം_len > BLAKE3_BLOCK_LEN) {
    blake3_compress_in_place(self->cv, നിവേശം_2, BLAKE3_BLOCK_LEN,
                             self->chunk_counter,
                             self->flags | chunk_state_maybe_start_flag(self));
    self->blocks_compressed += 1;
    നിവേശം_2 += BLAKE3_BLOCK_LEN;
    നിവേശം_len -= BLAKE3_BLOCK_LEN;
  }

  chunk_state_fill_buf(self, നിവേശം_2, നിവേശം_len);
}

INLINE output_t chunk_state_output(സ്ഥിരം blake3_chunk_state *self) {
  uint8_t block_flags =
      self->flags | chunk_state_maybe_start_flag(self) | CHUNK_END;
  തിരികെ make_output(self->cv, self->buf, self->buf_len, self->chunk_counter,
                     block_flags);
}

INLINE output_t parent_output(സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                              സ്ഥിരം uint32_t താക്കോൽ_2[8], uint8_t flags) {
  തിരികെ make_output(താക്കോൽ_2, block, BLAKE3_BLOCK_LEN, 0, flags | PARENT);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t ഇടത്_subtree_len(size_t നിവേശം_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t full_chunks = (നിവേശം_len - 1) / BLAKE3_CHUNK_LEN;
  തിരികെ round_down_to_power_of_2(full_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(സ്ഥിരം uint8_t *നിവേശം_2, size_t നിവേശം_len,
                                       സ്ഥിരം uint32_t താക്കോൽ_2[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t *out) {
#എങ്കിൽ defined(BLAKE3_TESTING)
  പ്രതിജ്ഞാ(0 < നിവേശം_len);
  പ്രതിജ്ഞാ(നിവേശം_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#endif

  സ്ഥിരം uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t നിവേശം_position = 0;
  size_t chunks_array_len = 0;
  ആകുമ്പോൾ (നിവേശം_len - നിവേശം_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &നിവേശം_2[നിവേശം_position];
    നിവേശം_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_BLOCK_LEN, താക്കോൽ_2, chunk_counter,
                   true, flags, CHUNK_START, CHUNK_END, out);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  എങ്കിൽ (നിവേശം_len > നിവേശം_position) {
    uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, താക്കോൽ_2, flags);
    chunk_state.chunk_counter = counter;
    chunk_state_update(&chunk_state, &നിവേശം_2[നിവേശം_position],
                       നിവേശം_len - നിവേശം_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_മൂല്യം(&output, &out[chunks_array_len * BLAKE3_OUT_LEN]);
    തിരികെ chunks_array_len + 1;
  } അല്ലെങ്കിൽ {
    തിരികെ chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(സ്ഥിരം uint8_t *child_chaining_മൂല്യങ്ങൾ,
                                        size_t അക്കം_chaining_മൂല്യങ്ങൾ,
                                        സ്ഥിരം uint32_t താക്കോൽ_2[8], uint8_t flags,
                                        uint8_t *out) {
#എങ്കിൽ defined(BLAKE3_TESTING)
  പ്രതിജ്ഞാ(2 <= അക്കം_chaining_മൂല്യങ്ങൾ);
  പ്രതിജ്ഞാ(അക്കം_chaining_മൂല്യങ്ങൾ <= 2 * MAX_SIMD_DEGREE_OR_2);
#endif

  സ്ഥിരം uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  ആകുമ്പോൾ (അക്കം_chaining_മൂല്യങ്ങൾ - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &child_chaining_മൂല്യങ്ങൾ[2 * parents_array_len * BLAKE3_OUT_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, താക്കോൽ_2,
                   0, // Parents always use counter 0.
                   false, flags | PARENT,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   out);

  // If there's an odd child left over, it becomes an output.
  എങ്കിൽ (അക്കം_chaining_മൂല്യങ്ങൾ > 2 * parents_array_len) {
    memcpy(&out[parents_array_len * BLAKE3_OUT_LEN],
           &child_chaining_മൂല്യങ്ങൾ[2 * parents_array_len * BLAKE3_OUT_LEN],
           BLAKE3_OUT_LEN);
    തിരികെ parents_array_len + 1;
  } അല്ലെങ്കിൽ {
    തിരികെ parents_array_len;
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
size_t blake3_compress_subtree_wide(സ്ഥിരം uint8_t *നിവേശം_2, size_t നിവേശം_len,
                                    സ്ഥിരം uint32_t താക്കോൽ_2[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t *out, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  എങ്കിൽ (നിവേശം_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    തിരികെ compress_chunks_parallel(നിവേശം_2, നിവേശം_len, താക്കോൽ_2, chunk_counter, flags,
                                    out);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t ഇടത്_നിവേശം_len = ഇടത്_subtree_len(നിവേശം_len);
  size_t വലത്_നിവേശം_len = നിവേശം_len - ഇടത്_നിവേശം_len;
  സ്ഥിരം uint8_t *വലത്_നിവേശം = &നിവേശം_2[ഇടത്_നിവേശം_len];
  uint64_t വലത്_chunk_counter =
      chunk_counter + (uint64_t)(ഇടത്_നിവേശം_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
  size_t degree = blake3_simd_degree();
  എങ്കിൽ (ഇടത്_നിവേശം_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *വലത്_cvs = &cv_array[degree * BLAKE3_OUT_LEN];

  // Recurse!
  size_t ഇടത്_n = -1;
  size_t വലത്_n = -1;

#എങ്കിൽ defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      താക്കോൽ_2, flags, use_tbb,
      // left-hand side
      നിവേശം_2, ഇടത്_നിവേശം_len, chunk_counter, cv_array, &ഇടത്_n,
      // right-hand side
      വലത്_നിവേശം, വലത്_നിവേശം_len, വലത്_chunk_counter, വലത്_cvs, &വലത്_n);
#അല്ലെങ്കിൽ
  ഇടത്_n = blake3_compress_subtree_wide(
      നിവേശം_2, ഇടത്_നിവേശം_len, താക്കോൽ_2, chunk_counter, flags, cv_array, use_tbb);
  വലത്_n = blake3_compress_subtree_wide(വലത്_നിവേശം, വലത്_നിവേശം_len, താക്കോൽ_2,
                                         വലത്_chunk_counter, flags, വലത്_cvs,
                                         use_tbb);
#endif // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  എങ്കിൽ (ഇടത്_n == 1) {
    memcpy(out, cv_array, 2 * BLAKE3_OUT_LEN);
    തിരികെ 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t അക്കം_chaining_മൂല്യങ്ങൾ = ഇടത്_n + വലത്_n;
  തിരികെ compress_parents_parallel(cv_array, അക്കം_chaining_മൂല്യങ്ങൾ, താക്കോൽ_2, flags,
                                   out);
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
INLINE ശൂന്യം
compress_subtree_to_parent_കണ്ണി(സ്ഥിരം uint8_t *നിവേശം_2, size_t നിവേശം_len,
                                സ്ഥിരം uint32_t താക്കോൽ_2[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t out[2 * BLAKE3_OUT_LEN],
                                bool use_tbb) {
#എങ്കിൽ defined(BLAKE3_TESTING)
  പ്രതിജ്ഞാ(നിവേശം_len > BLAKE3_CHUNK_LEN);
#endif

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
  size_t അക്കം_cvs = blake3_compress_subtree_wide(നിവേശം_2, നിവേശം_len, താക്കോൽ_2,
                                                chunk_counter, flags, cv_array, use_tbb);
  പ്രതിജ്ഞാ(അക്കം_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#എങ്കിൽ MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t out_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN / 2];
  ആകുമ്പോൾ (അക്കം_cvs > 2) {
    അക്കം_cvs =
        compress_parents_parallel(cv_array, അക്കം_cvs, താക്കോൽ_2, flags, out_array);
    memcpy(cv_array, out_array, അക്കം_cvs * BLAKE3_OUT_LEN);
  }
#endif
  memcpy(out, cv_array, 2 * BLAKE3_OUT_LEN);
}

INLINE ശൂന്യം hasher_init_base(blake3_hasher *self, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                             uint8_t flags) {
  memcpy(self->താക്കോൽ_2, താക്കോൽ_2, BLAKE3_താക്കോൽ_LEN);
  chunk_state_init(&self->chunk, താക്കോൽ_2, flags);
  self->cv_അട്ടി_len = 0;
}

ശൂന്യം llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_base(self, IV, 0); }

ശൂന്യം llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              സ്ഥിരം uint8_t താക്കോൽ_2[BLAKE3_താക്കോൽ_LEN]) {
  uint32_t താക്കോൽ_വാക്കുകൾ[8];
  load_താക്കോൽ_വാക്കുകൾ(താക്കോൽ_2, താക്കോൽ_വാക്കുകൾ);
  hasher_init_base(self, താക്കോൽ_വാക്കുകൾ, KEYED_HASH);
}

ശൂന്യം llvm_blake3_hasher_init_derive_താക്കോൽ_raw(blake3_hasher *self, സ്ഥിരം ശൂന്യം *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_base(&context_hasher, IV, DERIVE_താക്കോൽ_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_താക്കോൽ[BLAKE3_താക്കോൽ_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_താക്കോൽ, BLAKE3_താക്കോൽ_LEN);
  uint32_t context_താക്കോൽ_വാക്കുകൾ[8];
  load_താക്കോൽ_വാക്കുകൾ(context_താക്കോൽ, context_താക്കോൽ_വാക്കുകൾ);
  hasher_init_base(self, context_താക്കോൽ_വാക്കുകൾ, DERIVE_താക്കോൽ_MATERIAL);
}

ശൂന്യം llvm_blake3_hasher_init_derive_താക്കോൽ(blake3_hasher *self, സ്ഥിരം അക്ഷരം *context) {
  llvm_blake3_hasher_init_derive_താക്കോൽ_raw(self, context, strlen(context));
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
INLINE ശൂന്യം hasher_merge_cv_അട്ടി(blake3_hasher *self, uint64_t total_len) {
  size_t post_merge_അട്ടി_len = (size_t)popcnt(total_len);
  ആകുമ്പോൾ (self->cv_അട്ടി_len > post_merge_അട്ടി_len) {
    uint8_t *parent_കണ്ണി =
        &self->cv_അട്ടി[(self->cv_അട്ടി_len - 2) * BLAKE3_OUT_LEN];
    output_t output = parent_output(parent_കണ്ണി, self->താക്കോൽ_2, self->chunk.flags);
    output_chaining_മൂല്യം(&output, parent_കണ്ണി);
    self->cv_അട്ടി_len -= 1;
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
INLINE ശൂന്യം hasher_തള്ളുക_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_OUT_LEN],
                           uint64_t chunk_counter) {
  hasher_merge_cv_അട്ടി(self, chunk_counter);
  memcpy(&self->cv_അട്ടി[self->cv_അട്ടി_len * BLAKE3_OUT_LEN], new_cv,
         BLAKE3_OUT_LEN);
  self->cv_അട്ടി_len += 1;
}

INLINE ശൂന്യം blake3_hasher_update_base(blake3_hasher *self, സ്ഥിരം ശൂന്യം *നിവേശം_2,
                                      size_t നിവേശം_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  എങ്കിൽ (നിവേശം_len == 0) {
    തിരികെ;
  }

  സ്ഥിരം uint8_t *നിവേശം_bytes = (സ്ഥിരം uint8_t *)നിവേശം_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  എങ്കിൽ (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    എങ്കിൽ (take > നിവേശം_len) {
      take = നിവേശം_len;
    }
    chunk_state_update(&self->chunk, നിവേശം_bytes, take);
    നിവേശം_bytes += take;
    നിവേശം_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    എങ്കിൽ (നിവേശം_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_മൂല്യം(&output, chunk_cv);
      hasher_തള്ളുക_cv(self, chunk_cv, self->chunk.chunk_counter);
      chunk_state_reset(&self->chunk, self->താക്കോൽ_2, self->chunk.chunk_counter + 1);
    } അല്ലെങ്കിൽ {
      തിരികെ;
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
  ആകുമ്പോൾ (നിവേശം_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = round_down_to_power_of_2(നിവേശം_len);
    uint64_t എണ്ണം_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
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
    ആകുമ്പോൾ ((((uint64_t)(subtree_len - 1)) & എണ്ണം_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    എങ്കിൽ (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->താക്കോൽ_2, self->chunk.flags);
      chunk_state.chunk_counter = self->chunk.chunk_counter;
      chunk_state_update(&chunk_state, നിവേശം_bytes, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_OUT_LEN];
      output_chaining_മൂല്യം(&output, cv);
      hasher_തള്ളുക_cv(self, cv, chunk_state.chunk_counter);
    } അല്ലെങ്കിൽ {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_OUT_LEN];
      compress_subtree_to_parent_കണ്ണി(നിവേശം_bytes, subtree_len, self->താക്കോൽ_2,
                                      self->chunk.chunk_counter,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_തള്ളുക_cv(self, cv_pair, self->chunk.chunk_counter);
      hasher_തള്ളുക_cv(self, &cv_pair[BLAKE3_OUT_LEN],
                     self->chunk.chunk_counter + (subtree_chunks / 2));
    }
    self->chunk.chunk_counter += subtree_chunks;
    നിവേശം_bytes += subtree_len;
    നിവേശം_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  എങ്കിൽ (നിവേശം_len > 0) {
    chunk_state_update(&self->chunk, നിവേശം_bytes, നിവേശം_len);
    hasher_merge_cv_അട്ടി(self, self->chunk.chunk_counter);
  }
}

ശൂന്യം llvm_blake3_hasher_update(blake3_hasher *self, സ്ഥിരം ശൂന്യം *നിവേശം_2,
                          size_t നിവേശം_len) {
  bool use_tbb = false;
  blake3_hasher_update_base(self, നിവേശം_2, നിവേശം_len, use_tbb);
}

#എങ്കിൽ defined(BLAKE3_USE_TBB)
ശൂന്യം blake3_hasher_update_tbb(blake3_hasher *self, സ്ഥിരം ശൂന്യം *നിവേശം_2,
                              size_t നിവേശം_len) {
  bool use_tbb = true;
  blake3_hasher_update_base(self, നിവേശം_2, നിവേശം_len, use_tbb);
}
#endif // BLAKE3_USE_TBB

ശൂന്യം llvm_blake3_hasher_finalize(സ്ഥിരം blake3_hasher *self, uint8_t *out,
                            size_t out_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, out, out_len);
#എങ്കിൽ LLVM_MEMORY_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(out, out_len);
#endif
}

ശൂന്യം llvm_blake3_hasher_finalize_seek(സ്ഥിരം blake3_hasher *self, uint64_t seek,
                                 uint8_t *out, size_t out_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  എങ്കിൽ (out_len == 0) {
    തിരികെ;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  എങ്കിൽ (self->cv_അട്ടി_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_വേര്_bytes(&output, seek, out, out_len);
    തിരികെ;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_remaining;
  എങ്കിൽ (chunk_state_len(&self->chunk) > 0) {
    cvs_remaining = self->cv_അട്ടി_len;
    output = chunk_state_output(&self->chunk);
  } അല്ലെങ്കിൽ {
    // There are always at least 2 CVs in the stack in this case.
    cvs_remaining = self->cv_അട്ടി_len - 2;
    output = parent_output(&self->cv_അട്ടി[cvs_remaining * 32], self->താക്കോൽ_2,
                           self->chunk.flags);
  }
  ആകുമ്പോൾ (cvs_remaining > 0) {
    cvs_remaining -= 1;
    uint8_t parent_block[BLAKE3_BLOCK_LEN];
    memcpy(parent_block, &self->cv_അട്ടി[cvs_remaining * 32], 32);
    output_chaining_മൂല്യം(&output, &parent_block[32]);
    output = parent_output(parent_block, self->താക്കോൽ_2, self->chunk.flags);
  }
  output_വേര്_bytes(&output, seek, out, out_len);
}

ശൂന്യം llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->താക്കോൽ_2, 0);
  self->cv_അട്ടി_len = 0;
}
