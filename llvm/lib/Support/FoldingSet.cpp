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

#அந்தர்பாவ "llvm/ADT/FoldingSet.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Allocator.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/SwapByteOrder.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstring>
using நாமாகாஶ llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

தருக்க llvm::operator<(Folding_Set_முனை_IDRef LHS, Folding_Set_முனை_IDRef RHS) {
  எனில் (LHS.size() != RHS.size())
    திரும்பு LHS.size() < RHS.size();
  திரும்பு memcmp(LHS.data(), RHS.data(), LHS.size() * அளவு(unsigned)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

வெறுமை Folding_Set_முனை_ID::AddString(StringRef String) {
  unsigned Size = String.size();

  unsigned இலக்கம்_Inserts = 1 + divideCeil(Size, 4);
  Bits.reserve(Bits.size() + இலக்கம்_Inserts);

  Bits.தள்ளு_back(Size);
  எனில் (!Size)
    திரும்பு;

  unsigned Units = Size / 4;
  unsigned Pos = 0;
  மாறிலி unsigned *Base = (மாறிலி unsigned *)String.data();

  // If the string is aligned do a bulk transfer.
  எனில் (!((intptr_t)Base & 3)) {
    Bits.append(Base, Base + Units);
    Pos = (Units + 1) * 4;
  } இல்லையெனில் {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    எனில் (sys::IsBigEndianHost) {
      க்கு (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned எழுத்து)String[Pos - 4] << 24) |
                     ((unsigned எழுத்து)String[Pos - 3] << 16) |
                     ((unsigned எழுத்து)String[Pos - 2] << 8) |
                     (unsigned எழுத்து)String[Pos - 1];
        Bits.தள்ளு_back(V);
      }
    } இல்லையெனில் { // Little-endian host
      க்கு (Pos += 4; Pos <= Size; Pos += 4) {
        unsigned V = ((unsigned எழுத்து)String[Pos - 1] << 24) |
                     ((unsigned எழுத்து)String[Pos - 2] << 16) |
                     ((unsigned எழுத்து)String[Pos - 3] << 8) |
                     (unsigned எழுத்து)String[Pos - 4];
        Bits.தள்ளு_back(V);
      }
    }
  }

  // With the leftover bits.
  unsigned V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  தேர்வு (Pos - Size) {
  நிலை 1:
    V = (V << 8) | (unsigned எழுத்து)String[Size - 3];
    [[fallthrough]];
  நிலை 2:
    V = (V << 8) | (unsigned எழுத்து)String[Size - 2];
    [[fallthrough]];
  நிலை 3:
    V = (V << 8) | (unsigned எழுத்து)String[Size - 1];
    நிறுத்து;
  இயல்பு:
    திரும்பு; // Nothing left.
  }

  Bits.தள்ளு_back(V);
}

வெறுமை Folding_Set_முனை_ID::Add_முனை_ID(மாறிலி Folding_Set_முனை_ID &ID) {
  Bits.append(ID.Bits.begin(), ID.Bits.end());
}

