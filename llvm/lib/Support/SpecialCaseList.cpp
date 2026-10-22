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
  Error داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر, منطقي SlashAgnostic);
  بنانشانی میل(StringRef Query) ثابت;
  StringRef لبھو_قاعدہ(بنانشانی سطر_No) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, بنانشانی سطر_No, Regex &&Rg)
        : Name(Name), سطر_No(سطر_No), Rg(std::move(Rg)) {}
    StringRef Name;
    بنانشانی سطر_No;
    Regex Rg;
  };

  std::سمتیہ<Reg> RegExes;
};

صنف GlobMatcher {
عام:
  Error داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر, منطقي SlashAgnostic);
  بنانشانی میل(StringRef Query) ثابت;
  StringRef لبھو_قاعدہ(بنانشانی سطر_No) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, بنانشانی سطر_No, Glob_نمونہ &&نمونہ)
        : Name(Name), سطر_No(سطر_No), نمونہ(std::move(نمونہ)) {}
    StringRef Name;
    بنانشانی سطر_No;
    Glob_نمونہ نمونہ;
  };

  فراغ LazyInit() ثابت;

  std::سمتیہ<GlobMatcher::Glob> Globs;

  بدلنجوگ Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Radix_درخت<iterator_range<StringRef::const_الٹ_iterator>,
                              Small_سمتیہ<صحيح, 1>>>
      اگیتر_Suffix_منزل_Glob;

  بدلنجوگ Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Small_سمتیہ<صحيح, 1>>
      Substr_منزل_Glob;

  بدلنجوگ منطقي Initialized = خطأ;
};

بنية QueryOptions {
  منطقي UseGlobs = سچ;
  منطقي RemoveDotSlash = خطأ;
  منطقي Warn_Dot_Slash_میل = خطأ;
  منطقي SlashAgnostic = خطأ;
};

/// Represents a set of patterns and their line numbers
صنف Matcher {
عام:
  واضح Matcher(QueryOptions QOpts);

  Error داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر);
  بنانشانی میل(StringRef Query) ثابت;

  منطقي میل_Any(StringRef Query) ثابت { أرجع میل(Query); }

خاص:
  بنانشانی میل_اندرونی(StringRef Query) ثابت;
  StringRef لبھو_قاعدہ(بنانشانی سطر_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  بدلنجوگ std::once_flag Warned;
};

