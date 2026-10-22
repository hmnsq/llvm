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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SpecialCaseList.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/RadixTree.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/iterator_range.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/GlobPattern.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LineIterator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Regex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualFileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/WithColor.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>
#ਅਨ੍ਤਰ੍ਭਾਵ <utility>
#ਅਨ੍ਤਰ੍ਭਾਵ <variant>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਨਾਮਾਕਾਸ਼ llvm {

ਨਾਮਾਕਾਸ਼ {

// Lagacy v1 matcher.
ਵਰਗ RegexMatcher {
ਜਨਤਕ:
  Error ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ, ਤਾਰਕਿਕ SlashAgnostic);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ(StringRef Query) ਅਚਲ;
  StringRef ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ;

ਨਿਜੀ:
  ਬਣਤਰ Reg {
    Reg(StringRef Name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No, Regex &&Rg)
        : Name(Name), ਸਤਰ_No(ਸਤਰ_No), Rg(std::move(Rg)) {}
    StringRef Name;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No;
    Regex Rg;
  };

  std::ਸਦਿਸ਼<Reg> RegExes;
};

ਵਰਗ GlobMatcher {
ਜਨਤਕ:
  Error ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ, ਤਾਰਕਿਕ SlashAgnostic);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ(StringRef Query) ਅਚਲ;
  StringRef ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ;

ਨਿਜੀ:
  ਬਣਤਰ Glob {
    Glob(StringRef Name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No, Glob_ਨਮੂਨਾ &&ਨਮੂਨਾ)
        : Name(Name), ਸਤਰ_No(ਸਤਰ_No), ਨਮੂਨਾ(std::move(ਨਮੂਨਾ)) {}
    StringRef Name;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No;
    Glob_ਨਮੂਨਾ ਨਮੂਨਾ;
  };

  ਖਾਲੀ LazyInit() ਅਚਲ;

  std::ਸਦਿਸ਼<GlobMatcher::Glob> Globs;

  ਬਦਲਣਯੋਗ Radix_ਰੁੱਖ<iterator_range<StringRef::const_iterator>,
                    Radix_ਰੁੱਖ<iterator_range<StringRef::const_ਉਲਟ_iterator>,
                              Small_ਸਦਿਸ਼<ਪੂਰਨ, 1>>>
      ਅਗੇਤਰ_Suffix_ਮੰਜ਼ਿਲ_Glob;

  ਬਦਲਣਯੋਗ Radix_ਰੁੱਖ<iterator_range<StringRef::const_iterator>,
                    Small_ਸਦਿਸ਼<ਪੂਰਨ, 1>>
      Substr_ਮੰਜ਼ਿਲ_Glob;

  ਬਦਲਣਯੋਗ ਤਾਰਕਿਕ Initialized = ਝੂਠ;
};

ਬਣਤਰ QueryOptions {
  ਤਾਰਕਿਕ UseGlobs = ਸੱਚ;
  ਤਾਰਕਿਕ RemoveDotSlash = ਝੂਠ;
  ਤਾਰਕਿਕ Warn_Dot_Slash_ਮੇਲ = ਝੂਠ;
  ਤਾਰਕਿਕ SlashAgnostic = ਝੂਠ;
};

/// Represents a set of patterns and their line numbers
ਵਰਗ Matcher {
ਜਨਤਕ:
  ਸਪਸ਼ਟ Matcher(QueryOptions QOpts);

  Error ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ(StringRef Query) ਅਚਲ;

  ਤਾਰਕਿਕ ਮੇਲ_Any(StringRef Query) ਅਚਲ { ਵਾਪਸ ਮੇਲ(Query); }

ਨਿਜੀ:
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ_ਅੰਦਰੂਨੀ(StringRef Query) ਅਚਲ;
  StringRef ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  ਬਦਲਣਯੋਗ std::once_flag Warned;
};

Error RegexMatcher::ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ,
                           ਤਾਰਕਿਕ SlashAgnostic) {
  ਜੇ (ਨਮੂਨਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "Supplied regex was blank");

  // Replace * with .*
  ਸਵੈਚਾਲਿਤ Regexp = ਨਮੂਨਾ.str();
  ਲਈ (size_t pos = 0; (pos = Regexp.ਲੱਭੋ('*', pos)) != std::string::npos;
       pos += ਲੜੀ_ਲੰਬਾਈ(".*")) {
    Regexp.replace(pos, ਲੜੀ_ਲੰਬਾਈ("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ਜੇ (!CheckRE.isValid(REError))
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ, REError);

  RegExes.emplace_ਪਿੱਠ(ਨਮੂਨਾ, ਸਤਰ_ਸੰਖਿਆ, std::move(CheckRE));
  ਵਾਪਸ Error::success();
}

ਬਿਨਾਨਿਸ਼ਾਨੀ RegexMatcher::ਮੇਲ(StringRef Query) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &R : ਉਲਟ(RegExes))
    ਜੇ (R.Rg.ਮੇਲ(Query))
      ਵਾਪਸ R.ਸਤਰ_No;
  ਵਾਪਸ 0;
}

