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

استعمال نطاق llvm;

استعمال ميدان_2 = Optimized_Struct_Layout_ميدان;

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
ساكن فراغ checkValidLayout(ArrayRef<ميدان_2> Fields, uint64_t سائيز,
                             Align MaxAlign) {
  uint64_t آخري_پڇاڙي = 0;
  Align ComputedMaxAlign;
  لكل (خودڪار &ميدان_2 : Fields) {
    أكد(ميدان_2.has_Fixed_وٿي() &&
           "didn't assign a fixed offset to field");
    أكد(isAligned(ميدان_2.Alignment, ميدان_2.وٿي) &&
           "didn't assign a correctly-aligned offset to field");
    أكد(ميدان_2.وٿي >= آخري_پڇاڙي &&
           "didn't assign offsets in ascending order");
    آخري_پڇاڙي = ميدان_2.get_پڇاڙي_وٿي();
    أكد(ميدان_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(ميدان_2.Alignment, MaxAlign);
  }
  أكد(آخري_پڇاڙي == سائيز && "didn't compute LastEnd correctly");
  أكد(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#جيڪڏهن_پورو

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<ميدان_2> Fields) {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  // Do some simple precondition checks.
  {
    منطقي اندريون_Fixed_اڳياڙي = سچ;
    size_t آخري_پڇاڙي = 0;
    لكل (خودڪار &ميدان_2 : Fields) {
      أكد(ميدان_2.سائيز > 0 && "field of zero size");
      إذا (ميدان_2.has_Fixed_وٿي()) {
        أكد(اندريون_Fixed_اڳياڙي &&
               "fixed-offset fields are not a strict prefix of array");
        أكد(آخري_پڇاڙي <= ميدان_2.وٿي &&
               "fixed-offset fields overlap or are not in order");
        آخري_پڇاڙي = ميدان_2.get_پڇاڙي_وٿي();
        أكد(آخري_پڇاڙي > ميدان_2.وٿي &&
               "overflow in fixed-offset end offset");
      } وإلا {
        اندريون_Fixed_اڳياڙي = خطأ;
      }
    }
  }
#جيڪڏهن_پورو

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  خودڪار پهريون_Flexible = Fields.begin(), E = Fields.پڇاڙي();
  طالما (پهريون_Flexible != E && پهريون_Flexible->has_Fixed_وٿي()) {
    MaxAlign = std::max(MaxAlign, پهريون_Flexible->Alignment);
    ++پهريون_Flexible;
  }

  // If there are no flexible fields, we're done.
  إذا (پهريون_Flexible == E) {
    uint64_t سائيز = 0;
    إذا (!Fields.خالي_جايون())
      سائيز = Fields.پٺ().get_پڇاڙي_وٿي();

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
    checkValidLayout(Fields, سائيز, MaxAlign);
#جيڪڏهن_پورو
    أرجع {سائيز, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_عدد = 0;
    لكل (خودڪار I = پهريون_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<فراغ*>(Unique_عدد++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ترتيب_ڏيو(پهريون_Flexible, E,
                 [](ثابت ميدان_2 *lhs, ثابت ميدان_2 *rhs) -> صحيح {
    // Decreasing alignment.
    إذا (lhs->Alignment != rhs->Alignment)
      أرجع (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    إذا (lhs->سائيز != rhs->سائيز)
      أرجع (lhs->سائيز < rhs->سائيز ? 1 : -1);

    // Original order.
    خودڪار lhs_عدد = reinterpret_cast<uintptr_t>(lhs->Scratch);
    خودڪار rhs_عدد = reinterpret_cast<uintptr_t>(rhs->Scratch);
    إذا (lhs_عدد != rhs_عدد)
      أرجع (lhs_عدد < rhs_عدد ? -1 : 1);

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
    uint64_t آخري_پڇاڙي = 0;

    // Walk the fixed-offset fields.
    لكل (خودڪار I = Fields.begin(); I != پهريون_Flexible; ++I) {
      أكد(I->has_Fixed_وٿي());
      إذا (آخري_پڇاڙي != I->وٿي) {
        HasPadding = سچ;
        اكسر;
      }
      آخري_پڇاڙي = I->get_پڇاڙي_وٿي();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    إذا (!HasPadding) {
      لكل (خودڪار I = پهريون_Flexible; I != E; ++I) {
        خودڪار وٿي = align_منزل(آخري_پڇاڙي, I->Alignment);
        إذا (آخري_پڇاڙي != وٿي) {
          HasPadding = سچ;
          اكسر;
        }
        I->وٿي = وٿي;
        آخري_پڇاڙي = I->get_پڇاڙي_وٿي();
      }
    }

    // If we already have a perfect layout, we're done.
    إذا (!HasPadding) {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
      checkValidLayout(Fields, آخري_پڇاڙي, MaxAlign);
#جيڪڏهن_پورو
      أرجع {آخري_پڇاڙي, MaxAlign};
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
    uint64_t Min_سائيز;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_ميدان *سر_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ساكن ميدان_2 *get_ايندڙ(ميدان_2 *Cur) {
      أرجع static_cast<ميدان_2 *>(Cur->Scratch);
    }
  };
  Small_ويڪٽر<Alignment_قطار, 8> FlexibleFieldsByAlignment;
  لكل (خودڪار I = پهريون_Flexible; I != E; ) {
    خودڪار سر_2 = I;
    خودڪار Alignment = I->Alignment;

    uint64_t Min_سائيز = I->سائيز;
    خودڪار آخري_اندريون_قطار = I;
    لكل (++I; I != E && I->Alignment == Alignment; ++I) {
      آخري_اندريون_قطار->Scratch = I;
      آخري_اندريون_قطار = I;
      Min_سائيز = std::min(Min_سائيز, I->سائيز);
    }
    آخري_اندريون_قطار->Scratch = خالي_اشارو;

    FlexibleFieldsByAlignment.ڌڪو_پٺ({Min_سائيز, سر_2, Alignment});
  }

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  // Verify that we set the queues up correctly.
  خودڪار checkQueues = [&]{
    منطقي پهريون_قطار = سچ;
    Align آخري_قطار_Alignment;
    لكل (خودڪار &قطار : FlexibleFieldsByAlignment) {
      أكد((پهريون_قطار || قطار.Alignment < آخري_قطار_Alignment) &&
             "bins not in order of descending alignment");
      آخري_قطار_Alignment = قطار.Alignment;
      پهريون_قطار = خطأ;

      أكد(قطار.سر_2 && "queue was empty");
      uint64_t آخري_سائيز = ~(uint64_t)0;
      لكل (خودڪار I = قطار.سر_2; I; I = قطار.get_ايندڙ(I)) {
        أكد(I->Alignment == قطار.Alignment && "bad field in queue");
        أكد(I->سائيز <= آخري_سائيز && "queue not in descending size order");
        آخري_سائيز = I->سائيز;
      }
    }
  };
  checkQueues();
#جيڪڏهن_پورو

  /// Helper function to remove a field from a queue.
  خودڪار splice_From_قطار = [&](Alignment_قطار *قطار, ميدان_2 *آخري, ميدان_2 *Cur) {
    أكد(آخري ? قطار->get_ايندڙ(آخري) == Cur : قطار->سر_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    إذا (آخري) {
      آخري->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      إذا (!Cur->Scratch)
        قطار->Min_سائيز = آخري->سائيز;

    // Otherwise, replace the head.
    } وإلا {
      إذا (خودڪار New_سر = قطار->get_ايندڙ(Cur))
        قطار->سر_2 = New_سر;

      // If we just emptied the queue, destroy its bin.
      وإلا
        FlexibleFieldsByAlignment.erase(قطار);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_ويڪٽر<ميدان_2, 16> Layout;
  Layout.reserve(Fields.سائيز_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t آخري_پڇاڙي = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  خودڪار جوڙيو_منزل_Layout = [&](Alignment_قطار *قطار, ميدان_2 *آخري, ميدان_2 *Cur,
                         uint64_t وٿي) -> منطقي {
    أكد(وٿي == align_منزل(آخري_پڇاڙي, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_قطار(قطار, آخري, Cur);

    // Add Cur to the layout.
    Layout.ڌڪو_پٺ(*Cur);
    Layout.پٺ().وٿي = وٿي;
    آخري_پڇاڙي = Layout.پٺ().get_پڇاڙي_وٿي();

    // Always return true so that we can be tail-called.
    أرجع سچ;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  خودڪار try_جوڙيو_Filler_From_قطار = [&](Alignment_قطار *قطار, uint64_t شروعات_وٿي,
                                   std::optional<uint64_t> پڇاڙي_وٿي) -> منطقي {
    أكد(قطار->سر_2);
    أكد(شروعات_وٿي == align_منزل(آخري_پڇاڙي, قطار->Alignment));
    أكد(!پڇاڙي_وٿي || شروعات_وٿي < *پڇاڙي_وٿي);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    خودڪار Max_Viable_سائيز =
      (پڇاڙي_وٿي ? *پڇاڙي_وٿي - شروعات_وٿي : ~(uint64_t)0);
    إذا (قطار->Min_سائيز > Max_Viable_سائيز)
      أرجع خطأ;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    لكل (ميدان_2 *Cur = قطار->سر_2, *آخري = خالي_اشارو; سچ;
           آخري = Cur, Cur = قطار->get_ايندڙ(Cur)) {
      أكد(Cur && "didn't find a match in queue despite its MinSize");
      إذا (Cur->سائيز <= Max_Viable_سائيز)
        أرجع جوڙيو_منزل_Layout(قطار, آخري, Cur, شروعات_وٿي);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  خودڪار try_جوڙيو_بهترين_ميدان = [&](std::optional<uint64_t> Before_وٿي) -> منطقي {
    أكد(!Before_وٿي || آخري_پڇاڙي < *Before_وٿي);
    خودڪار قطار_B = FlexibleFieldsByAlignment.begin();
    خودڪار قطار_E = FlexibleFieldsByAlignment.پڇاڙي();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    خودڪار پهريون_قطار_منزل_ڳولا = قطار_B;
    لكل (; پهريون_قطار_منزل_ڳولا != قطار_E; ++پهريون_قطار_منزل_ڳولا) {
      إذا (isAligned(پهريون_قطار_منزل_ڳولا->Alignment, آخري_پڇاڙي))
        اكسر;
    }

    uint64_t وٿي = آخري_پڇاڙي;
    طالما (سچ) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      لكل (خودڪار قطار = پهريون_قطار_منزل_ڳولا; قطار != قطار_E; ++قطار) {
        إذا (try_جوڙيو_Filler_From_قطار(قطار, وٿي, Before_وٿي))
          أرجع سچ;
      }

      // Okay, we don't need to scan those again.
      قطار_E = پهريون_قطار_منزل_ڳولا;

      // If we started from the first queue, we're done.
      إذا (پهريون_قطار_منزل_ڳولا == قطار_B)
        أرجع خطأ;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --پهريون_قطار_منزل_ڳولا;
      وٿي = align_منزل(آخري_پڇاڙي, پهريون_قطار_منزل_ڳولا->Alignment);
      إذا (Before_وٿي && وٿي >= *Before_وٿي)
        أرجع خطأ;
      طالما (پهريون_قطار_منزل_ڳولا != قطار_B &&
             وٿي == align_منزل(آخري_پڇاڙي, پهريون_قطار_منزل_ڳولا[-1].Alignment))
        --پهريون_قطار_منزل_ڳولا;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  لكل (خودڪار I = Fields.begin(); I != پهريون_Flexible; ++I) {
    أكد(آخري_پڇاڙي <= I->وٿي);
    طالما (آخري_پڇاڙي != I->وٿي) {
      إذا (!try_جوڙيو_بهترين_ميدان(I->وٿي))
        اكسر;
    }
    Layout.ڌڪو_پٺ(*I);
    آخري_پڇاڙي = I->get_پڇاڙي_وٿي();
  }

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  checkQueues();
#جيڪڏهن_پورو

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  طالما (!FlexibleFieldsByAlignment.خالي_جايون()) {
    منطقي Success = try_جوڙيو_بهترين_ميدان(std::nullopt);
    أكد(Success && "didn't find a field with no fixed limit?");
    (فراغ) Success;
  }

  // Copy the layout back into place.
  أكد(Layout.سائيز_2() == Fields.سائيز_2());
  يادگيري_نقل(Fields.معلومات(), Layout.معلومات(),
         Fields.سائيز_2() * حجم(Optimized_Struct_Layout_ميدان));

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, آخري_پڇاڙي, MaxAlign);
#جيڪڏهن_پورو

  أرجع {آخري_پڇاڙي, MaxAlign};
}
