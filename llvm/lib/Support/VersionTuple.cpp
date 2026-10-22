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
  ᯀᯮᯞ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᯀᯮᯝ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᯀᯮᯝ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᯀᯮᯝ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᯀᯮᯝ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᯀᯮᯞ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᯀᯮᯝ (input.empty())
    ᯀᯮᯞ true;

  var_na next = input[0];
  input = input.substr(1);
  ᯀᯮᯝ (next < '0' || next > '9')
    ᯀᯮᯞ true;
  value = (unsigned)(next - '0');

  ᯀᯬᯝ (!input.empty()) {
    next = input[0];
    ᯀᯮᯝ (next < '0' || next > '9')
      ᯀᯮᯞ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᯀᯮᯞ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᯀᯮᯝ (parseInt(input, major))
    ᯀᯮᯞ true;

  ᯀᯮᯝ (input.empty()) {
    *this = VersionTuple(major);
    ᯀᯮᯞ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᯀᯮᯝ (input[0] != '.')
    ᯀᯮᯞ true;
  input = input.substr(1);
  ᯀᯮᯝ (parseInt(input, minor))
    ᯀᯮᯞ true;

  ᯀᯮᯝ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᯀᯮᯞ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᯀᯮᯝ (!input.consume_front("."))
    ᯀᯮᯞ true;
  ᯀᯮᯝ (parseInt(input, subminor))
    ᯀᯮᯞ true;

  ᯀᯮᯝ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᯀᯮᯞ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᯀᯮᯝ (!input.consume_front("."))
    ᯀᯮᯞ true;
  ᯀᯮᯝ (parseInt(input, build))
    ᯀᯮᯞ true;
  ᯀᯮᯝ (build >= 1024 * 1024)
    ᯀᯮᯞ true;

  ᯀᯮᯝ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᯀᯮᯞ false;
  }

  // And the subbuild version, \.[0-9]+
  ᯀᯮᯝ (!input.consume_front("."))
    ᯀᯮᯞ true;
  ᯀᯮᯝ (parseInt(input, subbuild))
    ᯀᯮᯞ true;
  ᯀᯮᯝ (subbuild >= 1024)
    ᯀᯮᯞ true;

  // If we have characters left over, it's an error.
  ᯀᯮᯝ (!input.empty())
    ᯀᯮᯞ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᯀᯮᯞ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᯀᯮᯝ (HasSubbuild)
    ᯀᯮᯞ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᯀᯮᯝ (HasBuild)
    ᯀᯮᯞ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᯀᯮᯝ (HasSubminor)
    ᯀᯮᯞ VersionTuple(NewMajor, Minor, Subminor);
  ᯀᯮᯝ (HasMinor)
    ᯀᯮᯞ VersionTuple(NewMajor, Minor);
  ᯀᯮᯞ VersionTuple(NewMajor);
}
