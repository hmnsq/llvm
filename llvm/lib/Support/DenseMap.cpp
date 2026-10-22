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

کارول نطاق llvm;
کارول نطاق llvm::densemap;
کارول نطاق llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
کينډۍ <size_t Fixed_اندازه, منطقي InlinePtrHash>
ساكن فراغ rehashLoop(فراغ *DstBuckets, کارول_شوی_T *Dst_کارول_شوی, بینښه ماسک,
                       ثابت فراغ *SrcBuckets, ثابت کارول_شوی_T *Src_کارول_شوی,
                       بینښه Src_عدد_Buckets, size_t Runtime_اندازه,
                       لوښی_Hasher Hasher) {
  ثابت size_t لوښی_اندازه = Fixed_اندازه ? Fixed_اندازه : Runtime_اندازه;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  for_Each_کارول_شوی(Src_کارول_شوی, Src_عدد_Buckets, [&](بینښه I) {
    ثابت حرف *Src_لوښی = Src + static_cast<size_t>(I) * لوښی_اندازه;
    بینښه Hash;
    إذا constexpr (InlinePtrHash) {
      فراغ *کلی;
      std::حافظه_نقل(&کلی, Src_لوښی, حجم(کلی));
      Hash = DenseMapInfo<فراغ *>::get_Hash_ارزښت(کلی);
    } وإلا {
      Hash = Hasher(Src_لوښی);
    }
    بینښه لوښی_No = Hash & ماسک;
    طالما (کارول_شوی(Dst_کارول_شوی, لوښی_No))
      لوښی_No = (لوښی_No + 1) & ماسک;
    std::حافظه_نقل(Dst + static_cast<size_t>(لوښی_No) * لوښی_اندازه, Src_لوښی,
                لوښی_اندازه);
    سېټ_کارول_شوی(Dst_کارول_شوی, لوښی_No);
  });
}

کينډۍ <منطقي InlinePtrHash>
ساكن فراغ rehash_By_اندازه(فراغ *Dst, کارول_شوی_T *Dst_کارول_شوی, بینښه ماسک,
                         ثابت فراغ *Src, ثابت کارول_شوی_T *Src_کارول_شوی,
                         بینښه Src_عدد_Buckets, size_t لوښی_اندازه,
                         لوښی_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (لوښی_اندازه) {
#تعریف_کړه REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, Dst_کارول_شوی, ماسک, Src, Src_کارول_شوی,      \
                                        Src_عدد_Buckets, لوښی_اندازه, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#تعریف_لغوه REHASH_CASE
  افتراضي:
    أرجع rehashLoop<0, InlinePtrHash>(Dst, Dst_کارول_شوی, ماسک, Src, Src_کارول_شوی,
                                        Src_عدد_Buckets, لوښی_اندازه, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, کارول_شوی_T *Dst_کارول_شوی,
                                         بینښه Dst_عدد_Buckets,
                                         ثابت فراغ *Src, ثابت کارول_شوی_T *Src_کارول_شوی,
                                         بینښه Src_عدد_Buckets,
                                         size_t لوښی_اندازه,
                                         لوښی_Hasher Hasher) {
  ثابت بینښه ماسک = Dst_عدد_Buckets - 1;
  إذا (!Hasher)
    أرجع rehash_By_اندازه<ریښتیا>(Dst, Dst_کارول_شوی, ماسک, Src, Src_کارول_شوی, Src_عدد_Buckets,
                              لوښی_اندازه, Hasher);
  أرجع rehash_By_اندازه<خطأ>(Dst, Dst_کارول_شوی, ماسک, Src, Src_کارول_شوی, Src_عدد_Buckets,
                             لوښی_اندازه, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت کارول_شوی_T *Old_کارول_شوی,
                                        بینښه Old_عدد_Buckets,
                                        بینښه New_عدد_Buckets,
                                        size_t لوښی_اندازه, size_t Align,
                                        لوښی_Hasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_بفر(alloc_بایټونه(لوښی_اندازه, New_عدد_Buckets), Align);
  کارول_شوی_T *New_کارول_شوی = کارول_شوی_For(Storage, لوښی_اندازه, New_عدد_Buckets);
  clear_کارول_شوی(New_کارول_شوی, New_عدد_Buckets);
  إذا (Old_عدد_Buckets) {
    rehashRelocatable(Storage, New_کارول_شوی, New_عدد_Buckets, OldBuckets, Old_کارول_شوی,
                      Old_عدد_Buckets, لوښی_اندازه, Hasher);
    إذا (FreeOld)
      deallocate_بفر(OldBuckets, alloc_بایټونه(لوښی_اندازه, Old_عدد_Buckets),
                        Align);
  }
  أرجع Storage;
}
