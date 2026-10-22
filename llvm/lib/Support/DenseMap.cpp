//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/DenseMap.h"
#અન્તર્ભાવ "llvm/Support/MemAlloc.h"
#અન્તર્ભાવ <cstring>

using નામાકાશ llvm;
using નામાકાશ llvm::densemap;
using નામાકાશ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, તાર્કિક InlinePtrHash>
સ્થિર શૂન્ય rehashLoop(શૂન્ય *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       અચલ શૂન્ય *SrcBuckets, અચલ UsedT *SrcUsed,
                       unsigned Src_અંક_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  અચલ size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  અક્ષર *Dst = static_cast<અક્ષર *>(DstBuckets);
  અચલ અક્ષર *Src = static_cast<અચલ અક્ષર *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_અંક_Buckets, [&](unsigned I) {
    અચલ અક્ષર *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    જો constexpr (InlinePtrHash) {
      શૂન્ય *ચાવી;
      std::memcpy(&ચાવી, SrcBucket, કદ(ચાવી));
      Hash = DenseMapInfo<શૂન્ય *>::get_Hash_કિંમત(ચાવી);
    } નહીંતર {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    જ્યારે (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <તાર્કિક InlinePtrHash>
સ્થિર શૂન્ય rehashBySize(શૂન્ય *Dst, UsedT *DstUsed, unsigned Mask,
                         અચલ શૂન્ય *Src, અચલ UsedT *SrcUsed,
                         unsigned Src_અંક_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  પસંદગી (BucketSize) {
#define REHASH_CASE(N)                                                         \
  સ્થિતિ N:                                                                      \
    પાછા rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_અંક_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  મૂળભૂત:
    પાછા rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_અંક_Buckets, BucketSize, Hasher);
  }
}

શૂન્ય densemap::detail::rehashRelocatable(શૂન્ય *Dst, UsedT *DstUsed,
                                         unsigned Dst_અંક_Buckets,
                                         અચલ શૂન્ય *Src, અચલ UsedT *SrcUsed,
                                         unsigned Src_અંક_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  અચલ unsigned Mask = Dst_અંક_Buckets - 1;
  જો (!Hasher)
    પાછા rehashBySize<સાચું>(Dst, DstUsed, Mask, Src, SrcUsed, Src_અંક_Buckets,
                              BucketSize, Hasher);
  પાછા rehashBySize<ખોટું>(Dst, DstUsed, Mask, Src, SrcUsed, Src_અંક_Buckets,
                             BucketSize, Hasher);
}

શૂન્ય *densemap::detail::growRelocatable(શૂન્ય *OldBuckets, અચલ UsedT *OldUsed,
                                        unsigned Old_અંક_Buckets,
                                        unsigned New_અંક_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, તાર્કિક FreeOld) {
  શૂન્ય *Storage = allocate_buffer(allocBytes(BucketSize, New_અંક_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_અંક_Buckets);
  clearUsed(NewUsed, New_અંક_Buckets);
  જો (Old_અંક_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_અંક_Buckets, OldBuckets, OldUsed,
                      Old_અંક_Buckets, BucketSize, Hasher);
    જો (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_અંક_Buckets),
                        Align);
  }
  પાછા Storage;
}
