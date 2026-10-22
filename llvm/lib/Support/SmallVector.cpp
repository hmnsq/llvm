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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ENABLE_EXCEPTIONS
#ಅನ್ತರ್ಭಾವ <stdexcept>
#ಆದರೆ_ಅಂತ್ಯ
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

// Check that no bytes are wasted and everything is well-aligned.
ನಾಮಾಕಾಶ {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#ಆದರೆ defined(_AIX)
#ಸೂಚನೆ GCC diagnostic ತಳ್ಳು
#ಸೂಚನೆ GCC diagnostic ignored "-Waix-compat"
#ಆದರೆ_ಅಂತ್ಯ
ರಚನೆ Struct16B {
  alignas(16) ಶೂನ್ಯ *X;
};
ರಚನೆ Struct32B {
  alignas(32) ಶೂನ್ಯ *X;
};
#ಆದರೆ defined(_AIX)
#ಸೂಚನೆ GCC diagnostic ತೆಗೆ
#ಆದರೆ_ಅಂತ್ಯ
}
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ<ಶೂನ್ಯ *, 0>) ==
                  ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ) * 2 + ಗಾತ್ರ(ಶೂನ್ಯ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_ಸದಿಶ<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_ಸದಿಶ<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ<ಶೂನ್ಯ *, 1>) ==
                  ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ) * 2 + ಗಾತ್ರ(ಶೂನ್ಯ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(ಗಾತ್ರ(Small_ಸದಿಶ<ಅಕ್ಷರ, 0>) ==
                  ಗಾತ್ರ(ಶೂನ್ಯ *) * 2 + ಗಾತ್ರ(ಶೂನ್ಯ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ಸ್ಥಿರ ಶೂನ್ಯ report_ಗಾತ್ರ_ಅತಿಪ್ರವಾಹ(size_t Min_ಗಾತ್ರ, size_t Max_ಗಾತ್ರ);
ಸ್ಥಿರ ಶೂನ್ಯ report_ಗಾತ್ರ_ಅತಿಪ್ರವಾಹ(size_t Min_ಗಾತ್ರ, size_t Max_ಗಾತ್ರ) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::ಗಮ್ಯ_string(Min_ಗಾತ್ರ) +
                       ") is larger than maximum value for size type (" +
                       std::ಗಮ್ಯ_string(Max_ಗಾತ್ರ) + ")";
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ENABLE_EXCEPTIONS
  ಎಸೆ std::ಉದ್ದಳತೆ_error(Reason);
#ಇಲ್ಲದಿದ್ದರೆ
  report_fatal_error(Twine(Reason));
#ಆದರೆ_ಅಂತ್ಯ
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ಸ್ಥಿರ ಶೂನ್ಯ report_at_ಗರಿಷ್ಠ_ಸಾಮರ್ಥ್ಯ(size_t Max_ಗಾತ್ರ);
ಸ್ಥಿರ ಶೂನ್ಯ report_at_ಗರಿಷ್ಠ_ಸಾಮರ್ಥ್ಯ(size_t Max_ಗಾತ್ರ) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::ಗಮ್ಯ_string(Max_ಗಾತ್ರ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ENABLE_EXCEPTIONS
  ಎಸೆ std::ಉದ್ದಳತೆ_error(Reason);
#ಇಲ್ಲದಿದ್ದರೆ
  report_fatal_error(Twine(Reason));
#ಆದರೆ_ಅಂತ್ಯ
}

// Note: Moving this function into the header may cause performance regression.
ಮಾದರಿ <ವರ್ಗ ಗಾತ್ರ_T>
ಸ್ಥಿರ size_t get_New_ಸಾಮರ್ಥ್ಯ(size_t Min_ಗಾತ್ರ, size_t TSize, size_t Old_ಸಾಮರ್ಥ್ಯ) {
  constexpr size_t Max_ಗಾತ್ರ = std::numeric_limits<ಗಾತ್ರ_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  ಆದರೆ (Min_ಗಾತ್ರ > Max_ಗಾತ್ರ)
    report_ಗಾತ್ರ_ಅತಿಪ್ರವಾಹ(Min_ಗಾತ್ರ, Max_ಗಾತ್ರ);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  ಆದರೆ (Old_ಸಾಮರ್ಥ್ಯ == Max_ಗಾತ್ರ)
    report_at_ಗರಿಷ್ಠ_ಸಾಮರ್ಥ್ಯ(Max_ಗಾತ್ರ);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_ಸಾಮರ್ಥ್ಯ = 2 * Old_ಸಾಮರ್ಥ್ಯ + 1; // Always grow.
  ಹಿಂತಿರುಗಿ std::clamp(New_ಸಾಮರ್ಥ್ಯ, Min_ಗಾತ್ರ, Max_ಗಾತ್ರ);
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
ಸ್ಥಿರ ಶೂನ್ಯ *replace_ಹಂಚಿಕೆ(ಶೂನ್ಯ *NewElts, size_t TSize, size_t New_ಸಾಮರ್ಥ್ಯ,
                               size_t VSize = 0) {
  ಶೂನ್ಯ *NewEltsReplace = llvm::ಸುರಕ್ಷಿತ_malloc(New_ಸಾಮರ್ಥ್ಯ * TSize);
  ಆದರೆ (VSize)
    ಸ್ಮರಣೆ_ನಕಲು(NewEltsReplace, NewElts, VSize * TSize);
  ಬಿಡುಗಡೆ(NewElts);
  ಹಿಂತಿರುಗಿ NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
ಮಾದರಿ <ವರ್ಗ ಗಾತ್ರ_T>
ಶೂನ್ಯ *Small_ಸದಿಶ_ತಳ<ಗಾತ್ರ_T>::mallocForGrow(ಶೂನ್ಯ *ಮೊದಲ_El, size_t Min_ಗಾತ್ರ,
                                             size_t TSize,
                                             size_t &New_ಸಾಮರ್ಥ್ಯ) {
  New_ಸಾಮರ್ಥ್ಯ = get_New_ಸಾಮರ್ಥ್ಯ<ಗಾತ್ರ_T>(Min_ಗಾತ್ರ, TSize, ಇದು->ಸಾಮರ್ಥ್ಯ_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  ಶೂನ್ಯ *NewElts = llvm::ಸುರಕ್ಷಿತ_malloc(New_ಸಾಮರ್ಥ್ಯ * TSize);
  ಆದರೆ (NewElts == ಮೊದಲ_El)
    NewElts = replace_ಹಂಚಿಕೆ(NewElts, TSize, New_ಸಾಮರ್ಥ್ಯ);
  ಹಿಂತಿರುಗಿ NewElts;
}

// Note: Moving this function into the header may cause performance regression.
ಮಾದರಿ <ವರ್ಗ ಗಾತ್ರ_T>
ಶೂನ್ಯ Small_ಸದಿಶ_ತಳ<ಗಾತ್ರ_T>::grow_pod(ಶೂನ್ಯ *ಮೊದಲ_El, size_t Min_ಗಾತ್ರ,
                                       size_t TSize) {
  size_t New_ಸಾಮರ್ಥ್ಯ = get_New_ಸಾಮರ್ಥ್ಯ<ಗಾತ್ರ_T>(Min_ಗಾತ್ರ, TSize, ಇದು->ಸಾಮರ್ಥ್ಯ_2());
  ಶೂನ್ಯ *NewElts;
  ಆದರೆ (BeginX == ಮೊದಲ_El) {
    NewElts = llvm::ಸುರಕ್ಷಿತ_malloc(New_ಸಾಮರ್ಥ್ಯ * TSize);
    ಆದರೆ (NewElts == ಮೊದಲ_El)
      NewElts = replace_ಹಂಚಿಕೆ(NewElts, TSize, New_ಸಾಮರ್ಥ್ಯ);

    // Copy the elements over.  No need to run dtors on PODs.
    ಸ್ಮರಣೆ_ನಕಲು(NewElts, ಇದು->BeginX, ಗಾತ್ರ_3() * TSize);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::ಸುರಕ್ಷಿತ_realloc(ಇದು->BeginX, New_ಸಾಮರ್ಥ್ಯ * TSize);
    ಆದರೆ (NewElts == ಮೊದಲ_El)
      NewElts = replace_ಹಂಚಿಕೆ(NewElts, TSize, New_ಸಾಮರ್ಥ್ಯ, ಗಾತ್ರ_3());
  }

  ಇದು->ಗಣ_ಹಂಚಿಕೆ_range(NewElts, New_ಸಾಮರ್ಥ್ಯ);
}

ಮಾದರಿ ವರ್ಗ llvm::Small_ಸದಿಶ_ತಳ<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#ಆದರೆ SIZE_MAX > UINT32_MAX
ಮಾದರಿ ವರ್ಗ llvm::Small_ಸದಿಶ_ತಳ<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ_ಗಾತ್ರ_Type<ಅಕ್ಷರ>) == ಗಾತ್ರ(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#ಇಲ್ಲದಿದ್ದರೆ
static_assert(ಗಾತ್ರ(Small_ಸದಿಶ_ಗಾತ್ರ_Type<ಅಕ್ಷರ>) == ಗಾತ್ರ(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#ಆದರೆ_ಅಂತ್ಯ
