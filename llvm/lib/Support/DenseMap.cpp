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

استفاده نطاق llvm;
استفاده نطاق llvm::densemap;
استفاده نطاق llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
قالب <size_t Fixed_اندازه, منطقي InlinePtrHash>
ساكن فراغ rehashLoop(فراغ *DstBuckets, استفاده_شده_T *Dst_استفاده_شده, بیعلامت ماسک,
                       ثابت فراغ *SrcBuckets, ثابت استفاده_شده_T *Src_استفاده_شده,
                       بیعلامت Src_رقم_Buckets, size_t Runtime_اندازه,
                       سطل_Hasher Hasher) {
  ثابت size_t سطل_اندازه = Fixed_اندازه ? Fixed_اندازه : Runtime_اندازه;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  for_Each_استفاده_شده(Src_استفاده_شده, Src_رقم_Buckets, [&](بیعلامت I) {
    ثابت حرف *Src_سطل = Src + static_cast<size_t>(I) * سطل_اندازه;
    بیعلامت Hash;
    إذا constexpr (InlinePtrHash) {
      فراغ *کلید;
      std::کپی_حافظه(&کلید, Src_سطل, حجم(کلید));
      Hash = DenseMapInfo<فراغ *>::get_Hash_مقدار(کلید);
    } وإلا {
      Hash = Hasher(Src_سطل);
    }
    بیعلامت سطل_No = Hash & ماسک;
    طالما (استفاده_شده(Dst_استفاده_شده, سطل_No))
      سطل_No = (سطل_No + 1) & ماسک;
    std::کپی_حافظه(Dst + static_cast<size_t>(سطل_No) * سطل_اندازه, Src_سطل,
                سطل_اندازه);
    مجموعه_استفاده_شده(Dst_استفاده_شده, سطل_No);
  });
}

قالب <منطقي InlinePtrHash>
ساكن فراغ rehash_By_اندازه(فراغ *Dst, استفاده_شده_T *Dst_استفاده_شده, بیعلامت ماسک,
                         ثابت فراغ *Src, ثابت استفاده_شده_T *Src_استفاده_شده,
                         بیعلامت Src_رقم_Buckets, size_t سطل_اندازه,
                         سطل_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (سطل_اندازه) {
#تعریف_کن REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, Dst_استفاده_شده, ماسک, Src, Src_استفاده_شده,      \
                                        Src_رقم_Buckets, سطل_اندازه, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#لغو_تعریف REHASH_CASE
  افتراضي:
    أرجع rehashLoop<0, InlinePtrHash>(Dst, Dst_استفاده_شده, ماسک, Src, Src_استفاده_شده,
                                        Src_رقم_Buckets, سطل_اندازه, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, استفاده_شده_T *Dst_استفاده_شده,
                                         بیعلامت Dst_رقم_Buckets,
                                         ثابت فراغ *Src, ثابت استفاده_شده_T *Src_استفاده_شده,
                                         بیعلامت Src_رقم_Buckets,
                                         size_t سطل_اندازه,
                                         سطل_Hasher Hasher) {
  ثابت بیعلامت ماسک = Dst_رقم_Buckets - 1;
  إذا (!Hasher)
    أرجع rehash_By_اندازه<درست>(Dst, Dst_استفاده_شده, ماسک, Src, Src_استفاده_شده, Src_رقم_Buckets,
                              سطل_اندازه, Hasher);
  أرجع rehash_By_اندازه<خطأ>(Dst, Dst_استفاده_شده, ماسک, Src, Src_استفاده_شده, Src_رقم_Buckets,
                             سطل_اندازه, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت استفاده_شده_T *Old_استفاده_شده,
                                        بیعلامت Old_رقم_Buckets,
                                        بیعلامت New_رقم_Buckets,
                                        size_t سطل_اندازه, size_t Align,
                                        سطل_Hasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_میانگیر(alloc_بایتها(سطل_اندازه, New_رقم_Buckets), Align);
  استفاده_شده_T *New_استفاده_شده = استفاده_شده_For(Storage, سطل_اندازه, New_رقم_Buckets);
  clear_استفاده_شده(New_استفاده_شده, New_رقم_Buckets);
  إذا (Old_رقم_Buckets) {
    rehashRelocatable(Storage, New_استفاده_شده, New_رقم_Buckets, OldBuckets, Old_استفاده_شده,
                      Old_رقم_Buckets, سطل_اندازه, Hasher);
    إذا (FreeOld)
      deallocate_میانگیر(OldBuckets, alloc_بایتها(سطل_اندازه, Old_رقم_Buckets),
                        Align);
  }
  أرجع Storage;
}
