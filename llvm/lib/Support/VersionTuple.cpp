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
  반환 Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  만약 (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  만약 (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  만약 (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  만약 (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  반환 Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  만약 (input.empty())
    반환 true;

  char next = input[0];
  input = input.substr(1);
  만약 (next < '0' || next > '9')
    반환 true;
  value = (unsigned)(next - '0');

  동안 (!input.empty()) {
    next = input[0];
    만약 (next < '0' || next > '9')
      반환 false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  반환 false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  만약 (parseInt(input, major))
    반환 true;

  만약 (input.empty()) {
    *this = VersionTuple(major);
    반환 false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  만약 (input[0] != '.')
    반환 true;
  input = input.substr(1);
  만약 (parseInt(input, minor))
    반환 true;

  만약 (input.empty()) {
    *this = VersionTuple(major, minor);
    반환 false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  만약 (!input.consume_front("."))
    반환 true;
  만약 (parseInt(input, subminor))
    반환 true;

  만약 (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    반환 false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  만약 (!input.consume_front("."))
    반환 true;
  만약 (parseInt(input, build))
    반환 true;
  만약 (build >= 1024 * 1024)
    반환 true;

  만약 (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    반환 false;
  }

  // And the subbuild version, \.[0-9]+
  만약 (!input.consume_front("."))
    반환 true;
  만약 (parseInt(input, subbuild))
    반환 true;
  만약 (subbuild >= 1024)
    반환 true;

  // If we have characters left over, it's an error.
  만약 (!input.empty())
    반환 true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  반환 false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  만약 (HasSubbuild)
    반환 VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  만약 (HasBuild)
    반환 VersionTuple(NewMajor, Minor, Subminor, Build);
  만약 (HasSubminor)
    반환 VersionTuple(NewMajor, Minor, Subminor);
  만약 (HasMinor)
    반환 VersionTuple(NewMajor, Minor);
  반환 VersionTuple(NewMajor);
}
