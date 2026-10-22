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
स्थिर ओळीत अचिन्हित getMinBucketToReserveForEntries(अचिन्हित अंक_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  जर (अंक_Entries == 0)
    परत 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  परत NextPowerOf2(अंक_Entries * 4 / 3 + 1);
}

स्थिर ओळीत StringMapEntryBase **createTable(अचिन्हित New_अंक_Buckets) {
  स्वतः **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_अंक_Buckets + 1, आकार(StringMapEntryBase **) + आकार(अचिन्हित)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_अंक_Buckets] = (StringMapEntryBase *)2;
  परत Table;
}

स्थिर ओळीत अचिन्हित *getHashTable(StringMapEntryBase **TheTable,
                                     अचिन्हित अंक_Buckets) {
  परत reinterpret_cast<अचिन्हित *>(TheTable + अंक_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef किल्ली) { परत xxh3_64bits(किल्ली); }

StringMapImpl::StringMapImpl(अचिन्हित InitSize, अचिन्हित itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  जर (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

शून्य StringMapImpl::init(अचिन्हित InitSize) {
  प्रतिज्ञा((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  अचिन्हित New_अंक_Buckets = InitSize ? InitSize : 16;
  अंक_वस्तुसंच = 0;

  TheTable = createTable(New_अंक_Buckets);

  // Set the member only if TheTable was successfully allocated
  अंक_Buckets = New_अंक_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
अचिन्हित StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_मूल्य) {
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मूल्य == hash(Name));
#endif
  // Hash table unallocated so far?
  जर (अंक_Buckets == 0)
    init(16);
  जर constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  पर्यंत (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    जर (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_मूल्य;
      परत BucketNo;
    }

    जर (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      अक्षर *ItemStr = (अक्षर *)BucketItem + ItemSize;
      जर (Name == StringRef(ItemStr, BucketItem->get_किल्ली_Length())) {
        // We found a match!
        परत BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णांक StringMapImpl::Find_किल्ली(StringRef किल्ली, uint32_t Full_Hash_मूल्य) स्थिरांक {
  जर (अंक_Buckets == 0)
    परत -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मूल्य == hash(किल्ली));
#endif
  जर constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  पर्यंत (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    जर (LLVM_LIKELY(!BucketItem))
      परत -1;

    जर (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      अक्षर *ItemStr = (अक्षर *)BucketItem + ItemSize;
      जर (किल्ली == StringRef(ItemStr, BucketItem->get_किल्ली_Length())) {
        // We found a match!
        परत BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
शून्य StringMapImpl::Remove_किल्ली(StringMapEntryBase *V) {
  स्थिरांक अक्षर *VStr = (अक्षर *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_किल्ली(StringRef(VStr, V->get_किल्ली_Length()));
  (शून्य)V2;
  प्रतिज्ञा(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
शून्य StringMapImpl::removeBucket(अचिन्हित Bucket) {
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);
  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Bucket, J = I;
  पर्यंत ((J = (J + 1) & Mask), TheTable[J]) {
    अचिन्हित Ideal = HashTable[J];
    जर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --अंक_वस्तुसंच;
}

StringMapEntryBase *StringMapImpl::Remove_किल्ली(StringRef किल्ली) {
  पूर्णांक Bucket = Find_किल्ली(किल्ली);
  जर (Bucket == -1)
    परत nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  परत Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
अचिन्हित StringMapImpl::RehashTable(अचिन्हित BucketNo) {
  अचिन्हित NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  जर (LLVM_UNLIKELY(अंक_वस्तुसंच * 4 > अंक_Buckets * 3)) {
    NewSize = अंक_Buckets * 2;
  } अन्यथा {
    परत BucketNo;
  }

  अचिन्हित NewBucketNo = BucketNo;
  स्वतः **NewTableArray = createTable(NewSize);
  अचिन्हित *NewHashArray = getHashTable(NewTableArray, NewSize);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  साठी (अचिन्हित I = 0, E = अंक_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    जर (Bucket) {
      // If the bucket is not available, probe for a spot.
      अचिन्हित FullHash = HashTable[I];
      अचिन्हित NewBucket = FullHash & (NewSize - 1);
      पर्यंत (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      जर (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  मुक्त(TheTable);

  TheTable = NewTableArray;
  अंक_Buckets = NewSize;
  परत NewBucketNo;
}
