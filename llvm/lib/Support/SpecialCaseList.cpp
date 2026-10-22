//===-- SpecialCaseList.cpp - special case list for sanitizers ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is a utility class for instrumentation passes (like AddressSanitizer
// or ThreadSanitizer) to avoid instrumenting some functions or global
// variables, or to instrument some functions or global variables in a specific
// way, based on a user-supplied list.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/SpecialCaseList.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/RadixTree.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/iterator_range.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/GlobPattern.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LineIterator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Regex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualFileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/WithColor.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಅನ್ತರ್ಭಾವ <utility>
#ಅನ್ತರ್ಭಾವ <variant>
#ಅನ್ತರ್ಭಾವ <vector>

ನಾಮಾಕಾಶ llvm {

ನಾಮಾಕಾಶ {

// Lagacy v1 matcher.
ವರ್ಗ RegexMatcher {
ಸಾರ್ವಜನಿಕ:
  Error ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ, ತಾರ್ಕಿಕ SlashAgnostic);
  ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ;
  StringRef ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ;

ಖಾಸಗಿ:
  ರಚನೆ Reg {
    Reg(StringRef Name, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No, Regex &&Rg)
        : Name(Name), ಸಾಲು_No(ಸಾಲು_No), Rg(std::move(Rg)) {}
    StringRef Name;
    ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No;
    Regex Rg;
  };

  std::ಸದಿಶ<Reg> RegExes;
};

ವರ್ಗ GlobMatcher {
ಸಾರ್ವಜನಿಕ:
  Error ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ, ತಾರ್ಕಿಕ SlashAgnostic);
  ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ;
  StringRef ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ;

ಖಾಸಗಿ:
  ರಚನೆ Glob {
    Glob(StringRef Name, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No, Glob_ನಮೂನೆ &&ನಮೂನೆ)
        : Name(Name), ಸಾಲು_No(ಸಾಲು_No), ನಮೂನೆ(std::move(ನಮೂನೆ)) {}
    StringRef Name;
    ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No;
    Glob_ನಮೂನೆ ನಮೂನೆ;
  };

  ಶೂನ್ಯ LazyInit() ಸ್ಥಿರಾಂಕ;

  std::ಸದಿಶ<GlobMatcher::Glob> Globs;

  ಬದಲಾಯಿಸಬಹುದಾದ Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_iterator>,
                    Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_ಹಿಮ್ಮುಖ_iterator>,
                              Small_ಸದಿಶ<ಪೂರ್ಣಾಂಕ, 1>>>
      ಪೂರ್ವಪ್ರತ್ಯಯ_Suffix_ಗಮ್ಯ_Glob;

  ಬದಲಾಯಿಸಬಹುದಾದ Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_iterator>,
                    Small_ಸದಿಶ<ಪೂರ್ಣಾಂಕ, 1>>
      Substr_ಗಮ್ಯ_Glob;

  ಬದಲಾಯಿಸಬಹುದಾದ ತಾರ್ಕಿಕ Initialized = ಸುಳ್ಳು;
};

ರಚನೆ QueryOptions {
  ತಾರ್ಕಿಕ UseGlobs = ನಿಜ;
  ತಾರ್ಕಿಕ RemoveDotSlash = ಸುಳ್ಳು;
  ತಾರ್ಕಿಕ Warn_Dot_Slash_ಹೊಂದಿಕೆ = ಸುಳ್ಳು;
  ತಾರ್ಕಿಕ SlashAgnostic = ಸುಳ್ಳು;
};

/// Represents a set of patterns and their line numbers
ವರ್ಗ Matcher {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ Matcher(QueryOptions QOpts);

  Error ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ);
  ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ;

  ತಾರ್ಕಿಕ ಹೊಂದಿಕೆ_Any(StringRef Query) ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ(Query); }

ಖಾಸಗಿ:
  ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ_ಆಂತರಿಕ(StringRef Query) ಸ್ಥಿರಾಂಕ;
  StringRef ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  ಬದಲಾಯಿಸಬಹುದಾದ std::once_flag Warned;
};

