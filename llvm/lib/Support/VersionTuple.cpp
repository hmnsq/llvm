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

ورتوں نطاق llvm;

std::string نسخہ_Tuple::getAsString() ثابت {
  std::string نتیجہ;
  {
    llvm::raw_string_ostream باہرلا(نتیجہ);
    باہرلا << *هذا;
  }
  أرجع نتیجہ;
}

raw_ostream &llvm::عامل<<(raw_ostream &باہرلا, ثابت نسخہ_Tuple &V) {
  باہرلا << V.getMajor();
  إذا (std::optional<بنانشانی> Minor = V.getMinor())
    باہرلا << '.' << *Minor;
  إذا (std::optional<بنانشانی> Subminor = V.getSubminor())
    باہرلا << '.' << *Subminor;
  إذا (std::optional<بنانشانی> Build = V.getBuild())
    باہرلا << '.' << *Build;
  إذا (std::optional<بنانشانی> Subbuild = V.getSubbuild())
    باہرلا << '.' << *Subbuild;
  أرجع باہرلا;
}

ساكن منطقي parseInt(StringRef &داخلہ_2, بنانشانی &قدر_2) {
  أكد(قدر_2 == 0);
  إذا (داخلہ_2.خالی_تھاواں())
    أرجع سچ;

  حرف اگلا = داخلہ_2[0];
  داخلہ_2 = داخلہ_2.substr(1);
  إذا (اگلا < '0' || اگلا > '9')
    أرجع سچ;
  قدر_2 = (بنانشانی)(اگلا - '0');

  طالما (!داخلہ_2.خالی_تھاواں()) {
    اگلا = داخلہ_2[0];
    إذا (اگلا < '0' || اگلا > '9')
      أرجع خطأ;
    داخلہ_2 = داخلہ_2.substr(1);
    قدر_2 = قدر_2 * 10 + (بنانشانی)(اگلا - '0');
  }

  أرجع خطأ;
}

منطقي نسخہ_Tuple::tryParse(StringRef داخلہ_2) {
  بنانشانی major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  إذا (parseInt(داخلہ_2, major))
    أرجع سچ;

  إذا (داخلہ_2.خالی_تھاواں()) {
    *هذا = نسخہ_Tuple(major);
    أرجع خطأ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  إذا (داخلہ_2[0] != '.')
    أرجع سچ;
  داخلہ_2 = داخلہ_2.substr(1);
  إذا (parseInt(داخلہ_2, minor))
    أرجع سچ;

  إذا (داخلہ_2.خالی_تھاواں()) {
    *هذا = نسخہ_Tuple(major, minor);
    أرجع خطأ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  إذا (!داخلہ_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلہ_2, subminor))
    أرجع سچ;

  إذا (داخلہ_2.خالی_تھاواں()) {
    *هذا = نسخہ_Tuple(major, minor, subminor);
    أرجع خطأ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  إذا (!داخلہ_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلہ_2, build))
    أرجع سچ;
  إذا (build >= 1024 * 1024)
    أرجع سچ;

  إذا (داخلہ_2.خالی_تھاواں()) {
    *هذا = نسخہ_Tuple(major, minor, subminor, build);
    أرجع خطأ;
  }

  // And the subbuild version, \.[0-9]+
  إذا (!داخلہ_2.consume_front("."))
    أرجع سچ;
  إذا (parseInt(داخلہ_2, subbuild))
    أرجع سچ;
  إذا (subbuild >= 1024)
    أرجع سچ;

  // If we have characters left over, it's an error.
  إذا (!داخلہ_2.خالی_تھاواں())
    أرجع سچ;

  *هذا = نسخہ_Tuple(major, minor, subminor, build, subbuild);
  أرجع خطأ;
}

نسخہ_Tuple نسخہ_Tuple::withMajorReplaced(بنانشانی NewMajor) ثابت {
  إذا (HasSubbuild)
    أرجع نسخہ_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  إذا (HasBuild)
    أرجع نسخہ_Tuple(NewMajor, Minor, Subminor, Build);
  إذا (HasSubminor)
    أرجع نسخہ_Tuple(NewMajor, Minor, Subminor);
  إذا (HasMinor)
    أرجع نسخہ_Tuple(NewMajor, Minor);
  أرجع نسخہ_Tuple(NewMajor);
}
