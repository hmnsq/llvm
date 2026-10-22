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

bool llvm::operator<(Folding_Set_बुहोल_IDRef LHS, Folding_Set_बुहोल_IDRef RHS) {
  if (LHS.size() != RHS.size())
    return LHS.size() < RHS.size();
  return memcmp(LHS.data(), RHS.data(), LHS.size() * sizeof(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

void Folding_Set_बुहोल_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned नुमेरो_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + नुमेरो_Inserts);

  Bits.इतुलाक_back(Size);
  if (!Size)
    return;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  const unsigned *Base = (const unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  if (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } else {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    if (sys::IsBigEndianHost) {
      for (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned char)String[Pos - 4] << 24) |
                     ((unsigned char)String[Pos - 3] << 16) |
                     ((unsigned char)String[Pos - 2] << 8) |
                     (unsigned char)String[Pos - 1];
        Bits.इतुलाक_back(V);
      }
    } else { // Little-endian host
      for (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned char)String[Pos - 1] << 24) |
                     ((unsigned char)String[Pos - 2] << 16) |
                     ((unsigned char)String[Pos - 3] << 8) |
                     (unsigned char)String[Pos - 4];
        Bits.इतुलाक_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  switch (Pos - Size) {
  case 1:
    V = (V << 8) | (unsigned char)String[Size - 3];
    [[fallthrough]];
  case 2:
    V = (V << 8) | (unsigned char)String[Size - 2];
    [[fallthrough]];
  case 3:
    V = (V << 8) | (unsigned char)String[Size - 1];
    break;
  default:
    return; // Nothing left.
  }

  Bits.इतुलाक_back(V);
}

void Folding_Set_बुहोल_ID::Add_बुहोल_ID(const Folding_Set_बुहोल_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_बुहोल_IDRef
Folding_Set_बुहोल_ID::Intern(BumpPtrAllocator &Allocator) const {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  return Folding_Set_बुहोल_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  assert(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  नुमेरो_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_बुहोल **>(
      safe_calloc(नुमेरो_Buckets, sizeof(Folding_Set_बुहोल *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      नुमेरो_Buckets(std::exchange(Arg.नुमेरो_Buckets, 0)),
      नुमेरो_Nodes(std::exchange(Arg.नुमेरो_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  if (this == &RHS)
    return *this;

  incrementEpoch();
  RHS.incrementEpoch();
  free(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  नुमेरो_Buckets = std::exchange(RHS.नुमेरो_Buckets, 0);
  नुमेरो_Nodes = std::exchange(RHS.नुमेरो_Nodes, 0);
  return *this;
}

FoldingSetBase::~FoldingSetBase() { free(Buckets); }

void FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  if (नुमेरो_Buckets)
    memset(Buckets, 0, नुमेरो_Buckets * sizeof(Folding_Set_बुहोल *));
  नुमेरो_Nodes = 0;
}

void FoldingSetBase::place_बुहोल(Folding_Set_बुहोल *N, uint32_t Hash) {
  unsigned Mask = नुमेरो_Buckets - 1;
  unsigned I = Hash & Mask;
  while (Buckets[I]) {
    assert(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++नुमेरो_Nodes;
}

void FoldingSetBase::grow(unsigned Min_नुमेरो_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_बिलांग = std::max(64u, llvm::bit_ceil(Min_नुमेरो_Buckets));
  assert(New_Bucket_बिलांग > नुमेरो_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_बिलांग));
  for (unsigned I = 0; I != नुमेरो_Buckets; ++I)
    if (Folding_Set_बुहोल *N = Buckets[I])
      Tmp.place_बुहोल(N, N->getFoldingSetHash());

  *this = std::move(Tmp);
}

void FoldingSetBase::reserve(unsigned N) {
  if (N * 4 <= नुमेरो_Buckets * 3)
    return;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

void FoldingSetBase::इसिंगित(Folding_Set_बुहोल *N, Folding_Set_इसिंगित_Token Token) {
  assert(N && "Cannot insert a null node");
  assert(Token && "Invalid token!");
  incrementEpoch();
  if (LLVM_UNLIKELY((नुमेरो_Nodes + 1) * 4 > नुमेरो_Buckets * 3))
    grow(नुमेरो_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_बुहोल(N, Hash);
  N->setFoldingSetHash(Hash);
}

bool FoldingSetBase::erase(Folding_Set_बुहोल *N) {
  uint32_t Hash = N->getFoldingSetHash();
  if (Hash == Folding_Set_बुहोल_IDRef::NotAHash)
    return false; // Never inserted.

  unsigned Mask = नुमेरो_Buckets - 1;
  unsigned I = Hash & Mask;
  while (Buckets[I] != N) {
    if (LLVM_UNLIKELY(!Buckets[I]))
      return false; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  for (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    if (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_बुहोल_IDRef::NotAHash);
  --नुमेरो_Nodes;
  return true;
}
