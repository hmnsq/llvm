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

using فضاینام llvm;

تهی SmallPtrSetImplBase::shrink_and_clear() {
  أكد(!isSmall() && "Can't shrink a small set!");
  آزاد(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  رقم_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ثابت تهی**)safe_malloc(اندازه(تهی*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*اندازه(تهی*));
}

std::pair<ثابت تهی *ثابت *, منطقی>
SmallPtrSetImplBase::درج_imp_big(ثابت تهی *Ptr) {
  اگر (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<تهی *>::get_Hash_مقدار(Ptr) & Mask;
  ثابت تهی **Array = CurArray;
  وقتی (Array[I] != getEmptyMarker()) {
    اگر (Array[I] == Ptr)
      بازگشت {Array + I, نادرست};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++رقم_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  بازگشت {Array + I, درست};
}

ثابت تهی *ثابت *SmallPtrSetImplBase::doFind(ثابت تهی *Ptr) ثابت {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<تهی *>::get_Hash_مقدار(Ptr) & Mask;
  وقتی (درست) {
    ثابت تهی *ثابت *Bucket = CurArray + BucketNo;
    اگر (LLVM_LIKELY(*Bucket == Ptr))
      بازگشت Bucket;
    اگر (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      بازگشت nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

تهی SmallPtrSetImplBase::eraseFromBucket(ثابت تهی **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ثابت تهی *Empty = getEmptyMarker();
  وقتی ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<تهی *>::get_Hash_مقدار(CurArray[J]);
    اگر (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
تهی SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  منطقی WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ثابت تهی **NewBuckets = (ثابت تهی**) safe_malloc(اندازه(تهی*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*اندازه(تهی*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  برای (ثابت تهی *Ptr : OldBuckets) {
    اگر (Ptr == getEmptyMarker())
      ادامه;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<تهی *>::get_Hash_مقدار(Ptr) & Mask;
    وقتی (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  اگر (!WasSmall)
    آزاد(OldBuckets.begin());
  IsSmall = نادرست;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ثابت تهی **SmallStorage,
                                         ثابت SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  اگر (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } وگرنه {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ثابت تهی**)safe_malloc(اندازه(تهی*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ثابت تهی **SmallStorage,
                                         unsigned SmallSize,
                                         ثابت تهی **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

تهی SmallPtrSetImplBase::copyFrom(ثابت تهی **SmallStorage,
                                   ثابت SmallPtrSetImplBase &RHS) {
  أكد(&RHS != این && "Self-copy should be handled by the caller.");

  اگر (isSmall() && RHS.isSmall())
    أكد(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  اگر (RHS.isSmall()) {
    اگر (!isSmall())
      آزاد(CurArray);
    CurArray = SmallStorage;
    IsSmall = درست;
    // Otherwise, allocate new heap space (unless we were the same size)
  } وگرنه اگر (CurArraySize != RHS.CurArraySize) {
    اگر (isSmall())
      CurArray = (ثابت تهی**)safe_malloc(اندازه(تهی*) * RHS.CurArraySize);
    وگرنه {
      ثابت تهی **T = (ثابت تهی**)safe_realloc(CurArray,
                                             اندازه(تهی*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = نادرست;
  }

  copyHelper(RHS);
}

تهی SmallPtrSetImplBase::copyHelper(ثابت SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  رقم_Entries = RHS.رقم_Entries;
}

تهی SmallPtrSetImplBase::moveFrom(ثابت تهی **SmallStorage,
                                   unsigned SmallSize,
                                   ثابت تهی **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  اگر (!isSmall())
    آزاد(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

تهی SmallPtrSetImplBase::moveHelper(ثابت تهی **SmallStorage,
                                     unsigned SmallSize,
                                     ثابت تهی **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  أكد(&RHS != این && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  اگر (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } وگرنه {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  رقم_Entries = RHS.رقم_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.رقم_Entries = 0;
  RHS.IsSmall = درست;
}

تهی SmallPtrSetImplBase::swap(ثابت تهی **SmallStorage,
                               ثابت تهی **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  اگر (این == &RHS) بازگشت;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  اگر (!این->isSmall() && !RHS.isSmall()) {
    std::swap(این->CurArray, RHS.CurArray);
    std::swap(این->CurArraySize, RHS.CurArraySize);
    std::swap(این->رقم_Entries, RHS.رقم_Entries);
    بازگشت;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  اگر (این->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(این->رقم_Entries, RHS.رقم_Entries);
    std::swap_ranges(این->CurArray, این->CurArray + MinEntries, RHS.CurArray);
    اگر (این->رقم_Entries > MinEntries) {
      std::copy(این->CurArray + MinEntries, این->CurArray + این->رقم_Entries,
                RHS.CurArray + MinEntries);
    } وگرنه {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.رقم_Entries,
                این->CurArray + MinEntries);
    }
    أكد(این->CurArraySize == RHS.CurArraySize);
    std::swap(این->رقم_Entries, RHS.رقم_Entries);
    بازگشت;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = این->isSmall() ? *این : RHS;
  SmallPtrSetImplBase &LargeSide = این->isSmall() ? RHS : *این;
  ثابت تهی **LargeSideInlineStorage =
      این->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.رقم_Entries, SmallSide.رقم_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = نادرست;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = درست;
}
