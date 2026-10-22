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
ढाँचा <माप_प्रकार FixedSize, तार्किक InlinePtrHash>
स्थिर शून्य rehashLoop(शून्य *DstBuckets, UsedT *DstUsed, अचिन्हित Mask,
                       स्थिरांक शून्य *SrcBuckets, स्थिरांक UsedT *SrcUsed,
                       अचिन्हित Src_अंक_Buckets, माप_प्रकार RuntimeSize,
                       BucketHasher Hasher) {
  स्थिरांक माप_प्रकार BucketSize = FixedSize ? FixedSize : RuntimeSize;
  अक्षर *Dst = static_cast<अक्षर *>(DstBuckets);
  स्थिरांक अक्षर *Src = static_cast<स्थिरांक अक्षर *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_अंक_Buckets, [&](अचिन्हित I) {
    स्थिरांक अक्षर *SrcBucket = Src + static_cast<माप_प्रकार>(I) * BucketSize;
    अचिन्हित Hash;
    जर constexpr (InlinePtrHash) {
      शून्य *किल्ली;
      std::स_नकल(&किल्ली, SrcBucket, आकार(किल्ली));
      Hash = DenseMapInfo<शून्य *>::get_Hash_मूल्य(किल्ली);
    } अन्यथा {
      Hash = Hasher(SrcBucket);
    }
    अचिन्हित BucketNo = Hash & Mask;
    पर्यंत (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::स_नकल(Dst + static_cast<माप_प्रकार>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

ढाँचा <तार्किक InlinePtrHash>
स्थिर शून्य rehashBySize(शून्य *Dst, UsedT *DstUsed, अचिन्हित Mask,
                         स्थिरांक शून्य *Src, स्थिरांक UsedT *SrcUsed,
                         अचिन्हित Src_अंक_Buckets, माप_प्रकार BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  निवड (BucketSize) {
#define REHASH_CASE(N)                                                         \
  स्थिती N:                                                                      \
    परत rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
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
  पूर्वनिर्धारित:
    परत rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_अंक_Buckets, BucketSize, Hasher);
  }
}

शून्य densemap::detail::rehashRelocatable(शून्य *Dst, UsedT *DstUsed,
                                         अचिन्हित Dst_अंक_Buckets,
                                         स्थिरांक शून्य *Src, स्थिरांक UsedT *SrcUsed,
                                         अचिन्हित Src_अंक_Buckets,
                                         माप_प्रकार BucketSize,
                                         BucketHasher Hasher) {
  स्थिरांक अचिन्हित Mask = Dst_अंक_Buckets - 1;
  जर (!Hasher)
    परत rehashBySize<सत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                              BucketSize, Hasher);
  परत rehashBySize<असत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                             BucketSize, Hasher);
}

शून्य *densemap::detail::growRelocatable(शून्य *OldBuckets, स्थिरांक UsedT *OldUsed,
                                        अचिन्हित Old_अंक_Buckets,
                                        अचिन्हित New_अंक_Buckets,
                                        माप_प्रकार BucketSize, माप_प्रकार Align,
                                        BucketHasher Hasher, तार्किक FreeOld) {
  शून्य *Storage = allocate_buffer(allocBytes(BucketSize, New_अंक_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_अंक_Buckets);
  clearUsed(NewUsed, New_अंक_Buckets);
  जर (Old_अंक_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_अंक_Buckets, OldBuckets, OldUsed,
                      Old_अंक_Buckets, BucketSize, Hasher);
    जर (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_अंक_Buckets),
                        Align);
  }
  परत Storage;
}
