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
  повернути Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  якщо (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  якщо (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  якщо (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  якщо (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  повернути Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  якщо (input.empty())
    повернути true;

  char next = input[0];
  input = input.substr(1);
  якщо (next < '0' || next > '9')
    повернути true;
  value = (unsigned)(next - '0');

  поки (!input.empty()) {
    next = input[0];
    якщо (next < '0' || next > '9')
      повернути false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  повернути false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  якщо (parseInt(input, major))
    повернути true;

  якщо (input.empty()) {
    *this = VersionTuple(major);
    повернути false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  якщо (input[0] != '.')
    повернути true;
  input = input.substr(1);
  якщо (parseInt(input, minor))
    повернути true;

  якщо (input.empty()) {
    *this = VersionTuple(major, minor);
    повернути false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  якщо (!input.consume_front("."))
    повернути true;
  якщо (parseInt(input, subminor))
    повернути true;

  якщо (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    повернути false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  якщо (!input.consume_front("."))
    повернути true;
  якщо (parseInt(input, build))
    повернути true;
  якщо (build >= 1024 * 1024)
    повернути true;

  якщо (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    повернути false;
  }

  // And the subbuild version, \.[0-9]+
  якщо (!input.consume_front("."))
    повернути true;
  якщо (parseInt(input, subbuild))
    повернути true;
  якщо (subbuild >= 1024)
    повернути true;

  // If we have characters left over, it's an error.
  якщо (!input.empty())
    повернути true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  повернути false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  якщо (HasSubbuild)
    повернути VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  якщо (HasBuild)
    повернути VersionTuple(NewMajor, Minor, Subminor, Build);
  якщо (HasSubminor)
    повернути VersionTuple(NewMajor, Minor, Subminor);
  якщо (HasMinor)
    повернути VersionTuple(NewMajor, Minor);
  повернути VersionTuple(NewMajor);
}
