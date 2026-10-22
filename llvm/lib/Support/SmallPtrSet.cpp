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

استعمال نطاق llvm;

فراغ Small_Ptr_سيٽ_Impl_بنياد::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  حرر(CurArray);

  // Reduce the number of buckets.
  بنانشان سائيز = سائيز_2();
  Cur_Array_سائيز = سائيز > 16 ? 1 << (Log2_32_Ceil(سائيز) + 1) : 32;
  انگ_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * Cur_Array_سائيز);

  memset(CurArray, -1, Cur_Array_سائيز*حجم(فراغ*));
}

std::pair<ثابت فراغ *ثابت *, منطقي>
Small_Ptr_سيٽ_Impl_بنياد::وجهو_imp_big(ثابت فراغ *Ptr) {
  إذا (LLVM_UNLIKELY(سائيز_2() * 3 >= Cur_Array_سائيز * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_سائيز < 64 ? 128 : Cur_Array_سائيز * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  بنانشان نقاب = Cur_Array_سائيز - 1;
  بنانشان I = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
  ثابت فراغ **Array = CurArray;
  طالما (Array[I] != get_خالي_جايون_Marker()) {
    إذا (Array[I] == Ptr)
      أرجع {Array + I, خطأ};
    I = (I + 1) & نقاب;
  }

  // Insert into the empty bucket.
  ++انگ_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  أرجع {Array + I, سچ};
}

ثابت فراغ *ثابت *Small_Ptr_سيٽ_Impl_بنياد::do_ڳوليو(ثابت فراغ *Ptr) ثابت {
  بنانشان نقاب = Cur_Array_سائيز - 1;
  بنانشان ٻالٽي_No = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
  طالما (سچ) {
    ثابت فراغ *ثابت *ٻالٽي = CurArray + ٻالٽي_No;
    إذا (LLVM_LIKELY(*ٻالٽي == Ptr))
      أرجع ٻالٽي;
    إذا (LLVM_LIKELY(*ٻالٽي == get_خالي_جايون_Marker()))
      أرجع خالي_اشارو;
    ٻالٽي_No = (ٻالٽي_No + 1) & نقاب;
  }
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::erase_From_ٻالٽي(ثابت فراغ **ٻالٽي) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  بنانشان نقاب = Cur_Array_سائيز - 1;
  بنانشان I = ٻالٽي - CurArray;
  بنانشان J = I;
  ثابت فراغ *خالي_جايون_2 = get_خالي_جايون_Marker();
  طالما ((J = (J + 1) & نقاب), CurArray[J] != خالي_جايون_2) {
    خودڪار Ideal = DenseMapInfo<فراغ *>::get_Hash_قدر(CurArray[J]);
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = خالي_جايون_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
فراغ Small_Ptr_سيٽ_Impl_بنياد::Grow(بنانشان New_سائيز) {
  خودڪار OldBuckets = buckets();
  منطقي WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت فراغ **NewBuckets = (ثابت فراغ**) محفوظ_malloc(حجم(فراغ*) * New_سائيز);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_سائيز = New_سائيز;
  memset(CurArray, -1, New_سائيز*حجم(فراغ*));

  // Copy over all valid entries.
  بنانشان نقاب = Cur_Array_سائيز - 1;
  لكل (ثابت فراغ *Ptr : OldBuckets) {
    إذا (Ptr == get_خالي_جايون_Marker())
      استمر;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    بنانشان I = DenseMapInfo<فراغ *>::get_Hash_قدر(Ptr) & نقاب;
    طالما (NewBuckets[I] != get_خالي_جايون_Marker())
      I = (I + 1) & نقاب;
    NewBuckets[I] = Ptr;
  }

  إذا (!WasSmall)
    حرر(OldBuckets.begin());
  IsSmall = خطأ;
}

Small_Ptr_سيٽ_Impl_بنياد::Small_Ptr_سيٽ_Impl_بنياد(ثابت فراغ **SmallStorage,
                                         ثابت Small_Ptr_سيٽ_Impl_بنياد &that) {
  IsSmall = that.isSmall();
  إذا (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وإلا {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * that.Cur_Array_سائيز);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_سيٽ_Impl_بنياد::Small_Ptr_سيٽ_Impl_بنياد(ثابت فراغ **SmallStorage,
                                         بنانشان Small_سائيز,
                                         ثابت فراغ **RHSSmallStorage,
                                         Small_Ptr_سيٽ_Impl_بنياد &&that) {
  moveHelper(SmallStorage, Small_سائيز, RHSSmallStorage, std::move(that));
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::copyFrom(ثابت فراغ **SmallStorage,
                                   ثابت Small_Ptr_سيٽ_Impl_بنياد &RHS) {
  أكد(&RHS != هذا && "Self-copy should be handled by the caller.");

  إذا (isSmall() && RHS.isSmall())
    أكد(Cur_Array_سائيز == RHS.Cur_Array_سائيز &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  إذا (RHS.isSmall()) {
    إذا (!isSmall())
      حرر(CurArray);
    CurArray = SmallStorage;
    IsSmall = سچ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وإلا إذا (Cur_Array_سائيز != RHS.Cur_Array_سائيز) {
    إذا (isSmall())
      CurArray = (ثابت فراغ**)محفوظ_malloc(حجم(فراغ*) * RHS.Cur_Array_سائيز);
    وإلا {
      ثابت فراغ **T = (ثابت فراغ**)محفوظ_realloc(CurArray,
                                             حجم(فراغ*) * RHS.Cur_Array_سائيز);
      CurArray = T;
    }
    IsSmall = خطأ;
  }

  copyHelper(RHS);
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::copyHelper(ثابت Small_Ptr_سيٽ_Impl_بنياد &RHS) {
  // Copy over the new array size
  Cur_Array_سائيز = RHS.Cur_Array_سائيز;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  انگ_Entries = RHS.انگ_Entries;
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::moveFrom(ثابت فراغ **SmallStorage,
                                   بنانشان Small_سائيز,
                                   ثابت فراغ **RHSSmallStorage,
                                   Small_Ptr_سيٽ_Impl_بنياد &&RHS) {
  إذا (!isSmall())
    حرر(CurArray);
  moveHelper(SmallStorage, Small_سائيز, RHSSmallStorage, std::move(RHS));
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::moveHelper(ثابت فراغ **SmallStorage,
                                     بنانشان Small_سائيز,
                                     ثابت فراغ **RHSSmallStorage,
                                     Small_Ptr_سيٽ_Impl_بنياد &&RHS) {
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
  Cur_Array_سائيز = RHS.Cur_Array_سائيز;
  انگ_Entries = RHS.انگ_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_سائيز = Small_سائيز;
  RHS.انگ_Entries = 0;
  RHS.IsSmall = سچ;
}

فراغ Small_Ptr_سيٽ_Impl_بنياد::مٽا_سٽا(ثابت فراغ **SmallStorage,
                               ثابت فراغ **RHSSmallStorage,
                               Small_Ptr_سيٽ_Impl_بنياد &RHS) {
  إذا (هذا == &RHS) أرجع;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  إذا (!هذا->isSmall() && !RHS.isSmall()) {
    std::مٽا_سٽا(هذا->CurArray, RHS.CurArray);
    std::مٽا_سٽا(هذا->Cur_Array_سائيز, RHS.Cur_Array_سائيز);
    std::مٽا_سٽا(هذا->انگ_Entries, RHS.انگ_Entries);
    أرجع;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  إذا (هذا->isSmall() && RHS.isSmall()) {
    بنانشان MinEntries = std::min(هذا->انگ_Entries, RHS.انگ_Entries);
    std::مٽا_سٽا_ranges(هذا->CurArray, هذا->CurArray + MinEntries, RHS.CurArray);
    إذا (هذا->انگ_Entries > MinEntries) {
      std::copy(هذا->CurArray + MinEntries, هذا->CurArray + هذا->انگ_Entries,
                RHS.CurArray + MinEntries);
    } وإلا {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.انگ_Entries,
                هذا->CurArray + MinEntries);
    }
    أكد(هذا->Cur_Array_سائيز == RHS.Cur_Array_سائيز);
    std::مٽا_سٽا(هذا->انگ_Entries, RHS.انگ_Entries);
    أرجع;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_سيٽ_Impl_بنياد &SmallSide = هذا->isSmall() ? *هذا : RHS;
  Small_Ptr_سيٽ_Impl_بنياد &LargeSide = هذا->isSmall() ? RHS : *هذا;
  ثابت فراغ **LargeSideInlineStorage =
      هذا->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::مٽا_سٽا(LargeSide.Cur_Array_سائيز, SmallSide.Cur_Array_سائيز);
  std::مٽا_سٽا(LargeSide.انگ_Entries, SmallSide.انگ_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = خطأ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = سچ;
}
