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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/OptimizedStructLayout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਵਰਤੋਂ ਖੇਤਰ_2 = Optimized_Struct_Layout_ਖੇਤਰ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਸਥਿਰ ਖਾਲੀ checkValidLayout(ArrayRef<ਖੇਤਰ_2> Fields, uint64_t ਮਾਪ,
                             Align MaxAlign) {
  uint64_t ਆਖਰੀ_ਸਿਰਾ = 0;
  Align ComputedMaxAlign;
  ਲਈ (ਸਵੈਚਾਲਿਤ &ਖੇਤਰ_2 : Fields) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਖੇਤਰ_2.has_Fixed_ਵਿੱਥ() &&
           "didn't assign a fixed offset to field");
    ਪ੍ਰਤਿਜ੍ਞਾ(isAligned(ਖੇਤਰ_2.Alignment, ਖੇਤਰ_2.ਵਿੱਥ) &&
           "didn't assign a correctly-aligned offset to field");
    ਪ੍ਰਤਿਜ੍ਞਾ(ਖੇਤਰ_2.ਵਿੱਥ >= ਆਖਰੀ_ਸਿਰਾ &&
           "didn't assign offsets in ascending order");
    ਆਖਰੀ_ਸਿਰਾ = ਖੇਤਰ_2.get_ਸਿਰਾ_ਵਿੱਥ();
    ਪ੍ਰਤਿਜ੍ਞਾ(ਖੇਤਰ_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(ਖੇਤਰ_2.Alignment, MaxAlign);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(ਆਖਰੀ_ਸਿਰਾ == ਮਾਪ && "didn't compute LastEnd correctly");
  ਪ੍ਰਤਿਜ੍ਞਾ(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#ਜੇ_ਅੰਤ

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<ਖੇਤਰ_2> Fields) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  // Do some simple precondition checks.
  {
    ਤਾਰਕਿਕ ਵਿੱਚ_Fixed_ਅਗੇਤਰ = ਸੱਚ;
    size_t ਆਖਰੀ_ਸਿਰਾ = 0;
    ਲਈ (ਸਵੈਚਾਲਿਤ &ਖੇਤਰ_2 : Fields) {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਖੇਤਰ_2.ਮਾਪ > 0 && "field of zero size");
      ਜੇ (ਖੇਤਰ_2.has_Fixed_ਵਿੱਥ()) {
        ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਚ_Fixed_ਅਗੇਤਰ &&
               "fixed-offset fields are not a strict prefix of array");
        ਪ੍ਰਤਿਜ੍ਞਾ(ਆਖਰੀ_ਸਿਰਾ <= ਖੇਤਰ_2.ਵਿੱਥ &&
               "fixed-offset fields overlap or are not in order");
        ਆਖਰੀ_ਸਿਰਾ = ਖੇਤਰ_2.get_ਸਿਰਾ_ਵਿੱਥ();
        ਪ੍ਰਤਿਜ੍ਞਾ(ਆਖਰੀ_ਸਿਰਾ > ਖੇਤਰ_2.ਵਿੱਥ &&
               "overflow in fixed-offset end offset");
      } ਨਹੀਂਤਾਂ {
        ਵਿੱਚ_Fixed_ਅਗੇਤਰ = ਝੂਠ;
      }
    }
  }
