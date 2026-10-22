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

#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/ReverseIteration.h"

استعمال نامخلا llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ساکن اندرونی غیرعلامتی get_Min_بالٹی_مقصد_Reserve_For_Entries(غیرعلامتی عدد_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  اگر (عدد_Entries == 0)
    واپس 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  واپس اگلا_Power_Of2(عدد_Entries * 4 / 3 + 1);
}

ساکن اندرونی String_Map_اندراج_بنیاد **create_جدول(غیرعلامتی New_عدد_Buckets) {
  خودکار **جدول = static_cast<String_Map_اندراج_بنیاد **>(محفوظ_calloc(
      New_عدد_Buckets + 1, حجم(String_Map_اندراج_بنیاد **) + حجم(غیرعلامتی)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  جدول[New_عدد_Buckets] = (String_Map_اندراج_بنیاد *)2;
  واپس جدول;
}

ساکن اندرونی غیرعلامتی *get_Hash_جدول(String_Map_اندراج_بنیاد **The_جدول,
                                     غیرعلامتی عدد_Buckets) {
  واپس reinterpret_cast<غیرعلامتی *>(The_جدول + عدد_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef کلید) { واپس xxh3_64bits(کلید); }

StringMapImpl::StringMapImpl(غیرعلامتی Init_سائز, غیرعلامتی شے_سائز)
    : شے_سائز_2(شے_سائز) {
  // If a size is specified, initialize the table with that many buckets.
  اگر (Init_سائز) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_بالٹی_مقصد_Reserve_For_Entries(Init_سائز));
  }
}

خالی StringMapImpl::init(غیرعلامتی Init_سائز) {
  أكد((Init_سائز & (Init_سائز - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  غیرعلامتی New_عدد_Buckets = Init_سائز ? Init_سائز : 16;
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
غیرعلامتی StringMapImpl::Lookup_بالٹی_For(StringRef Name,
                                        uint32_t بھرا_Hash_قدر) {
#اگر_تعریف_شدہ EXPENSIVE_CHECKS
  أكد(بھرا_Hash_قدر == hash(Name));
#اگر_ختم
  // Hash table unallocated so far?
  اگر (عدد_Buckets == 0)
    init(16);
  اگر constexpr (should_الٹ_Iterate())
    بھرا_Hash_قدر = ~بھرا_Hash_قدر;
  غیرعلامتی بالٹی_No = بھرا_Hash_قدر & (عدد_Buckets - 1);
  غیرعلامتی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  جب (سچ) {
    String_Map_اندراج_بنیاد *بالٹی_شے = The_جدول[بالٹی_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    اگر (LLVM_LIKELY(!بالٹی_شے)) {
      Hash_جدول[بالٹی_No] = بھرا_Hash_قدر;
      واپس بالٹی_No;
    }

    اگر (LLVM_LIKELY(Hash_جدول[بالٹی_No] == بھرا_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *شے_Str = (حرف *)بالٹی_شے + شے_سائز_2;
      اگر (Name == StringRef(شے_Str, بالٹی_شے->get_کلید_لمبائی())) {
        // We found a match!
        واپس بالٹی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    بالٹی_No = (بالٹی_No + 1) & (عدد_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحیح StringMapImpl::ڈھونڈو_کلید(StringRef کلید, uint32_t بھرا_Hash_قدر) ثابت {
  اگر (عدد_Buckets == 0)
    واپس -1; // Really empty table?
#اگر_تعریف_شدہ EXPENSIVE_CHECKS
  أكد(بھرا_Hash_قدر == hash(کلید));
#اگر_ختم
  اگر constexpr (should_الٹ_Iterate())
    بھرا_Hash_قدر = ~بھرا_Hash_قدر;
  غیرعلامتی بالٹی_No = بھرا_Hash_قدر & (عدد_Buckets - 1);
  غیرعلامتی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  جب (سچ) {
    String_Map_اندراج_بنیاد *بالٹی_شے = The_جدول[بالٹی_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    اگر (LLVM_LIKELY(!بالٹی_شے))
      واپس -1;

    اگر (LLVM_LIKELY(Hash_جدول[بالٹی_No] == بھرا_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *شے_Str = (حرف *)بالٹی_شے + شے_سائز_2;
      اگر (کلید == StringRef(شے_Str, بالٹی_شے->get_کلید_لمبائی())) {
        // We found a match!
        واپس بالٹی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    بالٹی_No = (بالٹی_No + 1) & (عدد_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
خالی StringMapImpl::Remove_کلید(String_Map_اندراج_بنیاد *V) {
  ثابت حرف *VStr = (حرف *)V + شے_سائز_2;
  String_Map_اندراج_بنیاد *V2 = Remove_کلید(StringRef(VStr, V->get_کلید_لمبائی()));
  (خالی)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
خالی StringMapImpl::remove_بالٹی(غیرعلامتی بالٹی) {
  غیرعلامتی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);
  غیرعلامتی نقاب = عدد_Buckets - 1;
  غیرعلامتی I = بالٹی, J = I;
  جب ((J = (J + 1) & نقاب), The_جدول[J]) {
    غیرعلامتی Ideal = Hash_جدول[J];
    اگر (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      The_جدول[I] = The_جدول[J];
      Hash_جدول[I] = Hash_جدول[J];
      I = J;
    }
  }
  The_جدول[I] = خالی_اشاریہ;
  --عدد_اشیاء;
}

String_Map_اندراج_بنیاد *StringMapImpl::Remove_کلید(StringRef کلید) {
  صحیح بالٹی = ڈھونڈو_کلید(کلید);
  اگر (بالٹی == -1)
    واپس خالی_اشاریہ;

  String_Map_اندراج_بنیاد *نتیجہ = The_جدول[بالٹی];
  remove_بالٹی(بالٹی);
  واپس نتیجہ;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
غیرعلامتی StringMapImpl::Rehash_جدول(غیرعلامتی بالٹی_No) {
  غیرعلامتی New_سائز;
  // If the hash table is now more than 3/4 full, grow the table.
  اگر (LLVM_UNLIKELY(عدد_اشیاء * 4 > عدد_Buckets * 3)) {
    New_سائز = عدد_Buckets * 2;
  } ورنہ {
    واپس بالٹی_No;
  }

  غیرعلامتی New_بالٹی_No = بالٹی_No;
  خودکار **New_جدول_Array = create_جدول(New_سائز);
  غیرعلامتی *NewHashArray = get_Hash_جدول(New_جدول_Array, New_سائز);
  غیرعلامتی *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  برائے (غیرعلامتی I = 0, E = عدد_Buckets; I != E; ++I) {
    String_Map_اندراج_بنیاد *بالٹی = The_جدول[I];
    اگر (بالٹی) {
      // If the bucket is not available, probe for a spot.
      غیرعلامتی بھرا_Hash = Hash_جدول[I];
      غیرعلامتی New_بالٹی = بھرا_Hash & (New_سائز - 1);
      جب (New_جدول_Array[New_بالٹی])
        New_بالٹی = (New_بالٹی + 1) & (New_سائز - 1);

      // Finally found a slot.  Fill it in.
      New_جدول_Array[New_بالٹی] = بالٹی;
      NewHashArray[New_بالٹی] = بھرا_Hash;
      اگر (I == بالٹی_No)
        New_بالٹی_No = New_بالٹی;
    }
  }

  آزاد(The_جدول);

  The_جدول = New_جدول_Array;
  عدد_Buckets = New_سائز;
  واپس New_بالٹی_No;
}
