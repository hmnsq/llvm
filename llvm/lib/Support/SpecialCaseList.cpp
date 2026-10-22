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
  struct Reg {
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
  struct Glob {
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
                              SmallVector<整数, 1>>>
      PrefixSuffixToGlob;

  mutable RadixTree<iterator_range<StringRef::const_iterator>,
                    SmallVector<整数, 1>>
      SubstrToGlob;

  mutable bool Initialized = false;
};

struct QueryOptions {
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

  bool matchAny(StringRef Query) const { 返回 match(Query); }

private:
  unsigned matchInternal(StringRef Query) const;
  StringRef findRule(unsigned LineNo) const;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::insert(StringRef Pattern, unsigned LineNumber,
                           bool SlashAgnostic) {
  如果 (Pattern.empty())
    返回 createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  为 (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  如果 (!CheckRE.isValid(REError))
    返回 createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, LineNumber, std::move(CheckRE));
  返回 Error::success();
}

unsigned RegexMatcher::match(StringRef Query) const {
  为 (const auto &R : reverse(RegExes))
    如果 (R.Rg.match(Query))
      返回 R.LineNo;
  返回 0;
}

StringRef RegexMatcher::findRule(unsigned LineNo) const {
  为 (const auto &R : RegExes)
    如果 (R.LineNo == LineNo)
      返回 R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  返回 {};
}

Error GlobMatcher::insert(StringRef Pattern, unsigned LineNumber,
                          bool SlashAgnostic) {
  如果 (Pattern.empty())
    返回 createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  如果 (auto Err = Res.takeError())
    返回 Err;
  Globs.emplace_back(Pattern, LineNumber, std::move(Res.get()));
  返回 Error::success();
}

void GlobMatcher::LazyInit() const {
  如果 (LLVM_LIKELY(Initialized))
    返回;
  Initialized = true;
  为 (const auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    如果 (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      如果 (!Substr.empty()) {
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

  整数 Best = -1;
  如果 (!PrefixSuffixToGlob.empty()) {
    为 (const auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      为 (const auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        为 (整数 Idx : reverse(V)) {
          如果 (Best > Idx)
            break;
          const GlobMatcher::Glob &G = Globs[Idx];
          如果 (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            break;
          }
        }
      }
    }
  }

  如果 (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    为 (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      为 (const auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        为 (整数 Idx : reverse(V)) {
          如果 (Best > Idx)
            break;
          const GlobMatcher::Glob &G = Globs[Idx];
          如果 (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            break;
          }
        }
      }
    }
  }
  返回 Best < 0 ? 0 : Globs[Best].LineNo;
}

StringRef GlobMatcher::findRule(unsigned LineNo) const {
  为 (const auto &G : Globs)
    如果 (G.LineNo == LineNo)
      返回 G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  返回 {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  如果 (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  否则
    M.emplace<RegexMatcher>();
}

Error Matcher::insert(StringRef Pattern, unsigned LineNumber) {
  返回 std::visit(
      [&](auto &V) {
        返回 V.insert(Pattern, LineNumber, Options.SlashAgnostic);
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
  如果 (!Options.RemoveDotSlash)
    返回 matchInternal(Query);

  如果 (!Options.WarnDotSlashMatch)
    返回 matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  如果 (FixedQuery == Query)
    返回 FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  如果 (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  返回 std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) const {
  返回 std::visit([&](auto &V) -> unsigned { 返回 V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned LineNo) const {
  返回 std::visit([&](auto &V) -> StringRef { 返回 V.findRule(LineNo); },
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
  如果 (SCL->createInternal(Paths, FS, Error))
    返回 SCL;
  返回 nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(const MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(new SpecialCaseList());
  如果 (SCL->createInternal(MB, Error))
    返回 SCL;
  返回 nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  如果 (auto SCL = create(Paths, FS, Error))
    返回 SCL;
  report_fatal_error(Twine(Error));
}

bool SpecialCaseList::createInternal(const std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  为 (size_t i = 0; i < Paths.size(); ++i) {
    const auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    如果 (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      返回 false;
    }
    std::string ParseError;
    如果 (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      返回 false;
    }
  }
  返回 true;
}

bool SpecialCaseList::createInternal(const MemoryBuffer *MB,
                                     std::string &Error) {
  如果 (!parse(0, MB, Error))
    返回 false;
  返回 true;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned LineNo, bool UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  如果 (auto Err = Section.Impl->SectionMatcher.insert(SectionStr, LineNo)) {
    返回 createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(LineNo) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  返回 &Section;
}

bool SpecialCaseList::parse(unsigned FileIdx, const MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  如果 (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { 返回 Version >= V; };

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
  如果 (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    返回 false;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  为 (line_iterator LineIt(*MB, /*SkipBlanks=*/true, /*CommentMarker=*/'#');
       !LineIt.is_at_eof(); LineIt++) {
    unsigned LineNo = LineIt.line_number();
    StringRef Line = LineIt->trim();
    如果 (Line.empty())
      continue;

    // Save section names
    如果 (Line.starts_with("[")) {
      如果 (!Line.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(LineNo) + ": " + Line)
                .str();
        返回 false;
      }

      auto ErrOrSection =
          addSection(Line.drop_front().drop_back(), FileIdx, LineNo, UseGlobs);
      如果 (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        返回 false;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      continue;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = Line.split(":");
    如果 (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(LineNo) + ": '" + Line + "'").str();
      返回 false;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    如果 (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    如果 (auto Err = It->second.insert(Pattern, LineNo)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(LineNo) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      返回 false;
    }
  }

  返回 true;
}

SpecialCaseList::~SpecialCaseList() = default;

bool SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) const {
  auto [FileIdx, LineNo] = inSectionBlame(Section, Prefix, Query, Category);
  返回 LineNo;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) const {
  为 (const auto &S : reverse(Sections)) {
    如果 (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      如果 (Blame)
        返回 {S.FileIdx, Blame};
    }
  }
  返回 NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  bool UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/false})) {}

SpecialCaseList::Section::Section(Section &&) = default;

SpecialCaseList::Section::~Section() = default;

bool SpecialCaseList::Section::matchName(StringRef Name) const {
  返回 Impl->SectionMatcher.matchAny(Name);
}

const Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) const {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  如果 (I == Entries.end())
    返回 nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  如果 (II == I->second.end())
    返回 nullptr;

  返回 &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) const {
  如果 (const Matcher *M = Impl->findMatcher(Prefix, Category))
    返回 M->match(Query);
  返回 0;
}

bool SpecialCaseList::Section::hasPrefix(StringRef Prefix) const {
  返回 Impl->Entries.contains(Prefix);
}

} // namespace llvm
