//===--- OptimizedStructLayout.cpp - Optimal data layout algorithm ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the performOptimizedStructLayout interface.
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/OptimizedStructLayout.h"
#அந்தர்பாவ <optional>

using நாமாகாஶ llvm;

using Field = OptimizedStructLayoutField;

#ifndef NDEBUG
நிலையான வெறுமை checkValidLayout(ArrayRef<Field> Fields, uint64_t Size,
                             Align MaxAlign) {
  uint64_t LastEnd = 0;
  Align ComputedMaxAlign;
  க்கு (auto &Field : Fields) {
    ப்ரதிஜ்ஞா(Field.hasFixedOffset() &&
           "didn't assign a fixed offset to field");
    ப்ரதிஜ்ஞா(isAligned(Field.Alignment, Field.Offset) &&
           "didn't assign a correctly-aligned offset to field");
    ப்ரதிஜ்ஞா(Field.Offset >= LastEnd &&
           "didn't assign offsets in ascending order");
    LastEnd = Field.getEndOffset();
    ப்ரதிஜ்ஞா(Field.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(Field.Alignment, MaxAlign);
  }
  ப்ரதிஜ்ஞா(LastEnd == Size && "didn't compute LastEnd correctly");
  ப்ரதிஜ்ஞா(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#endif

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<Field> Fields) {
#ifndef NDEBUG
  // Do some simple precondition checks.
  {
    தருக்க InFixedPrefix = உண்மை;
    size_t LastEnd = 0;
    க்கு (auto &Field : Fields) {
      ப்ரதிஜ்ஞா(Field.Size > 0 && "field of zero size");
      எனில் (Field.hasFixedOffset()) {
        ப்ரதிஜ்ஞா(InFixedPrefix &&
               "fixed-offset fields are not a strict prefix of array");
        ப்ரதிஜ்ஞா(LastEnd <= Field.Offset &&
               "fixed-offset fields overlap or are not in order");
        LastEnd = Field.getEndOffset();
        ப்ரதிஜ்ஞா(LastEnd > Field.Offset &&
               "overflow in fixed-offset end offset");
      } இல்லையெனில் {
        InFixedPrefix = பொய்;
      }
    }
  }
#endif

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  auto FirstFlexible = Fields.begin(), E = Fields.end();
  போது (FirstFlexible != E && FirstFlexible->hasFixedOffset()) {
    MaxAlign = std::max(MaxAlign, FirstFlexible->Alignment);
    ++FirstFlexible;
  }

  // If there are no flexible fields, we're done.
  எனில் (FirstFlexible == E) {
    uint64_t Size = 0;
    எனில் (!Fields.empty())
      Size = Fields.back().getEndOffset();

#ifndef NDEBUG
    checkValidLayout(Fields, Size, MaxAlign);
#endif
    திரும்பு {Size, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_எண் = 0;
    க்கு (auto I = FirstFlexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<வெறுமை*>(Unique_எண்++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_sort(FirstFlexible, E,
                 [](மாறிலி Field *lhs, மாறிலி Field *rhs) -> முழு {
    // Decreasing alignment.
    எனில் (lhs->Alignment != rhs->Alignment)
      திரும்பு (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    எனில் (lhs->Size != rhs->Size)
      திரும்பு (lhs->Size < rhs->Size ? 1 : -1);

    // Original order.
    auto lhs_எண் = reinterpret_cast<uintptr_t>(lhs->Scratch);
    auto rhs_எண் = reinterpret_cast<uintptr_t>(rhs->Scratch);
    எனில் (lhs_எண் != rhs_எண்)
      திரும்பு (lhs_எண் < rhs_எண் ? -1 : 1);

    திரும்பு 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    தருக்க HasPadding = பொய்;
    uint64_t LastEnd = 0;

    // Walk the fixed-offset fields.
    க்கு (auto I = Fields.begin(); I != FirstFlexible; ++I) {
      ப்ரதிஜ்ஞா(I->hasFixedOffset());
      எனில் (LastEnd != I->Offset) {
        HasPadding = உண்மை;
        நிறுத்து;
      }
      LastEnd = I->getEndOffset();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    எனில் (!HasPadding) {
      க்கு (auto I = FirstFlexible; I != E; ++I) {
        auto Offset = alignTo(LastEnd, I->Alignment);
        எனில் (LastEnd != Offset) {
          HasPadding = உண்மை;
          நிறுத்து;
        }
        I->Offset = Offset;
        LastEnd = I->getEndOffset();
      }
    }

    // If we already have a perfect layout, we're done.
    எனில் (!HasPadding) {
#ifndef NDEBUG
      checkValidLayout(Fields, LastEnd, MaxAlign);
#endif
      திரும்பு {LastEnd, MaxAlign};
    }
  }

  // The algorithm sketch at this point is as follows.
  //
  // Consider the padding gaps between fixed-offset fields in ascending
  // order.  Let LastEnd be the offset of the first byte following the
  // field before the gap, or 0 if the gap is at the beginning of the
  // structure.  Find the "best" flexible-offset field according to the
  // criteria below.  If no such field exists, proceed to the next gap.
  // Otherwise, add the field at the first properly-aligned offset for
  // that field that is >= LastEnd, then update LastEnd and repeat in
  // order to fill any remaining gap following that field.
  //
  // Next, let LastEnd to be the offset of the first byte following the
  // last fixed-offset field, or 0 if there are no fixed-offset fields.
  // While there are flexible-offset fields remaining, find the "best"
  // flexible-offset field according to the criteria below, add it at
  // the first properly-aligned offset for that field that is >= LastEnd,
  // and update LastEnd to the first byte following the field.
  //
  // The "best" field is chosen by the following criteria, considered
  // strictly in order:
  //
  // - When filling a gap betweeen fields, the field must fit.
  // - A field is preferred if it requires less padding following LastEnd.
  // - A field is preferred if it is more aligned.
  // - A field is preferred if it is larger.
  // - A field is preferred if it appeared earlier in the initial order.
  //
  // Minimizing leading padding is a greedy attempt to avoid padding
  // entirely.  Preferring more-aligned fields is an attempt to eliminate
  // stricter constraints earlier, with the idea that weaker alignment
  // constraints may be resolvable with less padding elsewhere.  These
  // These two rules are sufficient to ensure that we get the optimal
  // layout in the "C-style" case.  Preferring larger fields tends to take
  // better advantage of large gaps and may be more likely to have a size
  // that's a multiple of a useful alignment.  Preferring the initial
  // order may help somewhat with locality but is mostly just a way of
  // ensuring deterministic output.
  //
  // Note that this algorithm does not guarantee a minimal layout.  Picking
  // a larger object greedily may leave a gap that cannot be filled as
  // efficiently.  Unfortunately, solving this perfectly is an NP-complete
  // problem (by reduction from bin-packing: let B_i be the bin sizes and
  // O_j be the object sizes; add fixed-offset fields such that the gaps
  // between them have size B_i, and add flexible-offset fields with
  // alignment 1 and size O_j; if the layout size is equal to the end of
  // the last fixed-layout field, the objects fit in the bins; note that
  // this doesn't even require the complexity of alignment).

  // The implementation below is essentially just an optimized version of
  // scanning the list of remaining fields looking for the best, which
  // would be O(n^2).  In the worst case, it doesn't improve on that.
  // However, in practice it'll just scan the array of alignment bins
  // and consider the first few elements from one or two bins.  The
  // number of bins is bounded by a small constant: alignments are powers
  // of two that are vanishingly unlikely to be over 64 and fairly unlikely
  // to be over 8.  And multiple elements only need to be considered when
  // filling a gap between fixed-offset fields, which doesn't happen very
  // often.  We could use a data structure within bins that optimizes for
  // finding the best-sized match, but it would require allocating memory
  // and copying data, so it's unlikely to be worthwhile.


  // Start by organizing the flexible-offset fields into bins according to
  // their alignment.  We expect a small enough number of bins that we
  // don't care about the asymptotic costs of walking this.
  கட்டமைப்பு AlignmentQueue {
    /// The minimum size of anything currently in this queue.
    uint64_t MinSize;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    OptimizedStructLayoutField *Head;

    /// The alignment requirement of the queue.
    Align Alignment;

    நிலையான Field *getNext(Field *Cur) {
      திரும்பு static_cast<Field *>(Cur->Scratch);
    }
  };
  SmallVector<AlignmentQueue, 8> FlexibleFieldsByAlignment;
  க்கு (auto I = FirstFlexible; I != E; ) {
    auto Head = I;
    auto Alignment = I->Alignment;

    uint64_t MinSize = I->Size;
    auto LastInQueue = I;
    க்கு (++I; I != E && I->Alignment == Alignment; ++I) {
      LastInQueue->Scratch = I;
      LastInQueue = I;
      MinSize = std::min(MinSize, I->Size);
    }
    LastInQueue->Scratch = nullptr;

    FlexibleFieldsByAlignment.தள்ளு_back({MinSize, Head, Alignment});
  }

#ifndef NDEBUG
  // Verify that we set the queues up correctly.
  auto checkQueues = [&]{
    தருக்க FirstQueue = உண்மை;
    Align LastQueueAlignment;
    க்கு (auto &Queue : FlexibleFieldsByAlignment) {
      ப்ரதிஜ்ஞா((FirstQueue || Queue.Alignment < LastQueueAlignment) &&
             "bins not in order of descending alignment");
      LastQueueAlignment = Queue.Alignment;
      FirstQueue = பொய்;

      ப்ரதிஜ்ஞா(Queue.Head && "queue was empty");
      uint64_t LastSize = ~(uint64_t)0;
      க்கு (auto I = Queue.Head; I; I = Queue.getNext(I)) {
        ப்ரதிஜ்ஞா(I->Alignment == Queue.Alignment && "bad field in queue");
        ப்ரதிஜ்ஞா(I->Size <= LastSize && "queue not in descending size order");
        LastSize = I->Size;
      }
    }
  };
  checkQueues();
#endif

  /// Helper function to remove a field from a queue.
  auto spliceFromQueue = [&](AlignmentQueue *Queue, Field *Last, Field *Cur) {
    ப்ரதிஜ்ஞா(Last ? Queue->getNext(Last) == Cur : Queue->Head == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    எனில் (Last) {
      Last->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      எனில் (!Cur->Scratch)
        Queue->MinSize = Last->Size;

    // Otherwise, replace the head.
    } இல்லையெனில் {
      எனில் (auto NewHead = Queue->getNext(Cur))
        Queue->Head = NewHead;

      // If we just emptied the queue, destroy its bin.
      இல்லையெனில்
        FlexibleFieldsByAlignment.erase(Queue);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  SmallVector<Field, 16> Layout;
  Layout.reserve(Fields.size());

  // The offset that we're currently looking to insert at (or after).
  uint64_t LastEnd = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  auto addToLayout = [&](AlignmentQueue *Queue, Field *Last, Field *Cur,
                         uint64_t Offset) -> தருக்க {
    ப்ரதிஜ்ஞா(Offset == alignTo(LastEnd, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    spliceFromQueue(Queue, Last, Cur);

    // Add Cur to the layout.
    Layout.தள்ளு_back(*Cur);
    Layout.back().Offset = Offset;
    LastEnd = Layout.back().getEndOffset();

    // Always return true so that we can be tail-called.
    திரும்பு உண்மை;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  auto tryAddFillerFromQueue = [&](AlignmentQueue *Queue, uint64_t StartOffset,
                                   std::optional<uint64_t> EndOffset) -> தருக்க {
    ப்ரதிஜ்ஞா(Queue->Head);
    ப்ரதிஜ்ஞா(StartOffset == alignTo(LastEnd, Queue->Alignment));
    ப்ரதிஜ்ஞா(!EndOffset || StartOffset < *EndOffset);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    auto MaxViableSize =
      (EndOffset ? *EndOffset - StartOffset : ~(uint64_t)0);
    எனில் (Queue->MinSize > MaxViableSize)
      திரும்பு பொய்;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    க்கு (Field *Cur = Queue->Head, *Last = nullptr; உண்மை;
           Last = Cur, Cur = Queue->getNext(Cur)) {
      ப்ரதிஜ்ஞா(Cur && "didn't find a match in queue despite its MinSize");
      எனில் (Cur->Size <= MaxViableSize)
        திரும்பு addToLayout(Queue, Last, Cur, StartOffset);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  auto tryAddBestField = [&](std::optional<uint64_t> BeforeOffset) -> தருக்க {
    ப்ரதிஜ்ஞா(!BeforeOffset || LastEnd < *BeforeOffset);
    auto QueueB = FlexibleFieldsByAlignment.begin();
    auto QueueE = FlexibleFieldsByAlignment.end();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    auto First_Queue_To_தேடல் = QueueB;
    க்கு (; First_Queue_To_தேடல் != QueueE; ++First_Queue_To_தேடல்) {
      எனில் (isAligned(First_Queue_To_தேடல்->Alignment, LastEnd))
        நிறுத்து;
    }

    uint64_t Offset = LastEnd;
    போது (உண்மை) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      க்கு (auto Queue = First_Queue_To_தேடல்; Queue != QueueE; ++Queue) {
        எனில் (tryAddFillerFromQueue(Queue, Offset, BeforeOffset))
          திரும்பு உண்மை;
      }

      // Okay, we don't need to scan those again.
      QueueE = First_Queue_To_தேடல்;

      // If we started from the first queue, we're done.
      எனில் (First_Queue_To_தேடல் == QueueB)
        திரும்பு பொய்;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --First_Queue_To_தேடல்;
      Offset = alignTo(LastEnd, First_Queue_To_தேடல்->Alignment);
      எனில் (BeforeOffset && Offset >= *BeforeOffset)
        திரும்பு பொய்;
      போது (First_Queue_To_தேடல் != QueueB &&
             Offset == alignTo(LastEnd, First_Queue_To_தேடல்[-1].Alignment))
        --First_Queue_To_தேடல்;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  க்கு (auto I = Fields.begin(); I != FirstFlexible; ++I) {
    ப்ரதிஜ்ஞா(LastEnd <= I->Offset);
    போது (LastEnd != I->Offset) {
      எனில் (!tryAddBestField(I->Offset))
        நிறுத்து;
    }
    Layout.தள்ளு_back(*I);
    LastEnd = I->getEndOffset();
  }

#ifndef NDEBUG
  checkQueues();
#endif

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  போது (!FlexibleFieldsByAlignment.empty()) {
    தருக்க Success = tryAddBestField(std::nullopt);
    ப்ரதிஜ்ஞா(Success && "didn't find a field with no fixed limit?");
    (வெறுமை) Success;
  }

  // Copy the layout back into place.
  ப்ரதிஜ்ஞா(Layout.size() == Fields.size());
  memcpy(Fields.data(), Layout.data(),
         Fields.size() * அளவு(OptimizedStructLayoutField));

#ifndef NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, LastEnd, MaxAlign);
#endif

  திரும்பு {LastEnd, MaxAlign};
}
