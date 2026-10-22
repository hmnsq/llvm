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

کارول نطاق llvm;

std::string نسخه_Tuple::getAsString() ثابت {
  std::string پایله;
  {
    llvm::raw_string_ostream بهر(پایله);
    بهر << *هذا;
  }
  أرجع پایله;
}

raw_ostream &llvm::چلوونکی<<(raw_ostream &بهر, ثابت نسخه_Tuple &V) {
  بهر << V.getMajor();
  إذا (std::optional<بینښه> Minor = V.getMinor())
    بهر << '.' << *Minor;
  إذا (std::optional<بینښه> Subminor = V.getSubminor())
    بهر << '.' << *Subminor;
  إذا (std::optional<بینښه> Build = V.getBuild())
    بهر << '.' << *Build;
  إذا (std::optional<بینښه> Subbuild = V.getSubbuild())
    بهر << '.' << *Subbuild;
  أرجع بهر;
}

ساكن منطقي parseInt(StringRef &ننوت_2, بینښه &ارزښت_2) {
  أكد(ارزښت_2 == 0);
  إذا (ننوت_2.تشه())
    أرجع ریښتیا;

  حرف بل = ننوت_2[0];
  ننوت_2 = ننوت_2.substr(1);
  إذا (بل < '0' || بل > '9')
    أرجع ریښتیا;
  ارزښت_2 = (بینښه)(بل - '0');

  طالما (!ننوت_2.تشه()) {
    بل = ننوت_2[0];
    إذا (بل < '0' || بل > '9')
      أرجع خطأ;
    ننوت_2 = ننوت_2.substr(1);
    ارزښت_2 = ارزښت_2 * 10 + (بینښه)(بل - '0');
  }

  أرجع خطأ;
}

منطقي نسخه_Tuple::tryParse(StringRef ننوت_2) {
  بینښه major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(ننوت_2, major))
    أرجع ریښتیا;

  إذا (ننوت_2.تشه()) {
    *هذا = نسخه_Tuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (ننوت_2[0] != '.')
    أرجع ریښتیا;
  ننوت_2 = ننوت_2.substr(1);
  إذا (parseInt(ننوت_2, minor))
    أرجع ریښتیا;

  إذا (ننوت_2.تشه()) {
    *هذا = نسخه_Tuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!ننوت_2.consume_front("."))
    أرجع ریښتیا;
  إذا (parseInt(ننوت_2, subminor))
    أرجع ریښتیا;

  إذا (ننوت_2.تشه()) {
    *هذا = نسخه_Tuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!ننوت_2.consume_front("."))
    أرجع ریښتیا;
  إذا (parseInt(ننوت_2, build))
    أرجع ریښتیا;
  إذا (build >= 1024 * 1024)
    أرجع ریښتیا;

  إذا (ننوت_2.تشه()) {
    *هذا = نسخه_Tuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!ننوت_2.consume_front("."))
    أرجع ریښتیا;
  إذا (parseInt(ننوت_2, subbuild))
    أرجع ریښتیا;
  إذا (subbuild >= 1024)
    أرجع ریښتیا;

  // If we have characters left over, it's an error.
  إذا (!ننوت_2.تشه())
    أرجع ریښتیا;

  *هذا = نسخه_Tuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

نسخه_Tuple نسخه_Tuple::withMajorReplaced(بینښه NewMajor) ثابت {
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
