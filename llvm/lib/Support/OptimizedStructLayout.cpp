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

#अन्तर्भाव "llvm/Support/OptimizedStructLayout.h"
#अन्तर्भाव <optional>

उपयुज्य नामाकाश llvm;

उपयुज्य क्षेत्रम्_2 = Optimized_Struct_Layout_क्षेत्रम्;

#यदि_अपरिभाषितम् NDEBUG
स्थिर शून्य checkValidLayout(ArrayRef<क्षेत्रम्_2> Fields, uint64_t आकार,
                             Align MaxAlign) {
  uint64_t चरमः_अन्तः = 0;
  Align ComputedMaxAlign;
  प्रति (स्वचालित &क्षेत्रम्_2 : Fields) {
    प्रतिज्ञा(क्षेत्रम्_2.has_Fixed_विस्थापनम्() &&
           "didn't assign a fixed offset to field");
    प्रतिज्ञा(isAligned(क्षेत्रम्_2.Alignment, क्षेत्रम्_2.विस्थापनम्) &&
           "didn't assign a correctly-aligned offset to field");
    प्रतिज्ञा(क्षेत्रम्_2.विस्थापनम् >= चरमः_अन्तः &&
           "didn't assign offsets in ascending order");
    चरमः_अन्तः = क्षेत्रम्_2.get_अन्तः_विस्थापनम्();
    प्रतिज्ञा(क्षेत्रम्_2.Alignment <= MaxAlign &&
           "didn't compute MaxAlign correctly");
    ComputedMaxAlign = std::max(क्षेत्रम्_2.Alignment, MaxAlign);
  }
  प्रतिज्ञा(चरमः_अन्तः == आकार && "didn't compute LastEnd correctly");
  प्रतिज्ञा(ComputedMaxAlign == MaxAlign && "didn't compute MaxAlign correctly");
}
#यदि_अन्तः

