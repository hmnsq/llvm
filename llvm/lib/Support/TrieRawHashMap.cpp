//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/TrieRawHashMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/LazyAtomicPointer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/TrieHashIndexGenerator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Allocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ThreadSafeAllocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TrailingObjects.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ Trie_ਗੰਢ {
  ਅਚਲ ਤਾਰਕਿਕ IsSubtrie = ਝੂਠ;

  Trie_ਗੰਢ(ਤਾਰਕਿਕ IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ਸਥਿਰ ਖਾਲੀ *ਸੰਚਾਲਕ ਨਵਾਂ(size_t ਮਾਪ) { ਵਾਪਸ ::ਸੰਚਾਲਕ ਨਵਾਂ(ਮਾਪ); }
  ਖਾਲੀ ਸੰਚਾਲਕ ਮਿਟਾਓ(ਖਾਲੀ *Ptr) { ::ਸੰਚਾਲਕ ਮਿਟਾਓ(Ptr); }
};

ਬਣਤਰ TrieContent final : ਜਨਤਕ Trie_ਗੰਢ {
  ਅਚਲ uint8_t Content_ਵਿੱਥ;
  ਅਚਲ uint8_t Hash_ਮਾਪ;
  ਅਚਲ uint8_t Hash_ਵਿੱਥ;

  ਖਾਲੀ *get_ਮੁੱਲ_ਸੂਚਕ() ਅਚਲ {
    ਸਵੈਚਾਲਿਤ *Content = reinterpret_cast<ਅਚਲ uint8_t *>(ਇਹ) + Content_ਵਿੱਥ;
    ਵਾਪਸ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ਅਚਲ {
    ਸਵੈਚਾਲਿਤ *Begin = reinterpret_cast<ਅਚਲ uint8_t *>(ਇਹ) + Hash_ਵਿੱਥ;
    ਵਾਪਸ ArrayRef(Begin, Begin + Hash_ਮਾਪ);
  }

  TrieContent(size_t Content_ਵਿੱਥ, size_t Hash_ਮਾਪ, size_t Hash_ਵਿੱਥ)
      : Trie_ਗੰਢ(/*IsSubtrie=*/ਝੂਠ), Content_ਵਿੱਥ(Content_ਵਿੱਥ),
        Hash_ਮਾਪ(Hash_ਮਾਪ), Hash_ਵਿੱਥ(Hash_ਵਿੱਥ) {}

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ Trie_ਗੰਢ *TN) { ਵਾਪਸ !TN->IsSubtrie; }
};

static_assert(ਆਕਾਰ(TrieContent) ==
                  Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::Trie_Content_ਅਧਾਰ_ਮਾਪ,
              "Check header assumption!");

ਵਰਗ TrieSubtrie final
    : ਜਨਤਕ Trie_ਗੰਢ,
      ਨਿਜੀ TrailingObjects<TrieSubtrie, Lazy_Atomic_ਸੂਚਕ<Trie_ਗੰਢ>> {
ਜਨਤਕ:
  ਵਰਤੋਂ ਥਾਂ = Lazy_Atomic_ਸੂਚਕ<Trie_ਗੰਢ>;

  ਥਾਂ &get(size_t I) { ਵਾਪਸ getTrailingObjects()[I]; }
  Trie_ਗੰਢ *ਲੱਦੋ(size_t I) { ਵਾਪਸ get(I).ਲੱਦੋ(); }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_2() ਅਚਲ { ਵਾਪਸ ਮਾਪ; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t ਅੰਕ_Subtrie_ਬਿੱਟ, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ਸਥਿਰ std::unique_ptr<TrieSubtrie> create(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, size_t ਅੰਕ_ਬਿੱਟ_2);

  ਸਪਸ਼ਟ TrieSubtrie(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, size_t ਅੰਕ_ਬਿੱਟ_2);

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ Trie_ਗੰਢ *TN) { ਵਾਪਸ TN->IsSubtrie; }

  ਸਥਿਰ constexpr size_t ਮਾਪ_ਮੰਜ਼ਿਲ_Alloc(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਬਿੱਟ_2 < 20 && "Tries should have fewer than ~1M slots");
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 1u << ਅੰਕ_ਬਿੱਟ_2;
    ਵਾਪਸ ਕੁੱਲ_ਮਾਪ_ਮੰਜ਼ਿਲ_Alloc<Lazy_Atomic_ਸੂਚਕ<Trie_ਗੰਢ>>(ਗਿਣਤੀ_3);
  }

