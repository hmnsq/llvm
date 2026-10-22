//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/TrieRawHashMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/LazyAtomicPointer.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/TrieHashIndexGenerator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ThreadSafeAllocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/TrailingObjects.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <memory>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {
ರಚನೆ Trie_ಗಂಟು {
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ IsSubtrie = ಸುಳ್ಳು;

  Trie_ಗಂಟು(ತಾರ್ಕಿಕ IsSubtrie) : IsSubtrie(IsSubtrie) {}

  ಸ್ಥಿರ ಶೂನ್ಯ *ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(size_t ಗಾತ್ರ_2) { ಹಿಂತಿರುಗಿ ::ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(ಗಾತ್ರ_2); }
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(ಶೂನ್ಯ *Ptr) { ::ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(Ptr); }
};

ರಚನೆ TrieContent final : ಸಾರ್ವಜನಿಕ Trie_ಗಂಟು {
  ಸ್ಥಿರಾಂಕ uint8_t Content_ಸ್ಥಾನಾಂತರ;
  ಸ್ಥಿರಾಂಕ uint8_t Hash_ಗಾತ್ರ;
  ಸ್ಥಿರಾಂಕ uint8_t Hash_ಸ್ಥಾನಾಂತರ;

  ಶೂನ್ಯ *get_ಮೌಲ್ಯ_ಸೂಚಕ() ಸ್ಥಿರಾಂಕ {
    ಸ್ವಯಂಚಾಲಿತ *Content = reinterpret_cast<ಸ್ಥಿರಾಂಕ uint8_t *>(ಇದು) + Content_ಸ್ಥಾನಾಂತರ;
    ಹಿಂತಿರುಗಿ const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ಸ್ಥಿರಾಂಕ {
    ಸ್ವಯಂಚಾಲಿತ *Begin = reinterpret_cast<ಸ್ಥಿರಾಂಕ uint8_t *>(ಇದು) + Hash_ಸ್ಥಾನಾಂತರ;
    ಹಿಂತಿರುಗಿ ArrayRef(Begin, Begin + Hash_ಗಾತ್ರ);
  }

  TrieContent(size_t Content_ಸ್ಥಾನಾಂತರ, size_t Hash_ಗಾತ್ರ, size_t Hash_ಸ್ಥಾನಾಂತರ)
      : Trie_ಗಂಟು(/*IsSubtrie=*/ಸುಳ್ಳು), Content_ಸ್ಥಾನಾಂತರ(Content_ಸ್ಥಾನಾಂತರ),
        Hash_ಗಾತ್ರ(Hash_ಗಾತ್ರ), Hash_ಸ್ಥಾನಾಂತರ(Hash_ಸ್ಥಾನಾಂತರ) {}

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ Trie_ಗಂಟು *TN) { ಹಿಂತಿರುಗಿ !TN->IsSubtrie; }
};

static_assert(ಗಾತ್ರ(TrieContent) ==
                  Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::Trie_Content_ತಳ_ಗಾತ್ರ,
              "Check header assumption!");

ವರ್ಗ TrieSubtrie final
    : ಸಾರ್ವಜನಿಕ Trie_ಗಂಟು,
      ಖಾಸಗಿ TrailingObjects<TrieSubtrie, Lazy_Atomic_ಸೂಚಕ<Trie_ಗಂಟು>> {
ಸಾರ್ವಜನಿಕ:
  ಬಳಸಿ ಸ್ಥಾನ = Lazy_Atomic_ಸೂಚಕ<Trie_ಗಂಟು>;

  ಸ್ಥಾನ &get(size_t I) { ಹಿಂತಿರುಗಿ getTrailingObjects()[I]; }
  Trie_ಗಂಟು *ತುಂಬು(size_t I) { ಹಿಂತಿರುಗಿ get(I).ತುಂಬು(); }

  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_3() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  ಸ್ಥಿರ std::unique_ptr<TrieSubtrie> create(size_t ಆರಂಭ_ಬಿಟ್, size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);

  ಸ್ಪಷ್ಟ TrieSubtrie(size_t ಆರಂಭ_ಬಿಟ್, size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ Trie_ಗಂಟು *TN) { ಹಿಂತಿರುಗಿ TN->IsSubtrie; }

  ಸ್ಥಿರ constexpr size_t ಗಾತ್ರ_ಗಮ್ಯ_Alloc(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2) {
    ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 < 20 && "Tries should have fewer than ~1M slots");
    ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 1u << ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2;
    ಹಿಂತಿರುಗಿ ಒಟ್ಟು_ಗಾತ್ರ_ಗಮ್ಯ_Alloc<Lazy_Atomic_ಸೂಚಕ<Trie_ಗಂಟು>>(ಎಣಿಕೆ_3);
  }

