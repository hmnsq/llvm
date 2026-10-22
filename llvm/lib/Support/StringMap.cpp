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

#அந்தர்பாவ "llvm/ADT/StringMap.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/ReverseIteration.h"

using நாமாகாஶ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
நிலையான உள்வரி unsigned getMinBucketToReserveForEntries(unsigned இலக்கம்_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  எனில் (இலக்கம்_Entries == 0)
    திரும்பு 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  திரும்பு NextPowerOf2(இலக்கம்_Entries * 4 / 3 + 1);
}

நிலையான உள்வரி StringMapEntryBase **createTable(unsigned New_இலக்கம்_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_இலக்கம்_Buckets + 1, அளவு(StringMapEntryBase **) + அளவு(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_இலக்கம்_Buckets] = (StringMapEntryBase *)2;
  திரும்பு Table;
}

நிலையான உள்வரி unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned இலக்கம்_Buckets) {
  திரும்பு reinterpret_cast<unsigned *>(TheTable + இலக்கம்_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef சாவி) { திரும்பு xxh3_64bits(சாவி); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  எனில் (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

வெறுமை StringMapImpl::init(unsigned InitSize) {
  ப்ரதிஜ்ஞா((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_இலக்கம்_Buckets = InitSize ? InitSize : 16;
  இலக்கம்_பொருட்கள் = 0;

  TheTable = createTable(New_இலக்கம்_Buckets);

  // Set the member only if TheTable was successfully allocated
  இலக்கம்_Buckets = New_இலக்கம்_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_மதிப்பு) {
#ifdef EXPENSIVE_CHECKS
  ப்ரதிஜ்ஞா(Full_Hash_மதிப்பு == hash(Name));
#endif
  // Hash table unallocated so far?
  எனில் (இலக்கம்_Buckets == 0)
    init(16);
  எனில் constexpr (shouldReverseIterate())
    Full_Hash_மதிப்பு = ~Full_Hash_மதிப்பு;
  unsigned BucketNo = Full_Hash_மதிப்பு & (இலக்கம்_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, இலக்கம்_Buckets);

  போது (உண்மை) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    எனில் (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_மதிப்பு;
      திரும்பு BucketNo;
    }

    எனில் (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_மதிப்பு)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      எழுத்து *ItemStr = (எழுத்து *)BucketItem + ItemSize;
      எனில் (Name == StringRef(ItemStr, BucketItem->get_சாவி_Length())) {
        // We found a match!
        திரும்பு BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (இலக்கம்_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
முழு StringMapImpl::Find_சாவி(StringRef சாவி, uint32_t Full_Hash_மதிப்பு) மாறிலி {
  எனில் (இலக்கம்_Buckets == 0)
    திரும்பு -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  ப்ரதிஜ்ஞா(Full_Hash_மதிப்பு == hash(சாவி));
#endif
  எனில் constexpr (shouldReverseIterate())
    Full_Hash_மதிப்பு = ~Full_Hash_மதிப்பு;
  unsigned BucketNo = Full_Hash_மதிப்பு & (இலக்கம்_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, இலக்கம்_Buckets);

  போது (உண்மை) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    எனில் (LLVM_LIKELY(!BucketItem))
      திரும்பு -1;

    எனில் (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_மதிப்பு)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      எழுத்து *ItemStr = (எழுத்து *)BucketItem + ItemSize;
      எனில் (சாவி == StringRef(ItemStr, BucketItem->get_சாவி_Length())) {
        // We found a match!
        திரும்பு BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (இலக்கம்_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
வெறுமை StringMapImpl::Remove_சாவி(StringMapEntryBase *V) {
  மாறிலி எழுத்து *VStr = (எழுத்து *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_சாவி(StringRef(VStr, V->get_சாவி_Length()));
  (வெறுமை)V2;
  ப்ரதிஜ்ஞா(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
வெறுமை StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, இலக்கம்_Buckets);
  unsigned Mask = இலக்கம்_Buckets - 1;
  unsigned I = Bucket, J = I;
  போது ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    எனில் (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --இலக்கம்_பொருட்கள்;
}

StringMapEntryBase *StringMapImpl::Remove_சாவி(StringRef சாவி) {
  முழு Bucket = Find_சாவி(சாவி);
  எனில் (Bucket == -1)
    திரும்பு nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  திரும்பு Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  எனில் (LLVM_UNLIKELY(இலக்கம்_பொருட்கள் * 4 > இலக்கம்_Buckets * 3)) {
    NewSize = இலக்கம்_Buckets * 2;
  } இல்லையெனில் {
    திரும்பு BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, இலக்கம்_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  க்கு (unsigned I = 0, E = இலக்கம்_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    எனில் (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      போது (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      எனில் (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  விடுவி(TheTable);

  TheTable = NewTableArray;
  இலக்கம்_Buckets = NewSize;
  திரும்பு NewBucketNo;
}
