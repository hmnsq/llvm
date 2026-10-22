//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/ADT/DenseMap.h"
#অন্তর্ভাৱ "llvm/Support/MemAlloc.h"
#অন্তর্ভাৱ <cstring>

using নামাকাশ llvm;
using নামাকাশ llvm::densemap;
using নামাকাশ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, তার্কিক InlinePtrHash>
স্থির শূন্য rehashLoop(শূন্য *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ধ্রুৱ শূন্য *SrcBuckets, ধ্রুৱ UsedT *SrcUsed,
                       unsigned Src_অঙ্ক_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ধ্রুৱ size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  ৱর্ণ *Dst = static_cast<ৱর্ণ *>(DstBuckets);
  ধ্রুৱ ৱর্ণ *Src = static_cast<ধ্রুৱ ৱর্ণ *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_অঙ্ক_Buckets, [&](unsigned I) {
    ধ্রুৱ ৱর্ণ *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    যদি constexpr (InlinePtrHash) {
      শূন্য *চাবি;
      std::memcpy(&চাবি, SrcBucket, পরিমাণ(চাবি));
      Hash = DenseMapInfo<শূন্য *>::get_Hash_মান(চাবি);
    } অন্যথা {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    যেতিয়ালৈ (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <তার্কিক InlinePtrHash>
স্থির শূন্য rehashBySize(শূন্য *Dst, UsedT *DstUsed, unsigned Mask,
                         ধ্রুৱ শূন্য *Src, ধ্রুৱ UsedT *SrcUsed,
                         unsigned Src_অঙ্ক_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ৱিকল্প (BucketSize) {
#define REHASH_CASE(N)                                                         \
  পক্ষ N:                                                                      \
    ঘূৰাই rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_অঙ্ক_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  স্ৱভাৱ:
    ঘূৰাই rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_অঙ্ক_Buckets, BucketSize, Hasher);
  }
}

শূন্য densemap::detail::rehashRelocatable(শূন্য *Dst, UsedT *DstUsed,
                                         unsigned Dst_অঙ্ক_Buckets,
                                         ধ্রুৱ শূন্য *Src, ধ্রুৱ UsedT *SrcUsed,
                                         unsigned Src_অঙ্ক_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ধ্রুৱ unsigned Mask = Dst_অঙ্ক_Buckets - 1;
  যদি (!Hasher)
    ঘূৰাই rehashBySize<সঁচা>(Dst, DstUsed, Mask, Src, SrcUsed, Src_অঙ্ক_Buckets,
                              BucketSize, Hasher);
  ঘূৰাই rehashBySize<মিছা>(Dst, DstUsed, Mask, Src, SrcUsed, Src_অঙ্ক_Buckets,
                             BucketSize, Hasher);
}

শূন্য *densemap::detail::growRelocatable(শূন্য *OldBuckets, ধ্রুৱ UsedT *OldUsed,
                                        unsigned Old_অঙ্ক_Buckets,
                                        unsigned New_অঙ্ক_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, তার্কিক FreeOld) {
  শূন্য *Storage = allocate_buffer(allocBytes(BucketSize, New_অঙ্ক_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_অঙ্ক_Buckets);
  clearUsed(NewUsed, New_অঙ্ক_Buckets);
  যদি (Old_অঙ্ক_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_অঙ্ক_Buckets, OldBuckets, OldUsed,
                      Old_অঙ্ক_Buckets, BucketSize, Hasher);
    যদি (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_অঙ্ক_Buckets),
                        Align);
  }
  ঘূৰাই Storage;
}
