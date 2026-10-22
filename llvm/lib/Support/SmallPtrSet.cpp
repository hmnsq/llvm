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

#अन्तर्भाव "llvm/ADT/SmallPtrSet.h"
#अन्तर्भाव "llvm/ADT/DenseMapInfo.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/MemAlloc.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdlib>

using नामाकाश llvm;

शून्य SmallPtrSetImplBase::shrink_and_clear() {
  प्रतिज्ञा(!isSmall() && "Can't shrink a small set!");
  मुक्त(CurArray);

  // Reduce the number of buckets.
  अचिन्हित Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  अंक_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * CurArraySize);

  स_रखो(CurArray, -1, CurArraySize*परिमाण(शून्य*));
}

std::pair<ध्रुव शून्य *ध्रुव *, तार्किक>
SmallPtrSetImplBase::जोड़ो_imp_big(ध्रुव शून्य *Ptr) {
  यदि (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित I = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
  ध्रुव शून्य **Array = CurArray;
  यावत् (Array[I] != getEmptyMarker()) {
    यदि (Array[I] == Ptr)
      प्रत्यागम {Array + I, असत्य};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++अंक_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  प्रत्यागम {Array + I, सत्य};
}

ध्रुव शून्य *ध्रुव *SmallPtrSetImplBase::doFind(ध्रुव शून्य *Ptr) ध्रुव {
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित BucketNo = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
  यावत् (सत्य) {
    ध्रुव शून्य *ध्रुव *Bucket = CurArray + BucketNo;
    यदि (LLVM_LIKELY(*Bucket == Ptr))
      प्रत्यागम Bucket;
    यदि (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      प्रत्यागम nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

शून्य SmallPtrSetImplBase::eraseFromBucket(ध्रुव शून्य **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित I = Bucket - CurArray;
  अचिन्हित J = I;
  ध्रुव शून्य *Empty = getEmptyMarker();
  यावत् ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    स्वतः Ideal = DenseMapInfo<शून्य *>::get_Hash_मूल्य(CurArray[J]);
    यदि (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
शून्य SmallPtrSetImplBase::Grow(अचिन्हित NewSize) {
  स्वतः OldBuckets = buckets();
  तार्किक WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ध्रुव शून्य **NewBuckets = (ध्रुव शून्य**) safe_malloc(परिमाण(शून्य*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  स_रखो(CurArray, -1, NewSize*परिमाण(शून्य*));

  // Copy over all valid entries.
  अचिन्हित Mask = CurArraySize - 1;
  प्रति (ध्रुव शून्य *Ptr : OldBuckets) {
    यदि (Ptr == getEmptyMarker())
      अनुवर्त;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    अचिन्हित I = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
    यावत् (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  यदि (!WasSmall)
    मुक्त(OldBuckets.begin());
  IsSmall = असत्य;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ध्रुव शून्य **SmallStorage,
                                         ध्रुव SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  यदि (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } अन्यथा {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(ध्रुव शून्य **SmallStorage,
                                         अचिन्हित SmallSize,
                                         ध्रुव शून्य **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

शून्य SmallPtrSetImplBase::copyFrom(ध्रुव शून्य **SmallStorage,
                                   ध्रुव SmallPtrSetImplBase &RHS) {
  प्रतिज्ञा(&RHS != एतत् && "Self-copy should be handled by the caller.");

  यदि (isSmall() && RHS.isSmall())
    प्रतिज्ञा(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  यदि (RHS.isSmall()) {
    यदि (!isSmall())
      मुक्त(CurArray);
    CurArray = SmallStorage;
    IsSmall = सत्य;
    // Otherwise, allocate new heap space (unless we were the same size)
  } अन्यथा यदि (CurArraySize != RHS.CurArraySize) {
    यदि (isSmall())
      CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * RHS.CurArraySize);
    अन्यथा {
      ध्रुव शून्य **T = (ध्रुव शून्य**)safe_realloc(CurArray,
                                             परिमाण(शून्य*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = असत्य;
  }

  copyHelper(RHS);
}

शून्य SmallPtrSetImplBase::copyHelper(ध्रुव SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  अंक_Entries = RHS.अंक_Entries;
}

शून्य SmallPtrSetImplBase::moveFrom(ध्रुव शून्य **SmallStorage,
                                   अचिन्हित SmallSize,
                                   ध्रुव शून्य **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  यदि (!isSmall())
    मुक्त(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

शून्य SmallPtrSetImplBase::moveHelper(ध्रुव शून्य **SmallStorage,
                                     अचिन्हित SmallSize,
                                     ध्रुव शून्य **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  प्रतिज्ञा(&RHS != एतत् && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  यदि (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } अन्यथा {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  अंक_Entries = RHS.अंक_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.अंक_Entries = 0;
  RHS.IsSmall = सत्य;
}

शून्य SmallPtrSetImplBase::swap(ध्रुव शून्य **SmallStorage,
                               ध्रुव शून्य **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  यदि (एतत् == &RHS) प्रत्यागम;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  यदि (!एतत्->isSmall() && !RHS.isSmall()) {
    std::swap(एतत्->CurArray, RHS.CurArray);
    std::swap(एतत्->CurArraySize, RHS.CurArraySize);
    std::swap(एतत्->अंक_Entries, RHS.अंक_Entries);
    प्रत्यागम;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  यदि (एतत्->isSmall() && RHS.isSmall()) {
    अचिन्हित MinEntries = std::min(एतत्->अंक_Entries, RHS.अंक_Entries);
    std::swap_ranges(एतत्->CurArray, एतत्->CurArray + MinEntries, RHS.CurArray);
    यदि (एतत्->अंक_Entries > MinEntries) {
      std::copy(एतत्->CurArray + MinEntries, एतत्->CurArray + एतत्->अंक_Entries,
                RHS.CurArray + MinEntries);
    } अन्यथा {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.अंक_Entries,
                एतत्->CurArray + MinEntries);
    }
    प्रतिज्ञा(एतत्->CurArraySize == RHS.CurArraySize);
    std::swap(एतत्->अंक_Entries, RHS.अंक_Entries);
    प्रत्यागम;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = एतत्->isSmall() ? *एतत् : RHS;
  SmallPtrSetImplBase &LargeSide = एतत्->isSmall() ? RHS : *एतत्;
  ध्रुव शून्य **LargeSideInlineStorage =
      एतत्->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.अंक_Entries, SmallSide.अंक_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = असत्य;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = सत्य;
}
