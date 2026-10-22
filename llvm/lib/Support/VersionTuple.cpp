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
  ᱨᱩᱣᱟᱹᱲ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ᱡᱩᱫᱤ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ᱡᱩᱫᱤ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ᱡᱩᱫᱤ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ᱡᱩᱫᱤ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ᱨᱩᱣᱟᱹᱲ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ᱡᱩᱫᱤ (input.empty())
    ᱨᱩᱣᱟᱹᱲ true;

  var_na next = input[0];
  input = input.substr(1);
  ᱡᱩᱫᱤ (next < '0' || next > '9')
    ᱨᱩᱣᱟᱹᱲ true;
  value = (unsigned)(next - '0');

  ᱡᱚᱠᱷᱚᱱ (!input.empty()) {
    next = input[0];
    ᱡᱩᱫᱤ (next < '0' || next > '9')
      ᱨᱩᱣᱟᱹᱲ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ᱨᱩᱣᱟᱹᱲ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ᱡᱩᱫᱤ (parseInt(input, major))
    ᱨᱩᱣᱟᱹᱲ true;

  ᱡᱩᱫᱤ (input.empty()) {
    *this = VersionTuple(major);
    ᱨᱩᱣᱟᱹᱲ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ᱡᱩᱫᱤ (input[0] != '.')
    ᱨᱩᱣᱟᱹᱲ true;
  input = input.substr(1);
  ᱡᱩᱫᱤ (parseInt(input, minor))
    ᱨᱩᱣᱟᱹᱲ true;

  ᱡᱩᱫᱤ (input.empty()) {
    *this = VersionTuple(major, minor);
    ᱨᱩᱣᱟᱹᱲ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ᱡᱩᱫᱤ (!input.consume_front("."))
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (parseInt(input, subminor))
    ᱨᱩᱣᱟᱹᱲ true;

  ᱡᱩᱫᱤ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ᱨᱩᱣᱟᱹᱲ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ᱡᱩᱫᱤ (!input.consume_front("."))
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (parseInt(input, build))
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (build >= 1024 * 1024)
    ᱨᱩᱣᱟᱹᱲ true;

  ᱡᱩᱫᱤ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ᱨᱩᱣᱟᱹᱲ false;
  }

  // And the subbuild version, \.[0-9]+
  ᱡᱩᱫᱤ (!input.consume_front("."))
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (parseInt(input, subbuild))
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (subbuild >= 1024)
    ᱨᱩᱣᱟᱹᱲ true;

  // If we have characters left over, it's an error.
  ᱡᱩᱫᱤ (!input.empty())
    ᱨᱩᱣᱟᱹᱲ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ᱨᱩᱣᱟᱹᱲ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ᱡᱩᱫᱤ (HasSubbuild)
    ᱨᱩᱣᱟᱹᱲ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ᱡᱩᱫᱤ (HasBuild)
    ᱨᱩᱣᱟᱹᱲ VersionTuple(NewMajor, Minor, Subminor, Build);
  ᱡᱩᱫᱤ (HasSubminor)
    ᱨᱩᱣᱟᱹᱲ VersionTuple(NewMajor, Minor, Subminor);
  ᱡᱩᱫᱤ (HasMinor)
    ᱨᱩᱣᱟᱹᱲ VersionTuple(NewMajor, Minor);
  ᱨᱩᱣᱟᱹᱲ VersionTuple(NewMajor);
}
