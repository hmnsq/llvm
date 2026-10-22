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

استفاده نطاق llvm;

std::string نسخه_Tuple::getAsString() ثابت {
  std::string نتیجه;
  {
    llvm::raw_string_ostream بیرون(نتیجه);
    بیرون << *هذا;
  }
  أرجع نتیجه;
}

raw_ostream &llvm::عملگر<<(raw_ostream &بیرون, ثابت نسخه_Tuple &V) {
  بیرون << V.getMajor();
  إذا (std::optional<بیعلامت> Minor = V.getMinor())
    بیرون << '.' << *Minor;
  إذا (std::optional<بیعلامت> Subminor = V.getSubminor())
    بیرون << '.' << *Subminor;
  إذا (std::optional<بیعلامت> Build = V.getBuild())
    بیرون << '.' << *Build;
  إذا (std::optional<بیعلامت> Subbuild = V.getSubbuild())
    بیرون << '.' << *Subbuild;
  أرجع بیرون;
}

ساكن منطقي parseInt(StringRef &درون_داد_2, بیعلامت &مقدار_2) {
  أكد(مقدار_2 == 0);
  إذا (درون_داد_2.خالی())
    أرجع درست;

  حرف بعدی = درون_داد_2[0];
  درون_داد_2 = درون_داد_2.substr(1);
  إذا (بعدی < '0' || بعدی > '9')
    أرجع درست;
  مقدار_2 = (بیعلامت)(بعدی - '0');

  طالما (!درون_داد_2.خالی()) {
    بعدی = درون_داد_2[0];
    إذا (بعدی < '0' || بعدی > '9')
      أرجع خطأ;
    درون_داد_2 = درون_داد_2.substr(1);
    مقدار_2 = مقدار_2 * 10 + (بیعلامت)(بعدی - '0');
  }

  أرجع خطأ;
}

منطقي نسخه_Tuple::tryParse(StringRef درون_داد_2) {
  بیعلامت major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(درون_داد_2, major))
    أرجع درست;

  إذا (درون_داد_2.خالی()) {
    *هذا = نسخه_Tuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (درون_داد_2[0] != '.')
    أرجع درست;
  درون_داد_2 = درون_داد_2.substr(1);
  إذا (parseInt(درون_داد_2, minor))
    أرجع درست;

  إذا (درون_داد_2.خالی()) {
    *هذا = نسخه_Tuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!درون_داد_2.consume_front("."))
    أرجع درست;
  إذا (parseInt(درون_داد_2, subminor))
    أرجع درست;

  إذا (درون_داد_2.خالی()) {
    *هذا = نسخه_Tuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!درون_داد_2.consume_front("."))
    أرجع درست;
  إذا (parseInt(درون_داد_2, build))
    أرجع درست;
  إذا (build >= 1024 * 1024)
    أرجع درست;

  إذا (درون_داد_2.خالی()) {
    *هذا = نسخه_Tuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!درون_داد_2.consume_front("."))
    أرجع درست;
  إذا (parseInt(درون_داد_2, subbuild))
    أرجع درست;
  إذا (subbuild >= 1024)
    أرجع درست;

  // If we have characters left over, it's an error.
  إذا (!درون_داد_2.خالی())
    أرجع درست;

  *هذا = نسخه_Tuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

نسخه_Tuple نسخه_Tuple::withMajorReplaced(بیعلامت NewMajor) ثابت {
  إذا (HasSubbuild)
    أرجع نسخه_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  إذا (HasBuild)
    أرجع نسخه_Tuple(NewMajor, Minor, Subminor, Build);
  إذا (HasSubminor)
    أرجع نسخه_Tuple(NewMajor, Minor, Subminor);
  إذا (HasMinor)
    أرجع نسخه_Tuple(NewMajor, Minor);
  أرجع نسخه_Tuple(NewMajor);
}