ਨਿਜੀ:
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
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = 0;
  ਮਿੱਤਰ ਵਰਗ llvm::Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ;
  ਮਿੱਤਰ ਵਰਗ TrailingObjects;

ਜਨਤਕ:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> ਅਗਲਾ_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ,
                                                 size_t ਅੰਕ_ਬਿੱਟ_2) {
  ਖਾਲੀ *ਸਮ੍ਰਿਤੀ_2 = ::ਸੰਚਾਲਕ ਨਵਾਂ(ਮਾਪ_ਮੰਜ਼ਿਲ_Alloc(ਅੰਕ_ਬਿੱਟ_2));
  TrieSubtrie *S = ::ਨਵਾਂ (ਸਮ੍ਰਿਤੀ_2) TrieSubtrie(ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, ਅੰਕ_ਬਿੱਟ_2);
  ਵਾਪਸ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, size_t ਅੰਕ_ਬਿੱਟ_2)
    : Trie_ਗੰਢ(ਸੱਚ), ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ), ਅੰਕ_ਬਿੱਟ_2(ਅੰਕ_ਬਿੱਟ_2), ਮਾਪ(1u << ਅੰਕ_ਬਿੱਟ_2),
      ਅਗਲਾ_2(ਖਾਲੀ_ਸੂਚਕ) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਮਾਪ; ++I)
    ਨਵਾਂ (&get(I)) ਥਾਂ(ਖਾਲੀ_ਸੂਚਕ);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_ਸੂਚਕ<Trie_ਗੰਢ>>::ਮੁੱਲ_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t ਅੰਕ_Subtrie_ਬਿੱਟ, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_Subtrie_ਬਿੱਟ > 0);
  std::unique_ptr<TrieSubtrie> S = create(ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ + ਅੰਕ_ਬਿੱਟ_2, ਅੰਕ_Subtrie_ਬਿੱਟ);

  ਪ੍ਰਤਿਜ੍ਞਾ(NewI < ਮਾਪ);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ਗੰਢ *Existing_ਗੰਢ = &Content;
  ਪ੍ਰਤਿਜ੍ਞਾ(I < ਮਾਪ);
  ਜੇ (get(I).compare_exchange_strong(Existing_ਗੰਢ, S.get()))
    ਵਾਪਸ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ਵਾਪਸ cast<TrieSubtrie>(Existing_ਗੰਢ);
}

