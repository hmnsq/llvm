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

#अन्तर्भाव "llvm/Support/VersionTuple.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>

उपयुज्य नामाकाश llvm;

std::string संस्करणम्_Tuple::getAsString() ध्रुव {
  std::string परिणाम;
  {
    llvm::raw_string_ostream बहिर्गमनम्(परिणाम);
    बहिर्गमनम् << *एतत्;
  }
  प्रत्यागम परिणाम;
}

raw_ostream &llvm::संक्रियकः<<(raw_ostream &बहिर्गमनम्, ध्रुव संस्करणम्_Tuple &V) {
  बहिर्गमनम् << V.getMajor();
  यदि (std::optional<अचिह्नित> Minor = V.getMinor())
    बहिर्गमनम् << '.' << *Minor;
  यदि (std::optional<अचिह्नित> Subminor = V.getSubminor())
    बहिर्गमनम् << '.' << *Subminor;
  यदि (std::optional<अचिह्नित> Build = V.getBuild())
    बहिर्गमनम् << '.' << *Build;
  यदि (std::optional<अचिह्नित> Subbuild = V.getSubbuild())
    बहिर्गमनम् << '.' << *Subbuild;
  प्रत्यागम बहिर्गमनम्;
}

स्थिर तार्किक parseInt(StringRef &निवेशः_2, अचिह्नित &मानम्_2) {
  प्रतिज्ञा(मानम्_2 == 0);
  यदि (निवेशः_2.रिक्तानि())
    प्रत्यागम सत्य;

  वर्ण अग्र = निवेशः_2[0];
  निवेशः_2 = निवेशः_2.substr(1);
  यदि (अग्र < '0' || अग्र > '9')
    प्रत्यागम सत्य;
  मानम्_2 = (अचिह्नित)(अग्र - '0');

  यावत् (!निवेशः_2.रिक्तानि()) {
    अग्र = निवेशः_2[0];
    यदि (अग्र < '0' || अग्र > '9')
      प्रत्यागम असत्य;
    निवेशः_2 = निवेशः_2.substr(1);
    मानम्_2 = मानम्_2 * 10 + (अचिह्नित)(अग्र - '0');
  }

  प्रत्यागम असत्य;
}

तार्किक संस्करणम्_Tuple::tryParse(StringRef निवेशः_2) {
  अचिह्नित major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  यदि (parseInt(निवेशः_2, major))
    प्रत्यागम सत्य;

  यदि (निवेशः_2.रिक्तानि()) {
    *एतत् = संस्करणम्_Tuple(major);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  यदि (निवेशः_2[0] != '.')
    प्रत्यागम सत्य;
  निवेशः_2 = निवेशः_2.substr(1);
  यदि (parseInt(निवेशः_2, minor))
    प्रत्यागम सत्य;

  यदि (निवेशः_2.रिक्तानि()) {
    *एतत् = संस्करणम्_Tuple(major, minor);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  यदि (!निवेशः_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेशः_2, subminor))
    प्रत्यागम सत्य;

  यदि (निवेशः_2.रिक्तानि()) {
    *एतत् = संस्करणम्_Tuple(major, minor, subminor);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the build version, \.[0-9]+
  यदि (!निवेशः_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेशः_2, build))
    प्रत्यागम सत्य;
  यदि (build >= 1024 * 1024)
    प्रत्यागम सत्य;

  यदि (निवेशः_2.रिक्तानि()) {
    *एतत् = संस्करणम्_Tuple(major, minor, subminor, build);
    प्रत्यागम असत्य;
  }

  // And the subbuild version, \.[0-9]+
  यदि (!निवेशः_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेशः_2, subbuild))
    प्रत्यागम सत्य;
  यदि (subbuild >= 1024)
    प्रत्यागम सत्य;

  // If we have characters left over, it's an error.
  यदि (!निवेशः_2.रिक्तानि())
    प्रत्यागम सत्य;

  *एतत् = संस्करणम्_Tuple(major, minor, subminor, build, subbuild);
  प्रत्यागम असत्य;
}

संस्करणम्_Tuple संस्करणम्_Tuple::withMajorReplaced(अचिह्नित NewMajor) ध्रुव {
  यदि (HasSubbuild)
    प्रत्यागम संस्करणम्_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  यदि (HasBuild)
    प्रत्यागम संस्करणम्_Tuple(NewMajor, Minor, Subminor, Build);
  यदि (HasSubminor)
    प्रत्यागम संस्करणम्_Tuple(NewMajor, Minor, Subminor);
  यदि (HasMinor)
    प्रत्यागम संस्करणम्_Tuple(NewMajor, Minor);
  प्रत्यागम संस्करणम्_Tuple(NewMajor);
}
