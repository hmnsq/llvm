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

#అన్తర్భావ "llvm/ADT/SmallPtrSet.h"
#అన్తర్భావ "llvm/ADT/DenseMapInfo.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/MemAlloc.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdlib>

ఉపయోగించి నామాకాశ llvm;

శూన్య Small_Ptr_సమితి_Impl_పునాది::shrink_and_clear() {
  ప్రతిజ్ఞా(!isSmall() && "Can't shrink a small set!");
  విడుదల(CurArray);

  // Reduce the number of buckets.
  గుర్తులేని కొలత = కొలత_2();
  Cur_Array_కొలత = కొలత > 16 ? 1 << (Log2_32_Ceil(కొలత) + 1) : 32;
  అంకె_Entries = 0;

  // Install the new array.  Clear all the buckets to empty.
  CurArray = (స్థిరరాశి శూన్య**)సురక్షిత_malloc(పరిమాణం(శూన్య*) * Cur_Array_కొలత);

  memset(CurArray, -1, Cur_Array_కొలత*పరిమాణం(శూన్య*));
}

std::pair<స్థిరరాశి శూన్య *స్థిరరాశి *, తార్కిక>
Small_Ptr_సమితి_Impl_పునాది::చొప్పించు_imp_big(స్థిరరాశి శూన్య *Ptr) {
  ఒకవేళ (LLVM_UNLIKELY(కొలత_2() * 3 >= Cur_Array_కొలత * 2)) {
    // If more than 2/3 of the array is full, grow.
    Grow(Cur_Array_కొలత < 64 ? 128 : Cur_Array_కొలత * 2);
  }

  // Find the first empty bucket or Ptr itself on the probe chain.
  గుర్తులేని ముసుగు = Cur_Array_కొలత - 1;
  గుర్తులేని I = DenseMapInfo<శూన్య *>::get_Hash_విలువ(Ptr) & ముసుగు;
  స్థిరరాశి శూన్య **Array = CurArray;
  అయితే (Array[I] != get_ఖాళీలు_Marker()) {
    ఒకవేళ (Array[I] == Ptr)
      తిరిగి {Array + I, అబద్ధం};
    I = (I + 1) & ముసుగు;
  }

  // Insert into the empty bucket.
  ++అంకె_Entries;
  Array[I] = Ptr;
  incrementEpoch();
  తిరిగి {Array + I, నిజం};
}

