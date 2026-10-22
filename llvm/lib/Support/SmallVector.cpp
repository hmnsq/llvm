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
#جے_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
#ضمن <stdexcept>
#جے_انت
ورتوں نطاق llvm;

// Check that no bytes are wasted and everything is well-aligned.
نطاق {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#إذا defined(_AIX)
#ہدایت GCC diagnostic ڈالو
#ہدایت GCC diagnostic ignored "-Waix-compat"
#جے_انت
بنية Struct16B {
  alignas(16) فراغ *X;
};
بنية Struct32B {
  alignas(32) فراغ *X;
};
#إذا defined(_AIX)
#ہدایت GCC diagnostic نکالو
#جے_انت
}
static_assert(حجم(Small_سمتیہ<فراغ *, 0>) ==
                  حجم(بنانشانی) * 2 + حجم(فراغ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_سمتیہ<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_سمتیہ<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(حجم(Small_سمتیہ<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(حجم(Small_سمتیہ<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(حجم(Small_سمتیہ<فراغ *, 1>) ==
                  حجم(بنانشانی) * 2 + حجم(فراغ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(حجم(Small_سمتیہ<حرف, 0>) ==
                  حجم(فراغ *) * 2 + حجم(فراغ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_ناپ_حد_توں_ودھ(size_t Min_ناپ, size_t Max_ناپ);
ساكن فراغ report_ناپ_حد_توں_ودھ(size_t Min_ناپ, size_t Max_ناپ) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::منزل_string(Min_ناپ) +
                       ") is larger than maximum value for size type (" +
                       std::منزل_string(Max_ناپ) + ")";
#جے_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  ارم std::لمائی_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#جے_انت
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_at_ودھ_توں_ودھ_سمرتھا(size_t Max_ناپ);
ساكن فراغ report_at_ودھ_توں_ودھ_سمرتھا(size_t Max_ناپ) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::منزل_string(Max_ناپ);
#جے_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  ارم std::لمائی_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#جے_انت
}

// Note: Moving this function into the header may cause performance regression.
سانچا <صنف ناپ_T>
ساكن size_t get_New_سمرتھا(size_t Min_ناپ, size_t TSize, size_t Old_سمرتھا) {
  constexpr size_t Max_ناپ = std::numeric_limits<ناپ_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Min_ناپ > Max_ناپ)
    report_ناپ_حد_توں_ودھ(Min_ناپ, Max_ناپ);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Old_سمرتھا == Max_ناپ)
    report_at_ودھ_توں_ودھ_سمرتھا(Max_ناپ);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_سمرتھا = 2 * Old_سمرتھا + 1; // Always grow.
  أرجع std::clamp(New_سمرتھا, Min_ناپ, Max_ناپ);
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
ساكن فراغ *replace_ونڈ(فراغ *NewElts, size_t TSize, size_t New_سمرتھا,
                               size_t VSize = 0) {
  فراغ *NewEltsReplace = llvm::محفوظ_malloc(New_سمرتھا * TSize);
  إذا (VSize)
    حافظہ_نقل(NewEltsReplace, NewElts, VSize * TSize);
  حرر(NewElts);
  أرجع NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
سانچا <صنف ناپ_T>
فراغ *Small_سمتیہ_ادھار<ناپ_T>::mallocForGrow(فراغ *پہلا_El, size_t Min_ناپ,
                                             size_t TSize,
                                             size_t &New_سمرتھا) {
  New_سمرتھا = get_New_سمرتھا<ناپ_T>(Min_ناپ, TSize, هذا->سمرتھا_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  فراغ *NewElts = llvm::محفوظ_malloc(New_سمرتھا * TSize);
  إذا (NewElts == پہلا_El)
    NewElts = replace_ونڈ(NewElts, TSize, New_سمرتھا);
  أرجع NewElts;
}

// Note: Moving this function into the header may cause performance regression.
سانچا <صنف ناپ_T>
فراغ Small_سمتیہ_ادھار<ناپ_T>::grow_pod(فراغ *پہلا_El, size_t Min_ناپ,
                                       size_t TSize) {
  size_t New_سمرتھا = get_New_سمرتھا<ناپ_T>(Min_ناپ, TSize, هذا->سمرتھا_2());
  فراغ *NewElts;
  إذا (BeginX == پہلا_El) {
    NewElts = llvm::محفوظ_malloc(New_سمرتھا * TSize);
    إذا (NewElts == پہلا_El)
      NewElts = replace_ونڈ(NewElts, TSize, New_سمرتھا);

    // Copy the elements over.  No need to run dtors on PODs.
    حافظہ_نقل(NewElts, هذا->BeginX, ناپ_2() * TSize);
  } وإلا {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::محفوظ_realloc(هذا->BeginX, New_سمرتھا * TSize);
    إذا (NewElts == پہلا_El)
      NewElts = replace_ونڈ(NewElts, TSize, New_سمرتھا, ناپ_2());
  }

  هذا->سیٹ_ونڈ_range(NewElts, New_سمرتھا);
}

سانچا صنف llvm::Small_سمتیہ_ادھار<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#إذا SIZE_MAX > UINT32_MAX
سانچا صنف llvm::Small_سمتیہ_ادھار<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(حجم(Small_سمتیہ_ناپ_Type<حرف>) == حجم(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#وإلا
static_assert(حجم(Small_سمتیہ_ناپ_Type<حرف>) == حجم(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#جے_انت
