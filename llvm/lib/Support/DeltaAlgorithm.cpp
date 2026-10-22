//===--- DeltaAlgorithm.cpp - A Set Minimization Algorithm -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/DeltaAlgorithm.h"
#समावेश <algorithm>
#समावेश <iterator>
using namespace llvm;

DeltaAlgorithm::~DeltaAlgorithm() = शेष;

bool DeltaAlgorithm::Get_Test_परिणाम(स्थिर changeset_ty &Changes) {
  अगर (FailedTestsCache.गिनती(Changes))
    वापस false;

  bool परिणाम = ExecuteOneTest(Changes);
  अगर (!परिणाम)
    FailedTestsCache.जोड़ो(Changes);

  वापस परिणाम;
}

व्योम DeltaAlgorithm::Split(स्थिर changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  अचिन्हित idx = 0, N = S.आकार_2() / 2;
  क्रम (changeset_ty::const_iterator it = S.begin(),
         ie = S.end(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).जोड़ो(*it);
  अगर (!LHS.empty())
    Res.धकेलो_back(LHS);
  अगर (!RHS.empty())
    Res.धकेलो_back(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(स्थिर changeset_ty &Changes,
                      स्थिर changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  Updated_खोज_State(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  अगर (Sets.आकार_2() <= 1)
    वापस Changes;

  // Look for a passing subset.
  changeset_ty Res;
  अगर (खोज(Changes, Sets, Res))
    वापस Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty SplitSets;
  क्रम (स्थिर changeset_ty &Set : Sets)
    Split(Set, SplitSets);
  अगर (SplitSets.आकार_2() == Sets.आकार_2())
    वापस Changes;

  वापस Delta(Changes, SplitSets);
}

bool DeltaAlgorithm::खोज(स्थिर changeset_ty &Changes,
                            स्थिर changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  क्रम (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.end(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    अगर (Get_Test_परिणाम(*it)) {
      changesetlist_ty Sets;
      Split(*it, Sets);
      Res = Delta(*it, Sets);
      वापस true;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    अगर (Sets.आकार_2() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::set_difference(Changes.begin(), Changes.end(), it->begin(),
                          it->end(),
                          std::inserter(Complement, Complement.begin()));
      अगर (Get_Test_परिणाम(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.जोड़ो(ComplementSets.end(), Sets.begin(), it);
        ComplementSets.जोड़ो(ComplementSets.end(), it + 1, Sets.end());
        Res = Delta(Complement, ComplementSets);
        वापस true;
      }
    }
  }

  वापस false;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::Run(स्थिर changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  अगर (Get_Test_परिणाम(changeset_ty()))
    वापस changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  Split(Changes, Sets);

  वापस Delta(Changes, Sets);
}