ಖಾಸಗಿ:
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
  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_ಬಿಟ್ = 0;
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = 0;
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = 0;
  ಮಿತ್ರ ವರ್ಗ llvm::Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ;
  ಮಿತ್ರ ವರ್ಗ TrailingObjects;

ಸಾರ್ವಜನಿಕ:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> ಮುಂದಿನ_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t ಆರಂಭ_ಬಿಟ್,
                                                 size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2) {
  ಶೂನ್ಯ *ಸ್ಮರಣೆ_2 = ::ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(ಗಾತ್ರ_ಗಮ್ಯ_Alloc(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2));
  TrieSubtrie *S = ::ಹೊಸ (ಸ್ಮರಣೆ_2) TrieSubtrie(ಆರಂಭ_ಬಿಟ್, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);
  ಹಿಂತಿರುಗಿ std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t ಆರಂಭ_ಬಿಟ್, size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2)
    : Trie_ಗಂಟು(ನಿಜ), ಆರಂಭ_ಬಿಟ್(ಆರಂಭ_ಬಿಟ್), ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2), ಗಾತ್ರ_2(1u << ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2),
      ಮುಂದಿನ_2(ಶೂನ್ಯಸೂಚಕ) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಗಾತ್ರ_2; ++I)
    ಹೊಸ (&get(I)) ಸ್ಥಾನ(ಶೂನ್ಯಸೂಚಕ);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_ಸೂಚಕ<Trie_ಗಂಟು>>::ಮೌಲ್ಯ_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು > 0);
  std::unique_ptr<TrieSubtrie> S = create(ಆರಂಭ_ಬಿಟ್ + ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು);

  ಪ್ರತಿಜ್ಞಾ(NewI < ಗಾತ್ರ_2);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ಗಂಟು *Existing_ಗಂಟು = &Content;
  ಪ್ರತಿಜ್ಞಾ(I < ಗಾತ್ರ_2);
  ಆದರೆ (get(I).compare_exchange_strong(Existing_ಗಂಟು, S.get()))
    ಹಿಂತಿರುಗಿ Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  ಹಿಂತಿರುಗಿ cast<TrieSubtrie>(Existing_ಗಂಟು);
}

