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

کارول نطاق llvm;

نطاق {
بنية Trie_غوټه {
  ثابت منطقي IsSubtrie = خطأ;

  Trie_غوټه(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *چلوونکی جديد(size_t اندازه) { أرجع ::چلوونکی جديد(اندازه); }
  فراغ چلوونکی ړنګول(فراغ *Ptr) { ::چلوونکی ړنګول(Ptr); }
};

بنية TrieContent final : عام Trie_غوټه {
  ثابت uint8_t Content_دننه_واټن;
  ثابت uint8_t Hash_اندازه;
  ثابت uint8_t Hash_دننه_واټن;

  فراغ *get_ارزښت_اشاره_ګر() ثابت {
    خپلکاری *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + Content_دننه_واټن;
    أرجع const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    خپلکاری *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + Hash_دننه_واټن;
    أرجع ArrayRef(Begin, Begin + Hash_اندازه);
  }

  TrieContent(size_t Content_دننه_واټن, size_t Hash_اندازه, size_t Hash_دننه_واټن)
      : Trie_غوټه(/*IsSubtrie=*/خطأ), Content_دننه_واټن(Content_دننه_واټن),
        Hash_اندازه(Hash_اندازه), Hash_دننه_واټن(Hash_دننه_واټن) {}

  ساكن منطقي classof(ثابت Trie_غوټه *TN) { أرجع !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::Trie_Content_بنسټ_اندازه,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام Trie_غوټه,
      خاص TrailingObjects<TrieSubtrie, Lazy_Atomic_اشاره_ګر<Trie_غوټه>> {
عام:
  کارول خونه = Lazy_Atomic_اشاره_ګر<Trie_غوټه>;

  خونه &get(size_t I) { أرجع getTrailingObjects()[I]; }
  Trie_غوټه *بارول(size_t I) { أرجع get(I).بارول(); }

  بینښه اندازه_2() ثابت { أرجع اندازه; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t عدد_Subtrie_بټونه, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t پیل_بټ, size_t عدد_بټونه_2);

  څرګند TrieSubtrie(size_t پیل_بټ, size_t عدد_بټونه_2);

  ساكن منطقي classof(ثابت Trie_غوټه *TN) { أرجع TN->IsSubtrie; }

  ساكن constexpr size_t اندازه_منزل_Alloc(بینښه عدد_بټونه_2) {
    أكد(عدد_بټونه_2 < 20 && "Tries should have fewer than ~1M slots");
    بینښه شمېر_2 = 1u << عدد_بټونه_2;
    أرجع ټول_اندازه_منزل_Alloc<Lazy_Atomic_اشاره_ګر<Trie_غوټه>>(شمېر_2);
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
  بینښه پیل_بټ = 0;
  بینښه عدد_بټونه_2 = 0;
  بینښه اندازه = 0;
  ملګری صنف llvm::Thread_خوندي_Trie_Raw_Hash_Map_بنسټ;
  ملګری صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> بل_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t پیل_بټ,
                                                 size_t عدد_بټونه_2) {
  فراغ *حافظه_2 = ::چلوونکی جديد(اندازه_منزل_Alloc(عدد_بټونه_2));
  TrieSubtrie *S = ::جديد (حافظه_2) TrieSubtrie(پیل_بټ, عدد_بټونه_2);
  أرجع std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t پیل_بټ, size_t عدد_بټونه_2)
    : Trie_غوټه(ریښتیا), پیل_بټ(پیل_بټ), عدد_بټونه_2(عدد_بټونه_2), اندازه(1u << عدد_بټونه_2),
      بل_2(تش_اشاره) {
  لكل (بینښه I = 0; I < اندازه; ++I)
    جديد (&get(I)) خونه(تش_اشاره);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_اشاره_ګر<Trie_غوټه>>::ارزښت_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t عدد_Subtrie_بټونه, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(عدد_Subtrie_بټونه > 0);
  std::unique_ptr<TrieSubtrie> S = create(پیل_بټ + عدد_بټونه_2, عدد_Subtrie_بټونه);

  أكد(NewI < اندازه);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_غوټه *Existing_غوټه = &Content;
  أكد(I < اندازه);
  إذا (get(I).compare_exchange_strong(Existing_غوټه, S.get()))
    أرجع Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  أرجع cast<TrieSubtrie>(Existing_غوټه);
}

صنف Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::ImplType final
    : خاص TrailingObjects<Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t پیل_بټ, size_t عدد_بټونه_2) {
    size_t اندازه = حجم(ImplType) + TrieSubtrie::اندازه_منزل_Alloc(عدد_بټونه_2);
    فراغ *حافظه_2 = ::چلوونکی جديد(اندازه);
    ImplType *Impl = ::جديد (حافظه_2) ImplType(پیل_بټ, عدد_بټونه_2);
    أرجع std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->بل_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_سر = تش_اشاره;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    طالما (!get_ریښه()->بل_2.compare_exchange_weak(Current_سر, S.get()))
      S->بل_2.exchange(Current_سر);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    أرجع S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ریښه() { أرجع getTrailingObjects(); }

  ساكن فراغ *چلوونکی جديد(size_t اندازه) { أرجع ::چلوونکی جديد(اندازه); }
  فراغ چلوونکی ړنګول(فراغ *Ptr) { ::چلوونکی ړنګول(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_خوندي_Allocator<BumpPtrAllocator> ContentAlloc;

خاص:
  ملګری صنف TrailingObjects;

  ImplType(size_t پیل_بټ, size_t عدد_بټونه_2) {
    ::جديد (get_ریښه()) TrieSubtrie(پیل_بټ, عدد_بټونه_2);
  }
};

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::ImplType &
Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::getOrCreateImpl() {
  إذا (ImplType *Impl = ImplPtr.بارول())
    أرجع *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, عدد_ریښه_بټونه);
  ImplType *ExistingImpl = تش_اشاره;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  إذا (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    أرجع *Impl.release();

  // Already created, return the existing ImplType.
  أرجع *ExistingImpl;
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ
Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::پیدا_کړه(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.تشه() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.بارول();
  إذا (!Impl)
    أرجع اشاره_ګر_بنسټ();

  TrieSubtrie *S = Impl->get_ریښه();
  Trie_Hash_شاخص_Generator شاخص_Gen{عدد_ریښه_بټونه, عدد_Subtrie_بټونه, Hash};
  size_t شاخص_2 = شاخص_Gen.بل();
  طالما (شاخص_2 != شاخص_Gen.پای_ټکی()) {
    // Try to set the content.
    Trie_غوټه *Existing = S->get(شاخص_2);
    إذا (!Existing)
      أرجع اشاره_ګر_بنسټ(S, شاخص_2, *شاخص_Gen.پیل_بټ);

    // Check for an exact match.
    إذا (خپلکاری *ExistingContent = dyn_cast<TrieContent>(Existing))
      أرجع ExistingContent->getHash() == Hash
                 ? اشاره_ګر_بنسټ(ExistingContent->get_ارزښت_اشاره_ګر())
                 : اشاره_ګر_بنسټ(S, شاخص_2, *شاخص_Gen.پیل_بټ);

    شاخص_2 = شاخص_Gen.بل();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::ورننباسه(
    اشاره_ګر_بنسټ Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.تشه() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ریښه();
  Trie_Hash_شاخص_Generator شاخص_Gen{عدد_ریښه_بټونه, عدد_Subtrie_بټونه, Hash};
  size_t شاخص_2;
  إذا (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    شاخص_2 = شاخص_Gen.hint(Hint.I, Hint.B);
  } وإلا {
    شاخص_2 = شاخص_Gen.بل();
  }

  طالما (شاخص_2 != شاخص_Gen.پای_ټکی()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = خطأ;
    Trie_غوټه &Existing = S->get(شاخص_2).بارول_Or_Generate([&]() {
      Generated = ریښتیا;

      // Construct the value itself at the tail.
      uint8_t *حافظه_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_اندازه, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(حافظه_2 + Content_دننه_واټن, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (حافظه_2)
          TrieContent(Content_دننه_واټن, Hash.اندازه_2(), HashStorage - حافظه_2);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      أرجع Content;
    });
    // If we just generated it, return it!
    إذا (Generated)
      أرجع اشاره_ګر_بنسټ(cast<TrieContent>(Existing).get_ارزښت_اشاره_ګر());

    إذا (خپلکاری *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      شاخص_2 = شاخص_Gen.بل();
      استمر;
    }

    // Return the existing content if it's an exact match!
    خپلکاری &ExistingContent = cast<TrieContent>(Existing);
    إذا (ExistingContent.getHash() == Hash)
      أرجع اشاره_ګر_بنسټ(ExistingContent.get_ارزښت_اشاره_ګر());

    // Sink the existing content as long as the indexes match.
    size_t بل_شاخص = شاخص_Gen.بل();
    طالما (بل_شاخص != شاخص_Gen.پای_ټکی()) {
      size_t New_شاخص_For_Existing_Content =
          شاخص_Gen.get_Colliding_بټونه(ExistingContent.getHash());
      S = S->sink(شاخص_2, ExistingContent, شاخص_Gen.get_عدد_بټونه(),
                  New_شاخص_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    أرجع Impl.save(std::move(S));
                  });
      شاخص_2 = بل_شاخص;

      // Found the difference.
      إذا (بل_شاخص != New_شاخص_For_Existing_Content)
        اكسر;

      بل_شاخص = شاخص_Gen.بل();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::Thread_خوندي_Trie_Raw_Hash_Map_بنسټ(
    size_t Content_Alloc_اندازه, size_t ContentAllocAlign, size_t Content_دننه_واټن,
    std::optional<size_t> عدد_ریښه_بټونه, std::optional<size_t> عدد_Subtrie_بټونه)
    : Content_Alloc_اندازه(Content_Alloc_اندازه), ContentAllocAlign(ContentAllocAlign),
      Content_دننه_واټن(Content_دننه_واټن),
      عدد_ریښه_بټونه(عدد_ریښه_بټونه.ارزښت_or(Default_عدد_ریښه_بټونه)),
      عدد_Subtrie_بټونه(عدد_Subtrie_بټونه.ارزښت_or(Default_عدد_Subtrie_بټونه)),
      ImplPtr(تش_اشاره) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!عدد_ریښه_بټونه || *عدد_ریښه_بټونه < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!عدد_Subtrie_بټونه || *عدد_Subtrie_بټونه < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::Thread_خوندي_Trie_Raw_Hash_Map_بنسټ(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ &&RHS)
    : Content_Alloc_اندازه(RHS.Content_Alloc_اندازه),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_دننه_واټن(RHS.Content_دننه_واټن), عدد_ریښه_بټونه(RHS.عدد_ریښه_بټونه),
      عدد_Subtrie_بټونه(RHS.عدد_Subtrie_بټونه) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(تش_اشاره);
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::~Thread_خوندي_Trie_Raw_Hash_Map_بنسټ() {
  أكد(!ImplPtr.بارول() && "Expected subclass to call destroyImpl()");
}

فراغ Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(تش_اشاره));
  إذا (!Impl)
    أرجع;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  إذا (Destructor)
    لكل (TrieSubtrie *Trie = Impl->get_ریښه(); Trie; Trie = Trie->بل_2.بارول())
      لكل (بینښه I = 0; I < Trie->اندازه_2(); ++I)
        إذا (خپلکاری *Content = dyn_cast_or_null<TrieContent>(Trie->بارول(I)))
          Destructor(Content->get_ارزښت_اشاره_ګر());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ریښه()->بل_2;
  طالما (Trie) {
    TrieSubtrie *بل_2 = Trie->بل_2.exchange(تش_اشاره);
    ړنګول Trie;
    Trie = بل_2;
  }
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ
Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_ریښه() ثابت {
  ImplType *Impl = ImplPtr.بارول();
  إذا (!Impl)
    أرجع اشاره_ګر_بنسټ();
  أرجع اشاره_ګر_بنسټ(Impl->get_ریښه());
}

بینښه Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_پیل_بټ(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خپلکاری *S = dyn_cast<TrieSubtrie>((Trie_غوټه *)P.P))
    أرجع S->پیل_بټ;
  أرجع 0;
}

بینښه Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_عدد_بټونه(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خپلکاری *S = dyn_cast<TrieSubtrie>((Trie_غوټه *)P.P))
    أرجع S->عدد_بټونه_2;
  أرجع 0;
}

بینښه Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_عدد_خونه_کارول_شوی(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  خپلکاری *S = dyn_cast<TrieSubtrie>((Trie_غوټه *)P.P);
  إذا (!S)
    أرجع 0;
  بینښه عدد = 0;
  لكل (بینښه I = 0, E = S->اندازه_2(); I < E; ++I)
    إذا (S->بارول(I))
      ++عدد;
  أرجع عدد;
}

std::string Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_Trie_مختاړی_As_String(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع "";

  خپلکاری *S = dyn_cast<TrieSubtrie>((Trie_غوټه *)P.P);
  إذا (!S || !S->IsSubtrie)
    أرجع "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *غوټه = تش_اشاره;
  طالما (Current) {
    TrieSubtrie *بل_2 = تش_اشاره;
    // Find first used slot in the trie.
    لكل (بینښه I = 0, E = Current->اندازه_2(); I < E; ++I) {
      خپلکاری *S = Current->بارول(I);
      إذا (!S)
        استمر;

      إذا (خپلکاری *Content = dyn_cast<TrieContent>(S))
        غوټه = Content;
      وإلا إذا (خپلکاری *Sub = dyn_cast<TrieSubtrie>(S))
        بل_2 = Sub;
      اكسر;
    }

    // Found the node.
    إذا (غوټه)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = بل_2;
  }

  أكد(غوټه && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  بینښه پیل_ډک_بایټونه = (S->پیل_بټ + 1) / 8 - 1;
  SS << منزل_Hex(منزل_String_Ref(غوټه->getHash()).take_front(پیل_ډک_بایټونه),
              /*LowerCase=*/ریښتیا);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string بټونه_2;
  لكل (بینښه I = پیل_ډک_بایټونه * 8, E = S->پیل_بټ; I < E; ++I) {
    بینښه شاخص_2 = I / 8;
    بینښه دننه_واټن = 7 - I % 8;
    بټونه_2.ټېل_شا('0' + ((غوټه->getHash()[شاخص_2] >> دننه_واټن) & 1));
  }

  إذا (!بټونه_2.تشه())
    SS << "[" << بټونه_2 << "]";

  أرجع SS.str();
}

بینښه Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_عدد_Tries() ثابت {
  ImplType *Impl = ImplPtr.بارول();
  إذا (!Impl)
    أرجع 0;
  بینښه عدد = 0;
  لكل (TrieSubtrie *Trie = Impl->get_ریښه(); Trie; Trie = Trie->بل_2.بارول())
    ++عدد;
  أرجع عدد;
}

Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ
Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::get_بل_Trie(
    Thread_خوندي_Trie_Raw_Hash_Map_بنسټ::اشاره_ګر_بنسټ P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع اشاره_ګر_بنسټ();
  خپلکاری *S = dyn_cast<TrieSubtrie>((Trie_غوټه *)P.P);
  إذا (!S)
    أرجع اشاره_ګر_بنسټ();
  إذا (خپلکاری *E = S->بل_2.بارول())
    أرجع اشاره_ګر_بنسټ(E);
  أرجع اشاره_ګر_بنسټ();
}
