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
  ᮝᮜᮤ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᮚᮦᮔ᮪ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᮚᮦᮔ᮪ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᮚᮦᮔ᮪ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᮚᮦᮔ᮪ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᮝᮜᮤ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᮚᮦᮔ᮪ (input.empty())
    ᮝᮜᮤ true;

  var_na next = input[0];
  input = input.substr(1);
  ᮚᮦᮔ᮪ (next < '0' || next > '9')
    ᮝᮜᮤ true;
  value = (unsigned)(next - '0');

  ᮞᮨᮓᮨᮜ (!input.empty()) {
    next = input[0];
    ᮚᮦᮔ᮪ (next < '0' || next > '9')
      ᮝᮜᮤ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᮝᮜᮤ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᮚᮦᮔ᮪ (parseInt(input, major))
    ᮝᮜᮤ true;

  ᮚᮦᮔ᮪ (input.empty()) {
    *this = VersionTuple(major);
    ᮝᮜᮤ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᮚᮦᮔ᮪ (input[0] != '.')
    ᮝᮜᮤ true;
  input = input.substr(1);
  ᮚᮦᮔ᮪ (parseInt(input, minor))
    ᮝᮜᮤ true;

  ᮚᮦᮔ᮪ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᮝᮜᮤ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᮚᮦᮔ᮪ (!input.consume_front("."))
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (parseInt(input, subminor))
    ᮝᮜᮤ true;

  ᮚᮦᮔ᮪ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᮝᮜᮤ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᮚᮦᮔ᮪ (!input.consume_front("."))
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (parseInt(input, build))
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (build >= 1024 * 1024)
    ᮝᮜᮤ true;

  ᮚᮦᮔ᮪ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᮝᮜᮤ false;
  }

  // And the subbuild version, \.[0-9]+
  ᮚᮦᮔ᮪ (!input.consume_front("."))
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (parseInt(input, subbuild))
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (subbuild >= 1024)
    ᮝᮜᮤ true;

  // If we have characters left over, it's an error.
  ᮚᮦᮔ᮪ (!input.empty())
    ᮝᮜᮤ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᮝᮜᮤ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᮚᮦᮔ᮪ (HasSubbuild)
    ᮝᮜᮤ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᮚᮦᮔ᮪ (HasBuild)
    ᮝᮜᮤ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᮚᮦᮔ᮪ (HasSubminor)
    ᮝᮜᮤ VersionTuple(NewMajor, Minor, Subminor);
  ᮚᮦᮔ᮪ (HasMinor)
    ᮝᮜᮤ VersionTuple(NewMajor, Minor);
  ᮝᮜᮤ VersionTuple(NewMajor);
}
