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
संरचना Trie_गाठ {
  स्थिरांक तार्किक IsSubtrie = असत्य;

  Trie_गाठ(तार्किक IsSubtrie) : IsSubtrie(IsSubtrie) {}

  स्थिर शून्य *चालक नवीन(माप_प्रकार Size) { परत ::चालक नवीन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }
};

संरचना TrieContent final : सार्वजनिक Trie_गाठ {
  स्थिरांक uint8_t ContentOffset;
  स्थिरांक uint8_t HashSize;
  स्थिरांक uint8_t HashOffset;

  शून्य *get_मूल्य_Pointer() स्थिरांक {
    स्वतः *Content = reinterpret_cast<स्थिरांक uint8_t *>(हे) + ContentOffset;
    परत const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() स्थिरांक {
    स्वतः *Begin = reinterpret_cast<स्थिरांक uint8_t *>(हे) + HashOffset;
    परत ArrayRef(Begin, Begin + HashSize);
  }

  TrieContent(माप_प्रकार ContentOffset, माप_प्रकार HashSize, माप_प्रकार HashOffset)
      : Trie_गाठ(/*IsSubtrie=*/असत्य), ContentOffset(ContentOffset),
        HashSize(HashSize), HashOffset(HashOffset) {}

  स्थिर तार्किक classof(स्थिरांक Trie_गाठ *TN) { परत !TN->IsSubtrie; }
};

static_assert(आकार(TrieContent) ==
                  ThreadSafeTrieRawHashMapBase::TrieContentBaseSize,
              "Check header assumption!");

वर्ग TrieSubtrie final
    : सार्वजनिक Trie_गाठ,
      खाजगी TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_गाठ>> {
सार्वजनिक:
  using Slot = LazyAtomicPointer<Trie_गाठ>;

  Slot &get(माप_प्रकार I) { परत getTrailingObjects()[I]; }
  Trie_गाठ *load(माप_प्रकार I) { परत get(I).load(); }

  अचिन्हित size() स्थिरांक { परत Size; }

  TrieSubtrie *
  sink(माप_प्रकार I, TrieContent &Content, माप_प्रकार अंक_Subtrie_Bits, माप_प्रकार NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  स्थिर std::unique_ptr<TrieSubtrie> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  explicit TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2);

  स्थिर तार्किक classof(स्थिरांक Trie_गाठ *TN) { परत TN->IsSubtrie; }

  स्थिर constexpr माप_प्रकार sizeToAlloc(अचिन्हित अंक_Bits_2) {
    प्रतिज्ञा(अंक_Bits_2 < 20 && "Tries should have fewer than ~1M slots");
    अचिन्हित मोजणी_2 = 1u << अंक_Bits_2;
    परत totalSizeToAlloc<LazyAtomicPointer<Trie_गाठ>>(मोजणी_2);
  }

खाजगी:
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
  शून्य *Memory = ::चालक नवीन(sizeToAlloc(अंक_Bits_2));
  TrieSubtrie *S = ::नवीन (Memory) TrieSubtrie(StartBit, अंक_Bits_2);
  परत std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2)
    : Trie_गाठ(सत्य), StartBit(StartBit), अंक_Bits_2(अंक_Bits_2), Size(1u << अंक_Bits_2),
      Next(nullptr) {
  साठी (अचिन्हित I = 0; I < Size; ++I)
    नवीन (&get(I)) Slot(nullptr);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_गाठ>>::मूल्य_2,
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
  Trie_गाठ *Existing_गाठ = &Content;
  प्रतिज्ञा(I < Size);
  जर (get(I).compare_exchange_strong(Existing_गाठ, S.get()))
    परत Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  परत cast<TrieSubtrie>(Existing_गाठ);
}

वर्ग ThreadSafeTrieRawHashMapBase::ImplType final
    : खाजगी TrailingObjects<ThreadSafeTrieRawHashMapBase::ImplType,
                              TrieSubtrie> {
सार्वजनिक:
  स्थिर std::unique_ptr<ImplType> create(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    माप_प्रकार Size = आकार(ImplType) + TrieSubtrie::sizeToAlloc(अंक_Bits_2);
    शून्य *Memory = ::चालक नवीन(Size);
    ImplType *Impl = ::नवीन (Memory) ImplType(StartBit, अंक_Bits_2);
    परत std::unique_ptr<ImplType>(Impl);
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
    पर्यंत (!get_मूळ()->Next.compare_exchange_weak(CurrentHead, S.get()))
      S->Next.exchange(CurrentHead);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    परत S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_मूळ() { परत getTrailingObjects(); }

  स्थिर शून्य *चालक नवीन(माप_प्रकार Size) { परत ::चालक नवीन(Size); }
  शून्य चालक delete(शून्य *Ptr) { ::चालक delete(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

खाजगी:
  मित्र वर्ग TrailingObjects;

  ImplType(माप_प्रकार StartBit, माप_प्रकार अंक_Bits_2) {
    ::नवीन (get_मूळ()) TrieSubtrie(StartBit, अंक_Bits_2);
  }
};

ThreadSafeTrieRawHashMapBase::ImplType &
ThreadSafeTrieRawHashMapBase::getOrCreateImpl() {
  जर (ImplType *Impl = ImplPtr.load())
    परत *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अंक_मूळ_Bits);
  ImplType *ExistingImpl = nullptr;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  जर (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    परत *Impl.release();

  // Already created, return the existing ImplType.
  परत *ExistingImpl;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::find(ArrayRef<uint8_t> Hash) स्थिरांक {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.load();
  जर (!Impl)
    परत PointerBase();

  TrieSubtrie *S = Impl->get_मूळ();
  TrieHashIndexGenerator IndexGen{अंक_मूळ_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार Index = IndexGen.next();
  पर्यंत (Index != IndexGen.end()) {
    // Try to set the content.
    Trie_गाठ *Existing = S->get(Index);
    जर (!Existing)
      परत PointerBase(S, Index, *IndexGen.StartBit);

    // Check for an exact match.
    जर (स्वतः *ExistingContent = dyn_cast<TrieContent>(Existing))
      परत ExistingContent->getHash() == Hash
                 ? PointerBase(ExistingContent->get_मूल्य_Pointer())
                 : PointerBase(S, Index, *IndexGen.StartBit);

    Index = IndexGen.next();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::PointerBase ThreadSafeTrieRawHashMapBase::घाला(
    PointerBase Hint, ArrayRef<uint8_t> Hash,
    function_ref<स्थिरांक uint8_t *(शून्य *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  प्रतिज्ञा(!Hash.empty() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_मूळ();
  TrieHashIndexGenerator IndexGen{अंक_मूळ_Bits, अंक_Subtrie_Bits, Hash};
  माप_प्रकार Index;
  जर (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    Index = IndexGen.hint(Hint.I, Hint.B);
  } अन्यथा {
    Index = IndexGen.next();
  }

  पर्यंत (Index != IndexGen.end()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    तार्किक Generated = असत्य;
    Trie_गाठ &Existing = S->get(Index).loadOrGenerate([&]() {
      Generated = सत्य;

      // Construct the value itself at the tail.
      uint8_t *Memory = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(ContentAllocSize, ContentAllocAlign));
      स्थिरांक uint8_t *HashStorage = Constructor(Memory + ContentOffset, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नवीन (Memory)
          TrieContent(ContentOffset, Hash.size(), HashStorage - Memory);
      प्रतिज्ञा(Hash == Content->getHash() && "Hash not properly initialized");
      परत Content;
    });
    // If we just generated it, return it!
    जर (Generated)
      परत PointerBase(cast<TrieContent>(Existing).get_मूल्य_Pointer());

    जर (स्वतः *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      Index = IndexGen.next();
      सुरूठेवा;
    }

    // Return the existing content if it's an exact match!
    स्वतः &ExistingContent = cast<TrieContent>(Existing);
    जर (ExistingContent.getHash() == Hash)
      परत PointerBase(ExistingContent.get_मूल्य_Pointer());

    // Sink the existing content as long as the indexes match.
    माप_प्रकार NextIndex = IndexGen.next();
    पर्यंत (NextIndex != IndexGen.end()) {
      माप_प्रकार NewIndexForExistingContent =
          IndexGen.getCollidingBits(ExistingContent.getHash());
      S = S->sink(Index, ExistingContent, IndexGen.get_अंक_Bits(),
                  NewIndexForExistingContent,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    परत Impl.save(std::move(S));
                  });
      Index = NextIndex;

      // Found the difference.
      जर (NextIndex != NewIndexForExistingContent)
        थांबा;

      NextIndex = IndexGen.next();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    माप_प्रकार ContentAllocSize, माप_प्रकार ContentAllocAlign, माप_प्रकार ContentOffset,
    std::optional<माप_प्रकार> अंक_मूळ_Bits, std::optional<माप_प्रकार> अंक_Subtrie_Bits)
    : ContentAllocSize(ContentAllocSize), ContentAllocAlign(ContentAllocAlign),
      ContentOffset(ContentOffset),
      अंक_मूळ_Bits(अंक_मूळ_Bits.मूल्य_or(Default_अंक_मूळ_Bits)),
      अंक_Subtrie_Bits(अंक_Subtrie_Bits.मूल्य_or(Default_अंक_Subtrie_Bits)),
      ImplPtr(nullptr) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  प्रतिज्ञा((!अंक_मूळ_Bits || *अंक_मूळ_Bits < 20) &&
         "Root should have fewer than ~1M slots");
  प्रतिज्ञा((!अंक_Subtrie_Bits || *अंक_Subtrie_Bits < 10) &&
         "Subtries should have fewer than ~1K slots");
}

ThreadSafeTrieRawHashMapBase::ThreadSafeTrieRawHashMapBase(
    ThreadSafeTrieRawHashMapBase &&RHS)
    : ContentAllocSize(RHS.ContentAllocSize),
      ContentAllocAlign(RHS.ContentAllocAlign),
      ContentOffset(RHS.ContentOffset), अंक_मूळ_Bits(RHS.अंक_मूळ_Bits),
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
  जर (!Impl)
    परत;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  जर (Destructor)
    साठी (TrieSubtrie *Trie = Impl->get_मूळ(); Trie; Trie = Trie->Next.load())
      साठी (अचिन्हित I = 0; I < Trie->size(); ++I)
        जर (स्वतः *Content = dyn_cast_or_null<TrieContent>(Trie->load(I)))
          Destructor(Content->get_मूल्य_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_मूळ()->Next;
  पर्यंत (Trie) {
    TrieSubtrie *Next = Trie->Next.exchange(nullptr);
    delete Trie;
    Trie = Next;
  }
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::get_मूळ() स्थिरांक {
  ImplType *Impl = ImplPtr.load();
  जर (!Impl)
    परत PointerBase();
  परत PointerBase(Impl->get_मूळ());
}

अचिन्हित ThreadSafeTrieRawHashMapBase::getStartBit(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिरांक {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  जर (!P.P)
    परत 0;
  जर (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाठ *)P.P))
    परत S->StartBit;
  परत 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Bits(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिरांक {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  जर (!P.P)
    परत 0;
  जर (स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाठ *)P.P))
    परत S->अंक_Bits_2;
  परत 0;
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Slot_Used(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिरांक {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  जर (!P.P)
    परत 0;
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाठ *)P.P);
  जर (!S)
    परत 0;
  अचिन्हित अंक = 0;
  साठी (अचिन्हित I = 0, E = S->size(); I < E; ++I)
    जर (S->load(I))
      ++अंक;
  परत अंक;
}

std::string ThreadSafeTrieRawHashMapBase::getTriePrefixAsString(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिरांक {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  जर (!P.P)
    परत "";

  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाठ *)P.P);
  जर (!S || !S->IsSubtrie)
    परत "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *गाठ = nullptr;
  पर्यंत (Current) {
    TrieSubtrie *Next = nullptr;
    // Find first used slot in the trie.
    साठी (अचिन्हित I = 0, E = Current->size(); I < E; ++I) {
      स्वतः *S = Current->load(I);
      जर (!S)
        सुरूठेवा;

      जर (स्वतः *Content = dyn_cast<TrieContent>(S))
        गाठ = Content;
      अन्यथा जर (स्वतः *Sub = dyn_cast<TrieSubtrie>(S))
        Next = Sub;
      थांबा;
    }

    // Found the node.
    जर (गाठ)
      थांबा;

    // Continue to the next level if the node is not found.
    Current = Next;
  }

  प्रतिज्ञा(गाठ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  अचिन्हित StartFullBytes = (S->StartBit + 1) / 8 - 1;
  SS << toHex(toStringRef(गाठ->getHash()).take_front(StartFullBytes),
              /*LowerCase=*/सत्य);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string Bits;
  साठी (अचिन्हित I = StartFullBytes * 8, E = S->StartBit; I < E; ++I) {
    अचिन्हित Index = I / 8;
    अचिन्हित Offset = 7 - I % 8;
    Bits.ढकला_back('0' + ((गाठ->getHash()[Index] >> Offset) & 1));
  }

  जर (!Bits.empty())
    SS << "[" << Bits << "]";

  परत SS.str();
}

अचिन्हित ThreadSafeTrieRawHashMapBase::get_अंक_Tries() स्थिरांक {
  ImplType *Impl = ImplPtr.load();
  जर (!Impl)
    परत 0;
  अचिन्हित अंक = 0;
  साठी (TrieSubtrie *Trie = Impl->get_मूळ(); Trie; Trie = Trie->Next.load())
    ++अंक;
  परत अंक;
}

ThreadSafeTrieRawHashMapBase::PointerBase
ThreadSafeTrieRawHashMapBase::getNextTrie(
    ThreadSafeTrieRawHashMapBase::PointerBase P) स्थिरांक {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  जर (!P.P)
    परत PointerBase();
  स्वतः *S = dyn_cast<TrieSubtrie>((Trie_गाठ *)P.P);
  जर (!S)
    परत PointerBase();
  जर (स्वतः *E = S->Next.load())
    परत PointerBase(E);
  परत PointerBase();
}
