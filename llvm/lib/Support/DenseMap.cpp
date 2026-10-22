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
                       अचर शून्य *SrcBuckets, अचर UsedT *SrcUsed,
                       अचिन्हित Src_अङ्क_Buckets, माप_प्रकार RuntimeSize,
                       BucketHasher Hasher) {
  अचर माप_प्रकार BucketSize = FixedSize ? FixedSize : RuntimeSize;
  वर्ण *Dst = static_cast<वर्ण *>(DstBuckets);
  अचर वर्ण *Src = static_cast<अचर वर्ण *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_अङ्क_Buckets, [&](अचिन्हित I) {
    अचर वर्ण *SrcBucket = Src + static_cast<माप_प्रकार>(I) * BucketSize;
    अचिन्हित Hash;
    यदि constexpr (InlinePtrHash) {
      शून्य *साँचो;
      std::स_नकल(&साँचो, SrcBucket, परिमाण(साँचो));
      Hash = DenseMapInfo<शून्य *>::get_Hash_मान(साँचो);
    } अन्यथा {
      Hash = Hasher(SrcBucket);
    }
    अचिन्हित BucketNo = Hash & Mask;
    जबसम्म (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::स_नकल(Dst + static_cast<माप_प्रकार>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

ढाँचा <तार्किक InlinePtrHash>
स्थिर शून्य rehashBySize(शून्य *Dst, UsedT *DstUsed, अचिन्हित Mask,
                         अचर शून्य *Src, अचर UsedT *SrcUsed,
                         अचिन्हित Src_अङ्क_Buckets, माप_प्रकार BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  विकल्प (BucketSize) {
#define REHASH_CASE(N)                                                         \
  पक्ष N:                                                                      \
    फर्काऊ rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_अङ्क_Buckets, BucketSize, Hasher);
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
    फर्काऊ rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_अङ्क_Buckets, BucketSize, Hasher);
  }
}

शून्य densemap::detail::rehashRelocatable(शून्य *Dst, UsedT *DstUsed,
                                         अचिन्हित Dst_अङ्क_Buckets,
                                         अचर शून्य *Src, अचर UsedT *SrcUsed,
                                         अचिन्हित Src_अङ्क_Buckets,
                                         माप_प्रकार BucketSize,
                                         BucketHasher Hasher) {
  अचर अचिन्हित Mask = Dst_अङ्क_Buckets - 1;
  यदि (!Hasher)
    फर्काऊ rehashBySize<सत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अङ्क_Buckets,
                              BucketSize, Hasher);
  फर्काऊ rehashBySize<असत्य>(Dst, DstUsed, Mask, Src, SrcUsed, Src_अङ्क_Buckets,
                             BucketSize, Hasher);
}

शून्य *densemap::detail::growRelocatable(शून्य *OldBuckets, अचर UsedT *OldUsed,
                                        अचिन्हित Old_अङ्क_Buckets,
                                        अचिन्हित New_अङ्क_Buckets,
                                        माप_प्रकार BucketSize, माप_प्रकार Align,
                                        BucketHasher Hasher, तार्किक FreeOld) {
  शून्य *Storage = allocate_buffer(allocBytes(BucketSize, New_अङ्क_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_अङ्क_Buckets);
  clearUsed(NewUsed, New_अङ्क_Buckets);
  यदि (Old_अङ्क_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_अङ्क_Buckets, OldBuckets, OldUsed,
                      Old_अङ्क_Buckets, BucketSize, Hasher);
    यदि (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_अङ्क_Buckets),
                        Align);
  }
  फर्काऊ Storage;
}
