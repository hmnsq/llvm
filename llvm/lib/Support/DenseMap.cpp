//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/DenseMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemAlloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::densemap;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
ਸਾਂਚਾ <size_t Fixed_ਮਾਪ, ਤਾਰਕਿਕ InlinePtrHash>
ਸਥਿਰ ਖਾਲੀ rehashLoop(ਖਾਲੀ *DstBuckets, ਵਰਤਿਆ_T *Dst_ਵਰਤਿਆ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ,
                       ਅਚਲ ਖਾਲੀ *SrcBuckets, ਅਚਲ ਵਰਤਿਆ_T *Src_ਵਰਤਿਆ,
                       ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਅੰਕ_Buckets, size_t Runtime_ਮਾਪ,
                       ਬਾਲਟੀ_Hasher Hasher) {
  ਅਚਲ size_t ਬਾਲਟੀ_ਮਾਪ = Fixed_ਮਾਪ ? Fixed_ਮਾਪ : Runtime_ਮਾਪ;
  ਅੱਖਰ *Dst = static_cast<ਅੱਖਰ *>(DstBuckets);
  ਅਚਲ ਅੱਖਰ *Src = static_cast<ਅਚਲ ਅੱਖਰ *>(SrcBuckets);
  for_Each_ਵਰਤਿਆ(Src_ਵਰਤਿਆ, Src_ਅੰਕ_Buckets, [&](ਬਿਨਾਨਿਸ਼ਾਨੀ I) {
    ਅਚਲ ਅੱਖਰ *Src_ਬਾਲਟੀ = Src + static_cast<size_t>(I) * ਬਾਲਟੀ_ਮਾਪ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ Hash;
    ਜੇ constexpr (InlinePtrHash) {
      ਖਾਲੀ *ਕੁੰਜੀ;
      std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਕੁੰਜੀ, Src_ਬਾਲਟੀ, ਆਕਾਰ(ਕੁੰਜੀ));
      Hash = DenseMapInfo<ਖਾਲੀ *>::get_Hash_ਮੁੱਲ(ਕੁੰਜੀ);
    } ਨਹੀਂਤਾਂ {
      Hash = Hasher(Src_ਬਾਲਟੀ);
    }
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਲਟੀ_No = Hash & ਨਕਾਬ;
    ਜਦੋਂ (ਵਰਤਿਆ(Dst_ਵਰਤਿਆ, ਬਾਲਟੀ_No))
      ਬਾਲਟੀ_No = (ਬਾਲਟੀ_No + 1) & ਨਕਾਬ;
    std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst + static_cast<size_t>(ਬਾਲਟੀ_No) * ਬਾਲਟੀ_ਮਾਪ, Src_ਬਾਲਟੀ,
                ਬਾਲਟੀ_ਮਾਪ);
    ਸੈੱਟ_ਵਰਤਿਆ(Dst_ਵਰਤਿਆ, ਬਾਲਟੀ_No);
  });
}

ਸਾਂਚਾ <ਤਾਰਕਿਕ InlinePtrHash>
ਸਥਿਰ ਖਾਲੀ rehash_By_ਮਾਪ(ਖਾਲੀ *Dst, ਵਰਤਿਆ_T *Dst_ਵਰਤਿਆ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ,
                         ਅਚਲ ਖਾਲੀ *Src, ਅਚਲ ਵਰਤਿਆ_T *Src_ਵਰਤਿਆ,
                         ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਅੰਕ_Buckets, size_t ਬਾਲਟੀ_ਮਾਪ,
                         ਬਾਲਟੀ_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ਚੋਣ (ਬਾਲਟੀ_ਮਾਪ) {
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REHASH_CASE(N)                                                         \
  ਹਾਲਤ N:                                                                      \
    ਵਾਪਸ rehashLoop<N, InlinePtrHash>(Dst, Dst_ਵਰਤਿਆ, ਨਕਾਬ, Src, Src_ਵਰਤਿਆ,      \
                                        Src_ਅੰਕ_Buckets, ਬਾਲਟੀ_ਮਾਪ, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ REHASH_CASE
  ਮੂਲ:
    ਵਾਪਸ rehashLoop<0, InlinePtrHash>(Dst, Dst_ਵਰਤਿਆ, ਨਕਾਬ, Src, Src_ਵਰਤਿਆ,
                                        Src_ਅੰਕ_Buckets, ਬਾਲਟੀ_ਮਾਪ, Hasher);
  }
}

ਖਾਲੀ densemap::detail::rehashRelocatable(ਖਾਲੀ *Dst, ਵਰਤਿਆ_T *Dst_ਵਰਤਿਆ,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ Dst_ਅੰਕ_Buckets,
                                         ਅਚਲ ਖਾਲੀ *Src, ਅਚਲ ਵਰਤਿਆ_T *Src_ਵਰਤਿਆ,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਅੰਕ_Buckets,
                                         size_t ਬਾਲਟੀ_ਮਾਪ,
                                         ਬਾਲਟੀ_Hasher Hasher) {
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = Dst_ਅੰਕ_Buckets - 1;
  ਜੇ (!Hasher)
    ਵਾਪਸ rehash_By_ਮਾਪ<ਸੱਚ>(Dst, Dst_ਵਰਤਿਆ, ਨਕਾਬ, Src, Src_ਵਰਤਿਆ, Src_ਅੰਕ_Buckets,
                              ਬਾਲਟੀ_ਮਾਪ, Hasher);
  ਵਾਪਸ rehash_By_ਮਾਪ<ਝੂਠ>(Dst, Dst_ਵਰਤਿਆ, ਨਕਾਬ, Src, Src_ਵਰਤਿਆ, Src_ਅੰਕ_Buckets,
                             ਬਾਲਟੀ_ਮਾਪ, Hasher);
}

ਖਾਲੀ *densemap::detail::growRelocatable(ਖਾਲੀ *OldBuckets, ਅਚਲ ਵਰਤਿਆ_T *Old_ਵਰਤਿਆ,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ Old_ਅੰਕ_Buckets,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਅੰਕ_Buckets,
                                        size_t ਬਾਲਟੀ_ਮਾਪ, size_t Align,
                                        ਬਾਲਟੀ_Hasher Hasher, ਤਾਰਕਿਕ FreeOld) {
  ਖਾਲੀ *Storage = allocate_ਬਫਰ(alloc_ਬਾਈਟਾਂ(ਬਾਲਟੀ_ਮਾਪ, New_ਅੰਕ_Buckets), Align);
  ਵਰਤਿਆ_T *New_ਵਰਤਿਆ = ਵਰਤਿਆ_For(Storage, ਬਾਲਟੀ_ਮਾਪ, New_ਅੰਕ_Buckets);
  clear_ਵਰਤਿਆ(New_ਵਰਤਿਆ, New_ਅੰਕ_Buckets);
  ਜੇ (Old_ਅੰਕ_Buckets) {
    rehashRelocatable(Storage, New_ਵਰਤਿਆ, New_ਅੰਕ_Buckets, OldBuckets, Old_ਵਰਤਿਆ,
                      Old_ਅੰਕ_Buckets, ਬਾਲਟੀ_ਮਾਪ, Hasher);
    ਜੇ (FreeOld)
      deallocate_ਬਫਰ(OldBuckets, alloc_ਬਾਈਟਾਂ(ਬਾਲਟੀ_ਮਾਪ, Old_ਅੰਕ_Buckets),
                        Align);
  }
  ਵਾਪਸ Storage;
}
