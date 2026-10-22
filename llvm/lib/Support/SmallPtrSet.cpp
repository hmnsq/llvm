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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallPtrSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/DenseMapInfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemAlloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::shrink_and_clear() {
  ਪ੍ਰਤਿਜ੍ਞਾ(!isSmall() && "Can't shrink a small set!");
  ਮੁਕਤ(CurArray);

  // Reduce the number of buckets.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = ਮਾਪ_2();
  Cur_Array_ਮਾਪ = ਮਾਪ > 16 ? 1 << (Log2_32_Ceil(ਮਾਪ) + 1) : 32;
  ਅੰਕ_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ਅਚਲ ਖਾਲੀ**)ਸੁਰੱਖਿਅਤ_malloc(ਆਕਾਰ(ਖਾਲੀ*) * Cur_Array_ਮਾਪ);

  memset(CurArray, -1, Cur_Array_ਮਾਪ*ਆਕਾਰ(ਖਾਲੀ*));
}

std::pair<ਅਚਲ ਖਾਲੀ *ਅਚਲ *, ਤਾਰਕਿਕ>
Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::ਪਾਓ_imp_big(ਅਚਲ ਖਾਲੀ *Ptr) {
  ਜੇ (LLVM_UNLIKELY(ਮਾਪ_2() * 3 >= Cur_Array_ਮਾਪ * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_ਮਾਪ < 64 ? 128 : Cur_Array_ਮਾਪ * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = Cur_Array_ਮਾਪ - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ I = DenseMapInfo<ਖਾਲੀ *>::get_Hash_ਮੁੱਲ(Ptr) & ਨਕਾਬ;
  ਅਚਲ ਖਾਲੀ **Array = CurArray;
  ਜਦੋਂ (Array[I] != get_ਖਾਲੀ_ਥਾਵਾਂ_Marker()) {
    ਜੇ (Array[I] == Ptr)
      ਵਾਪਸ {Array + I, ਝੂਠ};
    I = (I + 1) & ਨਕਾਬ;
  }

  // Insert into the empty bucket.
  ++ਅੰਕ_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  ਵਾਪਸ {Array + I, ਸੱਚ};
}

ਅਚਲ ਖਾਲੀ *ਅਚਲ *Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::do_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *Ptr) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = Cur_Array_ਮਾਪ - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ_No = DenseMapInfo<ਖਾਲੀ *>::get_Hash_ਮੁੱਲ(Ptr) & ਨਕਾਬ;
  ਜਦੋਂ (ਸੱਚ) {
    ਅਚਲ ਖਾਲੀ *ਅਚਲ *ਬਾਲਟੀ = CurArray + ਬਾਲਟੀ_No;
    ਜੇ (LLVM_LIKELY(*ਬਾਲਟੀ == Ptr))
      ਵਾਪਸ ਬਾਲਟੀ;
    ਜੇ (LLVM_LIKELY(*ਬਾਲਟੀ == get_ਖਾਲੀ_ਥਾਵਾਂ_Marker()))
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    ਬਾਲਟੀ_No = (ਬਾਲਟੀ_No + 1) & ਨਕਾਬ;
  }
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::erase_From_ਬਾਲਟੀ(ਅਚਲ ਖਾਲੀ **ਬਾਲਟੀ) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = Cur_Array_ਮਾਪ - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ I = ਬਾਲਟੀ - CurArray;
  ਬਿਨਾਨਿਸ਼ਾਨੀ J = I;
  ਅਚਲ ਖਾਲੀ *ਖਾਲੀ_ਥਾਵਾਂ_2 = get_ਖਾਲੀ_ਥਾਵਾਂ_Marker();
  ਜਦੋਂ ((J = (J + 1) & ਨਕਾਬ), CurArray[J] != ਖਾਲੀ_ਥਾਵਾਂ_2) {
    ਸਵੈਚਾਲਿਤ Ideal = DenseMapInfo<ਖਾਲੀ *>::get_Hash_ਮੁੱਲ(CurArray[J]);
    ਜੇ (((I - Ideal) & ਨਕਾਬ) < ((J - Ideal) & ਨਕਾਬ)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = ਖਾਲੀ_ਥਾਵਾਂ_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::Grow(ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਮਾਪ) {
  ਸਵੈਚਾਲਿਤ OldBuckets = buckets();
  ਤਾਰਕਿਕ WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ਅਚਲ ਖਾਲੀ **NewBuckets = (ਅਚਲ ਖਾਲੀ**) ਸੁਰੱਖਿਅਤ_malloc(ਆਕਾਰ(ਖਾਲੀ*) * New_ਮਾਪ);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_ਮਾਪ = New_ਮਾਪ;
  memset(CurArray, -1, New_ਮਾਪ*ਆਕਾਰ(ਖਾਲੀ*));

  // Copy over all valid entries.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = Cur_Array_ਮਾਪ - 1;
  ਲਈ (ਅਚਲ ਖਾਲੀ *Ptr : OldBuckets) {
    ਜੇ (Ptr == get_ਖਾਲੀ_ਥਾਵਾਂ_Marker())
      ਜਾਰੀ;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    ਬਿਨਾਨਿਸ਼ਾਨੀ I = DenseMapInfo<ਖਾਲੀ *>::get_Hash_ਮੁੱਲ(Ptr) & ਨਕਾਬ;
    ਜਦੋਂ (NewBuckets[I] != get_ਖਾਲੀ_ਥਾਵਾਂ_Marker())
      I = (I + 1) & ਨਕਾਬ;
    NewBuckets[I] = Ptr;
  }

  ਜੇ (!WasSmall)
    ਮੁਕਤ(OldBuckets.begin());
  IsSmall = ਝੂਠ;
}

Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ(ਅਚਲ ਖਾਲੀ **SmallStorage,
                                         ਅਚਲ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &that) {
  IsSmall = that.isSmall();
  ਜੇ (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } ਨਹੀਂਤਾਂ {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ਅਚਲ ਖਾਲੀ**)ਸੁਰੱਖਿਅਤ_malloc(ਆਕਾਰ(ਖਾਲੀ*) * that.Cur_Array_ਮਾਪ);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ(ਅਚਲ ਖਾਲੀ **SmallStorage,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ Small_ਮਾਪ,
                                         ਅਚਲ ਖਾਲੀ **RHSSmallStorage,
                                         Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &&that) {
  moveHelper(SmallStorage, Small_ਮਾਪ, RHSSmallStorage, std::move(that));
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::copyFrom(ਅਚਲ ਖਾਲੀ **SmallStorage,
                                   ਅਚਲ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(&RHS != ਇਹ && "Self-copy should be handled by the caller.");

  ਜੇ (isSmall() && RHS.isSmall())
    ਪ੍ਰਤਿਜ੍ਞਾ(Cur_Array_ਮਾਪ == RHS.Cur_Array_ਮਾਪ &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  ਜੇ (RHS.isSmall()) {
    ਜੇ (!isSmall())
      ਮੁਕਤ(CurArray);
    CurArray = SmallStorage;
    IsSmall = ਸੱਚ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } ਨਹੀਂਤਾਂ ਜੇ (Cur_Array_ਮਾਪ != RHS.Cur_Array_ਮਾਪ) {
    ਜੇ (isSmall())
      CurArray = (ਅਚਲ ਖਾਲੀ**)ਸੁਰੱਖਿਅਤ_malloc(ਆਕਾਰ(ਖਾਲੀ*) * RHS.Cur_Array_ਮਾਪ);
    ਨਹੀਂਤਾਂ {
      ਅਚਲ ਖਾਲੀ **T = (ਅਚਲ ਖਾਲੀ**)ਸੁਰੱਖਿਅਤ_realloc(CurArray,
                                             ਆਕਾਰ(ਖਾਲੀ*) * RHS.Cur_Array_ਮਾਪ);
      CurArray = T;
    }
    IsSmall = ਝੂਠ;
  }

  copyHelper(RHS);
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::copyHelper(ਅਚਲ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &RHS) {
  // Copy over the new array size
  Cur_Array_ਮਾਪ = RHS.Cur_Array_ਮਾਪ;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  ਅੰਕ_Entries = RHS.ਅੰਕ_Entries;
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::moveFrom(ਅਚਲ ਖਾਲੀ **SmallStorage,
                                   ਬਿਨਾਨਿਸ਼ਾਨੀ Small_ਮਾਪ,
                                   ਅਚਲ ਖਾਲੀ **RHSSmallStorage,
                                   Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &&RHS) {
  ਜੇ (!isSmall())
    ਮੁਕਤ(CurArray);
  moveHelper(SmallStorage, Small_ਮਾਪ, RHSSmallStorage, std::move(RHS));
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::moveHelper(ਅਚਲ ਖਾਲੀ **SmallStorage,
                                     ਬਿਨਾਨਿਸ਼ਾਨੀ Small_ਮਾਪ,
                                     ਅਚਲ ਖਾਲੀ **RHSSmallStorage,
                                     Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &&RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(&RHS != ਇਹ && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  ਜੇ (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } ਨਹੀਂਤਾਂ {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_ਮਾਪ = RHS.Cur_Array_ਮਾਪ;
  ਅੰਕ_Entries = RHS.ਅੰਕ_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_ਮਾਪ = Small_ਮਾਪ;
  RHS.ਅੰਕ_Entries = 0;
  RHS.IsSmall = ਸੱਚ;
}

ਖਾਲੀ Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ::ਅਦਲਾ_ਬਦਲੀ(ਅਚਲ ਖਾਲੀ **SmallStorage,
                               ਅਚਲ ਖਾਲੀ **RHSSmallStorage,
                               Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &RHS) {
  ਜੇ (ਇਹ == &RHS) ਵਾਪਸ;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  ਜੇ (!ਇਹ->isSmall() && !RHS.isSmall()) {
    std::ਅਦਲਾ_ਬਦਲੀ(ਇਹ->CurArray, RHS.CurArray);
    std::ਅਦਲਾ_ਬਦਲੀ(ਇਹ->Cur_Array_ਮਾਪ, RHS.Cur_Array_ਮਾਪ);
    std::ਅਦਲਾ_ਬਦਲੀ(ਇਹ->ਅੰਕ_Entries, RHS.ਅੰਕ_Entries);
    ਵਾਪਸ;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  ਜੇ (ਇਹ->isSmall() && RHS.isSmall()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ MinEntries = std::min(ਇਹ->ਅੰਕ_Entries, RHS.ਅੰਕ_Entries);
    std::ਅਦਲਾ_ਬਦਲੀ_ranges(ਇਹ->CurArray, ਇਹ->CurArray + MinEntries, RHS.CurArray);
    ਜੇ (ਇਹ->ਅੰਕ_Entries > MinEntries) {
      std::copy(ਇਹ->CurArray + MinEntries, ਇਹ->CurArray + ਇਹ->ਅੰਕ_Entries,
                RHS.CurArray + MinEntries);
    } ਨਹੀਂਤਾਂ {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.ਅੰਕ_Entries,
                ਇਹ->CurArray + MinEntries);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->Cur_Array_ਮਾਪ == RHS.Cur_Array_ਮਾਪ);
    std::ਅਦਲਾ_ਬਦਲੀ(ਇਹ->ਅੰਕ_Entries, RHS.ਅੰਕ_Entries);
    ਵਾਪਸ;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &SmallSide = ਇਹ->isSmall() ? *ਇਹ : RHS;
  Small_Ptr_ਸੈੱਟ_Impl_ਅਧਾਰ &LargeSide = ਇਹ->isSmall() ? RHS : *ਇਹ;
  ਅਚਲ ਖਾਲੀ **LargeSideInlineStorage =
      ਇਹ->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::ਅਦਲਾ_ਬਦਲੀ(LargeSide.Cur_Array_ਮਾਪ, SmallSide.Cur_Array_ਮਾਪ);
  std::ਅਦਲਾ_ਬਦਲੀ(LargeSide.ਅੰਕ_Entries, SmallSide.ਅੰਕ_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = ਝੂਠ;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = ਸੱਚ;
}
