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

#شامل "llvm/ADT/DAGDeltaAlgorithm.h"
#شامل "llvm/ADT/DeltaAlgorithm.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <map>
استعمال نامخلا llvm;

#تعریف_کرو DEBUG_TYPE "dag-delta"

نامخلا {

جماعت DAGDeltaAlgorithmImpl {
  دوست جماعت Delta_فعال_سیٹ_Helper;

عوامی:
  استعمال change_ty = DAGDeltaAlgorithm::change_ty;
  استعمال changeset_ty = DAGDeltaAlgorithm::changeset_ty;
  استعمال changesetlist_ty = DAGDeltaAlgorithm::changesetlist_ty;
  استعمال کنارہ_ty = DAGDeltaAlgorithm::کنارہ_ty;

نجی:
  استعمال pred_iterator_ty = std::سمتیہ<change_ty>::iterator;
  استعمال succ_iterator_ty = std::سمتیہ<change_ty>::iterator;
  استعمال pred_احاطہ_iterator_ty = std::سیٹ<change_ty>::iterator;
  استعمال succ_احاطہ_iterator_ty = std::سیٹ<change_ty>::iterator;

  DAGDeltaAlgorithm &DDA;

  std::سمتیہ<change_ty> Roots;

  /// Cache of failed test results. Successful test results are never cached
  /// since we always reduce following a success. We maintain an independent
  /// cache from that used by the individual delta passes because we may get
  /// hits across multiple individual delta invocations.
  تغیرپذیر std::سیٹ<changeset_ty> FailedTestsCache;

  // FIXME: Gross.
  std::map<change_ty, std::سمتیہ<change_ty> > Predecessors;
  std::map<change_ty, std::سمتیہ<change_ty> > جانشین;

  std::map<change_ty, std::سیٹ<change_ty> > Pred_احاطہ;
  std::map<change_ty, std::سیٹ<change_ty> > Succ_احاطہ;

نجی:
  pred_iterator_ty pred_begin(change_ty گرہ) {
    أكد(Predecessors.گنتی(گرہ) && "Invalid node!");
    واپس Predecessors[گرہ].begin();
  }
  pred_iterator_ty pred_انتہا(change_ty گرہ) {
    أكد(Predecessors.گنتی(گرہ) && "Invalid node!");
    واپس Predecessors[گرہ].انتہا();
  }

  pred_احاطہ_iterator_ty pred_احاطہ_begin(change_ty گرہ) {
    أكد(Pred_احاطہ.گنتی(گرہ) && "Invalid node!");
    واپس Pred_احاطہ[گرہ].begin();
  }
  pred_احاطہ_iterator_ty pred_احاطہ_انتہا(change_ty گرہ) {
    أكد(Pred_احاطہ.گنتی(گرہ) && "Invalid node!");
    واپس Pred_احاطہ[گرہ].انتہا();
  }

  succ_iterator_ty succ_begin(change_ty گرہ) {
    أكد(جانشین.گنتی(گرہ) && "Invalid node!");
    واپس جانشین[گرہ].begin();
  }
  succ_iterator_ty succ_انتہا(change_ty گرہ) {
    أكد(جانشین.گنتی(گرہ) && "Invalid node!");
    واپس جانشین[گرہ].انتہا();
  }

  succ_احاطہ_iterator_ty succ_احاطہ_begin(change_ty گرہ) {
    أكد(Succ_احاطہ.گنتی(گرہ) && "Invalid node!");
    واپس Succ_احاطہ[گرہ].begin();
  }
  succ_احاطہ_iterator_ty succ_احاطہ_انتہا(change_ty گرہ) {
    أكد(Succ_احاطہ.گنتی(گرہ) && "Invalid node!");
    واپس Succ_احاطہ[گرہ].انتہا();
  }

  خالی Updated_تلاش_State(ثابت changeset_ty &Changes,
                          ثابت changesetlist_ty &Sets,
                          ثابت changeset_ty &Required) {
    DDA.Updated_تلاش_State(Changes, Sets, Required);
  }

  /// ExecuteOneTest - Execute a single test predicate on the change set \p S.
  منطقی ExecuteOneTest(ثابت changeset_ty &S) {
    // Check dependencies invariant.
    LLVM_DEBUG({
      برائے (changeset_ty::const_iterator it = S.begin(), ie = S.انتہا(); it != ie;
           ++it)
        برائے (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_انتہا(*it);
             it2 != ie2; ++it2)
          أكد(S.گنتی(*it2) && "Attempt to run invalid changeset!");
    });

    واپس DDA.ExecuteOneTest(S);
  }

عوامی:
  DAGDeltaAlgorithmImpl(DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
                        ثابت std::سمتیہ<کنارہ_ty> &Dependencies);

  changeset_ty تسلسل_2();

  /// GetTestResult - Get the test result for the active set \p Changes with
  /// \p Required changes from the cache, executing the test if necessary.
  ///
  /// \param Changes - The set of active changes being minimized, which should
  /// have their pred closure included in the test.
  /// \param Required - The set of changes which have previously been
  /// established to be required.
  /// \return - The test result.
  منطقی Get_Test_نتیجہ(ثابت changeset_ty &Changes, ثابت changeset_ty &Required);
};

/// Helper object for minimizing an active set of changes.
جماعت Delta_فعال_سیٹ_Helper : عوامی DeltaAlgorithm {
  DAGDeltaAlgorithmImpl &DDAI;

  ثابت changeset_ty &Required;

محفوظ:
  /// UpdatedSearchState - Callback used when the search state changes.
  خالی Updated_تلاش_State(ثابت changeset_ty &Changes,
                                  ثابت changesetlist_ty &Sets) override {
    DDAI.Updated_تلاش_State(Changes, Sets, Required);
  }

  منطقی ExecuteOneTest(ثابت changeset_ty &S) override {
    واپس DDAI.Get_Test_نتیجہ(S, Required);
  }

عوامی:
  Delta_فعال_سیٹ_Helper(DAGDeltaAlgorithmImpl &DDAI,
                       ثابت changeset_ty &Required)
      : DDAI(DDAI), Required(Required) {}
};

} // namespace