Error RegexMatcher::ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ,
                           ತಾರ್ಕಿಕ SlashAgnostic) {
  ಆದರೆ (ನಮೂನೆ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "Supplied regex was blank");

  // Replace * with .*
  ಸ್ವಯಂಚಾಲಿತ Regexp = ನಮೂನೆ.str();
  ಗಾಗಿ (size_t pos = 0; (pos = Regexp.ಕಂಡುಹಿಡಿ('*', pos)) != std::string::npos;
       pos += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(".*")) {
    Regexp.replace(pos, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ಆದರೆ (!CheckRE.isValid(REError))
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ, REError);

  RegExes.emplace_ಹಿಂಭಾಗ(ನಮೂನೆ, ಸಾಲು_ಸಂಖ್ಯೆ, std::move(CheckRE));
  ಹಿಂತಿರುಗಿ Error::success();
}

ಚಿಹ್ನೆರಹಿತ RegexMatcher::ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R : ಹಿಮ್ಮುಖ(RegExes))
    ಆದರೆ (R.Rg.ಹೊಂದಿಕೆ(Query))
      ಹಿಂತಿರುಗಿ R.ಸಾಲು_No;
  ಹಿಂತಿರುಗಿ 0;
}

StringRef RegexMatcher::ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R : RegExes)
    ಆದರೆ (R.ಸಾಲು_No == ಸಾಲು_No)
      ಹಿಂತಿರುಗಿ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ಹಿಂತಿರುಗಿ {};
}

Error GlobMatcher::ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ,
                          ತಾರ್ಕಿಕ SlashAgnostic) {
  ಆದರೆ (ನಮೂನೆ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ, "Supplied glob was blank");

  ಸ್ವಯಂಚಾಲಿತ Res =
      Glob_ನಮೂನೆ::create(ನಮೂನೆ, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = Res.takeError())
    ಹಿಂತಿರುಗಿ Err;
  Globs.emplace_ಹಿಂಭಾಗ(ನಮೂನೆ, ಸಾಲು_ಸಂಖ್ಯೆ, std::move(Res.get()));
  ಹಿಂತಿರುಗಿ Error::success();
}

ಶೂನ್ಯ GlobMatcher::LazyInit() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (LLVM_LIKELY(Initialized))
    ಹಿಂತಿರುಗಿ;
  Initialized = ನಿಜ;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[Idx, G] : enumerate(Globs)) {
    StringRef ಪೂರ್ವಪ್ರತ್ಯಯ = G.ನಮೂನೆ.ಪೂರ್ವಪ್ರತ್ಯಯ_2();
    StringRef Suffix = G.ನಮೂನೆ.suffix();

    ಆದರೆ (Suffix.ಖಾಲಿಗಳು() && ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.ನಮೂನೆ.longest_substr();
      ಆದರೆ (!Substr.ಖಾಲಿಗಳು()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        ಸ್ವಯಂಚಾಲಿತ &V = Substr_ಗಮ್ಯ_Glob.emplace(Substr).ಮೊದಲ_2->ಎರಡನೇ_2;
        V.emplace_ಹಿಂಭಾಗ(Idx);
        ಮುಂದುವರಿ;
      }
    }

    ಸ್ವಯಂಚಾಲಿತ &SToGlob = ಪೂರ್ವಪ್ರತ್ಯಯ_Suffix_ಗಮ್ಯ_Glob.emplace(ಪೂರ್ವಪ್ರತ್ಯಯ).ಮೊದಲ_2->ಎರಡನೇ_2;
    ಸ್ವಯಂಚಾಲಿತ &V = SToGlob.emplace(ಹಿಮ್ಮುಖ(Suffix)).ಮೊದಲ_2->ಎರಡನೇ_2;
    V.emplace_ಹಿಂಭಾಗ(Idx);
  }
}

