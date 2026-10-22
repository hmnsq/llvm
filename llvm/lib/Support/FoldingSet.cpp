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
using فضاینام llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

منطقی llvm::operator<(Folding_Set_گره_IDRef LHS, Folding_Set_گره_IDRef RHS) {
  اگر (LHS.size() != RHS.size())
    بازگشت LHS.size() < RHS.size();
  بازگشت memcmp(LHS.data(), RHS.data(), LHS.size() * اندازه(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

تهی Folding_Set_گره_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned رقم_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + رقم_Inserts);

  Bits.افزودن_back(Size);
  اگر (!Size)
    بازگشت;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  ثابت unsigned *Base = (ثابت unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  اگر (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } وگرنه {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    اگر (sys::IsBigEndianHost) {
      برای (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned نویسه)String[Pos - 4] << 24) |
                     ((unsigned نویسه)String[Pos - 3] << 16) |
                     ((unsigned نویسه)String[Pos - 2] << 8) |
                     (unsigned نویسه)String[Pos - 1];
        Bits.افزودن_back(V);
      }
    } وگرنه { // Little-endian host
      برای (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned نویسه)String[Pos - 1] << 24) |
                     ((unsigned نویسه)String[Pos - 2] << 16) |
                     ((unsigned نویسه)String[Pos - 3] << 8) |
                     (unsigned نویسه)String[Pos - 4];
        Bits.افزودن_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  انتخاب (Pos - Size) {
  حالت 1:
    V = (V << 8) | (unsigned نویسه)String[Size - 3];
    [[fallthrough]];
  حالت 2:
    V = (V << 8) | (unsigned نویسه)String[Size - 2];
    [[fallthrough]];
  حالت 3:
    V = (V << 8) | (unsigned نویسه)String[Size - 1];
    بشکن;
  پیشفرض:
    بازگشت; // Nothing left.
  }

  Bits.افزودن_back(V);
}

تهی Folding_Set_گره_ID::Add_گره_ID(ثابت Folding_Set_گره_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_گره_IDRef
Folding_Set_گره_ID::Intern(BumpPtrAllocator &Allocator) ثابت {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  بازگشت Folding_Set_گره_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  أكد(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  رقم_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_گره **>(
      safe_calloc(رقم_Buckets, اندازه(Folding_Set_گره *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      رقم_Buckets(std::exchange(Arg.رقم_Buckets, 0)),
      رقم_Nodes(std::exchange(Arg.رقم_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  اگر (این == &RHS)
    بازگشت *این;

  incrementEpoch();
  RHS.incrementEpoch();
  آزاد(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  رقم_Buckets = std::exchange(RHS.رقم_Buckets, 0);
  رقم_Nodes = std::exchange(RHS.رقم_Nodes, 0);
  بازگشت *این;
}

FoldingSetBase::~FoldingSetBase() { آزاد(Buckets); }

تهی FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  اگر (رقم_Buckets)
    memset(Buckets, 0, رقم_Buckets * اندازه(Folding_Set_گره *));
  رقم_Nodes = 0;
}

تهی FoldingSetBase::place_گره(Folding_Set_گره *N, uint32_t Hash) {
  unsigned Mask = رقم_Buckets - 1;
  unsigned I = Hash & Mask;
  وقتی (Buckets[I]) {
    أكد(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++رقم_Nodes;
}

تهی FoldingSetBase::grow(unsigned Min_رقم_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_شمار = std::max(64u, llvm::bit_ceil(Min_رقم_Buckets));
  أكد(New_Bucket_شمار > رقم_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_شمار));
  برای (unsigned I = 0; I != رقم_Buckets; ++I)
    اگر (Folding_Set_گره *N = Buckets[I])
      Tmp.place_گره(N, N->getFoldingSetHash());

  *این = std::move(Tmp);
}

تهی FoldingSetBase::reserve(unsigned N) {
  اگر (N * 4 <= رقم_Buckets * 3)
    بازگشت;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

تهی FoldingSetBase::درج(Folding_Set_گره *N, Folding_Set_درج_Token Token) {
  أكد(N && "Cannot insert a null node");
  أكد(Token && "Invalid token!");
  incrementEpoch();
  اگر (LLVM_UNLIKELY((رقم_Nodes + 1) * 4 > رقم_Buckets * 3))
    grow(رقم_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_گره(N, Hash);
  N->setFoldingSetHash(Hash);
}

منطقی FoldingSetBase::erase(Folding_Set_گره *N) {
  uint32_t Hash = N->getFoldingSetHash();
  اگر (Hash == Folding_Set_گره_IDRef::NotAHash)
    بازگشت نادرست; // Never inserted.

  unsigned Mask = رقم_Buckets - 1;
  unsigned I = Hash & Mask;
  وقتی (Buckets[I] != N) {
    اگر (LLVM_UNLIKELY(!Buckets[I]))
      بازگشت نادرست; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  برای (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    اگر (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_گره_IDRef::NotAHash);
  --رقم_Nodes;
  بازگشت درست;
}
