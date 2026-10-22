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

#অন্তর্ভাব "llvm/Support/VersionTuple.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cassert>

using নামাকাশ llvm;

std::string VersionTuple::getAsString() ধ্রুবক {
  std::string ফলাফল;
  {
    llvm::raw_string_ostream Out(ফলাফল);
    Out << *এই;
  }
  ফেরত ফলাফল;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ধ্রুবক VersionTuple &V) {
  Out << V.getMajor();
  যদি (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  যদি (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  যদি (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  যদি (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  ফেরত Out;
}

স্থির যৌক্তিক parseInt(StringRef &প্রবেশ_2, unsigned &মান_2) {
  প্রতিজ্ঞা(মান_2 == 0);
  যদি (প্রবেশ_2.empty())
    ফেরত সত্য;

  অক্ষর পরবর্তী = প্রবেশ_2[0];
  প্রবেশ_2 = প্রবেশ_2.substr(1);
  যদি (পরবর্তী < '0' || পরবর্তী > '9')
    ফেরত সত্য;
  মান_2 = (unsigned)(পরবর্তী - '0');

  যখন (!প্রবেশ_2.empty()) {
    পরবর্তী = প্রবেশ_2[0];
    যদি (পরবর্তী < '0' || পরবর্তী > '9')
      ফেরত মিথ্যা;
    প্রবেশ_2 = প্রবেশ_2.substr(1);
    মান_2 = মান_2 * 10 + (unsigned)(পরবর্তী - '0');
  }

  ফেরত মিথ্যা;
}

যৌক্তিক VersionTuple::tryParse(StringRef প্রবেশ_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  যদি (parseInt(প্রবেশ_2, major))
    ফেরত সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এই = VersionTuple(major);
    ফেরত মিথ্যা;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  যদি (প্রবেশ_2[0] != '.')
    ফেরত সত্য;
  প্রবেশ_2 = প্রবেশ_2.substr(1);
  যদি (parseInt(প্রবেশ_2, minor))
    ফেরত সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এই = VersionTuple(major, minor);
    ফেরত মিথ্যা;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    ফেরত সত্য;
  যদি (parseInt(প্রবেশ_2, subminor))
    ফেরত সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এই = VersionTuple(major, minor, subminor);
    ফেরত মিথ্যা;
  }

  // If we're not done, parse the build version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    ফেরত সত্য;
  যদি (parseInt(প্রবেশ_2, build))
    ফেরত সত্য;
  যদি (build >= 1024 * 1024)
    ফেরত সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এই = VersionTuple(major, minor, subminor, build);
    ফেরত মিথ্যা;
  }

  // And the subbuild version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    ফেরত সত্য;
  যদি (parseInt(প্রবেশ_2, subbuild))
    ফেরত সত্য;
  যদি (subbuild >= 1024)
    ফেরত সত্য;

  // If we have characters left over, it's an error.
  যদি (!প্রবেশ_2.empty())
    ফেরত সত্য;

  *এই = VersionTuple(major, minor, subminor, build, subbuild);
  ফেরত মিথ্যা;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ধ্রুবক {
  যদি (HasSubbuild)
    ফেরত VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  যদি (HasBuild)
    ফেরত VersionTuple(NewMajor, Minor, Subminor, Build);
  যদি (HasSubminor)
    ফেরত VersionTuple(NewMajor, Minor, Subminor);
  যদি (HasMinor)
    ফেরত VersionTuple(NewMajor, Minor);
  ফেরত VersionTuple(NewMajor);
}
