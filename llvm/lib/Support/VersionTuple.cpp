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
  إرجاع Result;
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
  إرجاع Out;
}

ساكن منطقي parseInt(StringRef &إدخال_2, unsigned &قيمة_2) {
  أكد(قيمة_2 == 0);
  إذا (إدخال_2.empty())
    إرجاع true;

  حرف next = إدخال_2[0];
  إدخال_2 = إدخال_2.substr(1);
  إذا (next < '0' || next > '9')
    إرجاع true;
  قيمة_2 = (unsigned)(next - '0');

  بينما (!إدخال_2.empty()) {
    next = إدخال_2[0];
    إذا (next < '0' || next > '9')
      إرجاع خطأ;
    إدخال_2 = إدخال_2.substr(1);
    قيمة_2 = قيمة_2 * 10 + (unsigned)(next - '0');
  }

  إرجاع خطأ;
}

منطقي VersionTuple::tryParse(StringRef إدخال_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(إدخال_2, major))
    إرجاع true;

  إذا (إدخال_2.empty()) {
    *هذا = VersionTuple(major);
    إرجاع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (إدخال_2[0] != '.')
    إرجاع true;
  إدخال_2 = إدخال_2.substr(1);
  إذا (parseInt(إدخال_2, minor))
    إرجاع true;

  إذا (إدخال_2.empty()) {
    *هذا = VersionTuple(major, minor);
    إرجاع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!إدخال_2.consume_front("."))
    إرجاع true;
  إذا (parseInt(إدخال_2, subminor))
    إرجاع true;

  إذا (إدخال_2.empty()) {
    *هذا = VersionTuple(major, minor, subminor);
    إرجاع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!إدخال_2.consume_front("."))
    إرجاع true;
  إذا (parseInt(إدخال_2, build))
    إرجاع true;
  إذا (build >= 1024 * 1024)
    إرجاع true;

  إذا (إدخال_2.empty()) {
    *هذا = VersionTuple(major, minor, subminor, build);
    إرجاع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!إدخال_2.consume_front("."))
    إرجاع true;
  إذا (parseInt(إدخال_2, subbuild))
    إرجاع true;
  إذا (subbuild >= 1024)
    إرجاع true;

  // If we have characters left over, it's an error.
  إذا (!إدخال_2.empty())
    إرجاع true;

  *هذا = VersionTuple(major, minor, subminor, build, subbuild);
  إرجاع خطأ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ثابت {
  إذا (HasSubbuild)
    إرجاع VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  إذا (HasBuild)
    إرجاع VersionTuple(NewMajor, Minor, Subminor, Build);
  إذا (HasSubminor)
    إرجاع VersionTuple(NewMajor, Minor, Subminor);
  إذا (HasMinor)
    إرجاع VersionTuple(NewMajor, Minor);
  إرجاع VersionTuple(NewMajor);
}
