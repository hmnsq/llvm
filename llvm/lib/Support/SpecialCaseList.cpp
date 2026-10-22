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
  Error درج(StringRef الگو, بیعلامت سطر_عدد, منطقي SlashAgnostic);
  بیعلامت تطابق(StringRef Query) ثابت;
  StringRef پیدا_کن_قاعده(بیعلامت سطر_No) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, بیعلامت سطر_No, Regex &&Rg)
        : Name(Name), سطر_No(سطر_No), Rg(std::move(Rg)) {}
    StringRef Name;
    بیعلامت سطر_No;
    Regex Rg;
  };

  std::وکتور<Reg> RegExes;
};

صنف GlobMatcher {
عام:
  Error درج(StringRef الگو, بیعلامت سطر_عدد, منطقي SlashAgnostic);
  بیعلامت تطابق(StringRef Query) ثابت;
  StringRef پیدا_کن_قاعده(بیعلامت سطر_No) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, بیعلامت سطر_No, Glob_الگو &&الگو)
        : Name(Name), سطر_No(سطر_No), الگو(std::move(الگو)) {}
    StringRef Name;
    بیعلامت سطر_No;
    Glob_الگو الگو;
  };

  فراغ LazyInit() ثابت;

  std::وکتور<GlobMatcher::Glob> Globs;

  تغییرپذیر Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Radix_درخت<iterator_range<StringRef::const_معکوس_iterator>,
                              Small_وکتور<صحيح, 1>>>
      پیشوند_Suffix_مقصد_Glob;

  تغییرپذیر Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Small_وکتور<صحيح, 1>>
      Substr_مقصد_Glob;

  تغییرپذیر منطقي Initialized = خطأ;
};

بنية QueryOptions {
  منطقي UseGlobs = درست;
  منطقي RemoveDotSlash = خطأ;
  منطقي Warn_Dot_Slash_تطابق = خطأ;
  منطقي SlashAgnostic = خطأ;
};

