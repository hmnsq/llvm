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

#شامل "llvm/Support/OptimizedStructLayout.h"
#شامل <optional>

استعمال نامخلا llvm;

استعمال میدان_2 = Optimized_Struct_Layout_میدان;

#اگر_غیر_تعریف_شدہ NDEBUG
ساکن خالی checkValidLayout(ArrayRef<میدان_2> Fields, uint64_t سائز,
                             Align MaxAlign) {
  uint64_t آخری_انتہا = 0;
  Align ComputedMaxAlign;
  برائے (خودکار &میدان_2 : Fields) {
    أكد(میدان_2.has_Fixed_انحراف() &&
           "didn't assign a fixed offset to field");
    أكد(isAligned(میدان_2.Alignment, میدان_2.انحراف) &&
           "didn't assign a correctly-aligned offset to field");
    أكد(میدان_2.انحراف >= آخری_انتہا &&
           "didn't assign offsets in ascending order");
    آخری_انتہا = میدان_2.get_انتہا_انحراف();
    أكد(میدان_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(میدان_2.Alignment, MaxAlign);
  }
  أكد(آخری_انتہا == سائز && "didn't compute LastEnd correctly");
  أكد(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#اگر_ختم

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<میدان_2> Fields) {
#اگر_غیر_تعریف_شدہ NDEBUG
  // Do some simple precondition checks.
  {
    منطقی اندر_Fixed_سابقہ = سچ;
    size_t آخری_انتہا = 0;
    برائے (خودکار &میدان_2 : Fields) {
      أكد(میدان_2.سائز > 0 && "field of zero size");
      اگر (میدان_2.has_Fixed_انحراف()) {
        أكد(اندر_Fixed_سابقہ &&
               "fixed-offset fields are not a strict prefix of array");
        أكد(آخری_انتہا <= میدان_2.انحراف &&
               "fixed-offset fields overlap or are not in order");
        آخری_انتہا = میدان_2.get_انتہا_انحراف();
        أكد(آخری_انتہا > میدان_2.انحراف &&
               "overflow in fixed-offset end offset");
      } ورنہ {
        اندر_Fixed_سابقہ = جھوٹ;
      }
    }
  }
#اگر_ختم

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  خودکار پہلا_Flexible = Fields.begin(), E = Fields.انتہا();
  جب (پہلا_Flexible != E && پہلا_Flexible->has_Fixed_انحراف()) {
    MaxAlign = std::max(MaxAlign, پہلا_Flexible->Alignment);
    ++پہلا_Flexible;
  }

  // If there are no flexible fields, we're done.
  اگر (پہلا_Flexible == E) {
    uint64_t سائز = 0;
    اگر (!Fields.خالی_خانے())
      سائز = Fields.عقب().get_انتہا_انحراف();

#اگر_غیر_تعریف_شدہ NDEBUG
    checkValidLayout(Fields, سائز, MaxAlign);
#اگر_ختم
    واپس {سائز, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_نمبر = 0;
    برائے (خودکار I = پہلا_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<خالی*>(Unique_نمبر++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_ترتیب_دو(پہلا_Flexible, E,
                 [](ثابت میدان_2 *lhs, ثابت میدان_2 *rhs) -> صحیح {
    // Decreasing alignment.
    اگر (lhs->Alignment != rhs->Alignment)
      واپس (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    اگر (lhs->سائز != rhs->سائز)
      واپس (lhs->سائز < rhs->سائز ? 1 : -1);

    // Original order.
    خودکار lhs_نمبر = reinterpret_cast<uintptr_t>(lhs->Scratch);
    خودکار rhs_نمبر = reinterpret_cast<uintptr_t>(rhs->Scratch);
    اگر (lhs_نمبر != rhs_نمبر)
      واپس (lhs_نمبر < rhs_نمبر ? -1 : 1);

    واپس 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    منطقی HasPadding = جھوٹ;
    uint64_t آخری_انتہا = 0;

    // Walk the fixed-offset fields.
    برائے (خودکار I = Fields.begin(); I != پہلا_Flexible; ++I) {
      أكد(I->has_Fixed_انحراف());
      اگر (آخری_انتہا != I->انحراف) {
        HasPadding = سچ;
        توڑو;
      }
      آخری_انتہا = I->get_انتہا_انحراف();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    اگر (!HasPadding) {
      برائے (خودکار I = پہلا_Flexible; I != E; ++I) {
        خودکار انحراف = align_مقصد(آخری_انتہا, I->Alignment);
        اگر (آخری_انتہا != انحراف) {
          HasPadding = سچ;
          توڑو;
        }
        I->انحراف = انحراف;
        آخری_انتہا = I->get_انتہا_انحراف();
      }
    }

    // If we already have a perfect layout, we're done.
    اگر (!HasPadding) {
#اگر_غیر_تعریف_شدہ NDEBUG
      checkValidLayout(Fields, آخری_انتہا, MaxAlign);
#اگر_ختم
      واپس {آخری_انتہا, MaxAlign};
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
  ساخت Alignment_قطار {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_سائز;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_میدان *سر_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    ساکن میدان_2 *get_اگلا(میدان_2 *Cur) {
      واپس static_cast<میدان_2 *>(Cur->Scratch);
    }
  };
  Small_سمتیہ<Alignment_قطار, 8> FlexibleFieldsByAlignment;
  برائے (خودکار I = پہلا_Flexible; I != E; ) {
    خودکار سر_2 = I;
    خودکار Alignment = I->Alignment;

    uint64_t Min_سائز = I->سائز;
    خودکار آخری_اندر_قطار = I;
    برائے (++I; I != E && I->Alignment == Alignment; ++I) {
      آخری_اندر_قطار->Scratch = I;
      آخری_اندر_قطار = I;
      Min_سائز = std::min(Min_سائز, I->سائز);
    }
    آخری_اندر_قطار->Scratch = خالی_اشاریہ;

    FlexibleFieldsByAlignment.ڈالو_عقب({Min_سائز, سر_2, Alignment});
  }

#اگر_غیر_تعریف_شدہ NDEBUG
  // Verify that we set the queues up correctly.
  خودکار checkQueues = [&]{
    منطقی پہلا_قطار = سچ;
    Align آخری_قطار_Alignment;
    برائے (خودکار &قطار : FlexibleFieldsByAlignment) {
      أكد((پہلا_قطار || قطار.Alignment < آخری_قطار_Alignment) &&
             "bins not in order of descending alignment");
      آخری_قطار_Alignment = قطار.Alignment;
      پہلا_قطار = جھوٹ;

      أكد(قطار.سر_2 && "queue was empty");
      uint64_t آخری_سائز = ~(uint64_t)0;
      برائے (خودکار I = قطار.سر_2; I; I = قطار.get_اگلا(I)) {
        أكد(I->Alignment == قطار.Alignment && "bad field in queue");
        أكد(I->سائز <= آخری_سائز && "queue not in descending size order");
        آخری_سائز = I->سائز;
      }
    }
  };
  checkQueues();
#اگر_ختم

  /// Helper function to remove a field from a queue.
  خودکار splice_From_قطار = [&](Alignment_قطار *قطار, میدان_2 *آخری, میدان_2 *Cur) {
    أكد(آخری ? قطار->get_اگلا(آخری) == Cur : قطار->سر_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    اگر (آخری) {
      آخری->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      اگر (!Cur->Scratch)
        قطار->Min_سائز = آخری->سائز;

    // Otherwise, replace the head.
    } ورنہ {
      اگر (خودکار New_سر = قطار->get_اگلا(Cur))
        قطار->سر_2 = New_سر;

      // If we just emptied the queue, destroy its bin.
      ورنہ
        FlexibleFieldsByAlignment.erase(قطار);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_سمتیہ<میدان_2, 16> Layout;
  Layout.reserve(Fields.سائز_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t آخری_انتہا = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  خودکار جوڑو_مقصد_Layout = [&](Alignment_قطار *قطار, میدان_2 *آخری, میدان_2 *Cur,
                         uint64_t انحراف) -> منطقی {
    أكد(انحراف == align_مقصد(آخری_انتہا, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_قطار(قطار, آخری, Cur);

    // Add Cur to the layout.
    Layout.ڈالو_عقب(*Cur);
    Layout.عقب().انحراف = انحراف;
    آخری_انتہا = Layout.عقب().get_انتہا_انحراف();

    // Always return true so that we can be tail-called.
    واپس سچ;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  خودکار try_جوڑو_Filler_From_قطار = [&](Alignment_قطار *قطار, uint64_t آغاز_انحراف,
                                   std::optional<uint64_t> انتہا_انحراف) -> منطقی {
    أكد(قطار->سر_2);
    أكد(آغاز_انحراف == align_مقصد(آخری_انتہا, قطار->Alignment));
    أكد(!انتہا_انحراف || آغاز_انحراف < *انتہا_انحراف);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    خودکار Max_Viable_سائز =
      (انتہا_انحراف ? *انتہا_انحراف - آغاز_انحراف : ~(uint64_t)0);
    اگر (قطار->Min_سائز > Max_Viable_سائز)
      واپس جھوٹ;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    برائے (میدان_2 *Cur = قطار->سر_2, *آخری = خالی_اشاریہ; سچ;
           آخری = Cur, Cur = قطار->get_اگلا(Cur)) {
      أكد(Cur && "didn't find a match in queue despite its MinSize");
      اگر (Cur->سائز <= Max_Viable_سائز)
        واپس جوڑو_مقصد_Layout(قطار, آخری, Cur, آغاز_انحراف);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  خودکار try_جوڑو_بہترین_میدان = [&](std::optional<uint64_t> Before_انحراف) -> منطقی {
    أكد(!Before_انحراف || آخری_انتہا < *Before_انحراف);
    خودکار قطار_B = FlexibleFieldsByAlignment.begin();
    خودکار قطار_E = FlexibleFieldsByAlignment.انتہا();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    خودکار پہلا_قطار_مقصد_تلاش = قطار_B;
    برائے (; پہلا_قطار_مقصد_تلاش != قطار_E; ++پہلا_قطار_مقصد_تلاش) {
      اگر (isAligned(پہلا_قطار_مقصد_تلاش->Alignment, آخری_انتہا))
        توڑو;
    }

    uint64_t انحراف = آخری_انتہا;
    جب (سچ) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      برائے (خودکار قطار = پہلا_قطار_مقصد_تلاش; قطار != قطار_E; ++قطار) {
        اگر (try_جوڑو_Filler_From_قطار(قطار, انحراف, Before_انحراف))
          واپس سچ;
      }

      // Okay, we don't need to scan those again.
      قطار_E = پہلا_قطار_مقصد_تلاش;

      // If we started from the first queue, we're done.
      اگر (پہلا_قطار_مقصد_تلاش == قطار_B)
        واپس جھوٹ;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --پہلا_قطار_مقصد_تلاش;
      انحراف = align_مقصد(آخری_انتہا, پہلا_قطار_مقصد_تلاش->Alignment);
      اگر (Before_انحراف && انحراف >= *Before_انحراف)
        واپس جھوٹ;
      جب (پہلا_قطار_مقصد_تلاش != قطار_B &&
             انحراف == align_مقصد(آخری_انتہا, پہلا_قطار_مقصد_تلاش[-1].Alignment))
        --پہلا_قطار_مقصد_تلاش;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  برائے (خودکار I = Fields.begin(); I != پہلا_Flexible; ++I) {
    أكد(آخری_انتہا <= I->انحراف);
    جب (آخری_انتہا != I->انحراف) {
      اگر (!try_جوڑو_بہترین_میدان(I->انحراف))
        توڑو;
    }
    Layout.ڈالو_عقب(*I);
    آخری_انتہا = I->get_انتہا_انحراف();
  }

#اگر_غیر_تعریف_شدہ NDEBUG
  checkQueues();
#اگر_ختم

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  جب (!FlexibleFieldsByAlignment.خالی_خانے()) {
    منطقی Success = try_جوڑو_بہترین_میدان(std::nullopt);
    أكد(Success && "didn't find a field with no fixed limit?");
    (خالی) Success;
  }

  // Copy the layout back into place.
  أكد(Layout.سائز_2() == Fields.سائز_2());
  حافظہ_نقل(Fields.ڈیٹا(), Layout.ڈیٹا(),
         Fields.سائز_2() * حجم(Optimized_Struct_Layout_میدان));

#اگر_غیر_تعریف_شدہ NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, آخری_انتہا, MaxAlign);
#اگر_ختم

  واپس {آخری_انتہا, MaxAlign};
}
