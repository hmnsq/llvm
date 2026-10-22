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
  ပြန်ပေး Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  အကယ်၍ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  အကယ်၍ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  အကယ်၍ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  အကယ်၍ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ပြန်ပေး Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  အကယ်၍ (input.empty())
    ပြန်ပေး true;

  var_na next = input[0];
  input = input.substr(1);
  အကယ်၍ (next < '0' || next > '9')
    ပြန်ပေး true;
  value = (unsigned)(next - '0');

  နေစဉ် (!input.empty()) {
    next = input[0];
    အကယ်၍ (next < '0' || next > '9')
      ပြန်ပေး false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ပြန်ပေး false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  အကယ်၍ (parseInt(input, major))
    ပြန်ပေး true;

  အကယ်၍ (input.empty()) {
    *this = VersionTuple(major);
    ပြန်ပေး false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  အကယ်၍ (input[0] != '.')
    ပြန်ပေး true;
  input = input.substr(1);
  အကယ်၍ (parseInt(input, minor))
    ပြန်ပေး true;

  အကယ်၍ (input.empty()) {
    *this = VersionTuple(major, minor);
    ပြန်ပေး false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  အကယ်၍ (!input.consume_front("."))
    ပြန်ပေး true;
  အကယ်၍ (parseInt(input, subminor))
    ပြန်ပေး true;

  အကယ်၍ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ပြန်ပေး false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  အကယ်၍ (!input.consume_front("."))
    ပြန်ပေး true;
  အကယ်၍ (parseInt(input, build))
    ပြန်ပေး true;
  အကယ်၍ (build >= 1024 * 1024)
    ပြန်ပေး true;

  အကယ်၍ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ပြန်ပေး false;
  }

  // And the subbuild version, \.[0-9]+
  အကယ်၍ (!input.consume_front("."))
    ပြန်ပေး true;
  အကယ်၍ (parseInt(input, subbuild))
    ပြန်ပေး true;
  အကယ်၍ (subbuild >= 1024)
    ပြန်ပေး true;

  // If we have characters left over, it's an error.
  အကယ်၍ (!input.empty())
    ပြန်ပေး true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ပြန်ပေး false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  အကယ်၍ (HasSubbuild)
    ပြန်ပေး VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  အကယ်၍ (HasBuild)
    ပြန်ပေး VersionTuple(NewMajor, Minor, Subminor, Build);
  အကယ်၍ (HasSubminor)
    ပြန်ပေး VersionTuple(NewMajor, Minor, Subminor);
  အကယ်၍ (HasMinor)
    ပြန်ပေး VersionTuple(NewMajor, Minor);
  ပြန်ပေး VersionTuple(NewMajor);
}
