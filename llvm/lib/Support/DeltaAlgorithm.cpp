//===--- DeltaAlgorithm.cpp - A Set Minimization Algorithm -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/DeltaAlgorithm.h"
#شامل <algorithm>
#شامل <iterator>
using فضاینام llvm;

DeltaAlgorithm::~DeltaAlgorithm() = پیشفرض;

منطقی DeltaAlgorithm::GetTestResult(ثابت changeset_ty &Changes) {
  اگر (FailedTestsCache.شمار(Changes))
    بازگشت نادرست;

  منطقی Result = ExecuteOneTest(Changes);
  اگر (!Result)
    FailedTestsCache.درج(Changes);

  بازگشت Result;
}

تهی DeltaAlgorithm::Split(ثابت changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  برای (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).درج(*it);
  اگر (!LHS.empty())
    Res.افزودن_back(LHS);
  اگر (!RHS.empty())
    Res.افزودن_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(ثابت changeset_ty &Changes,
                      ثابت changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  Updated_جستجو_State(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  اگر (Sets.size() <= 1)
    بازگشت Changes;

  // Look for a passing subset.
  changeset_ty Res;
  اگر (جستجو(Changes, Sets, Res))
    بازگشت Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  برای (ثابت changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  اگر (SplitSets.size() == Sets.size())
    بازگشت Changes;

  بازگشت Delta(Changes, SplitSets);
}

منطقی DeltaAlgorithm::جستجو(ثابت changeset_ty &Changes,
                            ثابت changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  برای (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    اگر (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      بازگشت درست;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    اگر (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      اگر (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.درج(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.درج(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        بازگشت درست;
      }
    }
  }

  بازگشت نادرست;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(ثابت changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  اگر (GetTestResult(changeset_ty()))
    بازگشت changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  بازگشت Delta(Changes, Sets);
}
