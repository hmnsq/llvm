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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ReverseIteration.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ get_Min_ਬਾਲਟੀ_ਮੰਜ਼ਿਲ_Reserve_For_Entries(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ਜੇ (ਅੰਕ_Entries == 0)
    ਵਾਪਸ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ਵਾਪਸ ਅਗਲਾ_Power_Of2(ਅੰਕ_Entries * 4 / 3 + 1);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ **create_ਸਾਰਣੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਅੰਕ_Buckets) {
  ਸਵੈਚਾਲਿਤ **ਸਾਰਣੀ = static_cast<String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ **>(ਸੁਰੱਖਿਅਤ_calloc(
      New_ਅੰਕ_Buckets + 1, ਆਕਾਰ(String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ **) + ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  ਸਾਰਣੀ[New_ਅੰਕ_Buckets] = (String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *)2;
  ਵਾਪਸ ਸਾਰਣੀ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ *get_Hash_ਸਾਰਣੀ(String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ **The_ਸਾਰਣੀ,
                                     ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Buckets) {
  ਵਾਪਸ reinterpret_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ *>(The_ਸਾਰਣੀ + ਅੰਕ_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ਕੁੰਜੀ) { ਵਾਪਸ xxh3_64bits(ਕੁੰਜੀ); }

StringMapImpl::StringMapImpl(ਬਿਨਾਨਿਸ਼ਾਨੀ Init_ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਸਤੂ_ਮਾਪ)
    : ਵਸਤੂ_ਮਾਪ_2(ਵਸਤੂ_ਮਾਪ) {
  // If a size is specified, initialize the table with that many buckets.
  ਜੇ (Init_ਮਾਪ) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_ਬਾਲਟੀ_ਮੰਜ਼ਿਲ_Reserve_For_Entries(Init_ਮਾਪ));
  }
}

ਖਾਲੀ StringMapImpl::init(ਬਿਨਾਨਿਸ਼ਾਨੀ Init_ਮਾਪ) {
  ਪ੍ਰਤਿਜ੍ਞਾ((Init_ਮਾਪ & (Init_ਮਾਪ - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਅੰਕ_Buckets = Init_ਮਾਪ ? Init_ਮਾਪ : 16;
  ਅੰਕ_ਚੀਜ਼ਾਂ = 0;

  The_ਸਾਰਣੀ = create_ਸਾਰਣੀ(New_ਅੰਕ_Buckets);

  // Set the member only if TheTable was successfully allocated
  ਅੰਕ_Buckets = New_ਅੰਕ_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
ਬਿਨਾਨਿਸ਼ਾਨੀ StringMapImpl::Lookup_ਬਾਲਟੀ_For(StringRef Name,
                                        uint32_t ਭਰਿਆ_Hash_ਮੁੱਲ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EXPENSIVE_CHECKS
  ਪ੍ਰਤਿਜ੍ਞਾ(ਭਰਿਆ_Hash_ਮੁੱਲ == hash(Name));
#ਜੇ_ਅੰਤ
  // Hash table unallocated so far?
  ਜੇ (ਅੰਕ_Buckets == 0)
    init(16);
  ਜੇ constexpr (should_ਉਲਟ_Iterate())
    ਭਰਿਆ_Hash_ਮੁੱਲ = ~ਭਰਿਆ_Hash_ਮੁੱਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ_No = ਭਰਿਆ_Hash_ਮੁੱਲ & (ਅੰਕ_Buckets - 1);
  ਬਿਨਾਨਿਸ਼ਾਨੀ *Hash_ਸਾਰਣੀ = get_Hash_ਸਾਰਣੀ(The_ਸਾਰਣੀ, ਅੰਕ_Buckets);

  ਜਦੋਂ (ਸੱਚ) {
    String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *ਬਾਲਟੀ_ਵਸਤੂ = The_ਸਾਰਣੀ[ਬਾਲਟੀ_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ਜੇ (LLVM_LIKELY(!ਬਾਲਟੀ_ਵਸਤੂ)) {
      Hash_ਸਾਰਣੀ[ਬਾਲਟੀ_No] = ਭਰਿਆ_Hash_ਮੁੱਲ;
      ਵਾਪਸ ਬਾਲਟੀ_No;
    }

    ਜੇ (LLVM_LIKELY(Hash_ਸਾਰਣੀ[ਬਾਲਟੀ_No] == ਭਰਿਆ_Hash_ਮੁੱਲ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      ਅੱਖਰ *ਵਸਤੂ_Str = (ਅੱਖਰ *)ਬਾਲਟੀ_ਵਸਤੂ + ਵਸਤੂ_ਮਾਪ_2;
      ਜੇ (Name == StringRef(ਵਸਤੂ_Str, ਬਾਲਟੀ_ਵਸਤੂ->get_ਕੁੰਜੀ_ਲੰਮਾਈ())) {
        // We found a match!
        ਵਾਪਸ ਬਾਲਟੀ_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ਬਾਲਟੀ_No = (ਬਾਲਟੀ_No + 1) & (ਅੰਕ_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
ਪੂਰਨ StringMapImpl::ਲੱਭੋ_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ, uint32_t ਭਰਿਆ_Hash_ਮੁੱਲ) ਅਚਲ {
  ਜੇ (ਅੰਕ_Buckets == 0)
    ਵਾਪਸ -1; // Really empty table?
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EXPENSIVE_CHECKS
  ਪ੍ਰਤਿਜ੍ਞਾ(ਭਰਿਆ_Hash_ਮੁੱਲ == hash(ਕੁੰਜੀ));
#ਜੇ_ਅੰਤ
  ਜੇ constexpr (should_ਉਲਟ_Iterate())
    ਭਰਿਆ_Hash_ਮੁੱਲ = ~ਭਰਿਆ_Hash_ਮੁੱਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ_No = ਭਰਿਆ_Hash_ਮੁੱਲ & (ਅੰਕ_Buckets - 1);
  ਬਿਨਾਨਿਸ਼ਾਨੀ *Hash_ਸਾਰਣੀ = get_Hash_ਸਾਰਣੀ(The_ਸਾਰਣੀ, ਅੰਕ_Buckets);

  ਜਦੋਂ (ਸੱਚ) {
    String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *ਬਾਲਟੀ_ਵਸਤੂ = The_ਸਾਰਣੀ[ਬਾਲਟੀ_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ਜੇ (LLVM_LIKELY(!ਬਾਲਟੀ_ਵਸਤੂ))
      ਵਾਪਸ -1;

    ਜੇ (LLVM_LIKELY(Hash_ਸਾਰਣੀ[ਬਾਲਟੀ_No] == ਭਰਿਆ_Hash_ਮੁੱਲ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      ਅੱਖਰ *ਵਸਤੂ_Str = (ਅੱਖਰ *)ਬਾਲਟੀ_ਵਸਤੂ + ਵਸਤੂ_ਮਾਪ_2;
      ਜੇ (ਕੁੰਜੀ == StringRef(ਵਸਤੂ_Str, ਬਾਲਟੀ_ਵਸਤੂ->get_ਕੁੰਜੀ_ਲੰਮਾਈ())) {
        // We found a match!
        ਵਾਪਸ ਬਾਲਟੀ_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ਬਾਲਟੀ_No = (ਬਾਲਟੀ_No + 1) & (ਅੰਕ_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
ਖਾਲੀ StringMapImpl::Remove_ਕੁੰਜੀ(String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *V) {
  ਅਚਲ ਅੱਖਰ *VStr = (ਅੱਖਰ *)V + ਵਸਤੂ_ਮਾਪ_2;
  String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *V2 = Remove_ਕੁੰਜੀ(StringRef(VStr, V->get_ਕੁੰਜੀ_ਲੰਮਾਈ()));
  (ਖਾਲੀ)V2;
  ਪ੍ਰਤਿਜ੍ਞਾ(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
ਖਾਲੀ StringMapImpl::remove_ਬਾਲਟੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ *Hash_ਸਾਰਣੀ = get_Hash_ਸਾਰਣੀ(The_ਸਾਰਣੀ, ਅੰਕ_Buckets);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = ਅੰਕ_Buckets - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ I = ਬਾਲਟੀ, J = I;
  ਜਦੋਂ ((J = (J + 1) & ਨਕਾਬ), The_ਸਾਰਣੀ[J]) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Ideal = Hash_ਸਾਰਣੀ[J];
    ਜੇ (((I - Ideal) & ਨਕਾਬ) < ((J - Ideal) & ਨਕਾਬ)) {
      The_ਸਾਰਣੀ[I] = The_ਸਾਰਣੀ[J];
      Hash_ਸਾਰਣੀ[I] = Hash_ਸਾਰਣੀ[J];
      I = J;
    }
  }
  The_ਸਾਰਣੀ[I] = ਖਾਲੀ_ਸੂਚਕ;
  --ਅੰਕ_ਚੀਜ਼ਾਂ;
}

String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *StringMapImpl::Remove_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ) {
  ਪੂਰਨ ਬਾਲਟੀ = ਲੱਭੋ_ਕੁੰਜੀ(ਕੁੰਜੀ);
  ਜੇ (ਬਾਲਟੀ == -1)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *ਨਤੀਜਾ = The_ਸਾਰਣੀ[ਬਾਲਟੀ];
  remove_ਬਾਲਟੀ(ਬਾਲਟੀ);
  ਵਾਪਸ ਨਤੀਜਾ;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
ਬਿਨਾਨਿਸ਼ਾਨੀ StringMapImpl::Rehash_ਸਾਰਣੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ_No) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਮਾਪ;
  // If the hash table is now more than 3/4 full, grow the table.
  ਜੇ (LLVM_UNLIKELY(ਅੰਕ_ਚੀਜ਼ਾਂ * 4 > ਅੰਕ_Buckets * 3)) {
    New_ਮਾਪ = ਅੰਕ_Buckets * 2;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ ਬਾਲਟੀ_No;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਬਾਲਟੀ_No = ਬਾਲਟੀ_No;
  ਸਵੈਚਾਲਿਤ **New_ਸਾਰਣੀ_Array = create_ਸਾਰਣੀ(New_ਮਾਪ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ *NewHashArray = get_Hash_ਸਾਰਣੀ(New_ਸਾਰਣੀ_Array, New_ਮਾਪ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ *Hash_ਸਾਰਣੀ = get_Hash_ਸਾਰਣੀ(The_ਸਾਰਣੀ, ਅੰਕ_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, E = ਅੰਕ_Buckets; I != E; ++I) {
    String_Map_ਇੰਦਰਾਜ_ਅਧਾਰ *ਬਾਲਟੀ = The_ਸਾਰਣੀ[I];
    ਜੇ (ਬਾਲਟੀ) {
      // If the bucket is not available, probe for a spot.
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਭਰਿਆ_Hash = Hash_ਸਾਰਣੀ[I];
      ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਬਾਲਟੀ = ਭਰਿਆ_Hash & (New_ਮਾਪ - 1);
      ਜਦੋਂ (New_ਸਾਰਣੀ_Array[New_ਬਾਲਟੀ])
        New_ਬਾਲਟੀ = (New_ਬਾਲਟੀ + 1) & (New_ਮਾਪ - 1);

      // Finally found a slot.  Fill it in.
      New_ਸਾਰਣੀ_Array[New_ਬਾਲਟੀ] = ਬਾਲਟੀ;
      NewHashArray[New_ਬਾਲਟੀ] = ਭਰਿਆ_Hash;
      ਜੇ (I == ਬਾਲਟੀ_No)
        New_ਬਾਲਟੀ_No = New_ਬਾਲਟੀ;
    }
  }

  ਮੁਕਤ(The_ਸਾਰਣੀ);

  The_ਸਾਰਣੀ = New_ਸਾਰਣੀ_Array;
  ਅੰਕ_Buckets = New_ਮਾਪ;
  ਵਾਪਸ New_ਬਾਲਟੀ_No;
}
