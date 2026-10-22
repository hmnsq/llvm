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

तार्किक llvm::चालक<(Folding_Set_गाँठो_IDRef LHS, Folding_Set_गाँठो_IDRef RHS) {
  यदि (LHS.size() != RHS.size())
    फर्काऊ LHS.size() < RHS.size();
  फर्काऊ स_भेद(LHS.data(), RHS.data(), LHS.size() * परिमाण(अचिन्हित)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

शून्य Folding_Set_गाँठो_ID::AddString(StringRef String) {
  अचिन्हित Size = String.size();

  अचिन्हित अङ्क_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + अङ्क_Inserts);

  Bits.थप_back(Size);
  यदि (!Size)
    फर्काऊ;

  अचिन्हित Units = Size / 4;
  अचिन्हित Pos = 0;
  अचर अचिन्हित *Base = (अचर अचिन्हित *)String.data();

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
      लागि (Pos += 4; Pos <= Size; Pos += 4) {
        अचिन्हित V = ((अचिन्हित वर्ण)String[Pos - 4] << 24) |
                     ((अचिन्हित वर्ण)String[Pos - 3] << 16) |
                     ((अचिन्हित वर्ण)String[Pos - 2] << 8) |
                     (अचिन्हित वर्ण)String[Pos - 1];
        Bits.थप_back(V);
      }
    } अन्यथा { // Little-endian host
      लागि (Pos += 4; Pos <= Size; Pos += 4) {
        अचिन्हित V = ((अचिन्हित वर्ण)String[Pos - 1] << 24) |
                     ((अचिन्हित वर्ण)String[Pos - 2] << 16) |
                     ((अचिन्हित वर्ण)String[Pos - 3] << 8) |
                     (अचिन्हित वर्ण)String[Pos - 4];
        Bits.थप_back(V);
      }
    }
  }

  // With the leftover bits.
  अचिन्हित V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  विकल्प (Pos - Size) {
  पक्ष 1:
    V = (V << 8) | (अचिन्हित वर्ण)String[Size - 3];
    [[fallthrough]];
  पक्ष 2:
    V = (V << 8) | (अचिन्हित वर्ण)String[Size - 2];
    [[fallthrough]];
  पक्ष 3:
    V = (V << 8) | (अचिन्हित वर्ण)String[Size - 1];
    रोक;
  स्वभाव:
    फर्काऊ; // Nothing left.
  }

  Bits.थप_back(V);
}

शून्य Folding_Set_गाँठो_ID::Add_गाँठो_ID(अचर Folding_Set_गाँठो_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_गाँठो_IDRef
Folding_Set_गाँठो_ID::Intern(BumpPtrAllocator &Allocator) अचर {
  अचिन्हित *New = Allocator.Allocate<अचिन्हित>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  फर्काऊ Folding_Set_गाँठो_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(अचिन्हित Log2InitSize) {
  प्रतिज्ञा(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  अङ्क_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_गाँठो **>(
      safe_calloc(अङ्क_Buckets, परिमाण(Folding_Set_गाँठो *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      अङ्क_Buckets(std::exchange(Arg.अङ्क_Buckets, 0)),
      अङ्क_Nodes(std::exchange(Arg.अङ्क_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::चालक=(FoldingSetBase &&RHS) {
  यदि (एतत् == &RHS)
    फर्काऊ *एतत्;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  अङ्क_Buckets = std::exchange(RHS.अङ्क_Buckets, 0);
  अङ्क_Nodes = std::exchange(RHS.अङ्क_Nodes, 0);
  फर्काऊ *एतत्;
}

FoldingSetBase::~FoldingSetBase() { मुक्त(Buckets); }

शून्य FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  यदि (अङ्क_Buckets)
    स_रखो(Buckets, 0, अङ्क_Buckets * परिमाण(Folding_Set_गाँठो *));
  अङ्क_Nodes = 0;
}

शून्य FoldingSetBase::place_गाँठो(Folding_Set_गाँठो *N, uint32_t Hash) {
  अचिन्हित Mask = अङ्क_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  जबसम्म (Buckets[I]) {
    प्रतिज्ञा(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++अङ्क_Nodes;
}

शून्य FoldingSetBase::grow(अचिन्हित Min_अङ्क_Buckets) {
  // The floor is the smallest size the constructor accepts.
  अचिन्हित New_Bucket_गन्ती = std::max(64u, llvm::bit_ceil(Min_अङ्क_Buckets));
  प्रतिज्ञा(New_Bucket_गन्ती > अङ्क_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_गन्ती));
  लागि (अचिन्हित I = 0; I != अङ्क_Buckets; ++I)
    यदि (Folding_Set_गाँठो *N = Buckets[I])
      Tmp.place_गाँठो(N, N->getFoldingSetHash());

  *एतत् = std::move(Tmp);
}

शून्य FoldingSetBase::reserve(अचिन्हित N) {
  यदि (N * 4 <= अङ्क_Buckets * 3)
    फर्काऊ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

शून्य FoldingSetBase::घुसाउ(Folding_Set_गाँठो *N, Folding_Set_घुसाउ_Token Token) {
  प्रतिज्ञा(N && "Cannot insert a null node");
  प्रतिज्ञा(Token && "Invalid token!");
  incrementEpoch();
  यदि (LLVM_UNLIKELY((अङ्क_Nodes + 1) * 4 > अङ्क_Buckets * 3))
    grow(अङ्क_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_गाँठो(N, Hash);
  N->setFoldingSetHash(Hash);
}

तार्किक FoldingSetBase::erase(Folding_Set_गाँठो *N) {
  uint32_t Hash = N->getFoldingSetHash();
  यदि (Hash == Folding_Set_गाँठो_IDRef::NotAHash)
    फर्काऊ असत्य; // Never inserted.

  अचिन्हित Mask = अङ्क_Buckets - 1;
  अचिन्हित I = Hash & Mask;
  जबसम्म (Buckets[I] != N) {
    यदि (LLVM_UNLIKELY(!Buckets[I]))
      फर्काऊ असत्य; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  लागि (अचिन्हित J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    अचिन्हित Ideal = Buckets[J]->getFoldingSetHash();
    यदि (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_गाँठो_IDRef::NotAHash);
  --अङ्क_Nodes;
  फर्काऊ सत्य;
}
