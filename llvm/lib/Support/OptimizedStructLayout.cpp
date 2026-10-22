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

#অন্তর্ভাব "llvm/Support/OptimizedStructLayout.h"
#অন্তর্ভাব <optional>

using নামাকাশ llvm;

using Field = OptimizedStructLayoutField;

#ifndef NDEBUG
স্থির শূন্য checkValidLayout(ArrayRef<Field> Fields, uint64_t আকার_2,
                             Align MaxAlign) {
  uint64_t LastEnd = 0;
  Align ComputedMaxAlign;
  জন্য (auto &Field : Fields) {
    প্রতিজ্ঞা(Field.hasFixedOffset() &&
           "didn't assign a fixed offset to field");
    প্রতিজ্ঞা(isAligned(Field.Alignment, Field.Offset) &&
           "didn't assign a correctly-aligned offset to field");
    প্রতিজ্ঞা(Field.Offset >= LastEnd &&
           "didn't assign offsets in ascending order");
    LastEnd = Field.getEndOffset();
    প্রতিজ্ঞা(Field.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(Field.Alignment, MaxAlign);
  }
  প্রতিজ্ঞা(LastEnd == আকার_2 && "didn't compute LastEnd correctly");
  প্রতিজ্ঞা(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#endif

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<Field> Fields) {
#ifndef NDEBUG
  // Do some simple precondition checks.
  {
    যৌক্তিক InFixedPrefix = সত্য;
    size_t LastEnd = 0;
    জন্য (auto &Field : Fields) {
      প্রতিজ্ঞা(Field.আকার_2 > 0 && "field of zero size");
      যদি (Field.hasFixedOffset()) {
        প্রতিজ্ঞা(InFixedPrefix &&
               "fixed-offset fields are not a strict prefix of array");
        প্রতিজ্ঞা(LastEnd <= Field.Offset &&
               "fixed-offset fields overlap or are not in order");
        LastEnd = Field.getEndOffset();
        প্রতিজ্ঞা(LastEnd > Field.Offset &&
               "overflow in fixed-offset end offset");
      } অন্যথা {
        InFixedPrefix = মিথ্যা;
      }
    }
  }
#endif

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  auto FirstFlexible = Fields.begin(), E = Fields.end();
  যখন (FirstFlexible != E && FirstFlexible->hasFixedOffset()) {
    MaxAlign = std::max(MaxAlign, FirstFlexible->Alignment);
    ++FirstFlexible;
  }

  // If there are no flexible fields, we're done.
  যদি (FirstFlexible == E) {
    uint64_t আকার_2 = 0;
    যদি (!Fields.empty())
      আকার_2 = Fields.back().getEndOffset();

#ifndef NDEBUG
    checkValidLayout(Fields, আকার_2, MaxAlign);
#endif
    ফেরত {আকার_2, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_সংখ্যা = 0;
    জন্য (auto I = FirstFlexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<শূন্য*>(Unique_সংখ্যা++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_sort(FirstFlexible, E,
                 [](ধ্রুবক Field *lhs, ধ্রুবক Field *rhs) -> পূর্ণসংখ্যা {
    // Decreasing alignment.
    যদি (lhs->Alignment != rhs->Alignment)
      ফেরত (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    যদি (lhs->আকার_2 != rhs->আকার_2)
      ফেরত (lhs->আকার_2 < rhs->আকার_2 ? 1 : -1);

    // Original order.
    auto lhs_সংখ্যা = reinterpret_cast<uintptr_t>(lhs->Scratch);
    auto rhs_সংখ্যা = reinterpret_cast<uintptr_t>(rhs->Scratch);
    যদি (lhs_সংখ্যা != rhs_সংখ্যা)
      ফেরত (lhs_সংখ্যা < rhs_সংখ্যা ? -1 : 1);

    ফেরত 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    যৌক্তিক HasPadding = মিথ্যা;
    uint64_t LastEnd = 0;

    // Walk the fixed-offset fields.
    জন্য (auto I = Fields.begin(); I != FirstFlexible; ++I) {
      প্রতিজ্ঞা(I->hasFixedOffset());
      যদি (LastEnd != I->Offset) {
        HasPadding = সত্য;
        বিরতি;
      }
      LastEnd = I->getEndOffset();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    যদি (!HasPadding) {
      জন্য (auto I = FirstFlexible; I != E; ++I) {
        auto Offset = alignTo(LastEnd, I->Alignment);
        যদি (LastEnd != Offset) {
          HasPadding = সত্য;
          বিরতি;
        }
        I->Offset = Offset;
        LastEnd = I->getEndOffset();
      }
    }

    // If we already have a perfect layout, we're done.
    যদি (!HasPadding) {
#ifndef NDEBUG
      checkValidLayout(Fields, LastEnd, MaxAlign);
#endif
      ফেরত {LastEnd, MaxAlign};
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
  কাঠামো AlignmentQueue {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_আকার;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    OptimizedStructLayoutField *শীর্ষ_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    স্থির Field *get_পরবর্তী(Field *Cur) {
      ফেরত static_cast<Field *>(Cur->Scratch);
    }
  };
  SmallVector<AlignmentQueue, 8> FlexibleFieldsByAlignment;
  জন্য (auto I = FirstFlexible; I != E; ) {
    auto শীর্ষ_2 = I;
    auto Alignment = I->Alignment;

    uint64_t Min_আকার = I->আকার_2;
    auto LastInQueue = I;
    জন্য (++I; I != E && I->Alignment == Alignment; ++I) {
      LastInQueue->Scratch = I;
      LastInQueue = I;
      Min_আকার = std::min(Min_আকার, I->আকার_2);
    }
    LastInQueue->Scratch = nullptr;

    FlexibleFieldsByAlignment.ঠেলো_back({Min_আকার, শীর্ষ_2, Alignment});
  }

#ifndef NDEBUG
  // Verify that we set the queues up correctly.
  auto checkQueues = [&]{
    যৌক্তিক FirstQueue = সত্য;
    Align LastQueueAlignment;
    জন্য (auto &Queue : FlexibleFieldsByAlignment) {
      প্রতিজ্ঞা((FirstQueue || Queue.Alignment < LastQueueAlignment) &&
             "bins not in order of descending alignment");
      LastQueueAlignment = Queue.Alignment;
      FirstQueue = মিথ্যা;

      প্রতিজ্ঞা(Queue.শীর্ষ_2 && "queue was empty");
      uint64_t Last_আকার = ~(uint64_t)0;
      জন্য (auto I = Queue.শীর্ষ_2; I; I = Queue.get_পরবর্তী(I)) {
        প্রতিজ্ঞা(I->Alignment == Queue.Alignment && "bad field in queue");
        প্রতিজ্ঞা(I->আকার_2 <= Last_আকার && "queue not in descending size order");
        Last_আকার = I->আকার_2;
      }
    }
  };
  checkQueues();
#endif

  /// Helper function to remove a field from a queue.
  auto spliceFromQueue = [&](AlignmentQueue *Queue, Field *Last, Field *Cur) {
    প্রতিজ্ঞা(Last ? Queue->get_পরবর্তী(Last) == Cur : Queue->শীর্ষ_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    যদি (Last) {
      Last->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      যদি (!Cur->Scratch)
        Queue->Min_আকার = Last->আকার_2;

    // Otherwise, replace the head.
    } অন্যথা {
      যদি (auto New_শীর্ষ = Queue->get_পরবর্তী(Cur))
        Queue->শীর্ষ_2 = New_শীর্ষ;

      // If we just emptied the queue, destroy its bin.
      অন্যথা
        FlexibleFieldsByAlignment.erase(Queue);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  SmallVector<Field, 16> Layout;
  Layout.reserve(Fields.আকার_3());

  // The offset that we're currently looking to insert at (or after).
  uint64_t LastEnd = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  auto addToLayout = [&](AlignmentQueue *Queue, Field *Last, Field *Cur,
                         uint64_t Offset) -> যৌক্তিক {
    প্রতিজ্ঞা(Offset == alignTo(LastEnd, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    spliceFromQueue(Queue, Last, Cur);

    // Add Cur to the layout.
    Layout.ঠেলো_back(*Cur);
    Layout.back().Offset = Offset;
    LastEnd = Layout.back().getEndOffset();

    // Always return true so that we can be tail-called.
    ফেরত সত্য;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  auto tryAddFillerFromQueue = [&](AlignmentQueue *Queue, uint64_t StartOffset,
                                   std::optional<uint64_t> EndOffset) -> যৌক্তিক {
    প্রতিজ্ঞা(Queue->শীর্ষ_2);
    প্রতিজ্ঞা(StartOffset == alignTo(LastEnd, Queue->Alignment));
    প্রতিজ্ঞা(!EndOffset || StartOffset < *EndOffset);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    auto Max_Viable_আকার =
      (EndOffset ? *EndOffset - StartOffset : ~(uint64_t)0);
    যদি (Queue->Min_আকার > Max_Viable_আকার)
      ফেরত মিথ্যা;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    জন্য (Field *Cur = Queue->শীর্ষ_2, *Last = nullptr; সত্য;
           Last = Cur, Cur = Queue->get_পরবর্তী(Cur)) {
      প্রতিজ্ঞা(Cur && "didn't find a match in queue despite its MinSize");
      যদি (Cur->আকার_2 <= Max_Viable_আকার)
        ফেরত addToLayout(Queue, Last, Cur, StartOffset);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  auto tryAddBestField = [&](std::optional<uint64_t> BeforeOffset) -> যৌক্তিক {
    প্রতিজ্ঞা(!BeforeOffset || LastEnd < *BeforeOffset);
    auto QueueB = FlexibleFieldsByAlignment.begin();
    auto QueueE = FlexibleFieldsByAlignment.end();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    auto First_Queue_To_অনুসন্ধান = QueueB;
    জন্য (; First_Queue_To_অনুসন্ধান != QueueE; ++First_Queue_To_অনুসন্ধান) {
      যদি (isAligned(First_Queue_To_অনুসন্ধান->Alignment, LastEnd))
        বিরতি;
    }

    uint64_t Offset = LastEnd;
    যখন (সত্য) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      জন্য (auto Queue = First_Queue_To_অনুসন্ধান; Queue != QueueE; ++Queue) {
        যদি (tryAddFillerFromQueue(Queue, Offset, BeforeOffset))
          ফেরত সত্য;
      }

      // Okay, we don't need to scan those again.
      QueueE = First_Queue_To_অনুসন্ধান;

      // If we started from the first queue, we're done.
      যদি (First_Queue_To_অনুসন্ধান == QueueB)
        ফেরত মিথ্যা;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --First_Queue_To_অনুসন্ধান;
      Offset = alignTo(LastEnd, First_Queue_To_অনুসন্ধান->Alignment);
      যদি (BeforeOffset && Offset >= *BeforeOffset)
        ফেরত মিথ্যা;
      যখন (First_Queue_To_অনুসন্ধান != QueueB &&
             Offset == alignTo(LastEnd, First_Queue_To_অনুসন্ধান[-1].Alignment))
        --First_Queue_To_অনুসন্ধান;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  জন্য (auto I = Fields.begin(); I != FirstFlexible; ++I) {
    প্রতিজ্ঞা(LastEnd <= I->Offset);
    যখন (LastEnd != I->Offset) {
      যদি (!tryAddBestField(I->Offset))
        বিরতি;
    }
    Layout.ঠেলো_back(*I);
    LastEnd = I->getEndOffset();
  }

#ifndef NDEBUG
  checkQueues();
#endif

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  যখন (!FlexibleFieldsByAlignment.empty()) {
    যৌক্তিক Success = tryAddBestField(std::nullopt);
    প্রতিজ্ঞা(Success && "didn't find a field with no fixed limit?");
    (শূন্য) Success;
  }

  // Copy the layout back into place.
  প্রতিজ্ঞা(Layout.আকার_3() == Fields.আকার_3());
  memcpy(Fields.তথ্য(), Layout.তথ্য(),
         Fields.আকার_3() * আকার(OptimizedStructLayoutField));

#ifndef NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, LastEnd, MaxAlign);
#endif

  ফেরত {LastEnd, MaxAlign};
}
