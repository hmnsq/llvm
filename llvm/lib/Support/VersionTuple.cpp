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

#अन्तर्भाव "llvm/Support/VersionTuple.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>

using नामाकाश llvm;

std::string VersionTuple::getAsString() स्थिरांक {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *हे;
  }
  परत Result;
}

raw_ostream &llvm::चालक<<(raw_ostream &Out, स्थिरांक VersionTuple &V) {
  Out << V.getMajor();
  जर (std::optional<अचिन्हित> Minor = V.getMinor())
    Out << '.' << *Minor;
  जर (std::optional<अचिन्हित> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  जर (std::optional<अचिन्हित> Build = V.getBuild())
    Out << '.' << *Build;
  जर (std::optional<अचिन्हित> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  परत Out;
}

स्थिर तार्किक parseInt(StringRef &आदान_2, अचिन्हित &मूल्य_2) {
  प्रतिज्ञा(मूल्य_2 == 0);
  जर (आदान_2.empty())
    परत सत्य;

  अक्षर next = आदान_2[0];
  आदान_2 = आदान_2.substr(1);
  जर (next < '0' || next > '9')
    परत सत्य;
  मूल्य_2 = (अचिन्हित)(next - '0');

  पर्यंत (!आदान_2.empty()) {
    next = आदान_2[0];
    जर (next < '0' || next > '9')
      परत असत्य;
    आदान_2 = आदान_2.substr(1);
    मूल्य_2 = मूल्य_2 * 10 + (अचिन्हित)(next - '0');
  }

  परत असत्य;
}

तार्किक VersionTuple::tryParse(StringRef आदान_2) {
  अचिन्हित major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  जर (parseInt(आदान_2, major))
    परत सत्य;

  जर (आदान_2.empty()) {
    *हे = VersionTuple(major);
    परत असत्य;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  जर (आदान_2[0] != '.')
    परत सत्य;
  आदान_2 = आदान_2.substr(1);
  जर (parseInt(आदान_2, minor))
    परत सत्य;

  जर (आदान_2.empty()) {
    *हे = VersionTuple(major, minor);
    परत असत्य;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  जर (!आदान_2.consume_front("."))
    परत सत्य;
  जर (parseInt(आदान_2, subminor))
    परत सत्य;

  जर (आदान_2.empty()) {
    *हे = VersionTuple(major, minor, subminor);
    परत असत्य;
  }

  // If we're not done, parse the build version, \.[0-9]+
  जर (!आदान_2.consume_front("."))
    परत सत्य;
  जर (parseInt(आदान_2, build))
    परत सत्य;
  जर (build >= 1024 * 1024)
    परत सत्य;

  जर (आदान_2.empty()) {
    *हे = VersionTuple(major, minor, subminor, build);
    परत असत्य;
  }

  // And the subbuild version, \.[0-9]+
  जर (!आदान_2.consume_front("."))
    परत सत्य;
  जर (parseInt(आदान_2, subbuild))
    परत सत्य;
  जर (subbuild >= 1024)
    परत सत्य;

  // If we have characters left over, it's an error.
  जर (!आदान_2.empty())
    परत सत्य;

  *हे = VersionTuple(major, minor, subminor, build, subbuild);
  परत असत्य;
}

VersionTuple VersionTuple::withMajorReplaced(अचिन्हित NewMajor) स्थिरांक {
  जर (HasSubbuild)
    परत VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  जर (HasBuild)
    परत VersionTuple(NewMajor, Minor, Subminor, Build);
  जर (HasSubminor)
    परत VersionTuple(NewMajor, Minor, Subminor);
  जर (HasMinor)
    परत VersionTuple(NewMajor, Minor);
  परत VersionTuple(NewMajor);
}