ಚಿಹ್ನೆರಹಿತ GlobMatcher::ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ {
  LazyInit();

  ಪೂರ್ಣಾಂಕ ಉತ್ತಮ = -1;
  ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ_Suffix_ಗಮ್ಯ_Glob.ಖಾಲಿಗಳು()) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[_, SToGlob] : ಪೂರ್ವಪ್ರತ್ಯಯ_Suffix_ಗಮ್ಯ_Glob.ಕಂಡುಹಿಡಿ_prefixes(Query)) {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[_, V] : SToGlob.ಕಂಡುಹಿಡಿ_prefixes(ಹಿಮ್ಮುಖ(Query))) {
        ಗಾಗಿ (ಪೂರ್ಣಾಂಕ Idx : ಹಿಮ್ಮುಖ(V)) {
          ಆದರೆ (ಉತ್ತಮ > Idx)
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿರಾಂಕ GlobMatcher::Glob &G = Globs[Idx];
          ಆದರೆ (G.ನಮೂನೆ.ಹೊಂದಿಕೆ(Query)) {
            ಉತ್ತಮ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ನಿಲ್ಲಿಸು;
          }
        }
      }
    }
  }

  ಆದರೆ (!Substr_ಗಮ್ಯ_Glob.ಖಾಲಿಗಳು()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    ಗಾಗಿ (StringRef Q = Query; !Q.ಖಾಲಿಗಳು(); Q = Q.drop_front()) {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[_, V] : Substr_ಗಮ್ಯ_Glob.ಕಂಡುಹಿಡಿ_prefixes(Q)) {
        ಗಾಗಿ (ಪೂರ್ಣಾಂಕ Idx : ಹಿಮ್ಮುಖ(V)) {
          ಆದರೆ (ಉತ್ತಮ > Idx)
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿರಾಂಕ GlobMatcher::Glob &G = Globs[Idx];
          ಆದರೆ (G.ನಮೂನೆ.ಹೊಂದಿಕೆ(Query)) {
            ಉತ್ತಮ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ನಿಲ್ಲಿಸು;
          }
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ ಉತ್ತಮ < 0 ? 0 : Globs[ಉತ್ತಮ].ಸಾಲು_No;
}

StringRef GlobMatcher::ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &G : Globs)
    ಆದರೆ (G.ಸಾಲು_No == ಸಾಲು_No)
      ಹಿಂತಿರುಗಿ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ಹಿಂತಿರುಗಿ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ಆದರೆ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ಇಲ್ಲದಿದ್ದರೆ
    M.emplace<RegexMatcher>();
}

Error Matcher::ಸೇರಿಸು(StringRef ನಮೂನೆ, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಸಂಖ್ಯೆ) {
  ಹಿಂತಿರುಗಿ std::ಭೇಟಿ(
      [&](ಸ್ವಯಂಚಾಲಿತ &V) {
        ಹಿಂತಿರುಗಿ V.ಸೇರಿಸು(ನಮೂನೆ, ಸಾಲು_ಸಂಖ್ಯೆ, Options.SlashAgnostic);
      },
      M);
}

/// Matches Query against the patterns. The behavior is controlled by
/// `#!special-case-list` version.
//
// - Version 1 and 2: Path is matched as-is, regardless of presence of "./".
// - Version 3, 5 and higher: Paths with leading dot-slash are canonicalized
//   to paths without dot-slash before matching. This means that a rule
//   like `src=./foo` will never match, and `src=foo` will match both
//   `foo` and `./foo`. (Version 3 never became default but has this behavior).
// - Version 4: Transitionary version. Paths are matched both ways
//   (canonicalized and non-canonicalized) to maintain backward compatibility.
//   If a match only works with the old behavior (non-canonicalized), a warning
//   is emitted.
ಚಿಹ್ನೆರಹಿತ Matcher::ಹೊಂದಿಕೆ(StringRef Query) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!Options.RemoveDotSlash)
    ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ_ಆಂತರಿಕ(Query);

  ಆದರೆ (!Options.Warn_Dot_Slash_ಹೊಂದಿಕೆ)
    ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ_ಆಂತರಿಕ(llvm::sys::ಮಾರ್ಗ_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::ಮಾರ್ಗ_2::remove_leading_dotslash(Query);
  ಚಿಹ್ನೆರಹಿತ FixedMatched = ಹೊಂದಿಕೆ_ಆಂತರಿಕ(FixedQuery);
  ಆದರೆ (FixedQuery == Query)
    ಹಿಂತಿರುಗಿ FixedMatched;

  ಚಿಹ್ನೆರಹಿತ Original_ಹೊಂದಿಕೆ = ಹೊಂದಿಕೆ_ಆಂತರಿಕ(Query);
  ಆದರೆ (Original_ಹೊಂದಿಕೆ > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << ಕಂಡುಹಿಡಿ_ನಿಯಮ(Original_ಹೊಂದಿಕೆ) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  ಹಿಂತಿರುಗಿ std::max(Original_ಹೊಂದಿಕೆ, FixedMatched);
}

ಚಿಹ್ನೆರಹಿತ Matcher::ಹೊಂದಿಕೆ_ಆಂತರಿಕ(StringRef Query) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::ಭೇಟಿ([&](ಸ್ವಯಂಚಾಲಿತ &V) -> ಚಿಹ್ನೆರಹಿತ { ಹಿಂತಿರುಗಿ V.ಹೊಂದಿಕೆ(Query); }, M);
}

