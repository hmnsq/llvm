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

#அந்தர்பாவ "llvm/ADT/SmallPtrSet.h"
#அந்தர்பாவ "llvm/ADT/DenseMapInfo.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/MemAlloc.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdlib>

using நாமாகாஶ llvm;

வெறுமை SmallPtrSetImplBase::shrink_and_clear() {
  ப்ரதிஜ்ஞா(!isSmall() && "Can't shrink a small set!");
  விடுவி(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  இலக்கம்_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (மாறிலி வெறுமை**)safe_malloc(அளவு(வெறுமை*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*அளவு(வெறுமை*));
}

std::pair<மாறிலி வெறுமை *மாறிலி *, தருக்க>
SmallPtrSetImplBase::செருகு_imp_big(மாறிலி வெறுமை *Ptr) {
  எனில் (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<வெறுமை *>::get_Hash_மதிப்பு(Ptr) & Mask;
  மாறிலி வெறுமை **Array = CurArray;
  போது (Array[I] != getEmptyMarker()) {
    எனில் (Array[I] == Ptr)
      திரும்பு {Array + I, பொய்};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++இலக்கம்_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  திரும்பு {Array + I, உண்மை};
}

மாறிலி வெறுமை *மாறிலி *SmallPtrSetImplBase::doFind(மாறிலி வெறுமை *Ptr) மாறிலி {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<வெறுமை *>::get_Hash_மதிப்பு(Ptr) & Mask;
  போது (உண்மை) {
    மாறிலி வெறுமை *மாறிலி *Bucket = CurArray + BucketNo;
    எனில் (LLVM_LIKELY(*Bucket == Ptr))
      திரும்பு Bucket;
    எனில் (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      திரும்பு nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

வெறுமை SmallPtrSetImplBase::eraseFromBucket(மாறிலி வெறுமை **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  மாறிலி வெறுமை *Empty = getEmptyMarker();
  போது ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<வெறுமை *>::get_Hash_மதிப்பு(CurArray[J]);
    எனில் (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
வெறுமை SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  தருக்க WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  மாறிலி வெறுமை **NewBuckets = (மாறிலி வெறுமை**) safe_malloc(அளவு(வெறுமை*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*அளவு(வெறுமை*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  க்கு (மாறிலி வெறுமை *Ptr : OldBuckets) {
    எனில் (Ptr == getEmptyMarker())
      தொடர்;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<வெறுமை *>::get_Hash_மதிப்பு(Ptr) & Mask;
    போது (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  எனில் (!WasSmall)
    விடுவி(OldBuckets.begin());
  IsSmall = பொய்;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(மாறிலி வெறுமை **SmallStorage,
                                         மாறிலி SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  எனில் (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } இல்லையெனில் {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (மாறிலி வெறுமை**)safe_malloc(அளவு(வெறுமை*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(மாறிலி வெறுமை **SmallStorage,
                                         unsigned SmallSize,
                                         மாறிலி வெறுமை **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

வெறுமை SmallPtrSetImplBase::copyFrom(மாறிலி வெறுமை **SmallStorage,
                                   மாறிலி SmallPtrSetImplBase &RHS) {
  ப்ரதிஜ்ஞா(&RHS != இது && "Self-copy should be handled by the caller.");

  எனில் (isSmall() && RHS.isSmall())
    ப்ரதிஜ்ஞா(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  எனில் (RHS.isSmall()) {
    எனில் (!isSmall())
      விடுவி(CurArray);
    CurArray = SmallStorage;
    IsSmall = உண்மை;
    // Otherwise, allocate new heap space (unless we were the same size)
  } இல்லையெனில் எனில் (CurArraySize != RHS.CurArraySize) {
    எனில் (isSmall())
      CurArray = (மாறிலி வெறுமை**)safe_malloc(அளவு(வெறுமை*) * RHS.CurArraySize);
    இல்லையெனில் {
      மாறிலி வெறுமை **T = (மாறிலி வெறுமை**)safe_realloc(CurArray,
                                             அளவு(வெறுமை*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = பொய்;
  }

  copyHelper(RHS);
}

வெறுமை SmallPtrSetImplBase::copyHelper(மாறிலி SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  இலக்கம்_Entries = RHS.இலக்கம்_Entries;
}

வெறுமை SmallPtrSetImplBase::moveFrom(மாறிலி வெறுமை **SmallStorage,
                                   unsigned SmallSize,
                                   மாறிலி வெறுமை **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  எனில் (!isSmall())
    விடுவி(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

வெறுமை SmallPtrSetImplBase::moveHelper(மாறிலி வெறுமை **SmallStorage,
                                     unsigned SmallSize,
                                     மாறிலி வெறுமை **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  ப்ரதிஜ்ஞா(&RHS != இது && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  எனில் (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } இல்லையெனில் {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  இலக்கம்_Entries = RHS.இலக்கம்_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.இலக்கம்_Entries = 0;
  RHS.IsSmall = உண்மை;
}

வெறுமை SmallPtrSetImplBase::swap(மாறிலி வெறுமை **SmallStorage,
                               மாறிலி வெறுமை **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  எனில் (இது == &RHS) திரும்பு;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  எனில் (!இது->isSmall() && !RHS.isSmall()) {
    std::swap(இது->CurArray, RHS.CurArray);
    std::swap(இது->CurArraySize, RHS.CurArraySize);
    std::swap(இது->இலக்கம்_Entries, RHS.இலக்கம்_Entries);
    திரும்பு;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  எனில் (இது->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(இது->இலக்கம்_Entries, RHS.இலக்கம்_Entries);
    std::swap_ranges(இது->CurArray, இது->CurArray + MinEntries, RHS.CurArray);
    எனில் (இது->இலக்கம்_Entries > MinEntries) {
      std::copy(இது->CurArray + MinEntries, இது->CurArray + இது->இலக்கம்_Entries,
                RHS.CurArray + MinEntries);
    } இல்லையெனில் {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.இலக்கம்_Entries,
                இது->CurArray + MinEntries);
    }
    ப்ரதிஜ்ஞா(இது->CurArraySize == RHS.CurArraySize);
    std::swap(இது->இலக்கம்_Entries, RHS.இலக்கம்_Entries);
    திரும்பு;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = இது->isSmall() ? *இது : RHS;
  SmallPtrSetImplBase &LargeSide = இது->isSmall() ? RHS : *இது;
  மாறிலி வெறுமை **LargeSideInlineStorage =
      இது->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.இலக்கம்_Entries, SmallSide.இலக்கம்_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = பொய்;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = உண்மை;
}
