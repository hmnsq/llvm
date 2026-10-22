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
  ສົ່ງຄືນ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, const VersionTuple &V) {
  Out << V.getMajor();
  ຖ້າ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ຖ້າ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ຖ້າ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ຖ້າ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ສົ່ງຄືນ Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  ຖ້າ (input.empty())
    ສົ່ງຄືນ true;

  var_na next = input[0];
  input = input.substr(1);
  ຖ້າ (next < '0' || next > '9')
    ສົ່ງຄືນ true;
  value = (unsigned)(next - '0');

  ໃນຂະນະທີ່ (!input.empty()) {
    next = input[0];
    ຖ້າ (next < '0' || next > '9')
      ສົ່ງຄືນ false;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ສົ່ງຄືນ false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ຖ້າ (parseInt(input, major))
    ສົ່ງຄືນ true;

  ຖ້າ (input.empty()) {
    *this = VersionTuple(major);
    ສົ່ງຄືນ false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ຖ້າ (input[0] != '.')
    ສົ່ງຄືນ true;
  input = input.substr(1);
  ຖ້າ (parseInt(input, minor))
    ສົ່ງຄືນ true;

  ຖ້າ (input.empty()) {
    *this = VersionTuple(major, minor);
    ສົ່ງຄືນ false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ຖ້າ (!input.consume_front("."))
    ສົ່ງຄືນ true;
  ຖ້າ (parseInt(input, subminor))
    ສົ່ງຄືນ true;

  ຖ້າ (input.empty()) {
    *this = VersionTuple(major, minor, subminor);
    ສົ່ງຄືນ false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ຖ້າ (!input.consume_front("."))
    ສົ່ງຄືນ true;
  ຖ້າ (parseInt(input, build))
    ສົ່ງຄືນ true;
  ຖ້າ (build >= 1024 * 1024)
    ສົ່ງຄືນ true;

  ຖ້າ (input.empty()) {
    *this = VersionTuple(major, minor, subminor, build);
    ສົ່ງຄືນ false;
  }

  // And the subbuild version, \.[0-9]+
  ຖ້າ (!input.consume_front("."))
    ສົ່ງຄືນ true;
  ຖ້າ (parseInt(input, subbuild))
    ສົ່ງຄືນ true;
  ຖ້າ (subbuild >= 1024)
    ສົ່ງຄືນ true;

  // If we have characters left over, it's an error.
  ຖ້າ (!input.empty())
    ສົ່ງຄືນ true;

  *this = VersionTuple(major, minor, subminor, build, subbuild);
  ສົ່ງຄືນ false;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) const {
  ຖ້າ (HasSubbuild)
    ສົ່ງຄືນ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ຖ້າ (HasBuild)
    ສົ່ງຄືນ VersionTuple(NewMajor, Minor, Subminor, Build);
  ຖ້າ (HasSubminor)
    ສົ່ງຄືນ VersionTuple(NewMajor, Minor, Subminor);
  ຖ້າ (HasMinor)
    ສົ່ງຄືນ VersionTuple(NewMajor, Minor);
  ສົ່ງຄືນ VersionTuple(NewMajor);
}
