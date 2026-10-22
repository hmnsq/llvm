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

#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/ReverseIteration.h"

using فضاینام llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ایستا درونخطی unsigned getMinBucketToReserveForEntries(unsigned رقم_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  اگر (رقم_Entries == 0)
    بازگشت 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  بازگشت NextPowerOf2(رقم_Entries * 4 / 3 + 1);
}

ایستا درونخطی StringMapEntryBase **createTable(unsigned New_رقم_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_رقم_Buckets + 1, اندازه(StringMapEntryBase **) + اندازه(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_رقم_Buckets] = (StringMapEntryBase *)2;
  بازگشت Table;
}

ایستا درونخطی unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned رقم_Buckets) {
  بازگشت reinterpret_cast<unsigned *>(TheTable + رقم_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef کلید) { بازگشت xxh3_64bits(کلید); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  اگر (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

تهی StringMapImpl::init(unsigned InitSize) {
  أكد((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_رقم_Buckets = InitSize ? InitSize : 16;
  رقم_اقلام = 0;

  TheTable = createTable(New_رقم_Buckets);

  // Set the member only if TheTable was successfully allocated
  رقم_Buckets = New_رقم_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_مقدار) {
#ifdef EXPENSIVE_CHECKS
  أكد(Full_Hash_مقدار == hash(Name));
#endif
  // Hash table unallocated so far?
  اگر (رقم_Buckets == 0)
    init(16);
  اگر constexpr (shouldReverseIterate())
    Full_Hash_مقدار = ~Full_Hash_مقدار;
  unsigned BucketNo = Full_Hash_مقدار & (رقم_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, رقم_Buckets);

  وقتی (درست) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    اگر (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_مقدار;
      بازگشت BucketNo;
    }

    اگر (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_مقدار)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      نویسه *ItemStr = (نویسه *)BucketItem + ItemSize;
      اگر (Name == StringRef(ItemStr, BucketItem->get_کلید_Length())) {
        // We found a match!
        بازگشت BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (رقم_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحیح StringMapImpl::Find_کلید(StringRef کلید, uint32_t Full_Hash_مقدار) ثابت {
  اگر (رقم_Buckets == 0)
    بازگشت -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  أكد(Full_Hash_مقدار == hash(کلید));
#endif
  اگر constexpr (shouldReverseIterate())
    Full_Hash_مقدار = ~Full_Hash_مقدار;
  unsigned BucketNo = Full_Hash_مقدار & (رقم_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, رقم_Buckets);

  وقتی (درست) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    اگر (LLVM_LIKELY(!BucketItem))
      بازگشت -1;

    اگر (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_مقدار)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      نویسه *ItemStr = (نویسه *)BucketItem + ItemSize;
      اگر (کلید == StringRef(ItemStr, BucketItem->get_کلید_Length())) {
        // We found a match!
        بازگشت BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (رقم_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
تهی StringMapImpl::Remove_کلید(StringMapEntryBase *V) {
  ثابت نویسه *VStr = (نویسه *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_کلید(StringRef(VStr, V->get_کلید_Length()));
  (تهی)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
تهی StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, رقم_Buckets);
  unsigned Mask = رقم_Buckets - 1;
  unsigned I = Bucket, J = I;
  وقتی ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    اگر (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --رقم_اقلام;
}

StringMapEntryBase *StringMapImpl::Remove_کلید(StringRef کلید) {
  صحیح Bucket = Find_کلید(کلید);
  اگر (Bucket == -1)
    بازگشت nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  بازگشت Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  اگر (LLVM_UNLIKELY(رقم_اقلام * 4 > رقم_Buckets * 3)) {
    NewSize = رقم_Buckets * 2;
  } وگرنه {
    بازگشت BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, رقم_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  برای (unsigned I = 0, E = رقم_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    اگر (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      وقتی (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      اگر (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  آزاد(TheTable);

  TheTable = NewTableArray;
  رقم_Buckets = NewSize;
  بازگشت NewBucketNo;
}
