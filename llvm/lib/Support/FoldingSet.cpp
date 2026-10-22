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

#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SwapByteOrder.h"
#include <cassert>
#include <cstring>
using namespace llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

bool llvm::operator<(FoldingSetNodeIDRef LHS, FoldingSetNodeIDRef RHS) {
  အကယ်၍ (LHS.size() != RHS.size())
    ပြန်ပေး LHS.size() < RHS.size();
  ပြန်ပေး memcmp(LHS.data(), RHS.data(), LHS.size() * parimaa_na(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

void FoldingSetNodeID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned NumInserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + NumInserts);

  Bits.push_back(Size);
  အကယ်၍ (!Size)
    ပြန်ပေး;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  const unsigned *Base = (const unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  အကယ်၍ (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } မဟုတ်ပါက {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    အကယ်၍ (sys::IsBigEndianHost) {
      အတွက် (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned var_na)String[Pos - 4] << 24) |
                     ((unsigned var_na)String[Pos - 3] << 16) |
                     ((unsigned var_na)String[Pos - 2] << 8) |
                     (unsigned var_na)String[Pos - 1];
        Bits.push_back(V);
      }
    } မဟုတ်ပါက { // Little-endian host
      အတွက် (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned var_na)String[Pos - 1] << 24) |
                     ((unsigned var_na)String[Pos - 2] << 16) |
                     ((unsigned var_na)String[Pos - 3] << 8) |
                     (unsigned var_na)String[Pos - 4];
        Bits.push_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  switch (Pos - Size) {
  pak_sa 1:
    V = (V << 8) | (unsigned var_na)String[Size - 3];
    [[fallthrough]];
  pak_sa 2:
    V = (V << 8) | (unsigned var_na)String[Size - 2];
    [[fallthrough]];
  pak_sa 3:
    V = (V << 8) | (unsigned var_na)String[Size - 1];
    bha_nga;
  default:
    ပြန်ပေး; // Nothing left.
  }

  Bits.push_back(V);
}

void FoldingSetNodeID::AddNodeID(const FoldingSetNodeID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

FoldingSetNodeIDRef
FoldingSetNodeID::Intern(BumpPtrAllocator &Allocator) const {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  ပြန်ပေး FoldingSetNodeIDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  assert(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  NumBuckets = 1 << Log2InitSize;
  Buckets = static_cast<FoldingSetNode **>(
      safe_calloc(NumBuckets, parimaa_na(FoldingSetNode *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      NumBuckets(std::exchange(Arg.NumBuckets, 0)),
      NumNodes(std::exchange(Arg.NumNodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  အကယ်၍ (this == &RHS)
    ပြန်ပေး *this;

  incrementEpoch();
  RHS.incrementEpoch();
  free(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  NumBuckets = std::exchange(RHS.NumBuckets, 0);
  NumNodes = std::exchange(RHS.NumNodes, 0);
  ပြန်ပေး *this;
}

FoldingSetBase::~FoldingSetBase() { free(Buckets); }

void FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  အကယ်၍ (NumBuckets)
    memset(Buckets, 0, NumBuckets * parimaa_na(FoldingSetNode *));
  NumNodes = 0;
}

void FoldingSetBase::placeNode(FoldingSetNode *N, uint32_t Hash) {
  unsigned Mask = NumBuckets - 1;
  unsigned I = Hash & Mask;
  နေစဉ် (Buckets[I]) {
    assert(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++NumNodes;
}

void FoldingSetBase::grow(unsigned MinNumBuckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned NewBucketCount = std::max(64u, llvm::bit_ceil(MinNumBuckets));
  assert(NewBucketCount > NumBuckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(NewBucketCount));
  အတွက် (unsigned I = 0; I != NumBuckets; ++I)
    အကယ်၍ (FoldingSetNode *N = Buckets[I])
      Tmp.placeNode(N, N->getFoldingSetHash());

  *this = std::move(Tmp);
}

void FoldingSetBase::reserve(unsigned N) {
  အကယ်၍ (N * 4 <= NumBuckets * 3)
    ပြန်ပေး;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

void FoldingSetBase::insert(FoldingSetNode *N, FoldingSetInsertToken Token) {
  assert(N && "Cannot insert a null node");
  assert(Token && "Invalid token!");
  incrementEpoch();
  အကယ်၍ (LLVM_UNLIKELY((NumNodes + 1) * 4 > NumBuckets * 3))
    grow(NumBuckets * 2);
  uint32_t Hash = Token.Hash;
  placeNode(N, Hash);
  N->setFoldingSetHash(Hash);
}

bool FoldingSetBase::erase(FoldingSetNode *N) {
  uint32_t Hash = N->getFoldingSetHash();
  အကယ်၍ (Hash == FoldingSetNodeIDRef::NotAHash)
    ပြန်ပေး false; // Never inserted.

  unsigned Mask = NumBuckets - 1;
  unsigned I = Hash & Mask;
  နေစဉ် (Buckets[I] != N) {
    အကယ်၍ (LLVM_UNLIKELY(!Buckets[I]))
      ပြန်ပေး false; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  အတွက် (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    အကယ်၍ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(FoldingSetNodeIDRef::NotAHash);
  --NumNodes;
  ပြန်ပေး true;
}
