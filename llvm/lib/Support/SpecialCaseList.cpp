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

#ضمن "llvm/Support/SpecialCaseList.h"
#ضمن "llvm/ADT/RadixTree.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/iterator_range.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/GlobPattern.h"
#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Regex.h"
#ضمن "llvm/Support/VirtualFileSystem.h"
#ضمن "llvm/Support/WithColor.h"
#ضمن <assert.h>
#ضمن <memory>
#ضمن <mutex>
#ضمن <stdio.h>
#ضمن <string>
#ضمن <system_error>
#ضمن <utility>
#ضمن <variant>
#ضمن <vector>

نطاق llvm {

نطاق {

// Lagacy v1 matcher.
מחלקה RegexMatcher {
عام:
  Error insert(StringRef Pattern, unsigned LineNumber, منطقي SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned LineNo) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, unsigned LineNo, Regex &&Rg)
        : Name(Name), LineNo(LineNo), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned LineNo;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

מחלקה GlobMatcher {
عام:
  Error insert(StringRef Pattern, unsigned LineNumber, منطقي SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned LineNo) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, unsigned LineNo, GlobPattern &&Pattern)
        : Name(Name), LineNo(LineNo), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned LineNo;
    GlobPattern Pattern;
  };

  فراغ LazyInit() ثابت;

  std::vector<GlobMatcher::Glob> Globs;

  mutable RadixTree<iterator_range<StringRef::const_iterator>,
                    RadixTree<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<صحيح, 1>>>
      PrefixSuffixToGlob;

  mutable RadixTree<iterator_range<StringRef::const_iterator>,
                    SmallVector<صحيح, 1>>
      SubstrToGlob;

  mutable منطقي Initialized = שקר;
};

بنية QueryOptions {
  منطقي UseGlobs = אמת;
  منطقي RemoveDotSlash = שקר;
  منطقي WarnDotSlashMatch = שקר;
  منطقي SlashAgnostic = שקר;
};

