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
  ལོག Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  གལ་སྲིད (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  གལ་སྲིད (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  གལ་སྲིད (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  གལ་སྲིད (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ལོག Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  གལ་སྲིད (input.empty())
    ལོག true;

  var_na next = input[0];
  input = input.substr(1);
  གལ་སྲིད (next < '0' || next > '9')
    ལོག true;
  value = (unsigned)(next - '0');

  སྐབས (!input.empty()) {
    next = input[0];
    གལ་སྲིད (next < '0' || next > '9')
      ལོག false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ལོག false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  གལ་སྲིད (parseInt(input, major))
    ལོག true;

  གལ་སྲིད (input.empty()) {
    *this = VersionTuple(major);
    ལོག false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  གལ་སྲིད (input[0] != '.')
    ལོག true;
  input = input.substr(1);
  གལ་སྲིད (parseInt(input, minor))
    ལོག true;

  གལ་སྲིད (input.empty()) {
    *this = VersionTuple(major, minor);
    ལོག false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  གལ་སྲིད (!input.consume_front("."))
    ལོག true;
  གལ་སྲིད (parseInt(input, subminor))
    ལོག true;

  གལ་སྲིད (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ལོག false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  གལ་སྲིད (!input.consume_front("."))
    ལོག true;
  གལ་སྲིད (parseInt(input, build))
    ལོག true;
  གལ་སྲིད (build >= 1024 * 1024)
    ལོག true;

  གལ་སྲིད (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ལོག false;
  }

  // And the subbuild version, \.[0-9]+
  གལ་སྲིད (!input.consume_front("."))
    ལོག true;
  གལ་སྲིད (parseInt(input, subbuild))
    ལོག true;
  གལ་སྲིད (subbuild >= 1024)
    ལོག true;

  // If we have characters left over, it's an error.
  གལ་སྲིད (!input.empty())
    ལོག true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ལོག false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  གལ་སྲིད (HasSubbuild)
    ལོག VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  གལ་སྲིད (HasBuild)
    ལོག VersionTuple(NewMajor, Minor, Subminor, Build);
  གལ་སྲིད (HasSubminor)
    ལོག VersionTuple(NewMajor, Minor, Subminor);
  གལ་སྲིད (HasMinor)
    ལོག VersionTuple(NewMajor, Minor);
  ལོག VersionTuple(NewMajor);
}