StringRef Matcher::ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::ಭೇಟಿ([&](ಸ್ವಯಂಚಾಲಿತ &V) -> StringRef { ಹಿಂತಿರುಗಿ V.ಕಂಡುಹಿಡಿ_ನಿಯಮ(ಸಾಲು_No); },
                    M);
}
} // namespace

ವರ್ಗ SpecialCaseList::Section::SectionImpl {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರಾಂಕ Matcher *ಕಂಡುಹಿಡಿ_Matcher(StringRef ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef Category) ಸ್ಥಿರಾಂಕ;

  ಬಳಸಿ SectionEntries = StringMap<StringMap<Matcher>>;

  ಸ್ಪಷ್ಟ SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ಸ್ಥಿರಾಂಕ std::ಸದಿಶ<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ಹೊಸ SpecialCaseList());
  ಆದರೆ (SCL->create_ಆಂತರಿಕ(Paths, FS, Error))
    ಹಿಂತಿರುಗಿ SCL;
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ಹೊಸ SpecialCaseList());
  ಆದರೆ (SCL->create_ಆಂತರಿಕ(MB, Error))
    ಹಿಂತಿರುಗಿ SCL;
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ಸ್ಥಿರಾಂಕ std::ಸದಿಶ<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ SCL = create(Paths, FS, Error))
    ಹಿಂತಿರುಗಿ SCL;
  report_fatal_error(Twine(Error));
}

ತಾರ್ಕಿಕ SpecialCaseList::create_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ std::ಸದಿಶ<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  ಗಾಗಿ (size_t i = 0; i < Paths.ಗಾತ್ರ_3(); ++i) {
    ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ಮಾರ್ಗ = Paths[i];
    ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> FileOrErr =
        VFS.get_ಬಫರ್_For_File(ಮಾರ್ಗ);
    ಆದರೆ (std::error_ಸಂಕೇತ EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + ಮಾರ್ಗ + "': " + EC.ಸಂದೇಶ_2()).str();
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    std::string ParseError;
    ಆದರೆ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + ಮಾರ್ಗ + "': " + ParseError).str();
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ SpecialCaseList::create_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ *MB,
                                     std::string &Error) {
  ಆದರೆ (!parse(0, MB, Error))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::ಕೂಡಿಸು_Section(StringRef SectionStr, ಚಿಹ್ನೆರಹಿತ FileNo,
                            ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No, ತಾರ್ಕಿಕ UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_ಹಿಂಭಾಗ(SectionStr, FileNo, UseGlobs);
  ಸ್ವಯಂಚಾಲಿತ &Section = Sections.ಹಿಂಭಾಗ();

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = Section.Impl->SectionMatcher.ಸೇರಿಸು(SectionStr, ಸಾಲು_No)) {
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "malformed section at line " + Twine(ಸಾಲು_No) +
                                 ": '" + SectionStr +
                                 "': " + ಗಮ್ಯ_String(std::move(Err)));
  }

  ಹಿಂತಿರುಗಿ &Section;
}

