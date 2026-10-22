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

#ضمن "llvm/Support/OptimizedStructLayout.h"
#ضمن <optional>

ورتوں نطاق llvm;

ورتوں میدان_2 = Optimized_Struct_Layout_میدان;

#جے_بنا_تعریف NDEBUG
ساكن فراغ checkValidLayout(ArrayRef<میدان_2> Fields, uint64_t ناپ,
                             Align MaxAlign) {
  uint64_t آخری_سرا = 0;
  Align ComputedMaxAlign;
  لكل (خودکار &میدان_2 : Fields) {
    أكد(میدان_2.has_Fixed_وتھ() &&
           "didn't assign a fixed offset to field");
    أكد(isAligned(میدان_2.Alignment, میدان_2.وتھ) &&
           "didn't assign a correctly-aligned offset to field");
    أكد(میدان_2.وتھ >= آخری_سرا &&
           "didn't assign offsets in ascending order");
    آخری_سرا = میدان_2.get_سرا_وتھ();
    أكد(میدان_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(میدان_2.Alignment, MaxAlign);
  }
  أكد(آخری_سرا == ناپ && "didn't compute LastEnd correctly");
  أكد(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#جے_انت

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<میدان_2> Fields) {
#جے_بنا_تعریف NDEBUG
  // Do some simple precondition checks.
  {
    منطقي وچ_Fixed_اگیتر = سچ;
    size_t آخری_سرا = 0;
    لكل (خودکار &میدان_2 : Fields) {
      أكد(میدان_2.ناپ > 0 && "field of zero size");
      إذا (میدان_2.has_Fixed_وتھ()) {
        أكد(وچ_Fixed_اگیتر &&
               "fixed-offset fields are not a strict prefix of array");
        أكد(آخری_سرا <= میدان_2.وتھ &&
               "fixed-offset fields overlap or are not in order");
        آخری_سرا = میدان_2.get_سرا_وتھ();
        أكد(آخری_سرا > میدان_2.وتھ &&
               "overflow in fixed-offset end offset");
      } وإلا {
        وچ_Fixed_اگیتر = خطأ;
      }
    }
  }
#جے_انت

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  خودکار پہلا_Flexible = Fields.begin(), E = Fields.سرا();
  طالما (پہلا_Flexible != E && پہلا_Flexible->has_Fixed_وتھ()) {
    MaxAlign = std::max(MaxAlign, پہلا_Flexible->Alignment);
    ++پہلا_Flexible;
  }

  // If there are no flexible fields, we're done.
  إذا (پہلا_Flexible == E) {
    uint64_t ناپ = 0;
    إذا (!Fields.خالی_تھاواں())
      ناپ = Fields.پٹھ().get_سرا_وتھ();

#جے_بنا_تعریف NDEBUG
    checkValidLayout(Fields, ناپ, MaxAlign);
#جے_انت
    أرجع {ناپ, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_نمبر = 0;
    لكل (خودکار I = پہلا_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<فراغ*>(Unique_نمبر++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ترتیب_دیو(پہلا_Flexible, E,
                 [](ثابت میدان_2 *lhs, ثابت میدان_2 *rhs) -> صحيح {
    // Decreasing alignment.
    إذا (lhs->Alignment != rhs->Alignment)
      أرجع (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    إذا (lhs->ناپ != rhs->ناپ)
      أرجع (lhs->ناپ < rhs->ناپ ? 1 : -1);

    // Original order.
    خودکار lhs_نمبر = reinterpret_cast<uintptr_t>(lhs->Scratch);
    خودکار rhs_نمبر = reinterpret_cast<uintptr_t>(rhs->Scratch);
    إذا (lhs_نمبر != rhs_نمبر)
      أرجع (lhs_نمبر < rhs_نمبر ? -1 : 1);

    أرجع 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    منطقي HasPadding = خطأ;
    uint64_t آخری_سرا = 0;

    // Walk the fixed-offset fields.
    لكل (خودکار I = Fields.begin(); I != پہلا_Flexible; ++I) {
      أكد(I->has_Fixed_وتھ());
      إذا (آخری_سرا != I->وتھ) {
        HasPadding = سچ;
        اكسر;
      }
      آخری_سرا = I->get_سرا_وتھ();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    إذا (!HasPadding) {
      لكل (خودکار I = پہلا_Flexible; I != E; ++I) {
        خودکار وتھ = align_منزل(آخری_سرا, I->Alignment);
        إذا (آخری_سرا != وتھ) {
          HasPadding = سچ;
          اكسر;
        }
        I->وتھ = وتھ;
        آخری_سرا = I->get_سرا_وتھ();
      }
    }

    // If we already have a perfect layout, we're done.
    إذا (!HasPadding) {
#جے_بنا_تعریف NDEBUG
      checkValidLayout(Fields, آخری_سرا, MaxAlign);
#جے_انت
      أرجع {آخری_سرا, MaxAlign};
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
  بنية Alignment_قطار {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_ناپ;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_میدان *سر_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ساكن میدان_2 *get_اگلا(میدان_2 *Cur) {
      أرجع static_cast<میدان_2 *>(Cur->Scratch);
    }
  };
  Small_سمتیہ<Alignment_قطار, 8> FlexibleFieldsByAlignment;
  لكل (خودکار I = پہلا_Flexible; I != E; ) {
    خودکار سر_2 = I;
    خودکار Alignment = I->Alignment;

    uint64_t Min_ناپ = I->ناپ;
    خودکار آخری_وچ_قطار = I;
    لكل (++I; I != E && I->Alignment == Alignment; ++I) {
      آخری_وچ_قطار->Scratch = I;
      آخری_وچ_قطار = I;
      Min_ناپ = std::min(Min_ناپ, I->ناپ);
    }
    آخری_وچ_قطار->Scratch = خالی_سوچک;

    FlexibleFieldsByAlignment.ڈالو_پٹھ({Min_ناپ, سر_2, Alignment});
  }

#جے_بنا_تعریف NDEBUG
  // Verify that we set the queues up correctly.
  خودکار checkQueues = [&]{
    منطقي پہلا_قطار = سچ;
    Align آخری_قطار_Alignment;
    لكل (خودکار &قطار : FlexibleFieldsByAlignment) {
      أكد((پہلا_قطار || قطار.Alignment < آخری_قطار_Alignment) &&
             "bins not in order of descending alignment");
      آخری_قطار_Alignment = قطار.Alignment;
      پہلا_قطار = خطأ;

      أكد(قطار.سر_2 && "queue was empty");
      uint64_t آخری_ناپ = ~(uint64_t)0;
      لكل (خودکار I = قطار.سر_2; I; I = قطار.get_اگلا(I)) {
        أكد(I->Alignment == قطار.Alignment && "bad field in queue");
        أكد(I->ناپ <= آخری_ناپ && "queue not in descending size order");
        آخری_ناپ = I->ناپ;
      }
    }
  };
  checkQueues();
#جے_انت

  /// Helper function to remove a field from a queue.
  خودکار splice_From_قطار = [&](Alignment_قطار *قطار, میدان_2 *آخری, میدان_2 *Cur) {
    أكد(آخری ? قطار->get_اگلا(آخری) == Cur : قطار->سر_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    إذا (آخری) {
      آخری->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      إذا (!Cur->Scratch)
        قطار->Min_ناپ = آخری->ناپ;

    // Otherwise, replace the head.
    } وإلا {
      إذا (خودکار New_سر = قطار->get_اگلا(Cur))
        قطار->سر_2 = New_سر;

      // If we just emptied the queue, destroy its bin.
      وإلا
        FlexibleFieldsByAlignment.erase(قطار);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_سمتیہ<میدان_2, 16> Layout;
  Layout.reserve(Fields.ناپ_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t آخری_سرا = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  خودکار جوڑو_منزل_Layout = [&](Alignment_قطار *قطار, میدان_2 *آخری, میدان_2 *Cur,
                         uint64_t وتھ) -> منطقي {
    أكد(وتھ == align_منزل(آخری_سرا, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_قطار(قطار, آخری, Cur);

    // Add Cur to the layout.
    Layout.ڈالو_پٹھ(*Cur);
    Layout.پٹھ().وتھ = وتھ;
    آخری_سرا = Layout.پٹھ().get_سرا_وتھ();

    // Always return true so that we can be tail-called.
    أرجع سچ;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  خودکار try_جوڑو_Filler_From_قطار = [&](Alignment_قطار *قطار, uint64_t شروع_وتھ,
                                   std::optional<uint64_t> سرا_وتھ) -> منطقي {
    أكد(قطار->سر_2);
    أكد(شروع_وتھ == align_منزل(آخری_سرا, قطار->Alignment));
    أكد(!سرا_وتھ || شروع_وتھ < *سرا_وتھ);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    خودکار Max_Viable_ناپ =
      (سرا_وتھ ? *سرا_وتھ - شروع_وتھ : ~(uint64_t)0);
    إذا (قطار->Min_ناپ > Max_Viable_ناپ)
      أرجع خطأ;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    لكل (میدان_2 *Cur = قطار->سر_2, *آخری = خالی_سوچک; سچ;
           آخری = Cur, Cur = قطار->get_اگلا(Cur)) {
      أكد(Cur && "didn't find a match in queue despite its MinSize");
      إذا (Cur->ناپ <= Max_Viable_ناپ)
        أرجع جوڑو_منزل_Layout(قطار, آخری, Cur, شروع_وتھ);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  خودکار try_جوڑو_بہترین_میدان = [&](std::optional<uint64_t> Before_وتھ) -> منطقي {
    أكد(!Before_وتھ || آخری_سرا < *Before_وتھ);
    خودکار قطار_B = FlexibleFieldsByAlignment.begin();
    خودکار قطار_E = FlexibleFieldsByAlignment.سرا();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    خودکار پہلا_قطار_منزل_تلاش = قطار_B;
    لكل (; پہلا_قطار_منزل_تلاش != قطار_E; ++پہلا_قطار_منزل_تلاش) {
      إذا (isAligned(پہلا_قطار_منزل_تلاش->Alignment, آخری_سرا))
        اكسر;
    }

    uint64_t وتھ = آخری_سرا;
    طالما (سچ) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      لكل (خودکار قطار = پہلا_قطار_منزل_تلاش; قطار != قطار_E; ++قطار) {
        إذا (try_جوڑو_Filler_From_قطار(قطار, وتھ, Before_وتھ))
          أرجع سچ;
      }

      // Okay, we don't need to scan those again.
      قطار_E = پہلا_قطار_منزل_تلاش;

      // If we started from the first queue, we're done.
      إذا (پہلا_قطار_منزل_تلاش == قطار_B)
        أرجع خطأ;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --پہلا_قطار_منزل_تلاش;
      وتھ = align_منزل(آخری_سرا, پہلا_قطار_منزل_تلاش->Alignment);
      إذا (Before_وتھ && وتھ >= *Before_وتھ)
        أرجع خطأ;
      طالما (پہلا_قطار_منزل_تلاش != قطار_B &&
             وتھ == align_منزل(آخری_سرا, پہلا_قطار_منزل_تلاش[-1].Alignment))
        --پہلا_قطار_منزل_تلاش;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  لكل (خودکار I = Fields.begin(); I != پہلا_Flexible; ++I) {
    أكد(آخری_سرا <= I->وتھ);
    طالما (آخری_سرا != I->وتھ) {
      إذا (!try_جوڑو_بہترین_میدان(I->وتھ))
        اكسر;
    }
    Layout.ڈالو_پٹھ(*I);
    آخری_سرا = I->get_سرا_وتھ();
  }

#جے_بنا_تعریف NDEBUG
  checkQueues();
#جے_انت

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  طالما (!FlexibleFieldsByAlignment.خالی_تھاواں()) {
    منطقي Success = try_جوڑو_بہترین_میدان(std::nullopt);
    أكد(Success && "didn't find a field with no fixed limit?");
    (فراغ) Success;
  }

  // Copy the layout back into place.
  أكد(Layout.ناپ_2() == Fields.ناپ_2());
  حافظہ_نقل(Fields.انکڑے(), Layout.انکڑے(),
         Fields.ناپ_2() * حجم(Optimized_Struct_Layout_میدان));

#جے_بنا_تعریف NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, آخری_سرا, MaxAlign);
#جے_انت

  أرجع {آخری_سرا, MaxAlign};
}
