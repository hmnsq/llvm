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

#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/ReverseIteration.h"

using നാമാകാശ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
സ്ഥിതം വരിയിൽ unsigned getMinBucketToReserveForEntries(unsigned അക്കം_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  എങ്കിൽ (അക്കം_Entries == 0)
    തിരികെ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  തിരികെ NextPowerOf2(അക്കം_Entries * 4 / 3 + 1);
}

സ്ഥിതം വരിയിൽ StringMapEntryBase **createTable(unsigned New_അക്കം_Buckets) {
  auto **Table = static_cast<StringMapEntryBase **>(safe_calloc(
      New_അക്കം_Buckets + 1, വലുപ്പം(StringMapEntryBase **) + വലുപ്പം(unsigned)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  Table[New_അക്കം_Buckets] = (StringMapEntryBase *)2;
  തിരികെ Table;
}

സ്ഥിതം വരിയിൽ unsigned *getHashTable(StringMapEntryBase **TheTable,
                                     unsigned അക്കം_Buckets) {
  തിരികെ reinterpret_cast<unsigned *>(TheTable + അക്കം_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef താക്കോൽ) { തിരികെ xxh3_64bits(താക്കോൽ); }

StringMapImpl::StringMapImpl(unsigned InitSize, unsigned itemSize)
    : ItemSize(itemSize) {
  // If a size is specified, initialize the table with that many buckets.
  എങ്കിൽ (InitSize) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(getMinBucketToReserveForEntries(InitSize));
  }
}

ശൂന്യം StringMapImpl::init(unsigned InitSize) {
  പ്രതിജ്ഞാ((InitSize & (InitSize - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  unsigned New_അക്കം_Buckets = InitSize ? InitSize : 16;
  അക്കം_ഇനങ്ങൾ = 0;

  TheTable = createTable(New_അക്കം_Buckets);

  // Set the member only if TheTable was successfully allocated
  അക്കം_Buckets = New_അക്കം_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
unsigned StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t Full_Hash_മൂല്യം) {
#ifdef EXPENSIVE_CHECKS
  പ്രതിജ്ഞാ(Full_Hash_മൂല്യം == hash(Name));
#endif
  // Hash table unallocated so far?
  എങ്കിൽ (അക്കം_Buckets == 0)
    init(16);
  എങ്കിൽ constexpr (shouldReverseIterate())
    Full_Hash_മൂല്യം = ~Full_Hash_മൂല്യം;
  unsigned BucketNo = Full_Hash_മൂല്യം & (അക്കം_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, അക്കം_Buckets);

  ആകുമ്പോൾ (സത്യം) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    എങ്കിൽ (LLVM_LIKELY(!BucketItem)) {
      HashTable[BucketNo] = Full_Hash_മൂല്യം;
      തിരികെ BucketNo;
    }

    എങ്കിൽ (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_മൂല്യം)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      അക്ഷരം *ItemStr = (അക്ഷരം *)BucketItem + ItemSize;
      എങ്കിൽ (Name == StringRef(ItemStr, BucketItem->get_താക്കോൽ_Length())) {
        // We found a match!
        തിരികെ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (അക്കം_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
പൂർണ്ണസംഖ്യ StringMapImpl::Find_താക്കോൽ(StringRef താക്കോൽ, uint32_t Full_Hash_മൂല്യം) സ്ഥിരം {
  എങ്കിൽ (അക്കം_Buckets == 0)
    തിരികെ -1; // Really empty table?
#ifdef EXPENSIVE_CHECKS
  പ്രതിജ്ഞാ(Full_Hash_മൂല്യം == hash(താക്കോൽ));
#endif
  എങ്കിൽ constexpr (shouldReverseIterate())
    Full_Hash_മൂല്യം = ~Full_Hash_മൂല്യം;
  unsigned BucketNo = Full_Hash_മൂല്യം & (അക്കം_Buckets - 1);
  unsigned *HashTable = getHashTable(TheTable, അക്കം_Buckets);

  ആകുമ്പോൾ (സത്യം) {
    StringMapEntryBase *BucketItem = TheTable[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    എങ്കിൽ (LLVM_LIKELY(!BucketItem))
      തിരികെ -1;

    എങ്കിൽ (LLVM_LIKELY(HashTable[BucketNo] == Full_Hash_മൂല്യം)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      അക്ഷരം *ItemStr = (അക്ഷരം *)BucketItem + ItemSize;
      എങ്കിൽ (താക്കോൽ == StringRef(ItemStr, BucketItem->get_താക്കോൽ_Length())) {
        // We found a match!
        തിരികെ BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (അക്കം_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
ശൂന്യം StringMapImpl::Remove_താക്കോൽ(StringMapEntryBase *V) {
  സ്ഥിരം അക്ഷരം *VStr = (അക്ഷരം *)V + ItemSize;
  StringMapEntryBase *V2 = Remove_താക്കോൽ(StringRef(VStr, V->get_താക്കോൽ_Length()));
  (ശൂന്യം)V2;
  പ്രതിജ്ഞാ(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
ശൂന്യം StringMapImpl::removeBucket(unsigned Bucket) {
  unsigned *HashTable = getHashTable(TheTable, അക്കം_Buckets);
  unsigned Mask = അക്കം_Buckets - 1;
  unsigned I = Bucket, J = I;
  ആകുമ്പോൾ ((J = (J + 1) & Mask), TheTable[J]) {
    unsigned Ideal = HashTable[J];
    എങ്കിൽ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      TheTable[I] = TheTable[J];
      HashTable[I] = HashTable[J];
      I = J;
    }
  }
  TheTable[I] = nullptr;
  --അക്കം_ഇനങ്ങൾ;
}

StringMapEntryBase *StringMapImpl::Remove_താക്കോൽ(StringRef താക്കോൽ) {
  പൂർണ്ണസംഖ്യ Bucket = Find_താക്കോൽ(താക്കോൽ);
  എങ്കിൽ (Bucket == -1)
    തിരികെ nullptr;

  StringMapEntryBase *Result = TheTable[Bucket];
  removeBucket(Bucket);
  തിരികെ Result;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
unsigned StringMapImpl::RehashTable(unsigned BucketNo) {
  unsigned NewSize;
  // If the hash table is now more than 3/4 full, grow the table.
  എങ്കിൽ (LLVM_UNLIKELY(അക്കം_ഇനങ്ങൾ * 4 > അക്കം_Buckets * 3)) {
    NewSize = അക്കം_Buckets * 2;
  } അല്ലെങ്കിൽ {
    തിരികെ BucketNo;
  }

  unsigned NewBucketNo = BucketNo;
  auto **NewTableArray = createTable(NewSize);
  unsigned *NewHashArray = getHashTable(NewTableArray, NewSize);
  unsigned *HashTable = getHashTable(TheTable, അക്കം_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  വേണ്ടി (unsigned I = 0, E = അക്കം_Buckets; I != E; ++I) {
    StringMapEntryBase *Bucket = TheTable[I];
    എങ്കിൽ (Bucket) {
      // If the bucket is not available, probe for a spot.
      unsigned FullHash = HashTable[I];
      unsigned NewBucket = FullHash & (NewSize - 1);
      ആകുമ്പോൾ (NewTableArray[NewBucket])
        NewBucket = (NewBucket + 1) & (NewSize - 1);

      // Finally found a slot.  Fill it in.
      NewTableArray[NewBucket] = Bucket;
      NewHashArray[NewBucket] = FullHash;
      എങ്കിൽ (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  സ്വതന്ത്രം(TheTable);

  TheTable = NewTableArray;
  അക്കം_Buckets = NewSize;
  തിരികെ NewBucketNo;
}
