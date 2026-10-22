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

#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/ReverseIteration.h"

ورتوں نطاق llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ساكن مضمن بنانشانی get_Min_بالٹی_منزل_Reserve_For_Entries(بنانشانی عدد_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  إذا (عدد_Entries == 0)
    أرجع 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  أرجع اگلا_Power_Of2(عدد_Entries * 4 / 3 + 1);
}

ساكن مضمن String_Map_اندراج_ادھار **create_جدول(بنانشانی New_عدد_Buckets) {
  خودکار **جدول = static_cast<String_Map_اندراج_ادھار **>(محفوظ_calloc(
      New_عدد_Buckets + 1, حجم(String_Map_اندراج_ادھار **) + حجم(بنانشانی)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  جدول[New_عدد_Buckets] = (String_Map_اندراج_ادھار *)2;
  أرجع جدول;
}

ساكن مضمن بنانشانی *get_Hash_جدول(String_Map_اندراج_ادھار **The_جدول,
                                     بنانشانی عدد_Buckets) {
  أرجع reinterpret_cast<بنانشانی *>(The_جدول + عدد_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef کلید) { أرجع xxh3_64bits(کلید); }

StringMapImpl::StringMapImpl(بنانشانی Init_ناپ, بنانشانی شے_ناپ)
    : شے_ناپ_2(شے_ناپ) {
  // If a size is specified, initialize the table with that many buckets.
  إذا (Init_ناپ) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_بالٹی_منزل_Reserve_For_Entries(Init_ناپ));
  }
}

فراغ StringMapImpl::init(بنانشانی Init_ناپ) {
  أكد((Init_ناپ & (Init_ناپ - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  بنانشانی New_عدد_Buckets = Init_ناپ ? Init_ناپ : 16;
  عدد_اشیاء = 0;

  The_جدول = create_جدول(New_عدد_Buckets);

  // Set the member only if TheTable was successfully allocated
  عدد_Buckets = New_عدد_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
بنانشانی StringMapImpl::Lookup_بالٹی_For(StringRef Name,
                                        uint32_t بھریا_Hash_قدر) {
#جے_تعریف_شدہ EXPENSIVE_CHECKS
  أكد(بھریا_Hash_قدر == hash(Name));
#جے_انت
  // Hash table unallocated so far?
  إذا (عدد_Buckets == 0)
    init(16);
  إذا constexpr (should_الٹ_Iterate())
    بھریا_Hash_قدر = ~بھریا_Hash_قدر;
  بنانشانی بالٹی_No = بھریا_Hash_قدر & (عدد_Buckets - 1);
  بنانشانی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  طالما (سچ) {
    String_Map_اندراج_ادھار *بالٹی_شے = The_جدول[بالٹی_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    إذا (LLVM_LIKELY(!بالٹی_شے)) {
      Hash_جدول[بالٹی_No] = بھریا_Hash_قدر;
      أرجع بالٹی_No;
    }

    إذا (LLVM_LIKELY(Hash_جدول[بالٹی_No] == بھریا_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *شے_Str = (حرف *)بالٹی_شے + شے_ناپ_2;
      إذا (Name == StringRef(شے_Str, بالٹی_شے->get_کلید_لمائی())) {
        // We found a match!
        أرجع بالٹی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    بالٹی_No = (بالٹی_No + 1) & (عدد_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحيح StringMapImpl::لبھو_کلید(StringRef کلید, uint32_t بھریا_Hash_قدر) ثابت {
  إذا (عدد_Buckets == 0)
    أرجع -1; // Really empty table?
#جے_تعریف_شدہ EXPENSIVE_CHECKS
  أكد(بھریا_Hash_قدر == hash(کلید));
#جے_انت
  إذا constexpr (should_الٹ_Iterate())
    بھریا_Hash_قدر = ~بھریا_Hash_قدر;
  بنانشانی بالٹی_No = بھریا_Hash_قدر & (عدد_Buckets - 1);
  بنانشانی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  طالما (سچ) {
    String_Map_اندراج_ادھار *بالٹی_شے = The_جدول[بالٹی_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    إذا (LLVM_LIKELY(!بالٹی_شے))
      أرجع -1;

    إذا (LLVM_LIKELY(Hash_جدول[بالٹی_No] == بھریا_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *شے_Str = (حرف *)بالٹی_شے + شے_ناپ_2;
      إذا (کلید == StringRef(شے_Str, بالٹی_شے->get_کلید_لمائی())) {
        // We found a match!
        أرجع بالٹی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    بالٹی_No = (بالٹی_No + 1) & (عدد_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
فراغ StringMapImpl::Remove_کلید(String_Map_اندراج_ادھار *V) {
  ثابت حرف *VStr = (حرف *)V + شے_ناپ_2;
  String_Map_اندراج_ادھار *V2 = Remove_کلید(StringRef(VStr, V->get_کلید_لمائی()));
  (فراغ)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
فراغ StringMapImpl::remove_بالٹی(بنانشانی بالٹی) {
  بنانشانی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);
  بنانشانی نقاب = عدد_Buckets - 1;
  بنانشانی I = بالٹی, J = I;
  طالما ((J = (J + 1) & نقاب), The_جدول[J]) {
    بنانشانی Ideal = Hash_جدول[J];
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      The_جدول[I] = The_جدول[J];
      Hash_جدول[I] = Hash_جدول[J];
      I = J;
    }
  }
  The_جدول[I] = خالی_سوچک;
  --عدد_اشیاء;
}

String_Map_اندراج_ادھار *StringMapImpl::Remove_کلید(StringRef کلید) {
  صحيح بالٹی = لبھو_کلید(کلید);
  إذا (بالٹی == -1)
    أرجع خالی_سوچک;

  String_Map_اندراج_ادھار *نتیجہ = The_جدول[بالٹی];
  remove_بالٹی(بالٹی);
  أرجع نتیجہ;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
بنانشانی StringMapImpl::Rehash_جدول(بنانشانی بالٹی_No) {
  بنانشانی New_ناپ;
  // If the hash table is now more than 3/4 full, grow the table.
  إذا (LLVM_UNLIKELY(عدد_اشیاء * 4 > عدد_Buckets * 3)) {
    New_ناپ = عدد_Buckets * 2;
  } وإلا {
    أرجع بالٹی_No;
  }

  بنانشانی New_بالٹی_No = بالٹی_No;
  خودکار **New_جدول_Array = create_جدول(New_ناپ);
  بنانشانی *NewHashArray = get_Hash_جدول(New_جدول_Array, New_ناپ);
  بنانشانی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  لكل (بنانشانی I = 0, E = عدد_Buckets; I != E; ++I) {
    String_Map_اندراج_ادھار *بالٹی = The_جدول[I];
    إذا (بالٹی) {
      // If the bucket is not available, probe for a spot.
      بنانشانی بھریا_Hash = Hash_جدول[I];
      بنانشانی New_بالٹی = بھریا_Hash & (New_ناپ - 1);
      طالما (New_جدول_Array[New_بالٹی])
        New_بالٹی = (New_بالٹی + 1) & (New_ناپ - 1);

      // Finally found a slot.  Fill it in.
      New_جدول_Array[New_بالٹی] = بالٹی;
      NewHashArray[New_بالٹی] = بھریا_Hash;
      إذا (I == بالٹی_No)
        New_بالٹی_No = New_بالٹی;
    }
  }

  حرر(The_جدول);

  The_جدول = New_جدول_Array;
  عدد_Buckets = New_ناپ;
  أرجع New_بالٹی_No;
}
