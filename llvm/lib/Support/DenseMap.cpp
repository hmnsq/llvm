//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/DenseMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemAlloc.h"
#ଅନ୍ତର୍ଭାଵ <cstring>

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::densemap;
using ନାମାକାଶ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, ତାର୍କିକ InlinePtrHash>
ସ୍ଥିର ଶୂନ୍ୟ rehashLoop(ଶୂନ୍ୟ *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       ଅଚଳ ଶୂନ୍ୟ *SrcBuckets, ଅଚଳ UsedT *SrcUsed,
                       unsigned Src_ଅଙ୍କ_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  ଅଚଳ size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  ଅକ୍ଷର *Dst = static_cast<ଅକ୍ଷର *>(DstBuckets);
  ଅଚଳ ଅକ୍ଷର *Src = static_cast<ଅଚଳ ଅକ୍ଷର *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_ଅଙ୍କ_Buckets, [&](unsigned I) {
    ଅଚଳ ଅକ୍ଷର *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    ଯଦି constexpr (InlinePtrHash) {
      ଶୂନ୍ୟ *ଚାବି;
      std::memcpy(&ଚାବି, SrcBucket, ଆକାର(ଚାବି));
      Hash = DenseMapInfo<ଶୂନ୍ୟ *>::get_Hash_ମୂଲ୍ୟ(ଚାବି);
    } ଅନ୍ୟଥା {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    ଯେତେବେଳେ (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <ତାର୍କିକ InlinePtrHash>
ସ୍ଥିର ଶୂନ୍ୟ rehashBySize(ଶୂନ୍ୟ *Dst, UsedT *DstUsed, unsigned Mask,
                         ଅଚଳ ଶୂନ୍ୟ *Src, ଅଚଳ UsedT *SrcUsed,
                         unsigned Src_ଅଙ୍କ_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ବାଛ (BucketSize) {
#define REHASH_CASE(N)                                                         \
  ସ୍ଥିତି N:                                                                      \
    ଫେରନ୍ତୁ rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_ଅଙ୍କ_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_ଅଙ୍କ_Buckets, BucketSize, Hasher);
  }
}

ଶୂନ୍ୟ densemap::detail::rehashRelocatable(ଶୂନ୍ୟ *Dst, UsedT *DstUsed,
                                         unsigned Dst_ଅଙ୍କ_Buckets,
                                         ଅଚଳ ଶୂନ୍ୟ *Src, ଅଚଳ UsedT *SrcUsed,
                                         unsigned Src_ଅଙ୍କ_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  ଅଚଳ unsigned Mask = Dst_ଅଙ୍କ_Buckets - 1;
  ଯଦି (!Hasher)
    ଫେରନ୍ତୁ rehashBySize<ସତ୍ୟ>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ଅଙ୍କ_Buckets,
                              BucketSize, Hasher);
  ଫେରନ୍ତୁ rehashBySize<ମିଥ୍ୟା>(Dst, DstUsed, Mask, Src, SrcUsed, Src_ଅଙ୍କ_Buckets,
                             BucketSize, Hasher);
}

ଶୂନ୍ୟ *densemap::detail::growRelocatable(ଶୂନ୍ୟ *OldBuckets, ଅଚଳ UsedT *OldUsed,
                                        unsigned Old_ଅଙ୍କ_Buckets,
                                        unsigned New_ଅଙ୍କ_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, ତାର୍କିକ FreeOld) {
  ଶୂନ୍ୟ *Storage = allocate_buffer(allocBytes(BucketSize, New_ଅଙ୍କ_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_ଅଙ୍କ_Buckets);
  clearUsed(NewUsed, New_ଅଙ୍କ_Buckets);
  ଯଦି (Old_ଅଙ୍କ_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_ଅଙ୍କ_Buckets, OldBuckets, OldUsed,
                      Old_ଅଙ୍କ_Buckets, BucketSize, Hasher);
    ଯଦି (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_ଅଙ୍କ_Buckets),
                        Align);
  }
  ଫେରନ୍ତୁ Storage;
}
