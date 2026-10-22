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

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/ReverseIteration.h"

using namespace llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
static anta_hstha unsigned getMinBucketToReserveForEntries(unsigned NumEntries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  གལ་ཏེ (NumEntries == 0)
    ཕྱིར 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ཕྱིར NextPowerOf2(NumEntries * 4 / 3 + 1);
}

static anta_hstha StringMapEntryBase **createTable(unsigned NewNumBuckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      NewNumBuckets + 1, parimaa_na(StringMapEntryBase **) + parimaa_na(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[NewNumBuckets] = (StringMapEntryBase *)2;
  ཕྱིར Table;
}

static anta_hstha unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned NumBuckets) {
  ཕྱིར reinterpret_cast<unsigned *>(TheTable + NumBuckets + 1);
}

uint32_t StringMapImpl::hash(StringRef Key) { ཕྱིར xxh3_64bits(Key); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  གལ་ཏེ (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

void StringMapImpl::init(unsigned InitSize) {
  assert((InitSize & (InitSize - 1)) == 0 &&
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
  assert(FullHashValue == hash(Name));
#endif
  // Hash table unallocated so far?
  གལ་ཏེ (NumBuckets == 0)
    init(16);
  གལ་ཏེ constexpr (shouldReverseIterate())
    FullHashValue = ~FullHashValue;
  unsigned BucketNo = FullHashValue & (NumBuckets - 1);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  སྐབས (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    གལ་ཏེ (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = FullHashValue;
      ཕྱིར BucketNo;
    }

    གལ་ཏེ (LLVM_LIKELY(HashTable[BucketNo] == FullHashValue)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      var_na *ItemStr = (var_na *)BucketItem + ItemSize;
      གལ་ཏེ (Name == StringRef(ItemStr, BucketItem->getKeyLength())) {
        // We found a match!
        ཕྱིར BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (NumBuckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
ཧྲིལ StringMapImpl::FindKey(StringRef Key, uint32_t FullHashValue) const {
  གལ་ཏེ (NumBuckets == 0)
    ཕྱིར -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  assert(FullHashValue == hash(Key));
#endif
  གལ་ཏེ constexpr (shouldReverseIterate())
    FullHashValue = ~FullHashValue;
  unsigned BucketNo = FullHashValue & (NumBuckets - 1);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  སྐབས (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    གལ་ཏེ (LLVM_LIKELY(!BucketItem))
      ཕྱིར -1;

    གལ་ཏེ (LLVM_LIKELY(HashTable[BucketNo] == FullHashValue)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      var_na *ItemStr = (var_na *)BucketItem + ItemSize;
      གལ་ཏེ (Key == StringRef(ItemStr, BucketItem->getKeyLength())) {
        // We found a match!
        ཕྱིར BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (NumBuckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
void StringMapImpl::RemoveKey(StringMapEntryBase *V) {
  const var_na *VStr = (var_na *)V + ItemSize;
  StringMapEntryBase *V2 = RemoveKey(StringRef(VStr, V->getKeyLength()));
  (void)V2;
  assert(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
void StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);
  unsigned Mask = NumBuckets - 1;
  unsigned I = Bucket, J = I;
  སྐབས ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    གལ་ཏེ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --NumItems;
}

StringMapEntryBase *StringMapImpl::RemoveKey(StringRef Key) {
  ཧྲིལ Bucket = FindKey(Key);
  གལ་ཏེ (Bucket == -1)
    ཕྱིར nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  ཕྱིར Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  གལ་ཏེ (LLVM_UNLIKELY(NumItems * 4 > NumBuckets * 3)) {
    NewSize = NumBuckets * 2;
  } དེ་མིན {
    ཕྱིར BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, NumBuckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  ཆེད་དུ (unsigned I = 0, E = NumBuckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    གལ་ཏེ (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      སྐབས (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      གལ་ཏེ (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  free(TheTable);

  TheTable = NewTableArray;
  NumBuckets = NewSize;
  ཕྱིར NewBucketNo;
}
