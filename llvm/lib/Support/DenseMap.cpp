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

using فضاینام llvm;
using فضاینام llvm::densemap;
using فضاینام llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, منطقی InlinePtrHash>
ایستا تهی rehashLoop(تهی *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ثابت تهی *SrcBuckets, ثابت UsedT *SrcUsed,
                       unsigned Src_رقم_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ثابت size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  نویسه *Dst = static_cast<نویسه *>(DstBuckets);
  ثابت نویسه *Src = static_cast<ثابت نویسه *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_رقم_Buckets, [&](unsigned I) {
    ثابت نویسه *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    اگر constexpr (InlinePtrHash) {
      تهی *کلید;
      std::memcpy(&کلید, SrcBucket, اندازه(کلید));
      Hash = DenseMapInfo<تهی *>::get_Hash_مقدار(کلید);
    } وگرنه {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    وقتی (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <منطقی InlinePtrHash>
ایستا تهی rehashBySize(تهی *Dst, UsedT *DstUsed, unsigned Mask,
                         ثابت تهی *Src, ثابت UsedT *SrcUsed,
                         unsigned Src_رقم_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  انتخاب (BucketSize) {
#define REHASH_CASE(N)                                                         \
  حالت N:                                                                      \
    بازگشت rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_رقم_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  پیشفرض:
    بازگشت rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_رقم_Buckets, BucketSize, Hasher);
  }
}

تهی densemap::detail::rehashRelocatable(تهی *Dst, UsedT *DstUsed,
                                         unsigned Dst_رقم_Buckets,
                                         ثابت تهی *Src, ثابت UsedT *SrcUsed,
                                         unsigned Src_رقم_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ثابت unsigned Mask = Dst_رقم_Buckets - 1;
  اگر (!Hasher)
    بازگشت rehashBySize<درست>(Dst, DstUsed, Mask, Src, SrcUsed, Src_رقم_Buckets,
                              BucketSize, Hasher);
  بازگشت rehashBySize<نادرست>(Dst, DstUsed, Mask, Src, SrcUsed, Src_رقم_Buckets,
                             BucketSize, Hasher);
}

تهی *densemap::detail::growRelocatable(تهی *OldBuckets, ثابت UsedT *OldUsed,
                                        unsigned Old_رقم_Buckets,
                                        unsigned New_رقم_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, منطقی FreeOld) {
  تهی *Storage = allocate_buffer(allocBytes(BucketSize, New_رقم_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_رقم_Buckets);
  clearUsed(NewUsed, New_رقم_Buckets);
  اگر (Old_رقم_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_رقم_Buckets, OldBuckets, OldUsed,
                      Old_رقم_Buckets, BucketSize, Hasher);
    اگر (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_رقم_Buckets),
                        Align);
  }
  بازگشت Storage;
}