std::pair<uint64_t, Align>
llvm::performOptimizedStructLayout(MutableArrayRef<क्षेत्रम्_2> Fields) {
#यदि_अपरिभाषितम् NDEBUG
  // Do some simple precondition checks.
  {
    तार्किक अन्तर्_Fixed_उपसर्गः = सत्य;
    size_t चरमः_अन्तः = 0;
    प्रति (स्वचालित &क्षेत्रम्_2 : Fields) {
      प्रतिज्ञा(क्षेत्रम्_2.आकार > 0 && "field of zero size");
      यदि (क्षेत्रम्_2.has_Fixed_विस्थापनम्()) {
        प्रतिज्ञा(अन्तर्_Fixed_उपसर्गः &&
               "fixed-offset fields are not a strict prefix of array");
        प्रतिज्ञा(चरमः_अन्तः <= क्षेत्रम्_2.विस्थापनम् &&
               "fixed-offset fields overlap or are not in order");
        चरमः_अन्तः = क्षेत्रम्_2.get_अन्तः_विस्थापनम्();
        प्रतिज्ञा(चरमः_अन्तः > क्षेत्रम्_2.विस्थापनम् &&
               "overflow in fixed-offset end offset");
      } अन्यथा {
        अन्तर्_Fixed_उपसर्गः = असत्य;
      }
    }
  }
#यदि_अन्तः

  // Do an initial pass over the fields.
  Align MaxAlign;

  // Find the first flexible-offset field, tracking MaxAlign.
  स्वचालित FirstFlexible = Fields.begin(), E = Fields.अन्तः();
  यावत् (FirstFlexible != E && FirstFlexible->has_Fixed_विस्थापनम्()) {
    MaxAlign = std::max(MaxAlign, FirstFlexible->Alignment);
    ++FirstFlexible;
  }

  // If there are no flexible fields, we're done.
  यदि (FirstFlexible == E) {
    uint64_t आकार = 0;
    यदि (!Fields.रिक्तानि())
      आकार = Fields.पृष्ठभागः().get_अन्तः_विस्थापनम्();

#यदि_अपरिभाषितम् NDEBUG
    checkValidLayout(Fields, आकार, MaxAlign);
#यदि_अन्तः
    प्रत्यागम {आकार, MaxAlign};
  }

  // Walk over the flexible-offset fields, tracking MaxAlign and
  // assigning them a unique number in order of their appearance.
  // We'll use this unique number in the comparison below so that
  // we can use array_pod_sort, which isn't stable.  We won't use it
  // past that point.
  {
    uintptr_t Unique_सङ्ख्या = 0;
    प्रति (स्वचालित I = FirstFlexible; I != E; ++I) {
      I->Scratch = reinterpret_cast<शून्य*>(Unique_सङ्ख्या++);
      MaxAlign = std::max(MaxAlign, I->Alignment);
    }
  }

  // Sort the flexible elements in order of decreasing alignment,
  // then decreasing size, and then the original order as recorded
  // in Scratch.  The decreasing-size aspect of this is only really
  // important if we get into the gap-filling stage below, but it
  // doesn't hurt here.
  array_pod_क्रमय(FirstFlexible, E,
                 [](ध्रुव क्षेत्रम्_2 *lhs, ध्रुव क्षेत्रम्_2 *rhs) -> पूर्णाङ्क {
    // Decreasing alignment.
    यदि (lhs->Alignment != rhs->Alignment)
      प्रत्यागम (lhs->Alignment < rhs->Alignment ? 1 : -1);

    // Decreasing size.
    यदि (lhs->आकार != rhs->आकार)
      प्रत्यागम (lhs->आकार < rhs->आकार ? 1 : -1);

    // Original order.
    स्वचालित lhs_सङ्ख्या = reinterpret_cast<uintptr_t>(lhs->Scratch);
    स्वचालित rhs_सङ्ख्या = reinterpret_cast<uintptr_t>(rhs->Scratch);
    यदि (lhs_सङ्ख्या != rhs_सङ्ख्या)
      प्रत्यागम (lhs_सङ्ख्या < rhs_सङ्ख्या ? -1 : 1);

    प्रत्यागम 0;
  });

  // Do a quick check for whether that sort alone has given us a perfect
  // layout with no interior padding.  This is very common: if the
  // fixed-layout fields have no interior padding, and they end at a
  // sufficiently-aligned offset for all the flexible-layout fields,
  // and the flexible-layout fields all have sizes that are multiples
  // of their alignment, then this will reliably trigger.
  {
    तार्किक HasPadding = असत्य;
    uint64_t चरमः_अन्तः = 0;

    // Walk the fixed-offset fields.
    प्रति (स्वचालित I = Fields.begin(); I != FirstFlexible; ++I) {
      प्रतिज्ञा(I->has_Fixed_विस्थापनम्());
      यदि (चरमः_अन्तः != I->विस्थापनम्) {
        HasPadding = सत्य;
        भङ्ग;
      }
      चरमः_अन्तः = I->get_अन्तः_विस्थापनम्();
    }

    // Walk the flexible-offset fields, optimistically assigning fixed
    // offsets.  Note that we maintain a strict division between the
    // fixed-offset and flexible-offset fields, so if we end up
    // discovering padding later in this loop, we can just abandon this
    // work and we'll ignore the offsets we already assigned.
    यदि (!HasPadding) {
      प्रति (स्वचालित I = FirstFlexible; I != E; ++I) {
        स्वचालित विस्थापनम् = align_गन्तव्यम्(चरमः_अन्तः, I->Alignment);
        यदि (चरमः_अन्तः != विस्थापनम्) {
          HasPadding = सत्य;
          भङ्ग;
        }
        I->विस्थापनम् = विस्थापनम्;
        चरमः_अन्तः = I->get_अन्तः_विस्थापनम्();
      }
    }

    // If we already have a perfect layout, we're done.
    यदि (!HasPadding) {
#यदि_अपरिभाषितम् NDEBUG
      checkValidLayout(Fields, चरमः_अन्तः, MaxAlign);
#यदि_अन्तः
      प्रत्यागम {चरमः_अन्तः, MaxAlign};
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
  संरचना AlignmentQueue {
    /// The minimum size of anything currently in this queue.
    uint64_t Min_आकार;

    /// The head of the queue.  A singly-linked list.  The order here should
    /// be consistent with the earlier sort, i.e. the elements should be
    /// monotonically descending in size and otherwise in the original order.
    ///
    /// We remove the queue from the array as soon as this is empty.
    Optimized_Struct_Layout_क्षेत्रम् *शीर्ष_2;

    /// The alignment requirement of the queue.
    Align Alignment;

    स्थिर क्षेत्रम्_2 *get_अग्र(क्षेत्रम्_2 *Cur) {
      प्रत्यागम static_cast<क्षेत्रम्_2 *>(Cur->Scratch);
    }
  };
  Small_सदिशम्<AlignmentQueue, 8> FlexibleFieldsByAlignment;
  प्रति (स्वचालित I = FirstFlexible; I != E; ) {
    स्वचालित शीर्ष_2 = I;
    स्वचालित Alignment = I->Alignment;

    uint64_t Min_आकार = I->आकार;
    स्वचालित चरमः_अन्तर्_Queue = I;
    प्रति (++I; I != E && I->Alignment == Alignment; ++I) {
      चरमः_अन्तर्_Queue->Scratch = I;
      चरमः_अन्तर्_Queue = I;
      Min_आकार = std::min(Min_आकार, I->आकार);
    }
    चरमः_अन्तर्_Queue->Scratch = शून्यसूचकः;

    FlexibleFieldsByAlignment.प्रक्षिप_पृष्ठभागः({Min_आकार, शीर्ष_2, Alignment});
  }

#यदि_अपरिभाषितम् NDEBUG
  // Verify that we set the queues up correctly.
  स्वचालित checkQueues = [&]{
    तार्किक FirstQueue = सत्य;
    Align चरमः_Queue_Alignment;
    प्रति (स्वचालित &Queue : FlexibleFieldsByAlignment) {
      प्रतिज्ञा((FirstQueue || Queue.Alignment < चरमः_Queue_Alignment) &&
             "bins not in order of descending alignment");
      चरमः_Queue_Alignment = Queue.Alignment;
      FirstQueue = असत्य;

      प्रतिज्ञा(Queue.शीर्ष_2 && "queue was empty");
      uint64_t चरमः_आकार = ~(uint64_t)0;
      प्रति (स्वचालित I = Queue.शीर्ष_2; I; I = Queue.get_अग्र(I)) {
        प्रतिज्ञा(I->Alignment == Queue.Alignment && "bad field in queue");
        प्रतिज्ञा(I->आकार <= चरमः_आकार && "queue not in descending size order");
        चरमः_आकार = I->आकार;
      }
    }
  };
  checkQueues();
#यदि_अन्तः

  /// Helper function to remove a field from a queue.
  स्वचालित spliceFromQueue = [&](AlignmentQueue *Queue, क्षेत्रम्_2 *चरमः, क्षेत्रम्_2 *Cur) {
    प्रतिज्ञा(चरमः ? Queue->get_अग्र(चरमः) == Cur : Queue->शीर्ष_2 == Cur);

    // If we're removing Cur from a non-initial position, splice it out
    // of the linked list.
    यदि (चरमः) {
      चरमः->Scratch = Cur->Scratch;

      // If Cur was the last field in the list, we need to update MinSize.
      // We can just use the last field's size because the list is in
      // descending order of size.
      यदि (!Cur->Scratch)
        Queue->Min_आकार = चरमः->आकार;

    // Otherwise, replace the head.
    } अन्यथा {
      यदि (स्वचालित New_शीर्ष = Queue->get_अग्र(Cur))
        Queue->शीर्ष_2 = New_शीर्ष;

      // If we just emptied the queue, destroy its bin.
      अन्यथा
        FlexibleFieldsByAlignment.erase(Queue);
    }
  };

  // Do layout into a local array.  Doing this in-place on Fields is
  // not really feasible.
  Small_सदिशम्<क्षेत्रम्_2, 16> Layout;
  Layout.reserve(Fields.आकार_2());

  // The offset that we're currently looking to insert at (or after).
  uint64_t चरमः_अन्तः = 0;

  // Helper function to splice Cur out of the given queue and add it
  // to the layout at the given offset.
  स्वचालित सङ्कलय_गन्तव्यम्_Layout = [&](AlignmentQueue *Queue, क्षेत्रम्_2 *चरमः, क्षेत्रम्_2 *Cur,
                         uint64_t विस्थापनम्) -> तार्किक {
    प्रतिज्ञा(विस्थापनम् == align_गन्तव्यम्(चरमः_अन्तः, Cur->Alignment));

    // Splice out.  This potentially invalidates Queue.
    spliceFromQueue(Queue, चरमः, Cur);

    // Add Cur to the layout.
    Layout.प्रक्षिप_पृष्ठभागः(*Cur);
    Layout.पृष्ठभागः().विस्थापनम् = विस्थापनम्;
    चरमः_अन्तः = Layout.पृष्ठभागः().get_अन्तः_विस्थापनम्();

    // Always return true so that we can be tail-called.
    प्रत्यागम सत्य;
  };

  // Helper function to try to find a field in the given queue that'll
  // fit starting at StartOffset but before EndOffset (if present).
  // Note that this never fails if EndOffset is not provided.
  स्वचालित try_सङ्कलय_Filler_From_Queue = [&](AlignmentQueue *Queue, uint64_t Start_विस्थापनम्,
                                   std::optional<uint64_t> अन्तः_विस्थापनम्) -> तार्किक {
    प्रतिज्ञा(Queue->शीर्ष_2);
    प्रतिज्ञा(Start_विस्थापनम् == align_गन्तव्यम्(चरमः_अन्तः, Queue->Alignment));
    प्रतिज्ञा(!अन्तः_विस्थापनम् || Start_विस्थापनम् < *अन्तः_विस्थापनम्);

    // Figure out the maximum size that a field can be, and ignore this
    // queue if there's nothing in it that small.
    स्वचालित Max_Viable_आकार =
      (अन्तः_विस्थापनम् ? *अन्तः_विस्थापनम् - Start_विस्थापनम् : ~(uint64_t)0);
    यदि (Queue->Min_आकार > Max_Viable_आकार)
      प्रत्यागम असत्य;

    // Find the matching field.  Note that this should always find
    // something because of the MinSize check above.
    प्रति (क्षेत्रम्_2 *Cur = Queue->शीर्ष_2, *चरमः = शून्यसूचकः; सत्य;
           चरमः = Cur, Cur = Queue->get_अग्र(Cur)) {
      प्रतिज्ञा(Cur && "didn't find a match in queue despite its MinSize");
      यदि (Cur->आकार <= Max_Viable_आकार)
        प्रत्यागम सङ्कलय_गन्तव्यम्_Layout(Queue, चरमः, Cur, Start_विस्थापनम्);
    }

    llvm_unreachable("didn't find a match in queue despite its MinSize");
  };

  // Helper function to find the "best" flexible-offset field according
  // to the criteria described above.
  स्वचालित try_सङ्कलय_Best_क्षेत्रम् = [&](std::optional<uint64_t> Before_विस्थापनम्) -> तार्किक {
    प्रतिज्ञा(!Before_विस्थापनम् || चरमः_अन्तः < *Before_विस्थापनम्);
    स्वचालित QueueB = FlexibleFieldsByAlignment.begin();
    स्वचालित QueueE = FlexibleFieldsByAlignment.अन्तः();

    // Start by looking for the most-aligned queue that doesn't need any
    // leading padding after LastEnd.
    स्वचालित First_Queue_गन्तव्यम्_अन्वेषणम् = QueueB;
    प्रति (; First_Queue_गन्तव्यम्_अन्वेषणम् != QueueE; ++First_Queue_गन्तव्यम्_अन्वेषणम्) {
      यदि (isAligned(First_Queue_गन्तव्यम्_अन्वेषणम्->Alignment, चरमः_अन्तः))
        भङ्ग;
    }

    uint64_t विस्थापनम् = चरमः_अन्तः;
    यावत् (सत्य) {
      // Invariant: all of the queues in [FirstQueueToSearch, QueueE)
      // require the same initial padding offset.

      // Search those queues in descending order of alignment for a
      // satisfactory field.
      प्रति (स्वचालित Queue = First_Queue_गन्तव्यम्_अन्वेषणम्; Queue != QueueE; ++Queue) {
        यदि (try_सङ्कलय_Filler_From_Queue(Queue, विस्थापनम्, Before_विस्थापनम्))
          प्रत्यागम सत्य;
      }

      // Okay, we don't need to scan those again.
      QueueE = First_Queue_गन्तव्यम्_अन्वेषणम्;

      // If we started from the first queue, we're done.
      यदि (First_Queue_गन्तव्यम्_अन्वेषणम् == QueueB)
        प्रत्यागम असत्य;

      // Otherwise, scan backwards to find the most-aligned queue that
      // still has minimal leading padding after LastEnd.  If that
      // minimal padding is already at or past the end point, we're done.
      --First_Queue_गन्तव्यम्_अन्वेषणम्;
      विस्थापनम् = align_गन्तव्यम्(चरमः_अन्तः, First_Queue_गन्तव्यम्_अन्वेषणम्->Alignment);
      यदि (Before_विस्थापनम् && विस्थापनम् >= *Before_विस्थापनम्)
        प्रत्यागम असत्य;
      यावत् (First_Queue_गन्तव्यम्_अन्वेषणम् != QueueB &&
             विस्थापनम् == align_गन्तव्यम्(चरमः_अन्तः, First_Queue_गन्तव्यम्_अन्वेषणम्[-1].Alignment))
        --First_Queue_गन्तव्यम्_अन्वेषणम्;
    }
  };

  // Phase 1: fill the gaps between fixed-offset fields with the best
  // flexible-offset field that fits.
  प्रति (स्वचालित I = Fields.begin(); I != FirstFlexible; ++I) {
    प्रतिज्ञा(चरमः_अन्तः <= I->विस्थापनम्);
    यावत् (चरमः_अन्तः != I->विस्थापनम्) {
      यदि (!try_सङ्कलय_Best_क्षेत्रम्(I->विस्थापनम्))
        भङ्ग;
    }
    Layout.प्रक्षिप_पृष्ठभागः(*I);
    चरमः_अन्तः = I->get_अन्तः_विस्थापनम्();
  }

#यदि_अपरिभाषितम् NDEBUG
  checkQueues();
#यदि_अन्तः

  // Phase 2: repeatedly add the best flexible-offset field until
  // they're all gone.
  यावत् (!FlexibleFieldsByAlignment.रिक्तानि()) {
    तार्किक Success = try_सङ्कलय_Best_क्षेत्रम्(std::nullopt);
    प्रतिज्ञा(Success && "didn't find a field with no fixed limit?");
    (शून्य) Success;
  }

  // Copy the layout back into place.
  प्रतिज्ञा(Layout.आकार_2() == Fields.आकार_2());
  स्मृतिं_प्रतिलिख(Fields.दत्त(), Layout.दत्त(),
         Fields.आकार_2() * परिमाण(Optimized_Struct_Layout_क्षेत्रम्));

#यदि_अपरिभाषितम् NDEBUG
  // Make a final check that the layout is valid.
  checkValidLayout(Fields, चरमः_अन्तः, MaxAlign);
#यदि_अन्तः

  प्रत्यागम {चरमः_अन्तः, MaxAlign};
}
