//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/DenseMap.h"
#അന്തര്ഭാവ "llvm/Support/MemAlloc.h"
#അന്തര്ഭാവ <cstring>

using നാമാകാശ llvm;
using നാമാകാശ llvm::densemap;
using നാമാകാശ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, തർക്കം InlinePtrHash>
സ്ഥിതം ശൂന്യം rehashLoop(ശൂന്യം *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       സ്ഥിരം ശൂന്യം *SrcBuckets, സ്ഥിരം UsedT *SrcUsed,
                       unsigned Src_അക്കം_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  സ്ഥിരം size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  അക്ഷരം *Dst = static_cast<അക്ഷരം *>(DstBuckets);
  സ്ഥിരം അക്ഷരം *Src = static_cast<സ്ഥിരം അക്ഷരം *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_അക്കം_Buckets, [&](unsigned I) {
    സ്ഥിരം അക്ഷരം *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    എങ്കിൽ constexpr (InlinePtrHash) {
      ശൂന്യം *താക്കോൽ;
      std::memcpy(&താക്കോൽ, SrcBucket, വലുപ്പം(താക്കോൽ));
      Hash = DenseMapInfo<ശൂന്യം *>::get_Hash_മൂല്യം(താക്കോൽ);
    } അല്ലെങ്കിൽ {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    ആകുമ്പോൾ (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <തർക്കം InlinePtrHash>
സ്ഥിതം ശൂന്യം rehashBySize(ശൂന്യം *Dst, UsedT *DstUsed, unsigned Mask,
                         സ്ഥിരം ശൂന്യം *Src, സ്ഥിരം UsedT *SrcUsed,
                         unsigned Src_അക്കം_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  തിരഞ്ഞെടുപ്പ് (BucketSize) {
#define REHASH_CASE(N)                                                         \
  അവസ്ഥ N:                                                                      \
    തിരികെ rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_അക്കം_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  സ്ഥിരസ്ഥിതി:
    തിരികെ rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_അക്കം_Buckets, BucketSize, Hasher);
  }
}

ശൂന്യം densemap::detail::rehashRelocatable(ശൂന്യം *Dst, UsedT *DstUsed,
                                         unsigned Dst_അക്കം_Buckets,
                                         സ്ഥിരം ശൂന്യം *Src, സ്ഥിരം UsedT *SrcUsed,
                                         unsigned Src_അക്കം_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  സ്ഥിരം unsigned Mask = Dst_അക്കം_Buckets - 1;
  എങ്കിൽ (!Hasher)
    തിരികെ rehashBySize<സത്യം>(Dst, DstUsed, Mask, Src, SrcUsed, Src_അക്കം_Buckets,
                              BucketSize, Hasher);
  തിരികെ rehashBySize<അസത്യം>(Dst, DstUsed, Mask, Src, SrcUsed, Src_അക്കം_Buckets,
                             BucketSize, Hasher);
}

ശൂന്യം *densemap::detail::growRelocatable(ശൂന്യം *OldBuckets, സ്ഥിരം UsedT *OldUsed,
                                        unsigned Old_അക്കം_Buckets,
                                        unsigned New_അക്കം_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, തർക്കം FreeOld) {
  ശൂന്യം *Storage = allocate_buffer(allocBytes(BucketSize, New_അക്കം_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_അക്കം_Buckets);
  clearUsed(NewUsed, New_അക്കം_Buckets);
  എങ്കിൽ (Old_അക്കം_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_അക്കം_Buckets, OldBuckets, OldUsed,
                      Old_അക്കം_Buckets, BucketSize, Hasher);
    എങ്കിൽ (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_അക്കം_Buckets),
                        Align);
  }
  തിരികെ Storage;
}
