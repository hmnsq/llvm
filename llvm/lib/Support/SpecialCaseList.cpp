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
  Error ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ, ತಾರ್ಕಿಕ SlashAgnostic);
  unsigned match(StringRef Query) ಧ್ರುವ;
  StringRef findRule(unsigned ಸಾಲು_No) ಧ್ರುವ;

ಗುಪ್ತ:
  ಸಂರಚನಾ Reg {
    Reg(StringRef Name, unsigned ಸಾಲು_No, Regex &&Rg)
        : Name(Name), ಸಾಲು_No(ಸಾಲು_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned ಸಾಲು_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

ವರ್ಗ GlobMatcher {
ಸಾರ್ವಜನಿಕ:
  Error ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ, ತಾರ್ಕಿಕ SlashAgnostic);
  unsigned match(StringRef Query) ಧ್ರುವ;
  StringRef findRule(unsigned ಸಾಲು_No) ಧ್ರುವ;

ಗುಪ್ತ:
  ಸಂರಚನಾ Glob {
    Glob(StringRef Name, unsigned ಸಾಲು_No, GlobPattern &&Pattern)
        : Name(Name), ಸಾಲು_No(ಸಾಲು_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned ಸಾಲು_No;
    GlobPattern Pattern;
  };

  ಶೂನ್ಯ LazyInit() ಧ್ರುವ;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_iterator>,
                    Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<ಪೂರ್ಣಾಙ್ಕ, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_ವೃಕ್ಷ<iterator_range<StringRef::const_iterator>,
                    SmallVector<ಪೂರ್ಣಾಙ್ಕ, 1>>
      SubstrToGlob;

  mutable ತಾರ್ಕಿಕ Initialized = ಅಸತ್ಯ;
};

ಸಂರಚನಾ QueryOptions {
  ತಾರ್ಕಿಕ UseGlobs = ಸತ್ಯ;
  ತಾರ್ಕಿಕ RemoveDotSlash = ಅಸತ್ಯ;
  ತಾರ್ಕಿಕ WarnDotSlashMatch = ಅಸತ್ಯ;
  ತಾರ್ಕಿಕ SlashAgnostic = ಅಸತ್ಯ;
};

/// Represents a set of patterns and their line numbers
ವರ್ಗ Matcher {
ಸಾರ್ವಜನಿಕ:
  explicit Matcher(QueryOptions QOpts);

  Error ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ);
  unsigned match(StringRef Query) ಧ್ರುವ;

  ತಾರ್ಕಿಕ matchAny(StringRef Query) ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ match(Query); }

ಗುಪ್ತ:
  unsigned matchInternal(StringRef Query) ಧ್ರುವ;
  StringRef findRule(unsigned ಸಾಲು_No) ಧ್ರುವ;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ,
                           ತಾರ್ಕಿಕ SlashAgnostic) {
  ಯದಿ (Pattern.empty())
    ಪ್ರತ್ಯಾಗಮ createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  ಪ್ರತಿ (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ಯದಿ (!CheckRE.isValid(REError))
    ಪ್ರತ್ಯಾಗಮ createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, ಸಾಲು_ಸಂಖ್ಯೆ, std::move(CheckRE));
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ಧ್ರುವ {
  ಪ್ರತಿ (ಧ್ರುವ auto &R : reverse(RegExes))
    ಯದಿ (R.Rg.match(Query))
      ಪ್ರತ್ಯಾಗಮ R.ಸಾಲು_No;
  ಪ್ರತ್ಯಾಗಮ 0;
}

StringRef RegexMatcher::findRule(unsigned ಸಾಲು_No) ಧ್ರುವ {
  ಪ್ರತಿ (ಧ್ರುವ auto &R : RegExes)
    ಯದಿ (R.ಸಾಲು_No == ಸಾಲು_No)
      ಪ್ರತ್ಯಾಗಮ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ಪ್ರತ್ಯಾಗಮ {};
}

Error GlobMatcher::ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ,
                          ತಾರ್ಕಿಕ SlashAgnostic) {
  ಯದಿ (Pattern.empty())
    ಪ್ರತ್ಯಾಗಮ createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ಯದಿ (auto Err = Res.takeError())
    ಪ್ರತ್ಯಾಗಮ Err;
  Globs.emplace_back(Pattern, ಸಾಲು_ಸಂಖ್ಯೆ, std::move(Res.get()));
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

ಶೂನ್ಯ GlobMatcher::LazyInit() ಧ್ರುವ {
  ಯದಿ (LLVM_LIKELY(Initialized))
    ಪ್ರತ್ಯಾಗಮ;
  Initialized = ಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    ಯದಿ (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      ಯದಿ (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        ಅನುವರ್ತ;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ಧ್ರುವ {
  LazyInit();

  ಪೂರ್ಣಾಙ್ಕ Best = -1;
  ಯದಿ (!PrefixSuffixToGlob.empty()) {
    ಪ್ರತಿ (ಧ್ರುವ auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      ಪ್ರತಿ (ಧ್ರುವ auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ Idx : reverse(V)) {
          ಯದಿ (Best > Idx)
            ಭಙ್ಗ;
          ಧ್ರುವ GlobMatcher::Glob &G = Globs[Idx];
          ಯದಿ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ಭಙ್ಗ;
          }
        }
      }
    }
  }

  ಯದಿ (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    ಪ್ರತಿ (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      ಪ್ರತಿ (ಧ್ರುವ auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ Idx : reverse(V)) {
          ಯದಿ (Best > Idx)
            ಭಙ್ಗ;
          ಧ್ರುವ GlobMatcher::Glob &G = Globs[Idx];
          ಯದಿ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ಭಙ್ಗ;
          }
        }
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ Best < 0 ? 0 : Globs[Best].ಸಾಲು_No;
}

StringRef GlobMatcher::findRule(unsigned ಸಾಲು_No) ಧ್ರುವ {
  ಪ್ರತಿ (ಧ್ರುವ auto &G : Globs)
    ಯದಿ (G.ಸಾಲು_No == ಸಾಲು_No)
      ಪ್ರತ್ಯಾಗಮ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ಪ್ರತ್ಯಾಗಮ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ಯದಿ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ಅನ್ಯಥಾ
    M.emplace<RegexMatcher>();
}

Error Matcher::ಸೇರಿಸು(StringRef Pattern, unsigned ಸಾಲು_ಸಂಖ್ಯೆ) {
  ಪ್ರತ್ಯಾಗಮ std::visit(
      [&](auto &V) {
        ಪ್ರತ್ಯಾಗಮ V.ಸೇರಿಸು(Pattern, ಸಾಲು_ಸಂಖ್ಯೆ, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) ಧ್ರುವ {
  ಯದಿ (!Options.RemoveDotSlash)
    ಪ್ರತ್ಯಾಗಮ matchInternal(Query);

  ಯದಿ (!Options.WarnDotSlashMatch)
    ಪ್ರತ್ಯಾಗಮ matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  ಯದಿ (FixedQuery == Query)
    ಪ್ರತ್ಯಾಗಮ FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  ಯದಿ (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  ಪ್ರತ್ಯಾಗಮ std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::visit([&](auto &V) -> unsigned { ಪ್ರತ್ಯಾಗಮ V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned ಸಾಲು_No) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::visit([&](auto &V) -> StringRef { ಪ್ರತ್ಯಾಗಮ V.findRule(ಸಾಲು_No); },
                    M);
}
} // namespace

ವರ್ಗ SpecialCaseList::Section::SectionImpl {
ಸಾರ್ವಜನಿಕ:
  ಧ್ರುವ Matcher *findMatcher(StringRef Prefix, StringRef Category) ಧ್ರುವ;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ಧ್ರುವ std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ನೂತನ SpecialCaseList());
  ಯದಿ (SCL->createInternal(Paths, FS, Error))
    ಪ್ರತ್ಯಾಗಮ SCL;
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ಧ್ರುವ MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ನೂತನ SpecialCaseList());
  ಯದಿ (SCL->createInternal(MB, Error))
    ಪ್ರತ್ಯಾಗಮ SCL;
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ಧ್ರುವ std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ಯದಿ (auto SCL = create(Paths, FS, Error))
    ಪ್ರತ್ಯಾಗಮ SCL;
  report_fatal_error(Twine(Error));
}

ತಾರ್ಕಿಕ SpecialCaseList::createInternal(ಧ್ರುವ std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  ಪ್ರತಿ (size_t i = 0; i < Paths.size(); ++i) {
    ಧ್ರುವ auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    ಯದಿ (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    std::string ParseError;
    ಯದಿ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ SpecialCaseList::createInternal(ಧ್ರುವ MemoryBuffer *MB,
                                     std::string &Error) {
  ಯದಿ (!parse(0, MB, Error))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned ಸಾಲು_No, ತಾರ್ಕಿಕ UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  ಯದಿ (auto Err = Section.Impl->SectionMatcher.ಸೇರಿಸು(SectionStr, ಸಾಲು_No)) {
    ಪ್ರತ್ಯಾಗಮ createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(ಸಾಲು_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  ಪ್ರತ್ಯಾಗಮ &Section;
}

ತಾರ್ಕಿಕ SpecialCaseList::parse(unsigned FileIdx, ಧ್ರುವ MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  ಯದಿ (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { ಪ್ರತ್ಯಾಗಮ Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  ತಾರ್ಕಿಕ UseGlobs = MinVersion(2);
  ತಾರ್ಕಿಕ RemoveDotSlash = MinVersion(3);
  ತಾರ್ಕಿಕ WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  ತಾರ್ಕಿಕ SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, ಸತ್ಯ);
  ಯದಿ (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  ಪ್ರತಿ (ಸಾಲು_iterator ಸಾಲು_It(*MB, /*SkipBlanks=*/ಸತ್ಯ, /*CommentMarker=*/'#');
       !ಸಾಲು_It.is_at_eof(); ಸಾಲು_It++) {
    unsigned ಸಾಲು_No = ಸಾಲು_It.ಸಾಲು_ಸಂಖ್ಯೆ_2();
    StringRef ಸಾಲು = ಸಾಲು_It->trim();
    ಯದಿ (ಸಾಲು.empty())
      ಅನುವರ್ತ;

    // Save section names
    ಯದಿ (ಸಾಲು.starts_with("[")) {
      ಯದಿ (!ಸಾಲು.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(ಸಾಲು_No) + ": " + ಸಾಲು)
                .str();
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }

      auto ErrOrSection =
          addSection(ಸಾಲು.drop_front().drop_back(), FileIdx, ಸಾಲು_No, UseGlobs);
      ಯದಿ (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ಅನುವರ್ತ;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = ಸಾಲು.split(":");
    ಯದಿ (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(ಸಾಲು_No) + ": '" + ಸಾಲು + "'").str();
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ಯದಿ (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    ಯದಿ (auto Err = It->second.ಸೇರಿಸು(Pattern, ಸಾಲು_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(ಸಾಲು_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

SpecialCaseList::~SpecialCaseList() = ಸ್ವಭಾವ;

ತಾರ್ಕಿಕ SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ಧ್ರುವ {
  auto [FileIdx, ಸಾಲು_No] = inSectionBlame(Section, Prefix, Query, Category);
  ಪ್ರತ್ಯಾಗಮ ಸಾಲು_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ಧ್ರುವ {
  ಪ್ರತಿ (ಧ್ರುವ auto &S : reverse(Sections)) {
    ಯದಿ (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      ಯದಿ (Blame)
        ಪ್ರತ್ಯಾಗಮ {S.FileIdx, Blame};
    }
  }
  ಪ್ರತ್ಯಾಗಮ NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  ತಾರ್ಕಿಕ UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/ಅಸತ್ಯ})) {}

SpecialCaseList::Section::Section(Section &&) = ಸ್ವಭಾವ;

SpecialCaseList::Section::~Section() = ಸ್ವಭಾವ;

ತಾರ್ಕಿಕ SpecialCaseList::Section::matchName(StringRef Name) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Impl->SectionMatcher.matchAny(Name);
}

ಧ್ರುವ Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ಧ್ರುವ {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  ಯದಿ (I == Entries.end())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  ಯದಿ (II == I->second.end())
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಪ್ರತ್ಯಾಗಮ &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ಧ್ರುವ {
  ಯದಿ (ಧ್ರುವ Matcher *M = Impl->findMatcher(Prefix, Category))
    ಪ್ರತ್ಯಾಗಮ M->match(Query);
  ಪ್ರತ್ಯಾಗಮ 0;
}

ತಾರ್ಕಿಕ SpecialCaseList::Section::hasPrefix(StringRef Prefix) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Impl->Entries.contains(Prefix);
}

} // namespace llvm