StringRef RegexMatcher::ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &R : RegExes)
    ਜੇ (R.ਸਤਰ_No == ਸਤਰ_No)
      ਵਾਪਸ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ਵਾਪਸ {};
}

Error GlobMatcher::ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ,
                          ਤਾਰਕਿਕ SlashAgnostic) {
  ਜੇ (ਨਮੂਨਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ, "Supplied glob was blank");

  ਸਵੈਚਾਲਿਤ Res =
      Glob_ਨਮੂਨਾ::create(ਨਮੂਨਾ, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ਜੇ (ਸਵੈਚਾਲਿਤ Err = Res.takeError())
    ਵਾਪਸ Err;
  Globs.emplace_ਪਿੱਠ(ਨਮੂਨਾ, ਸਤਰ_ਸੰਖਿਆ, std::move(Res.get()));
  ਵਾਪਸ Error::success();
}

ਖਾਲੀ GlobMatcher::LazyInit() ਅਚਲ {
  ਜੇ (LLVM_LIKELY(Initialized))
    ਵਾਪਸ;
  Initialized = ਸੱਚ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[Idx, G] : enumerate(Globs)) {
    StringRef ਅਗੇਤਰ = G.ਨਮੂਨਾ.ਅਗੇਤਰ_2();
    StringRef Suffix = G.ਨਮੂਨਾ.suffix();

    ਜੇ (Suffix.ਖਾਲੀ_ਥਾਵਾਂ() && ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.ਨਮੂਨਾ.longest_substr();
      ਜੇ (!Substr.ਖਾਲੀ_ਥਾਵਾਂ()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        ਸਵੈਚਾਲਿਤ &V = Substr_ਮੰਜ਼ਿਲ_Glob.emplace(Substr).ਪਹਿਲਾ_2->ਦੂਜਾ_2;
        V.emplace_ਪਿੱਠ(Idx);
        ਜਾਰੀ;
      }
    }

    ਸਵੈਚਾਲਿਤ &SToGlob = ਅਗੇਤਰ_Suffix_ਮੰਜ਼ਿਲ_Glob.emplace(ਅਗੇਤਰ).ਪਹਿਲਾ_2->ਦੂਜਾ_2;
    ਸਵੈਚਾਲਿਤ &V = SToGlob.emplace(ਉਲਟ(Suffix)).ਪਹਿਲਾ_2->ਦੂਜਾ_2;
    V.emplace_ਪਿੱਠ(Idx);
  }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ GlobMatcher::ਮੇਲ(StringRef Query) ਅਚਲ {
  LazyInit();

  ਪੂਰਨ ਉੱਤਮ = -1;
  ਜੇ (!ਅਗੇਤਰ_Suffix_ਮੰਜ਼ਿਲ_Glob.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[_, SToGlob] : ਅਗੇਤਰ_Suffix_ਮੰਜ਼ਿਲ_Glob.ਲੱਭੋ_prefixes(Query)) {
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[_, V] : SToGlob.ਲੱਭੋ_prefixes(ਉਲਟ(Query))) {
        ਲਈ (ਪੂਰਨ Idx : ਉਲਟ(V)) {
          ਜੇ (ਉੱਤਮ > Idx)
            ਰੋਕੋ;
          ਅਚਲ GlobMatcher::Glob &G = Globs[Idx];
          ਜੇ (G.ਨਮੂਨਾ.ਮੇਲ(Query)) {
            ਉੱਤਮ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ਰੋਕੋ;
          }
        }
      }
    }
  }

  ਜੇ (!Substr_ਮੰਜ਼ਿਲ_Glob.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    ਲਈ (StringRef Q = Query; !Q.ਖਾਲੀ_ਥਾਵਾਂ(); Q = Q.drop_front()) {
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[_, V] : Substr_ਮੰਜ਼ਿਲ_Glob.ਲੱਭੋ_prefixes(Q)) {
        ਲਈ (ਪੂਰਨ Idx : ਉਲਟ(V)) {
          ਜੇ (ਉੱਤਮ > Idx)
            ਰੋਕੋ;
          ਅਚਲ GlobMatcher::Glob &G = Globs[Idx];
          ਜੇ (G.ਨਮੂਨਾ.ਮੇਲ(Query)) {
            ਉੱਤਮ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ਰੋਕੋ;
          }
        }
      }
    }
  }
  ਵਾਪਸ ਉੱਤਮ < 0 ? 0 : Globs[ਉੱਤਮ].ਸਤਰ_No;
}