ਵਰਗ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ImplType final
    : ਨਿਜੀ TrailingObjects<Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ImplType,
                              TrieSubtrie> {
ਜਨਤਕ:
  ਸਥਿਰ std::unique_ptr<ImplType> create(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, size_t ਅੰਕ_ਬਿੱਟ_2) {
    size_t ਮਾਪ = ਆਕਾਰ(ImplType) + TrieSubtrie::ਮਾਪ_ਮੰਜ਼ਿਲ_Alloc(ਅੰਕ_ਬਿੱਟ_2);
    ਖਾਲੀ *ਸਮ੍ਰਿਤੀ_2 = ::ਸੰਚਾਲਕ ਨਵਾਂ(ਮਾਪ);
    ImplType *Impl = ::ਨਵਾਂ (ਸਮ੍ਰਿਤੀ_2) ImplType(ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, ਅੰਕ_ਬਿੱਟ_2);
    ਵਾਪਸ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!S->ਅਗਲਾ_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_ਸਿਰ = ਖਾਲੀ_ਸੂਚਕ;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ਜਦੋਂ (!get_ਜੜ੍ਹ()->ਅਗਲਾ_2.compare_exchange_weak(Current_ਸਿਰ, S.get()))
      S->ਅਗਲਾ_2.exchange(Current_ਸਿਰ);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ਵਾਪਸ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ਜੜ੍ਹ() { ਵਾਪਸ getTrailingObjects(); }

  ਸਥਿਰ ਖਾਲੀ *ਸੰਚਾਲਕ ਨਵਾਂ(size_t ਮਾਪ) { ਵਾਪਸ ::ਸੰਚਾਲਕ ਨਵਾਂ(ਮਾਪ); }
  ਖਾਲੀ ਸੰਚਾਲਕ ਮਿਟਾਓ(ਖਾਲੀ *Ptr) { ::ਸੰਚਾਲਕ ਮਿਟਾਓ(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_ਸੁਰੱਖਿਅਤ_Allocator<BumpPtrAllocator> ContentAlloc;

ਨਿਜੀ:
  ਮਿੱਤਰ ਵਰਗ TrailingObjects;

  ImplType(size_t ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, size_t ਅੰਕ_ਬਿੱਟ_2) {
    ::ਨਵਾਂ (get_ਜੜ੍ਹ()) TrieSubtrie(ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ, ਅੰਕ_ਬਿੱਟ_2);
  }
};

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ImplType &
Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::getOrCreateImpl() {
  ਜੇ (ImplType *Impl = ImplPtr.ਲੱਦੋ())
    ਵਾਪਸ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ);
  ImplType *ExistingImpl = ਖਾਲੀ_ਸੂਚਕ;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ਜੇ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ਵਾਪਸ *Impl.release();

  // Already created, return the existing ImplType.
  ਵਾਪਸ *ExistingImpl;
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ
Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਲੱਭੋ(ArrayRef<uint8_t> Hash) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Hash.ਖਾਲੀ_ਥਾਵਾਂ() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.ਲੱਦੋ();
  ਜੇ (!Impl)
    ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ();

  TrieSubtrie *S = Impl->get_ਜੜ੍ਹ();
  Trie_Hash_ਸੂਚਕਾਂਕ_Generator ਸੂਚਕਾਂਕ_Gen{ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ, ਅੰਕ_Subtrie_ਬਿੱਟ, Hash};
  size_t ਸੂਚਕਾਂਕ_2 = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
  ਜਦੋਂ (ਸੂਚਕਾਂਕ_2 != ਸੂਚਕਾਂਕ_Gen.ਸਿਰਾ()) {
    // Try to set the content.
    Trie_ਗੰਢ *Existing = S->get(ਸੂਚਕਾਂਕ_2);
    ਜੇ (!Existing)
      ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ(S, ਸੂਚਕਾਂਕ_2, *ਸੂਚਕਾਂਕ_Gen.ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ);

    // Check for an exact match.
    ਜੇ (ਸਵੈਚਾਲਿਤ *ExistingContent = dyn_cast<TrieContent>(Existing))
      ਵਾਪਸ ExistingContent->getHash() == Hash
                 ? ਸੂਚਕ_ਅਧਾਰ(ExistingContent->get_ਮੁੱਲ_ਸੂਚਕ())
                 : ਸੂਚਕ_ਅਧਾਰ(S, ਸੂਚਕਾਂਕ_2, *ਸੂਚਕਾਂਕ_Gen.ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ);

    ਸੂਚਕਾਂਕ_2 = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਪਾਓ(
    ਸੂਚਕ_ਅਧਾਰ Hint, ArrayRef<uint8_t> Hash,
    function_ref<ਅਚਲ uint8_t *(ਖਾਲੀ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Hash.ਖਾਲੀ_ਥਾਵਾਂ() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ਜੜ੍ਹ();
  Trie_Hash_ਸੂਚਕਾਂਕ_Generator ਸੂਚਕਾਂਕ_Gen{ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ, ਅੰਕ_Subtrie_ਬਿੱਟ, Hash};
  size_t ਸੂਚਕਾਂਕ_2;
  ਜੇ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    ਸੂਚਕਾਂਕ_2 = ਸੂਚਕਾਂਕ_Gen.hint(Hint.I, Hint.B);
  } ਨਹੀਂਤਾਂ {
    ਸੂਚਕਾਂਕ_2 = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
  }

  ਜਦੋਂ (ਸੂਚਕਾਂਕ_2 != ਸੂਚਕਾਂਕ_Gen.ਸਿਰਾ()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    ਤਾਰਕਿਕ Generated = ਝੂਠ;
    Trie_ਗੰਢ &Existing = S->get(ਸੂਚਕਾਂਕ_2).ਲੱਦੋ_Or_Generate([&]() {
      Generated = ਸੱਚ;

      // Construct the value itself at the tail.
      uint8_t *ਸਮ੍ਰਿਤੀ_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_ਮਾਪ, ContentAllocAlign));
      ਅਚਲ uint8_t *HashStorage = Constructor(ਸਮ੍ਰਿਤੀ_2 + Content_ਵਿੱਥ, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::ਨਵਾਂ (ਸਮ੍ਰਿਤੀ_2)
          TrieContent(Content_ਵਿੱਥ, Hash.ਮਾਪ_2(), HashStorage - ਸਮ੍ਰਿਤੀ_2);
      ਪ੍ਰਤਿਜ੍ਞਾ(Hash == Content->getHash() && "Hash not properly initialized");
      ਵਾਪਸ Content;
    });
    // If we just generated it, return it!
    ਜੇ (Generated)
      ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ(cast<TrieContent>(Existing).get_ਮੁੱਲ_ਸੂਚਕ());

    ਜੇ (ਸਵੈਚਾਲਿਤ *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      ਸੂਚਕਾਂਕ_2 = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
      ਜਾਰੀ;
    }

    // Return the existing content if it's an exact match!
    ਸਵੈਚਾਲਿਤ &ExistingContent = cast<TrieContent>(Existing);
    ਜੇ (ExistingContent.getHash() == Hash)
      ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ(ExistingContent.get_ਮੁੱਲ_ਸੂਚਕ());

    // Sink the existing content as long as the indexes match.
    size_t ਅਗਲਾ_ਸੂਚਕਾਂਕ = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
    ਜਦੋਂ (ਅਗਲਾ_ਸੂਚਕਾਂਕ != ਸੂਚਕਾਂਕ_Gen.ਸਿਰਾ()) {
      size_t New_ਸੂਚਕਾਂਕ_For_Existing_Content =
          ਸੂਚਕਾਂਕ_Gen.get_Colliding_ਬਿੱਟ(ExistingContent.getHash());
      S = S->sink(ਸੂਚਕਾਂਕ_2, ExistingContent, ਸੂਚਕਾਂਕ_Gen.get_ਅੰਕ_ਬਿੱਟ(),
                  New_ਸੂਚਕਾਂਕ_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ਵਾਪਸ Impl.save(std::move(S));
                  });
      ਸੂਚਕਾਂਕ_2 = ਅਗਲਾ_ਸੂਚਕਾਂਕ;

      // Found the difference.
      ਜੇ (ਅਗਲਾ_ਸੂਚਕਾਂਕ != New_ਸੂਚਕਾਂਕ_For_Existing_Content)
        ਰੋਕੋ;

      ਅਗਲਾ_ਸੂਚਕਾਂਕ = ਸੂਚਕਾਂਕ_Gen.ਅਗਲਾ();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ(
    size_t Content_Alloc_ਮਾਪ, size_t ContentAllocAlign, size_t Content_ਵਿੱਥ,
    std::optional<size_t> ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ, std::optional<size_t> ਅੰਕ_Subtrie_ਬਿੱਟ)
    : Content_Alloc_ਮਾਪ(Content_Alloc_ਮਾਪ), ContentAllocAlign(ContentAllocAlign),
      Content_ਵਿੱਥ(Content_ਵਿੱਥ),
      ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ(ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ.ਮੁੱਲ_or(Default_ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ)),
      ਅੰਕ_Subtrie_ਬਿੱਟ(ਅੰਕ_Subtrie_ਬਿੱਟ.ਮੁੱਲ_or(Default_ਅੰਕ_Subtrie_ਬਿੱਟ)),
      ImplPtr(ਖਾਲੀ_ਸੂਚਕ) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ਪ੍ਰਤਿਜ੍ਞਾ((!ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ || *ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ < 20) &&
         "Root should have fewer than ~1M slots");
  ਪ੍ਰਤਿਜ੍ਞਾ((!ਅੰਕ_Subtrie_ਬਿੱਟ || *ਅੰਕ_Subtrie_ਬਿੱਟ < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ &&RHS)
    : Content_Alloc_ਮਾਪ(RHS.Content_Alloc_ਮਾਪ),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_ਵਿੱਥ(RHS.Content_ਵਿੱਥ), ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ(RHS.ਅੰਕ_ਜੜ੍ਹ_ਬਿੱਟ),
      ਅੰਕ_Subtrie_ਬਿੱਟ(RHS.ਅੰਕ_Subtrie_ਬਿੱਟ) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(ਖਾਲੀ_ਸੂਚਕ);
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::~Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ() {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ImplPtr.ਲੱਦੋ() && "Expected subclass to call destroyImpl()");
}

ਖਾਲੀ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::destroyImpl(
    function_ref<ਖਾਲੀ(ਖਾਲੀ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(ਖਾਲੀ_ਸੂਚਕ));
  ਜੇ (!Impl)
    ਵਾਪਸ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ਜੇ (Destructor)
    ਲਈ (TrieSubtrie *Trie = Impl->get_ਜੜ੍ਹ(); Trie; Trie = Trie->ਅਗਲਾ_2.ਲੱਦੋ())
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Trie->ਮਾਪ_2(); ++I)
        ਜੇ (ਸਵੈਚਾਲਿਤ *Content = dyn_cast_or_null<TrieContent>(Trie->ਲੱਦੋ(I)))
          Destructor(Content->get_ਮੁੱਲ_ਸੂਚਕ());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ਜੜ੍ਹ()->ਅਗਲਾ_2;
  ਜਦੋਂ (Trie) {
    TrieSubtrie *ਅਗਲਾ_2 = Trie->ਅਗਲਾ_2.exchange(ਖਾਲੀ_ਸੂਚਕ);
    ਮਿਟਾਓ Trie;
    Trie = ਅਗਲਾ_2;
  }
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ
Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਜੜ੍ਹ() ਅਚਲ {
  ImplType *Impl = ImplPtr.ਲੱਦੋ();
  ਜੇ (!Impl)
    ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ();
  ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ(Impl->get_ਜੜ੍ਹ());
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ P) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!P.isHint() && "Not a valid trie");
  ਜੇ (!P.P)
    ਵਾਪਸ 0;
  ਜੇ (ਸਵੈਚਾਲਿਤ *S = dyn_cast<TrieSubtrie>((Trie_ਗੰਢ *)P.P))
    ਵਾਪਸ S->ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ;
  ਵਾਪਸ 0;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਅੰਕ_ਬਿੱਟ(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ P) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!P.isHint() && "Not a valid trie");
  ਜੇ (!P.P)
    ਵਾਪਸ 0;
  ਜੇ (ਸਵੈਚਾਲਿਤ *S = dyn_cast<TrieSubtrie>((Trie_ਗੰਢ *)P.P))
    ਵਾਪਸ S->ਅੰਕ_ਬਿੱਟ_2;
  ਵਾਪਸ 0;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਅੰਕ_ਥਾਂ_ਵਰਤਿਆ(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ P) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!P.isHint() && "Not a valid trie");
  ਜੇ (!P.P)
    ਵਾਪਸ 0;
  ਸਵੈਚਾਲਿਤ *S = dyn_cast<TrieSubtrie>((Trie_ਗੰਢ *)P.P);
  ਜੇ (!S)
    ਵਾਪਸ 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, E = S->ਮਾਪ_2(); I < E; ++I)
    ਜੇ (S->ਲੱਦੋ(I))
      ++ਅੰਕ;
  ਵਾਪਸ ਅੰਕ;
}

