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

तार्किक llvm::operator<(Folding_Set_ग्रन्थिः_IDRef LHS, Folding_Set_ग्रन्थिः_IDRef RHS) {
  सचे (LHS.size() != RHS.size())
    पटिनिवत्त LHS.size() < RHS.size();
  पटिनिवत्त memcmp(LHS.data(), RHS.data(), LHS.size() * परिमाण(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

शून्य Folding_Set_ग्रन्थिः_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned अङ्कः_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + अङ्कः_Inserts);

  Bits.प्रक्षिप_back(Size);
  सचे (!Size)
    पटिनिवत्त;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ध्रुव unsigned *Base = (ध्रुव unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  सचे (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } अञ्ञथा {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    सचे (sys::IsBigEndianHost) {
      अत्थाय (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned वर्ण)String[Pos - 4] << 24) |
                     ((unsigned वर्ण)String[Pos - 3] << 16) |
                     ((unsigned वर्ण)String[Pos - 2] << 8) |
                     (unsigned वर्ण)String[Pos - 1];
        Bits.प्रक्षिप_back(V);
      }
    } अञ्ञथा { // Little-endian host
      अत्थाय (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned वर्ण)String[Pos - 1] << 24) |
                     ((unsigned वर्ण)String[Pos - 2] << 16) |
                     ((unsigned वर्ण)String[Pos - 3] << 8) |
                     (unsigned वर्ण)String[Pos - 4];
        Bits.प्रक्षिप_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  विकल्प (Pos - Size) {
  पक्ष 1:
    V = (V << 8) | (unsigned वर्ण)String[Size - 3];
    [[fallthrough]];
  पक्ष 2:
    V = (V << 8) | (unsigned वर्ण)String[Size - 2];
    [[fallthrough]];
  पक्ष 3:
    V = (V << 8) | (unsigned वर्ण)String[Size - 1];
    भङ्ग;
  स्वभाव:
    पटिनिवत्त; // Nothing left.
  }

  Bits.प्रक्षिप_back(V);
}

शून्य Folding_Set_ग्रन्थिः_ID::Add_ग्रन्थिः_ID(ध्रुव Folding_Set_ग्रन्थिः_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_ग्रन्थिः_IDRef
Folding_Set_ग्रन्थिः_ID::Intern(BumpPtrAllocator &Allocator) ध्रुव {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  पटिनिवत्त Folding_Set_ग्रन्थिः_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  प्रतिज्ञा(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  अङ्कः_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_ग्रन्थिः **>(
      safe_calloc(अङ्कः_Buckets, परिमाण(Folding_Set_ग्रन्थिः *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      अङ्कः_Buckets(std::exchange(Arg.अङ्कः_Buckets, 0)),
      अङ्कः_Nodes(std::exchange(Arg.अङ्कः_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  सचे (एतत् == &RHS)
    पटिनिवत्त *एतत्;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  अङ्कः_Buckets = std::exchange(RHS.अङ्कः_Buckets, 0);
  अङ्कः_Nodes = std::exchange(RHS.अङ्कः_Nodes, 0);
  पटिनिवत्त *एतत्;
}

FoldingSetBase::~FoldingSetBase() { मुक्त(Buckets); }

शून्य FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  सचे (अङ्कः_Buckets)
    memset(Buckets, 0, अङ्कः_Buckets * परिमाण(Folding_Set_ग्रन्थिः *));
  अङ्कः_Nodes = 0;
}

शून्य FoldingSetBase::place_ग्रन्थिः(Folding_Set_ग्रन्थिः *N, uint32_t Hash) {
  unsigned Mask = अङ्कः_Buckets - 1;
  unsigned I = Hash & Mask;
  याव (Buckets[I]) {
    प्रतिज्ञा(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++अङ्कः_Nodes;
}

शून्य FoldingSetBase::grow(unsigned Min_अङ्कः_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_गणनम् = std::max(64u, llvm::bit_ceil(Min_अङ्कः_Buckets));
  प्रतिज्ञा(New_Bucket_गणनम् > अङ्कः_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_गणनम्));
  अत्थाय (unsigned I = 0; I != अङ्कः_Buckets; ++I)
    सचे (Folding_Set_ग्रन्थिः *N = Buckets[I])
      Tmp.place_ग्रन्थिः(N, N->getFoldingSetHash());

  *एतत् = std::move(Tmp);
}

शून्य FoldingSetBase::reserve(unsigned N) {
  सचे (N * 4 <= अङ्कः_Buckets * 3)
    पटिनिवत्त;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

शून्य FoldingSetBase::निवेशय(Folding_Set_ग्रन्थिः *N, Folding_Set_निवेशय_Token Token) {
  प्रतिज्ञा(N && "Cannot insert a null node");
  प्रतिज्ञा(Token && "Invalid token!");
  incrementEpoch();
  सचे (LLVM_UNLIKELY((अङ्कः_Nodes + 1) * 4 > अङ्कः_Buckets * 3))
    grow(अङ्कः_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_ग्रन्थिः(N, Hash);
  N->setFoldingSetHash(Hash);
}

तार्किक FoldingSetBase::erase(Folding_Set_ग्रन्थिः *N) {
  uint32_t Hash = N->getFoldingSetHash();
  सचे (Hash == Folding_Set_ग्रन्थिः_IDRef::NotAHash)
    पटिनिवत्त मिच्छा; // Never inserted.

  unsigned Mask = अङ्कः_Buckets - 1;
  unsigned I = Hash & Mask;
  याव (Buckets[I] != N) {
    सचे (LLVM_UNLIKELY(!Buckets[I]))
      पटिनिवत्त मिच्छा; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  अत्थाय (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    सचे (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_ग्रन्थिः_IDRef::NotAHash);
  --अङ्कः_Nodes;
  पटिनिवत्त सच्च;
}