/// Represents a set of patterns and their line numbers
صنف Matcher {
عام:
  صریح Matcher(QueryOptions QOpts);

  Error درج(StringRef الگو, بیعلامت سطر_عدد);
  بیعلامت تطابق(StringRef Query) ثابت;

  منطقي تطابق_Any(StringRef Query) ثابت { أرجع تطابق(Query); }

خاص:
  بیعلامت تطابق_داخلی(StringRef Query) ثابت;
  StringRef پیدا_کن_قاعده(بیعلامت سطر_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  تغییرپذیر std::once_flag Warned;
};

Error RegexMatcher::درج(StringRef الگو, بیعلامت سطر_عدد,
                           منطقي SlashAgnostic) {
  إذا (الگو.خالی())
    أرجع createStringError(errc::invalid_آرگومان,
                             "Supplied regex was blank");

  // Replace * with .*
  خودکار Regexp = الگو.str();
  لكل (size_t pos = 0; (pos = Regexp.پیدا_کن('*', pos)) != std::string::npos;
       pos += طول_رشته(".*")) {
    Regexp.replace(pos, طول_رشته("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  إذا (!CheckRE.isValid(REError))
    أرجع createStringError(errc::invalid_آرگومان, REError);

  RegExes.emplace_پشت(الگو, سطر_عدد, std::move(CheckRE));
  أرجع Error::success();
}

بیعلامت RegexMatcher::تطابق(StringRef Query) ثابت {
  لكل (ثابت خودکار &R : معکوس(RegExes))
    إذا (R.Rg.تطابق(Query))
      أرجع R.سطر_No;
  أرجع 0;
}

StringRef RegexMatcher::پیدا_کن_قاعده(بیعلامت سطر_No) ثابت {
  لكل (ثابت خودکار &R : RegExes)
    إذا (R.سطر_No == سطر_No)
      أرجع R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  أرجع {};
}

Error GlobMatcher::درج(StringRef الگو, بیعلامت سطر_عدد,
                          منطقي SlashAgnostic) {
  إذا (الگو.خالی())
    أرجع createStringError(errc::invalid_آرگومان, "Supplied glob was blank");

  خودکار Res =
      Glob_الگو::create(الگو, /*MaxSubPatterns=*/1024, SlashAgnostic);
  إذا (خودکار Err = Res.takeError())
    أرجع Err;
  Globs.emplace_پشت(الگو, سطر_عدد, std::move(Res.get()));
  أرجع Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  إذا (LLVM_LIKELY(Initialized))
    أرجع;
  Initialized = درست;
  لكل (ثابت خودکار &[Idx, G] : enumerate(Globs)) {
    StringRef پیشوند = G.الگو.پیشوند_2();
    StringRef Suffix = G.الگو.suffix();

    إذا (Suffix.خالی() && پیشوند.خالی()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.الگو.longest_substr();
      إذا (!Substr.خالی()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        خودکار &V = Substr_مقصد_Glob.emplace(Substr).اولی_2->دومی_2;
        V.emplace_پشت(Idx);
        استمر;
      }
    }

    خودکار &SToGlob = پیشوند_Suffix_مقصد_Glob.emplace(پیشوند).اولی_2->دومی_2;
    خودکار &V = SToGlob.emplace(معکوس(Suffix)).اولی_2->دومی_2;
    V.emplace_پشت(Idx);
  }
}

بیعلامت GlobMatcher::تطابق(StringRef Query) ثابت {
  LazyInit();

  صحيح بهترین = -1;
  إذا (!پیشوند_Suffix_مقصد_Glob.خالی()) {
    لكل (ثابت خودکار &[_, SToGlob] : پیشوند_Suffix_مقصد_Glob.پیدا_کن_prefixes(Query)) {
      لكل (ثابت خودکار &[_, V] : SToGlob.پیدا_کن_prefixes(معکوس(Query))) {
        لكل (صحيح Idx : معکوس(V)) {
          إذا (بهترین > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.الگو.تطابق(Query)) {
            بهترین = Idx;
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

  إذا (!Substr_مقصد_Glob.خالی()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    لكل (StringRef Q = Query; !Q.خالی(); Q = Q.drop_front()) {
      لكل (ثابت خودکار &[_, V] : Substr_مقصد_Glob.پیدا_کن_prefixes(Q)) {
        لكل (صحيح Idx : معکوس(V)) {
          إذا (بهترین > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.الگو.تطابق(Query)) {
            بهترین = Idx;
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
  أرجع بهترین < 0 ? 0 : Globs[بهترین].سطر_No;
}

StringRef GlobMatcher::پیدا_کن_قاعده(بیعلامت سطر_No) ثابت {
  لكل (ثابت خودکار &G : Globs)
    إذا (G.سطر_No == سطر_No)
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

Error Matcher::درج(StringRef الگو, بیعلامت سطر_عدد) {
  أرجع std::بازدید(
      [&](خودکار &V) {
        أرجع V.درج(الگو, سطر_عدد, Options.SlashAgnostic);
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
بیعلامت Matcher::تطابق(StringRef Query) ثابت {
  إذا (!Options.RemoveDotSlash)
    أرجع تطابق_داخلی(Query);

  إذا (!Options.Warn_Dot_Slash_تطابق)
    أرجع تطابق_داخلی(llvm::sys::مسیر_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::مسیر_2::remove_leading_dotslash(Query);
  بیعلامت FixedMatched = تطابق_داخلی(FixedQuery);
  إذا (FixedQuery == Query)
    أرجع FixedMatched;

  بیعلامت Original_تطابق = تطابق_داخلی(Query);
  إذا (Original_تطابق > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << پیدا_کن_قاعده(Original_تطابق) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  أرجع std::max(Original_تطابق, FixedMatched);
}

بیعلامت Matcher::تطابق_داخلی(StringRef Query) ثابت {
  أرجع std::بازدید([&](خودکار &V) -> بیعلامت { أرجع V.تطابق(Query); }, M);
}

StringRef Matcher::پیدا_کن_قاعده(بیعلامت سطر_No) ثابت {
  أرجع std::بازدید([&](خودکار &V) -> StringRef { أرجع V.پیدا_کن_قاعده(سطر_No); },
                    M);
}
} // namespace

صنف SpecialCaseList::Section::SectionImpl {
عام:
  ثابت Matcher *پیدا_کن_Matcher(StringRef پیشوند, StringRef Category) ثابت;

  استفاده SectionEntries = StringMap<StringMap<Matcher>>;

  صریح SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::وکتور<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_داخلی(Paths, FS, Error))
    أرجع SCL;
  أرجع اشاره_تهی;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت حافظه_میانگیر *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_داخلی(MB, Error))
    أرجع SCL;
  أرجع اشاره_تهی;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::وکتور<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  إذا (خودکار SCL = create(Paths, FS, Error))
    أرجع SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::create_داخلی(ثابت std::وکتور<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  لكل (size_t i = 0; i < Paths.اندازه_2(); ++i) {
    ثابت خودکار &مسیر = Paths[i];
    ErrorOr<std::unique_ptr<حافظه_میانگیر>> FileOrErr =
        VFS.get_میانگیر_For_File(مسیر);
    إذا (std::error_کد EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + مسیر + "': " + EC.پیام_2()).str();
      أرجع خطأ;
    }
    std::string ParseError;
    إذا (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + مسیر + "': " + ParseError).str();
      أرجع خطأ;
    }
  }
  أرجع درست;
}

منطقي SpecialCaseList::create_داخلی(ثابت حافظه_میانگیر *MB,
                                     std::string &Error) {
  إذا (!parse(0, MB, Error))
    أرجع خطأ;
  أرجع درست;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::جمع_کن_Section(StringRef SectionStr, بیعلامت FileNo,
                            بیعلامت سطر_No, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_پشت(SectionStr, FileNo, UseGlobs);
  خودکار &Section = Sections.پشت();

  إذا (خودکار Err = Section.Impl->SectionMatcher.درج(SectionStr, سطر_No)) {
    أرجع createStringError(errc::invalid_آرگومان,
                             "malformed section at line " + Twine(سطر_No) +
                                 ": '" + SectionStr +
                                 "': " + مقصد_String(std::move(Err)));
  }

  أرجع &Section;
}

منطقي SpecialCaseList::parse(بیعلامت FileIdx, ثابت حافظه_میانگیر *MB,
                            std::string &Error) {
  بیعلامت دراز دراز نسخه = 2;

  StringRef سرآیند = MB->get_میانگیر();
  إذا (سرآیند.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(سرآیند, 10, نسخه);

  خودکار Min_نسخه = [&](بیعلامت V) { أرجع نسخه >= V; };

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
  منطقي SlashAgnostic = Min_نسخه(4) && llvm::sys::مسیر_2::is_style_windows(
                                            llvm::sys::مسیر_2::Style::native);

  خودکار ErrOrSection = جمع_کن_Section("*", FileIdx, 1, درست);
  إذا (خودکار Err = ErrOrSection.takeError()) {
    Error = مقصد_String(std::move(Err));
    أرجع خطأ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef مسیر_Prefixes[] = {"src", "!src", "mainfile", "source"};

  لكل (سطر_iterator سطر_It(*MB, /*SkipBlanks=*/درست, /*CommentMarker=*/'#');
       !سطر_It.is_at_eof(); سطر_It++) {
    بیعلامت سطر_No = سطر_It.سطر_عدد_2();
    StringRef سطر = سطر_It->trim();
    إذا (سطر.خالی())
      استمر;

    // Save section names
    إذا (سطر.starts_with("[")) {
      إذا (!سطر.دوسر_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سطر_No) + ": " + سطر)
                .str();
        أرجع خطأ;
      }

      خودکار ErrOrSection =
          جمع_کن_Section(سطر.drop_front().drop_پشت(), FileIdx, سطر_No, UseGlobs);
      إذا (خودکار Err = ErrOrSection.takeError()) {
        Error = مقصد_String(std::move(Err));
        أرجع خطأ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    خودکار [پیشوند, Postfix] = سطر.تقسیم(":");
    إذا (Postfix.خالی()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سطر_No) + ": '" + سطر + "'").str();
      أرجع خطأ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    إذا (llvm::is_contained(مسیر_Prefixes, پیشوند)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_تطابق = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    خودکار [الگو, Category] = Postfix.تقسیم("=");
    خودکار [It, _] = CurrentImpl->Entries[پیشوند].try_emplace(Category, QOpts);
    الگو = الگو.copy(StrAlloc);
    إذا (خودکار Err = It->دومی_2.درج(الگو, سطر_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سطر_No) + ": '" + الگو + "': " + مقصد_String(std::move(Err)))
              .str();
      أرجع خطأ;
    }
  }

  أرجع درست;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::درون_Section(StringRef Section, StringRef پیشوند,
                                StringRef Query, StringRef Category) ثابت {
  خودکار [FileIdx, سطر_No] = درون_Section_Blame(Section, پیشوند, Query, Category);
  أرجع سطر_No;
}

std::pair<بیعلامت, بیعلامت>
SpecialCaseList::درون_Section_Blame(StringRef Section, StringRef پیشوند,
                                StringRef Query, StringRef Category) ثابت {
  لكل (ثابت خودکار &S : معکوس(Sections)) {
    إذا (S.Impl->SectionMatcher.تطابق_Any(Section)) {
      بیعلامت Blame = S.get_آخرین_تطابق(پیشوند, Query, Category);
      إذا (Blame)
        أرجع {S.FileIdx, Blame};
    }
  }
  أرجع Not_یافت;
}

SpecialCaseList::Section::Section(StringRef Str, بیعلامت FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/خطأ})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::تطابق_Name(StringRef Name) ثابت {
  أرجع Impl->SectionMatcher.تطابق_Any(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::پیدا_کن_Matcher(StringRef پیشوند,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.پیدا_کن(پیشوند);
  إذا (I == Entries.انتها())
    أرجع اشاره_تهی;
  StringMap<Matcher>::const_iterator II = I->دومی_2.پیدا_کن(Category);
  إذا (II == I->دومی_2.انتها())
    أرجع اشاره_تهی;

  أرجع &II->دومی_2;
}

بیعلامت SpecialCaseList::Section::get_آخرین_تطابق(StringRef پیشوند,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  إذا (ثابت Matcher *M = Impl->پیدا_کن_Matcher(پیشوند, Category))
    أرجع M->تطابق(Query);
  أرجع 0;
}

منطقي SpecialCaseList::Section::has_پیشوند(StringRef پیشوند) ثابت {
  أرجع Impl->Entries.شامل_است(پیشوند);
}

} // namespace llvm
