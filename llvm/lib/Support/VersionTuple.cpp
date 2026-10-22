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

#ಅನ್ತರ್ಭಾವ "llvm/Support/VersionTuple.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

std::string ಆವೃತ್ತಿ_Tuple::getAsString() ಸ್ಥಿರಾಂಕ {
  std::string ಫಲಿತಾಂಶ;
  {
    llvm::raw_string_ostream ಹೊರಹೋಗು(ಫಲಿತಾಂಶ);
    ಹೊರಹೋಗು << *ಇದು;
  }
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ ಆವೃತ್ತಿ_Tuple &V) {
  ಹೊರಹೋಗು << V.getMajor();
  ಆದರೆ (std::optional<ಚಿಹ್ನೆರಹಿತ> Minor = V.getMinor())
    ಹೊರಹೋಗು << '.' << *Minor;
  ಆದರೆ (std::optional<ಚಿಹ್ನೆರಹಿತ> Subminor = V.getSubminor())
    ಹೊರಹೋಗು << '.' << *Subminor;
  ಆದರೆ (std::optional<ಚಿಹ್ನೆರಹಿತ> Build = V.getBuild())
    ಹೊರಹೋಗು << '.' << *Build;
  ಆದರೆ (std::optional<ಚಿಹ್ನೆರಹಿತ> Subbuild = V.getSubbuild())
    ಹೊರಹೋಗು << '.' << *Subbuild;
  ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ parseInt(StringRef &ಒಳಹರಿವು_2, ಚಿಹ್ನೆರಹಿತ &ಮೌಲ್ಯ_2) {
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ_2 == 0);
  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಅಕ್ಷರ ಮುಂದಿನ = ಒಳಹರಿವು_2[0];
  ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
  ಆದರೆ (ಮುಂದಿನ < '0' || ಮುಂದಿನ > '9')
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಮೌಲ್ಯ_2 = (ಚಿಹ್ನೆರಹಿತ)(ಮುಂದಿನ - '0');

  ಆಗ (!ಒಳಹರಿವು_2.ಖಾಲಿಗಳು()) {
    ಮುಂದಿನ = ಒಳಹರಿವು_2[0];
    ಆದರೆ (ಮುಂದಿನ < '0' || ಮುಂದಿನ > '9')
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
    ಮೌಲ್ಯ_2 = ಮೌಲ್ಯ_2 * 10 + (ಚಿಹ್ನೆರಹಿತ)(ಮುಂದಿನ - '0');
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ ಆವೃತ್ತಿ_Tuple::tryParse(StringRef ಒಳಹರಿವು_2) {
  ಚಿಹ್ನೆರಹಿತ major = 0, minor = 0, subminor = 0, build = 0, subbuild = 0;

  // Parse the major version, [0-9]+
  ಆದರೆ (parseInt(ಒಳಹರಿವು_2, major))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು()) {
    *ಇದು = ಆವೃತ್ತಿ_Tuple(major);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  ಆದರೆ (ಒಳಹರಿವು_2[0] != '.')
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಒಳಹರಿವು_2 = ಒಳಹರಿವು_2.substr(1);
  ಆದರೆ (parseInt(ಒಳಹರಿವು_2, minor))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು()) {
    *ಇದು = ಆವೃತ್ತಿ_Tuple(major, minor);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // If we're not done, parse the subminor version, \.[0-9]+
  ಆದರೆ (!ಒಳಹರಿವು_2.consume_front("."))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (parseInt(ಒಳಹರಿವು_2, subminor))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು()) {
    *ಇದು = ಆವೃತ್ತಿ_Tuple(major, minor, subminor);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // If we're not done, parse the build version, \.[0-9]+
  ಆದರೆ (!ಒಳಹರಿವು_2.consume_front("."))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (parseInt(ಒಳಹರಿವು_2, build))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (build >= 1024 * 1024)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು()) {
    *ಇದು = ಆವೃತ್ತಿ_Tuple(major, minor, subminor, build);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // And the subbuild version, \.[0-9]+
  ಆದರೆ (!ಒಳಹರಿವು_2.consume_front("."))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (parseInt(ಒಳಹರಿವು_2, subbuild))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (subbuild >= 1024)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // If we have characters left over, it's an error.
  ಆದರೆ (!ಒಳಹರಿವು_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  *ಇದು = ಆವೃತ್ತಿ_Tuple(major, minor, subminor, build, subbuild);
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಆವೃತ್ತಿ_Tuple ಆವೃತ್ತಿ_Tuple::withMajorReplaced(ಚಿಹ್ನೆರಹಿತ NewMajor) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (HasSubbuild)
    ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_Tuple(NewMajor, Minor, Subminor, Build, Subbuild);
  ಆದರೆ (HasBuild)
    ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_Tuple(NewMajor, Minor, Subminor, Build);
  ಆದರೆ (HasSubminor)
    ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_Tuple(NewMajor, Minor, Subminor);
  ಆದರೆ (HasMinor)
    ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_Tuple(NewMajor, Minor);
  ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_Tuple(NewMajor);
}
