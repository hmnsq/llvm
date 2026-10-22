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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ReverseIteration.h"

using ନାମାକାଶ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ସ୍ଥିର ଧାଡ଼ିଭିତରେ unsigned getMinBucketToReserveForEntries(unsigned ଅଙ୍କ_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ଯଦି (ଅଙ୍କ_Entries == 0)
    ଫେରନ୍ତୁ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ଫେରନ୍ତୁ NextPowerOf2(ଅଙ୍କ_Entries * 4 / 3 + 1);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ StringMapEntryBase **createTable(unsigned New_ଅଙ୍କ_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_ଅଙ୍କ_Buckets + 1, ଆକାର(StringMapEntryBase **) + ଆକାର(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_ଅଙ୍କ_Buckets] = (StringMapEntryBase *)2;
  ଫେରନ୍ତୁ Table;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned ଅଙ୍କ_Buckets) {
  ଫେରନ୍ତୁ reinterpret_cast<unsigned *>(TheTable + ଅଙ୍କ_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ଚାବି) { ଫେରନ୍ତୁ xxh3_64bits(ଚାବି); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  ଯଦି (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

ଶୂନ୍ୟ StringMapImpl::init(unsigned InitSize) {
  ପ୍ରତିଜ୍ଞା((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_ଅଙ୍କ_Buckets = InitSize ? InitSize : 16;
  ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ = 0;

  TheTable = createTable(New_ଅଙ୍କ_Buckets);

  // Set the member only if TheTable was successfully allocated
  ଅଙ୍କ_Buckets = New_ଅଙ୍କ_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_ମୂଲ୍ୟ) {
#ifdef EXPENSIVE_CHECKS
  ପ୍ରତିଜ୍ଞା(Full_Hash_ମୂଲ୍ୟ == hash(Name));
#endif
  // Hash table unallocated so far?
  ଯଦି (ଅଙ୍କ_Buckets == 0)
    init(16);
  ଯଦି constexpr (shouldReverseIterate())
    Full_Hash_ମୂଲ୍ୟ = ~Full_Hash_ମୂଲ୍ୟ;
  unsigned BucketNo = Full_Hash_ମୂଲ୍ୟ & (ଅଙ୍କ_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, ଅଙ୍କ_Buckets);

  ଯେତେବେଳେ (ସତ୍ୟ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ଯଦି (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_ମୂଲ୍ୟ;
      ଫେରନ୍ତୁ BucketNo;
    }

    ଯଦି (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_ମୂଲ୍ୟ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      ଅକ୍ଷର *ItemStr = (ଅକ୍ଷର *)BucketItem + ItemSize;
      ଯଦି (Name == StringRef(ItemStr, BucketItem->get_ଚାବି_Length())) {
        // We found a match!
        ଫେରନ୍ତୁ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (ଅଙ୍କ_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା StringMapImpl::Find_ଚାବି(StringRef ଚାବି, uint32_t Full_Hash_ମୂଲ୍ୟ) ଅଚଳ {
  ଯଦି (ଅଙ୍କ_Buckets == 0)
    ଫେରନ୍ତୁ -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  ପ୍ରତିଜ୍ଞା(Full_Hash_ମୂଲ୍ୟ == hash(ଚାବି));
#endif
  ଯଦି constexpr (shouldReverseIterate())
    Full_Hash_ମୂଲ୍ୟ = ~Full_Hash_ମୂଲ୍ୟ;
  unsigned BucketNo = Full_Hash_ମୂଲ୍ୟ & (ଅଙ୍କ_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, ଅଙ୍କ_Buckets);

  ଯେତେବେଳେ (ସତ୍ୟ) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ଯଦି (LLVM_LIKELY(!BucketItem))
      ଫେରନ୍ତୁ -1;

    ଯଦି (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_ମୂଲ୍ୟ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      ଅକ୍ଷର *ItemStr = (ଅକ୍ଷର *)BucketItem + ItemSize;
      ଯଦି (ଚାବି == StringRef(ItemStr, BucketItem->get_ଚାବି_Length())) {
        // We found a match!
        ଫେରନ୍ତୁ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (ଅଙ୍କ_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
ଶୂନ୍ୟ StringMapImpl::Remove_ଚାବି(StringMapEntryBase *V) {
  ଅଚଳ ଅକ୍ଷର *VStr = (ଅକ୍ଷର *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_ଚାବି(StringRef(VStr, V->get_ଚାବି_Length()));
  (ଶୂନ୍ୟ)V2;
  ପ୍ରତିଜ୍ଞା(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
ଶୂନ୍ୟ StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, ଅଙ୍କ_Buckets);
  unsigned Mask = ଅଙ୍କ_Buckets - 1;
  unsigned I = Bucket, J = I;
  ଯେତେବେଳେ ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    ଯଦି (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ;
}

StringMapEntryBase *StringMapImpl::Remove_ଚାବି(StringRef ଚାବି) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Bucket = Find_ଚାବି(ଚାବି);
  ଯଦି (Bucket == -1)
    ଫେରନ୍ତୁ nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  ଫେରନ୍ତୁ Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  ଯଦି (LLVM_UNLIKELY(ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ * 4 > ଅଙ୍କ_Buckets * 3)) {
    NewSize = ଅଙ୍କ_Buckets * 2;
  } ଅନ୍ୟଥା {
    ଫେରନ୍ତୁ BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, ଅଙ୍କ_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  ପାଇଁ (unsigned I = 0, E = ଅଙ୍କ_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    ଯଦି (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      ଯେତେବେଳେ (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      ଯଦି (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  ମୁକ୍ତ(TheTable);

  TheTable = NewTableArray;
  ଅଙ୍କ_Buckets = NewSize;
  ଫେରନ୍ତୁ NewBucketNo;
}
