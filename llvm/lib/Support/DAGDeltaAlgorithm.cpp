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
using فضاینام llvm;

#define DEBUG_TYPE "dag-delta"

فضاینام {

رده DAGDeltaAlgorithmImpl {
  friend رده DeltaActiveSetHelper;

عمومی:
  using change_ty = DAGDeltaAlgorithm::change_ty;
  using changeset_ty = DAGDeltaAlgorithm::changeset_ty;
  using changesetlist_ty = DAGDeltaAlgorithm::changesetlist_ty;
  using edge_ty = DAGDeltaAlgorithm::edge_ty;

خصوصی:
  using pred_iterator_ty = std::vector<change_ty>::iterator;
  using succ_iterator_ty = std::vector<change_ty>::iterator;
  using pred_closure_iterator_ty = std::set<change_ty>::iterator;
  using succ_closure_iterator_ty = std::set<change_ty>::iterator;

  DAGDeltaAlgorithm &DDA;

  std::vector<change_ty> Roots;

  /// Cache of failed test results. Successful test results are never cached
  /// since we always reduce following a success. We maintain an independent
  /// cache from that used by the individual delta passes because we may get
  /// hits across multiple individual delta invocations.
  mutable std::set<changeset_ty> FailedTestsCache;

  // FIXME: Gross.
  std::map<change_ty, std::vector<change_ty> > Predecessors;
  std::map<change_ty, std::vector<change_ty> > Successors;

  std::map<change_ty, std::set<change_ty> > PredClosure;
  std::map<change_ty, std::set<change_ty> > SuccClosure;

خصوصی:
  pred_iterator_ty pred_begin(change_ty گره) {
    أكد(Predecessors.شمار(گره) && "Invalid node!");
    بازگشت Predecessors[گره].begin();
  }
  pred_iterator_ty pred_end(change_ty گره) {
    أكد(Predecessors.شمار(گره) && "Invalid node!");
    بازگشت Predecessors[گره].end();
  }

  pred_closure_iterator_ty pred_closure_begin(change_ty گره) {
    أكد(PredClosure.شمار(گره) && "Invalid node!");
    بازگشت PredClosure[گره].begin();
  }
  pred_closure_iterator_ty pred_closure_end(change_ty گره) {
    أكد(PredClosure.شمار(گره) && "Invalid node!");
    بازگشت PredClosure[گره].end();
  }

  succ_iterator_ty succ_begin(change_ty گره) {
    أكد(Successors.شمار(گره) && "Invalid node!");
    بازگشت Successors[گره].begin();
  }
  succ_iterator_ty succ_end(change_ty گره) {
    أكد(Successors.شمار(گره) && "Invalid node!");
    بازگشت Successors[گره].end();
  }

  succ_closure_iterator_ty succ_closure_begin(change_ty گره) {
    أكد(SuccClosure.شمار(گره) && "Invalid node!");
    بازگشت SuccClosure[گره].begin();
  }
  succ_closure_iterator_ty succ_closure_end(change_ty گره) {
    أكد(SuccClosure.شمار(گره) && "Invalid node!");
    بازگشت SuccClosure[گره].end();
  }

  تهی Updated_جستجو_State(ثابت changeset_ty &Changes,
                          ثابت changesetlist_ty &Sets,
                          ثابت changeset_ty &Required) {
    DDA.Updated_جستجو_State(Changes, Sets, Required);
  }

  /// ExecuteOneTest - Execute a single test predicate on the change set \p S.
  منطقی ExecuteOneTest(ثابت changeset_ty &S) {
    // Check dependencies invariant.
    LLVM_DEBUG({
      برای (changeset_ty::const_iterator it = S.begin(), ie = S.end(); it != ie;
           ++it)
        برای (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_end(*it);
             it2 != ie2; ++it2)
          أكد(S.شمار(*it2) && "Attempt to run invalid changeset!");
    });

    بازگشت DDA.ExecuteOneTest(S);
  }

عمومی:
  DAGDeltaAlgorithmImpl(DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
                        ثابت std::vector<edge_ty> &Dependencies);

  changeset_ty Run();

  /// GetTestResult - Get the test result for the active set \p Changes with
  /// \p Required changes from the cache, executing the test if necessary.
  ///
  /// \param Changes - The set of active changes being minimized, which should
  /// have their pred closure included in the test.
  /// \param Required - The set of changes which have previously been
  /// established to be required.
  /// \return - The test result.
  منطقی GetTestResult(ثابت changeset_ty &Changes, ثابت changeset_ty &Required);
};

/// Helper object for minimizing an active set of changes.
رده DeltaActiveSetHelper : عمومی DeltaAlgorithm {
  DAGDeltaAlgorithmImpl &DDAI;

  ثابت changeset_ty &Required;

محافظتشده:
  /// UpdatedSearchState - Callback used when the search state changes.
  تهی Updated_جستجو_State(ثابت changeset_ty &Changes,
                                  ثابت changesetlist_ty &Sets) override {
    DDAI.Updated_جستجو_State(Changes, Sets, Required);
  }

  منطقی ExecuteOneTest(ثابت changeset_ty &S) override {
    بازگشت DDAI.GetTestResult(S, Required);
  }

عمومی:
  DeltaActiveSetHelper(DAGDeltaAlgorithmImpl &DDAI,
                       ثابت changeset_ty &Required)
      : DDAI(DDAI), Required(Required) {}
};

} // namespace

