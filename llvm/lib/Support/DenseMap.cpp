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
                       unsigned SrcNumBuckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ثابت size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  حرف *Dst = static_cast<حرف *>(DstBuckets);
  ثابت حرف *Src = static_cast<ثابت حرف *>(SrcBuckets);
  forEachUsed(SrcUsed, SrcNumBuckets, [&](unsigned I) {
    ثابت حرف *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    𐡄𐡍 constexpr (InlinePtrHash) {
      فراغ *Key;
      std::memcpy(&Key, SrcBucket, حجم(Key));
      Hash = DenseMapInfo<فراغ *>::getHashValue(Key);
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
                         unsigned SrcNumBuckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  بدل (BucketSize) {
#define REHASH_CASE(N)                                                         \
  حالة N:                                                                      \
    أرجع rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        SrcNumBuckets, BucketSize, Hasher);
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
                                        SrcNumBuckets, BucketSize, Hasher);
  }
}

فراغ densemap::detail::rehashRelocatable(فراغ *Dst, UsedT *DstUsed,
                                         unsigned DstNumBuckets,
                                         ثابت فراغ *Src, ثابت UsedT *SrcUsed,
                                         unsigned SrcNumBuckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ثابت unsigned Mask = DstNumBuckets - 1;
  𐡄𐡍 (!Hasher)
    أرجع rehashBySize<𐡒𐡔𐡈>(Dst, DstUsed, Mask, Src, SrcUsed, SrcNumBuckets,
                              BucketSize, Hasher);
  أرجع rehashBySize<خطأ>(Dst, DstUsed, Mask, Src, SrcUsed, SrcNumBuckets,
                             BucketSize, Hasher);
}

فراغ *densemap::detail::growRelocatable(فراغ *OldBuckets, ثابت UsedT *OldUsed,
                                        unsigned OldNumBuckets,
                                        unsigned NewNumBuckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, منطقي FreeOld) {
  فراغ *Storage = allocate_buffer(allocBytes(BucketSize, NewNumBuckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, NewNumBuckets);
  clearUsed(NewUsed, NewNumBuckets);
  𐡄𐡍 (OldNumBuckets) {
    rehashRelocatable(Storage, NewUsed, NewNumBuckets, OldBuckets, OldUsed,
                      OldNumBuckets, BucketSize, Hasher);
    𐡄𐡍 (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, OldNumBuckets),
                        Align);
  }
  أرجع Storage;
}
