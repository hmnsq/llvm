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

#অন্তর্ভাব "llvm/ADT/FoldingSet.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Allocator.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/SwapByteOrder.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstring>
using নামাকাশ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

যৌক্তিক llvm::operator<(Folding_Set_গ্রন্থি_IDRef LHS, Folding_Set_গ্রন্থি_IDRef RHS) {
  যদি (LHS.আকার_3() != RHS.আকার_3())
    ফেরত LHS.আকার_3() < RHS.আকার_3();
  ফেরত memcmp(LHS.তথ্য(), RHS.তথ্য(), LHS.আকার_3() * আকার(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

শূন্য Folding_Set_গ্রন্থি_ID::AddString(StringRef String) {
  unsigned আকার_2 = String.আকার_3();

  unsigned অঙ্ক_Inserts = 1 + divideCeil(আকার_2, 4);
  Bits.reserve(Bits.আকার_3() + অঙ্ক_Inserts);

  Bits.ঠেলো_back(আকার_2);
  যদি (!আকার_2)
    ফেরত;

  unsigned Units = আকার_2 / 4;
  unsigned Pos = 0;
  ধ্রুবক unsigned *Base = (ধ্রুবক unsigned *)String.তথ্য();

  // If the string is aligned do a bulk transfer.
  যদি (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } অন্যথা {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    যদি (sys::IsBigEndianHost) {
      জন্য (Pos += 4; Pos <= আকার_2; Pos += 4) {
        unsigned V = ((unsigned অক্ষর)String[Pos - 4] << 24) |
                     ((unsigned অক্ষর)String[Pos - 3] << 16) |
                     ((unsigned অক্ষর)String[Pos - 2] << 8) |
                     (unsigned অক্ষর)String[Pos - 1];
        Bits.ঠেলো_back(V);
      }
    } অন্যথা { // Little-endian host
      জন্য (Pos += 4; Pos <= আকার_2; Pos += 4) {
        unsigned V = ((unsigned অক্ষর)String[Pos - 1] << 24) |
                     ((unsigned অক্ষর)String[Pos - 2] << 16) |
                     ((unsigned অক্ষর)String[Pos - 3] << 8) |
                     (unsigned অক্ষর)String[Pos - 4];
        Bits.ঠেলো_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  নির্বাচন (Pos - আকার_2) {
  ক্ষেত্র 1:
    V = (V << 8) | (unsigned অক্ষর)String[আকার_2 - 3];
    [[fallthrough]];
  ক্ষেত্র 2:
    V = (V << 8) | (unsigned অক্ষর)String[আকার_2 - 2];
    [[fallthrough]];
  ক্ষেত্র 3:
    V = (V << 8) | (unsigned অক্ষর)String[আকার_2 - 1];
    বিরতি;
  পূর্বনির্ধারিত:
    ফেরত; // Nothing left.
  }

  Bits.ঠেলো_back(V);
}

শূন্য Folding_Set_গ্রন্থি_ID::Add_গ্রন্থি_ID(ধ্রুবক Folding_Set_গ্রন্থি_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_গ্রন্থি_IDRef
Folding_Set_গ্রন্থি_ID::Intern(BumpPtrAllocator &Allocator) ধ্রুবক {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.আকার_3());
  llvm::uninitialized_copy(Bits, New);
  ফেরত Folding_Set_গ্রন্থি_IDRef(New, Bits.আকার_3());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2_Init_আকার) {
  প্রতিজ্ঞা(5 < Log2_Init_আকার && Log2_Init_আকার < 32 &&
         "Initial hash table size out of range");
  অঙ্ক_Buckets = 1 << Log2_Init_আকার;
  Buckets = static_cast<Folding_Set_গ্রন্থি **>(
      safe_calloc(অঙ্ক_Buckets, আকার(Folding_Set_গ্রন্থি *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      অঙ্ক_Buckets(std::exchange(Arg.অঙ্ক_Buckets, 0)),
      অঙ্ক_Nodes(std::exchange(Arg.অঙ্ক_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  যদি (এই == &RHS)
    ফেরত *এই;

  incrementEpoch();
  RHS.incrementEpoch();
  মুক্ত(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  অঙ্ক_Buckets = std::exchange(RHS.অঙ্ক_Buckets, 0);
  অঙ্ক_Nodes = std::exchange(RHS.অঙ্ক_Nodes, 0);
  ফেরত *এই;
}

FoldingSetBase::~FoldingSetBase() { মুক্ত(Buckets); }

শূন্য FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  যদি (অঙ্ক_Buckets)
    memset(Buckets, 0, অঙ্ক_Buckets * আকার(Folding_Set_গ্রন্থি *));
  অঙ্ক_Nodes = 0;
}

শূন্য FoldingSetBase::place_গ্রন্থি(Folding_Set_গ্রন্থি *N, uint32_t Hash) {
  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Hash & Mask;
  যখন (Buckets[I]) {
    প্রতিজ্ঞা(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++অঙ্ক_Nodes;
}

শূন্য FoldingSetBase::grow(unsigned Min_অঙ্ক_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_গুনতি = std::max(64u, llvm::bit_ceil(Min_অঙ্ক_Buckets));
  প্রতিজ্ঞা(New_Bucket_গুনতি > অঙ্ক_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_গুনতি));
  জন্য (unsigned I = 0; I != অঙ্ক_Buckets; ++I)
    যদি (Folding_Set_গ্রন্থি *N = Buckets[I])
      Tmp.place_গ্রন্থি(N, N->getFoldingSetHash());

  *এই = std::move(Tmp);
}

শূন্য FoldingSetBase::reserve(unsigned N) {
  যদি (N * 4 <= অঙ্ক_Buckets * 3)
    ফেরত;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

শূন্য FoldingSetBase::ঢোকাও(Folding_Set_গ্রন্থি *N, Folding_Set_ঢোকাও_Token Token) {
  প্রতিজ্ঞা(N && "Cannot insert a null node");
  প্রতিজ্ঞা(Token && "Invalid token!");
  incrementEpoch();
  যদি (LLVM_UNLIKELY((অঙ্ক_Nodes + 1) * 4 > অঙ্ক_Buckets * 3))
    grow(অঙ্ক_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_গ্রন্থি(N, Hash);
  N->setFoldingSetHash(Hash);
}

যৌক্তিক FoldingSetBase::erase(Folding_Set_গ্রন্থি *N) {
  uint32_t Hash = N->getFoldingSetHash();
  যদি (Hash == Folding_Set_গ্রন্থি_IDRef::NotAHash)
    ফেরত মিথ্যা; // Never inserted.

  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Hash & Mask;
  যখন (Buckets[I] != N) {
    যদি (LLVM_UNLIKELY(!Buckets[I]))
      ফেরত মিথ্যা; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  জন্য (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    যদি (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_গ্রন্থি_IDRef::NotAHash);
  --অঙ্ক_Nodes;
  ফেরত সত্য;
}
