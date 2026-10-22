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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ReverseIteration.h"

using ನಾಮಾಕಾಶ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ unsigned getMinBucketToReserveForEntries(unsigned ಅಂಕೆ_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ಯದಿ (ಅಂಕೆ_Entries == 0)
    ಪ್ರತ್ಯಾಗಮ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ಪ್ರತ್ಯಾಗಮ NextPowerOf2(ಅಂಕೆ_Entries * 4 / 3 + 1);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ StringMapEntryBase **createTable(unsigned New_ಅಂಕೆ_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_ಅಂಕೆ_Buckets + 1, ಪರಿಮಾಣ(StringMapEntryBase **) + ಪರಿಮಾಣ(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_ಅಂಕೆ_Buckets] = (StringMapEntryBase *)2;
  ಪ್ರತ್ಯಾಗಮ Table;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned ಅಂಕೆ_Buckets) {
  ಪ್ರತ್ಯಾಗಮ reinterpret_cast<unsigned *>(TheTable + ಅಂಕೆ_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ಕೀಲಿ) { ಪ್ರತ್ಯಾಗಮ xxh3_64bits(ಕೀಲಿ); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  ಯದಿ (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

ಶೂನ್ಯ StringMapImpl::init(unsigned InitSize) {
  ಪ್ರತಿಜ್ಞಾ((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_ಅಂಕೆ_Buckets = InitSize ? InitSize : 16;
  ಅಂಕೆ_ವಸ್ತುಗಳು = 0;

  TheTable = createTable(New_ಅಂಕೆ_Buckets);

  // Set the member only if TheTable was successfully allocated
  ಅಂಕೆ_Buckets = New_ಅಂಕೆ_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_ಮೌಲ್ಯ) {
#ifdef EXPENSIVE_CHECKS
  ಪ್ರತಿಜ್ಞಾ(Full_Hash_ಮೌಲ್ಯ == hash(Name));
#endif
  // Hash table unallocated so far?
  ಯದಿ (ಅಂಕೆ_Buckets == 0)
    init(16);
  ಯದಿ constexpr (shouldReverseIterate())
    Full_Hash_ಮೌಲ್ಯ = ~Full_Hash_ಮೌಲ್ಯ;
  unsigned BucketNo = Full_Hash_ಮೌಲ್ಯ & (ಅಂಕೆ_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, ಅಂಕೆ_Buckets);

  ಯಾವತ್ (ಸತ್ಯ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ಯದಿ (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_ಮೌಲ್ಯ;
      ಪ್ರತ್ಯಾಗಮ BucketNo;
    }

    ಯದಿ (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_ಮೌಲ್ಯ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      ವರ್ಣ *ItemStr = (ವರ್ಣ *)BucketItem + ItemSize;
      ಯದಿ (Name == StringRef(ItemStr, BucketItem->get_ಕೀಲಿ_Length())) {
        // We found a match!
        ಪ್ರತ್ಯಾಗಮ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (ಅಂಕೆ_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
ಪೂರ್ಣಾಙ್ಕ StringMapImpl::Find_ಕೀಲಿ(StringRef ಕೀಲಿ, uint32_t Full_Hash_ಮೌಲ್ಯ) ಧ್ರುವ {
  ಯದಿ (ಅಂಕೆ_Buckets == 0)
    ಪ್ರತ್ಯಾಗಮ -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  ಪ್ರತಿಜ್ಞಾ(Full_Hash_ಮೌಲ್ಯ == hash(ಕೀಲಿ));
#endif
  ಯದಿ constexpr (shouldReverseIterate())
    Full_Hash_ಮೌಲ್ಯ = ~Full_Hash_ಮೌಲ್ಯ;
  unsigned BucketNo = Full_Hash_ಮೌಲ್ಯ & (ಅಂಕೆ_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, ಅಂಕೆ_Buckets);

  ಯಾವತ್ (ಸತ್ಯ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ಯದಿ (LLVM_LIKELY(!BucketItem))
      ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_ಮೌಲ್ಯ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      ವರ್ಣ *ItemStr = (ವರ್ಣ *)BucketItem + ItemSize;
      ಯದಿ (ಕೀಲಿ == StringRef(ItemStr, BucketItem->get_ಕೀಲಿ_Length())) {
        // We found a match!
        ಪ್ರತ್ಯಾಗಮ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (ಅಂಕೆ_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
ಶೂನ್ಯ StringMapImpl::Remove_ಕೀಲಿ(StringMapEntryBase *V) {
  ಧ್ರುವ ವರ್ಣ *VStr = (ವರ್ಣ *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_ಕೀಲಿ(StringRef(VStr, V->get_ಕೀಲಿ_Length()));
  (ಶೂನ್ಯ)V2;
  ಪ್ರತಿಜ್ಞಾ(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
ಶೂನ್ಯ StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, ಅಂಕೆ_Buckets);
  unsigned Mask = ಅಂಕೆ_Buckets - 1;
  unsigned I = Bucket, J = I;
  ಯಾವತ್ ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    ಯದಿ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --ಅಂಕೆ_ವಸ್ತುಗಳು;
}

StringMapEntryBase *StringMapImpl::Remove_ಕೀಲಿ(StringRef ಕೀಲಿ) {
  ಪೂರ್ಣಾಙ್ಕ Bucket = Find_ಕೀಲಿ(ಕೀಲಿ);
  ಯದಿ (Bucket == -1)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  ಪ್ರತ್ಯಾಗಮ Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  ಯದಿ (LLVM_UNLIKELY(ಅಂಕೆ_ವಸ್ತುಗಳು * 4 > ಅಂಕೆ_Buckets * 3)) {
    NewSize = ಅಂಕೆ_Buckets * 2;
  } ಅನ್ಯಥಾ {
    ಪ್ರತ್ಯಾಗಮ BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, ಅಂಕೆ_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  ಪ್ರತಿ (unsigned I = 0, E = ಅಂಕೆ_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    ಯದಿ (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      ಯಾವತ್ (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      ಯದಿ (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  ಮುಕ್ತ(TheTable);

  TheTable = NewTableArray;
  ಅಂಕೆ_Buckets = NewSize;
  ಪ್ರತ್ಯಾಗಮ NewBucketNo;
}
