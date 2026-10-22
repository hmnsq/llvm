//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/TrieRawHashMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/LazyAtomicPointer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/TrieHashIndexGenerator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Allocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ThreadSafeAllocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/TrailingObjects.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <memory>

using ନାମାକାଶ llvm;

ନାମାକାଶ {
ସଂରଚନା Trie_ଗଣ୍ଠି {
  ଅଚଳ ତାର୍କିକ IsSubtrie = ମିଥ୍ୟା;

  Trie_ଗଣ୍ଠି(ତାର୍କିକ IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ସ୍ଥିର ଶୂନ୍ୟ *operator ନୂତନ(size_t Size) { ଫେରନ୍ତୁ ::operator ନୂତନ(Size); }
  ଶୂନ୍ୟ operator delete(ଶୂନ୍ୟ *Ptr) { ::operator delete(Ptr); }
};

ସଂରଚନା TrieContent final : ସର୍ବସାଧାରଣ Trie_ଗଣ୍ଠି {
  ଅଚଳ uint8_t ContentOffset;
  ଅଚଳ uint8_t HashSize;
  ଅଚଳ uint8_t HashOffset;

  ଶୂନ୍ୟ *get_ମୂଲ୍ୟ_Pointer() ଅଚଳ {
    auto *Content = reinterpret_cast<ଅଚଳ uint8_t *>(ଏହି) + ContentOffset;
    ଫେରନ୍ତୁ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ଅଚଳ {
    auto *Begin = reinterpret_cast<ଅଚଳ uint8_t *>(ଏହି) + HashOffset;
    ଫେରନ୍ତୁ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_ଗଣ୍ଠି(/*IsSubtrie=*/ମିଥ୍ୟା), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ Trie_ଗଣ୍ଠି *TN) { ଫେରନ୍ତୁ !TN->IsSubtrie; }
};

static_assert(ଆକାର(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

ଶ୍ରେଣୀ TrieSubtrie final
    : ସର୍ବସାଧାରଣ Trie_ଗଣ୍ଠି,
      ବ୍ୟକ୍ତିଗତ TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_ଗଣ୍ଠି>> {
ସର୍ବସାଧାରଣ:
  using Slot = LazyAtomicPointer<Trie_ଗଣ୍ଠି>;

  Slot &get(size_t I) { ଫେରନ୍ତୁ getTrailingObjects()[I]; }
  Trie_ଗଣ୍ଠି *load(size_t I) { ଫେରନ୍ତୁ get(I).load(); }

  unsigned size() ଅଚଳ { ଫେରନ୍ତୁ Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t ଅଙ୍କ_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ସ୍ଥିର std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t ଅଙ୍କ_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t ଅଙ୍କ_Bits_2);

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ Trie_ଗଣ୍ଠି *TN) { ଫେରନ୍ତୁ TN->IsSubtrie; }

  ସ୍ଥିର constexpr size_t sizeToAlloc(unsigned ଅଙ୍କ_Bits_2) {
    ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned ଗଣନା_3 = 1u << ଅଙ୍କ_Bits_2;
    ଫେରନ୍ତୁ totalSizeToAlloc<LazyAtomicPointer<Trie_ଗଣ୍ଠି>>(ଗଣନା_3);
  }

ବ୍ୟକ୍ତିଗତ:
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
  unsigned ଅଙ୍କ_Bits_2 = 0;
  unsigned Size = 0;
  friend ଶ୍ରେଣୀ llvm::ThreadSafeTrieRawHashMapBase;
  friend ଶ୍ରେଣୀ TrailingObjects;

ସର୍ବସାଧାରଣ:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t ଅଙ୍କ_Bits_2) {
  ଶୂନ୍ୟ *Memory = ::operator ନୂତନ(sizeToAlloc(ଅଙ୍କ_Bits_2));
  TrieSubtrie *S = ::ନୂତନ (Memory) TrieSubtrie(StartBit, ଅଙ୍କ_Bits_2);
  ଫେରନ୍ତୁ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t ଅଙ୍କ_Bits_2)
    : Trie_ଗଣ୍ଠି(ସତ୍ୟ), StartBit(StartBit), ଅଙ୍କ_Bits_2(ଅଙ୍କ_Bits_2), Size(1u << ଅଙ୍କ_Bits_2),
      Next(nullptr) {
  ପାଇଁ (unsigned I = 0; I < Size; ++I)
    ନୂତନ (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_ଗଣ୍ଠି>>::ମୂଲ୍ୟ_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t ଅଙ୍କ_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + ଅଙ୍କ_Bits_2, ଅଙ୍କ_Subtrie_Bits);

  ପ୍ରତିଜ୍ଞା(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ଗଣ୍ଠି *Existing_ଗଣ୍ଠି = &Content;
  ପ୍ରତିଜ୍ଞା(I < Size);
  ଯଦି (get(I).compare_exchange_strong(Existing_ଗଣ୍ଠି, S.get()))
    ଫେରନ୍ତୁ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ଫେରନ୍ତୁ cast<TrieSubtrie>(Existing_ଗଣ୍ଠି);
}

ଶ୍ରେଣୀ ThreadSafeTrieRawHashMapBase::ImplType final
    : ବ୍ୟକ୍ତିଗତ TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
ସର୍ବସାଧାରଣ:
  ସ୍ଥିର std::unique_ptr<ImplType> create(size_t StartBit, size_t ଅଙ୍କ_Bits_2) {
    size_t Size = ଆକାର(ImplType) + TrieSubtrie::sizeToAlloc(ଅଙ୍କ_Bits_2);
    ଶୂନ୍ୟ *Memory = ::operator ନୂତନ(Size);
    ImplType *Impl = ::ନୂତନ (Memory) ImplType(StartBit, ଅଙ୍କ_Bits_2);
    ଫେରନ୍ତୁ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ପ୍ରତିଜ୍ଞା(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ଯେତେବେଳେ (!get_ମୂଳ()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ଫେରନ୍ତୁ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ମୂଳ() { ଫେରନ୍ତୁ getTrailingObjects(); }

  ସ୍ଥିର ଶୂନ୍ୟ *operator ନୂତନ(size_t Size) { ଫେରନ୍ତୁ ::operator ନୂତନ(Size); }
  ଶୂନ୍ୟ operator delete(ଶୂନ୍ୟ *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

ବ୍ୟକ୍ତିଗତ:
  friend ଶ୍ରେଣୀ TrailingObjects;

  ImplType(size_t StartBit, size_t ଅଙ୍କ_Bits_2) {
    ::ନୂତନ (get_ମୂଳ()) TrieSubtrie(StartBit, ଅଙ୍କ_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  ଯଦି (ImplType *Impl = ImplPtr.load())
    ଫେରନ୍ତୁ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, ଅଙ୍କ_ମୂଳ_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ଯଦି (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ଫେରନ୍ତୁ *Impl.release();

  // Already created, return the existing ImplType.
  ଫେରନ୍ତୁ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  ଯଦି (!Impl)
    ଫେରନ୍ତୁ PointerBase();

  TrieSubtrie *S = Impl->get_ମୂଳ();
  TrieHashIndexGenerator IndexGen{ଅଙ୍କ_ମୂଳ_Bits, ଅଙ୍କ_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  ଯେତେବେଳେ (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_ଗଣ୍ଠି *Existing = S->get(Index);
    ଯଦି (!Existing)
      ଫେରନ୍ତୁ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    ଯଦି (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      ଫେରନ୍ତୁ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_ମୂଲ୍ୟ_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::ଭର୍ତ୍ତି(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ଅଚଳ uint8_t *(ଶୂନ୍ୟ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ପ୍ରତିଜ୍ଞା(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ମୂଳ();
  TrieHashIndexGenerator IndexGen{ଅଙ୍କ_ମୂଳ_Bits, ଅଙ୍କ_Subtrie_Bits, Hash};
  size_t Index;
  ଯଦି (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } ଅନ୍ୟଥା {
    Index = IndexGen.next();
  }

  ଯେତେବେଳେ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    ତାର୍କିକ Generated = ମିଥ୍ୟା;
    Trie_ଗଣ୍ଠି &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = ସତ୍ୟ;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ଅଚଳ uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::ନୂତନ (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      ପ୍ରତିଜ୍ଞା(Hash == Content->getHash() && "Hash not properly initialized");
      ଫେରନ୍ତୁ Content;
    });
    // If we just generated it, return it!
    ଯଦି (Generated)
      ଫେରନ୍ତୁ PointerBase(cast<TrieContent>(Existing).get_ମୂଲ୍ୟ_Pointer());

    ଯଦି (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      ଜାରି;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    ଯଦି (ExistingContent.getHash() == Hash)
      ଫେରନ୍ତୁ PointerBase(ExistingContent.get_ମୂଲ୍ୟ_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    ଯେତେବେଳେ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_ଅଙ୍କ_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ଫେରନ୍ତୁ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      ଯଦି (NextIndex != NewIndexForExistingContent)
        ଭାଙ୍ଗ;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> ଅଙ୍କ_ମୂଳ_Bits, std::optional<size_t> ଅଙ୍କ_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      ଅଙ୍କ_ମୂଳ_Bits(ଅଙ୍କ_ମୂଳ_Bits.ମୂଲ୍ୟ_or(Default_ଅଙ୍କ_ମୂଳ_Bits)),
      ଅଙ୍କ_Subtrie_Bits(ଅଙ୍କ_Subtrie_Bits.ମୂଲ୍ୟ_or(Default_ଅଙ୍କ_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ପ୍ରତିଜ୍ଞା((!ଅଙ୍କ_ମୂଳ_Bits || *ଅଙ୍କ_ମୂଳ_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  ପ୍ରତିଜ୍ଞା((!ଅଙ୍କ_Subtrie_Bits || *ଅଙ୍କ_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), ଅଙ୍କ_ମୂଳ_Bits(RHS.ଅଙ୍କ_ମୂଳ_Bits),
      ଅଙ୍କ_Subtrie_Bits(RHS.ଅଙ୍କ_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  ପ୍ରତିଜ୍ଞା(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

ଶୂନ୍ୟ ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<ଶୂନ୍ୟ(ଶୂନ୍ୟ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  ଯଦି (!Impl)
    ଫେରନ୍ତୁ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ଯଦି (Destructor)
    ପାଇଁ (TrieSubtrie *Trie = Impl->get_ମୂଳ(); Trie; Trie = Trie->Next.load())
      ପାଇଁ (unsigned I = 0; I < Trie->size(); ++I)
        ଯଦି (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_ମୂଲ୍ୟ_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ମୂଳ()->Next;
  ଯେତେବେଳେ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_ମୂଳ() ଅଚଳ {
  ImplType *Impl = ImplPtr.load();
  ଯଦି (!Impl)
    ଫେରନ୍ତୁ PointerBase();
  ଫେରନ୍ତୁ PointerBase(Impl->get_ମୂଳ());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!P.isHint() && "Not a valid trie");
  ଯଦି (!P.P)
    ଫେରନ୍ତୁ 0;
  ଯଦି (auto *S = dyn_cast<TrieSubtrie>((Trie_ଗଣ୍ଠି *)P.P))
    ଫେରନ୍ତୁ S->StartBit;
  ଫେରନ୍ତୁ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_ଅଙ୍କ_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!P.isHint() && "Not a valid trie");
  ଯଦି (!P.P)
    ଫେରନ୍ତୁ 0;
  ଯଦି (auto *S = dyn_cast<TrieSubtrie>((Trie_ଗଣ୍ଠି *)P.P))
    ଫେରନ୍ତୁ S->ଅଙ୍କ_Bits_2;
  ଫେରନ୍ତୁ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_ଅଙ୍କ_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!P.isHint() && "Not a valid trie");
  ଯଦି (!P.P)
    ଫେରନ୍ତୁ 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_ଗଣ୍ଠି *)P.P);
  ଯଦି (!S)
    ଫେରନ୍ତୁ 0;
  unsigned ଅଙ୍କ = 0;
  ପାଇଁ (unsigned I = 0, E = S->size(); I < E; ++I)
    ଯଦି (S->load(I))
      ++ଅଙ୍କ;
  ଫେରନ୍ତୁ ଅଙ୍କ;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!P.isHint() && "Not a valid trie");
  ଯଦି (!P.P)
    ଫେରନ୍ତୁ "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_ଗଣ୍ଠି *)P.P);
  ଯଦି (!S || !S->IsSubtrie)
    ଫେରନ୍ତୁ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ଗଣ୍ଠି = nullptr;
  ଯେତେବେଳେ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    ପାଇଁ (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      ଯଦି (!S)
        ଜାରି;

      ଯଦି (auto *Content = dyn_cast<TrieContent>(S))
        ଗଣ୍ଠି = Content;
      ଅନ୍ୟଥା ଯଦି (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      ଭାଙ୍ଗ;
    }

    // Found the node.
    ଯଦି (ଗଣ୍ଠି)
      ଭାଙ୍ଗ;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  ପ୍ରତିଜ୍ଞା(ଗଣ୍ଠି && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(ଗଣ୍ଠି->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/ସତ୍ୟ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  ପାଇଁ (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.ଠେଲ_back('0' + ((ଗଣ୍ଠି->getHash()[Index] >> Offset) & 1));
  }

  ଯଦି (!Bits.empty())
    SS << "[" << Bits << "]";

  ଫେରନ୍ତୁ SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_ଅଙ୍କ_Tries() ଅଚଳ {
  ImplType *Impl = ImplPtr.load();
  ଯଦି (!Impl)
    ଫେରନ୍ତୁ 0;
  unsigned ଅଙ୍କ = 0;
  ପାଇଁ (TrieSubtrie *Trie = Impl->get_ମୂଳ(); Trie; Trie = Trie->Next.load())
    ++ଅଙ୍କ;
  ଫେରନ୍ତୁ ଅଙ୍କ;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!P.isHint() && "Not a valid trie");
  ଯଦି (!P.P)
    ଫେରନ୍ତୁ PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_ଗଣ୍ଠି *)P.P);
  ଯଦି (!S)
    ଫେରନ୍ତୁ PointerBase();
  ଯଦି (auto *E = S->Next.load())
    ଫେରନ୍ତୁ PointerBase(E);
  ଫେରନ୍ତୁ PointerBase();
}
