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

#अन्तर्भाव "llvm/ADT/FoldingSet.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstring>
उपयुज्य नामाकाश llvm;

//===----------------------------------------------------------------------===//
// FoldingSetNodeIDRef Implementation

तार्किक llvm::संक्रियकः<(Folding_समुच्चयः_ग्रन्थिः_IDRef LHS, Folding_समुच्चयः_ग्रन्थिः_IDRef RHS) {
  यदि (LHS.आकार_2() != RHS.आकार_2())
    प्रत्यागम LHS.आकार_2() < RHS.आकार_2();
  प्रत्यागम memcmp(LHS.दत्त(), RHS.दत्त(), LHS.आकार_2() * परिमाण(अचिह्नित)) < 0;
}

//===----------------------------------------------------------------------===//
// FoldingSetNodeID Implementation

शून्य Folding_समुच्चयः_ग्रन्थिः_ID::सङ्कलय_String(StringRef String) {
  अचिह्नित आकार = String.आकार_2();

  अचिह्नित अङ्कः_Inserts = 1 + divideCeil(आकार, 4);
  द्वयङ्काः_2.reserve(द्वयङ्काः_2.आकार_2() + अङ्कः_Inserts);

  द्वयङ्काः_2.प्रक्षिप_पृष्ठभागः(आकार);
  यदि (!आकार)
    प्रत्यागम;

  अचिह्नित Units = आकार / 4;
  अचिह्नित Pos = 0;
  ध्रुव अचिह्नित *आधारः = (ध्रुव अचिह्नित *)String.दत्त();

  // If the string is aligned do a bulk transfer.
  यदि (!((intptr_t)आधारः & 3)) {
    द्वयङ्काः_2.append(आधारः, आधारः + Units);
    Pos = (Units + 1) * 4;
  } अन्यथा {
    // Otherwise do it the hard way.
    // To be compatible with above bulk transfer, we need to take endianness
    // into account.
    static_assert(sys::IsBigEndianHost || sys::IsLittleEndianHost,
                  "Unexpected host endianness");
    यदि (sys::IsBigEndianHost) {
      प्रति (Pos += 4; Pos <= आकार; Pos += 4) {
        अचिह्नित V = ((अचिह्नित वर्ण)String[Pos - 4] << 24) |
                     ((अचिह्नित वर्ण)String[Pos - 3] << 16) |
                     ((अचिह्नित वर्ण)String[Pos - 2] << 8) |
                     (अचिह्नित वर्ण)String[Pos - 1];
        द्वयङ्काः_2.प्रक्षिप_पृष्ठभागः(V);
      }
    } अन्यथा { // Little-endian host
      प्रति (Pos += 4; Pos <= आकार; Pos += 4) {
        अचिह्नित V = ((अचिह्नित वर्ण)String[Pos - 1] << 24) |
                     ((अचिह्नित वर्ण)String[Pos - 2] << 16) |
                     ((अचिह्नित वर्ण)String[Pos - 3] << 8) |
                     (अचिह्नित वर्ण)String[Pos - 4];
        द्वयङ्काः_2.प्रक्षिप_पृष्ठभागः(V);
      }
    }
  }

  // With the leftover bits.
  अचिह्नित V = 0;
  // Pos will have overshot size by 4 - #bytes left over.
  // No need to take endianness into account here - this is always executed.
  विकल्प (Pos - आकार) {
  पक्ष 1:
    V = (V << 8) | (अचिह्नित वर्ण)String[आकार - 3];
    [[fallthrough]];
  पक्ष 2:
    V = (V << 8) | (अचिह्नित वर्ण)String[आकार - 2];
    [[fallthrough]];
  पक्ष 3:
    V = (V << 8) | (अचिह्नित वर्ण)String[आकार - 1];
    भङ्ग;
  स्वभाव:
    प्रत्यागम; // Nothing left.
  }

  द्वयङ्काः_2.प्रक्षिप_पृष्ठभागः(V);
}

शून्य Folding_समुच्चयः_ग्रन्थिः_ID::सङ्कलय_ग्रन्थिः_ID(ध्रुव Folding_समुच्चयः_ग्रन्थिः_ID &ID) {
  द्वयङ्काः_2.append(ID.द्वयङ्काः_2.begin(), ID.द्वयङ्काः_2.अन्तः());
}

Folding_समुच्चयः_ग्रन्थिः_IDRef
Folding_समुच्चयः_ग्रन्थिः_ID::Intern(BumpPtrAllocator &Allocator) ध्रुव {
  अचिह्नित *New = Allocator.Allocate<अचिह्नित>(द्वयङ्काः_2.आकार_2());
  llvm::uninitialized_copy(द्वयङ्काः_2, New);
  प्रत्यागम Folding_समुच्चयः_ग्रन्थिः_IDRef(New, द्वयङ्काः_2.आकार_2());
}

//===----------------------------------------------------------------------===//
// FoldingSetBase Implementation

Folding_समुच्चयः_आधारः::Folding_समुच्चयः_आधारः(अचिह्नित Log2_Init_आकार) {
  प्रतिज्ञा(5 < Log2_Init_आकार && Log2_Init_आकार < 32 &&
         "Initial hash table size out of range");
  अङ्कः_Buckets = 1 << Log2_Init_आकार;
  Buckets = static_cast<Folding_समुच्चयः_ग्रन्थिः **>(
      safe_calloc(अङ्कः_Buckets, परिमाण(Folding_समुच्चयः_ग्रन्थिः *)));
}

