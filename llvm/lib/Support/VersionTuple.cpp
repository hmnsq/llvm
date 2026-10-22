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
  вернуть Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  если (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  если (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  если (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  если (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  вернуть Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  если (input.empty())
    вернуть true;

  char next = input[0];
  input = input.substr(1);
  если (next < '0' || next > '9')
    вернуть true;
  value = (unsigned)(next - '0');

  пока (!input.empty()) {
    next = input[0];
    если (next < '0' || next > '9')
      вернуть false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  вернуть false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  если (parseInt(input, major))
    вернуть true;

  если (input.empty()) {
    *this = VersionTuple(major);
    вернуть false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  если (input[0] != '.')
    вернуть true;
  input = input.substr(1);
  если (parseInt(input, minor))
    вернуть true;

  если (input.empty()) {
    *this = VersionTuple(major, minor);
    вернуть false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  если (!input.consume_front("."))
    вернуть true;
  если (parseInt(input, subminor))
    вернуть true;

  если (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    вернуть false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  если (!input.consume_front("."))
    вернуть true;
  если (parseInt(input, build))
    вернуть true;
  если (build >= 1024 * 1024)
    вернуть true;

  если (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    вернуть false;
  }

  // And the subbuild version, \.[0-9]+
  если (!input.consume_front("."))
    вернуть true;
  если (parseInt(input, subbuild))
    вернуть true;
  если (subbuild >= 1024)
    вернуть true;

  // If we have characters left over, it's an error.
  если (!input.empty())
    вернуть true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  вернуть false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  если (HasSubbuild)
    вернуть VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  если (HasBuild)
    вернуть VersionTuple(NewMajor, Minor, Subminor, Build);
  если (HasSubminor)
    вернуть VersionTuple(NewMajor, Minor, Subminor);
  если (HasMinor)
    вернуть VersionTuple(NewMajor, Minor);
  вернуть VersionTuple(NewMajor);
}
