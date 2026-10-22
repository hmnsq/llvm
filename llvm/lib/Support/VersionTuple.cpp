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
  ပွၵ်ႈ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ပေႃး (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ပေႃး (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ပေႃး (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ပေႃး (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ပွၵ်ႈ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ပေႃး (input.empty())
    ပွၵ်ႈ true;

  var_na next = input[0];
  input = input.substr(1);
  ပေႃး (next < '0' || next > '9')
    ပွၵ်ႈ true;
  value = (unsigned)(next - '0');

  ၶၢဝ်း (!input.empty()) {
    next = input[0];
    ပေႃး (next < '0' || next > '9')
      ပွၵ်ႈ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ပွၵ်ႈ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ပေႃး (parseInt(input, major))
    ပွၵ်ႈ true;

  ပေႃး (input.empty()) {
    *this = VersionTuple(major);
    ပွၵ်ႈ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ပေႃး (input[0] != '.')
    ပွၵ်ႈ true;
  input = input.substr(1);
  ပေႃး (parseInt(input, minor))
    ပွၵ်ႈ true;

  ပေႃး (input.empty()) {
    *this = VersionTuple(major, minor);
    ပွၵ်ႈ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ပေႃး (!input.consume_front("."))
    ပွၵ်ႈ true;
  ပေႃး (parseInt(input, subminor))
    ပွၵ်ႈ true;

  ပေႃး (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ပွၵ်ႈ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ပေႃး (!input.consume_front("."))
    ပွၵ်ႈ true;
  ပေႃး (parseInt(input, build))
    ပွၵ်ႈ true;
  ပေႃး (build >= 1024 * 1024)
    ပွၵ်ႈ true;

  ပေႃး (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ပွၵ်ႈ false;
  }

  // And the subbuild version, \.[0-9]+
  ပေႃး (!input.consume_front("."))
    ပွၵ်ႈ true;
  ပေႃး (parseInt(input, subbuild))
    ပွၵ်ႈ true;
  ပေႃး (subbuild >= 1024)
    ပွၵ်ႈ true;

  // If we have characters left over, it's an error.
  ပေႃး (!input.empty())
    ပွၵ်ႈ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ပွၵ်ႈ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ပေႃး (HasSubbuild)
    ပွၵ်ႈ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ပေႃး (HasBuild)
    ပွၵ်ႈ VersionTuple(NewMajor, Minor, Subminor, Build);
  ပေႃး (HasSubminor)
    ပွၵ်ႈ VersionTuple(NewMajor, Minor, Subminor);
  ပေႃး (HasMinor)
    ပွၵ်ႈ VersionTuple(NewMajor, Minor);
  ပွၵ်ႈ VersionTuple(NewMajor);
}