Folding_Set_முனை_IDRef
Folding_Set_முனை_ID::Intern(BumpPtrAllocator &Allocator) மாறிலி {
  unsigned *New = Allocator.Allocate<unsigned>(Bits.size());
  llvm::uninitialized_copy(Bits, New);
  திரும்பு Folding_Set_முனை_IDRef(New, Bits.size());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

FoldingSetBase::FoldingSetBase(unsigned Log2InitSize) {
  ப்ரதிஜ்ஞா(5 < Log2InitSize && Log2InitSize < 32 &&
         "Initial hash table size out of range");
  இலக்கம்_Buckets = 1 << Log2InitSize;
  Buckets = static_cast<Folding_Set_முனை **>(
      safe_calloc(இலக்கம்_Buckets, அளவு(Folding_Set_முனை *)));
}

FoldingSetBase::FoldingSetBase(FoldingSetBase &&Arg)
    : Buckets(std::exchange(Arg.Buckets, nullptr)),
      இலக்கம்_Buckets(std::exchange(Arg.இலக்கம்_Buckets, 0)),
      இலக்கம்_Nodes(std::exchange(Arg.இலக்கம்_Nodes, 0)) {
  Arg.incrementEpoch();
}

FoldingSetBase &FoldingSetBase::operator=(FoldingSetBase &&RHS) {
  எனில் (இது == &RHS)
    திரும்பு *இது;

  incrementEpoch();
  RHS.incrementEpoch();
  விடுவி(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, nullptr);
  இலக்கம்_Buckets = std::exchange(RHS.இலக்கம்_Buckets, 0);
  இலக்கம்_Nodes = std::exchange(RHS.இலக்கம்_Nodes, 0);
  திரும்பு *இது;
}

FoldingSetBase::~FoldingSetBase() { விடுவி(Buckets); }

வெறுமை FoldingSetBase::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  எனில் (இலக்கம்_Buckets)
    memset(Buckets, 0, இலக்கம்_Buckets * அளவு(Folding_Set_முனை *));
  இலக்கம்_Nodes = 0;
}

வெறுமை FoldingSetBase::place_முனை(Folding_Set_முனை *N, uint32_t Hash) {
  unsigned Mask = இலக்கம்_Buckets - 1;
  unsigned I = Hash & Mask;
  போது (Buckets[I]) {
    ப்ரதிஜ்ஞா(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & Mask;
  }
  Buckets[I] = N;
  ++இலக்கம்_Nodes;
}

வெறுமை FoldingSetBase::grow(unsigned Min_இலக்கம்_Buckets) {
  // The floor is the smallest size the constructor accepts.
  unsigned New_Bucket_மொத்தம் = std::max(64u, llvm::bit_ceil(Min_இலக்கம்_Buckets));
  ப்ரதிஜ்ஞா(New_Bucket_மொத்தம் > இலக்கம்_Buckets && "Can't shrink a folding set");

  FoldingSetBase Tmp(llvm::Log2_32(New_Bucket_மொத்தம்));
  க்கு (unsigned I = 0; I != இலக்கம்_Buckets; ++I)
    எனில் (Folding_Set_முனை *N = Buckets[I])
      Tmp.place_முனை(N, N->getFoldingSetHash());

  *இது = std::move(Tmp);
}

வெறுமை FoldingSetBase::reserve(unsigned N) {
  எனில் (N * 4 <= இலக்கம்_Buckets * 3)
    திரும்பு;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

வெறுமை FoldingSetBase::செருகு(Folding_Set_முனை *N, Folding_Set_செருகு_Token Token) {
  ப்ரதிஜ்ஞா(N && "Cannot insert a null node");
  ப்ரதிஜ்ஞா(Token && "Invalid token!");
  incrementEpoch();
  எனில் (LLVM_UNLIKELY((இலக்கம்_Nodes + 1) * 4 > இலக்கம்_Buckets * 3))
    grow(இலக்கம்_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_முனை(N, Hash);
  N->setFoldingSetHash(Hash);
}

தருக்க FoldingSetBase::erase(Folding_Set_முனை *N) {
  uint32_t Hash = N->getFoldingSetHash();
  எனில் (Hash == Folding_Set_முனை_IDRef::NotAHash)
    திரும்பு பொய்; // Never inserted.

  unsigned Mask = இலக்கம்_Buckets - 1;
  unsigned I = Hash & Mask;
  போது (Buckets[I] != N) {
    எனில் (LLVM_UNLIKELY(!Buckets[I]))
      திரும்பு பொய்; // Not in folding set.
    I = (I + 1) & Mask;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  க்கு (unsigned J = (I + 1) & Mask; Buckets[J]; J = (J + 1) & Mask) {
    unsigned Ideal = Buckets[J]->getFoldingSetHash();
    எனில் (((I - Ideal) & Mask) < ((J - Ideal) & Mask)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = nullptr;
  N->setFoldingSetHash(Folding_Set_முனை_IDRef::NotAHash);
  --இலக்கம்_Nodes;
  திரும்பு உண்மை;
}
