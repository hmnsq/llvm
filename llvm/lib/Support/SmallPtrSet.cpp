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

استفاده نطاق llvm;

فراغ Small_Ptr_مجموعه_Impl_پایه::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  حرر(CurArray);

  // Reduce the number of buckets.
  بیعلامت اندازه = اندازه_2();
  Cur_Array_اندازه = اندازه > 16 ? 1 << (Log2_32_Ceil(اندازه) + 1) : 32;
  رقم_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت فراغ**)امن_malloc(حجم(فراغ*) * Cur_Array_اندازه);

  memset(CurArray, -1, Cur_Array_اندازه*حجم(فراغ*));
}

std::pair<ثابت فراغ *ثابت *, منطقي>
Small_Ptr_مجموعه_Impl_پایه::درج_imp_big(ثابت فراغ *Ptr) {
  إذا (LLVM_UNLIKELY(اندازه_2() * 3 >= Cur_Array_اندازه * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_اندازه < 64 ? 128 : Cur_Array_اندازه * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  بیعلامت ماسک = Cur_Array_اندازه - 1;
  بیعلامت I = DenseMapInfo<فراغ *>::get_Hash_مقدار(Ptr) & ماسک;
  ثابت فراغ **Array = CurArray;
  طالما (Array[I] != get_خالی_Marker()) {
    إذا (Array[I] == Ptr)
      أرجع {Array + I, خطأ};
    I = (I + 1) & ماسک;
  }

  // Insert into the empty bucket.
  ++رقم_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  أرجع {Array + I, درست};
}

ثابت فراغ *ثابت *Small_Ptr_مجموعه_Impl_پایه::do_پیدا_کن(ثابت فراغ *Ptr) ثابت {
  بیعلامت ماسک = Cur_Array_اندازه - 1;
  بیعلامت سطل_No = DenseMapInfo<فراغ *>::get_Hash_مقدار(Ptr) & ماسک;
  طالما (درست) {
    ثابت فراغ *ثابت *سطل = CurArray + سطل_No;
    إذا (LLVM_LIKELY(*سطل == Ptr))
      أرجع سطل;
    إذا (LLVM_LIKELY(*سطل == get_خالی_Marker()))
      أرجع اشاره_تهی;
    سطل_No = (سطل_No + 1) & ماسک;
  }
}

فراغ Small_Ptr_مجموعه_Impl_پایه::erase_From_سطل(ثابت فراغ **سطل) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  بیعلامت ماسک = Cur_Array_اندازه - 1;
  بیعلامت I = سطل - CurArray;
  بیعلامت J = I;
  ثابت فراغ *خالی_2 = get_خالی_Marker();
  طالما ((J = (J + 1) & ماسک), CurArray[J] != خالی_2) {
    خودکار Ideal = DenseMapInfo<فراغ *>::get_Hash_مقدار(CurArray[J]);
    إذا (((I - Ideal) & ماسک) < ((J - Ideal) & ماسک)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = خالی_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
فراغ Small_Ptr_مجموعه_Impl_پایه::Grow(بیعلامت New_اندازه) {
  خودکار OldBuckets = buckets();
  منطقي WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت فراغ **NewBuckets = (ثابت فراغ**) امن_malloc(حجم(فراغ*) * New_اندازه);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_اندازه = New_اندازه;
  memset(CurArray, -1, New_اندازه*حجم(فراغ*));

  // Copy over all valid entries.
  بیعلامت ماسک = Cur_Array_اندازه - 1;
  لكل (ثابت فراغ *Ptr : OldBuckets) {
    إذا (Ptr == get_خالی_Marker())
      استمر;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    بیعلامت I = DenseMapInfo<فراغ *>::get_Hash_مقدار(Ptr) & ماسک;
    طالما (NewBuckets[I] != get_خالی_Marker())
      I = (I + 1) & ماسک;
    NewBuckets[I] = Ptr;
  }

  إذا (!WasSmall)
    حرر(OldBuckets.begin());
  IsSmall = خطأ;
}

Small_Ptr_مجموعه_Impl_پایه::Small_Ptr_مجموعه_Impl_پایه(ثابت فراغ **SmallStorage,
                                         ثابت Small_Ptr_مجموعه_Impl_پایه &that) {
  IsSmall = that.isSmall();
  إذا (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وإلا {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت فراغ**)امن_malloc(حجم(فراغ*) * that.Cur_Array_اندازه);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_مجموعه_Impl_پایه::Small_Ptr_مجموعه_Impl_پایه(ثابت فراغ **SmallStorage,
                                         بیعلامت Small_اندازه,
                                         ثابت فراغ **RHSSmallStorage,
                                         Small_Ptr_مجموعه_Impl_پایه &&that) {
  moveHelper(SmallStorage, Small_اندازه, RHSSmallStorage, std::move(that));
}

فراغ Small_Ptr_مجموعه_Impl_پایه::copyFrom(ثابت فراغ **SmallStorage,
                                   ثابت Small_Ptr_مجموعه_Impl_پایه &RHS) {
  أكد(&RHS != هذا && "Self-copy should be handled by the caller.");

  إذا (isSmall() && RHS.isSmall())
    أكد(Cur_Array_اندازه == RHS.Cur_Array_اندازه &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  إذا (RHS.isSmall()) {
    إذا (!isSmall())
      حرر(CurArray);
    CurArray = SmallStorage;
    IsSmall = درست;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وإلا إذا (Cur_Array_اندازه != RHS.Cur_Array_اندازه) {
    إذا (isSmall())
      CurArray = (ثابت فراغ**)امن_malloc(حجم(فراغ*) * RHS.Cur_Array_اندازه);
    وإلا {
      ثابت فراغ **T = (ثابت فراغ**)امن_realloc(CurArray,
                                             حجم(فراغ*) * RHS.Cur_Array_اندازه);
      CurArray = T;
    }
    IsSmall = خطأ;
  }

  copyHelper(RHS);
}

فراغ Small_Ptr_مجموعه_Impl_پایه::copyHelper(ثابت Small_Ptr_مجموعه_Impl_پایه &RHS) {
  // Copy over the new array size
  Cur_Array_اندازه = RHS.Cur_Array_اندازه;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  رقم_Entries = RHS.رقم_Entries;
}

فراغ Small_Ptr_مجموعه_Impl_پایه::moveFrom(ثابت فراغ **SmallStorage,
                                   بیعلامت Small_اندازه,
                                   ثابت فراغ **RHSSmallStorage,
                                   Small_Ptr_مجموعه_Impl_پایه &&RHS) {
  إذا (!isSmall())
    حرر(CurArray);
  moveHelper(SmallStorage, Small_اندازه, RHSSmallStorage, std::move(RHS));
}

فراغ Small_Ptr_مجموعه_Impl_پایه::moveHelper(ثابت فراغ **SmallStorage,
                                     بیعلامت Small_اندازه,
                                     ثابت فراغ **RHSSmallStorage,
                                     Small_Ptr_مجموعه_Impl_پایه &&RHS) {
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
  رقم_Entries = RHS.رقم_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_اندازه = Small_اندازه;
  RHS.رقم_Entries = 0;
  RHS.IsSmall = درست;
}

فراغ Small_Ptr_مجموعه_Impl_پایه::جابجا(ثابت فراغ **SmallStorage,
                               ثابت فراغ **RHSSmallStorage,
                               Small_Ptr_مجموعه_Impl_پایه &RHS) {
  إذا (هذا == &RHS) أرجع;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  إذا (!هذا->isSmall() && !RHS.isSmall()) {
    std::جابجا(هذا->CurArray, RHS.CurArray);
    std::جابجا(هذا->Cur_Array_اندازه, RHS.Cur_Array_اندازه);
    std::جابجا(هذا->رقم_Entries, RHS.رقم_Entries);
    أرجع;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  إذا (هذا->isSmall() && RHS.isSmall()) {
    بیعلامت MinEntries = std::min(هذا->رقم_Entries, RHS.رقم_Entries);
    std::جابجا_ranges(هذا->CurArray, هذا->CurArray + MinEntries, RHS.CurArray);
    إذا (هذا->رقم_Entries > MinEntries) {
      std::copy(هذا->CurArray + MinEntries, هذا->CurArray + هذا->رقم_Entries,
                RHS.CurArray + MinEntries);
    } وإلا {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.رقم_Entries,
                هذا->CurArray + MinEntries);
    }
    أكد(هذا->Cur_Array_اندازه == RHS.Cur_Array_اندازه);
    std::جابجا(هذا->رقم_Entries, RHS.رقم_Entries);
    أرجع;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_مجموعه_Impl_پایه &SmallSide = هذا->isSmall() ? *هذا : RHS;
  Small_Ptr_مجموعه_Impl_پایه &LargeSide = هذا->isSmall() ? RHS : *هذا;
  ثابت فراغ **LargeSideInlineStorage =
      هذا->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::جابجا(LargeSide.Cur_Array_اندازه, SmallSide.Cur_Array_اندازه);
  std::جابجا(LargeSide.رقم_Entries, SmallSide.رقم_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = خطأ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = درست;
}
