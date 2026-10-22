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
صنف RegexMatcher {
عام:
  Error أدرج(StringRef Pattern, unsigned سطر_العدد, منطقي SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, unsigned سطر_No, Regex &&Rg)
        : Name(Name), سطر_No(سطر_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned سطر_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

صنف GlobMatcher {
عام:
  Error أدرج(StringRef Pattern, unsigned سطر_العدد, منطقي SlashAgnostic);
  unsigned match(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, unsigned سطر_No, GlobPattern &&Pattern)
        : Name(Name), سطر_No(سطر_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned سطر_No;
    GlobPattern Pattern;
  };

  فراغ LazyInit() ثابت;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_شجرة<iterator_range<StringRef::const_iterator>,
                    Radix_شجرة<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<صحيح, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_شجرة<iterator_range<StringRef::const_iterator>,
                    SmallVector<صحيح, 1>>
      SubstrToGlob;

  mutable منطقي Initialized = خطأ;
};

بنية QueryOptions {
  منطقي UseGlobs = true;
  منطقي RemoveDotSlash = خطأ;
  منطقي WarnDotSlashMatch = خطأ;
  منطقي SlashAgnostic = خطأ;
};

/// Represents a set of patterns and their line numbers
صنف Matcher {
عام:
  explicit Matcher(QueryOptions QOpts);

  Error أدرج(StringRef Pattern, unsigned سطر_العدد);
  unsigned match(StringRef Query) ثابت;

  منطقي matchAny(StringRef Query) ثابت { إرجاع match(Query); }

خاص:
  unsigned matchInternal(StringRef Query) ثابت;
  StringRef findRule(unsigned سطر_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::أدرج(StringRef Pattern, unsigned سطر_العدد,
                           منطقي SlashAgnostic) {
  إذا (Pattern.empty())
    إرجاع createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  لكل (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  إذا (!CheckRE.isValid(REError))
    إرجاع createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, سطر_العدد, std::move(CheckRE));
  إرجاع Error::success();
}

unsigned RegexMatcher::match(StringRef Query) ثابت {
  لكل (ثابت auto &R : reverse(RegExes))
    إذا (R.Rg.match(Query))
      إرجاع R.سطر_No;
  إرجاع 0;
}

StringRef RegexMatcher::findRule(unsigned سطر_No) ثابت {
  لكل (ثابت auto &R : RegExes)
    إذا (R.سطر_No == سطر_No)
      إرجاع R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  إرجاع {};
}

Error GlobMatcher::أدرج(StringRef Pattern, unsigned سطر_العدد,
                          منطقي SlashAgnostic) {
  إذا (Pattern.empty())
    إرجاع createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  إذا (auto Err = Res.takeError())
    إرجاع Err;
  Globs.emplace_back(Pattern, سطر_العدد, std::move(Res.get()));
  إرجاع Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  إذا (LLVM_LIKELY(Initialized))
    إرجاع;
  Initialized = true;
  لكل (ثابت auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    إذا (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      إذا (!Substr.empty()) {
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
  إذا (!PrefixSuffixToGlob.empty()) {
    لكل (ثابت auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      لكل (ثابت auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        لكل (صحيح Idx : reverse(V)) {
          إذا (Best > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.Pattern.match(Query)) {
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

  إذا (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    لكل (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      لكل (ثابت auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        لكل (صحيح Idx : reverse(V)) {
          إذا (Best > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.Pattern.match(Query)) {
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
  إرجاع Best < 0 ? 0 : Globs[Best].سطر_No;
}

StringRef GlobMatcher::findRule(unsigned سطر_No) ثابت {
  لكل (ثابت auto &G : Globs)
    إذا (G.سطر_No == سطر_No)
      إرجاع G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  إرجاع {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  إذا (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  وإلا
    M.emplace<RegexMatcher>();
}

Error Matcher::أدرج(StringRef Pattern, unsigned سطر_العدد) {
  إرجاع std::visit(
      [&](auto &V) {
        إرجاع V.أدرج(Pattern, سطر_العدد, Options.SlashAgnostic);
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
  إذا (!Options.RemoveDotSlash)
    إرجاع matchInternal(Query);

  إذا (!Options.WarnDotSlashMatch)
    إرجاع matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  إذا (FixedQuery == Query)
    إرجاع FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  إذا (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  إرجاع std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) ثابت {
  إرجاع std::visit([&](auto &V) -> unsigned { إرجاع V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned سطر_No) ثابت {
  إرجاع std::visit([&](auto &V) -> StringRef { إرجاع V.findRule(سطر_No); },
                    M);
}
} // namespace

صنف SpecialCaseList::Section::SectionImpl {
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
  إذا (SCL->createInternal(Paths, FS, Error))
    إرجاع SCL;
  إرجاع nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->createInternal(MB, Error))
    إرجاع SCL;
  إرجاع nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  إذا (auto SCL = create(Paths, FS, Error))
    إرجاع SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::createInternal(ثابت std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  لكل (size_t i = 0; i < Paths.size(); ++i) {
    ثابت auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    إذا (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      إرجاع خطأ;
    }
    std::string ParseError;
    إذا (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      إرجاع خطأ;
    }
  }
  إرجاع true;
}

منطقي SpecialCaseList::createInternal(ثابت MemoryBuffer *MB,
                                     std::string &Error) {
  إذا (!parse(0, MB, Error))
    إرجاع خطأ;
  إرجاع true;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned سطر_No, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  إذا (auto Err = Section.Impl->SectionMatcher.أدرج(SectionStr, سطر_No)) {
    إرجاع createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(سطر_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  إرجاع &Section;
}

منطقي SpecialCaseList::parse(unsigned FileIdx, ثابت MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  إذا (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { إرجاع Version >= V; };

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

  auto ErrOrSection = addSection("*", FileIdx, 1, true);
  إذا (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    إرجاع خطأ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  لكل (سطر_iterator سطر_It(*MB, /*SkipBlanks=*/true, /*CommentMarker=*/'#');
       !سطر_It.is_at_eof(); سطر_It++) {
    unsigned سطر_No = سطر_It.سطر_العدد_2();
    StringRef سطر = سطر_It->trim();
    إذا (سطر.empty())
      استمر;

    // Save section names
    إذا (سطر.starts_with("[")) {
      إذا (!سطر.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سطر_No) + ": " + سطر)
                .str();
        إرجاع خطأ;
      }

      auto ErrOrSection =
          addSection(سطر.drop_front().drop_back(), FileIdx, سطر_No, UseGlobs);
      إذا (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        إرجاع خطأ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = سطر.split(":");
    إذا (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سطر_No) + ": '" + سطر + "'").str();
      إرجاع خطأ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    إذا (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    إذا (auto Err = It->second.أدرج(Pattern, سطر_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سطر_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      إرجاع خطأ;
    }
  }

  إرجاع true;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  auto [FileIdx, سطر_No] = inSectionBlame(Section, Prefix, Query, Category);
  إرجاع سطر_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ثابت {
  لكل (ثابت auto &S : reverse(Sections)) {
    إذا (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      إذا (Blame)
        إرجاع {S.FileIdx, Blame};
    }
  }
  إرجاع NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/خطأ})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::matchName(StringRef Name) ثابت {
  إرجاع Impl->SectionMatcher.matchAny(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  إذا (I == Entries.end())
    إرجاع nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  إذا (II == I->second.end())
    إرجاع nullptr;

  إرجاع &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  إذا (ثابت Matcher *M = Impl->findMatcher(Prefix, Category))
    إرجاع M->match(Query);
  إرجاع 0;
}

منطقي SpecialCaseList::Section::hasPrefix(StringRef Prefix) ثابت {
  إرجاع Impl->Entries.contains(Prefix);
}

} // namespace llvm
