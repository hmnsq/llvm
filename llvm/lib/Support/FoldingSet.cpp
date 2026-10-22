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

#समावेश "llvm/ADT/FoldingSet.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Allocator.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/SwapByteOrder.h"
#समावेश <cassert>
#समावेश <cstring>
using namespace llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

bool llvm::चालक<(Folding_Set_गाँठ_IDRef LHS, Folding_Set_गाँठ_IDRef RHS) {
  अगर (LHS.आकार_2() != RHS.आकार_2())
    वापस LHS.आकार_2() < RHS.आकार_2();
  वापस स_भेद(LHS.तथ्य(), RHS.तथ्य(), LHS.आकार_2() * माप(अचिन्हित)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

व्योम Folding_Set_गाँठ_ID::AddString(StringRef String) {
  अचिन्हित आकार = String.आकार_2();

  अचिन्हित अंक_Inserts = 1 + divideCeil(आकार, 4);
  Bits.reserve(Bits.आकार_2() + अंक_Inserts);

  Bits.धकेलो_back(आकार);
  अगर (!आकार)
    वापस;

  अचिन्हित Units = आकार / 4;
  अचिन्हित Pos = 0;
  स्थिर अचिन्हित *Base = (स्थिर अचिन्हित *)String.तथ्य();

  // If the string is aligned do a bulk transfer.
  अगर (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } अन्यथा {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    अगर (sys::IsBigEndianHost) {
      क्रम (Pos += 4; Pos <= आकार; Pos += 4) {
        अचिन्हित V = ((अचिन्हित अक्षर)String[Pos - 4] << 24) |
                     ((अचिन्हित अक्षर)String[Pos - 3] << 16) |
                     ((अचिन्हित अक्षर)String[Pos - 2] << 8) |
                     (अचिन्हित अक्षर)String[Pos - 1];
        Bits.धकेलो_back(V);
      }
    } अन्यथा { // Little-endian host
      क्रम (Pos += 4; Pos <= आकार; Pos += 4) {
        अचिन्हित V = ((अचिन्हित अक्षर)String[Pos - 1] << 24) |
                     ((अचिन्हित अक्षर)String[Pos - 2] << 16) |
                     ((अचिन्हित अक्षर)String[Pos - 3] << 8) |
                     (अचिन्हित अक्षर)String[Pos - 4];
        Bits.धकेलो_back(V);
      }
    }
  }

  // With the leftover bits.
  अचिन्हित V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  चयन (Pos - आकार) {
  हाल 1:
    V = (V << 8) | (अचिन्हित अक्षर)String[आकार - 3];
    [[fallthrough]];
  हाल 2:
    V = (V << 8) | (अचिन्हित अक्षर)String[आकार - 2];
    [[fallthrough]];
  हाल 3:
    V = (V << 8) | (अचिन्हित अक्षर)String[आकार - 1];
    अवरोध;
  शेष:
    वापस; // Nothing left.
  }

  Bits.धकेलो_back(V);
}

व्योम Folding_Set_गाँठ_ID::Add_गाँठ_ID(स्थिर Folding_Set_गाँठ_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_गाँठ_IDRef
Folding_Set_गाँठ_ID::Intern(BumpPtrAllocator &Allocator) स्थिर {
  अचिन्हित *New = Allocator.Allocate<अचिन्हित>(Bits.आकार_2());
  llvm::uninitialized_copy(Bits, New);
  वापस Folding_Set_गाँठ_IDRef(New, Bits.आकार_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(अचिन्हित Log2_Init_आकार) {
  निश्चित(5 < Log2_Init_आकार && Log2_Init_आकार < 32 &&
         "Initial hash table size out of range");
  अंक_Buckets = 1 << Log2_Init_आकार;
  Buckets = static_cast<Folding_Set_गाँठ **>(
      safe_calloc(अंक_Buckets, माप(Folding_Set_गाँठ *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      अंक_Buckets(std::exchange(Arg.अंक_Buckets, 0)),
      अंक_Nodes(std::exchange(Arg.अंक_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::चालक=(FoldingSetBase &&RHS) {
  अगर (यह == &RHS)
    वापस *यह;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  अंक_Buckets = std::exchange(RHS.अंक_Buckets, 0);
  अंक_Nodes = std::exchange(RHS.अंक_Nodes, 0);
  वापस *यह;
}

FoldingSetBase::~FoldingSetBase() { मुक्त(Buckets); }

व्योम FoldingSetBase::सफा() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  अगर (अंक_Buckets)
    स_रखो(Buckets, 0, अंक_Buckets * माप(Folding_Set_गाँठ *));
  अंक_Nodes = 0;
}

व्योम FoldingSetBase::place_गाँठ(Folding_Set_गाँठ *N, uint32_t Hash) {
  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  जबतक (Buckets[I]) {
    निश्चित(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++अंक_Nodes;
}

व्योम FoldingSetBase::grow(अचिन्हित Min_अंक_Buckets) {
  // The floor is the smallest size the constructor accepts.
  अचिन्हित New_Bucket_गिनती = std::max(64u, llvm::bit_ceil(Min_अंक_Buckets));
  निश्चित(New_Bucket_गिनती > अंक_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_गिनती));
  क्रम (अचिन्हित I = 0; I != अंक_Buckets; ++I)
    अगर (Folding_Set_गाँठ *N = Buckets[I])
      Tmp.place_गाँठ(N, N->getFoldingSetHash());

  *यह = std::move(Tmp);
}

व्योम FoldingSetBase::reserve(अचिन्हित N) {
  अगर (N * 4 <= अंक_Buckets * 3)
    वापस;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

व्योम FoldingSetBase::जोड़ो(Folding_Set_गाँठ *N, Folding_Set_जोड़ो_Token Token) {
  निश्चित(N && "Cannot insert a null node");
  निश्चित(Token && "Invalid token!");
  incrementEpoch();
  अगर (LLVM_UNLIKELY((अंक_Nodes + 1) * 4 > अंक_Buckets * 3))
    grow(अंक_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_गाँठ(N, Hash);
  N->setFoldingSetHash(Hash);
}

bool FoldingSetBase::erase(Folding_Set_गाँठ *N) {
  uint32_t Hash = N->getFoldingSetHash();
  अगर (Hash == Folding_Set_गाँठ_IDRef::NotAHash)
    वापस false; // Never inserted.

  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  जबतक (Buckets[I] != N) {
    अगर (LLVM_UNLIKELY(!Buckets[I]))
      वापस false; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  क्रम (अचिन्हित J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    अचिन्हित Ideal = Buckets[J]->getFoldingSetHash();
    अगर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_गाँठ_IDRef::NotAHash);
  --अंक_Nodes;
  वापस true;
}
