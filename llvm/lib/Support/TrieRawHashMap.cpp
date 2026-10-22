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

ورتوں نطاق llvm;

نطاق {
بنية Trie_گرہ {
  ثابت منطقي IsSubtrie = خطأ;

  Trie_گرہ(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *عامل جديد(size_t ناپ) { أرجع ::عامل جديد(ناپ); }
  فراغ عامل مٹاؤ(فراغ *Ptr) { ::عامل مٹاؤ(Ptr); }
};

بنية TrieContent final : عام Trie_گرہ {
  ثابت uint8_t Content_وتھ;
  ثابت uint8_t Hash_ناپ;
  ثابت uint8_t Hash_وتھ;

  فراغ *get_قدر_اشارہ_گر() ثابت {
    خودکار *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + Content_وتھ;
    أرجع const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    خودکار *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + Hash_وتھ;
    أرجع ArrayRef(Begin, Begin + Hash_ناپ);
  }

  TrieContent(size_t Content_وتھ, size_t Hash_ناپ, size_t Hash_وتھ)
      : Trie_گرہ(/*IsSubtrie=*/خطأ), Content_وتھ(Content_وتھ),
        Hash_ناپ(Hash_ناپ), Hash_وتھ(Hash_وتھ) {}

  ساكن منطقي classof(ثابت Trie_گرہ *TN) { أرجع !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::Trie_Content_ادھار_ناپ,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام Trie_گرہ,
      خاص TrailingObjects<TrieSubtrie, Lazy_Atomic_اشارہ_گر<Trie_گرہ>> {
عام:
  ورتوں تھاں = Lazy_Atomic_اشارہ_گر<Trie_گرہ>;

  تھاں &get(size_t I) { أرجع getTrailingObjects()[I]; }
  Trie_گرہ *لدو(size_t I) { أرجع get(I).لدو(); }

  بنانشانی ناپ_2() ثابت { أرجع ناپ; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t عدد_Subtrie_بٹ, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t شروع_ثنائی_ہندسہ, size_t عدد_بٹ_2);

  واضح TrieSubtrie(size_t شروع_ثنائی_ہندسہ, size_t عدد_بٹ_2);

  ساكن منطقي classof(ثابت Trie_گرہ *TN) { أرجع TN->IsSubtrie; }

  ساكن constexpr size_t ناپ_منزل_Alloc(بنانشانی عدد_بٹ_2) {
    أكد(عدد_بٹ_2 < 20 && "Tries should have fewer than ~1M slots");
    بنانشانی گنتی_2 = 1u << عدد_بٹ_2;
    أرجع کل_ناپ_منزل_Alloc<Lazy_Atomic_اشارہ_گر<Trie_گرہ>>(گنتی_2);
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
  بنانشانی شروع_ثنائی_ہندسہ = 0;
  بنانشانی عدد_بٹ_2 = 0;
  بنانشانی ناپ = 0;
  یار صنف llvm::Thread_محفوظ_Trie_Raw_Hash_Map_ادھار;
  یار صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> اگلا_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t شروع_ثنائی_ہندسہ,
                                                 size_t عدد_بٹ_2) {
  فراغ *حافظہ_2 = ::عامل جديد(ناپ_منزل_Alloc(عدد_بٹ_2));
  TrieSubtrie *S = ::جديد (حافظہ_2) TrieSubtrie(شروع_ثنائی_ہندسہ, عدد_بٹ_2);
  أرجع std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t شروع_ثنائی_ہندسہ, size_t عدد_بٹ_2)
    : Trie_گرہ(سچ), شروع_ثنائی_ہندسہ(شروع_ثنائی_ہندسہ), عدد_بٹ_2(عدد_بٹ_2), ناپ(1u << عدد_بٹ_2),
      اگلا_2(خالی_سوچک) {
  لكل (بنانشانی I = 0; I < ناپ; ++I)
    جديد (&get(I)) تھاں(خالی_سوچک);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_اشارہ_گر<Trie_گرہ>>::قدر_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t عدد_Subtrie_بٹ, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(عدد_Subtrie_بٹ > 0);
  std::unique_ptr<TrieSubtrie> S = create(شروع_ثنائی_ہندسہ + عدد_بٹ_2, عدد_Subtrie_بٹ);

  أكد(NewI < ناپ);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_گرہ *Existing_گرہ = &Content;
  أكد(I < ناپ);
  إذا (get(I).compare_exchange_strong(Existing_گرہ, S.get()))
    أرجع Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  أرجع cast<TrieSubtrie>(Existing_گرہ);
}

صنف Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::ImplType final
    : خاص TrailingObjects<Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t شروع_ثنائی_ہندسہ, size_t عدد_بٹ_2) {
    size_t ناپ = حجم(ImplType) + TrieSubtrie::ناپ_منزل_Alloc(عدد_بٹ_2);
    فراغ *حافظہ_2 = ::عامل جديد(ناپ);
    ImplType *Impl = ::جديد (حافظہ_2) ImplType(شروع_ثنائی_ہندسہ, عدد_بٹ_2);
    أرجع std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->اگلا_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_سر = خالی_سوچک;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    طالما (!get_جڑ()->اگلا_2.compare_exchange_weak(Current_سر, S.get()))
      S->اگلا_2.exchange(Current_سر);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    أرجع S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_جڑ() { أرجع getTrailingObjects(); }

  ساكن فراغ *عامل جديد(size_t ناپ) { أرجع ::عامل جديد(ناپ); }
  فراغ عامل مٹاؤ(فراغ *Ptr) { ::عامل مٹاؤ(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_محفوظ_Allocator<BumpPtrAllocator> ContentAlloc;

خاص:
  یار صنف TrailingObjects;

  ImplType(size_t شروع_ثنائی_ہندسہ, size_t عدد_بٹ_2) {
    ::جديد (get_جڑ()) TrieSubtrie(شروع_ثنائی_ہندسہ, عدد_بٹ_2);
  }
};

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::ImplType &
Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::getOrCreateImpl() {
  إذا (ImplType *Impl = ImplPtr.لدو())
    أرجع *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, عدد_جڑ_بٹ);
  ImplType *ExistingImpl = خالی_سوچک;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  إذا (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    أرجع *Impl.release();

  // Already created, return the existing ImplType.
  أرجع *ExistingImpl;
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار
Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::لبھو(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.خالی_تھاواں() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.لدو();
  إذا (!Impl)
    أرجع اشارہ_گر_ادھار();

  TrieSubtrie *S = Impl->get_جڑ();
  Trie_Hash_اشاریہ_Generator اشاریہ_Gen{عدد_جڑ_بٹ, عدد_Subtrie_بٹ, Hash};
  size_t اشاریہ_2 = اشاریہ_Gen.اگلا();
  طالما (اشاریہ_2 != اشاریہ_Gen.سرا()) {
    // Try to set the content.
    Trie_گرہ *Existing = S->get(اشاریہ_2);
    إذا (!Existing)
      أرجع اشارہ_گر_ادھار(S, اشاریہ_2, *اشاریہ_Gen.شروع_ثنائی_ہندسہ);

    // Check for an exact match.
    إذا (خودکار *ExistingContent = dyn_cast<TrieContent>(Existing))
      أرجع ExistingContent->getHash() == Hash
                 ? اشارہ_گر_ادھار(ExistingContent->get_قدر_اشارہ_گر())
                 : اشارہ_گر_ادھار(S, اشاریہ_2, *اشاریہ_Gen.شروع_ثنائی_ہندسہ);

    اشاریہ_2 = اشاریہ_Gen.اگلا();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::داخل_کرو(
    اشارہ_گر_ادھار Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.خالی_تھاواں() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_جڑ();
  Trie_Hash_اشاریہ_Generator اشاریہ_Gen{عدد_جڑ_بٹ, عدد_Subtrie_بٹ, Hash};
  size_t اشاریہ_2;
  إذا (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    اشاریہ_2 = اشاریہ_Gen.hint(Hint.I, Hint.B);
  } وإلا {
    اشاریہ_2 = اشاریہ_Gen.اگلا();
  }

  طالما (اشاریہ_2 != اشاریہ_Gen.سرا()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = خطأ;
    Trie_گرہ &Existing = S->get(اشاریہ_2).لدو_Or_Generate([&]() {
      Generated = سچ;

      // Construct the value itself at the tail.
      uint8_t *حافظہ_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_ناپ, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(حافظہ_2 + Content_وتھ, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (حافظہ_2)
          TrieContent(Content_وتھ, Hash.ناپ_2(), HashStorage - حافظہ_2);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      أرجع Content;
    });
    // If we just generated it, return it!
    إذا (Generated)
      أرجع اشارہ_گر_ادھار(cast<TrieContent>(Existing).get_قدر_اشارہ_گر());

    إذا (خودکار *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      اشاریہ_2 = اشاریہ_Gen.اگلا();
      استمر;
    }

    // Return the existing content if it's an exact match!
    خودکار &ExistingContent = cast<TrieContent>(Existing);
    إذا (ExistingContent.getHash() == Hash)
      أرجع اشارہ_گر_ادھار(ExistingContent.get_قدر_اشارہ_گر());

    // Sink the existing content as long as the indexes match.
    size_t اگلا_اشاریہ = اشاریہ_Gen.اگلا();
    طالما (اگلا_اشاریہ != اشاریہ_Gen.سرا()) {
      size_t New_اشاریہ_For_Existing_Content =
          اشاریہ_Gen.get_Colliding_بٹ(ExistingContent.getHash());
      S = S->sink(اشاریہ_2, ExistingContent, اشاریہ_Gen.get_عدد_بٹ(),
                  New_اشاریہ_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    أرجع Impl.save(std::move(S));
                  });
      اشاریہ_2 = اگلا_اشاریہ;

      // Found the difference.
      إذا (اگلا_اشاریہ != New_اشاریہ_For_Existing_Content)
        اكسر;

      اگلا_اشاریہ = اشاریہ_Gen.اگلا();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::Thread_محفوظ_Trie_Raw_Hash_Map_ادھار(
    size_t Content_Alloc_ناپ, size_t ContentAllocAlign, size_t Content_وتھ,
    std::optional<size_t> عدد_جڑ_بٹ, std::optional<size_t> عدد_Subtrie_بٹ)
    : Content_Alloc_ناپ(Content_Alloc_ناپ), ContentAllocAlign(ContentAllocAlign),
      Content_وتھ(Content_وتھ),
      عدد_جڑ_بٹ(عدد_جڑ_بٹ.قدر_or(Default_عدد_جڑ_بٹ)),
      عدد_Subtrie_بٹ(عدد_Subtrie_بٹ.قدر_or(Default_عدد_Subtrie_بٹ)),
      ImplPtr(خالی_سوچک) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!عدد_جڑ_بٹ || *عدد_جڑ_بٹ < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!عدد_Subtrie_بٹ || *عدد_Subtrie_بٹ < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::Thread_محفوظ_Trie_Raw_Hash_Map_ادھار(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار &&RHS)
    : Content_Alloc_ناپ(RHS.Content_Alloc_ناپ),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_وتھ(RHS.Content_وتھ), عدد_جڑ_بٹ(RHS.عدد_جڑ_بٹ),
      عدد_Subtrie_بٹ(RHS.عدد_Subtrie_بٹ) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(خالی_سوچک);
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::~Thread_محفوظ_Trie_Raw_Hash_Map_ادھار() {
  أكد(!ImplPtr.لدو() && "Expected subclass to call destroyImpl()");
}

فراغ Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(خالی_سوچک));
  إذا (!Impl)
    أرجع;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  إذا (Destructor)
    لكل (TrieSubtrie *Trie = Impl->get_جڑ(); Trie; Trie = Trie->اگلا_2.لدو())
      لكل (بنانشانی I = 0; I < Trie->ناپ_2(); ++I)
        إذا (خودکار *Content = dyn_cast_or_null<TrieContent>(Trie->لدو(I)))
          Destructor(Content->get_قدر_اشارہ_گر());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_جڑ()->اگلا_2;
  طالما (Trie) {
    TrieSubtrie *اگلا_2 = Trie->اگلا_2.exchange(خالی_سوچک);
    مٹاؤ Trie;
    Trie = اگلا_2;
  }
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار
Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_جڑ() ثابت {
  ImplType *Impl = ImplPtr.لدو();
  إذا (!Impl)
    أرجع اشارہ_گر_ادھار();
  أرجع اشارہ_گر_ادھار(Impl->get_جڑ());
}

بنانشانی Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_شروع_ثنائی_ہندسہ(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P))
    أرجع S->شروع_ثنائی_ہندسہ;
  أرجع 0;
}

بنانشانی Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_عدد_بٹ(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P))
    أرجع S->عدد_بٹ_2;
  أرجع 0;
}

بنانشانی Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_عدد_تھاں_ورتیا(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  إذا (!S)
    أرجع 0;
  بنانشانی عدد = 0;
  لكل (بنانشانی I = 0, E = S->ناپ_2(); I < E; ++I)
    إذا (S->لدو(I))
      ++عدد;
  أرجع عدد;
}

std::string Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_Trie_اگیتر_As_String(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع "";

  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  إذا (!S || !S->IsSubtrie)
    أرجع "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *گرہ = خالی_سوچک;
  طالما (Current) {
    TrieSubtrie *اگلا_2 = خالی_سوچک;
    // Find first used slot in the trie.
    لكل (بنانشانی I = 0, E = Current->ناپ_2(); I < E; ++I) {
      خودکار *S = Current->لدو(I);
      إذا (!S)
        استمر;

      إذا (خودکار *Content = dyn_cast<TrieContent>(S))
        گرہ = Content;
      وإلا إذا (خودکار *Sub = dyn_cast<TrieSubtrie>(S))
        اگلا_2 = Sub;
      اكسر;
    }

    // Found the node.
    إذا (گرہ)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = اگلا_2;
  }

  أكد(گرہ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  بنانشانی شروع_بھریا_بائٹاں = (S->شروع_ثنائی_ہندسہ + 1) / 8 - 1;
  SS << منزل_Hex(منزل_String_Ref(گرہ->getHash()).take_front(شروع_بھریا_بائٹاں),
              /*LowerCase=*/سچ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string بٹ_2;
  لكل (بنانشانی I = شروع_بھریا_بائٹاں * 8, E = S->شروع_ثنائی_ہندسہ; I < E; ++I) {
    بنانشانی اشاریہ_2 = I / 8;
    بنانشانی وتھ = 7 - I % 8;
    بٹ_2.ڈالو_پٹھ('0' + ((گرہ->getHash()[اشاریہ_2] >> وتھ) & 1));
  }

  إذا (!بٹ_2.خالی_تھاواں())
    SS << "[" << بٹ_2 << "]";

  أرجع SS.str();
}

بنانشانی Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_عدد_Tries() ثابت {
  ImplType *Impl = ImplPtr.لدو();
  إذا (!Impl)
    أرجع 0;
  بنانشانی عدد = 0;
  لكل (TrieSubtrie *Trie = Impl->get_جڑ(); Trie; Trie = Trie->اگلا_2.لدو())
    ++عدد;
  أرجع عدد;
}

Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار
Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::get_اگلا_Trie(
    Thread_محفوظ_Trie_Raw_Hash_Map_ادھار::اشارہ_گر_ادھار P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع اشارہ_گر_ادھار();
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  إذا (!S)
    أرجع اشارہ_گر_ادھار();
  إذا (خودکار *E = S->اگلا_2.لدو())
    أرجع اشارہ_گر_ادھار(E);
  أرجع اشارہ_گر_ادھار();
}
