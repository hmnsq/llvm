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
  אם (FailedTestsCache.count(Changes))
    החזר שקר;

  منطقي Result = ExecuteOneTest(Changes);
  אם (!Result)
    FailedTestsCache.insert(Changes);

  החזר Result;
}

فراغ DeltaAlgorithm::Split(ثابت changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  עבור (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  אם (!LHS.empty())
    Res.push_back(LHS);
  אם (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(ثابت changeset_ty &Changes,
                      ثابت changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  אם (Sets.size() <= 1)
    החזר Changes;

  // Look for a passing subset.
  changeset_ty Res;
  אם (Search(Changes, Sets, Res))
    החזר Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  עבור (ثابت changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  אם (SplitSets.size() == Sets.size())
    החזר Changes;

  החזר Delta(Changes, SplitSets);
}

منطقي DeltaAlgorithm::Search(ثابت changeset_ty &Changes,
                            ثابت changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  עבור (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    אם (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      החזר אמת;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    אם (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      אם (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        החזר אמת;
      }
    }
  }

  החזר שקר;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(ثابت changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  אם (GetTestResult(changeset_ty()))
    החזר changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  החזר Delta(Changes, Sets);
}