ತಾರ್ಕಿಕ SpecialCaseList::parse(ಚಿಹ್ನೆರಹಿತ FileIdx, ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ *MB,
                            std::string &Error) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಆವೃತ್ತಿ = 2;

  StringRef ಶೀರ್ಷಿಕೆ = MB->get_ಬಫರ್();
  ಆದರೆ (ಶೀರ್ಷಿಕೆ.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(ಶೀರ್ಷಿಕೆ, 10, ಆವೃತ್ತಿ);

  ಸ್ವಯಂಚಾಲಿತ Min_ಆವೃತ್ತಿ = [&](ಚಿಹ್ನೆರಹಿತ V) { ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  ತಾರ್ಕಿಕ UseGlobs = Min_ಆವೃತ್ತಿ(2);
  ತಾರ್ಕಿಕ RemoveDotSlash = Min_ಆವೃತ್ತಿ(3);
  ತಾರ್ಕಿಕ WarnDotSlash = Min_ಆವೃತ್ತಿ(4) && !Min_ಆವೃತ್ತಿ(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  ತಾರ್ಕಿಕ SlashAgnostic = Min_ಆವೃತ್ತಿ(4) && llvm::sys::ಮಾರ್ಗ_2::is_style_windows(
                                            llvm::sys::ಮಾರ್ಗ_2::Style::native);

  ಸ್ವಯಂಚಾಲಿತ ErrOrSection = ಕೂಡಿಸು_Section("*", FileIdx, 1, ನಿಜ);
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = ErrOrSection.takeError()) {
    Error = ಗಮ್ಯ_String(std::move(Err));
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef ಮಾರ್ಗ_Prefixes[] = {"src", "!src", "mainfile", "source"};

  ಗಾಗಿ (ಸಾಲು_iterator ಸಾಲು_It(*MB, /*SkipBlanks=*/ನಿಜ, /*CommentMarker=*/'#');
       !ಸಾಲು_It.is_at_eof(); ಸಾಲು_It++) {
    ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No = ಸಾಲು_It.ಸಾಲು_ಸಂಖ್ಯೆ_2();
    StringRef ಸಾಲು = ಸಾಲು_It->trim();
    ಆದರೆ (ಸಾಲು.ಖಾಲಿಗಳು())
      ಮುಂದುವರಿ;

    // Save section names
    ಆದರೆ (ಸಾಲು.starts_with("[")) {
      ಆದರೆ (!ಸಾಲು.ತುದಿಗಳು_with("]")) {
        Error =
            ("malformed section header on line " + Twine(ಸಾಲು_No) + ": " + ಸಾಲು)
                .str();
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }

      ಸ್ವಯಂಚಾಲಿತ ErrOrSection =
          ಕೂಡಿಸು_Section(ಸಾಲು.drop_front().drop_ಹಿಂಭಾಗ(), FileIdx, ಸಾಲು_No, UseGlobs);
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = ErrOrSection.takeError()) {
        Error = ಗಮ್ಯ_String(std::move(Err));
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ಮುಂದುವರಿ;
    }

    // Get our prefix and unparsed glob.
    ಸ್ವಯಂಚಾಲಿತ [ಪೂರ್ವಪ್ರತ್ಯಯ, Postfix] = ಸಾಲು.ವಿಭಜಿಸು(":");
    ಆದರೆ (Postfix.ಖಾಲಿಗಳು()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(ಸಾಲು_No) + ": '" + ಸಾಲು + "'").str();
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ಆದರೆ (llvm::is_contained(ಮಾರ್ಗ_Prefixes, ಪೂರ್ವಪ್ರತ್ಯಯ)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_ಹೊಂದಿಕೆ = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    ಸ್ವಯಂಚಾಲಿತ [ನಮೂನೆ, Category] = Postfix.ವಿಭಜಿಸು("=");
    ಸ್ವಯಂಚಾಲಿತ [It, _] = CurrentImpl->Entries[ಪೂರ್ವಪ್ರತ್ಯಯ].try_emplace(Category, QOpts);
    ನಮೂನೆ = ನಮೂನೆ.copy(StrAlloc);
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = It->ಎರಡನೇ_2.ಸೇರಿಸು(ನಮೂನೆ, ಸಾಲು_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(ಸಾಲು_No) + ": '" + ನಮೂನೆ + "': " + ಗಮ್ಯ_String(std::move(Err)))
              .str();
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  }

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

SpecialCaseList::~SpecialCaseList() = ಪೂರ್ವನಿಯೋಜಿತ;

ತಾರ್ಕಿಕ SpecialCaseList::ಒಳ_Section(StringRef Section, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                                StringRef Query, StringRef Category) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ [FileIdx, ಸಾಲು_No] = ಒಳ_Section_Blame(Section, ಪೂರ್ವಪ್ರತ್ಯಯ, Query, Category);
  ಹಿಂತಿರುಗಿ ಸಾಲು_No;
}

std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>
SpecialCaseList::ಒಳ_Section_Blame(StringRef Section, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                                StringRef Query, StringRef Category) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &S : ಹಿಮ್ಮುಖ(Sections)) {
    ಆದರೆ (S.Impl->SectionMatcher.ಹೊಂದಿಕೆ_Any(Section)) {
      ಚಿಹ್ನೆರಹಿತ Blame = S.get_ಕೊನೆಯ_ಹೊಂದಿಕೆ(ಪೂರ್ವಪ್ರತ್ಯಯ, Query, Category);
      ಆದರೆ (Blame)
        ಹಿಂತಿರುಗಿ {S.FileIdx, Blame};
    }
  }
  ಹಿಂತಿರುಗಿ Not_ಸಿಕ್ಕಿತು;
}

SpecialCaseList::Section::Section(StringRef Str, ಚಿಹ್ನೆರಹಿತ FileIdx,
                                  ತಾರ್ಕಿಕ UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/ಸುಳ್ಳು})) {}

