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

#समावेश "llvm/Support/OptimizedStructLayout.h"
#समावेश <optional>

using namespace llvm;

using Field = OptimizedStructLayoutField;

#अगर_अघोषित न_संशोधन
अटल व्योम checkValidLayout(ArrayRef<Field> Fields, uint64_t आकार,
                             Align MaxAlign) {
  uint64_t LastEnd = 0;
  Align ComputedMaxAlign;
  क्रम (स्वतः &Field : Fields) {
    निश्चित(Field.hasFixedOffset() &&
           "didn't assign a fixed offset to field");
    निश्चित(isAligned(Field.Alignment, Field.Offset) &&
           "didn't assign a correctly-aligned offset to field");
    निश्चित(Field.Offset >= LastEnd &&
           "didn't assign offsets in ascending order");
    LastEnd = Field.getEndOffset();
    निश्चित(Field.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(Field.Alignment, MaxAlign);
  }
  निश्चित(LastEnd == आकार && "didn't compute LastEnd correctly");
  निश्चित(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#पूर्ण_अगर

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<Field> Fields) {
#अगर_अघोषित न_संशोधन
  // Do some simple precondition checks.
  {
    bool InFixedPrefix = true;
    माप_प्रकार LastEnd = 0;
    क्रम (स्वतः &Field : Fields) {
      निश्चित(Field.आकार > 0 && "field of zero size");
      अगर (Field.hasFixedOffset()) {
        निश्चित(InFixedPrefix &&
               "fixed-offset fields are not a strict prefix of array");
        निश्चित(LastEnd <= Field.Offset &&
               "fixed-offset fields overlap or are not in order");
        LastEnd = Field.getEndOffset();
        निश्चित(LastEnd > Field.Offset &&
               "overflow in fixed-offset end offset");
      } अन्यथा {
        InFixedPrefix = false;
      }
    }
  }
#पूर्ण_अगर

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  स्वतः FirstFlexible = Fields.begin(), E = Fields.end();
  जबतक (FirstFlexible != E && FirstFlexible->hasFixedOffset()) {
    MaxAlign = std::max(MaxAlign, FirstFlexible->Alignment);
    ++FirstFlexible;
  }

  // If there are no flexible fields, we're done.
  अगर (FirstFlexible == E) {
    uint64_t आकार = 0;
    अगर (!Fields.empty())
      आकार = Fields.back().getEndOffset();

#अगर_अघोषित न_संशोधन
    checkValidLayout(Fields, आकार, MaxAlign);
#पूर्ण_अगर
    वापस {आकार, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_संख्या = 0;
    क्रम (स्वतः I = FirstFlexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<व्योम*>(Unique_संख्या++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_sort(FirstFlexible, E,
                 [](स्थिर Field *lhs, स्थिर Field *rhs) -> पूर्णांक {
    // Decreasing alignment.
    अगर (lhs->Alignment != rhs->Alignment)
      वापस (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    अगर (lhs->आकार != rhs->आकार)
      वापस (lhs->आकार < rhs->आकार ? 1 : -1);

    // Original order.
    स्वतः lhs_संख्या = reinterpret_cast<uintptr_t>(lhs->Scratch);
    स्वतः rhs_संख्या = reinterpret_cast<uintptr_t>(rhs->Scratch);
    अगर (lhs_संख्या != rhs_संख्या)
      वापस (lhs_संख्या < rhs_संख्या ? -1 : 1);

    वापस 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    bool HasPadding = false;
    uint64_t LastEnd = 0;

    // Walk the fixed-offset fields.
    क्रम (स्वतः I = Fields.begin(); I != FirstFlexible; ++I) {
      निश्चित(I->hasFixedOffset());
      अगर (LastEnd != I->Offset) {
        HasPadding = true;
        अवरोध;
      }
      LastEnd = I->getEndOffset();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    अगर (!HasPadding) {
      क्रम (स्वतः I = FirstFlexible; I != E; ++I) {
        स्वतः Offset = alignTo(LastEnd, I->Alignment);
        अगर (LastEnd != Offset) {
          HasPadding = true;
          अवरोध;
        }
        I->Offset = Offset;
        LastEnd = I->getEndOffset();
      }
    }

    // If we already have a perfect layout, we're done.
    अगर (!HasPadding) {
#अगर_अघोषित न_संशोधन
      checkValidLayout(Fields, LastEnd, MaxAlign);
#पूर्ण_अगर
      वापस {LastEnd, MaxAlign};
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
  काष्ठा AlignmentQueue {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_आकार;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    OptimizedStructLayoutField *शीर्ष_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    अटल Field *get_अग्र(Field *Cur) {
      वापस static_cast<Field *>(Cur->Scratch);
    }
  };
  SmallVector<AlignmentQueue, 8> FlexibleFieldsByAlignment;
  क्रम (स्वतः I = FirstFlexible; I != E; ) {
    स्वतः शीर्ष_2 = I;
    स्वतः Alignment = I->Alignment;

    uint64_t Min_आकार = I->आकार;
    स्वतः LastInQueue = I;
    क्रम (++I; I != E && I->Alignment == Alignment; ++I) {
      LastInQueue->Scratch = I;
      LastInQueue = I;
      Min_आकार = std::min(Min_आकार, I->आकार);
    }
    LastInQueue->Scratch = nullptr;

    FlexibleFieldsByAlignment.धकेलो_back({Min_आकार, शीर्ष_2, Alignment});
  }

#अगर_अघोषित न_संशोधन
  // Verify that we set the queues up correctly.
  स्वतः checkQueues = [&]{
    bool FirstQueue = true;
    Align LastQueueAlignment;
    क्रम (स्वतः &Queue : FlexibleFieldsByAlignment) {
      निश्चित((FirstQueue || Queue.Alignment < LastQueueAlignment) &&
             "bins not in order of descending alignment");
      LastQueueAlignment = Queue.Alignment;
      FirstQueue = false;

      निश्चित(Queue.शीर्ष_2 && "queue was empty");
      uint64_t Last_आकार = ~(uint64_t)0;
      क्रम (स्वतः I = Queue.शीर्ष_2; I; I = Queue.get_अग्र(I)) {
        निश्चित(I->Alignment == Queue.Alignment && "bad field in queue");
        निश्चित(I->आकार <= Last_आकार && "queue not in descending size order");
        Last_आकार = I->आकार;
      }
    }
  };
  checkQueues();
#पूर्ण_अगर

  /// Helper function to remove a field from a queue.
  स्वतः spliceFromQueue = [&](AlignmentQueue *Queue, Field *Last, Field *Cur) {
    निश्चित(Last ? Queue->get_अग्र(Last) == Cur : Queue->शीर्ष_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    अगर (Last) {
      Last->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      अगर (!Cur->Scratch)
        Queue->Min_आकार = Last->आकार;

    // Otherwise, replace the head.
    } अन्यथा {
      अगर (स्वतः New_शीर्ष = Queue->get_अग्र(Cur))
        Queue->शीर्ष_2 = New_शीर्ष;

      // If we just emptied the queue, destroy its bin.
      अन्यथा
        FlexibleFieldsByAlignment.erase(Queue);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  SmallVector<Field, 16> Layout;
  Layout.reserve(Fields.आकार_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t LastEnd = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  स्वतः addToLayout = [&](AlignmentQueue *Queue, Field *Last, Field *Cur,
                         uint64_t Offset) -> bool {
    निश्चित(Offset == alignTo(LastEnd, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    spliceFromQueue(Queue, Last, Cur);

    // Add Cur to the layout.
    Layout.धकेलो_back(*Cur);
    Layout.back().Offset = Offset;
    LastEnd = Layout.back().getEndOffset();

    // Always return true so that we can be tail-called.
    वापस true;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  स्वतः tryAddFillerFromQueue = [&](AlignmentQueue *Queue, uint64_t StartOffset,
                                   std::optional<uint64_t> EndOffset) -> bool {
    निश्चित(Queue->शीर्ष_2);
    निश्चित(StartOffset == alignTo(LastEnd, Queue->Alignment));
    निश्चित(!EndOffset || StartOffset < *EndOffset);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    स्वतः Max_Viable_आकार =
      (EndOffset ? *EndOffset - StartOffset : ~(uint64_t)0);
    अगर (Queue->Min_आकार > Max_Viable_आकार)
      वापस false;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    क्रम (Field *Cur = Queue->शीर्ष_2, *Last = nullptr; true;
           Last = Cur, Cur = Queue->get_अग्र(Cur)) {
      निश्चित(Cur && "didn't find a match in queue despite its MinSize");
      अगर (Cur->आकार <= Max_Viable_आकार)
        वापस addToLayout(Queue, Last, Cur, StartOffset);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  स्वतः tryAddBestField = [&](std::optional<uint64_t> BeforeOffset) -> bool {
    निश्चित(!BeforeOffset || LastEnd < *BeforeOffset);
    स्वतः QueueB = FlexibleFieldsByAlignment.begin();
    स्वतः QueueE = FlexibleFieldsByAlignment.end();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    स्वतः First_Queue_To_खोज = QueueB;
    क्रम (; First_Queue_To_खोज != QueueE; ++First_Queue_To_खोज) {
      अगर (isAligned(First_Queue_To_खोज->Alignment, LastEnd))
        अवरोध;
    }

    uint64_t Offset = LastEnd;
    जबतक (true) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      क्रम (स्वतः Queue = First_Queue_To_खोज; Queue != QueueE; ++Queue) {
        अगर (tryAddFillerFromQueue(Queue, Offset, BeforeOffset))
          वापस true;
      }

      // Okay, we don't need to scan those again.
      QueueE = First_Queue_To_खोज;

      // If we started from the first queue, we're done.
      अगर (First_Queue_To_खोज == QueueB)
        वापस false;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --First_Queue_To_खोज;
      Offset = alignTo(LastEnd, First_Queue_To_खोज->Alignment);
      अगर (BeforeOffset && Offset >= *BeforeOffset)
        वापस false;
      जबतक (First_Queue_To_खोज != QueueB &&
             Offset == alignTo(LastEnd, First_Queue_To_खोज[-1].Alignment))
        --First_Queue_To_खोज;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  क्रम (स्वतः I = Fields.begin(); I != FirstFlexible; ++I) {
    निश्चित(LastEnd <= I->Offset);
    जबतक (LastEnd != I->Offset) {
      अगर (!tryAddBestField(I->Offset))
        अवरोध;
    }
    Layout.धकेलो_back(*I);
    LastEnd = I->getEndOffset();
  }

#अगर_अघोषित न_संशोधन
  checkQueues();
#पूर्ण_अगर

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  जबतक (!FlexibleFieldsByAlignment.empty()) {
    bool Success = tryAddBestField(std::nullopt);
    निश्चित(Success && "didn't find a field with no fixed limit?");
    (व्योम) Success;
  }

  // Copy the layout back into place.
  निश्चित(Layout.आकार_2() == Fields.आकार_2());
  स_नकल(Fields.तथ्य(), Layout.तथ्य(),
         Fields.आकार_2() * माप(OptimizedStructLayoutField));

#अगर_अघोषित न_संशोधन
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, LastEnd, MaxAlign);
#पूर्ण_अगर

  वापस {LastEnd, MaxAlign};
}
