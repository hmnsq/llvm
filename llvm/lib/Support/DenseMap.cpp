//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/DenseMap.h"
#شامل "llvm/Support/MemAlloc.h"
#شامل <cstring>

استعمال نامخلا llvm;
استعمال نامخلا llvm::densemap;
استعمال نامخلا llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
سانچہ <size_t Fixed_سائز, منطقی InlinePtrHash>
ساکن خالی rehashLoop(خالی *DstBuckets, مستعمل_T *Dst_مستعمل, غیرعلامتی نقاب,
                       ثابت خالی *SrcBuckets, ثابت مستعمل_T *Src_مستعمل,
                       غیرعلامتی Src_عدد_Buckets, size_t Runtime_سائز,
                       بالٹی_Hasher Hasher) {
  ثابت size_t بالٹی_سائز = Fixed_سائز ? Fixed_سائز : Runtime_سائز;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  for_Each_مستعمل(Src_مستعمل, Src_عدد_Buckets, [&](غیرعلامتی I) {
    ثابت حرف *Src_بالٹی = Src + static_cast<size_t>(I) * بالٹی_سائز;
    غیرعلامتی Hash;
    اگر constexpr (InlinePtrHash) {
      خالی *کلید;
      std::حافظہ_نقل(&کلید, Src_بالٹی, حجم(کلید));
      Hash = DenseMapInfo<خالی *>::get_Hash_قدر(کلید);
    } ورنہ {
      Hash = Hasher(Src_بالٹی);
    }
    غیرعلامتی بالٹی_No = Hash & نقاب;
    جب (مستعمل(Dst_مستعمل, بالٹی_No))
      بالٹی_No = (بالٹی_No + 1) & نقاب;
    std::حافظہ_نقل(Dst + static_cast<size_t>(بالٹی_No) * بالٹی_سائز, Src_بالٹی,
                بالٹی_سائز);
    سیٹ_مستعمل(Dst_مستعمل, بالٹی_No);
  });
}

سانچہ <منطقی InlinePtrHash>
ساکن خالی rehash_By_سائز(خالی *Dst, مستعمل_T *Dst_مستعمل, غیرعلامتی نقاب,
                         ثابت خالی *Src, ثابت مستعمل_T *Src_مستعمل,
                         غیرعلامتی Src_عدد_Buckets, size_t بالٹی_سائز,
                         بالٹی_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  انتخاب (بالٹی_سائز) {
#تعریف_کرو REHASH_CASE(N)                                                         \
  حالت N:                                                                      \
    واپس rehashLoop<N, InlinePtrHash>(Dst, Dst_مستعمل, نقاب, Src, Src_مستعمل,      \
                                        Src_عدد_Buckets, بالٹی_سائز, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#تعریف_منسوخ REHASH_CASE
  طےشدہ:
    واپس rehashLoop<0, InlinePtrHash>(Dst, Dst_مستعمل, نقاب, Src, Src_مستعمل,
                                        Src_عدد_Buckets, بالٹی_سائز, Hasher);
  }
}

خالی densemap::detail::rehashRelocatable(خالی *Dst, مستعمل_T *Dst_مستعمل,
                                         غیرعلامتی Dst_عدد_Buckets,
                                         ثابت خالی *Src, ثابت مستعمل_T *Src_مستعمل,
                                         غیرعلامتی Src_عدد_Buckets,
                                         size_t بالٹی_سائز,
                                         بالٹی_Hasher Hasher) {
  ثابت غیرعلامتی نقاب = Dst_عدد_Buckets - 1;
  اگر (!Hasher)
    واپس rehash_By_سائز<سچ>(Dst, Dst_مستعمل, نقاب, Src, Src_مستعمل, Src_عدد_Buckets,
                              بالٹی_سائز, Hasher);
  واپس rehash_By_سائز<جھوٹ>(Dst, Dst_مستعمل, نقاب, Src, Src_مستعمل, Src_عدد_Buckets,
                             بالٹی_سائز, Hasher);
}

خالی *densemap::detail::growRelocatable(خالی *OldBuckets, ثابت مستعمل_T *Old_مستعمل,
                                        غیرعلامتی Old_عدد_Buckets,
                                        غیرعلامتی New_عدد_Buckets,
                                        size_t بالٹی_سائز, size_t Align,
                                        بالٹی_Hasher Hasher, منطقی FreeOld) {
  خالی *Storage = allocate_بفر(alloc_بائٹیں(بالٹی_سائز, New_عدد_Buckets), Align);
  مستعمل_T *New_مستعمل = مستعمل_For(Storage, بالٹی_سائز, New_عدد_Buckets);
  clear_مستعمل(New_مستعمل, New_عدد_Buckets);
  اگر (Old_عدد_Buckets) {
    rehashRelocatable(Storage, New_مستعمل, New_عدد_Buckets, OldBuckets, Old_مستعمل,
                      Old_عدد_Buckets, بالٹی_سائز, Hasher);
    اگر (FreeOld)
      deallocate_بفر(OldBuckets, alloc_بائٹیں(بالٹی_سائز, Old_عدد_Buckets),
                        Align);
  }
  واپس Storage;
}
