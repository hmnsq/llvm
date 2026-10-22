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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VersionTuple.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

std::string ਸੰਸਕਰਣ_Tuple::getAsString() ਅਚਲ {
  std::string ਨਤੀਜਾ;
  {
    llvm::raw_string_ostream ਬਾਹਰਲਾ(ਨਤੀਜਾ);
    ਬਾਹਰਲਾ << *ਇਹ;
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &ਬਾਹਰਲਾ, ਅਚਲ ਸੰਸਕਰਣ_Tuple &V) {
  ਬਾਹਰਲਾ << V.getMajor();
  ਜੇ (std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> Minor = V.getMinor())
    ਬਾਹਰਲਾ << '.' << *Minor;
  ਜੇ (std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> Subminor = V.getSubminor())
    ਬਾਹਰਲਾ << '.' << *Subminor;
  ਜੇ (std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> Build = V.getBuild())
    ਬਾਹਰਲਾ << '.' << *Build;
  ਜੇ (std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> Subbuild = V.getSubbuild())
    ਬਾਹਰਲਾ << '.' << *Subbuild;
  ਵਾਪਸ ਬਾਹਰਲਾ;
}

ਸਥਿਰ ਤਾਰਕਿਕ parseInt(StringRef &ਨਿਵੇਸ਼_2, ਬਿਨਾਨਿਸ਼ਾਨੀ &ਮੁੱਲ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮੁੱਲ_2 == 0);
  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;

  ਅੱਖਰ ਅਗਲਾ = ਨਿਵੇਸ਼_2[0];
  ਨਿਵੇਸ਼_2 = ਨਿਵੇਸ਼_2.substr(1);
  ਜੇ (ਅਗਲਾ < '0' || ਅਗਲਾ > '9')
    ਵਾਪਸ ਸੱਚ;
  ਮੁੱਲ_2 = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਅਗਲਾ - '0');

  ਜਦੋਂ (!ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਅਗਲਾ = ਨਿਵੇਸ਼_2[0];
    ਜੇ (ਅਗਲਾ < '0' || ਅਗਲਾ > '9')
      ਵਾਪਸ ਝੂਠ;
    ਨਿਵੇਸ਼_2 = ਨਿਵੇਸ਼_2.substr(1);
    ਮੁੱਲ_2 = ਮੁੱਲ_2 * 10 + (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਅਗਲਾ - '0');
  }

  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ ਸੰਸਕਰਣ_Tuple::tryParse(StringRef ਨਿਵੇਸ਼_2) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ਜੇ (parseInt(ਨਿਵੇਸ਼_2, major))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    *ਇਹ = ਸੰਸਕਰਣ_Tuple(major);
    ਵਾਪਸ ਝੂਠ;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ਜੇ (ਨਿਵੇਸ਼_2[0] != '.')
    ਵਾਪਸ ਸੱਚ;
  ਨਿਵੇਸ਼_2 = ਨਿਵੇਸ਼_2.substr(1);
  ਜੇ (parseInt(ਨਿਵੇਸ਼_2, minor))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    *ਇਹ = ਸੰਸਕਰਣ_Tuple(major, minor);
    ਵਾਪਸ ਝੂਠ;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ਜੇ (!ਨਿਵੇਸ਼_2.consume_front("."))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (parseInt(ਨਿਵੇਸ਼_2, subminor))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    *ਇਹ = ਸੰਸਕਰਣ_Tuple(major, minor, subminor);
    ਵਾਪਸ ਝੂਠ;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ਜੇ (!ਨਿਵੇਸ਼_2.consume_front("."))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (parseInt(ਨਿਵੇਸ਼_2, build))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (build >= 1024 * 1024)
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    *ਇਹ = ਸੰਸਕਰਣ_Tuple(major, minor, subminor, build);
    ਵਾਪਸ ਝੂਠ;
  }

  // And the subbuild version, \.[0-9]+
  ਜੇ (!ਨਿਵੇਸ਼_2.consume_front("."))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (parseInt(ਨਿਵੇਸ਼_2, subbuild))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (subbuild >= 1024)
    ਵਾਪਸ ਸੱਚ;

  // If we have characters left over, it's an error.
  ਜੇ (!ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;

  *ਇਹ = ਸੰਸਕਰਣ_Tuple(major, minor, subminor, build, subbuild);
  ਵਾਪਸ ਝੂਠ;
}

ਸੰਸਕਰਣ_Tuple ਸੰਸਕਰਣ_Tuple::withMajorReplaced(ਬਿਨਾਨਿਸ਼ਾਨੀ NewMajor) ਅਚਲ {
  ਜੇ (HasSubbuild)
    ਵਾਪਸ ਸੰਸਕਰਣ_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ਜੇ (HasBuild)
    ਵਾਪਸ ਸੰਸਕਰਣ_Tuple(NewMajor, Minor, Subminor, Build);
  ਜੇ (HasSubminor)
    ਵਾਪਸ ਸੰਸਕਰਣ_Tuple(NewMajor, Minor, Subminor);
  ਜੇ (HasMinor)
    ਵਾਪਸ ਸੰਸਕਰਣ_Tuple(NewMajor, Minor);
  ਵਾਪਸ ਸੰਸਕਰਣ_Tuple(NewMajor);
}
