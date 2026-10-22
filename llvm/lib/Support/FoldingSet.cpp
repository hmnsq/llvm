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

#अन्तर्भाव "llvm/ADT/FoldingSet.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstring>
using नामाकाश llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

तार्किक llvm::चालक<(Folding_Set_गाठ_IDRef LHS, Folding_Set_गाठ_IDRef RHS) {
  जर (LHS.size() != RHS.size())
    परत LHS.size() < RHS.size();
  परत स_भेद(LHS.data(), RHS.data(), LHS.size() * आकार(अचिन्हित)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

शून्य Folding_Set_गाठ_ID::AddString(StringRef String) {
  अचिन्हित Size = String.size();

  अचिन्हित अंक_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + अंक_Inserts);

  Bits.ढकला_back(Size);
  जर (!Size)
    परत;

  अचिन्हित Units = Size / 4;
  अचिन्हित Pos = 0;
  स्थिरांक अचिन्हित *Base = (स्थिरांक अचिन्हित *)String.data();

  // If the string is aligned do a bulk transfer.
  जर (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } अन्यथा {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    जर (sys::IsBigEndianHost) {
      साठी (Pos += 4; Pos <= Size; Pos += 4) {
        अचिन्हित V = ((अचिन्हित अक्षर)String[Pos - 4] << 24) |
                     ((अचिन्हित अक्षर)String[Pos - 3] << 16) |
                     ((अचिन्हित अक्षर)String[Pos - 2] << 8) |
                     (अचिन्हित अक्षर)String[Pos - 1];
        Bits.ढकला_back(V);
      }
    } अन्यथा { // Little-endian host
      साठी (Pos += 4; Pos <= Size; Pos += 4) {
        अचिन्हित V = ((अचिन्हित अक्षर)String[Pos - 1] << 24) |
                     ((अचिन्हित अक्षर)String[Pos - 2] << 16) |
                     ((अचिन्हित अक्षर)String[Pos - 3] << 8) |
                     (अचिन्हित अक्षर)String[Pos - 4];
        Bits.ढकला_back(V);
      }
    }
  }

  // With the leftover bits.
  अचिन्हित V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  निवड (Pos - Size) {
  स्थिती 1:
    V = (V << 8) | (अचिन्हित अक्षर)String[Size - 3];
    [[fallthrough]];
  स्थिती 2:
    V = (V << 8) | (अचिन्हित अक्षर)String[Size - 2];
    [[fallthrough]];
  स्थिती 3:
    V = (V << 8) | (अचिन्हित अक्षर)String[Size - 1];
    थांबा;
  पूर्वनिर्धारित:
    परत; // Nothing left.
  }

  Bits.ढकला_back(V);
}

शून्य Folding_Set_गाठ_ID::Add_गाठ_ID(स्थिरांक Folding_Set_गाठ_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_गाठ_IDRef
Folding_Set_गाठ_ID::Intern(BumpPtrAllocator &Allocator) स्थिरांक {
  अचिन्हित *New = Allocator.Allocate<अचिन्हित>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  परत Folding_Set_गाठ_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(अचिन्हित Log2InitSize) {
  प्रतिज्ञा(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  अंक_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_गाठ **>(
      safe_calloc(अंक_Buckets, आकार(Folding_Set_गाठ *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      अंक_Buckets(std::exchange(Arg.अंक_Buckets, 0)),
      अंक_Nodes(std::exchange(Arg.अंक_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::चालक=(FoldingSetBase &&RHS) {
  जर (हे == &RHS)
    परत *हे;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  अंक_Buckets = std::exchange(RHS.अंक_Buckets, 0);
  अंक_Nodes = std::exchange(RHS.अंक_Nodes, 0);
  परत *हे;
}

FoldingSetBase::~FoldingSetBase() { मुक्त(Buckets); }

शून्य FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  जर (अंक_Buckets)
    स_रखो(Buckets, 0, अंक_Buckets * आकार(Folding_Set_गाठ *));
  अंक_Nodes = 0;
}

शून्य FoldingSetBase::place_गाठ(Folding_Set_गाठ *N, uint32_t Hash) {
  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  पर्यंत (Buckets[I]) {
    प्रतिज्ञा(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++अंक_Nodes;
}

शून्य FoldingSetBase::grow(अचिन्हित Min_अंक_Buckets) {
  // The floor is the smallest size the constructor accepts.
  अचिन्हित New_Bucket_मोजणी = std::max(64u, llvm::bit_ceil(Min_अंक_Buckets));
  प्रतिज्ञा(New_Bucket_मोजणी > अंक_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_मोजणी));
  साठी (अचिन्हित I = 0; I != अंक_Buckets; ++I)
    जर (Folding_Set_गाठ *N = Buckets[I])
      Tmp.place_गाठ(N, N->getFoldingSetHash());

  *हे = std::move(Tmp);
}

शून्य FoldingSetBase::reserve(अचिन्हित N) {
  जर (N * 4 <= अंक_Buckets * 3)
    परत;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

शून्य FoldingSetBase::घाला(Folding_Set_गाठ *N, Folding_Set_घाला_Token Token) {
  प्रतिज्ञा(N && "Cannot insert a null node");
  प्रतिज्ञा(Token && "Invalid token!");
  incrementEpoch();
  जर (LLVM_UNLIKELY((अंक_Nodes + 1) * 4 > अंक_Buckets * 3))
    grow(अंक_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_गाठ(N, Hash);
  N->setFoldingSetHash(Hash);
}

तार्किक FoldingSetBase::erase(Folding_Set_गाठ *N) {
  uint32_t Hash = N->getFoldingSetHash();
  जर (Hash == Folding_Set_गाठ_IDRef::NotAHash)
    परत असत्य; // Never inserted.

  अचिन्हित Mask = अंक_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  पर्यंत (Buckets[I] != N) {
    जर (LLVM_UNLIKELY(!Buckets[I]))
      परत असत्य; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  साठी (अचिन्हित J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    अचिन्हित Ideal = Buckets[J]->getFoldingSetHash();
    जर (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_गाठ_IDRef::NotAHash);
  --अंक_Nodes;
  परत सत्य;
}
