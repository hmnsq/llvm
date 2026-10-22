//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/TrieRawHashMap.h"
#অন্তর্ভাব "llvm/ADT/LazyAtomicPointer.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/TrieHashIndexGenerator.h"
#অন্তর্ভাব "llvm/Support/Allocator.h"
#অন্তর্ভাব "llvm/Support/Casting.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/ThreadSafeAllocator.h"
#অন্তর্ভাব "llvm/Support/TrailingObjects.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <memory>

using নামাকাশ llvm;

নামাকাশ {
সংরচনা Trie_গ্রন্থি {
  ধ্রুব তার্কিক IsSubtrie = অসত্য;

  Trie_গ্রন্থি(তার্কিক IsSubtrie) : IsSubtrie(IsSubtrie) {}

  স্থির শূন্য *operator নূতন(size_t Size) { প্রত্যাগম ::operator নূতন(Size); }
  শূন্য operator delete(শূন্য *Ptr) { ::operator delete(Ptr); }
};

সংরচনা TrieContent final : সার্বজনিক Trie_গ্রন্থি {
  ধ্রুব uint8_t ContentOffset;
  ধ্রুব uint8_t HashSize;
  ধ্রুব uint8_t HashOffset;

  শূন্য *get_মান_Pointer() ধ্রুব {
    auto *Content = reinterpret_cast<ধ্রুব uint8_t *>(এতত্) + ContentOffset;
    প্রত্যাগম const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ধ্রুব {
    auto *Begin = reinterpret_cast<ধ্রুব uint8_t *>(এতত্) + HashOffset;
    প্রত্যাগম ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_গ্রন্থি(/*IsSubtrie=*/অসত্য), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  স্থির তার্কিক classof(ধ্রুব Trie_গ্রন্থি *TN) { প্রত্যাগম !TN->IsSubtrie; }
};

static_assert(পরিমাণ(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

বর্গ TrieSubtrie final
    : সার্বজনিক Trie_গ্রন্থি,
      গুপ্ত TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_গ্রন্থি>> {
সার্বজনিক:
  using Slot = LazyAtomicPointer<Trie_গ্রন্থি>;

  Slot &get(size_t I) { প্রত্যাগম getTrailingObjects()[I]; }
  Trie_গ্রন্থি *load(size_t I) { প্রত্যাগম get(I).load(); }

  unsigned size() ধ্রুব { প্রত্যাগম Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t অঙ্ক_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  স্থির std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t অঙ্ক_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t অঙ্ক_Bits_2);

  স্থির তার্কিক classof(ধ্রুব Trie_গ্রন্থি *TN) { প্রত্যাগম TN->IsSubtrie; }

  স্থির constexpr size_t sizeToAlloc(unsigned অঙ্ক_Bits_2) {
    প্রতিজ্ঞা(অঙ্ক_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned গুনতি_2 = 1u << অঙ্ক_Bits_2;
    প্রত্যাগম totalSizeToAlloc<LazyAtomicPointer<Trie_গ্রন্থি>>(গুনতি_2);
  }

গুপ্ত:
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
  unsigned অঙ্ক_Bits_2 = 0;
  unsigned Size = 0;
  friend বর্গ llvm::ThreadSafeTrieRawHashMapBase;
  friend বর্গ TrailingObjects;

সার্বজনিক:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t অঙ্ক_Bits_2) {
  শূন্য *Memory = ::operator নূতন(sizeToAlloc(অঙ্ক_Bits_2));
  TrieSubtrie *S = ::নূতন (Memory) TrieSubtrie(StartBit, অঙ্ক_Bits_2);
  প্রত্যাগম std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t অঙ্ক_Bits_2)
    : Trie_গ্রন্থি(সত্য), StartBit(StartBit), অঙ্ক_Bits_2(অঙ্ক_Bits_2), Size(1u << অঙ্ক_Bits_2),
      Next(nullptr) {
  প্রতি (unsigned I = 0; I < Size; ++I)
    নূতন (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_গ্রন্থি>>::মান_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t অঙ্ক_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  প্রতিজ্ঞা(অঙ্ক_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + অঙ্ক_Bits_2, অঙ্ক_Subtrie_Bits);

  প্রতিজ্ঞা(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_গ্রন্থি *Existing_গ্রন্থি = &Content;
  প্রতিজ্ঞা(I < Size);
  যদি (get(I).compare_exchange_strong(Existing_গ্রন্থি, S.get()))
    প্রত্যাগম Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  প্রত্যাগম cast<TrieSubtrie>(Existing_গ্রন্থি);
}

বর্গ ThreadSafeTrieRawHashMapBase::ImplType final
    : গুপ্ত TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
সার্বজনিক:
  স্থির std::unique_ptr<ImplType> create(size_t StartBit, size_t অঙ্ক_Bits_2) {
    size_t Size = পরিমাণ(ImplType) + TrieSubtrie::sizeToAlloc(অঙ্ক_Bits_2);
    শূন্য *Memory = ::operator নূতন(Size);
    ImplType *Impl = ::নূতন (Memory) ImplType(StartBit, অঙ্ক_Bits_2);
    প্রত্যাগম std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    প্রতিজ্ঞা(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    যাবত্ (!get_মূল()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    প্রত্যাগম S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_মূল() { প্রত্যাগম getTrailingObjects(); }

  স্থির শূন্য *operator নূতন(size_t Size) { প্রত্যাগম ::operator নূতন(Size); }
  শূন্য operator delete(শূন্য *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

গুপ্ত:
  friend বর্গ TrailingObjects;

  ImplType(size_t StartBit, size_t অঙ্ক_Bits_2) {
    ::নূতন (get_মূল()) TrieSubtrie(StartBit, অঙ্ক_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  যদি (ImplType *Impl = ImplPtr.load())
    প্রত্যাগম *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, অঙ্ক_মূল_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  যদি (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    প্রত্যাগম *Impl.release();

  // Already created, return the existing ImplType.
  প্রত্যাগম *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ধ্রুব {
  প্রতিজ্ঞা(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    প্রত্যাগম PointerBase();

  TrieSubtrie *S = Impl->get_মূল();
  TrieHashIndexGenerator IndexGen{অঙ্ক_মূল_Bits, অঙ্ক_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  যাবত্ (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_গ্রন্থি *Existing = S->get(Index);
    যদি (!Existing)
      প্রত্যাগম PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    যদি (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      প্রত্যাগম ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_মান_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::ঢোকাও(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ধ্রুব uint8_t *(শূন্য *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  প্রতিজ্ঞা(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_মূল();
  TrieHashIndexGenerator IndexGen{অঙ্ক_মূল_Bits, অঙ্ক_Subtrie_Bits, Hash};
  size_t Index;
  যদি (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } অন্যথা {
    Index = IndexGen.next();
  }

  যাবত্ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    তার্কিক Generated = অসত্য;
    Trie_গ্রন্থি &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = সত্য;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ধ্রুব uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::নূতন (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      প্রতিজ্ঞা(Hash == Content->getHash() && "Hash not properly initialized");
      প্রত্যাগম Content;
    });
    // If we just generated it, return it!
    যদি (Generated)
      প্রত্যাগম PointerBase(cast<TrieContent>(Existing).get_মান_Pointer());

    যদি (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      অনুবর্ত;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    যদি (ExistingContent.getHash() == Hash)
      প্রত্যাগম PointerBase(ExistingContent.get_মান_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    যাবত্ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_অঙ্ক_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    প্রত্যাগম Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      যদি (NextIndex != NewIndexForExistingContent)
        ভঙ্গ;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> অঙ্ক_মূল_Bits, std::optional<size_t> অঙ্ক_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      অঙ্ক_মূল_Bits(অঙ্ক_মূল_Bits.মান_or(Default_অঙ্ক_মূল_Bits)),
      অঙ্ক_Subtrie_Bits(অঙ্ক_Subtrie_Bits.মান_or(Default_অঙ্ক_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  প্রতিজ্ঞা((!অঙ্ক_মূল_Bits || *অঙ্ক_মূল_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  প্রতিজ্ঞা((!অঙ্ক_Subtrie_Bits || *অঙ্ক_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), অঙ্ক_মূল_Bits(RHS.অঙ্ক_মূল_Bits),
      অঙ্ক_Subtrie_Bits(RHS.অঙ্ক_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  প্রতিজ্ঞা(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

শূন্য ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<শূন্য(শূন্য *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  যদি (!Impl)
    প্রত্যাগম;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  যদি (Destructor)
    প্রতি (TrieSubtrie *Trie = Impl->get_মূল(); Trie; Trie = Trie->Next.load())
      প্রতি (unsigned I = 0; I < Trie->size(); ++I)
        যদি (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_মান_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_মূল()->Next;
  যাবত্ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_মূল() ধ্রুব {
  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    প্রত্যাগম PointerBase();
  প্রত্যাগম PointerBase(Impl->get_মূল());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুব {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    প্রত্যাগম 0;
  যদি (auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P))
    প্রত্যাগম S->StartBit;
  প্রত্যাগম 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুব {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    প্রত্যাগম 0;
  যদি (auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P))
    প্রত্যাগম S->অঙ্ক_Bits_2;
  প্রত্যাগম 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুব {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    প্রত্যাগম 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S)
    প্রত্যাগম 0;
  unsigned অঙ্ক = 0;
  প্রতি (unsigned I = 0, E = S->size(); I < E; ++I)
    যদি (S->load(I))
      ++অঙ্ক;
  প্রত্যাগম অঙ্ক;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুব {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    প্রত্যাগম "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S || !S->IsSubtrie)
    প্রত্যাগম "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *গ্রন্থি = nullptr;
  যাবত্ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    প্রতি (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      যদি (!S)
        অনুবর্ত;

      যদি (auto *Content = dyn_cast<TrieContent>(S))
        গ্রন্থি = Content;
      অন্যথা যদি (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      ভঙ্গ;
    }

    // Found the node.
    যদি (গ্রন্থি)
      ভঙ্গ;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  প্রতিজ্ঞা(গ্রন্থি && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(গ্রন্থি->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/সত্য);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  প্রতি (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.ঠেলো_back('0' + ((গ্রন্থি->getHash()[Index] >> Offset) & 1));
  }

  যদি (!Bits.empty())
    SS << "[" << Bits << "]";

  প্রত্যাগম SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Tries() ধ্রুব {
  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    প্রত্যাগম 0;
  unsigned অঙ্ক = 0;
  প্রতি (TrieSubtrie *Trie = Impl->get_মূল(); Trie; Trie = Trie->Next.load())
    ++অঙ্ক;
  প্রত্যাগম অঙ্ক;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুব {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    প্রত্যাগম PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S)
    প্রত্যাগম PointerBase();
  যদি (auto *E = S->Next.load())
    প্রত্যাগম PointerBase(E);
  প্রত্যাগম PointerBase();
}
