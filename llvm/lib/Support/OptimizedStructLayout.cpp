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

کارول نطاق llvm;

کارول ډګر_2 = Optimized_Struct_Layout_ډګر;

#که_نه_تعریف_شوی NDEBUG
ساكن فراغ checkValidLayout(ArrayRef<ډګر_2> Fields, uint64_t اندازه,
                             Align MaxAlign) {
  uint64_t وروستی_پای_ټکی = 0;
  Align ComputedMaxAlign;
  لكل (خپلکاری &ډګر_2 : Fields) {
    أكد(ډګر_2.has_Fixed_دننه_واټن() &&
           "didn't assign a fixed offset to field");
    أكد(isAligned(ډګر_2.Alignment, ډګر_2.دننه_واټن) &&
           "didn't assign a correctly-aligned offset to field");
    أكد(ډګر_2.دننه_واټن >= وروستی_پای_ټکی &&
           "didn't assign offsets in ascending order");
    وروستی_پای_ټکی = ډګر_2.get_پای_ټکی_دننه_واټن();
    أكد(ډګر_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(ډګر_2.Alignment, MaxAlign);
  }
  أكد(وروستی_پای_ټکی == اندازه && "didn't compute LastEnd correctly");
  أكد(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#د_که_پای

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<ډګر_2> Fields) {
#که_نه_تعریف_شوی NDEBUG
  // Do some simple precondition checks.
  {
    منطقي دننه_Fixed_مختاړی = ریښتیا;
    size_t وروستی_پای_ټکی = 0;
    لكل (خپلکاری &ډګر_2 : Fields) {
      أكد(ډګر_2.اندازه > 0 && "field of zero size");
      إذا (ډګر_2.has_Fixed_دننه_واټن()) {
        أكد(دننه_Fixed_مختاړی &&
               "fixed-offset fields are not a strict prefix of array");
        أكد(وروستی_پای_ټکی <= ډګر_2.دننه_واټن &&
               "fixed-offset fields overlap or are not in order");
        وروستی_پای_ټکی = ډګر_2.get_پای_ټکی_دننه_واټن();
        أكد(وروستی_پای_ټکی > ډګر_2.دننه_واټن &&
               "overflow in fixed-offset end offset");
      } وإلا {
        دننه_Fixed_مختاړی = خطأ;
      }
    }
  }
#د_که_پای

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  خپلکاری لومړی_Flexible = Fields.begin(), E = Fields.پای_ټکی();
  طالما (لومړی_Flexible != E && لومړی_Flexible->has_Fixed_دننه_واټن()) {
    MaxAlign = std::max(MaxAlign, لومړی_Flexible->Alignment);
    ++لومړی_Flexible;
  }

  // If there are no flexible fields, we're done.
  إذا (لومړی_Flexible == E) {
    uint64_t اندازه = 0;
    إذا (!Fields.تشه())
      اندازه = Fields.شا().get_پای_ټکی_دننه_واټن();

#که_نه_تعریف_شوی NDEBUG
    checkValidLayout(Fields, اندازه, MaxAlign);
#د_که_پای
    أرجع {اندازه, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_شمېره = 0;
    لكل (خپلکاری I = لومړی_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<فراغ*>(Unique_شمېره++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ترتیبول(لومړی_Flexible, E,
                 [](ثابت ډګر_2 *lhs, ثابت ډګر_2 *rhs) -> صحيح {
    // Decreasing alignment.
    إذا (lhs->Alignment != rhs->Alignment)
      أرجع (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    إذا (lhs->اندازه != rhs->اندازه)
      أرجع (lhs->اندازه < rhs->اندازه ? 1 : -1);

    // Original order.
    خپلکاری lhs_شمېره = reinterpret_cast<uintptr_t>(lhs->Scratch);
    خپلکاری rhs_شمېره = reinterpret_cast<uintptr_t>(rhs->Scratch);
    إذا (lhs_شمېره != rhs_شمېره)
      أرجع (lhs_شمېره < rhs_شمېره ? -1 : 1);

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
    uint64_t وروستی_پای_ټکی = 0;

    // Walk the fixed-offset fields.
    لكل (خپلکاری I = Fields.begin(); I != لومړی_Flexible; ++I) {
      أكد(I->has_Fixed_دننه_واټن());
      إذا (وروستی_پای_ټکی != I->دننه_واټن) {
        HasPadding = ریښتیا;
        اكسر;
      }
      وروستی_پای_ټکی = I->get_پای_ټکی_دننه_واټن();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    إذا (!HasPadding) {
      لكل (خپلکاری I = لومړی_Flexible; I != E; ++I) {
        خپلکاری دننه_واټن = align_منزل(وروستی_پای_ټکی, I->Alignment);
        إذا (وروستی_پای_ټکی != دننه_واټن) {
          HasPadding = ریښتیا;
          اكسر;
        }
        I->دننه_واټن = دننه_واټن;
        وروستی_پای_ټکی = I->get_پای_ټکی_دننه_واټن();
      }
    }

    // If we already have a perfect layout, we're done.
    إذا (!HasPadding) {
#که_نه_تعریف_شوی NDEBUG
      checkValidLayout(Fields, وروستی_پای_ټکی, MaxAlign);
#د_که_پای
      أرجع {وروستی_پای_ټکی, MaxAlign};
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
  بنية Alignment_کتار {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_اندازه;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_ډګر *سر_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ساكن ډګر_2 *get_بل(ډګر_2 *Cur) {
      أرجع static_cast<ډګر_2 *>(Cur->Scratch);
    }
  };
  Small_ویکټور<Alignment_کتار, 8> FlexibleFieldsByAlignment;
  لكل (خپلکاری I = لومړی_Flexible; I != E; ) {
    خپلکاری سر_2 = I;
    خپلکاری Alignment = I->Alignment;

    uint64_t Min_اندازه = I->اندازه;
    خپلکاری وروستی_دننه_کتار = I;
    لكل (++I; I != E && I->Alignment == Alignment; ++I) {
      وروستی_دننه_کتار->Scratch = I;
      وروستی_دننه_کتار = I;
      Min_اندازه = std::min(Min_اندازه, I->اندازه);
    }
    وروستی_دننه_کتار->Scratch = تش_اشاره;

    FlexibleFieldsByAlignment.ټېل_شا({Min_اندازه, سر_2, Alignment});
  }

#که_نه_تعریف_شوی NDEBUG
  // Verify that we set the queues up correctly.
  خپلکاری checkQueues = [&]{
    منطقي لومړی_کتار = ریښتیا;
    Align وروستی_کتار_Alignment;
    لكل (خپلکاری &کتار : FlexibleFieldsByAlignment) {
      أكد((لومړی_کتار || کتار.Alignment < وروستی_کتار_Alignment) &&
             "bins not in order of descending alignment");
      وروستی_کتار_Alignment = کتار.Alignment;
      لومړی_کتار = خطأ;

      أكد(کتار.سر_2 && "queue was empty");
      uint64_t وروستی_اندازه = ~(uint64_t)0;
      لكل (خپلکاری I = کتار.سر_2; I; I = کتار.get_بل(I)) {
        أكد(I->Alignment == کتار.Alignment && "bad field in queue");
        أكد(I->اندازه <= وروستی_اندازه && "queue not in descending size order");
        وروستی_اندازه = I->اندازه;
      }
    }
  };
  checkQueues();
#د_که_پای

  /// Helper function to remove a field from a queue.
  خپلکاری splice_From_کتار = [&](Alignment_کتار *کتار, ډګر_2 *وروستی, ډګر_2 *Cur) {
    أكد(وروستی ? کتار->get_بل(وروستی) == Cur : کتار->سر_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    إذا (وروستی) {
      وروستی->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      إذا (!Cur->Scratch)
        کتار->Min_اندازه = وروستی->اندازه;

    // Otherwise, replace the head.
    } وإلا {
      إذا (خپلکاری New_سر = کتار->get_بل(Cur))
        کتار->سر_2 = New_سر;

      // If we just emptied the queue, destroy its bin.
      وإلا
        FlexibleFieldsByAlignment.erase(کتار);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_ویکټور<ډګر_2, 16> Layout;
  Layout.reserve(Fields.اندازه_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t وروستی_پای_ټکی = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  خپلکاری جمع_کړه_منزل_Layout = [&](Alignment_کتار *کتار, ډګر_2 *وروستی, ډګر_2 *Cur,
                         uint64_t دننه_واټن) -> منطقي {
    أكد(دننه_واټن == align_منزل(وروستی_پای_ټکی, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_کتار(کتار, وروستی, Cur);

    // Add Cur to the layout.
    Layout.ټېل_شا(*Cur);
    Layout.شا().دننه_واټن = دننه_واټن;
    وروستی_پای_ټکی = Layout.شا().get_پای_ټکی_دننه_واټن();

    // Always return true so that we can be tail-called.
    أرجع ریښتیا;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  خپلکاری try_جمع_کړه_Filler_From_کتار = [&](Alignment_کتار *کتار, uint64_t پیل_دننه_واټن,
                                   std::optional<uint64_t> پای_ټکی_دننه_واټن) -> منطقي {
    أكد(کتار->سر_2);
    أكد(پیل_دننه_واټن == align_منزل(وروستی_پای_ټکی, کتار->Alignment));
    أكد(!پای_ټکی_دننه_واټن || پیل_دننه_واټن < *پای_ټکی_دننه_واټن);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    خپلکاری Max_Viable_اندازه =
      (پای_ټکی_دننه_واټن ? *پای_ټکی_دننه_واټن - پیل_دننه_واټن : ~(uint64_t)0);
    إذا (کتار->Min_اندازه > Max_Viable_اندازه)
      أرجع خطأ;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    لكل (ډګر_2 *Cur = کتار->سر_2, *وروستی = تش_اشاره; ریښتیا;
           وروستی = Cur, Cur = کتار->get_بل(Cur)) {
      أكد(Cur && "didn't find a match in queue despite its MinSize");
      إذا (Cur->اندازه <= Max_Viable_اندازه)
        أرجع جمع_کړه_منزل_Layout(کتار, وروستی, Cur, پیل_دننه_واټن);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  خپلکاری try_جمع_کړه_غوره_ډګر = [&](std::optional<uint64_t> Before_دننه_واټن) -> منطقي {
    أكد(!Before_دننه_واټن || وروستی_پای_ټکی < *Before_دننه_واټن);
    خپلکاری کتار_B = FlexibleFieldsByAlignment.begin();
    خپلکاری کتار_E = FlexibleFieldsByAlignment.پای_ټکی();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    خپلکاری لومړی_کتار_منزل_پلټنه = کتار_B;
    لكل (; لومړی_کتار_منزل_پلټنه != کتار_E; ++لومړی_کتار_منزل_پلټنه) {
      إذا (isAligned(لومړی_کتار_منزل_پلټنه->Alignment, وروستی_پای_ټکی))
        اكسر;
    }

    uint64_t دننه_واټن = وروستی_پای_ټکی;
    طالما (ریښتیا) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      لكل (خپلکاری کتار = لومړی_کتار_منزل_پلټنه; کتار != کتار_E; ++کتار) {
        إذا (try_جمع_کړه_Filler_From_کتار(کتار, دننه_واټن, Before_دننه_واټن))
          أرجع ریښتیا;
      }

      // Okay, we don't need to scan those again.
      کتار_E = لومړی_کتار_منزل_پلټنه;

      // If we started from the first queue, we're done.
      إذا (لومړی_کتار_منزل_پلټنه == کتار_B)
        أرجع خطأ;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --لومړی_کتار_منزل_پلټنه;
      دننه_واټن = align_منزل(وروستی_پای_ټکی, لومړی_کتار_منزل_پلټنه->Alignment);
      إذا (Before_دننه_واټن && دننه_واټن >= *Before_دننه_واټن)
        أرجع خطأ;
      طالما (لومړی_کتار_منزل_پلټنه != کتار_B &&
             دننه_واټن == align_منزل(وروستی_پای_ټکی, لومړی_کتار_منزل_پلټنه[-1].Alignment))
        --لومړی_کتار_منزل_پلټنه;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  لكل (خپلکاری I = Fields.begin(); I != لومړی_Flexible; ++I) {
    أكد(وروستی_پای_ټکی <= I->دننه_واټن);
    طالما (وروستی_پای_ټکی != I->دننه_واټن) {
      إذا (!try_جمع_کړه_غوره_ډګر(I->دننه_واټن))
        اكسر;
    }
    Layout.ټېل_شا(*I);
    وروستی_پای_ټکی = I->get_پای_ټکی_دننه_واټن();
  }

#که_نه_تعریف_شوی NDEBUG
  checkQueues();
#د_که_پای

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  طالما (!FlexibleFieldsByAlignment.تشه()) {
    منطقي Success = try_جمع_کړه_غوره_ډګر(std::nullopt);
    أكد(Success && "didn't find a field with no fixed limit?");
    (فراغ) Success;
  }

  // Copy the layout back into place.
  أكد(Layout.اندازه_2() == Fields.اندازه_2());
  حافظه_نقل(Fields.معلومات(), Layout.معلومات(),
         Fields.اندازه_2() * حجم(Optimized_Struct_Layout_ډګر));

#که_نه_تعریف_شوی NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, وروستی_پای_ټکی, MaxAlign);
#د_که_پای

  أرجع {وروستی_پای_ټکی, MaxAlign};
}
