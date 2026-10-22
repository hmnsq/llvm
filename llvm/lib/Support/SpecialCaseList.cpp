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
  Error وجهو(StringRef نمونو, بنانشان سٽ_عدد, منطقي SlashAgnostic);
  بنانشان ميلاپ(StringRef Query) ثابت;
  StringRef ڳوليو_قاعدو(بنانشان سٽ_No) ثابت;

خاص:
  بنية Reg {
    Reg(StringRef Name, بنانشان سٽ_No, Regex &&Rg)
        : Name(Name), سٽ_No(سٽ_No), Rg(std::move(Rg)) {}
    StringRef Name;
    بنانشان سٽ_No;
    Regex Rg;
  };

  std::ويڪٽر<Reg> RegExes;
};

صنف GlobMatcher {
عام:
  Error وجهو(StringRef نمونو, بنانشان سٽ_عدد, منطقي SlashAgnostic);
  بنانشان ميلاپ(StringRef Query) ثابت;
  StringRef ڳوليو_قاعدو(بنانشان سٽ_No) ثابت;

خاص:
  بنية Glob {
    Glob(StringRef Name, بنانشان سٽ_No, Glob_نمونو &&نمونو)
        : Name(Name), سٽ_No(سٽ_No), نمونو(std::move(نمونو)) {}
    StringRef Name;
    بنانشان سٽ_No;
    Glob_نمونو نمونو;
  };

  فراغ LazyInit() ثابت;

  std::ويڪٽر<GlobMatcher::Glob> Globs;

  بدلجندڙ Radix_وڻ<iterator_range<StringRef::const_iterator>,
                    Radix_وڻ<iterator_range<StringRef::const_ابتو_iterator>,
                              Small_ويڪٽر<صحيح, 1>>>
      اڳياڙي_Suffix_منزل_Glob;

  بدلجندڙ Radix_وڻ<iterator_range<StringRef::const_iterator>,
                    Small_ويڪٽر<صحيح, 1>>
      Substr_منزل_Glob;

  بدلجندڙ منطقي Initialized = خطأ;
};

بنية QueryOptions {
  منطقي UseGlobs = سچ;
  منطقي RemoveDotSlash = خطأ;
  منطقي Warn_Dot_Slash_ميلاپ = خطأ;
  منطقي SlashAgnostic = خطأ;
};

/// Represents a set of patterns and their line numbers
صنف Matcher {
عام:
  واضح Matcher(QueryOptions QOpts);

  Error وجهو(StringRef نمونو, بنانشان سٽ_عدد);
  بنانشان ميلاپ(StringRef Query) ثابت;

  منطقي ميلاپ_Any(StringRef Query) ثابت { أرجع ميلاپ(Query); }

خاص:
  بنانشان ميلاپ_اندروني(StringRef Query) ثابت;
  StringRef ڳوليو_قاعدو(بنانشان سٽ_No) ثابت;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  بدلجندڙ std::once_flag Warned;
};

