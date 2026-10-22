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

#অন্তর্ভাৱ "llvm/Support/VersionTuple.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <cassert>

using নামাকাশ llvm;

std::string VersionTuple::getAsString() ধ্রুৱ {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *এতত্;
  }
  ঘূৰাই Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ধ্রুৱ VersionTuple &V) {
  Out << V.getMajor();
  যদি (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  যদি (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  যদি (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  যদি (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ঘূৰাই Out;
}

স্থির তার্কিক parseInt(StringRef &প্ৰবেশ_2, unsigned &মান_2) {
  প্রতিজ্ঞা(মান_2 == 0);
  যদি (প্ৰবেশ_2.empty())
    ঘূৰাই সঁচা;

  ৱর্ণ next = প্ৰবেশ_2[0];
  প্ৰবেশ_2 = প্ৰবেশ_2.substr(1);
  যদি (next < '0' || next > '9')
    ঘূৰাই সঁচা;
  মান_2 = (unsigned)(next - '0');

  যেতিয়ালৈ (!প্ৰবেশ_2.empty()) {
    next = প্ৰবেশ_2[0];
    যদি (next < '0' || next > '9')
      ঘূৰাই মিছা;
    প্ৰবেশ_2 = প্ৰবেশ_2.substr(1);
    মান_2 = মান_2 * 10 + (unsigned)(next - '0');
  }

  ঘূৰাই মিছা;
}

তার্কিক VersionTuple::tryParse(StringRef প্ৰবেশ_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  যদি (parseInt(প্ৰবেশ_2, major))
    ঘূৰাই সঁচা;

  যদি (প্ৰবেশ_2.empty()) {
    *এতত্ = VersionTuple(major);
    ঘূৰাই মিছা;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  যদি (প্ৰবেশ_2[0] != '.')
    ঘূৰাই সঁচা;
  প্ৰবেশ_2 = প্ৰবেশ_2.substr(1);
  যদি (parseInt(প্ৰবেশ_2, minor))
    ঘূৰাই সঁচা;

  যদি (প্ৰবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor);
    ঘূৰাই মিছা;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  যদি (!প্ৰবেশ_2.consume_front("."))
    ঘূৰাই সঁচা;
  যদি (parseInt(প্ৰবেশ_2, subminor))
    ঘূৰাই সঁচা;

  যদি (প্ৰবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor, subminor);
    ঘূৰাই মিছা;
  }

  // If we're not done, parse the build version, \.[0-9]+
  যদি (!প্ৰবেশ_2.consume_front("."))
    ঘূৰাই সঁচা;
  যদি (parseInt(প্ৰবেশ_2, build))
    ঘূৰাই সঁচা;
  যদি (build >= 1024 * 1024)
    ঘূৰাই সঁচা;

  যদি (প্ৰবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor, subminor, build);
    ঘূৰাই মিছা;
  }

  // And the subbuild version, \.[0-9]+
  যদি (!প্ৰবেশ_2.consume_front("."))
    ঘূৰাই সঁচা;
  যদি (parseInt(প্ৰবেশ_2, subbuild))
    ঘূৰাই সঁচা;
  যদি (subbuild >= 1024)
    ঘূৰাই সঁচা;

  // If we have characters left over, it's an error.
  যদি (!প্ৰবেশ_2.empty())
    ঘূৰাই সঁচা;

  *এতত্ = VersionTuple(major, minor, subminor, build, subbuild);
  ঘূৰাই মিছা;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ধ্রুৱ {
  যদি (HasSubbuild)
    ঘূৰাই VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  যদি (HasBuild)
    ঘূৰাই VersionTuple(NewMajor, Minor, Subminor, Build);
  যদি (HasSubminor)
    ঘূৰাই VersionTuple(NewMajor, Minor, Subminor);
  যদি (HasMinor)
    ঘূৰাই VersionTuple(NewMajor, Minor);
  ঘূৰাই VersionTuple(NewMajor);
}
