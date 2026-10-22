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

std::string VersionTuple::getAsString() ধ্রুব {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *এতত্;
  }
  প্রত্যাগম Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, ধ্রুব VersionTuple &V) {
  Out << V.getMajor();
  যদি (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  যদি (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  যদি (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  যদি (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  প্রত্যাগম Out;
}

স্থির তার্কিক parseInt(StringRef &প্রবেশ_2, unsigned &মান_2) {
  প্রতিজ্ঞা(মান_2 == 0);
  যদি (প্রবেশ_2.empty())
    প্রত্যাগম সত্য;

  বর্ণ next = প্রবেশ_2[0];
  প্রবেশ_2 = প্রবেশ_2.substr(1);
  যদি (next < '0' || next > '9')
    প্রত্যাগম সত্য;
  মান_2 = (unsigned)(next - '0');

  যাবত্ (!প্রবেশ_2.empty()) {
    next = প্রবেশ_2[0];
    যদি (next < '0' || next > '9')
      প্রত্যাগম অসত্য;
    প্রবেশ_2 = প্রবেশ_2.substr(1);
    মান_2 = মান_2 * 10 + (unsigned)(next - '0');
  }

  প্রত্যাগম অসত্য;
}

তার্কিক VersionTuple::tryParse(StringRef প্রবেশ_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  যদি (parseInt(প্রবেশ_2, major))
    প্রত্যাগম সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এতত্ = VersionTuple(major);
    প্রত্যাগম অসত্য;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  যদি (প্রবেশ_2[0] != '.')
    প্রত্যাগম সত্য;
  প্রবেশ_2 = প্রবেশ_2.substr(1);
  যদি (parseInt(প্রবেশ_2, minor))
    প্রত্যাগম সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor);
    প্রত্যাগম অসত্য;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    প্রত্যাগম সত্য;
  যদি (parseInt(প্রবেশ_2, subminor))
    প্রত্যাগম সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor, subminor);
    প্রত্যাগম অসত্য;
  }

  // If we're not done, parse the build version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    প্রত্যাগম সত্য;
  যদি (parseInt(প্রবেশ_2, build))
    প্রত্যাগম সত্য;
  যদি (build >= 1024 * 1024)
    প্রত্যাগম সত্য;

  যদি (প্রবেশ_2.empty()) {
    *এতত্ = VersionTuple(major, minor, subminor, build);
    প্রত্যাগম অসত্য;
  }

  // And the subbuild version, \.[0-9]+
  যদি (!প্রবেশ_2.consume_front("."))
    প্রত্যাগম সত্য;
  যদি (parseInt(প্রবেশ_2, subbuild))
    প্রত্যাগম সত্য;
  যদি (subbuild >= 1024)
    প্রত্যাগম সত্য;

  // If we have characters left over, it's an error.
  যদি (!প্রবেশ_2.empty())
    প্রত্যাগম সত্য;

  *এতত্ = VersionTuple(major, minor, subminor, build, subbuild);
  প্রত্যাগম অসত্য;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) ধ্রুব {
  যদি (HasSubbuild)
    প্রত্যাগম VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  যদি (HasBuild)
    প্রত্যাগম VersionTuple(NewMajor, Minor, Subminor, Build);
  যদি (HasSubminor)
    প্রত্যাগম VersionTuple(NewMajor, Minor, Subminor);
  যদি (HasMinor)
    প্রত্যাগম VersionTuple(NewMajor, Minor);
  প্রত্যাগম VersionTuple(NewMajor);
}
