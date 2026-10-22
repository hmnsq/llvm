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

raw_ostream &llvm::चालक<<(raw_ostream &Out, ध्रुव VersionTuple &V) {
  Out << V.getMajor();
  यदि (std::optional<अचिन्हित> Minor = V.getMinor())
    Out << '.' << *Minor;
  यदि (std::optional<अचिन्हित> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  यदि (std::optional<अचिन्हित> Build = V.getBuild())
    Out << '.' << *Build;
  यदि (std::optional<अचिन्हित> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  प्रत्यागम Out;
}

स्थिर तार्किक parseInt(StringRef &निवेश_2, अचिन्हित &मूल्य_2) {
  प्रतिज्ञा(मूल्य_2 == 0);
  यदि (निवेश_2.empty())
    प्रत्यागम सत्य;

  वर्ण next = निवेश_2[0];
  निवेश_2 = निवेश_2.substr(1);
  यदि (next < '0' || next > '9')
    प्रत्यागम सत्य;
  मूल्य_2 = (अचिन्हित)(next - '0');

  यावत् (!निवेश_2.empty()) {
    next = निवेश_2[0];
    यदि (next < '0' || next > '9')
      प्रत्यागम असत्य;
    निवेश_2 = निवेश_2.substr(1);
    मूल्य_2 = मूल्य_2 * 10 + (अचिन्हित)(next - '0');
  }

  प्रत्यागम असत्य;
}

तार्किक VersionTuple::tryParse(StringRef निवेश_2) {
  अचिन्हित major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  यदि (parseInt(निवेश_2, major))
    प्रत्यागम सत्य;

  यदि (निवेश_2.empty()) {
    *एतत् = VersionTuple(major);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  यदि (निवेश_2[0] != '.')
    प्रत्यागम सत्य;
  निवेश_2 = निवेश_2.substr(1);
  यदि (parseInt(निवेश_2, minor))
    प्रत्यागम सत्य;

  यदि (निवेश_2.empty()) {
    *एतत् = VersionTuple(major, minor);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  यदि (!निवेश_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेश_2, subminor))
    प्रत्यागम सत्य;

  यदि (निवेश_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor);
    प्रत्यागम असत्य;
  }

  // If we're not done, parse the build version, \.[0-9]+
  यदि (!निवेश_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेश_2, build))
    प्रत्यागम सत्य;
  यदि (build >= 1024 * 1024)
    प्रत्यागम सत्य;

  यदि (निवेश_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor, build);
    प्रत्यागम असत्य;
  }

  // And the subbuild version, \.[0-9]+
  यदि (!निवेश_2.consume_front("."))
    प्रत्यागम सत्य;
  यदि (parseInt(निवेश_2, subbuild))
    प्रत्यागम सत्य;
  यदि (subbuild >= 1024)
    प्रत्यागम सत्य;

  // If we have characters left over, it's an error.
  यदि (!निवेश_2.empty())
    प्रत्यागम सत्य;

  *एतत् = VersionTuple(major, minor, subminor, build, subbuild);
  प्रत्यागम असत्य;
}

VersionTuple VersionTuple::withMajorReplaced(अचिन्हित NewMajor) ध्रुव {
  यदि (HasSubbuild)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  यदि (HasBuild)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor, Build);
  यदि (HasSubminor)
    प्रत्यागम VersionTuple(NewMajor, Minor, Subminor);
  यदि (HasMinor)
    प्रत्यागम VersionTuple(NewMajor, Minor);
  प्रत्यागम VersionTuple(NewMajor);
}
