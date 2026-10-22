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
  Error ورننباسه(StringRef نمونه, بینښه کرښه_شمېره, منطقي SlashAgnostic);
  بینښه سمون(StringRef Query) ثابت;
  StringRef پیدا_کړه_قاعده(بینښه کرښه_No) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, بینښه کرښه_No, Regex &&Rg)
        : Name(Name), کرښه_No(کرښه_No), Rg(std::move(Rg)) {}
    StringRef Name;
    بینښه کرښه_No;
    Regex Rg;
  };

  std::ویکټور<Reg> RegExes;
};

صنف GlobMatcher {
عام:
  Error ورننباسه(StringRef نمونه, بینښه کرښه_شمېره, منطقي SlashAgnostic);
  بینښه سمون(StringRef Query) ثابت;
  StringRef پیدا_کړه_قاعده(بینښه کرښه_No) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, بینښه کرښه_No, Glob_نمونه &&نمونه)
        : Name(Name), کرښه_No(کرښه_No), نمونه(std::move(نمونه)) {}
    StringRef Name;
    بینښه کرښه_No;
    Glob_نمونه نمونه;
  };

  فراغ LazyInit() ثابت;

  std::ویکټور<GlobMatcher::Glob> Globs;

  بدلیدونکی Radix_ونه<iterator_range<StringRef::const_iterator>,
                    Radix_ونه<iterator_range<StringRef::const_سرچپه_iterator>,
                              Small_ویکټور<صحيح, 1>>>
      مختاړی_Suffix_منزل_Glob;

  بدلیدونکی Radix_ونه<iterator_range<StringRef::const_iterator>,
                    Small_ویکټور<صحيح, 1>>
      Substr_منزل_Glob;

  بدلیدونکی منطقي Initialized = خطأ;
};

بنية QueryOptions {
  منطقي UseGlobs = ریښتیا;
  منطقي RemoveDotSlash = خطأ;
  منطقي Warn_Dot_Slash_سمون = خطأ;
  منطقي SlashAgnostic = خطأ;
};

