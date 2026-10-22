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
संरचना Trie_गाँठ {
  ध्रुव तार्किक IsSubtrie = असत्य;

  Trie_गाँठ(तार्किक IsSubtrie) : IsSubtrie(IsSubtrie) {}

  स्थिर शून्य *चालक नूतन(माप_प्रकार Size) { प्रत्यागम ::चालक नूतन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }
};

संरचना TrieContent final : सार्वजनिक Trie_गाँठ {
  ध्रुव uint8_t ContentOffset;
  ध्रुव uint8_t HashSize;
  ध्रुव uint8_t HashOffset;

  शून्य *get_मूल्य_Pointer() ध्रुव {
    स्वतः *Content = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + ContentOffset;
    प्रत्यागम const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ध्रुव {
    स्वतः *Begin = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + HashOffset;
    प्रत्यागम ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(माप_प्रकार ContentOffset, माप_प्रकार HashSize, माप_प्रकार HashOffset)
      : Trie_गाँठ(/*IsSubtrie=*/असत्य), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  स्थिर तार्किक classof(ध्रुव Trie_गाँठ *TN) { प्रत्यागम !TN->IsSubtrie; }
};

static_assert(परिमाण(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

वर्ग TrieSubtrie final
    : सार्वजनिक Trie_गाँठ,
      गुप्त TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_गाँठ>> {
सार्वजनिक:
  using Slot = LazyAtomicPointer<Trie_गाँठ>;

  Slot &get(माप_प्रकार I) { प्रत्यागम getTrailingObjects()[I]; }
  Trie_गाँठ *load(माप_प्रकार I) { प्रत्यागम get(I).load(); }

  अचिन्हित size() ध्रुव { प्रत्यागम Size; }

  TrieSubtrie *
  sink(माप_प्रकार I, TrieContent &Content, माप_प्रकार अंक_Subtrie_Bits, माप_प्रकार NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  स्थिर std::unique_ptr<TrieSubtrie> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  explicit TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  स्थिर तार्किक classof(ध्रुव Trie_गाँठ *TN) { प्रत्यागम TN->IsSubtrie; }

  स्थिर constexpr माप_प्रकार sizeToAlloc(अचिन्हित अंक_Bits_2) {
    प्रतिज्ञा(अंक_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    अचिन्हित गिनती_2 = 1u << अंक_Bits_2;
    प्रत्यागम totalSizeToAlloc<LazyAtomicPointer<Trie_गाँठ>>(गिनती_2);
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
  अचिन्हित अंक_Bits_2 = 0;
  अचिन्हित Size = 0;
  मित्र वर्ग llvm::ThreadSafeTrieRawHashMapBase;
  मित्र वर्ग TrailingObjects;

सार्वजनिक:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> Next;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(माप_प्रकार StartBit,
                                                 माप_प्रकार अंक_Bits_2) {
  शून्य *Memory = ::चालक नूतन(sizeToAlloc(अंक_Bits_2));
  TrieSubtrie *S = ::नूतन (Memory) TrieSubtrie(StartBit, अंक_Bits_2);
  प्रत्यागम std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2)
    : Trie_गाँठ(सत्य), StartBit(StartBit), अंक_Bits_2(अंक_Bits_2), Size(1u << अंक_Bits_2),
      Next(nullptr) {
  प्रति (अचिन्हित I = 0; I < Size; ++I)
    नूतन (&get(I)) Slot(nullptr);

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
  प्रतिज्ञा(अंक_Subtrie_Bits > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + अंक_Bits_2, अंक_Subtrie_Bits);

  प्रतिज्ञा(NewI < Size);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_गाँठ *Existing_गाँठ = &Content;
  प्रतिज्ञा(I < Size);
  यदि (get(I).compare_exchange_strong(Existing_गाँठ, S.get()))
    प्रत्यागम Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  प्रत्यागम cast<TrieSubtrie>(Existing_गाँठ);
}

वर्ग ThreadSafeTrieRawHashMapBase::ImplType final
    : गुप्त TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
सार्वजनिक:
  स्थिर std::unique_ptr<ImplType> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    माप_प्रकार Size = परिमाण(ImplType) + TrieSubtrie::sizeToAlloc(अंक_Bits_2);
    शून्य *Memory = ::चालक नूतन(Size);
    ImplType *Impl = ::नूतन (Memory) ImplType(StartBit, अंक_Bits_2);
    प्रत्यागम std::unique_ptr<ImplType>(Impl);
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
    यावत् (!get_मूल()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    प्रत्यागम S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_मूल() { प्रत्यागम getTrailingObjects(); }

  स्थिर शून्य *चालक नूतन(माप_प्रकार Size) { प्रत्यागम ::चालक नूतन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

गुप्त:
  मित्र वर्ग TrailingObjects;

  ImplType(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    ::नूतन (get_मूल()) TrieSubtrie(StartBit, अंक_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  यदि (ImplType *Impl = ImplPtr.load())
    प्रत्यागम *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अंक_मूल_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  यदि (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    प्रत्यागम *Impl.release();

  // Already created, return the existing ImplType.
  प्रत्यागम *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) ध्रुव {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    प्रत्यागम PointerBase();

  TrieSubtrie *S = Impl->get_मूल();
  TrieHashIndexGenerator IndexGen{अंक_मूल_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार Index = IndexGen.next();
  यावत् (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_गाँठ *Existing = S->get(Index);
    यदि (!Existing)
      प्रत्यागम PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    यदि (स्वतः *ExistingContent = dyn_cast<TrieContent>(Existing))
      प्रत्यागम ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_मूल्य_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::जोड़ो(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<ध्रुव uint8_t *(शून्य *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_मूल();
  TrieHashIndexGenerator IndexGen{अंक_मूल_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार Index;
  यदि (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } अन्यथा {
    Index = IndexGen.next();
  }

  यावत् (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    तार्किक Generated = असत्य;
    Trie_गाँठ &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = सत्य;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      ध्रुव uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नूतन (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      प्रतिज्ञा(Hash == Content->getHash() && "Hash not properly initialized");
      प्रत्यागम Content;
    });
    // If we just generated it, return it!
    यदि (Generated)
      प्रत्यागम PointerBase(cast<TrieContent>(Existing).get_मूल्य_Pointer());

    यदि (स्वतः *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      अनुवर्त;
    }

    // Return the existing content if it's an exact match!
    स्वतः &ExistingContent = cast<TrieContent>(Existing);
    यदि (ExistingContent.getHash() == Hash)
      प्रत्यागम PointerBase(ExistingContent.get_मूल्य_Pointer());

    // Sink the existing content as long as the indexes match.
    माप_प्रकार NextIndex = IndexGen.next();
    यावत् (NextIndex != IndexGen.end()) {
      माप_प्रकार NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_अंक_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    प्रत्यागम Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      यदि (NextIndex != NewIndexForExistingContent)
        भङ्ग;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    माप_प्रकार ContentAllocSize, माप_प्रकार ContentAllocAlign, माप_प्रकार ContentOffset,
    std::optional<माप_प्रकार> अंक_मूल_Bits, std::optional<माप_प्रकार> अंक_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      अंक_मूल_Bits(अंक_मूल_Bits.मूल्य_or(Default_अंक_मूल_Bits)),
      अंक_Subtrie_Bits(अंक_Subtrie_Bits.मूल्य_or(Default_अंक_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  प्रतिज्ञा((!अंक_मूल_Bits || *अंक_मूल_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  प्रतिज्ञा((!अंक_Subtrie_Bits || *अंक_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), अंक_मूल_Bits(RHS.अंक_मूल_Bits),
      अंक_Subtrie_Bits(RHS.अंक_Subtrie_Bits) {
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
    प्रत्यागम;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  यदि (Destructor)
    प्रति (TrieSubtrie *Trie = Impl->get_मूल(); Trie; Trie = Trie->Next.load())
      प्रति (अचिन्हित I = 0; I < Trie->size(); ++I)
        यदि (स्वतः *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_मूल्य_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_मूल()->Next;
  यावत् (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_मूल() ध्रुव {
  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    प्रत्यागम PointerBase();
  प्रत्यागम PointerBase(Impl->get_मूल());
}

अचिन्हित ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  यदि (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P))
    प्रत्यागम S->StartBit;
  प्रत्यागम 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  यदि (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P))
    प्रत्यागम S->अंक_Bits_2;
  प्रत्यागम 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  यदि (!S)
    प्रत्यागम 0;
  अचिन्हित अंक = 0;
  प्रति (अचिन्हित I = 0, E = S->size(); I < E; ++I)
    यदि (S->load(I))
      ++अंक;
  प्रत्यागम अंक;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम "";

  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  यदि (!S || !S->IsSubtrie)
    प्रत्यागम "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *गाँठ = nullptr;
  यावत् (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    प्रति (अचिन्हित I = 0, E = Current->size(); I < E; ++I) {
      स्वतः *S = Current->load(I);
      यदि (!S)
        अनुवर्त;

      यदि (स्वतः *Content = dyn_cast<TrieContent>(S))
        गाँठ = Content;
      अन्यथा यदि (स्वतः *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      भङ्ग;
    }

    // Found the node.
    यदि (गाँठ)
      भङ्ग;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  प्रतिज्ञा(गाँठ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  अचिन्हित StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(गाँठ->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/सत्य);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  प्रति (अचिन्हित I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    अचिन्हित Index = I / 8;
    अचिन्हित Offset = 7 - I % 8;
    Bits.धकेलो_back('0' + ((गाँठ->getHash()[Index] >> Offset) & 1));
  }

  यदि (!Bits.empty())
    SS << "[" << Bits << "]";

  प्रत्यागम SS.str();
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Tries() ध्रुव {
  ImplType *Impl = ImplPtr.load();
  यदि (!Impl)
    प्रत्यागम 0;
  अचिन्हित अंक = 0;
  प्रति (TrieSubtrie *Trie = Impl->get_मूल(); Trie; Trie = Trie->Next.load())
    ++अंक;
  प्रत्यागम अंक;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम PointerBase();
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाँठ *)P.P);
  यदि (!S)
    प्रत्यागम PointerBase();
  यदि (स्वतः *E = S->Next.load())
    प्रत्यागम PointerBase(E);
  प्रत्यागम PointerBase();
}
