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

using نطاق llvm;
using نطاق llvm::densemap;
using نطاق llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, منطقي InlinePtrHash>
ساكن فراغ rehashLoop(فراغ *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ثابت فراغ *SrcBuckets, ثابت UsedT *SrcUsed,
                       unsigned Src_ژمارە_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ثابت size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_ژمارە_Buckets, [&](unsigned I) {
    ثابت حرف *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    إذا constexpr (InlinePtrHash) {
      فراغ *کلیل;
      std::memcpy(&کلیل, SrcBucket, حجم(کلیل));
      Hash = DenseMapInfo<فراغ *>::get_Hash_بەها(کلیل);
    } وإلا {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    طالما (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <منطقي InlinePtrHash>
ساكن فراغ rehashBySize(فراغ *Dst, UsedT *DstUsed, unsigned Mask,
                         ثابت فراغ *Src, ثابت UsedT *SrcUsed,
                         unsigned Src_ژمارە_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (BucketSize) {
#define REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_ژمارە_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  افتراضي:
    أرجع rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_ژمارە_Buckets, BucketSize, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, UsedT *DstUsed,
                                         unsigned Dst_ژمارە_Buckets,
                                         ثابت فراغ *Src, ثابت UsedT *SrcUsed,
                                         unsigned Src_ژمارە_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ثابت unsigned Mask = Dst_ژمارە_Buckets - 1;
  إذا (!Hasher)
    أرجع rehashBySize<true>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ژمارە_Buckets,
                              BucketSize, Hasher);
  أرجع rehashBySize<خطأ>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ژمارە_Buckets,
                             BucketSize, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت UsedT *OldUsed,
                                        unsigned Old_ژمارە_Buckets,
                                        unsigned New_ژمارە_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_buffer(allocBytes(BucketSize, New_ژمارە_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_ژمارە_Buckets);
  clearUsed(NewUsed, New_ژمارە_Buckets);
  إذا (Old_ژمارە_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_ژمارە_Buckets, OldBuckets, OldUsed,
                      Old_ژمارە_Buckets, BucketSize, Hasher);
    إذا (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_ژمارە_Buckets),
                        Align);
  }
  أرجع Storage;
}
