//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/DenseMap.h"
#అన్తర్భావ "llvm/Support/MemAlloc.h"
#అన్తర్భావ <cstring>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::densemap;
ఉపయోగించి నామాకాశ llvm::densemap::detail;

// A nonzero FixedSize turns the bucket copy into a couple of stores.
మూస <size_t Fixed_కొలత, తార్కిక InlinePtrHash>
స్థిర శూన్య rehashLoop(శూన్య *DstBuckets, వాడిన_T *Dst_వాడిన, గుర్తులేని ముసుగు,
                       స్థిరరాశి శూన్య *SrcBuckets, స్థిరరాశి వాడిన_T *Src_వాడిన,
                       గుర్తులేని Src_అంకె_Buckets, size_t Runtime_కొలత,
                       పాత్ర_Hasher Hasher) {
  స్థిరరాశి size_t పాత్ర_కొలత = Fixed_కొలత ? Fixed_కొలత : Runtime_కొలత;
  అక్షరం *Dst = static_cast<అక్షరం *>(DstBuckets);
  స్థిరరాశి అక్షరం *Src = static_cast<స్థిరరాశి అక్షరం *>(SrcBuckets);
  for_Each_వాడిన(Src_వాడిన, Src_అంకె_Buckets, [&](గుర్తులేని I) {
    స్థిరరాశి అక్షరం *Src_పాత్ర = Src + static_cast<size_t>(I) * పాత్ర_కొలత;
    గుర్తులేని Hash;
    ఒకవేళ constexpr (InlinePtrHash) {
      శూన్య *కీలకం;
      std::జ్ఞాపకం_నకలు(&కీలకం, Src_పాత్ర, పరిమాణం(కీలకం));
      Hash = DenseMapInfo<శూన్య *>::get_Hash_విలువ(కీలకం);
    } లేకపోతే {
      Hash = Hasher(Src_పాత్ర);
    }
    గుర్తులేని పాత్ర_No = Hash & ముసుగు;
    అయితే (వాడిన(Dst_వాడిన, పాత్ర_No))
      పాత్ర_No = (పాత్ర_No + 1) & ముసుగు;
    std::జ్ఞాపకం_నకలు(Dst + static_cast<size_t>(పాత్ర_No) * పాత్ర_కొలత, Src_పాత్ర,
                పాత్ర_కొలత);
    సమితి_వాడిన(Dst_వాడిన, పాత్ర_No);
  });
}

మూస <తార్కిక InlinePtrHash>
స్థిర శూన్య rehash_By_కొలత(శూన్య *Dst, వాడిన_T *Dst_వాడిన, గుర్తులేని ముసుగు,
                         స్థిరరాశి శూన్య *Src, స్థిరరాశి వాడిన_T *Src_వాడిన,
                         గుర్తులేని Src_అంకె_Buckets, size_t పాత్ర_కొలత,
                         పాత్ర_Hasher Hasher) {
  // The bucket sizes of 95% of the grow instantiations in an LLVM build.
  ఎంపిక (పాత్ర_కొలత) {
#నిర్వచించు REHASH_CASE(N)                                                         \
  స్థితి N:                                                                      \
    తిరిగి rehashLoop<N, InlinePtrHash>(Dst, Dst_వాడిన, ముసుగు, Src, Src_వాడిన,      \
                                        Src_అంకె_Buckets, పాత్ర_కొలత, Hasher);
    REHASH_CASE(4)
    REHASH_CASE(8)
    REHASH_CASE(12)
    REHASH_CASE(16)
    REHASH_CASE(24)
    REHASH_CASE(32)
    REHASH_CASE(40)
    REHASH_CASE(48)
#నిర్వచనం_రద్దు REHASH_CASE
  అప్రమేయ:
    తిరిగి rehashLoop<0, InlinePtrHash>(Dst, Dst_వాడిన, ముసుగు, Src, Src_వాడిన,
                                        Src_అంకె_Buckets, పాత్ర_కొలత, Hasher);
  }
}

శూన్య densemap::detail::rehashRelocatable(శూన్య *Dst, వాడిన_T *Dst_వాడిన,
                                         గుర్తులేని Dst_అంకె_Buckets,
                                         స్థిరరాశి శూన్య *Src, స్థిరరాశి వాడిన_T *Src_వాడిన,
                                         గుర్తులేని Src_అంకె_Buckets,
                                         size_t పాత్ర_కొలత,
                                         పాత్ర_Hasher Hasher) {
  స్థిరరాశి గుర్తులేని ముసుగు = Dst_అంకె_Buckets - 1;
  ఒకవేళ (!Hasher)
    తిరిగి rehash_By_కొలత<నిజం>(Dst, Dst_వాడిన, ముసుగు, Src, Src_వాడిన, Src_అంకె_Buckets,
                              పాత్ర_కొలత, Hasher);
  తిరిగి rehash_By_కొలత<అబద్ధం>(Dst, Dst_వాడిన, ముసుగు, Src, Src_వాడిన, Src_అంకె_Buckets,
                             పాత్ర_కొలత, Hasher);
}

శూన్య *densemap::detail::growRelocatable(శూన్య *OldBuckets, స్థిరరాశి వాడిన_T *Old_వాడిన,
                                        గుర్తులేని Old_అంకె_Buckets,
                                        గుర్తులేని New_అంకె_Buckets,
                                        size_t పాత్ర_కొలత, size_t Align,
                                        పాత్ర_Hasher Hasher, తార్కిక FreeOld) {
  శూన్య *Storage = allocate_బఫరు(alloc_బైట్లు(పాత్ర_కొలత, New_అంకె_Buckets), Align);
  వాడిన_T *New_వాడిన = వాడిన_For(Storage, పాత్ర_కొలత, New_అంకె_Buckets);
  clear_వాడిన(New_వాడిన, New_అంకె_Buckets);
  ఒకవేళ (Old_అంకె_Buckets) {
    rehashRelocatable(Storage, New_వాడిన, New_అంకె_Buckets, OldBuckets, Old_వాడిన,
                      Old_అంకె_Buckets, పాత్ర_కొలత, Hasher);
    ఒకవేళ (FreeOld)
      deallocate_బఫరు(OldBuckets, alloc_బైట్లు(పాత్ర_కొలత, Old_అంకె_Buckets),
                        Align);
  }
  తిరిగి Storage;
}
