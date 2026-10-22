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

کارول نطاق llvm;

/// Returns the number of buckets to allocate to ensure that the DenseMap can
/// accommodate \p NumEntries without need to grow().
ساكن مضمن بینښه get_Min_لوښی_منزل_Reserve_For_Entries(بینښه عدد_Entries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  إذا (عدد_Entries == 0)
    أرجع 0;
  // +1 is required because of the strict equality.
  // For example if NumEntries is 48, we need to return 401.
  أرجع بل_Power_Of2(عدد_Entries * 4 / 3 + 1);
}

ساكن مضمن String_Map_ثبت_بنسټ **create_جدول(بینښه New_عدد_Buckets) {
  خپلکاری **جدول = static_cast<String_Map_ثبت_بنسټ **>(خوندي_calloc(
      New_عدد_Buckets + 1, حجم(String_Map_ثبت_بنسټ **) + حجم(بینښه)));

  // Allocate one extra bucket, set it to look filled so the iterators stop at
  // end.
  جدول[New_عدد_Buckets] = (String_Map_ثبت_بنسټ *)2;
  أرجع جدول;
}

ساكن مضمن بینښه *get_Hash_جدول(String_Map_ثبت_بنسټ **The_جدول,
                                     بینښه عدد_Buckets) {
  أرجع reinterpret_cast<بینښه *>(The_جدول + عدد_Buckets + 1);
}

uint32_t StringMapImpl::hash(StringRef کلی) { أرجع xxh3_64bits(کلی); }

StringMapImpl::StringMapImpl(بینښه Init_اندازه, بینښه توکی_اندازه)
    : توکی_اندازه_2(توکی_اندازه) {
  // If a size is specified, initialize the table with that many buckets.
  إذا (Init_اندازه) {
    // The table will grow when the number of entries reach 3/4 of the number of
    // buckets. To guarantee that "InitSize" number of entries can be inserted
    // in the table without growing, we allocate just what is needed here.
    init(get_Min_لوښی_منزل_Reserve_For_Entries(Init_اندازه));
  }
}

فراغ StringMapImpl::init(بینښه Init_اندازه) {
  أكد((Init_اندازه & (Init_اندازه - 1)) == 0 &&
         "Init Size must be a power of 2 or zero!");

  بینښه New_عدد_Buckets = Init_اندازه ? Init_اندازه : 16;
  عدد_توکي = 0;

  The_جدول = create_جدول(New_عدد_Buckets);

  // Set the member only if TheTable was successfully allocated
  عدد_Buckets = New_عدد_Buckets;
}

