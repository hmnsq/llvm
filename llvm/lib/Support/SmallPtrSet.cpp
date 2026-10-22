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

#അന്തര്ഭാവ "llvm/ADT/SmallPtrSet.h"
#അന്തര്ഭാവ "llvm/ADT/DenseMapInfo.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/MemAlloc.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstdlib>

using നാമാകാശ llvm;

ശൂന്യം SmallPtrSetImplBase::shrink_and_clear() {
  പ്രതിജ്ഞാ(!isSmall() && "Can't shrink a small set!");
  സ്വതന്ത്രം(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  അക്കം_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (സ്ഥിരം ശൂന്യം**)safe_malloc(വലുപ്പം(ശൂന്യം*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*വലുപ്പം(ശൂന്യം*));
}

std::pair<സ്ഥിരം ശൂന്യം *സ്ഥിരം *, തർക്കം>
SmallPtrSetImplBase::തിരുകുക_imp_big(സ്ഥിരം ശൂന്യം *Ptr) {
  എങ്കിൽ (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<ശൂന്യം *>::get_Hash_മൂല്യം(Ptr) & Mask;
  സ്ഥിരം ശൂന്യം **Array = CurArray;
  ആകുമ്പോൾ (Array[I] != getEmptyMarker()) {
    എങ്കിൽ (Array[I] == Ptr)
      തിരികെ {Array + I, അസത്യം};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++അക്കം_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  തിരികെ {Array + I, സത്യം};
}

സ്ഥിരം ശൂന്യം *സ്ഥിരം *SmallPtrSetImplBase::doFind(സ്ഥിരം ശൂന്യം *Ptr) സ്ഥിരം {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<ശൂന്യം *>::get_Hash_മൂല്യം(Ptr) & Mask;
  ആകുമ്പോൾ (സത്യം) {
    സ്ഥിരം ശൂന്യം *സ്ഥിരം *Bucket = CurArray + BucketNo;
    എങ്കിൽ (LLVM_LIKELY(*Bucket == Ptr))
      തിരികെ Bucket;
    എങ്കിൽ (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      തിരികെ nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

ശൂന്യം SmallPtrSetImplBase::eraseFromBucket(സ്ഥിരം ശൂന്യം **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  സ്ഥിരം ശൂന്യം *Empty = getEmptyMarker();
  ആകുമ്പോൾ ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<ശൂന്യം *>::get_Hash_മൂല്യം(CurArray[J]);
    എങ്കിൽ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
ശൂന്യം SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  തർക്കം WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  സ്ഥിരം ശൂന്യം **NewBuckets = (സ്ഥിരം ശൂന്യം**) safe_malloc(വലുപ്പം(ശൂന്യം*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*വലുപ്പം(ശൂന്യം*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  വേണ്ടി (സ്ഥിരം ശൂന്യം *Ptr : OldBuckets) {
    എങ്കിൽ (Ptr == getEmptyMarker())
      തുടരുക;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<ശൂന്യം *>::get_Hash_മൂല്യം(Ptr) & Mask;
    ആകുമ്പോൾ (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  എങ്കിൽ (!WasSmall)
    സ്വതന്ത്രം(OldBuckets.begin());
  IsSmall = അസത്യം;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(സ്ഥിരം ശൂന്യം **SmallStorage,
                                         സ്ഥിരം SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  എങ്കിൽ (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } അല്ലെങ്കിൽ {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (സ്ഥിരം ശൂന്യം**)safe_malloc(വലുപ്പം(ശൂന്യം*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(സ്ഥിരം ശൂന്യം **SmallStorage,
                                         unsigned SmallSize,
                                         സ്ഥിരം ശൂന്യം **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

ശൂന്യം SmallPtrSetImplBase::copyFrom(സ്ഥിരം ശൂന്യം **SmallStorage,
                                   സ്ഥിരം SmallPtrSetImplBase &RHS) {
  പ്രതിജ്ഞാ(&RHS != ഇത് && "Self-copy should be handled by the caller.");

  എങ്കിൽ (isSmall() && RHS.isSmall())
    പ്രതിജ്ഞാ(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  എങ്കിൽ (RHS.isSmall()) {
    എങ്കിൽ (!isSmall())
      സ്വതന്ത്രം(CurArray);
    CurArray = SmallStorage;
    IsSmall = സത്യം;
    // Otherwise, allocate new heap space (unless we were the same size)
  } അല്ലെങ്കിൽ എങ്കിൽ (CurArraySize != RHS.CurArraySize) {
    എങ്കിൽ (isSmall())
      CurArray = (സ്ഥിരം ശൂന്യം**)safe_malloc(വലുപ്പം(ശൂന്യം*) * RHS.CurArraySize);
    അല്ലെങ്കിൽ {
      സ്ഥിരം ശൂന്യം **T = (സ്ഥിരം ശൂന്യം**)safe_realloc(CurArray,
                                             വലുപ്പം(ശൂന്യം*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = അസത്യം;
  }

  copyHelper(RHS);
}

ശൂന്യം SmallPtrSetImplBase::copyHelper(സ്ഥിരം SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  അക്കം_Entries = RHS.അക്കം_Entries;
}

ശൂന്യം SmallPtrSetImplBase::moveFrom(സ്ഥിരം ശൂന്യം **SmallStorage,
                                   unsigned SmallSize,
                                   സ്ഥിരം ശൂന്യം **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  എങ്കിൽ (!isSmall())
    സ്വതന്ത്രം(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

ശൂന്യം SmallPtrSetImplBase::moveHelper(സ്ഥിരം ശൂന്യം **SmallStorage,
                                     unsigned SmallSize,
                                     സ്ഥിരം ശൂന്യം **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  പ്രതിജ്ഞാ(&RHS != ഇത് && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  എങ്കിൽ (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } അല്ലെങ്കിൽ {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  അക്കം_Entries = RHS.അക്കം_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.അക്കം_Entries = 0;
  RHS.IsSmall = സത്യം;
}

ശൂന്യം SmallPtrSetImplBase::swap(സ്ഥിരം ശൂന്യം **SmallStorage,
                               സ്ഥിരം ശൂന്യം **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  എങ്കിൽ (ഇത് == &RHS) തിരികെ;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  എങ്കിൽ (!ഇത്->isSmall() && !RHS.isSmall()) {
    std::swap(ഇത്->CurArray, RHS.CurArray);
    std::swap(ഇത്->CurArraySize, RHS.CurArraySize);
    std::swap(ഇത്->അക്കം_Entries, RHS.അക്കം_Entries);
    തിരികെ;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  എങ്കിൽ (ഇത്->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(ഇത്->അക്കം_Entries, RHS.അക്കം_Entries);
    std::swap_ranges(ഇത്->CurArray, ഇത്->CurArray + MinEntries, RHS.CurArray);
    എങ്കിൽ (ഇത്->അക്കം_Entries > MinEntries) {
      std::copy(ഇത്->CurArray + MinEntries, ഇത്->CurArray + ഇത്->അക്കം_Entries,
                RHS.CurArray + MinEntries);
    } അല്ലെങ്കിൽ {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.അക്കം_Entries,
                ഇത്->CurArray + MinEntries);
    }
    പ്രതിജ്ഞാ(ഇത്->CurArraySize == RHS.CurArraySize);
    std::swap(ഇത്->അക്കം_Entries, RHS.അക്കം_Entries);
    തിരികെ;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = ഇത്->isSmall() ? *ഇത് : RHS;
  SmallPtrSetImplBase &LargeSide = ഇത്->isSmall() ? RHS : *ഇത്;
  സ്ഥിരം ശൂന്യം **LargeSideInlineStorage =
      ഇത്->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.അക്കം_Entries, SmallSide.അക്കം_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = അസത്യം;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = സത്യം;
}
