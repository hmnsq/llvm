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

#समावेश "llvm/Support/SpecialCaseList.h"
#समावेश "llvm/ADT/RadixTree.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringMap.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/iterator_range.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/GlobPattern.h"
#समावेश "llvm/Support/LineIterator.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Regex.h"
#समावेश "llvm/Support/VirtualFileSystem.h"
#समावेश "llvm/Support/WithColor.h"
#समावेश <assert.h>
#समावेश <memory>
#समावेश <mutex>
#समावेश <stdio.h>
#समावेश <string>
#समावेश <system_error>
#समावेश <utility>
#समावेश <variant>
#समावेश <vector>

namespace llvm {

namespace {

// Lagacy v1 matcher.
श्रेणी RegexMatcher {
खुला:
  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या, bool SlashAgnostic);
  अचिन्हित match(StringRef Query) स्थिर;
  StringRef findRule(अचिन्हित पंक्ति_No) स्थिर;

निजी:
  काष्ठा Reg {
    Reg(StringRef Name, अचिन्हित पंक्ति_No, Regex &&Rg)
        : Name(Name), पंक्ति_No(पंक्ति_No), Rg(std::move(Rg)) {}
    StringRef Name;
    अचिन्हित पंक्ति_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

श्रेणी GlobMatcher {
खुला:
  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या, bool SlashAgnostic);
  अचिन्हित match(StringRef Query) स्थिर;
  StringRef findRule(अचिन्हित पंक्ति_No) स्थिर;

निजी:
  काष्ठा Glob {
    Glob(StringRef Name, अचिन्हित पंक्ति_No, GlobPattern &&Pattern)
        : Name(Name), पंक्ति_No(पंक्ति_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    अचिन्हित पंक्ति_No;
    GlobPattern Pattern;
  };

  व्योम LazyInit() स्थिर;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
                    Radix_वृक्ष<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<पूर्णांक, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
                    SmallVector<पूर्णांक, 1>>
      SubstrToGlob;

  mutable bool Initialized = false;
};

काष्ठा QueryOptions {
  bool UseGlobs = true;
  bool RemoveDotSlash = false;
  bool WarnDotSlashMatch = false;
  bool SlashAgnostic = false;
};

/// Represents a set of patterns and their line numbers
श्रेणी Matcher {
खुला:
  explicit Matcher(QueryOptions QOpts);

  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या);
  अचिन्हित match(StringRef Query) स्थिर;

  bool matchAny(StringRef Query) स्थिर { वापस match(Query); }

निजी:
  अचिन्हित matchInternal(StringRef Query) स्थिर;
  StringRef findRule(अचिन्हित पंक्ति_No) स्थिर;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या,
                           bool SlashAgnostic) {
  अगर (Pattern.empty())
    वापस createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  स्वतः Regexp = Pattern.str();
  क्रम (माप_प्रकार pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += म_माप(".*")) {
    Regexp.replace(pos, म_माप("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  अगर (!CheckRE.isValid(REError))
    वापस createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, पंक्ति_संख्या, std::move(CheckRE));
  वापस Error::success();
}

अचिन्हित RegexMatcher::match(StringRef Query) स्थिर {
  क्रम (स्थिर स्वतः &R : reverse(RegExes))
    अगर (R.Rg.match(Query))
      वापस R.पंक्ति_No;
  वापस 0;
}

StringRef RegexMatcher::findRule(अचिन्हित पंक्ति_No) स्थिर {
  क्रम (स्थिर स्वतः &R : RegExes)
    अगर (R.पंक्ति_No == पंक्ति_No)
      वापस R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  वापस {};
}

Error GlobMatcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या,
                          bool SlashAgnostic) {
  अगर (Pattern.empty())
    वापस createStringError(errc::invalid_argument, "Supplied glob was blank");

  स्वतः Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  अगर (स्वतः Err = Res.takeError())
    वापस Err;
  Globs.emplace_back(Pattern, पंक्ति_संख्या, std::move(Res.लो()));
  वापस Error::success();
}

व्योम GlobMatcher::LazyInit() स्थिर {
  अगर (LLVM_LIKELY(Initialized))
    वापस;
  Initialized = true;
  क्रम (स्थिर स्वतः &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    अगर (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      अगर (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        स्वतः &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        जारी;
      }
    }

    स्वतः &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    स्वतः &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

अचिन्हित GlobMatcher::match(StringRef Query) स्थिर {
  LazyInit();

  पूर्णांक Best = -1;
  अगर (!PrefixSuffixToGlob.empty()) {
    क्रम (स्थिर स्वतः &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      क्रम (स्थिर स्वतः &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        क्रम (पूर्णांक Idx : reverse(V)) {
          अगर (Best > Idx)
            अवरोध;
          स्थिर GlobMatcher::Glob &G = Globs[Idx];
          अगर (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            अवरोध;
          }
        }
      }
    }
  }

  अगर (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    क्रम (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      क्रम (स्थिर स्वतः &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        क्रम (पूर्णांक Idx : reverse(V)) {
          अगर (Best > Idx)
            अवरोध;
          स्थिर GlobMatcher::Glob &G = Globs[Idx];
          अगर (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            अवरोध;
          }
        }
      }
    }
  }
  वापस Best < 0 ? 0 : Globs[Best].पंक्ति_No;
}

StringRef GlobMatcher::findRule(अचिन्हित पंक्ति_No) स्थिर {
  क्रम (स्थिर स्वतः &G : Globs)
    अगर (G.पंक्ति_No == पंक्ति_No)
      वापस G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  वापस {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  अगर (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  अन्यथा
    M.emplace<RegexMatcher>();
}

Error Matcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या) {
  वापस std::visit(
      [&](स्वतः &V) {
        वापस V.जोड़ो(Pattern, पंक्ति_संख्या, Options.SlashAgnostic);
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
अचिन्हित Matcher::match(StringRef Query) स्थिर {
  अगर (!Options.RemoveDotSlash)
    वापस matchInternal(Query);

  अगर (!Options.WarnDotSlashMatch)
    वापस matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  अचिन्हित FixedMatched = matchInternal(FixedQuery);
  अगर (FixedQuery == Query)
    वापस FixedMatched;

  अचिन्हित OriginalMatch = matchInternal(Query);
  अगर (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  वापस std::max(OriginalMatch, FixedMatched);
}

अचिन्हित Matcher::matchInternal(StringRef Query) स्थिर {
  वापस std::visit([&](स्वतः &V) -> अचिन्हित { वापस V.match(Query); }, M);
}

StringRef Matcher::findRule(अचिन्हित पंक्ति_No) स्थिर {
  वापस std::visit([&](स्वतः &V) -> StringRef { वापस V.findRule(पंक्ति_No); },
                    M);
}
} // namespace

श्रेणी SpecialCaseList::Section::SectionImpl {
खुला:
  स्थिर Matcher *findMatcher(StringRef Prefix, StringRef Category) स्थिर;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(स्थिर std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नया SpecialCaseList());
  अगर (SCL->createInternal(Paths, FS, Error))
    वापस SCL;
  वापस nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(स्थिर Memory_बफर *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नया SpecialCaseList());
  अगर (SCL->createInternal(MB, Error))
    वापस SCL;
  वापस nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(स्थिर std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  अगर (स्वतः SCL = create(Paths, FS, Error))
    वापस SCL;
  report_fatal_error(Twine(Error));
}

bool SpecialCaseList::createInternal(स्थिर std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  क्रम (माप_प्रकार i = 0; i < Paths.आकार_2(); ++i) {
    स्थिर स्वतः &Path = Paths[i];
    ErrorOr<std::unique_ptr<Memory_बफर>> FileOrErr =
        VFS.get_बफर_For_File(Path);
    अगर (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      वापस false;
    }
    std::string ParseError;
    अगर (!parse(i, FileOrErr.लो().लो(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      वापस false;
    }
  }
  वापस true;
}

bool SpecialCaseList::createInternal(स्थिर Memory_बफर *MB,
                                     std::string &Error) {
  अगर (!parse(0, MB, Error))
    वापस false;
  वापस true;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, अचिन्हित FileNo,
                            अचिन्हित पंक्ति_No, bool UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  स्वतः &Section = Sections.back();

  अगर (स्वतः Err = Section.Impl->SectionMatcher.जोड़ो(SectionStr, पंक्ति_No)) {
    वापस createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(पंक्ति_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  वापस &Section;
}

bool SpecialCaseList::parse(अचिन्हित FileIdx, स्थिर Memory_बफर *MB,
                            std::string &Error) {
  अचिन्हित दीर्घ दीर्घ Version = 2;

  StringRef Header = MB->get_बफर();
  अगर (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  स्वतः MinVersion = [&](अचिन्हित V) { वापस Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  bool UseGlobs = MinVersion(2);
  bool RemoveDotSlash = MinVersion(3);
  bool WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  bool SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  स्वतः ErrOrSection = addSection("*", FileIdx, 1, true);
  अगर (स्वतः Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    वापस false;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.लो()->Impl.लो();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  क्रम (पंक्ति_iterator पंक्ति_It(*MB, /*SkipBlanks=*/true, /*CommentMarker=*/'#');
       !पंक्ति_It.is_at_eof(); पंक्ति_It++) {
    अचिन्हित पंक्ति_No = पंक्ति_It.पंक्ति_संख्या_2();
    StringRef पंक्ति_2 = पंक्ति_It->trim();
    अगर (पंक्ति_2.empty())
      जारी;

    // Save section names
    अगर (पंक्ति_2.starts_with("[")) {
      अगर (!पंक्ति_2.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(पंक्ति_No) + ": " + पंक्ति_2)
                .str();
        वापस false;
      }

      स्वतः ErrOrSection =
          addSection(पंक्ति_2.drop_front().drop_back(), FileIdx, पंक्ति_No, UseGlobs);
      अगर (स्वतः Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        वापस false;
      }
      CurrentImpl = ErrOrSection.लो()->Impl.लो();
      जारी;
    }

    // Get our prefix and unparsed glob.
    स्वतः [Prefix, Postfix] = पंक्ति_2.split(":");
    अगर (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(पंक्ति_No) + ": '" + पंक्ति_2 + "'").str();
      वापस false;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    अगर (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    स्वतः [Pattern, Category] = Postfix.split("=");
    स्वतः [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    अगर (स्वतः Err = It->second.जोड़ो(Pattern, पंक्ति_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(पंक्ति_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      वापस false;
    }
  }

  वापस true;
}

SpecialCaseList::~SpecialCaseList() = शेष;

bool SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) स्थिर {
  स्वतः [FileIdx, पंक्ति_No] = inSectionBlame(Section, Prefix, Query, Category);
  वापस पंक्ति_No;
}

std::pair<अचिन्हित, अचिन्हित>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) स्थिर {
  क्रम (स्थिर स्वतः &S : reverse(Sections)) {
    अगर (S.Impl->SectionMatcher.matchAny(Section)) {
      अचिन्हित Blame = S.getLastMatch(Prefix, Query, Category);
      अगर (Blame)
        वापस {S.FileIdx, Blame};
    }
  }
  वापस NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, अचिन्हित FileIdx,
                                  bool UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/false})) {}

SpecialCaseList::Section::Section(Section &&) = शेष;

SpecialCaseList::Section::~Section() = शेष;

bool SpecialCaseList::Section::matchName(StringRef Name) स्थिर {
  वापस Impl->SectionMatcher.matchAny(Name);
}

स्थिर Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) स्थिर {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  अगर (I == Entries.end())
    वापस nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  अगर (II == I->second.end())
    वापस nullptr;

  वापस &II->second;
}

अचिन्हित SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) स्थिर {
  अगर (स्थिर Matcher *M = Impl->findMatcher(Prefix, Category))
    वापस M->match(Query);
  वापस 0;
}

bool SpecialCaseList::Section::hasPrefix(StringRef Prefix) स्थिर {
  वापस Impl->Entries.contains(Prefix);
}

} // namespace llvm
