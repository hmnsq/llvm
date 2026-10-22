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
  إذا (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  إذا (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  إذا (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  إذا (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  أرجع Out;
}

ساكن منطقي parseInt(StringRef &ماسوقن_2, unsigned &نيلاي_2) {
  أكد(نيلاي_2 == 0);
  إذا (ماسوقن_2.empty())
    أرجع true;

  حرف next = ماسوقن_2[0];
  ماسوقن_2 = ماسوقن_2.substr(1);
  إذا (next < '0' || next > '9')
    أرجع true;
  نيلاي_2 = (unsigned)(next - '0');

  طالما (!ماسوقن_2.empty()) {
    next = ماسوقن_2[0];
    إذا (next < '0' || next > '9')
      أرجع خطأ;
    ماسوقن_2 = ماسوقن_2.substr(1);
    نيلاي_2 = نيلاي_2 * 10 + (unsigned)(next - '0');
  }

  أرجع خطأ;
}

منطقي VersionTuple::tryParse(StringRef ماسوقن_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(ماسوقن_2, major))
    أرجع true;

  إذا (ماسوقن_2.empty()) {
    *هذا = VersionTuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (ماسوقن_2[0] != '.')
    أرجع true;
  ماسوقن_2 = ماسوقن_2.substr(1);
  إذا (parseInt(ماسوقن_2, minor))
    أرجع true;

  إذا (ماسوقن_2.empty()) {
    *هذا = VersionTuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!ماسوقن_2.consume_front("."))
    أرجع true;
  إذا (parseInt(ماسوقن_2, subminor))
    أرجع true;

  إذا (ماسوقن_2.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!ماسوقن_2.consume_front("."))
    أرجع true;
  إذا (parseInt(ماسوقن_2, build))
    أرجع true;
  إذا (build >= 1024 * 1024)
    أرجع true;

  إذا (ماسوقن_2.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!ماسوقن_2.consume_front("."))
    أرجع true;
  إذا (parseInt(ماسوقن_2, subbuild))
    أرجع true;
  إذا (subbuild >= 1024)
    أرجع true;

  // If we have characters left over, it's an error.
  إذا (!ماسوقن_2.empty())
    أرجع true;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  إذا (HasSubbuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  إذا (HasBuild)
    أرجع VersionTuple(NewMajor, Minor, Subminor, Build);
  إذا (HasSubminor)
    أرجع VersionTuple(NewMajor, Minor, Subminor);
  إذا (HasMinor)
    أرجع VersionTuple(NewMajor, Minor);
  أرجع VersionTuple(NewMajor);
}
