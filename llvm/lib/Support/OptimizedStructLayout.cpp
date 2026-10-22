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

#ಅನ್ತರ್ಭಾವ "llvm/Support/OptimizedStructLayout.h"
#ಅನ್ತರ್ಭಾವ <optional>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಬಳಸಿ ಕ್ಷೇತ್ರ_2 = Optimized_Struct_Layout_ಕ್ಷೇತ್ರ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಸ್ಥಿರ ಶೂನ್ಯ checkValidLayout(ArrayRef<ಕ್ಷೇತ್ರ_2> Fields, uint64_t ಗಾತ್ರ_2,
                             Align MaxAlign) {
  uint64_t ಕೊನೆಯ_ಕೊನೆ = 0;
  Align ComputedMaxAlign;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಕ್ಷೇತ್ರ_2 : Fields) {
    ಪ್ರತಿಜ್ಞಾ(ಕ್ಷೇತ್ರ_2.has_Fixed_ಸ್ಥಾನಾಂತರ() &&
           "didn't assign a fixed offset to field");
    ಪ್ರತಿಜ್ಞಾ(isAligned(ಕ್ಷೇತ್ರ_2.Alignment, ಕ್ಷೇತ್ರ_2.ಸ್ಥಾನಾಂತರ) &&
           "didn't assign a correctly-aligned offset to field");
    ಪ್ರತಿಜ್ಞಾ(ಕ್ಷೇತ್ರ_2.ಸ್ಥಾನಾಂತರ >= ಕೊನೆಯ_ಕೊನೆ &&
           "didn't assign offsets in ascending order");
    ಕೊನೆಯ_ಕೊನೆ = ಕ್ಷೇತ್ರ_2.get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();
    ಪ್ರತಿಜ್ಞಾ(ಕ್ಷೇತ್ರ_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(ಕ್ಷೇತ್ರ_2.Alignment, MaxAlign);
  }
  ಪ್ರತಿಜ್ಞಾ(ಕೊನೆಯ_ಕೊನೆ == ಗಾತ್ರ_2 && "didn't compute LastEnd correctly");
  ಪ್ರತಿಜ್ಞಾ(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#ಆದರೆ_ಅಂತ್ಯ

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<ಕ್ಷೇತ್ರ_2> Fields) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  // Do some simple precondition checks.
  {
    ತಾರ್ಕಿಕ ಒಳ_Fixed_ಪೂರ್ವಪ್ರತ್ಯಯ = ನಿಜ;
    size_t ಕೊನೆಯ_ಕೊನೆ = 0;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಕ್ಷೇತ್ರ_2 : Fields) {
      ಪ್ರತಿಜ್ಞಾ(ಕ್ಷೇತ್ರ_2.ಗಾತ್ರ_2 > 0 && "field of zero size");
      ಆದರೆ (ಕ್ಷೇತ್ರ_2.has_Fixed_ಸ್ಥಾನಾಂತರ()) {
        ಪ್ರತಿಜ್ಞಾ(ಒಳ_Fixed_ಪೂರ್ವಪ್ರತ್ಯಯ &&
               "fixed-offset fields are not a strict prefix of array");
        ಪ್ರತಿಜ್ಞಾ(ಕೊನೆಯ_ಕೊನೆ <= ಕ್ಷೇತ್ರ_2.ಸ್ಥಾನಾಂತರ &&
               "fixed-offset fields overlap or are not in order");
        ಕೊನೆಯ_ಕೊನೆ = ಕ್ಷೇತ್ರ_2.get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();
        ಪ್ರತಿಜ್ಞಾ(ಕೊನೆಯ_ಕೊನೆ > ಕ್ಷೇತ್ರ_2.ಸ್ಥಾನಾಂತರ &&
               "overflow in fixed-offset end offset");
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಒಳ_Fixed_ಪೂರ್ವಪ್ರತ್ಯಯ = ಸುಳ್ಳು;
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  ಸ್ವಯಂಚಾಲಿತ ಮೊದಲ_Flexible = Fields.begin(), E = Fields.ಕೊನೆ();
  ಆಗ (ಮೊದಲ_Flexible != E && ಮೊದಲ_Flexible->has_Fixed_ಸ್ಥಾನಾಂತರ()) {
    MaxAlign = std::max(MaxAlign, ಮೊದಲ_Flexible->Alignment);
    ++ಮೊದಲ_Flexible;
  }

  // If there are no flexible fields, we're done.
  ಆದರೆ (ಮೊದಲ_Flexible == E) {
    uint64_t ಗಾತ್ರ_2 = 0;
    ಆದರೆ (!Fields.ಖಾಲಿಗಳು())
      ಗಾತ್ರ_2 = Fields.ಹಿಂಭಾಗ().get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
    checkValidLayout(Fields, ಗಾತ್ರ_2, MaxAlign);
#ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ {ಗಾತ್ರ_2, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_ಸಂಖ್ಯೆ = 0;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = ಮೊದಲ_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<ಶೂನ್ಯ*>(Unique_ಸಂಖ್ಯೆ++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ವಿಂಗಡಿಸು(ಮೊದಲ_Flexible, E,
                 [](ಸ್ಥಿರಾಂಕ ಕ್ಷೇತ್ರ_2 *lhs, ಸ್ಥಿರಾಂಕ ಕ್ಷೇತ್ರ_2 *rhs) -> ಪೂರ್ಣಾಂಕ {
    // Decreasing alignment.
    ಆದರೆ (lhs->Alignment != rhs->Alignment)
      ಹಿಂತಿರುಗಿ (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    ಆದರೆ (lhs->ಗಾತ್ರ_2 != rhs->ಗಾತ್ರ_2)
      ಹಿಂತಿರುಗಿ (lhs->ಗಾತ್ರ_2 < rhs->ಗಾತ್ರ_2 ? 1 : -1);

    // Original order.
    ಸ್ವಯಂಚಾಲಿತ lhs_ಸಂಖ್ಯೆ = reinterpret_cast<uintptr_t>(lhs->Scratch);
    ಸ್ವಯಂಚಾಲಿತ rhs_ಸಂಖ್ಯೆ = reinterpret_cast<uintptr_t>(rhs->Scratch);
    ಆದರೆ (lhs_ಸಂಖ್ಯೆ != rhs_ಸಂಖ್ಯೆ)
      ಹಿಂತಿರುಗಿ (lhs_ಸಂಖ್ಯೆ < rhs_ಸಂಖ್ಯೆ ? -1 : 1);

    ಹಿಂತಿರುಗಿ 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    ತಾರ್ಕಿಕ HasPadding = ಸುಳ್ಳು;
    uint64_t ಕೊನೆಯ_ಕೊನೆ = 0;

    // Walk the fixed-offset fields.
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = Fields.begin(); I != ಮೊದಲ_Flexible; ++I) {
      ಪ್ರತಿಜ್ಞಾ(I->has_Fixed_ಸ್ಥಾನಾಂತರ());
      ಆದರೆ (ಕೊನೆಯ_ಕೊನೆ != I->ಸ್ಥಾನಾಂತರ) {
        HasPadding = ನಿಜ;
        ನಿಲ್ಲಿಸು;
      }
      ಕೊನೆಯ_ಕೊನೆ = I->get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    ಆದರೆ (!HasPadding) {
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = ಮೊದಲ_Flexible; I != E; ++I) {
        ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಾನಾಂತರ = align_ಗಮ್ಯ(ಕೊನೆಯ_ಕೊನೆ, I->Alignment);
        ಆದರೆ (ಕೊನೆಯ_ಕೊನೆ != ಸ್ಥಾನಾಂತರ) {
          HasPadding = ನಿಜ;
          ನಿಲ್ಲಿಸು;
        }
        I->ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;
        ಕೊನೆಯ_ಕೊನೆ = I->get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();
      }
    }

    // If we already have a perfect layout, we're done.
    ಆದರೆ (!HasPadding) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
      checkValidLayout(Fields, ಕೊನೆಯ_ಕೊನೆ, MaxAlign);
#ಆದರೆ_ಅಂತ್ಯ
      ಹಿಂತಿರುಗಿ {ಕೊನೆಯ_ಕೊನೆ, MaxAlign};
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
  ರಚನೆ Alignment_ಸರತಿ {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_ಗಾತ್ರ;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_ಕ್ಷೇತ್ರ *ತಲೆ_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ಸ್ಥಿರ ಕ್ಷೇತ್ರ_2 *get_ಮುಂದಿನ(ಕ್ಷೇತ್ರ_2 *Cur) {
      ಹಿಂತಿರುಗಿ static_cast<ಕ್ಷೇತ್ರ_2 *>(Cur->Scratch);
    }
  };
  Small_ಸದಿಶ<Alignment_ಸರತಿ, 8> FlexibleFieldsByAlignment;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = ಮೊದಲ_Flexible; I != E; ) {
    ಸ್ವಯಂಚಾಲಿತ ತಲೆ_2 = I;
    ಸ್ವಯಂಚಾಲಿತ Alignment = I->Alignment;

    uint64_t Min_ಗಾತ್ರ = I->ಗಾತ್ರ_2;
    ಸ್ವಯಂಚಾಲಿತ ಕೊನೆಯ_ಒಳ_ಸರತಿ = I;
    ಗಾಗಿ (++I; I != E && I->Alignment == Alignment; ++I) {
      ಕೊನೆಯ_ಒಳ_ಸರತಿ->Scratch = I;
      ಕೊನೆಯ_ಒಳ_ಸರತಿ = I;
      Min_ಗಾತ್ರ = std::min(Min_ಗಾತ್ರ, I->ಗಾತ್ರ_2);
    }
    ಕೊನೆಯ_ಒಳ_ಸರತಿ->Scratch = ಶೂನ್ಯಸೂಚಕ;

    FlexibleFieldsByAlignment.ತಳ್ಳು_ಹಿಂಭಾಗ({Min_ಗಾತ್ರ, ತಲೆ_2, Alignment});
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  // Verify that we set the queues up correctly.
  ಸ್ವಯಂಚಾಲಿತ checkQueues = [&]{
    ತಾರ್ಕಿಕ ಮೊದಲ_ಸರತಿ = ನಿಜ;
    Align ಕೊನೆಯ_ಸರತಿ_Alignment;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಸರತಿ : FlexibleFieldsByAlignment) {
      ಪ್ರತಿಜ್ಞಾ((ಮೊದಲ_ಸರತಿ || ಸರತಿ.Alignment < ಕೊನೆಯ_ಸರತಿ_Alignment) &&
             "bins not in order of descending alignment");
      ಕೊನೆಯ_ಸರತಿ_Alignment = ಸರತಿ.Alignment;
      ಮೊದಲ_ಸರತಿ = ಸುಳ್ಳು;

      ಪ್ರತಿಜ್ಞಾ(ಸರತಿ.ತಲೆ_2 && "queue was empty");
      uint64_t ಕೊನೆಯ_ಗಾತ್ರ = ~(uint64_t)0;
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = ಸರತಿ.ತಲೆ_2; I; I = ಸರತಿ.get_ಮುಂದಿನ(I)) {
        ಪ್ರತಿಜ್ಞಾ(I->Alignment == ಸರತಿ.Alignment && "bad field in queue");
        ಪ್ರತಿಜ್ಞಾ(I->ಗಾತ್ರ_2 <= ಕೊನೆಯ_ಗಾತ್ರ && "queue not in descending size order");
        ಕೊನೆಯ_ಗಾತ್ರ = I->ಗಾತ್ರ_2;
      }
    }
  };
  checkQueues();
#ಆದರೆ_ಅಂತ್ಯ

  /// Helper function to remove a field from a queue.
  ಸ್ವಯಂಚಾಲಿತ splice_From_ಸರತಿ = [&](Alignment_ಸರತಿ *ಸರತಿ, ಕ್ಷೇತ್ರ_2 *ಕೊನೆಯ, ಕ್ಷೇತ್ರ_2 *Cur) {
    ಪ್ರತಿಜ್ಞಾ(ಕೊನೆಯ ? ಸರತಿ->get_ಮುಂದಿನ(ಕೊನೆಯ) == Cur : ಸರತಿ->ತಲೆ_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    ಆದರೆ (ಕೊನೆಯ) {
      ಕೊನೆಯ->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      ಆದರೆ (!Cur->Scratch)
        ಸರತಿ->Min_ಗಾತ್ರ = ಕೊನೆಯ->ಗಾತ್ರ_2;

    // Otherwise, replace the head.
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ New_ತಲೆ = ಸರತಿ->get_ಮುಂದಿನ(Cur))
        ಸರತಿ->ತಲೆ_2 = New_ತಲೆ;

      // If we just emptied the queue, destroy its bin.
      ಇಲ್ಲದಿದ್ದರೆ
        FlexibleFieldsByAlignment.erase(ಸರತಿ);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_ಸದಿಶ<ಕ್ಷೇತ್ರ_2, 16> Layout;
  Layout.reserve(Fields.ಗಾತ್ರ_3());

  // The offset that we're currently looking to insert at (or after).
  uint64_t ಕೊನೆಯ_ಕೊನೆ = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  ಸ್ವಯಂಚಾಲಿತ ಕೂಡಿಸು_ಗಮ್ಯ_Layout = [&](Alignment_ಸರತಿ *ಸರತಿ, ಕ್ಷೇತ್ರ_2 *ಕೊನೆಯ, ಕ್ಷೇತ್ರ_2 *Cur,
                         uint64_t ಸ್ಥಾನಾಂತರ) -> ತಾರ್ಕಿಕ {
    ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಾನಾಂತರ == align_ಗಮ್ಯ(ಕೊನೆಯ_ಕೊನೆ, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_ಸರತಿ(ಸರತಿ, ಕೊನೆಯ, Cur);

    // Add Cur to the layout.
    Layout.ತಳ್ಳು_ಹಿಂಭಾಗ(*Cur);
    Layout.ಹಿಂಭಾಗ().ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;
    ಕೊನೆಯ_ಕೊನೆ = Layout.ಹಿಂಭಾಗ().get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();

    // Always return true so that we can be tail-called.
    ಹಿಂತಿರುಗಿ ನಿಜ;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  ಸ್ವಯಂಚಾಲಿತ try_ಕೂಡಿಸು_Filler_From_ಸರತಿ = [&](Alignment_ಸರತಿ *ಸರತಿ, uint64_t ಆರಂಭ_ಸ್ಥಾನಾಂತರ,
                                   std::optional<uint64_t> ಕೊನೆ_ಸ್ಥಾನಾಂತರ) -> ತಾರ್ಕಿಕ {
    ಪ್ರತಿಜ್ಞಾ(ಸರತಿ->ತಲೆ_2);
    ಪ್ರತಿಜ್ಞಾ(ಆರಂಭ_ಸ್ಥಾನಾಂತರ == align_ಗಮ್ಯ(ಕೊನೆಯ_ಕೊನೆ, ಸರತಿ->Alignment));
    ಪ್ರತಿಜ್ಞಾ(!ಕೊನೆ_ಸ್ಥಾನಾಂತರ || ಆರಂಭ_ಸ್ಥಾನಾಂತರ < *ಕೊನೆ_ಸ್ಥಾನಾಂತರ);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    ಸ್ವಯಂಚಾಲಿತ Max_Viable_ಗಾತ್ರ =
      (ಕೊನೆ_ಸ್ಥಾನಾಂತರ ? *ಕೊನೆ_ಸ್ಥಾನಾಂತರ - ಆರಂಭ_ಸ್ಥಾನಾಂತರ : ~(uint64_t)0);
    ಆದರೆ (ಸರತಿ->Min_ಗಾತ್ರ > Max_Viable_ಗಾತ್ರ)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    ಗಾಗಿ (ಕ್ಷೇತ್ರ_2 *Cur = ಸರತಿ->ತಲೆ_2, *ಕೊನೆಯ = ಶೂನ್ಯಸೂಚಕ; ನಿಜ;
           ಕೊನೆಯ = Cur, Cur = ಸರತಿ->get_ಮುಂದಿನ(Cur)) {
      ಪ್ರತಿಜ್ಞಾ(Cur && "didn't find a match in queue despite its MinSize");
      ಆದರೆ (Cur->ಗಾತ್ರ_2 <= Max_Viable_ಗಾತ್ರ)
        ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_ಗಮ್ಯ_Layout(ಸರತಿ, ಕೊನೆಯ, Cur, ಆರಂಭ_ಸ್ಥಾನಾಂತರ);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  ಸ್ವಯಂಚಾಲಿತ try_ಕೂಡಿಸು_ಉತ್ತಮ_ಕ್ಷೇತ್ರ = [&](std::optional<uint64_t> Before_ಸ್ಥಾನಾಂತರ) -> ತಾರ್ಕಿಕ {
    ಪ್ರತಿಜ್ಞಾ(!Before_ಸ್ಥಾನಾಂತರ || ಕೊನೆಯ_ಕೊನೆ < *Before_ಸ್ಥಾನಾಂತರ);
    ಸ್ವಯಂಚಾಲಿತ ಸರತಿ_B = FlexibleFieldsByAlignment.begin();
    ಸ್ವಯಂಚಾಲಿತ ಸರತಿ_E = FlexibleFieldsByAlignment.ಕೊನೆ();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    ಸ್ವಯಂಚಾಲಿತ ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು = ಸರತಿ_B;
    ಗಾಗಿ (; ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು != ಸರತಿ_E; ++ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು) {
      ಆದರೆ (isAligned(ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು->Alignment, ಕೊನೆಯ_ಕೊನೆ))
        ನಿಲ್ಲಿಸು;
    }

    uint64_t ಸ್ಥಾನಾಂತರ = ಕೊನೆಯ_ಕೊನೆ;
    ಆಗ (ನಿಜ) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ ಸರತಿ = ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು; ಸರತಿ != ಸರತಿ_E; ++ಸರತಿ) {
        ಆದರೆ (try_ಕೂಡಿಸು_Filler_From_ಸರತಿ(ಸರತಿ, ಸ್ಥಾನಾಂತರ, Before_ಸ್ಥಾನಾಂತರ))
          ಹಿಂತಿರುಗಿ ನಿಜ;
      }

      // Okay, we don't need to scan those again.
      ಸರತಿ_E = ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು;

      // If we started from the first queue, we're done.
      ಆದರೆ (ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು == ಸರತಿ_B)
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು;
      ಸ್ಥಾನಾಂತರ = align_ಗಮ್ಯ(ಕೊನೆಯ_ಕೊನೆ, ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು->Alignment);
      ಆದರೆ (Before_ಸ್ಥಾನಾಂತರ && ಸ್ಥಾನಾಂತರ >= *Before_ಸ್ಥಾನಾಂತರ)
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಆಗ (ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು != ಸರತಿ_B &&
             ಸ್ಥಾನಾಂತರ == align_ಗಮ್ಯ(ಕೊನೆಯ_ಕೊನೆ, ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು[-1].Alignment))
        --ಮೊದಲ_ಸರತಿ_ಗಮ್ಯ_ಹುಡುಕು;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = Fields.begin(); I != ಮೊದಲ_Flexible; ++I) {
    ಪ್ರತಿಜ್ಞಾ(ಕೊನೆಯ_ಕೊನೆ <= I->ಸ್ಥಾನಾಂತರ);
    ಆಗ (ಕೊನೆಯ_ಕೊನೆ != I->ಸ್ಥಾನಾಂತರ) {
      ಆದರೆ (!try_ಕೂಡಿಸು_ಉತ್ತಮ_ಕ್ಷೇತ್ರ(I->ಸ್ಥಾನಾಂತರ))
        ನಿಲ್ಲಿಸು;
    }
    Layout.ತಳ್ಳು_ಹಿಂಭಾಗ(*I);
    ಕೊನೆಯ_ಕೊನೆ = I->get_ಕೊನೆ_ಸ್ಥಾನಾಂತರ();
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  checkQueues();
#ಆದರೆ_ಅಂತ್ಯ

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  ಆಗ (!FlexibleFieldsByAlignment.ಖಾಲಿಗಳು()) {
    ತಾರ್ಕಿಕ Success = try_ಕೂಡಿಸು_ಉತ್ತಮ_ಕ್ಷೇತ್ರ(std::nullopt);
    ಪ್ರತಿಜ್ಞಾ(Success && "didn't find a field with no fixed limit?");
    (ಶೂನ್ಯ) Success;
  }

  // Copy the layout back into place.
  ಪ್ರತಿಜ್ಞಾ(Layout.ಗಾತ್ರ_3() == Fields.ಗಾತ್ರ_3());
  ಸ್ಮರಣೆ_ನಕಲು(Fields.ದತ್ತಾಂಶ(), Layout.ದತ್ತಾಂಶ(),
         Fields.ಗಾತ್ರ_3() * ಗಾತ್ರ(Optimized_Struct_Layout_ಕ್ಷೇತ್ರ));

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, ಕೊನೆಯ_ಕೊನೆ, MaxAlign);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ {ಕೊನೆಯ_ಕೊನೆ, MaxAlign};
}
