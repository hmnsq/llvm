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
  ត្រឡប់ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ប្រសិនបើ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ប្រសិនបើ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ប្រសិនបើ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ប្រសិនបើ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ត្រឡប់ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ប្រសិនបើ (input.empty())
    ត្រឡប់ true;

  var_na next = input[0];
  input = input.substr(1);
  ប្រសិនបើ (next < '0' || next > '9')
    ត្រឡប់ true;
  value = (unsigned)(next - '0');

  ខណៈពេល (!input.empty()) {
    next = input[0];
    ប្រសិនបើ (next < '0' || next > '9')
      ត្រឡប់ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ត្រឡប់ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ប្រសិនបើ (parseInt(input, major))
    ត្រឡប់ true;

  ប្រសិនបើ (input.empty()) {
    *this = VersionTuple(major);
    ត្រឡប់ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ប្រសិនបើ (input[0] != '.')
    ត្រឡប់ true;
  input = input.substr(1);
  ប្រសិនបើ (parseInt(input, minor))
    ត្រឡប់ true;

  ប្រសិនបើ (input.empty()) {
    *this = VersionTuple(major, minor);
    ត្រឡប់ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ប្រសិនបើ (!input.consume_front("."))
    ត្រឡប់ true;
  ប្រសិនបើ (parseInt(input, subminor))
    ត្រឡប់ true;

  ប្រសិនបើ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ត្រឡប់ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ប្រសិនបើ (!input.consume_front("."))
    ត្រឡប់ true;
  ប្រសិនបើ (parseInt(input, build))
    ត្រឡប់ true;
  ប្រសិនបើ (build >= 1024 * 1024)
    ត្រឡប់ true;

  ប្រសិនបើ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ត្រឡប់ false;
  }

  // And the subbuild version, \.[0-9]+
  ប្រសិនបើ (!input.consume_front("."))
    ត្រឡប់ true;
  ប្រសិនបើ (parseInt(input, subbuild))
    ត្រឡប់ true;
  ប្រសិនបើ (subbuild >= 1024)
    ត្រឡប់ true;

  // If we have characters left over, it's an error.
  ប្រសិនបើ (!input.empty())
    ត្រឡប់ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ត្រឡប់ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ប្រសិនបើ (HasSubbuild)
    ត្រឡប់ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ប្រសិនបើ (HasBuild)
    ត្រឡប់ VersionTuple(NewMajor, Minor, Subminor, Build);
  ប្រសិនបើ (HasSubminor)
    ត្រឡប់ VersionTuple(NewMajor, Minor, Subminor);
  ប្រសិនបើ (HasMinor)
    ត្រឡប់ VersionTuple(NewMajor, Minor);
  ត្រឡប់ VersionTuple(NewMajor);
}
