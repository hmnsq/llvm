//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/TrieRawHashMap.h"
#અન્તર્ભાવ "llvm/ADT/LazyAtomicPointer.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/TrieHashIndexGenerator.h"
#અન્તર્ભાવ "llvm/Support/Allocator.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/ThreadSafeAllocator.h"
#અન્તર્ભાવ "llvm/Support/TrailingObjects.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <memory>

using નામાકાશ llvm;

નામાકાશ {
સંરચના Trie_ગાંઠ {
  અચલ તાર્કિક IsSubtrie = ખોટું;

  Trie_ગાંઠ(તાર્કિક IsSubtrie) : IsSubtrie(IsSubtrie) {}

  સ્થિર શૂન્ય *operator નવું(size_t Size) { પાછા ::operator નવું(Size); }
  શૂન્ય operator delete(શૂન્ય *Ptr) { ::operator delete(Ptr); }
};

સંરચના TrieContent final : જાહેર Trie_ગાંઠ {
  અચલ uint8_t ContentOffset;
  અચલ uint8_t HashSize;
  અચલ uint8_t HashOffset;

  શૂન્ય *get_કિંમત_Pointer() અચલ {
    auto *Content = reinterpret_cast<અચલ uint8_t *>(આ) + ContentOffset;
    પાછા const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() અચલ {
    auto *Begin = reinterpret_cast<અચલ uint8_t *>(આ) + HashOffset;
    પાછા ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_ગાંઠ(/*IsSubtrie=*/ખોટું), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  સ્થિર તાર્કિક classof(અચલ Trie_ગાંઠ *TN) { પાછા !TN->IsSubtrie; }
};

static_assert(કદ(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

વર્ગ TrieSubtrie final
    : જાહેર Trie_ગાંઠ,
      ખાનગી TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_ગાંઠ>> {
જાહેર:
  using Slot = LazyAtomicPointer<Trie_ગાંઠ>;

  Slot &get(size_t I) { પાછા getTrailingObjects()[I]; }
  Trie_ગાંઠ *load(size_t I) { પાછા get(I).load(); }

  unsigned size() અચલ { પાછા Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t અંક_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  સ્થિર std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t અંક_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t અંક_Bits_2);

  સ્થિર તાર્કિક classof(અચલ Trie_ગાંઠ *TN) { પાછા TN->IsSubtrie; }

  સ્થિર constexpr size_t sizeToAlloc(unsigned અંક_Bits_2) {
    પ્રતિજ્ઞા(અંક_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned ગણતરી_2 = 1u << અંક_Bits_2;
    પાછા totalSizeToAlloc<LazyAtomicPointer<Trie_ગાંઠ>>(ગણતરી_2);
  }

ખાનગી:
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
  unsigned અંક_Bits_2 = 0;
  unsigned Size = 0;
  friend વર્ગ llvm::ThreadSafeTrieRawHashMapBase;
  friend વર્ગ TrailingObjects;

જાહેર:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t અંક_Bits_2) {
  શૂન્ય *Memory = ::operator નવું(sizeToAlloc(અંક_Bits_2));
  TrieSubtrie *S = ::નવું (Memory) TrieSubtrie(StartBit, અંક_Bits_2);
  પાછા std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t અંક_Bits_2)
    : Trie_ગાંઠ(સાચું), StartBit(StartBit), અંક_Bits_2(અંક_Bits_2), Size(1u << અંક_Bits_2),
      Next(nullptr) {
  માટે (unsigned I = 0; I < Size; ++I)
    નવું (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_ગાંઠ>>::કિંમત_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t અંક_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  પ્રતિજ્ઞા(અંક_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + અંક_Bits_2, અંક_Subtrie_Bits);

  પ્રતિજ્ઞા(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ગાંઠ *Existing_ગાંઠ = &Content;
  પ્રતિજ્ઞા(I < Size);
  જો (get(I).compare_exchange_strong(Existing_ગાંઠ, S.get()))
    પાછા Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  પાછા cast<TrieSubtrie>(Existing_ગાંઠ);
}

વર્ગ ThreadSafeTrieRawHashMapBase::ImplType final
    : ખાનગી TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
જાહેર:
  સ્થિર std::unique_ptr<ImplType> create(size_t StartBit, size_t અંક_Bits_2) {
    size_t Size = કદ(ImplType) + TrieSubtrie::sizeToAlloc(અંક_Bits_2);
    શૂન્ય *Memory = ::operator નવું(Size);
    ImplType *Impl = ::નવું (Memory) ImplType(StartBit, અંક_Bits_2);
    પાછા std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    પ્રતિજ્ઞા(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    જ્યારે (!get_મૂળ()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    પાછા S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_મૂળ() { પાછા getTrailingObjects(); }

  સ્થિર શૂન્ય *operator નવું(size_t Size) { પાછા ::operator નવું(Size); }
  શૂન્ય operator delete(શૂન્ય *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

ખાનગી:
  friend વર્ગ TrailingObjects;

  ImplType(size_t StartBit, size_t અંક_Bits_2) {
    ::નવું (get_મૂળ()) TrieSubtrie(StartBit, અંક_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  જો (ImplType *Impl = ImplPtr.load())
    પાછા *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, અંક_મૂળ_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  જો (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    પાછા *Impl.release();

  // Already created, return the existing ImplType.
  પાછા *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) અચલ {
  પ્રતિજ્ઞા(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  જો (!Impl)
    પાછા PointerBase();

  TrieSubtrie *S = Impl->get_મૂળ();
  TrieHashIndexGenerator IndexGen{અંક_મૂળ_Bits, અંક_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  જ્યારે (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_ગાંઠ *Existing = S->get(Index);
    જો (!Existing)
      પાછા PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    જો (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      પાછા ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_કિંમત_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::ઉમેરો(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<અચલ uint8_t *(શૂન્ય *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  પ્રતિજ્ઞા(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_મૂળ();
  TrieHashIndexGenerator IndexGen{અંક_મૂળ_Bits, અંક_Subtrie_Bits, Hash};
  size_t Index;
  જો (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } નહીંતર {
    Index = IndexGen.next();
  }

  જ્યારે (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    તાર્કિક Generated = ખોટું;
    Trie_ગાંઠ &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = સાચું;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      અચલ uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::નવું (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      પ્રતિજ્ઞા(Hash == Content->getHash() && "Hash not properly initialized");
      પાછા Content;
    });
    // If we just generated it, return it!
    જો (Generated)
      પાછા PointerBase(cast<TrieContent>(Existing).get_કિંમત_Pointer());

    જો (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      ચાલુ;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    જો (ExistingContent.getHash() == Hash)
      પાછા PointerBase(ExistingContent.get_કિંમત_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    જ્યારે (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_અંક_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    પાછા Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      જો (NextIndex != NewIndexForExistingContent)
        તોડો;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> અંક_મૂળ_Bits, std::optional<size_t> અંક_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      અંક_મૂળ_Bits(અંક_મૂળ_Bits.કિંમત_or(Default_અંક_મૂળ_Bits)),
      અંક_Subtrie_Bits(અંક_Subtrie_Bits.કિંમત_or(Default_અંક_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  પ્રતિજ્ઞા((!અંક_મૂળ_Bits || *અંક_મૂળ_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  પ્રતિજ્ઞા((!અંક_Subtrie_Bits || *અંક_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), અંક_મૂળ_Bits(RHS.અંક_મૂળ_Bits),
      અંક_Subtrie_Bits(RHS.અંક_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  પ્રતિજ્ઞા(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

શૂન્ય ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<શૂન્ય(શૂન્ય *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  જો (!Impl)
    પાછા;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  જો (Destructor)
    માટે (TrieSubtrie *Trie = Impl->get_મૂળ(); Trie; Trie = Trie->Next.load())
      માટે (unsigned I = 0; I < Trie->size(); ++I)
        જો (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_કિંમત_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_મૂળ()->Next;
  જ્યારે (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_મૂળ() અચલ {
  ImplType *Impl = ImplPtr.load();
  જો (!Impl)
    પાછા PointerBase();
  પાછા PointerBase(Impl->get_મૂળ());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) અચલ {
  પ્રતિજ્ઞા(!P.isHint() && "Not a valid trie");
  જો (!P.P)
    પાછા 0;
  જો (auto *S = dyn_cast<TrieSubtrie>((Trie_ગાંઠ *)P.P))
    પાછા S->StartBit;
  પાછા 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_અંક_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) અચલ {
  પ્રતિજ્ઞા(!P.isHint() && "Not a valid trie");
  જો (!P.P)
    પાછા 0;
  જો (auto *S = dyn_cast<TrieSubtrie>((Trie_ગાંઠ *)P.P))
    પાછા S->અંક_Bits_2;
  પાછા 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_અંક_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) અચલ {
  પ્રતિજ્ઞા(!P.isHint() && "Not a valid trie");
  જો (!P.P)
    પાછા 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_ગાંઠ *)P.P);
  જો (!S)
    પાછા 0;
  unsigned અંક = 0;
  માટે (unsigned I = 0, E = S->size(); I < E; ++I)
    જો (S->load(I))
      ++અંક;
  પાછા અંક;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) અચલ {
  પ્રતિજ્ઞા(!P.isHint() && "Not a valid trie");
  જો (!P.P)
    પાછા "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_ગાંઠ *)P.P);
  જો (!S || !S->IsSubtrie)
    પાછા "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ગાંઠ = nullptr;
  જ્યારે (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    માટે (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      જો (!S)
        ચાલુ;

      જો (auto *Content = dyn_cast<TrieContent>(S))
        ગાંઠ = Content;
      નહીંતર જો (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      તોડો;
    }

    // Found the node.
    જો (ગાંઠ)
      તોડો;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  પ્રતિજ્ઞા(ગાંઠ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(ગાંઠ->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/સાચું);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  માટે (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.ધકેલો_back('0' + ((ગાંઠ->getHash()[Index] >> Offset) & 1));
  }

  જો (!Bits.empty())
    SS << "[" << Bits << "]";

  પાછા SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_અંક_Tries() અચલ {
  ImplType *Impl = ImplPtr.load();
  જો (!Impl)
    પાછા 0;
  unsigned અંક = 0;
  માટે (TrieSubtrie *Trie = Impl->get_મૂળ(); Trie; Trie = Trie->Next.load())
    ++અંક;
  પાછા અંક;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) અચલ {
  પ્રતિજ્ઞા(!P.isHint() && "Not a valid trie");
  જો (!P.P)
    પાછા PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_ગાંઠ *)P.P);
  જો (!S)
    પાછા PointerBase();
  જો (auto *E = S->Next.load())
    પાછા PointerBase(E);
  પાછા PointerBase();
}
