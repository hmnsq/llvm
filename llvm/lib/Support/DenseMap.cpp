//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/DenseMap.h"
#अन्तर्भाव "llvm/Support/MemAlloc.h"
#अन्तर्भाव <cstring>

using नामाकाश llvm;
using नामाकाश llvm::densemap;
using नामाकाश llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, तार्किक InlinePtrHash>
स्थिर शून्य rehashLoop(शून्य *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ध्रुव शून्य *SrcBuckets, ध्रुव UsedT *SrcUsed,
                       unsigned Src_अंक_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ध्रुव size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  वर्ण *Dst = static_cast<वर्ण *>(DstBuckets);
  ध्रुव वर्ण *Src = static_cast<ध्रुव वर्ण *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_अंक_Buckets, [&](unsigned I) {
    ध्रुव वर्ण *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    यदि constexpr (InlinePtrHash) {
      शून्य *किल्ली;
      std::memcpy(&किल्ली, SrcBucket, परिमाण(किल्ली));
      Hash = DenseMapInfo<शून्य *>::get_Hash_मूल्य(किल्ली);
    } अन्यथा {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    यावत् (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <तार्किक InlinePtrHash>
स्थिर शून्य rehashBySize(शून्य *Dst, UsedT *DstUsed, unsigned Mask,
                         ध्रुव शून्य *Src, ध्रुव UsedT *SrcUsed,
                         unsigned Src_अंक_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  विकल्प (BucketSize) {
#define REHASH_CASE(N)                                                         \
  पक्ष N:                                                                      \
    प्रत्यागम rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_अंक_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  स्वभाव:
    प्रत्यागम rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_अंक_Buckets, BucketSize, Hasher);
  }
}

शून्य densemap::detail::rehashRelocatable(शून्य *Dst, UsedT *DstUsed,
                                         unsigned Dst_अंक_Buckets,
                                         ध्रुव शून्य *Src, ध्रुव UsedT *SrcUsed,
                                         unsigned Src_अंक_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ध्रुव unsigned Mask = Dst_अंक_Buckets - 1;
  यदि (!Hasher)
    प्रत्यागम rehashBySize<सत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                              BucketSize, Hasher);
  प्रत्यागम rehashBySize<असत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                             BucketSize, Hasher);
}

शून्य *densemap::detail::growRelocatable(शून्य *OldBuckets, ध्रुव UsedT *OldUsed,
                                        unsigned Old_अंक_Buckets,
                                        unsigned New_अंक_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, तार्किक FreeOld) {
  शून्य *Storage = allocate_buffer(allocBytes(BucketSize, New_अंक_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_अंक_Buckets);
  clearUsed(NewUsed, New_अंक_Buckets);
  यदि (Old_अंक_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_अंक_Buckets, OldBuckets, OldUsed,
                      Old_अंक_Buckets, BucketSize, Hasher);
    यदि (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_अंक_Buckets),
                        Align);
  }
  प्रत्यागम Storage;
}
