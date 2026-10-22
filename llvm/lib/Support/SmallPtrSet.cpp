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

#অন্তর্ভাব "llvm/ADT/SmallPtrSet.h"
#অন্তর্ভাব "llvm/ADT/DenseMapInfo.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/MemAlloc.h"
#অন্তর্ভাব <algorithm>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdlib>

using নামাকাশ llvm;

শূন্য SmallPtrSetImplBase::shrink_and_clear() {
  প্রতিজ্ঞা(!isSmall() && "Can't shrink a small set!");
  মুক্ত(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  অঙ্ক_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ধ্রুব শূন্য**)safe_malloc(পরিমাণ(শূন্য*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*পরিমাণ(শূন্য*));
}

std::pair<ধ্রুব শূন্য *ধ্রুব *, তার্কিক>
SmallPtrSetImplBase::ঢোকাও_imp_big(ধ্রুব শূন্য *Ptr) {
  যদি (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
  ধ্রুব শূন্য **Array = CurArray;
  যাবত্ (Array[I] != getEmptyMarker()) {
    যদি (Array[I] == Ptr)
      প্রত্যাগম {Array + I, অসত্য};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++অঙ্ক_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  প্রত্যাগম {Array + I, সত্য};
}

ধ্রুব শূন্য *ধ্রুব *SmallPtrSetImplBase::doFind(ধ্রুব শূন্য *Ptr) ধ্রুব {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
  যাবত্ (সত্য) {
    ধ্রুব শূন্য *ধ্রুব *Bucket = CurArray + BucketNo;
    যদি (LLVM_LIKELY(*Bucket == Ptr))
      প্রত্যাগম Bucket;
    যদি (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      প্রত্যাগম nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

শূন্য SmallPtrSetImplBase::eraseFromBucket(ধ্রুব শূন্য **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ধ্রুব শূন্য *Empty = getEmptyMarker();
  যাবত্ ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<শূন্য *>::get_Hash_মান(CurArray[J]);
    যদি (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
শূন্য SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  তার্কিক WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ধ্রুব শূন্য **NewBuckets = (ধ্রুব শূন্য**) safe_malloc(পরিমাণ(শূন্য*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*পরিমাণ(শূন্য*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  প্রতি (ধ্রুব শূন্য *Ptr : OldBuckets) {
    যদি (Ptr == getEmptyMarker())
      অনুবর্ত;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
    যাবত্ (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  যদি (!WasSmall)
    মুক্ত(OldBuckets.begin());
  IsSmall = অসত্য;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ধ্রুব শূন্য **SmallStorage,
                                         ধ্রুব SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  যদি (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } অন্যথা {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ধ্রুব শূন্য**)safe_malloc(পরিমাণ(শূন্য*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ধ্রুব শূন্য **SmallStorage,
                                         unsigned SmallSize,
                                         ধ্রুব শূন্য **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

শূন্য SmallPtrSetImplBase::copyFrom(ধ্রুব শূন্য **SmallStorage,
                                   ধ্রুব SmallPtrSetImplBase &RHS) {
  প্রতিজ্ঞা(&RHS != এতত্ && "Self-copy should be handled by the caller.");

  যদি (isSmall() && RHS.isSmall())
    প্রতিজ্ঞা(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  যদি (RHS.isSmall()) {
    যদি (!isSmall())
      মুক্ত(CurArray);
    CurArray = SmallStorage;
    IsSmall = সত্য;
    // Otherwise, allocate new heap space (unless we were the same size)
  } অন্যথা যদি (CurArraySize != RHS.CurArraySize) {
    যদি (isSmall())
      CurArray = (ধ্রুব শূন্য**)safe_malloc(পরিমাণ(শূন্য*) * RHS.CurArraySize);
    অন্যথা {
      ধ্রুব শূন্য **T = (ধ্রুব শূন্য**)safe_realloc(CurArray,
                                             পরিমাণ(শূন্য*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = অসত্য;
  }

  copyHelper(RHS);
}

শূন্য SmallPtrSetImplBase::copyHelper(ধ্রুব SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  অঙ্ক_Entries = RHS.অঙ্ক_Entries;
}

শূন্য SmallPtrSetImplBase::moveFrom(ধ্রুব শূন্য **SmallStorage,
                                   unsigned SmallSize,
                                   ধ্রুব শূন্য **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  যদি (!isSmall())
    মুক্ত(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

শূন্য SmallPtrSetImplBase::moveHelper(ধ্রুব শূন্য **SmallStorage,
                                     unsigned SmallSize,
                                     ধ্রুব শূন্য **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  প্রতিজ্ঞা(&RHS != এতত্ && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  যদি (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } অন্যথা {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  অঙ্ক_Entries = RHS.অঙ্ক_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.অঙ্ক_Entries = 0;
  RHS.IsSmall = সত্য;
}

শূন্য SmallPtrSetImplBase::swap(ধ্রুব শূন্য **SmallStorage,
                               ধ্রুব শূন্য **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  যদি (এতত্ == &RHS) প্রত্যাগম;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  যদি (!এতত্->isSmall() && !RHS.isSmall()) {
    std::swap(এতত্->CurArray, RHS.CurArray);
    std::swap(এতত্->CurArraySize, RHS.CurArraySize);
    std::swap(এতত্->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    প্রত্যাগম;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  যদি (এতত্->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(এতত্->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    std::swap_ranges(এতত্->CurArray, এতত্->CurArray + MinEntries, RHS.CurArray);
    যদি (এতত্->অঙ্ক_Entries > MinEntries) {
      std::copy(এতত্->CurArray + MinEntries, এতত্->CurArray + এতত্->অঙ্ক_Entries,
                RHS.CurArray + MinEntries);
    } অন্যথা {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.অঙ্ক_Entries,
                এতত্->CurArray + MinEntries);
    }
    প্রতিজ্ঞা(এতত্->CurArraySize == RHS.CurArraySize);
    std::swap(এতত্->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    প্রত্যাগম;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = এতত্->isSmall() ? *এতত্ : RHS;
  SmallPtrSetImplBase &LargeSide = এতত্->isSmall() ? RHS : *এতত্;
  ধ্রুব শূন্য **LargeSideInlineStorage =
      এতত্->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.অঙ্ক_Entries, SmallSide.অঙ্ক_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = অসত্য;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = সত্য;
}
