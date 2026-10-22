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
  ꦮꦭꦶ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ꦪꦼꦤ꧀ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ꦪꦼꦤ꧀ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ꦪꦼꦤ꧀ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ꦪꦼꦤ꧀ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ꦮꦭꦶ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ꦪꦼꦤ꧀ (input.empty())
    ꦮꦭꦶ true;

  var_na next = input[0];
  input = input.substr(1);
  ꦪꦼꦤ꧀ (next < '0' || next > '9')
    ꦮꦭꦶ true;
  value = (unsigned)(next - '0');

  ꦱꦼꦢꦼꦭ (!input.empty()) {
    next = input[0];
    ꦪꦼꦤ꧀ (next < '0' || next > '9')
      ꦮꦭꦶ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ꦮꦭꦶ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ꦪꦼꦤ꧀ (parseInt(input, major))
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (input.empty()) {
    *this = VersionTuple(major);
    ꦮꦭꦶ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ꦪꦼꦤ꧀ (input[0] != '.')
    ꦮꦭꦶ true;
  input = input.substr(1);
  ꦪꦼꦤ꧀ (parseInt(input, minor))
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (input.empty()) {
    *this = VersionTuple(major, minor);
    ꦮꦭꦶ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ꦪꦼꦤ꧀ (!input.consume_front("."))
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (parseInt(input, subminor))
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ꦮꦭꦶ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ꦪꦼꦤ꧀ (!input.consume_front("."))
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (parseInt(input, build))
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (build >= 1024 * 1024)
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ꦮꦭꦶ false;
  }

  // And the subbuild version, \.[0-9]+
  ꦪꦼꦤ꧀ (!input.consume_front("."))
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (parseInt(input, subbuild))
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (subbuild >= 1024)
    ꦮꦭꦶ true;

  // If we have characters left over, it's an error.
  ꦪꦼꦤ꧀ (!input.empty())
    ꦮꦭꦶ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ꦮꦭꦶ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ꦪꦼꦤ꧀ (HasSubbuild)
    ꦮꦭꦶ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ꦪꦼꦤ꧀ (HasBuild)
    ꦮꦭꦶ VersionTuple(NewMajor, Minor, Subminor, Build);
  ꦪꦼꦤ꧀ (HasSubminor)
    ꦮꦭꦶ VersionTuple(NewMajor, Minor, Subminor);
  ꦪꦼꦤ꧀ (HasMinor)
    ꦮꦭꦶ VersionTuple(NewMajor, Minor);
  ꦮꦭꦶ VersionTuple(NewMajor);
}
