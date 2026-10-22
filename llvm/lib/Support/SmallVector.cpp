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
#اگر_تعریف_شده LLVM_ENABLE_EXCEPTIONS
#ضمن <stdexcept>
#پایان_اگر
استفاده نطاق llvm;

// Check that no bytes are wasted and everything is well-aligned.
نطاق {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#إذا defined(_AIX)
#رهنمود GCC diagnostic افزودن
#رهنمود GCC diagnostic ignored "-Waix-compat"
#پایان_اگر
بنية Struct16B {
  alignas(16) فراغ *X;
};
بنية Struct32B {
  alignas(32) فراغ *X;
};
#إذا defined(_AIX)
#رهنمود GCC diagnostic برداشت
#پایان_اگر
}
static_assert(حجم(Small_وکتور<فراغ *, 0>) ==
                  حجم(بیعلامت) * 2 + حجم(فراغ *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_وکتور<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_وکتور<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(حجم(Small_وکتور<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(حجم(Small_وکتور<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(حجم(Small_وکتور<فراغ *, 1>) ==
                  حجم(بیعلامت) * 2 + حجم(فراغ *) * 2,
              "wasted space in SmallVector size 1");

static_assert(حجم(Small_وکتور<حرف, 0>) ==
                  حجم(فراغ *) * 2 + حجم(فراغ *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_اندازه_سرریز(size_t Min_اندازه, size_t Max_اندازه);
ساكن فراغ report_اندازه_سرریز(size_t Min_اندازه, size_t Max_اندازه) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::مقصد_string(Min_اندازه) +
                       ") is larger than maximum value for size type (" +
                       std::مقصد_string(Max_اندازه) + ")";
#اگر_تعریف_شده LLVM_ENABLE_EXCEPTIONS
  ارم std::درازی_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#پایان_اگر
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساكن فراغ report_at_بیشینه_ظرفیت(size_t Max_اندازه);
ساكن فراغ report_at_بیشینه_ظرفیت(size_t Max_اندازه) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::مقصد_string(Max_اندازه);
#اگر_تعریف_شده LLVM_ENABLE_EXCEPTIONS
  ارم std::درازی_error(Reason);
#وإلا
  report_fatal_error(Twine(Reason));
#پایان_اگر
}

// Note: Moving this function into the header may cause performance regression.
قالب <صنف اندازه_T>
ساكن size_t get_New_ظرفیت(size_t Min_اندازه, size_t TSize, size_t Old_ظرفیت) {
  constexpr size_t Max_اندازه = std::numeric_limits<اندازه_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Min_اندازه > Max_اندازه)
    report_اندازه_سرریز(Min_اندازه, Max_اندازه);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  إذا (Old_ظرفیت == Max_اندازه)
    report_at_بیشینه_ظرفیت(Max_اندازه);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_ظرفیت = 2 * Old_ظرفیت + 1; // Always grow.
  أرجع std::clamp(New_ظرفیت, Min_اندازه, Max_اندازه);
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
ساكن فراغ *replace_تخصیص(فراغ *NewElts, size_t TSize, size_t New_ظرفیت,
                               size_t VSize = 0) {
  فراغ *NewEltsReplace = llvm::امن_malloc(New_ظرفیت * TSize);
  إذا (VSize)
    کپی_حافظه(NewEltsReplace, NewElts, VSize * TSize);
  حرر(NewElts);
  أرجع NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
قالب <صنف اندازه_T>
فراغ *Small_وکتور_پایه<اندازه_T>::mallocForGrow(فراغ *اولی_El, size_t Min_اندازه,
                                             size_t TSize,
                                             size_t &New_ظرفیت) {
  New_ظرفیت = get_New_ظرفیت<اندازه_T>(Min_اندازه, TSize, هذا->ظرفیت_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  فراغ *NewElts = llvm::امن_malloc(New_ظرفیت * TSize);
  إذا (NewElts == اولی_El)
    NewElts = replace_تخصیص(NewElts, TSize, New_ظرفیت);
  أرجع NewElts;
}

// Note: Moving this function into the header may cause performance regression.
قالب <صنف اندازه_T>
فراغ Small_وکتور_پایه<اندازه_T>::grow_pod(فراغ *اولی_El, size_t Min_اندازه,
                                       size_t TSize) {
  size_t New_ظرفیت = get_New_ظرفیت<اندازه_T>(Min_اندازه, TSize, هذا->ظرفیت_2());
  فراغ *NewElts;
  إذا (BeginX == اولی_El) {
    NewElts = llvm::امن_malloc(New_ظرفیت * TSize);
    إذا (NewElts == اولی_El)
      NewElts = replace_تخصیص(NewElts, TSize, New_ظرفیت);

    // Copy the elements over.  No need to run dtors on PODs.
    کپی_حافظه(NewElts, هذا->BeginX, اندازه_2() * TSize);
  } وإلا {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::امن_realloc(هذا->BeginX, New_ظرفیت * TSize);
    إذا (NewElts == اولی_El)
      NewElts = replace_تخصیص(NewElts, TSize, New_ظرفیت, اندازه_2());
  }

  هذا->مجموعه_تخصیص_range(NewElts, New_ظرفیت);
}

قالب صنف llvm::Small_وکتور_پایه<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#إذا SIZE_MAX > UINT32_MAX
قالب صنف llvm::Small_وکتور_پایه<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(حجم(Small_وکتور_اندازه_Type<حرف>) == حجم(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#وإلا
static_assert(حجم(Small_وکتور_اندازه_Type<حرف>) == حجم(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#پایان_اگر
