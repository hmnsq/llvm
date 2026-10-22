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

#അന്തര്ഭാവ "llvm/Support/SpecialCaseList.h"
#അന്തര്ഭാവ "llvm/ADT/RadixTree.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/iterator_range.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/GlobPattern.h"
#അന്തര്ഭാവ "llvm/Support/LineIterator.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Regex.h"
#അന്തര്ഭാവ "llvm/Support/VirtualFileSystem.h"
#അന്തര്ഭാവ "llvm/Support/WithColor.h"
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <mutex>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <system_error>
#അന്തര്ഭാവ <utility>
#അന്തര്ഭാവ <variant>
#അന്തര്ഭാവ <vector>

നാമാകാശ llvm {

നാമാകാശ {

// Lagacy v1 matcher.
വർഗ്ഗം RegexMatcher {
പൊതു:
  Error തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ, തർക്കം SlashAgnostic);
  unsigned match(StringRef Query) സ്ഥിരം;
  StringRef findRule(unsigned വരി_No) സ്ഥിരം;

സ്വകാര്യ:
  ഘടന Reg {
    Reg(StringRef Name, unsigned വരി_No, Regex &&Rg)
        : Name(Name), വരി_No(വരി_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned വരി_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

വർഗ്ഗം GlobMatcher {
പൊതു:
  Error തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ, തർക്കം SlashAgnostic);
  unsigned match(StringRef Query) സ്ഥിരം;
  StringRef findRule(unsigned വരി_No) സ്ഥിരം;

സ്വകാര്യ:
  ഘടന Glob {
    Glob(StringRef Name, unsigned വരി_No, GlobPattern &&Pattern)
        : Name(Name), വരി_No(വരി_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned വരി_No;
    GlobPattern Pattern;
  };

  ശൂന്യം LazyInit() സ്ഥിരം;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_വൃക്ഷം<iterator_range<StringRef::const_iterator>,
                    Radix_വൃക്ഷം<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<പൂർണ്ണസംഖ്യ, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_വൃക്ഷം<iterator_range<StringRef::const_iterator>,
                    SmallVector<പൂർണ്ണസംഖ്യ, 1>>
      SubstrToGlob;

  mutable തർക്കം Initialized = അസത്യം;
};

ഘടന QueryOptions {
  തർക്കം UseGlobs = സത്യം;
  തർക്കം RemoveDotSlash = അസത്യം;
  തർക്കം WarnDotSlashMatch = അസത്യം;
  തർക്കം SlashAgnostic = അസത്യം;
};

/// Represents a set of patterns and their line numbers
വർഗ്ഗം Matcher {
പൊതു:
  explicit Matcher(QueryOptions QOpts);

  Error തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ);
  unsigned match(StringRef Query) സ്ഥിരം;

  തർക്കം matchAny(StringRef Query) സ്ഥിരം { തിരികെ match(Query); }

സ്വകാര്യ:
  unsigned matchInternal(StringRef Query) സ്ഥിരം;
  StringRef findRule(unsigned വരി_No) സ്ഥിരം;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ,
                           തർക്കം SlashAgnostic) {
  എങ്കിൽ (Pattern.empty())
    തിരികെ createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  വേണ്ടി (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  എങ്കിൽ (!CheckRE.isValid(REError))
    തിരികെ createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, വരി_സംഖ്യ, std::move(CheckRE));
  തിരികെ Error::success();
}

unsigned RegexMatcher::match(StringRef Query) സ്ഥിരം {
  വേണ്ടി (സ്ഥിരം auto &R : reverse(RegExes))
    എങ്കിൽ (R.Rg.match(Query))
      തിരികെ R.വരി_No;
  തിരികെ 0;
}

StringRef RegexMatcher::findRule(unsigned വരി_No) സ്ഥിരം {
  വേണ്ടി (സ്ഥിരം auto &R : RegExes)
    എങ്കിൽ (R.വരി_No == വരി_No)
      തിരികെ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  തിരികെ {};
}

Error GlobMatcher::തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ,
                          തർക്കം SlashAgnostic) {
  എങ്കിൽ (Pattern.empty())
    തിരികെ createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  എങ്കിൽ (auto Err = Res.takeError())
    തിരികെ Err;
  Globs.emplace_back(Pattern, വരി_സംഖ്യ, std::move(Res.get()));
  തിരികെ Error::success();
}

ശൂന്യം GlobMatcher::LazyInit() സ്ഥിരം {
  എങ്കിൽ (LLVM_LIKELY(Initialized))
    തിരികെ;
  Initialized = സത്യം;
  വേണ്ടി (സ്ഥിരം auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    എങ്കിൽ (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      എങ്കിൽ (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        തുടരുക;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) സ്ഥിരം {
  LazyInit();

  പൂർണ്ണസംഖ്യ Best = -1;
  എങ്കിൽ (!PrefixSuffixToGlob.empty()) {
    വേണ്ടി (സ്ഥിരം auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      വേണ്ടി (സ്ഥിരം auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        വേണ്ടി (പൂർണ്ണസംഖ്യ Idx : reverse(V)) {
          എങ്കിൽ (Best > Idx)
            നിർത്തുക;
          സ്ഥിരം GlobMatcher::Glob &G = Globs[Idx];
          എങ്കിൽ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            നിർത്തുക;
          }
        }
      }
    }
  }

  എങ്കിൽ (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    വേണ്ടി (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      വേണ്ടി (സ്ഥിരം auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        വേണ്ടി (പൂർണ്ണസംഖ്യ Idx : reverse(V)) {
          എങ്കിൽ (Best > Idx)
            നിർത്തുക;
          സ്ഥിരം GlobMatcher::Glob &G = Globs[Idx];
          എങ്കിൽ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            നിർത്തുക;
          }
        }
      }
    }
  }
  തിരികെ Best < 0 ? 0 : Globs[Best].വരി_No;
}

StringRef GlobMatcher::findRule(unsigned വരി_No) സ്ഥിരം {
  വേണ്ടി (സ്ഥിരം auto &G : Globs)
    എങ്കിൽ (G.വരി_No == വരി_No)
      തിരികെ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  തിരികെ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  എങ്കിൽ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  അല്ലെങ്കിൽ
    M.emplace<RegexMatcher>();
}

Error Matcher::തിരുകുക(StringRef Pattern, unsigned വരി_സംഖ്യ) {
  തിരികെ std::visit(
      [&](auto &V) {
        തിരികെ V.തിരുകുക(Pattern, വരി_സംഖ്യ, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) സ്ഥിരം {
  എങ്കിൽ (!Options.RemoveDotSlash)
    തിരികെ matchInternal(Query);

  എങ്കിൽ (!Options.WarnDotSlashMatch)
    തിരികെ matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  എങ്കിൽ (FixedQuery == Query)
    തിരികെ FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  എങ്കിൽ (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  തിരികെ std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) സ്ഥിരം {
  തിരികെ std::visit([&](auto &V) -> unsigned { തിരികെ V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned വരി_No) സ്ഥിരം {
  തിരികെ std::visit([&](auto &V) -> StringRef { തിരികെ V.findRule(വരി_No); },
                    M);
}
} // namespace

വർഗ്ഗം SpecialCaseList::Section::SectionImpl {
പൊതു:
  സ്ഥിരം Matcher *findMatcher(StringRef Prefix, StringRef Category) സ്ഥിരം;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(സ്ഥിരം std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(പുതിയ SpecialCaseList());
  എങ്കിൽ (SCL->createInternal(Paths, FS, Error))
    തിരികെ SCL;
  തിരികെ nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(സ്ഥിരം MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(പുതിയ SpecialCaseList());
  എങ്കിൽ (SCL->createInternal(MB, Error))
    തിരികെ SCL;
  തിരികെ nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(സ്ഥിരം std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  എങ്കിൽ (auto SCL = create(Paths, FS, Error))
    തിരികെ SCL;
  report_fatal_error(Twine(Error));
}

തർക്കം SpecialCaseList::createInternal(സ്ഥിരം std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  വേണ്ടി (size_t i = 0; i < Paths.size(); ++i) {
    സ്ഥിരം auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    എങ്കിൽ (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      തിരികെ അസത്യം;
    }
    std::string ParseError;
    എങ്കിൽ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      തിരികെ അസത്യം;
    }
  }
  തിരികെ സത്യം;
}

തർക്കം SpecialCaseList::createInternal(സ്ഥിരം MemoryBuffer *MB,
                                     std::string &Error) {
  എങ്കിൽ (!parse(0, MB, Error))
    തിരികെ അസത്യം;
  തിരികെ സത്യം;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned വരി_No, തർക്കം UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  എങ്കിൽ (auto Err = Section.Impl->SectionMatcher.തിരുകുക(SectionStr, വരി_No)) {
    തിരികെ createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(വരി_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  തിരികെ &Section;
}

തർക്കം SpecialCaseList::parse(unsigned FileIdx, സ്ഥിരം MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  എങ്കിൽ (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { തിരികെ Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  തർക്കം UseGlobs = MinVersion(2);
  തർക്കം RemoveDotSlash = MinVersion(3);
  തർക്കം WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  തർക്കം SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, സത്യം);
  എങ്കിൽ (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    തിരികെ അസത്യം;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  വേണ്ടി (വരി_iterator വരി_It(*MB, /*SkipBlanks=*/സത്യം, /*CommentMarker=*/'#');
       !വരി_It.is_at_eof(); വരി_It++) {
    unsigned വരി_No = വരി_It.വരി_സംഖ്യ_2();
    StringRef വരി = വരി_It->trim();
    എങ്കിൽ (വരി.empty())
      തുടരുക;

    // Save section names
    എങ്കിൽ (വരി.starts_with("[")) {
      എങ്കിൽ (!വരി.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(വരി_No) + ": " + വരി)
                .str();
        തിരികെ അസത്യം;
      }

      auto ErrOrSection =
          addSection(വരി.drop_front().drop_back(), FileIdx, വരി_No, UseGlobs);
      എങ്കിൽ (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        തിരികെ അസത്യം;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      തുടരുക;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = വരി.split(":");
    എങ്കിൽ (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(വരി_No) + ": '" + വരി + "'").str();
      തിരികെ അസത്യം;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    എങ്കിൽ (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    എങ്കിൽ (auto Err = It->second.തിരുകുക(Pattern, വരി_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(വരി_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      തിരികെ അസത്യം;
    }
  }

  തിരികെ സത്യം;
}

SpecialCaseList::~SpecialCaseList() = സ്ഥിരസ്ഥിതി;

തർക്കം SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) സ്ഥിരം {
  auto [FileIdx, വരി_No] = inSectionBlame(Section, Prefix, Query, Category);
  തിരികെ വരി_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) സ്ഥിരം {
  വേണ്ടി (സ്ഥിരം auto &S : reverse(Sections)) {
    എങ്കിൽ (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      എങ്കിൽ (Blame)
        തിരികെ {S.FileIdx, Blame};
    }
  }
  തിരികെ NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  തർക്കം UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/അസത്യം})) {}

SpecialCaseList::Section::Section(Section &&) = സ്ഥിരസ്ഥിതി;

SpecialCaseList::Section::~Section() = സ്ഥിരസ്ഥിതി;

തർക്കം SpecialCaseList::Section::matchName(StringRef Name) സ്ഥിരം {
  തിരികെ Impl->SectionMatcher.matchAny(Name);
}

സ്ഥിരം Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) സ്ഥിരം {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  എങ്കിൽ (I == Entries.end())
    തിരികെ nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  എങ്കിൽ (II == I->second.end())
    തിരികെ nullptr;

  തിരികെ &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) സ്ഥിരം {
  എങ്കിൽ (സ്ഥിരം Matcher *M = Impl->findMatcher(Prefix, Category))
    തിരികെ M->match(Query);
  തിരികെ 0;
}

തർക്കം SpecialCaseList::Section::hasPrefix(StringRef Prefix) സ്ഥിരം {
  തിരികെ Impl->Entries.contains(Prefix);
}

} // namespace llvm
