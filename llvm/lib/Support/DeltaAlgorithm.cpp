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
  ако (FailedTestsCache.count(Changes))
    врати false;

  bool Result = ExecuteOneTest(Changes);
  ако (!Result)
    FailedTestsCache.insert(Changes);

  врати Result;
}

void DeltaAlgorithm::Split(const changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  за (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  ако (!LHS.empty())
    Res.push_back(LHS);
  ако (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(const changeset_ty &Changes,
                      const changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  ако (Sets.size() <= 1)
    врати Changes;

  // Look for a passing subset.
  changeset_ty Res;
  ако (Search(Changes, Sets, Res))
    врати Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  за (const changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  ако (SplitSets.size() == Sets.size())
    врати Changes;

  врати Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::Search(const changeset_ty &Changes,
                            const changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  за (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    ако (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      врати true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    ако (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      ако (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        врати true;
      }
    }
  }

  врати false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(const changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  ако (GetTestResult(changeset_ty()))
    врати changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  врати Delta(Changes, Sets);
}
