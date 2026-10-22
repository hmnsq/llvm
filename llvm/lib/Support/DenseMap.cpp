//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/DenseMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <cstring>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::densemap;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
ಮಾದರಿ <size_t Fixed_ಗಾತ್ರ, ತಾರ್ಕಿಕ InlinePtrHash>
ಸ್ಥಿರ ಶೂನ್ಯ rehashLoop(ಶೂನ್ಯ *DstBuckets, ಬಳಸಿದ_T *Dst_ಬಳಸಿದ, ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ,
                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *SrcBuckets, ಸ್ಥಿರಾಂಕ ಬಳಸಿದ_T *Src_ಬಳಸಿದ,
                       ಚಿಹ್ನೆರಹಿತ Src_ಅಂಕೆ_Buckets, size_t Runtime_ಗಾತ್ರ,
                       ಪಾತ್ರೆ_Hasher Hasher) {
  ಸ್ಥಿರಾಂಕ size_t ಪಾತ್ರೆ_ಗಾತ್ರ = Fixed_ಗಾತ್ರ ? Fixed_ಗಾತ್ರ : Runtime_ಗಾತ್ರ;
  ಅಕ್ಷರ *Dst = static_cast<ಅಕ್ಷರ *>(DstBuckets);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Src = static_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(SrcBuckets);
  for_Each_ಬಳಸಿದ(Src_ಬಳಸಿದ, Src_ಅಂಕೆ_Buckets, [&](ಚಿಹ್ನೆರಹಿತ I) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Src_ಪಾತ್ರೆ = Src + static_cast<size_t>(I) * ಪಾತ್ರೆ_ಗಾತ್ರ;
    ಚಿಹ್ನೆರಹಿತ Hash;
    ಆದರೆ constexpr (InlinePtrHash) {
      ಶೂನ್ಯ *ಕೀಲಿ;
      std::ಸ್ಮರಣೆ_ನಕಲು(&ಕೀಲಿ, Src_ಪಾತ್ರೆ, ಗಾತ್ರ(ಕೀಲಿ));
      Hash = DenseMapInfo<ಶೂನ್ಯ *>::get_Hash_ಮೌಲ್ಯ(ಕೀಲಿ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      Hash = Hasher(Src_ಪಾತ್ರೆ);
    }
    ಚಿಹ್ನೆರಹಿತ ಪಾತ್ರೆ_No = Hash & ಮುಖವಾಡ;
    ಆಗ (ಬಳಸಿದ(Dst_ಬಳಸಿದ, ಪಾತ್ರೆ_No))
      ಪಾತ್ರೆ_No = (ಪಾತ್ರೆ_No + 1) & ಮುಖವಾಡ;
    std::ಸ್ಮರಣೆ_ನಕಲು(Dst + static_cast<size_t>(ಪಾತ್ರೆ_No) * ಪಾತ್ರೆ_ಗಾತ್ರ, Src_ಪಾತ್ರೆ,
                ಪಾತ್ರೆ_ಗಾತ್ರ);
    ಗಣ_ಬಳಸಿದ(Dst_ಬಳಸಿದ, ಪಾತ್ರೆ_No);
  });
}

