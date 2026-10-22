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
  ᬬᬾᬦ᭄ (FailedTestsCache.count(Changes))
    ᬯᬮᬶ false;

  bool Result = ExecuteOneTest(Changes);
  ᬬᬾᬦ᭄ (!Result)
    FailedTestsCache.insert(Changes);

  ᬯᬮᬶ Result;
}

void DeltaAlgorithm::Split(const changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  for (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  ᬬᬾᬦ᭄ (!LHS.empty())
    Res.push_back(LHS);
  ᬬᬾᬦ᭄ (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(const changeset_ty &Changes,
                      const changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  ᬬᬾᬦ᭄ (Sets.size() <= 1)
    ᬯᬮᬶ Changes;

  // Look for a passing subset.
  changeset_ty Res;
  ᬬᬾᬦ᭄ (Search(Changes, Sets, Res))
    ᬯᬮᬶ Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  for (const changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  ᬬᬾᬦ᭄ (SplitSets.size() == Sets.size())
    ᬯᬮᬶ Changes;

  ᬯᬮᬶ Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::Search(const changeset_ty &Changes,
                            const changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  for (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    ᬬᬾᬦ᭄ (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      ᬯᬮᬶ true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    ᬬᬾᬦ᭄ (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      ᬬᬾᬦ᭄ (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        ᬯᬮᬶ true;
      }
    }
  }

  ᬯᬮᬶ false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(const changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  ᬬᬾᬦ᭄ (GetTestResult(changeset_ty()))
    ᬯᬮᬶ changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  ᬯᬮᬶ Delta(Changes, Sets);
}
