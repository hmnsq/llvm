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
  unsigned আকার_2 = আকার_3();
  Cur_Array_আকার = আকার_2 > 16 ? 1 << (Log2_32_Ceil(আকার_2) + 1) : 32;
  অঙ্ক_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ধ্রুবক শূন্য**)safe_malloc(আকার(শূন্য*) * Cur_Array_আকার);

  memset(CurArray, -1, Cur_Array_আকার*আকার(শূন্য*));
}

std::pair<ধ্রুবক শূন্য *ধ্রুবক *, যৌক্তিক>
SmallPtrSetImplBase::ঢোকাও_imp_big(ধ্রুবক শূন্য *Ptr) {
  যদি (LLVM_UNLIKELY(আকার_3() * 3 >= Cur_Array_আকার * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_আকার < 64 ? 128 : Cur_Array_আকার * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = Cur_Array_আকার - 1;
  unsigned I = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
  ধ্রুবক শূন্য **Array = CurArray;
  যখন (Array[I] != getEmptyMarker()) {
    যদি (Array[I] == Ptr)
      ফেরত {Array + I, মিথ্যা};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++অঙ্ক_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  ফেরত {Array + I, সত্য};
}

ধ্রুবক শূন্য *ধ্রুবক *SmallPtrSetImplBase::doFind(ধ্রুবক শূন্য *Ptr) ধ্রুবক {
  unsigned Mask = Cur_Array_আকার - 1;
  unsigned BucketNo = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
  যখন (সত্য) {
    ধ্রুবক শূন্য *ধ্রুবক *Bucket = CurArray + BucketNo;
    যদি (LLVM_LIKELY(*Bucket == Ptr))
      ফেরত Bucket;
    যদি (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      ফেরত nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

শূন্য SmallPtrSetImplBase::eraseFromBucket(ধ্রুবক শূন্য **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = Cur_Array_আকার - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  ধ্রুবক শূন্য *Empty = getEmptyMarker();
  যখন ((J = (J + 1) & Mask), CurArray[J] != Empty) {
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
শূন্য SmallPtrSetImplBase::Grow(unsigned New_আকার) {
  auto OldBuckets = buckets();
  যৌক্তিক WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ধ্রুবক শূন্য **NewBuckets = (ধ্রুবক শূন্য**) safe_malloc(আকার(শূন্য*) * New_আকার);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_আকার = New_আকার;
  memset(CurArray, -1, New_আকার*আকার(শূন্য*));

  // Copy over all valid entries.
  unsigned Mask = Cur_Array_আকার - 1;
  জন্য (ধ্রুবক শূন্য *Ptr : OldBuckets) {
    যদি (Ptr == getEmptyMarker())
      চালু;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<শূন্য *>::get_Hash_মান(Ptr) & Mask;
    যখন (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  যদি (!WasSmall)
    মুক্ত(OldBuckets.begin());
  IsSmall = মিথ্যা;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ধ্রুবক শূন্য **SmallStorage,
                                         ধ্রুবক SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  যদি (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } অন্যথা {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ধ্রুবক শূন্য**)safe_malloc(আকার(শূন্য*) * that.Cur_Array_আকার);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ধ্রুবক শূন্য **SmallStorage,
                                         unsigned Small_আকার,
                                         ধ্রুবক শূন্য **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, Small_আকার, RHSSmallStorage, std::move(that));
}

শূন্য SmallPtrSetImplBase::copyFrom(ধ্রুবক শূন্য **SmallStorage,
                                   ধ্রুবক SmallPtrSetImplBase &RHS) {
  প্রতিজ্ঞা(&RHS != এই && "Self-copy should be handled by the caller.");

  যদি (isSmall() && RHS.isSmall())
    প্রতিজ্ঞা(Cur_Array_আকার == RHS.Cur_Array_আকার &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  যদি (RHS.isSmall()) {
    যদি (!isSmall())
      মুক্ত(CurArray);
    CurArray = SmallStorage;
    IsSmall = সত্য;
    // Otherwise, allocate new heap space (unless we were the same size)
  } অন্যথা যদি (Cur_Array_আকার != RHS.Cur_Array_আকার) {
    যদি (isSmall())
      CurArray = (ধ্রুবক শূন্য**)safe_malloc(আকার(শূন্য*) * RHS.Cur_Array_আকার);
    অন্যথা {
      ধ্রুবক শূন্য **T = (ধ্রুবক শূন্য**)safe_realloc(CurArray,
                                             আকার(শূন্য*) * RHS.Cur_Array_আকার);
      CurArray = T;
    }
    IsSmall = মিথ্যা;
  }

  copyHelper(RHS);
}

শূন্য SmallPtrSetImplBase::copyHelper(ধ্রুবক SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  Cur_Array_আকার = RHS.Cur_Array_আকার;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  অঙ্ক_Entries = RHS.অঙ্ক_Entries;
}

শূন্য SmallPtrSetImplBase::moveFrom(ধ্রুবক শূন্য **SmallStorage,
                                   unsigned Small_আকার,
                                   ধ্রুবক শূন্য **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  যদি (!isSmall())
    মুক্ত(CurArray);
  moveHelper(SmallStorage, Small_আকার, RHSSmallStorage, std::move(RHS));
}

শূন্য SmallPtrSetImplBase::moveHelper(ধ্রুবক শূন্য **SmallStorage,
                                     unsigned Small_আকার,
                                     ধ্রুবক শূন্য **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  প্রতিজ্ঞা(&RHS != এই && "Self-move should be handled by the caller.");
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
  Cur_Array_আকার = RHS.Cur_Array_আকার;
  অঙ্ক_Entries = RHS.অঙ্ক_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_আকার = Small_আকার;
  RHS.অঙ্ক_Entries = 0;
  RHS.IsSmall = সত্য;
}

শূন্য SmallPtrSetImplBase::swap(ধ্রুবক শূন্য **SmallStorage,
                               ধ্রুবক শূন্য **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  যদি (এই == &RHS) ফেরত;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  যদি (!এই->isSmall() && !RHS.isSmall()) {
    std::swap(এই->CurArray, RHS.CurArray);
    std::swap(এই->Cur_Array_আকার, RHS.Cur_Array_আকার);
    std::swap(এই->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    ফেরত;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  যদি (এই->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(এই->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    std::swap_ranges(এই->CurArray, এই->CurArray + MinEntries, RHS.CurArray);
    যদি (এই->অঙ্ক_Entries > MinEntries) {
      std::copy(এই->CurArray + MinEntries, এই->CurArray + এই->অঙ্ক_Entries,
                RHS.CurArray + MinEntries);
    } অন্যথা {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.অঙ্ক_Entries,
                এই->CurArray + MinEntries);
    }
    প্রতিজ্ঞা(এই->Cur_Array_আকার == RHS.Cur_Array_আকার);
    std::swap(এই->অঙ্ক_Entries, RHS.অঙ্ক_Entries);
    ফেরত;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = এই->isSmall() ? *এই : RHS;
  SmallPtrSetImplBase &LargeSide = এই->isSmall() ? RHS : *এই;
  ধ্রুবক শূন্য **LargeSideInlineStorage =
      এই->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.Cur_Array_আকার, SmallSide.Cur_Array_আকার);
  std::swap(LargeSide.অঙ্ক_Entries, SmallSide.অঙ্ক_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = মিথ্যা;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = সত্য;
}
