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

استعمال نطاق llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ساكن مضمن بنانشان get_Min_ٻالٽي_منزل_Reserve_For_Entries(بنانشان انگ_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  إذا (انگ_Entries == 0)
    أرجع 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  أرجع ايندڙ_Power_Of2(انگ_Entries * 4 / 3 + 1);
}

ساكن مضمن String_Map_داخلا_بنياد **create_جدول(بنانشان New_انگ_Buckets) {
  خودڪار **جدول = static_cast<String_Map_داخلا_بنياد **>(محفوظ_calloc(
      New_انگ_Buckets + 1, حجم(String_Map_داخلا_بنياد **) + حجم(بنانشان)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  جدول[New_انگ_Buckets] = (String_Map_داخلا_بنياد *)2;
  أرجع جدول;
}

ساكن مضمن بنانشان *get_Hash_جدول(String_Map_داخلا_بنياد **The_جدول,
                                     بنانشان انگ_Buckets) {
  أرجع reinterpret_cast<بنانشان *>(The_جدول + انگ_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef ڪنجي) { أرجع xxh3_64bits(ڪنجي); }

StringMapImpl::StringMapImpl(بنانشان Init_سائيز, بنانشان شي_سائيز)
    : شي_سائيز_2(شي_سائيز) {
  // If a size is specified, initialize the table with that many buckets.
  إذا (Init_سائيز) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_ٻالٽي_منزل_Reserve_For_Entries(Init_سائيز));
  }
}

فراغ StringMapImpl::init(بنانشان Init_سائيز) {
  أكد((Init_سائيز & (Init_سائيز - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  بنانشان New_انگ_Buckets = Init_سائيز ? Init_سائيز : 16;
  انگ_شيون = 0;

  The_جدول = create_جدول(New_انگ_Buckets);

  // Set the member only if TheTable was successfully allocated
  انگ_Buckets = New_انگ_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
بنانشان StringMapImpl::Lookup_ٻالٽي_For(StringRef Name,
                                        uint32_t ڀريل_Hash_قدر) {
#جيڪڏهن_وصف_ٿيل EXPENSIVE_CHECKS
  أكد(ڀريل_Hash_قدر == hash(Name));
#جيڪڏهن_پورو
  // Hash table unallocated so far?
  إذا (انگ_Buckets == 0)
    init(16);
  إذا constexpr (should_ابتو_Iterate())
    ڀريل_Hash_قدر = ~ڀريل_Hash_قدر;
  بنانشان ٻالٽي_No = ڀريل_Hash_قدر & (انگ_Buckets - 1);
  بنانشان *Hash_جدول = get_Hash_جدول(The_جدول, انگ_Buckets);

  طالما (سچ) {
    String_Map_داخلا_بنياد *ٻالٽي_شي = The_جدول[ٻالٽي_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    إذا (LLVM_LIKELY(!ٻالٽي_شي)) {
      Hash_جدول[ٻالٽي_No] = ڀريل_Hash_قدر;
      أرجع ٻالٽي_No;
    }

    إذا (LLVM_LIKELY(Hash_جدول[ٻالٽي_No] == ڀريل_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *شي_Str = (حرف *)ٻالٽي_شي + شي_سائيز_2;
      إذا (Name == StringRef(شي_Str, ٻالٽي_شي->get_ڪنجي_ڊيگهائي())) {
        // We found a match!
        أرجع ٻالٽي_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ٻالٽي_No = (ٻالٽي_No + 1) & (انگ_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحيح StringMapImpl::ڳوليو_ڪنجي(StringRef ڪنجي, uint32_t ڀريل_Hash_قدر) ثابت {
  إذا (انگ_Buckets == 0)
    أرجع -1; // Really empty table?
#جيڪڏهن_وصف_ٿيل EXPENSIVE_CHECKS
  أكد(ڀريل_Hash_قدر == hash(ڪنجي));
#جيڪڏهن_پورو
  إذا constexpr (should_ابتو_Iterate())
    ڀريل_Hash_قدر = ~ڀريل_Hash_قدر;
  بنانشان ٻالٽي_No = ڀريل_Hash_قدر & (انگ_Buckets - 1);
  بنانشان *Hash_جدول = get_Hash_جدول(The_جدول, انگ_Buckets);

  طالما (سچ) {
    String_Map_داخلا_بنياد *ٻالٽي_شي = The_جدول[ٻالٽي_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    إذا (LLVM_LIKELY(!ٻالٽي_شي))
      أرجع -1;

    إذا (LLVM_LIKELY(Hash_جدول[ٻالٽي_No] == ڀريل_Hash_قدر)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *شي_Str = (حرف *)ٻالٽي_شي + شي_سائيز_2;
      إذا (ڪنجي == StringRef(شي_Str, ٻالٽي_شي->get_ڪنجي_ڊيگهائي())) {
        // We found a match!
        أرجع ٻالٽي_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    ٻالٽي_No = (ٻالٽي_No + 1) & (انگ_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
فراغ StringMapImpl::Remove_ڪنجي(String_Map_داخلا_بنياد *V) {
  ثابت حرف *VStr = (حرف *)V + شي_سائيز_2;
  String_Map_داخلا_بنياد *V2 = Remove_ڪنجي(StringRef(VStr, V->get_ڪنجي_ڊيگهائي()));
  (فراغ)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
فراغ StringMapImpl::remove_ٻالٽي(بنانشان ٻالٽي) {
  بنانشان *Hash_جدول = get_Hash_جدول(The_جدول, انگ_Buckets);
  بنانشان نقاب = انگ_Buckets - 1;
  بنانشان I = ٻالٽي, J = I;
  طالما ((J = (J + 1) & نقاب), The_جدول[J]) {
    بنانشان Ideal = Hash_جدول[J];
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      The_جدول[I] = The_جدول[J];
      Hash_جدول[I] = Hash_جدول[J];
      I = J;
    }
  }
  The_جدول[I] = خالي_اشارو;
  --انگ_شيون;
}

String_Map_داخلا_بنياد *StringMapImpl::Remove_ڪنجي(StringRef ڪنجي) {
  صحيح ٻالٽي = ڳوليو_ڪنجي(ڪنجي);
  إذا (ٻالٽي == -1)
    أرجع خالي_اشارو;

  String_Map_داخلا_بنياد *نتيجو = The_جدول[ٻالٽي];
  remove_ٻالٽي(ٻالٽي);
  أرجع نتيجو;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
بنانشان StringMapImpl::Rehash_جدول(بنانشان ٻالٽي_No) {
  بنانشان New_سائيز;
  // If the hash table is now more than 3/4 full, grow the table.
  إذا (LLVM_UNLIKELY(انگ_شيون * 4 > انگ_Buckets * 3)) {
    New_سائيز = انگ_Buckets * 2;
  } وإلا {
    أرجع ٻالٽي_No;
  }

  بنانشان New_ٻالٽي_No = ٻالٽي_No;
  خودڪار **New_جدول_Array = create_جدول(New_سائيز);
  بنانشان *NewHashArray = get_Hash_جدول(New_جدول_Array, New_سائيز);
  بنانشان *Hash_جدول = get_Hash_جدول(The_جدول, انگ_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  لكل (بنانشان I = 0, E = انگ_Buckets; I != E; ++I) {
    String_Map_داخلا_بنياد *ٻالٽي = The_جدول[I];
    إذا (ٻالٽي) {
      // If the bucket is not available, probe for a spot.
      بنانشان ڀريل_Hash = Hash_جدول[I];
      بنانشان New_ٻالٽي = ڀريل_Hash & (New_سائيز - 1);
      طالما (New_جدول_Array[New_ٻالٽي])
        New_ٻالٽي = (New_ٻالٽي + 1) & (New_سائيز - 1);

      // Finally found a slot.  Fill it in.
      New_جدول_Array[New_ٻالٽي] = ٻالٽي;
      NewHashArray[New_ٻالٽي] = ڀريل_Hash;
      إذا (I == ٻالٽي_No)
        New_ٻالٽي_No = New_ٻالٽي;
    }
  }

  حرر(The_جدول);

  The_جدول = New_جدول_Array;
  انگ_Buckets = New_سائيز;
  أرجع New_ٻالٽي_No;
}
