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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FoldingSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Allocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SwapByteOrder.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

ਤਾਰਕਿਕ llvm::ਸੰਚਾਲਕ<(Folding_ਸੈੱਟ_ਗੰਢ_IDRef LHS, Folding_ਸੈੱਟ_ਗੰਢ_IDRef RHS) {
  ਜੇ (LHS.ਮਾਪ_2() != RHS.ਮਾਪ_2())
    ਵਾਪਸ LHS.ਮਾਪ_2() < RHS.ਮਾਪ_2();
  ਵਾਪਸ memcmp(LHS.ਅੰਕੜੇ(), RHS.ਅੰਕੜੇ(), LHS.ਮਾਪ_2() * ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

ਖਾਲੀ Folding_ਸੈੱਟ_ਗੰਢ_ID::ਜੋੜੋ_String(StringRef String) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = String.ਮਾਪ_2();

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Inserts = 1 + divideCeil(ਮਾਪ, 4);
  ਬਿੱਟ_2.reserve(ਬਿੱਟ_2.ਮਾਪ_2() + ਅੰਕ_Inserts);

  ਬਿੱਟ_2.ਧੱਕੋ_ਪਿੱਠ(ਮਾਪ);
  ਜੇ (!ਮਾਪ)
    ਵਾਪਸ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Units = ਮਾਪ / 4;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Pos = 0;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ *ਅਧਾਰ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ *)String.ਅੰਕੜੇ();

  // If the string is aligned do a bulk transfer.
  ਜੇ (!((intptr_t)ਅਧਾਰ & 3)) {
    ਬਿੱਟ_2.append(ਅਧਾਰ, ਅਧਾਰ + Units);
    Pos = (Units + 1) * 4;
  } ਨਹੀਂਤਾਂ {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    ਜੇ (sys::IsBigEndianHost) {
      ਲਈ (Pos += 4; Pos <= ਮਾਪ; Pos += 4) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ V = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 4] << 24) |
                     ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 3] << 16) |
                     ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 2] << 8) |
                     (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 1];
        ਬਿੱਟ_2.ਧੱਕੋ_ਪਿੱਠ(V);
      }
    } ਨਹੀਂਤਾਂ { // Little-endian host
      ਲਈ (Pos += 4; Pos <= ਮਾਪ; Pos += 4) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ V = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 1] << 24) |
                     ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 2] << 16) |
                     ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 3] << 8) |
                     (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[Pos - 4];
        ਬਿੱਟ_2.ਧੱਕੋ_ਪਿੱਠ(V);
      }
    }
  }

  // With the leftover bits.
  ਬਿਨਾਨਿਸ਼ਾਨੀ V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  ਚੋਣ (Pos - ਮਾਪ) {
  ਹਾਲਤ 1:
    V = (V << 8) | (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[ਮਾਪ - 3];
    [[fallthrough]];
  ਹਾਲਤ 2:
    V = (V << 8) | (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[ਮਾਪ - 2];
    [[fallthrough]];
  ਹਾਲਤ 3:
    V = (V << 8) | (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)String[ਮਾਪ - 1];
    ਰੋਕੋ;
  ਮੂਲ:
    ਵਾਪਸ; // Nothing left.
  }

  ਬਿੱਟ_2.ਧੱਕੋ_ਪਿੱਠ(V);
}

ਖਾਲੀ Folding_ਸੈੱਟ_ਗੰਢ_ID::ਜੋੜੋ_ਗੰਢ_ID(ਅਚਲ Folding_ਸੈੱਟ_ਗੰਢ_ID &ID) {
  ਬਿੱਟ_2.append(ID.ਬਿੱਟ_2.begin(), ID.ਬਿੱਟ_2.ਸਿਰਾ());
}

Folding_ਸੈੱਟ_ਗੰਢ_IDRef
Folding_ਸੈੱਟ_ਗੰਢ_ID::Intern(BumpPtrAllocator &Allocator) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ *New = Allocator.Allocate<ਬਿਨਾਨਿਸ਼ਾਨੀ>(ਬਿੱਟ_2.ਮਾਪ_2());
  llvm::uninitialized_copy(ਬਿੱਟ_2, New);
  ਵਾਪਸ Folding_ਸੈੱਟ_ਗੰਢ_IDRef(New, ਬਿੱਟ_2.ਮਾਪ_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_ਸੈੱਟ_ਅਧਾਰ::Folding_ਸੈੱਟ_ਅਧਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ Log2_Init_ਮਾਪ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(5 < Log2_Init_ਮਾਪ && Log2_Init_ਮਾਪ < 32 &&
         "Initial hash table size out of range");
  ਅੰਕ_Buckets = 1 << Log2_Init_ਮਾਪ;
  Buckets = static_cast<Folding_ਸੈੱਟ_ਗੰਢ **>(
      ਸੁਰੱਖਿਅਤ_calloc(ਅੰਕ_Buckets, ਆਕਾਰ(Folding_ਸੈੱਟ_ਗੰਢ *)));
}

Folding_ਸੈੱਟ_ਅਧਾਰ::Folding_ਸੈੱਟ_ਅਧਾਰ(Folding_ਸੈੱਟ_ਅਧਾਰ &&Arg)
    : Buckets(std::exchange(Arg.Buckets, ਖਾਲੀ_ਸੂਚਕ)),
      ਅੰਕ_Buckets(std::exchange(Arg.ਅੰਕ_Buckets, 0)),
      ਅੰਕ_Nodes(std::exchange(Arg.ਅੰਕ_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_ਸੈੱਟ_ਅਧਾਰ &Folding_ਸੈੱਟ_ਅਧਾਰ::ਸੰਚਾਲਕ=(Folding_ਸੈੱਟ_ਅਧਾਰ &&RHS) {
  ਜੇ (ਇਹ == &RHS)
    ਵਾਪਸ *ਇਹ;

  incrementEpoch();
  RHS.incrementEpoch();
  ਮੁਕਤ(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, ਖਾਲੀ_ਸੂਚਕ);
  ਅੰਕ_Buckets = std::exchange(RHS.ਅੰਕ_Buckets, 0);
  ਅੰਕ_Nodes = std::exchange(RHS.ਅੰਕ_Nodes, 0);
  ਵਾਪਸ *ਇਹ;
}

Folding_ਸੈੱਟ_ਅਧਾਰ::~Folding_ਸੈੱਟ_ਅਧਾਰ() { ਮੁਕਤ(Buckets); }

ਖਾਲੀ Folding_ਸੈੱਟ_ਅਧਾਰ::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  ਜੇ (ਅੰਕ_Buckets)
    memset(Buckets, 0, ਅੰਕ_Buckets * ਆਕਾਰ(Folding_ਸੈੱਟ_ਗੰਢ *));
  ਅੰਕ_Nodes = 0;
}

ਖਾਲੀ Folding_ਸੈੱਟ_ਅਧਾਰ::ਰੱਖੋ_ਗੰਢ(Folding_ਸੈੱਟ_ਗੰਢ *N, uint32_t Hash) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = ਅੰਕ_Buckets - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ I = Hash & ਨਕਾਬ;
  ਜਦੋਂ (Buckets[I]) {
    ਪ੍ਰਤਿਜ੍ਞਾ(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & ਨਕਾਬ;
  }
  Buckets[I] = N;
  ++ਅੰਕ_Nodes;
}

ਖਾਲੀ Folding_ਸੈੱਟ_ਅਧਾਰ::grow(ਬਿਨਾਨਿਸ਼ਾਨੀ Min_ਅੰਕ_Buckets) {
  // The floor is the smallest size the constructor accepts.
  ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਬਾਲਟੀ_ਗਿਣਤੀ = std::max(64u, llvm::ਦੋਆਧਾਰੀ_ਅੰਕ_ceil(Min_ਅੰਕ_Buckets));
  ਪ੍ਰਤਿਜ੍ਞਾ(New_ਬਾਲਟੀ_ਗਿਣਤੀ > ਅੰਕ_Buckets && "Can't shrink a folding set");

  Folding_ਸੈੱਟ_ਅਧਾਰ Tmp(llvm::Log2_32(New_ਬਾਲਟੀ_ਗਿਣਤੀ));
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I != ਅੰਕ_Buckets; ++I)
    ਜੇ (Folding_ਸੈੱਟ_ਗੰਢ *N = Buckets[I])
      Tmp.ਰੱਖੋ_ਗੰਢ(N, N->get_Folding_ਸੈੱਟ_Hash());

  *ਇਹ = std::move(Tmp);
}

ਖਾਲੀ Folding_ਸੈੱਟ_ਅਧਾਰ::reserve(ਬਿਨਾਨਿਸ਼ਾਨੀ N) {
  ਜੇ (N * 4 <= ਅੰਕ_Buckets * 3)
    ਵਾਪਸ;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

ਖਾਲੀ Folding_ਸੈੱਟ_ਅਧਾਰ::ਪਾਓ(Folding_ਸੈੱਟ_ਗੰਢ *N, Folding_ਸੈੱਟ_ਪਾਓ_Token Token) {
  ਪ੍ਰਤਿਜ੍ਞਾ(N && "Cannot insert a null node");
  ਪ੍ਰਤਿਜ੍ਞਾ(Token && "Invalid token!");
  incrementEpoch();
  ਜੇ (LLVM_UNLIKELY((ਅੰਕ_Nodes + 1) * 4 > ਅੰਕ_Buckets * 3))
    grow(ਅੰਕ_Buckets * 2);
  uint32_t Hash = Token.Hash;
  ਰੱਖੋ_ਗੰਢ(N, Hash);
  N->ਸੈੱਟ_Folding_ਸੈੱਟ_Hash(Hash);
}

ਤਾਰਕਿਕ Folding_ਸੈੱਟ_ਅਧਾਰ::erase(Folding_ਸੈੱਟ_ਗੰਢ *N) {
  uint32_t Hash = N->get_Folding_ਸੈੱਟ_Hash();
  ਜੇ (Hash == Folding_ਸੈੱਟ_ਗੰਢ_IDRef::NotAHash)
    ਵਾਪਸ ਝੂਠ; // Never inserted.

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ = ਅੰਕ_Buckets - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ I = Hash & ਨਕਾਬ;
  ਜਦੋਂ (Buckets[I] != N) {
    ਜੇ (LLVM_UNLIKELY(!Buckets[I]))
      ਵਾਪਸ ਝੂਠ; // Not in folding set.
    I = (I + 1) & ਨਕਾਬ;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ J = (I + 1) & ਨਕਾਬ; Buckets[J]; J = (J + 1) & ਨਕਾਬ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Ideal = Buckets[J]->get_Folding_ਸੈੱਟ_Hash();
    ਜੇ (((I - Ideal) & ਨਕਾਬ) < ((J - Ideal) & ਨਕਾਬ)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = ਖਾਲੀ_ਸੂਚਕ;
  N->ਸੈੱਟ_Folding_ਸੈੱਟ_Hash(Folding_ਸੈੱਟ_ਗੰਢ_IDRef::NotAHash);
  --ਅੰਕ_Nodes;
  ਵਾਪਸ ਸੱਚ;
}
