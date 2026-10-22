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

std::string VersionTuple::getAsString() अचर {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *एतत्;
  }
  फर्काऊ Result;
}

raw_ostream &llvm::चालक<<(raw_ostream &Out, अचर VersionTuple &V) {
  Out << V.getMajor();
  यदि (std::optional<अचिन्हित> Minor = V.getMinor())
    Out << '.' << *Minor;
  यदि (std::optional<अचिन्हित> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  यदि (std::optional<अचिन्हित> Build = V.getBuild())
    Out << '.' << *Build;
  यदि (std::optional<अचिन्हित> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  फर्काऊ Out;
}

स्थिर तार्किक parseInt(StringRef &आगत_2, अचिन्हित &मान_2) {
  प्रतिज्ञा(मान_2 == 0);
  यदि (आगत_2.empty())
    फर्काऊ सत्य;

  वर्ण next = आगत_2[0];
  आगत_2 = आगत_2.substr(1);
  यदि (next < '0' || next > '9')
    फर्काऊ सत्य;
  मान_2 = (अचिन्हित)(next - '0');

  जबसम्म (!आगत_2.empty()) {
    next = आगत_2[0];
    यदि (next < '0' || next > '9')
      फर्काऊ असत्य;
    आगत_2 = आगत_2.substr(1);
    मान_2 = मान_2 * 10 + (अचिन्हित)(next - '0');
  }

  फर्काऊ असत्य;
}

तार्किक VersionTuple::tryParse(StringRef आगत_2) {
  अचिन्हित major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  यदि (parseInt(आगत_2, major))
    फर्काऊ सत्य;

  यदि (आगत_2.empty()) {
    *एतत् = VersionTuple(major);
    फर्काऊ असत्य;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  यदि (आगत_2[0] != '.')
    फर्काऊ सत्य;
  आगत_2 = आगत_2.substr(1);
  यदि (parseInt(आगत_2, minor))
    फर्काऊ सत्य;

  यदि (आगत_2.empty()) {
    *एतत् = VersionTuple(major, minor);
    फर्काऊ असत्य;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  यदि (!आगत_2.consume_front("."))
    फर्काऊ सत्य;
  यदि (parseInt(आगत_2, subminor))
    फर्काऊ सत्य;

  यदि (आगत_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor);
    फर्काऊ असत्य;
  }

  // If we're not done, parse the build version, \.[0-9]+
  यदि (!आगत_2.consume_front("."))
    फर्काऊ सत्य;
  यदि (parseInt(आगत_2, build))
    फर्काऊ सत्य;
  यदि (build >= 1024 * 1024)
    फर्काऊ सत्य;

  यदि (आगत_2.empty()) {
    *एतत् = VersionTuple(major, minor, subminor, build);
    फर्काऊ असत्य;
  }

  // And the subbuild version, \.[0-9]+
  यदि (!आगत_2.consume_front("."))
    फर्काऊ सत्य;
  यदि (parseInt(आगत_2, subbuild))
    फर्काऊ सत्य;
  यदि (subbuild >= 1024)
    फर्काऊ सत्य;

  // If we have characters left over, it's an error.
  यदि (!आगत_2.empty())
    फर्काऊ सत्य;

  *एतत् = VersionTuple(major, minor, subminor, build, subbuild);
  फर्काऊ असत्य;
}

VersionTuple VersionTuple::withMajorReplaced(अचिन्हित NewMajor) अचर {
  यदि (HasSubbuild)
    फर्काऊ VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  यदि (HasBuild)
    फर्काऊ VersionTuple(NewMajor, Minor, Subminor, Build);
  यदि (HasSubminor)
    फर्काऊ VersionTuple(NewMajor, Minor, Subminor);
  यदि (HasMinor)
    फर्काऊ VersionTuple(NewMajor, Minor);
  फर्काऊ VersionTuple(NewMajor);
}
