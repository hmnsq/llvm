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

#समावेश "llvm/ADT/StringMap.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/ReverseIteration.h"

using namespace llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
अटल अंतरभूत अचिन्हित getMinBucketToReserveForEntries(अचिन्हित अंक_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  अगर (अंक_Entries == 0)
    वापस 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  वापस अग्र_Power_Of2(अंक_Entries * 4 / 3 + 1);
}

अटल अंतरभूत StringMapEntryBase **createTable(अचिन्हित New_अंक_Buckets) {
  स्वतः **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_अंक_Buckets + 1, माप(StringMapEntryBase **) + माप(अचिन्हित)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_अंक_Buckets] = (StringMapEntryBase *)2;
  वापस Table;
}

अटल अंतरभूत अचिन्हित *getHashTable(StringMapEntryBase **TheTable,
                                     अचिन्हित अंक_Buckets) {
  वापस reinterpret_cast<अचिन्हित *>(TheTable + अंक_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef कुंजी) { वापस xxh3_64bits(कुंजी); }

StringMapImpl::StringMapImpl(अचिन्हित Init_आकार, अचिन्हित item_आकार)
    : Item_आकार(item_आकार) {
  // If a size is specified, initialize the table with that many buckets.
  अगर (Init_आकार) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(Init_आकार));
  }
}

व्योम StringMapImpl::init(अचिन्हित Init_आकार) {
  निश्चित((Init_आकार & (Init_आकार - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  अचिन्हित New_अंक_Buckets = Init_आकार ? Init_आकार : 16;
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
#अगर_घोषित EXPENSIVE_CHECKS
  निश्चित(Full_Hash_मूल्य == hash(Name));
#पूर्ण_अगर
  // Hash table unallocated so far?
  अगर (अंक_Buckets == 0)
    init(16);
  अगर constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  जबतक (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    अगर (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_मूल्य;
      वापस BucketNo;
    }

    अगर (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      अक्षर *ItemStr = (अक्षर *)BucketItem + Item_आकार;
      अगर (Name == StringRef(ItemStr, BucketItem->get_कुंजी_लंबाई())) {
        // We found a match!
        वापस BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णांक StringMapImpl::Find_कुंजी(StringRef कुंजी, uint32_t Full_Hash_मूल्य) स्थिर {
  अगर (अंक_Buckets == 0)
    वापस -1; // Really empty table?
#अगर_घोषित EXPENSIVE_CHECKS
  निश्चित(Full_Hash_मूल्य == hash(कुंजी));
#पूर्ण_अगर
  अगर constexpr (shouldReverseIterate())
    Full_Hash_मूल्य = ~Full_Hash_मूल्य;
  अचिन्हित BucketNo = Full_Hash_मूल्य & (अंक_Buckets - 1);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  जबतक (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    अगर (LLVM_LIKELY(!BucketItem))
      वापस -1;

    अगर (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_मूल्य)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      अक्षर *ItemStr = (अक्षर *)BucketItem + Item_आकार;
      अगर (कुंजी == StringRef(ItemStr, BucketItem->get_कुंजी_लंबाई())) {
        // We found a match!
        वापस BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अंक_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
व्योम StringMapImpl::Remove_कुंजी(StringMapEntryBase *V) {
  स्थिर अक्षर *VStr = (अक्षर *)V + Item_आकार;
  StringMapEntryBase *V2 = Remove_कुंजी(StringRef(VStr, V->get_कुंजी_लंबाई()));
  (व्योम)V2;
  निश्चित(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
व्योम StringMapImpl::removeBucket(अचिन्हित Bucket) {
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);
  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Bucket, J = I;
  जबतक ((J = (J + 1) & Mask), TheTable[J]) {
    अचिन्हित Ideal = HashTable[J];
    अगर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --अंक_वस्तुएँ;
}

StringMapEntryBase *StringMapImpl::Remove_कुंजी(StringRef कुंजी) {
  पूर्णांक Bucket = Find_कुंजी(कुंजी);
  अगर (Bucket == -1)
    वापस nullptr;

  StringMapEntryBase *परिणाम = TheTable[Bucket];
  removeBucket(Bucket);
  वापस परिणाम;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
अचिन्हित StringMapImpl::RehashTable(अचिन्हित BucketNo) {
  अचिन्हित New_आकार;
  // If the hash table is now more than 3/4 full, grow the table.
  अगर (LLVM_UNLIKELY(अंक_वस्तुएँ * 4 > अंक_Buckets * 3)) {
    New_आकार = अंक_Buckets * 2;
  } अन्यथा {
    वापस BucketNo;
  }

  अचिन्हित NewBucketNo = BucketNo;
  स्वतः **NewTableArray = createTable(New_आकार);
  अचिन्हित *NewHashArray = getHashTable(NewTableArray, New_आकार);
  अचिन्हित *HashTable = getHashTable(TheTable, अंक_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  क्रम (अचिन्हित I = 0, E = अंक_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    अगर (Bucket) {
      // If the bucket is not available, probe for a spot.
      अचिन्हित FullHash = HashTable[I];
      अचिन्हित NewBucket = FullHash & (New_आकार - 1);
      जबतक (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (New_आकार - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      अगर (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  मुक्त(TheTable);

  TheTable = NewTableArray;
  अंक_Buckets = New_आकार;
  वापस NewBucketNo;
}
