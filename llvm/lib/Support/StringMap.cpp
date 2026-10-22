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

#અન્તર્ભાવ "llvm/ADT/StringMap.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/ReverseIteration.h"

using નામાકાશ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
સ્થિર અંતઃરેખ unsigned getMinBucketToReserveForEntries(unsigned અંક_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  જો (અંક_Entries == 0)
    પાછા 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  પાછા NextPowerOf2(અંક_Entries * 4 / 3 + 1);
}

સ્થિર અંતઃરેખ StringMapEntryBase **createTable(unsigned New_અંક_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_અંક_Buckets + 1, કદ(StringMapEntryBase **) + કદ(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_અંક_Buckets] = (StringMapEntryBase *)2;
  પાછા Table;
}

સ્થિર અંતઃરેખ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned અંક_Buckets) {
  પાછા reinterpret_cast<unsigned *>(TheTable + અંક_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ચાવી) { પાછા xxh3_64bits(ચાવી); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  જો (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

શૂન્ય StringMapImpl::init(unsigned InitSize) {
  પ્રતિજ્ઞા((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_અંક_Buckets = InitSize ? InitSize : 16;
  અંક_વસ્તુઓ = 0;

  TheTable = createTable(New_અંક_Buckets);

  // Set the member only if TheTable was successfully allocated
  અંક_Buckets = New_અંક_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_કિંમત) {
#ifdef EXPENSIVE_CHECKS
  પ્રતિજ્ઞા(Full_Hash_કિંમત == hash(Name));
#endif
  // Hash table unallocated so far?
  જો (અંક_Buckets == 0)
    init(16);
  જો constexpr (shouldReverseIterate())
    Full_Hash_કિંમત = ~Full_Hash_કિંમત;
  unsigned BucketNo = Full_Hash_કિંમત & (અંક_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, અંક_Buckets);

  જ્યારે (સાચું) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    જો (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_કિંમત;
      પાછા BucketNo;
    }

    જો (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_કિંમત)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      અક્ષર *ItemStr = (અક્ષર *)BucketItem + ItemSize;
      જો (Name == StringRef(ItemStr, BucketItem->get_ચાવી_Length())) {
        // We found a match!
        પાછા BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (અંક_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
પૂર્ણાંક StringMapImpl::Find_ચાવી(StringRef ચાવી, uint32_t Full_Hash_કિંમત) અચલ {
  જો (અંક_Buckets == 0)
    પાછા -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  પ્રતિજ્ઞા(Full_Hash_કિંમત == hash(ચાવી));
#endif
  જો constexpr (shouldReverseIterate())
    Full_Hash_કિંમત = ~Full_Hash_કિંમત;
  unsigned BucketNo = Full_Hash_કિંમત & (અંક_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, અંક_Buckets);

  જ્યારે (સાચું) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    જો (LLVM_LIKELY(!BucketItem))
      પાછા -1;

    જો (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_કિંમત)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      અક્ષર *ItemStr = (અક્ષર *)BucketItem + ItemSize;
      જો (ચાવી == StringRef(ItemStr, BucketItem->get_ચાવી_Length())) {
        // We found a match!
        પાછા BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (અંક_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
શૂન્ય StringMapImpl::Remove_ચાવી(StringMapEntryBase *V) {
  અચલ અક્ષર *VStr = (અક્ષર *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_ચાવી(StringRef(VStr, V->get_ચાવી_Length()));
  (શૂન્ય)V2;
  પ્રતિજ્ઞા(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
શૂન્ય StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, અંક_Buckets);
  unsigned Mask = અંક_Buckets - 1;
  unsigned I = Bucket, J = I;
  જ્યારે ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    જો (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --અંક_વસ્તુઓ;
}

StringMapEntryBase *StringMapImpl::Remove_ચાવી(StringRef ચાવી) {
  પૂર્ણાંક Bucket = Find_ચાવી(ચાવી);
  જો (Bucket == -1)
    પાછા nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  પાછા Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  જો (LLVM_UNLIKELY(અંક_વસ્તુઓ * 4 > અંક_Buckets * 3)) {
    NewSize = અંક_Buckets * 2;
  } નહીંતર {
    પાછા BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, અંક_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  માટે (unsigned I = 0, E = અંક_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    જો (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      જ્યારે (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      જો (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  મુક્ત(TheTable);

  TheTable = NewTableArray;
  અંક_Buckets = NewSize;
  પાછા NewBucketNo;
}
