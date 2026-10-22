//===--- DeltaAlgorithm.cpp - A Set Minimization Algorithm -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/DeltaAlgorithm.h"
#ضمن <algorithm>
#ضمن <iterator>
استفاده نطاق llvm;

DeltaAlgorithm::~DeltaAlgorithm() = افتراضي;

منطقي DeltaAlgorithm::Get_Test_نتیجه(ثابت changeset_ty &Changes) {
  إذا (FailedTestsCache.شمار(Changes))
    أرجع خطأ;

  منطقي نتیجه = ExecuteOneTest(Changes);
  إذا (!نتیجه)
    FailedTestsCache.درج(Changes);

  أرجع نتیجه;
}

فراغ DeltaAlgorithm::تقسیم_2(ثابت changeset_ty &S, changesetlist_ty &Res) {
  // FIXME: Allow clients to provide heuristics for improved splitting.

  // FIXME: This is really slow.
  changeset_ty LHS, RHS;
  بیعلامت idx = 0, N = S.اندازه_2() / 2;
  لكل (changeset_ty::const_iterator it = S.begin(),
         ie = S.انتها(); it != ie; ++it, ++idx)
    ((idx < N) ? LHS : RHS).درج(*it);
  إذا (!LHS.خالی())
    Res.افزودن_پشت(LHS);
  إذا (!RHS.خالی())
    Res.افزودن_پشت(RHS);
}

DeltaAlgorithm::changeset_ty
DeltaAlgorithm::Delta(ثابت changeset_ty &Changes,
                      ثابت changesetlist_ty &Sets) {
  // Invariant: union(Res) == Changes
  Updated_جستجو_State(Changes, Sets);

  // If there is nothing left we can remove, we are done.
  إذا (Sets.اندازه_2() <= 1)
    أرجع Changes;

  // Look for a passing subset.
  changeset_ty Res;
  إذا (جستجو(Changes, Sets, Res))
    أرجع Res;

  // Otherwise, partition the sets if possible; if not we are done.
  changesetlist_ty تقسیم_Sets;
  لكل (ثابت changeset_ty &مجموعه_2 : Sets)
    تقسیم_2(مجموعه_2, تقسیم_Sets);
  إذا (تقسیم_Sets.اندازه_2() == Sets.اندازه_2())
    أرجع Changes;

  أرجع Delta(Changes, تقسیم_Sets);
}

منطقي DeltaAlgorithm::جستجو(ثابت changeset_ty &Changes,
                            ثابت changesetlist_ty &Sets,
                            changeset_ty &Res) {
  // FIXME: Parallelize.
  لكل (changesetlist_ty::const_iterator it = Sets.begin(),
         ie = Sets.انتها(); it != ie; ++it) {
    // If the test passes on this subset alone, recurse.
    إذا (Get_Test_نتیجه(*it)) {
      changesetlist_ty Sets;
      تقسیم_2(*it, Sets);
      Res = Delta(*it, Sets);
      أرجع درست;
    }

    // Otherwise, if we have more than two sets, see if test passes on the
    // complement.
    إذا (Sets.اندازه_2() > 2) {
      // FIXME: This is really slow.
      changeset_ty Complement;
      std::مجموعه_difference(Changes.begin(), Changes.انتها(), it->begin(),
                          it->انتها(),
                          std::inserter(Complement, Complement.begin()));
      إذا (Get_Test_نتیجه(Complement)) {
        changesetlist_ty ComplementSets;
        ComplementSets.درج(ComplementSets.انتها(), Sets.begin(), it);
        ComplementSets.درج(ComplementSets.انتها(), it + 1, Sets.انتها());
        Res = Delta(Complement, ComplementSets);
        أرجع درست;
      }
    }
  }

  أرجع خطأ;
}

DeltaAlgorithm::changeset_ty DeltaAlgorithm::پیاپی_2(ثابت changeset_ty &Changes) {
  // Check empty set first to quickly find poor test functions.
  إذا (Get_Test_نتیجه(changeset_ty()))
    أرجع changeset_ty();

  // Otherwise run the real delta algorithm.
  changesetlist_ty Sets;
  تقسیم_2(Changes, Sets);

  أرجع Delta(Changes, Sets);
}
