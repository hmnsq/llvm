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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VersionTuple.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cassert>

using ନାମାକାଶ llvm;

std::string VersionTuple::getAsString() ଅଚଳ {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *ଏହି;
  }
  ଫେରନ୍ତୁ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ଅଚଳ VersionTuple &V) {
  Out << V.getMajor();
  ଯଦି (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ଯଦି (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ଯଦି (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ଯଦି (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ଫେରନ୍ତୁ Out;
}

ସ୍ଥିର ତାର୍କିକ parseInt(StringRef &ନିବେଶ_2, unsigned &ମୂଲ୍ୟ_2) {
  ପ୍ରତିଜ୍ଞା(ମୂଲ୍ୟ_2 == 0);
  ଯଦି (ନିବେଶ_2.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅକ୍ଷର next = ନିବେଶ_2[0];
  ନିବେଶ_2 = ନିବେଶ_2.substr(1);
  ଯଦି (next < '0' || next > '9')
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ମୂଲ୍ୟ_2 = (unsigned)(next - '0');

  ଯେତେବେଳେ (!ନିବେଶ_2.empty()) {
    next = ନିବେଶ_2[0];
    ଯଦି (next < '0' || next > '9')
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    ନିବେଶ_2 = ନିବେଶ_2.substr(1);
    ମୂଲ୍ୟ_2 = ମୂଲ୍ୟ_2 * 10 + (unsigned)(next - '0');
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ VersionTuple::tryParse(StringRef ନିବେଶ_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ଯଦି (parseInt(ନିବେଶ_2, major))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (ନିବେଶ_2.empty()) {
    *ଏହି = VersionTuple(major);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ଯଦି (ନିବେଶ_2[0] != '.')
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ନିବେଶ_2 = ନିବେଶ_2.substr(1);
  ଯଦି (parseInt(ନିବେଶ_2, minor))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (ନିବେଶ_2.empty()) {
    *ଏହି = VersionTuple(major, minor);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ଯଦି (!ନିବେଶ_2.consume_front("."))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (parseInt(ନିବେଶ_2, subminor))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (ନିବେଶ_2.empty()) {
    *ଏହି = VersionTuple(major, minor, subminor);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ଯଦି (!ନିବେଶ_2.consume_front("."))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (parseInt(ନିବେଶ_2, build))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (build >= 1024 * 1024)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (ନିବେଶ_2.empty()) {
    *ଏହି = VersionTuple(major, minor, subminor, build);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // And the subbuild version, \.[0-9]+
  ଯଦି (!ନିବେଶ_2.consume_front("."))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (parseInt(ନିବେଶ_2, subbuild))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (subbuild >= 1024)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // If we have characters left over, it's an error.
  ଯଦି (!ନିବେଶ_2.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  *ଏହି = VersionTuple(major, minor, subminor, build, subbuild);
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ଅଚଳ {
  ଯଦି (HasSubbuild)
    ଫେରନ୍ତୁ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ଯଦି (HasBuild)
    ଫେରନ୍ତୁ VersionTuple(NewMajor, Minor, Subminor, Build);
  ଯଦି (HasSubminor)
    ଫେରନ୍ତୁ VersionTuple(NewMajor, Minor, Subminor);
  ଯଦି (HasMinor)
    ଫେରନ୍ତୁ VersionTuple(NewMajor, Minor);
  ଫେରନ୍ତୁ VersionTuple(NewMajor);
}
