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
struct TrieNode {
  const bool IsSubtrie = false;

  TrieNode(bool IsSubtrie) : IsSubtrie(IsSubtrie) {}

  static void *operator new(size_t Size) { 返回 ::operator new(Size); }
  void operator delete(void *Ptr) { ::operator delete(Ptr); }
};

struct TrieContent final : public TrieNode {
  const uint8_t ContentOffset;
  const uint8_t HashSize;
  const uint8_t HashOffset;

  void *getValuePointer() const {
    auto *Content = reinterpret_cast<const uint8_t *>(this) + ContentOffset;
    返回 const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() const {
    auto *Begin = reinterpret_cast<const uint8_t *>(this) + HashOffset;
    返回 ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(size_t ContentOffset, size_t HashSize, size_t HashOffset)
      : TrieNode(/*IsSubtrie=*/false), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  static bool classof(const TrieNode *TN) { 返回 !TN->IsSubtrie; }
};

static_assert(sizeof(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

class TrieSubtrie final
    : public TrieNode,
      private TrailingObjects<TrieSubtrie, LazyAtomicPointer<TrieNode>> {
public:
  using Slot = LazyAtomicPointer<TrieNode>;

  Slot &get(size_t I) { 返回 getTrailingObjects()[I]; }
  TrieNode *load(size_t I) { 返回 get(I).load(); }

  unsigned size() const { 返回 Size; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t NumSubtrieBits, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  static std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t NumBits);

  explicit TrieSubtrie(size_t StartBit, size_t NumBits);

  static bool classof(const TrieNode *TN) { 返回 TN->IsSubtrie; }

  static constexpr size_t sizeToAlloc(unsigned NumBits) {
    assert(NumBits < 20 && "Tries should have fewer than ~1M slots");
    unsigned Count = 1u << NumBits;
    返回 totalSizeToAlloc<LazyAtomicPointer<TrieNode>>(Count);
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
  返回 std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t NumBits)
    : TrieNode(true), StartBit(StartBit), NumBits(NumBits), Size(1u << NumBits),
      Next(nullptr) {
  为 (unsigned I = 0; I < Size; ++I)
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
  如果 (get(I).compare_exchange_strong(ExistingNode, S.get()))
    返回 Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  返回 cast<TrieSubtrie>(ExistingNode);
}

class ThreadSafeTrieRawHashMapBase::ImplType final
    : private TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
public:
  static std::unique_ptr<ImplType> create(size_t StartBit, size_t NumBits) {
    size_t Size = sizeof(ImplType) + TrieSubtrie::sizeToAlloc(NumBits);
    void *Memory = ::operator new(Size);
    ImplType *Impl = ::new (Memory) ImplType(StartBit, NumBits);
    返回 std::unique_ptr<ImplType>(Impl);
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
    当 (!getRoot()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    返回 S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *getRoot() { 返回 getTrailingObjects(); }

  static void *operator new(size_t Size) { 返回 ::operator new(Size); }
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
  如果 (ImplType *Impl = ImplPtr.load())
    返回 *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, NumRootBits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  如果 (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    返回 *Impl.release();

  // Already created, return the existing ImplType.
  返回 *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) const {
  assert(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  如果 (!Impl)
    返回 PointerBase();

  TrieSubtrie *S = Impl->getRoot();
  TrieHashIndexGenerator IndexGen{NumRootBits, NumSubtrieBits, Hash};
  size_t Index = IndexGen.next();
  当 (Index != IndexGen.end()) {
    // Try to set the content.
    TrieNode *Existing = S->get(Index);
    如果 (!Existing)
      返回 PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    如果 (auto *ExistingContent = dyn_cast<TrieContent>(Existing))
      返回 ExistingContent->getHash() == Hash
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
  如果 (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } 否则 {
    Index = IndexGen.next();
  }

  当 (Index != IndexGen.end()) {
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
      返回 Content;
    });
    // If we just generated it, return it!
    如果 (Generated)
      返回 PointerBase(cast<TrieContent>(Existing).getValuePointer());

    如果 (auto *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      continue;
    }

    // Return the existing content if it's an exact match!
    auto &ExistingContent = cast<TrieContent>(Existing);
    如果 (ExistingContent.getHash() == Hash)
      返回 PointerBase(ExistingContent.getValuePointer());

    // Sink the existing content as long as the indexes match.
    size_t NextIndex = IndexGen.next();
    当 (NextIndex != IndexGen.end()) {
      size_t NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.getNumBits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    返回 Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      如果 (NextIndex != NewIndexForExistingContent)
        break;

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
  如果 (!Impl)
    返回;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  如果 (Destructor)
    为 (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
      为 (unsigned I = 0; I < Trie->size(); ++I)
        如果 (auto *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->getValuePointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->getRoot()->Next;
  当 (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getRoot() const {
  ImplType *Impl = ImplPtr.load();
  如果 (!Impl)
    返回 PointerBase();
  返回 PointerBase(Impl->getRoot());
}

unsigned ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  如果 (!P.P)
    返回 0;
  如果 (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    返回 S->StartBit;
  返回 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumBits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  如果 (!P.P)
    返回 0;
  如果 (auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P))
    返回 S->NumBits;
  返回 0;
}

unsigned ThreadSafeTrieRawHashMapBase::getNumSlotUsed(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  如果 (!P.P)
    返回 0;
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  如果 (!S)
    返回 0;
  unsigned Num = 0;
  为 (unsigned I = 0, E = S->size(); I < E; ++I)
    如果 (S->load(I))
      ++Num;
  返回 Num;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  如果 (!P.P)
    返回 "";

  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  如果 (!S || !S->IsSubtrie)
    返回 "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *Node = nullptr;
  当 (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    为 (unsigned I = 0, E = Current->size(); I < E; ++I) {
      auto *S = Current->load(I);
      如果 (!S)
        continue;

      如果 (auto *Content = dyn_cast<TrieContent>(S))
        Node = Content;
      否则 如果 (auto *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      break;
    }

    // Found the node.
    如果 (Node)
      break;

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
  为 (unsigned I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    unsigned Index = I / 8;
    unsigned Offset = 7 - I % 8;
    Bits.push_back('0' + ((Node->getHash()[Index] >> Offset) & 1));
  }

  如果 (!Bits.empty())
    SS << "[" << Bits << "]";

  返回 SS.str();
}

unsigned ThreadSafeTrieRawHashMapBase::getNumTries() const {
  ImplType *Impl = ImplPtr.load();
  如果 (!Impl)
    返回 0;
  unsigned Num = 0;
  为 (TrieSubtrie *Trie = Impl->getRoot(); Trie; Trie = Trie->Next.load())
    ++Num;
  返回 Num;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) const {
  assert(!P.isHint() && "Not a valid trie");
  如果 (!P.P)
    返回 PointerBase();
  auto *S = dyn_cast<TrieSubtrie>((TrieNode *)P.P);
  如果 (!S)
    返回 PointerBase();
  如果 (auto *E = S->Next.load())
    返回 PointerBase(E);
  返回 PointerBase();
}
