//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/DenseMap.h"
#அந்தர்பாவ "llvm/Support/MemAlloc.h"
#அந்தர்பாவ <cstring>

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::densemap;
using நாமாகாஶ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, தருக்க InlinePtrHash>
நிலையான வெறுமை rehashLoop(வெறுமை *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       மாறிலி வெறுமை *SrcBuckets, மாறிலி UsedT *SrcUsed,
                       unsigned Src_இலக்கம்_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  மாறிலி size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  எழுத்து *Dst = static_cast<எழுத்து *>(DstBuckets);
  மாறிலி எழுத்து *Src = static_cast<மாறிலி எழுத்து *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_இலக்கம்_Buckets, [&](unsigned I) {
    மாறிலி எழுத்து *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    எனில் constexpr (InlinePtrHash) {
      வெறுமை *சாவி;
      std::memcpy(&சாவி, SrcBucket, அளவு(சாவி));
      Hash = DenseMapInfo<வெறுமை *>::get_Hash_மதிப்பு(சாவி);
    } இல்லையெனில் {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    போது (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <தருக்க InlinePtrHash>
நிலையான வெறுமை rehashBySize(வெறுமை *Dst, UsedT *DstUsed, unsigned Mask,
                         மாறிலி வெறுமை *Src, மாறிலி UsedT *SrcUsed,
                         unsigned Src_இலக்கம்_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  தேர்வு (BucketSize) {
#define REHASH_CASE(N)                                                         \
  நிலை N:                                                                      \
    திரும்பு rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_இலக்கம்_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  இயல்பு:
    திரும்பு rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_இலக்கம்_Buckets, BucketSize, Hasher);
  }
}

வெறுமை densemap::detail::rehashRelocatable(வெறுமை *Dst, UsedT *DstUsed,
                                         unsigned Dst_இலக்கம்_Buckets,
                                         மாறிலி வெறுமை *Src, மாறிலி UsedT *SrcUsed,
                                         unsigned Src_இலக்கம்_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  மாறிலி unsigned Mask = Dst_இலக்கம்_Buckets - 1;
  எனில் (!Hasher)
    திரும்பு rehashBySize<உண்மை>(Dst, DstUsed, Mask, Src, SrcUsed, Src_இலக்கம்_Buckets,
                              BucketSize, Hasher);
  திரும்பு rehashBySize<பொய்>(Dst, DstUsed, Mask, Src, SrcUsed, Src_இலக்கம்_Buckets,
                             BucketSize, Hasher);
}

வெறுமை *densemap::detail::growRelocatable(வெறுமை *OldBuckets, மாறிலி UsedT *OldUsed,
                                        unsigned Old_இலக்கம்_Buckets,
                                        unsigned New_இலக்கம்_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, தருக்க FreeOld) {
  வெறுமை *Storage = allocate_buffer(allocBytes(BucketSize, New_இலக்கம்_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_இலக்கம்_Buckets);
  clearUsed(NewUsed, New_இலக்கம்_Buckets);
  எனில் (Old_இலக்கம்_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_இலக்கம்_Buckets, OldBuckets, OldUsed,
                      Old_இலக்கம்_Buckets, BucketSize, Hasher);
    எனில் (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_இலக்கம்_Buckets),
                        Align);
  }
  திரும்பு Storage;
}
