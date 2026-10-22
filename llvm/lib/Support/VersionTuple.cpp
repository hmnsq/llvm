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

#شامل "llvm/Support/VersionTuple.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>

استعمال نامخلا llvm;

std::string نسخہ_Tuple::getAsString() ثابت {
  std::string نتیجہ;
  {
    llvm::raw_string_ostream برآمد(نتیجہ);
    برآمد << *یہ;
  }
  واپس نتیجہ;
}

raw_ostream &llvm::عامل<<(raw_ostream &برآمد, ثابت نسخہ_Tuple &V) {
  برآمد << V.getMajor();
  اگر (std::optional<غیرعلامتی> Minor = V.getMinor())
    برآمد << '.' << *Minor;
  اگر (std::optional<غیرعلامتی> Subminor = V.getSubminor())
    برآمد << '.' << *Subminor;
  اگر (std::optional<غیرعلامتی> Build = V.getBuild())
    برآمد << '.' << *Build;
  اگر (std::optional<غیرعلامتی> Subbuild = V.getSubbuild())
    برآمد << '.' << *Subbuild;
  واپس برآمد;
}

ساکن منطقی parseInt(StringRef &داخلہ_2, غیرعلامتی &قدر_2) {
  أكد(قدر_2 == 0);
  اگر (داخلہ_2.خالی_خانے())
    واپس سچ;

  حرف اگلا = داخلہ_2[0];
  داخلہ_2 = داخلہ_2.substr(1);
  اگر (اگلا < '0' || اگلا > '9')
    واپس سچ;
  قدر_2 = (غیرعلامتی)(اگلا - '0');

  جب (!داخلہ_2.خالی_خانے()) {
    اگلا = داخلہ_2[0];
    اگر (اگلا < '0' || اگلا > '9')
      واپس جھوٹ;
    داخلہ_2 = داخلہ_2.substr(1);
    قدر_2 = قدر_2 * 10 + (غیرعلامتی)(اگلا - '0');
  }

  واپس جھوٹ;
}

منطقی نسخہ_Tuple::tryParse(StringRef داخلہ_2) {
  غیرعلامتی major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  اگر (parseInt(داخلہ_2, major))
    واپس سچ;

  اگر (داخلہ_2.خالی_خانے()) {
    *یہ = نسخہ_Tuple(major);
    واپس جھوٹ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  اگر (داخلہ_2[0] != '.')
    واپس سچ;
  داخلہ_2 = داخلہ_2.substr(1);
  اگر (parseInt(داخلہ_2, minor))
    واپس سچ;

  اگر (داخلہ_2.خالی_خانے()) {
    *یہ = نسخہ_Tuple(major, minor);
    واپس جھوٹ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  اگر (!داخلہ_2.consume_front("."))
    واپس سچ;
  اگر (parseInt(داخلہ_2, subminor))
    واپس سچ;

  اگر (داخلہ_2.خالی_خانے()) {
    *یہ = نسخہ_Tuple(major, minor, subminor);
    واپس جھوٹ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  اگر (!داخلہ_2.consume_front("."))
    واپس سچ;
  اگر (parseInt(داخلہ_2, build))
    واپس سچ;
  اگر (build >= 1024 * 1024)
    واپس سچ;

  اگر (داخلہ_2.خالی_خانے()) {
    *یہ = نسخہ_Tuple(major, minor, subminor, build);
    واپس جھوٹ;
  }

  // And the subbuild version, \.[0-9]+
  اگر (!داخلہ_2.consume_front("."))
    واپس سچ;
  اگر (parseInt(داخلہ_2, subbuild))
    واپس سچ;
  اگر (subbuild >= 1024)
    واپس سچ;

  // If we have characters left over, it's an error.
  اگر (!داخلہ_2.خالی_خانے())
    واپس سچ;

  *یہ = نسخہ_Tuple(major, minor, subminor, build, subbuild);
  واپس جھوٹ;
}

نسخہ_Tuple نسخہ_Tuple::withMajorReplaced(غیرعلامتی NewMajor) ثابت {
  اگر (HasSubbuild)
    واپس نسخہ_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  اگر (HasBuild)
    واپس نسخہ_Tuple(NewMajor, Minor, Subminor, Build);
  اگر (HasSubminor)
    واپس نسخہ_Tuple(NewMajor, Minor, Subminor);
  اگر (HasMinor)
    واپس نسخہ_Tuple(NewMajor, Minor);
  واپس نسخہ_Tuple(NewMajor);
}