std::string Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_Trie_ਅਗੇਤਰ_As_String(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ P) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!P.isHint() && "Not a valid trie");
  ਜੇ (!P.P)
    ਵਾਪਸ "";

  ਸਵੈਚਾਲਿਤ *S = dyn_cast<TrieSubtrie>((Trie_ਗੰਢ *)P.P);
  ਜੇ (!S || !S->IsSubtrie)
    ਵਾਪਸ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ਗੰਢ = ਖਾਲੀ_ਸੂਚਕ;
  ਜਦੋਂ (Current) {
    TrieSubtrie *ਅਗਲਾ_2 = ਖਾਲੀ_ਸੂਚਕ;
    // Find first used slot in the trie.
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, E = Current->ਮਾਪ_2(); I < E; ++I) {
      ਸਵੈਚਾਲਿਤ *S = Current->ਲੱਦੋ(I);
      ਜੇ (!S)
        ਜਾਰੀ;

      ਜੇ (ਸਵੈਚਾਲਿਤ *Content = dyn_cast<TrieContent>(S))
        ਗੰਢ = Content;
      ਨਹੀਂਤਾਂ ਜੇ (ਸਵੈਚਾਲਿਤ *Sub = dyn_cast<TrieSubtrie>(S))
        ਅਗਲਾ_2 = Sub;
      ਰੋਕੋ;
    }

    // Found the node.
    ਜੇ (ਗੰਢ)
      ਰੋਕੋ;

    // Continue to the next level if the node is not found.
    Current = ਅਗਲਾ_2;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(ਗੰਢ && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_ਭਰਿਆ_ਬਾਈਟਾਂ = (S->ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ + 1) / 8 - 1;
  SS << ਮੰਜ਼ਿਲ_Hex(ਮੰਜ਼ਿਲ_String_Ref(ਗੰਢ->getHash()).take_front(ਸ਼ੁਰੂ_ਭਰਿਆ_ਬਾਈਟਾਂ),
              /*LowerCase=*/ਸੱਚ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string ਬਿੱਟ_2;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = ਸ਼ੁਰੂ_ਭਰਿਆ_ਬਾਈਟਾਂ * 8, E = S->ਸ਼ੁਰੂ_ਦੋਆਧਾਰੀ_ਅੰਕ; I < E; ++I) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ_2 = I / 8;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ = 7 - I % 8;
    ਬਿੱਟ_2.ਧੱਕੋ_ਪਿੱਠ('0' + ((ਗੰਢ->getHash()[ਸੂਚਕਾਂਕ_2] >> ਵਿੱਥ) & 1));
  }

  ਜੇ (!ਬਿੱਟ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    SS << "[" << ਬਿੱਟ_2 << "]";

  ਵਾਪਸ SS.str();
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਅੰਕ_Tries() ਅਚਲ {
  ImplType *Impl = ImplPtr.ਲੱਦੋ();
  ਜੇ (!Impl)
    ਵਾਪਸ 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ = 0;
  ਲਈ (TrieSubtrie *Trie = Impl->get_ਜੜ੍ਹ(); Trie; Trie = Trie->ਅਗਲਾ_2.ਲੱਦੋ())
    ++ਅੰਕ;
  ਵਾਪਸ ਅੰਕ;
}

Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ
Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::get_ਅਗਲਾ_Trie(
    Thread_ਸੁਰੱਖਿਅਤ_Trie_Raw_Hash_Map_ਅਧਾਰ::ਸੂਚਕ_ਅਧਾਰ P) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!P.isHint() && "Not a valid trie");
  ਜੇ (!P.P)
    ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ();
  ਸਵੈਚਾਲਿਤ *S = dyn_cast<TrieSubtrie>((Trie_ਗੰਢ *)P.P);
  ਜੇ (!S)
    ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ();
  ਜੇ (ਸਵੈਚਾਲਿਤ *E = S->ਅਗਲਾ_2.ਲੱਦੋ())
    ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ(E);
  ਵਾਪਸ ਸੂਚਕ_ਅਧਾਰ();
}
