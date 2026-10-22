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
  ཕྱིར Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  གལ་ཏེ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  གལ་ཏེ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  གལ་ཏེ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  གལ་ཏེ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ཕྱིར Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  གལ་ཏེ (input.empty())
    ཕྱིར true;

  var_na next = input[0];
  input = input.substr(1);
  གལ་ཏེ (next < '0' || next > '9')
    ཕྱིར true;
  value = (unsigned)(next - '0');

  སྐབས (!input.empty()) {
    next = input[0];
    གལ་ཏེ (next < '0' || next > '9')
      ཕྱིར false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ཕྱིར false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  གལ་ཏེ (parseInt(input, major))
    ཕྱིར true;

  གལ་ཏེ (input.empty()) {
    *this = VersionTuple(major);
    ཕྱིར false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  གལ་ཏེ (input[0] != '.')
    ཕྱིར true;
  input = input.substr(1);
  གལ་ཏེ (parseInt(input, minor))
    ཕྱིར true;

  གལ་ཏེ (input.empty()) {
    *this = VersionTuple(major, minor);
    ཕྱིར false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  གལ་ཏེ (!input.consume_front("."))
    ཕྱིར true;
  གལ་ཏེ (parseInt(input, subminor))
    ཕྱིར true;

  གལ་ཏེ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ཕྱིར false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  གལ་ཏེ (!input.consume_front("."))
    ཕྱིར true;
  གལ་ཏེ (parseInt(input, build))
    ཕྱིར true;
  གལ་ཏེ (build >= 1024 * 1024)
    ཕྱིར true;

  གལ་ཏེ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ཕྱིར false;
  }

  // And the subbuild version, \.[0-9]+
  གལ་ཏེ (!input.consume_front("."))
    ཕྱིར true;
  གལ་ཏེ (parseInt(input, subbuild))
    ཕྱིར true;
  གལ་ཏེ (subbuild >= 1024)
    ཕྱིར true;

  // If we have characters left over, it's an error.
  གལ་ཏེ (!input.empty())
    ཕྱིར true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ཕྱིར false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  གལ་ཏེ (HasSubbuild)
    ཕྱིར VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  གལ་ཏེ (HasBuild)
    ཕྱིར VersionTuple(NewMajor, Minor, Subminor, Build);
  གལ་ཏེ (HasSubminor)
    ཕྱིར VersionTuple(NewMajor, Minor, Subminor);
  གལ་ཏེ (HasMinor)
    ཕྱིར VersionTuple(NewMajor, Minor);
  ཕྱིར VersionTuple(NewMajor);
}
