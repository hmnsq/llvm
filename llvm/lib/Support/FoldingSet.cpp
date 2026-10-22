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
using نطاق llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقي llvm::operator<(Folding_Set_عقدة_IDRef LHS, Folding_Set_عقدة_IDRef RHS) {
  إذا (LHS.size() != RHS.size())
    إرجاع LHS.size() < RHS.size();
  إرجاع memcmp(LHS.data(), RHS.data(), LHS.size() * حجم(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

فراغ Folding_Set_عقدة_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned رقم_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + رقم_Inserts);

  Bits.ادفع_back(Size);
  إذا (!Size)
    إرجاع;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ثابت unsigned *Base = (ثابت unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  إذا (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } وإلا {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    إذا (sys::IsBigEndianHost) {
      لكل (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned حرف)String[Pos - 4] << 24) |
                     ((unsigned حرف)String[Pos - 3] << 16) |
                     ((unsigned حرف)String[Pos - 2] << 8) |
                     (unsigned حرف)String[Pos - 1];
        Bits.ادفع_back(V);
      }
    } وإلا { // Little-endian host
      لكل (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned حرف)String[Pos - 1] << 24) |
                     ((unsigned حرف)String[Pos - 2] << 16) |
                     ((unsigned حرف)String[Pos - 3] << 8) |
                     (unsigned حرف)String[Pos - 4];
        Bits.ادفع_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  بدل (Pos - Size) {
  حالة 1:
    V = (V << 8) | (unsigned حرف)String[Size - 3];
    [[fallthrough]];
  حالة 2:
    V = (V << 8) | (unsigned حرف)String[Size - 2];
    [[fallthrough]];
  حالة 3:
    V = (V << 8) | (unsigned حرف)String[Size - 1];
    اكسر;
  افتراضي:
    إرجاع; // Nothing left.
  }

  Bits.ادفع_back(V);
}

فراغ Folding_Set_عقدة_ID::Add_عقدة_ID(ثابت Folding_Set_عقدة_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_عقدة_IDRef
Folding_Set_عقدة_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  إرجاع Folding_Set_عقدة_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  أكد(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  رقم_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_عقدة **>(
      safe_calloc(رقم_Buckets, حجم(Folding_Set_عقدة *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      رقم_Buckets(std::exchange(Arg.رقم_Buckets, 0)),
      رقم_Nodes(std::exchange(Arg.رقم_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  إذا (هذا == &RHS)
    إرجاع *هذا;

  incrementEpoch();
  RHS.incrementEpoch();
  حرر(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  رقم_Buckets = std::exchange(RHS.رقم_Buckets, 0);
  رقم_Nodes = std::exchange(RHS.رقم_Nodes, 0);
  إرجاع *هذا;
}

FoldingSetBase::~FoldingSetBase() { حرر(Buckets); }

فراغ FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  إذا (رقم_Buckets)
    memset(Buckets, 0, رقم_Buckets * حجم(Folding_Set_عقدة *));
  رقم_Nodes = 0;
}

فراغ FoldingSetBase::place_عقدة(Folding_Set_عقدة *N, uint32_t Hash) {
  unsigned Mask = رقم_Buckets - 1;
  unsigned I = Hash & Mask;
  بينما (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++رقم_Nodes;
}

فراغ FoldingSetBase::grow(unsigned Min_رقم_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_تعداد = std::max(64u, llvm::bit_ceil(Min_رقم_Buckets));
  أكد(New_Bucket_تعداد > رقم_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_تعداد));
  لكل (unsigned I = 0; I != رقم_Buckets; ++I)
    إذا (Folding_Set_عقدة *N = Buckets[I])
      Tmp.place_عقدة(N, N->getFoldingSetHash());

  *هذا = std::move(Tmp);
}

فراغ FoldingSetBase::reserve(unsigned N) {
  إذا (N * 4 <= رقم_Buckets * 3)
    إرجاع;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

فراغ FoldingSetBase::أدرج(Folding_Set_عقدة *N, Folding_Set_أدرج_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  إذا (LLVM_UNLIKELY((رقم_Nodes + 1) * 4 > رقم_Buckets * 3))
    grow(رقم_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_عقدة(N, Hash);
  N->setFoldingSetHash(Hash);
}

منطقي FoldingSetBase::erase(Folding_Set_عقدة *N) {
  uint32_t Hash = N->getFoldingSetHash();
  إذا (Hash == Folding_Set_عقدة_IDRef::NotAHash)
    إرجاع خطأ; // Never inserted.

  unsigned Mask = رقم_Buckets - 1;
  unsigned I = Hash & Mask;
  بينما (Buckets[I] != N) {
    إذا (LLVM_UNLIKELY(!Buckets[I]))
      إرجاع خطأ; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  لكل (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    إذا (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_عقدة_IDRef::NotAHash);
  --رقم_Nodes;
  إرجاع true;
}