Folding_समुच्चयः_आधारः::Folding_समुच्चयः_आधारः(Folding_समुच्चयः_आधारः &&Arg)
    : Buckets(std::exchange(Arg.Buckets, शून्यसूचकः)),
      अङ्कः_Buckets(std::exchange(Arg.अङ्कः_Buckets, 0)),
      अङ्कः_Nodes(std::exchange(Arg.अङ्कः_Nodes, 0)) {
  Arg.incrementEpoch();
}

Folding_समुच्चयः_आधारः &Folding_समुच्चयः_आधारः::संक्रियकः=(Folding_समुच्चयः_आधारः &&RHS) {
  यदि (एतत् == &RHS)
    प्रत्यागम *एतत्;

  incrementEpoch();
  RHS.incrementEpoch();
  मुक्त(Buckets); // This may be null if the set is in a moved-from state.
  Buckets = std::exchange(RHS.Buckets, शून्यसूचकः);
  अङ्कः_Buckets = std::exchange(RHS.अङ्कः_Buckets, 0);
  अङ्कः_Nodes = std::exchange(RHS.अङ्कः_Nodes, 0);
  प्रत्यागम *एतत्;
}

Folding_समुच्चयः_आधारः::~Folding_समुच्चयः_आधारः() { मुक्त(Buckets); }

शून्य Folding_समुच्चयः_आधारः::clear() {
  incrementEpoch();
  // Stale hashes are unreachable, so only the occupancy needs resetting.
  यदि (अङ्कः_Buckets)
    स्मृतिं_पूरय(Buckets, 0, अङ्कः_Buckets * परिमाण(Folding_समुच्चयः_ग्रन्थिः *));
  अङ्कः_Nodes = 0;
}

शून्य Folding_समुच्चयः_आधारः::place_ग्रन्थिः(Folding_समुच्चयः_ग्रन्थिः *N, uint32_t Hash) {
  अचिह्नित आवरणम् = अङ्कः_Buckets - 1;
  अचिह्नित I = Hash & आवरणम्;
  यावत् (Buckets[I]) {
    प्रतिज्ञा(Buckets[I] != N && "Node already in the folding set");
    I = (I + 1) & आवरणम्;
  }
  Buckets[I] = N;
  ++अङ्कः_Nodes;
}

शून्य Folding_समुच्चयः_आधारः::grow(अचिह्नित Min_अङ्कः_Buckets) {
  // The floor is the smallest size the constructor accepts.
  अचिह्नित New_Bucket_गणनम् = std::max(64u, llvm::bit_ceil(Min_अङ्कः_Buckets));
  प्रतिज्ञा(New_Bucket_गणनम् > अङ्कः_Buckets && "Can't shrink a folding set");

  Folding_समुच्चयः_आधारः Tmp(llvm::Log2_32(New_Bucket_गणनम्));
  प्रति (अचिह्नित I = 0; I != अङ्कः_Buckets; ++I)
    यदि (Folding_समुच्चयः_ग्रन्थिः *N = Buckets[I])
      Tmp.place_ग्रन्थिः(N, N->get_Folding_समुच्चयः_Hash());

  *एतत् = std::move(Tmp);
}

शून्य Folding_समुच्चयः_आधारः::reserve(अचिह्नित N) {
  यदि (N * 4 <= अङ्कः_Buckets * 3)
    प्रत्यागम;
  // N + (N + 2) / 3 is ceil(4N/3).
  grow(N + (N + 2) / 3);
}

शून्य Folding_समुच्चयः_आधारः::निवेशय(Folding_समुच्चयः_ग्रन्थिः *N, Folding_समुच्चयः_निवेशय_Token Token) {
  प्रतिज्ञा(N && "Cannot insert a null node");
  प्रतिज्ञा(Token && "Invalid token!");
  incrementEpoch();
  यदि (LLVM_UNLIKELY((अङ्कः_Nodes + 1) * 4 > अङ्कः_Buckets * 3))
    grow(अङ्कः_Buckets * 2);
  uint32_t Hash = Token.Hash;
  place_ग्रन्थिः(N, Hash);
  N->समुच्चयः_Folding_समुच्चयः_Hash(Hash);
}

तार्किक Folding_समुच्चयः_आधारः::erase(Folding_समुच्चयः_ग्रन्थिः *N) {
  uint32_t Hash = N->get_Folding_समुच्चयः_Hash();
  यदि (Hash == Folding_समुच्चयः_ग्रन्थिः_IDRef::NotAHash)
    प्रत्यागम असत्य; // Never inserted.

  अचिह्नित आवरणम् = अङ्कः_Buckets - 1;
  अचिह्नित I = Hash & आवरणम्;
  यावत् (Buckets[I] != N) {
    यदि (LLVM_UNLIKELY(!Buckets[I]))
      प्रत्यागम असत्य; // Not in folding set.
    I = (I + 1) & आवरणम्;
  }

  incrementEpoch();

  // Knuth TAOCP 6.4 Algorithm R: walk forward sliding each following entry
  // whose probe path crosses the hole.
  प्रति (अचिह्नित J = (I + 1) & आवरणम्; Buckets[J]; J = (J + 1) & आवरणम्) {
    अचिह्नित Ideal = Buckets[J]->get_Folding_समुच्चयः_Hash();
    यदि (((I - Ideal) & आवरणम्) < ((J - Ideal) & आवरणम्)) {
      Buckets[I] = Buckets[J];
      I = J;
    }
  }
  Buckets[I] = शून्यसूचकः;
  N->समुच्चयः_Folding_समुच्चयः_Hash(Folding_समुच्चयः_ग्रन्थिः_IDRef::NotAHash);
  --अङ्कः_Nodes;
  प्रत्यागम सत्य;
}
