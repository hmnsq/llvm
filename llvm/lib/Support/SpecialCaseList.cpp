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

#अन्तर्भाव "llvm/Support/SpecialCaseList.h"
#अन्तर्भाव "llvm/ADT/RadixTree.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/iterator_range.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/GlobPattern.h"
#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Regex.h"
#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/Support/WithColor.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>
#अन्तर्भाव <variant>
#अन्तर्भाव <vector>

नामाकाश llvm {

नामाकाश {

// Lagacy v1 matcher.
वर्ग RegexMatcher {
सार्वजनिक:
  Error निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या, तार्किक SlashAgnostic);
  unsigned match(StringRef Query) ध्रुव;
  StringRef findRule(unsigned पङ्क्तिः_No) ध्रुव;

गुप्त:
  संरचना Reg {
    Reg(StringRef Name, unsigned पङ्क्तिः_No, Regex &&Rg)
        : Name(Name), पङ्क्तिः_No(पङ्क्तिः_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned पङ्क्तिः_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

वर्ग GlobMatcher {
सार्वजनिक:
  Error निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या, तार्किक SlashAgnostic);
  unsigned match(StringRef Query) ध्रुव;
  StringRef findRule(unsigned पङ्क्तिः_No) ध्रुव;

गुप्त:
  संरचना Glob {
    Glob(StringRef Name, unsigned पङ्क्तिः_No, GlobPattern &&Pattern)
        : Name(Name), पङ्क्तिः_No(पङ्क्तिः_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned पङ्क्तिः_No;
    GlobPattern Pattern;
  };

  शून्य LazyInit() ध्रुव;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_वृक्षः<iterator_range<StringRef::const_iterator>,
                    Radix_वृक्षः<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<पूर्णाङ्क, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_वृक्षः<iterator_range<StringRef::const_iterator>,
                    SmallVector<पूर्णाङ्क, 1>>
      SubstrToGlob;

  mutable तार्किक Initialized = मिच्छ;
};

संरचना QueryOptions {
  तार्किक UseGlobs = सच्च;
  तार्किक RemoveDotSlash = मिच्छ;
  तार्किक WarnDotSlashMatch = मिच्छ;
  तार्किक SlashAgnostic = मिच्छ;
};

/// Represents a set of patterns and their line numbers
वर्ग Matcher {
सार्वजनिक:
  explicit Matcher(QueryOptions QOpts);

  Error निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या);
  unsigned match(StringRef Query) ध्रुव;

  तार्किक matchAny(StringRef Query) ध्रुव { प्रत्यागम match(Query); }

गुप्त:
  unsigned matchInternal(StringRef Query) ध्रुव;
  StringRef findRule(unsigned पङ्क्तिः_No) ध्रुव;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या,
                           तार्किक SlashAgnostic) {
  जइ (Pattern.empty())
    प्रत्यागम createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  प्रति (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  जइ (!CheckRE.isValid(REError))
    प्रत्यागम createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, पङ्क्तिः_सङ्ख्या, std::move(CheckRE));
  प्रत्यागम Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ध्रुव {
  प्रति (ध्रुव auto &R : reverse(RegExes))
    जइ (R.Rg.match(Query))
      प्रत्यागम R.पङ्क्तिः_No;
  प्रत्यागम 0;
}

StringRef RegexMatcher::findRule(unsigned पङ्क्तिः_No) ध्रुव {
  प्रति (ध्रुव auto &R : RegExes)
    जइ (R.पङ्क्तिः_No == पङ्क्तिः_No)
      प्रत्यागम R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  प्रत्यागम {};
}

Error GlobMatcher::निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या,
                          तार्किक SlashAgnostic) {
  जइ (Pattern.empty())
    प्रत्यागम createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  जइ (auto Err = Res.takeError())
    प्रत्यागम Err;
  Globs.emplace_back(Pattern, पङ्क्तिः_सङ्ख्या, std::move(Res.get()));
  प्रत्यागम Error::success();
}

शून्य GlobMatcher::LazyInit() ध्रुव {
  जइ (LLVM_LIKELY(Initialized))
    प्रत्यागम;
  Initialized = सच्च;
  प्रति (ध्रुव auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    जइ (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      जइ (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        अनुवर्त;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ध्रुव {
  LazyInit();

  पूर्णाङ्क Best = -1;
  जइ (!PrefixSuffixToGlob.empty()) {
    प्रति (ध्रुव auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      प्रति (ध्रुव auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          जइ (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          जइ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            भङ्ग;
          }
        }
      }
    }
  }

  जइ (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    प्रति (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      प्रति (ध्रुव auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          जइ (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          जइ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            भङ्ग;
          }
        }
      }
    }
  }
  प्रत्यागम Best < 0 ? 0 : Globs[Best].पङ्क्तिः_No;
}

StringRef GlobMatcher::findRule(unsigned पङ्क्तिः_No) ध्रुव {
  प्रति (ध्रुव auto &G : Globs)
    जइ (G.पङ्क्तिः_No == पङ्क्तिः_No)
      प्रत्यागम G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  प्रत्यागम {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  जइ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  अण्णहा
    M.emplace<RegexMatcher>();
}

Error Matcher::निवेशय(StringRef Pattern, unsigned पङ्क्तिः_सङ्ख्या) {
  प्रत्यागम std::visit(
      [&](auto &V) {
        प्रत्यागम V.निवेशय(Pattern, पङ्क्तिः_सङ्ख्या, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) ध्रुव {
  जइ (!Options.RemoveDotSlash)
    प्रत्यागम matchInternal(Query);

  जइ (!Options.WarnDotSlashMatch)
    प्रत्यागम matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  जइ (FixedQuery == Query)
    प्रत्यागम FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  जइ (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  प्रत्यागम std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ध्रुव {
  प्रत्यागम std::visit([&](auto &V) -> unsigned { प्रत्यागम V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned पङ्क्तिः_No) ध्रुव {
  प्रत्यागम std::visit([&](auto &V) -> StringRef { प्रत्यागम V.findRule(पङ्क्तिः_No); },
                    M);
}
} // namespace

वर्ग SpecialCaseList::Section::SectionImpl {
सार्वजनिक:
  ध्रुव Matcher *findMatcher(StringRef Prefix, StringRef Category) ध्रुव;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ध्रुव std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  जइ (SCL->createInternal(Paths, FS, Error))
    प्रत्यागम SCL;
  प्रत्यागम nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ध्रुव MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  जइ (SCL->createInternal(MB, Error))
    प्रत्यागम SCL;
  प्रत्यागम nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ध्रुव std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  जइ (auto SCL = create(Paths, FS, Error))
    प्रत्यागम SCL;
  report_fatal_error(Twine(Error));
}

तार्किक SpecialCaseList::createInternal(ध्रुव std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  प्रति (size_t i = 0; i < Paths.size(); ++i) {
    ध्रुव auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    जइ (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      प्रत्यागम मिच्छ;
    }
    std::string ParseError;
    जइ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      प्रत्यागम मिच्छ;
    }
  }
  प्रत्यागम सच्च;
}

तार्किक SpecialCaseList::createInternal(ध्रुव MemoryBuffer *MB,
                                     std::string &Error) {
  जइ (!parse(0, MB, Error))
    प्रत्यागम मिच्छ;
  प्रत्यागम सच्च;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned पङ्क्तिः_No, तार्किक UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  जइ (auto Err = Section.Impl->SectionMatcher.निवेशय(SectionStr, पङ्क्तिः_No)) {
    प्रत्यागम createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(पङ्क्तिः_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  प्रत्यागम &Section;
}

तार्किक SpecialCaseList::parse(unsigned FileIdx, ध्रुव MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  जइ (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { प्रत्यागम Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  तार्किक UseGlobs = MinVersion(2);
  तार्किक RemoveDotSlash = MinVersion(3);
  तार्किक WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  तार्किक SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, सच्च);
  जइ (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    प्रत्यागम मिच्छ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  प्रति (पङ्क्तिः_iterator पङ्क्तिः_It(*MB, /*SkipBlanks=*/सच्च, /*CommentMarker=*/'#');
       !पङ्क्तिः_It.is_at_eof(); पङ्क्तिः_It++) {
    unsigned पङ्क्तिः_No = पङ्क्तिः_It.पङ्क्तिः_सङ्ख्या_2();
    StringRef पङ्क्तिः = पङ्क्तिः_It->trim();
    जइ (पङ्क्तिः.empty())
      अनुवर्त;

    // Save section names
    जइ (पङ्क्तिः.starts_with("[")) {
      जइ (!पङ्क्तिः.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(पङ्क्तिः_No) + ": " + पङ्क्तिः)
                .str();
        प्रत्यागम मिच्छ;
      }

      auto ErrOrSection =
          addSection(पङ्क्तिः.drop_front().drop_back(), FileIdx, पङ्क्तिः_No, UseGlobs);
      जइ (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        प्रत्यागम मिच्छ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      अनुवर्त;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = पङ्क्तिः.split(":");
    जइ (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(पङ्क्तिः_No) + ": '" + पङ्क्तिः + "'").str();
      प्रत्यागम मिच्छ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    जइ (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    जइ (auto Err = It->second.निवेशय(Pattern, पङ्क्तिः_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(पङ्क्तिः_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      प्रत्यागम मिच्छ;
    }
  }

  प्रत्यागम सच्च;
}

SpecialCaseList::~SpecialCaseList() = स्वभाव;

तार्किक SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ध्रुव {
  auto [FileIdx, पङ्क्तिः_No] = inSectionBlame(Section, Prefix, Query, Category);
  प्रत्यागम पङ्क्तिः_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ध्रुव {
  प्रति (ध्रुव auto &S : reverse(Sections)) {
    जइ (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      जइ (Blame)
        प्रत्यागम {S.FileIdx, Blame};
    }
  }
  प्रत्यागम NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  तार्किक UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/मिच्छ})) {}

SpecialCaseList::Section::Section(Section &&) = स्वभाव;

SpecialCaseList::Section::~Section() = स्वभाव;

तार्किक SpecialCaseList::Section::matchName(StringRef Name) ध्रुव {
  प्रत्यागम Impl->SectionMatcher.matchAny(Name);
}

ध्रुव Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ध्रुव {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  जइ (I == Entries.end())
    प्रत्यागम nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  जइ (II == I->second.end())
    प्रत्यागम nullptr;

  प्रत्यागम &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ध्रुव {
  जइ (ध्रुव Matcher *M = Impl->findMatcher(Prefix, Category))
    प्रत्यागम M->match(Query);
  प्रत्यागम 0;
}

तार्किक SpecialCaseList::Section::hasPrefix(StringRef Prefix) ध्रुव {
  प्रत्यागम Impl->Entries.contains(Prefix);
}

} // namespace llvm
