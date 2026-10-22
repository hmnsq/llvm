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
بنية Trie_نود {
  ثابت منطقي IsSubtrie = خطأ;

  Trie_نود(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *operator جديد(size_t Size) { أرجع ::operator جديد(Size); }
  فراغ operator delete(فراغ *Ptr) { ::operator delete(Ptr); }
};

بنية TrieContent final : عام Trie_نود {
  ثابت uint8_t ContentOffset;
  ثابت uint8_t HashSize;
  ثابت uint8_t HashOffset;

  فراغ *get_نيلاي_Pointer() ثابت {
    auto *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + ContentOffset;
    أرجع const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    auto *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + HashOffset;
    أرجع ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_نود(/*IsSubtrie=*/خطأ), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  ساكن منطقي classof(ثابت Trie_نود *TN) { أرجع !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام Trie_نود,
      خاص TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_نود>> {
عام:
  using Slot = LazyAtomicPointer<Trie_نود>;

  Slot &get(size_t I) { أرجع getTrailingObjects()[I]; }
  Trie_نود *load(size_t I) { أرجع get(I).load(); }

  unsigned size() ثابت { أرجع Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t اڠک_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t اڠک_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t اڠک_Bits_2);

  ساكن منطقي classof(ثابت Trie_نود *TN) { أرجع TN->IsSubtrie; }

  ساكن constexpr size_t sizeToAlloc(unsigned اڠک_Bits_2) {
    أكد(اڠک_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned کيراءن_2 = 1u << اڠک_Bits_2;
    أرجع totalSizeToAlloc<LazyAtomicPointer<Trie_نود>>(کيراءن_2);
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
  unsigned اڠک_Bits_2 = 0;
  unsigned Size = 0;
  friend صنف llvm::ThreadSafeTrieRawHashMapBase;
  friend صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t اڠک_Bits_2) {
  فراغ *Memory = ::operator جديد(sizeToAlloc(اڠک_Bits_2));
  TrieSubtrie *S = ::جديد (Memory) TrieSubtrie(StartBit, اڠک_Bits_2);
  أرجع std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t اڠک_Bits_2)
    : Trie_نود(true), StartBit(StartBit), اڠک_Bits_2(اڠک_Bits_2), Size(1u << اڠک_Bits_2),
      Next(nullptr) {
  لكل (unsigned I = 0; I < Size; ++I)
    جديد (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_نود>>::نيلاي_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t اڠک_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(اڠک_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + اڠک_Bits_2, اڠک_Subtrie_Bits);

  أكد(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_نود *Existing_نود = &Content;
  أكد(I < Size);
  إذا (get(I).compare_exchange_strong(Existing_نود, S.get()))
    أرجع Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  أرجع cast<TrieSubtrie>(Existing_نود);
}

صنف ThreadSafeTrieRawHashMapBase::ImplType final
    : خاص TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t StartBit, size_t اڠک_Bits_2) {
    size_t Size = حجم(ImplType) + TrieSubtrie::sizeToAlloc(اڠک_Bits_2);
    فراغ *Memory = ::operator جديد(Size);
    ImplType *Impl = ::جديد (Memory) ImplType(StartBit, اڠک_Bits_2);
    أرجع std::unique_ptr<ImplType>(Impl);
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
    طالما (!get_اکر()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    أرجع S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_اکر() { أرجع getTrailingObjects(); }

  ساكن فراغ *operator جديد(size_t Size) { أرجع ::operator جديد(Size); }
  فراغ operator delete(فراغ *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

خاص:
  friend صنف TrailingObjects;

  ImplType(size_t StartBit, size_t اڠک_Bits_2) {
    ::جديد (get_اکر()) TrieSubtrie(StartBit, اڠک_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  إذا (ImplType *Impl = ImplPtr.load())
    أرجع *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, اڠک_اکر_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  إذا (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    أرجع *Impl.release();

  // Already created, return the existing ImplType.
  أرجع *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  إذا (!Impl)
    أرجع PointerBase();

  TrieSubtrie *S = Impl->get_اکر();
  TrieHashIndexGenerator IndexGen{اڠک_اکر_Bits, اڠک_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  طالما (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_نود *Existing = S->get(Index);
    إذا (!Existing)
      أرجع PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    إذا (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      أرجع ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_نيلاي_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::سيسيڤ(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_اکر();
  TrieHashIndexGenerator IndexGen{اڠک_اکر_Bits, اڠک_Subtrie_Bits, Hash};
  size_t Index;
  إذا (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } وإلا {
    Index = IndexGen.next();
  }

  طالما (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = خطأ;
    Trie_نود &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = true;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      أرجع Content;
    });
    // If we just generated it, return it!
    إذا (Generated)
      أرجع PointerBase(cast<TrieContent>(Existing).get_نيلاي_Pointer());

    إذا (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      استمر;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    إذا (ExistingContent.getHash() == Hash)
      أرجع PointerBase(ExistingContent.get_نيلاي_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    طالما (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_اڠک_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    أرجع Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      إذا (NextIndex != NewIndexForExistingContent)
        اكسر;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> اڠک_اکر_Bits, std::optional<size_t> اڠک_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      اڠک_اکر_Bits(اڠک_اکر_Bits.نيلاي_or(Default_اڠک_اکر_Bits)),
      اڠک_Subtrie_Bits(اڠک_Subtrie_Bits.نيلاي_or(Default_اڠک_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!اڠک_اکر_Bits || *اڠک_اکر_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!اڠک_Subtrie_Bits || *اڠک_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), اڠک_اکر_Bits(RHS.اڠک_اکر_Bits),
      اڠک_Subtrie_Bits(RHS.اڠک_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  أكد(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

فراغ ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  إذا (!Impl)
    أرجع;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  إذا (Destructor)
    لكل (TrieSubtrie *Trie = Impl->get_اکر(); Trie; Trie = Trie->Next.load())
      لكل (unsigned I = 0; I < Trie->size(); ++I)
        إذا (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_نيلاي_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_اکر()->Next;
  طالما (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_اکر() ثابت {
  ImplType *Impl = ImplPtr.load();
  إذا (!Impl)
    أرجع PointerBase();
  أرجع PointerBase(Impl->get_اکر());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (auto *S = dyn_cast<TrieSubtrie>((Trie_نود *)P.P))
    أرجع S->StartBit;
  أرجع 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_اڠک_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (auto *S = dyn_cast<TrieSubtrie>((Trie_نود *)P.P))
    أرجع S->اڠک_Bits_2;
  أرجع 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_اڠک_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_نود *)P.P);
  إذا (!S)
    أرجع 0;
  unsigned اڠک = 0;
  لكل (unsigned I = 0, E = S->size(); I < E; ++I)
    إذا (S->load(I))
      ++اڠک;
  أرجع اڠک;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_نود *)P.P);
  إذا (!S || !S->IsSubtrie)
    أرجع "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *نود = nullptr;
  طالما (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    لكل (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      إذا (!S)
        استمر;

      إذا (auto *Content = dyn_cast<TrieContent>(S))
        نود = Content;
      وإلا إذا (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      اكسر;
    }

    // Found the node.
    إذا (نود)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  أكد(نود && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(نود->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/true);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  لكل (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.تولق_back('0' + ((نود->getHash()[Index] >> Offset) & 1));
  }

  إذا (!Bits.empty())
    SS << "[" << Bits << "]";

  أرجع SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_اڠک_Tries() ثابت {
  ImplType *Impl = ImplPtr.load();
  إذا (!Impl)
    أرجع 0;
  unsigned اڠک = 0;
  لكل (TrieSubtrie *Trie = Impl->get_اکر(); Trie; Trie = Trie->Next.load())
    ++اڠک;
  أرجع اڠک;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_نود *)P.P);
  إذا (!S)
    أرجع PointerBase();
  إذا (auto *E = S->Next.load())
    أرجع PointerBase(E);
  أرجع PointerBase();
}
