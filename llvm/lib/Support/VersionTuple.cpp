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
  ᰀᰧᰌ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᰀᰧ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᰀᰧ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᰀᰧ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᰀᰧ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᰀᰧᰌ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᰀᰧ (input.empty())
    ᰀᰧᰌ true;

  var_na next = input[0];
  input = input.substr(1);
  ᰀᰧ (next < '0' || next > '9')
    ᰀᰧᰌ true;
  value = (unsigned)(next - '0');

  ᰀᰬ (!input.empty()) {
    next = input[0];
    ᰀᰧ (next < '0' || next > '9')
      ᰀᰧᰌ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᰀᰧᰌ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᰀᰧ (parseInt(input, major))
    ᰀᰧᰌ true;

  ᰀᰧ (input.empty()) {
    *this = VersionTuple(major);
    ᰀᰧᰌ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᰀᰧ (input[0] != '.')
    ᰀᰧᰌ true;
  input = input.substr(1);
  ᰀᰧ (parseInt(input, minor))
    ᰀᰧᰌ true;

  ᰀᰧ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᰀᰧᰌ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᰀᰧ (!input.consume_front("."))
    ᰀᰧᰌ true;
  ᰀᰧ (parseInt(input, subminor))
    ᰀᰧᰌ true;

  ᰀᰧ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᰀᰧᰌ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᰀᰧ (!input.consume_front("."))
    ᰀᰧᰌ true;
  ᰀᰧ (parseInt(input, build))
    ᰀᰧᰌ true;
  ᰀᰧ (build >= 1024 * 1024)
    ᰀᰧᰌ true;

  ᰀᰧ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᰀᰧᰌ false;
  }

  // And the subbuild version, \.[0-9]+
  ᰀᰧ (!input.consume_front("."))
    ᰀᰧᰌ true;
  ᰀᰧ (parseInt(input, subbuild))
    ᰀᰧᰌ true;
  ᰀᰧ (subbuild >= 1024)
    ᰀᰧᰌ true;

  // If we have characters left over, it's an error.
  ᰀᰧ (!input.empty())
    ᰀᰧᰌ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᰀᰧᰌ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᰀᰧ (HasSubbuild)
    ᰀᰧᰌ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᰀᰧ (HasBuild)
    ᰀᰧᰌ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᰀᰧ (HasSubminor)
    ᰀᰧᰌ VersionTuple(NewMajor, Minor, Subminor);
  ᰀᰧ (HasMinor)
    ᰀᰧᰌ VersionTuple(NewMajor, Minor);
  ᰀᰧᰌ VersionTuple(NewMajor);
}
