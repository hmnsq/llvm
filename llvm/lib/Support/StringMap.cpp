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
स्थिर अन्तःस्थ अचिन्हित getMinBucketToReserveForEntries(अचिन्हित अंक_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  यदि (अंक_Entries == 0)
    प्रत्यागम 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  प्रत्यागम NextPowerOf2(अंक_Entries * 4 / 3 + 1);
}

स्थिर अन्तःस्थ StringMapEntryBase **createTable(अचिन्हित New_अंक_Buckets) {
  स्वतः **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_अंक_Buckets + 1, परिमाण(StringMapEntryBase **) + परिमाण(अचिन्हित)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_अंक_Buckets] = (StringMapEntryBase *)2;
  प्रत्यागम Table;
}

स्थिर अन्तःस्थ अचिन्हित *getHashTable(StringMapEntryBase **TheTable,
                                     अचिन्हित अंक_Buckets) {
  प्रत्यागम reinterpret_cast<अचिन्हित *>(TheTable + अंक_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef कुंजी) { प्रत्यागम xxh3_64bits(कुंजी); }

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

  अचिन्हित New_अंक_Buckets = InitSize ? InitSize : 16;
  अंक_वस्तुएँ = 0;

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
  यदि (अंक_Buckets == 0)
    init(16);
  यदि constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  यावत् (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    यदि (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_मूल्य;
      प्रत्यागम BucketNo;
    }

    यदि (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      यदि (Name == StringRef(ItemStr, BucketItem->get_कुंजी_Length())) {
        // We found a match!
        प्रत्यागम BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णाङ्क StringMapImpl::Find_कुंजी(StringRef कुंजी, uint32_t Full_Hash_मूल्य) ध्रुव {
  यदि (अंक_Buckets == 0)
    प्रत्यागम -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  प्रतिज्ञा(Full_Hash_मूल्य == hash(कुंजी));
#endif
  यदि constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  यावत् (सत्य) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    यदि (LLVM_LIKELY(!BucketItem))
      प्रत्यागम -1;

    यदि (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + ItemSize;
      यदि (कुंजी == StringRef(ItemStr, BucketItem->get_कुंजी_Length())) {
        // We found a match!
        प्रत्यागम BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
शून्य StringMapImpl::Remove_कुंजी(StringMapEntryBase *V) {
  ध्रुव वर्ण *VStr = (वर्ण *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_कुंजी(StringRef(VStr, V->get_कुंजी_Length()));
  (शून्य)V2;
  प्रतिज्ञा(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
शून्य StringMapImpl::removeBucket(अचिन्हित Bucket) {
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);
  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Bucket, J = I;
  यावत् ((J = (J + 1) & Mask), TheTable[J]) {
    अचिन्हित Ideal = HashTable[J];
    यदि (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --अंक_वस्तुएँ;
}

StringMapEntryBase *StringMapImpl::Remove_कुंजी(StringRef कुंजी) {
  पूर्णाङ्क Bucket = Find_कुंजी(कुंजी);
  यदि (Bucket == -1)
    प्रत्यागम nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  प्रत्यागम Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
अचिन्हित StringMapImpl::RehashTable(अचिन्हित BucketNo) {
  अचिन्हित NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  यदि (LLVM_UNLIKELY(अंक_वस्तुएँ * 4 > अंक_Buckets * 3)) {
    NewSize = अंक_Buckets * 2;
  } अन्यथा {
    प्रत्यागम BucketNo;
  }

  अचिन्हित NewBucketNo = BucketNo;
  स्वतः **NewTableArray = createTable(NewSize);
  अचिन्हित *NewHashArray = getHashTable(NewTableArray, NewSize);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  प्रति (अचिन्हित I = 0, E = अंक_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    यदि (Bucket) {
      // If the bucket is not available, probe for a spot.
      अचिन्हित FullHash = HashTable[I];
      अचिन्हित NewBucket = FullHash & (NewSize - 1);
      यावत् (NewTableArray[NewBucket])
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
  अंक_Buckets = NewSize;
  प्रत्यागम NewBucketNo;
}
