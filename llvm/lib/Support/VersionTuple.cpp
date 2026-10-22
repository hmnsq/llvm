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
  ܦܢܐ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ثابت VersionTuple &V) {
  Out << V.getMajor();
  ܐܢ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  ܐܢ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  ܐܢ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  ܐܢ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ܦܢܐ Out;
}

ساكن منطقي parseInt(StringRef &input, unsigned &value) {
  أكد(value == 0);
  ܐܢ (input.empty())
    ܦܢܐ ܫܪܪܐ;

  حرف next = input[0];
  input = input.substr(1);
  ܐܢ (next < '0' || next > '9')
    ܦܢܐ ܫܪܪܐ;
  value = (unsigned)(next - '0');

  ܟܡܐ (!input.empty()) {
    next = input[0];
    ܐܢ (next < '0' || next > '9')
      ܦܢܐ ܕܓܠܐ;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  ܦܢܐ ܕܓܠܐ;
}

منطقي VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ܐܢ (parseInt(input, major))
    ܦܢܐ ܫܪܪܐ;

  ܐܢ (input.empty()) {
    *هذا = VersionTuple(major);
    ܦܢܐ ܕܓܠܐ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ܐܢ (input[0] != '.')
    ܦܢܐ ܫܪܪܐ;
  input = input.substr(1);
  ܐܢ (parseInt(input, minor))
    ܦܢܐ ܫܪܪܐ;

  ܐܢ (input.empty()) {
    *هذا = VersionTuple(major, minor);
    ܦܢܐ ܕܓܠܐ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ܐܢ (!input.consume_front("."))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (parseInt(input, subminor))
    ܦܢܐ ܫܪܪܐ;

  ܐܢ (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    ܦܢܐ ܕܓܠܐ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ܐܢ (!input.consume_front("."))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (parseInt(input, build))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (build >= 1024 * 1024)
    ܦܢܐ ܫܪܪܐ;

  ܐܢ (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    ܦܢܐ ܕܓܠܐ;
  }

  // And the subbuild version, \.[0-9]+
  ܐܢ (!input.consume_front("."))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (parseInt(input, subbuild))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (subbuild >= 1024)
    ܦܢܐ ܫܪܪܐ;

  // If we have characters left over, it's an error.
  ܐܢ (!input.empty())
    ܦܢܐ ܫܪܪܐ;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  ܦܢܐ ܕܓܠܐ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  ܐܢ (HasSubbuild)
    ܦܢܐ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ܐܢ (HasBuild)
    ܦܢܐ VersionTuple(NewMajor, Minor, Subminor, Build);
  ܐܢ (HasSubminor)
    ܦܢܐ VersionTuple(NewMajor, Minor, Subminor);
  ܐܢ (HasMinor)
    ܦܢܐ VersionTuple(NewMajor, Minor);
  ܦܢܐ VersionTuple(NewMajor);
}