DAGDeltaAlgorithmImpl::DAGDeltaAlgorithmImpl(
    DAGDeltaAlgorithm &DDA, ثابت changeset_ty &Changes,
    ثابت std::vector<edge_ty> &Dependencies)
    : DDA(DDA) {
  برای (change_ty Change : Changes) {
    Predecessors.try_emplace(Change);
    Successors.try_emplace(Change);
  }
  برای (ثابت edge_ty &Dep : Dependencies) {
    Predecessors[Dep.second].افزودن_back(Dep.first);
    Successors[Dep.first].افزودن_back(Dep.second);
  }

  // Compute the roots.
  برای (change_ty Change : Changes)
    اگر (succ_begin(Change) == succ_end(Change))
      Roots.افزودن_back(Change);

  // Pre-compute the closure of the successor relation.
  std::vector<change_ty> Worklist(Roots.begin(), Roots.end());
  وقتی (!Worklist.empty()) {
    change_ty Change = Worklist.back();
    Worklist.برداشت_back();

    std::set<change_ty> &ChangeSuccs = SuccClosure[Change];
    برای (pred_iterator_ty it = pred_begin(Change),
           ie = pred_end(Change); it != ie; ++it) {
      auto &SC = SuccClosure[*it];
      SC.درج(Change);
      SC.درج(ChangeSuccs.begin(), ChangeSuccs.end());
      Worklist.افزودن_back(*it);
    }
  }

  // Invert to form the predecessor closure map.
  برای (change_ty Change : Changes)
    PredClosure.try_emplace(Change);
  برای (change_ty Change : Changes)
    برای (succ_closure_iterator_ty it2 = succ_closure_begin(Change),
                                  ie2 = succ_closure_end(Change);
         it2 != ie2; ++it2)
      PredClosure[*it2].درج(Change);

  // Dump useful debug info.
  LLVM_DEBUG({
    llvm::errs() << "-- DAGDeltaAlgorithmImpl --\n";
    llvm::errs() << "Changes: [";
    برای (changeset_ty::const_iterator it = Changes.begin(), ie = Changes.end();
         it != ie; ++it) {
      اگر (it != Changes.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;

      اگر (succ_begin(*it) != succ_end(*it)) {
        llvm::errs() << "(";
        برای (succ_iterator_ty it2 = succ_begin(*it), ie2 = succ_end(*it);
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
    برای (std::vector<change_ty>::const_iterator it = Roots.begin(),
                                                ie = Roots.end();
         it != ie; ++it) {
      اگر (it != Roots.begin())
        llvm::errs() << ", ";
      llvm::errs() << *it;
    }
    llvm::errs() << "]\n";

    llvm::errs() << "Predecessor Closure:\n";
    برای (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      برای (pred_closure_iterator_ty it2 = pred_closure_begin(Change),
                                    ie2 = pred_closure_end(Change);
           it2 != ie2; ++it2) {
        اگر (it2 != pred_closure_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "Successor Closure:\n";
    برای (change_ty Change : Changes) {
      llvm::errs() << format("  %-4d: [", Change);
      برای (succ_closure_iterator_ty it2 = succ_closure_begin(Change),
                                    ie2 = succ_closure_end(Change);
           it2 != ie2; ++it2) {
        اگر (it2 != succ_closure_begin(Change))
          llvm::errs() << ", ";
        llvm::errs() << *it2;
      }
      llvm::errs() << "]\n";
    }

    llvm::errs() << "\n\n";
  });
}

منطقی DAGDeltaAlgorithmImpl::GetTestResult(ثابت changeset_ty &Changes,
                                          ثابت changeset_ty &Required) {
  changeset_ty Extended(Required);
  Extended.درج(Changes.begin(), Changes.end());
  برای (change_ty Change : Changes)
    Extended.درج(pred_closure_begin(Change), pred_closure_end(Change));

  اگر (FailedTestsCache.شمار(Extended))
    بازگشت نادرست;

  منطقی Result = ExecuteOneTest(Extended);
  اگر (!Result)
    FailedTestsCache.درج(Extended);

  بازگشت Result;
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithmImpl::Run() {
  // The current set of changes we are minimizing, starting at the roots.
  changeset_ty CurrentSet(Roots.begin(), Roots.end());

  // The set of required changes.
  changeset_ty Required;

  // Iterate until the active set of changes is empty. Convergence is guaranteed
  // assuming input was a DAG.
  //
  // Invariant:  CurrentSet intersect Required == {}
  // Invariant:  Required == (Required union succ*(Required))
  وقتی (!CurrentSet.empty()) {
    LLVM_DEBUG({
      llvm::errs() << "DAG_DD - " << CurrentSet.size() << " active changes, "
                   << Required.size() << " required changes\n";
    });

    // Minimize the current set of changes.
    DeltaActiveSetHelper Helper(*این, Required);
    changeset_ty CurrentMinSet = Helper.Run(CurrentSet);

    // Update the set of required changes. Since
    //   CurrentMinSet subset CurrentSet
    // and after the last iteration,
    //   succ(CurrentSet) subset Required
    // then
    //   succ(CurrentMinSet) subset Required
    // and our invariant on Required is maintained.
    Required.درج(CurrentMinSet.begin(), CurrentMinSet.end());

    // Replace the current set with the predecssors of the minimized set of
    // active changes.
    CurrentSet.clear();
    برای (change_ty CT : CurrentMinSet)
      CurrentSet.درج(pred_begin(CT), pred_end(CT));

    // FIXME: We could enforce CurrentSet intersect Required == {} here if we
    // wanted to protect against cyclic graphs.
  }

  بازگشت Required;
}

تهی DAGDeltaAlgorithm::anchor() {
}

DAGDeltaAlgorithm::changeset_ty
DAGDeltaAlgorithm::Run(ثابت changeset_ty &Changes,
                       ثابت std::vector<edge_ty> &Dependencies) {
  بازگشت DAGDeltaAlgorithmImpl(*این, Changes, Dependencies).Run();
}