SpecialCaseList::Section::Section(Section &&) = ಪೂರ್ವನಿಯೋಜಿತ;

SpecialCaseList::Section::~Section() = ಪೂರ್ವನಿಯೋಜಿತ;

ತಾರ್ಕಿಕ SpecialCaseList::Section::ಹೊಂದಿಕೆ_Name(StringRef Name) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Impl->SectionMatcher.ಹೊಂದಿಕೆ_Any(Name);
}

ಸ್ಥಿರಾಂಕ Matcher *
SpecialCaseList::Section::SectionImpl::ಕಂಡುಹಿಡಿ_Matcher(StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                                                   StringRef Category) ಸ್ಥಿರಾಂಕ {
  SectionEntries::const_iterator I = Entries.ಕಂಡುಹಿಡಿ(ಪೂರ್ವಪ್ರತ್ಯಯ);
  ಆದರೆ (I == Entries.ಕೊನೆ())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  StringMap<Matcher>::const_iterator II = I->ಎರಡನೇ_2.ಕಂಡುಹಿಡಿ(Category);
  ಆದರೆ (II == I->ಎರಡನೇ_2.ಕೊನೆ())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಹಿಂತಿರುಗಿ &II->ಎರಡನೇ_2;
}

ಚಿಹ್ನೆರಹಿತ SpecialCaseList::Section::get_ಕೊನೆಯ_ಹೊಂದಿಕೆ(StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                                                StringRef Query,
                                                StringRef Category) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ಥಿರಾಂಕ Matcher *M = Impl->ಕಂಡುಹಿಡಿ_Matcher(ಪೂರ್ವಪ್ರತ್ಯಯ, Category))
    ಹಿಂತಿರುಗಿ M->ಹೊಂದಿಕೆ(Query);
  ಹಿಂತಿರುಗಿ 0;
}

ತಾರ್ಕಿಕ SpecialCaseList::Section::has_ಪೂರ್ವಪ್ರತ್ಯಯ(StringRef ಪೂರ್ವಪ್ರತ್ಯಯ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Impl->Entries.ಒಳಗೊಂಡಿದೆ(ಪೂರ್ವಪ್ರತ್ಯಯ);
}

} // namespace llvm
