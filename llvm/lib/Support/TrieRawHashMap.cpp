//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/TrieRawHashMap.h"
#अन्तर्भाव "llvm/ADT/LazyAtomicPointer.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/TrieHashIndexGenerator.h"
#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/ThreadSafeAllocator.h"
#अन्तर्भाव "llvm/Support/TrailingObjects.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <memory>

using नामाकाश llvm;

नामाकाश {
संरचना Trie_गाँठो {
  अचर तार्किक IsSubtrie = असत्य;

  Trie_गाँठो(तार्किक IsSubtrie) : IsSubtrie(IsSubtrie) {}

  स्थिर शून्य *चालक नूतन(माप_प्रकार Size) { फर्काऊ ::चालक नूतन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }
};

संरचना TrieContent final : सार्वजनिक Trie_गाँठो {
  अचर uint8_t ContentOffset;
  अचर uint8_t HashSize;
  अचर uint8_t HashOffset;

  शून्य *get_मान_Pointer() अचर {
    स्वतः *Content = reinterpret_cast<अचर uint8_t *>(एतत्) + ContentOffset;
    फर्काऊ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() अचर {
    स्वतः *Begin = reinterpret_cast<अचर uint8_t *>(एतत्) + HashOffset;
    फर्काऊ ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(माप_प्रकार ContentOffset, माप_प्रकार HashSize, माप_प्रकार HashOffset)
      : Trie_गाँठो(/*IsSubtrie=*/असत्य), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  स्थिर तार्किक classof(अचर Trie_गाँठो *TN) { फर्काऊ !TN->IsSubtrie; }
};

static_assert(परिमाण(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

वर्ग TrieSubtrie final
    : सार्वजनिक Trie_गाँठो,
      गुप्त TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_गाँठो>> {
सार्वजनिक:
  using Slot = LazyAtomicPointer<Trie_गाँठो>;

  Slot &get(माप_प्रकार I) { फर्काऊ getTrailingObjects()[I]; }
  Trie_गाँठो *load(माप_प्रकार I) { फर्काऊ get(I).load(); }

  अचिन्हित size() अचर { फर्काऊ Size; }

  TrieSubtrie *
  sink(माप_प्रकार I, TrieContent &Content, माप_प्रकार अङ्क_Subtrie_Bits, माप_प्रकार NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  स्थिर std::unique_ptr<TrieSubtrie> create(माप_प्रकार StartBit, माप_प्रकार अङ्क_Bits_2);

  explicit TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अङ्क_Bits_2);

  स्थिर तार्किक classof(अचर Trie_गाँठो *TN) { फर्काऊ TN->IsSubtrie; }

  स्थिर constexpr माप_प्रकार sizeToAlloc(अचिन्हित अङ्क_Bits_2) {
    प्रतिज्ञा(अङ्क_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    अचिन्हित गन्ती_2 = 1u << अङ्क_Bits_2;
    फर्काऊ totalSizeToAlloc<LazyAtomicPointer<Trie_गाँठो>>(गन्ती_2);
  }

गुप्त:
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
  अचिन्हित अङ्क_Bits_2 = 0;
  अचिन्हित Size = 0;
  मित्र वर्ग llvm::ThreadSafeTrieRawHashMapBase;
  मित्र वर्ग TrailingObjects;

सार्वजनिक:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(माप_प्रकार StartBit,
                                                 माप_प्रकार अङ्क_Bits_2) {
  शून्य *Memory = ::चालक नूतन(sizeToAlloc(अङ्क_Bits_2));
  TrieSubtrie *S = ::नूतन (Memory) TrieSubtrie(StartBit, अङ्क_Bits_2);
  फर्काऊ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अङ्क_Bits_2)
    : Trie_गाँठो(सत्य), StartBit(StartBit), अङ्क_Bits_2(अङ्क_Bits_2), Size(1u << अङ्क_Bits_2),
      Next(nullptr) {
  लागि (अचिन्हित I = 0; I < Size; ++I)
    नूतन (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_गाँठो>>::मान_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    माप_प्रकार I, TrieContent &Content, माप_प्रकार अङ्क_Subtrie_Bits, माप_प्रकार NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  प्रतिज्ञा(अङ्क_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + अङ्क_Bits_2, अङ्क_Subtrie_Bits);

  प्रतिज्ञा(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_गाँठो *Existing_गाँठो = &Content;
  प्रतिज्ञा(I < Size);
  यदि (get(I).compare_exchange_strong(Existing_गाँठो, S.get()))
    फर्काऊ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  फर्काऊ cast<TrieSubtrie>(Existing_गाँठो);
}

वर्ग ThreadSafeTrieRawHashMapBase::ImplType final
    : गुप्त TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
सार्वजनिक:
  स्थिर std::unique_ptr<ImplType> create(माप_प्रकार StartBit, माप_प्रकार अङ्क_Bits_2) {
    माप_प्रकार Size = परिमाण(ImplType) + TrieSubtrie::sizeToAlloc(अङ्क_Bits_2);
    शून्य *Memory = ::चालक नूतन(Size);
    ImplType *Impl = ::नूतन (Memory) ImplType(StartBit, अङ्क_Bits_2);
    फर्काऊ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    प्रतिज्ञा(!S->Next && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *CurrentHead = nullptr;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    जबसम्म (!get_जरा()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    फर्काऊ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_जरा() { फर्काऊ getTrailingObjects(); }

  स्थिर शून्य *चालक नूतन(माप_प्रकार Size) { फर्काऊ ::चालक नूतन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

गुप्त:
  मित्र वर्ग TrailingObjects;

  ImplType(माप_प्रकार StartBit, माप_प्रकार अङ्क_Bits_2) {
    ::नूतन (get_जरा()) TrieSubtrie(StartBit, अङ्क_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  यदि (ImplType *Impl = ImplPtr.load())
    फर्काऊ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अङ्क_जरा_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  यदि (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    फर्काऊ *Impl.release();

  // Already created, return the existing ImplType.
  फर्काऊ *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) अचर {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    फर्काऊ PointerBase();

  TrieSubtrie *S = Impl->get_जरा();
  TrieHashIndexGenerator IndexGen{अङ्क_जरा_Bits, अङ्क_Subtrie_Bits, Hash};
  माप_प्रकार Index = IndexGen.next();
  जबसम्म (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_गाँठो *Existing = S->get(Index);
    यदि (!Existing)
      फर्काऊ PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    यदि (स्वतः *ExistingContent = dyn_cast<TrieContent>(Existing))
      फर्काऊ ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_मान_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::घुसाउ(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<अचर uint8_t *(शून्य *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_जरा();
  TrieHashIndexGenerator IndexGen{अङ्क_जरा_Bits, अङ्क_Subtrie_Bits, Hash};
  माप_प्रकार Index;
  यदि (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } अन्यथा {
    Index = IndexGen.next();
  }

  जबसम्म (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    तार्किक Generated = असत्य;
    Trie_गाँठो &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = सत्य;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      अचर uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नूतन (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      प्रतिज्ञा(Hash == Content->getHash() && "Hash not properly initialized");
      फर्काऊ Content;
    });
    // If we just generated it, return it!
    यदि (Generated)
      फर्काऊ PointerBase(cast<TrieContent>(Existing).get_मान_Pointer());

    यदि (स्वतः *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      जारी;
    }

    // Return the existing content if it's an exact match!
    स्वतः &ExistingContent = cast<TrieContent>(Existing);
    यदि (ExistingContent.getHash() == Hash)
      फर्काऊ PointerBase(ExistingContent.get_मान_Pointer());

    // Sink the existing content as long as the indexes match.
    माप_प्रकार NextIndex = IndexGen.next();
    जबसम्म (NextIndex != IndexGen.end()) {
      माप_प्रकार NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_अङ्क_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    फर्काऊ Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      यदि (NextIndex != NewIndexForExistingContent)
        रोक;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    माप_प्रकार ContentAllocSize, माप_प्रकार ContentAllocAlign, माप_प्रकार ContentOffset,
    std::optional<माप_प्रकार> अङ्क_जरा_Bits, std::optional<माप_प्रकार> अङ्क_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      अङ्क_जरा_Bits(अङ्क_जरा_Bits.मान_or(Default_अङ्क_जरा_Bits)),
      अङ्क_Subtrie_Bits(अङ्क_Subtrie_Bits.मान_or(Default_अङ्क_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  प्रतिज्ञा((!अङ्क_जरा_Bits || *अङ्क_जरा_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  प्रतिज्ञा((!अङ्क_Subtrie_Bits || *अङ्क_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), अङ्क_जरा_Bits(RHS.अङ्क_जरा_Bits),
      अङ्क_Subtrie_Bits(RHS.अङ्क_Subtrie_Bits) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(nullptr);
}

ThreadSafeTrieRawHashMapBase::~ThreadSafeTrieRawHashMapBase() {
  प्रतिज्ञा(!ImplPtr.load() && "Expected subclass to call destroyImpl()");
}

शून्य ThreadSafeTrieRawHashMapBase::destroyImpl(
    function_ref<शून्य(शून्य *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(nullptr));
  यदि (!Impl)
    फर्काऊ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  यदि (Destructor)
    लागि (TrieSubtrie *Trie = Impl->get_जरा(); Trie; Trie = Trie->Next.load())
      लागि (अचिन्हित I = 0; I < Trie->size(); ++I)
        यदि (स्वतः *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_मान_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_जरा()->Next;
  जबसम्म (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_जरा() अचर {
  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    फर्काऊ PointerBase();
  फर्काऊ PointerBase(Impl->get_जरा());
}

अचिन्हित ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) अचर {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    फर्काऊ 0;
  यदि (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठो *)P.P))
    फर्काऊ S->StartBit;
  फर्काऊ 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अङ्क_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) अचर {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    फर्काऊ 0;
  यदि (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठो *)P.P))
    फर्काऊ S->अङ्क_Bits_2;
  फर्काऊ 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अङ्क_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) अचर {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    फर्काऊ 0;
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठो *)P.P);
  यदि (!S)
    फर्काऊ 0;
  अचिन्हित अङ्क = 0;
  लागि (अचिन्हित I = 0, E = S->size(); I < E; ++I)
    यदि (S->load(I))
      ++अङ्क;
  फर्काऊ अङ्क;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) अचर {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    फर्काऊ "";

  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठो *)P.P);
  यदि (!S || !S->IsSubtrie)
    फर्काऊ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *गाँठो = nullptr;
  जबसम्म (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    लागि (अचिन्हित I = 0, E = Current->size(); I < E; ++I) {
      स्वतः *S = Current->load(I);
      यदि (!S)
        जारी;

      यदि (स्वतः *Content = dyn_cast<TrieContent>(S))
        गाँठो = Content;
      अन्यथा यदि (स्वतः *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      रोक;
    }

    // Found the node.
    यदि (गाँठो)
      रोक;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  प्रतिज्ञा(गाँठो && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  अचिन्हित StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(गाँठो->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/सत्य);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  लागि (अचिन्हित I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    अचिन्हित Index = I / 8;
    अचिन्हित Offset = 7 - I % 8;
    Bits.थप_back('0' + ((गाँठो->getHash()[Index] >> Offset) & 1));
  }

  यदि (!Bits.empty())
    SS << "[" << Bits << "]";

  फर्काऊ SS.str();
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अङ्क_Tries() अचर {
  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    फर्काऊ 0;
  अचिन्हित अङ्क = 0;
  लागि (TrieSubtrie *Trie = Impl->get_जरा(); Trie; Trie = Trie->Next.load())
    ++अङ्क;
  फर्काऊ अङ्क;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) अचर {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    फर्काऊ PointerBase();
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठो *)P.P);
  यदि (!S)
    फर्काऊ PointerBase();
  यदि (स्वतः *E = S->Next.load())
    फर्काऊ PointerBase(E);
  फर्काऊ PointerBase();
}
