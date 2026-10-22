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

#અન્તર્ભાવ "llvm/Support/SpecialCaseList.h"
#અન્તર્ભાવ "llvm/ADT/RadixTree.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringMap.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/iterator_range.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/GlobPattern.h"
#અન્તર્ભાવ "llvm/Support/LineIterator.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Regex.h"
#અન્તર્ભાવ "llvm/Support/VirtualFileSystem.h"
#અન્તર્ભાવ "llvm/Support/WithColor.h"
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <mutex>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <utility>
#અન્તર્ભાવ <variant>
#અન્તર્ભાવ <vector>

નામાકાશ llvm {

નામાકાશ {

// Lagacy v1 matcher.
વર્ગ RegexMatcher {
જાહેર:
  Error ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા, તાર્કિક SlashAgnostic);
  unsigned match(StringRef Query) અચલ;
  StringRef findRule(unsigned લીટી_No) અચલ;

ખાનગી:
  સંરચના Reg {
    Reg(StringRef Name, unsigned લીટી_No, Regex &&Rg)
        : Name(Name), લીટી_No(લીટી_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned લીટી_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

વર્ગ GlobMatcher {
જાહેર:
  Error ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા, તાર્કિક SlashAgnostic);
  unsigned match(StringRef Query) અચલ;
  StringRef findRule(unsigned લીટી_No) અચલ;

ખાનગી:
  સંરચના Glob {
    Glob(StringRef Name, unsigned લીટી_No, GlobPattern &&Pattern)
        : Name(Name), લીટી_No(લીટી_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned લીટી_No;
    GlobPattern Pattern;
  };

  શૂન્ય LazyInit() અચલ;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_વૃક્ષ<iterator_range<StringRef::const_iterator>,
                    Radix_વૃક્ષ<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<પૂર્ણાંક, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_વૃક્ષ<iterator_range<StringRef::const_iterator>,
                    SmallVector<પૂર્ણાંક, 1>>
      SubstrToGlob;

  mutable તાર્કિક Initialized = ખોટું;
};

સંરચના QueryOptions {
  તાર્કિક UseGlobs = સાચું;
  તાર્કિક RemoveDotSlash = ખોટું;
  તાર્કિક WarnDotSlashMatch = ખોટું;
  તાર્કિક SlashAgnostic = ખોટું;
};

/// Represents a set of patterns and their line numbers
વર્ગ Matcher {
જાહેર:
  explicit Matcher(QueryOptions QOpts);

  Error ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા);
  unsigned match(StringRef Query) અચલ;

  તાર્કિક matchAny(StringRef Query) અચલ { પાછા match(Query); }

ખાનગી:
  unsigned matchInternal(StringRef Query) અચલ;
  StringRef findRule(unsigned લીટી_No) અચલ;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા,
                           તાર્કિક SlashAgnostic) {
  જો (Pattern.empty())
    પાછા createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  માટે (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  જો (!CheckRE.isValid(REError))
    પાછા createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, લીટી_સંખ્યા, std::move(CheckRE));
  પાછા Error::success();
}

unsigned RegexMatcher::match(StringRef Query) અચલ {
  માટે (અચલ auto &R : reverse(RegExes))
    જો (R.Rg.match(Query))
      પાછા R.લીટી_No;
  પાછા 0;
}

StringRef RegexMatcher::findRule(unsigned લીટી_No) અચલ {
  માટે (અચલ auto &R : RegExes)
    જો (R.લીટી_No == લીટી_No)
      પાછા R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  પાછા {};
}

Error GlobMatcher::ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા,
                          તાર્કિક SlashAgnostic) {
  જો (Pattern.empty())
    પાછા createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  જો (auto Err = Res.takeError())
    પાછા Err;
  Globs.emplace_back(Pattern, લીટી_સંખ્યા, std::move(Res.get()));
  પાછા Error::success();
}

શૂન્ય GlobMatcher::LazyInit() અચલ {
  જો (LLVM_LIKELY(Initialized))
    પાછા;
  Initialized = સાચું;
  માટે (અચલ auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    જો (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      જો (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        ચાલુ;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) અચલ {
  LazyInit();

  પૂર્ણાંક Best = -1;
  જો (!PrefixSuffixToGlob.empty()) {
    માટે (અચલ auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      માટે (અચલ auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        માટે (પૂર્ણાંક Idx : reverse(V)) {
          જો (Best > Idx)
            તોડો;
          અચલ GlobMatcher::Glob &G = Globs[Idx];
          જો (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            તોડો;
          }
        }
      }
    }
  }

  જો (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    માટે (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      માટે (અચલ auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        માટે (પૂર્ણાંક Idx : reverse(V)) {
          જો (Best > Idx)
            તોડો;
          અચલ GlobMatcher::Glob &G = Globs[Idx];
          જો (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            તોડો;
          }
        }
      }
    }
  }
  પાછા Best < 0 ? 0 : Globs[Best].લીટી_No;
}

StringRef GlobMatcher::findRule(unsigned લીટી_No) અચલ {
  માટે (અચલ auto &G : Globs)
    જો (G.લીટી_No == લીટી_No)
      પાછા G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  પાછા {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  જો (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  નહીંતર
    M.emplace<RegexMatcher>();
}

Error Matcher::ઉમેરો(StringRef Pattern, unsigned લીટી_સંખ્યા) {
  પાછા std::visit(
      [&](auto &V) {
        પાછા V.ઉમેરો(Pattern, લીટી_સંખ્યા, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) અચલ {
  જો (!Options.RemoveDotSlash)
    પાછા matchInternal(Query);

  જો (!Options.WarnDotSlashMatch)
    પાછા matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  જો (FixedQuery == Query)
    પાછા FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  જો (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  પાછા std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) અચલ {
  પાછા std::visit([&](auto &V) -> unsigned { પાછા V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned લીટી_No) અચલ {
  પાછા std::visit([&](auto &V) -> StringRef { પાછા V.findRule(લીટી_No); },
                    M);
}
} // namespace

વર્ગ SpecialCaseList::Section::SectionImpl {
જાહેર:
  અચલ Matcher *findMatcher(StringRef Prefix, StringRef Category) અચલ;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(અચલ std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(નવું SpecialCaseList());
  જો (SCL->createInternal(Paths, FS, Error))
    પાછા SCL;
  પાછા nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(અચલ MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(નવું SpecialCaseList());
  જો (SCL->createInternal(MB, Error))
    પાછા SCL;
  પાછા nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(અચલ std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  જો (auto SCL = create(Paths, FS, Error))
    પાછા SCL;
  report_fatal_error(Twine(Error));
}

તાર્કિક SpecialCaseList::createInternal(અચલ std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  માટે (size_t i = 0; i < Paths.size(); ++i) {
    અચલ auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    જો (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      પાછા ખોટું;
    }
    std::string ParseError;
    જો (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      પાછા ખોટું;
    }
  }
  પાછા સાચું;
}

તાર્કિક SpecialCaseList::createInternal(અચલ MemoryBuffer *MB,
                                     std::string &Error) {
  જો (!parse(0, MB, Error))
    પાછા ખોટું;
  પાછા સાચું;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned લીટી_No, તાર્કિક UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  જો (auto Err = Section.Impl->SectionMatcher.ઉમેરો(SectionStr, લીટી_No)) {
    પાછા createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(લીટી_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  પાછા &Section;
}

તાર્કિક SpecialCaseList::parse(unsigned FileIdx, અચલ MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  જો (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { પાછા Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  તાર્કિક UseGlobs = MinVersion(2);
  તાર્કિક RemoveDotSlash = MinVersion(3);
  તાર્કિક WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  તાર્કિક SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, સાચું);
  જો (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    પાછા ખોટું;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  માટે (લીટી_iterator લીટી_It(*MB, /*SkipBlanks=*/સાચું, /*CommentMarker=*/'#');
       !લીટી_It.is_at_eof(); લીટી_It++) {
    unsigned લીટી_No = લીટી_It.લીટી_સંખ્યા_2();
    StringRef લીટી = લીટી_It->trim();
    જો (લીટી.empty())
      ચાલુ;

    // Save section names
    જો (લીટી.starts_with("[")) {
      જો (!લીટી.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(લીટી_No) + ": " + લીટી)
                .str();
        પાછા ખોટું;
      }

      auto ErrOrSection =
          addSection(લીટી.drop_front().drop_back(), FileIdx, લીટી_No, UseGlobs);
      જો (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        પાછા ખોટું;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ચાલુ;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = લીટી.split(":");
    જો (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(લીટી_No) + ": '" + લીટી + "'").str();
      પાછા ખોટું;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    જો (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    જો (auto Err = It->second.ઉમેરો(Pattern, લીટી_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(લીટી_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      પાછા ખોટું;
    }
  }

  પાછા સાચું;
}

SpecialCaseList::~SpecialCaseList() = મૂળભૂત;

તાર્કિક SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) અચલ {
  auto [FileIdx, લીટી_No] = inSectionBlame(Section, Prefix, Query, Category);
  પાછા લીટી_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) અચલ {
  માટે (અચલ auto &S : reverse(Sections)) {
    જો (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      જો (Blame)
        પાછા {S.FileIdx, Blame};
    }
  }
  પાછા NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  તાર્કિક UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/ખોટું})) {}

SpecialCaseList::Section::Section(Section &&) = મૂળભૂત;

SpecialCaseList::Section::~Section() = મૂળભૂત;

તાર્કિક SpecialCaseList::Section::matchName(StringRef Name) અચલ {
  પાછા Impl->SectionMatcher.matchAny(Name);
}

અચલ Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) અચલ {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  જો (I == Entries.end())
    પાછા nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  જો (II == I->second.end())
    પાછા nullptr;

  પાછા &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) અચલ {
  જો (અચલ Matcher *M = Impl->findMatcher(Prefix, Category))
    પાછા M->match(Query);
  પાછા 0;
}

તાર્કિક SpecialCaseList::Section::hasPrefix(StringRef Prefix) અચલ {
  પાછા Impl->Entries.contains(Prefix);
}

} // namespace llvm