స్థిరరాశి శూన్య *స్థిరరాశి *Small_Ptr_సమితి_Impl_పునాది::do_కనుగొను(స్థిరరాశి శూన్య *Ptr) స్థిరరాశి {
  గుర్తులేని ముసుగు = Cur_Array_కొలత - 1;
  గుర్తులేని పాత్ర_No = DenseMapInfo<శూన్య *>::get_Hash_విలువ(Ptr) & ముసుగు;
  అయితే (నిజం) {
    స్థిరరాశి శూన్య *స్థిరరాశి *పాత్ర = CurArray + పాత్ర_No;
    ఒకవేళ (LLVM_LIKELY(*పాత్ర == Ptr))
      తిరిగి పాత్ర;
    ఒకవేళ (LLVM_LIKELY(*పాత్ర == get_ఖాళీలు_Marker()))
      తిరిగి శూన్యసూచిక;
    పాత్ర_No = (పాత్ర_No + 1) & ముసుగు;
  }
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::erase_From_పాత్ర(స్థిరరాశి శూన్య **పాత్ర) {
  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  గుర్తులేని ముసుగు = Cur_Array_కొలత - 1;
  గుర్తులేని I = పాత్ర - CurArray;
  గుర్తులేని J = I;
  స్థిరరాశి శూన్య *ఖాళీలు_2 = get_ఖాళీలు_Marker();
  అయితే ((J = (J + 1) & ముసుగు), CurArray[J] != ఖాళీలు_2) {
    స్వయంచాలక Ideal = DenseMapInfo<శూన్య *>::get_Hash_విలువ(CurArray[J]);
    ఒకవేళ (((I - Ideal) & ముసుగు) < ((J - Ideal) & ముసుగు)) {
      CurArray[I] = CurArray[J];
      I = J;
    }
  }
  CurArray[I] = ఖాళీలు_2;
}

/// Grow - Allocate a larger backing store for the buckets and move it over.
///
శూన్య Small_Ptr_సమితి_Impl_పునాది::Grow(గుర్తులేని New_కొలత) {
  స్వయంచాలక OldBuckets = buckets();
  తార్కిక WasSmall = isSmall();

  // Install the new array.  Clear all the buckets to empty.
  స్థిరరాశి శూన్య **NewBuckets = (స్థిరరాశి శూన్య**) సురక్షిత_malloc(పరిమాణం(శూన్య*) * New_కొలత);

  // Reset member only if memory was allocated successfully
  CurArray = NewBuckets;
  Cur_Array_కొలత = New_కొలత;
  memset(CurArray, -1, New_కొలత*పరిమాణం(శూన్య*));

  // Copy over all valid entries.
  గుర్తులేని ముసుగు = Cur_Array_కొలత - 1;
  కొరకు (స్థిరరాశి శూన్య *Ptr : OldBuckets) {
    ఒకవేళ (Ptr == get_ఖాళీలు_Marker())
      కొనసాగు;
    // Find the first empty bucket on this key's probe chain; there is no equal
    // key, so nothing to compare against.
    గుర్తులేని I = DenseMapInfo<శూన్య *>::get_Hash_విలువ(Ptr) & ముసుగు;
    అయితే (NewBuckets[I] != get_ఖాళీలు_Marker())
      I = (I + 1) & ముసుగు;
    NewBuckets[I] = Ptr;
  }

  ఒకవేళ (!WasSmall)
    విడుదల(OldBuckets.begin());
  IsSmall = అబద్ధం;
}

Small_Ptr_సమితి_Impl_పునాది::Small_Ptr_సమితి_Impl_పునాది(స్థిరరాశి శూన్య **SmallStorage,
                                         స్థిరరాశి Small_Ptr_సమితి_Impl_పునాది &that) {
  IsSmall = that.isSmall();
  ఒకవేళ (IsSmall) {
    // If we're becoming small, prepare to insert into our stack space
    CurArray = SmallStorage;
  } లేకపోతే {
    // Otherwise, allocate new heap space (unless we were the same size)
    CurArray = (స్థిరరాశి శూన్య**)సురక్షిత_malloc(పరిమాణం(శూన్య*) * that.Cur_Array_కొలత);
  }

  // Copy over the that array.
  copyHelper(that);
}

Small_Ptr_సమితి_Impl_పునాది::Small_Ptr_సమితి_Impl_పునాది(స్థిరరాశి శూన్య **SmallStorage,
                                         గుర్తులేని Small_కొలత,
                                         స్థిరరాశి శూన్య **RHSSmallStorage,
                                         Small_Ptr_సమితి_Impl_పునాది &&that) {
  moveHelper(SmallStorage, Small_కొలత, RHSSmallStorage, std::move(that));
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::copyFrom(స్థిరరాశి శూన్య **SmallStorage,
                                   స్థిరరాశి Small_Ptr_సమితి_Impl_పునాది &RHS) {
  ప్రతిజ్ఞా(&RHS != ఇది && "Self-copy should be handled by the caller.");

  ఒకవేళ (isSmall() && RHS.isSmall())
    ప్రతిజ్ఞా(Cur_Array_కొలత == RHS.Cur_Array_కొలత &&
           "Cannot assign sets with different small sizes");

  // If we're becoming small, prepare to insert into our stack space
  ఒకవేళ (RHS.isSmall()) {
    ఒకవేళ (!isSmall())
      విడుదల(CurArray);
    CurArray = SmallStorage;
    IsSmall = నిజం;
    // Otherwise, allocate new heap space (unless we were the same size)
  } లేకపోతే ఒకవేళ (Cur_Array_కొలత != RHS.Cur_Array_కొలత) {
    ఒకవేళ (isSmall())
      CurArray = (స్థిరరాశి శూన్య**)సురక్షిత_malloc(పరిమాణం(శూన్య*) * RHS.Cur_Array_కొలత);
    లేకపోతే {
      స్థిరరాశి శూన్య **T = (స్థిరరాశి శూన్య**)సురక్షిత_realloc(CurArray,
                                             పరిమాణం(శూన్య*) * RHS.Cur_Array_కొలత);
      CurArray = T;
    }
    IsSmall = అబద్ధం;
  }

  copyHelper(RHS);
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::copyHelper(స్థిరరాశి Small_Ptr_సమితి_Impl_పునాది &RHS) {
  // Copy over the new array size
  Cur_Array_కొలత = RHS.Cur_Array_కొలత;

  // Copy over the contents from the other set
  llvm::copy(RHS.buckets(), CurArray);

  అంకె_Entries = RHS.అంకె_Entries;
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::moveFrom(స్థిరరాశి శూన్య **SmallStorage,
                                   గుర్తులేని Small_కొలత,
                                   స్థిరరాశి శూన్య **RHSSmallStorage,
                                   Small_Ptr_సమితి_Impl_పునాది &&RHS) {
  ఒకవేళ (!isSmall())
    విడుదల(CurArray);
  moveHelper(SmallStorage, Small_కొలత, RHSSmallStorage, std::move(RHS));
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::moveHelper(స్థిరరాశి శూన్య **SmallStorage,
                                     గుర్తులేని Small_కొలత,
                                     స్థిరరాశి శూన్య **RHSSmallStorage,
                                     Small_Ptr_సమితి_Impl_పునాది &&RHS) {
  ప్రతిజ్ఞా(&RHS != ఇది && "Self-move should be handled by the caller.");
  incrementEpoch();
  RHS.incrementEpoch();

  ఒకవేళ (RHS.isSmall()) {
    // Copy a small RHS rather than moving.
    CurArray = SmallStorage;
    llvm::copy(RHS.small_buckets(), CurArray);
  } లేకపోతే {
    CurArray = RHS.CurArray;
    RHS.CurArray = RHSSmallStorage;
  }

  // Copy the rest of the trivial members.
  Cur_Array_కొలత = RHS.Cur_Array_కొలత;
  అంకె_Entries = RHS.అంకె_Entries;
  IsSmall = RHS.IsSmall;

  // Make the RHS small and empty.
  RHS.Cur_Array_కొలత = Small_కొలత;
  RHS.అంకె_Entries = 0;
  RHS.IsSmall = నిజం;
}

శూన్య Small_Ptr_సమితి_Impl_పునాది::మార్పిడి(స్థిరరాశి శూన్య **SmallStorage,
                               స్థిరరాశి శూన్య **RHSSmallStorage,
                               Small_Ptr_సమితి_Impl_పునాది &RHS) {
  ఒకవేళ (ఇది == &RHS) తిరిగి;
  incrementEpoch();
  RHS.incrementEpoch();

  // We can only avoid copying elements if neither set is small.
  ఒకవేళ (!ఇది->isSmall() && !RHS.isSmall()) {
    std::మార్పిడి(ఇది->CurArray, RHS.CurArray);
    std::మార్పిడి(ఇది->Cur_Array_కొలత, RHS.Cur_Array_కొలత);
    std::మార్పిడి(ఇది->అంకె_Entries, RHS.అంకె_Entries);
    తిరిగి;
  }

  // FIXME: From here on we assume that both sets have the same small size.

  // Both a small, just swap the small elements.
  ఒకవేళ (ఇది->isSmall() && RHS.isSmall()) {
    గుర్తులేని MinEntries = std::min(ఇది->అంకె_Entries, RHS.అంకె_Entries);
    std::మార్పిడి_ranges(ఇది->CurArray, ఇది->CurArray + MinEntries, RHS.CurArray);
    ఒకవేళ (ఇది->అంకె_Entries > MinEntries) {
      std::copy(ఇది->CurArray + MinEntries, ఇది->CurArray + ఇది->అంకె_Entries,
                RHS.CurArray + MinEntries);
    } లేకపోతే {
      std::copy(RHS.CurArray + MinEntries, RHS.CurArray + RHS.అంకె_Entries,
                ఇది->CurArray + MinEntries);
    }
    ప్రతిజ్ఞా(ఇది->Cur_Array_కొలత == RHS.Cur_Array_కొలత);
    std::మార్పిడి(ఇది->అంకె_Entries, RHS.అంకె_Entries);
    తిరిగి;
  }

  // If only one side is small, copy the small elements into the large side and
  // move the pointer from the large side to the small side.
  Small_Ptr_సమితి_Impl_పునాది &SmallSide = ఇది->isSmall() ? *ఇది : RHS;
  Small_Ptr_సమితి_Impl_పునాది &LargeSide = ఇది->isSmall() ? RHS : *ఇది;
  స్థిరరాశి శూన్య **LargeSideInlineStorage =
      ఇది->isSmall() ? RHSSmallStorage : SmallStorage;
  llvm::copy(SmallSide.small_buckets(), LargeSideInlineStorage);
  std::మార్పిడి(LargeSide.Cur_Array_కొలత, SmallSide.Cur_Array_కొలత);
  std::మార్పిడి(LargeSide.అంకె_Entries, SmallSide.అంకె_Entries);
  SmallSide.CurArray = LargeSide.CurArray;
  SmallSide.IsSmall = అబద్ధం;
  LargeSide.CurArray = LargeSideInlineStorage;
  LargeSide.IsSmall = నిజం;
}
