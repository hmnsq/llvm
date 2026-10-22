//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/DenseMap.h"
#ضمن "llvm/Support/MemAlloc.h"
#ضمن <cstring>

استعمال نطاق llvm;
استعمال نطاق llvm::densemap;
استعمال نطاق llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
سانچو <size_t Fixed_سائيز, منطقي InlinePtrHash>
ساكن فراغ rehashLoop(فراغ *DstBuckets, استعمال_ٿيل_T *Dst_استعمال_ٿيل, بنانشان نقاب,
                       ثابت فراغ *SrcBuckets, ثابت استعمال_ٿيل_T *Src_استعمال_ٿيل,
                       بنانشان Src_انگ_Buckets, size_t Runtime_سائيز,
                       ٻالٽي_Hasher Hasher) {
  ثابت size_t ٻالٽي_سائيز = Fixed_سائيز ? Fixed_سائيز : Runtime_سائيز;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  for_Each_استعمال_ٿيل(Src_استعمال_ٿيل, Src_انگ_Buckets, [&](بنانشان I) {
    ثابت حرف *Src_ٻالٽي = Src + static_cast<size_t>(I) * ٻالٽي_سائيز;
    بنانشان Hash;
    إذا constexpr (InlinePtrHash) {
      فراغ *ڪنجي;
      std::يادگيري_نقل(&ڪنجي, Src_ٻالٽي, حجم(ڪنجي));
      Hash = DenseMapInfo<فراغ *>::get_Hash_قدر(ڪنجي);
    } وإلا {
      Hash = Hasher(Src_ٻالٽي);
    }
    بنانشان ٻالٽي_No = Hash & نقاب;
    طالما (استعمال_ٿيل(Dst_استعمال_ٿيل, ٻالٽي_No))
      ٻالٽي_No = (ٻالٽي_No + 1) & نقاب;
    std::يادگيري_نقل(Dst + static_cast<size_t>(ٻالٽي_No) * ٻالٽي_سائيز, Src_ٻالٽي,
                ٻالٽي_سائيز);
    سيٽ_استعمال_ٿيل(Dst_استعمال_ٿيل, ٻالٽي_No);
  });
}

سانچو <منطقي InlinePtrHash>
ساكن فراغ rehash_By_سائيز(فراغ *Dst, استعمال_ٿيل_T *Dst_استعمال_ٿيل, بنانشان نقاب,
                         ثابت فراغ *Src, ثابت استعمال_ٿيل_T *Src_استعمال_ٿيل,
                         بنانشان Src_انگ_Buckets, size_t ٻالٽي_سائيز,
                         ٻالٽي_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (ٻالٽي_سائيز) {
#وصف_ڪريو REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, Dst_استعمال_ٿيل, نقاب, Src, Src_استعمال_ٿيل,      \
                                        Src_انگ_Buckets, ٻالٽي_سائيز, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#وصف_رد REHASH_CASE
  افتراضي:
    أرجع rehashLoop<0, InlinePtrHash>(Dst, Dst_استعمال_ٿيل, نقاب, Src, Src_استعمال_ٿيل,
                                        Src_انگ_Buckets, ٻالٽي_سائيز, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, استعمال_ٿيل_T *Dst_استعمال_ٿيل,
                                         بنانشان Dst_انگ_Buckets,
                                         ثابت فراغ *Src, ثابت استعمال_ٿيل_T *Src_استعمال_ٿيل,
                                         بنانشان Src_انگ_Buckets,
                                         size_t ٻالٽي_سائيز,
                                         ٻالٽي_Hasher Hasher) {
  ثابت بنانشان نقاب = Dst_انگ_Buckets - 1;
  إذا (!Hasher)
    أرجع rehash_By_سائيز<سچ>(Dst, Dst_استعمال_ٿيل, نقاب, Src, Src_استعمال_ٿيل, Src_انگ_Buckets,
                              ٻالٽي_سائيز, Hasher);
  أرجع rehash_By_سائيز<خطأ>(Dst, Dst_استعمال_ٿيل, نقاب, Src, Src_استعمال_ٿيل, Src_انگ_Buckets,
                             ٻالٽي_سائيز, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت استعمال_ٿيل_T *Old_استعمال_ٿيل,
                                        بنانشان Old_انگ_Buckets,
                                        بنانشان New_انگ_Buckets,
                                        size_t ٻالٽي_سائيز, size_t Align,
                                        ٻالٽي_Hasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_بفر(alloc_بائيٽون(ٻالٽي_سائيز, New_انگ_Buckets), Align);
  استعمال_ٿيل_T *New_استعمال_ٿيل = استعمال_ٿيل_For(Storage, ٻالٽي_سائيز, New_انگ_Buckets);
  clear_استعمال_ٿيل(New_استعمال_ٿيل, New_انگ_Buckets);
  إذا (Old_انگ_Buckets) {
    rehashRelocatable(Storage, New_استعمال_ٿيل, New_انگ_Buckets, OldBuckets, Old_استعمال_ٿيل,
                      Old_انگ_Buckets, ٻالٽي_سائيز, Hasher);
    إذا (FreeOld)
      deallocate_بفر(OldBuckets, alloc_بائيٽون(ٻالٽي_سائيز, Old_انگ_Buckets),
                        Align);
  }
  أرجع Storage;
}
