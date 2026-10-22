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

#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/MemAlloc.h"
#شامل <cstdint>
#اگر_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
#شامل <stdexcept>
#اگر_ختم
استعمال نامخلا llvm;

// Check that no bytes are wasted and everything is well-aligned.
نامخلا {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#اگر defined(_AIX)
#ہدایت GCC diagnostic ڈالو
#ہدایت GCC diagnostic ignored "-Waix-compat"
#اگر_ختم
ساخت Struct16B {
  alignas(16) خالی *X;
};
ساخت Struct32B {
  alignas(32) خالی *X;
};
#اگر defined(_AIX)
#ہدایت GCC diagnostic نکالو
#اگر_ختم
}
static_assert(حجم(Small_سمتیہ<خالی *, 0>) ==
                  حجم(غیرعلامتی) * 2 + حجم(خالی *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_سمتیہ<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_سمتیہ<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(حجم(Small_سمتیہ<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(حجم(Small_سمتیہ<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(حجم(Small_سمتیہ<خالی *, 1>) ==
                  حجم(غیرعلامتی) * 2 + حجم(خالی *) * 2,
              "wasted space in SmallVector size 1");

static_assert(حجم(Small_سمتیہ<حرف, 0>) ==
                  حجم(خالی *) * 2 + حجم(خالی *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساکن خالی report_سائز_حد_سے_تجاوز(size_t Min_سائز, size_t Max_سائز);
ساکن خالی report_سائز_حد_سے_تجاوز(size_t Min_سائز, size_t Max_سائز) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::مقصد_string(Min_سائز) +
                       ") is larger than maximum value for size type (" +
                       std::مقصد_string(Max_سائز) + ")";
#اگر_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  پھینکو std::لمبائی_error(Reason);
#ورنہ
  report_fatal_error(Twine(Reason));
#اگر_ختم
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] ساکن خالی report_at_زیادہ_سے_زیادہ_استعداد(size_t Max_سائز);
ساکن خالی report_at_زیادہ_سے_زیادہ_استعداد(size_t Max_سائز) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::مقصد_string(Max_سائز);
#اگر_تعریف_شدہ LLVM_ENABLE_EXCEPTIONS
  پھینکو std::لمبائی_error(Reason);
#ورنہ
  report_fatal_error(Twine(Reason));
#اگر_ختم
}

// Note: Moving this function into the header may cause performance regression.
سانچہ <جماعت سائز_T>
ساکن size_t get_New_استعداد(size_t Min_سائز, size_t TSize, size_t Old_استعداد) {
  constexpr size_t Max_سائز = std::numeric_limits<سائز_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  اگر (Min_سائز > Max_سائز)
    report_سائز_حد_سے_تجاوز(Min_سائز, Max_سائز);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  اگر (Old_استعداد == Max_سائز)
    report_at_زیادہ_سے_زیادہ_استعداد(Max_سائز);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_استعداد = 2 * Old_استعداد + 1; // Always grow.
  واپس std::clamp(New_استعداد, Min_سائز, Max_سائز);
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
ساکن خالی *replace_تخصیص(خالی *NewElts, size_t TSize, size_t New_استعداد,
                               size_t VSize = 0) {
  خالی *NewEltsReplace = llvm::محفوظ_malloc(New_استعداد * TSize);
  اگر (VSize)
    حافظہ_نقل(NewEltsReplace, NewElts, VSize * TSize);
  آزاد(NewElts);
  واپس NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
سانچہ <جماعت سائز_T>
خالی *Small_سمتیہ_بنیاد<سائز_T>::mallocForGrow(خالی *پہلا_El, size_t Min_سائز,
                                             size_t TSize,
                                             size_t &New_استعداد) {
  New_استعداد = get_New_استعداد<سائز_T>(Min_سائز, TSize, یہ->استعداد_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  خالی *NewElts = llvm::محفوظ_malloc(New_استعداد * TSize);
  اگر (NewElts == پہلا_El)
    NewElts = replace_تخصیص(NewElts, TSize, New_استعداد);
  واپس NewElts;
}

// Note: Moving this function into the header may cause performance regression.
سانچہ <جماعت سائز_T>
خالی Small_سمتیہ_بنیاد<سائز_T>::grow_pod(خالی *پہلا_El, size_t Min_سائز,
                                       size_t TSize) {
  size_t New_استعداد = get_New_استعداد<سائز_T>(Min_سائز, TSize, یہ->استعداد_2());
  خالی *NewElts;
  اگر (BeginX == پہلا_El) {
    NewElts = llvm::محفوظ_malloc(New_استعداد * TSize);
    اگر (NewElts == پہلا_El)
      NewElts = replace_تخصیص(NewElts, TSize, New_استعداد);

    // Copy the elements over.  No need to run dtors on PODs.
    حافظہ_نقل(NewElts, یہ->BeginX, سائز_2() * TSize);
  } ورنہ {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::محفوظ_realloc(یہ->BeginX, New_استعداد * TSize);
    اگر (NewElts == پہلا_El)
      NewElts = replace_تخصیص(NewElts, TSize, New_استعداد, سائز_2());
  }

  یہ->سیٹ_تخصیص_range(NewElts, New_استعداد);
}

سانچہ جماعت llvm::Small_سمتیہ_بنیاد<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#اگر SIZE_MAX > UINT32_MAX
سانچہ جماعت llvm::Small_سمتیہ_بنیاد<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(حجم(Small_سمتیہ_سائز_Type<حرف>) == حجم(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#ورنہ
static_assert(حجم(Small_سمتیہ_سائز_Type<حرف>) == حجم(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#اگر_ختم
