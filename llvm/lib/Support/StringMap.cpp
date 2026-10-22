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
স্থির অন্তঃস্থ unsigned getMinBucketToReserveForEntries(unsigned অঙ্ক_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  যদি (অঙ্ক_Entries == 0)
    প্রত্যাগম 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  প্রত্যাগম NextPowerOf2(অঙ্ক_Entries * 4 / 3 + 1);
}

স্থির অন্তঃস্থ StringMapEntryBase **createTable(unsigned New_অঙ্ক_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_অঙ্ক_Buckets + 1, পরিমাণ(StringMapEntryBase **) + পরিমাণ(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_অঙ্ক_Buckets] = (StringMapEntryBase *)2;
  প্রত্যাগম Table;
}

স্থির অন্তঃস্থ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned অঙ্ক_Buckets) {
  প্রত্যাগম reinterpret_cast<unsigned *>(TheTable + অঙ্ক_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef চাবি) { প্রত্যাগম xxh3_64bits(চাবি); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  যদি (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

শূন্য StringMapImpl::init(unsigned InitSize) {
  প্রতিজ্ঞা((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_অঙ্ক_Buckets = InitSize ? InitSize : 16;
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

  যাবত্ (সত্য) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    যদি (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_মান;
      প্রত্যাগম BucketNo;
    }

    যদি (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_মান)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      বর্ণ *ItemStr = (বর্ণ *)BucketItem + ItemSize;
      যদি (Name == StringRef(ItemStr, BucketItem->get_চাবি_Length())) {
        // We found a match!
        প্রত্যাগম BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (অঙ্ক_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
পূর্ণাঙ্ক StringMapImpl::Find_চাবি(StringRef চাবি, uint32_t Full_Hash_মান) ধ্রুব {
  যদি (অঙ্ক_Buckets == 0)
    প্রত্যাগম -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  প্রতিজ্ঞা(Full_Hash_মান == hash(চাবি));
#endif
  যদি constexpr (shouldReverseIterate())
    Full_Hash_মান = ~Full_Hash_মান;
  unsigned BucketNo = Full_Hash_মান & (অঙ্ক_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);

  যাবত্ (সত্য) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    যদি (LLVM_LIKELY(!BucketItem))
      প্রত্যাগম -1;

    যদি (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_মান)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      বর্ণ *ItemStr = (বর্ণ *)BucketItem + ItemSize;
      যদি (চাবি == StringRef(ItemStr, BucketItem->get_চাবি_Length())) {
        // We found a match!
        প্রত্যাগম BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (অঙ্ক_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
শূন্য StringMapImpl::Remove_চাবি(StringMapEntryBase *V) {
  ধ্রুব বর্ণ *VStr = (বর্ণ *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_চাবি(StringRef(VStr, V->get_চাবি_Length()));
  (শূন্য)V2;
  প্রতিজ্ঞা(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
শূন্য StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);
  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Bucket, J = I;
  যাবত্ ((J = (J + 1) & Mask), TheTable[J]) {
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
  পূর্ণাঙ্ক Bucket = Find_চাবি(চাবি);
  যদি (Bucket == -1)
    প্রত্যাগম nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  প্রত্যাগম Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  যদি (LLVM_UNLIKELY(অঙ্ক_বস্তুসমূহ * 4 > অঙ্ক_Buckets * 3)) {
    NewSize = অঙ্ক_Buckets * 2;
  } অন্যথা {
    প্রত্যাগম BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, অঙ্ক_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  প্রতি (unsigned I = 0, E = অঙ্ক_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    যদি (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      যাবত্ (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      যদি (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  মুক্ত(TheTable);

  TheTable = NewTableArray;
  অঙ্ক_Buckets = NewSize;
  প্রত্যাগম NewBucketNo;
}
