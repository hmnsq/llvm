//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/TrieRawHashMap.h"
#شامل "llvm/ADT/LazyAtomicPointer.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/TrieHashIndexGenerator.h"
#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/ThreadSafeAllocator.h"
#شامل "llvm/Support/TrailingObjects.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <memory>

using فضاینام llvm;

فضاینام {
ساختار Trie_گره {
  ثابت منطقی IsSubtrie = نادرست;

  Trie_گره(منطقی IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ایستا تهی *operator جدید(size_t Size) { بازگشت ::operator جدید(Size); }
  تهی operator delete(تهی *Ptr) { ::operator delete(Ptr); }
};

ساختار TrieContent final : عمومی Trie_گره {
  ثابت uint8_t ContentOffset;
  ثابت uint8_t HashSize;
  ثابت uint8_t HashOffset;

  تهی *get_مقدار_Pointer() ثابت {
    auto *Content = reinterpret_cast<ثابت uint8_t *>(این) + ContentOffset;
    بازگشت const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    auto *Begin = reinterpret_cast<ثابت uint8_t *>(این) + HashOffset;
    بازگشت ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : Trie_گره(/*IsSubtrie=*/نادرست), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  ایستا منطقی classof(ثابت Trie_گره *TN) { بازگشت !TN->IsSubtrie; }
};

static_assert(اندازه(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

رده TrieSubtrie final
    : عمومی Trie_گره,
      خصوصی TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_گره>> {
عمومی:
  using Slot = LazyAtomicPointer<Trie_گره>;

  Slot &get(size_t I) { بازگشت getTrailingObjects()[I]; }
  Trie_گره *load(size_t I) { بازگشت get(I).load(); }

  unsigned size() ثابت { بازگشت Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t رقم_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ایستا std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t رقم_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t رقم_Bits_2);

  ایستا منطقی classof(ثابت Trie_گره *TN) { بازگشت TN->IsSubtrie; }

  ایستا constexpr size_t sizeToAlloc(unsigned رقم_Bits_2) {
    أكد(رقم_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned شمار_2 = 1u << رقم_Bits_2;
    بازگشت totalSizeToAlloc<LazyAtomicPointer<Trie_گره>>(شمار_2);
  }

خصوصی:
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
  unsigned رقم_Bits_2 = 0;
  unsigned Size = 0;
  friend رده llvm::ThreadSafeTrieRawHashMapBase;
  friend رده TrailingObjects;

عمومی:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t رقم_Bits_2) {
  تهی *Memory = ::operator جدید(sizeToAlloc(رقم_Bits_2));
  TrieSubtrie *S = ::جدید (Memory) TrieSubtrie(StartBit, رقم_Bits_2);
  بازگشت std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t رقم_Bits_2)
    : Trie_گره(درست), StartBit(StartBit), رقم_Bits_2(رقم_Bits_2), Size(1u << رقم_Bits_2),
      Next(nullptr) {
  برای (unsigned I = 0; I < Size; ++I)
    جدید (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_گره>>::مقدار_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t رقم_Subtrie_Bits, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(رقم_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + رقم_Bits_2, رقم_Subtrie_Bits);

  أكد(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_گره *Existing_گره = &Content;
  أكد(I < Size);
  اگر (get(I).compare_exchange_strong(Existing_گره, S.get()))
    بازگشت Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  بازگشت cast<TrieSubtrie>(Existing_گره);
}

رده ThreadSafeTrieRawHashMapBase::ImplType final
    : خصوصی TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
عمومی:
  ایستا std::unique_ptr<ImplType> create(size_t StartBit, size_t رقم_Bits_2) {
    size_t Size = اندازه(ImplType) + TrieSubtrie::sizeToAlloc(رقم_Bits_2);
    تهی *Memory = ::operator جدید(Size);
    ImplType *Impl = ::جدید (Memory) ImplType(StartBit, رقم_Bits_2);
    بازگشت std::unique_ptr<ImplType>(Impl);
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
    وقتی (!get_ریشه()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    بازگشت S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ریشه() { بازگشت getTrailingObjects(); }

  ایستا تهی *operator جدید(size_t Size) { بازگشت ::operator جدید(Size); }
  تهی operator delete(تهی *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

خصوصی:
  friend رده TrailingObjects;

  ImplType(size_t StartBit, size_t رقم_Bits_2) {
    ::جدید (get_ریشه()) TrieSubtrie(StartBit, رقم_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  اگر (ImplType *Impl = ImplPtr.load())
    بازگشت *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, رقم_ریشه_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  اگر (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    بازگشت *Impl.release();

  // Already created, return the existing ImplType.
  بازگشت *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  اگر (!Impl)
    بازگشت PointerBase();

  TrieSubtrie *S = Impl->get_ریشه();
  TrieHashIndexGenerator IndexGen{رقم_ریشه_Bits, رقم_Subtrie_Bits, Hash};
  size_t Index = IndexGen.next();
  وقتی (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_گره *Existing = S->get(Index);
    اگر (!Existing)
      بازگشت PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    اگر (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      بازگشت ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_مقدار_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::درج(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(تهی *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ریشه();
  TrieHashIndexGenerator IndexGen{رقم_ریشه_Bits, رقم_Subtrie_Bits, Hash};
  size_t Index;
  اگر (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } وگرنه {
    Index = IndexGen.next();
  }

  وقتی (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقی Generated = نادرست;
    Trie_گره &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = درست;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جدید (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      بازگشت Content;
    });
    // If we just generated it, return it!
    اگر (Generated)
      بازگشت PointerBase(cast<TrieContent>(Existing).get_مقدار_Pointer());

    اگر (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      ادامه;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    اگر (ExistingContent.getHash() == Hash)
      بازگشت PointerBase(ExistingContent.get_مقدار_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    وقتی (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_رقم_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    بازگشت Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      اگر (NextIndex != NewIndexForExistingContent)
        بشکن;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t ContentAllocSize, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> رقم_ریشه_Bits, std::optional<size_t> رقم_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      رقم_ریشه_Bits(رقم_ریشه_Bits.مقدار_or(Default_رقم_ریشه_Bits)),
      رقم_Subtrie_Bits(رقم_Subtrie_Bits.مقدار_or(Default_رقم_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!رقم_ریشه_Bits || *رقم_ریشه_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!رقم_Subtrie_Bits || *رقم_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), رقم_ریشه_Bits(RHS.رقم_ریشه_Bits),
      رقم_Subtrie_Bits(RHS.رقم_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  أكد(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

تهی ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<تهی(تهی *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  اگر (!Impl)
    بازگشت;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  اگر (Destructor)
    برای (TrieSubtrie *Trie = Impl->get_ریشه(); Trie; Trie = Trie->Next.load())
      برای (unsigned I = 0; I < Trie->size(); ++I)
        اگر (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_مقدار_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ریشه()->Next;
  وقتی (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_ریشه() ثابت {
  ImplType *Impl = ImplPtr.load();
  اگر (!Impl)
    بازگشت PointerBase();
  بازگشت PointerBase(Impl->get_ریشه());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    بازگشت 0;
  اگر (auto *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P))
    بازگشت S->StartBit;
  بازگشت 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_رقم_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    بازگشت 0;
  اگر (auto *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P))
    بازگشت S->رقم_Bits_2;
  بازگشت 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_رقم_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    بازگشت 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  اگر (!S)
    بازگشت 0;
  unsigned رقم = 0;
  برای (unsigned I = 0, E = S->size(); I < E; ++I)
    اگر (S->load(I))
      ++رقم;
  بازگشت رقم;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    بازگشت "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  اگر (!S || !S->IsSubtrie)
    بازگشت "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *گره = nullptr;
  وقتی (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    برای (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      اگر (!S)
        ادامه;

      اگر (auto *Content = dyn_cast<TrieContent>(S))
        گره = Content;
      وگرنه اگر (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      بشکن;
    }

    // Found the node.
    اگر (گره)
      بشکن;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  أكد(گره && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(گره->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/درست);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  برای (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.افزودن_back('0' + ((گره->getHash()[Index] >> Offset) & 1));
  }

  اگر (!Bits.empty())
    SS << "[" << Bits << "]";

  بازگشت SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_رقم_Tries() ثابت {
  ImplType *Impl = ImplPtr.load();
  اگر (!Impl)
    بازگشت 0;
  unsigned رقم = 0;
  برای (TrieSubtrie *Trie = Impl->get_ریشه(); Trie; Trie = Trie->Next.load())
    ++رقم;
  بازگشت رقم;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    بازگشت PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  اگر (!S)
    بازگشت PointerBase();
  اگر (auto *E = S->Next.load())
    بازگشت PointerBase(E);
  بازگشت PointerBase();
}
