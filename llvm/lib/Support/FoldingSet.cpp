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
ورتوں نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::عامل<(Folding_سیٹ_گرہ_IDRef LHS, Folding_سیٹ_گرہ_IDRef RHS) {
  إذا (LHS.ناپ_2() != RHS.ناپ_2())
    أرجع LHS.ناپ_2() < RHS.ناپ_2();
  أرجع memcmp(LHS.انکڑے(), RHS.انکڑے(), LHS.ناپ_2() * حجم(بنانشانی)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ Folding_سیٹ_گرہ_ID::جوڑو_String(StringRef String) {
  بنانشانی ناپ = String.ناپ_2();

  بنانشانی عدد_Inserts = 1 + divideCeil(ناپ, 4);
  بٹ_2.reserve(بٹ_2.ناپ_2() + عدد_Inserts);

  بٹ_2.ڈالو_پٹھ(ناپ);
  إذا (!ناپ)
    أرجع;

  بنانشانی Units = ناپ / 4;
  بنانشانی Pos = 0;
  ثابت بنانشانی *ادھار = (ثابت بنانشانی *)String.انکڑے();

  // If the string is aligned do a bulk transfer.
  إذا (!((intptr_t)ادھار & 3)) {
    بٹ_2.append(ادھار, ادھار + Units);
    Pos = (Units + 1) * 4;
  } وإلا {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    إذا (sys::IsBigEndianHost) {
      لكل (Pos += 4; Pos <= ناپ; Pos += 4) {
        بنانشانی V = ((بنانشانی حرف)String[Pos - 4] << 24) |
                     ((بنانشانی حرف)String[Pos - 3] << 16) |
                     ((بنانشانی حرف)String[Pos - 2] << 8) |
                     (بنانشانی حرف)String[Pos - 1];
        بٹ_2.ڈالو_پٹھ(V);
      }
    } وإلا { // Little-endian host
      لكل (Pos += 4; Pos <= ناپ; Pos += 4) {
        بنانشانی V = ((بنانشانی حرف)String[Pos - 1] << 24) |
                     ((بنانشانی حرف)String[Pos - 2] << 16) |
                     ((بنانشانی حرف)String[Pos - 3] << 8) |
                     (بنانشانی حرف)String[Pos - 4];
        بٹ_2.ڈالو_پٹھ(V);
      }
    }
  }

  // With the leftover bits.
  بنانشانی V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - ناپ) {
  حالة 1:
    V = (V << 8) | (بنانشانی حرف)String[ناپ - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (بنانشانی حرف)String[ناپ - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (بنانشانی حرف)String[ناپ - 1];
    اكسر;
  افتراضي:
    أرجع; // Nothing left.
  }

  بٹ_2.ڈالو_پٹھ(V);
}

فراغ Folding_سیٹ_گرہ_ID::جوڑو_گرہ_ID(ثابت Folding_سیٹ_گرہ_ID &ID) {
  بٹ_2.append(ID.بٹ_2.begin(), ID.بٹ_2.سرا());
}

Folding_سیٹ_گرہ_IDRef
Folding_سیٹ_گرہ_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  بنانشانی *New = Allocator.Allocate<بنانشانی>(بٹ_2.ناپ_2());
  llvm::uninitialized_copy(بٹ_2, New);
  أرجع Folding_سیٹ_گرہ_IDRef(New, بٹ_2.ناپ_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_سیٹ_ادھار::Folding_سیٹ_ادھار(بنانشانی Log2_Init_ناپ) {
  أكد(5 < Log2_Init_ناپ && Log2_Init_ناپ < 32 &&
         "Initial hash table size out of range");
  عدد_Buckets = 1 << Log2_Init_ناپ;
  Buckets = static_cast<Folding_سیٹ_گرہ **>(
      محفوظ_calloc(عدد_Buckets, حجم(Folding_سیٹ_گرہ *)));
}

Folding_سیٹ_ادھار::Folding_سیٹ_ادھار(Folding_سیٹ_ادھار &&Arg)
    : Buckets(std::exchange(Arg.Buckets, خالی_سوچک)),
      عدد_Buckets(std::exchange(Arg.عدد_Buckets, 0)),
      عدد_Nodes(std::exchange(Arg.عدد_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_سیٹ_ادھار &Folding_سیٹ_ادھار::عامل=(Folding_سیٹ_ادھار &&RHS) {
  إذا (هذا == &RHS)
    أرجع *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, خالی_سوچک);
  عدد_Buckets = std::exchange(RHS.عدد_Buckets, 0);
  عدد_Nodes = std::exchange(RHS.عدد_Nodes, 0);
  أرجع *هذا;
}

Folding_سیٹ_ادھار::~Folding_سیٹ_ادھار() { حرر(Buckets); }

فراغ Folding_سیٹ_ادھار::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  إذا (عدد_Buckets)
    memset(Buckets, 0, عدد_Buckets * حجم(Folding_سیٹ_گرہ *));
  عدد_Nodes = 0;
}

فراغ Folding_سیٹ_ادھار::رکھو_گرہ(Folding_سیٹ_گرہ *N, uint32_t Hash) {
  بنانشانی نقاب = عدد_Buckets - 1;
  بنانشانی I = Hash & نقاب;
  طالما (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & نقاب;
  }
  Buckets[I] = N;
  ++عدد_Nodes;
}

فراغ Folding_سیٹ_ادھار::grow(بنانشانی Min_عدد_Buckets) {
  // The floor is the smallest size the constructor accepts.
  بنانشانی New_بالٹی_گنتی = std::max(64u, llvm::ثنائی_ہندسہ_ceil(Min_عدد_Buckets));
  أكد(New_بالٹی_گنتی > عدد_Buckets && "Can't shrink a folding set");

  Folding_سیٹ_ادھار Tmp(llvm::Log2_32(New_بالٹی_گنتی));
  لكل (بنانشانی I = 0; I != عدد_Buckets; ++I)
    إذا (Folding_سیٹ_گرہ *N = Buckets[I])
      Tmp.رکھو_گرہ(N, N->get_Folding_سیٹ_Hash());

  *هذا = std::move(Tmp);
}

فراغ Folding_سیٹ_ادھار::reserve(بنانشانی N) {
  إذا (N * 4 <= عدد_Buckets * 3)
    أرجع;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ Folding_سیٹ_ادھار::داخل_کرو(Folding_سیٹ_گرہ *N, Folding_سیٹ_داخل_کرو_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  إذا (LLVM_UNLIKELY((عدد_Nodes + 1) * 4 > عدد_Buckets * 3))
    grow(عدد_Buckets * 2);
  uint32_t Hash = Token.Hash;
  رکھو_گرہ(N, Hash);
  N->سیٹ_Folding_سیٹ_Hash(Hash);
}

منطقي Folding_سیٹ_ادھار::erase(Folding_سیٹ_گرہ *N) {
  uint32_t Hash = N->get_Folding_سیٹ_Hash();
  إذا (Hash == Folding_سیٹ_گرہ_IDRef::NotAHash)
    أرجع خطأ; // Never inserted.

  بنانشانی نقاب = عدد_Buckets - 1;
  بنانشانی I = Hash & نقاب;
  طالما (Buckets[I] != N) {
    إذا (LLVM_UNLIKELY(!Buckets[I]))
      أرجع خطأ; // Not in folding set.
    I = (I + 1) & نقاب;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  لكل (بنانشانی J = (I + 1) & نقاب; Buckets[J]; J = (J + 1) & نقاب) {
    بنانشانی Ideal = Buckets[J]->get_Folding_سیٹ_Hash();
    إذا (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = خالی_سوچک;
  N->سیٹ_Folding_سیٹ_Hash(Folding_سیٹ_گرہ_IDRef::NotAHash);
  --عدد_Nodes;
  أرجع سچ;
}
