//===--- DeltaAlgorithm.cpp - A Set Minimization Algorithm -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/DeltaAlgorithm.h"
#ضمن <algorithm>
#ضمن <iterator>
using نطاق llvm;

DeltaAlgorithm::~DeltaAlgorithm() = افتراضي;

منطقي DeltaAlgorithm::GetTestResult(ثابت changeset_ty &Changes) {
  إذا (FailedTestsCache.تعداد_2(Changes))
    إرجاع خطأ;

  منطقي Result = ExecuteOneTest(Changes);
  إذا (!Result)
    FailedTestsCache.أدرج(Changes);

  إرجاع Result;
}

فراغ DeltaAlgorithm::Split(ثابت changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  لكل (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).أدرج(*it);
  إذا (!LHS.empty())
    Res.ادفع_back(LHS);
  إذا (!RHS.empty())
    Res.ادفع_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(ثابت changeset_ty &Changes,
                      ثابت changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  Updated_بحث_State(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  إذا (Sets.size() <= 1)
    إرجاع Changes;

  // Look for a passing subset.
  changeset_ty Res;
  إذا (بحث(Changes, Sets, Res))
    إرجاع Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  لكل (ثابت changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  إذا (SplitSets.size() == Sets.size())
    إرجاع Changes;

  إرجاع Delta(Changes, SplitSets);
}

منطقي DeltaAlgorithm::بحث(ثابت changeset_ty &Changes,
                            ثابت changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  لكل (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    إذا (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      إرجاع true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    إذا (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      إذا (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.أدرج(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.أدرج(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        إرجاع true;
      }
    }
  }

  إرجاع خطأ;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(ثابت changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  إذا (GetTestResult(changeset_ty()))
    إرجاع changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  إرجاع Delta(Changes, Sets);
}
