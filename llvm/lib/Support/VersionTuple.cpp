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

استعمال نطاق llvm;

std::string نسخو_Tuple::getAsString() ثابت {
  std::string نتيجو;
  {
    llvm::raw_string_ostream ٻاهريون(نتيجو);
    ٻاهريون << *هذا;
  }
  أرجع نتيجو;
}

raw_ostream &llvm::عامل<<(raw_ostream &ٻاهريون, ثابت نسخو_Tuple &V) {
  ٻاهريون << V.getMajor();
  إذا (std::optional<بنانشان> Minor = V.getMinor())
    ٻاهريون << '.' << *Minor;
  إذا (std::optional<بنانشان> Subminor = V.getSubminor())
    ٻاهريون << '.' << *Subminor;
  إذا (std::optional<بنانشان> Build = V.getBuild())
    ٻاهريون << '.' << *Build;
  إذا (std::optional<بنانشان> Subbuild = V.getSubbuild())
    ٻاهريون << '.' << *Subbuild;
  أرجع ٻاهريون;
}

ساكن منطقي parseInt(StringRef &داخلا_2, بنانشان &قدر_2) {
  أكد(قدر_2 == 0);
  إذا (داخلا_2.خالي_جايون())
    أرجع سچ;

  حرف ايندڙ = داخلا_2[0];
  داخلا_2 = داخلا_2.substr(1);
  إذا (ايندڙ < '0' || ايندڙ > '9')
    أرجع سچ;
  قدر_2 = (بنانشان)(ايندڙ - '0');

  طالما (!داخلا_2.خالي_جايون()) {
    ايندڙ = داخلا_2[0];
    إذا (ايندڙ < '0' || ايندڙ > '9')
      أرجع خطأ;
    داخلا_2 = داخلا_2.substr(1);
    قدر_2 = قدر_2 * 10 + (بنانشان)(ايندڙ - '0');
  }

  أرجع خطأ;
}

منطقي نسخو_Tuple::tryParse(StringRef داخلا_2) {
  بنانشان major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(داخلا_2, major))
    أرجع سچ;

  إذا (داخلا_2.خالي_جايون()) {
    *هذا = نسخو_Tuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (داخلا_2[0] != '.')
    أرجع سچ;
  داخلا_2 = داخلا_2.substr(1);
  إذا (parseInt(داخلا_2, minor))
    أرجع سچ;

  إذا (داخلا_2.خالي_جايون()) {
    *هذا = نسخو_Tuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!داخلا_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلا_2, subminor))
    أرجع سچ;

  إذا (داخلا_2.خالي_جايون()) {
    *هذا = نسخو_Tuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!داخلا_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلا_2, build))
    أرجع سچ;
  إذا (build >= 1024 * 1024)
    أرجع سچ;

  إذا (داخلا_2.خالي_جايون()) {
    *هذا = نسخو_Tuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!داخلا_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلا_2, subbuild))
    أرجع سچ;
  إذا (subbuild >= 1024)
    أرجع سچ;

  // If we have characters left over, it's an error.
  إذا (!داخلا_2.خالي_جايون())
    أرجع سچ;

  *هذا = نسخو_Tuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

نسخو_Tuple نسخو_Tuple::withMajorReplaced(بنانشان NewMajor) ثابت {
  إذا (HasSubbuild)
    أرجع نسخو_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  إذا (HasBuild)
    أرجع نسخو_Tuple(NewMajor, Minor, Subminor, Build);
  إذا (HasSubminor)
    أرجع نسخو_Tuple(NewMajor, Minor, Subminor);
  إذا (HasMinor)
    أرجع نسخو_Tuple(NewMajor, Minor);
  أرجع نسخو_Tuple(NewMajor);
}