StringRef GlobMatcher::ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &G : Globs)
    ਜੇ (G.ਸਤਰ_No == ਸਤਰ_No)
      ਵਾਪਸ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ਵਾਪਸ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ਜੇ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ਨਹੀਂਤਾਂ
    M.emplace<RegexMatcher>();
}

Error Matcher::ਪਾਓ(StringRef ਨਮੂਨਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਸੰਖਿਆ) {
  ਵਾਪਸ std::ਦੇਖੋ(
      [&](ਸਵੈਚਾਲਿਤ &V) {
        ਵਾਪਸ V.ਪਾਓ(ਨਮੂਨਾ, ਸਤਰ_ਸੰਖਿਆ, Options.SlashAgnostic);
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
ਬਿਨਾਨਿਸ਼ਾਨੀ Matcher::ਮੇਲ(StringRef Query) ਅਚਲ {
  ਜੇ (!Options.RemoveDotSlash)
    ਵਾਪਸ ਮੇਲ_ਅੰਦਰੂਨੀ(Query);

  ਜੇ (!Options.Warn_Dot_Slash_ਮੇਲ)
    ਵਾਪਸ ਮੇਲ_ਅੰਦਰੂਨੀ(llvm::sys::ਰਸਤਾ_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::ਰਸਤਾ_2::remove_leading_dotslash(Query);
  ਬਿਨਾਨਿਸ਼ਾਨੀ FixedMatched = ਮੇਲ_ਅੰਦਰੂਨੀ(FixedQuery);
  ਜੇ (FixedQuery == Query)
    ਵਾਪਸ FixedMatched;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Original_ਮੇਲ = ਮੇਲ_ਅੰਦਰੂਨੀ(Query);
  ਜੇ (Original_ਮੇਲ > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << ਲੱਭੋ_ਨਿਯਮ(Original_ਮੇਲ) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  ਵਾਪਸ std::max(Original_ਮੇਲ, FixedMatched);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Matcher::ਮੇਲ_ਅੰਦਰੂਨੀ(StringRef Query) ਅਚਲ {
  ਵਾਪਸ std::ਦੇਖੋ([&](ਸਵੈਚਾਲਿਤ &V) -> ਬਿਨਾਨਿਸ਼ਾਨੀ { ਵਾਪਸ V.ਮੇਲ(Query); }, M);
}

StringRef Matcher::ਲੱਭੋ_ਨਿਯਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ {
  ਵਾਪਸ std::ਦੇਖੋ([&](ਸਵੈਚਾਲਿਤ &V) -> StringRef { ਵਾਪਸ V.ਲੱਭੋ_ਨਿਯਮ(ਸਤਰ_No); },
                    M);
}
} // namespace

ਵਰਗ SpecialCaseList::Section::SectionImpl {
ਜਨਤਕ:
  ਅਚਲ Matcher *ਲੱਭੋ_Matcher(StringRef ਅਗੇਤਰ, StringRef Category) ਅਚਲ;

  ਵਰਤੋਂ SectionEntries = StringMap<StringMap<Matcher>>;

  ਸਪਸ਼ਟ SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ਅਚਲ std::ਸਦਿਸ਼<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ਨਵਾਂ SpecialCaseList());
  ਜੇ (SCL->create_ਅੰਦਰੂਨੀ(Paths, FS, Error))
    ਵਾਪਸ SCL;
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ਨਵਾਂ SpecialCaseList());
  ਜੇ (SCL->create_ਅੰਦਰੂਨੀ(MB, Error))
    ਵਾਪਸ SCL;
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ਅਚਲ std::ਸਦਿਸ਼<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ਜੇ (ਸਵੈਚਾਲਿਤ SCL = create(Paths, FS, Error))
    ਵਾਪਸ SCL;
  report_fatal_error(Twine(Error));
}

ਤਾਰਕਿਕ SpecialCaseList::create_ਅੰਦਰੂਨੀ(ਅਚਲ std::ਸਦਿਸ਼<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  ਲਈ (size_t i = 0; i < Paths.ਮਾਪ_2(); ++i) {
    ਅਚਲ ਸਵੈਚਾਲਿਤ &ਰਸਤਾ = Paths[i];
    ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> FileOrErr =
        VFS.get_ਬਫਰ_For_File(ਰਸਤਾ);
    ਜੇ (std::error_ਕੋਡ EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + ਰਸਤਾ + "': " + EC.ਸੁਨੇਹਾ_2()).str();
      ਵਾਪਸ ਝੂਠ;
    }
    std::string ParseError;
    ਜੇ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + ਰਸਤਾ + "': " + ParseError).str();
      ਵਾਪਸ ਝੂਠ;
    }
  }
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ SpecialCaseList::create_ਅੰਦਰੂਨੀ(ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *MB,
                                     std::string &Error) {
  ਜੇ (!parse(0, MB, Error))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::ਜੋੜੋ_Section(StringRef SectionStr, ਬਿਨਾਨਿਸ਼ਾਨੀ FileNo,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No, ਤਾਰਕਿਕ UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_ਪਿੱਠ(SectionStr, FileNo, UseGlobs);
  ਸਵੈਚਾਲਿਤ &Section = Sections.ਪਿੱਠ();

  ਜੇ (ਸਵੈਚਾਲਿਤ Err = Section.Impl->SectionMatcher.ਪਾਓ(SectionStr, ਸਤਰ_No)) {
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "malformed section at line " + Twine(ਸਤਰ_No) +
                                 ": '" + SectionStr +
                                 "': " + ਮੰਜ਼ਿਲ_String(std::move(Err)));
  }

  ਵਾਪਸ &Section;
}

ਤਾਰਕਿਕ SpecialCaseList::parse(ਬਿਨਾਨਿਸ਼ਾਨੀ FileIdx, ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *MB,
                            std::string &Error) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਸੰਸਕਰਣ = 2;

  StringRef ਸਿਰਲੇਖ = MB->get_ਬਫਰ();
  ਜੇ (ਸਿਰਲੇਖ.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(ਸਿਰਲੇਖ, 10, ਸੰਸਕਰਣ);

  ਸਵੈਚਾਲਿਤ Min_ਸੰਸਕਰਣ = [&](ਬਿਨਾਨਿਸ਼ਾਨੀ V) { ਵਾਪਸ ਸੰਸਕਰਣ >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  ਤਾਰਕਿਕ UseGlobs = Min_ਸੰਸਕਰਣ(2);
  ਤਾਰਕਿਕ RemoveDotSlash = Min_ਸੰਸਕਰਣ(3);
  ਤਾਰਕਿਕ WarnDotSlash = Min_ਸੰਸਕਰਣ(4) && !Min_ਸੰਸਕਰਣ(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  ਤਾਰਕਿਕ SlashAgnostic = Min_ਸੰਸਕਰਣ(4) && llvm::sys::ਰਸਤਾ_2::is_style_windows(
                                            llvm::sys::ਰਸਤਾ_2::Style::native);

  ਸਵੈਚਾਲਿਤ ErrOrSection = ਜੋੜੋ_Section("*", FileIdx, 1, ਸੱਚ);
  ਜੇ (ਸਵੈਚਾਲਿਤ Err = ErrOrSection.takeError()) {
    Error = ਮੰਜ਼ਿਲ_String(std::move(Err));
    ਵਾਪਸ ਝੂਠ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef ਰਸਤਾ_Prefixes[] = {"src", "!src", "mainfile", "source"};

  ਲਈ (ਸਤਰ_iterator ਸਤਰ_It(*MB, /*SkipBlanks=*/ਸੱਚ, /*CommentMarker=*/'#');
       !ਸਤਰ_It.is_at_eof(); ਸਤਰ_It++) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No = ਸਤਰ_It.ਸਤਰ_ਸੰਖਿਆ_2();
    StringRef ਸਤਰ = ਸਤਰ_It->trim();
    ਜੇ (ਸਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;

    // Save section names
    ਜੇ (ਸਤਰ.starts_with("[")) {
      ਜੇ (!ਸਤਰ.ਸਿਰੇ_with("]")) {
        Error =
            ("malformed section header on line " + Twine(ਸਤਰ_No) + ": " + ਸਤਰ)
                .str();
        ਵਾਪਸ ਝੂਠ;
      }

      ਸਵੈਚਾਲਿਤ ErrOrSection =
          ਜੋੜੋ_Section(ਸਤਰ.drop_front().drop_ਪਿੱਠ(), FileIdx, ਸਤਰ_No, UseGlobs);
      ਜੇ (ਸਵੈਚਾਲਿਤ Err = ErrOrSection.takeError()) {
        Error = ਮੰਜ਼ਿਲ_String(std::move(Err));
        ਵਾਪਸ ਝੂਠ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ਜਾਰੀ;
    }

    // Get our prefix and unparsed glob.
    ਸਵੈਚਾਲਿਤ [ਅਗੇਤਰ, Postfix] = ਸਤਰ.ਵੰਡੋ(":");
    ਜੇ (Postfix.ਖਾਲੀ_ਥਾਵਾਂ()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(ਸਤਰ_No) + ": '" + ਸਤਰ + "'").str();
      ਵਾਪਸ ਝੂਠ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ਜੇ (llvm::is_contained(ਰਸਤਾ_Prefixes, ਅਗੇਤਰ)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_ਮੇਲ = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    ਸਵੈਚਾਲਿਤ [ਨਮੂਨਾ, Category] = Postfix.ਵੰਡੋ("=");
    ਸਵੈਚਾਲਿਤ [It, _] = CurrentImpl->Entries[ਅਗੇਤਰ].try_emplace(Category, QOpts);
    ਨਮੂਨਾ = ਨਮੂਨਾ.copy(StrAlloc);
    ਜੇ (ਸਵੈਚਾਲਿਤ Err = It->ਦੂਜਾ_2.ਪਾਓ(ਨਮੂਨਾ, ਸਤਰ_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(ਸਤਰ_No) + ": '" + ਨਮੂਨਾ + "': " + ਮੰਜ਼ਿਲ_String(std::move(Err)))
              .str();
      ਵਾਪਸ ਝੂਠ;
    }
  }

  ਵਾਪਸ ਸੱਚ;
}

SpecialCaseList::~SpecialCaseList() = ਮੂਲ;

ਤਾਰਕਿਕ SpecialCaseList::ਵਿੱਚ_Section(StringRef Section, StringRef ਅਗੇਤਰ,
                                StringRef Query, StringRef Category) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ [FileIdx, ਸਤਰ_No] = ਵਿੱਚ_Section_Blame(Section, ਅਗੇਤਰ, Query, Category);
  ਵਾਪਸ ਸਤਰ_No;
}

std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>
SpecialCaseList::ਵਿੱਚ_Section_Blame(StringRef Section, StringRef ਅਗੇਤਰ,
                                StringRef Query, StringRef Category) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &S : ਉਲਟ(Sections)) {
    ਜੇ (S.Impl->SectionMatcher.ਮੇਲ_Any(Section)) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ Blame = S.get_ਆਖਰੀ_ਮੇਲ(ਅਗੇਤਰ, Query, Category);
      ਜੇ (Blame)
        ਵਾਪਸ {S.FileIdx, Blame};
    }
  }
  ਵਾਪਸ Not_ਮਿਲਿਆ;
}

SpecialCaseList::Section::Section(StringRef Str, ਬਿਨਾਨਿਸ਼ਾਨੀ FileIdx,
                                  ਤਾਰਕਿਕ UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/ਝੂਠ})) {}

