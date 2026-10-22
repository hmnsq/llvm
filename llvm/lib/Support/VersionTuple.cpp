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

#समावेश "llvm/Support/VersionTuple.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>

using namespace llvm;

std::string VersionTuple::getAsString() स्थिर {
  std::string परिणाम;
  {
    llvm::raw_string_ostream Out(परिणाम);
    Out << *यह;
  }
  वापस परिणाम;
}

raw_ostream &llvm::चालक<<(raw_ostream &Out, स्थिर VersionTuple &V) {
  Out << V.getMajor();
  अगर (std::optional<अचिन्हित> Minor = V.getMinor())
    Out << '.' << *Minor;
  अगर (std::optional<अचिन्हित> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  अगर (std::optional<अचिन्हित> Build = V.getBuild())
    Out << '.' << *Build;
  अगर (std::optional<अचिन्हित> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  वापस Out;
}

अटल bool parseInt(StringRef &निवेश_2, अचिन्हित &मूल्य_2) {
  निश्चित(मूल्य_2 == 0);
  अगर (निवेश_2.empty())
    वापस true;

  अक्षर अग्र = निवेश_2[0];
  निवेश_2 = निवेश_2.substr(1);
  अगर (अग्र < '0' || अग्र > '9')
    वापस true;
  मूल्य_2 = (अचिन्हित)(अग्र - '0');

  जबतक (!निवेश_2.empty()) {
    अग्र = निवेश_2[0];
    अगर (अग्र < '0' || अग्र > '9')
      वापस false;
    निवेश_2 = निवेश_2.substr(1);
    मूल्य_2 = मूल्य_2 * 10 + (अचिन्हित)(अग्र - '0');
  }

  वापस false;
}

bool VersionTuple::tryParse(StringRef निवेश_2) {
  अचिन्हित major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  अगर (parseInt(निवेश_2, major))
    वापस true;

  अगर (निवेश_2.empty()) {
    *यह = VersionTuple(major);
    वापस false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  अगर (निवेश_2[0] != '.')
    वापस true;
  निवेश_2 = निवेश_2.substr(1);
  अगर (parseInt(निवेश_2, minor))
    वापस true;

  अगर (निवेश_2.empty()) {
    *यह = VersionTuple(major, minor);
    वापस false;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  अगर (!निवेश_2.consume_front("."))
    वापस true;
  अगर (parseInt(निवेश_2, subminor))
    वापस true;

  अगर (निवेश_2.empty()) {
    *यह = VersionTuple(major, minor, subminor);
    वापस false;
  }

  // If we're not done, parse the build version, \.[0-9]+
  अगर (!निवेश_2.consume_front("."))
    वापस true;
  अगर (parseInt(निवेश_2, build))
    वापस true;
  अगर (build >= 1024 * 1024)
    वापस true;

  अगर (निवेश_2.empty()) {
    *यह = VersionTuple(major, minor, subminor, build);
    वापस false;
  }

  // And the subbuild version, \.[0-9]+
  अगर (!निवेश_2.consume_front("."))
    वापस true;
  अगर (parseInt(निवेश_2, subbuild))
    वापस true;
  अगर (subbuild >= 1024)
    वापस true;

  // If we have characters left over, it's an error.
  अगर (!निवेश_2.empty())
    वापस true;

  *यह = VersionTuple(major, minor, subminor, build, subbuild);
  वापस false;
}

VersionTuple VersionTuple::withMajorReplaced(अचिन्हित NewMajor) स्थिर {
  अगर (HasSubbuild)
    वापस VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  अगर (HasBuild)
    वापस VersionTuple(NewMajor, Minor, Subminor, Build);
  अगर (HasSubminor)
    वापस VersionTuple(NewMajor, Minor, Subminor);
  अगर (HasMinor)
    वापस VersionTuple(NewMajor, Minor);
  वापस VersionTuple(NewMajor);
}
