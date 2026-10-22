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

#include "llvm/Support/VersionTuple.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

std::string VersionTuple::getAsString() const {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *this;
  }
  ᤀᤡᤴ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᤀᤡ᤺ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᤀᤡ᤺ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᤀᤡ᤺ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᤀᤡ᤺ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᤀᤡᤴ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᤀᤡ᤺ (input.empty())
    ᤀᤡᤴ true;

  var_na next = input[0];
  input = input.substr(1);
  ᤀᤡ᤺ (next < '0' || next > '9')
    ᤀᤡᤴ true;
  value = (unsigned)(next - '0');

  ᤀᤠ᤺ (!input.empty()) {
    next = input[0];
    ᤀᤡ᤺ (next < '0' || next > '9')
      ᤀᤡᤴ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᤀᤡᤴ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᤀᤡ᤺ (parseInt(input, major))
    ᤀᤡᤴ true;

  ᤀᤡ᤺ (input.empty()) {
    *this = VersionTuple(major);
    ᤀᤡᤴ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᤀᤡ᤺ (input[0] != '.')
    ᤀᤡᤴ true;
  input = input.substr(1);
  ᤀᤡ᤺ (parseInt(input, minor))
    ᤀᤡᤴ true;

  ᤀᤡ᤺ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᤀᤡᤴ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᤀᤡ᤺ (!input.consume_front("."))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (parseInt(input, subminor))
    ᤀᤡᤴ true;

  ᤀᤡ᤺ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᤀᤡᤴ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᤀᤡ᤺ (!input.consume_front("."))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (parseInt(input, build))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (build >= 1024 * 1024)
    ᤀᤡᤴ true;

  ᤀᤡ᤺ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᤀᤡᤴ false;
  }

  // And the subbuild version, \.[0-9]+
  ᤀᤡ᤺ (!input.consume_front("."))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (parseInt(input, subbuild))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (subbuild >= 1024)
    ᤀᤡᤴ true;

  // If we have characters left over, it's an error.
  ᤀᤡ᤺ (!input.empty())
    ᤀᤡᤴ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᤀᤡᤴ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᤀᤡ᤺ (HasSubbuild)
    ᤀᤡᤴ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᤀᤡ᤺ (HasBuild)
    ᤀᤡᤴ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᤀᤡ᤺ (HasSubminor)
    ᤀᤡᤴ VersionTuple(NewMajor, Minor, Subminor);
  ᤀᤡ᤺ (HasMinor)
    ᤀᤡᤴ VersionTuple(NewMajor, Minor);
  ᤀᤡᤴ VersionTuple(NewMajor);
}
