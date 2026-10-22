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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallPtrSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/DenseMapInfo.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdlib>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::shrink_and_clear() {
  ಪ್ರತಿಜ್ಞಾ(!isSmall() && "Can't shrink a small set!");
  ಬಿಡುಗಡೆ(CurArray);

  // Reduce the number of buckets.
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = ಗಾತ್ರ_3();
  Cur_Array_ಗಾತ್ರ = ಗಾತ್ರ_2 > 16 ? 1 << (Log2_32_Ceil(ಗಾತ್ರ_2) + 1) : 32;
  ಅಂಕೆ_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**)ಸುರಕ್ಷಿತ_malloc(ಗಾತ್ರ(ಶೂನ್ಯ*) * Cur_Array_ಗಾತ್ರ);

  memset(CurArray, -1, Cur_Array_ಗಾತ್ರ*ಗಾತ್ರ(ಶೂನ್ಯ*));
}

std::pair<ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *, ತಾರ್ಕಿಕ>
Small_Ptr_ಗಣ_Impl_ತಳ::ಸೇರಿಸು_imp_big(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Ptr) {
  ಆದರೆ (LLVM_UNLIKELY(ಗಾತ್ರ_3() * 3 >= Cur_Array_ಗಾತ್ರ * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_ಗಾತ್ರ < 64 ? 128 : Cur_Array_ಗಾತ್ರ * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = Cur_Array_ಗಾತ್ರ - 1;
  ಚಿಹ್ನೆರಹಿತ I = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & ಮುಖವಾಡ;
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **Array = CurArray;
  ಆಗ (Array[I] != get_ಖಾಲಿಗಳು_Marker()) {
    ಆದರೆ (Array[I] == Ptr)
      ಹಿಂತಿರುಗಿ {Array + I, ಸುಳ್ಳು};
    I = (I + 1) & ಮುಖವಾಡ;
  }

  // Insert into the empty bucket.
  ++ಅಂಕೆ_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  ಹಿಂತಿರುಗಿ {Array + I, ನಿಜ};
}

ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *Small_Ptr_ಗಣ_Impl_ತಳ::do_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Ptr) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = Cur_Array_ಗಾತ್ರ - 1;
  ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ_No = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & ಮುಖವಾಡ;
  ಆಗ (ನಿಜ) {
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *ಪಾತ್ರೆ = CurArray + ಪಾತ್ರೆ_No;
    ಆದರೆ (LLVM_LIKELY(*ಪಾತ್ರೆ == Ptr))
      ಹಿಂತಿರುಗಿ ಪಾತ್ರೆ;
    ಆದರೆ (LLVM_LIKELY(*ಪಾತ್ರೆ == get_ಖಾಲಿಗಳು_Marker()))
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    ಪಾತ್ರೆ_No = (ಪಾತ್ರೆ_No + 1) & ಮುಖವಾಡ;
  }
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::erase_From_ಪಾತ್ರೆ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **ಪಾತ್ರೆ) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = Cur_Array_ಗಾತ್ರ - 1;
  ಚಿಹ್ನೆರಹಿತ I = ಪಾತ್ರೆ - CurArray;
  ಚಿಹ್ನೆರಹಿತ J = I;
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಖಾಲಿಗಳು_2 = get_ಖಾಲಿಗಳು_Marker();
  ಆಗ ((J = (J + 1) & ಮುಖವಾಡ), CurArray[J] != ಖಾಲಿಗಳು_2) {
    ಸ್ವಯಂಚಾಲಿತ Ideal = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(CurArray[J]);
    ಆದರೆ (((I - Ideal) & ಮುಖವಾಡ) < ((J - Ideal) & ಮುಖವಾಡ)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = ಖಾಲಿಗಳು_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::Grow(ಚಿಹ್ನೆರಹಿತ New_ಗಾತ್ರ) {
  ಸ್ವಯಂಚಾಲಿತ OldBuckets = buckets();
  ತಾರ್ಕಿಕ WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **NewBuckets = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**) ಸುರಕ್ಷಿತ_malloc(ಗಾತ್ರ(ಶೂನ್ಯ*) * New_ಗಾತ್ರ);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_ಗಾತ್ರ = New_ಗಾತ್ರ;
  memset(CurArray, -1, New_ಗಾತ್ರ*ಗಾತ್ರ(ಶೂನ್ಯ*));

  // Copy over all valid entries.
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = Cur_Array_ಗಾತ್ರ - 1;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Ptr : OldBuckets) {
    ಆದರೆ (Ptr == get_ಖಾಲಿಗಳು_Marker())
      ಮುಂದುವರಿ;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    ಚಿಹ್ನೆರಹಿತ I = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(Ptr) & ಮುಖವಾಡ;
    ಆಗ (NewBuckets[I] != get_ಖಾಲಿಗಳು_Marker())
      I = (I + 1) & ಮುಖವಾಡ;
    NewBuckets[I] = Ptr;
  }

  ಆದರೆ (!WasSmall)
    ಬಿಡುಗಡೆ(OldBuckets.begin());
  IsSmall = ಸುಳ್ಳು;
}

