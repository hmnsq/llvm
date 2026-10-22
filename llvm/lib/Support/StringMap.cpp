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

#অন্তর্ভাব "llvm/ADT/StringMap.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/ReverseIteration.h"

using নামাকাশ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
স্থির অন্তঃরেখ unsigned getMinBucketToReserveForEntries(unsigned অঙ্ক_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  যদি (অঙ্ক_Entries == 0)
    ফেরত 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ফেরত পরবর্তী_Power_Of2(অঙ্ক_Entries * 4 / 3 + 1);
}

স্থির অন্তঃরেখ StringMapEntryBase **createTable(unsigned New_অঙ্ক_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_অঙ্ক_Buckets + 1, আকার(StringMapEntryBase **) + আকার(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_অঙ্ক_Buckets] = (StringMapEntryBase *)2;
  ফেরত Table;
}

স্থির অন্তঃরেখ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned অঙ্ক_Buckets) {
  ফেরত reinterpret_cast<unsigned *>(TheTable + অঙ্ক_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef চাবি) { ফেরত xxh3_64bits(চাবি); }

StringMapImpl::StringMapImpl(unsigned Init_আকার, unsigned item_আকার)
    : Item_আকার(item_আকার) {
  // If a size is specified, initialize the table with that many buckets.
  যদি (Init_আকার) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(Init_আকার));
  }
}

শূন্য StringMapImpl::init(unsigned Init_আকার) {
  প্রতিজ্ঞা((Init_আকার & (Init_আকার - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_অঙ্ক_Buckets = Init_আকার ? Init_আকার : 16;
  অঙ্ক_বস্তুসমূহ = 0;

  TheTable = createTable(New_অঙ্ক_Buckets);

  // Set the member only if TheTable was successfully allocated
  অঙ্ক_Buckets = New_অঙ্ক_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_মান) {
#ifdef EXPENSIVE_CHECKS
  প্রতিজ্ঞা(Full_Hash_মান == hash(Name));
#endif
  // Hash table unallocated so far?
  যদি (অঙ্ক_Buckets == 0)
    init(16);
  যদি constexpr (shouldReverseIterate())
    Full_Hash_মান = ~Full_Hash_মান;
  unsigned BucketNo = Full_Hash_মান & (অঙ্ক_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);

  যখন (সত্য) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    যদি (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_মান;
      ফেরত BucketNo;
    }

    যদি (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_মান)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      অক্ষর *ItemStr = (অক্ষর *)BucketItem + Item_আকার;
      যদি (Name == StringRef(ItemStr, BucketItem->get_চাবি_দৈর্ঘ্য())) {
        // We found a match!
        ফেরত BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (অঙ্ক_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
পূর্ণসংখ্যা StringMapImpl::Find_চাবি(StringRef চাবি, uint32_t Full_Hash_মান) ধ্রুবক {
  যদি (অঙ্ক_Buckets == 0)
    ফেরত -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  প্রতিজ্ঞা(Full_Hash_মান == hash(চাবি));
#endif
  যদি constexpr (shouldReverseIterate())
    Full_Hash_মান = ~Full_Hash_মান;
  unsigned BucketNo = Full_Hash_মান & (অঙ্ক_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);

  যখন (সত্য) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    যদি (LLVM_LIKELY(!BucketItem))
      ফেরত -1;

    যদি (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_মান)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      অক্ষর *ItemStr = (অক্ষর *)BucketItem + Item_আকার;
      যদি (চাবি == StringRef(ItemStr, BucketItem->get_চাবি_দৈর্ঘ্য())) {
        // We found a match!
        ফেরত BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (অঙ্ক_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
শূন্য StringMapImpl::Remove_চাবি(StringMapEntryBase *V) {
  ধ্রুবক অক্ষর *VStr = (অক্ষর *)V + Item_আকার;
  StringMapEntryBase *V2 = Remove_চাবি(StringRef(VStr, V->get_চাবি_দৈর্ঘ্য()));
  (শূন্য)V2;
  প্রতিজ্ঞা(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
শূন্য StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);
  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Bucket, J = I;
  যখন ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    যদি (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --অঙ্ক_বস্তুসমূহ;
}

StringMapEntryBase *StringMapImpl::Remove_চাবি(StringRef চাবি) {
  পূর্ণসংখ্যা Bucket = Find_চাবি(চাবি);
  যদি (Bucket == -1)
    ফেরত nullptr;

  StringMapEntryBase *ফলাফল = TheTable[Bucket];
  removeBucket(Bucket);
  ফেরত ফলাফল;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned New_আকার;
  // If the hash table is now more than 3/4 full, grow the table.
  যদি (LLVM_UNLIKELY(অঙ্ক_বস্তুসমূহ * 4 > অঙ্ক_Buckets * 3)) {
    New_আকার = অঙ্ক_Buckets * 2;
  } অন্যথা {
    ফেরত BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(New_আকার);
  unsigned *NewHashArray = getHashTable(NewTableArray, New_আকার);
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  জন্য (unsigned I = 0, E = অঙ্ক_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    যদি (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (New_আকার - 1);
      যখন (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (New_আকার - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      যদি (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  মুক্ত(TheTable);

  TheTable = NewTableArray;
  অঙ্ক_Buckets = New_আকার;
  ফেরত NewBucketNo;
}
