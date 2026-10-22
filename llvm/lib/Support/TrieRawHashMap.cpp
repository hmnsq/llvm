//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/TrieRawHashMap.h"
#அந்தர்பாவ "llvm/ADT/LazyAtomicPointer.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/TrieHashIndexGenerator.h"
#அந்தர்பாவ "llvm/Support/Allocator.h"
#அந்தர்பாவ "llvm/Support/Casting.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/ThreadSafeAllocator.h"
#அந்தர்பாவ "llvm/Support/TrailingObjects.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <memory>

using நாமாகாஶ llvm;

நாமாகாஶ {
கட்டமைப்பு Trie_முனை {
  மாறிலி தருக்க IsSubtrie = பொய்;

  Trie_முனை(தருக்க IsSubtrie) : IsSubtrie(IsSubtrie) {}

  நிலையான வெறுமை *operator புதிய(size_t Size) { திரும்பு ::operator புதிய(Size); }
  வெறுமை operator delete(வெறுமை *Ptr) { ::operator delete(Ptr); }
};

கட்டமைப்பு TrieContent final : பொது Trie_முனை {
  மாறிலி uint8_t ContentOffset;
  மாறிலி uint8_t HashSize;
  மாறிலி uint8_t HashOffset;

  வெறுமை *get_மதிப்பு_Pointer() மாறிலி {
    auto *Content = reinterpret_cast<மாறிலி uint8_t *>(இது) + ContentOffset;
    திரும்பு const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() மாறிலி {
    auto *Begin = reinterpret_cast<மாறிலி uint8_t *>(இது) + HashOffset;
    திரும்பு ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_முனை(/*IsSubtrie=*/பொய்), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  நிலையான தருக்க classof(மாறிலி Trie_முனை *TN) { திரும்பு !TN->IsSubtrie; }
};

static_assert(அளவு(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

வகுப்பு TrieSubtrie final
    : பொது Trie_முனை,
      தனிப்பட்ட TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_முனை>> {
பொது:
  using Slot = LazyAtomicPointer<Trie_முனை>;

  Slot &get(size_t I) { திரும்பு getTrailingObjects()[I]; }
  Trie_முனை *load(size_t I) { திரும்பு get(I).load(); }

  unsigned size() மாறிலி { திரும்பு Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t இலக்கம்_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  நிலையான std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t இலக்கம்_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t இலக்கம்_Bits_2);

  நிலையான தருக்க classof(மாறிலி Trie_முனை *TN) { திரும்பு TN->IsSubtrie; }

  நிலையான constexpr size_t sizeToAlloc(unsigned இலக்கம்_Bits_2) {
    ப்ரதிஜ்ஞா(இலக்கம்_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned மொத்தம்_2 = 1u << இலக்கம்_Bits_2;
    திரும்பு totalSizeToAlloc<LazyAtomicPointer<Trie_முனை>>(மொத்தம்_2);
  }

தனிப்பட்ட:
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
  unsigned இலக்கம்_Bits_2 = 0;
  unsigned Size = 0;
  friend வகுப்பு llvm::ThreadSafeTrieRawHashMapBase;
  friend வகுப்பு TrailingObjects;

பொது:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t இலக்கம்_Bits_2) {
  வெறுமை *Memory = ::operator புதிய(sizeToAlloc(இலக்கம்_Bits_2));
  TrieSubtrie *S = ::புதிய (Memory) TrieSubtrie(StartBit, இலக்கம்_Bits_2);
  திரும்பு std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t இலக்கம்_Bits_2)
    : Trie_முனை(உண்மை), StartBit(StartBit), இலக்கம்_Bits_2(இலக்கம்_Bits_2), Size(1u << இலக்கம்_Bits_2),
      Next(nullptr) {
  க்கு (unsigned I = 0; I < Size; ++I)
    புதிய (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_முனை>>::மதிப்பு_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t இலக்கம்_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ப்ரதிஜ்ஞா(இலக்கம்_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + இலக்கம்_Bits_2, இலக்கம்_Subtrie_Bits);

  ப்ரதிஜ்ஞா(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_முனை *Existing_முனை = &Content;
  ப்ரதிஜ்ஞா(I < Size);
  எனில் (get(I).compare_exchange_strong(Existing_முனை, S.get()))
    திரும்பு Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  திரும்பு cast<TrieSubtrie>(Existing_முனை);
}

வகுப்பு ThreadSafeTrieRawHashMapBase::ImplType final
    : தனிப்பட்ட TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
பொது:
  நிலையான std::unique_ptr<ImplType> create(size_t StartBit, size_t இலக்கம்_Bits_2) {
    size_t Size = அளவு(ImplType) + TrieSubtrie::sizeToAlloc(இலக்கம்_Bits_2);
    வெறுமை *Memory = ::operator புதிய(Size);
    ImplType *Impl = ::புதிய (Memory) ImplType(StartBit, இலக்கம்_Bits_2);
    திரும்பு std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ப்ரதிஜ்ஞா(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    போது (!get_வேர்()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    திரும்பு S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_வேர்() { திரும்பு getTrailingObjects(); }

  நிலையான வெறுமை *operator புதிய(size_t Size) { திரும்பு ::operator புதிய(Size); }
  வெறுமை operator delete(வெறுமை *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

தனிப்பட்ட:
  friend வகுப்பு TrailingObjects;

  ImplType(size_t StartBit, size_t இலக்கம்_Bits_2) {
    ::புதிய (get_வேர்()) TrieSubtrie(StartBit, இலக்கம்_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  எனில் (ImplType *Impl = ImplPtr.load())
    திரும்பு *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, இலக்கம்_வேர்_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  எனில் (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    திரும்பு *Impl.release();

  // Already created, return the existing ImplType.
  திரும்பு *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) மாறிலி {
  ப்ரதிஜ்ஞா(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  எனில் (!Impl)
    திரும்பு PointerBase();

  TrieSubtrie *S = Impl->get_வேர்();
  TrieHashIndexGenerator IndexGen{இலக்கம்_வேர்_Bits, இலக்கம்_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  போது (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_முனை *Existing = S->get(Index);
    எனில் (!Existing)
      திரும்பு PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    எனில் (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      திரும்பு ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_மதிப்பு_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::செருகு(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<மாறிலி uint8_t *(வெறுமை *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ப்ரதிஜ்ஞா(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_வேர்();
  TrieHashIndexGenerator IndexGen{இலக்கம்_வேர்_Bits, இலக்கம்_Subtrie_Bits, Hash};
  size_t Index;
  எனில் (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } இல்லையெனில் {
    Index = IndexGen.next();
  }

  போது (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    தருக்க Generated = பொய்;
    Trie_முனை &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = உண்மை;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      மாறிலி uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::புதிய (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      ப்ரதிஜ்ஞா(Hash == Content->getHash() && "Hash not properly initialized");
      திரும்பு Content;
    });
    // If we just generated it, return it!
    எனில் (Generated)
      திரும்பு PointerBase(cast<TrieContent>(Existing).get_மதிப்பு_Pointer());

    எனில் (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      தொடர்;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    எனில் (ExistingContent.getHash() == Hash)
      திரும்பு PointerBase(ExistingContent.get_மதிப்பு_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    போது (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_இலக்கம்_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    திரும்பு Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      எனில் (NextIndex != NewIndexForExistingContent)
        நிறுத்து;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> இலக்கம்_வேர்_Bits, std::optional<size_t> இலக்கம்_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      இலக்கம்_வேர்_Bits(இலக்கம்_வேர்_Bits.மதிப்பு_or(Default_இலக்கம்_வேர்_Bits)),
      இலக்கம்_Subtrie_Bits(இலக்கம்_Subtrie_Bits.மதிப்பு_or(Default_இலக்கம்_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ப்ரதிஜ்ஞா((!இலக்கம்_வேர்_Bits || *இலக்கம்_வேர்_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  ப்ரதிஜ்ஞா((!இலக்கம்_Subtrie_Bits || *இலக்கம்_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), இலக்கம்_வேர்_Bits(RHS.இலக்கம்_வேர்_Bits),
      இலக்கம்_Subtrie_Bits(RHS.இலக்கம்_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  ப்ரதிஜ்ஞா(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

வெறுமை ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<வெறுமை(வெறுமை *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  எனில் (!Impl)
    திரும்பு;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  எனில் (Destructor)
    க்கு (TrieSubtrie *Trie = Impl->get_வேர்(); Trie; Trie = Trie->Next.load())
      க்கு (unsigned I = 0; I < Trie->size(); ++I)
        எனில் (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_மதிப்பு_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_வேர்()->Next;
  போது (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_வேர்() மாறிலி {
  ImplType *Impl = ImplPtr.load();
  எனில் (!Impl)
    திரும்பு PointerBase();
  திரும்பு PointerBase(Impl->get_வேர்());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) மாறிலி {
  ப்ரதிஜ்ஞா(!P.isHint() && "Not a valid trie");
  எனில் (!P.P)
    திரும்பு 0;
  எனில் (auto *S = dyn_cast<TrieSubtrie>((Trie_முனை *)P.P))
    திரும்பு S->StartBit;
  திரும்பு 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_இலக்கம்_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) மாறிலி {
  ப்ரதிஜ்ஞா(!P.isHint() && "Not a valid trie");
  எனில் (!P.P)
    திரும்பு 0;
  எனில் (auto *S = dyn_cast<TrieSubtrie>((Trie_முனை *)P.P))
    திரும்பு S->இலக்கம்_Bits_2;
  திரும்பு 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_இலக்கம்_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) மாறிலி {
  ப்ரதிஜ்ஞா(!P.isHint() && "Not a valid trie");
  எனில் (!P.P)
    திரும்பு 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_முனை *)P.P);
  எனில் (!S)
    திரும்பு 0;
  unsigned இலக்கம் = 0;
  க்கு (unsigned I = 0, E = S->size(); I < E; ++I)
    எனில் (S->load(I))
      ++இலக்கம்;
  திரும்பு இலக்கம்;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) மாறிலி {
  ப்ரதிஜ்ஞா(!P.isHint() && "Not a valid trie");
  எனில் (!P.P)
    திரும்பு "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_முனை *)P.P);
  எனில் (!S || !S->IsSubtrie)
    திரும்பு "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *முனை = nullptr;
  போது (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    க்கு (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      எனில் (!S)
        தொடர்;

      எனில் (auto *Content = dyn_cast<TrieContent>(S))
        முனை = Content;
      இல்லையெனில் எனில் (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      நிறுத்து;
    }

    // Found the node.
    எனில் (முனை)
      நிறுத்து;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  ப்ரதிஜ்ஞா(முனை && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(முனை->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/உண்மை);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  க்கு (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.தள்ளு_back('0' + ((முனை->getHash()[Index] >> Offset) & 1));
  }

  எனில் (!Bits.empty())
    SS << "[" << Bits << "]";

  திரும்பு SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_இலக்கம்_Tries() மாறிலி {
  ImplType *Impl = ImplPtr.load();
  எனில் (!Impl)
    திரும்பு 0;
  unsigned இலக்கம் = 0;
  க்கு (TrieSubtrie *Trie = Impl->get_வேர்(); Trie; Trie = Trie->Next.load())
    ++இலக்கம்;
  திரும்பு இலக்கம்;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) மாறிலி {
  ப்ரதிஜ்ஞா(!P.isHint() && "Not a valid trie");
  எனில் (!P.P)
    திரும்பு PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_முனை *)P.P);
  எனில் (!S)
    திரும்பு PointerBase();
  எனில் (auto *E = S->Next.load())
    திரும்பு PointerBase(E);
  திரும்பு PointerBase();
}