ಮಾದರಿ <ತಾರ್ಕಿಕ InlinePtrHash>
ಸ್ಥಿರ ಶೂನ್ಯ rehash_By_ಗಾತ್ರ(ಶೂನ್ಯ *Dst, ಬಳಸಿದ_T *Dst_ಬಳಸಿದ, ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ,
                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Src, ಸ್ಥಿರಾಂಕ ಬಳಸಿದ_T *Src_ಬಳಸಿದ,
                         ಚಿಹ್ನೆರಹಿತ Src_ಅಂಕೆ_Buckets, size_t ಪಾತ್ರೆ_ಗಾತ್ರ,
                         ಪಾತ್ರೆ_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ಆಯ್ಕೆ (ಪಾತ್ರೆ_ಗಾತ್ರ) {
#ವ್ಯಾಖ್ಯಾನಿಸು REHASH_CASE(N)                                                         \
  ಸ್ಥಿತಿ N:                                                                      \
    ಹಿಂತಿರುಗಿ rehashLoop<N, InlinePtrHash>(Dst, Dst_ಬಳಸಿದ, ಮುಖವಾಡ, Src, Src_ಬಳಸಿದ,      \
                                        Src_ಅಂಕೆ_Buckets, ಪಾತ್ರೆ_ಗಾತ್ರ, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು REHASH_CASE
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ rehashLoop<0, InlinePtrHash>(Dst, Dst_ಬಳಸಿದ, ಮುಖವಾಡ, Src, Src_ಬಳಸಿದ,
                                        Src_ಅಂಕೆ_Buckets, ಪಾತ್ರೆ_ಗಾತ್ರ, Hasher);
  }
}

ಶೂನ್ಯ densemap::detail::rehashRelocatable(ಶೂನ್ಯ *Dst, ಬಳಸಿದ_T *Dst_ಬಳಸಿದ,
                                         ಚಿಹ್ನೆರಹಿತ Dst_ಅಂಕೆ_Buckets,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Src, ಸ್ಥಿರಾಂಕ ಬಳಸಿದ_T *Src_ಬಳಸಿದ,
                                         ಚಿಹ್ನೆರಹಿತ Src_ಅಂಕೆ_Buckets,
                                         size_t ಪಾತ್ರೆ_ಗಾತ್ರ,
                                         ಪಾತ್ರೆ_Hasher Hasher) {
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = Dst_ಅಂಕೆ_Buckets - 1;
  ಆದರೆ (!Hasher)
    ಹಿಂತಿರುಗಿ rehash_By_ಗಾತ್ರ<ನಿಜ>(Dst, Dst_ಬಳಸಿದ, ಮುಖವಾಡ, Src, Src_ಬಳಸಿದ, Src_ಅಂಕೆ_Buckets,
                              ಪಾತ್ರೆ_ಗಾತ್ರ, Hasher);
  ಹಿಂತಿರುಗಿ rehash_By_ಗಾತ್ರ<ಸುಳ್ಳು>(Dst, Dst_ಬಳಸಿದ, ಮುಖವಾಡ, Src, Src_ಬಳಸಿದ, Src_ಅಂಕೆ_Buckets,
                             ಪಾತ್ರೆ_ಗಾತ್ರ, Hasher);
}

ಶೂನ್ಯ *densemap::detail::growRelocatable(ಶೂನ್ಯ *OldBuckets, ಸ್ಥಿರಾಂಕ ಬಳಸಿದ_T *Old_ಬಳಸಿದ,
                                        ಚಿಹ್ನೆರಹಿತ Old_ಅಂಕೆ_Buckets,
                                        ಚಿಹ್ನೆರಹಿತ New_ಅಂಕೆ_Buckets,
                                        size_t ಪಾತ್ರೆ_ಗಾತ್ರ, size_t Align,
                                        ಪಾತ್ರೆ_Hasher Hasher, ತಾರ್ಕಿಕ FreeOld) {
  ಶೂನ್ಯ *Storage = allocate_ಬಫರ್(alloc_ಬೈಟುಗಳು(ಪಾತ್ರೆ_ಗಾತ್ರ, New_ಅಂಕೆ_Buckets), Align);
  ಬಳಸಿದ_T *New_ಬಳಸಿದ = ಬಳಸಿದ_For(Storage, ಪಾತ್ರೆ_ಗಾತ್ರ, New_ಅಂಕೆ_Buckets);
  clear_ಬಳಸಿದ(New_ಬಳಸಿದ, New_ಅಂಕೆ_Buckets);
  ಆದರೆ (Old_ಅಂಕೆ_Buckets) {
    rehashRelocatable(Storage, New_ಬಳಸಿದ, New_ಅಂಕೆ_Buckets, OldBuckets, Old_ಬಳಸಿದ,
                      Old_ಅಂಕೆ_Buckets, ಪಾತ್ರೆ_ಗಾತ್ರ, Hasher);
    ಆದರೆ (FreeOld)
      deallocate_ಬಫರ್(OldBuckets, alloc_ಬೈಟುಗಳು(ಪಾತ್ರೆ_ಗಾತ್ರ, Old_ಅಂಕೆ_Buckets),
                        Align);
  }
  ಹಿಂತಿರುಗಿ Storage;
}
