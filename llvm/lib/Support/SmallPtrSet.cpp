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
  CurArray = (स्थिरांक शून्य**)safe_malloc(आकार(शून्य*) * CurArraySize);

  स_रखो(CurArray, -1, CurArraySize*आकार(शून्य*));
}

std::pair<स्थिरांक शून्य *स्थिरांक *, तार्किक>
SmallPtrSetImplBase::घाला_imp_big(स्थिरांक शून्य *Ptr) {
  जर (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित I = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
  स्थिरांक शून्य **Array = CurArray;
  पर्यंत (Array[I] != getEmptyMarker()) {
    जर (Array[I] == Ptr)
      परत {Array + I, असत्य};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++अंक_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  परत {Array + I, सत्य};
}

स्थिरांक शून्य *स्थिरांक *SmallPtrSetImplBase::doFind(स्थिरांक शून्य *Ptr) स्थिरांक {
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित BucketNo = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
  पर्यंत (सत्य) {
    स्थिरांक शून्य *स्थिरांक *Bucket = CurArray + BucketNo;
    जर (LLVM_LIKELY(*Bucket == Ptr))
      परत Bucket;
    जर (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      परत nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

शून्य SmallPtrSetImplBase::eraseFromBucket(स्थिरांक शून्य **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  अचिन्हित Mask = CurArraySize - 1;
  अचिन्हित I = Bucket - CurArray;
  अचिन्हित J = I;
  स्थिरांक शून्य *Empty = getEmptyMarker();
  पर्यंत ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    स्वतः Ideal = DenseMapInfo<शून्य *>::get_Hash_मूल्य(CurArray[J]);
    जर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
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
  स्थिरांक शून्य **NewBuckets = (स्थिरांक शून्य**) safe_malloc(आकार(शून्य*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  स_रखो(CurArray, -1, NewSize*आकार(शून्य*));

  // Copy over all valid entries.
  अचिन्हित Mask = CurArraySize - 1;
  साठी (स्थिरांक शून्य *Ptr : OldBuckets) {
    जर (Ptr == getEmptyMarker())
      सुरूठेवा;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    अचिन्हित I = DenseMapInfo<शून्य *>::get_Hash_मूल्य(Ptr) & Mask;
    पर्यंत (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  जर (!WasSmall)
    मुक्त(OldBuckets.begin());
  IsSmall = असत्य;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(स्थिरांक शून्य **SmallStorage,
                                         स्थिरांक SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  जर (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } अन्यथा {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (स्थिरांक शून्य**)safe_malloc(आकार(शून्य*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(स्थिरांक शून्य **SmallStorage,
                                         अचिन्हित SmallSize,
                                         स्थिरांक शून्य **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

शून्य SmallPtrSetImplBase::copyFrom(स्थिरांक शून्य **SmallStorage,
                                   स्थिरांक SmallPtrSetImplBase &RHS) {
  प्रतिज्ञा(&RHS != हे && "Self-copy should be handled by the caller.");

  जर (isSmall() && RHS.isSmall())
    प्रतिज्ञा(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  जर (RHS.isSmall()) {
    जर (!isSmall())
      मुक्त(CurArray);
    CurArray = SmallStorage;
    IsSmall = सत्य;
    // Otherwise, allocate new heap space (unless we were the same size)
  } अन्यथा जर (CurArraySize != RHS.CurArraySize) {
    जर (isSmall())
      CurArray = (स्थिरांक शून्य**)safe_malloc(आकार(शून्य*) * RHS.CurArraySize);
    अन्यथा {
      स्थिरांक शून्य **T = (स्थिरांक शून्य**)safe_realloc(CurArray,
                                             आकार(शून्य*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = असत्य;
  }

  copyHelper(RHS);
}

शून्य SmallPtrSetImplBase::copyHelper(स्थिरांक SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  अंक_Entries = RHS.अंक_Entries;
}

शून्य SmallPtrSetImplBase::moveFrom(स्थिरांक शून्य **SmallStorage,
                                   अचिन्हित SmallSize,
                                   स्थिरांक शून्य **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  जर (!isSmall())
    मुक्त(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

शून्य SmallPtrSetImplBase::moveHelper(स्थिरांक शून्य **SmallStorage,
                                     अचिन्हित SmallSize,
                                     स्थिरांक शून्य **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  प्रतिज्ञा(&RHS != हे && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  जर (RHS.isSmall()) {
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

शून्य SmallPtrSetImplBase::swap(स्थिरांक शून्य **SmallStorage,
                               स्थिरांक शून्य **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  जर (हे == &RHS) परत;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  जर (!हे->isSmall() && !RHS.isSmall()) {
    std::swap(हे->CurArray, RHS.CurArray);
    std::swap(हे->CurArraySize, RHS.CurArraySize);
    std::swap(हे->अंक_Entries, RHS.अंक_Entries);
    परत;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  जर (हे->isSmall() && RHS.isSmall()) {
    अचिन्हित MinEntries = std::min(हे->अंक_Entries, RHS.अंक_Entries);
    std::swap_ranges(हे->CurArray, हे->CurArray + MinEntries, RHS.CurArray);
    जर (हे->अंक_Entries > MinEntries) {
      std::copy(हे->CurArray + MinEntries, हे->CurArray + हे->अंक_Entries,
                RHS.CurArray + MinEntries);
    } अन्यथा {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.अंक_Entries,
                हे->CurArray + MinEntries);
    }
    प्रतिज्ञा(हे->CurArraySize == RHS.CurArraySize);
    std::swap(हे->अंक_Entries, RHS.अंक_Entries);
    परत;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = हे->isSmall() ? *हे : RHS;
  SmallPtrSetImplBase &LargeSide = हे->isSmall() ? RHS : *हे;
  स्थिरांक शून्य **LargeSideInlineStorage =
      हे->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.अंक_Entries, SmallSide.अंक_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = असत्य;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = सत्य;
}
