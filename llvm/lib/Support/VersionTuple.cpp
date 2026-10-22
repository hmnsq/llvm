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

#అన్తర్భావ "llvm/Support/VersionTuple.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cassert>

ఉపయోగించి నామాకాశ llvm;

std::string సంస్కరణ_Tuple::getAsString() స్థిరరాశి {
  std::string ఫలితం;
  {
    llvm::raw_string_ostream బహిర్గతం(ఫలితం);
    బహిర్గతం << *ఇది;
  }
  తిరిగి ఫలితం;
}

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &బహిర్గతం, స్థిరరాశి సంస్కరణ_Tuple &V) {
  బహిర్గతం << V.getMajor();
  ఒకవేళ (std::optional<గుర్తులేని> Minor = V.getMinor())
    బహిర్గతం << '.' << *Minor;
  ఒకవేళ (std::optional<గుర్తులేని> Subminor = V.getSubminor())
    బహిర్గతం << '.' << *Subminor;
  ఒకవేళ (std::optional<గుర్తులేని> Build = V.getBuild())
    బహిర్గతం << '.' << *Build;
  ఒకవేళ (std::optional<గుర్తులేని> Subbuild = V.getSubbuild())
    బహిర్గతం << '.' << *Subbuild;
  తిరిగి బహిర్గతం;
}

స్థిర తార్కిక parseInt(StringRef &ఆదానం_2, గుర్తులేని &విలువ_2) {
  ప్రతిజ్ఞా(విలువ_2 == 0);
  ఒకవేళ (ఆదానం_2.ఖాళీలు())
    తిరిగి నిజం;

  అక్షరం తదుపరి = ఆదానం_2[0];
  ఆదానం_2 = ఆదానం_2.substr(1);
  ఒకవేళ (తదుపరి < '0' || తదుపరి > '9')
    తిరిగి నిజం;
  విలువ_2 = (గుర్తులేని)(తదుపరి - '0');

  అయితే (!ఆదానం_2.ఖాళీలు()) {
    తదుపరి = ఆదానం_2[0];
    ఒకవేళ (తదుపరి < '0' || తదుపరి > '9')
      తిరిగి అబద్ధం;
    ఆదానం_2 = ఆదానం_2.substr(1);
    విలువ_2 = విలువ_2 * 10 + (గుర్తులేని)(తదుపరి - '0');
  }

  తిరిగి అబద్ధం;
}

తార్కిక సంస్కరణ_Tuple::tryParse(StringRef ఆదానం_2) {
  గుర్తులేని major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ఒకవేళ (parseInt(ఆదానం_2, major))
    తిరిగి నిజం;

  ఒకవేళ (ఆదానం_2.ఖాళీలు()) {
    *ఇది = సంస్కరణ_Tuple(major);
    తిరిగి అబద్ధం;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ఒకవేళ (ఆదానం_2[0] != '.')
    తిరిగి నిజం;
  ఆదానం_2 = ఆదానం_2.substr(1);
  ఒకవేళ (parseInt(ఆదానం_2, minor))
    తిరిగి నిజం;

  ఒకవేళ (ఆదానం_2.ఖాళీలు()) {
    *ఇది = సంస్కరణ_Tuple(major, minor);
    తిరిగి అబద్ధం;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ఒకవేళ (!ఆదానం_2.consume_front("."))
    తిరిగి నిజం;
  ఒకవేళ (parseInt(ఆదానం_2, subminor))
    తిరిగి నిజం;

  ఒకవేళ (ఆదానం_2.ఖాళీలు()) {
    *ఇది = సంస్కరణ_Tuple(major, minor, subminor);
    తిరిగి అబద్ధం;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ఒకవేళ (!ఆదానం_2.consume_front("."))
    తిరిగి నిజం;
  ఒకవేళ (parseInt(ఆదానం_2, build))
    తిరిగి నిజం;
  ఒకవేళ (build >= 1024 * 1024)
    తిరిగి నిజం;

  ఒకవేళ (ఆదానం_2.ఖాళీలు()) {
    *ఇది = సంస్కరణ_Tuple(major, minor, subminor, build);
    తిరిగి అబద్ధం;
  }

  // And the subbuild version, \.[0-9]+
  ఒకవేళ (!ఆదానం_2.consume_front("."))
    తిరిగి నిజం;
  ఒకవేళ (parseInt(ఆదానం_2, subbuild))
    తిరిగి నిజం;
  ఒకవేళ (subbuild >= 1024)
    తిరిగి నిజం;

  // If we have characters left over, it's an error.
  ఒకవేళ (!ఆదానం_2.ఖాళీలు())
    తిరిగి నిజం;

  *ఇది = సంస్కరణ_Tuple(major, minor, subminor, build, subbuild);
  తిరిగి అబద్ధం;
}

సంస్కరణ_Tuple సంస్కరణ_Tuple::withMajorReplaced(గుర్తులేని NewMajor) స్థిరరాశి {
  ఒకవేళ (HasSubbuild)
    తిరిగి సంస్కరణ_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ఒకవేళ (HasBuild)
    తిరిగి సంస్కరణ_Tuple(NewMajor, Minor, Subminor, Build);
  ఒకవేళ (HasSubminor)
    తిరిగి సంస్కరణ_Tuple(NewMajor, Minor, Subminor);
  ఒకవేళ (HasMinor)
    తిరిగి సంస్కరణ_Tuple(NewMajor, Minor);
  తిరిగి సంస్కరణ_Tuple(NewMajor);
}
