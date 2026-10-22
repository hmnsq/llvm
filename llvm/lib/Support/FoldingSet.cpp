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

#অন্তর্ভাৱ "llvm/ADT/FoldingSet.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Support/Allocator.h"
#অন্তর্ভাৱ "llvm/Support/MathExtras.h"
#অন্তর্ভাৱ "llvm/Support/SwapByteOrder.h"
#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <cstring>
using নামাকাশ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

তার্কিক llvm::operator<(Folding_Set_গ্ৰন্থি_IDRef LHS, Folding_Set_গ্ৰন্থি_IDRef RHS) {
  যদি (LHS.size() != RHS.size())
    ঘূৰাই LHS.size() < RHS.size();
  ঘূৰাই memcmp(LHS.data(), RHS.data(), LHS.size() * পরিমাণ(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

শূন্য Folding_Set_গ্ৰন্থি_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned অঙ্ক_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + অঙ্ক_Inserts);

  Bits.ঠেলো_back(Size);
  যদি (!Size)
    ঘূৰাই;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ধ্রুৱ unsigned *Base = (ধ্রুৱ unsigned *)String.data();

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
      বাবে (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ৱর্ণ)String[Pos - 4] << 24) |
                     ((unsigned ৱর্ণ)String[Pos - 3] << 16) |
                     ((unsigned ৱর্ণ)String[Pos - 2] << 8) |
                     (unsigned ৱর্ণ)String[Pos - 1];
        Bits.ঠেলো_back(V);
      }
    } অন্যথা { // Little-endian host
      বাবে (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ৱর্ণ)String[Pos - 1] << 24) |
                     ((unsigned ৱর্ণ)String[Pos - 2] << 16) |
                     ((unsigned ৱর্ণ)String[Pos - 3] << 8) |
                     (unsigned ৱর্ণ)String[Pos - 4];
        Bits.ঠেলো_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ৱিকল্প (Pos - Size) {
  পক্ষ 1:
    V = (V << 8) | (unsigned ৱর্ণ)String[Size - 3];
    [[fallthrough]];
  পক্ষ 2:
    V = (V << 8) | (unsigned ৱর্ণ)String[Size - 2];
    [[fallthrough]];
  পক্ষ 3:
    V = (V << 8) | (unsigned ৱর্ণ)String[Size - 1];
    ভঙ্গ;
  স্ৱভাৱ:
    ঘূৰাই; // Nothing left.
  }

  Bits.ঠেলো_back(V);
}

শূন্য Folding_Set_গ্ৰন্থি_ID::Add_গ্ৰন্থি_ID(ধ্রুৱ Folding_Set_গ্ৰন্থি_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_গ্ৰন্থি_IDRef
Folding_Set_গ্ৰন্থি_ID::Intern(BumpPtrAllocator &Allocator) ধ্রুৱ {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  ঘূৰাই Folding_Set_গ্ৰন্থি_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  প্রতিজ্ঞা(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  অঙ্ক_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_গ্ৰন্থি **>(
      safe_calloc(অঙ্ক_Buckets, পরিমাণ(Folding_Set_গ্ৰন্থি *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      অঙ্ক_Buckets(std::exchange(Arg.অঙ্ক_Buckets, 0)),
      অঙ্ক_Nodes(std::exchange(Arg.অঙ্ক_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  যদি (এতত্ == &RHS)
    ঘূৰাই *এতত্;

  incrementEpoch();
  RHS.incrementEpoch();
  মুক্ত(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  অঙ্ক_Buckets = std::exchange(RHS.অঙ্ক_Buckets, 0);
  অঙ্ক_Nodes = std::exchange(RHS.অঙ্ক_Nodes, 0);
  ঘূৰাই *এতত্;
}

FoldingSetBase::~FoldingSetBase() { মুক্ত(Buckets); }

শূন্য FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  যদি (অঙ্ক_Buckets)
    memset(Buckets, 0, অঙ্ক_Buckets * পরিমাণ(Folding_Set_গ্ৰন্থি *));
  অঙ্ক_Nodes = 0;
}

শূন্য FoldingSetBase::place_গ্ৰন্থি(Folding_Set_গ্ৰন্থি *N, uint32_t Hash) {
  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Hash & Mask;
  যেতিয়ালৈ (Buckets[I]) {
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
  বাবে (unsigned I = 0; I != অঙ্ক_Buckets; ++I)
    যদি (Folding_Set_গ্ৰন্থি *N = Buckets[I])
      Tmp.place_গ্ৰন্থি(N, N->getFoldingSetHash());

  *এতত্ = std::move(Tmp);
}

শূন্য FoldingSetBase::reserve(unsigned N) {
  যদি (N * 4 <= অঙ্ক_Buckets * 3)
    ঘূৰাই;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

শূন্য FoldingSetBase::ঢোকাও(Folding_Set_গ্ৰন্থি *N, Folding_Set_ঢোকাও_Token Token) {
  প্রতিজ্ঞা(N && "Cannot insert a null node");
  প্রতিজ্ঞা(Token && "Invalid token!");
  incrementEpoch();
  যদি (LLVM_UNLIKELY((অঙ্ক_Nodes + 1) * 4 > অঙ্ক_Buckets * 3))
    grow(অঙ্ক_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_গ্ৰন্থি(N, Hash);
  N->setFoldingSetHash(Hash);
}

তার্কিক FoldingSetBase::erase(Folding_Set_গ্ৰন্থি *N) {
  uint32_t Hash = N->getFoldingSetHash();
  যদি (Hash == Folding_Set_গ্ৰন্থি_IDRef::NotAHash)
    ঘূৰাই মিছা; // Never inserted.

  unsigned Mask = অঙ্ক_Buckets - 1;
  unsigned I = Hash & Mask;
  যেতিয়ালৈ (Buckets[I] != N) {
    যদি (LLVM_UNLIKELY(!Buckets[I]))
      ঘূৰাই মিছা; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  বাবে (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    যদি (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_গ্ৰন্থি_IDRef::NotAHash);
  --অঙ্ক_Nodes;
  ঘূৰাই সঁচা;
}
