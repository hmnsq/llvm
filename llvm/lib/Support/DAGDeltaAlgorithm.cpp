//===--- DAGDeltaAlgorithm.cpp - A DAG Minimization Algorithm --*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//
//
// The algorithm we use attempts to exploit the dependency information by
// minimizing top-down. We start by constructing an initial root set R, and
// then iteratively:
//
//   1. Minimize the set R using the test predicate:
//       P'(S) = P(S union pred*(S))
//
//   2. Extend R to R' = R union pred(R).
//
// until a fixed point is reached.
//
// The idea is that we want to quickly prune entire portions of the graph, so we
// try to find high-level nodes that can be eliminated with all of their
// dependents.
//
// FIXME: The current algorithm doesn't actually provide a strong guarantee
// about the minimality of the result. The problem is that after adding nodes to
// the required set, we no longer consider them for elimination. For strictly
// well formed predicates, this doesn't happen, but it commonly occurs in
// practice when there are unmodelled dependencies. I believe we can resolve
// this by allowing the required set to be minimized as well, but need more test
// cases first.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/DAGDeltaAlgorithm.h"
#ضمن "llvm/ADT/DeltaAlgorithm.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <map>
استفاده نطاق llvm;

#تعریف_کن DEBUG_TYPE "dag-delta"

نطاق {

صنف DAGDeltaAlgorithmImpl {
  دوست صنف Delta_فعال_مجموعه_Helper;

عام:
  استفاده change_ty = DAGDeltaAlgorithm::change_ty;
  استفاده changeset_ty = DAGDeltaAlgorithm::changeset_ty;
  استفاده changesetlist_ty = DAGDeltaAlgorithm::changesetlist_ty;
  استفاده یال_ty = DAGDeltaAlgorithm::یال_ty;

خاص:
  استفاده pred_iterator_ty = std::وکتور<change_ty>::iterator;
  استفاده succ_iterator_ty = std::وکتور<change_ty>::iterator;
  استفاده pred_بستگی_iterator_ty = std::مجموعه<change_ty>::iterator;
  استفاده succ_بستگی_iterator_ty = std::مجموعه<change_ty>::iterator;

  DAGDeltaAlgorithm &DDA;

  std::وکتور<change_ty> Roots;

  /// Cache of failed test results. Successful test results are never cached
  /// since we always reduce following a success. We maintain an independent
  /// cache from that used by the individual delta passes because we may get
  /// hits across multiple individual delta invocations.
  تغییرپذیر std::مجموعه<changeset_ty> FailedTestsCache;

  // FIXME: Gross.
  std::map<change_ty, std::وکتور<change_ty> > Predecessors;
  std::map<change_ty, std::وکتور<change_ty> > جانشینها;

  std::map<change_ty, std::مجموعه<change_ty> > Pred_بستگی;
  std::map<change_ty, std::مجموعه<change_ty> > Succ_بستگی;

خاص:
  pred_iterator_ty pred_begin(change_ty گره) {
    أكد(Predecessors.شمار(گره) && "Invalid node!");
    أرجع Predecessors[گره].begin();
  }
  pred_iterator_ty pred_انتها(change_ty گره) {
    أكد(Predecessors.شمار(گره) && "Invalid node!");
    أرجع Predecessors[گره].انتها();
  }

  pred_بستگی_iterator_ty pred_بستگی_begin(change_ty گره) {
    أكد(Pred_بستگی.شمار(گره) && "Invalid node!");
    أرجع Pred_بستگی[گره].begin();
  }
  pred_بستگی_iterator_ty pred_بستگی_انتها(change_ty گره) {
    أكد(Pred_بستگی.شمار(گره) && "Invalid node!");
    أرجع Pred_بستگی[گره].انتها();
  }

  succ_iterator_ty succ_begin(change_ty گره) {
    أكد(جانشینها.شمار(گره) && "Invalid node!");
    أرجع جانشینها[گره].begin();
  }
  succ_iterator_ty succ_انتها(change_ty گره) {
    أكد(جانشینها.شمار(گره) && "Invalid node!");
    أرجع جانشینها[گره].انتها();
  }

  succ_بستگی_iterator_ty succ_بستگی_begin(change_ty گره) {
    أكد(Succ_بستگی.شمار(گره) && "Invalid node!");
    أرجع Succ_بستگی[گره].begin();
  }
  succ_بستگی_iterator_ty succ_بستگی_انتها(change_ty گره) {
    أكد(Succ_بستگی.شمار(گره) && "Invalid node!");
    أرجع Succ_بستگی[گره].انتها();
  }

  فراغ Updated_جستجو_State(ثابت changeset_ty &Changes,
                          ثابت changesetlist_ty &Sets,
                          ثابت changeset_ty &Required) {
    DDA.Updated_جستجو_State(Changes, Sets, Required);
  }

  /// ExecuteOneTest - Execute a single test predicate on the change set \p S.
  منطقي ExecuteOneTest(ثابت changeset_ty &S) {
    // Check dependencies invariant.
    LLVM_DEBUG({
      لكل (changeset_ty::const_iterator it = S.begin(), ie = S.انتها(); it != ie;
           ++it)
        لكل (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_انتها(*it);
             it2 != ie2; ++it2)
          أكد(S.شمار(*it2) && "Attempt to run invalid changeset!");
    });

    أرجع DDA.ExecuteOneTest(S);
  }

عام:
  DAGDeltaAlgorithmImpl(DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
                        ثابت std::وکتور<یال_ty> &Dependencies);

  changeset_ty پیاپی_2();

  /// GetTestResult - Get the test result for the active set \p Changes with
  /// \p Required changes from the cache, executing the test if necessary.
  ///
  /// \param Changes - The set of active changes being minimized, which should
  /// have their pred closure included in the test.
  /// \param Required - The set of changes which have previously been
  /// established to be required.
  /// \return - The test result.
  منطقي Get_Test_نتیجه(ثابت changeset_ty &Changes, ثابت changeset_ty &Required);
};

/// Helper object for minimizing an active set of changes.
صنف Delta_فعال_مجموعه_Helper : عام DeltaAlgorithm {
  DAGDeltaAlgorithmImpl &DDAI;

  ثابت changeset_ty &Required;

محمي:
  /// UpdatedSearchState - Callback used when the search state changes.
  فراغ Updated_جستجو_State(ثابت changeset_ty &Changes,
                                  ثابت changesetlist_ty &Sets) override {
    DDAI.Updated_جستجو_State(Changes, Sets, Required);
  }

  منطقي ExecuteOneTest(ثابت changeset_ty &S) override {
    أرجع DDAI.Get_Test_نتیجه(S, Required);
  }

عام:
  Delta_فعال_مجموعه_Helper(DAGDeltaAlgorithmImpl &DDAI,
                       ثابت changeset_ty &Required)
      : DDAI(DDAI), Required(Required) {}
};

} // namespace

