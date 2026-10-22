/*===-- blake3.c - BLAKE3 C Implementation ------------------------*- C -*-===*\
|*                                                                            *|
|* Released into the public domain with CC0 1.0                               *|
|* See 'llvm/lib/Support/BLAKE3/LICENSE' for info.                            *|
|* SPDX-License-Identifier: CC0-1.0                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>

#شامل "blake3_impl.h"

ثابت حرف *llvm_blake3_نسخہ(خالی) { واپس BLAKE3_نسخہ_STRING; }

INLINE خالی chunk_state_init(blake3_chunk_state *self, ثابت uint32_t کلید_2[8],
                             uint8_t flags) {
  حافظہ_نقل(self->cv, کلید_2, BLAKE3_کلید_LEN);
  self->chunk_شمار = 0;
  memset(self->buf, 0, BLAKE3_بلاک_LEN);
  self->buf_len = 0;
  self->بلاکیں_compressed = 0;
  self->flags = flags;
}

INLINE خالی chunk_state_reset(blake3_chunk_state *self, ثابت uint32_t کلید_2[8],
                              uint64_t chunk_شمار) {
  حافظہ_نقل(self->cv, کلید_2, BLAKE3_کلید_LEN);
  self->chunk_شمار = chunk_شمار;
  self->بلاکیں_compressed = 0;
  memset(self->buf, 0, BLAKE3_بلاک_LEN);
  self->buf_len = 0;
}

INLINE size_t chunk_state_len(ثابت blake3_chunk_state *self) {
  واپس (BLAKE3_بلاک_LEN * (size_t)self->بلاکیں_compressed) +
         ((size_t)self->buf_len);
}

INLINE size_t chunk_state_fill_buf(blake3_chunk_state *self,
                                   ثابت uint8_t *داخلہ_2, size_t داخلہ_len) {
  size_t take = BLAKE3_بلاک_LEN - ((size_t)self->buf_len);
  اگر (take > داخلہ_len) {
    take = داخلہ_len;
  }
  uint8_t *dest = self->buf + ((size_t)self->buf_len);
  حافظہ_نقل(dest, داخلہ_2, take);
  self->buf_len += (uint8_t)take;
  واپس take;
}

INLINE uint8_t chunk_state_maybe_آغاز_flag(ثابت blake3_chunk_state *self) {
  اگر (self->بلاکیں_compressed == 0) {
    واپس CHUNK_آغاز;
  } ورنہ {
    واپس 0;
  }
}

قسمتعریف ساخت {
  uint32_t داخلہ_cv[8];
  uint64_t شمار_3;
  uint8_t بلاک[BLAKE3_بلاک_LEN];
  uint8_t بلاک_len;
  uint8_t flags;
} output_t;

INLINE output_t make_output(ثابت uint32_t داخلہ_cv[8],
                            ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                            uint8_t بلاک_len, uint64_t شمار_3,
                            uint8_t flags) {
  output_t ret;
  حافظہ_نقل(ret.داخلہ_cv, داخلہ_cv, 32);
  حافظہ_نقل(ret.بلاک, بلاک, BLAKE3_بلاک_LEN);
  ret.بلاک_len = بلاک_len;
  ret.شمار_3 = شمار_3;
  ret.flags = flags;
  واپس ret;
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
INLINE خالی output_chaining_قدر(ثابت output_t *self, uint8_t cv[32]) {
  uint32_t cv_الفاظ[8];
  حافظہ_نقل(cv_الفاظ, self->داخلہ_cv, 32);
  blake3_compress_اندر_رکھو(cv_الفاظ, self->بلاک, self->بلاک_len,
                           self->شمار_3, self->flags);
  store_cv_الفاظ(cv, cv_الفاظ);
}

INLINE خالی output_جڑ_بائٹیں(ثابت output_t *self, uint64_t seek, uint8_t *برآمد_2,
                              size_t برآمد_len) {
  اگر (برآمد_len == 0) {
      واپس;
  }
  uint64_t output_بلاک_شمار = seek / 64;
  size_t انحراف_within_بلاک = seek % 64;
  uint8_t wide_buf[64];
  اگر(انحراف_within_بلاک) {
    blake3_compress_xof(self->داخلہ_cv, self->بلاک, self->بلاک_len, output_بلاک_شمار, self->flags | جڑ, wide_buf);
    ثابت size_t دستیاب_بائٹیں = 64 - انحراف_within_بلاک;
    ثابت size_t بائٹیں = برآمد_len > دستیاب_بائٹیں ? دستیاب_بائٹیں : برآمد_len;
    حافظہ_نقل(برآمد_2, wide_buf + انحراف_within_بلاک, بائٹیں);
    برآمد_2 += بائٹیں;
    برآمد_len -= بائٹیں;
    output_بلاک_شمار += 1;
  }
  اگر(برآمد_len / 64) {
    blake3_xof_many(self->داخلہ_cv, self->بلاک, self->بلاک_len, output_بلاک_شمار, self->flags | جڑ, برآمد_2, برآمد_len / 64);
  }
  output_بلاک_شمار += برآمد_len / 64;
  برآمد_2 += برآمد_len & -64;
  برآمد_len -= برآمد_len & -64;
  اگر(برآمد_len) {
    blake3_compress_xof(self->داخلہ_cv, self->بلاک, self->بلاک_len, output_بلاک_شمار, self->flags | جڑ, wide_buf);
    حافظہ_نقل(برآمد_2, wide_buf, برآمد_len);
  }
}

INLINE خالی chunk_state_update(blake3_chunk_state *self, ثابت uint8_t *داخلہ_2,
                               size_t داخلہ_len) {
  اگر (self->buf_len > 0) {
    size_t take = chunk_state_fill_buf(self, داخلہ_2, داخلہ_len);
    داخلہ_2 += take;
    داخلہ_len -= take;
    اگر (داخلہ_len > 0) {
      blake3_compress_اندر_رکھو(
          self->cv, self->buf, BLAKE3_بلاک_LEN, self->chunk_شمار,
          self->flags | chunk_state_maybe_آغاز_flag(self));
      self->بلاکیں_compressed += 1;
      self->buf_len = 0;
      memset(self->buf, 0, BLAKE3_بلاک_LEN);
    }
  }

  جب (داخلہ_len > BLAKE3_بلاک_LEN) {
    blake3_compress_اندر_رکھو(self->cv, داخلہ_2, BLAKE3_بلاک_LEN,
                             self->chunk_شمار,
                             self->flags | chunk_state_maybe_آغاز_flag(self));
    self->بلاکیں_compressed += 1;
    داخلہ_2 += BLAKE3_بلاک_LEN;
    داخلہ_len -= BLAKE3_بلاک_LEN;
  }

  chunk_state_fill_buf(self, داخلہ_2, داخلہ_len);
}

INLINE output_t chunk_state_output(ثابت blake3_chunk_state *self) {
  uint8_t بلاک_flags =
      self->flags | chunk_state_maybe_آغاز_flag(self) | CHUNK_انتہا;
  واپس make_output(self->cv, self->buf, self->buf_len, self->chunk_شمار,
                     بلاک_flags);
}

INLINE output_t والد_output(ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                              ثابت uint32_t کلید_2[8], uint8_t flags) {
  واپس make_output(کلید_2, بلاک, BLAKE3_بلاک_LEN, 0, flags | والد);
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
INLINE size_t بایاں_subtree_len(size_t داخلہ_len) {
  // Subtract 1 to reserve at least one byte for the right side. input_len
  // should always be greater than BLAKE3_CHUNK_LEN.
  size_t بھرا_chunks = (داخلہ_len - 1) / BLAKE3_CHUNK_LEN;
  واپس باری_نیچے_کو_مقصد_power_کا_2(بھرا_chunks) * BLAKE3_CHUNK_LEN;
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
INLINE size_t compress_chunks_parallel(ثابت uint8_t *داخلہ_2, size_t داخلہ_len,
                                       ثابت uint32_t کلید_2[8],
                                       uint64_t chunk_شمار, uint8_t flags,
                                       uint8_t *برآمد_2) {
#اگر defined(BLAKE3_TESTING)
  أكد(0 < داخلہ_len);
  أكد(داخلہ_len <= MAX_SIMD_DEGREE * BLAKE3_CHUNK_LEN);
#اگر_ختم

  ثابت uint8_t *chunks_array[MAX_SIMD_DEGREE];
  size_t داخلہ_position = 0;
  size_t chunks_array_len = 0;
  جب (داخلہ_len - داخلہ_position >= BLAKE3_CHUNK_LEN) {
    chunks_array[chunks_array_len] = &داخلہ_2[داخلہ_position];
    داخلہ_position += BLAKE3_CHUNK_LEN;
    chunks_array_len += 1;
  }

  blake3_hash_many(chunks_array, chunks_array_len,
                   BLAKE3_CHUNK_LEN / BLAKE3_بلاک_LEN, کلید_2, chunk_شمار,
                   true, flags, CHUNK_آغاز, CHUNK_انتہا, برآمد_2);

  // Hash the remaining partial chunk, if there is one. Note that the empty
  // chunk (meaning the empty message) is a different codepath.
  اگر (داخلہ_len > داخلہ_position) {
    uint64_t شمار_3 = chunk_شمار + (uint64_t)chunks_array_len;
    blake3_chunk_state chunk_state;
    chunk_state_init(&chunk_state, کلید_2, flags);
    chunk_state.chunk_شمار = شمار_3;
    chunk_state_update(&chunk_state, &داخلہ_2[داخلہ_position],
                       داخلہ_len - داخلہ_position);
    output_t output = chunk_state_output(&chunk_state);
    output_chaining_قدر(&output, &برآمد_2[chunks_array_len * BLAKE3_برآمد_LEN]);
    واپس chunks_array_len + 1;
  } ورنہ {
    واپس chunks_array_len;
  }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
INLINE size_t compress_parents_parallel(ثابت uint8_t *بچہ_chaining_اقدار,
                                        size_t عدد_chaining_اقدار,
                                        ثابت uint32_t کلید_2[8], uint8_t flags,
                                        uint8_t *برآمد_2) {
#اگر defined(BLAKE3_TESTING)
  أكد(2 <= عدد_chaining_اقدار);
  أكد(عدد_chaining_اقدار <= 2 * MAX_SIMD_DEGREE_OR_2);
#اگر_ختم

  ثابت uint8_t *parents_array[MAX_SIMD_DEGREE_OR_2];
  size_t parents_array_len = 0;
  جب (عدد_chaining_اقدار - (2 * parents_array_len) >= 2) {
    parents_array[parents_array_len] =
        &بچہ_chaining_اقدار[2 * parents_array_len * BLAKE3_برآمد_LEN];
    parents_array_len += 1;
  }

  blake3_hash_many(parents_array, parents_array_len, 1, کلید_2,
                   0, // Parents always use counter 0.
                   false, flags | والد,
                   0, // Parents have no start flags.
                   0, // Parents have no end flags.
                   برآمد_2);

  // If there's an odd child left over, it becomes an output.
  اگر (عدد_chaining_اقدار > 2 * parents_array_len) {
    حافظہ_نقل(&برآمد_2[parents_array_len * BLAKE3_برآمد_LEN],
           &بچہ_chaining_اقدار[2 * parents_array_len * BLAKE3_برآمد_LEN],
           BLAKE3_برآمد_LEN);
    واپس parents_array_len + 1;
  } ورنہ {
    واپس parents_array_len;
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
size_t blake3_compress_subtree_wide(ثابت uint8_t *داخلہ_2, size_t داخلہ_len,
                                    ثابت uint32_t کلید_2[8],
                                    uint64_t chunk_شمار, uint8_t flags,
                                    uint8_t *برآمد_2, bool use_tbb) {
  // Note that the single chunk case does *not* bump the SIMD degree up to 2
  // when it is 1. If this implementation adds multi-threading in the future,
  // this gives us the option of multi-threading even the 2-chunk case, which
  // can help performance on smaller platforms.
  اگر (داخلہ_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
    واپس compress_chunks_parallel(داخلہ_2, داخلہ_len, کلید_2, chunk_شمار, flags,
                                    برآمد_2);
  }

  // With more than simd_degree chunks, we need to recurse. Start by dividing
  // the input into left and right subtrees. (Note that this is only optimal
  // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
  // of 3 or something, we'll need a more complicated strategy.)
  size_t بایاں_داخلہ_len = بایاں_subtree_len(داخلہ_len);
  size_t دایاں_داخلہ_len = داخلہ_len - بایاں_داخلہ_len;
  ثابت uint8_t *دایاں_داخلہ = &داخلہ_2[بایاں_داخلہ_len];
  uint64_t دایاں_chunk_شمار =
      chunk_شمار + (uint64_t)(بایاں_داخلہ_len / BLAKE3_CHUNK_LEN);

  // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
  // account for the special case of returning 2 outputs when the SIMD degree
  // is 1.
  uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_برآمد_LEN];
  size_t degree = blake3_simd_degree();
  اگر (بایاں_داخلہ_len > BLAKE3_CHUNK_LEN && degree == 1) {
    // The special case: We always use a degree of at least two, to make
    // sure there are two outputs. Except, as noted above, at the chunk
    // level, where we allow degree=1. (Note that the 1-chunk-input case is
    // a different codepath.)
    degree = 2;
  }
  uint8_t *دایاں_cvs = &cv_array[degree * BLAKE3_برآمد_LEN];

  // Recurse!
  size_t بایاں_n = -1;
  size_t دایاں_n = -1;

#اگر defined(BLAKE3_USE_TBB)
  blake3_compress_subtree_wide_join_tbb(
      کلید_2, flags, use_tbb,
      // left-hand side
      داخلہ_2, بایاں_داخلہ_len, chunk_شمار, cv_array, &بایاں_n,
      // right-hand side
      دایاں_داخلہ, دایاں_داخلہ_len, دایاں_chunk_شمار, دایاں_cvs, &دایاں_n);
#ورنہ
  بایاں_n = blake3_compress_subtree_wide(
      داخلہ_2, بایاں_داخلہ_len, کلید_2, chunk_شمار, flags, cv_array, use_tbb);
  دایاں_n = blake3_compress_subtree_wide(دایاں_داخلہ, دایاں_داخلہ_len, کلید_2,
                                         دایاں_chunk_شمار, flags, دایاں_cvs,
                                         use_tbb);
#اگر_ختم // BLAKE3_USE_TBB

  // The special case again. If simd_degree=1, then we'll have left_n=1 and
  // right_n=1. Rather than compressing them into a single output, return
  // them directly, to make sure we always have at least two outputs.
  اگر (بایاں_n == 1) {
    حافظہ_نقل(برآمد_2, cv_array, 2 * BLAKE3_برآمد_LEN);
    واپس 2;
  }

  // Otherwise, do one layer of parent node compression.
  size_t عدد_chaining_اقدار = بایاں_n + دایاں_n;
  واپس compress_parents_parallel(cv_array, عدد_chaining_اقدار, کلید_2, flags,
                                   برآمد_2);
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
INLINE خالی
compress_subtree_مقصد_والد_گرہ(ثابت uint8_t *داخلہ_2, size_t داخلہ_len,
                                ثابت uint32_t کلید_2[8], uint64_t chunk_شمار,
                                uint8_t flags, uint8_t برآمد_2[2 * BLAKE3_برآمد_LEN],
                                bool use_tbb) {
#اگر defined(BLAKE3_TESTING)
  أكد(داخلہ_len > BLAKE3_CHUNK_LEN);
#اگر_ختم

  uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_برآمد_LEN];
  size_t عدد_cvs = blake3_compress_subtree_wide(داخلہ_2, داخلہ_len, کلید_2,
                                                chunk_شمار, flags, cv_array, use_tbb);
  أكد(عدد_cvs <= MAX_SIMD_DEGREE_OR_2);
  // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
  // as we just asserted, num_cvs will always be <=2 in that case. But GCC
  // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
  // set then it emits incorrect warnings here. We tried a few different
  // hacks to silence these, but in the end our hacks just produced different
  // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
  // desperation, we ifdef out this entire loop when we know it's not needed.
#اگر MAX_SIMD_DEGREE_OR_2 > 2
  // If MAX_SIMD_DEGREE_OR_2 is greater than 2 and there's enough input,
  // compress_subtree_wide() returns more than 2 chaining values. Condense
  // them into 2 by forming parent nodes repeatedly.
  uint8_t برآمد_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_برآمد_LEN / 2];
  جب (عدد_cvs > 2) {
    عدد_cvs =
        compress_parents_parallel(cv_array, عدد_cvs, کلید_2, flags, برآمد_array);
    حافظہ_نقل(cv_array, برآمد_array, عدد_cvs * BLAKE3_برآمد_LEN);
  }
#اگر_ختم
  حافظہ_نقل(برآمد_2, cv_array, 2 * BLAKE3_برآمد_LEN);
}

INLINE خالی hasher_init_بنیاد(blake3_hasher *self, ثابت uint32_t کلید_2[8],
                             uint8_t flags) {
  حافظہ_نقل(self->کلید_2, کلید_2, BLAKE3_کلید_LEN);
  chunk_state_init(&self->chunk, کلید_2, flags);
  self->cv_ڈھیر_len = 0;
}

خالی llvm_blake3_hasher_init(blake3_hasher *self) { hasher_init_بنیاد(self, IV, 0); }

خالی llvm_blake3_hasher_init_keyed(blake3_hasher *self,
                              ثابت uint8_t کلید_2[BLAKE3_کلید_LEN]) {
  uint32_t کلید_الفاظ[8];
  بھرو_کلید_الفاظ(کلید_2, کلید_الفاظ);
  hasher_init_بنیاد(self, کلید_الفاظ, KEYED_HASH);
}

خالی llvm_blake3_hasher_init_derive_کلید_raw(blake3_hasher *self, ثابت خالی *context,
                                       size_t context_len) {
  blake3_hasher context_hasher;
  hasher_init_بنیاد(&context_hasher, IV, DERIVE_کلید_CONTEXT);
  llvm_blake3_hasher_update(&context_hasher, context, context_len);
  uint8_t context_کلید[BLAKE3_کلید_LEN];
  llvm_blake3_hasher_finalize(&context_hasher, context_کلید, BLAKE3_کلید_LEN);
  uint32_t context_کلید_الفاظ[8];
  بھرو_کلید_الفاظ(context_کلید, context_کلید_الفاظ);
  hasher_init_بنیاد(self, context_کلید_الفاظ, DERIVE_کلید_MATERIAL);
}

خالی llvm_blake3_hasher_init_derive_کلید(blake3_hasher *self, ثابت حرف *context) {
  llvm_blake3_hasher_init_derive_کلید_raw(self, context, لڑی_لمبائی(context));
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
INLINE خالی hasher_ملاؤ_cv_ڈھیر(blake3_hasher *self, uint64_t کل_len) {
  size_t post_ملاؤ_ڈھیر_len = (size_t)popcnt(کل_len);
  جب (self->cv_ڈھیر_len > post_ملاؤ_ڈھیر_len) {
    uint8_t *والد_گرہ =
        &self->cv_ڈھیر[(self->cv_ڈھیر_len - 2) * BLAKE3_برآمد_LEN];
    output_t output = والد_output(والد_گرہ, self->کلید_2, self->chunk.flags);
    output_chaining_قدر(&output, والد_گرہ);
    self->cv_ڈھیر_len -= 1;
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
INLINE خالی hasher_ڈالو_cv(blake3_hasher *self, uint8_t new_cv[BLAKE3_برآمد_LEN],
                           uint64_t chunk_شمار) {
  hasher_ملاؤ_cv_ڈھیر(self, chunk_شمار);
  حافظہ_نقل(&self->cv_ڈھیر[self->cv_ڈھیر_len * BLAKE3_برآمد_LEN], new_cv,
         BLAKE3_برآمد_LEN);
  self->cv_ڈھیر_len += 1;
}

INLINE خالی blake3_hasher_update_بنیاد(blake3_hasher *self, ثابت خالی *داخلہ_2,
                                      size_t داخلہ_len, bool use_tbb) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_update(&hasher, v.data(), v.size());
  اگر (داخلہ_len == 0) {
    واپس;
  }

  ثابت uint8_t *داخلہ_بائٹیں = (ثابت uint8_t *)داخلہ_2;

  // If we have some partial chunk bytes in the internal chunk_state, we need
  // to finish that chunk first.
  اگر (chunk_state_len(&self->chunk) > 0) {
    size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
    اگر (take > داخلہ_len) {
      take = داخلہ_len;
    }
    chunk_state_update(&self->chunk, داخلہ_بائٹیں, take);
    داخلہ_بائٹیں += take;
    داخلہ_len -= take;
    // If we've filled the current chunk and there's more coming, finalize this
    // chunk and proceed. In this case we know it's not the root.
    اگر (داخلہ_len > 0) {
      output_t output = chunk_state_output(&self->chunk);
      uint8_t chunk_cv[32];
      output_chaining_قدر(&output, chunk_cv);
      hasher_ڈالو_cv(self, chunk_cv, self->chunk.chunk_شمار);
      chunk_state_reset(&self->chunk, self->کلید_2, self->chunk.chunk_شمار + 1);
    } ورنہ {
      واپس;
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
  جب (داخلہ_len > BLAKE3_CHUNK_LEN) {
    size_t subtree_len = باری_نیچے_کو_مقصد_power_کا_2(داخلہ_len);
    uint64_t گنتی_so_far = self->chunk.chunk_شمار * BLAKE3_CHUNK_LEN;
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
    جب ((((uint64_t)(subtree_len - 1)) & گنتی_so_far) != 0) {
      subtree_len /= 2;
    }
    // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
    // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
    uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
    اگر (subtree_len <= BLAKE3_CHUNK_LEN) {
      blake3_chunk_state chunk_state;
      chunk_state_init(&chunk_state, self->کلید_2, self->chunk.flags);
      chunk_state.chunk_شمار = self->chunk.chunk_شمار;
      chunk_state_update(&chunk_state, داخلہ_بائٹیں, subtree_len);
      output_t output = chunk_state_output(&chunk_state);
      uint8_t cv[BLAKE3_برآمد_LEN];
      output_chaining_قدر(&output, cv);
      hasher_ڈالو_cv(self, cv, chunk_state.chunk_شمار);
    } ورنہ {
      // This is the high-performance happy path, though getting here depends
      // on the caller giving us a long enough input.
      uint8_t cv_pair[2 * BLAKE3_برآمد_LEN];
      compress_subtree_مقصد_والد_گرہ(داخلہ_بائٹیں, subtree_len, self->کلید_2,
                                      self->chunk.chunk_شمار,
                                      self->chunk.flags, cv_pair, use_tbb);
      hasher_ڈالو_cv(self, cv_pair, self->chunk.chunk_شمار);
      hasher_ڈالو_cv(self, &cv_pair[BLAKE3_برآمد_LEN],
                     self->chunk.chunk_شمار + (subtree_chunks / 2));
    }
    self->chunk.chunk_شمار += subtree_chunks;
    داخلہ_بائٹیں += subtree_len;
    داخلہ_len -= subtree_len;
  }

  // If there's any remaining input less than a full chunk, add it to the chunk
  // state. In that case, also do a final merge loop to make sure the subtree
  // stack doesn't contain any unmerged pairs. The remaining input means we
  // know these merges are non-root. This merge loop isn't strictly necessary
  // here, because hasher_push_chunk_cv already does its own merge loop, but it
  // simplifies blake3_hasher_finalize below.
  اگر (داخلہ_len > 0) {
    chunk_state_update(&self->chunk, داخلہ_بائٹیں, داخلہ_len);
    hasher_ملاؤ_cv_ڈھیر(self, self->chunk.chunk_شمار);
  }
}

خالی llvm_blake3_hasher_update(blake3_hasher *self, ثابت خالی *داخلہ_2,
                          size_t داخلہ_len) {
  bool use_tbb = false;
  blake3_hasher_update_بنیاد(self, داخلہ_2, داخلہ_len, use_tbb);
}

#اگر defined(BLAKE3_USE_TBB)
خالی blake3_hasher_update_tbb(blake3_hasher *self, ثابت خالی *داخلہ_2,
                              size_t داخلہ_len) {
  bool use_tbb = true;
  blake3_hasher_update_بنیاد(self, داخلہ_2, داخلہ_len, use_tbb);
}
#اگر_ختم // BLAKE3_USE_TBB

خالی llvm_blake3_hasher_finalize(ثابت blake3_hasher *self, uint8_t *برآمد_2,
                            size_t برآمد_len) {
  llvm_blake3_hasher_finalize_seek(self, 0, برآمد_2, برآمد_len);
#اگر LLVM_حافظہ_SANITIZER_BUILD
  // Avoid false positives due to uninstrumented assembly code.
  __msan_unpoison(برآمد_2, برآمد_len);
#اگر_ختم
}

خالی llvm_blake3_hasher_finalize_seek(ثابت blake3_hasher *self, uint64_t seek,
                                 uint8_t *برآمد_2, size_t برآمد_len) {
  // Explicitly checking for zero avoids causing UB by passing a null pointer
  // to memcpy. This comes up in practice with things like:
  //   std::vector<uint8_t> v;
  //   blake3_hasher_finalize(&hasher, v.data(), v.size());
  اگر (برآمد_len == 0) {
    واپس;
  }

  // If the subtree stack is empty, then the current chunk is the root.
  اگر (self->cv_ڈھیر_len == 0) {
    output_t output = chunk_state_output(&self->chunk);
    output_جڑ_بائٹیں(&output, seek, برآمد_2, برآمد_len);
    واپس;
  }
  // If there are any bytes in the chunk state, finalize that chunk and do a
  // roll-up merge between that chunk hash and every subtree in the stack. In
  // this case, the extra merge loop at the end of blake3_hasher_update
  // guarantees that none of the subtrees in the stack need to be merged with
  // each other first. Otherwise, if there are no bytes in the chunk state,
  // then the top of the stack is a chunk hash, and we start the merge from
  // that.
  output_t output;
  size_t cvs_بقایا;
  اگر (chunk_state_len(&self->chunk) > 0) {
    cvs_بقایا = self->cv_ڈھیر_len;
    output = chunk_state_output(&self->chunk);
  } ورنہ {
    // There are always at least 2 CVs in the stack in this case.
    cvs_بقایا = self->cv_ڈھیر_len - 2;
    output = والد_output(&self->cv_ڈھیر[cvs_بقایا * 32], self->کلید_2,
                           self->chunk.flags);
  }
  جب (cvs_بقایا > 0) {
    cvs_بقایا -= 1;
    uint8_t والد_بلاک[BLAKE3_بلاک_LEN];
    حافظہ_نقل(والد_بلاک, &self->cv_ڈھیر[cvs_بقایا * 32], 32);
    output_chaining_قدر(&output, &والد_بلاک[32]);
    output = والد_output(والد_بلاک, self->کلید_2, self->chunk.flags);
  }
  output_جڑ_بائٹیں(&output, seek, برآمد_2, برآمد_len);
}

خالی llvm_blake3_hasher_reset(blake3_hasher *self) {
  chunk_state_reset(&self->chunk, self->کلید_2, 0);
  self->cv_ڈھیر_len = 0;
}
