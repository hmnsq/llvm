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

using نطاق llvm;

فراغ SmallPtrSetImplBase::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  حرر(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  اڠک_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت فراغ**)safe_malloc(حجم(فراغ*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*حجم(فراغ*));
}

std::pair<ثابت فراغ *ثابت *, منطقي>
SmallPtrSetImplBase::سيسيڤ_imp_big(ثابت فراغ *Ptr) {
  إذا (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<فراغ *>::get_Hash_نيلاي(Ptr) & Mask;
  ثابت فراغ **Array = CurArray;
  طالما (Array[I] != getEmptyMarker()) {
    إذا (Array[I] == Ptr)
      أرجع {Array + I, خطأ};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++اڠک_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  أرجع {Array + I, true};
}

ثابت فراغ *ثابت *SmallPtrSetImplBase::doFind(ثابت فراغ *Ptr) ثابت {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<فراغ *>::get_Hash_نيلاي(Ptr) & Mask;
  طالما (true) {
    ثابت فراغ *ثابت *Bucket = CurArray + BucketNo;
    إذا (LLVM_LIKELY(*Bucket == Ptr))
      أرجع Bucket;
    إذا (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      أرجع nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

فراغ SmallPtrSetImplBase::eraseFromBucket(ثابت فراغ **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ثابت فراغ *Empty = getEmptyMarker();
  طالما ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<فراغ *>::get_Hash_نيلاي(CurArray[J]);
    إذا (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
فراغ SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  منطقي WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت فراغ **NewBuckets = (ثابت فراغ**) safe_malloc(حجم(فراغ*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*حجم(فراغ*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  لكل (ثابت فراغ *Ptr : OldBuckets) {
    إذا (Ptr == getEmptyMarker())
      استمر;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<فراغ *>::get_Hash_نيلاي(Ptr) & Mask;
    طالما (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  إذا (!WasSmall)
    حرر(OldBuckets.begin());
  IsSmall = خطأ;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ثابت فراغ **SmallStorage,
                                         ثابت SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  إذا (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وإلا {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت فراغ**)safe_malloc(حجم(فراغ*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ثابت فراغ **SmallStorage,
                                         unsigned SmallSize,
                                         ثابت فراغ **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

فراغ SmallPtrSetImplBase::copyFrom(ثابت فراغ **SmallStorage,
                                   ثابت SmallPtrSetImplBase &RHS) {
  أكد(&RHS != هذا && "Self-copy should be handled by the caller.");

  إذا (isSmall() && RHS.isSmall())
    أكد(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  إذا (RHS.isSmall()) {
    إذا (!isSmall())
      حرر(CurArray);
    CurArray = SmallStorage;
    IsSmall = true;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وإلا إذا (CurArraySize != RHS.CurArraySize) {
    إذا (isSmall())
      CurArray = (ثابت فراغ**)safe_malloc(حجم(فراغ*) * RHS.CurArraySize);
    وإلا {
      ثابت فراغ **T = (ثابت فراغ**)safe_realloc(CurArray,
                                             حجم(فراغ*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = خطأ;
  }

  copyHelper(RHS);
}

فراغ SmallPtrSetImplBase::copyHelper(ثابت SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  اڠک_Entries = RHS.اڠک_Entries;
}

فراغ SmallPtrSetImplBase::moveFrom(ثابت فراغ **SmallStorage,
                                   unsigned SmallSize,
                                   ثابت فراغ **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  إذا (!isSmall())
    حرر(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

فراغ SmallPtrSetImplBase::moveHelper(ثابت فراغ **SmallStorage,
                                     unsigned SmallSize,
                                     ثابت فراغ **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
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
  CurArraySize = RHS.CurArraySize;
  اڠک_Entries = RHS.اڠک_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.اڠک_Entries = 0;
  RHS.IsSmall = true;
}

فراغ SmallPtrSetImplBase::swap(ثابت فراغ **SmallStorage,
                               ثابت فراغ **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  إذا (هذا == &RHS) أرجع;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  إذا (!هذا->isSmall() && !RHS.isSmall()) {
    std::swap(هذا->CurArray, RHS.CurArray);
    std::swap(هذا->CurArraySize, RHS.CurArraySize);
    std::swap(هذا->اڠک_Entries, RHS.اڠک_Entries);
    أرجع;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  إذا (هذا->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(هذا->اڠک_Entries, RHS.اڠک_Entries);
    std::swap_ranges(هذا->CurArray, هذا->CurArray + MinEntries, RHS.CurArray);
    إذا (هذا->اڠک_Entries > MinEntries) {
      std::copy(هذا->CurArray + MinEntries, هذا->CurArray + هذا->اڠک_Entries,
                RHS.CurArray + MinEntries);
    } وإلا {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.اڠک_Entries,
                هذا->CurArray + MinEntries);
    }
    أكد(هذا->CurArraySize == RHS.CurArraySize);
    std::swap(هذا->اڠک_Entries, RHS.اڠک_Entries);
    أرجع;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = هذا->isSmall() ? *هذا : RHS;
  SmallPtrSetImplBase &LargeSide = هذا->isSmall() ? RHS : *هذا;
  ثابت فراغ **LargeSideInlineStorage =
      هذا->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.اڠک_Entries, SmallSide.اڠک_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = خطأ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = true;
}
