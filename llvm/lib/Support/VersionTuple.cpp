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

#شامل "llvm/Support/VersionTuple.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>

using فضاینام llvm;

std::string VersionTuple::getAsString() ثابت {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *این;
  }
  بازگشت Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ثابت VersionTuple &V) {
  Out << V.getMajor();
  اگر (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  اگر (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  اگر (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  اگر (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  بازگشت Out;
}

ایستا منطقی parseInt(StringRef &درون_داد_2, unsigned &مقدار_2) {
  أكد(مقدار_2 == 0);
  اگر (درون_داد_2.empty())
    بازگشت درست;

  نویسه next = درون_داد_2[0];
  درون_داد_2 = درون_داد_2.substr(1);
  اگر (next < '0' || next > '9')
    بازگشت درست;
  مقدار_2 = (unsigned)(next - '0');

  وقتی (!درون_داد_2.empty()) {
    next = درون_داد_2[0];
    اگر (next < '0' || next > '9')
      بازگشت نادرست;
    درون_داد_2 = درون_داد_2.substr(1);
    مقدار_2 = مقدار_2 * 10 + (unsigned)(next - '0');
  }

  بازگشت نادرست;
}

منطقی VersionTuple::tryParse(StringRef درون_داد_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  اگر (parseInt(درون_داد_2, major))
    بازگشت درست;

  اگر (درون_داد_2.empty()) {
    *این = VersionTuple(major);
    بازگشت نادرست;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  اگر (درون_داد_2[0] != '.')
    بازگشت درست;
  درون_داد_2 = درون_داد_2.substr(1);
  اگر (parseInt(درون_داد_2, minor))
    بازگشت درست;

  اگر (درون_داد_2.empty()) {
    *این = VersionTuple(major, minor);
    بازگشت نادرست;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  اگر (!درون_داد_2.consume_front("."))
    بازگشت درست;
  اگر (parseInt(درون_داد_2, subminor))
    بازگشت درست;

  اگر (درون_داد_2.empty()) {
    *این = VersionTuple(major, minor, subminor);
    بازگشت نادرست;
  }

  // If we're not done, parse the build version, \.[0-9]+
  اگر (!درون_داد_2.consume_front("."))
    بازگشت درست;
  اگر (parseInt(درون_داد_2, build))
    بازگشت درست;
  اگر (build >= 1024 * 1024)
    بازگشت درست;

  اگر (درون_داد_2.empty()) {
    *این = VersionTuple(major, minor, subminor, build);
    بازگشت نادرست;
  }

  // And the subbuild version, \.[0-9]+
  اگر (!درون_داد_2.consume_front("."))
    بازگشت درست;
  اگر (parseInt(درون_داد_2, subbuild))
    بازگشت درست;
  اگر (subbuild >= 1024)
    بازگشت درست;

  // If we have characters left over, it's an error.
  اگر (!درون_داد_2.empty())
    بازگشت درست;

  *این = VersionTuple(major, minor, subminor, build, subbuild);
  بازگشت نادرست;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  اگر (HasSubbuild)
    بازگشت VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  اگر (HasBuild)
    بازگشت VersionTuple(NewMajor, Minor, Subminor, Build);
  اگر (HasSubminor)
    بازگشت VersionTuple(NewMajor, Minor, Subminor);
  اگر (HasMinor)
    بازگشت VersionTuple(NewMajor, Minor);
  بازگشت VersionTuple(NewMajor);
}
