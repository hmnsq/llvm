//===- TrieRawHashMap.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/TrieRawHashMap.h"
#అన్తర్భావ "llvm/ADT/LazyAtomicPointer.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/TrieHashIndexGenerator.h"
#అన్తర్భావ "llvm/Support/Allocator.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/ThreadSafeAllocator.h"
#అన్తర్భావ "llvm/Support/TrailingObjects.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <memory>

ఉపయోగించి నామాకాశ llvm;

నామాకాశ {
నిర్మాణం Trie_కణుపు {
  స్థిరరాశి తార్కిక IsSubtrie = అబద్ధం;

  Trie_కణుపు(తార్కిక IsSubtrie) : IsSubtrie(IsSubtrie) {}

  స్థిర శూన్య *క్రియాకారకం కొత్త(size_t కొలత) { తిరిగి ::క్రియాకారకం కొత్త(కొలత); }
  శూన్య క్రియాకారకం తొలగించు(శూన్య *Ptr) { ::క్రియాకారకం తొలగించు(Ptr); }
};

నిర్మాణం TrieContent final : బహిరంగ Trie_కణుపు {
  స్థిరరాశి uint8_t Content_స్థానభ్రంశం;
  స్థిరరాశి uint8_t Hash_కొలత;
  స్థిరరాశి uint8_t Hash_స్థానభ్రంశం;

  శూన్య *get_విలువ_సూచిక() స్థిరరాశి {
    స్వయంచాలక *Content = reinterpret_cast<స్థిరరాశి uint8_t *>(ఇది) + Content_స్థానభ్రంశం;
    తిరిగి const_cast<uint8_t *>(Content);
  }

  ArrayRef<uint8_t> getHash() స్థిరరాశి {
    స్వయంచాలక *Begin = reinterpret_cast<స్థిరరాశి uint8_t *>(ఇది) + Hash_స్థానభ్రంశం;
    తిరిగి ArrayRef(Begin, Begin + Hash_కొలత);
  }

  TrieContent(size_t Content_స్థానభ్రంశం, size_t Hash_కొలత, size_t Hash_స్థానభ్రంశం)
      : Trie_కణుపు(/*IsSubtrie=*/అబద్ధం), Content_స్థానభ్రంశం(Content_స్థానభ్రంశం),
        Hash_కొలత(Hash_కొలత), Hash_స్థానభ్రంశం(Hash_స్థానభ్రంశం) {}

  స్థిర తార్కిక classof(స్థిరరాశి Trie_కణుపు *TN) { తిరిగి !TN->IsSubtrie; }
};

static_assert(పరిమాణం(TrieContent) ==
                  Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::Trie_Content_పునాది_కొలత,
              "Check header assumption!");

తరగతి TrieSubtrie final
    : బహిరంగ Trie_కణుపు,
      అంతరంగ TrailingObjects<TrieSubtrie, Lazy_Atomic_సూచిక<Trie_కణుపు>> {
బహిరంగ:
  ఉపయోగించి స్థానం = Lazy_Atomic_సూచిక<Trie_కణుపు>;

  స్థానం &get(size_t I) { తిరిగి getTrailingObjects()[I]; }
  Trie_కణుపు *ఎక్కించు(size_t I) { తిరిగి get(I).ఎక్కించు(); }

  గుర్తులేని కొలత_2() స్థిరరాశి { తిరిగి కొలత; }

  TrieSubtrie *
  sink(size_t I, TrieContent &Content, size_t అంకె_Subtrie_బిట్లు, size_t NewI,
       function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver);

  స్థిర std::unique_ptr<TrieSubtrie> create(size_t ప్రారంభం_బిట్టు, size_t అంకె_బిట్లు_2);

  స్పష్టమైన TrieSubtrie(size_t ప్రారంభం_బిట్టు, size_t అంకె_బిట్లు_2);

  స్థిర తార్కిక classof(స్థిరరాశి Trie_కణుపు *TN) { తిరిగి TN->IsSubtrie; }

  స్థిర constexpr size_t కొలత_గమ్యం_Alloc(గుర్తులేని అంకె_బిట్లు_2) {
    ప్రతిజ్ఞా(అంకె_బిట్లు_2 < 20 && "Tries should have fewer than ~1M slots");
    గుర్తులేని లెక్క_2 = 1u << అంకె_బిట్లు_2;
    తిరిగి మొత్తం_కొలత_గమ్యం_Alloc<Lazy_Atomic_సూచిక<Trie_కణుపు>>(లెక్క_2);
  }

అంతరంగ:
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
  గుర్తులేని ప్రారంభం_బిట్టు = 0;
  గుర్తులేని అంకె_బిట్లు_2 = 0;
  గుర్తులేని కొలత = 0;
  మిత్ర తరగతి llvm::Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది;
  మిత్ర తరగతి TrailingObjects;

బహిరంగ:
  /// Linked list for ownership of tries. The pointer is owned by TrieSubtrie.
  std::atomic<TrieSubtrie *> తదుపరి_2;
};
} // end namespace

