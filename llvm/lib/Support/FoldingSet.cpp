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

#ضمن "llvm/ADT/FoldingSet.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/SwapByteOrder.h"
#ضمن <cassert>
#ضمن <cstring>
استعمال نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::عامل<(Folding_سيٽ_ڳنڍ_IDRef LHS, Folding_سيٽ_ڳنڍ_IDRef RHS) {
  إذا (LHS.سائيز_2() != RHS.سائيز_2())
    أرجع LHS.سائيز_2() < RHS.سائيز_2();
  أرجع memcmp(LHS.معلومات(), RHS.معلومات(), LHS.سائيز_2() * حجم(بنانشان)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ Folding_سيٽ_ڳنڍ_ID::جوڙيو_String(StringRef String) {
  بنانشان سائيز = String.سائيز_2();

  بنانشان انگ_Inserts = 1 + divideCeil(سائيز, 4);
  بٽ_2.reserve(بٽ_2.سائيز_2() + انگ_Inserts);

  بٽ_2.ڌڪو_پٺ(سائيز);
  إذا (!سائيز)
    أرجع;

  بنانشان Units = سائيز / 4;
  بنانشان Pos = 0;
  ثابت بنانشان *بنياد = (ثابت بنانشان *)String.معلومات();

  // If the string is aligned do a bulk transfer.
  إذا (!((intptr_t)بنياد & 3)) {
    بٽ_2.append(بنياد, بنياد + Units);
    Pos = (Units + 1) * 4;
  } وإلا {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    إذا (sys::IsBigEndianHost) {
      لكل (Pos += 4; Pos <= سائيز; Pos += 4) {
        بنانشان V = ((بنانشان حرف)String[Pos - 4] << 24) |
                     ((بنانشان حرف)String[Pos - 3] << 16) |
                     ((بنانشان حرف)String[Pos - 2] << 8) |
                     (بنانشان حرف)String[Pos - 1];
        بٽ_2.ڌڪو_پٺ(V);
      }
    } وإلا { // Little-endian host
      لكل (Pos += 4; Pos <= سائيز; Pos += 4) {
        بنانشان V = ((بنانشان حرف)String[Pos - 1] << 24) |
                     ((بنانشان حرف)String[Pos - 2] << 16) |
                     ((بنانشان حرف)String[Pos - 3] << 8) |
                     (بنانشان حرف)String[Pos - 4];
        بٽ_2.ڌڪو_پٺ(V);
      }
    }
  }

  // With the leftover bits.
  بنانشان V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - سائيز) {
  حالة 1:
    V = (V << 8) | (بنانشان حرف)String[سائيز - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (بنانشان حرف)String[سائيز - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (بنانشان حرف)String[سائيز - 1];
    اكسر;
  افتراضي:
    أرجع; // Nothing left.
  }

  بٽ_2.ڌڪو_پٺ(V);
}

فراغ Folding_سيٽ_ڳنڍ_ID::جوڙيو_ڳنڍ_ID(ثابت Folding_سيٽ_ڳنڍ_ID &ID) {
  بٽ_2.append(ID.بٽ_2.begin(), ID.بٽ_2.پڇاڙي());
}

Folding_سيٽ_ڳنڍ_IDRef
Folding_سيٽ_ڳنڍ_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  بنانشان *New = Allocator.Allocate<بنانشان>(بٽ_2.سائيز_2());
  llvm::uninitialized_copy(بٽ_2, New);
  أرجع Folding_سيٽ_ڳنڍ_IDRef(New, بٽ_2.سائيز_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_سيٽ_بنياد::Folding_سيٽ_بنياد(بنانشان Log2_Init_سائيز) {
  أكد(5 < Log2_Init_سائيز && Log2_Init_سائيز < 32 &&
         "Initial hash table size out of range");
  انگ_Buckets = 1 << Log2_Init_سائيز;
  Buckets = static_cast<Folding_سيٽ_ڳنڍ **>(
      محفوظ_calloc(انگ_Buckets, حجم(Folding_سيٽ_ڳنڍ *)));
}

Folding_سيٽ_بنياد::Folding_سيٽ_بنياد(Folding_سيٽ_بنياد &&Arg)
    : Buckets(std::exchange(Arg.Buckets, خالي_اشارو)),
      انگ_Buckets(std::exchange(Arg.انگ_Buckets, 0)),
      انگ_Nodes(std::exchange(Arg.انگ_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_سيٽ_بنياد &Folding_سيٽ_بنياد::عامل=(Folding_سيٽ_بنياد &&RHS) {
  إذا (هذا == &RHS)
    أرجع *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, خالي_اشارو);
  انگ_Buckets = std::exchange(RHS.انگ_Buckets, 0);
  انگ_Nodes = std::exchange(RHS.انگ_Nodes, 0);
  أرجع *هذا;
}

Folding_سيٽ_بنياد::~Folding_سيٽ_بنياد() { حرر(Buckets); }

فراغ Folding_سيٽ_بنياد::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  إذا (انگ_Buckets)
    memset(Buckets, 0, انگ_Buckets * حجم(Folding_سيٽ_ڳنڍ *));
  انگ_Nodes = 0;
}

فراغ Folding_سيٽ_بنياد::رکو_ڳنڍ(Folding_سيٽ_ڳنڍ *N, uint32_t Hash) {
  بنانشان نقاب = انگ_Buckets - 1;
  بنانشان I = Hash & نقاب;
  طالما (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & نقاب;
  }
  Buckets[I] = N;
  ++انگ_Nodes;
}

فراغ Folding_سيٽ_بنياد::grow(بنانشان Min_انگ_Buckets) {
  // The floor is the smallest size the constructor accepts.
  بنانشان New_ٻالٽي_ڳڻپ = std::max(64u, llvm::ٻٽو_انگ_ceil(Min_انگ_Buckets));
  أكد(New_ٻالٽي_ڳڻپ > انگ_Buckets && "Can't shrink a folding set");

  Folding_سيٽ_بنياد Tmp(llvm::Log2_32(New_ٻالٽي_ڳڻپ));
  لكل (بنانشان I = 0; I != انگ_Buckets; ++I)
    إذا (Folding_سيٽ_ڳنڍ *N = Buckets[I])
      Tmp.رکو_ڳنڍ(N, N->get_Folding_سيٽ_Hash());

  *هذا = std::move(Tmp);
}

فراغ Folding_سيٽ_بنياد::reserve(بنانشان N) {
  إذا (N * 4 <= انگ_Buckets * 3)
    أرجع;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ Folding_سيٽ_بنياد::وجهو(Folding_سيٽ_ڳنڍ *N, Folding_سيٽ_وجهو_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  إذا (LLVM_UNLIKELY((انگ_Nodes + 1) * 4 > انگ_Buckets * 3))
    grow(انگ_Buckets * 2);
  uint32_t Hash = Token.Hash;
  رکو_ڳنڍ(N, Hash);
  N->سيٽ_Folding_سيٽ_Hash(Hash);
}

منطقي Folding_سيٽ_بنياد::erase(Folding_سيٽ_ڳنڍ *N) {
  uint32_t Hash = N->get_Folding_سيٽ_Hash();
  إذا (Hash == Folding_سيٽ_ڳنڍ_IDRef::NotAHash)
    أرجع خطأ; // Never inserted.

  بنانشان نقاب = انگ_Buckets - 1;
  بنانشان I = Hash & نقاب;
  طالما (Buckets[I] != N) {
    إذا (LLVM_UNLIKELY(!Buckets[I]))
      أرجع خطأ; // Not in folding set.
    I = (I + 1) & نقاب;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  لكل (بنانشان J = (I + 1) & نقاب; Buckets[J]; J = (J + 1) & نقاب) {
    بنانشان Ideal = Buckets[J]->get_Folding_سيٽ_Hash();
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = خالي_اشارو;
  N->سيٽ_Folding_سيٽ_Hash(Folding_سيٽ_ڳنڍ_IDRef::NotAHash);
  --انگ_Nodes;
  أرجع سچ;
}
