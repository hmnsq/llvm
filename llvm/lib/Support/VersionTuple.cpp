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
  ᬯᬮᬶ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᬬᬾᬦ᭄ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᬬᬾᬦ᭄ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᬬᬾᬦ᭄ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᬬᬾᬦ᭄ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᬯᬮᬶ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᬬᬾᬦ᭄ (input.empty())
    ᬯᬮᬶ true;

  var_na next = input[0];
  input = input.substr(1);
  ᬬᬾᬦ᭄ (next < '0' || next > '9')
    ᬯᬮᬶ true;
  value = (unsigned)(next - '0');

  ᬲᭂᬤᭂᬮ (!input.empty()) {
    next = input[0];
    ᬬᬾᬦ᭄ (next < '0' || next > '9')
      ᬯᬮᬶ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᬯᬮᬶ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᬬᬾᬦ᭄ (parseInt(input, major))
    ᬯᬮᬶ true;

  ᬬᬾᬦ᭄ (input.empty()) {
    *this = VersionTuple(major);
    ᬯᬮᬶ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᬬᬾᬦ᭄ (input[0] != '.')
    ᬯᬮᬶ true;
  input = input.substr(1);
  ᬬᬾᬦ᭄ (parseInt(input, minor))
    ᬯᬮᬶ true;

  ᬬᬾᬦ᭄ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᬯᬮᬶ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᬬᬾᬦ᭄ (!input.consume_front("."))
    ᬯᬮᬶ true;
  ᬬᬾᬦ᭄ (parseInt(input, subminor))
    ᬯᬮᬶ true;

  ᬬᬾᬦ᭄ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᬯᬮᬶ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᬬᬾᬦ᭄ (!input.consume_front("."))
    ᬯᬮᬶ true;
  ᬬᬾᬦ᭄ (parseInt(input, build))
    ᬯᬮᬶ true;
  ᬬᬾᬦ᭄ (build >= 1024 * 1024)
    ᬯᬮᬶ true;

  ᬬᬾᬦ᭄ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᬯᬮᬶ false;
  }

  // And the subbuild version, \.[0-9]+
  ᬬᬾᬦ᭄ (!input.consume_front("."))
    ᬯᬮᬶ true;
  ᬬᬾᬦ᭄ (parseInt(input, subbuild))
    ᬯᬮᬶ true;
  ᬬᬾᬦ᭄ (subbuild >= 1024)
    ᬯᬮᬶ true;

  // If we have characters left over, it's an error.
  ᬬᬾᬦ᭄ (!input.empty())
    ᬯᬮᬶ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᬯᬮᬶ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᬬᬾᬦ᭄ (HasSubbuild)
    ᬯᬮᬶ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᬬᬾᬦ᭄ (HasBuild)
    ᬯᬮᬶ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᬬᬾᬦ᭄ (HasSubminor)
    ᬯᬮᬶ VersionTuple(NewMajor, Minor, Subminor);
  ᬬᬾᬦ᭄ (HasMinor)
    ᬯᬮᬶ VersionTuple(NewMajor, Minor);
  ᬯᬮᬶ VersionTuple(NewMajor);
}
