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
  คืน Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ถ้า (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ถ้า (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ถ้า (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ถ้า (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  คืน Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ถ้า (input.empty())
    คืน true;

  var_na next = input[0];
  input = input.substr(1);
  ถ้า (next < '0' || next > '9')
    คืน true;
  value = (unsigned)(next - '0');

  ขณะ (!input.empty()) {
    next = input[0];
    ถ้า (next < '0' || next > '9')
      คืน false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  คืน false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ถ้า (parseInt(input, major))
    คืน true;

  ถ้า (input.empty()) {
    *this = VersionTuple(major);
    คืน false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ถ้า (input[0] != '.')
    คืน true;
  input = input.substr(1);
  ถ้า (parseInt(input, minor))
    คืน true;

  ถ้า (input.empty()) {
    *this = VersionTuple(major, minor);
    คืน false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ถ้า (!input.consume_front("."))
    คืน true;
  ถ้า (parseInt(input, subminor))
    คืน true;

  ถ้า (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    คืน false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ถ้า (!input.consume_front("."))
    คืน true;
  ถ้า (parseInt(input, build))
    คืน true;
  ถ้า (build >= 1024 * 1024)
    คืน true;

  ถ้า (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    คืน false;
  }

  // And the subbuild version, \.[0-9]+
  ถ้า (!input.consume_front("."))
    คืน true;
  ถ้า (parseInt(input, subbuild))
    คืน true;
  ถ้า (subbuild >= 1024)
    คืน true;

  // If we have characters left over, it's an error.
  ถ้า (!input.empty())
    คืน true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  คืน false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ถ้า (HasSubbuild)
    คืน VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ถ้า (HasBuild)
    คืน VersionTuple(NewMajor, Minor, Subminor, Build);
  ถ้า (HasSubminor)
    คืน VersionTuple(NewMajor, Minor, Subminor);
  ถ้า (HasMinor)
    คืน VersionTuple(NewMajor, Minor);
  คืน VersionTuple(NewMajor);
}
