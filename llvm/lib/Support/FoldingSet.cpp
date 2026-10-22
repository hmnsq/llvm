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
استفاده نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::عملگر<(Folding_مجموعه_گره_IDRef LHS, Folding_مجموعه_گره_IDRef RHS) {
  إذا (LHS.اندازه_2() != RHS.اندازه_2())
    أرجع LHS.اندازه_2() < RHS.اندازه_2();
  أرجع memcmp(LHS.داده(), RHS.داده(), LHS.اندازه_2() * حجم(بیعلامت)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ Folding_مجموعه_گره_ID::جمع_کن_String(StringRef String) {
  بیعلامت اندازه = String.اندازه_2();

  بیعلامت رقم_Inserts = 1 + divideCeil(اندازه, 4);
  بیتها_2.reserve(بیتها_2.اندازه_2() + رقم_Inserts);

  بیتها_2.افزودن_پشت(اندازه);
  إذا (!اندازه)
    أرجع;

  بیعلامت Units = اندازه / 4;
  بیعلامت Pos = 0;
  ثابت بیعلامت *پایه = (ثابت بیعلامت *)String.داده();

  // If the string is aligned do a bulk transfer.
  إذا (!((intptr_t)پایه & 3)) {
    بیتها_2.append(پایه, پایه + Units);
    Pos = (Units + 1) * 4;
  } وإلا {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    إذا (sys::IsBigEndianHost) {
      لكل (Pos += 4; Pos <= اندازه; Pos += 4) {
        بیعلامت V = ((بیعلامت حرف)String[Pos - 4] << 24) |
                     ((بیعلامت حرف)String[Pos - 3] << 16) |
                     ((بیعلامت حرف)String[Pos - 2] << 8) |
                     (بیعلامت حرف)String[Pos - 1];
        بیتها_2.افزودن_پشت(V);
      }
    } وإلا { // Little-endian host
      لكل (Pos += 4; Pos <= اندازه; Pos += 4) {
        بیعلامت V = ((بیعلامت حرف)String[Pos - 1] << 24) |
                     ((بیعلامت حرف)String[Pos - 2] << 16) |
                     ((بیعلامت حرف)String[Pos - 3] << 8) |
                     (بیعلامت حرف)String[Pos - 4];
        بیتها_2.افزودن_پشت(V);
      }
    }
  }

  // With the leftover bits.
  بیعلامت V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - اندازه) {
  حالة 1:
    V = (V << 8) | (بیعلامت حرف)String[اندازه - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (بیعلامت حرف)String[اندازه - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (بیعلامت حرف)String[اندازه - 1];
    اكسر;
  افتراضي:
    أرجع; // Nothing left.
  }

  بیتها_2.افزودن_پشت(V);
}

فراغ Folding_مجموعه_گره_ID::جمع_کن_گره_ID(ثابت Folding_مجموعه_گره_ID &ID) {
  بیتها_2.append(ID.بیتها_2.begin(), ID.بیتها_2.انتها());
}

Folding_مجموعه_گره_IDRef
Folding_مجموعه_گره_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  بیعلامت *New = Allocator.Allocate<بیعلامت>(بیتها_2.اندازه_2());
  llvm::uninitialized_copy(بیتها_2, New);
  أرجع Folding_مجموعه_گره_IDRef(New, بیتها_2.اندازه_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_مجموعه_پایه::Folding_مجموعه_پایه(بیعلامت Log2_Init_اندازه) {
  أكد(5 < Log2_Init_اندازه && Log2_Init_اندازه < 32 &&
         "Initial hash table size out of range");
  رقم_Buckets = 1 << Log2_Init_اندازه;
  Buckets = static_cast<Folding_مجموعه_گره **>(
      امن_calloc(رقم_Buckets, حجم(Folding_مجموعه_گره *)));
}

Folding_مجموعه_پایه::Folding_مجموعه_پایه(Folding_مجموعه_پایه &&Arg)
    : Buckets(std::exchange(Arg.Buckets, اشاره_تهی)),
      رقم_Buckets(std::exchange(Arg.رقم_Buckets, 0)),
      رقم_Nodes(std::exchange(Arg.رقم_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_مجموعه_پایه &Folding_مجموعه_پایه::عملگر=(Folding_مجموعه_پایه &&RHS) {
  إذا (هذا == &RHS)
    أرجع *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, اشاره_تهی);
  رقم_Buckets = std::exchange(RHS.رقم_Buckets, 0);
  رقم_Nodes = std::exchange(RHS.رقم_Nodes, 0);
  أرجع *هذا;
}

Folding_مجموعه_پایه::~Folding_مجموعه_پایه() { حرر(Buckets); }

فراغ Folding_مجموعه_پایه::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  إذا (رقم_Buckets)
    memset(Buckets, 0, رقم_Buckets * حجم(Folding_مجموعه_گره *));
  رقم_Nodes = 0;
}

فراغ Folding_مجموعه_پایه::قراربده_گره(Folding_مجموعه_گره *N, uint32_t Hash) {
  بیعلامت ماسک = رقم_Buckets - 1;
  بیعلامت I = Hash & ماسک;
  طالما (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & ماسک;
  }
  Buckets[I] = N;
  ++رقم_Nodes;
}

فراغ Folding_مجموعه_پایه::grow(بیعلامت Min_رقم_Buckets) {
  // The floor is the smallest size the constructor accepts.
  بیعلامت New_سطل_شمار = std::max(64u, llvm::بیت_ceil(Min_رقم_Buckets));
  أكد(New_سطل_شمار > رقم_Buckets && "Can't shrink a folding set");

  Folding_مجموعه_پایه Tmp(llvm::Log2_32(New_سطل_شمار));
  لكل (بیعلامت I = 0; I != رقم_Buckets; ++I)
    إذا (Folding_مجموعه_گره *N = Buckets[I])
      Tmp.قراربده_گره(N, N->get_Folding_مجموعه_Hash());

  *هذا = std::move(Tmp);
}

فراغ Folding_مجموعه_پایه::reserve(بیعلامت N) {
  إذا (N * 4 <= رقم_Buckets * 3)
    أرجع;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ Folding_مجموعه_پایه::درج(Folding_مجموعه_گره *N, Folding_مجموعه_درج_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  إذا (LLVM_UNLIKELY((رقم_Nodes + 1) * 4 > رقم_Buckets * 3))
    grow(رقم_Buckets * 2);
  uint32_t Hash = Token.Hash;
  قراربده_گره(N, Hash);
  N->مجموعه_Folding_مجموعه_Hash(Hash);
}

منطقي Folding_مجموعه_پایه::erase(Folding_مجموعه_گره *N) {
  uint32_t Hash = N->get_Folding_مجموعه_Hash();
  إذا (Hash == Folding_مجموعه_گره_IDRef::NotAHash)
    أرجع خطأ; // Never inserted.

  بیعلامت ماسک = رقم_Buckets - 1;
  بیعلامت I = Hash & ماسک;
  طالما (Buckets[I] != N) {
    إذا (LLVM_UNLIKELY(!Buckets[I]))
      أرجع خطأ; // Not in folding set.
    I = (I + 1) & ماسک;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  لكل (بیعلامت J = (I + 1) & ماسک; Buckets[J]; J = (J + 1) & ماسک) {
    بیعلامت Ideal = Buckets[J]->get_Folding_مجموعه_Hash();
    إذا (((I - Ideal) & ماسک) < ((J - Ideal) & ماسک)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = اشاره_تهی;
  N->مجموعه_Folding_مجموعه_Hash(Folding_مجموعه_گره_IDRef::NotAHash);
  --رقم_Nodes;
  أرجع درست;
}
