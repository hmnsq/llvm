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

#അന്തര്ഭാവ "llvm/ADT/FoldingSet.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Support/Allocator.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/SwapByteOrder.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstring>
using നാമാകാശ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

തർക്കം llvm::operator<(Folding_Set_കണ്ണി_IDRef LHS, Folding_Set_കണ്ണി_IDRef RHS) {
  എങ്കിൽ (LHS.size() != RHS.size())
    തിരികെ LHS.size() < RHS.size();
  തിരികെ memcmp(LHS.data(), RHS.data(), LHS.size() * വലുപ്പം(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

ശൂന്യം Folding_Set_കണ്ണി_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned അക്കം_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + അക്കം_Inserts);

  Bits.തള്ളുക_back(Size);
  എങ്കിൽ (!Size)
    തിരികെ;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  സ്ഥിരം unsigned *Base = (സ്ഥിരം unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  എങ്കിൽ (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } അല്ലെങ്കിൽ {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    എങ്കിൽ (sys::IsBigEndianHost) {
      വേണ്ടി (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned അക്ഷരം)String[Pos - 4] << 24) |
                     ((unsigned അക്ഷരം)String[Pos - 3] << 16) |
                     ((unsigned അക്ഷരം)String[Pos - 2] << 8) |
                     (unsigned അക്ഷരം)String[Pos - 1];
        Bits.തള്ളുക_back(V);
      }
    } അല്ലെങ്കിൽ { // Little-endian host
      വേണ്ടി (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned അക്ഷരം)String[Pos - 1] << 24) |
                     ((unsigned അക്ഷരം)String[Pos - 2] << 16) |
                     ((unsigned അക്ഷരം)String[Pos - 3] << 8) |
                     (unsigned അക്ഷരം)String[Pos - 4];
        Bits.തള്ളുക_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  തിരഞ്ഞെടുപ്പ് (Pos - Size) {
  അവസ്ഥ 1:
    V = (V << 8) | (unsigned അക്ഷരം)String[Size - 3];
    [[fallthrough]];
  അവസ്ഥ 2:
    V = (V << 8) | (unsigned അക്ഷരം)String[Size - 2];
    [[fallthrough]];
  അവസ്ഥ 3:
    V = (V << 8) | (unsigned അക്ഷരം)String[Size - 1];
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    തിരികെ; // Nothing left.
  }

  Bits.തള്ളുക_back(V);
}

ശൂന്യം Folding_Set_കണ്ണി_ID::Add_കണ്ണി_ID(സ്ഥിരം Folding_Set_കണ്ണി_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_കണ്ണി_IDRef
Folding_Set_കണ്ണി_ID::Intern(BumpPtrAllocator &Allocator) സ്ഥിരം {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  തിരികെ Folding_Set_കണ്ണി_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  പ്രതിജ്ഞാ(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  അക്കം_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_കണ്ണി **>(
      safe_calloc(അക്കം_Buckets, വലുപ്പം(Folding_Set_കണ്ണി *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      അക്കം_Buckets(std::exchange(Arg.അക്കം_Buckets, 0)),
      അക്കം_Nodes(std::exchange(Arg.അക്കം_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  എങ്കിൽ (ഇത് == &RHS)
    തിരികെ *ഇത്;

  incrementEpoch();
  RHS.incrementEpoch();
  സ്വതന്ത്രം(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  അക്കം_Buckets = std::exchange(RHS.അക്കം_Buckets, 0);
  അക്കം_Nodes = std::exchange(RHS.അക്കം_Nodes, 0);
  തിരികെ *ഇത്;
}

FoldingSetBase::~FoldingSetBase() { സ്വതന്ത്രം(Buckets); }

ശൂന്യം FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  എങ്കിൽ (അക്കം_Buckets)
    memset(Buckets, 0, അക്കം_Buckets * വലുപ്പം(Folding_Set_കണ്ണി *));
  അക്കം_Nodes = 0;
}

ശൂന്യം FoldingSetBase::place_കണ്ണി(Folding_Set_കണ്ണി *N, uint32_t Hash) {
  unsigned Mask = അക്കം_Buckets - 1;
  unsigned I = Hash & Mask;
  ആകുമ്പോൾ (Buckets[I]) {
    പ്രതിജ്ഞാ(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++അക്കം_Nodes;
}

ശൂന്യം FoldingSetBase::grow(unsigned Min_അക്കം_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_എണ്ണം = std::max(64u, llvm::bit_ceil(Min_അക്കം_Buckets));
  പ്രതിജ്ഞാ(New_Bucket_എണ്ണം > അക്കം_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_എണ്ണം));
  വേണ്ടി (unsigned I = 0; I != അക്കം_Buckets; ++I)
    എങ്കിൽ (Folding_Set_കണ്ണി *N = Buckets[I])
      Tmp.place_കണ്ണി(N, N->getFoldingSetHash());

  *ഇത് = std::move(Tmp);
}

ശൂന്യം FoldingSetBase::reserve(unsigned N) {
  എങ്കിൽ (N * 4 <= അക്കം_Buckets * 3)
    തിരികെ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

ശൂന്യം FoldingSetBase::തിരുകുക(Folding_Set_കണ്ണി *N, Folding_Set_തിരുകുക_Token Token) {
  പ്രതിജ്ഞാ(N && "Cannot insert a null node");
  പ്രതിജ്ഞാ(Token && "Invalid token!");
  incrementEpoch();
  എങ്കിൽ (LLVM_UNLIKELY((അക്കം_Nodes + 1) * 4 > അക്കം_Buckets * 3))
    grow(അക്കം_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_കണ്ണി(N, Hash);
  N->setFoldingSetHash(Hash);
}

തർക്കം FoldingSetBase::erase(Folding_Set_കണ്ണി *N) {
  uint32_t Hash = N->getFoldingSetHash();
  എങ്കിൽ (Hash == Folding_Set_കണ്ണി_IDRef::NotAHash)
    തിരികെ അസത്യം; // Never inserted.

  unsigned Mask = അക്കം_Buckets - 1;
  unsigned I = Hash & Mask;
  ആകുമ്പോൾ (Buckets[I] != N) {
    എങ്കിൽ (LLVM_UNLIKELY(!Buckets[I]))
      തിരികെ അസത്യം; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  വേണ്ടി (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    എങ്കിൽ (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_കണ്ണി_IDRef::NotAHash);
  --അക്കം_Nodes;
  തിരികെ സത്യം;
}
