//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/TrieRawHashMap.h"
#ضمن "llvm/ADT/LazyAtomicPointer.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/TrieHashIndexGenerator.h"
#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/ThreadSafeAllocator.h"
#ضمن "llvm/Support/TrailingObjects.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <memory>

using نطاق llvm;

نطاق {
بنية TrieNode {
  ثابت منطقي IsSubtrie = ܕܓܠܐ;

  TrieNode(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *operator جديد(size_t Size) { ܦܢܐ ::operator جديد(Size); }
  فراغ operator delete(فراغ *Ptr) { ::operator delete(Ptr); }
};

بنية TrieContent final : عام TrieNode {
  ثابت uint8_t ContentOffset;
  ثابت uint8_t HashSize;
  ثابت uint8_t HashOffset;

  فراغ *getValuePointer() ثابت {
    auto *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + ContentOffset;
    ܦܢܐ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    auto *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + HashOffset;
    ܦܢܐ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : TrieNode(/*IsSubtrie=*/ܕܓܠܐ), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  ساكن منطقي classof(ثابت TrieNode *TN) { ܦܢܐ !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام TrieNode,
      خاص TrailingObjects<TrieSubtrie, LazyAtomicPointer<TrieNode>> {
عام:
  using Slot = LazyAtomicPointer<TrieNode>;

  Slot &get(size_t I) { ܦܢܐ getTrailingObjects()[I]; }
  TrieNode *load(size_t I) { ܦܢܐ get(I).load(); }

  unsigned size() ثابت { ܦܢܐ Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t NumSubtrieBits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t NumBits);

  explicit TrieSubtrie(size_t StartBit, size_t NumBits);

  ساكن منطقي classof(ثابت TrieNode *TN) { ܦܢܐ TN->IsSubtrie; }

  ساكن constexpr size_t sizeToAlloc(unsigned NumBits) {
    أكد(NumBits < 20 && "Tries should have fewer than ~1M slots");
    unsigned Count = 1u << NumBits;
    ܦܢܐ totalSizeToAlloc<LazyAtomicPointer<TrieNode>>(Count);
  }

خاص:
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
  unsigned NumBits = 0;
  unsigned Size = 0;
  friend صنف llvm::ThreadSafeTrieRawHashMapBase;
  friend صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t NumBits) {
  فراغ *Memory = ::operator جديد(sizeToAlloc(NumBits));
  TrieSubtrie *S = ::جديد (Memory) TrieSubtrie(StartBit, NumBits);
  ܦܢܐ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t NumBits)
    : TrieNode(ܫܪܪܐ), StartBit(StartBit), NumBits(NumBits), Size(1u << NumBits),
      Next(nullptr) {
  لكل (unsigned I = 0; I < Size; ++I)
    جديد (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<TrieNode>>::value,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t NumSubtrieBits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(NumSubtrieBits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + NumBits, NumSubtrieBits);

  أكد(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  TrieNode *ExistingNode = &Content;
  أكد(I < Size);
  ܐܢ (get(I).compare_exchange_strong(ExistingNode, S.get()))
    ܦܢܐ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ܦܢܐ cast<TrieSubtrie>(ExistingNode);
}

صنف ThreadSafeTrieRawHashMapBase::ImplType final
    : خاص TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t StartBit, size_t NumBits) {
    size_t Size = حجم(ImplType) + TrieSubtrie::sizeToAlloc(NumBits);
    فراغ *Memory = ::operator جديد(Size);
    ImplType *Impl = ::جديد (Memory) ImplType(StartBit, NumBits);
    ܦܢܐ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ܟܡܐ (!getRoot()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ܦܢܐ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *getRoot() { ܦܢܐ getTrailingObjects(); }

  ساكن فراغ *operator جديد(size_t Size) { ܦܢܐ ::operator جديد(Size); }
  فراغ operator delete(فراغ *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

خاص:
  friend صنف TrailingObjects;

  ImplType(size_t StartBit, size_t NumBits) {
    ::جديد (getRoot()) TrieSubtrie(StartBit, NumBits);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  ܐܢ (ImplType *Impl = ImplPtr.load())
    ܦܢܐ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, NumRootBits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ܐܢ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ܦܢܐ *Impl.release();

  // Already created, return the existing ImplType.
  ܦܢܐ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  ܐܢ (!Impl)
    ܦܢܐ PointerBase();

  TrieSubtrie *S = Impl->getRoot();
  TrieHashIndexGenerator IndexGen{NumRootBits, NumSubtrieBits, Hash};
  size_t Index = IndexGen.next();
  ܟܡܐ (Index != IndexGen.end()) {
    // Try to set the content.
    TrieNode *Existing = S->get(Index);
    ܐܢ (!Existing)
      ܦܢܐ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    ܐܢ (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      ܦܢܐ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->getValuePointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::insert(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.getRoot();
  TrieHashIndexGenerator IndexGen{NumRootBits, NumSubtrieBits, Hash};
  size_t Index;
  ܐܢ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } ܐܚܪܢܐ {
    Index = IndexGen.next();
  }

  ܟܡܐ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = ܕܓܠܐ;
    TrieNode &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = ܫܪܪܐ;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      ܦܢܐ Content;
    });
    // If we just generated it, return it!
    ܐܢ (Generated)
      ܦܢܐ PointerBase(cast<TrieContent>(Existing).getValuePointer());

    ܐܢ (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      استمر;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    ܐܢ (ExistingContent.getHash() == Hash)
      ܦܢܐ PointerBase(ExistingContent.getValuePointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    ܟܡܐ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.getNumBits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ܦܢܐ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      ܐܢ (NextIndex != NewIndexForExistingContent)
        اكسر;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> NumRootBits, std::optional<size_t> NumSubtrieBits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      NumRootBits(NumRootBits.value_or(DefaultNumRootBits)),
      NumSubtrieBits(NumSubtrieBits.value_or(DefaultNumSubtrieBits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!NumRootBits || *NumRootBits < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!NumSubtrieBits || *NumSubtrieBits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), NumRootBits(RHS.NumRootBits),
      NumSubtrieBits(RHS.NumSubtrieBits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  أكد(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

فراغ ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  ܐܢ (!Impl)
    ܦܢܐ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ܐܢ (Destructor)
    لكل (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
      لكل (unsigned I = 0; I < Trie->size(); ++I)
        ܐܢ (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->getValuePointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->getRoot()->Next;
  ܟܡܐ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getRoot() ثابت {
  ImplType *Impl = ImplPtr.load();
  ܐܢ (!Impl)
    ܦܢܐ PointerBase();
  ܦܢܐ PointerBase(Impl->getRoot());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  ܐܢ (!P.P)
    ܦܢܐ 0;
  ܐܢ (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    ܦܢܐ S->StartBit;
  ܦܢܐ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumBits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  ܐܢ (!P.P)
    ܦܢܐ 0;
  ܐܢ (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    ܦܢܐ S->NumBits;
  ܦܢܐ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumSlotUsed(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  ܐܢ (!P.P)
    ܦܢܐ 0;
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ܐܢ (!S)
    ܦܢܐ 0;
  unsigned Num = 0;
  لكل (unsigned I = 0, E = S->size(); I < E; ++I)
    ܐܢ (S->load(I))
      ++Num;
  ܦܢܐ Num;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  ܐܢ (!P.P)
    ܦܢܐ "";

  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ܐܢ (!S || !S->IsSubtrie)
    ܦܢܐ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *Node = nullptr;
  ܟܡܐ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    لكل (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      ܐܢ (!S)
        استمر;

      ܐܢ (auto *Content = dyn_cast<TrieContent>(S))
        Node = Content;
      ܐܚܪܢܐ ܐܢ (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      اكسر;
    }

    // Found the node.
    ܐܢ (Node)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  أكد(Node && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(Node->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/ܫܪܪܐ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  لكل (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.push_back('0' + ((Node->getHash()[Index] >> Offset) & 1));
  }

  ܐܢ (!Bits.empty())
    SS << "[" << Bits << "]";

  ܦܢܐ SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::getNumTries() ثابت {
  ImplType *Impl = ImplPtr.load();
  ܐܢ (!Impl)
    ܦܢܐ 0;
  unsigned Num = 0;
  لكل (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
    ++Num;
  ܦܢܐ Num;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  ܐܢ (!P.P)
    ܦܢܐ PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ܐܢ (!S)
    ܦܢܐ PointerBase();
  ܐܢ (auto *E = S->Next.load())
    ܦܢܐ PointerBase(E);
  ܦܢܐ PointerBase();
}
