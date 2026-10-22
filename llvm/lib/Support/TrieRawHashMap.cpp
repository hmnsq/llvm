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

استعمال نطاق llvm;

نطاق {
بنية Trie_ڳنڍ {
  ثابت منطقي IsSubtrie = خطأ;

  Trie_ڳنڍ(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *عامل جديد(size_t سائيز) { أرجع ::عامل جديد(سائيز); }
  فراغ عامل ميٽيو(فراغ *Ptr) { ::عامل ميٽيو(Ptr); }
};

بنية TrieContent final : عام Trie_ڳنڍ {
  ثابت uint8_t Content_وٿي;
  ثابت uint8_t Hash_سائيز;
  ثابت uint8_t Hash_وٿي;

  فراغ *get_قدر_اشارو() ثابت {
    خودڪار *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + Content_وٿي;
    أرجع const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    خودڪار *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + Hash_وٿي;
    أرجع ArrayRef(Begin, Begin + Hash_سائيز);
  }

  TrieContent(size_t Content_وٿي, size_t Hash_سائيز, size_t Hash_وٿي)
      : Trie_ڳنڍ(/*IsSubtrie=*/خطأ), Content_وٿي(Content_وٿي),
        Hash_سائيز(Hash_سائيز), Hash_وٿي(Hash_وٿي) {}

  ساكن منطقي classof(ثابت Trie_ڳنڍ *TN) { أرجع !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::Trie_Content_بنياد_سائيز,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام Trie_ڳنڍ,
      خاص TrailingObjects<TrieSubtrie, Lazy_Atomic_اشارو<Trie_ڳنڍ>> {
عام:
  استعمال جڳهه = Lazy_Atomic_اشارو<Trie_ڳنڍ>;

  جڳهه &get(size_t I) { أرجع getTrailingObjects()[I]; }
  Trie_ڳنڍ *ڀريو(size_t I) { أرجع get(I).ڀريو(); }

  بنانشان سائيز_2() ثابت { أرجع سائيز; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t انگ_Subtrie_بٽ, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t شروعات_ٻٽو_انگ, size_t انگ_بٽ_2);

  واضح TrieSubtrie(size_t شروعات_ٻٽو_انگ, size_t انگ_بٽ_2);

  ساكن منطقي classof(ثابت Trie_ڳنڍ *TN) { أرجع TN->IsSubtrie; }

  ساكن constexpr size_t سائيز_منزل_Alloc(بنانشان انگ_بٽ_2) {
    أكد(انگ_بٽ_2 < 20 && "Tries should have fewer than ~1M slots");
    بنانشان ڳڻپ_2 = 1u << انگ_بٽ_2;
    أرجع ڪل_سائيز_منزل_Alloc<Lazy_Atomic_اشارو<Trie_ڳنڍ>>(ڳڻپ_2);
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
  بنانشان شروعات_ٻٽو_انگ = 0;
  بنانشان انگ_بٽ_2 = 0;
  بنانشان سائيز = 0;
  دوست صنف llvm::Thread_محفوظ_Trie_Raw_Hash_Map_بنياد;
  دوست صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> ايندڙ_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t شروعات_ٻٽو_انگ,
                                                 size_t انگ_بٽ_2) {
  فراغ *يادگيري_2 = ::عامل جديد(سائيز_منزل_Alloc(انگ_بٽ_2));
  TrieSubtrie *S = ::جديد (يادگيري_2) TrieSubtrie(شروعات_ٻٽو_انگ, انگ_بٽ_2);
  أرجع std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t شروعات_ٻٽو_انگ, size_t انگ_بٽ_2)
    : Trie_ڳنڍ(سچ), شروعات_ٻٽو_انگ(شروعات_ٻٽو_انگ), انگ_بٽ_2(انگ_بٽ_2), سائيز(1u << انگ_بٽ_2),
      ايندڙ_2(خالي_اشارو) {
  لكل (بنانشان I = 0; I < سائيز; ++I)
    جديد (&get(I)) جڳهه(خالي_اشارو);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_اشارو<Trie_ڳنڍ>>::قدر_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t انگ_Subtrie_بٽ, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(انگ_Subtrie_بٽ > 0);
  std::unique_ptr<TrieSubtrie> S = create(شروعات_ٻٽو_انگ + انگ_بٽ_2, انگ_Subtrie_بٽ);

  أكد(NewI < سائيز);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ڳنڍ *Existing_ڳنڍ = &Content;
  أكد(I < سائيز);
  إذا (get(I).compare_exchange_strong(Existing_ڳنڍ, S.get()))
    أرجع Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  أرجع cast<TrieSubtrie>(Existing_ڳنڍ);
}

صنف Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::ImplType final
    : خاص TrailingObjects<Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t شروعات_ٻٽو_انگ, size_t انگ_بٽ_2) {
    size_t سائيز = حجم(ImplType) + TrieSubtrie::سائيز_منزل_Alloc(انگ_بٽ_2);
    فراغ *يادگيري_2 = ::عامل جديد(سائيز);
    ImplType *Impl = ::جديد (يادگيري_2) ImplType(شروعات_ٻٽو_انگ, انگ_بٽ_2);
    أرجع std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->ايندڙ_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_سر = خالي_اشارو;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    طالما (!get_پاڙ()->ايندڙ_2.compare_exchange_weak(Current_سر, S.get()))
      S->ايندڙ_2.exchange(Current_سر);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    أرجع S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_پاڙ() { أرجع getTrailingObjects(); }

  ساكن فراغ *عامل جديد(size_t سائيز) { أرجع ::عامل جديد(سائيز); }
  فراغ عامل ميٽيو(فراغ *Ptr) { ::عامل ميٽيو(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_محفوظ_Allocator<BumpPtrAllocator> ContentAlloc;

خاص:
  دوست صنف TrailingObjects;

  ImplType(size_t شروعات_ٻٽو_انگ, size_t انگ_بٽ_2) {
    ::جديد (get_پاڙ()) TrieSubtrie(شروعات_ٻٽو_انگ, انگ_بٽ_2);
  }
};

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::ImplType &
Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::getOrCreateImpl() {
  إذا (ImplType *Impl = ImplPtr.ڀريو())
    أرجع *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, انگ_پاڙ_بٽ);
  ImplType *ExistingImpl = خالي_اشارو;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  إذا (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    أرجع *Impl.release();

  // Already created, return the existing ImplType.
  أرجع *ExistingImpl;
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد
Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::ڳوليو(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.خالي_جايون() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.ڀريو();
  إذا (!Impl)
    أرجع اشارو_بنياد();

  TrieSubtrie *S = Impl->get_پاڙ();
  Trie_Hash_اشاريو_Generator اشاريو_Gen{انگ_پاڙ_بٽ, انگ_Subtrie_بٽ, Hash};
  size_t اشاريو_2 = اشاريو_Gen.ايندڙ();
  طالما (اشاريو_2 != اشاريو_Gen.پڇاڙي()) {
    // Try to set the content.
    Trie_ڳنڍ *Existing = S->get(اشاريو_2);
    إذا (!Existing)
      أرجع اشارو_بنياد(S, اشاريو_2, *اشاريو_Gen.شروعات_ٻٽو_انگ);

    // Check for an exact match.
    إذا (خودڪار *ExistingContent = dyn_cast<TrieContent>(Existing))
      أرجع ExistingContent->getHash() == Hash
                 ? اشارو_بنياد(ExistingContent->get_قدر_اشارو())
                 : اشارو_بنياد(S, اشاريو_2, *اشاريو_Gen.شروعات_ٻٽو_انگ);

    اشاريو_2 = اشاريو_Gen.ايندڙ();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::وجهو(
    اشارو_بنياد Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.خالي_جايون() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_پاڙ();
  Trie_Hash_اشاريو_Generator اشاريو_Gen{انگ_پاڙ_بٽ, انگ_Subtrie_بٽ, Hash};
  size_t اشاريو_2;
  إذا (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    اشاريو_2 = اشاريو_Gen.hint(Hint.I, Hint.B);
  } وإلا {
    اشاريو_2 = اشاريو_Gen.ايندڙ();
  }

  طالما (اشاريو_2 != اشاريو_Gen.پڇاڙي()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = خطأ;
    Trie_ڳنڍ &Existing = S->get(اشاريو_2).ڀريو_Or_Generate([&]() {
      Generated = سچ;

      // Construct the value itself at the tail.
      uint8_t *يادگيري_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_سائيز, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(يادگيري_2 + Content_وٿي, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (يادگيري_2)
          TrieContent(Content_وٿي, Hash.سائيز_2(), HashStorage - يادگيري_2);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      أرجع Content;
    });
    // If we just generated it, return it!
    إذا (Generated)
      أرجع اشارو_بنياد(cast<TrieContent>(Existing).get_قدر_اشارو());

    إذا (خودڪار *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      اشاريو_2 = اشاريو_Gen.ايندڙ();
      استمر;
    }

    // Return the existing content if it's an exact match!
    خودڪار &ExistingContent = cast<TrieContent>(Existing);
    إذا (ExistingContent.getHash() == Hash)
      أرجع اشارو_بنياد(ExistingContent.get_قدر_اشارو());

    // Sink the existing content as long as the indexes match.
    size_t ايندڙ_اشاريو = اشاريو_Gen.ايندڙ();
    طالما (ايندڙ_اشاريو != اشاريو_Gen.پڇاڙي()) {
      size_t New_اشاريو_For_Existing_Content =
          اشاريو_Gen.get_Colliding_بٽ(ExistingContent.getHash());
      S = S->sink(اشاريو_2, ExistingContent, اشاريو_Gen.get_انگ_بٽ(),
                  New_اشاريو_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    أرجع Impl.save(std::move(S));
                  });
      اشاريو_2 = ايندڙ_اشاريو;

      // Found the difference.
      إذا (ايندڙ_اشاريو != New_اشاريو_For_Existing_Content)
        اكسر;

      ايندڙ_اشاريو = اشاريو_Gen.ايندڙ();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::Thread_محفوظ_Trie_Raw_Hash_Map_بنياد(
    size_t Content_Alloc_سائيز, size_t ContentAllocAlign, size_t Content_وٿي,
    std::optional<size_t> انگ_پاڙ_بٽ, std::optional<size_t> انگ_Subtrie_بٽ)
    : Content_Alloc_سائيز(Content_Alloc_سائيز), ContentAllocAlign(ContentAllocAlign),
      Content_وٿي(Content_وٿي),
      انگ_پاڙ_بٽ(انگ_پاڙ_بٽ.قدر_or(Default_انگ_پاڙ_بٽ)),
      انگ_Subtrie_بٽ(انگ_Subtrie_بٽ.قدر_or(Default_انگ_Subtrie_بٽ)),
      ImplPtr(خالي_اشارو) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!انگ_پاڙ_بٽ || *انگ_پاڙ_بٽ < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!انگ_Subtrie_بٽ || *انگ_Subtrie_بٽ < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::Thread_محفوظ_Trie_Raw_Hash_Map_بنياد(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد &&RHS)
    : Content_Alloc_سائيز(RHS.Content_Alloc_سائيز),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_وٿي(RHS.Content_وٿي), انگ_پاڙ_بٽ(RHS.انگ_پاڙ_بٽ),
      انگ_Subtrie_بٽ(RHS.انگ_Subtrie_بٽ) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(خالي_اشارو);
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::~Thread_محفوظ_Trie_Raw_Hash_Map_بنياد() {
  أكد(!ImplPtr.ڀريو() && "Expected subclass to call destroyImpl()");
}

فراغ Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(خالي_اشارو));
  إذا (!Impl)
    أرجع;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  إذا (Destructor)
    لكل (TrieSubtrie *Trie = Impl->get_پاڙ(); Trie; Trie = Trie->ايندڙ_2.ڀريو())
      لكل (بنانشان I = 0; I < Trie->سائيز_2(); ++I)
        إذا (خودڪار *Content = dyn_cast_or_null<TrieContent>(Trie->ڀريو(I)))
          Destructor(Content->get_قدر_اشارو());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_پاڙ()->ايندڙ_2;
  طالما (Trie) {
    TrieSubtrie *ايندڙ_2 = Trie->ايندڙ_2.exchange(خالي_اشارو);
    ميٽيو Trie;
    Trie = ايندڙ_2;
  }
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد
Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_پاڙ() ثابت {
  ImplType *Impl = ImplPtr.ڀريو();
  إذا (!Impl)
    أرجع اشارو_بنياد();
  أرجع اشارو_بنياد(Impl->get_پاڙ());
}

بنانشان Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_شروعات_ٻٽو_انگ(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودڪار *S = dyn_cast<TrieSubtrie>((Trie_ڳنڍ *)P.P))
    أرجع S->شروعات_ٻٽو_انگ;
  أرجع 0;
}

بنانشان Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_انگ_بٽ(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودڪار *S = dyn_cast<TrieSubtrie>((Trie_ڳنڍ *)P.P))
    أرجع S->انگ_بٽ_2;
  أرجع 0;
}

بنانشان Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_انگ_جڳهه_استعمال_ٿيل(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  خودڪار *S = dyn_cast<TrieSubtrie>((Trie_ڳنڍ *)P.P);
  إذا (!S)
    أرجع 0;
  بنانشان انگ = 0;
  لكل (بنانشان I = 0, E = S->سائيز_2(); I < E; ++I)
    إذا (S->ڀريو(I))
      ++انگ;
  أرجع انگ;
}

std::string Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_Trie_اڳياڙي_As_String(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع "";

  خودڪار *S = dyn_cast<TrieSubtrie>((Trie_ڳنڍ *)P.P);
  إذا (!S || !S->IsSubtrie)
    أرجع "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ڳنڍ = خالي_اشارو;
  طالما (Current) {
    TrieSubtrie *ايندڙ_2 = خالي_اشارو;
    // Find first used slot in the trie.
    لكل (بنانشان I = 0, E = Current->سائيز_2(); I < E; ++I) {
      خودڪار *S = Current->ڀريو(I);
      إذا (!S)
        استمر;

      إذا (خودڪار *Content = dyn_cast<TrieContent>(S))
        ڳنڍ = Content;
      وإلا إذا (خودڪار *Sub = dyn_cast<TrieSubtrie>(S))
        ايندڙ_2 = Sub;
      اكسر;
    }

    // Found the node.
    إذا (ڳنڍ)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = ايندڙ_2;
  }

  أكد(ڳنڍ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  بنانشان شروعات_ڀريل_بائيٽون = (S->شروعات_ٻٽو_انگ + 1) / 8 - 1;
  SS << منزل_Hex(منزل_String_Ref(ڳنڍ->getHash()).take_front(شروعات_ڀريل_بائيٽون),
              /*LowerCase=*/سچ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string بٽ_2;
  لكل (بنانشان I = شروعات_ڀريل_بائيٽون * 8, E = S->شروعات_ٻٽو_انگ; I < E; ++I) {
    بنانشان اشاريو_2 = I / 8;
    بنانشان وٿي = 7 - I % 8;
    بٽ_2.ڌڪو_پٺ('0' + ((ڳنڍ->getHash()[اشاريو_2] >> وٿي) & 1));
  }

  إذا (!بٽ_2.خالي_جايون())
    SS << "[" << بٽ_2 << "]";

  أرجع SS.str();
}

بنانشان Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_انگ_Tries() ثابت {
  ImplType *Impl = ImplPtr.ڀريو();
  إذا (!Impl)
    أرجع 0;
  بنانشان انگ = 0;
  لكل (TrieSubtrie *Trie = Impl->get_پاڙ(); Trie; Trie = Trie->ايندڙ_2.ڀريو())
    ++انگ;
  أرجع انگ;
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد
Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::get_ايندڙ_Trie(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنياد::اشارو_بنياد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع اشارو_بنياد();
  خودڪار *S = dyn_cast<TrieSubtrie>((Trie_ڳنڍ *)P.P);
  إذا (!S)
    أرجع اشارو_بنياد();
  إذا (خودڪار *E = S->ايندڙ_2.ڀريو())
    أرجع اشارو_بنياد(E);
  أرجع اشارو_بنياد();
}
