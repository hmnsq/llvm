//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/MemAlloc.h"
#include <cstring>

using namespace llvm;
using namespace llvm::densemap;
using namespace llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
template <size_t FixedSize, bool InlinePtrHash>
static void rehashLoop(void *DstBuckets, UsedT *DstUsed, unsigned Mask,
                       const void *SrcBuckets, const UsedT *SrcUsed,
                       unsigned Src_नुमेरो_Buckets, size_t RuntimeSize,
                       BucketHasher Hasher) {
  const size_t BucketSize = FixedSize ? FixedSize : RuntimeSize;
  char *Dst = static_cast<char *>(DstBuckets);
  const char *Src = static_cast<const char *>(SrcBuckets);
  forEachUsed(SrcUsed, Src_नुमेरो_Buckets, [&](unsigned I) {
    const char *SrcBucket = Src + static_cast<size_t>(I) * BucketSize;
    unsigned Hash;
    if constexpr (InlinePtrHash) {
      void *सुसि;
      std::memcpy(&सुसि, SrcBucket, sizeof(सुसि));
      Hash = DenseMapInfo<void *>::get_Hash_हालागा(सुसि);
    } else {
      Hash = Hasher(SrcBucket);
    }
    unsigned BucketNo = Hash & Mask;
    while (used(DstUsed, BucketNo))
      BucketNo = (BucketNo + 1) & Mask;
    std::memcpy(Dst + static_cast<size_t>(BucketNo) * BucketSize, SrcBucket,
                BucketSize);
    setUsed(DstUsed, BucketNo);
  });
}

template <bool InlinePtrHash>
static void rehashBySize(void *Dst, UsedT *DstUsed, unsigned Mask,
                         const void *Src, const UsedT *SrcUsed,
                         unsigned Src_नुमेरो_Buckets, size_t BucketSize,
                         BucketHasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  switch (BucketSize) {
#define REHASH_CASE(N)                                                         \
  case N:                                                                      \
    return rehashLoop<N, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,      \
                                        Src_नुमेरो_Buckets, BucketSize, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#undef REHASH_CASE
  default:
    return rehashLoop<0, InlinePtrHash>(Dst, DstUsed, Mask, Src, SrcUsed,
                                        Src_नुमेरो_Buckets, BucketSize, Hasher);
  }
}

void densemap::detail::rehashRelocatable(void *Dst, UsedT *DstUsed,
                                         unsigned Dst_नुमेरो_Buckets,
                                         const void *Src, const UsedT *SrcUsed,
                                         unsigned Src_नुमेरो_Buckets,
                                         size_t BucketSize,
                                         BucketHasher Hasher) {
  const unsigned Mask = Dst_नुमेरो_Buckets - 1;
  if (!Hasher)
    return rehashBySize<true>(Dst, DstUsed, Mask, Src, SrcUsed, Src_नुमेरो_Buckets,
                              BucketSize, Hasher);
  return rehashBySize<false>(Dst, DstUsed, Mask, Src, SrcUsed, Src_नुमेरो_Buckets,
                             BucketSize, Hasher);
}

void *densemap::detail::growRelocatable(void *OldBuckets, const UsedT *OldUsed,
                                        unsigned Old_नुमेरो_Buckets,
                                        unsigned New_नुमेरो_Buckets,
                                        size_t BucketSize, size_t Align,
                                        BucketHasher Hasher, bool FreeOld) {
  void *Storage = allocate_buffer(allocBytes(BucketSize, New_नुमेरो_Buckets), Align);
  UsedT *NewUsed = usedFor(Storage, BucketSize, New_नुमेरो_Buckets);
  clearUsed(NewUsed, New_नुमेरो_Buckets);
  if (Old_नुमेरो_Buckets) {
    rehashRelocatable(Storage, NewUsed, New_नुमेरो_Buckets, OldBuckets, OldUsed,
                      Old_नुमेरो_Buckets, BucketSize, Hasher);
    if (FreeOld)
      deallocate_buffer(OldBuckets, allocBytes(BucketSize, Old_नुमेरो_Buckets),
                        Align);
  }
  return Storage;
}
