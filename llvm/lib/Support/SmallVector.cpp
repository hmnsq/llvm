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

#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/MemAlloc.h"
#ضمن <cstdint>
#ifdef LLVM_ENABLE_EXCEPTIONS
#ضمن <stdexcept>
#endif
using نطاق llvm;

// Check that no bytes are wasted and everything is well-aligned.
نطاق {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#إذا defined(_AIX)
#pragma GCC diagnostic ڈالو
#pragma GCC diagnostic ignored "-Waix-compat"
#endif
بنية Struct16B {
  alignas(16) فراغ *X;
};
بنية Struct32B {
  alignas(32) فراغ *X;
};
#إذا defined(_AIX)
#pragma GCC diagnostic نکالو
#endif
}
static_assert(حجم(SmallVector<فراغ *, 0>) ==
                  حجم(unsigned) * 2 + حجم(فراغ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(حجم(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(حجم(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(حجم(SmallVector<فراغ *, 1>) ==
                  حجم(unsigned) * 2 + حجم(فراغ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(حجم(SmallVector<حرف, 0>) ==
                  حجم(فراغ *) * 2 + حجم(فراغ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_size_overflow(size_t MinSize, size_t MaxSize);
ساكن فراغ report_size_overflow(size_t MinSize, size_t MaxSize) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::to_string(MinSize) +
                       ") is larger than maximum value for size type (" +
                       std::to_string(MaxSize) + ")";
#ifdef LLVM_ENABLE_EXCEPTIONS
  ارم std::length_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#endif
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_at_maximum_capacity(size_t MaxSize);
ساكن فراغ report_at_maximum_capacity(size_t MaxSize) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::to_string(MaxSize);
#ifdef LLVM_ENABLE_EXCEPTIONS
  ارم std::length_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#endif
}

// Note: Moving this function into the header may cause performance regression.
template <صنف Size_T>
ساكن size_t getNewCapacity(size_t MinSize, size_t TSize, size_t OldCapacity) {
  constexpr size_t MaxSize = std::numeric_limits<Size_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (MinSize > MaxSize)
    report_size_overflow(MinSize, MaxSize);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (OldCapacity == MaxSize)
    report_at_maximum_capacity(MaxSize);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t NewCapacity = 2 * OldCapacity + 1; // Always grow.
  أرجع std::clamp(NewCapacity, MinSize, MaxSize);
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
ساكن فراغ *replaceAllocation(فراغ *NewElts, size_t TSize, size_t NewCapacity,
                               size_t VSize = 0) {
  فراغ *NewEltsReplace = llvm::safe_malloc(NewCapacity * TSize);
  إذا (VSize)
    memcpy(NewEltsReplace, NewElts, VSize * TSize);
  حرر(NewElts);
  أرجع NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
template <صنف Size_T>
فراغ *SmallVectorBase<Size_T>::mallocForGrow(فراغ *FirstEl, size_t MinSize,
                                             size_t TSize,
                                             size_t &NewCapacity) {
  NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, هذا->capacity());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  فراغ *NewElts = llvm::safe_malloc(NewCapacity * TSize);
  إذا (NewElts == FirstEl)
    NewElts = replaceAllocation(NewElts, TSize, NewCapacity);
  أرجع NewElts;
}

// Note: Moving this function into the header may cause performance regression.
template <صنف Size_T>
فراغ SmallVectorBase<Size_T>::grow_pod(فراغ *FirstEl, size_t MinSize,
                                       size_t TSize) {
  size_t NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, هذا->capacity());
  فراغ *NewElts;
  إذا (BeginX == FirstEl) {
    NewElts = llvm::safe_malloc(NewCapacity * TSize);
    إذا (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity);

    // Copy the elements over.  No need to run dtors on PODs.
    memcpy(NewElts, هذا->BeginX, size() * TSize);
  } وإلا {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::safe_realloc(هذا->BeginX, NewCapacity * TSize);
    إذا (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity, size());
  }

  هذا->set_allocation_range(NewElts, NewCapacity);
}

template صنف llvm::SmallVectorBase<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#إذا SIZE_MAX > UINT32_MAX
template صنف llvm::SmallVectorBase<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(حجم(SmallVectorSizeType<حرف>) == حجم(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#وإلا
static_assert(حجم(SmallVectorSizeType<حرف>) == حجم(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#endif
