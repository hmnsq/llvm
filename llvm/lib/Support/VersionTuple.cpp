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

#அந்தர்பாவ "llvm/Support/VersionTuple.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>

using நாமாகாஶ llvm;

std::string VersionTuple::getAsString() மாறிலி {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *இது;
  }
  திரும்பு Result;
}

raw_ostream &llvm::operator<<(raw_ostream &Out, மாறிலி VersionTuple &V) {
  Out << V.getMajor();
  எனில் (std::optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  எனில் (std::optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  எனில் (std::optional<unsigned> Build = V.getBuild())
    Out << '.' << *Build;
  எனில் (std::optional<unsigned> Subbuild = V.getSubbuild())
    Out << '.' << *Subbuild;
  திரும்பு Out;
}

நிலையான தருக்க parseInt(StringRef &உள்ளீடு_2, unsigned &மதிப்பு_2) {
  ப்ரதிஜ்ஞா(மதிப்பு_2 == 0);
  எனில் (உள்ளீடு_2.empty())
    திரும்பு உண்மை;

  எழுத்து next = உள்ளீடு_2[0];
  உள்ளீடு_2 = உள்ளீடு_2.substr(1);
  எனில் (next < '0' || next > '9')
    திரும்பு உண்மை;
  மதிப்பு_2 = (unsigned)(next - '0');

  போது (!உள்ளீடு_2.empty()) {
    next = உள்ளீடு_2[0];
    எனில் (next < '0' || next > '9')
      திரும்பு பொய்;
    உள்ளீடு_2 = உள்ளீடு_2.substr(1);
    மதிப்பு_2 = மதிப்பு_2 * 10 + (unsigned)(next - '0');
  }

  திரும்பு பொய்;
}

தருக்க VersionTuple::tryParse(StringRef உள்ளீடு_2) {
  unsigned major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  எனில் (parseInt(உள்ளீடு_2, major))
    திரும்பு உண்மை;

  எனில் (உள்ளீடு_2.empty()) {
    *இது = VersionTuple(major);
    திரும்பு பொய்;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  எனில் (உள்ளீடு_2[0] != '.')
    திரும்பு உண்மை;
  உள்ளீடு_2 = உள்ளீடு_2.substr(1);
  எனில் (parseInt(உள்ளீடு_2, minor))
    திரும்பு உண்மை;

  எனில் (உள்ளீடு_2.empty()) {
    *இது = VersionTuple(major, minor);
    திரும்பு பொய்;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  எனில் (!உள்ளீடு_2.consume_front("."))
    திரும்பு உண்மை;
  எனில் (parseInt(உள்ளீடு_2, subminor))
    திரும்பு உண்மை;

  எனில் (உள்ளீடு_2.empty()) {
    *இது = VersionTuple(major, minor, subminor);
    திரும்பு பொய்;
  }

  // If we're not done, parse the build version, \.[0-9]+
  எனில் (!உள்ளீடு_2.consume_front("."))
    திரும்பு உண்மை;
  எனில் (parseInt(உள்ளீடு_2, build))
    திரும்பு உண்மை;
  எனில் (build >= 1024 * 1024)
    திரும்பு உண்மை;

  எனில் (உள்ளீடு_2.empty()) {
    *இது = VersionTuple(major, minor, subminor, build);
    திரும்பு பொய்;
  }

  // And the subbuild version, \.[0-9]+
  எனில் (!உள்ளீடு_2.consume_front("."))
    திரும்பு உண்மை;
  எனில் (parseInt(உள்ளீடு_2, subbuild))
    திரும்பு உண்மை;
  எனில் (subbuild >= 1024)
    திரும்பு உண்மை;

  // If we have characters left over, it's an error.
  எனில் (!உள்ளீடு_2.empty())
    திரும்பு உண்மை;

  *இது = VersionTuple(major, minor, subminor, build, subbuild);
  திரும்பு பொய்;
}

VersionTuple VersionTuple::withMajorReplaced(unsigned NewMajor) மாறிலி {
  எனில் (HasSubbuild)
    திரும்பு VersionTuple(NewMajor, Minor, Subminor, Build, Subbuild);
  எனில் (HasBuild)
    திரும்பு VersionTuple(NewMajor, Minor, Subminor, Build);
  எனில் (HasSubminor)
    திரும்பு VersionTuple(NewMajor, Minor, Subminor);
  எனில் (HasMinor)
    திரும்பு VersionTuple(NewMajor, Minor);
  திரும்பு VersionTuple(NewMajor);
}
