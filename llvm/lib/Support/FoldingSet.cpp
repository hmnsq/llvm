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

#ضمن "llvm/ADT/FoldingSet.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/SwapByteOrder.h"
#ضمن <cassert>
#ضمن <cstring>
using نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::operator<(FoldingSetNodeIDRef LHS, FoldingSetNodeIDRef RHS) {
  אם (LHS.size() != RHS.size())
    החזר LHS.size() < RHS.size();
  החזר memcmp(LHS.data(), RHS.data(), LHS.size() * حجم(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ FoldingSetNodeID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned NumInserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + NumInserts);

  Bits.push_back(Size);
  אם (!Size)
    החזר;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ثابت unsigned *Base = (ثابت unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  אם (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } אחרת {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    אם (sys::IsBigEndianHost) {
      עבור (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned حرف)String[Pos - 4] << 24) |
                     ((unsigned حرف)String[Pos - 3] << 16) |
                     ((unsigned حرف)String[Pos - 2] << 8) |
                     (unsigned حرف)String[Pos - 1];
        Bits.push_back(V);
      }
    } אחרת { // Little-endian host
      עבור (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned حرف)String[Pos - 1] << 24) |
                     ((unsigned حرف)String[Pos - 2] << 16) |
                     ((unsigned حرف)String[Pos - 3] << 8) |
                     (unsigned حرف)String[Pos - 4];
        Bits.push_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - Size) {
  حالة 1:
    V = (V << 8) | (unsigned حرف)String[Size - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (unsigned حرف)String[Size - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (unsigned حرف)String[Size - 1];
    اكسر;
  افتراضي:
    החזר; // Nothing left.
  }

  Bits.push_back(V);
}

فراغ FoldingSetNodeID::AddNodeID(ثابت FoldingSetNodeID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

FoldingSetNodeIDRef
FoldingSetNodeID::Intern(BumpPtrAllocator &Allocator) ثابت {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  החזר FoldingSetNodeIDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  أكد(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  NumBuckets = 1 << Log2InitSize;
  Buckets = static_cast<FoldingSetNode **>(
      safe_calloc(NumBuckets, حجم(FoldingSetNode *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      NumBuckets(std::exchange(Arg.NumBuckets, 0)),
      NumNodes(std::exchange(Arg.NumNodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  אם (هذا == &RHS)
    החזר *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  NumBuckets = std::exchange(RHS.NumBuckets, 0);
  NumNodes = std::exchange(RHS.NumNodes, 0);
  החזר *هذا;
}

FoldingSetBase::~FoldingSetBase() { حرر(Buckets); }

فراغ FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  אם (NumBuckets)
    memset(Buckets, 0, NumBuckets * حجم(FoldingSetNode *));
  NumNodes = 0;
}

فراغ FoldingSetBase::placeNode(FoldingSetNode *N, uint32_t Hash) {
  unsigned Mask = NumBuckets - 1;
  unsigned I = Hash & Mask;
  כלעוד (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++NumNodes;
}

فراغ FoldingSetBase::grow(unsigned MinNumBuckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned NewBucketCount = std::max(64u, llvm::bit_ceil(MinNumBuckets));
  أكد(NewBucketCount > NumBuckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(NewBucketCount));
  עבור (unsigned I = 0; I != NumBuckets; ++I)
    אם (FoldingSetNode *N = Buckets[I])
      Tmp.placeNode(N, N->getFoldingSetHash());

  *هذا = std::move(Tmp);
}

فراغ FoldingSetBase::reserve(unsigned N) {
  אם (N * 4 <= NumBuckets * 3)
    החזר;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ FoldingSetBase::insert(FoldingSetNode *N, FoldingSetInsertToken Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  אם (LLVM_UNLIKELY((NumNodes + 1) * 4 > NumBuckets * 3))
    grow(NumBuckets * 2);
  uint32_t Hash = Token.Hash;
  placeNode(N, Hash);
  N->setFoldingSetHash(Hash);
}

منطقي FoldingSetBase::erase(FoldingSetNode *N) {
  uint32_t Hash = N->getFoldingSetHash();
  אם (Hash == FoldingSetNodeIDRef::NotAHash)
    החזר שקר; // Never inserted.

  unsigned Mask = NumBuckets - 1;
  unsigned I = Hash & Mask;
  כלעוד (Buckets[I] != N) {
    אם (LLVM_UNLIKELY(!Buckets[I]))
      החזר שקר; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  עבור (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    אם (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(FoldingSetNodeIDRef::NotAHash);
  --NumNodes;
  החזר אמת;
}
