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

तार्किक llvm::operator<(Folding_Set_गाठ_IDRef LHS, Folding_Set_गाठ_IDRef RHS) {
  यदि (LHS.size() != RHS.size())
    प्रत्यागम LHS.size() < RHS.size();
  प्रत्यागम memcmp(LHS.data(), RHS.data(), LHS.size() * परिमाण(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

शून्य Folding_Set_गाठ_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned अंक_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + अंक_Inserts);

  Bits.ढकला_back(Size);
  यदि (!Size)
    प्रत्यागम;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ध्रुव unsigned *Base = (ध्रुव unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  यदि (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } अन्यथा {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    यदि (sys::IsBigEndianHost) {
      प्रति (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned वर्ण)String[Pos - 4] << 24) |
                     ((unsigned वर्ण)String[Pos - 3] << 16) |
                     ((unsigned वर्ण)String[Pos - 2] << 8) |
                     (unsigned वर्ण)String[Pos - 1];
        Bits.ढकला_back(V);
      }
    } अन्यथा { // Little-endian host
      प्रति (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned वर्ण)String[Pos - 1] << 24) |
                     ((unsigned वर्ण)String[Pos - 2] << 16) |
                     ((unsigned वर्ण)String[Pos - 3] << 8) |
                     (unsigned वर्ण)String[Pos - 4];
        Bits.ढकला_back(V);
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
    प्रत्यागम; // Nothing left.
  }

  Bits.ढकला_back(V);
}

शून्य Folding_Set_गाठ_ID::Add_गाठ_ID(ध्रुव Folding_Set_गाठ_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_गाठ_IDRef
Folding_Set_गाठ_ID::Intern(BumpPtrAllocator &Allocator) ध्रुव {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  प्रत्यागम Folding_Set_गाठ_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  प्रतिज्ञा(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  अंक_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_गाठ **>(
      safe_calloc(अंक_Buckets, परिमाण(Folding_Set_गाठ *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      अंक_Buckets(std::exchange(Arg.अंक_Buckets, 0)),
      अंक_Nodes(std::exchange(Arg.अंक_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  यदि (एतत् == &RHS)
    प्रत्यागम *एतत्;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  अंक_Buckets = std::exchange(RHS.अंक_Buckets, 0);
  अंक_Nodes = std::exchange(RHS.अंक_Nodes, 0);
  प्रत्यागम *एतत्;
}

FoldingSetBase::~FoldingSetBase() { मुक्त(Buckets); }

शून्य FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  यदि (अंक_Buckets)
    memset(Buckets, 0, अंक_Buckets * परिमाण(Folding_Set_गाठ *));
  अंक_Nodes = 0;
}

शून्य FoldingSetBase::place_गाठ(Folding_Set_गाठ *N, uint32_t Hash) {
  unsigned Mask = अंक_Buckets - 1;
  unsigned I = Hash & Mask;
  यावत् (Buckets[I]) {
    प्रतिज्ञा(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++अंक_Nodes;
}

शून्य FoldingSetBase::grow(unsigned Min_अंक_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_मोजणी = std::max(64u, llvm::bit_ceil(Min_अंक_Buckets));
  प्रतिज्ञा(New_Bucket_मोजणी > अंक_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_मोजणी));
  प्रति (unsigned I = 0; I != अंक_Buckets; ++I)
    यदि (Folding_Set_गाठ *N = Buckets[I])
      Tmp.place_गाठ(N, N->getFoldingSetHash());

  *एतत् = std::move(Tmp);
}

शून्य FoldingSetBase::reserve(unsigned N) {
  यदि (N * 4 <= अंक_Buckets * 3)
    प्रत्यागम;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

शून्य FoldingSetBase::घाला(Folding_Set_गाठ *N, Folding_Set_घाला_Token Token) {
  प्रतिज्ञा(N && "Cannot insert a null node");
  प्रतिज्ञा(Token && "Invalid token!");
  incrementEpoch();
  यदि (LLVM_UNLIKELY((अंक_Nodes + 1) * 4 > अंक_Buckets * 3))
    grow(अंक_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_गाठ(N, Hash);
  N->setFoldingSetHash(Hash);
}

तार्किक FoldingSetBase::erase(Folding_Set_गाठ *N) {
  uint32_t Hash = N->getFoldingSetHash();
  यदि (Hash == Folding_Set_गाठ_IDRef::NotAHash)
    प्रत्यागम असत्य; // Never inserted.

  unsigned Mask = अंक_Buckets - 1;
  unsigned I = Hash & Mask;
  यावत् (Buckets[I] != N) {
    यदि (LLVM_UNLIKELY(!Buckets[I]))
      प्रत्यागम असत्य; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  प्रति (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    यदि (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_गाठ_IDRef::NotAHash);
  --अंक_Nodes;
  प्रत्यागम सत्य;
}
