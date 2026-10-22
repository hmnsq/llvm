//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/TrieRawHashMap.h"
#समावेश "llvm/ADT/LazyAtomicPointer.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/TrieHashIndexGenerator.h"
#समावेश "llvm/Support/Allocator.h"
#समावेश "llvm/Support/Casting.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/ThreadSafeAllocator.h"
#समावेश "llvm/Support/TrailingObjects.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <memory>

using namespace llvm;

namespace {
काष्ठा Trie_गाँठ {
  स्थिर bool IsSubtrie = false;

  Trie_गाँठ(bool IsSubtrie) : IsSubtrie(IsSubtrie) {}

  अटल व्योम *चालक नया(माप_प्रकार आकार) { वापस ::चालक नया(आकार); }
  व्योम चालक मिटाओ(व्योम *Ptr) { ::चालक मिटाओ(Ptr); }
};

काष्ठा TrieContent final : खुला Trie_गाँठ {
  स्थिर uint8_t ContentOffset;
  स्थिर uint8_t Hash_आकार;
  स्थिर uint8_t HashOffset;

  व्योम *get_मूल्य_Pointer() स्थिर {
    स्वतः *Content = reinterpret_cast<स्थिर uint8_t *>(यह) + ContentOffset;
    वापस const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() स्थिर {
    स्वतः *Begin = reinterpret_cast<स्थिर uint8_t *>(यह) + HashOffset;
    वापस ArrayRef(Begin, Begin + Hash_आकार);
  }

  TrieContent(माप_प्रकार ContentOffset, माप_प्रकार Hash_आकार, माप_प्रकार HashOffset)
      : Trie_गाँठ(/*IsSubtrie=*/false), ContentOffset(ContentOffset),
        Hash_आकार(Hash_आकार), HashOffset(HashOffset) {}

  अटल bool classof(स्थिर Trie_गाँठ *TN) { वापस !TN->IsSubtrie; }
};

static_assert(माप(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::Trie_Content_Base_आकार,
              "Check header assumption!");

श्रेणी TrieSubtrie final
    : खुला Trie_गाँठ,
      निजी TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_गाँठ>> {
खुला:
  using Slot = LazyAtomicPointer<Trie_गाँठ>;

  Slot &लो(माप_प्रकार I) { वापस getTrailingObjects()[I]; }
  Trie_गाँठ *load(माप_प्रकार I) { वापस लो(I).load(); }

  अचिन्हित आकार_2() स्थिर { वापस आकार; }

  TrieSubtrie *
  sink(माप_प्रकार I, TrieContent &Content, माप_प्रकार अंक_Subtrie_Bits, माप_प्रकार NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  अटल std::unique_ptr<TrieSubtrie> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  explicit TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  अटल bool classof(स्थिर Trie_गाँठ *TN) { वापस TN->IsSubtrie; }

  अटल constexpr माप_प्रकार आकार_To_Alloc(अचिन्हित अंक_Bits_2) {
    निश्चित(अंक_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    अचिन्हित गिनती_2 = 1u << अंक_Bits_2;
    वापस योग_आकार_To_Alloc<LazyAtomicPointer<Trie_गाँठ>>(गिनती_2);
  }

निजी:
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
  अचिन्हित StartBit = 0;
  अचिन्हित अंक_Bits_2 = 0;
  अचिन्हित आकार = 0;
  मित्र श्रेणी llvm::ThreadSafeTrieRawHashMapBase;
  मित्र श्रेणी TrailingObjects;

खुला:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> अग्र_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(माप_प्रकार StartBit,
                                                 माप_प्रकार अंक_Bits_2) {
  व्योम *Memory = ::चालक नया(आकार_To_Alloc(अंक_Bits_2));
  TrieSubtrie *S = ::नया (Memory) TrieSubtrie(StartBit, अंक_Bits_2);
  वापस std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2)
    : Trie_गाँठ(true), StartBit(StartBit), अंक_Bits_2(अंक_Bits_2), आकार(1u << अंक_Bits_2),
      अग्र_2(nullptr) {
  क्रम (अचिन्हित I = 0; I < आकार; ++I)
    नया (&लो(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_गाँठ>>::मूल्य_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    माप_प्रकार I, TrieContent &Content, माप_प्रकार अंक_Subtrie_Bits, माप_प्रकार NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  निश्चित(अंक_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + अंक_Bits_2, अंक_Subtrie_Bits);

  निश्चित(NewI < आकार);
  S->लो(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_गाँठ *Existing_गाँठ = &Content;
  निश्चित(I < आकार);
  अगर (लो(I).compare_exchange_strong(Existing_गाँठ, S.लो()))
    वापस Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  वापस cast<TrieSubtrie>(Existing_गाँठ);
}

श्रेणी ThreadSafeTrieRawHashMapBase::ImplType final
    : निजी TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
खुला:
  अटल std::unique_ptr<ImplType> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    माप_प्रकार आकार = माप(ImplType) + TrieSubtrie::आकार_To_Alloc(अंक_Bits_2);
    व्योम *Memory = ::चालक नया(आकार);
    ImplType *Impl = ::नया (Memory) ImplType(StartBit, अंक_Bits_2);
    वापस std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    निश्चित(!S->अग्र_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_शीर्ष = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    जबतक (!get_मूल()->अग्र_2.compare_exchange_weak(Current_शीर्ष, S.लो()))
      S->अग्र_2.exchange(Current_शीर्ष);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    वापस S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_मूल() { वापस getTrailingObjects(); }

  अटल व्योम *चालक नया(माप_प्रकार आकार) { वापस ::चालक नया(आकार); }
  व्योम चालक मिटाओ(व्योम *Ptr) { ::चालक मिटाओ(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

निजी:
  मित्र श्रेणी TrailingObjects;

  ImplType(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    ::नया (get_मूल()) TrieSubtrie(StartBit, अंक_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  अगर (ImplType *Impl = ImplPtr.load())
    वापस *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अंक_मूल_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  अगर (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.लो()))
    वापस *Impl.release();

  // Already created, return the existing ImplType.
  वापस *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) स्थिर {
  निश्चित(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  अगर (!Impl)
    वापस PointerBase();

  TrieSubtrie *S = Impl->get_मूल();
  Trie_Hash_सूचकांक_Generator सूचकांक_Gen{अंक_मूल_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार सूचकांक_2 = सूचकांक_Gen.अग्र();
  जबतक (सूचकांक_2 != सूचकांक_Gen.end()) {
    // Try to set the content.
    Trie_गाँठ *Existing = S->लो(सूचकांक_2);
    अगर (!Existing)
      वापस PointerBase(S, सूचकांक_2, *सूचकांक_Gen.StartBit);

    // Check for an exact match.
    अगर (स्वतः *ExistingContent = dyn_cast<TrieContent>(Existing))
      वापस ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_मूल्य_Pointer())
                 : PointerBase(S, सूचकांक_2, *सूचकांक_Gen.StartBit);

    सूचकांक_2 = सूचकांक_Gen.अग्र();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::जोड़ो(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<स्थिर uint8_t *(व्योम *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  निश्चित(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_मूल();
  Trie_Hash_सूचकांक_Generator सूचकांक_Gen{अंक_मूल_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार सूचकांक_2;
  अगर (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    सूचकांक_2 = सूचकांक_Gen.hint(Hint.I, Hint.B);
  } अन्यथा {
    सूचकांक_2 = सूचकांक_Gen.अग्र();
  }

  जबतक (सूचकांक_2 != सूचकांक_Gen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    bool Generated = false;
    Trie_गाँठ &Existing = S->लो(सूचकांक_2).loadOrGenerate([&]() {
      Generated = true;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_आकार, ContentAllocAlign));
      स्थिर uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नया (Memory)
          TrieContent(ContentOffset, Hash.आकार_2(), HashStorage - Memory);
      निश्चित(Hash == Content->getHash() && "Hash not properly initialized");
      वापस Content;
    });
    // If we just generated it, return it!
    अगर (Generated)
      वापस PointerBase(cast<TrieContent>(Existing).get_मूल्य_Pointer());

    अगर (स्वतः *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      सूचकांक_2 = सूचकांक_Gen.अग्र();
      जारी;
    }

    // Return the existing content if it's an exact match!
    स्वतः &ExistingContent = cast<TrieContent>(Existing);
    अगर (ExistingContent.getHash() == Hash)
      वापस PointerBase(ExistingContent.get_मूल्य_Pointer());

    // Sink the existing content as long as the indexes match.
    माप_प्रकार अग्र_सूचकांक = सूचकांक_Gen.अग्र();
    जबतक (अग्र_सूचकांक != सूचकांक_Gen.end()) {
      माप_प्रकार New_सूचकांक_For_Existing_Content =
          सूचकांक_Gen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(सूचकांक_2, ExistingContent, सूचकांक_Gen.get_अंक_Bits(),
                  New_सूचकांक_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    वापस Impl.save(std::move(S));
                  });
      सूचकांक_2 = अग्र_सूचकांक;

      // Found the difference.
      अगर (अग्र_सूचकांक != New_सूचकांक_For_Existing_Content)
        अवरोध;

      अग्र_सूचकांक = सूचकांक_Gen.अग्र();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    माप_प्रकार Content_Alloc_आकार, माप_प्रकार ContentAllocAlign, माप_प्रकार ContentOffset,
    std::optional<माप_प्रकार> अंक_मूल_Bits, std::optional<माप_प्रकार> अंक_Subtrie_Bits)
    : Content_Alloc_आकार(Content_Alloc_आकार), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      अंक_मूल_Bits(अंक_मूल_Bits.मूल्य_or(Default_अंक_मूल_Bits)),
      अंक_Subtrie_Bits(अंक_Subtrie_Bits.मूल्य_or(Default_अंक_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  निश्चित((!अंक_मूल_Bits || *अंक_मूल_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  निश्चित((!अंक_Subtrie_Bits || *अंक_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : Content_Alloc_आकार(RHS.Content_Alloc_आकार),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), अंक_मूल_Bits(RHS.अंक_मूल_Bits),
      अंक_Subtrie_Bits(RHS.अंक_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  निश्चित(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

व्योम ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<व्योम(व्योम *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  अगर (!Impl)
    वापस;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  अगर (Destructor)
    क्रम (TrieSubtrie *Trie = Impl->get_मूल(); Trie; Trie = Trie->अग्र_2.load())
      क्रम (अचिन्हित I = 0; I < Trie->आकार_2(); ++I)
        अगर (स्वतः *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_मूल्य_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_मूल()->अग्र_2;
  जबतक (Trie) {
    TrieSubtrie *अग्र_2 = Trie->अग्र_2.exchange(nullptr);
    मिटाओ Trie;
    Trie = अग्र_2;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_मूल() स्थिर {
  ImplType *Impl = ImplPtr.load();
  अगर (!Impl)
    वापस PointerBase();
  वापस PointerBase(Impl->get_मूल());
}

अचिन्हित ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिर {
  निश्चित(!P.isHint() && "Not a valid trie");
  अगर (!P.P)
    वापस 0;
  अगर (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P))
    वापस S->StartBit;
  वापस 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिर {
  निश्चित(!P.isHint() && "Not a valid trie");
  अगर (!P.P)
    वापस 0;
  अगर (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P))
    वापस S->अंक_Bits_2;
  वापस 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिर {
  निश्चित(!P.isHint() && "Not a valid trie");
  अगर (!P.P)
    वापस 0;
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  अगर (!S)
    वापस 0;
  अचिन्हित अंक = 0;
  क्रम (अचिन्हित I = 0, E = S->आकार_2(); I < E; ++I)
    अगर (S->load(I))
      ++अंक;
  वापस अंक;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिर {
  निश्चित(!P.isHint() && "Not a valid trie");
  अगर (!P.P)
    वापस "";

  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  अगर (!S || !S->IsSubtrie)
    वापस "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *गाँठ = nullptr;
  जबतक (Current) {
    TrieSubtrie *अग्र_2 = nullptr;
    // Find first used slot in the trie.
    क्रम (अचिन्हित I = 0, E = Current->आकार_2(); I < E; ++I) {
      स्वतः *S = Current->load(I);
      अगर (!S)
        जारी;

      अगर (स्वतः *Content = dyn_cast<TrieContent>(S))
        गाँठ = Content;
      अन्यथा अगर (स्वतः *Sub = dyn_cast<TrieSubtrie>(S))
        अग्र_2 = Sub;
      अवरोध;
    }

    // Found the node.
    अगर (गाँठ)
      अवरोध;

    // Continue to the next level if the node is not found.
    Current = अग्र_2;
  }

  निश्चित(गाँठ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  अचिन्हित StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(गाँठ->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/true);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  क्रम (अचिन्हित I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    अचिन्हित सूचकांक_2 = I / 8;
    अचिन्हित Offset = 7 - I % 8;
    Bits.धकेलो_back('0' + ((गाँठ->getHash()[सूचकांक_2] >> Offset) & 1));
  }

  अगर (!Bits.empty())
    SS << "[" << Bits << "]";

  वापस SS.str();
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Tries() स्थिर {
  ImplType *Impl = ImplPtr.load();
  अगर (!Impl)
    वापस 0;
  अचिन्हित अंक = 0;
  क्रम (TrieSubtrie *Trie = Impl->get_मूल(); Trie; Trie = Trie->अग्र_2.load())
    ++अंक;
  वापस अंक;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_अग्र_Trie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिर {
  निश्चित(!P.isHint() && "Not a valid trie");
  अगर (!P.P)
    वापस PointerBase();
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  अगर (!S)
    वापस PointerBase();
  अगर (स्वतः *E = S->अग्र_2.load())
    वापस PointerBase(E);
  वापस PointerBase();
}
