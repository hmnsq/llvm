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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallPtrSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/DenseMapInfo.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdlib>

using ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ SmallPtrSetImplBase::shrink_and_clear() {
  ಪ್ರತಿಜ್ಞಾ(!isSmall() && "Can't shrink a small set!");
  ಮುಕ್ತ(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  ಅಂಕೆ_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ಧ್ರುವ ಶೂನ್ಯ**)safe_malloc(ಪರಿಮಾಣ(ಶೂನ್ಯ*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*ಪರಿಮಾಣ(ಶೂನ್ಯ*));
}

std::pair<ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *, ತಾರ್ಕಿಕ>
SmallPtrSetImplBase::ಸೇರಿಸು_imp_big(ಧ್ರುವ ಶೂನ್ಯ *Ptr) {
  ಯದಿ (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & Mask;
  ಧ್ರುವ ಶೂನ್ಯ **Array = CurArray;
  ಯಾವತ್ (Array[I] != getEmptyMarker()) {
    ಯದಿ (Array[I] == Ptr)
      ಪ್ರತ್ಯಾಗಮ {Array + I, ಅಸತ್ಯ};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++ಅಂಕೆ_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  ಪ್ರತ್ಯಾಗಮ {Array + I, ಸತ್ಯ};
}

ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *SmallPtrSetImplBase::doFind(ಧ್ರುವ ಶೂನ್ಯ *Ptr) ಧ್ರುವ {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & Mask;
  ಯಾವತ್ (ಸತ್ಯ) {
    ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *Bucket = CurArray + BucketNo;
    ಯದಿ (LLVM_LIKELY(*Bucket == Ptr))
      ಪ್ರತ್ಯಾಗಮ Bucket;
    ಯದಿ (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      ಪ್ರತ್ಯಾಗಮ nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

ಶೂನ್ಯ SmallPtrSetImplBase::eraseFromBucket(ಧ್ರುವ ಶೂನ್ಯ **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ಧ್ರುವ ಶೂನ್ಯ *Empty = getEmptyMarker();
  ಯಾವತ್ ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(CurArray[J]);
    ಯದಿ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
ಶೂನ್ಯ SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  ತಾರ್ಕಿಕ WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ಧ್ರುವ ಶೂನ್ಯ **NewBuckets = (ಧ್ರುವ ಶೂನ್ಯ**) safe_malloc(ಪರಿಮಾಣ(ಶೂನ್ಯ*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*ಪರಿಮಾಣ(ಶೂನ್ಯ*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  ಪ್ರತಿ (ಧ್ರುವ ಶೂನ್ಯ *Ptr : OldBuckets) {
    ಯದಿ (Ptr == getEmptyMarker())
      ಅನುವರ್ತ;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & Mask;
    ಯಾವತ್ (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  ಯದಿ (!WasSmall)
    ಮುಕ್ತ(OldBuckets.begin());
  IsSmall = ಅಸತ್ಯ;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                                         ಧ್ರುವ SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  ಯದಿ (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } ಅನ್ಯಥಾ {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ಧ್ರುವ ಶೂನ್ಯ**)safe_malloc(ಪರಿಮಾಣ(ಶೂನ್ಯ*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                                         unsigned SmallSize,
                                         ಧ್ರುವ ಶೂನ್ಯ **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

ಶೂನ್ಯ SmallPtrSetImplBase::copyFrom(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                                   ಧ್ರುವ SmallPtrSetImplBase &RHS) {
  ಪ್ರತಿಜ್ಞಾ(&RHS != ಏತತ್ && "Self-copy should be handled by the caller.");

  ಯದಿ (isSmall() && RHS.isSmall())
    ಪ್ರತಿಜ್ಞಾ(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  ಯದಿ (RHS.isSmall()) {
    ಯದಿ (!isSmall())
      ಮುಕ್ತ(CurArray);
    CurArray = SmallStorage;
    IsSmall = ಸತ್ಯ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } ಅನ್ಯಥಾ ಯದಿ (CurArraySize != RHS.CurArraySize) {
    ಯದಿ (isSmall())
      CurArray = (ಧ್ರುವ ಶೂನ್ಯ**)safe_malloc(ಪರಿಮಾಣ(ಶೂನ್ಯ*) * RHS.CurArraySize);
    ಅನ್ಯಥಾ {
      ಧ್ರುವ ಶೂನ್ಯ **T = (ಧ್ರುವ ಶೂನ್ಯ**)safe_realloc(CurArray,
                                             ಪರಿಮಾಣ(ಶೂನ್ಯ*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = ಅಸತ್ಯ;
  }

  copyHelper(RHS);
}

ಶೂನ್ಯ SmallPtrSetImplBase::copyHelper(ಧ್ರುವ SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  ಅಂಕೆ_Entries = RHS.ಅಂಕೆ_Entries;
}

ಶೂನ್ಯ SmallPtrSetImplBase::moveFrom(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                                   unsigned SmallSize,
                                   ಧ್ರುವ ಶೂನ್ಯ **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  ಯದಿ (!isSmall())
    ಮುಕ್ತ(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

ಶೂನ್ಯ SmallPtrSetImplBase::moveHelper(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                                     unsigned SmallSize,
                                     ಧ್ರುವ ಶೂನ್ಯ **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  ಪ್ರತಿಜ್ಞಾ(&RHS != ಏತತ್ && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  ಯದಿ (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } ಅನ್ಯಥಾ {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  ಅಂಕೆ_Entries = RHS.ಅಂಕೆ_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.ಅಂಕೆ_Entries = 0;
  RHS.IsSmall = ಸತ್ಯ;
}

ಶೂನ್ಯ SmallPtrSetImplBase::swap(ಧ್ರುವ ಶೂನ್ಯ **SmallStorage,
                               ಧ್ರುವ ಶೂನ್ಯ **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  ಯದಿ (ಏತತ್ == &RHS) ಪ್ರತ್ಯಾಗಮ;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  ಯದಿ (!ಏತತ್->isSmall() && !RHS.isSmall()) {
    std::swap(ಏತತ್->CurArray, RHS.CurArray);
    std::swap(ಏತತ್->CurArraySize, RHS.CurArraySize);
    std::swap(ಏತತ್->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  ಯದಿ (ಏತತ್->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(ಏತತ್->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    std::swap_ranges(ಏತತ್->CurArray, ಏತತ್->CurArray + MinEntries, RHS.CurArray);
    ಯದಿ (ಏತತ್->ಅಂಕೆ_Entries > MinEntries) {
      std::copy(ಏತತ್->CurArray + MinEntries, ಏತತ್->CurArray + ಏತತ್->ಅಂಕೆ_Entries,
                RHS.CurArray + MinEntries);
    } ಅನ್ಯಥಾ {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.ಅಂಕೆ_Entries,
                ಏತತ್->CurArray + MinEntries);
    }
    ಪ್ರತಿಜ್ಞಾ(ಏತತ್->CurArraySize == RHS.CurArraySize);
    std::swap(ಏತತ್->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = ಏತತ್->isSmall() ? *ಏತತ್ : RHS;
  SmallPtrSetImplBase &LargeSide = ಏತತ್->isSmall() ? RHS : *ಏತತ್;
  ಧ್ರುವ ಶೂನ್ಯ **LargeSideInlineStorage =
      ಏತತ್->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.ಅಂಕೆ_Entries, SmallSide.ಅಂಕೆ_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = ಅಸತ್ಯ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = ಸತ್ಯ;
}