Error RegexMatcher::وجهو(StringRef نمونو, بنانشان سٽ_عدد,
                           منطقي SlashAgnostic) {
  إذا (نمونو.خالي_جايون())
    أرجع createStringError(errc::invalid_دليل,
                             "Supplied regex was blank");

  // Replace * with .*
  خودڪار Regexp = نمونو.str();
  لكل (size_t pos = 0; (pos = Regexp.ڳوليو('*', pos)) != std::string::npos;
       pos += لڙي_ڊيگهه(".*")) {
    Regexp.replace(pos, لڙي_ڊيگهه("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  إذا (!CheckRE.isValid(REError))
    أرجع createStringError(errc::invalid_دليل, REError);

  RegExes.emplace_پٺ(نمونو, سٽ_عدد, std::move(CheckRE));
  أرجع Error::success();
}

بنانشان RegexMatcher::ميلاپ(StringRef Query) ثابت {
  لكل (ثابت خودڪار &R : ابتو(RegExes))
    إذا (R.Rg.ميلاپ(Query))
      أرجع R.سٽ_No;
  أرجع 0;
}

StringRef RegexMatcher::ڳوليو_قاعدو(بنانشان سٽ_No) ثابت {
  لكل (ثابت خودڪار &R : RegExes)
    إذا (R.سٽ_No == سٽ_No)
      أرجع R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  أرجع {};
}

Error GlobMatcher::وجهو(StringRef نمونو, بنانشان سٽ_عدد,
                          منطقي SlashAgnostic) {
  إذا (نمونو.خالي_جايون())
    أرجع createStringError(errc::invalid_دليل, "Supplied glob was blank");

  خودڪار Res =
      Glob_نمونو::create(نمونو, /*MaxSubPatterns=*/1024, SlashAgnostic);
  إذا (خودڪار Err = Res.takeError())
    أرجع Err;
  Globs.emplace_پٺ(نمونو, سٽ_عدد, std::move(Res.get()));
  أرجع Error::success();
}

فراغ GlobMatcher::LazyInit() ثابت {
  إذا (LLVM_LIKELY(Initialized))
    أرجع;
  Initialized = سچ;
  لكل (ثابت خودڪار &[Idx, G] : enumerate(Globs)) {
    StringRef اڳياڙي = G.نمونو.اڳياڙي_2();
    StringRef Suffix = G.نمونو.suffix();

    إذا (Suffix.خالي_جايون() && اڳياڙي.خالي_جايون()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.نمونو.longest_substr();
      إذا (!Substr.خالي_جايون()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        خودڪار &V = Substr_منزل_Glob.emplace(Substr).پهريون_2->ٻيو_2;
        V.emplace_پٺ(Idx);
        استمر;
      }
    }

    خودڪار &SToGlob = اڳياڙي_Suffix_منزل_Glob.emplace(اڳياڙي).پهريون_2->ٻيو_2;
    خودڪار &V = SToGlob.emplace(ابتو(Suffix)).پهريون_2->ٻيو_2;
    V.emplace_پٺ(Idx);
  }
}

بنانشان GlobMatcher::ميلاپ(StringRef Query) ثابت {
  LazyInit();

  صحيح بهترين = -1;
  إذا (!اڳياڙي_Suffix_منزل_Glob.خالي_جايون()) {
    لكل (ثابت خودڪار &[_, SToGlob] : اڳياڙي_Suffix_منزل_Glob.ڳوليو_prefixes(Query)) {
      لكل (ثابت خودڪار &[_, V] : SToGlob.ڳوليو_prefixes(ابتو(Query))) {
        لكل (صحيح Idx : ابتو(V)) {
          إذا (بهترين > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونو.ميلاپ(Query)) {
            بهترين = Idx;
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

  إذا (!Substr_منزل_Glob.خالي_جايون()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    لكل (StringRef Q = Query; !Q.خالي_جايون(); Q = Q.drop_front()) {
      لكل (ثابت خودڪار &[_, V] : Substr_منزل_Glob.ڳوليو_prefixes(Q)) {
        لكل (صحيح Idx : ابتو(V)) {
          إذا (بهترين > Idx)
            اكسر;
          ثابت GlobMatcher::Glob &G = Globs[Idx];
          إذا (G.نمونو.ميلاپ(Query)) {
            بهترين = Idx;
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
  أرجع بهترين < 0 ? 0 : Globs[بهترين].سٽ_No;
}

StringRef GlobMatcher::ڳوليو_قاعدو(بنانشان سٽ_No) ثابت {
  لكل (ثابت خودڪار &G : Globs)
    إذا (G.سٽ_No == سٽ_No)
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

Error Matcher::وجهو(StringRef نمونو, بنانشان سٽ_عدد) {
  أرجع std::گهمو(
      [&](خودڪار &V) {
        أرجع V.وجهو(نمونو, سٽ_عدد, Options.SlashAgnostic);
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
بنانشان Matcher::ميلاپ(StringRef Query) ثابت {
  إذا (!Options.RemoveDotSlash)
    أرجع ميلاپ_اندروني(Query);

  إذا (!Options.Warn_Dot_Slash_ميلاپ)
    أرجع ميلاپ_اندروني(llvm::sys::رستو_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::رستو_2::remove_leading_dotslash(Query);
  بنانشان FixedMatched = ميلاپ_اندروني(FixedQuery);
  إذا (FixedQuery == Query)
    أرجع FixedMatched;

  بنانشان Original_ميلاپ = ميلاپ_اندروني(Query);
  إذا (Original_ميلاپ > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << ڳوليو_قاعدو(Original_ميلاپ) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  أرجع std::max(Original_ميلاپ, FixedMatched);
}

بنانشان Matcher::ميلاپ_اندروني(StringRef Query) ثابت {
  أرجع std::گهمو([&](خودڪار &V) -> بنانشان { أرجع V.ميلاپ(Query); }, M);
}

StringRef Matcher::ڳوليو_قاعدو(بنانشان سٽ_No) ثابت {
  أرجع std::گهمو([&](خودڪار &V) -> StringRef { أرجع V.ڳوليو_قاعدو(سٽ_No); },
                    M);
}
} // namespace

صنف SpecialCaseList::Section::SectionImpl {
عام:
  ثابت Matcher *ڳوليو_Matcher(StringRef اڳياڙي, StringRef Category) ثابت;

  استعمال SectionEntries = StringMap<StringMap<Matcher>>;

  واضح SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ثابت std::ويڪٽر<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_اندروني(Paths, FS, Error))
    أرجع SCL;
  أرجع خالي_اشارو;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ثابت يادگيري_بفر *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(جديد SpecialCaseList());
  إذا (SCL->create_اندروني(MB, Error))
    أرجع SCL;
  أرجع خالي_اشارو;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ثابت std::ويڪٽر<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  إذا (خودڪار SCL = create(Paths, FS, Error))
    أرجع SCL;
  report_fatal_error(Twine(Error));
}

منطقي SpecialCaseList::create_اندروني(ثابت std::ويڪٽر<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  لكل (size_t i = 0; i < Paths.سائيز_2(); ++i) {
    ثابت خودڪار &رستو = Paths[i];
    ErrorOr<std::unique_ptr<يادگيري_بفر>> FileOrErr =
        VFS.get_بفر_For_File(رستو);
    إذا (std::error_ڪوڊ EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + رستو + "': " + EC.پيغام()).str();
      أرجع خطأ;
    }
    std::string ParseError;
    إذا (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + رستو + "': " + ParseError).str();
      أرجع خطأ;
    }
  }
  أرجع سچ;
}

منطقي SpecialCaseList::create_اندروني(ثابت يادگيري_بفر *MB,
                                     std::string &Error) {
  إذا (!parse(0, MB, Error))
    أرجع خطأ;
  أرجع سچ;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::جوڙيو_Section(StringRef SectionStr, بنانشان FileNo,
                            بنانشان سٽ_No, منطقي UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_پٺ(SectionStr, FileNo, UseGlobs);
  خودڪار &Section = Sections.پٺ();

  إذا (خودڪار Err = Section.Impl->SectionMatcher.وجهو(SectionStr, سٽ_No)) {
    أرجع createStringError(errc::invalid_دليل,
                             "malformed section at line " + Twine(سٽ_No) +
                                 ": '" + SectionStr +
                                 "': " + منزل_String(std::move(Err)));
  }

  أرجع &Section;
}

منطقي SpecialCaseList::parse(بنانشان FileIdx, ثابت يادگيري_بفر *MB,
                            std::string &Error) {
  بنانشان ڊگهو ڊگهو نسخو = 2;

  StringRef سرو = MB->get_بفر();
  إذا (سرو.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(سرو, 10, نسخو);

  خودڪار Min_نسخو = [&](بنانشان V) { أرجع نسخو >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  منطقي UseGlobs = Min_نسخو(2);
  منطقي RemoveDotSlash = Min_نسخو(3);
  منطقي WarnDotSlash = Min_نسخو(4) && !Min_نسخو(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  منطقي SlashAgnostic = Min_نسخو(4) && llvm::sys::رستو_2::is_style_windows(
                                            llvm::sys::رستو_2::Style::native);

  خودڪار ErrOrSection = جوڙيو_Section("*", FileIdx, 1, سچ);
  إذا (خودڪار Err = ErrOrSection.takeError()) {
    Error = منزل_String(std::move(Err));
    أرجع خطأ;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef رستو_Prefixes[] = {"src", "!src", "mainfile", "source"};

  لكل (سٽ_iterator سٽ_It(*MB, /*SkipBlanks=*/سچ, /*CommentMarker=*/'#');
       !سٽ_It.is_at_eof(); سٽ_It++) {
    بنانشان سٽ_No = سٽ_It.سٽ_عدد_2();
    StringRef سٽ = سٽ_It->trim();
    إذا (سٽ.خالي_جايون())
      استمر;

    // Save section names
    إذا (سٽ.starts_with("[")) {
      إذا (!سٽ.پڇاڙيون_with("]")) {
        Error =
            ("malformed section header on line " + Twine(سٽ_No) + ": " + سٽ)
                .str();
        أرجع خطأ;
      }

      خودڪار ErrOrSection =
          جوڙيو_Section(سٽ.drop_front().drop_پٺ(), FileIdx, سٽ_No, UseGlobs);
      إذا (خودڪار Err = ErrOrSection.takeError()) {
        Error = منزل_String(std::move(Err));
        أرجع خطأ;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      استمر;
    }

    // Get our prefix and unparsed glob.
    خودڪار [اڳياڙي, Postfix] = سٽ.ورهايو(":");
    إذا (Postfix.خالي_جايون()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(سٽ_No) + ": '" + سٽ + "'").str();
      أرجع خطأ;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    إذا (llvm::is_contained(رستو_Prefixes, اڳياڙي)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_ميلاپ = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    خودڪار [نمونو, Category] = Postfix.ورهايو("=");
    خودڪار [It, _] = CurrentImpl->Entries[اڳياڙي].try_emplace(Category, QOpts);
    نمونو = نمونو.copy(StrAlloc);
    إذا (خودڪار Err = It->ٻيو_2.وجهو(نمونو, سٽ_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(سٽ_No) + ": '" + نمونو + "': " + منزل_String(std::move(Err)))
              .str();
      أرجع خطأ;
    }
  }

  أرجع سچ;
}

SpecialCaseList::~SpecialCaseList() = افتراضي;

منطقي SpecialCaseList::اندريون_Section(StringRef Section, StringRef اڳياڙي,
                                StringRef Query, StringRef Category) ثابت {
  خودڪار [FileIdx, سٽ_No] = اندريون_Section_Blame(Section, اڳياڙي, Query, Category);
  أرجع سٽ_No;
}

std::pair<بنانشان, بنانشان>
SpecialCaseList::اندريون_Section_Blame(StringRef Section, StringRef اڳياڙي,
                                StringRef Query, StringRef Category) ثابت {
  لكل (ثابت خودڪار &S : ابتو(Sections)) {
    إذا (S.Impl->SectionMatcher.ميلاپ_Any(Section)) {
      بنانشان Blame = S.get_آخري_ميلاپ(اڳياڙي, Query, Category);
      إذا (Blame)
        أرجع {S.FileIdx, Blame};
    }
  }
  أرجع Not_مليو;
}

SpecialCaseList::Section::Section(StringRef Str, بنانشان FileIdx,
                                  منطقي UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/خطأ})) {}

SpecialCaseList::Section::Section(Section &&) = افتراضي;

SpecialCaseList::Section::~Section() = افتراضي;

منطقي SpecialCaseList::Section::ميلاپ_Name(StringRef Name) ثابت {
  أرجع Impl->SectionMatcher.ميلاپ_Any(Name);
}

ثابت Matcher *
SpecialCaseList::Section::SectionImpl::ڳوليو_Matcher(StringRef اڳياڙي,
                                                   StringRef Category) ثابت {
  SectionEntries::const_iterator I = Entries.ڳوليو(اڳياڙي);
  إذا (I == Entries.پڇاڙي())
    أرجع خالي_اشارو;
  StringMap<Matcher>::const_iterator II = I->ٻيو_2.ڳوليو(Category);
  إذا (II == I->ٻيو_2.پڇاڙي())
    أرجع خالي_اشارو;

  أرجع &II->ٻيو_2;
}

بنانشان SpecialCaseList::Section::get_آخري_ميلاپ(StringRef اڳياڙي,
                                                StringRef Query,
                                                StringRef Category) ثابت {
  إذا (ثابت Matcher *M = Impl->ڳوليو_Matcher(اڳياڙي, Category))
    أرجع M->ميلاپ(Query);
  أرجع 0;
}

منطقي SpecialCaseList::Section::has_اڳياڙي(StringRef اڳياڙي) ثابت {
  أرجع Impl->Entries.شامل_آهي(اڳياڙي);
}

} // namespace llvm
