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

using नामाकाश llvm;

std::string VersionTuple::getAsString() ध्रुव {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *एतत्;
  }
  प्रत्यागम Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ध्रुव VersionTuple &V) {
  Out << V.getMajor();
  जइ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  जइ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  जइ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  जइ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  प्रत्यागम Out;
}

स्थिर तार्किक parseInt(StringRef &निवेशः_2, unsigned &मानम्_2) {
  प्रतिज्ञा(मानम्_2 == 0);
  जइ (निवेशः_2.empty())
    प्रत्यागम सच्च;

  वर्ण next = निवेशः_2[0];
  निवेशः_2 = निवेशः_2.substr(1);
  जइ (next < '0' || next > '9')
    प्रत्यागम सच्च;
  मानम्_2 = (unsigned)(next - '0');

  जाव (!निवेशः_2.empty()) {
    next = निवेशः_2[0];
    जइ (next < '0' || next > '9')
      प्रत्यागम मिच्छ;
    निवेशः_2 = निवेशः_2.substr(1);
    मानम्_2 = मानम्_2 * 10 + (unsigned)(next - '0');
  }

  प्रत्यागम मिच्छ;
}

तार्किक VersionTuple::tryParse(StringRef निवेशः_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  जइ (parseInt(निवेशः_2, major))
    प्रत्यागम सच्च;

  जइ (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major);
    प्रत्यागम मिच्छ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  जइ (निवेशः_2[0] != '.')
    प्रत्यागम सच्च;
  निवेशः_2 = निवेशः_2.substr(1);
  जइ (parseInt(निवेशः_2, minor))
    प्रत्यागम सच्च;

  जइ (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor);
    प्रत्यागम मिच्छ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  जइ (!निवेशः_2.consume_front("."))
    प्रत्यागम सच्च;
  जइ (parseInt(निवेशः_2, subminor))
    प्रत्यागम सच्च;

  जइ (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor);
    प्रत्यागम मिच्छ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  जइ (!निवेशः_2.consume_front("."))
    प्रत्यागम सच्च;
  जइ (parseInt(निवेशः_2, build))
    प्रत्यागम सच्च;
  जइ (build >= 1024 * 1024)
    प्रत्यागम सच्च;

  जइ (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor, build);
    प्रत्यागम मिच्छ;
  }

  // And the subbuild version, \.[0-9]+
  जइ (!निवेशः_2.consume_front("."))
    प्रत्यागम सच्च;
  जइ (parseInt(निवेशः_2, subbuild))
    प्रत्यागम सच्च;
  जइ (subbuild >= 1024)
    प्रत्यागम सच्च;

  // If we have characters left over, it's an error.
  जइ (!निवेशः_2.empty())
    प्रत्यागम सच्च;

  *एतत् = VersionTuple(major, minor, subminor, build, subbuild);
  प्रत्यागम मिच्छ;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ध्रुव {
  जइ (HasSubbuild)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  जइ (HasBuild)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor, Build);
  जइ (HasSubminor)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor);
  जइ (HasMinor)
    प्रत्यागम VersionTuple(NewMajor, Minor);
  प्रत्यागम VersionTuple(NewMajor);
}