/// Represents a set of patterns and their line numbers
מחלקה Matcher {
عام:
  explicit Matcher(QueryOptions QOpts);

  Error insert(StringRef Pattern, unsigned LineNumber);
  unsigned match(StringRef Query) ثابت;

  منطقي matchAny(StringRef Query) ثابت { החזר match(Query); }

خاص:
  unsigned matchInternal(StringRef Query) ثابت;
  StringRef findRule(unsigned LineNo) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::insert(StringRef Pattern, unsigned LineNumber,
                           منطقي SlashAgnostic) {
  אם (Pattern.empty())
    החזר createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  עבור (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  אם (!CheckRE.isValid(REError))
    החזר createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, LineNumber, std::move(CheckRE));
  החזר Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ثابت {
  עבור (ثابت auto &R : reverse(RegExes))
    אם (R.Rg.match(Query))
      החזר R.LineNo;
  החזר 0;
}

StringRef RegexMatcher::findRule(unsigned LineNo) ثابت {
  עבור (ثابت auto &R : RegExes)
    אם (R.LineNo == LineNo)
      החזר R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  החזר {};
}

Error GlobMatcher::insert(StringRef Pattern, unsigned LineNumber,
                          منطقي SlashAgnostic) {
  אם (Pattern.empty())
    החזר createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  אם (auto Err = Res.takeError())
    החזר Err;
  Globs.emplace_back(Pattern, LineNumber, std::move(Res.get()));
  החזר Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  אם (LLVM_LIKELY(Initialized))
    החזר;
  Initialized = אמת;
  עבור (ثابت auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    אם (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      אם (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        استمر;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ثابت {
  LazyInit();

  صحيح Best = -1;
  אם (!PrefixSuffixToGlob.empty()) {
    עבור (ثابت auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      עבור (ثابت auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        עבור (صحيح Idx : reverse(V)) {
          אם (Best > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          אם (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            اكسر;
          }
        }
      }
    }
  }

  אם (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    עבור (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      עבור (ثابت auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        עבור (صحيح Idx : reverse(V)) {
          אם (Best > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          אם (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            اكسر;
          }
        }
      }
    }
  }
  החזר Best < 0 ? 0 : Globs[Best].LineNo;
}

StringRef GlobMatcher::findRule(unsigned LineNo) ثابت {
  עבור (ثابت auto &G : Globs)
    אם (G.LineNo == LineNo)
      החזר G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  החזר {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  אם (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  אחרת
    M.emplace<RegexMatcher>();
}

Error Matcher::insert(StringRef Pattern, unsigned LineNumber) {
  החזר std::visit(
      [&](auto &V) {
        החזר V.insert(Pattern, LineNumber, Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) ثابت {
  אם (!Options.RemoveDotSlash)
    החזר matchInternal(Query);

  אם (!Options.WarnDotSlashMatch)
    החזר matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  אם (FixedQuery == Query)
    החזר FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  אם (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  החזר std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ثابت {
  החזר std::visit([&](auto &V) -> unsigned { החזר V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned LineNo) ثابت {
  החזר std::visit([&](auto &V) -> StringRef { החזר V.findRule(LineNo); },
                    M);
}
} // namespace

מחלקה SpecialCaseList::Section::SectionImpl {
عام:
  ثابت Matcher *findMatcher(StringRef Prefix, StringRef Category) ثابت;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  אם (SCL->createInternal(Paths, FS, Error))
    החזר SCL;
  החזר nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  אם (SCL->createInternal(MB, Error))
    החזר SCL;
  החזר nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  אם (auto SCL = create(Paths, FS, Error))
    החזר SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::createInternal(ثابت std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  עבור (size_t i = 0; i < Paths.size(); ++i) {
    ثابت auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    אם (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      החזר שקר;
    }
    std::string ParseError;
    אם (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      החזר שקר;
    }
  }
  החזר אמת;
}

منطقي SpecialCaseList::createInternal(ثابت MemoryBuffer *MB,
                                     std::string &Error) {
  אם (!parse(0, MB, Error))
    החזר שקר;
  החזר אמת;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned LineNo, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  אם (auto Err = Section.Impl->SectionMatcher.insert(SectionStr, LineNo)) {
    החזר createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(LineNo) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  החזר &Section;
}

منطقي SpecialCaseList::parse(unsigned FileIdx, ثابت MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  אם (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { החזר Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقي UseGlobs = MinVersion(2);
  منطقي RemoveDotSlash = MinVersion(3);
  منطقي WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقي SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, אמת);
  אם (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    החזר שקר;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  עבור (line_iterator LineIt(*MB, /*SkipBlanks=*/אמת, /*CommentMarker=*/'#');
       !LineIt.is_at_eof(); LineIt++) {
    unsigned LineNo = LineIt.line_number();
    StringRef Line = LineIt->trim();
    אם (Line.empty())
      استمر;

    // Save section names
    אם (Line.starts_with("[")) {
      אם (!Line.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(LineNo) + ": " + Line)
                .str();
        החזר שקר;
      }

      auto ErrOrSection =
          addSection(Line.drop_front().drop_back(), FileIdx, LineNo, UseGlobs);
      אם (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        החזר שקר;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = Line.split(":");
    אם (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(LineNo) + ": '" + Line + "'").str();
      החזר שקר;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    אם (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    אם (auto Err = It->second.insert(Pattern, LineNo)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(LineNo) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      החזר שקר;
    }
  }

  החזר אמת;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  auto [FileIdx, LineNo] = inSectionBlame(Section, Prefix, Query, Category);
  החזר LineNo;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  עבור (ثابت auto &S : reverse(Sections)) {
    אם (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      אם (Blame)
        החזר {S.FileIdx, Blame};
    }
  }
  החזר NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/שקר})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::matchName(StringRef Name) ثابت {
  החזר Impl->SectionMatcher.matchAny(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  אם (I == Entries.end())
    החזר nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  אם (II == I->second.end())
    החזר nullptr;

  החזר &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  אם (ثابت Matcher *M = Impl->findMatcher(Prefix, Category))
    החזר M->match(Query);
  החזר 0;
}

منطقي SpecialCaseList::Section::hasPrefix(StringRef Prefix) ثابت {
  החזר Impl->Entries.contains(Prefix);
}

} // namespace llvm
