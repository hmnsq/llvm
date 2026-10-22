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

#شامل "llvm/ADT/FoldingSet.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/SwapByteOrder.h"
#شامل <cassert>
#شامل <cstring>
استعمال نامخلا llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقی llvm::عامل<(Folding_سیٹ_گرہ_IDRef LHS, Folding_سیٹ_گرہ_IDRef RHS) {
  اگر (LHS.سائز_2() != RHS.سائز_2())
    واپس LHS.سائز_2() < RHS.سائز_2();
  واپس memcmp(LHS.ڈیٹا(), RHS.ڈیٹا(), LHS.سائز_2() * حجم(غیرعلامتی)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

خالی Folding_سیٹ_گرہ_ID::جوڑو_String(StringRef String) {
  غیرعلامتی سائز = String.سائز_2();

  غیرعلامتی عدد_Inserts = 1 + divideCeil(سائز, 4);
  بٹ_2.reserve(بٹ_2.سائز_2() + عدد_Inserts);

  بٹ_2.ڈالو_عقب(سائز);
  اگر (!سائز)
    واپس;

  غیرعلامتی Units = سائز / 4;
  غیرعلامتی Pos = 0;
  ثابت غیرعلامتی *بنیاد = (ثابت غیرعلامتی *)String.ڈیٹا();

  // If the string is aligned do a bulk transfer.
  اگر (!((intptr_t)بنیاد & 3)) {
    بٹ_2.append(بنیاد, بنیاد + Units);
    Pos = (Units + 1) * 4;
  } ورنہ {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    اگر (sys::IsBigEndianHost) {
      برائے (Pos += 4; Pos <= سائز; Pos += 4) {
        غیرعلامتی V = ((غیرعلامتی حرف)String[Pos - 4] << 24) |
                     ((غیرعلامتی حرف)String[Pos - 3] << 16) |
                     ((غیرعلامتی حرف)String[Pos - 2] << 8) |
                     (غیرعلامتی حرف)String[Pos - 1];
        بٹ_2.ڈالو_عقب(V);
      }
    } ورنہ { // Little-endian host
      برائے (Pos += 4; Pos <= سائز; Pos += 4) {
        غیرعلامتی V = ((غیرعلامتی حرف)String[Pos - 1] << 24) |
                     ((غیرعلامتی حرف)String[Pos - 2] << 16) |
                     ((غیرعلامتی حرف)String[Pos - 3] << 8) |
                     (غیرعلامتی حرف)String[Pos - 4];
        بٹ_2.ڈالو_عقب(V);
      }
    }
  }

  // With the leftover bits.
  غیرعلامتی V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  انتخاب (Pos - سائز) {
  حالت 1:
    V = (V << 8) | (غیرعلامتی حرف)String[سائز - 3];
    [[fallthrough]];
  حالت 2:
    V = (V << 8) | (غیرعلامتی حرف)String[سائز - 2];
    [[fallthrough]];
  حالت 3:
    V = (V << 8) | (غیرعلامتی حرف)String[سائز - 1];
    توڑو;
  طےشدہ:
    واپس; // Nothing left.
  }

  بٹ_2.ڈالو_عقب(V);
}

خالی Folding_سیٹ_گرہ_ID::جوڑو_گرہ_ID(ثابت Folding_سیٹ_گرہ_ID &ID) {
  بٹ_2.append(ID.بٹ_2.begin(), ID.بٹ_2.انتہا());
}

Folding_سیٹ_گرہ_IDRef
Folding_سیٹ_گرہ_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  غیرعلامتی *New = Allocator.Allocate<غیرعلامتی>(بٹ_2.سائز_2());
  llvm::uninitialized_copy(بٹ_2, New);
  واپس Folding_سیٹ_گرہ_IDRef(New, بٹ_2.سائز_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_سیٹ_بنیاد::Folding_سیٹ_بنیاد(غیرعلامتی Log2_Init_سائز) {
  أكد(5 < Log2_Init_سائز && Log2_Init_سائز < 32 &&
         "Initial hash table size out of range");
  عدد_Buckets = 1 << Log2_Init_سائز;
  Buckets = static_cast<Folding_سیٹ_گرہ **>(
      محفوظ_calloc(عدد_Buckets, حجم(Folding_سیٹ_گرہ *)));
}

Folding_سیٹ_بنیاد::Folding_سیٹ_بنیاد(Folding_سیٹ_بنیاد &&Arg)
    : Buckets(std::exchange(Arg.Buckets, خالی_اشاریہ)),
      عدد_Buckets(std::exchange(Arg.عدد_Buckets, 0)),
      عدد_Nodes(std::exchange(Arg.عدد_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_سیٹ_بنیاد &Folding_سیٹ_بنیاد::عامل=(Folding_سیٹ_بنیاد &&RHS) {
  اگر (یہ == &RHS)
    واپس *یہ;

  incrementEpoch();
  RHS.incrementEpoch();
  آزاد(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, خالی_اشاریہ);
  عدد_Buckets = std::exchange(RHS.عدد_Buckets, 0);
  عدد_Nodes = std::exchange(RHS.عدد_Nodes, 0);
  واپس *یہ;
}

Folding_سیٹ_بنیاد::~Folding_سیٹ_بنیاد() { آزاد(Buckets); }

خالی Folding_سیٹ_بنیاد::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  اگر (عدد_Buckets)
    memset(Buckets, 0, عدد_Buckets * حجم(Folding_سیٹ_گرہ *));
  عدد_Nodes = 0;
}

خالی Folding_سیٹ_بنیاد::رکھو_گرہ(Folding_سیٹ_گرہ *N, uint32_t Hash) {
  غیرعلامتی نقاب = عدد_Buckets - 1;
  غیرعلامتی I = Hash & نقاب;
  جب (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & نقاب;
  }
  Buckets[I] = N;
  ++عدد_Nodes;
}

خالی Folding_سیٹ_بنیاد::grow(غیرعلامتی Min_عدد_Buckets) {
  // The floor is the smallest size the constructor accepts.
  غیرعلامتی New_بالٹی_گنتی = std::max(64u, llvm::ثنائی_ہندسہ_ceil(Min_عدد_Buckets));
  أكد(New_بالٹی_گنتی > عدد_Buckets && "Can't shrink a folding set");

  Folding_سیٹ_بنیاد Tmp(llvm::Log2_32(New_بالٹی_گنتی));
  برائے (غیرعلامتی I = 0; I != عدد_Buckets; ++I)
    اگر (Folding_سیٹ_گرہ *N = Buckets[I])
      Tmp.رکھو_گرہ(N, N->get_Folding_سیٹ_Hash());

  *یہ = std::move(Tmp);
}

خالی Folding_سیٹ_بنیاد::reserve(غیرعلامتی N) {
  اگر (N * 4 <= عدد_Buckets * 3)
    واپس;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

خالی Folding_سیٹ_بنیاد::داخل_کرو(Folding_سیٹ_گرہ *N, Folding_سیٹ_داخل_کرو_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  اگر (LLVM_UNLIKELY((عدد_Nodes + 1) * 4 > عدد_Buckets * 3))
    grow(عدد_Buckets * 2);
  uint32_t Hash = Token.Hash;
  رکھو_گرہ(N, Hash);
  N->سیٹ_Folding_سیٹ_Hash(Hash);
}

منطقی Folding_سیٹ_بنیاد::erase(Folding_سیٹ_گرہ *N) {
  uint32_t Hash = N->get_Folding_سیٹ_Hash();
  اگر (Hash == Folding_سیٹ_گرہ_IDRef::NotAHash)
    واپس جھوٹ; // Never inserted.

  غیرعلامتی نقاب = عدد_Buckets - 1;
  غیرعلامتی I = Hash & نقاب;
  جب (Buckets[I] != N) {
    اگر (LLVM_UNLIKELY(!Buckets[I]))
      واپس جھوٹ; // Not in folding set.
    I = (I + 1) & نقاب;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  برائے (غیرعلامتی J = (I + 1) & نقاب; Buckets[J]; J = (J + 1) & نقاب) {
    غیرعلامتی Ideal = Buckets[J]->get_Folding_سیٹ_Hash();
    اگر (((I - Ideal) & نقاب) < ((J - Ideal) & نقاب)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = خالی_اشاریہ;
  N->سیٹ_Folding_سیٹ_Hash(Folding_سیٹ_گرہ_IDRef::NotAHash);
  --عدد_Nodes;
  واپس سچ;
}