DAGDeltaAlgorithmImpl::DAGDeltaAlgorithmImpl(
    DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
    ثابت std::سمتیہ<کنارہ_ty> &Dependencies)
    : DDA(DDA) {
  برائے (change_ty Change : Changes) {
    Predecessors.try_emplace(Change);
    جانشین.try_emplace(Change);
  }
  برائے (ثابت کنارہ_ty &Dep : Dependencies) {
    Predecessors[Dep.دوسرا_2].ڈالو_عقب(Dep.پہلا_2);
    جانشین[Dep.پہلا_2].ڈالو_عقب(Dep.دوسرا_2);
  }

  // Compute the roots.
  برائے (change_ty Change : Changes)
    اگر (succ_begin(Change) == succ_انتہا(Change))
      Roots.ڈالو_عقب(Change);

  // Pre-compute the closure of the successor relation.
  std::سمتیہ<change_ty> Worklist(Roots.begin(), Roots.انتہا());
  جب (!Worklist.خالی_خانے()) {
    change_ty Change = Worklist.عقب();
    Worklist.نکالو_عقب();

    std::سیٹ<change_ty> &ChangeSuccs = Succ_احاطہ[Change];
    برائے (pred_iterator_ty it = pred_begin(Change),
           ie = pred_انتہا(Change); it != ie; ++it) {
      خودکار &SC = Succ_احاطہ[*it];
      SC.داخل_کرو(Change);
      SC.داخل_کرو(ChangeSuccs.begin(), ChangeSuccs.انتہا());
      Worklist.ڈالو_عقب(*it);
    }
  }

  // Invert to form the predecessor closure map.
  برائے (change_ty Change : Changes)
    Pred_احاطہ.try_emplace(Change);
  برائے (change_ty Change : Changes)
    برائے (succ_احاطہ_iterator_ty it2 = succ_احاطہ_begin(Change),
                                  ie2 = succ_احاطہ_انتہا(Change);
         it2 != ie2; ++it2)
      Pred_احاطہ[*it2].داخل_کرو(Change);

  // Dump useful debug info.
  LLVM_DEBUG({
    llvm::errs() << "-- DAGDeltaAlgorithmImpl --\n";
    llvm::errs() << "Changes: [";
    برائے (changeset_ty::const_iterator it = Changes.begin(), ie = Changes.انتہا();
         it != ie; ++it) {
      اگر (it != Changes.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;

      اگر (succ_begin(*it) != succ_انتہا(*it)) {
        llvm::errs() << "(";
        برائے (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_انتہا(*it);
             it2 != ie2; ++it2) {
          اگر (it2 != succ_begin(*it))
            llvm::errs() << ", ";
          llvm::errs() << "->" << *it2;
        }
        llvm::errs() << ")";
      }
    }
    llvm::errs() << "]\n";

    llvm::errs() << "Roots: [";
    برائے (std::سمتیہ<change_ty>::const_iterator it = Roots.begin(),
                                                ie = Roots.انتہا();
         it != ie; ++it) {
      اگر (it != Roots.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;
    }
    llvm::errs() << "]\n";

    llvm::errs() << "Predecessor Closure:\n";
    برائے (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      برائے (pred_احاطہ_iterator_ty it2 = pred_احاطہ_begin(Change),
                                    ie2 = pred_احاطہ_انتہا(Change);
           it2 != ie2; ++it2) {
        اگر (it2 != pred_احاطہ_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "Successor Closure:\n";
    برائے (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      برائے (succ_احاطہ_iterator_ty it2 = succ_احاطہ_begin(Change),
                                    ie2 = succ_احاطہ_انتہا(Change);
           it2 != ie2; ++it2) {
        اگر (it2 != succ_احاطہ_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "\n\n";
  });
}

منطقی DAGDeltaAlgorithmImpl::Get_Test_نتیجہ(ثابت changeset_ty &Changes,
                                          ثابت changeset_ty &Required) {
  changeset_ty Extended(Required);
  Extended.داخل_کرو(Changes.begin(), Changes.انتہا());
  برائے (change_ty Change : Changes)
    Extended.داخل_کرو(pred_احاطہ_begin(Change), pred_احاطہ_انتہا(Change));

  اگر (FailedTestsCache.گنتی(Extended))
    واپس جھوٹ;

  منطقی نتیجہ = ExecuteOneTest(Extended);
  اگر (!نتیجہ)
    FailedTestsCache.داخل_کرو(Extended);

  واپس نتیجہ;
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithmImpl::تسلسل_2() {
  // The current set of changes we are minimizing, starting at the roots.
  changeset_ty Current_سیٹ(Roots.begin(), Roots.انتہا());

  // The set of required changes.
  changeset_ty Required;

  // Iterate until the active set of changes is empty. Convergence is guaranteed
  // assuming input was a DAG.
  //
  // Invariant:  CurrentSet intersect Required == {}
  // Invariant:  Required == (Required union succ*(Required))
  جب (!Current_سیٹ.خالی_خانے()) {
    LLVM_DEBUG({
      llvm::errs() << "DAG_DD - " << Current_سیٹ.سائز_2() << " active changes, "
                   << Required.سائز_2() << " required changes\n";
    });

    // Minimize the current set of changes.
    Delta_فعال_سیٹ_Helper Helper(*یہ, Required);
    changeset_ty Current_Min_سیٹ = Helper.تسلسل_2(Current_سیٹ);

    // Update the set of required changes. Since
    //   CurrentMinSet subset CurrentSet
    // and after the last iteration,
    //   succ(CurrentSet) subset Required
    // then
    //   succ(CurrentMinSet) subset Required
    // and our invariant on Required is maintained.
    Required.داخل_کرو(Current_Min_سیٹ.begin(), Current_Min_سیٹ.انتہا());

    // Replace the current set with the predecssors of the minimized set of
    // active changes.
    Current_سیٹ.clear();
    برائے (change_ty CT : Current_Min_سیٹ)
      Current_سیٹ.داخل_کرو(pred_begin(CT), pred_انتہا(CT));

    // FIXME: We could enforce CurrentSet intersect Required == {} here if we
    // wanted to protect against cyclic graphs.
  }

  واپس Required;
}

خالی DAGDeltaAlgorithm::anchor() {
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithm::تسلسل_2(ثابت changeset_ty &Changes,
                       ثابت std::سمتیہ<کنارہ_ty> &Dependencies) {
  واپس DAGDeltaAlgorithmImpl(*یہ, Changes, Dependencies).تسلسل_2();
}
