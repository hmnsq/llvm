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

उपयुज्य नामाकाश llvm;

नामाकाश {
संरचना Trie_ग्रन्थिः {
  ध्रुव तार्किक IsSubtrie = असत्य;

  Trie_ग्रन्थिः(तार्किक IsSubtrie) : IsSubtrie(IsSubtrie) {}

  स्थिर शून्य *संक्रियकः नूतन(size_t आकार) { प्रत्यागम ::संक्रियकः नूतन(आकार); }
  शून्य संक्रियकः विलोपय(शून्य *Ptr) { ::संक्रियकः विलोपय(Ptr); }
};

संरचना TrieContent final : सार्वजनिक Trie_ग्रन्थिः {
  ध्रुव uint8_t Content_विस्थापनम्;
  ध्रुव uint8_t Hash_आकार;
  ध्रुव uint8_t Hash_विस्थापनम्;

  शून्य *get_मानम्_Pointer() ध्रुव {
    स्वचालित *Content = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + Content_विस्थापनम्;
    प्रत्यागम const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() ध्रुव {
    स्वचालित *Begin = reinterpret_cast<ध्रुव uint8_t *>(एतत्) + Hash_विस्थापनम्;
    प्रत्यागम ArrayRef(Begin, Begin + Hash_आकार);
  }

  TrieContent(size_t Content_विस्थापनम्, size_t Hash_आकार, size_t Hash_विस्थापनम्)
      : Trie_ग्रन्थिः(/*IsSubtrie=*/असत्य), Content_विस्थापनम्(Content_विस्थापनम्),
        Hash_आकार(Hash_आकार), Hash_विस्थापनम्(Hash_विस्थापनम्) {}

  स्थिर तार्किक classof(ध्रुव Trie_ग्रन्थिः *TN) { प्रत्यागम !TN->IsSubtrie; }
};

static_assert(परिमाण(TrieContent) ==
                  Thread_Safe_Trie_Raw_Hash_Map_आधारः::Trie_Content_आधारः_आकार,
              "Check header assumption!");

वर्ग TrieSubtrie final
    : सार्वजनिक Trie_ग्रन्थिः,
      गुप्त TrailingObjects<TrieSubtrie, LazyAtomicPointer<Trie_ग्रन्थिः>> {
सार्वजनिक:
  उपयुज्य Slot = LazyAtomicPointer<Trie_ग्रन्थिः>;

  Slot &get(size_t I) { प्रत्यागम getTrailingObjects()[I]; }
  Trie_ग्रन्थिः *आरोपय(size_t I) { प्रत्यागम get(I).आरोपय(); }

  अचिह्नित आकार_2() ध्रुव { प्रत्यागम आकार; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t अङ्कः_Subtrie_द्वयङ्काः, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  स्थिर std::unique_ptr<TrieSubtrie> create(size_t StartBit, size_t अङ्कः_द्वयङ्काः_2);

  स्पष्ट TrieSubtrie(size_t StartBit, size_t अङ्कः_द्वयङ्काः_2);

  स्थिर तार्किक classof(ध्रुव Trie_ग्रन्थिः *TN) { प्रत्यागम TN->IsSubtrie; }

  स्थिर constexpr size_t आकार_गन्तव्यम्_Alloc(अचिह्नित अङ्कः_द्वयङ्काः_2) {
    प्रतिज्ञा(अङ्कः_द्वयङ्काः_2 < 20 && "Tries should have fewer than ~1M slots");
    अचिह्नित गणनम्_2 = 1u << अङ्कः_द्वयङ्काः_2;
    प्रत्यागम योग_आकार_गन्तव्यम्_Alloc<LazyAtomicPointer<Trie_ग्रन्थिः>>(गणनम्_2);
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
  अचिह्नित StartBit = 0;
  अचिह्नित अङ्कः_द्वयङ्काः_2 = 0;
  अचिह्नित आकार = 0;
  मित्रम् वर्ग llvm::Thread_Safe_Trie_Raw_Hash_Map_आधारः;
  मित्रम् वर्ग TrailingObjects;

सार्वजनिक:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> अग्र_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t StartBit,
                                                 size_t अङ्कः_द्वयङ्काः_2) {
  शून्य *स्मृतिः_2 = ::संक्रियकः नूतन(आकार_गन्तव्यम्_Alloc(अङ्कः_द्वयङ्काः_2));
  TrieSubtrie *S = ::नूतन (स्मृतिः_2) TrieSubtrie(StartBit, अङ्कः_द्वयङ्काः_2);
  प्रत्यागम std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t StartBit, size_t अङ्कः_द्वयङ्काः_2)
    : Trie_ग्रन्थिः(सत्य), StartBit(StartBit), अङ्कः_द्वयङ्काः_2(अङ्कः_द्वयङ्काः_2), आकार(1u << अङ्कः_द्वयङ्काः_2),
      अग्र_2(शून्यसूचकः) {
  प्रति (अचिह्नित I = 0; I < आकार; ++I)
    नूतन (&get(I)) Slot(शून्यसूचकः);

  static_assert(
      std::is_trivially_destructible<LazyAtomicPointer<Trie_ग्रन्थिः>>::मानम्_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t अङ्कः_Subtrie_द्वयङ्काः, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  प्रतिज्ञा(अङ्कः_Subtrie_द्वयङ्काः > 0);
  std::unique_ptr<TrieSubtrie> S = create(StartBit + अङ्कः_द्वयङ्काः_2, अङ्कः_Subtrie_द्वयङ्काः);

  प्रतिज्ञा(NewI < आकार);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_ग्रन्थिः *Existing_ग्रन्थिः = &Content;
  प्रतिज्ञा(I < आकार);
  यदि (get(I).compare_exchange_strong(Existing_ग्रन्थिः, S.get()))
    प्रत्यागम Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  प्रत्यागम cast<TrieSubtrie>(Existing_ग्रन्थिः);
}

वर्ग Thread_Safe_Trie_Raw_Hash_Map_आधारः::ImplType final
    : गुप्त TrailingObjects<Thread_Safe_Trie_Raw_Hash_Map_आधारः::ImplType,
                              TrieSubtrie> {
सार्वजनिक:
  स्थिर std::unique_ptr<ImplType> create(size_t StartBit, size_t अङ्कः_द्वयङ्काः_2) {
    size_t आकार = परिमाण(ImplType) + TrieSubtrie::आकार_गन्तव्यम्_Alloc(अङ्कः_द्वयङ्काः_2);
    शून्य *स्मृतिः_2 = ::संक्रियकः नूतन(आकार);
    ImplType *Impl = ::नूतन (स्मृतिः_2) ImplType(StartBit, अङ्कः_द्वयङ्काः_2);
    प्रत्यागम std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    प्रतिज्ञा(!S->अग्र_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_शीर्ष = शून्यसूचकः;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    यावत् (!get_मूलम्()->अग्र_2.compare_exchange_weak(Current_शीर्ष, S.get()))
      S->अग्र_2.exchange(Current_शीर्ष);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    प्रत्यागम S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_मूलम्() { प्रत्यागम getTrailingObjects(); }

  स्थिर शून्य *संक्रियकः नूतन(size_t आकार) { प्रत्यागम ::संक्रियकः नूतन(आकार); }
  शून्य संक्रियकः विलोपय(शून्य *Ptr) { ::संक्रियकः विलोपय(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  ThreadSafeAllocator<BumpPtrAllocator> ContentAlloc;

गुप्त:
  मित्रम् वर्ग TrailingObjects;

  ImplType(size_t StartBit, size_t अङ्कः_द्वयङ्काः_2) {
    ::नूतन (get_मूलम्()) TrieSubtrie(StartBit, अङ्कः_द्वयङ्काः_2);
  }
};

Thread_Safe_Trie_Raw_Hash_Map_आधारः::ImplType &
Thread_Safe_Trie_Raw_Hash_Map_आधारः::getOrCreateImpl() {
  यदि (ImplType *Impl = ImplPtr.आरोपय())
    प्रत्यागम *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, अङ्कः_मूलम्_द्वयङ्काः);
  ImplType *ExistingImpl = शून्यसूचकः;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  यदि (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    प्रत्यागम *Impl.release();

  // Already created, return the existing ImplType.
  प्रत्यागम *ExistingImpl;
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः
Thread_Safe_Trie_Raw_Hash_Map_आधारः::find(ArrayRef<uint8_t> Hash) ध्रुव {
  प्रतिज्ञा(!Hash.रिक्तानि() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.आरोपय();
  यदि (!Impl)
    प्रत्यागम Pointer_आधारः();

  TrieSubtrie *S = Impl->get_मूलम्();
  Trie_Hash_सूचकांक_Generator सूचकांक_Gen{अङ्कः_मूलम्_द्वयङ्काः, अङ्कः_Subtrie_द्वयङ्काः, Hash};
  size_t सूचकांक_2 = सूचकांक_Gen.अग्र();
  यावत् (सूचकांक_2 != सूचकांक_Gen.अन्तः()) {
    // Try to set the content.
    Trie_ग्रन्थिः *Existing = S->get(सूचकांक_2);
    यदि (!Existing)
      प्रत्यागम Pointer_आधारः(S, सूचकांक_2, *सूचकांक_Gen.StartBit);

    // Check for an exact match.
    यदि (स्वचालित *ExistingContent = dyn_cast<TrieContent>(Existing))
      प्रत्यागम ExistingContent->getHash() == Hash
                 ? Pointer_आधारः(ExistingContent->get_मानम्_Pointer())
                 : Pointer_आधारः(S, सूचकांक_2, *सूचकांक_Gen.StartBit);

    सूचकांक_2 = सूचकांक_Gen.अग्र();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः Thread_Safe_Trie_Raw_Hash_Map_आधारः::निवेशय(
    Pointer_आधारः Hint, ArrayRef<uint8_t> Hash,
    function_ref<ध्रुव uint8_t *(शून्य *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  प्रतिज्ञा(!Hash.रिक्तानि() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_मूलम्();
  Trie_Hash_सूचकांक_Generator सूचकांक_Gen{अङ्कः_मूलम्_द्वयङ्काः, अङ्कः_Subtrie_द्वयङ्काः, Hash};
  size_t सूचकांक_2;
  यदि (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    सूचकांक_2 = सूचकांक_Gen.hint(Hint.I, Hint.B);
  } अन्यथा {
    सूचकांक_2 = सूचकांक_Gen.अग्र();
  }

  यावत् (सूचकांक_2 != सूचकांक_Gen.अन्तः()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    तार्किक Generated = असत्य;
    Trie_ग्रन्थिः &Existing = S->get(सूचकांक_2).आरोपय_Or_Generate([&]() {
      Generated = सत्य;

      // Construct the value itself at the tail.
      uint8_t *स्मृतिः_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_आकार, ContentAllocAlign));
      ध्रुव uint8_t *HashStorage = Constructor(स्मृतिः_2 + Content_विस्थापनम्, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::नूतन (स्मृतिः_2)
          TrieContent(Content_विस्थापनम्, Hash.आकार_2(), HashStorage - स्मृतिः_2);
      प्रतिज्ञा(Hash == Content->getHash() && "Hash not properly initialized");
      प्रत्यागम Content;
    });
    // If we just generated it, return it!
    यदि (Generated)
      प्रत्यागम Pointer_आधारः(cast<TrieContent>(Existing).get_मानम्_Pointer());

    यदि (स्वचालित *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      सूचकांक_2 = सूचकांक_Gen.अग्र();
      अनुवर्त;
    }

    // Return the existing content if it's an exact match!
    स्वचालित &ExistingContent = cast<TrieContent>(Existing);
    यदि (ExistingContent.getHash() == Hash)
      प्रत्यागम Pointer_आधारः(ExistingContent.get_मानम्_Pointer());

    // Sink the existing content as long as the indexes match.
    size_t अग्र_सूचकांक = सूचकांक_Gen.अग्र();
    यावत् (अग्र_सूचकांक != सूचकांक_Gen.अन्तः()) {
      size_t New_सूचकांक_For_Existing_Content =
          सूचकांक_Gen.get_Colliding_द्वयङ्काः(ExistingContent.getHash());
      S = S->sink(सूचकांक_2, ExistingContent, सूचकांक_Gen.get_अङ्कः_द्वयङ्काः(),
                  New_सूचकांक_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    प्रत्यागम Impl.save(std::move(S));
                  });
      सूचकांक_2 = अग्र_सूचकांक;

      // Found the difference.
      यदि (अग्र_सूचकांक != New_सूचकांक_For_Existing_Content)
        भङ्ग;

      अग्र_सूचकांक = सूचकांक_Gen.अग्र();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Thread_Safe_Trie_Raw_Hash_Map_आधारः(
    size_t Content_Alloc_आकार, size_t ContentAllocAlign, size_t Content_विस्थापनम्,
    std::optional<size_t> अङ्कः_मूलम्_द्वयङ्काः, std::optional<size_t> अङ्कः_Subtrie_द्वयङ्काः)
    : Content_Alloc_आकार(Content_Alloc_आकार), ContentAllocAlign(ContentAllocAlign),
      Content_विस्थापनम्(Content_विस्थापनम्),
      अङ्कः_मूलम्_द्वयङ्काः(अङ्कः_मूलम्_द्वयङ्काः.मानम्_or(Default_अङ्कः_मूलम्_द्वयङ्काः)),
      अङ्कः_Subtrie_द्वयङ्काः(अङ्कः_Subtrie_द्वयङ्काः.मानम्_or(Default_अङ्कः_Subtrie_द्वयङ्काः)),
      ImplPtr(शून्यसूचकः) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  प्रतिज्ञा((!अङ्कः_मूलम्_द्वयङ्काः || *अङ्कः_मूलम्_द्वयङ्काः < 20) &&
         "Root should have fewer than ~1M slots");
  प्रतिज्ञा((!अङ्कः_Subtrie_द्वयङ्काः || *अङ्कः_Subtrie_द्वयङ्काः < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Thread_Safe_Trie_Raw_Hash_Map_आधारः(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः &&RHS)
    : Content_Alloc_आकार(RHS.Content_Alloc_आकार),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_विस्थापनम्(RHS.Content_विस्थापनम्), अङ्कः_मूलम्_द्वयङ्काः(RHS.अङ्कः_मूलम्_द्वयङ्काः),
      अङ्कः_Subtrie_द्वयङ्काः(RHS.अङ्कः_Subtrie_द्वयङ्काः) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(शून्यसूचकः);
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::~Thread_Safe_Trie_Raw_Hash_Map_आधारः() {
  प्रतिज्ञा(!ImplPtr.आरोपय() && "Expected subclass to call destroyImpl()");
}

शून्य Thread_Safe_Trie_Raw_Hash_Map_आधारः::destroyImpl(
    function_ref<शून्य(शून्य *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(शून्यसूचकः));
  यदि (!Impl)
    प्रत्यागम;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  यदि (Destructor)
    प्रति (TrieSubtrie *Trie = Impl->get_मूलम्(); Trie; Trie = Trie->अग्र_2.आरोपय())
      प्रति (अचिह्नित I = 0; I < Trie->आकार_2(); ++I)
        यदि (स्वचालित *Content = dyn_cast_or_null<TrieContent>(Trie->आरोपय(I)))
          Destructor(Content->get_मानम्_Pointer());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_मूलम्()->अग्र_2;
  यावत् (Trie) {
    TrieSubtrie *अग्र_2 = Trie->अग्र_2.exchange(शून्यसूचकः);
    विलोपय Trie;
    Trie = अग्र_2;
  }
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः
Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_मूलम्() ध्रुव {
  ImplType *Impl = ImplPtr.आरोपय();
  यदि (!Impl)
    प्रत्यागम Pointer_आधारः();
  प्रत्यागम Pointer_आधारः(Impl->get_मूलम्());
}

अचिह्नित Thread_Safe_Trie_Raw_Hash_Map_आधारः::getStartBit(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  यदि (स्वचालित *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P))
    प्रत्यागम S->StartBit;
  प्रत्यागम 0;
}

अचिह्नित Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_अङ्कः_द्वयङ्काः(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  यदि (स्वचालित *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P))
    प्रत्यागम S->अङ्कः_द्वयङ्काः_2;
  प्रत्यागम 0;
}

अचिह्नित Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_अङ्कः_Slot_प्रयुक्तम्(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम 0;
  स्वचालित *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  यदि (!S)
    प्रत्यागम 0;
  अचिह्नित अङ्कः = 0;
  प्रति (अचिह्नित I = 0, E = S->आकार_2(); I < E; ++I)
    यदि (S->आरोपय(I))
      ++अङ्कः;
  प्रत्यागम अङ्कः;
}

std::string Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_Trie_उपसर्गः_As_String(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम "";

  स्वचालित *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  यदि (!S || !S->IsSubtrie)
    प्रत्यागम "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *ग्रन्थिः = शून्यसूचकः;
  यावत् (Current) {
    TrieSubtrie *अग्र_2 = शून्यसूचकः;
    // Find first used slot in the trie.
    प्रति (अचिह्नित I = 0, E = Current->आकार_2(); I < E; ++I) {
      स्वचालित *S = Current->आरोपय(I);
      यदि (!S)
        अनुवर्त;

      यदि (स्वचालित *Content = dyn_cast<TrieContent>(S))
        ग्रन्थिः = Content;
      अन्यथा यदि (स्वचालित *Sub = dyn_cast<TrieSubtrie>(S))
        अग्र_2 = Sub;
      भङ्ग;
    }

    // Found the node.
    यदि (ग्रन्थिः)
      भङ्ग;

    // Continue to the next level if the node is not found.
    Current = अग्र_2;
  }

  प्रतिज्ञा(ग्रन्थिः && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  अचिह्नित Start_पूर्णम्_अष्टकानि = (S->StartBit + 1) / 8 - 1;
  SS << गन्तव्यम्_Hex(गन्तव्यम्_String_Ref(ग्रन्थिः->getHash()).take_front(Start_पूर्णम्_अष्टकानि),
              /*LowerCase=*/सत्य);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string द्वयङ्काः_2;
  प्रति (अचिह्नित I = Start_पूर्णम्_अष्टकानि * 8, E = S->StartBit; I < E; ++I) {
    अचिह्नित सूचकांक_2 = I / 8;
    अचिह्नित विस्थापनम् = 7 - I % 8;
    द्वयङ्काः_2.प्रक्षिप_पृष्ठभागः('0' + ((ग्रन्थिः->getHash()[सूचकांक_2] >> विस्थापनम्) & 1));
  }

  यदि (!द्वयङ्काः_2.रिक्तानि())
    SS << "[" << द्वयङ्काः_2 << "]";

  प्रत्यागम SS.str();
}

अचिह्नित Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_अङ्कः_Tries() ध्रुव {
  ImplType *Impl = ImplPtr.आरोपय();
  यदि (!Impl)
    प्रत्यागम 0;
  अचिह्नित अङ्कः = 0;
  प्रति (TrieSubtrie *Trie = Impl->get_मूलम्(); Trie; Trie = Trie->अग्र_2.आरोपय())
    ++अङ्कः;
  प्रत्यागम अङ्कः;
}

Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः
Thread_Safe_Trie_Raw_Hash_Map_आधारः::get_अग्र_Trie(
    Thread_Safe_Trie_Raw_Hash_Map_आधारः::Pointer_आधारः P) ध्रुव {
  प्रतिज्ञा(!P.isHint() && "Not a valid trie");
  यदि (!P.P)
    प्रत्यागम Pointer_आधारः();
  स्वचालित *S = dyn_cast<TrieSubtrie>((Trie_ग्रन्थिः *)P.P);
  यदि (!S)
    प्रत्यागम Pointer_आधारः();
  यदि (स्वचालित *E = S->अग्र_2.आरोपय())
    प्रत्यागम Pointer_आधारः(E);
  प्रत्यागम Pointer_आधारः();
}
