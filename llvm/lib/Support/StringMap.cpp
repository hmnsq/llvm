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
ساكن مضمن unsigned getMinBucketToReserveForEntries(unsigned رەقەم_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  إذا (رەقەم_Entries == 0)
    أرجع 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  أرجع NextPowerOf2(رەقەم_Entries * 4 / 3 + 1);
}

ساكن مضمن StringMapEntryBase **createTable(unsigned New_رەقەم_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_رەقەم_Buckets + 1, حجم(StringMapEntryBase **) + حجم(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_رەقەم_Buckets] = (StringMapEntryBase *)2;
  أرجع Table;
}

ساكن مضمن unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned رەقەم_Buckets) {
  أرجع reinterpret_cast<unsigned *>(TheTable + رەقەم_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ئاچقۇچ) { أرجع xxh3_64bits(ئاچقۇچ); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  إذا (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

فراغ StringMapImpl::init(unsigned InitSize) {
  أكد((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_رەقەم_Buckets = InitSize ? InitSize : 16;
  رەقەم_بۇيۇملار = 0;

  TheTable = createTable(New_رەقەم_Buckets);

  // Set the member only if TheTable was successfully allocated
  رەقەم_Buckets = New_رەقەم_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_قىممەت) {
#ifdef EXPENSIVE_CHECKS
  أكد(Full_Hash_قىممەت == hash(Name));
#endif
  // Hash table unallocated so far?
  إذا (رەقەم_Buckets == 0)
    init(16);
  إذا constexpr (shouldReverseIterate())
    Full_Hash_قىممەت = ~Full_Hash_قىممەت;
  unsigned BucketNo = Full_Hash_قىممەت & (رەقەم_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, رەقەم_Buckets);

  طالما (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    إذا (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_قىممەت;
      أرجع BucketNo;
    }

    إذا (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_قىممەت)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *ItemStr = (حرف *)BucketItem + ItemSize;
      إذا (Name == StringRef(ItemStr, BucketItem->get_ئاچقۇچ_Length())) {
        // We found a match!
        أرجع BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (رەقەم_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحيح StringMapImpl::Find_ئاچقۇچ(StringRef ئاچقۇچ, uint32_t Full_Hash_قىممەت) ثابت {
  إذا (رەقەم_Buckets == 0)
    أرجع -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  أكد(Full_Hash_قىممەت == hash(ئاچقۇچ));
#endif
  إذا constexpr (shouldReverseIterate())
    Full_Hash_قىممەت = ~Full_Hash_قىممەت;
  unsigned BucketNo = Full_Hash_قىممەت & (رەقەم_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, رەقەم_Buckets);

  طالما (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    إذا (LLVM_LIKELY(!BucketItem))
      أرجع -1;

    إذا (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_قىممەت)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *ItemStr = (حرف *)BucketItem + ItemSize;
      إذا (ئاچقۇچ == StringRef(ItemStr, BucketItem->get_ئاچقۇچ_Length())) {
        // We found a match!
        أرجع BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (رەقەم_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
فراغ StringMapImpl::Remove_ئاچقۇچ(StringMapEntryBase *V) {
  ثابت حرف *VStr = (حرف *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_ئاچقۇچ(StringRef(VStr, V->get_ئاچقۇچ_Length()));
  (فراغ)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
فراغ StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, رەقەم_Buckets);
  unsigned Mask = رەقەم_Buckets - 1;
  unsigned I = Bucket, J = I;
  طالما ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    إذا (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --رەقەم_بۇيۇملار;
}

StringMapEntryBase *StringMapImpl::Remove_ئاچقۇچ(StringRef ئاچقۇچ) {
  صحيح Bucket = Find_ئاچقۇچ(ئاچقۇچ);
  إذا (Bucket == -1)
    أرجع nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  أرجع Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  إذا (LLVM_UNLIKELY(رەقەم_بۇيۇملار * 4 > رەقەم_Buckets * 3)) {
    NewSize = رەقەم_Buckets * 2;
  } وإلا {
    أرجع BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, رەقەم_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  لكل (unsigned I = 0, E = رەقەم_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    إذا (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      طالما (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      إذا (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  حرر(TheTable);

  TheTable = NewTableArray;
  رەقەم_Buckets = NewSize;
  أرجع NewBucketNo;
}