SpecialCaseList::Section::Section(Section &&) = ਮੂਲ;

SpecialCaseList::Section::~Section() = ਮੂਲ;

ਤਾਰਕਿਕ SpecialCaseList::Section::ਮੇਲ_Name(StringRef Name) ਅਚਲ {
  ਵਾਪਸ Impl->SectionMatcher.ਮੇਲ_Any(Name);
}

ਅਚਲ Matcher *
SpecialCaseList::Section::SectionImpl::ਲੱਭੋ_Matcher(StringRef ਅਗੇਤਰ,
                                                   StringRef Category) ਅਚਲ {
  SectionEntries::const_iterator I = Entries.ਲੱਭੋ(ਅਗੇਤਰ);
  ਜੇ (I == Entries.ਸਿਰਾ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  StringMap<Matcher>::const_iterator II = I->ਦੂਜਾ_2.ਲੱਭੋ(Category);
  ਜੇ (II == I->ਦੂਜਾ_2.ਸਿਰਾ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਵਾਪਸ &II->ਦੂਜਾ_2;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ SpecialCaseList::Section::get_ਆਖਰੀ_ਮੇਲ(StringRef ਅਗੇਤਰ,
                                                StringRef Query,
                                                StringRef Category) ਅਚਲ {
  ਜੇ (ਅਚਲ Matcher *M = Impl->ਲੱਭੋ_Matcher(ਅਗੇਤਰ, Category))
    ਵਾਪਸ M->ਮੇਲ(Query);
  ਵਾਪਸ 0;
}

ਤਾਰਕਿਕ SpecialCaseList::Section::has_ਅਗੇਤਰ(StringRef ਅਗੇਤਰ) ਅਚਲ {
  ਵਾਪਸ Impl->Entries.ਸ਼ਾਮਲ_ਹੈ(ਅਗੇਤਰ);
}

} // namespace llvm
