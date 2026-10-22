//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/TrieRawHashMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/LazyAtomicPointer.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/TrieHashIndexGenerator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ThreadSafeAllocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/TrailingObjects.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <memory>

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {
ಸಂರಚನಾ Trie_ಗಂಟು {
  ಧ್ರುವ ತಾರ್ಕಿಕ IsSubtrie = ಅಸತ್ಯ;

  Trie_ಗಂಟು(ತಾರ್ಕಿಕ IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ಸ್ಥಿರ ಶೂನ್ಯ *operator ನೂತನ(size_t Size) { ಪ್ರತ್ಯಾಗಮ ::operator ನೂತನ(Size); }
  ಶೂನ್ಯ operator delete(ಶೂನ್ಯ *Ptr) { ::operator delete(Ptr); }
};

ಸಂರಚನಾ TrieContent final : ಸಾರ್ವಜನಿಕ Trie_ಗಂಟು {
  ಧ್ರುವ uint8_t ContentOffset;
  ಧ್ರುವ uint8_t HashSize;
  ಧ್ರುವ uint8_t HashOffset;

  ಶೂನ್ಯ *get_ಮೌಲ್ಯ_Pointer() ಧ್ರುವ {
    auto *Content = reinterpret_cast<ಧ್ರುವ uint8_t *>(ಏತತ್) + ContentOffset;
    ಪ್ರತ್ಯಾಗಮ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ಧ್ರುವ {
    auto *Begin = reinterpret_cast<ಧ್ರುವ uint8_t *>(ಏತತ್) + HashOffset;
    ಪ್ರತ್ಯಾಗಮ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_ಗಂಟು(/*IsSubtrie=*/ಅಸತ್ಯ), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ Trie_ಗಂಟು *TN) { ಪ್ರತ್ಯಾಗಮ !TN->IsSubtrie; }
};

static_assert(ಪರಿಮಾಣ(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

ವರ್ಗ TrieSubtrie final
    : ಸಾರ್ವಜನಿಕ Trie_ಗಂಟು,
      ಗುಪ್ತ TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_ಗಂಟು>> {
ಸಾರ್ವಜನಿಕ:
  using Slot = LazyAtomicPointer<Trie_ಗಂಟು>;

  Slot &get(size_t I) { ಪ್ರತ್ಯಾಗಮ getTrailingObjects()[I]; }
  Trie_ಗಂಟು *load(size_t I) { ಪ್ರತ್ಯಾಗಮ get(I).load(); }

  unsigned size() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t ಅಂಕೆ_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ಸ್ಥಿರ std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t ಅಂಕೆ_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t ಅಂಕೆ_Bits_2);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ Trie_ಗಂಟು *TN) { ಪ್ರತ್ಯಾಗಮ TN->IsSubtrie; }

  ಸ್ಥಿರ constexpr size_t sizeToAlloc(unsigned ಅಂಕೆ_Bits_2) {
    ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned ಎಣಿಕೆ_2 = 1u << ಅಂಕೆ_Bits_2;
    ಪ್ರತ್ಯಾಗಮ totalSizeToAlloc<LazyAtomicPointer<Trie_ಗಂಟು>>(ಎಣಿಕೆ_2);
  }

ಗುಪ್ತ:
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
  unsigned ಅಂಕೆ_Bits_2 = 0;
  unsigned Size = 0;
  friend ವರ್ಗ llvm::ThreadSafeTrieRawHashMapBase;
  friend ವರ್ಗ TrailingObjects;

ಸಾರ್ವಜನಿಕ:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t ಅಂಕೆ_Bits_2) {
  ಶೂನ್ಯ *Memory = ::operator ನೂತನ(sizeToAlloc(ಅಂಕೆ_Bits_2));
  TrieSubtrie *S = ::ನೂತನ (Memory) TrieSubtrie(StartBit, ಅಂಕೆ_Bits_2);
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t ಅಂಕೆ_Bits_2)
    : Trie_ಗಂಟು(ಸತ್ಯ), StartBit(StartBit), ಅಂಕೆ_Bits_2(ಅಂಕೆ_Bits_2), Size(1u << ಅಂಕೆ_Bits_2),
      Next(nullptr) {
  ಪ್ರತಿ (unsigned I = 0; I < Size; ++I)
    ನೂತನ (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_ಗಂಟು>>::ಮೌಲ್ಯ_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t ಅಂಕೆ_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + ಅಂಕೆ_Bits_2, ಅಂಕೆ_Subtrie_Bits);

  ಪ್ರತಿಜ್ಞಾ(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ಗಂಟು *Existing_ಗಂಟು = &Content;
  ಪ್ರತಿಜ್ಞಾ(I < Size);
  ಯದಿ (get(I).compare_exchange_strong(Existing_ಗಂಟು, S.get()))
    ಪ್ರತ್ಯಾಗಮ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ಪ್ರತ್ಯಾಗಮ cast<TrieSubtrie>(Existing_ಗಂಟು);
}

ವರ್ಗ ThreadSafeTrieRawHashMapBase::ImplType final
    : ಗುಪ್ತ TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ std::unique_ptr<ImplType> create(size_t StartBit, size_t ಅಂಕೆ_Bits_2) {
    size_t Size = ಪರಿಮಾಣ(ImplType) + TrieSubtrie::sizeToAlloc(ಅಂಕೆ_Bits_2);
    ಶೂನ್ಯ *Memory = ::operator ನೂತನ(Size);
    ImplType *Impl = ::ನೂತನ (Memory) ImplType(StartBit, ಅಂಕೆ_Bits_2);
    ಪ್ರತ್ಯಾಗಮ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ಪ್ರತಿಜ್ಞಾ(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ಯಾವತ್ (!get_ಬೇರು()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ಪ್ರತ್ಯಾಗಮ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ಬೇರು() { ಪ್ರತ್ಯಾಗಮ getTrailingObjects(); }

  ಸ್ಥಿರ ಶೂನ್ಯ *operator ನೂತನ(size_t Size) { ಪ್ರತ್ಯಾಗಮ ::operator ನೂತನ(Size); }
  ಶೂನ್ಯ operator delete(ಶೂನ್ಯ *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

ಗುಪ್ತ:
  friend ವರ್ಗ TrailingObjects;

  ImplType(size_t StartBit, size_t ಅಂಕೆ_Bits_2) {
    ::ನೂತನ (get_ಬೇರು()) TrieSubtrie(StartBit, ಅಂಕೆ_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  ಯದಿ (ImplType *Impl = ImplPtr.load())
    ಪ್ರತ್ಯಾಗಮ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, ಅಂಕೆ_ಬೇರು_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ಯದಿ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ಪ್ರತ್ಯಾಗಮ *Impl.release();

  // Already created, return the existing ImplType.
  ಪ್ರತ್ಯಾಗಮ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  ಯದಿ (!Impl)
    ಪ್ರತ್ಯಾಗಮ PointerBase();

  TrieSubtrie *S = Impl->get_ಬೇರು();
  TrieHashIndexGenerator IndexGen{ಅಂಕೆ_ಬೇರು_Bits, ಅಂಕೆ_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  ಯಾವತ್ (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_ಗಂಟು *Existing = S->get(Index);
    ಯದಿ (!Existing)
      ಪ್ರತ್ಯಾಗಮ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    ಯದಿ (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      ಪ್ರತ್ಯಾಗಮ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_ಮೌಲ್ಯ_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::ಸೇರಿಸು(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ಧ್ರುವ uint8_t *(ಶೂನ್ಯ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ಪ್ರತಿಜ್ಞಾ(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ಬೇರು();
  TrieHashIndexGenerator IndexGen{ಅಂಕೆ_ಬೇರು_Bits, ಅಂಕೆ_Subtrie_Bits, Hash};
  size_t Index;
  ಯದಿ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } ಅನ್ಯಥಾ {
    Index = IndexGen.next();
  }

  ಯಾವತ್ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    ತಾರ್ಕಿಕ Generated = ಅಸತ್ಯ;
    Trie_ಗಂಟು &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = ಸತ್ಯ;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ಧ್ರುವ uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::ನೂತನ (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      ಪ್ರತಿಜ್ಞಾ(Hash == Content->getHash() && "Hash not properly initialized");
      ಪ್ರತ್ಯಾಗಮ Content;
    });
    // If we just generated it, return it!
    ಯದಿ (Generated)
      ಪ್ರತ್ಯಾಗಮ PointerBase(cast<TrieContent>(Existing).get_ಮೌಲ್ಯ_Pointer());

    ಯದಿ (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      ಅನುವರ್ತ;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    ಯದಿ (ExistingContent.getHash() == Hash)
      ಪ್ರತ್ಯಾಗಮ PointerBase(ExistingContent.get_ಮೌಲ್ಯ_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    ಯಾವತ್ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_ಅಂಕೆ_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ಪ್ರತ್ಯಾಗಮ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      ಯದಿ (NextIndex != NewIndexForExistingContent)
        ಭಙ್ಗ;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> ಅಂಕೆ_ಬೇರು_Bits, std::optional<size_t> ಅಂಕೆ_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      ಅಂಕೆ_ಬೇರು_Bits(ಅಂಕೆ_ಬೇರು_Bits.ಮೌಲ್ಯ_or(Default_ಅಂಕೆ_ಬೇರು_Bits)),
      ಅಂಕೆ_Subtrie_Bits(ಅಂಕೆ_Subtrie_Bits.ಮೌಲ್ಯ_or(Default_ಅಂಕೆ_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ಪ್ರತಿಜ್ಞಾ((!ಅಂಕೆ_ಬೇರು_Bits || *ಅಂಕೆ_ಬೇರು_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  ಪ್ರತಿಜ್ಞಾ((!ಅಂಕೆ_Subtrie_Bits || *ಅಂಕೆ_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), ಅಂಕೆ_ಬೇರು_Bits(RHS.ಅಂಕೆ_ಬೇರು_Bits),
      ಅಂಕೆ_Subtrie_Bits(RHS.ಅಂಕೆ_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  ಪ್ರತಿಜ್ಞಾ(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

ಶೂನ್ಯ ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<ಶೂನ್ಯ(ಶೂನ್ಯ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  ಯದಿ (!Impl)
    ಪ್ರತ್ಯಾಗಮ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ಯದಿ (Destructor)
    ಪ್ರತಿ (TrieSubtrie *Trie = Impl->get_ಬೇರು(); Trie; Trie = Trie->Next.load())
      ಪ್ರತಿ (unsigned I = 0; I < Trie->size(); ++I)
        ಯದಿ (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_ಮೌಲ್ಯ_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ಬೇರು()->Next;
  ಯಾವತ್ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_ಬೇರು() ಧ್ರುವ {
  ImplType *Impl = ImplPtr.load();
  ಯದಿ (!Impl)
    ಪ್ರತ್ಯಾಗಮ PointerBase();
  ಪ್ರತ್ಯಾಗಮ PointerBase(Impl->get_ಬೇರು());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಯದಿ (!P.P)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (auto *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P))
    ಪ್ರತ್ಯಾಗಮ S->StartBit;
  ಪ್ರತ್ಯಾಗಮ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_ಅಂಕೆ_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಯದಿ (!P.P)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (auto *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P))
    ಪ್ರತ್ಯಾಗಮ S->ಅಂಕೆ_Bits_2;
  ಪ್ರತ್ಯಾಗಮ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_ಅಂಕೆ_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಯದಿ (!P.P)
    ಪ್ರತ್ಯಾಗಮ 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಯದಿ (!S)
    ಪ್ರತ್ಯಾಗಮ 0;
  unsigned ಅಂಕೆ = 0;
  ಪ್ರತಿ (unsigned I = 0, E = S->size(); I < E; ++I)
    ಯದಿ (S->load(I))
      ++ಅಂಕೆ;
  ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಯದಿ (!P.P)
    ಪ್ರತ್ಯಾಗಮ "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಯದಿ (!S || !S->IsSubtrie)
    ಪ್ರತ್ಯಾಗಮ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ಗಂಟು = nullptr;
  ಯಾವತ್ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    ಪ್ರತಿ (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      ಯದಿ (!S)
        ಅನುವರ್ತ;

      ಯದಿ (auto *Content = dyn_cast<TrieContent>(S))
        ಗಂಟು = Content;
      ಅನ್ಯಥಾ ಯದಿ (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      ಭಙ್ಗ;
    }

    // Found the node.
    ಯದಿ (ಗಂಟು)
      ಭಙ್ಗ;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  ಪ್ರತಿಜ್ಞಾ(ಗಂಟು && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(ಗಂಟು->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/ಸತ್ಯ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  ಪ್ರತಿ (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.ತಳ್ಳು_back('0' + ((ಗಂಟು->getHash()[Index] >> Offset) & 1));
  }

  ಯದಿ (!Bits.empty())
    SS << "[" << Bits << "]";

  ಪ್ರತ್ಯಾಗಮ SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_ಅಂಕೆ_Tries() ಧ್ರುವ {
  ImplType *Impl = ImplPtr.load();
  ಯದಿ (!Impl)
    ಪ್ರತ್ಯಾಗಮ 0;
  unsigned ಅಂಕೆ = 0;
  ಪ್ರತಿ (TrieSubtrie *Trie = Impl->get_ಬೇರು(); Trie; Trie = Trie->Next.load())
    ++ಅಂಕೆ;
  ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಯದಿ (!P.P)
    ಪ್ರತ್ಯಾಗಮ PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಯದಿ (!S)
    ಪ್ರತ್ಯಾಗಮ PointerBase();
  ಯದಿ (auto *E = S->Next.load())
    ಪ್ರತ್ಯಾಗಮ PointerBase(E);
  ಪ್ರತ್ಯಾಗಮ PointerBase();
}
