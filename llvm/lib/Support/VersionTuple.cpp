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
  ආපසු Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  නම් (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  නම් (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  නම් (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  නම් (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ආපසු Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  නම් (input.empty())
    ආපසු true;

  var_na next = input[0];
  input = input.substr(1);
  නම් (next < '0' || next > '9')
    ආපසු true;
  value = (unsigned)(next - '0');

  අතරතුර (!input.empty()) {
    next = input[0];
    නම් (next < '0' || next > '9')
      ආපසු false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ආපසු false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  නම් (parseInt(input, major))
    ආපසු true;

  නම් (input.empty()) {
    *this = VersionTuple(major);
    ආපසු false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  නම් (input[0] != '.')
    ආපසු true;
  input = input.substr(1);
  නම් (parseInt(input, minor))
    ආපසු true;

  නම් (input.empty()) {
    *this = VersionTuple(major, minor);
    ආපසු false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  නම් (!input.consume_front("."))
    ආපසු true;
  නම් (parseInt(input, subminor))
    ආපසු true;

  නම් (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ආපසු false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  නම් (!input.consume_front("."))
    ආපසු true;
  නම් (parseInt(input, build))
    ආපසු true;
  නම් (build >= 1024 * 1024)
    ආපසු true;

  නම් (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ආපසු false;
  }

  // And the subbuild version, \.[0-9]+
  නම් (!input.consume_front("."))
    ආපසු true;
  නම් (parseInt(input, subbuild))
    ආපසු true;
  නම් (subbuild >= 1024)
    ආපසු true;

  // If we have characters left over, it's an error.
  නම් (!input.empty())
    ආපසු true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ආපසු false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  නම් (HasSubbuild)
    ආපසු VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  නම් (HasBuild)
    ආපසු VersionTuple(NewMajor, Minor, Subminor, Build);
  නම් (HasSubminor)
    ආපසු VersionTuple(NewMajor, Minor, Subminor);
  නම් (HasMinor)
    ආපසු VersionTuple(NewMajor, Minor);
  ආපසු VersionTuple(NewMajor);
}
