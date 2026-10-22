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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/SpecialCaseList.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/RadixTree.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/iterator_range.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/GlobPattern.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LineIterator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Regex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualFileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/WithColor.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <mutex>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <system_error>
#ଅନ୍ତର୍ଭାଵ <utility>
#ଅନ୍ତର୍ଭାଵ <variant>
#ଅନ୍ତର୍ଭାଵ <vector>

ନାମାକାଶ llvm {

ନାମାକାଶ {

// Lagacy v1 matcher.
ଶ୍ରେଣୀ RegexMatcher {
ସର୍ବସାଧାରଣ:
  Error ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା, ତାର୍କିକ SlashAgnostic);
  unsigned match(StringRef Query) ଅଚଳ;
  StringRef findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ;

ବ୍ୟକ୍ତିଗତ:
  ସଂରଚନା Reg {
    Reg(StringRef Name, unsigned ଧାଡ଼ି_No, Regex &&Rg)
        : Name(Name), ଧାଡ଼ି_No(ଧାଡ଼ି_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned ଧାଡ଼ି_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

ଶ୍ରେଣୀ GlobMatcher {
ସର୍ବସାଧାରଣ:
  Error ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା, ତାର୍କିକ SlashAgnostic);
  unsigned match(StringRef Query) ଅଚଳ;
  StringRef findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ;

ବ୍ୟକ୍ତିଗତ:
  ସଂରଚନା Glob {
    Glob(StringRef Name, unsigned ଧାଡ଼ି_No, GlobPattern &&Pattern)
        : Name(Name), ଧାଡ଼ି_No(ଧାଡ଼ି_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned ଧାଡ଼ି_No;
    GlobPattern Pattern;
  };

  ଶୂନ୍ୟ LazyInit() ଅଚଳ;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_ବୃକ୍ଷ<iterator_range<StringRef::const_iterator>,
                    Radix_ବୃକ୍ଷ<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<ପୂର୍ଣ୍ଣସଂଖ୍ୟା, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_ବୃକ୍ଷ<iterator_range<StringRef::const_iterator>,
                    SmallVector<ପୂର୍ଣ୍ଣସଂଖ୍ୟା, 1>>
      SubstrToGlob;

  mutable ତାର୍କିକ Initialized = ମିଥ୍ୟା;
};

ସଂରଚନା QueryOptions {
  ତାର୍କିକ UseGlobs = ସତ୍ୟ;
  ତାର୍କିକ RemoveDotSlash = ମିଥ୍ୟା;
  ତାର୍କିକ WarnDotSlashMatch = ମିଥ୍ୟା;
  ତାର୍କିକ SlashAgnostic = ମିଥ୍ୟା;
};

/// Represents a set of patterns and their line numbers
ଶ୍ରେଣୀ Matcher {
ସର୍ବସାଧାରଣ:
  explicit Matcher(QueryOptions QOpts);

  Error ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା);
  unsigned match(StringRef Query) ଅଚଳ;

  ତାର୍କିକ matchAny(StringRef Query) ଅଚଳ { ଫେରନ୍ତୁ match(Query); }

ବ୍ୟକ୍ତିଗତ:
  unsigned matchInternal(StringRef Query) ଅଚଳ;
  StringRef findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା,
                           ତାର୍କିକ SlashAgnostic) {
  ଯଦି (Pattern.empty())
    ଫେରନ୍ତୁ createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  ପାଇଁ (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ଯଦି (!CheckRE.isValid(REError))
    ଫେରନ୍ତୁ createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, ଧାଡ଼ି_ସଂଖ୍ୟା, std::move(CheckRE));
  ଫେରନ୍ତୁ Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ଅଚଳ {
  ପାଇଁ (ଅଚଳ auto &R : reverse(RegExes))
    ଯଦି (R.Rg.match(Query))
      ଫେରନ୍ତୁ R.ଧାଡ଼ି_No;
  ଫେରନ୍ତୁ 0;
}

StringRef RegexMatcher::findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ {
  ପାଇଁ (ଅଚଳ auto &R : RegExes)
    ଯଦି (R.ଧାଡ଼ି_No == ଧାଡ଼ି_No)
      ଫେରନ୍ତୁ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ଫେରନ୍ତୁ {};
}

Error GlobMatcher::ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା,
                          ତାର୍କିକ SlashAgnostic) {
  ଯଦି (Pattern.empty())
    ଫେରନ୍ତୁ createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ଯଦି (auto Err = Res.takeError())
    ଫେରନ୍ତୁ Err;
  Globs.emplace_back(Pattern, ଧାଡ଼ି_ସଂଖ୍ୟା, std::move(Res.get()));
  ଫେରନ୍ତୁ Error::success();
}

ଶୂନ୍ୟ GlobMatcher::LazyInit() ଅଚଳ {
  ଯଦି (LLVM_LIKELY(Initialized))
    ଫେରନ୍ତୁ;
  Initialized = ସତ୍ୟ;
  ପାଇଁ (ଅଚଳ auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    ଯଦି (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      ଯଦି (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        ଜାରି;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ଅଚଳ {
  LazyInit();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Best = -1;
  ଯଦି (!PrefixSuffixToGlob.empty()) {
    ପାଇଁ (ଅଚଳ auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      ପାଇଁ (ଅଚଳ auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Idx : reverse(V)) {
          ଯଦି (Best > Idx)
            ଭାଙ୍ଗ;
          ଅଚଳ GlobMatcher::Glob &G = Globs[Idx];
          ଯଦି (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ଭାଙ୍ଗ;
          }
        }
      }
    }
  }

  ଯଦି (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    ପାଇଁ (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      ପାଇଁ (ଅଚଳ auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Idx : reverse(V)) {
          ଯଦି (Best > Idx)
            ଭାଙ୍ଗ;
          ଅଚଳ GlobMatcher::Glob &G = Globs[Idx];
          ଯଦି (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ଭାଙ୍ଗ;
          }
        }
      }
    }
  }
  ଫେରନ୍ତୁ Best < 0 ? 0 : Globs[Best].ଧାଡ଼ି_No;
}

StringRef GlobMatcher::findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ {
  ପାଇଁ (ଅଚଳ auto &G : Globs)
    ଯଦି (G.ଧାଡ଼ି_No == ଧାଡ଼ି_No)
      ଫେରନ୍ତୁ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ଫେରନ୍ତୁ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ଯଦି (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ଅନ୍ୟଥା
    M.emplace<RegexMatcher>();
}

Error Matcher::ଭର୍ତ୍ତି(StringRef Pattern, unsigned ଧାଡ଼ି_ସଂଖ୍ୟା) {
  ଫେରନ୍ତୁ std::visit(
      [&](auto &V) {
        ଫେରନ୍ତୁ V.ଭର୍ତ୍ତି(Pattern, ଧାଡ଼ି_ସଂଖ୍ୟା, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) ଅଚଳ {
  ଯଦି (!Options.RemoveDotSlash)
    ଫେରନ୍ତୁ matchInternal(Query);

  ଯଦି (!Options.WarnDotSlashMatch)
    ଫେରନ୍ତୁ matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  ଯଦି (FixedQuery == Query)
    ଫେରନ୍ତୁ FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  ଯଦି (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  ଫେରନ୍ତୁ std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ଅଚଳ {
  ଫେରନ୍ତୁ std::visit([&](auto &V) -> unsigned { ଫେରନ୍ତୁ V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned ଧାଡ଼ି_No) ଅଚଳ {
  ଫେରନ୍ତୁ std::visit([&](auto &V) -> StringRef { ଫେରନ୍ତୁ V.findRule(ଧାଡ଼ି_No); },
                    M);
}
} // namespace

ଶ୍ରେଣୀ SpecialCaseList::Section::SectionImpl {
ସର୍ବସାଧାରଣ:
  ଅଚଳ Matcher *findMatcher(StringRef Prefix, StringRef Category) ଅଚଳ;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ଅଚଳ std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ନୂତନ SpecialCaseList());
  ଯଦି (SCL->createInternal(Paths, FS, Error))
    ଫେରନ୍ତୁ SCL;
  ଫେରନ୍ତୁ nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ଅଚଳ MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(ନୂତନ SpecialCaseList());
  ଯଦି (SCL->createInternal(MB, Error))
    ଫେରନ୍ତୁ SCL;
  ଫେରନ୍ତୁ nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ଅଚଳ std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ଯଦି (auto SCL = create(Paths, FS, Error))
    ଫେରନ୍ତୁ SCL;
  report_fatal_error(Twine(Error));
}

ତାର୍କିକ SpecialCaseList::createInternal(ଅଚଳ std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  ପାଇଁ (size_t i = 0; i < Paths.size(); ++i) {
    ଅଚଳ auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    ଯଦି (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    std::string ParseError;
    ଯଦି (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ SpecialCaseList::createInternal(ଅଚଳ MemoryBuffer *MB,
                                     std::string &Error) {
  ଯଦି (!parse(0, MB, Error))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned ଧାଡ଼ି_No, ତାର୍କିକ UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  ଯଦି (auto Err = Section.Impl->SectionMatcher.ଭର୍ତ୍ତି(SectionStr, ଧାଡ଼ି_No)) {
    ଫେରନ୍ତୁ createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(ଧାଡ଼ି_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  ଫେରନ୍ତୁ &Section;
}

ତାର୍କିକ SpecialCaseList::parse(unsigned FileIdx, ଅଚଳ MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  ଯଦି (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { ଫେରନ୍ତୁ Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  ତାର୍କିକ UseGlobs = MinVersion(2);
  ତାର୍କିକ RemoveDotSlash = MinVersion(3);
  ତାର୍କିକ WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  ତାର୍କିକ SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, ସତ୍ୟ);
  ଯଦି (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  ପାଇଁ (ଧାଡ଼ି_iterator ଧାଡ଼ି_It(*MB, /*SkipBlanks=*/ସତ୍ୟ, /*CommentMarker=*/'#');
       !ଧାଡ଼ି_It.is_at_eof(); ଧାଡ଼ି_It++) {
    unsigned ଧାଡ଼ି_No = ଧାଡ଼ି_It.ଧାଡ଼ି_ସଂଖ୍ୟା_2();
    StringRef ଧାଡ଼ି = ଧାଡ଼ି_It->trim();
    ଯଦି (ଧାଡ଼ି.empty())
      ଜାରି;

    // Save section names
    ଯଦି (ଧାଡ଼ି.starts_with("[")) {
      ଯଦି (!ଧାଡ଼ି.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(ଧାଡ଼ି_No) + ": " + ଧାଡ଼ି)
                .str();
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }

      auto ErrOrSection =
          addSection(ଧାଡ଼ି.drop_front().drop_back(), FileIdx, ଧାଡ଼ି_No, UseGlobs);
      ଯଦି (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ଜାରି;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = ଧାଡ଼ି.split(":");
    ଯଦି (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(ଧାଡ଼ି_No) + ": '" + ଧାଡ଼ି + "'").str();
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ଯଦି (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    ଯଦି (auto Err = It->second.ଭର୍ତ୍ତି(Pattern, ଧାଡ଼ି_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(ଧାଡ଼ି_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  }

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

SpecialCaseList::~SpecialCaseList() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ତାର୍କିକ SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ଅଚଳ {
  auto [FileIdx, ଧାଡ଼ି_No] = inSectionBlame(Section, Prefix, Query, Category);
  ଫେରନ୍ତୁ ଧାଡ଼ି_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ଅଚଳ {
  ପାଇଁ (ଅଚଳ auto &S : reverse(Sections)) {
    ଯଦି (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      ଯଦି (Blame)
        ଫେରନ୍ତୁ {S.FileIdx, Blame};
    }
  }
  ଫେରନ୍ତୁ NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  ତାର୍କିକ UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/ମିଥ୍ୟା})) {}

SpecialCaseList::Section::Section(Section &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

SpecialCaseList::Section::~Section() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ତାର୍କିକ SpecialCaseList::Section::matchName(StringRef Name) ଅଚଳ {
  ଫେରନ୍ତୁ Impl->SectionMatcher.matchAny(Name);
}

ଅଚଳ Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ଅଚଳ {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  ଯଦି (I == Entries.end())
    ଫେରନ୍ତୁ nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  ଯଦି (II == I->second.end())
    ଫେରନ୍ତୁ nullptr;

  ଫେରନ୍ତୁ &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ଅଚଳ {
  ଯଦି (ଅଚଳ Matcher *M = Impl->findMatcher(Prefix, Category))
    ଫେରନ୍ତୁ M->match(Query);
  ଫେରନ୍ତୁ 0;
}

ତାର୍କିକ SpecialCaseList::Section::hasPrefix(StringRef Prefix) ଅଚଳ {
  ଫେରନ୍ତୁ Impl->Entries.contains(Prefix);
}

} // namespace llvm
