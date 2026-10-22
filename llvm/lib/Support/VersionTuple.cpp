//===- VersionTuple.cpp - Version Number Handling ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the VersionTuple class, which represents a version in
// the form major[.minor[.subminor]].
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/VersionTuple.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>

using ನಾಮಾಕಾಶ llvm;

std::string VersionTuple::getAsString() ಧ್ರುವ {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *ಏತತ್;
  }
  ಪ್ರತ್ಯಾಗಮ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ಧ್ರುವ VersionTuple &V) {
  Out << V.getMajor();
  ಯದಿ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ಯದಿ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ಯದಿ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ಯದಿ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ಪ್ರತ್ಯಾಗಮ Out;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ parseInt(StringRef &ಒಳಹರಿವು_2, unsigned &ಮೌಲ್ಯ_2) {
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ_2 == 0);
  ಯದಿ (ಒಳಹರಿವು_2.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ವರ್ಣ next = ಒಳಹರಿವು_2[0];
  ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
  ಯದಿ (next < '0' || next > '9')
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಮೌಲ್ಯ_2 = (unsigned)(next - '0');

  ಯಾವತ್ (!ಒಳಹರಿವು_2.empty()) {
    next = ಒಳಹರಿವು_2[0];
    ಯದಿ (next < '0' || next > '9')
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
    ಮೌಲ್ಯ_2 = ಮೌಲ್ಯ_2 * 10 + (unsigned)(next - '0');
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ VersionTuple::tryParse(StringRef ಒಳಹರಿವು_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ಯದಿ (parseInt(ಒಳಹರಿವು_2, major))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (ಒಳಹರಿವು_2.empty()) {
    *ಏತತ್ = VersionTuple(major);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ಯದಿ (ಒಳಹರಿವು_2[0] != '.')
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
  ಯದಿ (parseInt(ಒಳಹರಿವು_2, minor))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (ಒಳಹರಿವು_2.empty()) {
    *ಏತತ್ = VersionTuple(major, minor);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ಯದಿ (!ಒಳಹರಿವು_2.consume_front("."))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (parseInt(ಒಳಹರಿವು_2, subminor))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (ಒಳಹರಿವು_2.empty()) {
    *ಏತತ್ = VersionTuple(major, minor, subminor);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ಯದಿ (!ಒಳಹರಿವು_2.consume_front("."))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (parseInt(ಒಳಹರಿವು_2, build))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (build >= 1024 * 1024)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (ಒಳಹರಿವು_2.empty()) {
    *ಏತತ್ = VersionTuple(major, minor, subminor, build);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // And the subbuild version, \.[0-9]+
  ಯದಿ (!ಒಳಹರಿವು_2.consume_front("."))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (parseInt(ಒಳಹರಿವು_2, subbuild))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (subbuild >= 1024)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // If we have characters left over, it's an error.
  ಯದಿ (!ಒಳಹರಿವು_2.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  *ಏತತ್ = VersionTuple(major, minor, subminor, build, subbuild);
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ಧ್ರುವ {
  ಯದಿ (HasSubbuild)
    ಪ್ರತ್ಯಾಗಮ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ಯದಿ (HasBuild)
    ಪ್ರತ್ಯಾಗಮ VersionTuple(NewMajor, Minor, Subminor, Build);
  ಯದಿ (HasSubminor)
    ಪ್ರತ್ಯಾಗಮ VersionTuple(NewMajor, Minor, Subminor);
  ಯದಿ (HasMinor)
    ಪ್ರತ್ಯಾಗಮ VersionTuple(NewMajor, Minor);
  ಪ್ರತ್ಯಾಗಮ VersionTuple(NewMajor);
}
