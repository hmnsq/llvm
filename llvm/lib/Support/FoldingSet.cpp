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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/FoldingSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstring>
using ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

ತಾರ್ಕಿಕ llvm::operator<(Folding_Set_ಗಂಟು_IDRef LHS, Folding_Set_ಗಂಟು_IDRef RHS) {
  ಯದಿ (LHS.size() != RHS.size())
    ಪ್ರತ್ಯಾಗಮ LHS.size() < RHS.size();
  ಪ್ರತ್ಯಾಗಮ memcmp(LHS.data(), RHS.data(), LHS.size() * ಪರಿಮಾಣ(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

ಶೂನ್ಯ Folding_Set_ಗಂಟು_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned ಅಂಕೆ_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + ಅಂಕೆ_Inserts);

  Bits.ತಳ್ಳು_back(Size);
  ಯದಿ (!Size)
    ಪ್ರತ್ಯಾಗಮ;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ಧ್ರುವ unsigned *Base = (ಧ್ರುವ unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  ಯದಿ (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } ಅನ್ಯಥಾ {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    ಯದಿ (sys::IsBigEndianHost) {
      ಪ್ರತಿ (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ವರ್ಣ)String[Pos - 4] << 24) |
                     ((unsigned ವರ್ಣ)String[Pos - 3] << 16) |
                     ((unsigned ವರ್ಣ)String[Pos - 2] << 8) |
                     (unsigned ವರ್ಣ)String[Pos - 1];
        Bits.ತಳ್ಳು_back(V);
      }
    } ಅನ್ಯಥಾ { // Little-endian host
      ಪ್ರತಿ (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned ವರ್ಣ)String[Pos - 1] << 24) |
                     ((unsigned ವರ್ಣ)String[Pos - 2] << 16) |
                     ((unsigned ವರ್ಣ)String[Pos - 3] << 8) |
                     (unsigned ವರ್ಣ)String[Pos - 4];
        Bits.ತಳ್ಳು_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ವಿಕಲ್ಪ (Pos - Size) {
  ಪಕ್ಷ 1:
    V = (V << 8) | (unsigned ವರ್ಣ)String[Size - 3];
    [[fallthrough]];
  ಪಕ್ಷ 2:
    V = (V << 8) | (unsigned ವರ್ಣ)String[Size - 2];
    [[fallthrough]];
  ಪಕ್ಷ 3:
    V = (V << 8) | (unsigned ವರ್ಣ)String[Size - 1];
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ; // Nothing left.
  }

  Bits.ತಳ್ಳು_back(V);
}

ಶೂನ್ಯ Folding_Set_ಗಂಟು_ID::Add_ಗಂಟು_ID(ಧ್ರುವ Folding_Set_ಗಂಟು_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_ಗಂಟು_IDRef
Folding_Set_ಗಂಟು_ID::Intern(BumpPtrAllocator &Allocator) ಧ್ರುವ {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  ಪ್ರತ್ಯಾಗಮ Folding_Set_ಗಂಟು_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  ಪ್ರತಿಜ್ಞಾ(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  ಅಂಕೆ_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_ಗಂಟು **>(
      safe_calloc(ಅಂಕೆ_Buckets, ಪರಿಮಾಣ(Folding_Set_ಗಂಟು *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      ಅಂಕೆ_Buckets(std::exchange(Arg.ಅಂಕೆ_Buckets, 0)),
      ಅಂಕೆ_Nodes(std::exchange(Arg.ಅಂಕೆ_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  ಯದಿ (ಏತತ್ == &RHS)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  incrementEpoch();
  RHS.incrementEpoch();
  ಮುಕ್ತ(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  ಅಂಕೆ_Buckets = std::exchange(RHS.ಅಂಕೆ_Buckets, 0);
  ಅಂಕೆ_Nodes = std::exchange(RHS.ಅಂಕೆ_Nodes, 0);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

FoldingSetBase::~FoldingSetBase() { ಮುಕ್ತ(Buckets); }

ಶೂನ್ಯ FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  ಯದಿ (ಅಂಕೆ_Buckets)
    memset(Buckets, 0, ಅಂಕೆ_Buckets * ಪರಿಮಾಣ(Folding_Set_ಗಂಟು *));
  ಅಂಕೆ_Nodes = 0;
}

ಶೂನ್ಯ FoldingSetBase::place_ಗಂಟು(Folding_Set_ಗಂಟು *N, uint32_t Hash) {
  unsigned Mask = ಅಂಕೆ_Buckets - 1;
  unsigned I = Hash & Mask;
  ಯಾವತ್ (Buckets[I]) {
    ಪ್ರತಿಜ್ಞಾ(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++ಅಂಕೆ_Nodes;
}

ಶೂನ್ಯ FoldingSetBase::grow(unsigned Min_ಅಂಕೆ_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_ಎಣಿಕೆ = std::max(64u, llvm::bit_ceil(Min_ಅಂಕೆ_Buckets));
  ಪ್ರತಿಜ್ಞಾ(New_Bucket_ಎಣಿಕೆ > ಅಂಕೆ_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_ಎಣಿಕೆ));
  ಪ್ರತಿ (unsigned I = 0; I != ಅಂಕೆ_Buckets; ++I)
    ಯದಿ (Folding_Set_ಗಂಟು *N = Buckets[I])
      Tmp.place_ಗಂಟು(N, N->getFoldingSetHash());

  *ಏತತ್ = std::move(Tmp);
}

ಶೂನ್ಯ FoldingSetBase::reserve(unsigned N) {
  ಯದಿ (N * 4 <= ಅಂಕೆ_Buckets * 3)
    ಪ್ರತ್ಯಾಗಮ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

ಶೂನ್ಯ FoldingSetBase::ಸೇರಿಸು(Folding_Set_ಗಂಟು *N, Folding_Set_ಸೇರಿಸು_Token Token) {
  ಪ್ರತಿಜ್ಞಾ(N && "Cannot insert a null node");
  ಪ್ರತಿಜ್ಞಾ(Token && "Invalid token!");
  incrementEpoch();
  ಯದಿ (LLVM_UNLIKELY((ಅಂಕೆ_Nodes + 1) * 4 > ಅಂಕೆ_Buckets * 3))
    grow(ಅಂಕೆ_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_ಗಂಟು(N, Hash);
  N->setFoldingSetHash(Hash);
}

ತಾರ್ಕಿಕ FoldingSetBase::erase(Folding_Set_ಗಂಟು *N) {
  uint32_t Hash = N->getFoldingSetHash();
  ಯದಿ (Hash == Folding_Set_ಗಂಟು_IDRef::NotAHash)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; // Never inserted.

  unsigned Mask = ಅಂಕೆ_Buckets - 1;
  unsigned I = Hash & Mask;
  ಯಾವತ್ (Buckets[I] != N) {
    ಯದಿ (LLVM_UNLIKELY(!Buckets[I]))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ಪ್ರತಿ (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    ಯದಿ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_ಗಂಟು_IDRef::NotAHash);
  --ಅಂಕೆ_Nodes;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}
