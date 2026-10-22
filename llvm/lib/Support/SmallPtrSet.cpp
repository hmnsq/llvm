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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallPtrSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/DenseMapInfo.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemAlloc.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstdlib>

using ନାମାକାଶ llvm;

ଶୂନ୍ୟ SmallPtrSetImplBase::shrink_and_clear() {
  ପ୍ରତିଜ୍ଞା(!isSmall() && "Can't shrink a small set!");
  ମୁକ୍ତ(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  ଅଙ୍କ_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ଅଚଳ ଶୂନ୍ୟ**)safe_malloc(ଆକାର(ଶୂନ୍ୟ*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*ଆକାର(ଶୂନ୍ୟ*));
}

std::pair<ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *, ତାର୍କିକ>
SmallPtrSetImplBase::ଭର୍ତ୍ତି_imp_big(ଅଚଳ ଶୂନ୍ୟ *Ptr) {
  ଯଦି (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<ଶୂନ୍ୟ *>::get_Hash_ମୂଲ୍ୟ(Ptr) & Mask;
  ଅଚଳ ଶୂନ୍ୟ **Array = CurArray;
  ଯେତେବେଳେ (Array[I] != getEmptyMarker()) {
    ଯଦି (Array[I] == Ptr)
      ଫେରନ୍ତୁ {Array + I, ମିଥ୍ୟା};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++ଅଙ୍କ_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  ଫେରନ୍ତୁ {Array + I, ସତ୍ୟ};
}

ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *SmallPtrSetImplBase::doFind(ଅଚଳ ଶୂନ୍ୟ *Ptr) ଅଚଳ {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<ଶୂନ୍ୟ *>::get_Hash_ମୂଲ୍ୟ(Ptr) & Mask;
  ଯେତେବେଳେ (ସତ୍ୟ) {
    ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *Bucket = CurArray + BucketNo;
    ଯଦି (LLVM_LIKELY(*Bucket == Ptr))
      ଫେରନ୍ତୁ Bucket;
    ଯଦି (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      ଫେରନ୍ତୁ nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

ଶୂନ୍ୟ SmallPtrSetImplBase::eraseFromBucket(ଅଚଳ ଶୂନ୍ୟ **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ଅଚଳ ଶୂନ୍ୟ *Empty = getEmptyMarker();
  ଯେତେବେଳେ ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<ଶୂନ୍ୟ *>::get_Hash_ମୂଲ୍ୟ(CurArray[J]);
    ଯଦି (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
ଶୂନ୍ୟ SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  ତାର୍କିକ WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ଅଚଳ ଶୂନ୍ୟ **NewBuckets = (ଅଚଳ ଶୂନ୍ୟ**) safe_malloc(ଆକାର(ଶୂନ୍ୟ*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*ଆକାର(ଶୂନ୍ୟ*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  ପାଇଁ (ଅଚଳ ଶୂନ୍ୟ *Ptr : OldBuckets) {
    ଯଦି (Ptr == getEmptyMarker())
      ଜାରି;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<ଶୂନ୍ୟ *>::get_Hash_ମୂଲ୍ୟ(Ptr) & Mask;
    ଯେତେବେଳେ (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  ଯଦି (!WasSmall)
    ମୁକ୍ତ(OldBuckets.begin());
  IsSmall = ମିଥ୍ୟା;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                                         ଅଚଳ SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  ଯଦି (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } ଅନ୍ୟଥା {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ଅଚଳ ଶୂନ୍ୟ**)safe_malloc(ଆକାର(ଶୂନ୍ୟ*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                                         unsigned SmallSize,
                                         ଅଚଳ ଶୂନ୍ୟ **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

ଶୂନ୍ୟ SmallPtrSetImplBase::copyFrom(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                                   ଅଚଳ SmallPtrSetImplBase &RHS) {
  ପ୍ରତିଜ୍ଞା(&RHS != ଏହି && "Self-copy should be handled by the caller.");

  ଯଦି (isSmall() && RHS.isSmall())
    ପ୍ରତିଜ୍ଞା(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  ଯଦି (RHS.isSmall()) {
    ଯଦି (!isSmall())
      ମୁକ୍ତ(CurArray);
    CurArray = SmallStorage;
    IsSmall = ସତ୍ୟ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } ଅନ୍ୟଥା ଯଦି (CurArraySize != RHS.CurArraySize) {
    ଯଦି (isSmall())
      CurArray = (ଅଚଳ ଶୂନ୍ୟ**)safe_malloc(ଆକାର(ଶୂନ୍ୟ*) * RHS.CurArraySize);
    ଅନ୍ୟଥା {
      ଅଚଳ ଶୂନ୍ୟ **T = (ଅଚଳ ଶୂନ୍ୟ**)safe_realloc(CurArray,
                                             ଆକାର(ଶୂନ୍ୟ*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = ମିଥ୍ୟା;
  }

  copyHelper(RHS);
}

ଶୂନ୍ୟ SmallPtrSetImplBase::copyHelper(ଅଚଳ SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  ଅଙ୍କ_Entries = RHS.ଅଙ୍କ_Entries;
}

ଶୂନ୍ୟ SmallPtrSetImplBase::moveFrom(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                                   unsigned SmallSize,
                                   ଅଚଳ ଶୂନ୍ୟ **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  ଯଦି (!isSmall())
    ମୁକ୍ତ(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

ଶୂନ୍ୟ SmallPtrSetImplBase::moveHelper(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                                     unsigned SmallSize,
                                     ଅଚଳ ଶୂନ୍ୟ **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  ପ୍ରତିଜ୍ଞା(&RHS != ଏହି && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  ଯଦି (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } ଅନ୍ୟଥା {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  ଅଙ୍କ_Entries = RHS.ଅଙ୍କ_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.ଅଙ୍କ_Entries = 0;
  RHS.IsSmall = ସତ୍ୟ;
}

ଶୂନ୍ୟ SmallPtrSetImplBase::swap(ଅଚଳ ଶୂନ୍ୟ **SmallStorage,
                               ଅଚଳ ଶୂନ୍ୟ **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  ଯଦି (ଏହି == &RHS) ଫେରନ୍ତୁ;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  ଯଦି (!ଏହି->isSmall() && !RHS.isSmall()) {
    std::swap(ଏହି->CurArray, RHS.CurArray);
    std::swap(ଏହି->CurArraySize, RHS.CurArraySize);
    std::swap(ଏହି->ଅଙ୍କ_Entries, RHS.ଅଙ୍କ_Entries);
    ଫେରନ୍ତୁ;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  ଯଦି (ଏହି->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(ଏହି->ଅଙ୍କ_Entries, RHS.ଅଙ୍କ_Entries);
    std::swap_ranges(ଏହି->CurArray, ଏହି->CurArray + MinEntries, RHS.CurArray);
    ଯଦି (ଏହି->ଅଙ୍କ_Entries > MinEntries) {
      std::copy(ଏହି->CurArray + MinEntries, ଏହି->CurArray + ଏହି->ଅଙ୍କ_Entries,
                RHS.CurArray + MinEntries);
    } ଅନ୍ୟଥା {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.ଅଙ୍କ_Entries,
                ଏହି->CurArray + MinEntries);
    }
    ପ୍ରତିଜ୍ଞା(ଏହି->CurArraySize == RHS.CurArraySize);
    std::swap(ଏହି->ଅଙ୍କ_Entries, RHS.ଅଙ୍କ_Entries);
    ଫେରନ୍ତୁ;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = ଏହି->isSmall() ? *ଏହି : RHS;
  SmallPtrSetImplBase &LargeSide = ଏହି->isSmall() ? RHS : *ଏହି;
  ଅଚଳ ଶୂନ୍ୟ **LargeSideInlineStorage =
      ଏହି->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.ଅଙ୍କ_Entries, SmallSide.ଅଙ୍କ_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = ମିଥ୍ୟା;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = ସତ୍ୟ;
}
