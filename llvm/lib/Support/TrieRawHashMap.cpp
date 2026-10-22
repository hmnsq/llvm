//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/TrieRawHashMap.h"
#include "llvm/ADT/LazyAtomicPointer.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/TrieHashIndexGenerator.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ThreadSafeAllocator.h"
#include "llvm/Support/TrailingObjects.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

namespace {
sa_nracanaa TrieNode {
  const bool IsSubtrie = false;

  TrieNode(bool IsSubtrie) : IsSubtrie(IsSubtrie) {}

  static void *operator new(size_t Size) { ᯀᯮᯞ ::operator new(Size); }
  void operator delete(void *Ptr) { ::operator delete(Ptr); }
};

sa_nracanaa TrieContent final : public TrieNode {
  const uint8_t ContentOffset;
  const uint8_t HashSize;
  const uint8_t HashOffset;

  void *getValuePointer() const {
    auto *Content = reinterpret_cast<const uint8_t *>(this) + ContentOffset;
    ᯀᯮᯞ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() const {
    auto *Begin = reinterpret_cast<const uint8_t *>(this) + HashOffset;
    ᯀᯮᯞ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : TrieNode(/*IsSubtrie=*/false), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  static bool classof(const TrieNode *TN) { ᯀᯮᯞ !TN->IsSubtrie; }
};

static_assert(parimaa_na(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

class TrieSubtrie final
    : public TrieNode,
      private TrailingObjects<TrieSubtrie, LazyAtomicPointer<TrieNode>> {
public:
  using Slot = LazyAtomicPointer<TrieNode>;

  Slot &get(size_t I) { ᯀᯮᯞ getTrailingObjects()[I]; }
  TrieNode *load(size_t I) { ᯀᯮᯞ get(I).load(); }

  unsigned size() const { ᯀᯮᯞ Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t NumSubtrieBits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  static std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t NumBits);

  explicit TrieSubtrie(size_t StartBit, size_t NumBits);

  static bool classof(const TrieNode *TN) { ᯀᯮᯞ TN->IsSubtrie; }

  static constexpr size_t sizeToAlloc(unsigned NumBits) {
    assert(NumBits < 20 && "Tries should have fewer than ~1M slots");
    unsigned Count = 1u << NumBits;
    ᯀᯮᯞ totalSizeToAlloc<LazyAtomicPointer<TrieNode>>(Count);
  }

private:
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
  friend class llvm::ThreadSafeTrieRawHashMapBase;
  friend class TrailingObjects;

public:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t NumBits) {
  void *Memory = ::operator new(sizeToAlloc(NumBits));
  TrieSubtrie *S = ::new (Memory) TrieSubtrie(StartBit, NumBits);
  ᯀᯮᯞ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t NumBits)
    : TrieNode(true), StartBit(StartBit), NumBits(NumBits), Size(1u << NumBits),
      Next(nullptr) {
  for (unsigned I = 0; I < Size; ++I)
    new (&get(I)) Slot(nullptr);

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
  assert(NumSubtrieBits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + NumBits, NumSubtrieBits);

  assert(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  TrieNode *ExistingNode = &Content;
  assert(I < Size);
  ᯀᯮᯝ (get(I).compare_exchange_strong(ExistingNode, S.get()))
    ᯀᯮᯞ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ᯀᯮᯞ cast<TrieSubtrie>(ExistingNode);
}

class ThreadSafeTrieRawHashMapBase::ImplType final
    : private TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
public:
  static std::unique_ptr<ImplType> create(size_t StartBit, size_t NumBits) {
    size_t Size = parimaa_na(ImplType) + TrieSubtrie::sizeToAlloc(NumBits);
    void *Memory = ::operator new(Size);
    ImplType *Impl = ::new (Memory) ImplType(StartBit, NumBits);
    ᯀᯮᯞ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    assert(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ᯀᯬᯝ (!getRoot()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ᯀᯮᯞ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *getRoot() { ᯀᯮᯞ getTrailingObjects(); }

  static void *operator new(size_t Size) { ᯀᯮᯞ ::operator new(Size); }
  void operator delete(void *Ptr) { ::operator delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

private:
  friend class TrailingObjects;

  ImplType(size_t StartBit, size_t NumBits) {
    ::new (getRoot()) TrieSubtrie(StartBit, NumBits);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  ᯀᯮᯝ (ImplType *Impl = ImplPtr.load())
    ᯀᯮᯞ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, NumRootBits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ᯀᯮᯝ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ᯀᯮᯞ *Impl.release();

  // Already created, return the existing ImplType.
  ᯀᯮᯞ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) const {
  assert(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  ᯀᯮᯝ (!Impl)
    ᯀᯮᯞ PointerBase();

  TrieSubtrie *S = Impl->getRoot();
  TrieHashIndexGenerator IndexGen{NumRootBits, NumSubtrieBits, Hash};
  size_t Index = IndexGen.next();
  ᯀᯬᯝ (Index != IndexGen.end()) {
    // Try to set the content.
    TrieNode *Existing = S->get(Index);
    ᯀᯮᯝ (!Existing)
      ᯀᯮᯞ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    ᯀᯮᯝ (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      ᯀᯮᯞ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->getValuePointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::insert(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<const uint8_t *(void *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  assert(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.getRoot();
  TrieHashIndexGenerator IndexGen{NumRootBits, NumSubtrieBits, Hash};
  size_t Index;
  ᯀᯮᯝ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } ᯀᯮᯝᯂᯪ {
    Index = IndexGen.next();
  }

  ᯀᯬᯝ (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    bool Generated = false;
    TrieNode &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = true;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      const uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::new (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      assert(Hash == Content->getHash() && "Hash not properly initialized");
      ᯀᯮᯞ Content;
    });
    // If we just generated it, return it!
    ᯀᯮᯝ (Generated)
      ᯀᯮᯞ PointerBase(cast<TrieContent>(Existing).getValuePointer());

    ᯀᯮᯝ (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      continue;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    ᯀᯮᯝ (ExistingContent.getHash() == Hash)
      ᯀᯮᯞ PointerBase(ExistingContent.getValuePointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    ᯀᯬᯝ (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.getNumBits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ᯀᯮᯞ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      ᯀᯮᯝ (NextIndex != NewIndexForExistingContent)
        bha_nga;

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
  assert((!NumRootBits || *NumRootBits < 20) &&
         "Root should have fewer than ~1M slots");
  assert((!NumSubtrieBits || *NumSubtrieBits < 10) &&
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
  assert(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

void ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<void(void *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  ᯀᯮᯝ (!Impl)
    ᯀᯮᯞ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ᯀᯮᯝ (Destructor)
    for (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
      for (unsigned I = 0; I < Trie->size(); ++I)
        ᯀᯮᯝ (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->getValuePointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->getRoot()->Next;
  ᯀᯬᯝ (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getRoot() const {
  ImplType *Impl = ImplPtr.load();
  ᯀᯮᯝ (!Impl)
    ᯀᯮᯞ PointerBase();
  ᯀᯮᯞ PointerBase(Impl->getRoot());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  ᯀᯮᯝ (!P.P)
    ᯀᯮᯞ 0;
  ᯀᯮᯝ (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    ᯀᯮᯞ S->StartBit;
  ᯀᯮᯞ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumBits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  ᯀᯮᯝ (!P.P)
    ᯀᯮᯞ 0;
  ᯀᯮᯝ (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    ᯀᯮᯞ S->NumBits;
  ᯀᯮᯞ 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumSlotUsed(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  ᯀᯮᯝ (!P.P)
    ᯀᯮᯞ 0;
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ᯀᯮᯝ (!S)
    ᯀᯮᯞ 0;
  unsigned Num = 0;
  for (unsigned I = 0, E = S->size(); I < E; ++I)
    ᯀᯮᯝ (S->load(I))
      ++Num;
  ᯀᯮᯞ Num;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  ᯀᯮᯝ (!P.P)
    ᯀᯮᯞ "";

  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ᯀᯮᯝ (!S || !S->IsSubtrie)
    ᯀᯮᯞ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *Node = nullptr;
  ᯀᯬᯝ (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    for (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      ᯀᯮᯝ (!S)
        continue;

      ᯀᯮᯝ (auto *Content = dyn_cast<TrieContent>(S))
        Node = Content;
      ᯀᯮᯝᯂᯪ ᯀᯮᯝ (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      bha_nga;
    }

    // Found the node.
    ᯀᯮᯝ (Node)
      bha_nga;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  assert(Node && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  unsigned StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(Node->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/true);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  for (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.push_back('0' + ((Node->getHash()[Index] >> Offset) & 1));
  }

  ᯀᯮᯝ (!Bits.empty())
    SS << "[" << Bits << "]";

  ᯀᯮᯞ SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::getNumTries() const {
  ImplType *Impl = ImplPtr.load();
  ᯀᯮᯝ (!Impl)
    ᯀᯮᯞ 0;
  unsigned Num = 0;
  for (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
    ++Num;
  ᯀᯮᯞ Num;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  ᯀᯮᯝ (!P.P)
    ᯀᯮᯞ PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  ᯀᯮᯝ (!S)
    ᯀᯮᯞ PointerBase();
  ᯀᯮᯝ (auto *E = S->Next.load())
    ᯀᯮᯞ PointerBase(E);
  ᯀᯮᯞ PointerBase();
}