#ਜੇ_ਅੰਤ

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  ਸਵੈਚਾਲਿਤ ਪਹਿਲਾ_Flexible = Fields.begin(), E = Fields.ਸਿਰਾ();
  ਜਦੋਂ (ਪਹਿਲਾ_Flexible != E && ਪਹਿਲਾ_Flexible->has_Fixed_ਵਿੱਥ()) {
    MaxAlign = std::max(MaxAlign, ਪਹਿਲਾ_Flexible->Alignment);
    ++ਪਹਿਲਾ_Flexible;
  }

  // If there are no flexible fields, we're done.
  ਜੇ (ਪਹਿਲਾ_Flexible == E) {
    uint64_t ਮਾਪ = 0;
    ਜੇ (!Fields.ਖਾਲੀ_ਥਾਵਾਂ())
      ਮਾਪ = Fields.ਪਿੱਠ().get_ਸਿਰਾ_ਵਿੱਥ();

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
    checkValidLayout(Fields, ਮਾਪ, MaxAlign);
#ਜੇ_ਅੰਤ
    ਵਾਪਸ {ਮਾਪ, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_ਸੰਖਿਆ = 0;
    ਲਈ (ਸਵੈਚਾਲਿਤ I = ਪਹਿਲਾ_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<ਖਾਲੀ*>(Unique_ਸੰਖਿਆ++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ਛਾਂਟੋ(ਪਹਿਲਾ_Flexible, E,
                 [](ਅਚਲ ਖੇਤਰ_2 *lhs, ਅਚਲ ਖੇਤਰ_2 *rhs) -> ਪੂਰਨ {
    // Decreasing alignment.
    ਜੇ (lhs->Alignment != rhs->Alignment)
      ਵਾਪਸ (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    ਜੇ (lhs->ਮਾਪ != rhs->ਮਾਪ)
      ਵਾਪਸ (lhs->ਮਾਪ < rhs->ਮਾਪ ? 1 : -1);

    // Original order.
    ਸਵੈਚਾਲਿਤ lhs_ਸੰਖਿਆ = reinterpret_cast<uintptr_t>(lhs->Scratch);
    ਸਵੈਚਾਲਿਤ rhs_ਸੰਖਿਆ = reinterpret_cast<uintptr_t>(rhs->Scratch);
    ਜੇ (lhs_ਸੰਖਿਆ != rhs_ਸੰਖਿਆ)
      ਵਾਪਸ (lhs_ਸੰਖਿਆ < rhs_ਸੰਖਿਆ ? -1 : 1);

    ਵਾਪਸ 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    ਤਾਰਕਿਕ HasPadding = ਝੂਠ;
    uint64_t ਆਖਰੀ_ਸਿਰਾ = 0;

    // Walk the fixed-offset fields.
    ਲਈ (ਸਵੈਚਾਲਿਤ I = Fields.begin(); I != ਪਹਿਲਾ_Flexible; ++I) {
      ਪ੍ਰਤਿਜ੍ਞਾ(I->has_Fixed_ਵਿੱਥ());
      ਜੇ (ਆਖਰੀ_ਸਿਰਾ != I->ਵਿੱਥ) {
        HasPadding = ਸੱਚ;
        ਰੋਕੋ;
      }
      ਆਖਰੀ_ਸਿਰਾ = I->get_ਸਿਰਾ_ਵਿੱਥ();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    ਜੇ (!HasPadding) {
      ਲਈ (ਸਵੈਚਾਲਿਤ I = ਪਹਿਲਾ_Flexible; I != E; ++I) {
        ਸਵੈਚਾਲਿਤ ਵਿੱਥ = align_ਮੰਜ਼ਿਲ(ਆਖਰੀ_ਸਿਰਾ, I->Alignment);
        ਜੇ (ਆਖਰੀ_ਸਿਰਾ != ਵਿੱਥ) {
          HasPadding = ਸੱਚ;
          ਰੋਕੋ;
        }
        I->ਵਿੱਥ = ਵਿੱਥ;
        ਆਖਰੀ_ਸਿਰਾ = I->get_ਸਿਰਾ_ਵਿੱਥ();
      }
    }

    // If we already have a perfect layout, we're done.
    ਜੇ (!HasPadding) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
      checkValidLayout(Fields, ਆਖਰੀ_ਸਿਰਾ, MaxAlign);
#ਜੇ_ਅੰਤ
      ਵਾਪਸ {ਆਖਰੀ_ਸਿਰਾ, MaxAlign};
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
  ਬਣਤਰ Alignment_ਕਤਾਰ {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_ਮਾਪ;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_ਖੇਤਰ *ਸਿਰ_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ਸਥਿਰ ਖੇਤਰ_2 *get_ਅਗਲਾ(ਖੇਤਰ_2 *Cur) {
      ਵਾਪਸ static_cast<ਖੇਤਰ_2 *>(Cur->Scratch);
    }
  };
  Small_ਸਦਿਸ਼<Alignment_ਕਤਾਰ, 8> FlexibleFieldsByAlignment;
  ਲਈ (ਸਵੈਚਾਲਿਤ I = ਪਹਿਲਾ_Flexible; I != E; ) {
    ਸਵੈਚਾਲਿਤ ਸਿਰ_2 = I;
    ਸਵੈਚਾਲਿਤ Alignment = I->Alignment;

    uint64_t Min_ਮਾਪ = I->ਮਾਪ;
    ਸਵੈਚਾਲਿਤ ਆਖਰੀ_ਵਿੱਚ_ਕਤਾਰ = I;
    ਲਈ (++I; I != E && I->Alignment == Alignment; ++I) {
      ਆਖਰੀ_ਵਿੱਚ_ਕਤਾਰ->Scratch = I;
      ਆਖਰੀ_ਵਿੱਚ_ਕਤਾਰ = I;
      Min_ਮਾਪ = std::min(Min_ਮਾਪ, I->ਮਾਪ);
    }
    ਆਖਰੀ_ਵਿੱਚ_ਕਤਾਰ->Scratch = ਖਾਲੀ_ਸੂਚਕ;

    FlexibleFieldsByAlignment.ਧੱਕੋ_ਪਿੱਠ({Min_ਮਾਪ, ਸਿਰ_2, Alignment});
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  // Verify that we set the queues up correctly.
  ਸਵੈਚਾਲਿਤ checkQueues = [&]{
    ਤਾਰਕਿਕ ਪਹਿਲਾ_ਕਤਾਰ = ਸੱਚ;
    Align ਆਖਰੀ_ਕਤਾਰ_Alignment;
    ਲਈ (ਸਵੈਚਾਲਿਤ &ਕਤਾਰ : FlexibleFieldsByAlignment) {
      ਪ੍ਰਤਿਜ੍ਞਾ((ਪਹਿਲਾ_ਕਤਾਰ || ਕਤਾਰ.Alignment < ਆਖਰੀ_ਕਤਾਰ_Alignment) &&
             "bins not in order of descending alignment");
      ਆਖਰੀ_ਕਤਾਰ_Alignment = ਕਤਾਰ.Alignment;
      ਪਹਿਲਾ_ਕਤਾਰ = ਝੂਠ;

      ਪ੍ਰਤਿਜ੍ਞਾ(ਕਤਾਰ.ਸਿਰ_2 && "queue was empty");
      uint64_t ਆਖਰੀ_ਮਾਪ = ~(uint64_t)0;
      ਲਈ (ਸਵੈਚਾਲਿਤ I = ਕਤਾਰ.ਸਿਰ_2; I; I = ਕਤਾਰ.get_ਅਗਲਾ(I)) {
        ਪ੍ਰਤਿਜ੍ਞਾ(I->Alignment == ਕਤਾਰ.Alignment && "bad field in queue");
        ਪ੍ਰਤਿਜ੍ਞਾ(I->ਮਾਪ <= ਆਖਰੀ_ਮਾਪ && "queue not in descending size order");
        ਆਖਰੀ_ਮਾਪ = I->ਮਾਪ;
      }
    }
  };
  checkQueues();
#ਜੇ_ਅੰਤ

  /// Helper function to remove a field from a queue.
  ਸਵੈਚਾਲਿਤ splice_From_ਕਤਾਰ = [&](Alignment_ਕਤਾਰ *ਕਤਾਰ, ਖੇਤਰ_2 *ਆਖਰੀ, ਖੇਤਰ_2 *Cur) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਆਖਰੀ ? ਕਤਾਰ->get_ਅਗਲਾ(ਆਖਰੀ) == Cur : ਕਤਾਰ->ਸਿਰ_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    ਜੇ (ਆਖਰੀ) {
      ਆਖਰੀ->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      ਜੇ (!Cur->Scratch)
        ਕਤਾਰ->Min_ਮਾਪ = ਆਖਰੀ->ਮਾਪ;

    // Otherwise, replace the head.
    } ਨਹੀਂਤਾਂ {
      ਜੇ (ਸਵੈਚਾਲਿਤ New_ਸਿਰ = ਕਤਾਰ->get_ਅਗਲਾ(Cur))
        ਕਤਾਰ->ਸਿਰ_2 = New_ਸਿਰ;

      // If we just emptied the queue, destroy its bin.
      ਨਹੀਂਤਾਂ
        FlexibleFieldsByAlignment.erase(ਕਤਾਰ);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_ਸਦਿਸ਼<ਖੇਤਰ_2, 16> Layout;
  Layout.reserve(Fields.ਮਾਪ_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t ਆਖਰੀ_ਸਿਰਾ = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  ਸਵੈਚਾਲਿਤ ਜੋੜੋ_ਮੰਜ਼ਿਲ_Layout = [&](Alignment_ਕਤਾਰ *ਕਤਾਰ, ਖੇਤਰ_2 *ਆਖਰੀ, ਖੇਤਰ_2 *Cur,
                         uint64_t ਵਿੱਥ) -> ਤਾਰਕਿਕ {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਥ == align_ਮੰਜ਼ਿਲ(ਆਖਰੀ_ਸਿਰਾ, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_ਕਤਾਰ(ਕਤਾਰ, ਆਖਰੀ, Cur);

    // Add Cur to the layout.
    Layout.ਧੱਕੋ_ਪਿੱਠ(*Cur);
    Layout.ਪਿੱਠ().ਵਿੱਥ = ਵਿੱਥ;
    ਆਖਰੀ_ਸਿਰਾ = Layout.ਪਿੱਠ().get_ਸਿਰਾ_ਵਿੱਥ();

    // Always return true so that we can be tail-called.
    ਵਾਪਸ ਸੱਚ;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  ਸਵੈਚਾਲਿਤ try_ਜੋੜੋ_Filler_From_ਕਤਾਰ = [&](Alignment_ਕਤਾਰ *ਕਤਾਰ, uint64_t ਸ਼ੁਰੂ_ਵਿੱਥ,
                                   std::optional<uint64_t> ਸਿਰਾ_ਵਿੱਥ) -> ਤਾਰਕਿਕ {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਕਤਾਰ->ਸਿਰ_2);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਸ਼ੁਰੂ_ਵਿੱਥ == align_ਮੰਜ਼ਿਲ(ਆਖਰੀ_ਸਿਰਾ, ਕਤਾਰ->Alignment));
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਸਿਰਾ_ਵਿੱਥ || ਸ਼ੁਰੂ_ਵਿੱਥ < *ਸਿਰਾ_ਵਿੱਥ);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    ਸਵੈਚਾਲਿਤ Max_Viable_ਮਾਪ =
      (ਸਿਰਾ_ਵਿੱਥ ? *ਸਿਰਾ_ਵਿੱਥ - ਸ਼ੁਰੂ_ਵਿੱਥ : ~(uint64_t)0);
    ਜੇ (ਕਤਾਰ->Min_ਮਾਪ > Max_Viable_ਮਾਪ)
      ਵਾਪਸ ਝੂਠ;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    ਲਈ (ਖੇਤਰ_2 *Cur = ਕਤਾਰ->ਸਿਰ_2, *ਆਖਰੀ = ਖਾਲੀ_ਸੂਚਕ; ਸੱਚ;
           ਆਖਰੀ = Cur, Cur = ਕਤਾਰ->get_ਅਗਲਾ(Cur)) {
      ਪ੍ਰਤਿਜ੍ਞਾ(Cur && "didn't find a match in queue despite its MinSize");
      ਜੇ (Cur->ਮਾਪ <= Max_Viable_ਮਾਪ)
        ਵਾਪਸ ਜੋੜੋ_ਮੰਜ਼ਿਲ_Layout(ਕਤਾਰ, ਆਖਰੀ, Cur, ਸ਼ੁਰੂ_ਵਿੱਥ);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  ਸਵੈਚਾਲਿਤ try_ਜੋੜੋ_ਉੱਤਮ_ਖੇਤਰ = [&](std::optional<uint64_t> Before_ਵਿੱਥ) -> ਤਾਰਕਿਕ {
    ਪ੍ਰਤਿਜ੍ਞਾ(!Before_ਵਿੱਥ || ਆਖਰੀ_ਸਿਰਾ < *Before_ਵਿੱਥ);
    ਸਵੈਚਾਲਿਤ ਕਤਾਰ_B = FlexibleFieldsByAlignment.begin();
    ਸਵੈਚਾਲਿਤ ਕਤਾਰ_E = FlexibleFieldsByAlignment.ਸਿਰਾ();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    ਸਵੈਚਾਲਿਤ ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ = ਕਤਾਰ_B;
    ਲਈ (; ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ != ਕਤਾਰ_E; ++ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ) {
      ਜੇ (isAligned(ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ->Alignment, ਆਖਰੀ_ਸਿਰਾ))
        ਰੋਕੋ;
    }

    uint64_t ਵਿੱਥ = ਆਖਰੀ_ਸਿਰਾ;
    ਜਦੋਂ (ਸੱਚ) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      ਲਈ (ਸਵੈਚਾਲਿਤ ਕਤਾਰ = ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ; ਕਤਾਰ != ਕਤਾਰ_E; ++ਕਤਾਰ) {
        ਜੇ (try_ਜੋੜੋ_Filler_From_ਕਤਾਰ(ਕਤਾਰ, ਵਿੱਥ, Before_ਵਿੱਥ))
          ਵਾਪਸ ਸੱਚ;
      }

      // Okay, we don't need to scan those again.
      ਕਤਾਰ_E = ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ;

      // If we started from the first queue, we're done.
      ਜੇ (ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ == ਕਤਾਰ_B)
        ਵਾਪਸ ਝੂਠ;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ;
      ਵਿੱਥ = align_ਮੰਜ਼ਿਲ(ਆਖਰੀ_ਸਿਰਾ, ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ->Alignment);
      ਜੇ (Before_ਵਿੱਥ && ਵਿੱਥ >= *Before_ਵਿੱਥ)
        ਵਾਪਸ ਝੂਠ;
      ਜਦੋਂ (ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ != ਕਤਾਰ_B &&
             ਵਿੱਥ == align_ਮੰਜ਼ਿਲ(ਆਖਰੀ_ਸਿਰਾ, ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ[-1].Alignment))
        --ਪਹਿਲਾ_ਕਤਾਰ_ਮੰਜ਼ਿਲ_ਖੋਜ;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  ਲਈ (ਸਵੈਚਾਲਿਤ I = Fields.begin(); I != ਪਹਿਲਾ_Flexible; ++I) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਆਖਰੀ_ਸਿਰਾ <= I->ਵਿੱਥ);
    ਜਦੋਂ (ਆਖਰੀ_ਸਿਰਾ != I->ਵਿੱਥ) {
      ਜੇ (!try_ਜੋੜੋ_ਉੱਤਮ_ਖੇਤਰ(I->ਵਿੱਥ))
        ਰੋਕੋ;
    }
    Layout.ਧੱਕੋ_ਪਿੱਠ(*I);
    ਆਖਰੀ_ਸਿਰਾ = I->get_ਸਿਰਾ_ਵਿੱਥ();
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  checkQueues();
#ਜੇ_ਅੰਤ

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  ਜਦੋਂ (!FlexibleFieldsByAlignment.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਤਾਰਕਿਕ Success = try_ਜੋੜੋ_ਉੱਤਮ_ਖੇਤਰ(std::nullopt);
    ਪ੍ਰਤਿਜ੍ਞਾ(Success && "didn't find a field with no fixed limit?");
    (ਖਾਲੀ) Success;
  }

  // Copy the layout back into place.
  ਪ੍ਰਤਿਜ੍ਞਾ(Layout.ਮਾਪ_2() == Fields.ਮਾਪ_2());
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Fields.ਅੰਕੜੇ(), Layout.ਅੰਕੜੇ(),
         Fields.ਮਾਪ_2() * ਆਕਾਰ(Optimized_Struct_Layout_ਖੇਤਰ));

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, ਆਖਰੀ_ਸਿਰਾ, MaxAlign);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ {ਆਖਰੀ_ਸਿਰਾ, MaxAlign};
}
