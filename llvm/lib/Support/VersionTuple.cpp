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
  врати Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ако (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ако (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ако (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ако (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  врати Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ако (input.empty())
    врати true;

  char next = input[0];
  input = input.substr(1);
  ако (next < '0' || next > '9')
    врати true;
  value = (unsigned)(next - '0');

  док (!input.empty()) {
    next = input[0];
    ако (next < '0' || next > '9')
      врати false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  врати false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ако (parseInt(input, major))
    врати true;

  ако (input.empty()) {
    *this = VersionTuple(major);
    врати false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ако (input[0] != '.')
    врати true;
  input = input.substr(1);
  ако (parseInt(input, minor))
    врати true;

  ако (input.empty()) {
    *this = VersionTuple(major, minor);
    врати false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ако (!input.consume_front("."))
    врати true;
  ако (parseInt(input, subminor))
    врати true;

  ако (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    врати false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ако (!input.consume_front("."))
    врати true;
  ако (parseInt(input, build))
    врати true;
  ако (build >= 1024 * 1024)
    врати true;

  ако (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    врати false;
  }

  // And the subbuild version, \.[0-9]+
  ако (!input.consume_front("."))
    врати true;
  ако (parseInt(input, subbuild))
    врати true;
  ако (subbuild >= 1024)
    врати true;

  // If we have characters left over, it's an error.
  ако (!input.empty())
    врати true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  врати false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ако (HasSubbuild)
    врати VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ако (HasBuild)
    врати VersionTuple(NewMajor, Minor, Subminor, Build);
  ако (HasSubminor)
    врати VersionTuple(NewMajor, Minor, Subminor);
  ако (HasMinor)
    врати VersionTuple(NewMajor, Minor);
  врати VersionTuple(NewMajor);
}
