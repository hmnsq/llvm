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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ReverseIteration.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ get_Min_ಪಾತ್ರೆ_ಗಮ್ಯ_Reserve_For_Entries(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  ಆದರೆ (ಅಂಕೆ_Entries == 0)
    ಹಿಂತಿರುಗಿ 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  ಹಿಂತಿರುಗಿ ಮುಂದಿನ_Power_Of2(ಅಂಕೆ_Entries * 4 / 3 + 1);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ String_Map_ನಮೂದು_ತಳ **create_ಕೋಷ್ಟಕ(ಚಿಹ್ನೆರಹಿತ New_ಅಂಕೆ_Buckets) {
  ಸ್ವಯಂಚಾಲಿತ **ಕೋಷ್ಟಕ = static_cast<String_Map_ನಮೂದು_ತಳ **>(ಸುರಕ್ಷಿತ_calloc(
      New_ಅಂಕೆ_Buckets + 1, ಗಾತ್ರ(String_Map_ನಮೂದು_ತಳ **) + ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  ಕೋಷ್ಟಕ[New_ಅಂಕೆ_Buckets] = (String_Map_ನಮೂದು_ತಳ *)2;
  ಹಿಂತಿರುಗಿ ಕೋಷ್ಟಕ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ *get_Hash_ಕೋಷ್ಟಕ(String_Map_ನಮೂದು_ತಳ **The_ಕೋಷ್ಟಕ,
                                     ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Buckets) {
  ಹಿಂತಿರುಗಿ reinterpret_cast<ಚಿಹ್ನೆರಹಿತ *>(The_ಕೋಷ್ಟಕ + ಅಂಕೆ_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ಕೀಲಿ) { ಹಿಂತಿರುಗಿ xxh3_64bits(ಕೀಲಿ); }

StringMapImpl::StringMapImpl(ಚಿಹ್ನೆರಹಿತ Init_ಗಾತ್ರ, ಚಿಹ್ನೆರಹಿತ ಅಂಶ_ಗಾತ್ರ)
    : ಅಂಶ_ಗಾತ್ರ_2(ಅಂಶ_ಗಾತ್ರ) {
  // If a size is specified, initialize the table with that many buckets.
  ಆದರೆ (Init_ಗಾತ್ರ) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_ಪಾತ್ರೆ_ಗಮ್ಯ_Reserve_For_Entries(Init_ಗಾತ್ರ));
  }
}

ಶೂನ್ಯ StringMapImpl::init(ಚಿಹ್ನೆರಹಿತ Init_ಗಾತ್ರ) {
  ಪ್ರತಿಜ್ಞಾ((Init_ಗಾತ್ರ & (Init_ಗಾತ್ರ - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  ಚಿಹ್ನೆರಹಿತ New_ಅಂಕೆ_Buckets = Init_ಗಾತ್ರ ? Init_ಗಾತ್ರ : 16;
  ಅಂಕೆ_ವಸ್ತುಗಳು = 0;

  The_ಕೋಷ್ಟಕ = create_ಕೋಷ್ಟಕ(New_ಅಂಕೆ_Buckets);

  // Set the member only if TheTable was successfully allocated
  ಅಂಕೆ_Buckets = New_ಅಂಕೆ_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
ಚಿಹ್ನೆರಹಿತ StringMapImpl::Lookup_ಪಾತ್ರೆ_For(StringRef Name,
                                        uint32_t ತುಂಬಿದ_Hash_ಮೌಲ್ಯ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EXPENSIVE_CHECKS
  ಪ್ರತಿಜ್ಞಾ(ತುಂಬಿದ_Hash_ಮೌಲ್ಯ == hash(Name));
#ಆದರೆ_ಅಂತ್ಯ
  // Hash table unallocated so far?
  ಆದರೆ (ಅಂಕೆ_Buckets == 0)
    init(16);
  ಆದರೆ constexpr (should_ಹಿಮ್ಮುಖ_Iterate())
    ತುಂಬಿದ_Hash_ಮೌಲ್ಯ = ~ತುಂಬಿದ_Hash_ಮೌಲ್ಯ;
  ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ_No = ತುಂಬಿದ_Hash_ಮೌಲ್ಯ & (ಅಂಕೆ_Buckets - 1);
  ಚಿಹ್ನೆರಹಿತ *Hash_ಕೋಷ್ಟಕ = get_Hash_ಕೋಷ್ಟಕ(The_ಕೋಷ್ಟಕ, ಅಂಕೆ_Buckets);

  ಆಗ (ನಿಜ) {
    String_Map_ನಮೂದು_ತಳ *ಪಾತ್ರೆ_ಅಂಶ = The_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    ಆದರೆ (LLVM_LIKELY(!ಪಾತ್ರೆ_ಅಂಶ)) {
      Hash_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ_No] = ತುಂಬಿದ_Hash_ಮೌಲ್ಯ;
      ಹಿಂತಿರುಗಿ ಪಾತ್ರೆ_No;
    }

    ಆದರೆ (LLVM_LIKELY(Hash_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ_No] == ತುಂಬಿದ_Hash_ಮೌಲ್ಯ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      ಅಕ್ಷರ *ಅಂಶ_Str = (ಅಕ್ಷರ *)ಪಾತ್ರೆ_ಅಂಶ + ಅಂಶ_ಗಾತ್ರ_2;
      ಆದರೆ (Name == StringRef(ಅಂಶ_Str, ಪಾತ್ರೆ_ಅಂಶ->get_ಕೀಲಿ_ಉದ್ದಳತೆ())) {
        // We found a match!
        ಹಿಂತಿರುಗಿ ಪಾತ್ರೆ_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ಪಾತ್ರೆ_No = (ಪಾತ್ರೆ_No + 1) & (ಅಂಕೆ_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
ಪೂರ್ಣಾಂಕ StringMapImpl::ಕಂಡುಹಿಡಿ_ಕೀಲಿ(StringRef ಕೀಲಿ, uint32_t ತುಂಬಿದ_Hash_ಮೌಲ್ಯ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಅಂಕೆ_Buckets == 0)
    ಹಿಂತಿರುಗಿ -1; // Really empty table?
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EXPENSIVE_CHECKS
  ಪ್ರತಿಜ್ಞಾ(ತುಂಬಿದ_Hash_ಮೌಲ್ಯ == hash(ಕೀಲಿ));
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ constexpr (should_ಹಿಮ್ಮುಖ_Iterate())
    ತುಂಬಿದ_Hash_ಮೌಲ್ಯ = ~ತುಂಬಿದ_Hash_ಮೌಲ್ಯ;
  ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ_No = ತುಂಬಿದ_Hash_ಮೌಲ್ಯ & (ಅಂಕೆ_Buckets - 1);
  ಚಿಹ್ನೆರಹಿತ *Hash_ಕೋಷ್ಟಕ = get_Hash_ಕೋಷ್ಟಕ(The_ಕೋಷ್ಟಕ, ಅಂಕೆ_Buckets);

  ಆಗ (ನಿಜ) {
    String_Map_ನಮೂದು_ತಳ *ಪಾತ್ರೆ_ಅಂಶ = The_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    ಆದರೆ (LLVM_LIKELY(!ಪಾತ್ರೆ_ಅಂಶ))
      ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (LLVM_LIKELY(Hash_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ_No] == ತುಂಬಿದ_Hash_ಮೌಲ್ಯ)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      ಅಕ್ಷರ *ಅಂಶ_Str = (ಅಕ್ಷರ *)ಪಾತ್ರೆ_ಅಂಶ + ಅಂಶ_ಗಾತ್ರ_2;
      ಆದರೆ (ಕೀಲಿ == StringRef(ಅಂಶ_Str, ಪಾತ್ರೆ_ಅಂಶ->get_ಕೀಲಿ_ಉದ್ದಳತೆ())) {
        // We found a match!
        ಹಿಂತಿರುಗಿ ಪಾತ್ರೆ_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ಪಾತ್ರೆ_No = (ಪಾತ್ರೆ_No + 1) & (ಅಂಕೆ_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
ಶೂನ್ಯ StringMapImpl::Remove_ಕೀಲಿ(String_Map_ನಮೂದು_ತಳ *V) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *VStr = (ಅಕ್ಷರ *)V + ಅಂಶ_ಗಾತ್ರ_2;
  String_Map_ನಮೂದು_ತಳ *V2 = Remove_ಕೀಲಿ(StringRef(VStr, V->get_ಕೀಲಿ_ಉದ್ದಳತೆ()));
  (ಶೂನ್ಯ)V2;
  ಪ್ರತಿಜ್ಞಾ(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
ಶೂನ್ಯ StringMapImpl::remove_ಪಾತ್ರೆ(ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ) {
  ಚಿಹ್ನೆರಹಿತ *Hash_ಕೋಷ್ಟಕ = get_Hash_ಕೋಷ್ಟಕ(The_ಕೋಷ್ಟಕ, ಅಂಕೆ_Buckets);
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = ಅಂಕೆ_Buckets - 1;
  ಚಿಹ್ನೆರಹಿತ I = ಪಾತ್ರೆ, J = I;
  ಆಗ ((J = (J + 1) & ಮುಖವಾಡ), The_ಕೋಷ್ಟಕ[J]) {
    ಚಿಹ್ನೆರಹಿತ Ideal = Hash_ಕೋಷ್ಟಕ[J];
    ಆದರೆ (((I - Ideal) & ಮುಖವಾಡ) < ((J - Ideal) & ಮುಖವಾಡ)) {
      The_ಕೋಷ್ಟಕ[I] = The_ಕೋಷ್ಟಕ[J];
      Hash_ಕೋಷ್ಟಕ[I] = Hash_ಕೋಷ್ಟಕ[J];
      I = J;
    }
  }
  The_ಕೋಷ್ಟಕ[I] = ಶೂನ್ಯಸೂಚಕ;
  --ಅಂಕೆ_ವಸ್ತುಗಳು;
}

String_Map_ನಮೂದು_ತಳ *StringMapImpl::Remove_ಕೀಲಿ(StringRef ಕೀಲಿ) {
  ಪೂರ್ಣಾಂಕ ಪಾತ್ರೆ = ಕಂಡುಹಿಡಿ_ಕೀಲಿ(ಕೀಲಿ);
  ಆದರೆ (ಪಾತ್ರೆ == -1)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  String_Map_ನಮೂದು_ತಳ *ಫಲಿತಾಂಶ = The_ಕೋಷ್ಟಕ[ಪಾತ್ರೆ];
  remove_ಪಾತ್ರೆ(ಪಾತ್ರೆ);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
ಚಿಹ್ನೆರಹಿತ StringMapImpl::Rehash_ಕೋಷ್ಟಕ(ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ_No) {
  ಚಿಹ್ನೆರಹಿತ New_ಗಾತ್ರ;
  // If the hash table is now more than 3/4 full, grow the table.
  ಆದರೆ (LLVM_UNLIKELY(ಅಂಕೆ_ವಸ್ತುಗಳು * 4 > ಅಂಕೆ_Buckets * 3)) {
    New_ಗಾತ್ರ = ಅಂಕೆ_Buckets * 2;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ ಪಾತ್ರೆ_No;
  }

  ಚಿಹ್ನೆರಹಿತ New_ಪಾತ್ರೆ_No = ಪಾತ್ರೆ_No;
  ಸ್ವಯಂಚಾಲಿತ **New_ಕೋಷ್ಟಕ_Array = create_ಕೋಷ್ಟಕ(New_ಗಾತ್ರ);
  ಚಿಹ್ನೆರಹಿತ *NewHashArray = get_Hash_ಕೋಷ್ಟಕ(New_ಕೋಷ್ಟಕ_Array, New_ಗಾತ್ರ);
  ಚಿಹ್ನೆರಹಿತ *Hash_ಕೋಷ್ಟಕ = get_Hash_ಕೋಷ್ಟಕ(The_ಕೋಷ್ಟಕ, ಅಂಕೆ_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, E = ಅಂಕೆ_Buckets; I != E; ++I) {
    String_Map_ನಮೂದು_ತಳ *ಪಾತ್ರೆ = The_ಕೋಷ್ಟಕ[I];
    ಆದರೆ (ಪಾತ್ರೆ) {
      // If the bucket is not available, probe for a spot.
      ಚಿಹ್ನೆರಹಿತ ತುಂಬಿದ_Hash = Hash_ಕೋಷ್ಟಕ[I];
      ಚಿಹ್ನೆರಹಿತ New_ಪಾತ್ರೆ = ತುಂಬಿದ_Hash & (New_ಗಾತ್ರ - 1);
      ಆಗ (New_ಕೋಷ್ಟಕ_Array[New_ಪಾತ್ರೆ])
        New_ಪಾತ್ರೆ = (New_ಪಾತ್ರೆ + 1) & (New_ಗಾತ್ರ - 1);

      // Finally found a slot.  Fill it in.
      New_ಕೋಷ್ಟಕ_Array[New_ಪಾತ್ರೆ] = ಪಾತ್ರೆ;
      NewHashArray[New_ಪಾತ್ರೆ] = ತುಂಬಿದ_Hash;
      ಆದರೆ (I == ಪಾತ್ರೆ_No)
        New_ಪಾತ್ರೆ_No = New_ಪಾತ್ರೆ;
    }
  }

  ಬಿಡುಗಡೆ(The_ಕೋಷ್ಟಕ);

  The_ಕೋಷ್ಟಕ = New_ಕೋಷ್ಟಕ_Array;
  ಅಂಕೆ_Buckets = New_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ New_ಪಾತ್ರೆ_No;
}
