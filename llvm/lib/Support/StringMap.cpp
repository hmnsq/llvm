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
स्थिर अन्तःस्थ अचिन्हित getMinBucketToReserveForEntries(अचिन्हित अङ्क_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  यदि (अङ्क_Entries == 0)
    फर्काऊ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  फर्काऊ NextPowerOf2(अङ्क_Entries * 4 / 3 + 1);
}

स्थिर अन्तःस्थ StringMapEntryBase **createTable(अचिन्हित New_अङ्क_Buckets) {
  स्वतः **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_अङ्क_Buckets + 1, परिमाण(StringMapEntryBase **) + परिमाण(अचिन्हित)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_अङ्क_Buckets] = (StringMapEntryBase *)2;
  फर्काऊ Table;
}

स्थिर अन्तःस्थ अचिन्हित *getHashTable(StringMapEntryBase **TheTable,
                                     अचिन्हित अङ्क_Buckets) {
  फर्काऊ reinterpret_cast<अचिन्हित *>(TheTable + अङ्क_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef साँचो) { फर्काऊ xxh3_64bits(साँचो); }

StringMapImpl::StringMapImpl(अचिन्हित InitSize, अचिन्हित itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  यदि (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

शून्य StringMapImpl::init(अचिन्हित InitSize) {
  प्रतिज्ञा((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  अचिन्हित New_अङ्क_Buckets = InitSize ? InitSize : 16;
  अङ्क_वस्तुहरू = 0;

  TheTable = createTable(New_अङ्क_Buckets);

  // Set the member only if TheTable was successfully allocated
  अङ्क_Buckets = New_अङ्क_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
अचिन्हित StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_मान) {
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मान == hash(Name));
#endif
  // Hash table unallocated so far?
  यदि (अङ्क_Buckets == 0)
    init(16);
  यदि constexpr (shouldReverseIterate())
    Full_Hash_मान = ~Full_Hash_मान;
  अचिन्हित BucketNo = Full_Hash_मान & (अङ्क_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अङ्क_Buckets);

  जबसम्म (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    यदि (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_मान;
      फर्काऊ BucketNo;
    }

    यदि (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मान)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      यदि (Name == StringRef(ItemStr, BucketItem->get_साँचो_Length())) {
        // We found a match!
        फर्काऊ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्क_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णाङ्क StringMapImpl::Find_साँचो(StringRef साँचो, uint32_t Full_Hash_मान) अचर {
  यदि (अङ्क_Buckets == 0)
    फर्काऊ -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मान == hash(साँचो));
#endif
  यदि constexpr (shouldReverseIterate())
    Full_Hash_मान = ~Full_Hash_मान;
  अचिन्हित BucketNo = Full_Hash_मान & (अङ्क_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अङ्क_Buckets);

  जबसम्म (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    यदि (LLVM_LIKELY(!BucketItem))
      फर्काऊ -1;

    यदि (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मान)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      यदि (साँचो == StringRef(ItemStr, BucketItem->get_साँचो_Length())) {
        // We found a match!
        फर्काऊ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्क_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
शून्य StringMapImpl::Remove_साँचो(StringMapEntryBase *V) {
  अचर वर्ण *VStr = (वर्ण *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_साँचो(StringRef(VStr, V->get_साँचो_Length()));
  (शून्य)V2;
  प्रतिज्ञा(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
शून्य StringMapImpl::removeBucket(अचिन्हित Bucket) {
  अचिन्हित *HashTable = getHashTable(TheTable, अङ्क_Buckets);
  अचिन्हित Mask = अङ्क_Buckets - 1;
  अचिन्हित I = Bucket, J = I;
  जबसम्म ((J = (J + 1) & Mask), TheTable[J]) {
    अचिन्हित Ideal = HashTable[J];
    यदि (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --अङ्क_वस्तुहरू;
}

StringMapEntryBase *StringMapImpl::Remove_साँचो(StringRef साँचो) {
  पूर्णाङ्क Bucket = Find_साँचो(साँचो);
  यदि (Bucket == -1)
    फर्काऊ nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  फर्काऊ Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
अचिन्हित StringMapImpl::RehashTable(अचिन्हित BucketNo) {
  अचिन्हित NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  यदि (LLVM_UNLIKELY(अङ्क_वस्तुहरू * 4 > अङ्क_Buckets * 3)) {
    NewSize = अङ्क_Buckets * 2;
  } अन्यथा {
    फर्काऊ BucketNo;
  }

  अचिन्हित NewBucketNo = BucketNo;
  स्वतः **NewTableArray = createTable(NewSize);
  अचिन्हित *NewHashArray = getHashTable(NewTableArray, NewSize);
  अचिन्हित *HashTable = getHashTable(TheTable, अङ्क_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  लागि (अचिन्हित I = 0, E = अङ्क_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    यदि (Bucket) {
      // If the bucket is not available, probe for a spot.
      अचिन्हित FullHash = HashTable[I];
      अचिन्हित NewBucket = FullHash & (NewSize - 1);
      जबसम्म (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      यदि (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  मुक्त(TheTable);

  TheTable = NewTableArray;
  अङ्क_Buckets = NewSize;
  फर्काऊ NewBucketNo;
}
