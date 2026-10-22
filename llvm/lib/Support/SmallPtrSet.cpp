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

#समावेश "llvm/ADT/SmallPtrSet.h"
#समावेश "llvm/ADT/DenseMapInfo.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/MemAlloc.h"
#समावेश <algorithm>
#समावेश <cassert>
#समावेश <cstdlib>

using namespace llvm;

व्योम SmallPtrSetImplBase::shrink_and_clear() {
  निश्चित(!isSmall() && "Can't shrink a small set!");
  मुक्त(CurArray);

  // Reduce the number of buckets.
  अचिन्हित आकार = आकार_2();
  Cur_Array_आकार = आकार > 16 ? 1 << (Log2_32_Ceil(आकार) + 1) : 32;
  अंक_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (स्थिर व्योम**)safe_malloc(माप(व्योम*) * Cur_Array_आकार);

  स_रखो(CurArray, -1, Cur_Array_आकार*माप(व्योम*));
}

std::pair<स्थिर व्योम *स्थिर *, bool>
SmallPtrSetImplBase::जोड़ो_imp_big(स्थिर व्योम *Ptr) {
  अगर (LLVM_UNLIKELY(आकार_2() * 3 >= Cur_Array_आकार * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_आकार < 64 ? 128 : Cur_Array_आकार * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  अचिन्हित Mask = Cur_Array_आकार - 1;
  अचिन्हित I = DenseMapInfo<व्योम *>::get_Hash_मूल्य(Ptr) & Mask;
  स्थिर व्योम **Array = CurArray;
  जबतक (Array[I] != getEmptyMarker()) {
    अगर (Array[I] == Ptr)
      वापस {Array + I, false};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++अंक_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  वापस {Array + I, true};
}

स्थिर व्योम *स्थिर *SmallPtrSetImplBase::doFind(स्थिर व्योम *Ptr) स्थिर {
  अचिन्हित Mask = Cur_Array_आकार - 1;
  अचिन्हित BucketNo = DenseMapInfo<व्योम *>::get_Hash_मूल्य(Ptr) & Mask;
  जबतक (true) {
    स्थिर व्योम *स्थिर *Bucket = CurArray + BucketNo;
    अगर (LLVM_LIKELY(*Bucket == Ptr))
      वापस Bucket;
    अगर (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      वापस nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

व्योम SmallPtrSetImplBase::eraseFromBucket(स्थिर व्योम **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  अचिन्हित Mask = Cur_Array_आकार - 1;
  अचिन्हित I = Bucket - CurArray;
  अचिन्हित J = I;
  स्थिर व्योम *Empty = getEmptyMarker();
  जबतक ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    स्वतः Ideal = DenseMapInfo<व्योम *>::get_Hash_मूल्य(CurArray[J]);
    अगर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
व्योम SmallPtrSetImplBase::Grow(अचिन्हित New_आकार) {
  स्वतः OldBuckets = buckets();
  bool WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  स्थिर व्योम **NewBuckets = (स्थिर व्योम**) safe_malloc(माप(व्योम*) * New_आकार);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_आकार = New_आकार;
  स_रखो(CurArray, -1, New_आकार*माप(व्योम*));

  // Copy over all valid entries.
  अचिन्हित Mask = Cur_Array_आकार - 1;
  क्रम (स्थिर व्योम *Ptr : OldBuckets) {
    अगर (Ptr == getEmptyMarker())
      जारी;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    अचिन्हित I = DenseMapInfo<व्योम *>::get_Hash_मूल्य(Ptr) & Mask;
    जबतक (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  अगर (!WasSmall)
    मुक्त(OldBuckets.begin());
  IsSmall = false;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(स्थिर व्योम **SmallStorage,
                                         स्थिर SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  अगर (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } अन्यथा {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (स्थिर व्योम**)safe_malloc(माप(व्योम*) * that.Cur_Array_आकार);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(स्थिर व्योम **SmallStorage,
                                         अचिन्हित Small_आकार,
                                         स्थिर व्योम **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, Small_आकार, RHSSmallStorage, std::move(that));
}

व्योम SmallPtrSetImplBase::copyFrom(स्थिर व्योम **SmallStorage,
                                   स्थिर SmallPtrSetImplBase &RHS) {
  निश्चित(&RHS != यह && "Self-copy should be handled by the caller.");

  अगर (isSmall() && RHS.isSmall())
    निश्चित(Cur_Array_आकार == RHS.Cur_Array_आकार &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  अगर (RHS.isSmall()) {
    अगर (!isSmall())
      मुक्त(CurArray);
    CurArray = SmallStorage;
    IsSmall = true;
    // Otherwise, allocate new heap space (unless we were the same size)
  } अन्यथा अगर (Cur_Array_आकार != RHS.Cur_Array_आकार) {
    अगर (isSmall())
      CurArray = (स्थिर व्योम**)safe_malloc(माप(व्योम*) * RHS.Cur_Array_आकार);
    अन्यथा {
      स्थिर व्योम **T = (स्थिर व्योम**)safe_realloc(CurArray,
                                             माप(व्योम*) * RHS.Cur_Array_आकार);
      CurArray = T;
    }
    IsSmall = false;
  }

  copyHelper(RHS);
}

व्योम SmallPtrSetImplBase::copyHelper(स्थिर SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  Cur_Array_आकार = RHS.Cur_Array_आकार;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  अंक_Entries = RHS.अंक_Entries;
}

व्योम SmallPtrSetImplBase::moveFrom(स्थिर व्योम **SmallStorage,
                                   अचिन्हित Small_आकार,
                                   स्थिर व्योम **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  अगर (!isSmall())
    मुक्त(CurArray);
  moveHelper(SmallStorage, Small_आकार, RHSSmallStorage, std::move(RHS));
}

व्योम SmallPtrSetImplBase::moveHelper(स्थिर व्योम **SmallStorage,
                                     अचिन्हित Small_आकार,
                                     स्थिर व्योम **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  निश्चित(&RHS != यह && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  अगर (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } अन्यथा {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_आकार = RHS.Cur_Array_आकार;
  अंक_Entries = RHS.अंक_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_आकार = Small_आकार;
  RHS.अंक_Entries = 0;
  RHS.IsSmall = true;
}

व्योम SmallPtrSetImplBase::swap(स्थिर व्योम **SmallStorage,
                               स्थिर व्योम **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  अगर (यह == &RHS) वापस;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  अगर (!यह->isSmall() && !RHS.isSmall()) {
    std::swap(यह->CurArray, RHS.CurArray);
    std::swap(यह->Cur_Array_आकार, RHS.Cur_Array_आकार);
    std::swap(यह->अंक_Entries, RHS.अंक_Entries);
    वापस;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  अगर (यह->isSmall() && RHS.isSmall()) {
    अचिन्हित MinEntries = std::min(यह->अंक_Entries, RHS.अंक_Entries);
    std::swap_ranges(यह->CurArray, यह->CurArray + MinEntries, RHS.CurArray);
    अगर (यह->अंक_Entries > MinEntries) {
      std::copy(यह->CurArray + MinEntries, यह->CurArray + यह->अंक_Entries,
                RHS.CurArray + MinEntries);
    } अन्यथा {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.अंक_Entries,
                यह->CurArray + MinEntries);
    }
    निश्चित(यह->Cur_Array_आकार == RHS.Cur_Array_आकार);
    std::swap(यह->अंक_Entries, RHS.अंक_Entries);
    वापस;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = यह->isSmall() ? *यह : RHS;
  SmallPtrSetImplBase &LargeSide = यह->isSmall() ? RHS : *यह;
  स्थिर व्योम **LargeSideInlineStorage =
      यह->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.Cur_Array_आकार, SmallSide.Cur_Array_आकार);
  std::swap(LargeSide.अंक_Entries, SmallSide.अंक_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = false;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = true;
}