Error RegexMatcher::داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر,
                           منطقي SlashAgnostic) {
  إذا (نمونہ.خالی_تھاواں())
    أرجع createStringError(errc::invalid_دلیل,
                             "Supplied regex was blank");

  // Replace * with .*
  خودکار Regexp = نمونہ.str();
  لكل (size_t pos = 0; (pos = Regexp.لبھو('*', pos)) != std::string::npos;
       pos += لڑی_لمبائی(".*")) {
    Regexp.replace(pos, لڑی_لمبائی("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  إذا (!CheckRE.isValid(REError))
    أرجع createStringError(errc::invalid_دلیل, REError);

  RegExes.emplace_پٹھ(نمونہ, سطر_نمبر, std::move(CheckRE));
  أرجع Error::success();
}

بنانشانی RegexMatcher::میل(StringRef Query) ثابت {
  لكل (ثابت خودکار &R : الٹ(RegExes))
    إذا (R.Rg.میل(Query))
      أرجع R.سطر_No;
  أرجع 0;
}

StringRef RegexMatcher::لبھو_قاعدہ(بنانشانی سطر_No) ثابت {
  لكل (ثابت خودکار &R : RegExes)
    إذا (R.سطر_No == سطر_No)
      أرجع R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  أرجع {};
}

Error GlobMatcher::داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر,
                          منطقي SlashAgnostic) {
  إذا (نمونہ.خالی_تھاواں())
    أرجع createStringError(errc::invalid_دلیل, "Supplied glob was blank");

  خودکار Res =
      Glob_نمونہ::create(نمونہ, /*MaxSubPatterns=*/1024, SlashAgnostic);
  إذا (خودکار Err = Res.takeError())
    أرجع Err;
  Globs.emplace_پٹھ(نمونہ, سطر_نمبر, std::move(Res.get()));
  أرجع Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  إذا (LLVM_LIKELY(Initialized))
    أرجع;
  Initialized = سچ;
  لكل (ثابت خودکار &[Idx, G] : enumerate(Globs)) {
    StringRef اگیتر = G.نمونہ.اگیتر_2();
    StringRef Suffix = G.نمونہ.suffix();

    إذا (Suffix.خالی_تھاواں() && اگیتر.خالی_تھاواں()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.نمونہ.longest_substr();
      إذا (!Substr.خالی_تھاواں()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        خودکار &V = Substr_منزل_Glob.emplace(Substr).پہلا_2->دوجا_2;
        V.emplace_پٹھ(Idx);
        استمر;
      }
    }

    خودکار &SToGlob = اگیتر_Suffix_منزل_Glob.emplace(اگیتر).پہلا_2->دوجا_2;
    خودکار &V = SToGlob.emplace(الٹ(Suffix)).پہلا_2->دوجا_2;
    V.emplace_پٹھ(Idx);
  }
}

بنانشانی GlobMatcher::میل(StringRef Query) ثابت {
  LazyInit();

  صحيح بہترین = -1;
  إذا (!اگیتر_Suffix_منزل_Glob.خالی_تھاواں()) {
    لكل (ثابت خودکار &[_, SToGlob] : اگیتر_Suffix_منزل_Glob.لبھو_prefixes(Query)) {
      لكل (ثابت خودکار &[_, V] : SToGlob.لبھو_prefixes(الٹ(Query))) {
        لكل (صحيح Idx : الٹ(V)) {
          إذا (بہترین > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونہ.میل(Query)) {
            بہترین = Idx;
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

  إذا (!Substr_منزل_Glob.خالی_تھاواں()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    لكل (StringRef Q = Query; !Q.خالی_تھاواں(); Q = Q.drop_front()) {
      لكل (ثابت خودکار &[_, V] : Substr_منزل_Glob.لبھو_prefixes(Q)) {
        لكل (صحيح Idx : الٹ(V)) {
          إذا (بہترین > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونہ.میل(Query)) {
            بہترین = Idx;
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
  أرجع بہترین < 0 ? 0 : Globs[بہترین].سطر_No;
}

StringRef GlobMatcher::لبھو_قاعدہ(بنانشانی سطر_No) ثابت {
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

Error Matcher::داخل_کرو(StringRef نمونہ, بنانشانی سطر_نمبر) {
  أرجع std::ویکھو(
      [&](خودکار &V) {
        أرجع V.داخل_کرو(نمونہ, سطر_نمبر, Options.SlashAgnostic);
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
بنانشانی Matcher::میل(StringRef Query) ثابت {
  إذا (!Options.RemoveDotSlash)
    أرجع میل_اندرونی(Query);

  إذا (!Options.Warn_Dot_Slash_میل)
    أرجع میل_اندرونی(llvm::sys::رستہ_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::رستہ_2::remove_leading_dotslash(Query);
  بنانشانی FixedMatched = میل_اندرونی(FixedQuery);
  إذا (FixedQuery == Query)
    أرجع FixedMatched;

  بنانشانی Original_میل = میل_اندرونی(Query);
  إذا (Original_میل > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << لبھو_قاعدہ(Original_میل) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  أرجع std::max(Original_میل, FixedMatched);
}

بنانشانی Matcher::میل_اندرونی(StringRef Query) ثابت {
  أرجع std::ویکھو([&](خودکار &V) -> بنانشانی { أرجع V.میل(Query); }, M);
}

StringRef Matcher::لبھو_قاعدہ(بنانشانی سطر_No) ثابت {
  أرجع std::ویکھو([&](خودکار &V) -> StringRef { أرجع V.لبھو_قاعدہ(سطر_No); },
                    M);
}
} // namespace

صنف SpecialCaseList::Section::SectionImpl {
عام:
  ثابت Matcher *لبھو_Matcher(StringRef اگیتر, StringRef Category) ثابت;

  ورتوں SectionEntries = StringMap<StringMap<Matcher>>;

  واضح SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::سمتیہ<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_اندرونی(Paths, FS, Error))
    أرجع SCL;
  أرجع خالی_سوچک;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت حافظہ_بفر *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_اندرونی(MB, Error))
    أرجع SCL;
  أرجع خالی_سوچک;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::سمتیہ<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  إذا (خودکار SCL = create(Paths, FS, Error))
    أرجع SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::create_اندرونی(ثابت std::سمتیہ<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  لكل (size_t i = 0; i < Paths.ناپ_2(); ++i) {
    ثابت خودکار &رستہ = Paths[i];
    ErrorOr<std::unique_ptr<حافظہ_بفر>> FileOrErr =
        VFS.get_بفر_For_File(رستہ);
    إذا (std::error_کوڈ EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + رستہ + "': " + EC.سنیہا()).str();
      أرجع خطأ;
    }
    std::string ParseError;
    إذا (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + رستہ + "': " + ParseError).str();
      أرجع خطأ;
    }
  }
  أرجع سچ;
}

منطقي SpecialCaseList::create_اندرونی(ثابت حافظہ_بفر *MB,
                                     std::string &Error) {
  إذا (!parse(0, MB, Error))
    أرجع خطأ;
  أرجع سچ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::جوڑو_Section(StringRef SectionStr, بنانشانی FileNo,
                            بنانشانی سطر_No, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_پٹھ(SectionStr, FileNo, UseGlobs);
  خودکار &Section = Sections.پٹھ();

  إذا (خودکار Err = Section.Impl->SectionMatcher.داخل_کرو(SectionStr, سطر_No)) {
    أرجع createStringError(errc::invalid_دلیل,
                             "malformed section at line " + Twine(سطر_No) +
                                 ": '" + SectionStr +
                                 "': " + منزل_String(std::move(Err)));
  }

  أرجع &Section;
}

منطقي SpecialCaseList::parse(بنانشانی FileIdx, ثابت حافظہ_بفر *MB,
                            std::string &Error) {
  بنانشانی لما لما نسخہ = 2;

  StringRef سرخی = MB->get_بفر();
  إذا (سرخی.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(سرخی, 10, نسخہ);

  خودکار Min_نسخہ = [&](بنانشانی V) { أرجع نسخہ >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقي UseGlobs = Min_نسخہ(2);
  منطقي RemoveDotSlash = Min_نسخہ(3);
  منطقي WarnDotSlash = Min_نسخہ(4) && !Min_نسخہ(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقي SlashAgnostic = Min_نسخہ(4) && llvm::sys::رستہ_2::is_style_windows(
                                            llvm::sys::رستہ_2::Style::native);

  خودکار ErrOrSection = جوڑو_Section("*", FileIdx, 1, سچ);
  إذا (خودکار Err = ErrOrSection.takeError()) {
    Error = منزل_String(std::move(Err));
    أرجع خطأ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef رستہ_Prefixes[] = {"src", "!src", "mainfile", "source"};

  لكل (سطر_iterator سطر_It(*MB, /*SkipBlanks=*/سچ, /*CommentMarker=*/'#');
       !سطر_It.is_at_eof(); سطر_It++) {
    بنانشانی سطر_No = سطر_It.سطر_نمبر_2();
    StringRef سطر = سطر_It->trim();
    إذا (سطر.خالی_تھاواں())
      استمر;

    // Save section names
    إذا (سطر.starts_with("[")) {
      إذا (!سطر.سرے_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سطر_No) + ": " + سطر)
                .str();
        أرجع خطأ;
      }

      خودکار ErrOrSection =
          جوڑو_Section(سطر.drop_front().drop_پٹھ(), FileIdx, سطر_No, UseGlobs);
      إذا (خودکار Err = ErrOrSection.takeError()) {
        Error = منزل_String(std::move(Err));
        أرجع خطأ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    خودکار [اگیتر, Postfix] = سطر.ونڈو(":");
    إذا (Postfix.خالی_تھاواں()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سطر_No) + ": '" + سطر + "'").str();
      أرجع خطأ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    إذا (llvm::is_contained(رستہ_Prefixes, اگیتر)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_میل = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    خودکار [نمونہ, Category] = Postfix.ونڈو("=");
    خودکار [It, _] = CurrentImpl->Entries[اگیتر].try_emplace(Category, QOpts);
    نمونہ = نمونہ.copy(StrAlloc);
    إذا (خودکار Err = It->دوجا_2.داخل_کرو(نمونہ, سطر_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سطر_No) + ": '" + نمونہ + "': " + منزل_String(std::move(Err)))
              .str();
      أرجع خطأ;
    }
  }

  أرجع سچ;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::وچ_Section(StringRef Section, StringRef اگیتر,
                                StringRef Query, StringRef Category) ثابت {
  خودکار [FileIdx, سطر_No] = وچ_Section_Blame(Section, اگیتر, Query, Category);
  أرجع سطر_No;
}

std::pair<بنانشانی, بنانشانی>
SpecialCaseList::وچ_Section_Blame(StringRef Section, StringRef اگیتر,
                                StringRef Query, StringRef Category) ثابت {
  لكل (ثابت خودکار &S : الٹ(Sections)) {
    إذا (S.Impl->SectionMatcher.میل_Any(Section)) {
      بنانشانی Blame = S.get_آخری_میل(اگیتر, Query, Category);
      إذا (Blame)
        أرجع {S.FileIdx, Blame};
    }
  }
  أرجع Not_لبھیا;
}

SpecialCaseList::Section::Section(StringRef Str, بنانشانی FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/خطأ})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::میل_Name(StringRef Name) ثابت {
  أرجع Impl->SectionMatcher.میل_Any(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::لبھو_Matcher(StringRef اگیتر,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.لبھو(اگیتر);
  إذا (I == Entries.سرا())
    أرجع خالی_سوچک;
  StringMap<Matcher>::const_iterator II = I->دوجا_2.لبھو(Category);
  إذا (II == I->دوجا_2.سرا())
    أرجع خالی_سوچک;

  أرجع &II->دوجا_2;
}

بنانشانی SpecialCaseList::Section::get_آخری_میل(StringRef اگیتر,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  إذا (ثابت Matcher *M = Impl->لبھو_Matcher(اگیتر, Category))
    أرجع M->میل(Query);
  أرجع 0;
}

منطقي SpecialCaseList::Section::has_اگیتر(StringRef اگیتر) ثابت {
  أرجع Impl->Entries.شامل_اے(اگیتر);
}

} // namespace llvm
