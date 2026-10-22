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

استفاده نطاق llvm;

نطاق {
بنية Trie_گره {
  ثابت منطقي IsSubtrie = خطأ;

  Trie_گره(منطقي IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ساكن فراغ *عملگر جديد(size_t اندازه) { أرجع ::عملگر جديد(اندازه); }
  فراغ عملگر حذف(فراغ *Ptr) { ::عملگر حذف(Ptr); }
};

بنية TrieContent final : عام Trie_گره {
  ثابت uint8_t Content_جابه_جایی;
  ثابت uint8_t Hash_اندازه;
  ثابت uint8_t Hash_جابه_جایی;

  فراغ *get_مقدار_اشاره_گر() ثابت {
    خودکار *Content = reinterpret_cast<ثابت uint8_t *>(هذا) + Content_جابه_جایی;
    أرجع const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ثابت {
    خودکار *Begin = reinterpret_cast<ثابت uint8_t *>(هذا) + Hash_جابه_جایی;
    أرجع ArrayRef(Begin, Begin + Hash_اندازه);
  }

  TrieContent(size_t Content_جابه_جایی, size_t Hash_اندازه, size_t Hash_جابه_جایی)
      : Trie_گره(/*IsSubtrie=*/خطأ), Content_جابه_جایی(Content_جابه_جایی),
        Hash_اندازه(Hash_اندازه), Hash_جابه_جایی(Hash_جابه_جایی) {}

  ساكن منطقي classof(ثابت Trie_گره *TN) { أرجع !TN->IsSubtrie; }
};

static_assert(حجم(TrieContent) ==
                  Thread_امن_Trie_Raw_Hash_Map_پایه::Trie_Content_پایه_اندازه,
              "Check header assumption!");

صنف TrieSubtrie final
    : عام Trie_گره,
      خاص TrailingObjects<TrieSubtrie, Lazy_Atomic_اشاره_گر<Trie_گره>> {
عام:
  استفاده جایگاه = Lazy_Atomic_اشاره_گر<Trie_گره>;

  جایگاه &get(size_t I) { أرجع getTrailingObjects()[I]; }
  Trie_گره *بارگیری(size_t I) { أرجع get(I).بارگیری(); }

  بیعلامت اندازه_2() ثابت { أرجع اندازه; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t رقم_Subtrie_بیتها, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ساكن std::unique_ptr<TrieSubtrie> create(size_t شروع_بیت, size_t رقم_بیتها_2);

  صریح TrieSubtrie(size_t شروع_بیت, size_t رقم_بیتها_2);

  ساكن منطقي classof(ثابت Trie_گره *TN) { أرجع TN->IsSubtrie; }

  ساكن constexpr size_t اندازه_مقصد_Alloc(بیعلامت رقم_بیتها_2) {
    أكد(رقم_بیتها_2 < 20 && "Tries should have fewer than ~1M slots");
    بیعلامت شمار_2 = 1u << رقم_بیتها_2;
    أرجع مجموع_اندازه_مقصد_Alloc<Lazy_Atomic_اشاره_گر<Trie_گره>>(شمار_2);
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
  بیعلامت شروع_بیت = 0;
  بیعلامت رقم_بیتها_2 = 0;
  بیعلامت اندازه = 0;
  دوست صنف llvm::Thread_امن_Trie_Raw_Hash_Map_پایه;
  دوست صنف TrailingObjects;

عام:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> بعدی_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t شروع_بیت,
                                                 size_t رقم_بیتها_2) {
  فراغ *حافظه_2 = ::عملگر جديد(اندازه_مقصد_Alloc(رقم_بیتها_2));
  TrieSubtrie *S = ::جديد (حافظه_2) TrieSubtrie(شروع_بیت, رقم_بیتها_2);
  أرجع std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t شروع_بیت, size_t رقم_بیتها_2)
    : Trie_گره(درست), شروع_بیت(شروع_بیت), رقم_بیتها_2(رقم_بیتها_2), اندازه(1u << رقم_بیتها_2),
      بعدی_2(اشاره_تهی) {
  لكل (بیعلامت I = 0; I < اندازه; ++I)
    جديد (&get(I)) جایگاه(اشاره_تهی);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_اشاره_گر<Trie_گره>>::مقدار_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t رقم_Subtrie_بیتها, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  أكد(رقم_Subtrie_بیتها > 0);
  std::unique_ptr<TrieSubtrie> S = create(شروع_بیت + رقم_بیتها_2, رقم_Subtrie_بیتها);

  أكد(NewI < اندازه);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_گره *Existing_گره = &Content;
  أكد(I < اندازه);
  إذا (get(I).compare_exchange_strong(Existing_گره, S.get()))
    أرجع Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  أرجع cast<TrieSubtrie>(Existing_گره);
}

صنف Thread_امن_Trie_Raw_Hash_Map_پایه::ImplType final
    : خاص TrailingObjects<Thread_امن_Trie_Raw_Hash_Map_پایه::ImplType,
                              TrieSubtrie> {
عام:
  ساكن std::unique_ptr<ImplType> create(size_t شروع_بیت, size_t رقم_بیتها_2) {
    size_t اندازه = حجم(ImplType) + TrieSubtrie::اندازه_مقصد_Alloc(رقم_بیتها_2);
    فراغ *حافظه_2 = ::عملگر جديد(اندازه);
    ImplType *Impl = ::جديد (حافظه_2) ImplType(شروع_بیت, رقم_بیتها_2);
    أرجع std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    أكد(!S->بعدی_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_سر = اشاره_تهی;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    طالما (!get_ریشه()->بعدی_2.compare_exchange_weak(Current_سر, S.get()))
      S->بعدی_2.exchange(Current_سر);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    أرجع S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ریشه() { أرجع getTrailingObjects(); }

  ساكن فراغ *عملگر جديد(size_t اندازه) { أرجع ::عملگر جديد(اندازه); }
  فراغ عملگر حذف(فراغ *Ptr) { ::عملگر حذف(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_امن_Allocator<BumpPtrAllocator> ContentAlloc;

خاص:
  دوست صنف TrailingObjects;

  ImplType(size_t شروع_بیت, size_t رقم_بیتها_2) {
    ::جديد (get_ریشه()) TrieSubtrie(شروع_بیت, رقم_بیتها_2);
  }
};

Thread_امن_Trie_Raw_Hash_Map_پایه::ImplType &
Thread_امن_Trie_Raw_Hash_Map_پایه::getOrCreateImpl() {
  إذا (ImplType *Impl = ImplPtr.بارگیری())
    أرجع *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, رقم_ریشه_بیتها);
  ImplType *ExistingImpl = اشاره_تهی;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  إذا (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    أرجع *Impl.release();

  // Already created, return the existing ImplType.
  أرجع *ExistingImpl;
}

Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه
Thread_امن_Trie_Raw_Hash_Map_پایه::پیدا_کن(ArrayRef<uint8_t> Hash) ثابت {
  أكد(!Hash.خالی() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.بارگیری();
  إذا (!Impl)
    أرجع اشاره_گر_پایه();

  TrieSubtrie *S = Impl->get_ریشه();
  Trie_Hash_شاخص_Generator شاخص_Gen{رقم_ریشه_بیتها, رقم_Subtrie_بیتها, Hash};
  size_t شاخص_2 = شاخص_Gen.بعدی();
  طالما (شاخص_2 != شاخص_Gen.انتها()) {
    // Try to set the content.
    Trie_گره *Existing = S->get(شاخص_2);
    إذا (!Existing)
      أرجع اشاره_گر_پایه(S, شاخص_2, *شاخص_Gen.شروع_بیت);

    // Check for an exact match.
    إذا (خودکار *ExistingContent = dyn_cast<TrieContent>(Existing))
      أرجع ExistingContent->getHash() == Hash
                 ? اشاره_گر_پایه(ExistingContent->get_مقدار_اشاره_گر())
                 : اشاره_گر_پایه(S, شاخص_2, *شاخص_Gen.شروع_بیت);

    شاخص_2 = شاخص_Gen.بعدی();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه Thread_امن_Trie_Raw_Hash_Map_پایه::درج(
    اشاره_گر_پایه Hint, ArrayRef<uint8_t> Hash,
    function_ref<ثابت uint8_t *(فراغ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  أكد(!Hash.خالی() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ریشه();
  Trie_Hash_شاخص_Generator شاخص_Gen{رقم_ریشه_بیتها, رقم_Subtrie_بیتها, Hash};
  size_t شاخص_2;
  إذا (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    شاخص_2 = شاخص_Gen.hint(Hint.I, Hint.B);
  } وإلا {
    شاخص_2 = شاخص_Gen.بعدی();
  }

  طالما (شاخص_2 != شاخص_Gen.انتها()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    منطقي Generated = خطأ;
    Trie_گره &Existing = S->get(شاخص_2).بارگیری_Or_Generate([&]() {
      Generated = درست;

      // Construct the value itself at the tail.
      uint8_t *حافظه_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_اندازه, ContentAllocAlign));
      ثابت uint8_t *HashStorage = Constructor(حافظه_2 + Content_جابه_جایی, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::جديد (حافظه_2)
          TrieContent(Content_جابه_جایی, Hash.اندازه_2(), HashStorage - حافظه_2);
      أكد(Hash == Content->getHash() && "Hash not properly initialized");
      أرجع Content;
    });
    // If we just generated it, return it!
    إذا (Generated)
      أرجع اشاره_گر_پایه(cast<TrieContent>(Existing).get_مقدار_اشاره_گر());

    إذا (خودکار *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      شاخص_2 = شاخص_Gen.بعدی();
      استمر;
    }

    // Return the existing content if it's an exact match!
    خودکار &ExistingContent = cast<TrieContent>(Existing);
    إذا (ExistingContent.getHash() == Hash)
      أرجع اشاره_گر_پایه(ExistingContent.get_مقدار_اشاره_گر());

    // Sink the existing content as long as the indexes match.
    size_t بعدی_شاخص = شاخص_Gen.بعدی();
    طالما (بعدی_شاخص != شاخص_Gen.انتها()) {
      size_t New_شاخص_For_Existing_Content =
          شاخص_Gen.get_Colliding_بیتها(ExistingContent.getHash());
      S = S->sink(شاخص_2, ExistingContent, شاخص_Gen.get_رقم_بیتها(),
                  New_شاخص_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    أرجع Impl.save(std::move(S));
                  });
      شاخص_2 = بعدی_شاخص;

      // Found the difference.
      إذا (بعدی_شاخص != New_شاخص_For_Existing_Content)
        اكسر;

      بعدی_شاخص = شاخص_Gen.بعدی();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_امن_Trie_Raw_Hash_Map_پایه::Thread_امن_Trie_Raw_Hash_Map_پایه(
    size_t Content_Alloc_اندازه, size_t ContentAllocAlign, size_t Content_جابه_جایی,
    std::optional<size_t> رقم_ریشه_بیتها, std::optional<size_t> رقم_Subtrie_بیتها)
    : Content_Alloc_اندازه(Content_Alloc_اندازه), ContentAllocAlign(ContentAllocAlign),
      Content_جابه_جایی(Content_جابه_جایی),
      رقم_ریشه_بیتها(رقم_ریشه_بیتها.مقدار_or(Default_رقم_ریشه_بیتها)),
      رقم_Subtrie_بیتها(رقم_Subtrie_بیتها.مقدار_or(Default_رقم_Subtrie_بیتها)),
      ImplPtr(اشاره_تهی) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  أكد((!رقم_ریشه_بیتها || *رقم_ریشه_بیتها < 20) &&
         "Root should have fewer than ~1M slots");
  أكد((!رقم_Subtrie_بیتها || *رقم_Subtrie_بیتها < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_امن_Trie_Raw_Hash_Map_پایه::Thread_امن_Trie_Raw_Hash_Map_پایه(
    Thread_امن_Trie_Raw_Hash_Map_پایه &&RHS)
    : Content_Alloc_اندازه(RHS.Content_Alloc_اندازه),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_جابه_جایی(RHS.Content_جابه_جایی), رقم_ریشه_بیتها(RHS.رقم_ریشه_بیتها),
      رقم_Subtrie_بیتها(RHS.رقم_Subtrie_بیتها) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(اشاره_تهی);
}

Thread_امن_Trie_Raw_Hash_Map_پایه::~Thread_امن_Trie_Raw_Hash_Map_پایه() {
  أكد(!ImplPtr.بارگیری() && "Expected subclass to call destroyImpl()");
}

فراغ Thread_امن_Trie_Raw_Hash_Map_پایه::destroyImpl(
    function_ref<فراغ(فراغ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(اشاره_تهی));
  إذا (!Impl)
    أرجع;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  إذا (Destructor)
    لكل (TrieSubtrie *Trie = Impl->get_ریشه(); Trie; Trie = Trie->بعدی_2.بارگیری())
      لكل (بیعلامت I = 0; I < Trie->اندازه_2(); ++I)
        إذا (خودکار *Content = dyn_cast_or_null<TrieContent>(Trie->بارگیری(I)))
          Destructor(Content->get_مقدار_اشاره_گر());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ریشه()->بعدی_2;
  طالما (Trie) {
    TrieSubtrie *بعدی_2 = Trie->بعدی_2.exchange(اشاره_تهی);
    حذف Trie;
    Trie = بعدی_2;
  }
}

Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه
Thread_امن_Trie_Raw_Hash_Map_پایه::get_ریشه() ثابت {
  ImplType *Impl = ImplPtr.بارگیری();
  إذا (!Impl)
    أرجع اشاره_گر_پایه();
  أرجع اشاره_گر_پایه(Impl->get_ریشه());
}

بیعلامت Thread_امن_Trie_Raw_Hash_Map_پایه::get_شروع_بیت(
    Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P))
    أرجع S->شروع_بیت;
  أرجع 0;
}

بیعلامت Thread_امن_Trie_Raw_Hash_Map_پایه::get_رقم_بیتها(
    Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  إذا (خودکار *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P))
    أرجع S->رقم_بیتها_2;
  أرجع 0;
}

بیعلامت Thread_امن_Trie_Raw_Hash_Map_پایه::get_رقم_جایگاه_استفاده_شده(
    Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع 0;
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  إذا (!S)
    أرجع 0;
  بیعلامت رقم = 0;
  لكل (بیعلامت I = 0, E = S->اندازه_2(); I < E; ++I)
    إذا (S->بارگیری(I))
      ++رقم;
  أرجع رقم;
}

std::string Thread_امن_Trie_Raw_Hash_Map_پایه::get_Trie_پیشوند_As_String(
    Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع "";

  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  إذا (!S || !S->IsSubtrie)
    أرجع "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *گره = اشاره_تهی;
  طالما (Current) {
    TrieSubtrie *بعدی_2 = اشاره_تهی;
    // Find first used slot in the trie.
    لكل (بیعلامت I = 0, E = Current->اندازه_2(); I < E; ++I) {
      خودکار *S = Current->بارگیری(I);
      إذا (!S)
        استمر;

      إذا (خودکار *Content = dyn_cast<TrieContent>(S))
        گره = Content;
      وإلا إذا (خودکار *Sub = dyn_cast<TrieSubtrie>(S))
        بعدی_2 = Sub;
      اكسر;
    }

    // Found the node.
    إذا (گره)
      اكسر;

    // Continue to the next level if the node is not found.
    Current = بعدی_2;
  }

  أكد(گره && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  بیعلامت شروع_پر_بایتها = (S->شروع_بیت + 1) / 8 - 1;
  SS << مقصد_Hex(مقصد_String_Ref(گره->getHash()).take_front(شروع_پر_بایتها),
              /*LowerCase=*/درست);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string بیتها_2;
  لكل (بیعلامت I = شروع_پر_بایتها * 8, E = S->شروع_بیت; I < E; ++I) {
    بیعلامت شاخص_2 = I / 8;
    بیعلامت جابه_جایی = 7 - I % 8;
    بیتها_2.افزودن_پشت('0' + ((گره->getHash()[شاخص_2] >> جابه_جایی) & 1));
  }

  إذا (!بیتها_2.خالی())
    SS << "[" << بیتها_2 << "]";

  أرجع SS.str();
}

بیعلامت Thread_امن_Trie_Raw_Hash_Map_پایه::get_رقم_Tries() ثابت {
  ImplType *Impl = ImplPtr.بارگیری();
  إذا (!Impl)
    أرجع 0;
  بیعلامت رقم = 0;
  لكل (TrieSubtrie *Trie = Impl->get_ریشه(); Trie; Trie = Trie->بعدی_2.بارگیری())
    ++رقم;
  أرجع رقم;
}

Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه
Thread_امن_Trie_Raw_Hash_Map_پایه::get_بعدی_Trie(
    Thread_امن_Trie_Raw_Hash_Map_پایه::اشاره_گر_پایه P) ثابت {
  أكد(!P.isHint() && "Not a valid trie");
  إذا (!P.P)
    أرجع اشاره_گر_پایه();
  خودکار *S = dyn_cast<TrieSubtrie>((Trie_گره *)P.P);
  إذا (!S)
    أرجع اشاره_گر_پایه();
  إذا (خودکار *E = S->بعدی_2.بارگیری())
    أرجع اشاره_گر_پایه(E);
  أرجع اشاره_گر_پایه();
}
