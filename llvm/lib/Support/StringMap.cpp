//===--- StringMap.cpp - String Hash table map implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the StringMap class.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/ReverseIteration.h"

ఉపయోగించి నామాకాశ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
స్థిర అంతఃవరుస గుర్తులేని get_Min_పాత్ర_గమ్యం_Reserve_For_Entries(గుర్తులేని అంకె_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ఒకవేళ (అంకె_Entries == 0)
    తిరిగి 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  తిరిగి తదుపరి_Power_Of2(అంకె_Entries * 4 / 3 + 1);
}

స్థిర అంతఃవరుస String_Map_నమోదు_పునాది **create_పట్టిక(గుర్తులేని New_అంకె_Buckets) {
  స్వయంచాలక **పట్టిక = static_cast<String_Map_నమోదు_పునాది **>(సురక్షిత_calloc(
      New_అంకె_Buckets + 1, పరిమాణం(String_Map_నమోదు_పునాది **) + పరిమాణం(గుర్తులేని)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  పట్టిక[New_అంకె_Buckets] = (String_Map_నమోదు_పునాది *)2;
  తిరిగి పట్టిక;
}

స్థిర అంతఃవరుస గుర్తులేని *get_Hash_పట్టిక(String_Map_నమోదు_పునాది **The_పట్టిక,
                                     గుర్తులేని అంకె_Buckets) {
  తిరిగి reinterpret_cast<గుర్తులేని *>(The_పట్టిక + అంకె_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef కీలకం) { తిరిగి xxh3_64bits(కీలకం); }

StringMapImpl::StringMapImpl(గుర్తులేని Init_కొలత, గుర్తులేని అంశం_కొలత)
    : అంశం_కొలత_2(అంశం_కొలత) {
  // If a size is specified, initialize the table with that many buckets.
  ఒకవేళ (Init_కొలత) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_పాత్ర_గమ్యం_Reserve_For_Entries(Init_కొలత));
  }
}

శూన్య StringMapImpl::init(గుర్తులేని Init_కొలత) {
  ప్రతిజ్ఞా((Init_కొలత & (Init_కొలత - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  గుర్తులేని New_అంకె_Buckets = Init_కొలత ? Init_కొలత : 16;
  అంకె_వస్తువులు = 0;

  The_పట్టిక = create_పట్టిక(New_అంకె_Buckets);

  // Set the member only if TheTable was successfully allocated
  అంకె_Buckets = New_అంకె_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
గుర్తులేని StringMapImpl::Lookup_పాత్ర_For(StringRef Name,
                                        uint32_t నిండిన_Hash_విలువ) {
#నిర్వచితమైతే EXPENSIVE_CHECKS
  ప్రతిజ్ఞా(నిండిన_Hash_విలువ == hash(Name));
#అయితే_ముగింపు
  // Hash table unallocated so far?
  ఒకవేళ (అంకె_Buckets == 0)
    init(16);
  ఒకవేళ constexpr (should_తిరగేయి_Iterate())
    నిండిన_Hash_విలువ = ~నిండిన_Hash_విలువ;
  గుర్తులేని పాత్ర_No = నిండిన_Hash_విలువ & (అంకె_Buckets - 1);
  గుర్తులేని *Hash_పట్టిక = get_Hash_పట్టిక(The_పట్టిక, అంకె_Buckets);

  అయితే (నిజం) {
    String_Map_నమోదు_పునాది *పాత్ర_అంశం = The_పట్టిక[పాత్ర_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ఒకవేళ (LLVM_LIKELY(!పాత్ర_అంశం)) {
      Hash_పట్టిక[పాత్ర_No] = నిండిన_Hash_విలువ;
      తిరిగి పాత్ర_No;
    }

    ఒకవేళ (LLVM_LIKELY(Hash_పట్టిక[పాత్ర_No] == నిండిన_Hash_విలువ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      అక్షరం *అంశం_Str = (అక్షరం *)పాత్ర_అంశం + అంశం_కొలత_2;
      ఒకవేళ (Name == StringRef(అంశం_Str, పాత్ర_అంశం->get_కీలకం_నిడివి())) {
        // We found a match!
        తిరిగి పాత్ర_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    పాత్ర_No = (పాత్ర_No + 1) & (అంకె_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
పూర్ణాంకం StringMapImpl::కనుగొను_కీలకం(StringRef కీలకం, uint32_t నిండిన_Hash_విలువ) స్థిరరాశి {
  ఒకవేళ (అంకె_Buckets == 0)
    తిరిగి -1; // Really empty table?
#నిర్వచితమైతే EXPENSIVE_CHECKS
  ప్రతిజ్ఞా(నిండిన_Hash_విలువ == hash(కీలకం));
#అయితే_ముగింపు
  ఒకవేళ constexpr (should_తిరగేయి_Iterate())
    నిండిన_Hash_విలువ = ~నిండిన_Hash_విలువ;
  గుర్తులేని పాత్ర_No = నిండిన_Hash_విలువ & (అంకె_Buckets - 1);
  గుర్తులేని *Hash_పట్టిక = get_Hash_పట్టిక(The_పట్టిక, అంకె_Buckets);

  అయితే (నిజం) {
    String_Map_నమోదు_పునాది *పాత్ర_అంశం = The_పట్టిక[పాత్ర_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ఒకవేళ (LLVM_LIKELY(!పాత్ర_అంశం))
      తిరిగి -1;

    ఒకవేళ (LLVM_LIKELY(Hash_పట్టిక[పాత్ర_No] == నిండిన_Hash_విలువ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      అక్షరం *అంశం_Str = (అక్షరం *)పాత్ర_అంశం + అంశం_కొలత_2;
      ఒకవేళ (కీలకం == StringRef(అంశం_Str, పాత్ర_అంశం->get_కీలకం_నిడివి())) {
        // We found a match!
        తిరిగి పాత్ర_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    పాత్ర_No = (పాత్ర_No + 1) & (అంకె_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
శూన్య StringMapImpl::Remove_కీలకం(String_Map_నమోదు_పునాది *V) {
  స్థిరరాశి అక్షరం *VStr = (అక్షరం *)V + అంశం_కొలత_2;
  String_Map_నమోదు_పునాది *V2 = Remove_కీలకం(StringRef(VStr, V->get_కీలకం_నిడివి()));
  (శూన్య)V2;
  ప్రతిజ్ఞా(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
శూన్య StringMapImpl::remove_పాత్ర(గుర్తులేని పాత్ర) {
  గుర్తులేని *Hash_పట్టిక = get_Hash_పట్టిక(The_పట్టిక, అంకె_Buckets);
  గుర్తులేని ముసుగు = అంకె_Buckets - 1;
  గుర్తులేని I = పాత్ర, J = I;
  అయితే ((J = (J + 1) & ముసుగు), The_పట్టిక[J]) {
    గుర్తులేని Ideal = Hash_పట్టిక[J];
    ఒకవేళ (((I - Ideal) & ముసుగు) < ((J - Ideal) & ముసుగు)) {
      The_పట్టిక[I] = The_పట్టిక[J];
      Hash_పట్టిక[I] = Hash_పట్టిక[J];
      I = J;
    }
  }
  The_పట్టిక[I] = శూన్యసూచిక;
  --అంకె_వస్తువులు;
}

String_Map_నమోదు_పునాది *StringMapImpl::Remove_కీలకం(StringRef కీలకం) {
  పూర్ణాంకం పాత్ర = కనుగొను_కీలకం(కీలకం);
  ఒకవేళ (పాత్ర == -1)
    తిరిగి శూన్యసూచిక;

  String_Map_నమోదు_పునాది *ఫలితం = The_పట్టిక[పాత్ర];
  remove_పాత్ర(పాత్ర);
  తిరిగి ఫలితం;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
గుర్తులేని StringMapImpl::Rehash_పట్టిక(గుర్తులేని పాత్ర_No) {
  గుర్తులేని New_కొలత;
  // If the hash table is now more than 3/4 full, grow the table.
  ఒకవేళ (LLVM_UNLIKELY(అంకె_వస్తువులు * 4 > అంకె_Buckets * 3)) {
    New_కొలత = అంకె_Buckets * 2;
  } లేకపోతే {
    తిరిగి పాత్ర_No;
  }

  గుర్తులేని New_పాత్ర_No = పాత్ర_No;
  స్వయంచాలక **New_పట్టిక_Array = create_పట్టిక(New_కొలత);
  గుర్తులేని *NewHashArray = get_Hash_పట్టిక(New_పట్టిక_Array, New_కొలత);
  గుర్తులేని *Hash_పట్టిక = get_Hash_పట్టిక(The_పట్టిక, అంకె_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  కొరకు (గుర్తులేని I = 0, E = అంకె_Buckets; I != E; ++I) {
    String_Map_నమోదు_పునాది *పాత్ర = The_పట్టిక[I];
    ఒకవేళ (పాత్ర) {
      // If the bucket is not available, probe for a spot.
      గుర్తులేని నిండిన_Hash = Hash_పట్టిక[I];
      గుర్తులేని New_పాత్ర = నిండిన_Hash & (New_కొలత - 1);
      అయితే (New_పట్టిక_Array[New_పాత్ర])
        New_పాత్ర = (New_పాత్ర + 1) & (New_కొలత - 1);

      // Finally found a slot.  Fill it in.
      New_పట్టిక_Array[New_పాత్ర] = పాత్ర;
      NewHashArray[New_పాత్ర] = నిండిన_Hash;
      ఒకవేళ (I == పాత్ర_No)
        New_పాత్ర_No = New_పాత్ర;
    }
  }

  విడుదల(The_పట్టిక);

  The_పట్టిక = New_పట్టిక_Array;
  అంకె_Buckets = New_కొలత;
  తిరిగి New_పాత్ర_No;
}
