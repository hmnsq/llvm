//===- llvm/ADT/SmallPtrSet.cpp - 'Normally small' pointer set ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SmallPtrSet class.  See SmallPtrSet.h for an
// overview of the algorithm.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/SmallPtrSet.h"
#ضمن "llvm/ADT/DenseMapInfo.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/MemAlloc.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <cstdlib>

ورتوں نطاق llvm;

فراغ Small_Ptr_سیٹ_Impl_ادھار::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  حرر(CurArray);

  // Reduce the number of buckets.
  بنانشانی ناپ = ناپ_2();
  Cur_Array_ناپ = ناپ > 16 ? 1 << (Log2_32_Ceil(ناپ) + 1) : 32;
  عدد_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * Cur_Array_ناپ);

  memset(CurArray, -1, Cur_Array_ناپ*حجم(فراغ*));
}

std::pair<ثابت فراغ *ثابت *, منطقي>
Small_Ptr_سیٹ_Impl_ادھار::داخل_کرو_imp_big(ثابت فراغ *Ptr) {
  إذا (LLVM_UNLIKELY(ناپ_2() * 3 >= Cur_Array_ناپ * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_ناپ < 64 ? 128 : Cur_Array_ناپ * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  بنانشانی نقاب = Cur_Array_ناپ - 1;
  بنانشانی I = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
  ثابت فراغ **Array = CurArray;
  طالما (Array[I] != get_خالی_تھاواں_Marker()) {
    إذا (Array[I] == Ptr)
      أرجع {Array + I, خطأ};
    I = (I + 1) & نقاب;
  }

  // Insert into the empty bucket.
  ++عدد_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  أرجع {Array + I, سچ};
}

ثابت فراغ *ثابت *Small_Ptr_سیٹ_Impl_ادھار::do_لبھو(ثابت فراغ *Ptr) ثابت {
  بنانشانی نقاب = Cur_Array_ناپ - 1;
  بنانشانی بالٹی_No = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
  طالما (سچ) {
    ثابت فراغ *ثابت *بالٹی = CurArray + بالٹی_No;
    إذا (LLVM_LIKELY(*بالٹی == Ptr))
      أرجع بالٹی;
    إذا (LLVM_LIKELY(*بالٹی == get_خالی_تھاواں_Marker()))
      أرجع خالی_سوچک;
    بالٹی_No = (بالٹی_No + 1) & نقاب;
  }
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::erase_From_بالٹی(ثابت فراغ **بالٹی) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  بنانشانی نقاب = Cur_Array_ناپ - 1;
  بنانشانی I = بالٹی - CurArray;
  بنانشانی J = I;
  ثابت فراغ *خالی_تھاواں_2 = get_خالی_تھاواں_Marker();
  طالما ((J = (J + 1) & نقاب), CurArray[J] != خالی_تھاواں_2) {
    خودکار Ideal = DenseMapInfo<فراغ *>::get_Hash_قدر(CurArray[J]);
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = خالی_تھاواں_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
فراغ Small_Ptr_سیٹ_Impl_ادھار::Grow(بنانشانی New_ناپ) {
  خودکار OldBuckets = buckets();
  منطقي WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت فراغ **NewBuckets = (ثابت فراغ**) محفوظ_malloc(حجم(فراغ*) * New_ناپ);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_ناپ = New_ناپ;
  memset(CurArray, -1, New_ناپ*حجم(فراغ*));

  // Copy over all valid entries.
  بنانشانی نقاب = Cur_Array_ناپ - 1;
  لكل (ثابت فراغ *Ptr : OldBuckets) {
    إذا (Ptr == get_خالی_تھاواں_Marker())
      استمر;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    بنانشانی I = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
    طالما (NewBuckets[I] != get_خالی_تھاواں_Marker())
      I = (I + 1) & نقاب;
    NewBuckets[I] = Ptr;
  }

  إذا (!WasSmall)
    حرر(OldBuckets.begin());
  IsSmall = خطأ;
}

Small_Ptr_سیٹ_Impl_ادھار::Small_Ptr_سیٹ_Impl_ادھار(ثابت فراغ **SmallStorage,
                                         ثابت Small_Ptr_سیٹ_Impl_ادھار &that) {
  IsSmall = that.isSmall();
  إذا (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وإلا {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * that.Cur_Array_ناپ);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_سیٹ_Impl_ادھار::Small_Ptr_سیٹ_Impl_ادھار(ثابت فراغ **SmallStorage,
                                         بنانشانی Small_ناپ,
                                         ثابت فراغ **RHSSmallStorage,
                                         Small_Ptr_سیٹ_Impl_ادھار &&that) {
  moveHelper(SmallStorage, Small_ناپ, RHSSmallStorage, std::move(that));
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::copyFrom(ثابت فراغ **SmallStorage,
                                   ثابت Small_Ptr_سیٹ_Impl_ادھار &RHS) {
  أكد(&RHS != هذا && "Self-copy should be handled by the caller.");

  إذا (isSmall() && RHS.isSmall())
    أكد(Cur_Array_ناپ == RHS.Cur_Array_ناپ &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  إذا (RHS.isSmall()) {
    إذا (!isSmall())
      حرر(CurArray);
    CurArray = SmallStorage;
    IsSmall = سچ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وإلا إذا (Cur_Array_ناپ != RHS.Cur_Array_ناپ) {
    إذا (isSmall())
      CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * RHS.Cur_Array_ناپ);
    وإلا {
      ثابت فراغ **T = (ثابت فراغ**)محفوظ_realloc(CurArray,
                                             حجم(فراغ*) * RHS.Cur_Array_ناپ);
      CurArray = T;
    }
    IsSmall = خطأ;
  }

  copyHelper(RHS);
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::copyHelper(ثابت Small_Ptr_سیٹ_Impl_ادھار &RHS) {
  // Copy over the new array size
  Cur_Array_ناپ = RHS.Cur_Array_ناپ;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  عدد_Entries = RHS.عدد_Entries;
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::moveFrom(ثابت فراغ **SmallStorage,
                                   بنانشانی Small_ناپ,
                                   ثابت فراغ **RHSSmallStorage,
                                   Small_Ptr_سیٹ_Impl_ادھار &&RHS) {
  إذا (!isSmall())
    حرر(CurArray);
  moveHelper(SmallStorage, Small_ناپ, RHSSmallStorage, std::move(RHS));
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::moveHelper(ثابت فراغ **SmallStorage,
                                     بنانشانی Small_ناپ,
                                     ثابت فراغ **RHSSmallStorage,
                                     Small_Ptr_سیٹ_Impl_ادھار &&RHS) {
  أكد(&RHS != هذا && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  إذا (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } وإلا {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_ناپ = RHS.Cur_Array_ناپ;
  عدد_Entries = RHS.عدد_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_ناپ = Small_ناپ;
  RHS.عدد_Entries = 0;
  RHS.IsSmall = سچ;
}

فراغ Small_Ptr_سیٹ_Impl_ادھار::وٹاندرا(ثابت فراغ **SmallStorage,
                               ثابت فراغ **RHSSmallStorage,
                               Small_Ptr_سیٹ_Impl_ادھار &RHS) {
  إذا (هذا == &RHS) أرجع;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  إذا (!هذا->isSmall() && !RHS.isSmall()) {
    std::وٹاندرا(هذا->CurArray, RHS.CurArray);
    std::وٹاندرا(هذا->Cur_Array_ناپ, RHS.Cur_Array_ناپ);
    std::وٹاندرا(هذا->عدد_Entries, RHS.عدد_Entries);
    أرجع;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  إذا (هذا->isSmall() && RHS.isSmall()) {
    بنانشانی MinEntries = std::min(هذا->عدد_Entries, RHS.عدد_Entries);
    std::وٹاندرا_ranges(هذا->CurArray, هذا->CurArray + MinEntries, RHS.CurArray);
    إذا (هذا->عدد_Entries > MinEntries) {
      std::copy(هذا->CurArray + MinEntries, هذا->CurArray + هذا->عدد_Entries,
                RHS.CurArray + MinEntries);
    } وإلا {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.عدد_Entries,
                هذا->CurArray + MinEntries);
    }
    أكد(هذا->Cur_Array_ناپ == RHS.Cur_Array_ناپ);
    std::وٹاندرا(هذا->عدد_Entries, RHS.عدد_Entries);
    أرجع;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_سیٹ_Impl_ادھار &SmallSide = هذا->isSmall() ? *هذا : RHS;
  Small_Ptr_سیٹ_Impl_ادھار &LargeSide = هذا->isSmall() ? RHS : *هذا;
  ثابت فراغ **LargeSideInlineStorage =
      هذا->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::وٹاندرا(LargeSide.Cur_Array_ناپ, SmallSide.Cur_Array_ناپ);
  std::وٹاندرا(LargeSide.عدد_Entries, SmallSide.عدد_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = خطأ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = سچ;
}
