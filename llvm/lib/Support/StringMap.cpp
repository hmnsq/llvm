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

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/ReverseIteration.h"

using namespace llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
static inline unsigned getMinBucketToReserveForEntries(unsigned नुमेरो_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  if (नुमेरो_Entries == 0)
    return 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  return NextPowerOf2(नुमेरो_Entries * 4 / 3 + 1);
}

static inline StringMapEntryBase **createTable(unsigned New_नुमेरो_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_नुमेरो_Buckets + 1, sizeof(StringMapEntryBase **) + sizeof(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_नुमेरो_Buckets] = (StringMapEntryBase *)2;
  return Table;
}

static inline unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned नुमेरो_Buckets) {
  return reinterpret_cast<unsigned *>(TheTable + नुमेरो_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef सुसि) { return xxh3_64bits(सुसि); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  if (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

void StringMapImpl::init(unsigned InitSize) {
  assert((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_नुमेरो_Buckets = InitSize ? InitSize : 16;
  नुमेरो_मगा_बागाय = 0;

  TheTable = createTable(New_नुमेरो_Buckets);

  // Set the member only if TheTable was successfully allocated
  नुमेरो_Buckets = New_नुमेरो_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_हालागा) {
#ifdef EXPENSIVE_CHECKS
  assert(Full_Hash_हालागा == hash(Name));
#endif
  // Hash table unallocated so far?
  if (नुमेरो_Buckets == 0)
    init(16);
  if constexpr (shouldReverseIterate())
    Full_Hash_हालागा = ~Full_Hash_हालागा;
  unsigned BucketNo = Full_Hash_हालागा & (नुमेरो_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, नुमेरो_Buckets);

  while (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    if (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_हालागा;
      return BucketNo;
    }

    if (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_हालागा)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      char *ItemStr = (char *)BucketItem + ItemSize;
      if (Name == StringRef(ItemStr, BucketItem->get_सुसि_Length())) {
        // We found a match!
        return BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (नुमेरो_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
int StringMapImpl::Find_सुसि(StringRef सुसि, uint32_t Full_Hash_हालागा) const {
  if (नुमेरो_Buckets == 0)
    return -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  assert(Full_Hash_हालागा == hash(सुसि));
#endif
  if constexpr (shouldReverseIterate())
    Full_Hash_हालागा = ~Full_Hash_हालागा;
  unsigned BucketNo = Full_Hash_हालागा & (नुमेरो_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, नुमेरो_Buckets);

  while (true) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    if (LLVM_LIKELY(!BucketItem))
      return -1;

    if (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_हालागा)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      char *ItemStr = (char *)BucketItem + ItemSize;
      if (सुसि == StringRef(ItemStr, BucketItem->get_सुसि_Length())) {
        // We found a match!
        return BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (नुमेरो_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
void StringMapImpl::Remove_सुसि(StringMapEntryBase *V) {
  const char *VStr = (char *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_सुसि(StringRef(VStr, V->get_सुसि_Length()));
  (void)V2;
  assert(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
void StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, नुमेरो_Buckets);
  unsigned Mask = नुमेरो_Buckets - 1;
  unsigned I = Bucket, J = I;
  while ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    if (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --नुमेरो_मगा_बागाय;
}

StringMapEntryBase *StringMapImpl::Remove_सुसि(StringRef सुसि) {
  int Bucket = Find_सुसि(सुसि);
  if (Bucket == -1)
    return nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  return Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  if (LLVM_UNLIKELY(नुमेरो_मगा_बागाय * 4 > नुमेरो_Buckets * 3)) {
    NewSize = नुमेरो_Buckets * 2;
  } else {
    return BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, नुमेरो_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  for (unsigned I = 0, E = नुमेरो_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    if (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      while (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      if (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  free(TheTable);

  TheTable = NewTableArray;
  नुमेरो_Buckets = NewSize;
  return NewBucketNo;
}
