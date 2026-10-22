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

ورتوں نطاق llvm;
ورتوں نطاق llvm::densemap;
ورتوں نطاق llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
سانچا <size_t Fixed_ناپ, منطقي InlinePtrHash>
ساكن فراغ rehashLoop(فراغ *DstBuckets, ورتیا_T *Dst_ورتیا, بنانشانی نقاب,
                       ثابت فراغ *SrcBuckets, ثابت ورتیا_T *Src_ورتیا,
                       بنانشانی Src_عدد_Buckets, size_t Runtime_ناپ,
                       بالٹی_Hasher Hasher) {
  ثابت size_t بالٹی_ناپ = Fixed_ناپ ? Fixed_ناپ : Runtime_ناپ;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  for_Each_ورتیا(Src_ورتیا, Src_عدد_Buckets, [&](بنانشانی I) {
    ثابت حرف *Src_بالٹی = Src + static_cast<size_t>(I) * بالٹی_ناپ;
    بنانشانی Hash;
    إذا constexpr (InlinePtrHash) {
      فراغ *کلید;
      std::حافظہ_نقل(&کلید, Src_بالٹی, حجم(کلید));
      Hash = DenseMapInfo<فراغ *>::get_Hash_قدر(کلید);
    } وإلا {
      Hash = Hasher(Src_بالٹی);
    }
    بنانشانی بالٹی_No = Hash & نقاب;
    طالما (ورتیا(Dst_ورتیا, بالٹی_No))
      بالٹی_No = (بالٹی_No + 1) & نقاب;
    std::حافظہ_نقل(Dst + static_cast<size_t>(بالٹی_No) * بالٹی_ناپ, Src_بالٹی,
                بالٹی_ناپ);
    سیٹ_ورتیا(Dst_ورتیا, بالٹی_No);
  });
}

سانچا <منطقي InlinePtrHash>
ساكن فراغ rehash_By_ناپ(فراغ *Dst, ورتیا_T *Dst_ورتیا, بنانشانی نقاب,
                         ثابت فراغ *Src, ثابت ورتیا_T *Src_ورتیا,
                         بنانشانی Src_عدد_Buckets, size_t بالٹی_ناپ,
                         بالٹی_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (بالٹی_ناپ) {
#تعریف_کرو REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, Dst_ورتیا, نقاب, Src, Src_ورتیا,      \
                                        Src_عدد_Buckets, بالٹی_ناپ, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#تعریف_رد REHASH_CASE
  افتراضي:
    أرجع rehashLoop<0, InlinePtrHash>(Dst, Dst_ورتیا, نقاب, Src, Src_ورتیا,
                                        Src_عدد_Buckets, بالٹی_ناپ, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, ورتیا_T *Dst_ورتیا,
                                         بنانشانی Dst_عدد_Buckets,
                                         ثابت فراغ *Src, ثابت ورتیا_T *Src_ورتیا,
                                         بنانشانی Src_عدد_Buckets,
                                         size_t بالٹی_ناپ,
                                         بالٹی_Hasher Hasher) {
  ثابت بنانشانی نقاب = Dst_عدد_Buckets - 1;
  إذا (!Hasher)
    أرجع rehash_By_ناپ<سچ>(Dst, Dst_ورتیا, نقاب, Src, Src_ورتیا, Src_عدد_Buckets,
                              بالٹی_ناپ, Hasher);
  أرجع rehash_By_ناپ<خطأ>(Dst, Dst_ورتیا, نقاب, Src, Src_ورتیا, Src_عدد_Buckets,
                             بالٹی_ناپ, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت ورتیا_T *Old_ورتیا,
                                        بنانشانی Old_عدد_Buckets,
                                        بنانشانی New_عدد_Buckets,
                                        size_t بالٹی_ناپ, size_t Align,
                                        بالٹی_Hasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_بفر(alloc_بائٹاں(بالٹی_ناپ, New_عدد_Buckets), Align);
  ورتیا_T *New_ورتیا = ورتیا_For(Storage, بالٹی_ناپ, New_عدد_Buckets);
  clear_ورتیا(New_ورتیا, New_عدد_Buckets);
  إذا (Old_عدد_Buckets) {
    rehashRelocatable(Storage, New_ورتیا, New_عدد_Buckets, OldBuckets, Old_ورتیا,
                      Old_عدد_Buckets, بالٹی_ناپ, Hasher);
    إذا (FreeOld)
      deallocate_بفر(OldBuckets, alloc_بائٹاں(بالٹی_ناپ, Old_عدد_Buckets),
                        Align);
  }
  أرجع Storage;
}