/// LookupBucketFor - Look up the bucket that the specified string should end
/// up in.  If it already exists as a key in the map, the Item pointer for the
/// specified bucket will be non-null.  Otherwise, it will be null.  In either
/// case, the FullHashValue field of the bucket will be set to the hash value
/// of the string.
بینښه StringMapImpl::Lookup_لوښی_For(StringRef Name,
                                        uint32_t ډک_Hash_ارزښت) {
#که_تعریف_شوی EXPENSIVE_CHECKS
  أكد(ډک_Hash_ارزښت == hash(Name));
#د_که_پای
  // Hash table unallocated so far?
  إذا (عدد_Buckets == 0)
    init(16);
  إذا constexpr (should_سرچپه_Iterate())
    ډک_Hash_ارزښت = ~ډک_Hash_ارزښت;
  بینښه لوښی_No = ډک_Hash_ارزښت & (عدد_Buckets - 1);
  بینښه *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  طالما (ریښتیا) {
    String_Map_ثبت_بنسټ *لوښی_توکی = The_جدول[لوښی_No];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    إذا (LLVM_LIKELY(!لوښی_توکی)) {
      Hash_جدول[لوښی_No] = ډک_Hash_ارزښت;
      أرجع لوښی_No;
    }

    إذا (LLVM_LIKELY(Hash_جدول[لوښی_No] == ډک_Hash_ارزښت)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because Name isn't necessarily
      // null-terminated!
      حرف *توکی_Str = (حرف *)لوښی_توکی + توکی_اندازه_2;
      إذا (Name == StringRef(توکی_Str, لوښی_توکی->get_کلی_اوږدتیا())) {
        // We found a match!
        أرجع لوښی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    لوښی_No = (لوښی_No + 1) & (عدد_Buckets - 1);
  }
}

/// FindKey - Look up the bucket that contains the specified key. If it exists
/// in the map, return the bucket number of the key.  Otherwise return -1.
/// This does not modify the map.
صحيح StringMapImpl::پیدا_کړه_کلی(StringRef کلی, uint32_t ډک_Hash_ارزښت) ثابت {
  إذا (عدد_Buckets == 0)
    أرجع -1; // Really empty table?
#که_تعریف_شوی EXPENSIVE_CHECKS
  أكد(ډک_Hash_ارزښت == hash(کلی));
#د_که_پای
  إذا constexpr (should_سرچپه_Iterate())
    ډک_Hash_ارزښت = ~ډک_Hash_ارزښت;
  بینښه لوښی_No = ډک_Hash_ارزښت & (عدد_Buckets - 1);
  بینښه *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  طالما (ریښتیا) {
    String_Map_ثبت_بنسټ *لوښی_توکی = The_جدول[لوښی_No];
    // If we found an empty bucket, this key isn't in the table yet, return.
    إذا (LLVM_LIKELY(!لوښی_توکی))
      أرجع -1;

    إذا (LLVM_LIKELY(Hash_جدول[لوښی_No] == ډک_Hash_ارزښت)) {
      // If the full hash value matches, check deeply for a match.  The common
      // case here is that we are only looking at the buckets (for item info
      // being non-null and for the full hash value) not at the items.  This
      // is important for cache locality.

      // Do the comparison like this because NameStart isn't necessarily
      // null-terminated!
      حرف *توکی_Str = (حرف *)لوښی_توکی + توکی_اندازه_2;
      إذا (کلی == StringRef(توکی_Str, لوښی_توکی->get_کلی_اوږدتیا())) {
        // We found a match!
        أرجع لوښی_No;
      }
    }

    // Okay, we didn't find the item.  Probe to the next bucket.
    لوښی_No = (لوښی_No + 1) & (عدد_Buckets - 1);
  }
}

/// RemoveKey - Remove the specified StringMapEntry from the table, but do not
/// delete it.  This aborts if the value isn't in the table.
فراغ StringMapImpl::Remove_کلی(String_Map_ثبت_بنسټ *V) {
  ثابت حرف *VStr = (حرف *)V + توکی_اندازه_2;
  String_Map_ثبت_بنسټ *V2 = Remove_کلی(StringRef(VStr, V->get_کلی_اوږدتیا()));
  (فراغ)V2;
  أكد(V == V2 && "Didn't find key?");
}

// Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
// whose probe path crosses the hole.
فراغ StringMapImpl::remove_لوښی(بینښه لوښی) {
  بینښه *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);
  بینښه ماسک = عدد_Buckets - 1;
  بینښه I = لوښی, J = I;
  طالما ((J = (J + 1) & ماسک), The_جدول[J]) {
    بینښه Ideal = Hash_جدول[J];
    إذا (((I - Ideal) & ماسک) < ((J - Ideal) & ماسک)) {
      The_جدول[I] = The_جدول[J];
      Hash_جدول[I] = Hash_جدول[J];
      I = J;
    }
  }
  The_جدول[I] = تش_اشاره;
  --عدد_توکي;
}

String_Map_ثبت_بنسټ *StringMapImpl::Remove_کلی(StringRef کلی) {
  صحيح لوښی = پیدا_کړه_کلی(کلی);
  إذا (لوښی == -1)
    أرجع تش_اشاره;

  String_Map_ثبت_بنسټ *پایله = The_جدول[لوښی];
  remove_لوښی(لوښی);
  أرجع پایله;
}

/// RehashTable - Grow the table, redistributing values into the buckets with
/// the appropriate mod-of-hashtable-size.
بینښه StringMapImpl::Rehash_جدول(بینښه لوښی_No) {
  بینښه New_اندازه;
  // If the hash table is now more than 3/4 full, grow the table.
  إذا (LLVM_UNLIKELY(عدد_توکي * 4 > عدد_Buckets * 3)) {
    New_اندازه = عدد_Buckets * 2;
  } وإلا {
    أرجع لوښی_No;
  }

  بینښه New_لوښی_No = لوښی_No;
  خپلکاری **New_جدول_Array = create_جدول(New_اندازه);
  بینښه *NewHashArray = get_Hash_جدول(New_جدول_Array, New_اندازه);
  بینښه *Hash_جدول = get_Hash_جدول(The_جدول, عدد_Buckets);

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to rehash any strings.
  لكل (بینښه I = 0, E = عدد_Buckets; I != E; ++I) {
    String_Map_ثبت_بنسټ *لوښی = The_جدول[I];
    إذا (لوښی) {
      // If the bucket is not available, probe for a spot.
      بینښه ډک_Hash = Hash_جدول[I];
      بینښه New_لوښی = ډک_Hash & (New_اندازه - 1);
      طالما (New_جدول_Array[New_لوښی])
        New_لوښی = (New_لوښی + 1) & (New_اندازه - 1);

      // Finally found a slot.  Fill it in.
      New_جدول_Array[New_لوښی] = لوښی;
      NewHashArray[New_لوښی] = ډک_Hash;
      إذا (I == لوښی_No)
        New_لوښی_No = New_لوښی;
    }
  }

  حرر(The_جدول);

  The_جدول = New_جدول_Array;
  عدد_Buckets = New_اندازه;
  أرجع New_لوښی_No;
}
