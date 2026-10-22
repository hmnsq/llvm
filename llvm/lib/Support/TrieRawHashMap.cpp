//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/TrieRawHashMap.h"
#അന്തര്ഭാവ "llvm/ADT/LazyAtomicPointer.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/TrieHashIndexGenerator.h"
#അന്തര്ഭാവ "llvm/Support/Allocator.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/ThreadSafeAllocator.h"
#അന്തര്ഭാവ "llvm/Support/TrailingObjects.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <memory>

using നാമാകാശ llvm;

നാമാകാശ {
ഘടന Trie_കണ്ണി {
  സ്ഥിരം തർക്കം IsSubtrie = അസത്യം;

  Trie_കണ്ണി(തർക്കം IsSubtrie) : IsSubtrie(IsSubtrie) {}

  സ്ഥിതം ശൂന്യം *operator പുതിയ(size_t Size) { തിരികെ ::operator പുതിയ(Size); }
  ശൂന്യം operator delete(ശൂന്യം *Ptr) { ::operator delete(Ptr); }
};

ഘടന TrieContent final : പൊതു Trie_കണ്ണി {
  സ്ഥിരം uint8_t ContentOffset;
  സ്ഥിരം uint8_t HashSize;
  സ്ഥിരം uint8_t HashOffset;

  ശൂന്യം *get_മൂല്യം_Pointer() സ്ഥിരം {
    auto *Content = reinterpret_cast<സ്ഥിരം uint8_t *>(ഇത്) + ContentOffset;
    തിരികെ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() സ്ഥിരം {
    auto *Begin = reinterpret_cast<സ്ഥിരം uint8_t *>(ഇത്) + HashOffset;
    തിരികെ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_കണ്ണി(/*IsSubtrie=*/അസത്യം), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  സ്ഥിതം തർക്കം classof(സ്ഥിരം Trie_കണ്ണി *TN) { തിരികെ !TN->IsSubtrie; }
};

static_assert(വലുപ്പം(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

വർഗ്ഗം TrieSubtrie final
    : പൊതു Trie_കണ്ണി,
      സ്വകാര്യ TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_കണ്ണി>> {
പൊതു:
  using Slot = LazyAtomicPointer<Trie_കണ്ണി>;

  Slot &get(size_t I) { തിരികെ getTrailingObjects()[I]; }
  Trie_കണ്ണി *load(size_t I) { തിരികെ get(I).load(); }

  unsigned size() സ്ഥിരം { തിരികെ Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t അക്കം_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  സ്ഥിതം std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t അക്കം_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t അക്കം_Bits_2);

  സ്ഥിതം തർക്കം classof(സ്ഥിരം Trie_കണ്ണി *TN) { തിരികെ TN->IsSubtrie; }

  സ്ഥിതം constexpr size_t sizeToAlloc(unsigned അക്കം_Bits_2) {
    പ്രതിജ്ഞാ(അക്കം_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned എണ്ണം_2 = 1u << അക്കം_Bits_2;
    തിരികെ totalSizeToAlloc<LazyAtomicPointer<Trie_കണ്ണി>>(എണ്ണം_2);
  }

സ്വകാര്യ:
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
  unsigned അക്കം_Bits_2 = 0;
  unsigned Size = 0;
  friend വർഗ്ഗം llvm::ThreadSafeTrieRawHashMapBase;
  friend വർഗ്ഗം TrailingObjects;

പൊതു:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t അക്കം_Bits_2) {
  ശൂന്യം *Memory = ::operator പുതിയ(sizeToAlloc(അക്കം_Bits_2));
  TrieSubtrie *S = ::പുതിയ (Memory) TrieSubtrie(StartBit, അക്കം_Bits_2);
  തിരികെ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t അക്കം_Bits_2)
    : Trie_കണ്ണി(സത്യം), StartBit(StartBit), അക്കം_Bits_2(അക്കം_Bits_2), Size(1u << അക്കം_Bits_2),
      Next(nullptr) {
  വേണ്ടി (unsigned I = 0; I < Size; ++I)
    പുതിയ (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_കണ്ണി>>::മൂല്യം_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t അക്കം_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  പ്രതിജ്ഞാ(അക്കം_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + അക്കം_Bits_2, അക്കം_Subtrie_Bits);

  പ്രതിജ്ഞാ(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_കണ്ണി *Existing_കണ്ണി = &Content;
  പ്രതിജ്ഞാ(I < Size);
  എങ്കിൽ (get(I).compare_exchange_strong(Existing_കണ്ണി, S.get()))
    തിരികെ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  തിരികെ cast<TrieSubtrie>(Existing_കണ്ണി);
}

വർഗ്ഗം ThreadSafeTrieRawHashMapBase::ImplType final
    : സ്വകാര്യ TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
പൊതു:
  സ്ഥിതം std::unique_ptr<ImplType> create(size_t StartBit, size_t അക്കം_Bits_2) {
    size_t Size = വലുപ്പം(ImplType) + TrieSubtrie::sizeToAlloc(അക്കം_Bits_2);
    ശൂന്യം *Memory = ::operator പുതിയ(Size);
    ImplType *Impl = ::പുതിയ (Memory) ImplType(StartBit, അക്കം_Bits_2);
    തിരികെ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    പ്രതിജ്ഞാ(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ആകുമ്പോൾ (!get_വേര്()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    തിരികെ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_വേര്() { തിരികെ getTrailingObjects(); }

  സ്ഥിതം ശൂന്യം *operator പുതിയ(size_t Size) { തിരികെ ::operator പുതിയ(Size); }
  ശൂന്യം operator delete(ശൂന്യം *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

സ്വകാര്യ:
  friend വർഗ്ഗം TrailingObjects;

  ImplType(size_t StartBit, size_t അക്കം_Bits_2) {
    ::പുതിയ (get_വേര്()) TrieSubtrie(StartBit, അക്കം_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  എങ്കിൽ (ImplType *Impl = ImplPtr.load())
    തിരികെ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, അക്കം_വേര്_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  എങ്കിൽ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    തിരികെ *Impl.release();

  // Already created, return the existing ImplType.
  തിരികെ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) സ്ഥിരം {
  പ്രതിജ്ഞാ(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  എങ്കിൽ (!Impl)
    തിരികെ PointerBase();

  TrieSubtrie *S = Impl->get_വേര്();
  TrieHashIndexGenerator IndexGen{അക്കം_വേര്_Bits, അക്കം_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  ആകുമ്പോൾ (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_കണ്ണി *Existing = S->get(Index);
    എങ്കിൽ (!Existing)
      തിരികെ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    എങ്കിൽ (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      തിരികെ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_മൂല്യം_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::തിരുകുക(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<സ്ഥിരം uint8_t *(ശൂന്യം *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  പ്രതിജ്ഞാ(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_വേര്();
  TrieHashIndexGenerator IndexGen{അക്കം_വേര്_Bits, അക്കം_Subtrie_Bits, Hash};
  size_t Index;
  എങ്കിൽ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } അല്ലെങ്കിൽ {
    Index = IndexGen.next();
  }

  ആകുമ്പോൾ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    തർക്കം Generated = അസത്യം;
    Trie_കണ്ണി &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = സത്യം;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      സ്ഥിരം uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::പുതിയ (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      പ്രതിജ്ഞാ(Hash == Content->getHash() && "Hash not properly initialized");
      തിരികെ Content;
    });
    // If we just generated it, return it!
    എങ്കിൽ (Generated)
      തിരികെ PointerBase(cast<TrieContent>(Existing).get_മൂല്യം_Pointer());

    എങ്കിൽ (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      തുടരുക;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    എങ്കിൽ (ExistingContent.getHash() == Hash)
      തിരികെ PointerBase(ExistingContent.get_മൂല്യം_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    ആകുമ്പോൾ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_അക്കം_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    തിരികെ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      എങ്കിൽ (NextIndex != NewIndexForExistingContent)
        നിർത്തുക;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> അക്കം_വേര്_Bits, std::optional<size_t> അക്കം_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      അക്കം_വേര്_Bits(അക്കം_വേര്_Bits.മൂല്യം_or(Default_അക്കം_വേര്_Bits)),
      അക്കം_Subtrie_Bits(അക്കം_Subtrie_Bits.മൂല്യം_or(Default_അക്കം_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  പ്രതിജ്ഞാ((!അക്കം_വേര്_Bits || *അക്കം_വേര്_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  പ്രതിജ്ഞാ((!അക്കം_Subtrie_Bits || *അക്കം_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), അക്കം_വേര്_Bits(RHS.അക്കം_വേര്_Bits),
      അക്കം_Subtrie_Bits(RHS.അക്കം_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  പ്രതിജ്ഞാ(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

ശൂന്യം ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<ശൂന്യം(ശൂന്യം *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  എങ്കിൽ (!Impl)
    തിരികെ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  എങ്കിൽ (Destructor)
    വേണ്ടി (TrieSubtrie *Trie = Impl->get_വേര്(); Trie; Trie = Trie->Next.load())
      വേണ്ടി (unsigned I = 0; I < Trie->size(); ++I)
        എങ്കിൽ (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_മൂല്യം_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_വേര്()->Next;
  ആകുമ്പോൾ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_വേര്() സ്ഥിരം {
  ImplType *Impl = ImplPtr.load();
  എങ്കിൽ (!Impl)
    തിരികെ PointerBase();
  തിരികെ PointerBase(Impl->get_വേര്());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) സ്ഥിരം {
  പ്രതിജ്ഞാ(!P.isHint() && "Not a valid trie");
  എങ്കിൽ (!P.P)
    തിരികെ 0;
  എങ്കിൽ (auto *S = dyn_cast<TrieSubtrie>((Trie_കണ്ണി *)P.P))
    തിരികെ S->StartBit;
  തിരികെ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_അക്കം_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) സ്ഥിരം {
  പ്രതിജ്ഞാ(!P.isHint() && "Not a valid trie");
  എങ്കിൽ (!P.P)
    തിരികെ 0;
  എങ്കിൽ (auto *S = dyn_cast<TrieSubtrie>((Trie_കണ്ണി *)P.P))
    തിരികെ S->അക്കം_Bits_2;
  തിരികെ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_അക്കം_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) സ്ഥിരം {
  പ്രതിജ്ഞാ(!P.isHint() && "Not a valid trie");
  എങ്കിൽ (!P.P)
    തിരികെ 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_കണ്ണി *)P.P);
  എങ്കിൽ (!S)
    തിരികെ 0;
  unsigned അക്കം = 0;
  വേണ്ടി (unsigned I = 0, E = S->size(); I < E; ++I)
    എങ്കിൽ (S->load(I))
      ++അക്കം;
  തിരികെ അക്കം;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) സ്ഥിരം {
  പ്രതിജ്ഞാ(!P.isHint() && "Not a valid trie");
  എങ്കിൽ (!P.P)
    തിരികെ "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_കണ്ണി *)P.P);
  എങ്കിൽ (!S || !S->IsSubtrie)
    തിരികെ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *കണ്ണി = nullptr;
  ആകുമ്പോൾ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    വേണ്ടി (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      എങ്കിൽ (!S)
        തുടരുക;

      എങ്കിൽ (auto *Content = dyn_cast<TrieContent>(S))
        കണ്ണി = Content;
      അല്ലെങ്കിൽ എങ്കിൽ (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      നിർത്തുക;
    }

    // Found the node.
    എങ്കിൽ (കണ്ണി)
      നിർത്തുക;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  പ്രതിജ്ഞാ(കണ്ണി && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(കണ്ണി->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/സത്യം);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  വേണ്ടി (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.തള്ളുക_back('0' + ((കണ്ണി->getHash()[Index] >> Offset) & 1));
  }

  എങ്കിൽ (!Bits.empty())
    SS << "[" << Bits << "]";

  തിരികെ SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_അക്കം_Tries() സ്ഥിരം {
  ImplType *Impl = ImplPtr.load();
  എങ്കിൽ (!Impl)
    തിരികെ 0;
  unsigned അക്കം = 0;
  വേണ്ടി (TrieSubtrie *Trie = Impl->get_വേര്(); Trie; Trie = Trie->Next.load())
    ++അക്കം;
  തിരികെ അക്കം;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) സ്ഥിരം {
  പ്രതിജ്ഞാ(!P.isHint() && "Not a valid trie");
  എങ്കിൽ (!P.P)
    തിരികെ PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_കണ്ണി *)P.P);
  എങ്കിൽ (!S)
    തിരികെ PointerBase();
  എങ്കിൽ (auto *E = S->Next.load())
    തിരികെ PointerBase(E);
  തിരികെ PointerBase();
}
