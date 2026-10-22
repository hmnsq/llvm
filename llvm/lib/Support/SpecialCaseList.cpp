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

#include "llvm/Support/SpecialCaseList.h"
#include "llvm/ADT/RadixTree.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/GlobPattern.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/WithColor.h"
#include <assert.h>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

namespace llvm {

namespace {

// Lagacy v1 matcher.
class RegexMatcher {
public:
  Error insert(StringRef Pattern, unsigned LineNumber, bool SlashAgnostic);
  unsigned match(StringRef Query) const;
  StringRef findRule(unsigned LineNo) const;

private:
  sa_nracanaa Reg {
    Reg(StringRef Name, unsigned LineNo, Regex &&Rg)
        : Name(Name), LineNo(LineNo), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned LineNo;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

class GlobMatcher {
public:
  Error insert(StringRef Pattern, unsigned LineNumber, bool SlashAgnostic);
  unsigned match(StringRef Query) const;
  StringRef findRule(unsigned LineNo) const;

private:
  sa_nracanaa Glob {
    Glob(StringRef Name, unsigned LineNo, GlobPattern &&Pattern)
        : Name(Name), LineNo(LineNo), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned LineNo;
    GlobPattern Pattern;
  };

  void LazyInit() const;

  std::vector<GlobMatcher::Glob> Globs;

  mutable RadixTree<iterator_range<StringRef::const_iterator>,
                    RadixTree<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<puur_naa_nka, 1>>>
      PrefixSuffixToGlob;

  mutable RadixTree<iterator_range<StringRef::const_iterator>,
                    SmallVector<puur_naa_nka, 1>>
      SubstrToGlob;

  mutable bool Initialized = false;
};

sa_nracanaa QueryOptions {
  bool UseGlobs = true;
  bool RemoveDotSlash = false;
  bool WarnDotSlashMatch = false;
  bool SlashAgnostic = false;
};

/// Represents a set of patterns and their line numbers
class Matcher {
public:
  explicit Matcher(QueryOptions QOpts);

  Error insert(StringRef Pattern, unsigned LineNumber);
  unsigned match(StringRef Query) const;

  bool matchAny(StringRef Query) const { ᱨᱩᱣᱟᱹᱲ match(Query); }

private:
  unsigned matchInternal(StringRef Query) const;
  StringRef findRule(unsigned LineNo) const;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::insert(StringRef Pattern, unsigned LineNumber,
                           bool SlashAgnostic) {
  ᱡᱩᱫᱤ (Pattern.empty())
    ᱨᱩᱣᱟᱹᱲ createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  for (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ᱡᱩᱫᱤ (!CheckRE.isValid(REError))
    ᱨᱩᱣᱟᱹᱲ createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, LineNumber, std::move(CheckRE));
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

unsigned RegexMatcher::match(StringRef Query) const {
  for (const auto &R : reverse(RegExes))
    ᱡᱩᱫᱤ (R.Rg.match(Query))
      ᱨᱩᱣᱟᱹᱲ R.LineNo;
  ᱨᱩᱣᱟᱹᱲ 0;
}

StringRef RegexMatcher::findRule(unsigned LineNo) const {
  for (const auto &R : RegExes)
    ᱡᱩᱫᱤ (R.LineNo == LineNo)
      ᱨᱩᱣᱟᱹᱲ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ᱨᱩᱣᱟᱹᱲ {};
}

Error GlobMatcher::insert(StringRef Pattern, unsigned LineNumber,
                          bool SlashAgnostic) {
  ᱡᱩᱫᱤ (Pattern.empty())
    ᱨᱩᱣᱟᱹᱲ createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ᱡᱩᱫᱤ (auto Err = Res.takeError())
    ᱨᱩᱣᱟᱹᱲ Err;
  Globs.emplace_back(Pattern, LineNumber, std::move(Res.get()));
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

void GlobMatcher::LazyInit() const {
  ᱡᱩᱫᱤ (LLVM_LIKELY(Initialized))
    ᱨᱩᱣᱟᱹᱲ;
  Initialized = true;
  for (const auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    ᱡᱩᱫᱤ (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      ᱡᱩᱫᱤ (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        continue;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) const {
  LazyInit();

  puur_naa_nka Best = -1;
  ᱡᱩᱫᱤ (!PrefixSuffixToGlob.empty()) {
    for (const auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      for (const auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        for (puur_naa_nka Idx : reverse(V)) {
          ᱡᱩᱫᱤ (Best > Idx)
            bha_nga;
          const GlobMatcher::Glob &G = Globs[Idx];
          ᱡᱩᱫᱤ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            bha_nga;
          }
        }
      }
    }
  }

  ᱡᱩᱫᱤ (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    for (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      for (const auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        for (puur_naa_nka Idx : reverse(V)) {
          ᱡᱩᱫᱤ (Best > Idx)
            bha_nga;
          const GlobMatcher::Glob &G = Globs[Idx];
          ᱡᱩᱫᱤ (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            bha_nga;
          }
        }
      }
    }
  }
  ᱨᱩᱣᱟᱹᱲ Best < 0 ? 0 : Globs[Best].LineNo;
}

StringRef GlobMatcher::findRule(unsigned LineNo) const {
  for (const auto &G : Globs)
    ᱡᱩᱫᱤ (G.LineNo == LineNo)
      ᱨᱩᱣᱟᱹᱲ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  ᱨᱩᱣᱟᱹᱲ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ᱡᱩᱫᱤ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ᱵᱟᱝ
    M.emplace<RegexMatcher>();
}

Error Matcher::insert(StringRef Pattern, unsigned LineNumber) {
  ᱨᱩᱣᱟᱹᱲ std::visit(
      [&](auto &V) {
        ᱨᱩᱣᱟᱹᱲ V.insert(Pattern, LineNumber, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) const {
  ᱡᱩᱫᱤ (!Options.RemoveDotSlash)
    ᱨᱩᱣᱟᱹᱲ matchInternal(Query);

  ᱡᱩᱫᱤ (!Options.WarnDotSlashMatch)
    ᱨᱩᱣᱟᱹᱲ matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  ᱡᱩᱫᱤ (FixedQuery == Query)
    ᱨᱩᱣᱟᱹᱲ FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  ᱡᱩᱫᱤ (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  ᱨᱩᱣᱟᱹᱲ std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) const {
  ᱨᱩᱣᱟᱹᱲ std::visit([&](auto &V) -> unsigned { ᱨᱩᱣᱟᱹᱲ V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned LineNo) const {
  ᱨᱩᱣᱟᱹᱲ std::visit([&](auto &V) -> StringRef { ᱨᱩᱣᱟᱹᱲ V.findRule(LineNo); },
                    M);
}
} // namespace

class SpecialCaseList::Section::SectionImpl {
public:
  const Matcher *findMatcher(StringRef Prefix, StringRef Category) const;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(const std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(new SpecialCaseList());
  ᱡᱩᱫᱤ (SCL->createInternal(Paths, FS, Error))
    ᱨᱩᱣᱟᱹᱲ SCL;
  ᱨᱩᱣᱟᱹᱲ nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(const MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(new SpecialCaseList());
  ᱡᱩᱫᱤ (SCL->createInternal(MB, Error))
    ᱨᱩᱣᱟᱹᱲ SCL;
  ᱨᱩᱣᱟᱹᱲ nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ᱡᱩᱫᱤ (auto SCL = create(Paths, FS, Error))
    ᱨᱩᱣᱟᱹᱲ SCL;
  report_fatal_error(Twine(Error));
}

bool SpecialCaseList::createInternal(const std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  for (size_t i = 0; i < Paths.size(); ++i) {
    const auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    ᱡᱩᱫᱤ (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      ᱨᱩᱣᱟᱹᱲ false;
    }
    std::string ParseError;
    ᱡᱩᱫᱤ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      ᱨᱩᱣᱟᱹᱲ false;
    }
  }
  ᱨᱩᱣᱟᱹᱲ true;
}

bool SpecialCaseList::createInternal(const MemoryBuffer *MB,
                                     std::string &Error) {
  ᱡᱩᱫᱤ (!parse(0, MB, Error))
    ᱨᱩᱣᱟᱹᱲ false;
  ᱨᱩᱣᱟᱹᱲ true;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned LineNo, bool UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  ᱡᱩᱫᱤ (auto Err = Section.Impl->SectionMatcher.insert(SectionStr, LineNo)) {
    ᱨᱩᱣᱟᱹᱲ createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(LineNo) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  ᱨᱩᱣᱟᱹᱲ &Section;
}

bool SpecialCaseList::parse(unsigned FileIdx, const MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  ᱡᱩᱫᱤ (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { ᱨᱩᱣᱟᱹᱲ Version >= V; };

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

  auto ErrOrSection = addSection("*", FileIdx, 1, true);
  ᱡᱩᱫᱤ (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    ᱨᱩᱣᱟᱹᱲ false;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  for (line_iterator LineIt(*MB, /*SkipBlanks=*/true, /*CommentMarker=*/'#');
       !LineIt.is_at_eof(); LineIt++) {
    unsigned LineNo = LineIt.line_number();
    StringRef Line = LineIt->trim();
    ᱡᱩᱫᱤ (Line.empty())
      continue;

    // Save section names
    ᱡᱩᱫᱤ (Line.starts_with("[")) {
      ᱡᱩᱫᱤ (!Line.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(LineNo) + ": " + Line)
                .str();
        ᱨᱩᱣᱟᱹᱲ false;
      }

      auto ErrOrSection =
          addSection(Line.drop_front().drop_back(), FileIdx, LineNo, UseGlobs);
      ᱡᱩᱫᱤ (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        ᱨᱩᱣᱟᱹᱲ false;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      continue;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = Line.split(":");
    ᱡᱩᱫᱤ (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(LineNo) + ": '" + Line + "'").str();
      ᱨᱩᱣᱟᱹᱲ false;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ᱡᱩᱫᱤ (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    ᱡᱩᱫᱤ (auto Err = It->second.insert(Pattern, LineNo)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(LineNo) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      ᱨᱩᱣᱟᱹᱲ false;
    }
  }

  ᱨᱩᱣᱟᱹᱲ true;
}

SpecialCaseList::~SpecialCaseList() = default;

bool SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) const {
  auto [FileIdx, LineNo] = inSectionBlame(Section, Prefix, Query, Category);
  ᱨᱩᱣᱟᱹᱲ LineNo;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) const {
  for (const auto &S : reverse(Sections)) {
    ᱡᱩᱫᱤ (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      ᱡᱩᱫᱤ (Blame)
        ᱨᱩᱣᱟᱹᱲ {S.FileIdx, Blame};
    }
  }
  ᱨᱩᱣᱟᱹᱲ NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  bool UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/false})) {}

SpecialCaseList::Section::Section(Section &&) = default;

SpecialCaseList::Section::~Section() = default;

bool SpecialCaseList::Section::matchName(StringRef Name) const {
  ᱨᱩᱣᱟᱹᱲ Impl->SectionMatcher.matchAny(Name);
}

const Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) const {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  ᱡᱩᱫᱤ (I == Entries.end())
    ᱨᱩᱣᱟᱹᱲ nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  ᱡᱩᱫᱤ (II == I->second.end())
    ᱨᱩᱣᱟᱹᱲ nullptr;

  ᱨᱩᱣᱟᱹᱲ &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) const {
  ᱡᱩᱫᱤ (const Matcher *M = Impl->findMatcher(Prefix, Category))
    ᱨᱩᱣᱟᱹᱲ M->match(Query);
  ᱨᱩᱣᱟᱹᱲ 0;
}

bool SpecialCaseList::Section::hasPrefix(StringRef Prefix) const {
  ᱨᱩᱣᱟᱹᱲ Impl->Entries.contains(Prefix);
}

} // namespace llvm
