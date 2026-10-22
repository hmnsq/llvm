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

#అన్తర్భావ "llvm/Support/OptimizedStructLayout.h"
#అన్తర్భావ <optional>

ఉపయోగించి నామాకాశ llvm;

ఉపయోగించి క్షేత్రం_2 = Optimized_Struct_Layout_క్షేత్రం;

#నిర్వచితం_కాకపోతే NDEBUG
స్థిర శూన్య checkValidLayout(ArrayRef<క్షేత్రం_2> Fields, uint64_t కొలత,
                             Align MaxAlign) {
  uint64_t చివరి_చివర = 0;
  Align ComputedMaxAlign;
  కొరకు (స్వయంచాలక &క్షేత్రం_2 : Fields) {
    ప్రతిజ్ఞా(క్షేత్రం_2.has_Fixed_స్థానభ్రంశం() &&
           "didn't assign a fixed offset to field");
    ప్రతిజ్ఞా(isAligned(క్షేత్రం_2.Alignment, క్షేత్రం_2.స్థానభ్రంశం) &&
           "didn't assign a correctly-aligned offset to field");
    ప్రతిజ్ఞా(క్షేత్రం_2.స్థానభ్రంశం >= చివరి_చివర &&
           "didn't assign offsets in ascending order");
    చివరి_చివర = క్షేత్రం_2.get_చివర_స్థానభ్రంశం();
    ప్రతిజ్ఞా(క్షేత్రం_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(క్షేత్రం_2.Alignment, MaxAlign);
  }
  ప్రతిజ్ఞా(చివరి_చివర == కొలత && "didn't compute LastEnd correctly");
  ప్రతిజ్ఞా(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#అయితే_ముగింపు

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<క్షేత్రం_2> Fields) {
#నిర్వచితం_కాకపోతే NDEBUG
  // Do some simple precondition checks.
  {
    తార్కిక లోపల_Fixed_ఉపసర్గ = నిజం;
    size_t చివరి_చివర = 0;
    కొరకు (స్వయంచాలక &క్షేత్రం_2 : Fields) {
      ప్రతిజ్ఞా(క్షేత్రం_2.కొలత > 0 && "field of zero size");
      ఒకవేళ (క్షేత్రం_2.has_Fixed_స్థానభ్రంశం()) {
        ప్రతిజ్ఞా(లోపల_Fixed_ఉపసర్గ &&
               "fixed-offset fields are not a strict prefix of array");
        ప్రతిజ్ఞా(చివరి_చివర <= క్షేత్రం_2.స్థానభ్రంశం &&
               "fixed-offset fields overlap or are not in order");
        చివరి_చివర = క్షేత్రం_2.get_చివర_స్థానభ్రంశం();
        ప్రతిజ్ఞా(చివరి_చివర > క్షేత్రం_2.స్థానభ్రంశం &&
               "overflow in fixed-offset end offset");
      } లేకపోతే {
        లోపల_Fixed_ఉపసర్గ = అబద్ధం;
      }
    }
  }
#అయితే_ముగింపు

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  స్వయంచాలక మొదటి_Flexible = Fields.begin(), E = Fields.చివర();
  అయితే (మొదటి_Flexible != E && మొదటి_Flexible->has_Fixed_స్థానభ్రంశం()) {
    MaxAlign = std::max(MaxAlign, మొదటి_Flexible->Alignment);
    ++మొదటి_Flexible;
  }

  // If there are no flexible fields, we're done.
  ఒకవేళ (మొదటి_Flexible == E) {
    uint64_t కొలత = 0;
    ఒకవేళ (!Fields.ఖాళీలు())
      కొలత = Fields.వెనుక().get_చివర_స్థానభ్రంశం();

#నిర్వచితం_కాకపోతే NDEBUG
    checkValidLayout(Fields, కొలత, MaxAlign);
#అయితే_ముగింపు
    తిరిగి {కొలత, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_సంఖ్య = 0;
    కొరకు (స్వయంచాలక I = మొదటి_Flexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<శూన్య*>(Unique_సంఖ్య++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_క్రమీకరించు(మొదటి_Flexible, E,
                 [](స్థిరరాశి క్షేత్రం_2 *lhs, స్థిరరాశి క్షేత్రం_2 *rhs) -> పూర్ణాంకం {
    // Decreasing alignment.
    ఒకవేళ (lhs->Alignment != rhs->Alignment)
      తిరిగి (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    ఒకవేళ (lhs->కొలత != rhs->కొలత)
      తిరిగి (lhs->కొలత < rhs->కొలత ? 1 : -1);

    // Original order.
    స్వయంచాలక lhs_సంఖ్య = reinterpret_cast<uintptr_t>(lhs->Scratch);
    స్వయంచాలక rhs_సంఖ్య = reinterpret_cast<uintptr_t>(rhs->Scratch);
    ఒకవేళ (lhs_సంఖ్య != rhs_సంఖ్య)
      తిరిగి (lhs_సంఖ్య < rhs_సంఖ్య ? -1 : 1);

    తిరిగి 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    తార్కిక HasPadding = అబద్ధం;
    uint64_t చివరి_చివర = 0;

    // Walk the fixed-offset fields.
    కొరకు (స్వయంచాలక I = Fields.begin(); I != మొదటి_Flexible; ++I) {
      ప్రతిజ్ఞా(I->has_Fixed_స్థానభ్రంశం());
      ఒకవేళ (చివరి_చివర != I->స్థానభ్రంశం) {
        HasPadding = నిజం;
        ఆపు;
      }
      చివరి_చివర = I->get_చివర_స్థానభ్రంశం();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    ఒకవేళ (!HasPadding) {
      కొరకు (స్వయంచాలక I = మొదటి_Flexible; I != E; ++I) {
        స్వయంచాలక స్థానభ్రంశం = align_గమ్యం(చివరి_చివర, I->Alignment);
        ఒకవేళ (చివరి_చివర != స్థానభ్రంశం) {
          HasPadding = నిజం;
          ఆపు;
        }
        I->స్థానభ్రంశం = స్థానభ్రంశం;
        చివరి_చివర = I->get_చివర_స్థానభ్రంశం();
      }
    }

    // If we already have a perfect layout, we're done.
    ఒకవేళ (!HasPadding) {
#నిర్వచితం_కాకపోతే NDEBUG
      checkValidLayout(Fields, చివరి_చివర, MaxAlign);
#అయితే_ముగింపు
      తిరిగి {చివరి_చివర, MaxAlign};
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
  నిర్మాణం Alignment_వరుస {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_కొలత;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_క్షేత్రం *తల_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    స్థిర క్షేత్రం_2 *get_తదుపరి(క్షేత్రం_2 *Cur) {
      తిరిగి static_cast<క్షేత్రం_2 *>(Cur->Scratch);
    }
  };
  Small_సదిశ<Alignment_వరుస, 8> FlexibleFieldsByAlignment;
  కొరకు (స్వయంచాలక I = మొదటి_Flexible; I != E; ) {
    స్వయంచాలక తల_2 = I;
    స్వయంచాలక Alignment = I->Alignment;

    uint64_t Min_కొలత = I->కొలత;
    స్వయంచాలక చివరి_లోపల_వరుస = I;
    కొరకు (++I; I != E && I->Alignment == Alignment; ++I) {
      చివరి_లోపల_వరుస->Scratch = I;
      చివరి_లోపల_వరుస = I;
      Min_కొలత = std::min(Min_కొలత, I->కొలత);
    }
    చివరి_లోపల_వరుస->Scratch = శూన్యసూచిక;

    FlexibleFieldsByAlignment.నెట్టు_వెనుక({Min_కొలత, తల_2, Alignment});
  }

#నిర్వచితం_కాకపోతే NDEBUG
  // Verify that we set the queues up correctly.
  స్వయంచాలక checkQueues = [&]{
    తార్కిక మొదటి_వరుస = నిజం;
    Align చివరి_వరుస_Alignment;
    కొరకు (స్వయంచాలక &వరుస : FlexibleFieldsByAlignment) {
      ప్రతిజ్ఞా((మొదటి_వరుస || వరుస.Alignment < చివరి_వరుస_Alignment) &&
             "bins not in order of descending alignment");
      చివరి_వరుస_Alignment = వరుస.Alignment;
      మొదటి_వరుస = అబద్ధం;

      ప్రతిజ్ఞా(వరుస.తల_2 && "queue was empty");
      uint64_t చివరి_కొలత = ~(uint64_t)0;
      కొరకు (స్వయంచాలక I = వరుస.తల_2; I; I = వరుస.get_తదుపరి(I)) {
        ప్రతిజ్ఞా(I->Alignment == వరుస.Alignment && "bad field in queue");
        ప్రతిజ్ఞా(I->కొలత <= చివరి_కొలత && "queue not in descending size order");
        చివరి_కొలత = I->కొలత;
      }
    }
  };
  checkQueues();
#అయితే_ముగింపు

  /// Helper function to remove a field from a queue.
  స్వయంచాలక splice_From_వరుస = [&](Alignment_వరుస *వరుస, క్షేత్రం_2 *చివరి, క్షేత్రం_2 *Cur) {
    ప్రతిజ్ఞా(చివరి ? వరుస->get_తదుపరి(చివరి) == Cur : వరుస->తల_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    ఒకవేళ (చివరి) {
      చివరి->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      ఒకవేళ (!Cur->Scratch)
        వరుస->Min_కొలత = చివరి->కొలత;

    // Otherwise, replace the head.
    } లేకపోతే {
      ఒకవేళ (స్వయంచాలక New_తల = వరుస->get_తదుపరి(Cur))
        వరుస->తల_2 = New_తల;

      // If we just emptied the queue, destroy its bin.
      లేకపోతే
        FlexibleFieldsByAlignment.erase(వరుస);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_సదిశ<క్షేత్రం_2, 16> Layout;
  Layout.reserve(Fields.కొలత_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t చివరి_చివర = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  స్వయంచాలక కూడు_గమ్యం_Layout = [&](Alignment_వరుస *వరుస, క్షేత్రం_2 *చివరి, క్షేత్రం_2 *Cur,
                         uint64_t స్థానభ్రంశం) -> తార్కిక {
    ప్రతిజ్ఞా(స్థానభ్రంశం == align_గమ్యం(చివరి_చివర, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    splice_From_వరుస(వరుస, చివరి, Cur);

    // Add Cur to the layout.
    Layout.నెట్టు_వెనుక(*Cur);
    Layout.వెనుక().స్థానభ్రంశం = స్థానభ్రంశం;
    చివరి_చివర = Layout.వెనుక().get_చివర_స్థానభ్రంశం();

    // Always return true so that we can be tail-called.
    తిరిగి నిజం;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  స్వయంచాలక try_కూడు_Filler_From_వరుస = [&](Alignment_వరుస *వరుస, uint64_t ప్రారంభం_స్థానభ్రంశం,
                                   std::optional<uint64_t> చివర_స్థానభ్రంశం) -> తార్కిక {
    ప్రతిజ్ఞా(వరుస->తల_2);
    ప్రతిజ్ఞా(ప్రారంభం_స్థానభ్రంశం == align_గమ్యం(చివరి_చివర, వరుస->Alignment));
    ప్రతిజ్ఞా(!చివర_స్థానభ్రంశం || ప్రారంభం_స్థానభ్రంశం < *చివర_స్థానభ్రంశం);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    స్వయంచాలక Max_Viable_కొలత =
      (చివర_స్థానభ్రంశం ? *చివర_స్థానభ్రంశం - ప్రారంభం_స్థానభ్రంశం : ~(uint64_t)0);
    ఒకవేళ (వరుస->Min_కొలత > Max_Viable_కొలత)
      తిరిగి అబద్ధం;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    కొరకు (క్షేత్రం_2 *Cur = వరుస->తల_2, *చివరి = శూన్యసూచిక; నిజం;
           చివరి = Cur, Cur = వరుస->get_తదుపరి(Cur)) {
      ప్రతిజ్ఞా(Cur && "didn't find a match in queue despite its MinSize");
      ఒకవేళ (Cur->కొలత <= Max_Viable_కొలత)
        తిరిగి కూడు_గమ్యం_Layout(వరుస, చివరి, Cur, ప్రారంభం_స్థానభ్రంశం);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  స్వయంచాలక try_కూడు_ఉత్తమ_క్షేత్రం = [&](std::optional<uint64_t> Before_స్థానభ్రంశం) -> తార్కిక {
    ప్రతిజ్ఞా(!Before_స్థానభ్రంశం || చివరి_చివర < *Before_స్థానభ్రంశం);
    స్వయంచాలక వరుస_B = FlexibleFieldsByAlignment.begin();
    స్వయంచాలక వరుస_E = FlexibleFieldsByAlignment.చివర();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    స్వయంచాలక మొదటి_వరుస_గమ్యం_శోధన = వరుస_B;
    కొరకు (; మొదటి_వరుస_గమ్యం_శోధన != వరుస_E; ++మొదటి_వరుస_గమ్యం_శోధన) {
      ఒకవేళ (isAligned(మొదటి_వరుస_గమ్యం_శోధన->Alignment, చివరి_చివర))
        ఆపు;
    }

    uint64_t స్థానభ్రంశం = చివరి_చివర;
    అయితే (నిజం) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      కొరకు (స్వయంచాలక వరుస = మొదటి_వరుస_గమ్యం_శోధన; వరుస != వరుస_E; ++వరుస) {
        ఒకవేళ (try_కూడు_Filler_From_వరుస(వరుస, స్థానభ్రంశం, Before_స్థానభ్రంశం))
          తిరిగి నిజం;
      }

      // Okay, we don't need to scan those again.
      వరుస_E = మొదటి_వరుస_గమ్యం_శోధన;

      // If we started from the first queue, we're done.
      ఒకవేళ (మొదటి_వరుస_గమ్యం_శోధన == వరుస_B)
        తిరిగి అబద్ధం;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --మొదటి_వరుస_గమ్యం_శోధన;
      స్థానభ్రంశం = align_గమ్యం(చివరి_చివర, మొదటి_వరుస_గమ్యం_శోధన->Alignment);
      ఒకవేళ (Before_స్థానభ్రంశం && స్థానభ్రంశం >= *Before_స్థానభ్రంశం)
        తిరిగి అబద్ధం;
      అయితే (మొదటి_వరుస_గమ్యం_శోధన != వరుస_B &&
             స్థానభ్రంశం == align_గమ్యం(చివరి_చివర, మొదటి_వరుస_గమ్యం_శోధన[-1].Alignment))
        --మొదటి_వరుస_గమ్యం_శోధన;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  కొరకు (స్వయంచాలక I = Fields.begin(); I != మొదటి_Flexible; ++I) {
    ప్రతిజ్ఞా(చివరి_చివర <= I->స్థానభ్రంశం);
    అయితే (చివరి_చివర != I->స్థానభ్రంశం) {
      ఒకవేళ (!try_కూడు_ఉత్తమ_క్షేత్రం(I->స్థానభ్రంశం))
        ఆపు;
    }
    Layout.నెట్టు_వెనుక(*I);
    చివరి_చివర = I->get_చివర_స్థానభ్రంశం();
  }

#నిర్వచితం_కాకపోతే NDEBUG
  checkQueues();
#అయితే_ముగింపు

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  అయితే (!FlexibleFieldsByAlignment.ఖాళీలు()) {
    తార్కిక Success = try_కూడు_ఉత్తమ_క్షేత్రం(std::nullopt);
    ప్రతిజ్ఞా(Success && "didn't find a field with no fixed limit?");
    (శూన్య) Success;
  }

  // Copy the layout back into place.
  ప్రతిజ్ఞా(Layout.కొలత_2() == Fields.కొలత_2());
  జ్ఞాపకం_నకలు(Fields.దత్తాంశం(), Layout.దత్తాంశం(),
         Fields.కొలత_2() * పరిమాణం(Optimized_Struct_Layout_క్షేత్రం));

#నిర్వచితం_కాకపోతే NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, చివరి_చివర, MaxAlign);
#అయితే_ముగింపు

  తిరిగి {చివరి_చివర, MaxAlign};
}
