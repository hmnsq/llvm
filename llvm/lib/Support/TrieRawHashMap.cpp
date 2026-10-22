//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/TrieRawHashMap.h"
#अन्तर्भाव "llvm/ADT/LazyAtomicPointer.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/TrieHashIndexGenerator.h"
#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/ThreadSafeAllocator.h"
#अन्तर्भाव "llvm/Support/TrailingObjects.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <memory>

using नामाकाश llvm;

नामाकाश {
संरचना Trie_ग्रन्थिः {
  ध्रुव तार्किक IsSubtrie = मिच्छा;

  Trie_ग्रन्थिः(तार्किक IsSubtrie) : IsSubtrie(IsSubtrie) {}

  स्थिर शून्य *operator नूतन(size_t Size) { पटिनिवत्त ::operator नूतन(Size); }
  शून्य operator delete(शून्य *Ptr) { ::operator delete(Ptr); }
};

संरचना TrieContent final : सार्वजनिक Trie_ग्रन्थिः {
  ध्रुव uint8_t ContentOffset;
  ध्रुव uint8_t HashSize;
  ध्रुव uint8_t HashOffset;

  शून्य *get_मानम्_Pointer() ध्रुव {
    auto *Content = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + ContentOffset;
    पटिनिवत्त const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ध्रुव {
    auto *Begin = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + HashOffset;
    पटिनिवत्त ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_ग्रन्थिः(/*IsSubtrie=*/मिच्छा), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  स्थिर तार्किक classof(ध्रुव Trie_ग्रन्थिः *TN) { पटिनिवत्त !TN->IsSubtrie; }
};

static_assert(परिमाण(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

वग्ग TrieSubtrie final
    : सार्वजनिक Trie_ग्रन्थिः,
      गुप्त TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_ग्रन्थिः>> {
सार्वजनिक:
  using Slot = LazyAtomicPointer<Trie_ग्रन्थिः>;

  Slot &get(size_t I) { पटिनिवत्त getTrailingObjects()[I]; }
  Trie_ग्रन्थिः *load(size_t I) { पटिनिवत्त get(I).load(); }

  unsigned size() ध्रुव { पटिनिवत्त Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t अङ्कः_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  स्थिर std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t अङ्कः_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t अङ्कः_Bits_2);

  स्थिर तार्किक classof(ध्रुव Trie_ग्रन्थिः *TN) { पटिनिवत्त TN->IsSubtrie; }

  स्थिर constexpr size_t sizeToAlloc(unsigned अङ्कः_Bits_2) {
    प्रतिज्ञा(अङ्कः_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned गणनम्_2 = 1u << अङ्कः_Bits_2;
    पटिनिवत्त totalSizeToAlloc<LazyAtomicPointer<Trie_ग्रन्थिः>>(गणनम्_2);
  }

गुप्त:
  // FIXME: Use a bitset to speed up access:
  //
  //     std::array<std::atomic<uint64_t>, NumSlots/64> IsSet;
  //
  // This will avoid needing to visit sparsely filled slots in
  // \a ThreadSafeTrieRawHashMapBase::destroyImpl() when there's a non-trivial
  // destructor.
  //
  // It would also greatly speed up iteration, if we add that some day, and
  // allow get() to return one level sooner.
  //
  // This would be the algorithm for updating IsSet (after updating Slots):
  //
  //     std::atomic<uint64_t> &Bits = IsSet[I.High];
  //     const uint64_t NewBit = 1ULL << I.Low;
  //     uint64_t Old = 0;
  //     while (!Bits.compare_exchange_weak(Old, Old | NewBit))
  //       ;

  // For debugging.
  unsigned StartBit = 0;
  unsigned अङ्कः_Bits_2 = 0;
  unsigned Size = 0;
  friend वग्ग llvm::ThreadSafeTrieRawHashMapBase;
  friend वग्ग TrailingObjects;

सार्वजनिक:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t अङ्कः_Bits_2) {
  शून्य *Memory = ::operator नूतन(sizeToAlloc(अङ्कः_Bits_2));
  TrieSubtrie *S = ::नूतन (Memory) TrieSubtrie(StartBit, अङ्कः_Bits_2);
  पटिनिवत्त std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t अङ्कः_Bits_2)
    : Trie_ग्रन्थिः(सच्च), StartBit(StartBit), अङ्कः_Bits_2(अङ्कः_Bits_2), Size(1u << अङ्कः_Bits_2),
      Next(nullptr) {
  अत्थाय (unsigned I = 0; I < Size; ++I)
    नूतन (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_ग्रन्थिः>>::मानम्_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t अङ्कः_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  प्रतिज्ञा(अङ्कः_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + अङ्कः_Bits_2, अङ्कः_Subtrie_Bits);

  प्रतिज्ञा(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ग्रन्थिः *Existing_ग्रन्थिः = &Content;
  प्रतिज्ञा(I < Size);
  सचे (get(I).compare_exchange_strong(Existing_ग्रन्थिः, S.get()))
    पटिनिवत्त Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  पटिनिवत्त cast<TrieSubtrie>(Existing_ग्रन्थिः);
}

वग्ग ThreadSafeTrieRawHashMapBase::ImplType final
    : गुप्त TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
सार्वजनिक:
  स्थिर std::unique_ptr<ImplType> create(size_t StartBit, size_t अङ्कः_Bits_2) {
    size_t Size = परिमाण(ImplType) + TrieSubtrie::sizeToAlloc(अङ्कः_Bits_2);
    शून्य *Memory = ::operator नूतन(Size);
    ImplType *Impl = ::नूतन (Memory) ImplType(StartBit, अङ्कः_Bits_2);
    पटिनिवत्त std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    प्रतिज्ञा(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    याव (!get_मूलम्()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    पटिनिवत्त S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_मूलम्() { पटिनिवत्त getTrailingObjects(); }

  स्थिर शून्य *operator नूतन(size_t Size) { पटिनिवत्त ::operator नूतन(Size); }
  शून्य operator delete(शून्य *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

गुप्त:
  friend वग्ग TrailingObjects;

  ImplType(size_t StartBit, size_t अङ्कः_Bits_2) {
    ::नूतन (get_मूलम्()) TrieSubtrie(StartBit, अङ्कः_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  सचे (ImplType *Impl = ImplPtr.load())
    पटिनिवत्त *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अङ्कः_मूलम्_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  सचे (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    पटिनिवत्त *Impl.release();

  // Already created, return the existing ImplType.
  पटिनिवत्त *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ध्रुव {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  सचे (!Impl)
    पटिनिवत्त PointerBase();

  TrieSubtrie *S = Impl->get_मूलम्();
  TrieHashIndexGenerator IndexGen{अङ्कः_मूलम्_Bits, अङ्कः_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  याव (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_ग्रन्थिः *Existing = S->get(Index);
    सचे (!Existing)
      पटिनिवत्त PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    सचे (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      पटिनिवत्त ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_मानम्_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::निवेशय(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ध्रुव uint8_t *(शून्य *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_मूलम्();
  TrieHashIndexGenerator IndexGen{अङ्कः_मूलम्_Bits, अङ्कः_Subtrie_Bits, Hash};
  size_t Index;
  सचे (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } अञ्ञथा {
    Index = IndexGen.next();
  }

  याव (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    तार्किक Generated = मिच्छा;
    Trie_ग्रन्थिः &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = सच्च;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ध्रुव uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नूतन (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      प्रतिज्ञा(Hash == Content->getHash() && "Hash not properly initialized");
      पटिनिवत्त Content;
    });
    // If we just generated it, return it!
    सचे (Generated)
      पटिनिवत्त PointerBase(cast<TrieContent>(Existing).get_मानम्_Pointer());

    सचे (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      अनुवर्त;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    सचे (ExistingContent.getHash() == Hash)
      पटिनिवत्त PointerBase(ExistingContent.get_मानम्_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    याव (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_अङ्कः_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    पटिनिवत्त Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      सचे (NextIndex != NewIndexForExistingContent)
        भङ्ग;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> अङ्कः_मूलम्_Bits, std::optional<size_t> अङ्कः_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      अङ्कः_मूलम्_Bits(अङ्कः_मूलम्_Bits.मानम्_or(Default_अङ्कः_मूलम्_Bits)),
      अङ्कः_Subtrie_Bits(अङ्कः_Subtrie_Bits.मानम्_or(Default_अङ्कः_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  प्रतिज्ञा((!अङ्कः_मूलम्_Bits || *अङ्कः_मूलम्_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  प्रतिज्ञा((!अङ्कः_Subtrie_Bits || *अङ्कः_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), अङ्कः_मूलम्_Bits(RHS.अङ्कः_मूलम्_Bits),
      अङ्कः_Subtrie_Bits(RHS.अङ्कः_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  प्रतिज्ञा(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

शून्य ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<शून्य(शून्य *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  सचे (!Impl)
    पटिनिवत्त;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  सचे (Destructor)
    अत्थाय (TrieSubtrie *Trie = Impl->get_मूलम्(); Trie; Trie = Trie->Next.load())
      अत्थाय (unsigned I = 0; I < Trie->size(); ++I)
        सचे (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_मानम्_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_मूलम्()->Next;
  याव (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_मूलम्() ध्रुव {
  ImplType *Impl = ImplPtr.load();
  सचे (!Impl)
    पटिनिवत्त PointerBase();
  पटिनिवत्त PointerBase(Impl->get_मूलम्());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  सचे (!P.P)
    पटिनिवत्त 0;
  सचे (auto *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P))
    पटिनिवत्त S->StartBit;
  पटिनिवत्त 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_अङ्कः_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  सचे (!P.P)
    पटिनिवत्त 0;
  सचे (auto *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P))
    पटिनिवत्त S->अङ्कः_Bits_2;
  पटिनिवत्त 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_अङ्कः_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  सचे (!P.P)
    पटिनिवत्त 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  सचे (!S)
    पटिनिवत्त 0;
  unsigned अङ्कः = 0;
  अत्थाय (unsigned I = 0, E = S->size(); I < E; ++I)
    सचे (S->load(I))
      ++अङ्कः;
  पटिनिवत्त अङ्कः;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  सचे (!P.P)
    पटिनिवत्त "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  सचे (!S || !S->IsSubtrie)
    पटिनिवत्त "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ग्रन्थिः = nullptr;
  याव (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    अत्थाय (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      सचे (!S)
        अनुवर्त;

      सचे (auto *Content = dyn_cast<TrieContent>(S))
        ग्रन्थिः = Content;
      अञ्ञथा सचे (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      भङ्ग;
    }

    // Found the node.
    सचे (ग्रन्थिः)
      भङ्ग;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  प्रतिज्ञा(ग्रन्थिः && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(ग्रन्थिः->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/सच्च);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  अत्थाय (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.प्रक्षिप_back('0' + ((ग्रन्थिः->getHash()[Index] >> Offset) & 1));
  }

  सचे (!Bits.empty())
    SS << "[" << Bits << "]";

  पटिनिवत्त SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_अङ्कः_Tries() ध्रुव {
  ImplType *Impl = ImplPtr.load();
  सचे (!Impl)
    पटिनिवत्त 0;
  unsigned अङ्कः = 0;
  अत्थाय (TrieSubtrie *Trie = Impl->get_मूलम्(); Trie; Trie = Trie->Next.load())
    ++अङ्कः;
  पटिनिवत्त अङ्कः;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  सचे (!P.P)
    पटिनिवत्त PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  सचे (!S)
    पटिनिवत्त PointerBase();
  सचे (auto *E = S->Next.load())
    पटिनिवत्त PointerBase(E);
  पटिनिवत्त PointerBase();
}