/// Represents a set of patterns and their line numbers
صنف Matcher {
عام:
  څرګند Matcher(QueryOptions QOpts);

  Error ورننباسه(StringRef نمونه, بینښه کرښه_شمېره);
  بینښه سمون(StringRef Query) ثابت;

  منطقي سمون_Any(StringRef Query) ثابت { أرجع سمون(Query); }

خاص:
  بینښه سمون_دننی(StringRef Query) ثابت;
  StringRef پیدا_کړه_قاعده(بینښه کرښه_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  بدلیدونکی std::once_flag Warned;
};

Error RegexMatcher::ورننباسه(StringRef نمونه, بینښه کرښه_شمېره,
                           منطقي SlashAgnostic) {
  إذا (نمونه.تشه())
    أرجع createStringError(errc::invalid_دلیل,
                             "Supplied regex was blank");

  // Replace * with .*
  خپلکاری Regexp = نمونه.str();
  لكل (size_t pos = 0; (pos = Regexp.پیدا_کړه('*', pos)) != std::string::npos;
       pos += مزي_اوږدوالی(".*")) {
    Regexp.replace(pos, مزي_اوږدوالی("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  إذا (!CheckRE.isValid(REError))
    أرجع createStringError(errc::invalid_دلیل, REError);

  RegExes.emplace_شا(نمونه, کرښه_شمېره, std::move(CheckRE));
  أرجع Error::success();
}

بینښه RegexMatcher::سمون(StringRef Query) ثابت {
  لكل (ثابت خپلکاری &R : سرچپه(RegExes))
    إذا (R.Rg.سمون(Query))
      أرجع R.کرښه_No;
  أرجع 0;
}

StringRef RegexMatcher::پیدا_کړه_قاعده(بینښه کرښه_No) ثابت {
  لكل (ثابت خپلکاری &R : RegExes)
    إذا (R.کرښه_No == کرښه_No)
      أرجع R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  أرجع {};
}

Error GlobMatcher::ورننباسه(StringRef نمونه, بینښه کرښه_شمېره,
                          منطقي SlashAgnostic) {
  إذا (نمونه.تشه())
    أرجع createStringError(errc::invalid_دلیل, "Supplied glob was blank");

  خپلکاری Res =
      Glob_نمونه::create(نمونه, /*MaxSubPatterns=*/1024, SlashAgnostic);
  إذا (خپلکاری Err = Res.takeError())
    أرجع Err;
  Globs.emplace_شا(نمونه, کرښه_شمېره, std::move(Res.get()));
  أرجع Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  إذا (LLVM_LIKELY(Initialized))
    أرجع;
  Initialized = ریښتیا;
  لكل (ثابت خپلکاری &[Idx, G] : enumerate(Globs)) {
    StringRef مختاړی = G.نمونه.مختاړی_2();
    StringRef Suffix = G.نمونه.suffix();

    إذا (Suffix.تشه() && مختاړی.تشه()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.نمونه.longest_substr();
      إذا (!Substr.تشه()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        خپلکاری &V = Substr_منزل_Glob.emplace(Substr).لومړی_2->دوهم_2;
        V.emplace_شا(Idx);
        استمر;
      }
    }

    خپلکاری &SToGlob = مختاړی_Suffix_منزل_Glob.emplace(مختاړی).لومړی_2->دوهم_2;
    خپلکاری &V = SToGlob.emplace(سرچپه(Suffix)).لومړی_2->دوهم_2;
    V.emplace_شا(Idx);
  }
}

بینښه GlobMatcher::سمون(StringRef Query) ثابت {
  LazyInit();

  صحيح غوره = -1;
  إذا (!مختاړی_Suffix_منزل_Glob.تشه()) {
    لكل (ثابت خپلکاری &[_, SToGlob] : مختاړی_Suffix_منزل_Glob.پیدا_کړه_prefixes(Query)) {
      لكل (ثابت خپلکاری &[_, V] : SToGlob.پیدا_کړه_prefixes(سرچپه(Query))) {
        لكل (صحيح Idx : سرچپه(V)) {
          إذا (غوره > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونه.سمون(Query)) {
            غوره = Idx;
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

  إذا (!Substr_منزل_Glob.تشه()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    لكل (StringRef Q = Query; !Q.تشه(); Q = Q.drop_front()) {
      لكل (ثابت خپلکاری &[_, V] : Substr_منزل_Glob.پیدا_کړه_prefixes(Q)) {
        لكل (صحيح Idx : سرچپه(V)) {
          إذا (غوره > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونه.سمون(Query)) {
            غوره = Idx;
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
  أرجع غوره < 0 ? 0 : Globs[غوره].کرښه_No;
}

StringRef GlobMatcher::پیدا_کړه_قاعده(بینښه کرښه_No) ثابت {
  لكل (ثابت خپلکاری &G : Globs)
    إذا (G.کرښه_No == کرښه_No)
      أرجع G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  أرجع {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  إذا (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  وإلا
    M.emplace<RegexMatcher>();
}

Error Matcher::ورننباسه(StringRef نمونه, بینښه کرښه_شمېره) {
  أرجع std::لیدنه(
      [&](خپلکاری &V) {
        أرجع V.ورننباسه(نمونه, کرښه_شمېره, Options.SlashAgnostic);
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
بینښه Matcher::سمون(StringRef Query) ثابت {
  إذا (!Options.RemoveDotSlash)
    أرجع سمون_دننی(Query);

  إذا (!Options.Warn_Dot_Slash_سمون)
    أرجع سمون_دننی(llvm::sys::لار_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::لار_2::remove_leading_dotslash(Query);
  بینښه FixedMatched = سمون_دننی(FixedQuery);
  إذا (FixedQuery == Query)
    أرجع FixedMatched;

  بینښه Original_سمون = سمون_دننی(Query);
  إذا (Original_سمون > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << پیدا_کړه_قاعده(Original_سمون) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  أرجع std::max(Original_سمون, FixedMatched);
}

بینښه Matcher::سمون_دننی(StringRef Query) ثابت {
  أرجع std::لیدنه([&](خپلکاری &V) -> بینښه { أرجع V.سمون(Query); }, M);
}

StringRef Matcher::پیدا_کړه_قاعده(بینښه کرښه_No) ثابت {
  أرجع std::لیدنه([&](خپلکاری &V) -> StringRef { أرجع V.پیدا_کړه_قاعده(کرښه_No); },
                    M);
}
} // namespace

صنف SpecialCaseList::Section::SectionImpl {
عام:
  ثابت Matcher *پیدا_کړه_Matcher(StringRef مختاړی, StringRef Category) ثابت;

  کارول SectionEntries = StringMap<StringMap<Matcher>>;

  څرګند SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::ویکټور<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_دننی(Paths, FS, Error))
    أرجع SCL;
  أرجع تش_اشاره;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت حافظه_بفر *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_دننی(MB, Error))
    أرجع SCL;
  أرجع تش_اشاره;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::ویکټور<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  إذا (خپلکاری SCL = create(Paths, FS, Error))
    أرجع SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::create_دننی(ثابت std::ویکټور<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  لكل (size_t i = 0; i < Paths.اندازه_2(); ++i) {
    ثابت خپلکاری &لار = Paths[i];
    ErrorOr<std::unique_ptr<حافظه_بفر>> FileOrErr =
        VFS.get_بفر_For_File(لار);
    إذا (std::error_کوډ EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + لار + "': " + EC.پیغام_2()).str();
      أرجع خطأ;
    }
    std::string ParseError;
    إذا (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + لار + "': " + ParseError).str();
      أرجع خطأ;
    }
  }
  أرجع ریښتیا;
}

منطقي SpecialCaseList::create_دننی(ثابت حافظه_بفر *MB,
                                     std::string &Error) {
  إذا (!parse(0, MB, Error))
    أرجع خطأ;
  أرجع ریښتیا;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::جمع_کړه_Section(StringRef SectionStr, بینښه FileNo,
                            بینښه کرښه_No, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_شا(SectionStr, FileNo, UseGlobs);
  خپلکاری &Section = Sections.شا();

  إذا (خپلکاری Err = Section.Impl->SectionMatcher.ورننباسه(SectionStr, کرښه_No)) {
    أرجع createStringError(errc::invalid_دلیل,
                             "malformed section at line " + Twine(کرښه_No) +
                                 ": '" + SectionStr +
                                 "': " + منزل_String(std::move(Err)));
  }

  أرجع &Section;
}

منطقي SpecialCaseList::parse(بینښه FileIdx, ثابت حافظه_بفر *MB,
                            std::string &Error) {
  بینښه اوږد اوږد نسخه = 2;

  StringRef سرلیک = MB->get_بفر();
  إذا (سرلیک.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(سرلیک, 10, نسخه);

  خپلکاری Min_نسخه = [&](بینښه V) { أرجع نسخه >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقي UseGlobs = Min_نسخه(2);
  منطقي RemoveDotSlash = Min_نسخه(3);
  منطقي WarnDotSlash = Min_نسخه(4) && !Min_نسخه(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقي SlashAgnostic = Min_نسخه(4) && llvm::sys::لار_2::is_style_windows(
                                            llvm::sys::لار_2::Style::native);

  خپلکاری ErrOrSection = جمع_کړه_Section("*", FileIdx, 1, ریښتیا);
  إذا (خپلکاری Err = ErrOrSection.takeError()) {
    Error = منزل_String(std::move(Err));
    أرجع خطأ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef لار_Prefixes[] = {"src", "!src", "mainfile", "source"};

  لكل (کرښه_iterator کرښه_It(*MB, /*SkipBlanks=*/ریښتیا, /*CommentMarker=*/'#');
       !کرښه_It.is_at_eof(); کرښه_It++) {
    بینښه کرښه_No = کرښه_It.کرښه_شمېره_2();
    StringRef کرښه = کرښه_It->trim();
    إذا (کرښه.تشه())
      استمر;

    // Save section names
    إذا (کرښه.starts_with("[")) {
      إذا (!کرښه.پایونه_with("]")) {
        Error =
            ("malformed section header on line " + Twine(کرښه_No) + ": " + کرښه)
                .str();
        أرجع خطأ;
      }

      خپلکاری ErrOrSection =
          جمع_کړه_Section(کرښه.drop_front().drop_شا(), FileIdx, کرښه_No, UseGlobs);
      إذا (خپلکاری Err = ErrOrSection.takeError()) {
        Error = منزل_String(std::move(Err));
        أرجع خطأ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    خپلکاری [مختاړی, Postfix] = کرښه.وېشل(":");
    إذا (Postfix.تشه()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(کرښه_No) + ": '" + کرښه + "'").str();
      أرجع خطأ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    إذا (llvm::is_contained(لار_Prefixes, مختاړی)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_سمون = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    خپلکاری [نمونه, Category] = Postfix.وېشل("=");
    خپلکاری [It, _] = CurrentImpl->Entries[مختاړی].try_emplace(Category, QOpts);
    نمونه = نمونه.copy(StrAlloc);
    إذا (خپلکاری Err = It->دوهم_2.ورننباسه(نمونه, کرښه_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(کرښه_No) + ": '" + نمونه + "': " + منزل_String(std::move(Err)))
              .str();
      أرجع خطأ;
    }
  }

  أرجع ریښتیا;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::دننه_Section(StringRef Section, StringRef مختاړی,
                                StringRef Query, StringRef Category) ثابت {
  خپلکاری [FileIdx, کرښه_No] = دننه_Section_Blame(Section, مختاړی, Query, Category);
  أرجع کرښه_No;
}

std::pair<بینښه, بینښه>
SpecialCaseList::دننه_Section_Blame(StringRef Section, StringRef مختاړی,
                                StringRef Query, StringRef Category) ثابت {
  لكل (ثابت خپلکاری &S : سرچپه(Sections)) {
    إذا (S.Impl->SectionMatcher.سمون_Any(Section)) {
      بینښه Blame = S.get_وروستی_سمون(مختاړی, Query, Category);
      إذا (Blame)
        أرجع {S.FileIdx, Blame};
    }
  }
  أرجع Not_وموندل;
}

SpecialCaseList::Section::Section(StringRef Str, بینښه FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/خطأ})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::سمون_Name(StringRef Name) ثابت {
  أرجع Impl->SectionMatcher.سمون_Any(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::پیدا_کړه_Matcher(StringRef مختاړی,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.پیدا_کړه(مختاړی);
  إذا (I == Entries.پای_ټکی())
    أرجع تش_اشاره;
  StringMap<Matcher>::const_iterator II = I->دوهم_2.پیدا_کړه(Category);
  إذا (II == I->دوهم_2.پای_ټکی())
    أرجع تش_اشاره;

  أرجع &II->دوهم_2;
}

بینښه SpecialCaseList::Section::get_وروستی_سمون(StringRef مختاړی,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  إذا (ثابت Matcher *M = Impl->پیدا_کړه_Matcher(مختاړی, Category))
    أرجع M->سمون(Query);
  أرجع 0;
}

منطقي SpecialCaseList::Section::has_مختاړی(StringRef مختاړی) ثابت {
  أرجع Impl->Entries.لري(مختاړی);
}

} // namespace llvm
