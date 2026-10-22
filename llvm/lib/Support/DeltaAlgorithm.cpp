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
  အကယ်၍ (FailedTestsCache.count(Changes))
    ပြန်ပေး false;

  bool Result = ExecuteOneTest(Changes);
  အကယ်၍ (!Result)
    FailedTestsCache.insert(Changes);

  ပြန်ပေး Result;
}

void DeltaAlgorithm::Split(const changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  unsigned idx = 0, N = S.size() / 2;
  အတွက် (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).insert(*it);
  အကယ်၍ (!LHS.empty())
    Res.push_back(LHS);
  အကယ်၍ (!RHS.empty())
    Res.push_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(const changeset_ty &Changes,
                      const changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  UpdatedSearchState(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  အကယ်၍ (Sets.size() <= 1)
    ပြန်ပေး Changes;

  // Look for a passing subset.
  changeset_ty Res;
  အကယ်၍ (Search(Changes, Sets, Res))
    ပြန်ပေး Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  အတွက် (const changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  အကယ်၍ (SplitSets.size() == Sets.size())
    ပြန်ပေး Changes;

  ပြန်ပေး Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::Search(const changeset_ty &Changes,
                            const changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  အတွက် (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    အကယ်၍ (GetTestResult(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      ပြန်ပေး true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    အကယ်၍ (Sets.size() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      အကယ်၍ (GetTestResult(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.insert(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.insert(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        ပြန်ပေး true;
      }
    }
  }

  ပြန်ပေး false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(const changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  အကယ်၍ (GetTestResult(changeset_ty()))
    ပြန်ပေး changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  ပြန်ပေး Delta(Changes, Sets);
}
