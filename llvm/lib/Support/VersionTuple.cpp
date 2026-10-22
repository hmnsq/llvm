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
  trả_về Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  nếu (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  nếu (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  nếu (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  nếu (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  trả_về Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  nếu (input.empty())
    trả_về true;

  char next = input[0];
  input = input.substr(1);
  nếu (next < '0' || next > '9')
    trả_về true;
  value = (unsigned)(next - '0');

  trong_khi (!input.empty()) {
    next = input[0];
    nếu (next < '0' || next > '9')
      trả_về false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  trả_về false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  nếu (parseInt(input, major))
    trả_về true;

  nếu (input.empty()) {
    *this = VersionTuple(major);
    trả_về false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  nếu (input[0] != '.')
    trả_về true;
  input = input.substr(1);
  nếu (parseInt(input, minor))
    trả_về true;

  nếu (input.empty()) {
    *this = VersionTuple(major, minor);
    trả_về false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  nếu (!input.consume_front("."))
    trả_về true;
  nếu (parseInt(input, subminor))
    trả_về true;

  nếu (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    trả_về false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  nếu (!input.consume_front("."))
    trả_về true;
  nếu (parseInt(input, build))
    trả_về true;
  nếu (build >= 1024 * 1024)
    trả_về true;

  nếu (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    trả_về false;
  }

  // And the subbuild version, \.[0-9]+
  nếu (!input.consume_front("."))
    trả_về true;
  nếu (parseInt(input, subbuild))
    trả_về true;
  nếu (subbuild >= 1024)
    trả_về true;

  // If we have characters left over, it's an error.
  nếu (!input.empty())
    trả_về true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  trả_về false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  nếu (HasSubbuild)
    trả_về VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  nếu (HasBuild)
    trả_về VersionTuple(NewMajor, Minor, Subminor, Build);
  nếu (HasSubminor)
    trả_về VersionTuple(NewMajor, Minor, Subminor);
  nếu (HasMinor)
    trả_về VersionTuple(NewMajor, Minor);
  trả_về VersionTuple(NewMajor);
}
