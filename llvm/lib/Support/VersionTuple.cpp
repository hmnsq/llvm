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
  𐤀𐤌 (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  𐤀𐤌 (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  𐤀𐤌 (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  𐤀𐤌 (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  أرجع Out;
}

ساكن منطقي parseInt(StringRef &input, unsigned &value) {
  أكد(value == 0);
  𐤀𐤌 (input.empty())
    أرجع true;

  حرف next = input[0];
  input = input.substr(1);
  𐤀𐤌 (next < '0' || next > '9')
    أرجع true;
  value = (unsigned)(next - '0');

  طالما (!input.empty()) {
    next = input[0];
    𐤀𐤌 (next < '0' || next > '9')
      أرجع خطأ;
    input = input.substr(1);
    value = value * 10 + (unsigned)(next - '0');
  }

  أرجع خطأ;
}

منطقي VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  𐤀𐤌 (parseInt(input, major))
    أرجع true;

  𐤀𐤌 (input.empty()) {
    *هذا = VersionTuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  𐤀𐤌 (input[0] != '.')
    أرجع true;
  input = input.substr(1);
  𐤀𐤌 (parseInt(input, minor))
    أرجع true;

  𐤀𐤌 (input.empty()) {
    *هذا = VersionTuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  𐤀𐤌 (!input.consume_front("."))
    أرجع true;
  𐤀𐤌 (parseInt(input, subminor))
    أرجع true;

  𐤀𐤌 (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  𐤀𐤌 (!input.consume_front("."))
    أرجع true;
  𐤀𐤌 (parseInt(input, build))
    أرجع true;
  𐤀𐤌 (build >= 1024 * 1024)
    أرجع true;

  𐤀𐤌 (input.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  𐤀𐤌 (!input.consume_front("."))
    أرجع true;
  𐤀𐤌 (parseInt(input, subbuild))
    أرجع true;
  𐤀𐤌 (subbuild >= 1024)
    أرجع true;

  // If we have characters left over, it's an error.
  𐤀𐤌 (!input.empty())
    أرجع true;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  𐤀𐤌 (HasSubbuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  𐤀𐤌 (HasBuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build);
  𐤀𐤌 (HasSubminor)
    أرجع VersionTuple(NewMajor, Minor, Subminor);
  𐤀𐤌 (HasMinor)
    أرجع VersionTuple(NewMajor, Minor);
  أرجع VersionTuple(NewMajor);
}