DAGDeltaAlgorithmImpl::DAGDeltaAlgorithmImpl(
    DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
    ثابت std::وکتور<یال_ty> &Dependencies)
    : DDA(DDA) {
  لكل (change_ty Change : Changes) {
    Predecessors.try_emplace(Change);
    جانشینها.try_emplace(Change);
  }
  لكل (ثابت یال_ty &Dep : Dependencies) {
    Predecessors[Dep.دومی_2].افزودن_پشت(Dep.اولی_2);
    جانشینها[Dep.اولی_2].افزودن_پشت(Dep.دومی_2);
  }

  // Compute the roots.
  لكل (change_ty Change : Changes)
    إذا (succ_begin(Change) == succ_انتها(Change))
      Roots.افزودن_پشت(Change);

  // Pre-compute the closure of the successor relation.
  std::وکتور<change_ty> Worklist(Roots.begin(), Roots.انتها());
  طالما (!Worklist.خالی()) {
    change_ty Change = Worklist.پشت();
    Worklist.برداشت_پشت();

    std::مجموعه<change_ty> &ChangeSuccs = Succ_بستگی[Change];
    لكل (pred_iterator_ty it = pred_begin(Change),
           ie = pred_انتها(Change); it != ie; ++it) {
      خودکار &SC = Succ_بستگی[*it];
      SC.درج(Change);
      SC.درج(ChangeSuccs.begin(), ChangeSuccs.انتها());
      Worklist.افزودن_پشت(*it);
    }
  }

  // Invert to form the predecessor closure map.
  لكل (change_ty Change : Changes)
    Pred_بستگی.try_emplace(Change);
  لكل (change_ty Change : Changes)
    لكل (succ_بستگی_iterator_ty it2 = succ_بستگی_begin(Change),
                                  ie2 = succ_بستگی_انتها(Change);
         it2 != ie2; ++it2)
      Pred_بستگی[*it2].درج(Change);

  // Dump useful debug info.
  LLVM_DEBUG({
    llvm::errs() << "-- DAGDeltaAlgorithmImpl --\n";
    llvm::errs() << "Changes: [";
    لكل (changeset_ty::const_iterator it = Changes.begin(), ie = Changes.انتها();
         it != ie; ++it) {
      إذا (it != Changes.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;

      إذا (succ_begin(*it) != succ_انتها(*it)) {
        llvm::errs() << "(";
        لكل (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_انتها(*it);
             it2 != ie2; ++it2) {
          إذا (it2 != succ_begin(*it))
            llvm::errs() << ", ";
          llvm::errs() << "->" << *it2;
        }
        llvm::errs() << ")";
      }
    }
    llvm::errs() << "]\n";

    llvm::errs() << "Roots: [";
    لكل (std::وکتور<change_ty>::const_iterator it = Roots.begin(),
                                                ie = Roots.انتها();
         it != ie; ++it) {
      إذا (it != Roots.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;
    }
    llvm::errs() << "]\n";

    llvm::errs() << "Predecessor Closure:\n";
    لكل (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      لكل (pred_بستگی_iterator_ty it2 = pred_بستگی_begin(Change),
                                    ie2 = pred_بستگی_انتها(Change);
           it2 != ie2; ++it2) {
        إذا (it2 != pred_بستگی_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "Successor Closure:\n";
    لكل (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      لكل (succ_بستگی_iterator_ty it2 = succ_بستگی_begin(Change),
                                    ie2 = succ_بستگی_انتها(Change);
           it2 != ie2; ++it2) {
        إذا (it2 != succ_بستگی_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "\n\n";
  });
}

منطقي DAGDeltaAlgorithmImpl::Get_Test_نتیجه(ثابت changeset_ty &Changes,
                                          ثابت changeset_ty &Required) {
  changeset_ty Extended(Required);
  Extended.درج(Changes.begin(), Changes.انتها());
  لكل (change_ty Change : Changes)
    Extended.درج(pred_بستگی_begin(Change), pred_بستگی_انتها(Change));

  إذا (FailedTestsCache.شمار(Extended))
    أرجع خطأ;

  منطقي نتیجه = ExecuteOneTest(Extended);
  إذا (!نتیجه)
    FailedTestsCache.درج(Extended);

  أرجع نتیجه;
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithmImpl::پیاپی_2() {
  // The current set of changes we are minimizing, starting at the roots.
  changeset_ty Current_مجموعه(Roots.begin(), Roots.انتها());

  // The set of required changes.
  changeset_ty Required;

  // Iterate until the active set of changes is empty. Convergence is guaranteed
  // assuming input was a DAG.
  //
  // Invariant:  CurrentSet intersect Required == {}
  // Invariant:  Required == (Required union succ*(Required))
  طالما (!Current_مجموعه.خالی()) {
    LLVM_DEBUG({
      llvm::errs() << "DAG_DD - " << Current_مجموعه.اندازه_2() << " active changes, "
                   << Required.اندازه_2() << " required changes\n";
    });

    // Minimize the current set of changes.
    Delta_فعال_مجموعه_Helper Helper(*هذا, Required);
    changeset_ty Current_Min_مجموعه = Helper.پیاپی_2(Current_مجموعه);

    // Update the set of required changes. Since
    //   CurrentMinSet subset CurrentSet
    // and after the last iteration,
    //   succ(CurrentSet) subset Required
    // then
    //   succ(CurrentMinSet) subset Required
    // and our invariant on Required is maintained.
    Required.درج(Current_Min_مجموعه.begin(), Current_Min_مجموعه.انتها());

    // Replace the current set with the predecssors of the minimized set of
    // active changes.
    Current_مجموعه.clear();
    لكل (change_ty CT : Current_Min_مجموعه)
      Current_مجموعه.درج(pred_begin(CT), pred_انتها(CT));

    // FIXME: We could enforce CurrentSet intersect Required == {} here if we
    // wanted to protect against cyclic graphs.
  }

  أرجع Required;
}

فراغ DAGDeltaAlgorithm::anchor() {
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithm::پیاپی_2(ثابت changeset_ty &Changes,
                       ثابت std::وکتور<یال_ty> &Dependencies) {
  أرجع DAGDeltaAlgorithmImpl(*هذا, Changes, Dependencies).پیاپی_2();
}
