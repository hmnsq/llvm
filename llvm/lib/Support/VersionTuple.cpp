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
  पटिनिवत्त Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ध्रुव VersionTuple &V) {
  Out << V.getMajor();
  सचे (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  सचे (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  सचे (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  सचे (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  पटिनिवत्त Out;
}

स्थिर तार्किक parseInt(StringRef &निवेशः_2, unsigned &मानम्_2) {
  प्रतिज्ञा(मानम्_2 == 0);
  सचे (निवेशः_2.empty())
    पटिनिवत्त सच्च;

  वर्ण next = निवेशः_2[0];
  निवेशः_2 = निवेशः_2.substr(1);
  सचे (next < '0' || next > '9')
    पटिनिवत्त सच्च;
  मानम्_2 = (unsigned)(next - '0');

  याव (!निवेशः_2.empty()) {
    next = निवेशः_2[0];
    सचे (next < '0' || next > '9')
      पटिनिवत्त मिच्छा;
    निवेशः_2 = निवेशः_2.substr(1);
    मानम्_2 = मानम्_2 * 10 + (unsigned)(next - '0');
  }

  पटिनिवत्त मिच्छा;
}

तार्किक VersionTuple::tryParse(StringRef निवेशः_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  सचे (parseInt(निवेशः_2, major))
    पटिनिवत्त सच्च;

  सचे (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major);
    पटिनिवत्त मिच्छा;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  सचे (निवेशः_2[0] != '.')
    पटिनिवत्त सच्च;
  निवेशः_2 = निवेशः_2.substr(1);
  सचे (parseInt(निवेशः_2, minor))
    पटिनिवत्त सच्च;

  सचे (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor);
    पटिनिवत्त मिच्छा;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  सचे (!निवेशः_2.consume_front("."))
    पटिनिवत्त सच्च;
  सचे (parseInt(निवेशः_2, subminor))
    पटिनिवत्त सच्च;

  सचे (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor);
    पटिनिवत्त मिच्छा;
  }

  // If we're not done, parse the build version, \.[0-9]+
  सचे (!निवेशः_2.consume_front("."))
    पटिनिवत्त सच्च;
  सचे (parseInt(निवेशः_2, build))
    पटिनिवत्त सच्च;
  सचे (build >= 1024 * 1024)
    पटिनिवत्त सच्च;

  सचे (निवेशः_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor, build);
    पटिनिवत्त मिच्छा;
  }

  // And the subbuild version, \.[0-9]+
  सचे (!निवेशः_2.consume_front("."))
    पटिनिवत्त सच्च;
  सचे (parseInt(निवेशः_2, subbuild))
    पटिनिवत्त सच्च;
  सचे (subbuild >= 1024)
    पटिनिवत्त सच्च;

  // If we have characters left over, it's an error.
  सचे (!निवेशः_2.empty())
    पटिनिवत्त सच्च;

  *एतत् = VersionTuple(major, minor, subminor, build, subbuild);
  पटिनिवत्त मिच्छा;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ध्रुव {
  सचे (HasSubbuild)
    पटिनिवत्त VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  सचे (HasBuild)
    पटिनिवत्त VersionTuple(NewMajor, Minor, Subminor, Build);
  सचे (HasSubminor)
    पटिनिवत्त VersionTuple(NewMajor, Minor, Subminor);
  सचे (HasMinor)
    पटिनिवत्त VersionTuple(NewMajor, Minor);
  पटिनिवत्त VersionTuple(NewMajor);
}
