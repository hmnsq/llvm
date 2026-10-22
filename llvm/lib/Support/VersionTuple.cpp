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
  返回 Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  如果 (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  如果 (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  如果 (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  如果 (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  返回 Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  如果 (input.empty())
    返回 true;

  char next = input[0];
  input = input.substr(1);
  如果 (next < '0' || next > '9')
    返回 true;
  value = (unsigned)(next - '0');

  当 (!input.empty()) {
    next = input[0];
    如果 (next < '0' || next > '9')
      返回 false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  返回 false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  如果 (parseInt(input, major))
    返回 true;

  如果 (input.empty()) {
    *this = VersionTuple(major);
    返回 false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  如果 (input[0] != '.')
    返回 true;
  input = input.substr(1);
  如果 (parseInt(input, minor))
    返回 true;

  如果 (input.empty()) {
    *this = VersionTuple(major, minor);
    返回 false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  如果 (!input.consume_front("."))
    返回 true;
  如果 (parseInt(input, subminor))
    返回 true;

  如果 (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    返回 false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  如果 (!input.consume_front("."))
    返回 true;
  如果 (parseInt(input, build))
    返回 true;
  如果 (build >= 1024 * 1024)
    返回 true;

  如果 (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    返回 false;
  }

  // And the subbuild version, \.[0-9]+
  如果 (!input.consume_front("."))
    返回 true;
  如果 (parseInt(input, subbuild))
    返回 true;
  如果 (subbuild >= 1024)
    返回 true;

  // If we have characters left over, it's an error.
  如果 (!input.empty())
    返回 true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  返回 false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  如果 (HasSubbuild)
    返回 VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  如果 (HasBuild)
    返回 VersionTuple(NewMajor, Minor, Subminor, Build);
  如果 (HasSubminor)
    返回 VersionTuple(NewMajor, Minor, Subminor);
  如果 (HasMinor)
    返回 VersionTuple(NewMajor, Minor);
  返回 VersionTuple(NewMajor);
}
