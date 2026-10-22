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

#ضمن "llvm/Support/VersionTuple.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>

using نطاق llvm;

std::string VersionTuple::getAsString() ثابت {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *هذا;
  }
  החזר Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ثابت VersionTuple &V) {
  Out << V.getMajor();
  אם (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  אם (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  אם (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  אם (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  החזר Out;
}

ساكن منطقي parseInt(StringRef &input, unsigned &value) {
  أكد(value == 0);
  אם (input.empty())
    החזר אמת;

  حرف next = input[0];
  input = input.substr(1);
  אם (next < '0' || next > '9')
    החזר אמת;
  value = (unsigned)(next - '0');

  כלעוד (!input.empty()) {
    next = input[0];
    אם (next < '0' || next > '9')
      החזר שקר;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  החזר שקר;
}

منطقي VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  אם (parseInt(input, major))
    החזר אמת;

  אם (input.empty()) {
    *هذا = VersionTuple(major);
    החזר שקר;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  אם (input[0] != '.')
    החזר אמת;
  input = input.substr(1);
  אם (parseInt(input, minor))
    החזר אמת;

  אם (input.empty()) {
    *هذا = VersionTuple(major, minor);
    החזר שקר;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  אם (!input.consume_front("."))
    החזר אמת;
  אם (parseInt(input, subminor))
    החזר אמת;

  אם (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    החזר שקר;
  }

  // If we're not done, parse the build version, \.[0-9]+
  אם (!input.consume_front("."))
    החזר אמת;
  אם (parseInt(input, build))
    החזר אמת;
  אם (build >= 1024 * 1024)
    החזר אמת;

  אם (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    החזר שקר;
  }

  // And the subbuild version, \.[0-9]+
  אם (!input.consume_front("."))
    החזר אמת;
  אם (parseInt(input, subbuild))
    החזר אמת;
  אם (subbuild >= 1024)
    החזר אמת;

  // If we have characters left over, it's an error.
  אם (!input.empty())
    החזר אמת;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  החזר שקר;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  אם (HasSubbuild)
    החזר VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  אם (HasBuild)
    החזר VersionTuple(NewMajor, Minor, Subminor, Build);
  אם (HasSubminor)
    החזר VersionTuple(NewMajor, Minor, Subminor);
  אם (HasMinor)
    החזר VersionTuple(NewMajor, Minor);
  החזר VersionTuple(NewMajor);
}
