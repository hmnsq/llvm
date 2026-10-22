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

उपयुज्य नामाकाश llvm;

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::shrink_and_clear() {
  प्रतिज्ञा(!isSmall() && "Can't shrink a small set!");
  मुक्त(CurArray);

  // Reduce the number of buckets.
  अचिह्नित आकार = आकार_2();
  Cur_Array_आकार = आकार > 16 ? 1 << (Log2_32_Ceil(आकार) + 1) : 32;
  अङ्कः_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * Cur_Array_आकार);

  स्मृतिं_पूरय(CurArray, -1, Cur_Array_आकार*परिमाण(शून्य*));
}

std::pair<ध्रुव शून्य *ध्रुव *, तार्किक>
Small_Ptr_समुच्चयः_Impl_आधारः::निवेशय_imp_big(ध्रुव शून्य *Ptr) {
  यदि (LLVM_UNLIKELY(आकार_2() * 3 >= Cur_Array_आकार * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_आकार < 64 ? 128 : Cur_Array_आकार * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  अचिह्नित आवरणम् = Cur_Array_आकार - 1;
  अचिह्नित I = DenseMapInfo<शून्य *>::get_Hash_मानम्(Ptr) & आवरणम्;
  ध्रुव शून्य **Array = CurArray;
  यावत् (Array[I] != get_रिक्तानि_Marker()) {
    यदि (Array[I] == Ptr)
      प्रत्यागम {Array + I, असत्य};
    I = (I + 1) & आवरणम्;
  }

  // Insert into the empty bucket.
  ++अङ्कः_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  प्रत्यागम {Array + I, सत्य};
}

ध्रुव शून्य *ध्रुव *Small_Ptr_समुच्चयः_Impl_आधारः::doFind(ध्रुव शून्य *Ptr) ध्रुव {
  अचिह्नित आवरणम् = Cur_Array_आकार - 1;
  अचिह्नित BucketNo = DenseMapInfo<शून्य *>::get_Hash_मानम्(Ptr) & आवरणम्;
  यावत् (सत्य) {
    ध्रुव शून्य *ध्रुव *Bucket = CurArray + BucketNo;
    यदि (LLVM_LIKELY(*Bucket == Ptr))
      प्रत्यागम Bucket;
    यदि (LLVM_LIKELY(*Bucket == get_रिक्तानि_Marker()))
      प्रत्यागम शून्यसूचकः;
    BucketNo = (BucketNo + 1) & आवरणम्;
  }
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::eraseFromBucket(ध्रुव शून्य **Bucket) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  अचिह्नित आवरणम् = Cur_Array_आकार - 1;
  अचिह्नित I = Bucket - CurArray;
  अचिह्नित J = I;
  ध्रुव शून्य *रिक्तानि_2 = get_रिक्तानि_Marker();
  यावत् ((J = (J + 1) & आवरणम्), CurArray[J] != रिक्तानि_2) {
    स्वचालित Ideal = DenseMapInfo<शून्य *>::get_Hash_मानम्(CurArray[J]);
    यदि (((I - Ideal) & आवरणम्) < ((J - Ideal) & आवरणम्)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = रिक्तानि_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
शून्य Small_Ptr_समुच्चयः_Impl_आधारः::Grow(अचिह्नित New_आकार) {
  स्वचालित OldBuckets = buckets();
  तार्किक WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  ध्रुव शून्य **NewBuckets = (ध्रुव शून्य**) safe_malloc(परिमाण(शून्य*) * New_आकार);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_आकार = New_आकार;
  स्मृतिं_पूरय(CurArray, -1, New_आकार*परिमाण(शून्य*));

  // Copy over all valid entries.
  अचिह्नित आवरणम् = Cur_Array_आकार - 1;
  प्रति (ध्रुव शून्य *Ptr : OldBuckets) {
    यदि (Ptr == get_रिक्तानि_Marker())
      अनुवर्त;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    अचिह्नित I = DenseMapInfo<शून्य *>::get_Hash_मानम्(Ptr) & आवरणम्;
    यावत् (NewBuckets[I] != get_रिक्तानि_Marker())
      I = (I + 1) & आवरणम्;
    NewBuckets[I] = Ptr;
  }

  यदि (!WasSmall)
    मुक्त(OldBuckets.begin());
  IsSmall = असत्य;
}

Small_Ptr_समुच्चयः_Impl_आधारः::Small_Ptr_समुच्चयः_Impl_आधारः(ध्रुव शून्य **SmallStorage,
                                         ध्रुव Small_Ptr_समुच्चयः_Impl_आधारः &that) {
  IsSmall = that.isSmall();
  यदि (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } अन्यथा {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * that.Cur_Array_आकार);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_समुच्चयः_Impl_आधारः::Small_Ptr_समुच्चयः_Impl_आधारः(ध्रुव शून्य **SmallStorage,
                                         अचिह्नित Small_आकार,
                                         ध्रुव शून्य **RHSSmallStorage,
                                         Small_Ptr_समुच्चयः_Impl_आधारः &&that) {
  moveHelper(SmallStorage, Small_आकार, RHSSmallStorage, std::move(that));
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::copyFrom(ध्रुव शून्य **SmallStorage,
                                   ध्रुव Small_Ptr_समुच्चयः_Impl_आधारः &RHS) {
  प्रतिज्ञा(&RHS != एतत् && "Self-copy should be handled by the caller.");

  यदि (isSmall() && RHS.isSmall())
    प्रतिज्ञा(Cur_Array_आकार == RHS.Cur_Array_आकार &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  यदि (RHS.isSmall()) {
    यदि (!isSmall())
      मुक्त(CurArray);
    CurArray = SmallStorage;
    IsSmall = सत्य;
    // Otherwise, allocate new heap space (unless we were the same size)
  } अन्यथा यदि (Cur_Array_आकार != RHS.Cur_Array_आकार) {
    यदि (isSmall())
      CurArray = (ध्रुव शून्य**)safe_malloc(परिमाण(शून्य*) * RHS.Cur_Array_आकार);
    अन्यथा {
      ध्रुव शून्य **T = (ध्रुव शून्य**)safe_realloc(CurArray,
                                             परिमाण(शून्य*) * RHS.Cur_Array_आकार);
      CurArray = T;
    }
    IsSmall = असत्य;
  }

  copyHelper(RHS);
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::copyHelper(ध्रुव Small_Ptr_समुच्चयः_Impl_आधारः &RHS) {
  // Copy over the new array size
  Cur_Array_आकार = RHS.Cur_Array_आकार;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  अङ्कः_Entries = RHS.अङ्कः_Entries;
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::moveFrom(ध्रुव शून्य **SmallStorage,
                                   अचिह्नित Small_आकार,
                                   ध्रुव शून्य **RHSSmallStorage,
                                   Small_Ptr_समुच्चयः_Impl_आधारः &&RHS) {
  यदि (!isSmall())
    मुक्त(CurArray);
  moveHelper(SmallStorage, Small_आकार, RHSSmallStorage, std::move(RHS));
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::moveHelper(ध्रुव शून्य **SmallStorage,
                                     अचिह्नित Small_आकार,
                                     ध्रुव शून्य **RHSSmallStorage,
                                     Small_Ptr_समुच्चयः_Impl_आधारः &&RHS) {
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
  Cur_Array_आकार = RHS.Cur_Array_आकार;
  अङ्कः_Entries = RHS.अङ्कः_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_आकार = Small_आकार;
  RHS.अङ्कः_Entries = 0;
  RHS.IsSmall = सत्य;
}

शून्य Small_Ptr_समुच्चयः_Impl_आधारः::swap(ध्रुव शून्य **SmallStorage,
                               ध्रुव शून्य **RHSSmallStorage,
                               Small_Ptr_समुच्चयः_Impl_आधारः &RHS) {
  यदि (एतत् == &RHS) प्रत्यागम;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  यदि (!एतत्->isSmall() && !RHS.isSmall()) {
    std::swap(एतत्->CurArray, RHS.CurArray);
    std::swap(एतत्->Cur_Array_आकार, RHS.Cur_Array_आकार);
    std::swap(एतत्->अङ्कः_Entries, RHS.अङ्कः_Entries);
    प्रत्यागम;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  यदि (एतत्->isSmall() && RHS.isSmall()) {
    अचिह्नित MinEntries = std::min(एतत्->अङ्कः_Entries, RHS.अङ्कः_Entries);
    std::swap_ranges(एतत्->CurArray, एतत्->CurArray + MinEntries, RHS.CurArray);
    यदि (एतत्->अङ्कः_Entries > MinEntries) {
      std::copy(एतत्->CurArray + MinEntries, एतत्->CurArray + एतत्->अङ्कः_Entries,
                RHS.CurArray + MinEntries);
    } अन्यथा {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.अङ्कः_Entries,
                एतत्->CurArray + MinEntries);
    }
    प्रतिज्ञा(एतत्->Cur_Array_आकार == RHS.Cur_Array_आकार);
    std::swap(एतत्->अङ्कः_Entries, RHS.अङ्कः_Entries);
    प्रत्यागम;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_समुच्चयः_Impl_आधारः &SmallSide = एतत्->isSmall() ? *एतत् : RHS;
  Small_Ptr_समुच्चयः_Impl_आधारः &LargeSide = एतत्->isSmall() ? RHS : *एतत्;
  ध्रुव शून्य **LargeSideInlineStorage =
      एतत्->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::swap(LargeSide.Cur_Array_आकार, SmallSide.Cur_Array_आकार);
  std::swap(LargeSide.अङ्कः_Entries, SmallSide.अङ्कः_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = असत्य;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = सत्य;
}
