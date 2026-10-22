//===-- Support/FoldingSet.cpp - Uniquing Hash Set --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a hash set that can be used to remove duplication of
// nodes in a graph.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/FoldingSet.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Allocator.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/SwapByteOrder.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstring>
ఉపయోగించి నామాకాశ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

తార్కిక llvm::క్రియాకారకం<(Folding_సమితి_కణుపు_IDRef LHS, Folding_సమితి_కణుపు_IDRef RHS) {
  ఒకవేళ (LHS.కొలత_2() != RHS.కొలత_2())
    తిరిగి LHS.కొలత_2() < RHS.కొలత_2();
  తిరిగి memcmp(LHS.దత్తాంశం(), RHS.దత్తాంశం(), LHS.కొలత_2() * పరిమాణం(గుర్తులేని)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

శూన్య Folding_సమితి_కణుపు_ID::కూడు_String(StringRef String) {
  గుర్తులేని కొలత = String.కొలత_2();

  గుర్తులేని అంకె_Inserts = 1 + divideCeil(కొలత, 4);
  బిట్లు_2.reserve(బిట్లు_2.కొలత_2() + అంకె_Inserts);

  బిట్లు_2.నెట్టు_వెనుక(కొలత);
  ఒకవేళ (!కొలత)
    తిరిగి;

  గుర్తులేని Units = కొలత / 4;
  గుర్తులేని Pos = 0;
  స్థిరరాశి గుర్తులేని *పునాది = (స్థిరరాశి గుర్తులేని *)String.దత్తాంశం();

  // If the string is aligned do a bulk transfer.
  ఒకవేళ (!((intptr_t)పునాది & 3)) {
    బిట్లు_2.append(పునాది, పునాది + Units);
    Pos = (Units + 1) * 4;
  } లేకపోతే {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    ఒకవేళ (sys::IsBigEndianHost) {
      కొరకు (Pos += 4; Pos <= కొలత; Pos += 4) {
        గుర్తులేని V = ((గుర్తులేని అక్షరం)String[Pos - 4] << 24) |
                     ((గుర్తులేని అక్షరం)String[Pos - 3] << 16) |
                     ((గుర్తులేని అక్షరం)String[Pos - 2] << 8) |
                     (గుర్తులేని అక్షరం)String[Pos - 1];
        బిట్లు_2.నెట్టు_వెనుక(V);
      }
    } లేకపోతే { // Little-endian host
      కొరకు (Pos += 4; Pos <= కొలత; Pos += 4) {
        గుర్తులేని V = ((గుర్తులేని అక్షరం)String[Pos - 1] << 24) |
                     ((గుర్తులేని అక్షరం)String[Pos - 2] << 16) |
                     ((గుర్తులేని అక్షరం)String[Pos - 3] << 8) |
                     (గుర్తులేని అక్షరం)String[Pos - 4];
        బిట్లు_2.నెట్టు_వెనుక(V);
      }
    }
  }

  // With the leftover bits.
  గుర్తులేని V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ఎంపిక (Pos - కొలత) {
  స్థితి 1:
    V = (V << 8) | (గుర్తులేని అక్షరం)String[కొలత - 3];
    [[fallthrough]];
  స్థితి 2:
    V = (V << 8) | (గుర్తులేని అక్షరం)String[కొలత - 2];
    [[fallthrough]];
  స్థితి 3:
    V = (V << 8) | (గుర్తులేని అక్షరం)String[కొలత - 1];
    ఆపు;
  అప్రమేయ:
    తిరిగి; // Nothing left.
  }

  బిట్లు_2.నెట్టు_వెనుక(V);
}

శూన్య Folding_సమితి_కణుపు_ID::కూడు_కణుపు_ID(స్థిరరాశి Folding_సమితి_కణుపు_ID &ID) {
  బిట్లు_2.append(ID.బిట్లు_2.begin(), ID.బిట్లు_2.చివర());
}

Folding_సమితి_కణుపు_IDRef
Folding_సమితి_కణుపు_ID::Intern(BumpPtrAllocator &Allocator) స్థిరరాశి {
  గుర్తులేని *New = Allocator.Allocate<గుర్తులేని>(బిట్లు_2.కొలత_2());
  llvm::uninitialized_copy(బిట్లు_2, New);
  తిరిగి Folding_సమితి_కణుపు_IDRef(New, బిట్లు_2.కొలత_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_సమితి_పునాది::Folding_సమితి_పునాది(గుర్తులేని Log2_Init_కొలత) {
  ప్రతిజ్ఞా(5 < Log2_Init_కొలత && Log2_Init_కొలత < 32 &&
         "Initial hash table size out of range");
  అంకె_Buckets = 1 << Log2_Init_కొలత;
  Buckets = static_cast<Folding_సమితి_కణుపు **>(
      సురక్షిత_calloc(అంకె_Buckets, పరిమాణం(Folding_సమితి_కణుపు *)));
}

Folding_సమితి_పునాది::Folding_సమితి_పునాది(Folding_సమితి_పునాది &&Arg)
    : Buckets(std::exchange(Arg.Buckets, శూన్యసూచిక)),
      అంకె_Buckets(std::exchange(Arg.అంకె_Buckets, 0)),
      అంకె_Nodes(std::exchange(Arg.అంకె_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_సమితి_పునాది &Folding_సమితి_పునాది::క్రియాకారకం=(Folding_సమితి_పునాది &&RHS) {
  ఒకవేళ (ఇది == &RHS)
    తిరిగి *ఇది;

  incrementEpoch();
  RHS.incrementEpoch();
  విడుదల(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, శూన్యసూచిక);
  అంకె_Buckets = std::exchange(RHS.అంకె_Buckets, 0);
  అంకె_Nodes = std::exchange(RHS.అంకె_Nodes, 0);
  తిరిగి *ఇది;
}

Folding_సమితి_పునాది::~Folding_సమితి_పునాది() { విడుదల(Buckets); }

శూన్య Folding_సమితి_పునాది::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  ఒకవేళ (అంకె_Buckets)
    memset(Buckets, 0, అంకె_Buckets * పరిమాణం(Folding_సమితి_కణుపు *));
  అంకె_Nodes = 0;
}

శూన్య Folding_సమితి_పునాది::ఉంచు_కణుపు(Folding_సమితి_కణుపు *N, uint32_t Hash) {
  గుర్తులేని ముసుగు = అంకె_Buckets - 1;
  గుర్తులేని I = Hash & ముసుగు;
  అయితే (Buckets[I]) {
    ప్రతిజ్ఞా(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & ముసుగు;
  }
  Buckets[I] = N;
  ++అంకె_Nodes;
}

శూన్య Folding_సమితి_పునాది::grow(గుర్తులేని Min_అంకె_Buckets) {
  // The floor is the smallest size the constructor accepts.
  గుర్తులేని New_పాత్ర_లెక్క = std::max(64u, llvm::బిట్టు_ceil(Min_అంకె_Buckets));
  ప్రతిజ్ఞా(New_పాత్ర_లెక్క > అంకె_Buckets && "Can't shrink a folding set");

  Folding_సమితి_పునాది Tmp(llvm::Log2_32(New_పాత్ర_లెక్క));
  కొరకు (గుర్తులేని I = 0; I != అంకె_Buckets; ++I)
    ఒకవేళ (Folding_సమితి_కణుపు *N = Buckets[I])
      Tmp.ఉంచు_కణుపు(N, N->get_Folding_సమితి_Hash());

  *ఇది = std::move(Tmp);
}

శూన్య Folding_సమితి_పునాది::reserve(గుర్తులేని N) {
  ఒకవేళ (N * 4 <= అంకె_Buckets * 3)
    తిరిగి;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

శూన్య Folding_సమితి_పునాది::చొప్పించు(Folding_సమితి_కణుపు *N, Folding_సమితి_చొప్పించు_Token Token) {
  ప్రతిజ్ఞా(N && "Cannot insert a null node");
  ప్రతిజ్ఞా(Token && "Invalid token!");
  incrementEpoch();
  ఒకవేళ (LLVM_UNLIKELY((అంకె_Nodes + 1) * 4 > అంకె_Buckets * 3))
    grow(అంకె_Buckets * 2);
  uint32_t Hash = Token.Hash;
  ఉంచు_కణుపు(N, Hash);
  N->సమితి_Folding_సమితి_Hash(Hash);
}

తార్కిక Folding_సమితి_పునాది::erase(Folding_సమితి_కణుపు *N) {
  uint32_t Hash = N->get_Folding_సమితి_Hash();
  ఒకవేళ (Hash == Folding_సమితి_కణుపు_IDRef::NotAHash)
    తిరిగి అబద్ధం; // Never inserted.

  గుర్తులేని ముసుగు = అంకె_Buckets - 1;
  గుర్తులేని I = Hash & ముసుగు;
  అయితే (Buckets[I] != N) {
    ఒకవేళ (LLVM_UNLIKELY(!Buckets[I]))
      తిరిగి అబద్ధం; // Not in folding set.
    I = (I + 1) & ముసుగు;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  కొరకు (గుర్తులేని J = (I + 1) & ముసుగు; Buckets[J]; J = (J + 1) & ముసుగు) {
    గుర్తులేని Ideal = Buckets[J]->get_Folding_సమితి_Hash();
    ఒకవేళ (((I - Ideal) & ముసుగు) < ((J - Ideal) & ముసుగు)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = శూన్యసూచిక;
  N->సమితి_Folding_సమితి_Hash(Folding_సమితి_కణుపు_IDRef::NotAHash);
  --అంకె_Nodes;
  తిరిగి నిజం;
}
