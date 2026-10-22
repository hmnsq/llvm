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
کارول نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::چلوونکی<(Folding_سېټ_غوټه_IDRef LHS, Folding_سېټ_غوټه_IDRef RHS) {
  إذا (LHS.اندازه_2() != RHS.اندازه_2())
    أرجع LHS.اندازه_2() < RHS.اندازه_2();
  أرجع memcmp(LHS.معلومات(), RHS.معلومات(), LHS.اندازه_2() * حجم(بینښه)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ Folding_سېټ_غوټه_ID::جمع_کړه_String(StringRef String) {
  بینښه اندازه = String.اندازه_2();

  بینښه عدد_Inserts = 1 + divideCeil(اندازه, 4);
  بټونه_2.reserve(بټونه_2.اندازه_2() + عدد_Inserts);

  بټونه_2.ټېل_شا(اندازه);
  إذا (!اندازه)
    أرجع;

  بینښه Units = اندازه / 4;
  بینښه Pos = 0;
  ثابت بینښه *بنسټ = (ثابت بینښه *)String.معلومات();

  // If the string is aligned do a bulk transfer.
  إذا (!((intptr_t)بنسټ & 3)) {
    بټونه_2.append(بنسټ, بنسټ + Units);
    Pos = (Units + 1) * 4;
  } وإلا {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    إذا (sys::IsBigEndianHost) {
      لكل (Pos += 4; Pos <= اندازه; Pos += 4) {
        بینښه V = ((بینښه حرف)String[Pos - 4] << 24) |
                     ((بینښه حرف)String[Pos - 3] << 16) |
                     ((بینښه حرف)String[Pos - 2] << 8) |
                     (بینښه حرف)String[Pos - 1];
        بټونه_2.ټېل_شا(V);
      }
    } وإلا { // Little-endian host
      لكل (Pos += 4; Pos <= اندازه; Pos += 4) {
        بینښه V = ((بینښه حرف)String[Pos - 1] << 24) |
                     ((بینښه حرف)String[Pos - 2] << 16) |
                     ((بینښه حرف)String[Pos - 3] << 8) |
                     (بینښه حرف)String[Pos - 4];
        بټونه_2.ټېل_شا(V);
      }
    }
  }

  // With the leftover bits.
  بینښه V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - اندازه) {
  حالة 1:
    V = (V << 8) | (بینښه حرف)String[اندازه - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (بینښه حرف)String[اندازه - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (بینښه حرف)String[اندازه - 1];
    اكسر;
  افتراضي:
    أرجع; // Nothing left.
  }

  بټونه_2.ټېل_شا(V);
}

فراغ Folding_سېټ_غوټه_ID::جمع_کړه_غوټه_ID(ثابت Folding_سېټ_غوټه_ID &ID) {
  بټونه_2.append(ID.بټونه_2.begin(), ID.بټونه_2.پای_ټکی());
}

Folding_سېټ_غوټه_IDRef
Folding_سېټ_غوټه_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  بینښه *New = Allocator.Allocate<بینښه>(بټونه_2.اندازه_2());
  llvm::uninitialized_copy(بټونه_2, New);
  أرجع Folding_سېټ_غوټه_IDRef(New, بټونه_2.اندازه_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_سېټ_بنسټ::Folding_سېټ_بنسټ(بینښه Log2_Init_اندازه) {
  أكد(5 < Log2_Init_اندازه && Log2_Init_اندازه < 32 &&
         "Initial hash table size out of range");
  عدد_Buckets = 1 << Log2_Init_اندازه;
  Buckets = static_cast<Folding_سېټ_غوټه **>(
      خوندي_calloc(عدد_Buckets, حجم(Folding_سېټ_غوټه *)));
}

Folding_سېټ_بنسټ::Folding_سېټ_بنسټ(Folding_سېټ_بنسټ &&Arg)
    : Buckets(std::exchange(Arg.Buckets, تش_اشاره)),
      عدد_Buckets(std::exchange(Arg.عدد_Buckets, 0)),
      عدد_Nodes(std::exchange(Arg.عدد_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_سېټ_بنسټ &Folding_سېټ_بنسټ::چلوونکی=(Folding_سېټ_بنسټ &&RHS) {
  إذا (هذا == &RHS)
    أرجع *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, تش_اشاره);
  عدد_Buckets = std::exchange(RHS.عدد_Buckets, 0);
  عدد_Nodes = std::exchange(RHS.عدد_Nodes, 0);
  أرجع *هذا;
}

Folding_سېټ_بنسټ::~Folding_سېټ_بنسټ() { حرر(Buckets); }

فراغ Folding_سېټ_بنسټ::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  إذا (عدد_Buckets)
    memset(Buckets, 0, عدد_Buckets * حجم(Folding_سېټ_غوټه *));
  عدد_Nodes = 0;
}

فراغ Folding_سېټ_بنسټ::ځای_پرځای_کړه_غوټه(Folding_سېټ_غوټه *N, uint32_t Hash) {
  بینښه ماسک = عدد_Buckets - 1;
  بینښه I = Hash & ماسک;
  طالما (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & ماسک;
  }
  Buckets[I] = N;
  ++عدد_Nodes;
}

فراغ Folding_سېټ_بنسټ::grow(بینښه Min_عدد_Buckets) {
  // The floor is the smallest size the constructor accepts.
  بینښه New_لوښی_شمېر = std::max(64u, llvm::بټ_ceil(Min_عدد_Buckets));
  أكد(New_لوښی_شمېر > عدد_Buckets && "Can't shrink a folding set");

  Folding_سېټ_بنسټ Tmp(llvm::Log2_32(New_لوښی_شمېر));
  لكل (بینښه I = 0; I != عدد_Buckets; ++I)
    إذا (Folding_سېټ_غوټه *N = Buckets[I])
      Tmp.ځای_پرځای_کړه_غوټه(N, N->get_Folding_سېټ_Hash());

  *هذا = std::move(Tmp);
}

فراغ Folding_سېټ_بنسټ::reserve(بینښه N) {
  إذا (N * 4 <= عدد_Buckets * 3)
    أرجع;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ Folding_سېټ_بنسټ::ورننباسه(Folding_سېټ_غوټه *N, Folding_سېټ_ورننباسه_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  إذا (LLVM_UNLIKELY((عدد_Nodes + 1) * 4 > عدد_Buckets * 3))
    grow(عدد_Buckets * 2);
  uint32_t Hash = Token.Hash;
  ځای_پرځای_کړه_غوټه(N, Hash);
  N->سېټ_Folding_سېټ_Hash(Hash);
}

منطقي Folding_سېټ_بنسټ::erase(Folding_سېټ_غوټه *N) {
  uint32_t Hash = N->get_Folding_سېټ_Hash();
  إذا (Hash == Folding_سېټ_غوټه_IDRef::NotAHash)
    أرجع خطأ; // Never inserted.

  بینښه ماسک = عدد_Buckets - 1;
  بینښه I = Hash & ماسک;
  طالما (Buckets[I] != N) {
    إذا (LLVM_UNLIKELY(!Buckets[I]))
      أرجع خطأ; // Not in folding set.
    I = (I + 1) & ماسک;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  لكل (بینښه J = (I + 1) & ماسک; Buckets[J]; J = (J + 1) & ماسک) {
    بینښه Ideal = Buckets[J]->get_Folding_سېټ_Hash();
    إذا (((I - Ideal) & ماسک) < ((J - Ideal) & ماسک)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = تش_اشاره;
  N->سېټ_Folding_سېټ_Hash(Folding_سېټ_غوټه_IDRef::NotAHash);
  --عدد_Nodes;
  أرجع ریښتیا;
}
