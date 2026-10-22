//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/TrieRawHashMap.h"
#شامل "llvm/ADT/LazyAtomicPointer.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/TrieHashIndexGenerator.h"
#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/ThreadSafeAllocator.h"
#شامل "llvm/Support/TrailingObjects.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <memory>

استعمال نامخلا llvm;

نامخلا {
ساخت Trie_گرہ {
  ثابت منطقی IsSubtrie = جھوٹ;

  Trie_گرہ(منطقی IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساکن خالی *عامل نیا(size_t سائز) { واپس ::عامل نیا(سائز); }
  خالی عامل حذف(خالی *Ptr) { ::عامل حذف(Ptr); }
};

ساخت TrieContent final : عوامی Trie_گرہ {
  ثابت uint8_t Content_انحراف;
  ثابت uint8_t Hash_سائز;
  ثابت uint8_t Hash_انحراف;

  خالی *get_قدر_اشارہ_گر() ثابت {
    خودکار *Content = reinterpret_cast<ثابت uint8_t *>(یہ) + Content_انحراف;
    واپس const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    خودکار *Begin = reinterpret_cast<ثابت uint8_t *>(یہ) + Hash_انحراف;
    واپس ArrayRef(Begin, Begin + Hash_سائز);
  }

  TrieContent(size_t Content_انحراف, size_t Hash_سائز, size_t Hash_انحراف)
      : Trie_گرہ(/*IsSubtrie=*/جھوٹ), Content_انحراف(Content_انحراف),
        Hash_سائز(Hash_سائز), Hash_انحراف(Hash_انحراف) {}

  ساکن منطقی classof(ثابت Trie_گرہ *TN) { واپس !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::Trie_Content_بنیاد_سائز,
              "Check header assumption!");

جماعت TrieSubtrie final
    : عوامی Trie_گرہ,
      نجی TrailingObjects<TrieSubtrie, Lazy_Atomic_اشارہ_گر<Trie_گرہ>> {
عوامی:
  استعمال خانہ = Lazy_Atomic_اشارہ_گر<Trie_گرہ>;

  خانہ &get(size_t I) { واپس getTrailingObjects()[I]; }
  Trie_گرہ *بھرو(size_t I) { واپس get(I).بھرو(); }

  غیرعلامتی سائز_2() ثابت { واپس سائز; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t عدد_Subtrie_بٹ, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساکن std::unique_ptr<TrieSubtrie> create(size_t آغاز_ثنائی_ہندسہ, size_t عدد_بٹ_2);

  صریح TrieSubtrie(size_t آغاز_ثنائی_ہندسہ, size_t عدد_بٹ_2);

  ساکن منطقی classof(ثابت Trie_گرہ *TN) { واپس TN->IsSubtrie; }

  ساکن constexpr size_t سائز_مقصد_Alloc(غیرعلامتی عدد_بٹ_2) {
    أكد(عدد_بٹ_2 < 20 && "Tries should have fewer than ~1M slots");
    غیرعلامتی گنتی_2 = 1u << عدد_بٹ_2;
    واپس کل_سائز_مقصد_Alloc<Lazy_Atomic_اشارہ_گر<Trie_گرہ>>(گنتی_2);
  }

نجی:
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
  غیرعلامتی آغاز_ثنائی_ہندسہ = 0;
  غیرعلامتی عدد_بٹ_2 = 0;
  غیرعلامتی سائز = 0;
  دوست جماعت llvm::Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد;
  دوست جماعت TrailingObjects;

عوامی:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> اگلا_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t آغاز_ثنائی_ہندسہ,
                                                 size_t عدد_بٹ_2) {
  خالی *حافظہ_2 = ::عامل نیا(سائز_مقصد_Alloc(عدد_بٹ_2));
  TrieSubtrie *S = ::نیا (حافظہ_2) TrieSubtrie(آغاز_ثنائی_ہندسہ, عدد_بٹ_2);
  واپس std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t آغاز_ثنائی_ہندسہ, size_t عدد_بٹ_2)
    : Trie_گرہ(سچ), آغاز_ثنائی_ہندسہ(آغاز_ثنائی_ہندسہ), عدد_بٹ_2(عدد_بٹ_2), سائز(1u << عدد_بٹ_2),
      اگلا_2(خالی_اشاریہ) {
  برائے (غیرعلامتی I = 0; I < سائز; ++I)
    نیا (&get(I)) خانہ(خالی_اشاریہ);

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
  std::unique_ptr<TrieSubtrie> S = create(آغاز_ثنائی_ہندسہ + عدد_بٹ_2, عدد_Subtrie_بٹ);

  أكد(NewI < سائز);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_گرہ *Existing_گرہ = &Content;
  أكد(I < سائز);
  اگر (get(I).compare_exchange_strong(Existing_گرہ, S.get()))
    واپس Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  واپس cast<TrieSubtrie>(Existing_گرہ);
}

جماعت Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::ImplType final
    : نجی TrailingObjects<Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::ImplType,
                              TrieSubtrie> {
عوامی:
  ساکن std::unique_ptr<ImplType> create(size_t آغاز_ثنائی_ہندسہ, size_t عدد_بٹ_2) {
    size_t سائز = حجم(ImplType) + TrieSubtrie::سائز_مقصد_Alloc(عدد_بٹ_2);
    خالی *حافظہ_2 = ::عامل نیا(سائز);
    ImplType *Impl = ::نیا (حافظہ_2) ImplType(آغاز_ثنائی_ہندسہ, عدد_بٹ_2);
    واپس std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->اگلا_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_سر = خالی_اشاریہ;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    جب (!get_جڑ()->اگلا_2.compare_exchange_weak(Current_سر, S.get()))
      S->اگلا_2.exchange(Current_سر);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    واپس S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_جڑ() { واپس getTrailingObjects(); }

  ساکن خالی *عامل نیا(size_t سائز) { واپس ::عامل نیا(سائز); }
  خالی عامل حذف(خالی *Ptr) { ::عامل حذف(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_محفوظ_Allocator<BumpPtrAllocator> ContentAlloc;

نجی:
  دوست جماعت TrailingObjects;

  ImplType(size_t آغاز_ثنائی_ہندسہ, size_t عدد_بٹ_2) {
    ::نیا (get_جڑ()) TrieSubtrie(آغاز_ثنائی_ہندسہ, عدد_بٹ_2);
  }
};

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::ImplType &
Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::getOrCreateImpl() {
  اگر (ImplType *Impl = ImplPtr.بھرو())
    واپس *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, عدد_جڑ_بٹ);
  ImplType *ExistingImpl = خالی_اشاریہ;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  اگر (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    واپس *Impl.release();

  // Already created, return the existing ImplType.
  واپس *ExistingImpl;
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد
Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::ڈھونڈو(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.خالی_خانے() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.بھرو();
  اگر (!Impl)
    واپس اشارہ_گر_بنیاد();

  TrieSubtrie *S = Impl->get_جڑ();
  Trie_Hash_اشاریہ_Generator اشاریہ_Gen{عدد_جڑ_بٹ, عدد_Subtrie_بٹ, Hash};
  size_t اشاریہ_2 = اشاریہ_Gen.اگلا();
  جب (اشاریہ_2 != اشاریہ_Gen.انتہا()) {
    // Try to set the content.
    Trie_گرہ *Existing = S->get(اشاریہ_2);
    اگر (!Existing)
      واپس اشارہ_گر_بنیاد(S, اشاریہ_2, *اشاریہ_Gen.آغاز_ثنائی_ہندسہ);

    // Check for an exact match.
    اگر (خودکار *ExistingContent = dyn_cast<TrieContent>(Existing))
      واپس ExistingContent->getHash() == Hash
                 ? اشارہ_گر_بنیاد(ExistingContent->get_قدر_اشارہ_گر())
                 : اشارہ_گر_بنیاد(S, اشاریہ_2, *اشاریہ_Gen.آغاز_ثنائی_ہندسہ);

    اشاریہ_2 = اشاریہ_Gen.اگلا();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::داخل_کرو(
    اشارہ_گر_بنیاد Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(خالی *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.خالی_خانے() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_جڑ();
  Trie_Hash_اشاریہ_Generator اشاریہ_Gen{عدد_جڑ_بٹ, عدد_Subtrie_بٹ, Hash};
  size_t اشاریہ_2;
  اگر (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    اشاریہ_2 = اشاریہ_Gen.hint(Hint.I, Hint.B);
  } ورنہ {
    اشاریہ_2 = اشاریہ_Gen.اگلا();
  }

  جب (اشاریہ_2 != اشاریہ_Gen.انتہا()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقی Generated = جھوٹ;
    Trie_گرہ &Existing = S->get(اشاریہ_2).بھرو_Or_Generate([&]() {
      Generated = سچ;

      // Construct the value itself at the tail.
      uint8_t *حافظہ_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_سائز, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(حافظہ_2 + Content_انحراف, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::نیا (حافظہ_2)
          TrieContent(Content_انحراف, Hash.سائز_2(), HashStorage - حافظہ_2);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      واپس Content;
    });
    // If we just generated it, return it!
    اگر (Generated)
      واپس اشارہ_گر_بنیاد(cast<TrieContent>(Existing).get_قدر_اشارہ_گر());

    اگر (خودکار *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      اشاریہ_2 = اشاریہ_Gen.اگلا();
      جاری;
    }

    // Return the existing content if it's an exact match!
    خودکار &ExistingContent = cast<TrieContent>(Existing);
    اگر (ExistingContent.getHash() == Hash)
      واپس اشارہ_گر_بنیاد(ExistingContent.get_قدر_اشارہ_گر());

    // Sink the existing content as long as the indexes match.
    size_t اگلا_اشاریہ = اشاریہ_Gen.اگلا();
    جب (اگلا_اشاریہ != اشاریہ_Gen.انتہا()) {
      size_t New_اشاریہ_For_Existing_Content =
          اشاریہ_Gen.get_Colliding_بٹ(ExistingContent.getHash());
      S = S->sink(اشاریہ_2, ExistingContent, اشاریہ_Gen.get_عدد_بٹ(),
                  New_اشاریہ_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    واپس Impl.save(std::move(S));
                  });
      اشاریہ_2 = اگلا_اشاریہ;

      // Found the difference.
      اگر (اگلا_اشاریہ != New_اشاریہ_For_Existing_Content)
        توڑو;

      اگلا_اشاریہ = اشاریہ_Gen.اگلا();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد(
    size_t Content_Alloc_سائز, size_t ContentAllocAlign, size_t Content_انحراف,
    std::optional<size_t> عدد_جڑ_بٹ, std::optional<size_t> عدد_Subtrie_بٹ)
    : Content_Alloc_سائز(Content_Alloc_سائز), ContentAllocAlign(ContentAllocAlign),
      Content_انحراف(Content_انحراف),
      عدد_جڑ_بٹ(عدد_جڑ_بٹ.قدر_or(Default_عدد_جڑ_بٹ)),
      عدد_Subtrie_بٹ(عدد_Subtrie_بٹ.قدر_or(Default_عدد_Subtrie_بٹ)),
      ImplPtr(خالی_اشاریہ) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!عدد_جڑ_بٹ || *عدد_جڑ_بٹ < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!عدد_Subtrie_بٹ || *عدد_Subtrie_بٹ < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد &&RHS)
    : Content_Alloc_سائز(RHS.Content_Alloc_سائز),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_انحراف(RHS.Content_انحراف), عدد_جڑ_بٹ(RHS.عدد_جڑ_بٹ),
      عدد_Subtrie_بٹ(RHS.عدد_Subtrie_بٹ) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(خالی_اشاریہ);
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::~Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد() {
  أكد(!ImplPtr.بھرو() && "Expected subclass to call destroyImpl()");
}

خالی Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::destroyImpl(
    function_ref<خالی(خالی *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(خالی_اشاریہ));
  اگر (!Impl)
    واپس;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  اگر (Destructor)
    برائے (TrieSubtrie *Trie = Impl->get_جڑ(); Trie; Trie = Trie->اگلا_2.بھرو())
      برائے (غیرعلامتی I = 0; I < Trie->سائز_2(); ++I)
        اگر (خودکار *Content = dyn_cast_or_null<TrieContent>(Trie->بھرو(I)))
          Destructor(Content->get_قدر_اشارہ_گر());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_جڑ()->اگلا_2;
  جب (Trie) {
    TrieSubtrie *اگلا_2 = Trie->اگلا_2.exchange(خالی_اشاریہ);
    حذف Trie;
    Trie = اگلا_2;
  }
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد
Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_جڑ() ثابت {
  ImplType *Impl = ImplPtr.بھرو();
  اگر (!Impl)
    واپس اشارہ_گر_بنیاد();
  واپس اشارہ_گر_بنیاد(Impl->get_جڑ());
}

غیرعلامتی Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_آغاز_ثنائی_ہندسہ(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    واپس 0;
  اگر (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P))
    واپس S->آغاز_ثنائی_ہندسہ;
  واپس 0;
}

غیرعلامتی Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_عدد_بٹ(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    واپس 0;
  اگر (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P))
    واپس S->عدد_بٹ_2;
  واپس 0;
}

غیرعلامتی Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_عدد_خانہ_مستعمل(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    واپس 0;
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  اگر (!S)
    واپس 0;
  غیرعلامتی عدد = 0;
  برائے (غیرعلامتی I = 0, E = S->سائز_2(); I < E; ++I)
    اگر (S->بھرو(I))
      ++عدد;
  واپس عدد;
}

std::string Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_Trie_سابقہ_As_String(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    واپس "";

  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  اگر (!S || !S->IsSubtrie)
    واپس "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *گرہ = خالی_اشاریہ;
  جب (Current) {
    TrieSubtrie *اگلا_2 = خالی_اشاریہ;
    // Find first used slot in the trie.
    برائے (غیرعلامتی I = 0, E = Current->سائز_2(); I < E; ++I) {
      خودکار *S = Current->بھرو(I);
      اگر (!S)
        جاری;

      اگر (خودکار *Content = dyn_cast<TrieContent>(S))
        گرہ = Content;
      ورنہ اگر (خودکار *Sub = dyn_cast<TrieSubtrie>(S))
        اگلا_2 = Sub;
      توڑو;
    }

    // Found the node.
    اگر (گرہ)
      توڑو;

    // Continue to the next level if the node is not found.
    Current = اگلا_2;
  }

  أكد(گرہ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  غیرعلامتی آغاز_بھرا_بائٹیں = (S->آغاز_ثنائی_ہندسہ + 1) / 8 - 1;
  SS << مقصد_Hex(مقصد_String_Ref(گرہ->getHash()).take_front(آغاز_بھرا_بائٹیں),
              /*LowerCase=*/سچ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string بٹ_2;
  برائے (غیرعلامتی I = آغاز_بھرا_بائٹیں * 8, E = S->آغاز_ثنائی_ہندسہ; I < E; ++I) {
    غیرعلامتی اشاریہ_2 = I / 8;
    غیرعلامتی انحراف = 7 - I % 8;
    بٹ_2.ڈالو_عقب('0' + ((گرہ->getHash()[اشاریہ_2] >> انحراف) & 1));
  }

  اگر (!بٹ_2.خالی_خانے())
    SS << "[" << بٹ_2 << "]";

  واپس SS.str();
}

غیرعلامتی Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_عدد_Tries() ثابت {
  ImplType *Impl = ImplPtr.بھرو();
  اگر (!Impl)
    واپس 0;
  غیرعلامتی عدد = 0;
  برائے (TrieSubtrie *Trie = Impl->get_جڑ(); Trie; Trie = Trie->اگلا_2.بھرو())
    ++عدد;
  واپس عدد;
}

Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد
Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::get_اگلا_Trie(
    Thread_محفوظ_Trie_Raw_Hash_Map_بنیاد::اشارہ_گر_بنیاد P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  اگر (!P.P)
    واپس اشارہ_گر_بنیاد();
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گرہ *)P.P);
  اگر (!S)
    واپس اشارہ_گر_بنیاد();
  اگر (خودکار *E = S->اگلا_2.بھرو())
    واپس اشارہ_گر_بنیاد(E);
  واپس اشارہ_گر_بنیاد();
}
