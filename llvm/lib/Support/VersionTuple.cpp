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
  戻る Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  もし (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  もし (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  もし (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  もし (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  戻る Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  もし (input.empty())
    戻る true;

  char next = input[0];
  input = input.substr(1);
  もし (next < '0' || next > '9')
    戻る true;
  value = (unsigned)(next - '0');

  ながら (!input.empty()) {
    next = input[0];
    もし (next < '0' || next > '9')
      戻る false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  戻る false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  もし (parseInt(input, major))
    戻る true;

  もし (input.empty()) {
    *this = VersionTuple(major);
    戻る false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  もし (input[0] != '.')
    戻る true;
  input = input.substr(1);
  もし (parseInt(input, minor))
    戻る true;

  もし (input.empty()) {
    *this = VersionTuple(major, minor);
    戻る false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  もし (!input.consume_front("."))
    戻る true;
  もし (parseInt(input, subminor))
    戻る true;

  もし (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    戻る false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  もし (!input.consume_front("."))
    戻る true;
  もし (parseInt(input, build))
    戻る true;
  もし (build >= 1024 * 1024)
    戻る true;

  もし (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    戻る false;
  }

  // And the subbuild version, \.[0-9]+
  もし (!input.consume_front("."))
    戻る true;
  もし (parseInt(input, subbuild))
    戻る true;
  もし (subbuild >= 1024)
    戻る true;

  // If we have characters left over, it's an error.
  もし (!input.empty())
    戻る true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  戻る false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  もし (HasSubbuild)
    戻る VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  もし (HasBuild)
    戻る VersionTuple(NewMajor, Minor, Subminor, Build);
  もし (HasSubminor)
    戻る VersionTuple(NewMajor, Minor, Subminor);
  もし (HasMinor)
    戻る VersionTuple(NewMajor, Minor);
  戻る VersionTuple(NewMajor);
}
