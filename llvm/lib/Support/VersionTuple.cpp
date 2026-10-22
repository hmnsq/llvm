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
  return Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  if (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  if (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  if (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  if (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  return Out;
}

static bool parseInt(StringRef &पासोक_2, unsigned &हालागा_2) {
  assert(हालागा_2 == 0);
  if (पासोक_2.empty())
    return true;

  char next = पासोक_2[0];
  पासोक_2 = पासोक_2.substr(1);
  if (next < '0' || next > '9')
    return true;
  हालागा_2 = (unsigned)(next - '0');

  while (!पासोक_2.empty()) {
    next = पासोक_2[0];
    if (next < '0' || next > '9')
      return false;
    पासोक_2 = पासोक_2.substr(1);
    हालागा_2 = हालागा_2 * 10 + (unsigned)(next - '0');
  }

  return false;
}

bool VersionTuple::tryParse(StringRef पासोक_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  if (parseInt(पासोक_2, major))
    return true;

  if (पासोक_2.empty()) {
    *this = VersionTuple(major);
    return false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  if (पासोक_2[0] != '.')
    return true;
  पासोक_2 = पासोक_2.substr(1);
  if (parseInt(पासोक_2, minor))
    return true;

  if (पासोक_2.empty()) {
    *this = VersionTuple(major, minor);
    return false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  if (!पासोक_2.consume_front("."))
    return true;
  if (parseInt(पासोक_2, subminor))
    return true;

  if (पासोक_2.empty()) {
    *this = VersionTuple(major, minor, subminor);
    return false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  if (!पासोक_2.consume_front("."))
    return true;
  if (parseInt(पासोक_2, build))
    return true;
  if (build >= 1024 * 1024)
    return true;

  if (पासोक_2.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    return false;
  }

  // And the subbuild version, \.[0-9]+
  if (!पासोक_2.consume_front("."))
    return true;
  if (parseInt(पासोक_2, subbuild))
    return true;
  if (subbuild >= 1024)
    return true;

  // If we have characters left over, it's an error.
  if (!पासोक_2.empty())
    return true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  return false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  if (HasSubbuild)
    return VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  if (HasBuild)
    return VersionTuple(NewMajor, Minor, Subminor, Build);
  if (HasSubminor)
    return VersionTuple(NewMajor, Minor, Subminor);
  if (HasMinor)
    return VersionTuple(NewMajor, Minor);
  return VersionTuple(NewMajor);
}