std::unique_ptr<TrieSubtrie> TrieSubtrie::create(size_t ప్రారంభం_బిట్టు,
                                                 size_t అంకె_బిట్లు_2) {
  శూన్య *జ్ఞాపకం_2 = ::క్రియాకారకం కొత్త(కొలత_గమ్యం_Alloc(అంకె_బిట్లు_2));
  TrieSubtrie *S = ::కొత్త (జ్ఞాపకం_2) TrieSubtrie(ప్రారంభం_బిట్టు, అంకె_బిట్లు_2);
  తిరిగి std::unique_ptr<TrieSubtrie>(S);
}

TrieSubtrie::TrieSubtrie(size_t ప్రారంభం_బిట్టు, size_t అంకె_బిట్లు_2)
    : Trie_కణుపు(నిజం), ప్రారంభం_బిట్టు(ప్రారంభం_బిట్టు), అంకె_బిట్లు_2(అంకె_బిట్లు_2), కొలత(1u << అంకె_బిట్లు_2),
      తదుపరి_2(శూన్యసూచిక) {
  కొరకు (గుర్తులేని I = 0; I < కొలత; ++I)
    కొత్త (&get(I)) స్థానం(శూన్యసూచిక);

  static_assert(
      std::is_trivially_destructible<Lazy_Atomic_సూచిక<Trie_కణుపు>>::విలువ_2,
      "Expected no work in destructor for TrieNode");
}

// Sink the nodes down sub-trie when the object being inserted collides with
// the index of existing object in the trie. In this case, a new sub-trie needs
// to be allocated to hold existing object.
TrieSubtrie *TrieSubtrie::sink(
    size_t I, TrieContent &Content, size_t అంకె_Subtrie_బిట్లు, size_t NewI,
    function_ref<TrieSubtrie *(std::unique_ptr<TrieSubtrie>)> Saver) {
  // Create a new sub-trie that points to the existing object with the new
  // index for the next level.
  ప్రతిజ్ఞా(అంకె_Subtrie_బిట్లు > 0);
  std::unique_ptr<TrieSubtrie> S = create(ప్రారంభం_బిట్టు + అంకె_బిట్లు_2, అంకె_Subtrie_బిట్లు);

  ప్రతిజ్ఞా(NewI < కొలత);
  S->get(NewI).store(&Content);

  // Using compare_exchange to atomically add back the new sub-trie to the trie
  // in the place of the exsiting object.
  Trie_కణుపు *Existing_కణుపు = &Content;
  ప్రతిజ్ఞా(I < కొలత);
  ఒకవేళ (get(I).compare_exchange_strong(Existing_కణుపు, S.get()))
    తిరిగి Saver(std::move(S));

  // Another thread created a subtrie already. Return it and let "S" be
  // destructed.
  తిరిగి cast<TrieSubtrie>(Existing_కణుపు);
}

