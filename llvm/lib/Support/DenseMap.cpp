//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/DenseMap.h"
#অন্তর্ভাব "llvm/Support/MemAlloc.h"
#অন্তর্ভাব <cstring>

using নামাকাশ llvm;
using নামাকাশ llvm::densemap;
using নামাকাশ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t Fixed_আকার, যৌক্তিক InlinePtrHash>
স্থির শূন্য rehashLoop(শূন্য *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ধ্রুবক শূন্য *SrcBuckets, ধ্রুবক UsedT *SrcUsed,
                       unsigned Src_অঙ্ক_Buckets, size_t Runtime_আকার,
                       BucketHasher Hasher) {
  ধ্রুবক size_t Bucket_আকার = Fixed_আকার ? Fixed_আকার : Runtime_আকার;
  অক্ষর *Dst = static_cast<অক্ষর *>(DstBuckets);
  ধ্রুবক অক্ষর *Src = static_cast<ধ্রুবক অক্ষর *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_অঙ্ক_Buckets, [&](unsigned I) {
    ধ্রুবক অক্ষর *SrcBucket = Src + static_cast<size_t>(I) * Bucket_আকার;
    unsigned Hash;
    যদি constexpr (InlinePtrHash) {
      শূন্য *চাবি;
      std::memcpy(&চাবি, SrcBucket, আকার(চাবি));
      Hash = DenseMapInfo<শূন্য *>::get_Hash_মান(চাবি);
    } অন্যথা {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    যখন (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * Bucket_আকার, SrcBucket,
                Bucket_আকার);
    setUsed(DstUsed, BucketNo);
  });
}

template <যৌক্তিক InlinePtrHash>
স্থির শূন্য rehash_By_আকার(শূন্য *Dst, UsedT *DstUsed, unsigned Mask,
                         ধ্রুবক শূন্য *Src, ধ্রুবক UsedT *SrcUsed,
                         unsigned Src_অঙ্ক_Buckets, size_t Bucket_আকার,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  নির্বাচন (Bucket_আকার) {
#define REHASH_CASE(N)                                                         \
  ক্ষেত্র N:                                                                      \
    ফেরত rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_অঙ্ক_Buckets, Bucket_আকার, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  পূর্বনির্ধারিত:
    ফেরত rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_অঙ্ক_Buckets, Bucket_আকার, Hasher);
  }
}

শূন্য densemap::detail::rehashRelocatable(শূন্য *Dst, UsedT *DstUsed,
                                         unsigned Dst_অঙ্ক_Buckets,
                                         ধ্রুবক শূন্য *Src, ধ্রুবক UsedT *SrcUsed,
                                         unsigned Src_অঙ্ক_Buckets,
                                         size_t Bucket_আকার,
                                         BucketHasher Hasher) {
  ধ্রুবক unsigned Mask = Dst_অঙ্ক_Buckets - 1;
  যদি (!Hasher)
    ফেরত rehash_By_আকার<সত্য>(Dst, DstUsed, Mask, Src, SrcUsed, Src_অঙ্ক_Buckets,
                              Bucket_আকার, Hasher);
  ফেরত rehash_By_আকার<মিথ্যা>(Dst, DstUsed, Mask, Src, SrcUsed, Src_অঙ্ক_Buckets,
                             Bucket_আকার, Hasher);
}

শূন্য *densemap::detail::growRelocatable(শূন্য *OldBuckets, ধ্রুবক UsedT *OldUsed,
                                        unsigned Old_অঙ্ক_Buckets,
                                        unsigned New_অঙ্ক_Buckets,
                                        size_t Bucket_আকার, size_t Align,
                                        BucketHasher Hasher, যৌক্তিক FreeOld) {
  শূন্য *Storage = allocate_বাফার(allocBytes(Bucket_আকার, New_অঙ্ক_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, Bucket_আকার, New_অঙ্ক_Buckets);
  clearUsed(NewUsed, New_অঙ্ক_Buckets);
  যদি (Old_অঙ্ক_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_অঙ্ক_Buckets, OldBuckets, OldUsed,
                      Old_অঙ্ক_Buckets, Bucket_আকার, Hasher);
    যদি (FreeOld)
      deallocate_বাফার(OldBuckets, allocBytes(Bucket_আকার, Old_অঙ্ক_Buckets),
                        Align);
  }
  ফেরত Storage;
}
