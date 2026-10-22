//===--- DeltaAlgorithm.cpp - A Set Minimization Algorithm -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DeltaAlgorithm.h"
#include <algorithm>
#include <iterator>
using namespace llvm;

DeltaAlgorithm::~DeltaAlgorithm() = default;

bool DeltaAlgorithm::GetTestResult(const changeset_ty &Changes) {
  만약 (FailedTestsCache.count(Changes))
    반환 false;

  bool Result = ExecuteOneTest(Changes);
  만약 (!Result)
    FailedTestsCache.insert(Changes);

  반환 Result;
}

void DeltaAlgorithm::Split(const changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  위해 (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  만약 (!LHS.empty())
    Res.push_back(LHS);
  만약 (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(const changeset_ty &Changes,
                      const changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  만약 (Sets.size() <= 1)
    반환 Changes;

  // Look for a passing subset.
  changeset_ty Res;
  만약 (Search(Changes, Sets, Res))
    반환 Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  위해 (const changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  만약 (SplitSets.size() == Sets.size())
    반환 Changes;

  반환 Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::Search(const changeset_ty &Changes,
                            const changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  위해 (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    만약 (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      반환 true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    만약 (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      만약 (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        반환 true;
      }
    }
  }

  반환 false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(const changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  만약 (GetTestResult(changeset_ty()))
    반환 changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  반환 Delta(Changes, Sets);
}
