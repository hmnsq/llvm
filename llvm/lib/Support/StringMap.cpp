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

#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/ReverseIteration.h"

using नामाकाश llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
स्थिर अन्तःस्थ unsigned getMinBucketToReserveForEntries(unsigned अङ्कः_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  सचे (अङ्कः_Entries == 0)
    पटिनिवत्त 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  पटिनिवत्त NextPowerOf2(अङ्कः_Entries * 4 / 3 + 1);
}

स्थिर अन्तःस्थ StringMapEntryBase **createTable(unsigned New_अङ्कः_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_अङ्कः_Buckets + 1, परिमाण(StringMapEntryBase **) + परिमाण(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_अङ्कः_Buckets] = (StringMapEntryBase *)2;
  पटिनिवत्त Table;
}

स्थिर अन्तःस्थ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned अङ्कः_Buckets) {
  पटिनिवत्त reinterpret_cast<unsigned *>(TheTable + अङ्कः_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef कुञ्चिका) { पटिनिवत्त xxh3_64bits(कुञ्चिका); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  सचे (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

शून्य StringMapImpl::init(unsigned InitSize) {
  प्रतिज्ञा((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_अङ्कः_Buckets = InitSize ? InitSize : 16;
  अङ्कः_वस्तूनि = 0;

  TheTable = createTable(New_अङ्कः_Buckets);

  // Set the member only if TheTable was successfully allocated
  अङ्कः_Buckets = New_अङ्कः_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_मानम्) {
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मानम् == hash(Name));
#endif
  // Hash table unallocated so far?
  सचे (अङ्कः_Buckets == 0)
    init(16);
  सचे constexpr (shouldReverseIterate())
    Full_Hash_मानम् = ~Full_Hash_मानम्;
  unsigned BucketNo = Full_Hash_मानम् & (अङ्कः_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, अङ्कः_Buckets);

  याव (सच्च) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    सचे (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_मानम्;
      पटिनिवत्त BucketNo;
    }

    सचे (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मानम्)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      सचे (Name == StringRef(ItemStr, BucketItem->get_कुञ्चिका_Length())) {
        // We found a match!
        पटिनिवत्त BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्कः_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णाङ्क StringMapImpl::Find_कुञ्चिका(StringRef कुञ्चिका, uint32_t Full_Hash_मानम्) ध्रुव {
  सचे (अङ्कः_Buckets == 0)
    पटिनिवत्त -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मानम् == hash(कुञ्चिका));
#endif
  सचे constexpr (shouldReverseIterate())
    Full_Hash_मानम् = ~Full_Hash_मानम्;
  unsigned BucketNo = Full_Hash_मानम् & (अङ्कः_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, अङ्कः_Buckets);

  याव (सच्च) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    सचे (LLVM_LIKELY(!BucketItem))
      पटिनिवत्त -1;

    सचे (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मानम्)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      सचे (कुञ्चिका == StringRef(ItemStr, BucketItem->get_कुञ्चिका_Length())) {
        // We found a match!
        पटिनिवत्त BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्कः_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
शून्य StringMapImpl::Remove_कुञ्चिका(StringMapEntryBase *V) {
  ध्रुव वर्ण *VStr = (वर्ण *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_कुञ्चिका(StringRef(VStr, V->get_कुञ्चिका_Length()));
  (शून्य)V2;
  प्रतिज्ञा(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
शून्य StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, अङ्कः_Buckets);
  unsigned Mask = अङ्कः_Buckets - 1;
  unsigned I = Bucket, J = I;
  याव ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    सचे (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --अङ्कः_वस्तूनि;
}

StringMapEntryBase *StringMapImpl::Remove_कुञ्चिका(StringRef कुञ्चिका) {
  पूर्णाङ्क Bucket = Find_कुञ्चिका(कुञ्चिका);
  सचे (Bucket == -1)
    पटिनिवत्त nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  पटिनिवत्त Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  सचे (LLVM_UNLIKELY(अङ्कः_वस्तूनि * 4 > अङ्कः_Buckets * 3)) {
    NewSize = अङ्कः_Buckets * 2;
  } अञ्ञथा {
    पटिनिवत्त BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, अङ्कः_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  अत्थाय (unsigned I = 0, E = अङ्कः_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    सचे (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      याव (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      सचे (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  मुक्त(TheTable);

  TheTable = NewTableArray;
  अङ्कः_Buckets = NewSize;
  पटिनिवत्त NewBucketNo;
}
