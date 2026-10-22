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

کارول نطاق llvm;

فراغ Small_Ptr_سېټ_Impl_بنسټ::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  حرر(CurArray);

  // Reduce the number of buckets.
  بینښه اندازه = اندازه_2();
  Cur_Array_اندازه = اندازه > 16 ? 1 << (Log2_32_Ceil(اندازه) + 1) : 32;
  عدد_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت فراغ**)خوندي_malloc(حجم(فراغ*) * Cur_Array_اندازه);

  memset(CurArray, -1, Cur_Array_اندازه*حجم(فراغ*));
}

std::pair<ثابت فراغ *ثابت *, منطقي>
Small_Ptr_سېټ_Impl_بنسټ::ورننباسه_imp_big(ثابت فراغ *Ptr) {
  إذا (LLVM_UNLIKELY(اندازه_2() * 3 >= Cur_Array_اندازه * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_اندازه < 64 ? 128 : Cur_Array_اندازه * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  بینښه ماسک = Cur_Array_اندازه - 1;
  بینښه I = DenseMapInfo<فراغ *>::get_Hash_ارزښت(Ptr) & ماسک;
  ثابت فراغ **Array = CurArray;
  طالما (Array[I] != get_تشه_Marker()) {
    إذا (Array[I] == Ptr)
      أرجع {Array + I, خطأ};
    I = (I + 1) & ماسک;
  }

  // Insert into the empty bucket.
  ++عدد_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  أرجع {Array + I, ریښتیا};
}

ثابت فراغ *ثابت *Small_Ptr_سېټ_Impl_بنسټ::do_پیدا_کړه(ثابت فراغ *Ptr) ثابت {
  بینښه ماسک = Cur_Array_اندازه - 1;
  بینښه لوښی_No = DenseMapInfo<فراغ *>::get_Hash_ارزښت(Ptr) & ماسک;
  طالما (ریښتیا) {
    ثابت فراغ *ثابت *لوښی = CurArray + لوښی_No;
    إذا (LLVM_LIKELY(*لوښی == Ptr))
      أرجع لوښی;
    إذا (LLVM_LIKELY(*لوښی == get_تشه_Marker()))
      أرجع تش_اشاره;
    لوښی_No = (لوښی_No + 1) & ماسک;
  }
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::erase_From_لوښی(ثابت فراغ **لوښی) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  بینښه ماسک = Cur_Array_اندازه - 1;
  بینښه I = لوښی - CurArray;
  بینښه J = I;
  ثابت فراغ *تشه_2 = get_تشه_Marker();
  طالما ((J = (J + 1) & ماسک), CurArray[J] != تشه_2) {
    خپلکاری Ideal = DenseMapInfo<فراغ *>::get_Hash_ارزښت(CurArray[J]);
    إذا (((I - Ideal) & ماسک) < ((J - Ideal) & ماسک)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = تشه_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
فراغ Small_Ptr_سېټ_Impl_بنسټ::Grow(بینښه New_اندازه) {
  خپلکاری OldBuckets = buckets();
  منطقي WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت فراغ **NewBuckets = (ثابت فراغ**) خوندي_malloc(حجم(فراغ*) * New_اندازه);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_اندازه = New_اندازه;
  memset(CurArray, -1, New_اندازه*حجم(فراغ*));

  // Copy over all valid entries.
  بینښه ماسک = Cur_Array_اندازه - 1;
  لكل (ثابت فراغ *Ptr : OldBuckets) {
    إذا (Ptr == get_تشه_Marker())
      استمر;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    بینښه I = DenseMapInfo<فراغ *>::get_Hash_ارزښت(Ptr) & ماسک;
    طالما (NewBuckets[I] != get_تشه_Marker())
      I = (I + 1) & ماسک;
    NewBuckets[I] = Ptr;
  }

  إذا (!WasSmall)
    حرر(OldBuckets.begin());
  IsSmall = خطأ;
}

Small_Ptr_سېټ_Impl_بنسټ::Small_Ptr_سېټ_Impl_بنسټ(ثابت فراغ **SmallStorage,
                                         ثابت Small_Ptr_سېټ_Impl_بنسټ &that) {
  IsSmall = that.isSmall();
  إذا (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وإلا {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت فراغ**)خوندي_malloc(حجم(فراغ*) * that.Cur_Array_اندازه);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_سېټ_Impl_بنسټ::Small_Ptr_سېټ_Impl_بنسټ(ثابت فراغ **SmallStorage,
                                         بینښه Small_اندازه,
                                         ثابت فراغ **RHSSmallStorage,
                                         Small_Ptr_سېټ_Impl_بنسټ &&that) {
  moveHelper(SmallStorage, Small_اندازه, RHSSmallStorage, std::move(that));
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::copyFrom(ثابت فراغ **SmallStorage,
                                   ثابت Small_Ptr_سېټ_Impl_بنسټ &RHS) {
  أكد(&RHS != هذا && "Self-copy should be handled by the caller.");

  إذا (isSmall() && RHS.isSmall())
    أكد(Cur_Array_اندازه == RHS.Cur_Array_اندازه &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  إذا (RHS.isSmall()) {
    إذا (!isSmall())
      حرر(CurArray);
    CurArray = SmallStorage;
    IsSmall = ریښتیا;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وإلا إذا (Cur_Array_اندازه != RHS.Cur_Array_اندازه) {
    إذا (isSmall())
      CurArray = (ثابت فراغ**)خوندي_malloc(حجم(فراغ*) * RHS.Cur_Array_اندازه);
    وإلا {
      ثابت فراغ **T = (ثابت فراغ**)خوندي_realloc(CurArray,
                                             حجم(فراغ*) * RHS.Cur_Array_اندازه);
      CurArray = T;
    }
    IsSmall = خطأ;
  }

  copyHelper(RHS);
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::copyHelper(ثابت Small_Ptr_سېټ_Impl_بنسټ &RHS) {
  // Copy over the new array size
  Cur_Array_اندازه = RHS.Cur_Array_اندازه;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  عدد_Entries = RHS.عدد_Entries;
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::moveFrom(ثابت فراغ **SmallStorage,
                                   بینښه Small_اندازه,
                                   ثابت فراغ **RHSSmallStorage,
                                   Small_Ptr_سېټ_Impl_بنسټ &&RHS) {
  إذا (!isSmall())
    حرر(CurArray);
  moveHelper(SmallStorage, Small_اندازه, RHSSmallStorage, std::move(RHS));
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::moveHelper(ثابت فراغ **SmallStorage,
                                     بینښه Small_اندازه,
                                     ثابت فراغ **RHSSmallStorage,
                                     Small_Ptr_سېټ_Impl_بنسټ &&RHS) {
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
  Cur_Array_اندازه = RHS.Cur_Array_اندازه;
  عدد_Entries = RHS.عدد_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_اندازه = Small_اندازه;
  RHS.عدد_Entries = 0;
  RHS.IsSmall = ریښتیا;
}

فراغ Small_Ptr_سېټ_Impl_بنسټ::بدلون(ثابت فراغ **SmallStorage,
                               ثابت فراغ **RHSSmallStorage,
                               Small_Ptr_سېټ_Impl_بنسټ &RHS) {
  إذا (هذا == &RHS) أرجع;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  إذا (!هذا->isSmall() && !RHS.isSmall()) {
    std::بدلون(هذا->CurArray, RHS.CurArray);
    std::بدلون(هذا->Cur_Array_اندازه, RHS.Cur_Array_اندازه);
    std::بدلون(هذا->عدد_Entries, RHS.عدد_Entries);
    أرجع;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  إذا (هذا->isSmall() && RHS.isSmall()) {
    بینښه MinEntries = std::min(هذا->عدد_Entries, RHS.عدد_Entries);
    std::بدلون_ranges(هذا->CurArray, هذا->CurArray + MinEntries, RHS.CurArray);
    إذا (هذا->عدد_Entries > MinEntries) {
      std::copy(هذا->CurArray + MinEntries, هذا->CurArray + هذا->عدد_Entries,
                RHS.CurArray + MinEntries);
    } وإلا {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.عدد_Entries,
                هذا->CurArray + MinEntries);
    }
    أكد(هذا->Cur_Array_اندازه == RHS.Cur_Array_اندازه);
    std::بدلون(هذا->عدد_Entries, RHS.عدد_Entries);
    أرجع;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_سېټ_Impl_بنسټ &SmallSide = هذا->isSmall() ? *هذا : RHS;
  Small_Ptr_سېټ_Impl_بنسټ &LargeSide = هذا->isSmall() ? RHS : *هذا;
  ثابت فراغ **LargeSideInlineStorage =
      هذا->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::بدلون(LargeSide.Cur_Array_اندازه, SmallSide.Cur_Array_اندازه);
  std::بدلون(LargeSide.عدد_Entries, SmallSide.عدد_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = خطأ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = ریښتیا;
}
