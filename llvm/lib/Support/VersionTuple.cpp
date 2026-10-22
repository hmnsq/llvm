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

#અન્તર્ભાવ "llvm/Support/VersionTuple.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>

using નામાકાશ llvm;

std::string VersionTuple::getAsString() અચલ {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *આ;
  }
  પાછા Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, અચલ VersionTuple &V) {
  Out << V.getMajor();
  જો (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  જો (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  જો (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  જો (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  પાછા Out;
}

સ્થિર તાર્કિક parseInt(StringRef &આગત_2, unsigned &કિંમત_2) {
  પ્રતિજ્ઞા(કિંમત_2 == 0);
  જો (આગત_2.empty())
    પાછા સાચું;

  અક્ષર next = આગત_2[0];
  આગત_2 = આગત_2.substr(1);
  જો (next < '0' || next > '9')
    પાછા સાચું;
  કિંમત_2 = (unsigned)(next - '0');

  જ્યારે (!આગત_2.empty()) {
    next = આગત_2[0];
    જો (next < '0' || next > '9')
      પાછા ખોટું;
    આગત_2 = આગત_2.substr(1);
    કિંમત_2 = કિંમત_2 * 10 + (unsigned)(next - '0');
  }

  પાછા ખોટું;
}

તાર્કિક VersionTuple::tryParse(StringRef આગત_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  જો (parseInt(આગત_2, major))
    પાછા સાચું;

  જો (આગત_2.empty()) {
    *આ = VersionTuple(major);
    પાછા ખોટું;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  જો (આગત_2[0] != '.')
    પાછા સાચું;
  આગત_2 = આગત_2.substr(1);
  જો (parseInt(આગત_2, minor))
    પાછા સાચું;

  જો (આગત_2.empty()) {
    *આ = VersionTuple(major, minor);
    પાછા ખોટું;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  જો (!આગત_2.consume_front("."))
    પાછા સાચું;
  જો (parseInt(આગત_2, subminor))
    પાછા સાચું;

  જો (આગત_2.empty()) {
    *આ = VersionTuple(major, minor, subminor);
    પાછા ખોટું;
  }

  // If we're not done, parse the build version, \.[0-9]+
  જો (!આગત_2.consume_front("."))
    પાછા સાચું;
  જો (parseInt(આગત_2, build))
    પાછા સાચું;
  જો (build >= 1024 * 1024)
    પાછા સાચું;

  જો (આગત_2.empty()) {
    *આ = VersionTuple(major, minor, subminor, build);
    પાછા ખોટું;
  }

  // And the subbuild version, \.[0-9]+
  જો (!આગત_2.consume_front("."))
    પાછા સાચું;
  જો (parseInt(આગત_2, subbuild))
    પાછા સાચું;
  જો (subbuild >= 1024)
    પાછા સાચું;

  // If we have characters left over, it's an error.
  જો (!આગત_2.empty())
    પાછા સાચું;

  *આ = VersionTuple(major, minor, subminor, build, subbuild);
  પાછા ખોટું;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) અચલ {
  જો (HasSubbuild)
    પાછા VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  જો (HasBuild)
    પાછા VersionTuple(NewMajor, Minor, Subminor, Build);
  જો (HasSubminor)
    પાછા VersionTuple(NewMajor, Minor, Subminor);
  જો (HasMinor)
    પાછા VersionTuple(NewMajor, Minor);
  પાછા VersionTuple(NewMajor);
}
