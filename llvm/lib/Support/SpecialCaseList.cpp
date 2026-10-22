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

#شامل "llvm/Support/SpecialCaseList.h"
#شامل "llvm/ADT/RadixTree.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/iterator_range.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/GlobPattern.h"
#شامل "llvm/Support/LineIterator.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Regex.h"
#شامل "llvm/Support/VirtualFileSystem.h"
#شامل "llvm/Support/WithColor.h"
#شامل <assert.h>
#شامل <memory>
#شامل <mutex>
#شامل <stdio.h>
#شامل <string>
#شامل <system_error>
#شامل <utility>
#شامل <variant>
#شامل <vector>

فضاینام llvm {

فضاینام {

// Lagacy v1 matcher.
رده RegexMatcher {
عمومی:
  Error درج(StringRef Pattern, unsigned سطر_عدد, منطقی SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

خصوصی:
  ساختار Reg {
    Reg(StringRef Name, unsigned سطر_No, Regex &&Rg)
        : Name(Name), سطر_No(سطر_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned سطر_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

رده GlobMatcher {
عمومی:
  Error درج(StringRef Pattern, unsigned سطر_عدد, منطقی SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

خصوصی:
  ساختار Glob {
    Glob(StringRef Name, unsigned سطر_No, GlobPattern &&Pattern)
        : Name(Name), سطر_No(سطر_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned سطر_No;
    GlobPattern Pattern;
  };

  تهی LazyInit() ثابت;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Radix_درخت<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<صحیح, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_درخت<iterator_range<StringRef::const_iterator>,
                    SmallVector<صحیح, 1>>
      SubstrToGlob;

  mutable منطقی Initialized = نادرست;
};

ساختار QueryOptions {
  منطقی UseGlobs = درست;
  منطقی RemoveDotSlash = نادرست;
  منطقی WarnDotSlashMatch = نادرست;
  منطقی SlashAgnostic = نادرست;
};

/// Represents a set of patterns and their line numbers
رده Matcher {
عمومی:
  explicit Matcher(QueryOptions QOpts);

  Error درج(StringRef Pattern, unsigned سطر_عدد);
  unsigned match(StringRef Query) ثابت;

  منطقی matchAny(StringRef Query) ثابت { بازگشت match(Query); }

خصوصی:
  unsigned matchInternal(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::درج(StringRef Pattern, unsigned سطر_عدد,
                           منطقی SlashAgnostic) {
  اگر (Pattern.empty())
    بازگشت createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  برای (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  اگر (!CheckRE.isValid(REError))
    بازگشت createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, سطر_عدد, std::move(CheckRE));
  بازگشت Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ثابت {
  برای (ثابت auto &R : reverse(RegExes))
    اگر (R.Rg.match(Query))
      بازگشت R.سطر_No;
  بازگشت 0;
}

StringRef RegexMatcher::findRule(unsigned سطر_No) ثابت {
  برای (ثابت auto &R : RegExes)
    اگر (R.سطر_No == سطر_No)
      بازگشت R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  بازگشت {};
}

Error GlobMatcher::درج(StringRef Pattern, unsigned سطر_عدد,
                          منطقی SlashAgnostic) {
  اگر (Pattern.empty())
    بازگشت createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  اگر (auto Err = Res.takeError())
    بازگشت Err;
  Globs.emplace_back(Pattern, سطر_عدد, std::move(Res.get()));
  بازگشت Error::success();
}

تهی GlobMatcher::LazyInit() ثابت {
  اگر (LLVM_LIKELY(Initialized))
    بازگشت;
  Initialized = درست;
  برای (ثابت auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    اگر (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      اگر (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        ادامه;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) ثابت {
  LazyInit();

  صحیح Best = -1;
  اگر (!PrefixSuffixToGlob.empty()) {
    برای (ثابت auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      برای (ثابت auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        برای (صحیح Idx : reverse(V)) {
          اگر (Best > Idx)
            بشکن;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          اگر (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            بشکن;
          }
        }
      }
    }
  }

  اگر (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    برای (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      برای (ثابت auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        برای (صحیح Idx : reverse(V)) {
          اگر (Best > Idx)
            بشکن;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          اگر (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            بشکن;
          }
        }
      }
    }
  }
  بازگشت Best < 0 ? 0 : Globs[Best].سطر_No;
}

StringRef GlobMatcher::findRule(unsigned سطر_No) ثابت {
  برای (ثابت auto &G : Globs)
    اگر (G.سطر_No == سطر_No)
      بازگشت G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  بازگشت {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  اگر (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  وگرنه
    M.emplace<RegexMatcher>();
}

Error Matcher::درج(StringRef Pattern, unsigned سطر_عدد) {
  بازگشت std::visit(
      [&](auto &V) {
        بازگشت V.درج(Pattern, سطر_عدد, Options.SlashAgnostic);
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
  اگر (!Options.RemoveDotSlash)
    بازگشت matchInternal(Query);

  اگر (!Options.WarnDotSlashMatch)
    بازگشت matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  اگر (FixedQuery == Query)
    بازگشت FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  اگر (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  بازگشت std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ثابت {
  بازگشت std::visit([&](auto &V) -> unsigned { بازگشت V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned سطر_No) ثابت {
  بازگشت std::visit([&](auto &V) -> StringRef { بازگشت V.findRule(سطر_No); },
                    M);
}
} // namespace

رده SpecialCaseList::Section::SectionImpl {
عمومی:
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
  std::unique_ptr<SpecialCaseList> SCL(جدید SpecialCaseList());
  اگر (SCL->createInternal(Paths, FS, Error))
    بازگشت SCL;
  بازگشت nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جدید SpecialCaseList());
  اگر (SCL->createInternal(MB, Error))
    بازگشت SCL;
  بازگشت nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  اگر (auto SCL = create(Paths, FS, Error))
    بازگشت SCL;
  report_fatal_error(Twine(Error));
}

منطقی SpecialCaseList::createInternal(ثابت std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  برای (size_t i = 0; i < Paths.size(); ++i) {
    ثابت auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    اگر (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      بازگشت نادرست;
    }
    std::string ParseError;
    اگر (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      بازگشت نادرست;
    }
  }
  بازگشت درست;
}

منطقی SpecialCaseList::createInternal(ثابت MemoryBuffer *MB,
                                     std::string &Error) {
  اگر (!parse(0, MB, Error))
    بازگشت نادرست;
  بازگشت درست;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned سطر_No, منطقی UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  اگر (auto Err = Section.Impl->SectionMatcher.درج(SectionStr, سطر_No)) {
    بازگشت createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(سطر_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  بازگشت &Section;
}

منطقی SpecialCaseList::parse(unsigned FileIdx, ثابت MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  اگر (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { بازگشت Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقی UseGlobs = MinVersion(2);
  منطقی RemoveDotSlash = MinVersion(3);
  منطقی WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقی SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, درست);
  اگر (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    بازگشت نادرست;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  برای (سطر_iterator سطر_It(*MB, /*SkipBlanks=*/درست, /*CommentMarker=*/'#');
       !سطر_It.is_at_eof(); سطر_It++) {
    unsigned سطر_No = سطر_It.سطر_عدد_2();
    StringRef سطر = سطر_It->trim();
    اگر (سطر.empty())
      ادامه;

    // Save section names
    اگر (سطر.starts_with("[")) {
      اگر (!سطر.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سطر_No) + ": " + سطر)
                .str();
        بازگشت نادرست;
      }

      auto ErrOrSection =
          addSection(سطر.drop_front().drop_back(), FileIdx, سطر_No, UseGlobs);
      اگر (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        بازگشت نادرست;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      ادامه;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = سطر.split(":");
    اگر (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سطر_No) + ": '" + سطر + "'").str();
      بازگشت نادرست;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    اگر (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    اگر (auto Err = It->second.درج(Pattern, سطر_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سطر_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      بازگشت نادرست;
    }
  }

  بازگشت درست;
}

SpecialCaseList::~SpecialCaseList() = پیشفرض;

منطقی SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  auto [FileIdx, سطر_No] = inSectionBlame(Section, Prefix, Query, Category);
  بازگشت سطر_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  برای (ثابت auto &S : reverse(Sections)) {
    اگر (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      اگر (Blame)
        بازگشت {S.FileIdx, Blame};
    }
  }
  بازگشت NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  منطقی UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/نادرست})) {}

SpecialCaseList::Section::Section(Section &&) = پیشفرض;

SpecialCaseList::Section::~Section() = پیشفرض;

منطقی SpecialCaseList::Section::matchName(StringRef Name) ثابت {
  بازگشت Impl->SectionMatcher.matchAny(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  اگر (I == Entries.end())
    بازگشت nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  اگر (II == I->second.end())
    بازگشت nullptr;

  بازگشت &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  اگر (ثابت Matcher *M = Impl->findMatcher(Prefix, Category))
    بازگشت M->match(Query);
  بازگشت 0;
}

منطقی SpecialCaseList::Section::hasPrefix(StringRef Prefix) ثابت {
  بازگشت Impl->Entries.contains(Prefix);
}

} // namespace llvm
