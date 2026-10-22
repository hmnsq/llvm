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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <cstdint>
#ifdef LLVM_ENABLE_EXCEPTIONS
#ಅನ್ತರ್ಭಾವ <stdexcept>
#endif
using ನಾಮಾಕಾಶ llvm;

// Check that no bytes are wasted and everything is well-aligned.
ನಾಮಾಕಾಶ {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#ಯದಿ defined(_AIX)
#pragma GCC diagnostic ತಳ್ಳು
#pragma GCC diagnostic ignored "-Waix-compat"
#endif
ಸಂರಚನಾ Struct16B {
  alignas(16) ಶೂನ್ಯ *X;
};
ಸಂರಚನಾ Struct32B {
  alignas(32) ಶೂನ್ಯ *X;
};
#ಯದಿ defined(_AIX)
#pragma GCC diagnostic ತೆಗೆ
#endif
}
static_assert(ಪರಿಮಾಣ(SmallVector<ಶೂನ್ಯ *, 0>) ==
                  ಪರಿಮಾಣ(unsigned) * 2 + ಪರಿಮಾಣ(ಶೂನ್ಯ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(ಪರಿಮಾಣ(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(ಪರಿಮಾಣ(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(ಪರಿಮಾಣ(SmallVector<ಶೂನ್ಯ *, 1>) ==
                  ಪರಿಮಾಣ(unsigned) * 2 + ಪರಿಮಾಣ(ಶೂನ್ಯ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(ಪರಿಮಾಣ(SmallVector<ವರ್ಣ, 0>) ==
                  ಪರಿಮಾಣ(ಶೂನ್ಯ *) * 2 + ಪರಿಮಾಣ(ಶೂನ್ಯ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ಸ್ಥಿರ ಶೂನ್ಯ report_size_overflow(size_t MinSize, size_t MaxSize);
ಸ್ಥಿರ ಶೂನ್ಯ report_size_overflow(size_t MinSize, size_t MaxSize) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::to_string(MinSize) +
                       ") is larger than maximum value for size type (" +
                       std::to_string(MaxSize) + ")";
#ifdef LLVM_ENABLE_EXCEPTIONS
  ಕ್ಷಿಪ std::length_error(Reason);
#ಅನ್ಯಥಾ
  report_fatal_error(Twine(Reason));
#endif
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ಸ್ಥಿರ ಶೂನ್ಯ report_at_maximum_capacity(size_t MaxSize);
ಸ್ಥಿರ ಶೂನ್ಯ report_at_maximum_capacity(size_t MaxSize) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::to_string(MaxSize);
#ifdef LLVM_ENABLE_EXCEPTIONS
  ಕ್ಷಿಪ std::length_error(Reason);
#ಅನ್ಯಥಾ
  report_fatal_error(Twine(Reason));
#endif
}

// Note: Moving this function into the header may cause performance regression.
template <ವರ್ಗ Size_T>
ಸ್ಥಿರ size_t getNewCapacity(size_t MinSize, size_t TSize, size_t OldCapacity) {
  constexpr size_t MaxSize = std::numeric_limits<Size_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  ಯದಿ (MinSize > MaxSize)
    report_size_overflow(MinSize, MaxSize);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  ಯದಿ (OldCapacity == MaxSize)
    report_at_maximum_capacity(MaxSize);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t NewCapacity = 2 * OldCapacity + 1; // Always grow.
  ಪ್ರತ್ಯಾಗಮ std::clamp(NewCapacity, MinSize, MaxSize);
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
ಸ್ಥಿರ ಶೂನ್ಯ *replaceAllocation(ಶೂನ್ಯ *NewElts, size_t TSize, size_t NewCapacity,
                               size_t VSize = 0) {
  ಶೂನ್ಯ *NewEltsReplace = llvm::safe_malloc(NewCapacity * TSize);
  ಯದಿ (VSize)
    memcpy(NewEltsReplace, NewElts, VSize * TSize);
  ಮುಕ್ತ(NewElts);
  ಪ್ರತ್ಯಾಗಮ NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
template <ವರ್ಗ Size_T>
ಶೂನ್ಯ *SmallVectorBase<Size_T>::mallocForGrow(ಶೂನ್ಯ *FirstEl, size_t MinSize,
                                             size_t TSize,
                                             size_t &NewCapacity) {
  NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, ಏತತ್->capacity());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  ಶೂನ್ಯ *NewElts = llvm::safe_malloc(NewCapacity * TSize);
  ಯದಿ (NewElts == FirstEl)
    NewElts = replaceAllocation(NewElts, TSize, NewCapacity);
  ಪ್ರತ್ಯಾಗಮ NewElts;
}

// Note: Moving this function into the header may cause performance regression.
template <ವರ್ಗ Size_T>
ಶೂನ್ಯ SmallVectorBase<Size_T>::grow_pod(ಶೂನ್ಯ *FirstEl, size_t MinSize,
                                       size_t TSize) {
  size_t NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, ಏತತ್->capacity());
  ಶೂನ್ಯ *NewElts;
  ಯದಿ (BeginX == FirstEl) {
    NewElts = llvm::safe_malloc(NewCapacity * TSize);
    ಯದಿ (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity);

    // Copy the elements over.  No need to run dtors on PODs.
    memcpy(NewElts, ಏತತ್->BeginX, size() * TSize);
  } ಅನ್ಯಥಾ {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::safe_realloc(ಏತತ್->BeginX, NewCapacity * TSize);
    ಯದಿ (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity, size());
  }

  ಏತತ್->set_allocation_range(NewElts, NewCapacity);
}

template ವರ್ಗ llvm::SmallVectorBase<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#ಯದಿ SIZE_MAX > UINT32_MAX
template ವರ್ಗ llvm::SmallVectorBase<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(ಪರಿಮಾಣ(SmallVectorSizeType<ವರ್ಣ>) == ಪರಿಮಾಣ(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#ಅನ್ಯಥಾ
static_assert(ಪರಿಮಾಣ(SmallVectorSizeType<ವರ್ಣ>) == ಪರಿಮಾಣ(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#endif