ವರ್ಗ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ImplType final
    : ಖಾಸಗಿ TrailingObjects<Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ImplType,
                              TrieSubtrie> {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ std::unique_ptr<ImplType> create(size_t ಆರಂಭ_ಬಿಟ್, size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2) {
    size_t ಗಾತ್ರ_2 = ಗಾತ್ರ(ImplType) + TrieSubtrie::ಗಾತ್ರ_ಗಮ್ಯ_Alloc(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);
    ಶೂನ್ಯ *ಸ್ಮರಣೆ_2 = ::ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(ಗಾತ್ರ_2);
    ImplType *Impl = ::ಹೊಸ (ಸ್ಮರಣೆ_2) ImplType(ಆರಂಭ_ಬಿಟ್, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);
    ಹಿಂತಿರುಗಿ std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ಪ್ರತಿಜ್ಞಾ(!S->ಮುಂದಿನ_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_ತಲೆ = ಶೂನ್ಯಸೂಚಕ;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    ಆಗ (!get_ಬೇರು()->ಮುಂದಿನ_2.compare_exchange_weak(Current_ತಲೆ, S.get()))
      S->ಮುಂದಿನ_2.exchange(Current_ತಲೆ);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    ಹಿಂತಿರುಗಿ S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_ಬೇರು() { ಹಿಂತಿರುಗಿ getTrailingObjects(); }

  ಸ್ಥಿರ ಶೂನ್ಯ *ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(size_t ಗಾತ್ರ_2) { ಹಿಂತಿರುಗಿ ::ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(ಗಾತ್ರ_2); }
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(ಶೂನ್ಯ *Ptr) { ::ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_ಸುರಕ್ಷಿತ_Allocator<BumpPtrAllocator> ContentAlloc;

ಖಾಸಗಿ:
  ಮಿತ್ರ ವರ್ಗ TrailingObjects;

  ImplType(size_t ಆರಂಭ_ಬಿಟ್, size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2) {
    ::ಹೊಸ (get_ಬೇರು()) TrieSubtrie(ಆರಂಭ_ಬಿಟ್, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2);
  }
};

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ImplType &
Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::getOrCreateImpl() {
  ಆದರೆ (ImplType *Impl = ImplPtr.ತುಂಬು())
    ಹಿಂತಿರುಗಿ *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು);
  ImplType *ExistingImpl = ಶೂನ್ಯಸೂಚಕ;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ಆದರೆ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    ಹಿಂತಿರುಗಿ *Impl.release();

  // Already created, return the existing ImplType.
  ಹಿಂತಿರುಗಿ *ExistingImpl;
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ
Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಕಂಡುಹಿಡಿ(ArrayRef<uint8_t> Hash) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!Hash.ಖಾಲಿಗಳು() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.ತುಂಬು();
  ಆದರೆ (!Impl)
    ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ();

  TrieSubtrie *S = Impl->get_ಬೇರು();
  Trie_Hash_ಸೂಚ್ಯಂಕ_Generator ಸೂಚ್ಯಂಕ_Gen{ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು, ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು, Hash};
  size_t ಸೂಚ್ಯಂಕ_2 = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
  ಆಗ (ಸೂಚ್ಯಂಕ_2 != ಸೂಚ್ಯಂಕ_Gen.ಕೊನೆ()) {
    // Try to set the content.
    Trie_ಗಂಟು *Existing = S->get(ಸೂಚ್ಯಂಕ_2);
    ಆದರೆ (!Existing)
      ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ(S, ಸೂಚ್ಯಂಕ_2, *ಸೂಚ್ಯಂಕ_Gen.ಆರಂಭ_ಬಿಟ್);

    // Check for an exact match.
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ExistingContent = dyn_cast<TrieContent>(Existing))
      ಹಿಂತಿರುಗಿ ExistingContent->getHash() == Hash
                 ? ಸೂಚಕ_ತಳ(ExistingContent->get_ಮೌಲ್ಯ_ಸೂಚಕ())
                 : ಸೂಚಕ_ತಳ(S, ಸೂಚ್ಯಂಕ_2, *ಸೂಚ್ಯಂಕ_Gen.ಆರಂಭ_ಬಿಟ್);

    ಸೂಚ್ಯಂಕ_2 = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೇರಿಸು(
    ಸೂಚಕ_ತಳ Hint, ArrayRef<uint8_t> Hash,
    function_ref<ಸ್ಥಿರಾಂಕ uint8_t *(ಶೂನ್ಯ *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ಪ್ರತಿಜ್ಞಾ(!Hash.ಖಾಲಿಗಳು() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_ಬೇರು();
  Trie_Hash_ಸೂಚ್ಯಂಕ_Generator ಸೂಚ್ಯಂಕ_Gen{ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು, ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು, Hash};
  size_t ಸೂಚ್ಯಂಕ_2;
  ಆದರೆ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    ಸೂಚ್ಯಂಕ_2 = ಸೂಚ್ಯಂಕ_Gen.hint(Hint.I, Hint.B);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಸೂಚ್ಯಂಕ_2 = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
  }

  ಆಗ (ಸೂಚ್ಯಂಕ_2 != ಸೂಚ್ಯಂಕ_Gen.ಕೊನೆ()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    ತಾರ್ಕಿಕ Generated = ಸುಳ್ಳು;
    Trie_ಗಂಟು &Existing = S->get(ಸೂಚ್ಯಂಕ_2).ತುಂಬು_Or_Generate([&]() {
      Generated = ನಿಜ;

      // Construct the value itself at the tail.
      uint8_t *ಸ್ಮರಣೆ_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_ಗಾತ್ರ, ContentAllocAlign));
      ಸ್ಥಿರಾಂಕ uint8_t *HashStorage = Constructor(ಸ್ಮರಣೆ_2 + Content_ಸ್ಥಾನಾಂತರ, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::ಹೊಸ (ಸ್ಮರಣೆ_2)
          TrieContent(Content_ಸ್ಥಾನಾಂತರ, Hash.ಗಾತ್ರ_3(), HashStorage - ಸ್ಮರಣೆ_2);
      ಪ್ರತಿಜ್ಞಾ(Hash == Content->getHash() && "Hash not properly initialized");
      ಹಿಂತಿರುಗಿ Content;
    });
    // If we just generated it, return it!
    ಆದರೆ (Generated)
      ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ(cast<TrieContent>(Existing).get_ಮೌಲ್ಯ_ಸೂಚಕ());

    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      ಸೂಚ್ಯಂಕ_2 = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
      ಮುಂದುವರಿ;
    }

    // Return the existing content if it's an exact match!
    ಸ್ವಯಂಚಾಲಿತ &ExistingContent = cast<TrieContent>(Existing);
    ಆದರೆ (ExistingContent.getHash() == Hash)
      ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ(ExistingContent.get_ಮೌಲ್ಯ_ಸೂಚಕ());

    // Sink the existing content as long as the indexes match.
    size_t ಮುಂದಿನ_ಸೂಚ್ಯಂಕ = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
    ಆಗ (ಮುಂದಿನ_ಸೂಚ್ಯಂಕ != ಸೂಚ್ಯಂಕ_Gen.ಕೊನೆ()) {
      size_t New_ಸೂಚ್ಯಂಕ_For_Existing_Content =
          ಸೂಚ್ಯಂಕ_Gen.get_Colliding_ಬಿಟ್ಟುಗಳು(ExistingContent.getHash());
      S = S->sink(ಸೂಚ್ಯಂಕ_2, ExistingContent, ಸೂಚ್ಯಂಕ_Gen.get_ಅಂಕೆ_ಬಿಟ್ಟುಗಳು(),
                  New_ಸೂಚ್ಯಂಕ_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    ಹಿಂತಿರುಗಿ Impl.save(std::move(S));
                  });
      ಸೂಚ್ಯಂಕ_2 = ಮುಂದಿನ_ಸೂಚ್ಯಂಕ;

      // Found the difference.
      ಆದರೆ (ಮುಂದಿನ_ಸೂಚ್ಯಂಕ != New_ಸೂಚ್ಯಂಕ_For_Existing_Content)
        ನಿಲ್ಲಿಸು;

      ಮುಂದಿನ_ಸೂಚ್ಯಂಕ = ಸೂಚ್ಯಂಕ_Gen.ಮುಂದಿನ();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ(
    size_t Content_Alloc_ಗಾತ್ರ, size_t ContentAllocAlign, size_t Content_ಸ್ಥಾನಾಂತರ,
    std::optional<size_t> ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು, std::optional<size_t> ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು)
    : Content_Alloc_ಗಾತ್ರ(Content_Alloc_ಗಾತ್ರ), ContentAllocAlign(ContentAllocAlign),
      Content_ಸ್ಥಾನಾಂತರ(Content_ಸ್ಥಾನಾಂತರ),
      ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು(ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು.ಮೌಲ್ಯ_or(Default_ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು)),
      ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು(ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು.ಮೌಲ್ಯ_or(Default_ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು)),
      ImplPtr(ಶೂನ್ಯಸೂಚಕ) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ಪ್ರತಿಜ್ಞಾ((!ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು || *ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು < 20) &&
         "Root should have fewer than ~1M slots");
  ಪ್ರತಿಜ್ಞಾ((!ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು || *ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ &&RHS)
    : Content_Alloc_ಗಾತ್ರ(RHS.Content_Alloc_ಗಾತ್ರ),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_ಸ್ಥಾನಾಂತರ(RHS.Content_ಸ್ಥಾನಾಂತರ), ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು(RHS.ಅಂಕೆ_ಬೇರು_ಬಿಟ್ಟುಗಳು),
      ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು(RHS.ಅಂಕೆ_Subtrie_ಬಿಟ್ಟುಗಳು) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(ಶೂನ್ಯಸೂಚಕ);
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::~Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ() {
  ಪ್ರತಿಜ್ಞಾ(!ImplPtr.ತುಂಬು() && "Expected subclass to call destroyImpl()");
}

ಶೂನ್ಯ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::destroyImpl(
    function_ref<ಶೂನ್ಯ(ಶೂನ್ಯ *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(ಶೂನ್ಯಸೂಚಕ));
  ಆದರೆ (!Impl)
    ಹಿಂತಿರುಗಿ;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ಆದರೆ (Destructor)
    ಗಾಗಿ (TrieSubtrie *Trie = Impl->get_ಬೇರು(); Trie; Trie = Trie->ಮುಂದಿನ_2.ತುಂಬು())
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Trie->ಗಾತ್ರ_3(); ++I)
        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Content = dyn_cast_or_null<TrieContent>(Trie->ತುಂಬು(I)))
          Destructor(Content->get_ಮೌಲ್ಯ_ಸೂಚಕ());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_ಬೇರು()->ಮುಂದಿನ_2;
  ಆಗ (Trie) {
    TrieSubtrie *ಮುಂದಿನ_2 = Trie->ಮುಂದಿನ_2.exchange(ಶೂನ್ಯಸೂಚಕ);
    ಅಳಿಸು Trie;
    Trie = ಮುಂದಿನ_2;
  }
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ
Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಬೇರು() ಸ್ಥಿರಾಂಕ {
  ImplType *Impl = ImplPtr.ತುಂಬು();
  ಆದರೆ (!Impl)
    ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ();
  ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ(Impl->get_ಬೇರು());
}

ಚಿಹ್ನೆರಹಿತ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಆರಂಭ_ಬಿಟ್(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ P) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಆದರೆ (!P.P)
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P))
    ಹಿಂತಿರುಗಿ S->ಆರಂಭ_ಬಿಟ್;
  ಹಿಂತಿರುಗಿ 0;
}

ಚಿಹ್ನೆರಹಿತ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಅಂಕೆ_ಬಿಟ್ಟುಗಳು(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ P) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಆದರೆ (!P.P)
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P))
    ಹಿಂತಿರುಗಿ S->ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2;
  ಹಿಂತಿರುಗಿ 0;
}

