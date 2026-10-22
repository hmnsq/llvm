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

نامخلا llvm {

نامخلا {

// Lagacy v1 matcher.
جماعت RegexMatcher {
عوامی:
  Error داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر, منطقی SlashAgnostic);
  غیرعلامتی مطابقت(StringRef Query) ثابت;
  StringRef ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت;

نجی:
  ساخت Reg {
    Reg(StringRef Name, غیرعلامتی سطر_No, Regex &&Rg)
        : Name(Name), سطر_No(سطر_No), Rg(std::move(Rg)) {}
    StringRef Name;
    غیرعلامتی سطر_No;
    Regex Rg;
  };

  std::سمتیہ<Reg> RegExes;
};

جماعت GlobMatcher {
عوامی:
  Error داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر, منطقی SlashAgnostic);
  غیرعلامتی مطابقت(StringRef Query) ثابت;
  StringRef ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت;

نجی:
  ساخت Glob {
    Glob(StringRef Name, غیرعلامتی سطر_No, Glob_نمونہ &&نمونہ)
        : Name(Name), سطر_No(سطر_No), نمونہ(std::move(نمونہ)) {}
    StringRef Name;
    غیرعلامتی سطر_No;
    Glob_نمونہ نمونہ;
  };

  خالی LazyInit() ثابت;

  std::سمتیہ<GlobMatcher::Glob> Globs;

  تغیرپذیر Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Radix_درخت<iterator_range<StringRef::const_الٹ_iterator>,
                              Small_سمتیہ<صحیح, 1>>>
      سابقہ_Suffix_مقصد_Glob;

  تغیرپذیر Radix_درخت<iterator_range<StringRef::const_iterator>,
                    Small_سمتیہ<صحیح, 1>>
      Substr_مقصد_Glob;

  تغیرپذیر منطقی Initialized = جھوٹ;
};

ساخت QueryOptions {
  منطقی UseGlobs = سچ;
  منطقی RemoveDotSlash = جھوٹ;
  منطقی Warn_Dot_Slash_مطابقت = جھوٹ;
  منطقی SlashAgnostic = جھوٹ;
};

/// Represents a set of patterns and their line numbers
جماعت Matcher {
عوامی:
  صریح Matcher(QueryOptions QOpts);

  Error داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر);
  غیرعلامتی مطابقت(StringRef Query) ثابت;

  منطقی مطابقت_Any(StringRef Query) ثابت { واپس مطابقت(Query); }

نجی:
  غیرعلامتی مطابقت_اندرونی(StringRef Query) ثابت;
  StringRef ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  تغیرپذیر std::once_flag Warned;
};

