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

#અન્તર્ભાવ "llvm/ADT/SmallPtrSet.h"
#અન્તર્ભાવ "llvm/ADT/DenseMapInfo.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/MemAlloc.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdlib>

using નામાકાશ llvm;

શૂન્ય SmallPtrSetImplBase::shrink_and_clear() {
  પ્રતિજ્ઞા(!isSmall() && "Can't shrink a small set!");
  મુક્ત(CurArray);

  // Reduce the number of buckets.
  unsigned Size = size();
  CurArraySize = Size > 16 ? 1 << (Log2_32_Ceil(Size) + 1) : 32;
  અંક_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (અચલ શૂન્ય**)safe_malloc(કદ(શૂન્ય*) * CurArraySize);

  memset(CurArray, -1, CurArraySize*કદ(શૂન્ય*));
}

std::pair<અચલ શૂન્ય *અચલ *, તાર્કિક>
SmallPtrSetImplBase::ઉમેરો_imp_big(અચલ શૂન્ય *Ptr) {
  જો (LLVM_UNLIKELY(size() * 3 >= CurArraySize * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(CurArraySize < 64 ? 128 : CurArraySize * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  unsigned Mask = CurArraySize - 1;
  unsigned I = DenseMapInfo<શૂન્ય *>::get_Hash_કિંમત(Ptr) & Mask;
  અચલ શૂન્ય **Array = CurArray;
  જ્યારે (Array[I] != getEmptyMarker()) {
    જો (Array[I] == Ptr)
      પાછા {Array + I, ખોટું};
    I = (I + 1) & Mask;
  }

  // Insert into the empty bucket.
  ++અંક_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  પાછા {Array + I, સાચું};
}

અચલ શૂન્ય *અચલ *SmallPtrSetImplBase::doFind(અચલ શૂન્ય *Ptr) અચલ {
  unsigned Mask = CurArraySize - 1;
  unsigned BucketNo = DenseMapInfo<શૂન્ય *>::get_Hash_કિંમત(Ptr) & Mask;
  જ્યારે (સાચું) {
    અચલ શૂન્ય *અચલ *Bucket = CurArray + BucketNo;
    જો (LLVM_LIKELY(*Bucket == Ptr))
      પાછા Bucket;
    જો (LLVM_LIKELY(*Bucket == getEmptyMarker()))
      પાછા nullptr;
    BucketNo = (BucketNo + 1) & Mask;
  }
}

શૂન્ય SmallPtrSetImplBase::eraseFromBucket(અચલ શૂન્ય **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  unsigned Mask = CurArraySize - 1;
  unsigned I = Bucket - CurArray;
  unsigned J = I;
  અચલ શૂન્ય *Empty = getEmptyMarker();
  જ્યારે ((J = (J + 1) & Mask), CurArray[J] != Empty) {
    auto Ideal = DenseMapInfo<શૂન્ય *>::get_Hash_કિંમત(CurArray[J]);
    જો (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = Empty;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
શૂન્ય SmallPtrSetImplBase::Grow(unsigned NewSize) {
  auto OldBuckets = buckets();
  તાર્કિક WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  અચલ શૂન્ય **NewBuckets = (અચલ શૂન્ય**) safe_malloc(કદ(શૂન્ય*) * NewSize);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  CurArraySize = NewSize;
  memset(CurArray, -1, NewSize*કદ(શૂન્ય*));

  // Copy over all valid entries.
  unsigned Mask = CurArraySize - 1;
  માટે (અચલ શૂન્ય *Ptr : OldBuckets) {
    જો (Ptr == getEmptyMarker())
      ચાલુ;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    unsigned I = DenseMapInfo<શૂન્ય *>::get_Hash_કિંમત(Ptr) & Mask;
    જ્યારે (NewBuckets[I] != getEmptyMarker())
      I = (I + 1) & Mask;
    NewBuckets[I] = Ptr;
  }

  જો (!WasSmall)
    મુક્ત(OldBuckets.begin());
  IsSmall = ખોટું;
}

SmallPtrSetImplBase::SmallPtrSetImplBase(અચલ શૂન્ય **SmallStorage,
                                         અચલ SmallPtrSetImplBase &that) {
  IsSmall = that.isSmall();
  જો (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } નહીંતર {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (અચલ શૂન્ય**)safe_malloc(કદ(શૂન્ય*) * that.CurArraySize);
  }

  // Copy over the that array.
  copyHelper(that);
}

SmallPtrSetImplBase::SmallPtrSetImplBase(અચલ શૂન્ય **SmallStorage,
                                         unsigned SmallSize,
                                         અચલ શૂન્ય **RHSSmallStorage,
                                         SmallPtrSetImplBase &&that) {
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(that));
}

શૂન્ય SmallPtrSetImplBase::copyFrom(અચલ શૂન્ય **SmallStorage,
                                   અચલ SmallPtrSetImplBase &RHS) {
  પ્રતિજ્ઞા(&RHS != આ && "Self-copy should be handled by the caller.");

  જો (isSmall() && RHS.isSmall())
    પ્રતિજ્ઞા(CurArraySize == RHS.CurArraySize &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  જો (RHS.isSmall()) {
    જો (!isSmall())
      મુક્ત(CurArray);
    CurArray = SmallStorage;
    IsSmall = સાચું;
    // Otherwise, allocate new heap space (unless we were the same size)
  } નહીંતર જો (CurArraySize != RHS.CurArraySize) {
    જો (isSmall())
      CurArray = (અચલ શૂન્ય**)safe_malloc(કદ(શૂન્ય*) * RHS.CurArraySize);
    નહીંતર {
      અચલ શૂન્ય **T = (અચલ શૂન્ય**)safe_realloc(CurArray,
                                             કદ(શૂન્ય*) * RHS.CurArraySize);
      CurArray = T;
    }
    IsSmall = ખોટું;
  }

  copyHelper(RHS);
}

શૂન્ય SmallPtrSetImplBase::copyHelper(અચલ SmallPtrSetImplBase &RHS) {
  // Copy over the new array size
  CurArraySize = RHS.CurArraySize;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  અંક_Entries = RHS.અંક_Entries;
}

શૂન્ય SmallPtrSetImplBase::moveFrom(અચલ શૂન્ય **SmallStorage,
                                   unsigned SmallSize,
                                   અચલ શૂન્ય **RHSSmallStorage,
                                   SmallPtrSetImplBase &&RHS) {
  જો (!isSmall())
    મુક્ત(CurArray);
  moveHelper(SmallStorage, SmallSize, RHSSmallStorage, std::move(RHS));
}

શૂન્ય SmallPtrSetImplBase::moveHelper(અચલ શૂન્ય **SmallStorage,
                                     unsigned SmallSize,
                                     અચલ શૂન્ય **RHSSmallStorage,
                                     SmallPtrSetImplBase &&RHS) {
  પ્રતિજ્ઞા(&RHS != આ && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  જો (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } નહીંતર {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  CurArraySize = RHS.CurArraySize;
  અંક_Entries = RHS.અંક_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.CurArraySize = SmallSize;
  RHS.અંક_Entries = 0;
  RHS.IsSmall = સાચું;
}

શૂન્ય SmallPtrSetImplBase::swap(અચલ શૂન્ય **SmallStorage,
                               અચલ શૂન્ય **RHSSmallStorage,
                               SmallPtrSetImplBase &RHS) {
  જો (આ == &RHS) પાછા;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  જો (!આ->isSmall() && !RHS.isSmall()) {
    std::swap(આ->CurArray, RHS.CurArray);
    std::swap(આ->CurArraySize, RHS.CurArraySize);
    std::swap(આ->અંક_Entries, RHS.અંક_Entries);
    પાછા;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  જો (આ->isSmall() && RHS.isSmall()) {
    unsigned MinEntries = std::min(આ->અંક_Entries, RHS.અંક_Entries);
    std::swap_ranges(આ->CurArray, આ->CurArray + MinEntries, RHS.CurArray);
    જો (આ->અંક_Entries > MinEntries) {
      std::copy(આ->CurArray + MinEntries, આ->CurArray + આ->અંક_Entries,
                RHS.CurArray + MinEntries);
    } નહીંતર {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.અંક_Entries,
                આ->CurArray + MinEntries);
    }
    પ્રતિજ્ઞા(આ->CurArraySize == RHS.CurArraySize);
    std::swap(આ->અંક_Entries, RHS.અંક_Entries);
    પાછા;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  SmallPtrSetImplBase &SmallSide = આ->isSmall() ? *આ : RHS;
  SmallPtrSetImplBase &LargeSide = આ->isSmall() ? RHS : *આ;
  અચલ શૂન્ય **LargeSideInlineStorage =
      આ->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.CurArraySize, SmallSide.CurArraySize);
  std::swap(LargeSide.અંક_Entries, SmallSide.અંક_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = ખોટું;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = સાચું;
}
