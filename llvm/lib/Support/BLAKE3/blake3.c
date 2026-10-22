/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <string.h>

#ضمن "blake3_impl.h"

ثابت حرف *llvm_blake3_نسخه(فراغ) { أرجع BLAKE3_نسخه_STRING; }

INLINE فراغ chunk_state_init(blake3_chunk_state *self, ثابت uint32_t کلی_2[8],
                             uint8_t flags) {
  حافظه_نقل(self->cv, کلی_2, BLAKE3_کلی_LEN);
  self->chunk_counter = 0;
  memset(self->buf, 0, BLAKE3_بلاک_LEN);
  self->buf_len = 0;
  self->بلاکونه_compressed = 0;
  self->flags = flags;
}

INLINE فراغ chunk_state_reset(blake3_chunk_state *self, ثابت uint32_t کلی_2[8],
                              uint64_t chunk_counter) {
  حافظه_نقل(self->cv, کلی_2, BLAKE3_کلی_LEN);
  self->chunk_counter = chunk_counter;
  self->بلاکونه_compressed = 0;
  memset(self->buf, 0, BLAKE3_بلاک_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(ثابت blake3_chunk_state *self) {
  أرجع (BLAKE3_بلاک_LEN * (size_t)self->بلاکونه_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   ثابت uint8_t *ننوت_2, size_t ننوت_len) {
  size_t take = BLAKE3_بلاک_LEN - ((size_t)self->buf_len);
  إذا (take > ننوت_len) {
    take = ننوت_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  حافظه_نقل(dest, ننوت_2, take);
  self->buf_len += (uint8_t)take;
  أرجع take;
}

INLINE uint8_t chunk_state_maybe_پیل_flag(ثابت blake3_chunk_state *self) {
  إذا (self->بلاکونه_compressed == 0) {
    أرجع CHUNK_پیل;
  } وإلا {
    أرجع 0;
  }
}

تعريفنوع بنية {
  uint32_t ننوت_cv[8];
  uint64_t counter;
  uint8_t بلاک[BLAKE3_بلاک_LEN];
  uint8_t بلاک_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(ثابت uint32_t ننوت_cv[8],
                            ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                            uint8_t بلاک_len, uint64_t counter,
                            uint8_t flags) {
  output_t ret;
  حافظه_نقل(ret.ننوت_cv, ننوت_cv, 32);
  حافظه_نقل(ret.بلاک, بلاک, BLAKE3_بلاک_LEN);
  ret.بلاک_len = بلاک_len;
  ret.counter = counter;
  ret.flags = flags;
  أرجع ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE فراغ output_chaining_ارزښت(ثابت output_t *self, uint8_t cv[32]) {
  uint32_t cv_ټکي[8];
  حافظه_نقل(cv_ټکي, self->ننوت_cv, 32);
  blake3_compress_دننه_ځای_پرځای_کړه(cv_ټکي, self->بلاک, self->بلاک_len,
                           self->counter, self->flags);
  store_cv_ټکي(cv, cv_ټکي);
}

INLINE فراغ output_ریښه_بایټونه(ثابت output_t *self, uint64_t seek, uint8_t *بهر_2,
                              size_t بهر_len) {
  إذا (بهر_len == 0) {
      أرجع;
  }
  uint64_t output_بلاک_counter = seek / 64;
  size_t دننه_واټن_within_بلاک = seek % 64;
  uint8_t wide_buf[64];
  إذا(دننه_واټن_within_بلاک) {
    blake3_compress_xof(self->ننوت_cv, self->بلاک, self->بلاک_len, output_بلاک_counter, self->flags | ریښه, wide_buf);
    ثابت size_t شته_بایټونه = 64 - دننه_واټن_within_بلاک;
    ثابت size_t بایټونه = بهر_len > شته_بایټونه ? شته_بایټونه : بهر_len;
    حافظه_نقل(بهر_2, wide_buf + دننه_واټن_within_بلاک, بایټونه);
    بهر_2 += بایټونه;
    بهر_len -= بایټونه;
    output_بلاک_counter += 1;
  }
  إذا(بهر_len / 64) {
    blake3_xof_many(self->ننوت_cv, self->بلاک, self->بلاک_len, output_بلاک_counter, self->flags | ریښه, بهر_2, بهر_len / 64);
  }
  output_بلاک_counter += بهر_len / 64;
  بهر_2 += بهر_len & -64;
  بهر_len -= بهر_len & -64;
  إذا(بهر_len) {
    blake3_compress_xof(self->ننوت_cv, self->بلاک, self->بلاک_len, output_بلاک_counter, self->flags | ریښه, wide_buf);
    حافظه_نقل(بهر_2, wide_buf, بهر_len);
  }
}

INLINE فراغ chunk_state_update(blake3_chunk_state *self, ثابت uint8_t *ننوت_2,
                               size_t ننوت_len) {
  إذا (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, ننوت_2, ننوت_len);
    ننوت_2 += take;
    ننوت_len -= take;
    إذا (ننوت_len > 0) {
      blake3_compress_دننه_ځای_پرځای_کړه(
          self->cv, self->buf, BLAKE3_بلاک_LEN, self->chunk_counter,
          self->flags | chunk_state_maybe_پیل_flag(self));
      self->بلاکونه_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_بلاک_LEN);
    }
  }

  طالما (ننوت_len > BLAKE3_بلاک_LEN) {
    blake3_compress_دننه_ځای_پرځای_کړه(self->cv, ننوت_2, BLAKE3_بلاک_LEN,
                             self->chunk_counter,
                             self->flags | chunk_state_maybe_پیل_flag(self));
    self->بلاکونه_compressed += 1;
    ننوت_2 += BLAKE3_بلاک_LEN;
    ننوت_len -= BLAKE3_بلاک_LEN;
  }

  chunk_state_fill_buf(self, ننوت_2, ننوت_len);
}

INLINE output_t chunk_state_output(ثابت blake3_chunk_state *self) {
  uint8_t بلاک_flags =
      self->flags | chunk_state_maybe_پیل_flag(self) | CHUNK_پای_ټکی;
  أرجع make_output(self->cv, self->buf, self->buf_len, self->chunk_counter,
                     بلاک_flags);
}

INLINE output_t پلار_output(ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                              ثابت uint32_t کلی_2[8], uint8_t flags) {
  أرجع make_output(کلی_2, بلاک, BLAKE3_بلاک_LEN, 0, flags | پلار);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t کیڼ_subtree_len(size_t ننوت_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t ډک_chunks = (ننوت_len - 1) / BLAKE3_CHUNK_LEN;
  أرجع پړاو_ښکته_منزل_power_د_2(ډک_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(ثابت uint8_t *ننوت_2, size_t ننوت_len,
                                       ثابت uint32_t کلی_2[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t *بهر_2) {
#إذا defined(BLAKE3_TESTING)
  أكد(0 < ننوت_len);
  أكد(ننوت_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#د_که_پای

  ثابت uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t ننوت_position = 0;
  size_t chunks_array_len = 0;
  طالما (ننوت_len - ننوت_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &ننوت_2[ننوت_position];
    ننوت_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_بلاک_LEN, کلی_2, chunk_counter,
                   true, flags, CHUNK_پیل, CHUNK_پای_ټکی, بهر_2);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  إذا (ننوت_len > ننوت_position) {
    uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, کلی_2, flags);
    chunk_state.chunk_counter = counter;
    chunk_state_update(&chunk_state, &ننوت_2[ننوت_position],
                       ننوت_len - ننوت_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_ارزښت(&output, &بهر_2[chunks_array_len * BLAKE3_بهر_LEN]);
    أرجع chunks_array_len + 1;
  } وإلا {
    أرجع chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(ثابت uint8_t *ماشوم_chaining_ارزښتونه,
                                        size_t عدد_chaining_ارزښتونه,
                                        ثابت uint32_t کلی_2[8], uint8_t flags,
                                        uint8_t *بهر_2) {
#إذا defined(BLAKE3_TESTING)
  أكد(2 <= عدد_chaining_ارزښتونه);
  أكد(عدد_chaining_ارزښتونه <= 2 * MAX_SIMD_DEGREE_OR_2);
#د_که_پای

  ثابت uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  طالما (عدد_chaining_ارزښتونه - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &ماشوم_chaining_ارزښتونه[2 * parents_array_len * BLAKE3_بهر_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, کلی_2,
                   0, // Parents always use counter 0.
                   false, flags | پلار,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   بهر_2);

  // If there's an odd child left over, it becomes an output.
  إذا (عدد_chaining_ارزښتونه > 2 * parents_array_len) {
    حافظه_نقل(&بهر_2[parents_array_len * BLAKE3_بهر_LEN],
           &ماشوم_chaining_ارزښتونه[2 * parents_array_len * BLAKE3_بهر_LEN],
           BLAKE3_بهر_LEN);
    أرجع parents_array_len + 1;
  } وإلا {
    أرجع parents_array_len;
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
size_t blake3_compress_subtree_wide(ثابت uint8_t *ننوت_2, size_t ننوت_len,
                                    ثابت uint32_t کلی_2[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t *بهر_2, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  إذا (ننوت_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    أرجع compress_chunks_parallel(ننوت_2, ننوت_len, کلی_2, chunk_counter, flags,
                                    بهر_2);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t کیڼ_ننوت_len = کیڼ_subtree_len(ننوت_len);
  size_t ښی_ننوت_len = ننوت_len - کیڼ_ننوت_len;
  ثابت uint8_t *ښی_ننوت = &ننوت_2[کیڼ_ننوت_len];
  uint64_t ښی_chunk_counter =
      chunk_counter + (uint64_t)(کیڼ_ننوت_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_بهر_LEN];
  size_t degree = blake3_simd_degree();
  إذا (کیڼ_ننوت_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *ښی_cvs = &cv_array[degree * BLAKE3_بهر_LEN];

  // Recurse!
  size_t کیڼ_n = -1;
  size_t ښی_n = -1;

#إذا defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      کلی_2, flags, use_tbb,
      // left-hand side
      ننوت_2, کیڼ_ننوت_len, chunk_counter, cv_array, &کیڼ_n,
      // right-hand side
      ښی_ننوت, ښی_ننوت_len, ښی_chunk_counter, ښی_cvs, &ښی_n);
#وإلا
  کیڼ_n = blake3_compress_subtree_wide(
      ننوت_2, کیڼ_ننوت_len, کلی_2, chunk_counter, flags, cv_array, use_tbb);
  ښی_n = blake3_compress_subtree_wide(ښی_ننوت, ښی_ننوت_len, کلی_2,
                                         ښی_chunk_counter, flags, ښی_cvs,
                                         use_tbb);
#د_که_پای // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  إذا (کیڼ_n == 1) {
    حافظه_نقل(بهر_2, cv_array, 2 * BLAKE3_بهر_LEN);
    أرجع 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t عدد_chaining_ارزښتونه = کیڼ_n + ښی_n;
  أرجع compress_parents_parallel(cv_array, عدد_chaining_ارزښتونه, کلی_2, flags,
                                   بهر_2);
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
INLINE فراغ
compress_subtree_منزل_پلار_غوټه(ثابت uint8_t *ننوت_2, size_t ننوت_len,
                                ثابت uint32_t کلی_2[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t بهر_2[2 * BLAKE3_بهر_LEN],
                                bool use_tbb) {
#إذا defined(BLAKE3_TESTING)
  أكد(ننوت_len > BLAKE3_CHUNK_LEN);
#د_که_پای

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_بهر_LEN];
  size_t عدد_cvs = blake3_compress_subtree_wide(ننوت_2, ننوت_len, کلی_2,
                                                chunk_counter, flags, cv_array, use_tbb);
  أكد(عدد_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#إذا MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t بهر_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_بهر_LEN / 2];
  طالما (عدد_cvs > 2) {
    عدد_cvs =
        compress_parents_parallel(cv_array, عدد_cvs, کلی_2, flags, بهر_array);
    حافظه_نقل(cv_array, بهر_array, عدد_cvs * BLAKE3_بهر_LEN);
  }
#د_که_پای
  حافظه_نقل(بهر_2, cv_array, 2 * BLAKE3_بهر_LEN);
}

INLINE فراغ hasher_init_بنسټ(blake3_hasher *self, ثابت uint32_t کلی_2[8],
                             uint8_t flags) {
  حافظه_نقل(self->کلی_2, کلی_2, BLAKE3_کلی_LEN);
  chunk_state_init(&self->chunk, کلی_2, flags);
  self->cv_ډېری_len = 0;
}

فراغ llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_بنسټ(self, IV, 0); }

فراغ llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              ثابت uint8_t کلی_2[BLAKE3_کلی_LEN]) {
  uint32_t کلی_ټکي[8];
  بارول_کلی_ټکي(کلی_2, کلی_ټکي);
  hasher_init_بنسټ(self, کلی_ټکي, KEYED_HASH);
}

فراغ llvm_blake3_hasher_init_derive_کلی_raw(blake3_hasher *self, ثابت فراغ *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_بنسټ(&context_hasher, IV, DERIVE_کلی_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_کلی[BLAKE3_کلی_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_کلی, BLAKE3_کلی_LEN);
  uint32_t context_کلی_ټکي[8];
  بارول_کلی_ټکي(context_کلی, context_کلی_ټکي);
  hasher_init_بنسټ(self, context_کلی_ټکي, DERIVE_کلی_MATERIAL);
}

فراغ llvm_blake3_hasher_init_derive_کلی(blake3_hasher *self, ثابت حرف *context) {
  llvm_blake3_hasher_init_derive_کلی_raw(self, context, مزي_اوږدوالی(context));
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
INLINE فراغ hasher_یوځای_cv_ډېری(blake3_hasher *self, uint64_t ټول_len) {
  size_t post_یوځای_ډېری_len = (size_t)popcnt(ټول_len);
  طالما (self->cv_ډېری_len > post_یوځای_ډېری_len) {
    uint8_t *پلار_غوټه =
        &self->cv_ډېری[(self->cv_ډېری_len - 2) * BLAKE3_بهر_LEN];
    output_t output = پلار_output(پلار_غوټه, self->کلی_2, self->chunk.flags);
    output_chaining_ارزښت(&output, پلار_غوټه);
    self->cv_ډېری_len -= 1;
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
INLINE فراغ hasher_ټېل_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_بهر_LEN],
                           uint64_t chunk_counter) {
  hasher_یوځای_cv_ډېری(self, chunk_counter);
  حافظه_نقل(&self->cv_ډېری[self->cv_ډېری_len * BLAKE3_بهر_LEN], new_cv,
         BLAKE3_بهر_LEN);
  self->cv_ډېری_len += 1;
}

INLINE فراغ blake3_hasher_update_بنسټ(blake3_hasher *self, ثابت فراغ *ننوت_2,
                                      size_t ننوت_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  إذا (ننوت_len == 0) {
    أرجع;
  }

  ثابت uint8_t *ننوت_بایټونه = (ثابت uint8_t *)ننوت_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  إذا (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    إذا (take > ننوت_len) {
      take = ننوت_len;
    }
    chunk_state_update(&self->chunk, ننوت_بایټونه, take);
    ننوت_بایټونه += take;
    ننوت_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    إذا (ننوت_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_ارزښت(&output, chunk_cv);
      hasher_ټېل_cv(self, chunk_cv, self->chunk.chunk_counter);
      chunk_state_reset(&self->chunk, self->کلی_2, self->chunk.chunk_counter + 1);
    } وإلا {
      أرجع;
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
  طالما (ننوت_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = پړاو_ښکته_منزل_power_د_2(ننوت_len);
    uint64_t شمېر_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
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
    طالما ((((uint64_t)(subtree_len - 1)) & شمېر_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    إذا (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->کلی_2, self->chunk.flags);
      chunk_state.chunk_counter = self->chunk.chunk_counter;
      chunk_state_update(&chunk_state, ننوت_بایټونه, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_بهر_LEN];
      output_chaining_ارزښت(&output, cv);
      hasher_ټېل_cv(self, cv, chunk_state.chunk_counter);
    } وإلا {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_بهر_LEN];
      compress_subtree_منزل_پلار_غوټه(ننوت_بایټونه, subtree_len, self->کلی_2,
                                      self->chunk.chunk_counter,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_ټېل_cv(self, cv_pair, self->chunk.chunk_counter);
      hasher_ټېل_cv(self, &cv_pair[BLAKE3_بهر_LEN],
                     self->chunk.chunk_counter + (subtree_chunks / 2));
    }
    self->chunk.chunk_counter += subtree_chunks;
    ننوت_بایټونه += subtree_len;
    ننوت_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  إذا (ننوت_len > 0) {
    chunk_state_update(&self->chunk, ننوت_بایټونه, ننوت_len);
    hasher_یوځای_cv_ډېری(self, self->chunk.chunk_counter);
  }
}

فراغ llvm_blake3_hasher_update(blake3_hasher *self, ثابت فراغ *ننوت_2,
                          size_t ننوت_len) {
  bool use_tbb = false;
  blake3_hasher_update_بنسټ(self, ننوت_2, ننوت_len, use_tbb);
}

#إذا defined(BLAKE3_USE_TBB)
فراغ blake3_hasher_update_tbb(blake3_hasher *self, ثابت فراغ *ننوت_2,
                              size_t ننوت_len) {
  bool use_tbb = true;
  blake3_hasher_update_بنسټ(self, ننوت_2, ننوت_len, use_tbb);
}
#د_که_پای // BLAKE3_USE_TBB

فراغ llvm_blake3_hasher_finalize(ثابت blake3_hasher *self, uint8_t *بهر_2,
                            size_t بهر_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, بهر_2, بهر_len);
#إذا LLVM_حافظه_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(بهر_2, بهر_len);
#د_که_پای
}

فراغ llvm_blake3_hasher_finalize_seek(ثابت blake3_hasher *self, uint64_t seek,
                                 uint8_t *بهر_2, size_t بهر_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  إذا (بهر_len == 0) {
    أرجع;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  إذا (self->cv_ډېری_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_ریښه_بایټونه(&output, seek, بهر_2, بهر_len);
    أرجع;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_پاتې_شوی;
  إذا (chunk_state_len(&self->chunk) > 0) {
    cvs_پاتې_شوی = self->cv_ډېری_len;
    output = chunk_state_output(&self->chunk);
  } وإلا {
    // There are always at least 2 CVs in the stack in this case.
    cvs_پاتې_شوی = self->cv_ډېری_len - 2;
    output = پلار_output(&self->cv_ډېری[cvs_پاتې_شوی * 32], self->کلی_2,
                           self->chunk.flags);
  }
  طالما (cvs_پاتې_شوی > 0) {
    cvs_پاتې_شوی -= 1;
    uint8_t پلار_بلاک[BLAKE3_بلاک_LEN];
    حافظه_نقل(پلار_بلاک, &self->cv_ډېری[cvs_پاتې_شوی * 32], 32);
    output_chaining_ارزښت(&output, &پلار_بلاک[32]);
    output = پلار_output(پلار_بلاک, self->کلی_2, self->chunk.flags);
  }
  output_ریښه_بایټونه(&output, seek, بهر_2, بهر_len);
}

فراغ llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->کلی_2, 0);
  self->cv_ډېری_len = 0;
}