Error RegexMatcher::داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر,
                           منطقی SlashAgnostic) {
  اگر (نمونہ.خالی_خانے())
    واپس createStringError(errc::invalid_دلیل,
                             "Supplied regex was blank");

  // Replace * with .*
  خودکار Regexp = نمونہ.str();
  برائے (size_t pos = 0; (pos = Regexp.ڈھونڈو('*', pos)) != std::string::npos;
       pos += لڑی_لمبائی(".*")) {
    Regexp.replace(pos, لڑی_لمبائی("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  اگر (!CheckRE.isValid(REError))
    واپس createStringError(errc::invalid_دلیل, REError);

  RegExes.emplace_عقب(نمونہ, سطر_نمبر, std::move(CheckRE));
  واپس Error::success();
}

غیرعلامتی RegexMatcher::مطابقت(StringRef Query) ثابت {
  برائے (ثابت خودکار &R : الٹ(RegExes))
    اگر (R.Rg.مطابقت(Query))
      واپس R.سطر_No;
  واپس 0;
}

StringRef RegexMatcher::ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت {
  برائے (ثابت خودکار &R : RegExes)
    اگر (R.سطر_No == سطر_No)
      واپس R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  واپس {};
}

Error GlobMatcher::داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر,
                          منطقی SlashAgnostic) {
  اگر (نمونہ.خالی_خانے())
    واپس createStringError(errc::invalid_دلیل, "Supplied glob was blank");

  خودکار Res =
      Glob_نمونہ::create(نمونہ, /*MaxSubPatterns=*/1024, SlashAgnostic);
  اگر (خودکار Err = Res.takeError())
    واپس Err;
  Globs.emplace_عقب(نمونہ, سطر_نمبر, std::move(Res.get()));
  واپس Error::success();
}

خالی GlobMatcher::LazyInit() ثابت {
  اگر (LLVM_LIKELY(Initialized))
    واپس;
  Initialized = سچ;
  برائے (ثابت خودکار &[Idx, G] : enumerate(Globs)) {
    StringRef سابقہ = G.نمونہ.سابقہ_2();
    StringRef Suffix = G.نمونہ.suffix();

    اگر (Suffix.خالی_خانے() && سابقہ.خالی_خانے()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.نمونہ.longest_substr();
      اگر (!Substr.خالی_خانے()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        خودکار &V = Substr_مقصد_Glob.emplace(Substr).پہلا_2->دوسرا_2;
        V.emplace_عقب(Idx);
        جاری;
      }
    }

    خودکار &SToGlob = سابقہ_Suffix_مقصد_Glob.emplace(سابقہ).پہلا_2->دوسرا_2;
    خودکار &V = SToGlob.emplace(الٹ(Suffix)).پہلا_2->دوسرا_2;
    V.emplace_عقب(Idx);
  }
}

غیرعلامتی GlobMatcher::مطابقت(StringRef Query) ثابت {
  LazyInit();

  صحیح بہترین = -1;
  اگر (!سابقہ_Suffix_مقصد_Glob.خالی_خانے()) {
    برائے (ثابت خودکار &[_, SToGlob] : سابقہ_Suffix_مقصد_Glob.ڈھونڈو_prefixes(Query)) {
      برائے (ثابت خودکار &[_, V] : SToGlob.ڈھونڈو_prefixes(الٹ(Query))) {
        برائے (صحیح Idx : الٹ(V)) {
          اگر (بہترین > Idx)
            توڑو;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          اگر (G.نمونہ.مطابقت(Query)) {
            بہترین = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            توڑو;
          }
        }
      }
    }
  }

  اگر (!Substr_مقصد_Glob.خالی_خانے()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    برائے (StringRef Q = Query; !Q.خالی_خانے(); Q = Q.drop_front()) {
      برائے (ثابت خودکار &[_, V] : Substr_مقصد_Glob.ڈھونڈو_prefixes(Q)) {
        برائے (صحیح Idx : الٹ(V)) {
          اگر (بہترین > Idx)
            توڑو;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          اگر (G.نمونہ.مطابقت(Query)) {
            بہترین = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            توڑو;
          }
        }
      }
    }
  }
  واپس بہترین < 0 ? 0 : Globs[بہترین].سطر_No;
}

StringRef GlobMatcher::ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت {
  برائے (ثابت خودکار &G : Globs)
    اگر (G.سطر_No == سطر_No)
      واپس G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  واپس {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  اگر (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  ورنہ
    M.emplace<RegexMatcher>();
}

Error Matcher::داخل_کرو(StringRef نمونہ, غیرعلامتی سطر_نمبر) {
  واپس std::دورہ(
      [&](خودکار &V) {
        واپس V.داخل_کرو(نمونہ, سطر_نمبر, Options.SlashAgnostic);
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
غیرعلامتی Matcher::مطابقت(StringRef Query) ثابت {
  اگر (!Options.RemoveDotSlash)
    واپس مطابقت_اندرونی(Query);

  اگر (!Options.Warn_Dot_Slash_مطابقت)
    واپس مطابقت_اندرونی(llvm::sys::راستہ_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::راستہ_2::remove_leading_dotslash(Query);
  غیرعلامتی FixedMatched = مطابقت_اندرونی(FixedQuery);
  اگر (FixedQuery == Query)
    واپس FixedMatched;

  غیرعلامتی Original_مطابقت = مطابقت_اندرونی(Query);
  اگر (Original_مطابقت > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << ڈھونڈو_قاعدہ(Original_مطابقت) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  واپس std::max(Original_مطابقت, FixedMatched);
}

غیرعلامتی Matcher::مطابقت_اندرونی(StringRef Query) ثابت {
  واپس std::دورہ([&](خودکار &V) -> غیرعلامتی { واپس V.مطابقت(Query); }, M);
}

StringRef Matcher::ڈھونڈو_قاعدہ(غیرعلامتی سطر_No) ثابت {
  واپس std::دورہ([&](خودکار &V) -> StringRef { واپس V.ڈھونڈو_قاعدہ(سطر_No); },
                    M);
}
} // namespace

جماعت SpecialCaseList::Section::SectionImpl {
عوامی:
  ثابت Matcher *ڈھونڈو_Matcher(StringRef سابقہ, StringRef Category) ثابت;

  استعمال SectionEntries = StringMap<StringMap<Matcher>>;

  صریح SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::سمتیہ<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(نیا SpecialCaseList());
  اگر (SCL->create_اندرونی(Paths, FS, Error))
    واپس SCL;
  واپس خالی_اشاریہ;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت حافظہ_بفر *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(نیا SpecialCaseList());
  اگر (SCL->create_اندرونی(MB, Error))
    واپس SCL;
  واپس خالی_اشاریہ;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::سمتیہ<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  اگر (خودکار SCL = create(Paths, FS, Error))
    واپس SCL;
  report_fatal_error(Twine(Error));
}

منطقی SpecialCaseList::create_اندرونی(ثابت std::سمتیہ<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  برائے (size_t i = 0; i < Paths.سائز_2(); ++i) {
    ثابت خودکار &راستہ = Paths[i];
    ErrorOr<std::unique_ptr<حافظہ_بفر>> FileOrErr =
        VFS.get_بفر_For_File(راستہ);
    اگر (std::error_رمز EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + راستہ + "': " + EC.پیغام_2()).str();
      واپس جھوٹ;
    }
    std::string ParseError;
    اگر (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + راستہ + "': " + ParseError).str();
      واپس جھوٹ;
    }
  }
  واپس سچ;
}

منطقی SpecialCaseList::create_اندرونی(ثابت حافظہ_بفر *MB,
                                     std::string &Error) {
  اگر (!parse(0, MB, Error))
    واپس جھوٹ;
  واپس سچ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::جوڑو_Section(StringRef SectionStr, غیرعلامتی FileNo,
                            غیرعلامتی سطر_No, منطقی UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_عقب(SectionStr, FileNo, UseGlobs);
  خودکار &Section = Sections.عقب();

  اگر (خودکار Err = Section.Impl->SectionMatcher.داخل_کرو(SectionStr, سطر_No)) {
    واپس createStringError(errc::invalid_دلیل,
                             "malformed section at line " + Twine(سطر_No) +
                                 ": '" + SectionStr +
                                 "': " + مقصد_String(std::move(Err)));
  }

  واپس &Section;
}

منطقی SpecialCaseList::parse(غیرعلامتی FileIdx, ثابت حافظہ_بفر *MB,
                            std::string &Error) {
  غیرعلامتی طویل طویل نسخہ = 2;

  StringRef سرخی = MB->get_بفر();
  اگر (سرخی.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(سرخی, 10, نسخہ);

  خودکار Min_نسخہ = [&](غیرعلامتی V) { واپس نسخہ >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقی UseGlobs = Min_نسخہ(2);
  منطقی RemoveDotSlash = Min_نسخہ(3);
  منطقی WarnDotSlash = Min_نسخہ(4) && !Min_نسخہ(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقی SlashAgnostic = Min_نسخہ(4) && llvm::sys::راستہ_2::is_style_windows(
                                            llvm::sys::راستہ_2::Style::native);

  خودکار ErrOrSection = جوڑو_Section("*", FileIdx, 1, سچ);
  اگر (خودکار Err = ErrOrSection.takeError()) {
    Error = مقصد_String(std::move(Err));
    واپس جھوٹ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef راستہ_Prefixes[] = {"src", "!src", "mainfile", "source"};

  برائے (سطر_iterator سطر_It(*MB, /*SkipBlanks=*/سچ, /*CommentMarker=*/'#');
       !سطر_It.is_at_eof(); سطر_It++) {
    غیرعلامتی سطر_No = سطر_It.سطر_نمبر_2();
    StringRef سطر = سطر_It->trim();
    اگر (سطر.خالی_خانے())
      جاری;

    // Save section names
    اگر (سطر.starts_with("[")) {
      اگر (!سطر.سرے_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سطر_No) + ": " + سطر)
                .str();
        واپس جھوٹ;
      }

      خودکار ErrOrSection =
          جوڑو_Section(سطر.drop_front().drop_عقب(), FileIdx, سطر_No, UseGlobs);
      اگر (خودکار Err = ErrOrSection.takeError()) {
        Error = مقصد_String(std::move(Err));
        واپس جھوٹ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      جاری;
    }

    // Get our prefix and unparsed glob.
    خودکار [سابقہ, Postfix] = سطر.بانٹو(":");
    اگر (Postfix.خالی_خانے()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سطر_No) + ": '" + سطر + "'").str();
      واپس جھوٹ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    اگر (llvm::is_contained(راستہ_Prefixes, سابقہ)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_مطابقت = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    خودکار [نمونہ, Category] = Postfix.بانٹو("=");
    خودکار [It, _] = CurrentImpl->Entries[سابقہ].try_emplace(Category, QOpts);
    نمونہ = نمونہ.copy(StrAlloc);
    اگر (خودکار Err = It->دوسرا_2.داخل_کرو(نمونہ, سطر_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سطر_No) + ": '" + نمونہ + "': " + مقصد_String(std::move(Err)))
              .str();
      واپس جھوٹ;
    }
  }

  واپس سچ;
}

SpecialCaseList::~SpecialCaseList() = طےشدہ;

منطقی SpecialCaseList::اندر_Section(StringRef Section, StringRef سابقہ,
                                StringRef Query, StringRef Category) ثابت {
  خودکار [FileIdx, سطر_No] = اندر_Section_Blame(Section, سابقہ, Query, Category);
  واپس سطر_No;
}

std::pair<غیرعلامتی, غیرعلامتی>
SpecialCaseList::اندر_Section_Blame(StringRef Section, StringRef سابقہ,
                                StringRef Query, StringRef Category) ثابت {
  برائے (ثابت خودکار &S : الٹ(Sections)) {
    اگر (S.Impl->SectionMatcher.مطابقت_Any(Section)) {
      غیرعلامتی Blame = S.get_آخری_مطابقت(سابقہ, Query, Category);
      اگر (Blame)
        واپس {S.FileIdx, Blame};
    }
  }
  واپس Not_ملا;
}

SpecialCaseList::Section::Section(StringRef Str, غیرعلامتی FileIdx,
                                  منطقی UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/جھوٹ})) {}

SpecialCaseList::Section::Section(Section &&) = طےشدہ;

SpecialCaseList::Section::~Section() = طےشدہ;

منطقی SpecialCaseList::Section::مطابقت_Name(StringRef Name) ثابت {
  واپس Impl->SectionMatcher.مطابقت_Any(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::ڈھونڈو_Matcher(StringRef سابقہ,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.ڈھونڈو(سابقہ);
  اگر (I == Entries.انتہا())
    واپس خالی_اشاریہ;
  StringMap<Matcher>::const_iterator II = I->دوسرا_2.ڈھونڈو(Category);
  اگر (II == I->دوسرا_2.انتہا())
    واپس خالی_اشاریہ;

  واپس &II->دوسرا_2;
}

غیرعلامتی SpecialCaseList::Section::get_آخری_مطابقت(StringRef سابقہ,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  اگر (ثابت Matcher *M = Impl->ڈھونڈو_Matcher(سابقہ, Category))
    واپس M->مطابقت(Query);
  واپس 0;
}

منطقی SpecialCaseList::Section::has_سابقہ(StringRef سابقہ) ثابت {
  واپس Impl->Entries.شامل_ہے(سابقہ);
}

} // namespace llvm
