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

#شامل "llvm/ADT/SmallPtrSet.h"
#شامل "llvm/ADT/DenseMapInfo.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/MemAlloc.h"
#شامل <algorithm>
#شامل <cassert>
#شامل <cstdlib>

استعمال نامخلا llvm;

خالی Small_Ptr_سیٹ_Impl_بنیاد::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  آزاد(CurArray);

  // Reduce the number of buckets.
  غیرعلامتی سائز = سائز_2();
  Cur_Array_سائز = سائز > 16 ? 1 << (Log2_32_Ceil(سائز) + 1) : 32;
  عدد_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت خالی**)محفوظ_malloc(حجم(خالی*) * Cur_Array_سائز);

  memset(CurArray, -1, Cur_Array_سائز*حجم(خالی*));
}

std::pair<ثابت خالی *ثابت *, منطقی>
Small_Ptr_سیٹ_Impl_بنیاد::داخل_کرو_imp_big(ثابت خالی *Ptr) {
  اگر (LLVM_UNLIKELY(سائز_2() * 3 >= Cur_Array_سائز * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_سائز < 64 ? 128 : Cur_Array_سائز * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  غیرعلامتی نقاب = Cur_Array_سائز - 1;
  غیرعلامتی I = DenseMapInfo<خالی *>::get_Hash_قدر(Ptr) & نقاب;
  ثابت خالی **Array = CurArray;
  جب (Array[I] != get_خالی_خانے_Marker()) {
    اگر (Array[I] == Ptr)
      واپس {Array + I, جھوٹ};
    I = (I + 1) & نقاب;
  }

  // Insert into the empty bucket.
  ++عدد_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  واپس {Array + I, سچ};
}

ثابت خالی *ثابت *Small_Ptr_سیٹ_Impl_بنیاد::do_ڈھونڈو(ثابت خالی *Ptr) ثابت {
  غیرعلامتی نقاب = Cur_Array_سائز - 1;
  غیرعلامتی بالٹی_No = DenseMapInfo<خالی *>::get_Hash_قدر(Ptr) & نقاب;
  جب (سچ) {
    ثابت خالی *ثابت *بالٹی = CurArray + بالٹی_No;
    اگر (LLVM_LIKELY(*بالٹی == Ptr))
      واپس بالٹی;
    اگر (LLVM_LIKELY(*بالٹی == get_خالی_خانے_Marker()))
      واپس خالی_اشاریہ;
    بالٹی_No = (بالٹی_No + 1) & نقاب;
  }
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::erase_From_بالٹی(ثابت خالی **بالٹی) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  غیرعلامتی نقاب = Cur_Array_سائز - 1;
  غیرعلامتی I = بالٹی - CurArray;
  غیرعلامتی J = I;
  ثابت خالی *خالی_خانے_2 = get_خالی_خانے_Marker();
  جب ((J = (J + 1) & نقاب), CurArray[J] != خالی_خانے_2) {
    خودکار Ideal = DenseMapInfo<خالی *>::get_Hash_قدر(CurArray[J]);
    اگر (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = خالی_خانے_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
خالی Small_Ptr_سیٹ_Impl_بنیاد::Grow(غیرعلامتی New_سائز) {
  خودکار OldBuckets = buckets();
  منطقی WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت خالی **NewBuckets = (ثابت خالی**) محفوظ_malloc(حجم(خالی*) * New_سائز);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_سائز = New_سائز;
  memset(CurArray, -1, New_سائز*حجم(خالی*));

  // Copy over all valid entries.
  غیرعلامتی نقاب = Cur_Array_سائز - 1;
  برائے (ثابت خالی *Ptr : OldBuckets) {
    اگر (Ptr == get_خالی_خانے_Marker())
      جاری;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    غیرعلامتی I = DenseMapInfo<خالی *>::get_Hash_قدر(Ptr) & نقاب;
    جب (NewBuckets[I] != get_خالی_خانے_Marker())
      I = (I + 1) & نقاب;
    NewBuckets[I] = Ptr;
  }

  اگر (!WasSmall)
    آزاد(OldBuckets.begin());
  IsSmall = جھوٹ;
}

Small_Ptr_سیٹ_Impl_بنیاد::Small_Ptr_سیٹ_Impl_بنیاد(ثابت خالی **SmallStorage,
                                         ثابت Small_Ptr_سیٹ_Impl_بنیاد &that) {
  IsSmall = that.isSmall();
  اگر (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } ورنہ {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت خالی**)محفوظ_malloc(حجم(خالی*) * that.Cur_Array_سائز);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_سیٹ_Impl_بنیاد::Small_Ptr_سیٹ_Impl_بنیاد(ثابت خالی **SmallStorage,
                                         غیرعلامتی Small_سائز,
                                         ثابت خالی **RHSSmallStorage,
                                         Small_Ptr_سیٹ_Impl_بنیاد &&that) {
  moveHelper(SmallStorage, Small_سائز, RHSSmallStorage, std::move(that));
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::copyFrom(ثابت خالی **SmallStorage,
                                   ثابت Small_Ptr_سیٹ_Impl_بنیاد &RHS) {
  أكد(&RHS != یہ && "Self-copy should be handled by the caller.");

  اگر (isSmall() && RHS.isSmall())
    أكد(Cur_Array_سائز == RHS.Cur_Array_سائز &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  اگر (RHS.isSmall()) {
    اگر (!isSmall())
      آزاد(CurArray);
    CurArray = SmallStorage;
    IsSmall = سچ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } ورنہ اگر (Cur_Array_سائز != RHS.Cur_Array_سائز) {
    اگر (isSmall())
      CurArray = (ثابت خالی**)محفوظ_malloc(حجم(خالی*) * RHS.Cur_Array_سائز);
    ورنہ {
      ثابت خالی **T = (ثابت خالی**)محفوظ_realloc(CurArray,
                                             حجم(خالی*) * RHS.Cur_Array_سائز);
      CurArray = T;
    }
    IsSmall = جھوٹ;
  }

  copyHelper(RHS);
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::copyHelper(ثابت Small_Ptr_سیٹ_Impl_بنیاد &RHS) {
  // Copy over the new array size
  Cur_Array_سائز = RHS.Cur_Array_سائز;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  عدد_Entries = RHS.عدد_Entries;
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::moveFrom(ثابت خالی **SmallStorage,
                                   غیرعلامتی Small_سائز,
                                   ثابت خالی **RHSSmallStorage,
                                   Small_Ptr_سیٹ_Impl_بنیاد &&RHS) {
  اگر (!isSmall())
    آزاد(CurArray);
  moveHelper(SmallStorage, Small_سائز, RHSSmallStorage, std::move(RHS));
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::moveHelper(ثابت خالی **SmallStorage,
                                     غیرعلامتی Small_سائز,
                                     ثابت خالی **RHSSmallStorage,
                                     Small_Ptr_سیٹ_Impl_بنیاد &&RHS) {
  أكد(&RHS != یہ && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  اگر (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } ورنہ {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_سائز = RHS.Cur_Array_سائز;
  عدد_Entries = RHS.عدد_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_سائز = Small_سائز;
  RHS.عدد_Entries = 0;
  RHS.IsSmall = سچ;
}

خالی Small_Ptr_سیٹ_Impl_بنیاد::تبادلہ(ثابت خالی **SmallStorage,
                               ثابت خالی **RHSSmallStorage,
                               Small_Ptr_سیٹ_Impl_بنیاد &RHS) {
  اگر (یہ == &RHS) واپس;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  اگر (!یہ->isSmall() && !RHS.isSmall()) {
    std::تبادلہ(یہ->CurArray, RHS.CurArray);
    std::تبادلہ(یہ->Cur_Array_سائز, RHS.Cur_Array_سائز);
    std::تبادلہ(یہ->عدد_Entries, RHS.عدد_Entries);
    واپس;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  اگر (یہ->isSmall() && RHS.isSmall()) {
    غیرعلامتی MinEntries = std::min(یہ->عدد_Entries, RHS.عدد_Entries);
    std::تبادلہ_ranges(یہ->CurArray, یہ->CurArray + MinEntries, RHS.CurArray);
    اگر (یہ->عدد_Entries > MinEntries) {
      std::copy(یہ->CurArray + MinEntries, یہ->CurArray + یہ->عدد_Entries,
                RHS.CurArray + MinEntries);
    } ورنہ {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.عدد_Entries,
                یہ->CurArray + MinEntries);
    }
    أكد(یہ->Cur_Array_سائز == RHS.Cur_Array_سائز);
    std::تبادلہ(یہ->عدد_Entries, RHS.عدد_Entries);
    واپس;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_سیٹ_Impl_بنیاد &SmallSide = یہ->isSmall() ? *یہ : RHS;
  Small_Ptr_سیٹ_Impl_بنیاد &LargeSide = یہ->isSmall() ? RHS : *یہ;
  ثابت خالی **LargeSideInlineStorage =
      یہ->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::تبادلہ(LargeSide.Cur_Array_سائز, SmallSide.Cur_Array_سائز);
  std::تبادلہ(LargeSide.عدد_Entries, SmallSide.عدد_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = جھوٹ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = سچ;
}
