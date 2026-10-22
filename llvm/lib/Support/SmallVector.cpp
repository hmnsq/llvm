//===- llvm/ADT/SmallVector.cpp - 'Normally small' vectors ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SmallVector class.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemAlloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ENABLE_EXCEPTIONS
#ਅਨ੍ਤਰ੍ਭਾਵ <stdexcept>
#ਜੇ_ਅੰਤ
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// Check that no bytes are wasted and everything is well-aligned.
ਨਾਮਾਕਾਸ਼ {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#ਜੇ defined(_AIX)
#ਹਦਾਇਤ GCC diagnostic ਧੱਕੋ
#ਹਦਾਇਤ GCC diagnostic ignored "-Waix-compat"
#ਜੇ_ਅੰਤ
ਬਣਤਰ Struct16B {
  alignas(16) ਖਾਲੀ *X;
};
ਬਣਤਰ Struct32B {
  alignas(32) ਖਾਲੀ *X;
};
#ਜੇ defined(_AIX)
#ਹਦਾਇਤ GCC diagnostic ਕੱਢੋ
#ਜੇ_ਅੰਤ
}
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼<ਖਾਲੀ *, 0>) ==
                  ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ) * 2 + ਆਕਾਰ(ਖਾਲੀ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_ਸਦਿਸ਼<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_ਸਦਿਸ਼<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼<ਖਾਲੀ *, 1>) ==
                  ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ) * 2 + ਆਕਾਰ(ਖਾਲੀ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼<ਅੱਖਰ, 0>) ==
                  ਆਕਾਰ(ਖਾਲੀ *) * 2 + ਆਕਾਰ(ਖਾਲੀ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ਸਥਿਰ ਖਾਲੀ report_ਮਾਪ_ਉਛਾਲ(size_t Min_ਮਾਪ, size_t Max_ਮਾਪ);
ਸਥਿਰ ਖਾਲੀ report_ਮਾਪ_ਉਛਾਲ(size_t Min_ਮਾਪ, size_t Max_ਮਾਪ) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::ਮੰਜ਼ਿਲ_string(Min_ਮਾਪ) +
                       ") is larger than maximum value for size type (" +
                       std::ਮੰਜ਼ਿਲ_string(Max_ਮਾਪ) + ")";
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ENABLE_EXCEPTIONS
  ਸੁੱਟੋ std::ਲੰਮਾਈ_error(Reason);
#ਨਹੀਂਤਾਂ
  report_fatal_error(Twine(Reason));
#ਜੇ_ਅੰਤ
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ਸਥਿਰ ਖਾਲੀ report_at_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਸਮਰੱਥਾ(size_t Max_ਮਾਪ);
ਸਥਿਰ ਖਾਲੀ report_at_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਸਮਰੱਥਾ(size_t Max_ਮਾਪ) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::ਮੰਜ਼ਿਲ_string(Max_ਮਾਪ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ENABLE_EXCEPTIONS
  ਸੁੱਟੋ std::ਲੰਮਾਈ_error(Reason);
#ਨਹੀਂਤਾਂ
  report_fatal_error(Twine(Reason));
#ਜੇ_ਅੰਤ
}

// Note: Moving this function into the header may cause performance regression.
ਸਾਂਚਾ <ਵਰਗ ਮਾਪ_T>
ਸਥਿਰ size_t get_New_ਸਮਰੱਥਾ(size_t Min_ਮਾਪ, size_t TSize, size_t Old_ਸਮਰੱਥਾ) {
  constexpr size_t Max_ਮਾਪ = std::numeric_limits<ਮਾਪ_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  ਜੇ (Min_ਮਾਪ > Max_ਮਾਪ)
    report_ਮਾਪ_ਉਛਾਲ(Min_ਮਾਪ, Max_ਮਾਪ);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  ਜੇ (Old_ਸਮਰੱਥਾ == Max_ਮਾਪ)
    report_at_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਸਮਰੱਥਾ(Max_ਮਾਪ);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_ਸਮਰੱਥਾ = 2 * Old_ਸਮਰੱਥਾ + 1; // Always grow.
  ਵਾਪਸ std::clamp(New_ਸਮਰੱਥਾ, Min_ਮਾਪ, Max_ਮਾਪ);
}

/// If vector was first created with capacity 0, getFirstEl() points to the
/// memory right after, an area unallocated. If a subsequent allocation,
/// that grows the vector, happens to return the same pointer as getFirstEl(),
/// get a new allocation, otherwise isSmall() will falsely return that no
/// allocation was done (true) and the memory will not be freed in the
/// destructor. If a VSize is given (vector size), also copy that many
/// elements to the new allocation - used if realloca fails to increase
/// space, and happens to allocate precisely at BeginX.
/// This is unlikely to be called often, but resolves a memory leak when the
/// situation does occur.
ਸਥਿਰ ਖਾਲੀ *replace_ਵੰਡ(ਖਾਲੀ *NewElts, size_t TSize, size_t New_ਸਮਰੱਥਾ,
                               size_t VSize = 0) {
  ਖਾਲੀ *NewEltsReplace = llvm::ਸੁਰੱਖਿਅਤ_malloc(New_ਸਮਰੱਥਾ * TSize);
  ਜੇ (VSize)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(NewEltsReplace, NewElts, VSize * TSize);
  ਮੁਕਤ(NewElts);
  ਵਾਪਸ NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
ਸਾਂਚਾ <ਵਰਗ ਮਾਪ_T>
ਖਾਲੀ *Small_ਸਦਿਸ਼_ਅਧਾਰ<ਮਾਪ_T>::mallocForGrow(ਖਾਲੀ *ਪਹਿਲਾ_El, size_t Min_ਮਾਪ,
                                             size_t TSize,
                                             size_t &New_ਸਮਰੱਥਾ) {
  New_ਸਮਰੱਥਾ = get_New_ਸਮਰੱਥਾ<ਮਾਪ_T>(Min_ਮਾਪ, TSize, ਇਹ->ਸਮਰੱਥਾ_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  ਖਾਲੀ *NewElts = llvm::ਸੁਰੱਖਿਅਤ_malloc(New_ਸਮਰੱਥਾ * TSize);
  ਜੇ (NewElts == ਪਹਿਲਾ_El)
    NewElts = replace_ਵੰਡ(NewElts, TSize, New_ਸਮਰੱਥਾ);
  ਵਾਪਸ NewElts;
}

// Note: Moving this function into the header may cause performance regression.
ਸਾਂਚਾ <ਵਰਗ ਮਾਪ_T>
ਖਾਲੀ Small_ਸਦਿਸ਼_ਅਧਾਰ<ਮਾਪ_T>::grow_pod(ਖਾਲੀ *ਪਹਿਲਾ_El, size_t Min_ਮਾਪ,
                                       size_t TSize) {
  size_t New_ਸਮਰੱਥਾ = get_New_ਸਮਰੱਥਾ<ਮਾਪ_T>(Min_ਮਾਪ, TSize, ਇਹ->ਸਮਰੱਥਾ_2());
  ਖਾਲੀ *NewElts;
  ਜੇ (BeginX == ਪਹਿਲਾ_El) {
    NewElts = llvm::ਸੁਰੱਖਿਅਤ_malloc(New_ਸਮਰੱਥਾ * TSize);
    ਜੇ (NewElts == ਪਹਿਲਾ_El)
      NewElts = replace_ਵੰਡ(NewElts, TSize, New_ਸਮਰੱਥਾ);

    // Copy the elements over.  No need to run dtors on PODs.
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(NewElts, ਇਹ->BeginX, ਮਾਪ_2() * TSize);
  } ਨਹੀਂਤਾਂ {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::ਸੁਰੱਖਿਅਤ_realloc(ਇਹ->BeginX, New_ਸਮਰੱਥਾ * TSize);
    ਜੇ (NewElts == ਪਹਿਲਾ_El)
      NewElts = replace_ਵੰਡ(NewElts, TSize, New_ਸਮਰੱਥਾ, ਮਾਪ_2());
  }

  ਇਹ->ਸੈੱਟ_ਵੰਡ_range(NewElts, New_ਸਮਰੱਥਾ);
}

ਸਾਂਚਾ ਵਰਗ llvm::Small_ਸਦਿਸ਼_ਅਧਾਰ<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#ਜੇ SIZE_MAX > UINT32_MAX
ਸਾਂਚਾ ਵਰਗ llvm::Small_ਸਦਿਸ਼_ਅਧਾਰ<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼_ਮਾਪ_Type<ਅੱਖਰ>) == ਆਕਾਰ(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#ਨਹੀਂਤਾਂ
static_assert(ਆਕਾਰ(Small_ਸਦਿਸ਼_ਮਾਪ_Type<ਅੱਖਰ>) == ਆਕਾਰ(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#ਜੇ_ਅੰਤ
