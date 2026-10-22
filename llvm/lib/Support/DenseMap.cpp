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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::densemap;
उपयुज्य नामाकाश llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
प्रतिरूपम् <size_t Fixed_आकार, तार्किक InlinePtrHash>
स्थिर शून्य rehashLoop(शून्य *DstBuckets, प्रयुक्तम्_T *Dst_प्रयुक्तम्, अचिह्नित आवरणम्,
                       ध्रुव शून्य *SrcBuckets, ध्रुव प्रयुक्तम्_T *Src_प्रयुक्तम्,
                       अचिह्नित Src_अङ्कः_Buckets, size_t Runtime_आकार,
                       BucketHasher Hasher) {
  ध्रुव size_t Bucket_आकार = Fixed_आकार ? Fixed_आकार : Runtime_आकार;
  वर्ण *Dst = static_cast<वर्ण *>(DstBuckets);
  ध्रुव वर्ण *Src = static_cast<ध्रुव वर्ण *>(SrcBuckets);
  for_Each_प्रयुक्तम्(Src_प्रयुक्तम्, Src_अङ्कः_Buckets, [&](अचिह्नित I) {
    ध्रुव वर्ण *SrcBucket = Src + static_cast<size_t>(I) * Bucket_आकार;
    अचिह्नित Hash;
    यदि constexpr (InlinePtrHash) {
      शून्य *कुञ्चिका;
      std::स्मृतिं_प्रतिलिख(&कुञ्चिका, SrcBucket, परिमाण(कुञ्चिका));
      Hash = DenseMapInfo<शून्य *>::get_Hash_मानम्(कुञ्चिका);
    } अन्यथा {
      Hash = Hasher(SrcBucket);
    }
    अचिह्नित BucketNo = Hash & आवरणम्;
    यावत् (प्रयुक्तम्(Dst_प्रयुक्तम्, BucketNo))
      BucketNo = (BucketNo + 1) & आवरणम्;
    std::स्मृतिं_प्रतिलिख(Dst + static_cast<size_t>(BucketNo) * Bucket_आकार, SrcBucket,
                Bucket_आकार);
    समुच्चयः_प्रयुक्तम्(Dst_प्रयुक्तम्, BucketNo);
  });
}

प्रतिरूपम् <तार्किक InlinePtrHash>
स्थिर शून्य rehash_By_आकार(शून्य *Dst, प्रयुक्तम्_T *Dst_प्रयुक्तम्, अचिह्नित आवरणम्,
                         ध्रुव शून्य *Src, ध्रुव प्रयुक्तम्_T *Src_प्रयुक्तम्,
                         अचिह्नित Src_अङ्कः_Buckets, size_t Bucket_आकार,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  विकल्प (Bucket_आकार) {
#परिभाषय REHASH_CASE(N)                                                         \
  पक्ष N:                                                                      \
    प्रत्यागम rehashLoop<N, InlinePtrHash>(Dst, Dst_प्रयुक्तम्, आवरणम्, Src, Src_प्रयुक्तम्,      \
                                        Src_अङ्कः_Buckets, Bucket_आकार, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#अपरिभाषय REHASH_CASE
  स्वभाव:
    प्रत्यागम rehashLoop<0, InlinePtrHash>(Dst, Dst_प्रयुक्तम्, आवरणम्, Src, Src_प्रयुक्तम्,
                                        Src_अङ्कः_Buckets, Bucket_आकार, Hasher);
  }
}

शून्य densemap::detail::rehashRelocatable(शून्य *Dst, प्रयुक्तम्_T *Dst_प्रयुक्तम्,
                                         अचिह्नित Dst_अङ्कः_Buckets,
                                         ध्रुव शून्य *Src, ध्रुव प्रयुक्तम्_T *Src_प्रयुक्तम्,
                                         अचिह्नित Src_अङ्कः_Buckets,
                                         size_t Bucket_आकार,
                                         BucketHasher Hasher) {
  ध्रुव अचिह्नित आवरणम् = Dst_अङ्कः_Buckets - 1;
  यदि (!Hasher)
    प्रत्यागम rehash_By_आकार<सत्य>(Dst, Dst_प्रयुक्तम्, आवरणम्, Src, Src_प्रयुक्तम्, Src_अङ्कः_Buckets,
                              Bucket_आकार, Hasher);
  प्रत्यागम rehash_By_आकार<असत्य>(Dst, Dst_प्रयुक्तम्, आवरणम्, Src, Src_प्रयुक्तम्, Src_अङ्कः_Buckets,
                             Bucket_आकार, Hasher);
}

शून्य *densemap::detail::growRelocatable(शून्य *OldBuckets, ध्रुव प्रयुक्तम्_T *Old_प्रयुक्तम्,
                                        अचिह्नित Old_अङ्कः_Buckets,
                                        अचिह्नित New_अङ्कः_Buckets,
                                        size_t Bucket_आकार, size_t Align,
                                        BucketHasher Hasher, तार्किक FreeOld) {
  शून्य *Storage = allocate_बफर(alloc_अष्टकानि(Bucket_आकार, New_अङ्कः_Buckets), Align);
  प्रयुक्तम्_T *New_प्रयुक्तम् = प्रयुक्तम्_For(Storage, Bucket_आकार, New_अङ्कः_Buckets);
  clear_प्रयुक्तम्(New_प्रयुक्तम्, New_अङ्कः_Buckets);
  यदि (Old_अङ्कः_Buckets) {
    rehashRelocatable(Storage, New_प्रयुक्तम्, New_अङ्कः_Buckets, OldBuckets, Old_प्रयुक्तम्,
                      Old_अङ्कः_Buckets, Bucket_आकार, Hasher);
    यदि (FreeOld)
      deallocate_बफर(OldBuckets, alloc_अष्टकानि(Bucket_आकार, Old_अङ्कः_Buckets),
                        Align);
  }
  प्रत्यागम Storage;
}
