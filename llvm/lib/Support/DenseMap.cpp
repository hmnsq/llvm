//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/DenseMap.h"
#समावेश "llvm/Support/MemAlloc.h"
#समावेश <cstring>

using namespace llvm;
using namespace llvm::densemap;
using namespace llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
ढाँचा <माप_प्रकार Fixed_आकार, bool InlinePtrHash>
अटल व्योम rehashLoop(व्योम *DstBuckets, UsedT *DstUsed, अचिन्हित Mask,
                       स्थिर व्योम *SrcBuckets, स्थिर UsedT *SrcUsed,
                       अचिन्हित Src_अंक_Buckets, माप_प्रकार Runtime_आकार,
                       BucketHasher Hasher) {
  स्थिर माप_प्रकार Bucket_आकार = Fixed_आकार ? Fixed_आकार : Runtime_आकार;
  अक्षर *Dst = static_cast<अक्षर *>(DstBuckets);
  स्थिर अक्षर *Src = static_cast<स्थिर अक्षर *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_अंक_Buckets, [&](अचिन्हित I) {
    स्थिर अक्षर *SrcBucket = Src + static_cast<माप_प्रकार>(I) * Bucket_आकार;
    अचिन्हित Hash;
    अगर constexpr (InlinePtrHash) {
      व्योम *कुंजी;
      std::स_नकल(&कुंजी, SrcBucket, माप(कुंजी));
      Hash = DenseMapInfo<व्योम *>::get_Hash_मूल्य(कुंजी);
    } अन्यथा {
      Hash = Hasher(SrcBucket);
    }
    अचिन्हित BucketNo = Hash & Mask;
    जबतक (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::स_नकल(Dst + static_cast<माप_प्रकार>(BucketNo) * Bucket_आकार, SrcBucket,
                Bucket_आकार);
    setUsed(DstUsed, BucketNo);
  });
}

ढाँचा <bool InlinePtrHash>
अटल व्योम rehash_By_आकार(व्योम *Dst, UsedT *DstUsed, अचिन्हित Mask,
                         स्थिर व्योम *Src, स्थिर UsedT *SrcUsed,
                         अचिन्हित Src_अंक_Buckets, माप_प्रकार Bucket_आकार,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  चयन (Bucket_आकार) {
#घोषणा REHASH_CASE(N)                                                         \
  हाल N:                                                                      \
    वापस rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_अंक_Buckets, Bucket_आकार, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#अघोषित REHASH_CASE
  शेष:
    वापस rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_अंक_Buckets, Bucket_आकार, Hasher);
  }
}

व्योम densemap::detail::rehashRelocatable(व्योम *Dst, UsedT *DstUsed,
                                         अचिन्हित Dst_अंक_Buckets,
                                         स्थिर व्योम *Src, स्थिर UsedT *SrcUsed,
                                         अचिन्हित Src_अंक_Buckets,
                                         माप_प्रकार Bucket_आकार,
                                         BucketHasher Hasher) {
  स्थिर अचिन्हित Mask = Dst_अंक_Buckets - 1;
  अगर (!Hasher)
    वापस rehash_By_आकार<true>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                              Bucket_आकार, Hasher);
  वापस rehash_By_आकार<false>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अंक_Buckets,
                             Bucket_आकार, Hasher);
}

व्योम *densemap::detail::growRelocatable(व्योम *OldBuckets, स्थिर UsedT *OldUsed,
                                        अचिन्हित Old_अंक_Buckets,
                                        अचिन्हित New_अंक_Buckets,
                                        माप_प्रकार Bucket_आकार, माप_प्रकार Align,
                                        BucketHasher Hasher, bool FreeOld) {
  व्योम *Storage = allocate_बफर(allocBytes(Bucket_आकार, New_अंक_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, Bucket_आकार, New_अंक_Buckets);
  clearUsed(NewUsed, New_अंक_Buckets);
  अगर (Old_अंक_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_अंक_Buckets, OldBuckets, OldUsed,
                      Old_अंक_Buckets, Bucket_आकार, Hasher);
    अगर (FreeOld)
      deallocate_बफर(OldBuckets, allocBytes(Bucket_आकार, Old_अंक_Buckets),
                        Align);
  }
  वापस Storage;
}