తరగతి Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::ImplType final
    : అంతరంగ TrailingObjects<Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::ImplType,
                              TrieSubtrie> {
బహిరంగ:
  స్థిర std::unique_ptr<ImplType> create(size_t ప్రారంభం_బిట్టు, size_t అంకె_బిట్లు_2) {
    size_t కొలత = పరిమాణం(ImplType) + TrieSubtrie::కొలత_గమ్యం_Alloc(అంకె_బిట్లు_2);
    శూన్య *జ్ఞాపకం_2 = ::క్రియాకారకం కొత్త(కొలత);
    ImplType *Impl = ::కొత్త (జ్ఞాపకం_2) ImplType(ప్రారంభం_బిట్టు, అంకె_బిట్లు_2);
    తిరిగి std::unique_ptr<ImplType>(Impl);
  }

  // Save the Subtrie into the ownship list of the trie structure in a
  // thread-safe way. The ownership transfer is done by compare_exchange the
  // pointer value inside the unique_ptr.
  TrieSubtrie *save(std::unique_ptr<TrieSubtrie> S) {
    ప్రతిజ్ఞా(!S->తదుపరి_2 && "Expected S to a freshly-constructed leaf");

    TrieSubtrie *Current_తల = శూన్యసూచిక;
    // Add ownership of "S" to front of the list, so that Root -> S ->
    // Root.Next. This works by repeatedly setting S->Next to a candidate value
    // of Root.Next (initially nullptr), then setting Root.Next to S once the
    // candidate matches reality.
    అయితే (!get_మూలం()->తదుపరి_2.compare_exchange_weak(Current_తల, S.get()))
      S->తదుపరి_2.exchange(Current_తల);

    // Ownership transferred to subtrie successfully. Release the unique_ptr.
    తిరిగి S.release();
  }

  // Get the root which is the trailing object.
  TrieSubtrie *get_మూలం() { తిరిగి getTrailingObjects(); }

  స్థిర శూన్య *క్రియాకారకం కొత్త(size_t కొలత) { తిరిగి ::క్రియాకారకం కొత్త(కొలత); }
  శూన్య క్రియాకారకం తొలగించు(శూన్య *Ptr) { ::క్రియాకారకం తొలగించు(Ptr); }

  /// FIXME: This should take a function that allocates and constructs the
  /// content lazily (taking the hash as a separate parameter), in case of
  /// collision.
  Thread_సురక్షిత_Allocator<BumpPtrAllocator> ContentAlloc;

అంతరంగ:
  మిత్ర తరగతి TrailingObjects;

  ImplType(size_t ప్రారంభం_బిట్టు, size_t అంకె_బిట్లు_2) {
    ::కొత్త (get_మూలం()) TrieSubtrie(ప్రారంభం_బిట్టు, అంకె_బిట్లు_2);
  }
};

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::ImplType &
Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::getOrCreateImpl() {
  ఒకవేళ (ImplType *Impl = ImplPtr.ఎక్కించు())
    తిరిగి *Impl;

  // Create a new ImplType and store it if another thread doesn't do so first.
  // If another thread wins this one is destroyed locally.
  std::unique_ptr<ImplType> Impl = ImplType::create(0, అంకె_మూలం_బిట్లు);
  ImplType *ExistingImpl = శూన్యసూచిక;

  // If the ownership transferred succesfully, release unique_ptr and return
  // the pointer to the new ImplType.
  ఒకవేళ (ImplPtr.compare_exchange_strong(ExistingImpl, Impl.get()))
    తిరిగి *Impl.release();

  // Already created, return the existing ImplType.
  తిరిగి *ExistingImpl;
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది
Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::కనుగొను(ArrayRef<uint8_t> Hash) స్థిరరాశి {
  ప్రతిజ్ఞా(!Hash.ఖాళీలు() && "Uninitialized hash");

  ImplType *Impl = ImplPtr.ఎక్కించు();
  ఒకవేళ (!Impl)
    తిరిగి సూచిక_పునాది();

  TrieSubtrie *S = Impl->get_మూలం();
  Trie_Hash_సూచీ_Generator సూచీ_Gen{అంకె_మూలం_బిట్లు, అంకె_Subtrie_బిట్లు, Hash};
  size_t సూచీ_2 = సూచీ_Gen.తదుపరి();
  అయితే (సూచీ_2 != సూచీ_Gen.చివర()) {
    // Try to set the content.
    Trie_కణుపు *Existing = S->get(సూచీ_2);
    ఒకవేళ (!Existing)
      తిరిగి సూచిక_పునాది(S, సూచీ_2, *సూచీ_Gen.ప్రారంభం_బిట్టు);

    // Check for an exact match.
    ఒకవేళ (స్వయంచాలక *ExistingContent = dyn_cast<TrieContent>(Existing))
      తిరిగి ExistingContent->getHash() == Hash
                 ? సూచిక_పునాది(ExistingContent->get_విలువ_సూచిక())
                 : సూచిక_పునాది(S, సూచీ_2, *సూచీ_Gen.ప్రారంభం_బిట్టు);

    సూచీ_2 = సూచీ_Gen.తదుపరి();
    S = cast<TrieSubtrie>(Existing);
  }
  llvm_unreachable("failed to locate the node after consuming all hash bytes");
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::చొప్పించు(
    సూచిక_పునాది Hint, ArrayRef<uint8_t> Hash,
    function_ref<స్థిరరాశి uint8_t *(శూన్య *Mem, ArrayRef<uint8_t> Hash)>
        Constructor) {
  ప్రతిజ్ఞా(!Hash.ఖాళీలు() && "Uninitialized hash");

  ImplType &Impl = getOrCreateImpl();
  TrieSubtrie *S = Impl.get_మూలం();
  Trie_Hash_సూచీ_Generator సూచీ_Gen{అంకె_మూలం_బిట్లు, అంకె_Subtrie_బిట్లు, Hash};
  size_t సూచీ_2;
  ఒకవేళ (Hint.isHint()) {
    S = static_cast<TrieSubtrie *>(Hint.P);
    సూచీ_2 = సూచీ_Gen.hint(Hint.I, Hint.B);
  } లేకపోతే {
    సూచీ_2 = సూచీ_Gen.తదుపరి();
  }

  అయితే (సూచీ_2 != సూచీ_Gen.చివర()) {
    // Load the node from the slot, allocating and calling the constructor if
    // the slot is empty.
    తార్కిక Generated = అబద్ధం;
    Trie_కణుపు &Existing = S->get(సూచీ_2).ఎక్కించు_Or_Generate([&]() {
      Generated = నిజం;

      // Construct the value itself at the tail.
      uint8_t *జ్ఞాపకం_2 = reinterpret_cast<uint8_t *>(
          Impl.ContentAlloc.Allocate(Content_Alloc_కొలత, ContentAllocAlign));
      స్థిరరాశి uint8_t *HashStorage = Constructor(జ్ఞాపకం_2 + Content_స్థానభ్రంశం, Hash);

      // Construct the TrieContent header, passing in the offset to the hash.
      TrieContent *Content = ::కొత్త (జ్ఞాపకం_2)
          TrieContent(Content_స్థానభ్రంశం, Hash.కొలత_2(), HashStorage - జ్ఞాపకం_2);
      ప్రతిజ్ఞా(Hash == Content->getHash() && "Hash not properly initialized");
      తిరిగి Content;
    });
    // If we just generated it, return it!
    ఒకవేళ (Generated)
      తిరిగి సూచిక_పునాది(cast<TrieContent>(Existing).get_విలువ_సూచిక());

    ఒకవేళ (స్వయంచాలక *ST = dyn_cast<TrieSubtrie>(&Existing)) {
      S = ST;
      సూచీ_2 = సూచీ_Gen.తదుపరి();
      కొనసాగు;
    }

    // Return the existing content if it's an exact match!
    స్వయంచాలక &ExistingContent = cast<TrieContent>(Existing);
    ఒకవేళ (ExistingContent.getHash() == Hash)
      తిరిగి సూచిక_పునాది(ExistingContent.get_విలువ_సూచిక());

    // Sink the existing content as long as the indexes match.
    size_t తదుపరి_సూచీ = సూచీ_Gen.తదుపరి();
    అయితే (తదుపరి_సూచీ != సూచీ_Gen.చివర()) {
      size_t New_సూచీ_For_Existing_Content =
          సూచీ_Gen.get_Colliding_బిట్లు(ExistingContent.getHash());
      S = S->sink(సూచీ_2, ExistingContent, సూచీ_Gen.get_అంకె_బిట్లు(),
                  New_సూచీ_For_Existing_Content,
                  [&Impl](std::unique_ptr<TrieSubtrie> S) {
                    తిరిగి Impl.save(std::move(S));
                  });
      సూచీ_2 = తదుపరి_సూచీ;

      // Found the difference.
      ఒకవేళ (తదుపరి_సూచీ != New_సూచీ_For_Existing_Content)
        ఆపు;

      తదుపరి_సూచీ = సూచీ_Gen.తదుపరి();
    }
  }
  llvm_unreachable("failed to insert the node after consuming all hash bytes");
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది(
    size_t Content_Alloc_కొలత, size_t ContentAllocAlign, size_t Content_స్థానభ్రంశం,
    std::optional<size_t> అంకె_మూలం_బిట్లు, std::optional<size_t> అంకె_Subtrie_బిట్లు)
    : Content_Alloc_కొలత(Content_Alloc_కొలత), ContentAllocAlign(ContentAllocAlign),
      Content_స్థానభ్రంశం(Content_స్థానభ్రంశం),
      అంకె_మూలం_బిట్లు(అంకె_మూలం_బిట్లు.విలువ_or(Default_అంకె_మూలం_బిట్లు)),
      అంకె_Subtrie_బిట్లు(అంకె_Subtrie_బిట్లు.విలువ_or(Default_అంకె_Subtrie_బిట్లు)),
      ImplPtr(శూన్యసూచిక) {
  // Assertion checks for reasonable configuration. The settings below are not
  // hard limits on most platforms, but a reasonable configuration should fall
  // within those limits.
  ప్రతిజ్ఞా((!అంకె_మూలం_బిట్లు || *అంకె_మూలం_బిట్లు < 20) &&
         "Root should have fewer than ~1M slots");
  ప్రతిజ్ఞా((!అంకె_Subtrie_బిట్లు || *అంకె_Subtrie_బిట్లు < 10) &&
         "Subtries should have fewer than ~1K slots");
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది &&RHS)
    : Content_Alloc_కొలత(RHS.Content_Alloc_కొలత),
      ContentAllocAlign(RHS.ContentAllocAlign),
      Content_స్థానభ్రంశం(RHS.Content_స్థానభ్రంశం), అంకె_మూలం_బిట్లు(RHS.అంకె_మూలం_బిట్లు),
      అంకె_Subtrie_బిట్లు(RHS.అంకె_Subtrie_బిట్లు) {
  // Steal the root from RHS.
  ImplPtr = RHS.ImplPtr.exchange(శూన్యసూచిక);
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::~Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది() {
  ప్రతిజ్ఞా(!ImplPtr.ఎక్కించు() && "Expected subclass to call destroyImpl()");
}

శూన్య Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::destroyImpl(
    function_ref<శూన్య(శూన్య *)> Destructor) {
  std::unique_ptr<ImplType> Impl(ImplPtr.exchange(శూన్యసూచిక));
  ఒకవేళ (!Impl)
    తిరిగి;

  // Destroy content nodes throughout trie. Avoid destroying any subtries since
  // we need TrieNode::classof() to find the content nodes.
  //
  // FIXME: Once we have bitsets (see FIXME in TrieSubtrie class), use them
  // facilitate sparse iteration here.
  ఒకవేళ (Destructor)
    కొరకు (TrieSubtrie *Trie = Impl->get_మూలం(); Trie; Trie = Trie->తదుపరి_2.ఎక్కించు())
      కొరకు (గుర్తులేని I = 0; I < Trie->కొలత_2(); ++I)
        ఒకవేళ (స్వయంచాలక *Content = dyn_cast_or_null<TrieContent>(Trie->ఎక్కించు(I)))
          Destructor(Content->get_విలువ_సూచిక());

  // Destroy the subtries. Incidentally, this destroys them in the reverse order
  // of saving.
  TrieSubtrie *Trie = Impl->get_మూలం()->తదుపరి_2;
  అయితే (Trie) {
    TrieSubtrie *తదుపరి_2 = Trie->తదుపరి_2.exchange(శూన్యసూచిక);
    తొలగించు Trie;
    Trie = తదుపరి_2;
  }
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది
Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_మూలం() స్థిరరాశి {
  ImplType *Impl = ImplPtr.ఎక్కించు();
  ఒకవేళ (!Impl)
    తిరిగి సూచిక_పునాది();
  తిరిగి సూచిక_పునాది(Impl->get_మూలం());
}

గుర్తులేని Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_ప్రారంభం_బిట్టు(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది P) స్థిరరాశి {
  ప్రతిజ్ఞా(!P.isHint() && "Not a valid trie");
  ఒకవేళ (!P.P)
    తిరిగి 0;
  ఒకవేళ (స్వయంచాలక *S = dyn_cast<TrieSubtrie>((Trie_కణుపు *)P.P))
    తిరిగి S->ప్రారంభం_బిట్టు;
  తిరిగి 0;
}

గుర్తులేని Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_అంకె_బిట్లు(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది P) స్థిరరాశి {
  ప్రతిజ్ఞా(!P.isHint() && "Not a valid trie");
  ఒకవేళ (!P.P)
    తిరిగి 0;
  ఒకవేళ (స్వయంచాలక *S = dyn_cast<TrieSubtrie>((Trie_కణుపు *)P.P))
    తిరిగి S->అంకె_బిట్లు_2;
  తిరిగి 0;
}

గుర్తులేని Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_అంకె_స్థానం_వాడిన(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది P) స్థిరరాశి {
  ప్రతిజ్ఞా(!P.isHint() && "Not a valid trie");
  ఒకవేళ (!P.P)
    తిరిగి 0;
  స్వయంచాలక *S = dyn_cast<TrieSubtrie>((Trie_కణుపు *)P.P);
  ఒకవేళ (!S)
    తిరిగి 0;
  గుర్తులేని అంకె = 0;
  కొరకు (గుర్తులేని I = 0, E = S->కొలత_2(); I < E; ++I)
    ఒకవేళ (S->ఎక్కించు(I))
      ++అంకె;
  తిరిగి అంకె;
}

std::string Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_Trie_ఉపసర్గ_As_String(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది P) స్థిరరాశి {
  ప్రతిజ్ఞా(!P.isHint() && "Not a valid trie");
  ఒకవేళ (!P.P)
    తిరిగి "";

  స్వయంచాలక *S = dyn_cast<TrieSubtrie>((Trie_కణుపు *)P.P);
  ఒకవేళ (!S || !S->IsSubtrie)
    తిరిగి "";

  // Find a TrieContent node which has hash stored. Depth search following the
  // first used slot until a TrieContent node is found.
  TrieSubtrie *Current = S;
  TrieContent *కణుపు = శూన్యసూచిక;
  అయితే (Current) {
    TrieSubtrie *తదుపరి_2 = శూన్యసూచిక;
    // Find first used slot in the trie.
    కొరకు (గుర్తులేని I = 0, E = Current->కొలత_2(); I < E; ++I) {
      స్వయంచాలక *S = Current->ఎక్కించు(I);
      ఒకవేళ (!S)
        కొనసాగు;

      ఒకవేళ (స్వయంచాలక *Content = dyn_cast<TrieContent>(S))
        కణుపు = Content;
      లేకపోతే ఒకవేళ (స్వయంచాలక *Sub = dyn_cast<TrieSubtrie>(S))
        తదుపరి_2 = Sub;
      ఆపు;
    }

    // Found the node.
    ఒకవేళ (కణుపు)
      ఆపు;

    // Continue to the next level if the node is not found.
    Current = తదుపరి_2;
  }

  ప్రతిజ్ఞా(కణుపు && "malformed trie, cannot find TrieContent on leaf node");
  // The prefix for the current trie is the first `StartBit` of the content
  // stored underneath this subtrie.
  std::string Str;
  raw_string_ostream SS(Str);

  గుర్తులేని ప్రారంభం_నిండిన_బైట్లు = (S->ప్రారంభం_బిట్టు + 1) / 8 - 1;
  SS << గమ్యం_Hex(గమ్యం_String_Ref(కణుపు->getHash()).take_front(ప్రారంభం_నిండిన_బైట్లు),
              /*LowerCase=*/నిజం);

  // For the part of the prefix that doesn't fill a byte, print raw bit values.
  std::string బిట్లు_2;
  కొరకు (గుర్తులేని I = ప్రారంభం_నిండిన_బైట్లు * 8, E = S->ప్రారంభం_బిట్టు; I < E; ++I) {
    గుర్తులేని సూచీ_2 = I / 8;
    గుర్తులేని స్థానభ్రంశం = 7 - I % 8;
    బిట్లు_2.నెట్టు_వెనుక('0' + ((కణుపు->getHash()[సూచీ_2] >> స్థానభ్రంశం) & 1));
  }

  ఒకవేళ (!బిట్లు_2.ఖాళీలు())
    SS << "[" << బిట్లు_2 << "]";

  తిరిగి SS.str();
}

గుర్తులేని Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_అంకె_Tries() స్థిరరాశి {
  ImplType *Impl = ImplPtr.ఎక్కించు();
  ఒకవేళ (!Impl)
    తిరిగి 0;
  గుర్తులేని అంకె = 0;
  కొరకు (TrieSubtrie *Trie = Impl->get_మూలం(); Trie; Trie = Trie->తదుపరి_2.ఎక్కించు())
    ++అంకె;
  తిరిగి అంకె;
}

Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది
Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::get_తదుపరి_Trie(
    Thread_సురక్షిత_Trie_Raw_Hash_Map_పునాది::సూచిక_పునాది P) స్థిరరాశి {
  ప్రతిజ్ఞా(!P.isHint() && "Not a valid trie");
  ఒకవేళ (!P.P)
    తిరిగి సూచిక_పునాది();
  స్వయంచాలక *S = dyn_cast<TrieSubtrie>((Trie_కణుపు *)P.P);
  ఒకవేళ (!S)
    తిరిగి సూచిక_పునాది();
  ఒకవేళ (స్వయంచాలక *E = S->తదుపరి_2.ఎక్కించు())
    తిరిగి సూచిక_పునాది(E);
  తిరిగి సూచిక_పునాది();
}
