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

उपयुज्य नामाकाश llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
स्थिर अन्तःस्थ अचिह्नित get_Min_Bucket_गन्तव्यम्_Reserve_For_Entries(अचिह्नित अङ्कः_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  यदि (अङ्कः_Entries == 0)
    प्रत्यागम 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  प्रत्यागम अग्र_Power_Of2(अङ्कः_Entries * 4 / 3 + 1);
}

स्थिर अन्तःस्थ String_Map_प्रविष्टिः_आधारः **create_सारणी(अचिह्नित New_अङ्कः_Buckets) {
  स्वचालित **सारणी = static_cast<String_Map_प्रविष्टिः_आधारः **>(safe_calloc(
      New_अङ्कः_Buckets + 1, परिमाण(String_Map_प्रविष्टिः_आधारः **) + परिमाण(अचिह्नित)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  सारणी[New_अङ्कः_Buckets] = (String_Map_प्रविष्टिः_आधारः *)2;
  प्रत्यागम सारणी;
}

स्थिर अन्तःस्थ अचिह्नित *get_Hash_सारणी(String_Map_प्रविष्टिः_आधारः **The_सारणी,
                                     अचिह्नित अङ्कः_Buckets) {
  प्रत्यागम reinterpret_cast<अचिह्नित *>(The_सारणी + अङ्कः_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef कुञ्चिका) { प्रत्यागम xxh3_64bits(कुञ्चिका); }

StringMapImpl::StringMapImpl(अचिह्नित Init_आकार, अचिह्नित item_आकार)
    : Item_आकार(item_आकार) {
  // If a size is specified, initialize the table with that many buckets.
  यदि (Init_आकार) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_Bucket_गन्तव्यम्_Reserve_For_Entries(Init_आकार));
  }
}

शून्य StringMapImpl::init(अचिह्नित Init_आकार) {
  प्रतिज्ञा((Init_आकार & (Init_आकार - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  अचिह्नित New_अङ्कः_Buckets = Init_आकार ? Init_आकार : 16;
  अङ्कः_वस्तूनि = 0;

  The_सारणी = create_सारणी(New_अङ्कः_Buckets);

  // Set the member only if TheTable was successfully allocated
  अङ्कः_Buckets = New_अङ्कः_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
अचिह्नित StringMapImpl::LookupBucketFor(StringRef Name,
                                        uint32_t पूर्णम्_Hash_मानम्) {
#यदि_परिभाषितम् EXPENSIVE_CHECKS
  प्रतिज्ञा(पूर्णम्_Hash_मानम् == hash(Name));
#यदि_अन्तः
  // Hash table unallocated so far?
  यदि (अङ्कः_Buckets == 0)
    init(16);
  यदि constexpr (shouldReverseIterate())
    पूर्णम्_Hash_मानम् = ~पूर्णम्_Hash_मानम्;
  अचिह्नित BucketNo = पूर्णम्_Hash_मानम् & (अङ्कः_Buckets - 1);
  अचिह्नित *Hash_सारणी = get_Hash_सारणी(The_सारणी, अङ्कः_Buckets);

  यावत् (सत्य) {
    String_Map_प्रविष्टिः_आधारः *BucketItem = The_सारणी[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    यदि (LLVM_LIKELY(!BucketItem)) {
      Hash_सारणी[BucketNo] = पूर्णम्_Hash_मानम्;
      प्रत्यागम BucketNo;
    }

    यदि (LLVM_LIKELY(Hash_सारणी[BucketNo] == पूर्णम्_Hash_मानम्)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + Item_आकार;
      यदि (Name == StringRef(ItemStr, BucketItem->get_कुञ्चिका_दीर्घता())) {
        // We found a match!
        प्रत्यागम BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्कः_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
पूर्णाङ्क StringMapImpl::Find_कुञ्चिका(StringRef कुञ्चिका, uint32_t पूर्णम्_Hash_मानम्) ध्रुव {
  यदि (अङ्कः_Buckets == 0)
    प्रत्यागम -1; // Really empty table?
#यदि_परिभाषितम् EXPENSIVE_CHECKS
  प्रतिज्ञा(पूर्णम्_Hash_मानम् == hash(कुञ्चिका));
#यदि_अन्तः
  यदि constexpr (shouldReverseIterate())
    पूर्णम्_Hash_मानम् = ~पूर्णम्_Hash_मानम्;
  अचिह्नित BucketNo = पूर्णम्_Hash_मानम् & (अङ्कः_Buckets - 1);
  अचिह्नित *Hash_सारणी = get_Hash_सारणी(The_सारणी, अङ्कः_Buckets);

  यावत् (सत्य) {
    String_Map_प्रविष्टिः_आधारः *BucketItem = The_सारणी[BucketNo];
    // If we found an empty bucket, this key isn't in the table yet, return.
    यदि (LLVM_LIKELY(!BucketItem))
      प्रत्यागम -1;

    यदि (LLVM_LIKELY(Hash_सारणी[BucketNo] == पूर्णम्_Hash_मानम्)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      वर्ण *ItemStr = (वर्ण *)BucketItem + Item_आकार;
      यदि (कुञ्चिका == StringRef(ItemStr, BucketItem->get_कुञ्चिका_दीर्घता())) {
        // We found a match!
        प्रत्यागम BucketNo;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    BucketNo = (BucketNo + 1) & (अङ्कः_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
शून्य StringMapImpl::Remove_कुञ्चिका(String_Map_प्रविष्टिः_आधारः *V) {
  ध्रुव वर्ण *VStr = (वर्ण *)V + Item_आकार;
  String_Map_प्रविष्टिः_आधारः *V2 = Remove_कुञ्चिका(StringRef(VStr, V->get_कुञ्चिका_दीर्घता()));
  (शून्य)V2;
  प्रतिज्ञा(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
शून्य StringMapImpl::removeBucket(अचिह्नित Bucket) {
  अचिह्नित *Hash_सारणी = get_Hash_सारणी(The_सारणी, अङ्कः_Buckets);
  अचिह्नित आवरणम् = अङ्कः_Buckets - 1;
  अचिह्नित I = Bucket, J = I;
  यावत् ((J = (J + 1) & आवरणम्), The_सारणी[J]) {
    अचिह्नित Ideal = Hash_सारणी[J];
    यदि (((I - Ideal) & आवरणम्) < ((J - Ideal) & आवरणम्)) {
      The_सारणी[I] = The_सारणी[J];
      Hash_सारणी[I] = Hash_सारणी[J];
      I = J;
    }
  }
  The_सारणी[I] = शून्यसूचकः;
  --अङ्कः_वस्तूनि;
}

String_Map_प्रविष्टिः_आधारः *StringMapImpl::Remove_कुञ्चिका(StringRef कुञ्चिका) {
  पूर्णाङ्क Bucket = Find_कुञ्चिका(कुञ्चिका);
  यदि (Bucket == -1)
    प्रत्यागम शून्यसूचकः;

  String_Map_प्रविष्टिः_आधारः *परिणाम = The_सारणी[Bucket];
  removeBucket(Bucket);
  प्रत्यागम परिणाम;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
अचिह्नित StringMapImpl::Rehash_सारणी(अचिह्नित BucketNo) {
  अचिह्नित New_आकार;
  // If the hash table is now more than 3/4 full, grow the table.
  यदि (LLVM_UNLIKELY(अङ्कः_वस्तूनि * 4 > अङ्कः_Buckets * 3)) {
    New_आकार = अङ्कः_Buckets * 2;
  } अन्यथा {
    प्रत्यागम BucketNo;
  }

  अचिह्नित NewBucketNo = BucketNo;
  स्वचालित **New_सारणी_Array = create_सारणी(New_आकार);
  अचिह्नित *NewHashArray = get_Hash_सारणी(New_सारणी_Array, New_आकार);
  अचिह्नित *Hash_सारणी = get_Hash_सारणी(The_सारणी, अङ्कः_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  प्रति (अचिह्नित I = 0, E = अङ्कः_Buckets; I != E; ++I) {
    String_Map_प्रविष्टिः_आधारः *Bucket = The_सारणी[I];
    यदि (Bucket) {
      // If the bucket is not available, probe for a spot.
      अचिह्नित पूर्णम्_Hash = Hash_सारणी[I];
      अचिह्नित NewBucket = पूर्णम्_Hash & (New_आकार - 1);
      यावत् (New_सारणी_Array[NewBucket])
        NewBucket = (NewBucket + 1) & (New_आकार - 1);

      // Finally found a slot.  Fill it in.
      New_सारणी_Array[NewBucket] = Bucket;
      NewHashArray[NewBucket] = पूर्णम्_Hash;
      यदि (I == BucketNo)
        NewBucketNo = NewBucket;
    }
  }

  मुक्त(The_सारणी);

  The_सारणी = New_सारणी_Array;
  अङ्कः_Buckets = New_आकार;
  प्रत्यागम NewBucketNo;
}
