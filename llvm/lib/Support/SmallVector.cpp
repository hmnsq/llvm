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

#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/MemAlloc.h"
#अन्तर्भाव <cstdint>
#ifdef LLVM_ENABLE_EXCEPTIONS
#अन्तर्भाव <stdexcept>
#endif
using नामाकाश llvm;

// Check that no bytes are wasted and everything is well-aligned.
नामाकाश {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#यदि defined(_AIX)
#pragma GCC diagnostic ढकला
#pragma GCC diagnostic ignored "-Waix-compat"
#endif
संरचना Struct16B {
  alignas(16) शून्य *X;
};
संरचना Struct32B {
  alignas(32) शून्य *X;
};
#यदि defined(_AIX)
#pragma GCC diagnostic काढा
#endif
}
static_assert(परिमाण(SmallVector<शून्य *, 0>) ==
                  परिमाण(unsigned) * 2 + परिमाण(शून्य *),
              "wasted space in SmallVector size 0");
static_assert(alignof(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(परिमाण(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(परिमाण(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(परिमाण(SmallVector<शून्य *, 1>) ==
                  परिमाण(unsigned) * 2 + परिमाण(शून्य *) * 2,
              "wasted space in SmallVector size 1");

static_assert(परिमाण(SmallVector<वर्ण, 0>) ==
                  परिमाण(शून्य *) * 2 + परिमाण(शून्य *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] स्थिर शून्य report_size_overflow(size_t MinSize, size_t MaxSize);
स्थिर शून्य report_size_overflow(size_t MinSize, size_t MaxSize) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::to_string(MinSize) +
                       ") is larger than maximum value for size type (" +
                       std::to_string(MaxSize) + ")";
#ifdef LLVM_ENABLE_EXCEPTIONS
  क्षिप std::length_error(Reason);
#अन्यथा
  report_fatal_error(Twine(Reason));
#endif
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] स्थिर शून्य report_at_maximum_capacity(size_t MaxSize);
स्थिर शून्य report_at_maximum_capacity(size_t MaxSize) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::to_string(MaxSize);
#ifdef LLVM_ENABLE_EXCEPTIONS
  क्षिप std::length_error(Reason);
#अन्यथा
  report_fatal_error(Twine(Reason));
#endif
}

// Note: Moving this function into the header may cause performance regression.
template <वर्ग Size_T>
स्थिर size_t getNewCapacity(size_t MinSize, size_t TSize, size_t OldCapacity) {
  constexpr size_t MaxSize = std::numeric_limits<Size_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  यदि (MinSize > MaxSize)
    report_size_overflow(MinSize, MaxSize);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  यदि (OldCapacity == MaxSize)
    report_at_maximum_capacity(MaxSize);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t NewCapacity = 2 * OldCapacity + 1; // Always grow.
  प्रत्यागम std::clamp(NewCapacity, MinSize, MaxSize);
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
स्थिर शून्य *replaceAllocation(शून्य *NewElts, size_t TSize, size_t NewCapacity,
                               size_t VSize = 0) {
  शून्य *NewEltsReplace = llvm::safe_malloc(NewCapacity * TSize);
  यदि (VSize)
    memcpy(NewEltsReplace, NewElts, VSize * TSize);
  मुक्त(NewElts);
  प्रत्यागम NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
template <वर्ग Size_T>
शून्य *SmallVectorBase<Size_T>::mallocForGrow(शून्य *FirstEl, size_t MinSize,
                                             size_t TSize,
                                             size_t &NewCapacity) {
  NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, एतत्->capacity());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  शून्य *NewElts = llvm::safe_malloc(NewCapacity * TSize);
  यदि (NewElts == FirstEl)
    NewElts = replaceAllocation(NewElts, TSize, NewCapacity);
  प्रत्यागम NewElts;
}

// Note: Moving this function into the header may cause performance regression.
template <वर्ग Size_T>
शून्य SmallVectorBase<Size_T>::grow_pod(शून्य *FirstEl, size_t MinSize,
                                       size_t TSize) {
  size_t NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, एतत्->capacity());
  शून्य *NewElts;
  यदि (BeginX == FirstEl) {
    NewElts = llvm::safe_malloc(NewCapacity * TSize);
    यदि (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity);

    // Copy the elements over.  No need to run dtors on PODs.
    memcpy(NewElts, एतत्->BeginX, size() * TSize);
  } अन्यथा {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::safe_realloc(एतत्->BeginX, NewCapacity * TSize);
    यदि (NewElts == FirstEl)
      NewElts = replaceAllocation(NewElts, TSize, NewCapacity, size());
  }

  एतत्->set_allocation_range(NewElts, NewCapacity);
}

template वर्ग llvm::SmallVectorBase<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#यदि SIZE_MAX > UINT32_MAX
template वर्ग llvm::SmallVectorBase<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(परिमाण(SmallVectorSizeType<वर्ण>) == परिमाण(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#अन्यथा
static_assert(परिमाण(SmallVectorSizeType<वर्ण>) == परिमाण(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#endif