ಚಿಹ್ನೆರಹಿತ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಅಂಕೆ_ಸ್ಥಾನ_ಬಳಸಿದ(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ P) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಆದರೆ (!P.P)
    ಹಿಂತಿರುಗಿ 0;
  ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಆದರೆ (!S)
    ಹಿಂತಿರುಗಿ 0;
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, E = S->ಗಾತ್ರ_3(); I < E; ++I)
    ಆದರೆ (S->ತುಂಬು(I))
      ++ಅಂಕೆ;
  ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

std::string Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_Trie_ಪೂರ್ವಪ್ರತ್ಯಯ_As_String(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ P) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಆದರೆ (!P.P)
    ಹಿಂತಿರುಗಿ "";

  ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಆದರೆ (!S || !S->IsSubtrie)
    ಹಿಂತಿರುಗಿ "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ಗಂಟು = ಶೂನ್ಯಸೂಚಕ;
  ಆಗ (Current) {
    TrieSubtrie *ಮುಂದಿನ_2 = ಶೂನ್ಯಸೂಚಕ;
    // Find first used slot in the trie.
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, E = Current->ಗಾತ್ರ_3(); I < E; ++I) {
      ಸ್ವಯಂಚಾಲಿತ *S = Current->ತುಂಬು(I);
      ಆದರೆ (!S)
        ಮುಂದುವರಿ;

      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Content = dyn_cast<TrieContent>(S))
        ಗಂಟು = Content;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Sub = dyn_cast<TrieSubtrie>(S))
        ಮುಂದಿನ_2 = Sub;
      ನಿಲ್ಲಿಸು;
    }

    // Found the node.
    ಆದರೆ (ಗಂಟು)
      ನಿಲ್ಲಿಸು;

    // Continue to the next level if the node is not found.
    Current = ಮುಂದಿನ_2;
  }

  ಪ್ರತಿಜ್ಞಾ(ಗಂಟು && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_ತುಂಬಿದ_ಬೈಟುಗಳು = (S->ಆರಂಭ_ಬಿಟ್ + 1) / 8 - 1;
  SS << ಗಮ್ಯ_Hex(ಗಮ್ಯ_String_Ref(ಗಂಟು->getHash()).take_front(ಆರಂಭ_ತುಂಬಿದ_ಬೈಟುಗಳು),
              /*LowerCase=*/ನಿಜ);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string ಬಿಟ್ಟುಗಳು_2;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = ಆರಂಭ_ತುಂಬಿದ_ಬೈಟುಗಳು * 8, E = S->ಆರಂಭ_ಬಿಟ್; I < E; ++I) {
    ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ_2 = I / 8;
    ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ = 7 - I % 8;
    ಬಿಟ್ಟುಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ('0' + ((ಗಂಟು->getHash()[ಸೂಚ್ಯಂಕ_2] >> ಸ್ಥಾನಾಂತರ) & 1));
  }

  ಆದರೆ (!ಬಿಟ್ಟುಗಳು_2.ಖಾಲಿಗಳು())
    SS << "[" << ಬಿಟ್ಟುಗಳು_2 << "]";

  ಹಿಂತಿರುಗಿ SS.str();
}

ಚಿಹ್ನೆರಹಿತ Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಅಂಕೆ_Tries() ಸ್ಥಿರಾಂಕ {
  ImplType *Impl = ImplPtr.ತುಂಬು();
  ಆದರೆ (!Impl)
    ಹಿಂತಿರುಗಿ 0;
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ = 0;
  ಗಾಗಿ (TrieSubtrie *Trie = Impl->get_ಬೇರು(); Trie; Trie = Trie->ಮುಂದಿನ_2.ತುಂಬು())
    ++ಅಂಕೆ;
  ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ
Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::get_ಮುಂದಿನ_Trie(
    Thread_ಸುರಕ್ಷಿತ_Trie_Raw_Hash_Map_ತಳ::ಸೂಚಕ_ತಳ P) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!P.isHint() && "Not a valid trie");
  ಆದರೆ (!P.P)
    ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ();
  ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast<TrieSubtrie>((Trie_ಗಂಟು *)P.P);
  ಆದರೆ (!S)
    ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ();
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *E = S->ಮುಂದಿನ_2.ತುಂಬು())
    ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ(E);
  ಹಿಂತಿರುಗಿ ಸೂಚಕ_ತಳ();
}
