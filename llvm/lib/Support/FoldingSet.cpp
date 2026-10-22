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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FoldingSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Allocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SwapByteOrder.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstring>
using ନାମାକାଶ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

ତାର୍କିକ llvm::operator<(Folding_Set_ଗଣ୍ଠି_IDRef LHS, Folding_Set_ଗଣ୍ଠି_IDRef RHS) {
  ଯଦି (LHS.size() != RHS.size())
    ଫେରନ୍ତୁ LHS.size() < RHS.size();
  ଫେରନ୍ତୁ memcmp(LHS.data(), RHS.data(), LHS.size() * ଆକାର(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

ଶୂନ୍ୟ Folding_Set_ଗଣ୍ଠି_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned ଅଙ୍କ_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + ଅଙ୍କ_Inserts);

  Bits.ଠେଲ_back(Size);
  ଯଦି (!Size)
    ଫେରନ୍ତୁ;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ଅଚଳ unsigned *Base = (ଅଚଳ unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  ଯଦି (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } ଅନ୍ୟଥା {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    ଯଦି (sys::IsBigEndianHost) {
      ପାଇଁ (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ଅକ୍ଷର)String[Pos - 4] << 24) |
                     ((unsigned ଅକ୍ଷର)String[Pos - 3] << 16) |
                     ((unsigned ଅକ୍ଷର)String[Pos - 2] << 8) |
                     (unsigned ଅକ୍ଷର)String[Pos - 1];
        Bits.ଠେଲ_back(V);
      }
    } ଅନ୍ୟଥା { // Little-endian host
      ପାଇଁ (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ଅକ୍ଷର)String[Pos - 1] << 24) |
                     ((unsigned ଅକ୍ଷର)String[Pos - 2] << 16) |
                     ((unsigned ଅକ୍ଷର)String[Pos - 3] << 8) |
                     (unsigned ଅକ୍ଷର)String[Pos - 4];
        Bits.ଠେଲ_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ବାଛ (Pos - Size) {
  ସ୍ଥିତି 1:
    V = (V << 8) | (unsigned ଅକ୍ଷର)String[Size - 3];
    [[fallthrough]];
  ସ୍ଥିତି 2:
    V = (V << 8) | (unsigned ଅକ୍ଷର)String[Size - 2];
    [[fallthrough]];
  ସ୍ଥିତି 3:
    V = (V << 8) | (unsigned ଅକ୍ଷର)String[Size - 1];
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ; // Nothing left.
  }

  Bits.ଠେଲ_back(V);
}

ଶୂନ୍ୟ Folding_Set_ଗଣ୍ଠି_ID::Add_ଗଣ୍ଠି_ID(ଅଚଳ Folding_Set_ଗଣ୍ଠି_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_ଗଣ୍ଠି_IDRef
Folding_Set_ଗଣ୍ଠି_ID::Intern(BumpPtrAllocator &Allocator) ଅଚଳ {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  ଫେରନ୍ତୁ Folding_Set_ଗଣ୍ଠି_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  ପ୍ରତିଜ୍ଞା(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  ଅଙ୍କ_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_ଗଣ୍ଠି **>(
      safe_calloc(ଅଙ୍କ_Buckets, ଆକାର(Folding_Set_ଗଣ୍ଠି *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      ଅଙ୍କ_Buckets(std::exchange(Arg.ଅଙ୍କ_Buckets, 0)),
      ଅଙ୍କ_Nodes(std::exchange(Arg.ଅଙ୍କ_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  ଯଦି (ଏହି == &RHS)
    ଫେରନ୍ତୁ *ଏହି;

  incrementEpoch();
  RHS.incrementEpoch();
  ମୁକ୍ତ(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  ଅଙ୍କ_Buckets = std::exchange(RHS.ଅଙ୍କ_Buckets, 0);
  ଅଙ୍କ_Nodes = std::exchange(RHS.ଅଙ୍କ_Nodes, 0);
  ଫେରନ୍ତୁ *ଏହି;
}

FoldingSetBase::~FoldingSetBase() { ମୁକ୍ତ(Buckets); }

ଶୂନ୍ୟ FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  ଯଦି (ଅଙ୍କ_Buckets)
    memset(Buckets, 0, ଅଙ୍କ_Buckets * ଆକାର(Folding_Set_ଗଣ୍ଠି *));
  ଅଙ୍କ_Nodes = 0;
}

ଶୂନ୍ୟ FoldingSetBase::place_ଗଣ୍ଠି(Folding_Set_ଗଣ୍ଠି *N, uint32_t Hash) {
  unsigned Mask = ଅଙ୍କ_Buckets - 1;
  unsigned I = Hash & Mask;
  ଯେତେବେଳେ (Buckets[I]) {
    ପ୍ରତିଜ୍ଞା(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++ଅଙ୍କ_Nodes;
}

ଶୂନ୍ୟ FoldingSetBase::grow(unsigned Min_ଅଙ୍କ_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_ଗଣନା = std::max(64u, llvm::bit_ceil(Min_ଅଙ୍କ_Buckets));
  ପ୍ରତିଜ୍ଞା(New_Bucket_ଗଣନା > ଅଙ୍କ_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_ଗଣନା));
  ପାଇଁ (unsigned I = 0; I != ଅଙ୍କ_Buckets; ++I)
    ଯଦି (Folding_Set_ଗଣ୍ଠି *N = Buckets[I])
      Tmp.place_ଗଣ୍ଠି(N, N->getFoldingSetHash());

  *ଏହି = std::move(Tmp);
}

ଶୂନ୍ୟ FoldingSetBase::reserve(unsigned N) {
  ଯଦି (N * 4 <= ଅଙ୍କ_Buckets * 3)
    ଫେରନ୍ତୁ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

ଶୂନ୍ୟ FoldingSetBase::ଭର୍ତ୍ତି(Folding_Set_ଗଣ୍ଠି *N, Folding_Set_ଭର୍ତ୍ତି_Token Token) {
  ପ୍ରତିଜ୍ଞା(N && "Cannot insert a null node");
  ପ୍ରତିଜ୍ଞା(Token && "Invalid token!");
  incrementEpoch();
  ଯଦି (LLVM_UNLIKELY((ଅଙ୍କ_Nodes + 1) * 4 > ଅଙ୍କ_Buckets * 3))
    grow(ଅଙ୍କ_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_ଗଣ୍ଠି(N, Hash);
  N->setFoldingSetHash(Hash);
}

ତାର୍କିକ FoldingSetBase::erase(Folding_Set_ଗଣ୍ଠି *N) {
  uint32_t Hash = N->getFoldingSetHash();
  ଯଦି (Hash == Folding_Set_ଗଣ୍ଠି_IDRef::NotAHash)
    ଫେରନ୍ତୁ ମିଥ୍ୟା; // Never inserted.

  unsigned Mask = ଅଙ୍କ_Buckets - 1;
  unsigned I = Hash & Mask;
  ଯେତେବେଳେ (Buckets[I] != N) {
    ଯଦି (LLVM_UNLIKELY(!Buckets[I]))
      ଫେରନ୍ତୁ ମିଥ୍ୟା; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ପାଇଁ (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    ଯଦି (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_ଗଣ୍ଠି_IDRef::NotAHash);
  --ଅଙ୍କ_Nodes;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}
