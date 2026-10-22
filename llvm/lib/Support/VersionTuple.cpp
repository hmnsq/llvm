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

#അന്തര്ഭാവ "llvm/Support/VersionTuple.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cassert>

using നാമാകാശ llvm;

std::string VersionTuple::getAsString() സ്ഥിരം {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *ഇത്;
  }
  തിരികെ Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, സ്ഥിരം VersionTuple &V) {
  Out << V.getMajor();
  എങ്കിൽ (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  എങ്കിൽ (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  എങ്കിൽ (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  എങ്കിൽ (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  തിരികെ Out;
}

സ്ഥിതം തർക്കം parseInt(StringRef &നിവേശം_2, unsigned &മൂല്യം_2) {
  പ്രതിജ്ഞാ(മൂല്യം_2 == 0);
  എങ്കിൽ (നിവേശം_2.empty())
    തിരികെ സത്യം;

  അക്ഷരം next = നിവേശം_2[0];
  നിവേശം_2 = നിവേശം_2.substr(1);
  എങ്കിൽ (next < '0' || next > '9')
    തിരികെ സത്യം;
  മൂല്യം_2 = (unsigned)(next - '0');

  ആകുമ്പോൾ (!നിവേശം_2.empty()) {
    next = നിവേശം_2[0];
    എങ്കിൽ (next < '0' || next > '9')
      തിരികെ അസത്യം;
    നിവേശം_2 = നിവേശം_2.substr(1);
    മൂല്യം_2 = മൂല്യം_2 * 10 + (unsigned)(next - '0');
  }

  തിരികെ അസത്യം;
}

തർക്കം VersionTuple::tryParse(StringRef നിവേശം_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  എങ്കിൽ (parseInt(നിവേശം_2, major))
    തിരികെ സത്യം;

  എങ്കിൽ (നിവേശം_2.empty()) {
    *ഇത് = VersionTuple(major);
    തിരികെ അസത്യം;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  എങ്കിൽ (നിവേശം_2[0] != '.')
    തിരികെ സത്യം;
  നിവേശം_2 = നിവേശം_2.substr(1);
  എങ്കിൽ (parseInt(നിവേശം_2, minor))
    തിരികെ സത്യം;

  എങ്കിൽ (നിവേശം_2.empty()) {
    *ഇത് = VersionTuple(major, minor);
    തിരികെ അസത്യം;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  എങ്കിൽ (!നിവേശം_2.consume_front("."))
    തിരികെ സത്യം;
  എങ്കിൽ (parseInt(നിവേശം_2, subminor))
    തിരികെ സത്യം;

  എങ്കിൽ (നിവേശം_2.empty()) {
    *ഇത് = VersionTuple(major, minor, subminor);
    തിരികെ അസത്യം;
  }

  // If we're not done, parse the build version, \.[0-9]+
  എങ്കിൽ (!നിവേശം_2.consume_front("."))
    തിരികെ സത്യം;
  എങ്കിൽ (parseInt(നിവേശം_2, build))
    തിരികെ സത്യം;
  എങ്കിൽ (build >= 1024 * 1024)
    തിരികെ സത്യം;

  എങ്കിൽ (നിവേശം_2.empty()) {
    *ഇത് = VersionTuple(major, minor, subminor, build);
    തിരികെ അസത്യം;
  }

  // And the subbuild version, \.[0-9]+
  എങ്കിൽ (!നിവേശം_2.consume_front("."))
    തിരികെ സത്യം;
  എങ്കിൽ (parseInt(നിവേശം_2, subbuild))
    തിരികെ സത്യം;
  എങ്കിൽ (subbuild >= 1024)
    തിരികെ സത്യം;

  // If we have characters left over, it's an error.
  എങ്കിൽ (!നിവേശം_2.empty())
    തിരികെ സത്യം;

  *ഇത് = VersionTuple(major, minor, subminor, build, subbuild);
  തിരികെ അസത്യം;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) സ്ഥിരം {
  എങ്കിൽ (HasSubbuild)
    തിരികെ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  എങ്കിൽ (HasBuild)
    തിരികെ VersionTuple(NewMajor, Minor, Subminor, Build);
  എങ്കിൽ (HasSubminor)
    തിരികെ VersionTuple(NewMajor, Minor, Subminor);
  എങ്കിൽ (HasMinor)
    തിരികെ VersionTuple(NewMajor, Minor);
  തിരികെ VersionTuple(NewMajor);
}