Small_Ptr_ಗಣ_Impl_ತಳ::Small_Ptr_ಗಣ_Impl_ತಳ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                                         ಸ್ಥಿರಾಂಕ Small_Ptr_ಗಣ_Impl_ತಳ &that) {
  IsSmall = that.isSmall();
  ಆದರೆ (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**)ಸುರಕ್ಷಿತ_malloc(ಗಾತ್ರ(ಶೂನ್ಯ*) * that.Cur_Array_ಗಾತ್ರ);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_ಗಣ_Impl_ತಳ::Small_Ptr_ಗಣ_Impl_ತಳ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                                         ಚಿಹ್ನೆರಹಿತ Small_ಗಾತ್ರ,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **RHSSmallStorage,
                                         Small_Ptr_ಗಣ_Impl_ತಳ &&that) {
  moveHelper(SmallStorage, Small_ಗಾತ್ರ, RHSSmallStorage, std::move(that));
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::copyFrom(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                                   ಸ್ಥಿರಾಂಕ Small_Ptr_ಗಣ_Impl_ತಳ &RHS) {
  ಪ್ರತಿಜ್ಞಾ(&RHS != ಇದು && "Self-copy should be handled by the caller.");

  ಆದರೆ (isSmall() && RHS.isSmall())
    ಪ್ರತಿಜ್ಞಾ(Cur_Array_ಗಾತ್ರ == RHS.Cur_Array_ಗಾತ್ರ &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  ಆದರೆ (RHS.isSmall()) {
    ಆದರೆ (!isSmall())
      ಬಿಡುಗಡೆ(CurArray);
    CurArray = SmallStorage;
    IsSmall = ನಿಜ;
    // Otherwise, allocate new heap space (unless we were the same size)
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Cur_Array_ಗಾತ್ರ != RHS.Cur_Array_ಗಾತ್ರ) {
    ಆದರೆ (isSmall())
      CurArray = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**)ಸುರಕ್ಷಿತ_malloc(ಗಾತ್ರ(ಶೂನ್ಯ*) * RHS.Cur_Array_ಗಾತ್ರ);
    ಇಲ್ಲದಿದ್ದರೆ {
      ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **T = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**)ಸುರಕ್ಷಿತ_realloc(CurArray,
                                             ಗಾತ್ರ(ಶೂನ್ಯ*) * RHS.Cur_Array_ಗಾತ್ರ);
      CurArray = T;
    }
    IsSmall = ಸುಳ್ಳು;
  }

  copyHelper(RHS);
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::copyHelper(ಸ್ಥಿರಾಂಕ Small_Ptr_ಗಣ_Impl_ತಳ &RHS) {
  // Copy over the new array size
  Cur_Array_ಗಾತ್ರ = RHS.Cur_Array_ಗಾತ್ರ;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  ಅಂಕೆ_Entries = RHS.ಅಂಕೆ_Entries;
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::moveFrom(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                                   ಚಿಹ್ನೆರಹಿತ Small_ಗಾತ್ರ,
                                   ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **RHSSmallStorage,
                                   Small_Ptr_ಗಣ_Impl_ತಳ &&RHS) {
  ಆದರೆ (!isSmall())
    ಬಿಡುಗಡೆ(CurArray);
  moveHelper(SmallStorage, Small_ಗಾತ್ರ, RHSSmallStorage, std::move(RHS));
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::moveHelper(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                                     ಚಿಹ್ನೆರಹಿತ Small_ಗಾತ್ರ,
                                     ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **RHSSmallStorage,
                                     Small_Ptr_ಗಣ_Impl_ತಳ &&RHS) {
  ಪ್ರತಿಜ್ಞಾ(&RHS != ಇದು && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  ಆದರೆ (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } ಇಲ್ಲದಿದ್ದರೆ {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_ಗಾತ್ರ = RHS.Cur_Array_ಗಾತ್ರ;
  ಅಂಕೆ_Entries = RHS.ಅಂಕೆ_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_ಗಾತ್ರ = Small_ಗಾತ್ರ;
  RHS.ಅಂಕೆ_Entries = 0;
  RHS.IsSmall = ನಿಜ;
}

ಶೂನ್ಯ Small_Ptr_ಗಣ_Impl_ತಳ::ಅದಲುಬದಲು(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **SmallStorage,
                               ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **RHSSmallStorage,
                               Small_Ptr_ಗಣ_Impl_ತಳ &RHS) {
  ಆದರೆ (ಇದು == &RHS) ಹಿಂತಿರುಗಿ;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  ಆದರೆ (!ಇದು->isSmall() && !RHS.isSmall()) {
    std::ಅದಲುಬದಲು(ಇದು->CurArray, RHS.CurArray);
    std::ಅದಲುಬದಲು(ಇದು->Cur_Array_ಗಾತ್ರ, RHS.Cur_Array_ಗಾತ್ರ);
    std::ಅದಲುಬದಲು(ಇದು->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    ಹಿಂತಿರುಗಿ;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  ಆದರೆ (ಇದು->isSmall() && RHS.isSmall()) {
    ಚಿಹ್ನೆರಹಿತ MinEntries = std::min(ಇದು->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    std::ಅದಲುಬದಲು_ranges(ಇದು->CurArray, ಇದು->CurArray + MinEntries, RHS.CurArray);
    ಆದರೆ (ಇದು->ಅಂಕೆ_Entries > MinEntries) {
      std::copy(ಇದು->CurArray + MinEntries, ಇದು->CurArray + ಇದು->ಅಂಕೆ_Entries,
                RHS.CurArray + MinEntries);
    } ಇಲ್ಲದಿದ್ದರೆ {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.ಅಂಕೆ_Entries,
                ಇದು->CurArray + MinEntries);
    }
    ಪ್ರತಿಜ್ಞಾ(ಇದು->Cur_Array_ಗಾತ್ರ == RHS.Cur_Array_ಗಾತ್ರ);
    std::ಅದಲುಬದಲು(ಇದು->ಅಂಕೆ_Entries, RHS.ಅಂಕೆ_Entries);
    ಹಿಂತಿರುಗಿ;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_ಗಣ_Impl_ತಳ &SmallSide = ಇದು->isSmall() ? *ಇದು : RHS;
  Small_Ptr_ಗಣ_Impl_ತಳ &LargeSide = ಇದು->isSmall() ? RHS : *ಇದು;
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **LargeSideInlineStorage =
      ಇದು->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::ಅದಲುಬದಲು(LargeSide.Cur_Array_ಗಾತ್ರ, SmallSide.Cur_Array_ಗಾತ್ರ);
  std::ಅದಲುಬದಲು(LargeSide.ಅಂಕೆ_Entries, SmallSide.ಅಂಕೆ_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = ಸುಳ್ಳು;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = ನಿಜ;
}
