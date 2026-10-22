//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/DenseMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <cstring>

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::densemap;
using ನಾಮಾಕಾಶ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, ತಾರ್ಕಿಕ InlinePtrHash>
ಸ್ಥಿರ ಶೂನ್ಯ rehashLoop(ಶೂನ್ಯ *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ಧ್ರುವ ಶೂನ್ಯ *SrcBuckets, ಧ್ರುವ UsedT *SrcUsed,
                       unsigned Src_ಅಂಕೆ_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ಧ್ರುವ size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  ವರ್ಣ *Dst = static_cast<ವರ್ಣ *>(DstBuckets);
  ಧ್ರುವ ವರ್ಣ *Src = static_cast<ಧ್ರುವ ವರ್ಣ *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_ಅಂಕೆ_Buckets, [&](unsigned I) {
    ಧ್ರುವ ವರ್ಣ *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    ಯದಿ constexpr (InlinePtrHash) {
      ಶೂನ್ಯ *ಕೀಲಿ;
      std::memcpy(&ಕೀಲಿ, SrcBucket, ಪರಿಮಾಣ(ಕೀಲಿ));
      Hash = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(ಕೀಲಿ);
    } ಅನ್ಯಥಾ {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    ಯಾವತ್ (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <ತಾರ್ಕಿಕ InlinePtrHash>
ಸ್ಥಿರ ಶೂನ್ಯ rehashBySize(ಶೂನ್ಯ *Dst, UsedT *DstUsed, unsigned Mask,
                         ಧ್ರುವ ಶೂನ್ಯ *Src, ಧ್ರುವ UsedT *SrcUsed,
                         unsigned Src_ಅಂಕೆ_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ವಿಕಲ್ಪ (BucketSize) {
#define REHASH_CASE(N)                                                         \
  ಪಕ್ಷ N:                                                                      \
    ಪ್ರತ್ಯಾಗಮ rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_ಅಂಕೆ_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_ಅಂಕೆ_Buckets, BucketSize, Hasher);
  }
}

ಶೂನ್ಯ densemap::detail::rehashRelocatable(ಶೂನ್ಯ *Dst, UsedT *DstUsed,
                                         unsigned Dst_ಅಂಕೆ_Buckets,
                                         ಧ್ರುವ ಶೂನ್ಯ *Src, ಧ್ರುವ UsedT *SrcUsed,
                                         unsigned Src_ಅಂಕೆ_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ಧ್ರುವ unsigned Mask = Dst_ಅಂಕೆ_Buckets - 1;
  ಯದಿ (!Hasher)
    ಪ್ರತ್ಯಾಗಮ rehashBySize<ಸತ್ಯ>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ಅಂಕೆ_Buckets,
                              BucketSize, Hasher);
  ಪ್ರತ್ಯಾಗಮ rehashBySize<ಅಸತ್ಯ>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ಅಂಕೆ_Buckets,
                             BucketSize, Hasher);
}

ಶೂನ್ಯ *densemap::detail::growRelocatable(ಶೂನ್ಯ *OldBuckets, ಧ್ರುವ UsedT *OldUsed,
                                        unsigned Old_ಅಂಕೆ_Buckets,
                                        unsigned New_ಅಂಕೆ_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, ತಾರ್ಕಿಕ FreeOld) {
  ಶೂನ್ಯ *Storage = allocate_buffer(allocBytes(BucketSize, New_ಅಂಕೆ_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_ಅಂಕೆ_Buckets);
  clearUsed(NewUsed, New_ಅಂಕೆ_Buckets);
  ಯದಿ (Old_ಅಂಕೆ_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_ಅಂಕೆ_Buckets, OldBuckets, OldUsed,
                      Old_ಅಂಕೆ_Buckets, BucketSize, Hasher);
    ಯದಿ (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_ಅಂಕೆ_Buckets),
                        Align);
  }
  ಪ್ರತ್ಯಾಗಮ Storage;
}
