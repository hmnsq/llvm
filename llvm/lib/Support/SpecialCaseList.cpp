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

#অন্তর্ভাব "llvm/Support/SpecialCaseList.h"
#অন্তর্ভাব "llvm/ADT/RadixTree.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringMap.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/iterator_range.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/GlobPattern.h"
#অন্তর্ভাব "llvm/Support/LineIterator.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/Regex.h"
#অন্তর্ভাব "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাব "llvm/Support/WithColor.h"
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <mutex>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string>
#অন্তর্ভাব <system_error>
#অন্তর্ভাব <utility>
#অন্তর্ভাব <variant>
#অন্তর্ভাব <vector>

নামাকাশ llvm {

নামাকাশ {

// Lagacy v1 matcher.
বর্গ RegexMatcher {
সার্বজনিক:
  Error ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা, তার্কিক SlashAgnostic);
  unsigned match(StringRef Query) ধ্রুব;
  StringRef findRule(unsigned সারি_No) ধ্রুব;

গুপ্ত:
  সংরচনা Reg {
    Reg(StringRef Name, unsigned সারি_No, Regex &&Rg)
        : Name(Name), সারি_No(সারি_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned সারি_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

বর্গ GlobMatcher {
সার্বজনিক:
  Error ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা, তার্কিক SlashAgnostic);
  unsigned match(StringRef Query) ধ্রুব;
  StringRef findRule(unsigned সারি_No) ধ্রুব;

গুপ্ত:
  সংরচনা Glob {
    Glob(StringRef Name, unsigned সারি_No, GlobPattern &&Pattern)
        : Name(Name), সারি_No(সারি_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned সারি_No;
    GlobPattern Pattern;
  };

  শূন্য LazyInit() ধ্রুব;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_বৃক্ষ<iterator_range<StringRef::const_iterator>,
                    Radix_বৃক্ষ<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<পূর্ণাঙ্ক, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_বৃক্ষ<iterator_range<StringRef::const_iterator>,
                    SmallVector<পূর্ণাঙ্ক, 1>>
      SubstrToGlob;

  mutable তার্কিক Initialized = অসত্য;
};

সংরচনা QueryOptions {
  তার্কিক UseGlobs = সত্য;
  তার্কিক RemoveDotSlash = অসত্য;
  তার্কিক WarnDotSlashMatch = অসত্য;
  তার্কিক SlashAgnostic = অসত্য;
};

/// Represents a set of patterns and their line numbers
বর্গ Matcher {
সার্বজনিক:
  explicit Matcher(QueryOptions QOpts);

  Error ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা);
  unsigned match(StringRef Query) ধ্রুব;

  তার্কিক matchAny(StringRef Query) ধ্রুব { প্রত্যাগম match(Query); }

গুপ্ত:
  unsigned matchInternal(StringRef Query) ধ্রুব;
  StringRef findRule(unsigned সারি_No) ধ্রুব;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা,
                           তার্কিক SlashAgnostic) {
  যদি (Pattern.empty())
    প্রত্যাগম createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  প্রতি (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  যদি (!CheckRE.isValid(REError))
    প্রত্যাগম createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, সারি_সংখ্যা, std::move(CheckRE));
  প্রত্যাগম Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ধ্রুব {
  প্রতি (ধ্রুব auto &R : reverse(RegExes))
    যদি (R.Rg.match(Query))
      প্রত্যাগম R.সারি_No;
  প্রত্যাগম 0;
}

StringRef RegexMatcher::findRule(unsigned সারি_No) ধ্রুব {
  প্রতি (ধ্রুব auto &R : RegExes)
    যদি (R.সারি_No == সারি_No)
      প্রত্যাগম R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  প্রত্যাগম {};
}

Error GlobMatcher::ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা,
                          তার্কিক SlashAgnostic) {
  যদি (Pattern.empty())
    প্রত্যাগম createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  যদি (auto Err = Res.takeError())
    প্রত্যাগম Err;
  Globs.emplace_back(Pattern, সারি_সংখ্যা, std::move(Res.get()));
  প্রত্যাগম Error::success();
}

শূন্য GlobMatcher::LazyInit() ধ্রুব {
  যদি (LLVM_LIKELY(Initialized))
    প্রত্যাগম;
  Initialized = সত্য;
  প্রতি (ধ্রুব auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    যদি (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      যদি (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        অনুবর্ত;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ধ্রুব {
  LazyInit();

  পূর্ণাঙ্ক Best = -1;
  যদি (!PrefixSuffixToGlob.empty()) {
    প্রতি (ধ্রুব auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      প্রতি (ধ্রুব auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        প্রতি (পূর্ণাঙ্ক Idx : reverse(V)) {
          যদি (Best > Idx)
            ভঙ্গ;
          ধ্রুব GlobMatcher::Glob &G = Globs[Idx];
          যদি (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ভঙ্গ;
          }
        }
      }
    }
  }

  যদি (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    প্রতি (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      প্রতি (ধ্রুব auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        প্রতি (পূর্ণাঙ্ক Idx : reverse(V)) {
          যদি (Best > Idx)
            ভঙ্গ;
          ধ্রুব GlobMatcher::Glob &G = Globs[Idx];
          যদি (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ভঙ্গ;
          }
        }
      }
    }
  }
  প্রত্যাগম Best < 0 ? 0 : Globs[Best].সারি_No;
}

StringRef GlobMatcher::findRule(unsigned সারি_No) ধ্রুব {
  প্রতি (ধ্রুব auto &G : Globs)
    যদি (G.সারি_No == সারি_No)
      প্রত্যাগম G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  প্রত্যাগম {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  যদি (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  অন্যথা
    M.emplace<RegexMatcher>();
}

Error Matcher::ঢোকাও(StringRef Pattern, unsigned সারি_সংখ্যা) {
  প্রত্যাগম std::visit(
      [&](auto &V) {
        প্রত্যাগম V.ঢোকাও(Pattern, সারি_সংখ্যা, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) ধ্রুব {
  যদি (!Options.RemoveDotSlash)
    প্রত্যাগম matchInternal(Query);

  যদি (!Options.WarnDotSlashMatch)
    প্রত্যাগম matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  যদি (FixedQuery == Query)
    প্রত্যাগম FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  যদি (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  প্রত্যাগম std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ধ্রুব {
  প্রত্যাগম std::visit([&](auto &V) -> unsigned { প্রত্যাগম V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned সারি_No) ধ্রুব {
  প্রত্যাগম std::visit([&](auto &V) -> StringRef { প্রত্যাগম V.findRule(সারি_No); },
                    M);
}
} // namespace

বর্গ SpecialCaseList::Section::SectionImpl {
সার্বজনিক:
  ধ্রুব Matcher *findMatcher(StringRef Prefix, StringRef Category) ধ্রুব;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ধ্রুব std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(নূতন SpecialCaseList());
  যদি (SCL->createInternal(Paths, FS, Error))
    প্রত্যাগম SCL;
  প্রত্যাগম nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ধ্রুব MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(নূতন SpecialCaseList());
  যদি (SCL->createInternal(MB, Error))
    প্রত্যাগম SCL;
  প্রত্যাগম nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ধ্রুব std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  যদি (auto SCL = create(Paths, FS, Error))
    প্রত্যাগম SCL;
  report_fatal_error(Twine(Error));
}

তার্কিক SpecialCaseList::createInternal(ধ্রুব std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  প্রতি (size_t i = 0; i < Paths.size(); ++i) {
    ধ্রুব auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    যদি (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      প্রত্যাগম অসত্য;
    }
    std::string ParseError;
    যদি (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      প্রত্যাগম অসত্য;
    }
  }
  প্রত্যাগম সত্য;
}

তার্কিক SpecialCaseList::createInternal(ধ্রুব MemoryBuffer *MB,
                                     std::string &Error) {
  যদি (!parse(0, MB, Error))
    প্রত্যাগম অসত্য;
  প্রত্যাগম সত্য;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned সারি_No, তার্কিক UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  যদি (auto Err = Section.Impl->SectionMatcher.ঢোকাও(SectionStr, সারি_No)) {
    প্রত্যাগম createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(সারি_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  প্রত্যাগম &Section;
}

তার্কিক SpecialCaseList::parse(unsigned FileIdx, ধ্রুব MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  যদি (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { প্রত্যাগম Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  তার্কিক UseGlobs = MinVersion(2);
  তার্কিক RemoveDotSlash = MinVersion(3);
  তার্কিক WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  তার্কিক SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, সত্য);
  যদি (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    প্রত্যাগম অসত্য;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  প্রতি (সারি_iterator সারি_It(*MB, /*SkipBlanks=*/সত্য, /*CommentMarker=*/'#');
       !সারি_It.is_at_eof(); সারি_It++) {
    unsigned সারি_No = সারি_It.সারি_সংখ্যা_2();
    StringRef সারি = সারি_It->trim();
    যদি (সারি.empty())
      অনুবর্ত;

    // Save section names
    যদি (সারি.starts_with("[")) {
      যদি (!সারি.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(সারি_No) + ": " + সারি)
                .str();
        প্রত্যাগম অসত্য;
      }

      auto ErrOrSection =
          addSection(সারি.drop_front().drop_back(), FileIdx, সারি_No, UseGlobs);
      যদি (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        প্রত্যাগম অসত্য;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      অনুবর্ত;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = সারি.split(":");
    যদি (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(সারি_No) + ": '" + সারি + "'").str();
      প্রত্যাগম অসত্য;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    যদি (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    যদি (auto Err = It->second.ঢোকাও(Pattern, সারি_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(সারি_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      প্রত্যাগম অসত্য;
    }
  }

  প্রত্যাগম সত্য;
}

SpecialCaseList::~SpecialCaseList() = স্বভাব;

তার্কিক SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ধ্রুব {
  auto [FileIdx, সারি_No] = inSectionBlame(Section, Prefix, Query, Category);
  প্রত্যাগম সারি_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ধ্রুব {
  প্রতি (ধ্রুব auto &S : reverse(Sections)) {
    যদি (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      যদি (Blame)
        প্রত্যাগম {S.FileIdx, Blame};
    }
  }
  প্রত্যাগম NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  তার্কিক UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/অসত্য})) {}

SpecialCaseList::Section::Section(Section &&) = স্বভাব;

SpecialCaseList::Section::~Section() = স্বভাব;

তার্কিক SpecialCaseList::Section::matchName(StringRef Name) ধ্রুব {
  প্রত্যাগম Impl->SectionMatcher.matchAny(Name);
}

ধ্রুব Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ধ্রুব {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  যদি (I == Entries.end())
    প্রত্যাগম nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  যদি (II == I->second.end())
    প্রত্যাগম nullptr;

  প্রত্যাগম &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ধ্রুব {
  যদি (ধ্রুব Matcher *M = Impl->findMatcher(Prefix, Category))
    প্রত্যাগম M->match(Query);
  প্রত্যাগম 0;
}

তার্কিক SpecialCaseList::Section::hasPrefix(StringRef Prefix) ধ্রুব {
  প্রত্যাগম Impl->Entries.contains(Prefix);
}

} // namespace llvm
