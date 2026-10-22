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
  ᱡᱩᱫᱤ (FailedTestsCache.count(Changes))
    ᱨᱩᱣᱟᱹᱲ false;

  bool Result = ExecuteOneTest(Changes);
  ᱡᱩᱫᱤ (!Result)
    FailedTestsCache.insert(Changes);

  ᱨᱩᱣᱟᱹᱲ Result;
}

void DeltaAlgorithm::Split(const changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  for (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  ᱡᱩᱫᱤ (!LHS.empty())
    Res.push_back(LHS);
  ᱡᱩᱫᱤ (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(const changeset_ty &Changes,
                      const changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  ᱡᱩᱫᱤ (Sets.size() <= 1)
    ᱨᱩᱣᱟᱹᱲ Changes;

  // Look for a passing subset.
  changeset_ty Res;
  ᱡᱩᱫᱤ (Search(Changes, Sets, Res))
    ᱨᱩᱣᱟᱹᱲ Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  for (const changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  ᱡᱩᱫᱤ (SplitSets.size() == Sets.size())
    ᱨᱩᱣᱟᱹᱲ Changes;

  ᱨᱩᱣᱟᱹᱲ Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::Search(const changeset_ty &Changes,
                            const changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  for (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    ᱡᱩᱫᱤ (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      ᱨᱩᱣᱟᱹᱲ true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    ᱡᱩᱫᱤ (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      ᱡᱩᱫᱤ (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        ᱨᱩᱣᱟᱹᱲ true;
      }
    }
  }

  ᱨᱩᱣᱟᱹᱲ false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(const changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  ᱡᱩᱫᱤ (GetTestResult(changeset_ty()))
    ᱨᱩᱣᱟᱹᱲ changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  ᱨᱩᱣᱟᱹᱲ Delta(Changes, Sets);
}
