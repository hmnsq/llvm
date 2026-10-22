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

#અન્તર્ભાવ "llvm/ADT/FoldingSet.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/Allocator.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/SwapByteOrder.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstring>
using નામાકાશ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

તાર્કિક llvm::operator<(Folding_Set_ગાંઠ_IDRef LHS, Folding_Set_ગાંઠ_IDRef RHS) {
  જો (LHS.size() != RHS.size())
    પાછા LHS.size() < RHS.size();
  પાછા memcmp(LHS.data(), RHS.data(), LHS.size() * કદ(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

શૂન્ય Folding_Set_ગાંઠ_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned અંક_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + અંક_Inserts);

  Bits.ધકેલો_back(Size);
  જો (!Size)
    પાછા;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  અચલ unsigned *Base = (અચલ unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  જો (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } નહીંતર {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    જો (sys::IsBigEndianHost) {
      માટે (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned અક્ષર)String[Pos - 4] << 24) |
                     ((unsigned અક્ષર)String[Pos - 3] << 16) |
                     ((unsigned અક્ષર)String[Pos - 2] << 8) |
                     (unsigned અક્ષર)String[Pos - 1];
        Bits.ધકેલો_back(V);
      }
    } નહીંતર { // Little-endian host
      માટે (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned અક્ષર)String[Pos - 1] << 24) |
                     ((unsigned અક્ષર)String[Pos - 2] << 16) |
                     ((unsigned અક્ષર)String[Pos - 3] << 8) |
                     (unsigned અક્ષર)String[Pos - 4];
        Bits.ધકેલો_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  પસંદગી (Pos - Size) {
  સ્થિતિ 1:
    V = (V << 8) | (unsigned અક્ષર)String[Size - 3];
    [[fallthrough]];
  સ્થિતિ 2:
    V = (V << 8) | (unsigned અક્ષર)String[Size - 2];
    [[fallthrough]];
  સ્થિતિ 3:
    V = (V << 8) | (unsigned અક્ષર)String[Size - 1];
    તોડો;
  મૂળભૂત:
    પાછા; // Nothing left.
  }

  Bits.ધકેલો_back(V);
}

શૂન્ય Folding_Set_ગાંઠ_ID::Add_ગાંઠ_ID(અચલ Folding_Set_ગાંઠ_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_ગાંઠ_IDRef
Folding_Set_ગાંઠ_ID::Intern(BumpPtrAllocator &Allocator) અચલ {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  પાછા Folding_Set_ગાંઠ_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  પ્રતિજ્ઞા(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  અંક_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_ગાંઠ **>(
      safe_calloc(અંક_Buckets, કદ(Folding_Set_ગાંઠ *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      અંક_Buckets(std::exchange(Arg.અંક_Buckets, 0)),
      અંક_Nodes(std::exchange(Arg.અંક_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  જો (આ == &RHS)
    પાછા *આ;

  incrementEpoch();
  RHS.incrementEpoch();
  મુક્ત(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  અંક_Buckets = std::exchange(RHS.અંક_Buckets, 0);
  અંક_Nodes = std::exchange(RHS.અંક_Nodes, 0);
  પાછા *આ;
}

FoldingSetBase::~FoldingSetBase() { મુક્ત(Buckets); }

શૂન્ય FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  જો (અંક_Buckets)
    memset(Buckets, 0, અંક_Buckets * કદ(Folding_Set_ગાંઠ *));
  અંક_Nodes = 0;
}

શૂન્ય FoldingSetBase::place_ગાંઠ(Folding_Set_ગાંઠ *N, uint32_t Hash) {
  unsigned Mask = અંક_Buckets - 1;
  unsigned I = Hash & Mask;
  જ્યારે (Buckets[I]) {
    પ્રતિજ્ઞા(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++અંક_Nodes;
}

શૂન્ય FoldingSetBase::grow(unsigned Min_અંક_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_ગણતરી = std::max(64u, llvm::bit_ceil(Min_અંક_Buckets));
  પ્રતિજ્ઞા(New_Bucket_ગણતરી > અંક_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_ગણતરી));
  માટે (unsigned I = 0; I != અંક_Buckets; ++I)
    જો (Folding_Set_ગાંઠ *N = Buckets[I])
      Tmp.place_ગાંઠ(N, N->getFoldingSetHash());

  *આ = std::move(Tmp);
}

શૂન્ય FoldingSetBase::reserve(unsigned N) {
  જો (N * 4 <= અંક_Buckets * 3)
    પાછા;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

શૂન્ય FoldingSetBase::ઉમેરો(Folding_Set_ગાંઠ *N, Folding_Set_ઉમેરો_Token Token) {
  પ્રતિજ્ઞા(N && "Cannot insert a null node");
  પ્રતિજ્ઞા(Token && "Invalid token!");
  incrementEpoch();
  જો (LLVM_UNLIKELY((અંક_Nodes + 1) * 4 > અંક_Buckets * 3))
    grow(અંક_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_ગાંઠ(N, Hash);
  N->setFoldingSetHash(Hash);
}

તાર્કિક FoldingSetBase::erase(Folding_Set_ગાંઠ *N) {
  uint32_t Hash = N->getFoldingSetHash();
  જો (Hash == Folding_Set_ગાંઠ_IDRef::NotAHash)
    પાછા ખોટું; // Never inserted.

  unsigned Mask = અંક_Buckets - 1;
  unsigned I = Hash & Mask;
  જ્યારે (Buckets[I] != N) {
    જો (LLVM_UNLIKELY(!Buckets[I]))
      પાછા ખોટું; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  માટે (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    જો (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_ગાંઠ_IDRef::NotAHash);
  --અંક_Nodes;
  પાછા સાચું;
}
