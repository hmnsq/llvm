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
  أرجع Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ثابت VersionTuple &V) {
  Out << V.getMajor();
  𐡄𐡍 (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  𐡄𐡍 (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  𐡄𐡍 (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  𐡄𐡍 (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  أرجع Out;
}

ساكن منطقي parseInt(StringRef &input, unsigned &value) {
  أكد(value == 0);
  𐡄𐡍 (input.empty())
    أرجع 𐡒𐡔𐡈;

  حرف next = input[0];
  input = input.substr(1);
  𐡄𐡍 (next < '0' || next > '9')
    أرجع 𐡒𐡔𐡈;
  value = (unsigned)(next - '0');

  طالما (!input.empty()) {
    next = input[0];
    𐡄𐡍 (next < '0' || next > '9')
      أرجع خطأ;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  أرجع خطأ;
}

منطقي VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  𐡄𐡍 (parseInt(input, major))
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (input.empty()) {
    *هذا = VersionTuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  𐡄𐡍 (input[0] != '.')
    أرجع 𐡒𐡔𐡈;
  input = input.substr(1);
  𐡄𐡍 (parseInt(input, minor))
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (input.empty()) {
    *هذا = VersionTuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  𐡄𐡍 (!input.consume_front("."))
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (parseInt(input, subminor))
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  𐡄𐡍 (!input.consume_front("."))
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (parseInt(input, build))
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (build >= 1024 * 1024)
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  𐡄𐡍 (!input.consume_front("."))
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (parseInt(input, subbuild))
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (subbuild >= 1024)
    أرجع 𐡒𐡔𐡈;

  // If we have characters left over, it's an error.
  𐡄𐡍 (!input.empty())
    أرجع 𐡒𐡔𐡈;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  𐡄𐡍 (HasSubbuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  𐡄𐡍 (HasBuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build);
  𐡄𐡍 (HasSubminor)
    أرجع VersionTuple(NewMajor, Minor, Subminor);
  𐡄𐡍 (HasMinor)
    أرجع VersionTuple(NewMajor, Minor);
  أرجع VersionTuple(NewMajor);
}
