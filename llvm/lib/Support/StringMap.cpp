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

#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/ReverseIteration.h"

using نطاق llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ساكن مضمن unsigned getMinBucketToReserveForEntries(unsigned NumEntries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ܐܢ (NumEntries == 0)
    ܦܢܐ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ܦܢܐ NextPowerOf2(NumEntries * 4 / 3 + 1);
}

ساكن مضمن StringMapEntryBase **createTable(unsigned NewNumBuckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      NewNumBuckets + 1, حجم(StringMapEntryBase **) + حجم(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[NewNumBuckets] = (StringMapEntryBase *)2;
  ܦܢܐ Table;
}

ساكن مضمن unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned NumBuckets) {
  ܦܢܐ reinterpret_cast<unsigned *>(TheTable + NumBuckets + 1);
}

uint32_t StringMapImpl::hash(StringRef Key) { ܦܢܐ xxh3_64bits(Key); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  ܐܢ (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

فراغ StringMapImpl::init(unsigned InitSize) {
  أكد((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned NewNumBuckets = InitSize ? InitSize : 16;
  NumItems = 0;

  TheTable = createTable(NewNumBuckets);

  // Set the member only if TheTable was successfully allocated
  NumBuckets = NewNumBuckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t FullHashValue) {
#ifdef EXPENSIVE_CHECKS
  أكد(FullHashValue == hash(Name));
#endif
  // Hash table unallocated so far?
  ܐܢ (NumBuckets == 0)
    init(16);
  ܐܢ constexpr (shouldReverseIterate())
    FullHashValue = ~FullHashValue;
  unsigned BucketNo = FullHashValue & (NumBuckets - 1);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  ܟܡܐ (ܫܪܪܐ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ܐܢ (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = FullHashValue;
      ܦܢܐ BucketNo;
    }

    ܐܢ (LLVM_LIKELY(HashTable[BucketNo] == FullHashValue)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *ItemStr = (حرف *)BucketItem + ItemSize;
      ܐܢ (Name == StringRef(ItemStr, BucketItem->getKeyLength())) {
        // We found a match!
        ܦܢܐ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (NumBuckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحيح StringMapImpl::FindKey(StringRef Key, uint32_t FullHashValue) ثابت {
  ܐܢ (NumBuckets == 0)
    ܦܢܐ -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  أكد(FullHashValue == hash(Key));
#endif
  ܐܢ constexpr (shouldReverseIterate())
    FullHashValue = ~FullHashValue;
  unsigned BucketNo = FullHashValue & (NumBuckets - 1);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  ܟܡܐ (ܫܪܪܐ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ܐܢ (LLVM_LIKELY(!BucketItem))
      ܦܢܐ -1;

    ܐܢ (LLVM_LIKELY(HashTable[BucketNo] == FullHashValue)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *ItemStr = (حرف *)BucketItem + ItemSize;
      ܐܢ (Key == StringRef(ItemStr, BucketItem->getKeyLength())) {
        // We found a match!
        ܦܢܐ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (NumBuckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
فراغ StringMapImpl::RemoveKey(StringMapEntryBase *V) {
  ثابت حرف *VStr = (حرف *)V + ItemSize;
  StringMapEntryBase *V2 = RemoveKey(StringRef(VStr, V->getKeyLength()));
  (فراغ)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
فراغ StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);
  unsigned Mask = NumBuckets - 1;
  unsigned I = Bucket, J = I;
  ܟܡܐ ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    ܐܢ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --NumItems;
}

StringMapEntryBase *StringMapImpl::RemoveKey(StringRef Key) {
  صحيح Bucket = FindKey(Key);
  ܐܢ (Bucket == -1)
    ܦܢܐ nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  ܦܢܐ Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  ܐܢ (LLVM_UNLIKELY(NumItems * 4 > NumBuckets * 3)) {
    NewSize = NumBuckets * 2;
  } ܐܚܪܢܐ {
    ܦܢܐ BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  لكل (unsigned I = 0, E = NumBuckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    ܐܢ (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      ܟܡܐ (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      ܐܢ (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  حرر(TheTable);

  TheTable = NewTableArray;
  NumBuckets = NewSize;
  ܦܢܐ NewBucketNo;
}
