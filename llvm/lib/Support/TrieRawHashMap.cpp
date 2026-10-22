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
কাঠামো Trie_গ্রন্থি {
  ধ্রুবক যৌক্তিক IsSubtrie = মিথ্যা;

  Trie_গ্রন্থি(যৌক্তিক IsSubtrie) : IsSubtrie(IsSubtrie) {}

  স্থির শূন্য *operator নতুন(size_t আকার_2) { ফেরত ::operator নতুন(আকার_2); }
  শূন্য operator delete(শূন্য *Ptr) { ::operator delete(Ptr); }
};

কাঠামো TrieContent final : সর্বজনীন Trie_গ্রন্থি {
  ধ্রুবক uint8_t ContentOffset;
  ধ্রুবক uint8_t Hash_আকার;
  ধ্রুবক uint8_t HashOffset;

  শূন্য *get_মান_Pointer() ধ্রুবক {
    auto *Content = reinterpret_cast<ধ্রুবক uint8_t *>(এই) + ContentOffset;
    ফেরত const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ধ্রুবক {
    auto *Begin = reinterpret_cast<ধ্রুবক uint8_t *>(এই) + HashOffset;
    ফেরত ArrayRef(Begin, Begin + Hash_আকার);
  }

  TrieContent(size_t ContentOffset, size_t Hash_আকার, size_t HashOffset)
      : Trie_গ্রন্থি(/*IsSubtrie=*/মিথ্যা), ContentOffset(ContentOffset),
        Hash_আকার(Hash_আকার), HashOffset(HashOffset) {}

  স্থির যৌক্তিক classof(ধ্রুবক Trie_গ্রন্থি *TN) { ফেরত !TN->IsSubtrie; }
};

static_assert(আকার(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::Trie_Content_Base_আকার,
              "Check header assumption!");

শ্রেণী TrieSubtrie final
    : সর্বজনীন Trie_গ্রন্থি,
      ব্যক্তিগত TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_গ্রন্থি>> {
সর্বজনীন:
  using Slot = LazyAtomicPointer<Trie_গ্রন্থি>;

  Slot &get(size_t I) { ফেরত getTrailingObjects()[I]; }
  Trie_গ্রন্থি *load(size_t I) { ফেরত get(I).load(); }

  unsigned আকার_3() ধ্রুবক { ফেরত আকার_2; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t অঙ্ক_Subtrie_Bits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  স্থির std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t অঙ্ক_Bits_2);

  explicit TrieSubtrie(size_t StartBit, size_t অঙ্ক_Bits_2);

  স্থির যৌক্তিক classof(ধ্রুবক Trie_গ্রন্থি *TN) { ফেরত TN->IsSubtrie; }

  স্থির constexpr size_t আকার_To_Alloc(unsigned অঙ্ক_Bits_2) {
    প্রতিজ্ঞা(অঙ্ক_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    unsigned গুনতি_2 = 1u << অঙ্ক_Bits_2;
    ফেরত যোগ_আকার_To_Alloc<LazyAtomicPointer<Trie_গ্রন্থি>>(গুনতি_2);
  }

ব্যক্তিগত:
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
  unsigned আকার_2 = 0;
  friend শ্রেণী llvm::ThreadSafeTrieRawHashMapBase;
  friend শ্রেণী TrailingObjects;

সর্বজনীন:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> পরবর্তী_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t অঙ্ক_Bits_2) {
  শূন্য *Memory = ::operator নতুন(আকার_To_Alloc(অঙ্ক_Bits_2));
  TrieSubtrie *S = ::নতুন (Memory) TrieSubtrie(StartBit, অঙ্ক_Bits_2);
  ফেরত std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t অঙ্ক_Bits_2)
    : Trie_গ্রন্থি(সত্য), StartBit(StartBit), অঙ্ক_Bits_2(অঙ্ক_Bits_2), আকার_2(1u << অঙ্ক_Bits_2),
      পরবর্তী_2(nullptr) {
  জন্য (unsigned I = 0; I < আকার_2; ++I)
    নতুন (&get(I)) Slot(nullptr);

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

  প্রতিজ্ঞা(NewI < আকার_2);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_গ্রন্থি *Existing_গ্রন্থি = &Content;
  প্রতিজ্ঞা(I < আকার_2);
  যদি (get(I).compare_exchange_strong(Existing_গ্রন্থি, S.get()))
    ফেরত Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ফেরত cast<TrieSubtrie>(Existing_গ্রন্থি);
}

শ্রেণী ThreadSafeTrieRawHashMapBase::ImplType final
    : ব্যক্তিগত TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
সর্বজনীন:
  স্থির std::unique_ptr<ImplType> create(size_t StartBit, size_t অঙ্ক_Bits_2) {
    size_t আকার_2 = আকার(ImplType) + TrieSubtrie::আকার_To_Alloc(অঙ্ক_Bits_2);
    শূন্য *Memory = ::operator নতুন(আকার_2);
    ImplType *Impl = ::নতুন (Memory) ImplType(StartBit, অঙ্ক_Bits_2);
    ফেরত std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    প্রতিজ্ঞা(!S->পরবর্তী_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_শীর্ষ = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    যখন (!get_মূল()->পরবর্তী_2.compare_exchange_weak(Current_শীর্ষ, S.get()))
      S->পরবর্তী_2.exchange(Current_শীর্ষ);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ফেরত S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_মূল() { ফেরত getTrailingObjects(); }

  স্থির শূন্য *operator নতুন(size_t আকার_2) { ফেরত ::operator নতুন(আকার_2); }
  শূন্য operator delete(শূন্য *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

ব্যক্তিগত:
  friend শ্রেণী TrailingObjects;

  ImplType(size_t StartBit, size_t অঙ্ক_Bits_2) {
    ::নতুন (get_মূল()) TrieSubtrie(StartBit, অঙ্ক_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  যদি (ImplType *Impl = ImplPtr.load())
    ফেরত *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, অঙ্ক_মূল_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  যদি (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ফেরত *Impl.release();

  // Already created, return the existing ImplType.
  ফেরত *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ধ্রুবক {
  প্রতিজ্ঞা(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    ফেরত PointerBase();

  TrieSubtrie *S = Impl->get_মূল();
  Trie_Hash_সূচক_Generator সূচক_Gen{অঙ্ক_মূল_Bits, অঙ্ক_Subtrie_Bits, Hash};
  size_t সূচক_2 = সূচক_Gen.পরবর্তী();
  যখন (সূচক_2 != সূচক_Gen.end()) {
    // Try to set the content.
    Trie_গ্রন্থি *Existing = S->get(সূচক_2);
    যদি (!Existing)
      ফেরত PointerBase(S, সূচক_2, *সূচক_Gen.StartBit);

    // Check for an exact match.
    যদি (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      ফেরত ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_মান_Pointer())
                 : PointerBase(S, সূচক_2, *সূচক_Gen.StartBit);

    সূচক_2 = সূচক_Gen.পরবর্তী();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::ঢোকাও(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ধ্রুবক uint8_t *(শূন্য *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  প্রতিজ্ঞা(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_মূল();
  Trie_Hash_সূচক_Generator সূচক_Gen{অঙ্ক_মূল_Bits, অঙ্ক_Subtrie_Bits, Hash};
  size_t সূচক_2;
  যদি (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    সূচক_2 = সূচক_Gen.hint(Hint.I, Hint.B);
  } অন্যথা {
    সূচক_2 = সূচক_Gen.পরবর্তী();
  }

  যখন (সূচক_2 != সূচক_Gen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    যৌক্তিক Generated = মিথ্যা;
    Trie_গ্রন্থি &Existing = S->get(সূচক_2).loadOrGenerate([&]() {
      Generated = সত্য;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_আকার, ContentAllocAlign));
      ধ্রুবক uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::নতুন (Memory)
          TrieContent(ContentOffset, Hash.আকার_3(), HashStorage - Memory);
      প্রতিজ্ঞা(Hash == Content->getHash() && "Hash not properly initialized");
      ফেরত Content;
    });
    // If we just generated it, return it!
    যদি (Generated)
      ফেরত PointerBase(cast<TrieContent>(Existing).get_মান_Pointer());

    যদি (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      সূচক_2 = সূচক_Gen.পরবর্তী();
      চালু;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    যদি (ExistingContent.getHash() == Hash)
      ফেরত PointerBase(ExistingContent.get_মান_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t পরবর্তী_সূচক = সূচক_Gen.পরবর্তী();
    যখন (পরবর্তী_সূচক != সূচক_Gen.end()) {
      size_t New_সূচক_For_Existing_Content =
          সূচক_Gen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(সূচক_2, ExistingContent, সূচক_Gen.get_অঙ্ক_Bits(),
                  New_সূচক_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ফেরত Impl.save(std::move(S));
                  });
      সূচক_2 = পরবর্তী_সূচক;

      // Found the difference.
      যদি (পরবর্তী_সূচক != New_সূচক_For_Existing_Content)
        বিরতি;

      পরবর্তী_সূচক = সূচক_Gen.পরবর্তী();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    size_t Content_Alloc_আকার, size_t ContentAllocAlign, size_t ContentOffset,
    std::optional<size_t> অঙ্ক_মূল_Bits, std::optional<size_t> অঙ্ক_Subtrie_Bits)
    : Content_Alloc_আকার(Content_Alloc_আকার), ContentAllocAlign(ContentAllocAlign),
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
    : Content_Alloc_আকার(RHS.Content_Alloc_আকার),
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
    ফেরত;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  যদি (Destructor)
    জন্য (TrieSubtrie *Trie = Impl->get_মূল(); Trie; Trie = Trie->পরবর্তী_2.load())
      জন্য (unsigned I = 0; I < Trie->আকার_3(); ++I)
        যদি (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_মান_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_মূল()->পরবর্তী_2;
  যখন (Trie) {
    TrieSubtrie *পরবর্তী_2 = Trie->পরবর্তী_2.exchange(nullptr);
    delete Trie;
    Trie = পরবর্তী_2;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_মূল() ধ্রুবক {
  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    ফেরত PointerBase();
  ফেরত PointerBase(Impl->get_মূল());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুবক {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    ফেরত 0;
  যদি (auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P))
    ফেরত S->StartBit;
  ফেরত 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুবক {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    ফেরত 0;
  যদি (auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P))
    ফেরত S->অঙ্ক_Bits_2;
  ফেরত 0;
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুবক {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    ফেরত 0;
  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S)
    ফেরত 0;
  unsigned অঙ্ক = 0;
  জন্য (unsigned I = 0, E = S->আকার_3(); I < E; ++I)
    যদি (S->load(I))
      ++অঙ্ক;
  ফেরত অঙ্ক;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুবক {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    ফেরত "";

  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S || !S->IsSubtrie)
    ফেরত "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *গ্রন্থি = nullptr;
  যখন (Current) {
    TrieSubtrie *পরবর্তী_2 = nullptr;
    // Find first used slot in the trie.
    জন্য (unsigned I = 0, E = Current->আকার_3(); I < E; ++I) {
      auto *S = Current->load(I);
      যদি (!S)
        চালু;

      যদি (auto *Content = dyn_cast<TrieContent>(S))
        গ্রন্থি = Content;
      অন্যথা যদি (auto *Sub = dyn_cast<TrieSubtrie>(S))
        পরবর্তী_2 = Sub;
      বিরতি;
    }

    // Found the node.
    যদি (গ্রন্থি)
      বিরতি;

    // Continue to the next level if the node is not found.
    Current = পরবর্তী_2;
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
  জন্য (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned সূচক_2 = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.ঠেলো_back('0' + ((গ্রন্থি->getHash()[সূচক_2] >> Offset) & 1));
  }

  যদি (!Bits.empty())
    SS << "[" << Bits << "]";

  ফেরত SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::get_অঙ্ক_Tries() ধ্রুবক {
  ImplType *Impl = ImplPtr.load();
  যদি (!Impl)
    ফেরত 0;
  unsigned অঙ্ক = 0;
  জন্য (TrieSubtrie *Trie = Impl->get_মূল(); Trie; Trie = Trie->পরবর্তী_2.load())
    ++অঙ্ক;
  ফেরত অঙ্ক;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_পরবর্তী_Trie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ধ্রুবক {
  প্রতিজ্ঞা(!P.isHint() && "Not a valid trie");
  যদি (!P.P)
    ফেরত PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((Trie_গ্রন্থি *)P.P);
  যদি (!S)
    ফেরত PointerBase();
  যদি (auto *E = S->পরবর্তী_2.load())
    ফেরত PointerBase(E);
  ফেরত PointerBase();
}
