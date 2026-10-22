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
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

ತಾರ್ಕಿಕ llvm::ಕ್ರಿಯಾಕಾರಕ<(Folding_ಗಣ_ಗಂಟು_IDRef LHS, Folding_ಗಣ_ಗಂಟು_IDRef RHS) {
  ಆದರೆ (LHS.ಗಾತ್ರ_3() != RHS.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ LHS.ಗಾತ್ರ_3() < RHS.ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ memcmp(LHS.ದತ್ತಾಂಶ(), RHS.ದತ್ತಾಂಶ(), LHS.ಗಾತ್ರ_3() * ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

ಶೂನ್ಯ Folding_ಗಣ_ಗಂಟು_ID::ಕೂಡಿಸು_String(StringRef String) {
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = String.ಗಾತ್ರ_3();

  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Inserts = 1 + divideCeil(ಗಾತ್ರ_2, 4);
  ಬಿಟ್ಟುಗಳು_2.reserve(ಬಿಟ್ಟುಗಳು_2.ಗಾತ್ರ_3() + ಅಂಕೆ_Inserts);

  ಬಿಟ್ಟುಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(ಗಾತ್ರ_2);
  ಆದರೆ (!ಗಾತ್ರ_2)
    ಹಿಂತಿರುಗಿ;

  ಚಿಹ್ನೆರಹಿತ Units = ಗಾತ್ರ_2 / 4;
  ಚಿಹ್ನೆರಹಿತ Pos = 0;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ *ತಳ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ *)String.ದತ್ತಾಂಶ();

  // If the string is aligned do a bulk transfer.
  ಆದರೆ (!((intptr_t)ತಳ & 3)) {
    ಬಿಟ್ಟುಗಳು_2.append(ತಳ, ತಳ + Units);
    Pos = (Units + 1) * 4;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    ಆದರೆ (sys::IsBigEndianHost) {
      ಗಾಗಿ (Pos += 4; Pos <= ಗಾತ್ರ_2; Pos += 4) {
        ಚಿಹ್ನೆರಹಿತ V = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 4] << 24) |
                     ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 3] << 16) |
                     ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 2] << 8) |
                     (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 1];
        ಬಿಟ್ಟುಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(V);
      }
    } ಇಲ್ಲದಿದ್ದರೆ { // Little-endian host
      ಗಾಗಿ (Pos += 4; Pos <= ಗಾತ್ರ_2; Pos += 4) {
        ಚಿಹ್ನೆರಹಿತ V = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 1] << 24) |
                     ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 2] << 16) |
                     ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 3] << 8) |
                     (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[Pos - 4];
        ಬಿಟ್ಟುಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(V);
      }
    }
  }

  // With the leftover bits.
  ಚಿಹ್ನೆರಹಿತ V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ಆಯ್ಕೆ (Pos - ಗಾತ್ರ_2) {
  ಸ್ಥಿತಿ 1:
    V = (V << 8) | (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[ಗಾತ್ರ_2 - 3];
    [[fallthrough]];
  ಸ್ಥಿತಿ 2:
    V = (V << 8) | (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[ಗಾತ್ರ_2 - 2];
    [[fallthrough]];
  ಸ್ಥಿತಿ 3:
    V = (V << 8) | (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)String[ಗಾತ್ರ_2 - 1];
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ; // Nothing left.
  }

  ಬಿಟ್ಟುಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(V);
}

ಶೂನ್ಯ Folding_ಗಣ_ಗಂಟು_ID::ಕೂಡಿಸು_ಗಂಟು_ID(ಸ್ಥಿರಾಂಕ Folding_ಗಣ_ಗಂಟು_ID &ID) {
  ಬಿಟ್ಟುಗಳು_2.append(ID.ಬಿಟ್ಟುಗಳು_2.begin(), ID.ಬಿಟ್ಟುಗಳು_2.ಕೊನೆ());
}

Folding_ಗಣ_ಗಂಟು_IDRef
Folding_ಗಣ_ಗಂಟು_ID::Intern(BumpPtrAllocator &Allocator) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ *New = Allocator.Allocate<ಚಿಹ್ನೆರಹಿತ>(ಬಿಟ್ಟುಗಳು_2.ಗಾತ್ರ_3());
  llvm::uninitialized_copy(ಬಿಟ್ಟುಗಳು_2, New);
  ಹಿಂತಿರುಗಿ Folding_ಗಣ_ಗಂಟು_IDRef(New, ಬಿಟ್ಟುಗಳು_2.ಗಾತ್ರ_3());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_ಗಣ_ತಳ::Folding_ಗಣ_ತಳ(ಚಿಹ್ನೆರಹಿತ Log2_Init_ಗಾತ್ರ) {
  ಪ್ರತಿಜ್ಞಾ(5 < Log2_Init_ಗಾತ್ರ && Log2_Init_ಗಾತ್ರ < 32 &&
         "Initial hash table size out of range");
  ಅಂಕೆ_Buckets = 1 << Log2_Init_ಗಾತ್ರ;
  Buckets = static_cast<Folding_ಗಣ_ಗಂಟು **>(
      ಸುರಕ್ಷಿತ_calloc(ಅಂಕೆ_Buckets, ಗಾತ್ರ(Folding_ಗಣ_ಗಂಟು *)));
}

Folding_ಗಣ_ತಳ::Folding_ಗಣ_ತಳ(Folding_ಗಣ_ತಳ &&Arg)
    : Buckets(std::exchange(Arg.Buckets, ಶೂನ್ಯಸೂಚಕ)),
      ಅಂಕೆ_Buckets(std::exchange(Arg.ಅಂಕೆ_Buckets, 0)),
      ಅಂಕೆ_Nodes(std::exchange(Arg.ಅಂಕೆ_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_ಗಣ_ತಳ &Folding_ಗಣ_ತಳ::ಕ್ರಿಯಾಕಾರಕ=(Folding_ಗಣ_ತಳ &&RHS) {
  ಆದರೆ (ಇದು == &RHS)
    ಹಿಂತಿರುಗಿ *ಇದು;

  incrementEpoch();
  RHS.incrementEpoch();
  ಬಿಡುಗಡೆ(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, ಶೂನ್ಯಸೂಚಕ);
  ಅಂಕೆ_Buckets = std::exchange(RHS.ಅಂಕೆ_Buckets, 0);
  ಅಂಕೆ_Nodes = std::exchange(RHS.ಅಂಕೆ_Nodes, 0);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

Folding_ಗಣ_ತಳ::~Folding_ಗಣ_ತಳ() { ಬಿಡುಗಡೆ(Buckets); }

ಶೂನ್ಯ Folding_ಗಣ_ತಳ::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  ಆದರೆ (ಅಂಕೆ_Buckets)
    memset(Buckets, 0, ಅಂಕೆ_Buckets * ಗಾತ್ರ(Folding_ಗಣ_ಗಂಟು *));
  ಅಂಕೆ_Nodes = 0;
}

ಶೂನ್ಯ Folding_ಗಣ_ತಳ::ಇರಿಸು_ಗಂಟು(Folding_ಗಣ_ಗಂಟು *N, uint32_t Hash) {
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = ಅಂಕೆ_Buckets - 1;
  ಚಿಹ್ನೆರಹಿತ I = Hash & ಮುಖವಾಡ;
  ಆಗ (Buckets[I]) {
    ಪ್ರತಿಜ್ಞಾ(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & ಮುಖವಾಡ;
  }
  Buckets[I] = N;
  ++ಅಂಕೆ_Nodes;
}

ಶೂನ್ಯ Folding_ಗಣ_ತಳ::grow(ಚಿಹ್ನೆರಹಿತ Min_ಅಂಕೆ_Buckets) {
  // The floor is the smallest size the constructor accepts.
  ಚಿಹ್ನೆರಹಿತ New_ಪಾತ್ರೆ_ಎಣಿಕೆ = std::max(64u, llvm::ಬಿಟ್_ceil(Min_ಅಂಕೆ_Buckets));
  ಪ್ರತಿಜ್ಞಾ(New_ಪಾತ್ರೆ_ಎಣಿಕೆ > ಅಂಕೆ_Buckets && "Can't shrink a folding set");

  Folding_ಗಣ_ತಳ Tmp(llvm::Log2_32(New_ಪಾತ್ರೆ_ಎಣಿಕೆ));
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I != ಅಂಕೆ_Buckets; ++I)
    ಆದರೆ (Folding_ಗಣ_ಗಂಟು *N = Buckets[I])
      Tmp.ಇರಿಸು_ಗಂಟು(N, N->get_Folding_ಗಣ_Hash());

  *ಇದು = std::move(Tmp);
}

ಶೂನ್ಯ Folding_ಗಣ_ತಳ::reserve(ಚಿಹ್ನೆರಹಿತ N) {
  ಆದರೆ (N * 4 <= ಅಂಕೆ_Buckets * 3)
    ಹಿಂತಿರುಗಿ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

ಶೂನ್ಯ Folding_ಗಣ_ತಳ::ಸೇರಿಸು(Folding_ಗಣ_ಗಂಟು *N, Folding_ಗಣ_ಸೇರಿಸು_Token Token) {
  ಪ್ರತಿಜ್ಞಾ(N && "Cannot insert a null node");
  ಪ್ರತಿಜ್ಞಾ(Token && "Invalid token!");
  incrementEpoch();
  ಆದರೆ (LLVM_UNLIKELY((ಅಂಕೆ_Nodes + 1) * 4 > ಅಂಕೆ_Buckets * 3))
    grow(ಅಂಕೆ_Buckets * 2);
  uint32_t Hash = Token.Hash;
  ಇರಿಸು_ಗಂಟು(N, Hash);
  N->ಗಣ_Folding_ಗಣ_Hash(Hash);
}

ತಾರ್ಕಿಕ Folding_ಗಣ_ತಳ::erase(Folding_ಗಣ_ಗಂಟು *N) {
  uint32_t Hash = N->get_Folding_ಗಣ_Hash();
  ಆದರೆ (Hash == Folding_ಗಣ_ಗಂಟು_IDRef::NotAHash)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; // Never inserted.

  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ = ಅಂಕೆ_Buckets - 1;
  ಚಿಹ್ನೆರಹಿತ I = Hash & ಮುಖವಾಡ;
  ಆಗ (Buckets[I] != N) {
    ಆದರೆ (LLVM_UNLIKELY(!Buckets[I]))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; // Not in folding set.
    I = (I + 1) & ಮುಖವಾಡ;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ J = (I + 1) & ಮುಖವಾಡ; Buckets[J]; J = (J + 1) & ಮುಖವಾಡ) {
    ಚಿಹ್ನೆರಹಿತ Ideal = Buckets[J]->get_Folding_ಗಣ_Hash();
    ಆದರೆ (((I - Ideal) & ಮುಖವಾಡ) < ((J - Ideal) & ಮುಖವಾಡ)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = ಶೂನ್ಯಸೂಚಕ;
  N->ಗಣ_Folding_ಗಣ_Hash(Folding_ಗಣ_ಗಂಟು_IDRef::NotAHash);
  --ಅಂಕೆ_Nodes;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}
