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
#جيڪڏهن_وصف_ٿيل LLVM_ENABLE_EXCEPTIONS
#ضمن <stdexcept>
#جيڪڏهن_پورو
استعمال نطاق llvm;

// Check that no bytes are wasted and everything is well-aligned.
نطاق {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#إذا defined(_AIX)
#هدايت GCC diagnostic ڌڪو
#هدايت GCC diagnostic ignored "-Waix-compat"
#جيڪڏهن_پورو
بنية Struct16B {
  alignas(16) فراغ *X;
};
بنية Struct32B {
  alignas(32) فراغ *X;
};
#إذا defined(_AIX)
#هدايت GCC diagnostic ڪڍو
#جيڪڏهن_پورو
}
static_assert(حجم(Small_ويڪٽر<فراغ *, 0>) ==
                  حجم(بنانشان) * 2 + حجم(فراغ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_ويڪٽر<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_ويڪٽر<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(حجم(Small_ويڪٽر<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(حجم(Small_ويڪٽر<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(حجم(Small_ويڪٽر<فراغ *, 1>) ==
                  حجم(بنانشان) * 2 + حجم(فراغ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(حجم(Small_ويڪٽر<حرف, 0>) ==
                  حجم(فراغ *) * 2 + حجم(فراغ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_سائيز_حد_کان_مٿي(size_t Min_سائيز, size_t Max_سائيز);
ساكن فراغ report_سائيز_حد_کان_مٿي(size_t Min_سائيز, size_t Max_سائيز) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::منزل_string(Min_سائيز) +
                       ") is larger than maximum value for size type (" +
                       std::منزل_string(Max_سائيز) + ")";
#جيڪڏهن_وصف_ٿيل LLVM_ENABLE_EXCEPTIONS
  ارم std::ڊيگهائي_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#جيڪڏهن_پورو
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_at_وڌ_کان_وڌ_صلاحيت(size_t Max_سائيز);
ساكن فراغ report_at_وڌ_کان_وڌ_صلاحيت(size_t Max_سائيز) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::منزل_string(Max_سائيز);
#جيڪڏهن_وصف_ٿيل LLVM_ENABLE_EXCEPTIONS
  ارم std::ڊيگهائي_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#جيڪڏهن_پورو
}

// Note: Moving this function into the header may cause performance regression.
سانچو <صنف سائيز_T>
ساكن size_t get_New_صلاحيت(size_t Min_سائيز, size_t TSize, size_t Old_صلاحيت) {
  constexpr size_t Max_سائيز = std::numeric_limits<سائيز_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Min_سائيز > Max_سائيز)
    report_سائيز_حد_کان_مٿي(Min_سائيز, Max_سائيز);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Old_صلاحيت == Max_سائيز)
    report_at_وڌ_کان_وڌ_صلاحيت(Max_سائيز);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_صلاحيت = 2 * Old_صلاحيت + 1; // Always grow.
  أرجع std::clamp(New_صلاحيت, Min_سائيز, Max_سائيز);
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
ساكن فراغ *replace_ورهاست(فراغ *NewElts, size_t TSize, size_t New_صلاحيت,
                               size_t VSize = 0) {
  فراغ *NewEltsReplace = llvm::محفوظ_malloc(New_صلاحيت * TSize);
  إذا (VSize)
    يادگيري_نقل(NewEltsReplace, NewElts, VSize * TSize);
  حرر(NewElts);
  أرجع NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
سانچو <صنف سائيز_T>
فراغ *Small_ويڪٽر_بنياد<سائيز_T>::mallocForGrow(فراغ *پهريون_El, size_t Min_سائيز,
                                             size_t TSize,
                                             size_t &New_صلاحيت) {
  New_صلاحيت = get_New_صلاحيت<سائيز_T>(Min_سائيز, TSize, هذا->صلاحيت_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  فراغ *NewElts = llvm::محفوظ_malloc(New_صلاحيت * TSize);
  إذا (NewElts == پهريون_El)
    NewElts = replace_ورهاست(NewElts, TSize, New_صلاحيت);
  أرجع NewElts;
}

// Note: Moving this function into the header may cause performance regression.
سانچو <صنف سائيز_T>
فراغ Small_ويڪٽر_بنياد<سائيز_T>::grow_pod(فراغ *پهريون_El, size_t Min_سائيز,
                                       size_t TSize) {
  size_t New_صلاحيت = get_New_صلاحيت<سائيز_T>(Min_سائيز, TSize, هذا->صلاحيت_2());
  فراغ *NewElts;
  إذا (BeginX == پهريون_El) {
    NewElts = llvm::محفوظ_malloc(New_صلاحيت * TSize);
    إذا (NewElts == پهريون_El)
      NewElts = replace_ورهاست(NewElts, TSize, New_صلاحيت);

    // Copy the elements over.  No need to run dtors on PODs.
    يادگيري_نقل(NewElts, هذا->BeginX, سائيز_2() * TSize);
  } وإلا {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::محفوظ_realloc(هذا->BeginX, New_صلاحيت * TSize);
    إذا (NewElts == پهريون_El)
      NewElts = replace_ورهاست(NewElts, TSize, New_صلاحيت, سائيز_2());
  }

  هذا->سيٽ_ورهاست_range(NewElts, New_صلاحيت);
}

سانچو صنف llvm::Small_ويڪٽر_بنياد<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#إذا SIZE_MAX > UINT32_MAX
سانچو صنف llvm::Small_ويڪٽر_بنياد<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(حجم(Small_ويڪٽر_سائيز_Type<حرف>) == حجم(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#وإلا
static_assert(حجم(Small_ويڪٽر_سائيز_Type<حرف>) == حجم(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#جيڪڏهن_پورو
