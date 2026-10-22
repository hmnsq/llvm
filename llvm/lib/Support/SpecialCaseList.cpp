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

#अन्तर्भाव "llvm/Support/SpecialCaseList.h"
#अन्तर्भाव "llvm/ADT/RadixTree.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/iterator_range.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/GlobPattern.h"
#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Regex.h"
#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/Support/WithColor.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>
#अन्तर्भाव <variant>
#अन्तर्भाव <vector>

नामाकाश llvm {

नामाकाश {

// Lagacy v1 matcher.
वर्ग RegexMatcher {
सार्वजनिक:
  Error निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या, तार्किक SlashAgnostic);
  अचिह्नित मेलनम्(StringRef Query) ध्रुव;
  StringRef find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव;

गुप्त:
  संरचना Reg {
    Reg(StringRef Name, अचिह्नित पङ्क्तिः_No, Regex &&Rg)
        : Name(Name), पङ्क्तिः_No(पङ्क्तिः_No), Rg(std::move(Rg)) {}
    StringRef Name;
    अचिह्नित पङ्क्तिः_No;
    Regex Rg;
  };

  std::सदिशम्<Reg> RegExes;
};

वर्ग GlobMatcher {
सार्वजनिक:
  Error निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या, तार्किक SlashAgnostic);
  अचिह्नित मेलनम्(StringRef Query) ध्रुव;
  StringRef find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव;

गुप्त:
  संरचना Glob {
    Glob(StringRef Name, अचिह्नित पङ्क्तिः_No, Glob_आदर्शः &&आदर्शः)
        : Name(Name), पङ्क्तिः_No(पङ्क्तिः_No), आदर्शः(std::move(आदर्शः)) {}
    StringRef Name;
    अचिह्नित पङ्क्तिः_No;
    Glob_आदर्शः आदर्शः;
  };

  शून्य LazyInit() ध्रुव;

  std::सदिशम्<GlobMatcher::Glob> Globs;

  परिवर्तनीय Radix_वृक्षः<iterator_range<StringRef::const_iterator>,
                    Radix_वृक्षः<iterator_range<StringRef::const_reverse_iterator>,
                              Small_सदिशम्<पूर्णाङ्क, 1>>>
      उपसर्गः_Suffix_गन्तव्यम्_Glob;

  परिवर्तनीय Radix_वृक्षः<iterator_range<StringRef::const_iterator>,
                    Small_सदिशम्<पूर्णाङ्क, 1>>
      Substr_गन्तव्यम्_Glob;

  परिवर्तनीय तार्किक Initialized = असत्य;
};

संरचना QueryOptions {
  तार्किक UseGlobs = सत्य;
  तार्किक RemoveDotSlash = असत्य;
  तार्किक Warn_Dot_Slash_मेलनम् = असत्य;
  तार्किक SlashAgnostic = असत्य;
};

/// Represents a set of patterns and their line numbers
वर्ग Matcher {
सार्वजनिक:
  स्पष्ट Matcher(QueryOptions QOpts);

  Error निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या);
  अचिह्नित मेलनम्(StringRef Query) ध्रुव;

  तार्किक मेलनम्_Any(StringRef Query) ध्रुव { प्रत्यागम मेलनम्(Query); }

गुप्त:
  अचिह्नित मेलनम्_आन्तरिकम्(StringRef Query) ध्रुव;
  StringRef find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  परिवर्तनीय std::once_flag Warned;
};

Error RegexMatcher::निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या,
                           तार्किक SlashAgnostic) {
  यदि (आदर्शः.रिक्तानि())
    प्रत्यागम createStringError(errc::invalid_तर्कः,
                             "Supplied regex was blank");

  // Replace * with .*
  स्वचालित Regexp = आदर्शः.str();
  प्रति (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += वाक्यदैर्घ्यम्(".*")) {
    Regexp.replace(pos, वाक्यदैर्घ्यम्("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  यदि (!CheckRE.isValid(REError))
    प्रत्यागम createStringError(errc::invalid_तर्कः, REError);

  RegExes.emplace_पृष्ठभागः(आदर्शः, पङ्क्तिः_सङ्ख्या, std::move(CheckRE));
  प्रत्यागम Error::success();
}

अचिह्नित RegexMatcher::मेलनम्(StringRef Query) ध्रुव {
  प्रति (ध्रुव स्वचालित &R : reverse(RegExes))
    यदि (R.Rg.मेलनम्(Query))
      प्रत्यागम R.पङ्क्तिः_No;
  प्रत्यागम 0;
}

StringRef RegexMatcher::find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव {
  प्रति (ध्रुव स्वचालित &R : RegExes)
    यदि (R.पङ्क्तिः_No == पङ्क्तिः_No)
      प्रत्यागम R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  प्रत्यागम {};
}

Error GlobMatcher::निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या,
                          तार्किक SlashAgnostic) {
  यदि (आदर्शः.रिक्तानि())
    प्रत्यागम createStringError(errc::invalid_तर्कः, "Supplied glob was blank");

  स्वचालित Res =
      Glob_आदर्शः::create(आदर्शः, /*MaxSubPatterns=*/1024, SlashAgnostic);
  यदि (स्वचालित Err = Res.takeError())
    प्रत्यागम Err;
  Globs.emplace_पृष्ठभागः(आदर्शः, पङ्क्तिः_सङ्ख्या, std::move(Res.get()));
  प्रत्यागम Error::success();
}

शून्य GlobMatcher::LazyInit() ध्रुव {
  यदि (LLVM_LIKELY(Initialized))
    प्रत्यागम;
  Initialized = सत्य;
  प्रति (ध्रुव स्वचालित &[Idx, G] : enumerate(Globs)) {
    StringRef उपसर्गः = G.आदर्शः.उपसर्गः_2();
    StringRef Suffix = G.आदर्शः.suffix();

    यदि (Suffix.रिक्तानि() && उपसर्गः.रिक्तानि()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.आदर्शः.longest_substr();
      यदि (!Substr.रिक्तानि()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        स्वचालित &V = Substr_गन्तव्यम्_Glob.emplace(Substr).first->second;
        V.emplace_पृष्ठभागः(Idx);
        अनुवर्त;
      }
    }

    स्वचालित &SToGlob = उपसर्गः_Suffix_गन्तव्यम्_Glob.emplace(उपसर्गः).first->second;
    स्वचालित &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_पृष्ठभागः(Idx);
  }
}

अचिह्नित GlobMatcher::मेलनम्(StringRef Query) ध्रुव {
  LazyInit();

  पूर्णाङ्क Best = -1;
  यदि (!उपसर्गः_Suffix_गन्तव्यम्_Glob.रिक्तानि()) {
    प्रति (ध्रुव स्वचालित &[_, SToGlob] : उपसर्गः_Suffix_गन्तव्यम्_Glob.find_prefixes(Query)) {
      प्रति (ध्रुव स्वचालित &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.आदर्शः.मेलनम्(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            भङ्ग;
          }
        }
      }
    }
  }

  यदि (!Substr_गन्तव्यम्_Glob.रिक्तानि()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    प्रति (StringRef Q = Query; !Q.रिक्तानि(); Q = Q.drop_front()) {
      प्रति (ध्रुव स्वचालित &[_, V] : Substr_गन्तव्यम्_Glob.find_prefixes(Q)) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.आदर्शः.मेलनम्(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            भङ्ग;
          }
        }
      }
    }
  }
  प्रत्यागम Best < 0 ? 0 : Globs[Best].पङ्क्तिः_No;
}

StringRef GlobMatcher::find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव {
  प्रति (ध्रुव स्वचालित &G : Globs)
    यदि (G.पङ्क्तिः_No == पङ्क्तिः_No)
      प्रत्यागम G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  प्रत्यागम {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  यदि (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  अन्यथा
    M.emplace<RegexMatcher>();
}

Error Matcher::निवेशय(StringRef आदर्शः, अचिह्नित पङ्क्तिः_सङ्ख्या) {
  प्रत्यागम std::दर्शय(
      [&](स्वचालित &V) {
        प्रत्यागम V.निवेशय(आदर्शः, पङ्क्तिः_सङ्ख्या, Options.SlashAgnostic);
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
अचिह्नित Matcher::मेलनम्(StringRef Query) ध्रुव {
  यदि (!Options.RemoveDotSlash)
    प्रत्यागम मेलनम्_आन्तरिकम्(Query);

  यदि (!Options.Warn_Dot_Slash_मेलनम्)
    प्रत्यागम मेलनम्_आन्तरिकम्(llvm::sys::मार्गः_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::मार्गः_2::remove_leading_dotslash(Query);
  अचिह्नित FixedMatched = मेलनम्_आन्तरिकम्(FixedQuery);
  यदि (FixedQuery == Query)
    प्रत्यागम FixedMatched;

  अचिह्नित Original_मेलनम् = मेलनम्_आन्तरिकम्(Query);
  यदि (Original_मेलनम् > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << find_नियमः(Original_मेलनम्) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  प्रत्यागम std::max(Original_मेलनम्, FixedMatched);
}

अचिह्नित Matcher::मेलनम्_आन्तरिकम्(StringRef Query) ध्रुव {
  प्रत्यागम std::दर्शय([&](स्वचालित &V) -> अचिह्नित { प्रत्यागम V.मेलनम्(Query); }, M);
}

StringRef Matcher::find_नियमः(अचिह्नित पङ्क्तिः_No) ध्रुव {
  प्रत्यागम std::दर्शय([&](स्वचालित &V) -> StringRef { प्रत्यागम V.find_नियमः(पङ्क्तिः_No); },
                    M);
}
} // namespace

वर्ग SpecialCaseList::Section::SectionImpl {
सार्वजनिक:
  ध्रुव Matcher *findMatcher(StringRef उपसर्गः, StringRef Category) ध्रुव;

  उपयुज्य SectionEntries = StringMap<StringMap<Matcher>>;

  स्पष्ट SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ध्रुव std::सदिशम्<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->create_आन्तरिकम्(Paths, FS, Error))
    प्रत्यागम SCL;
  प्रत्यागम शून्यसूचकः;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ध्रुव स्मृतिः_बफर *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->create_आन्तरिकम्(MB, Error))
    प्रत्यागम SCL;
  प्रत्यागम शून्यसूचकः;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ध्रुव std::सदिशम्<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  यदि (स्वचालित SCL = create(Paths, FS, Error))
    प्रत्यागम SCL;
  report_fatal_error(Twine(Error));
}

तार्किक SpecialCaseList::create_आन्तरिकम्(ध्रुव std::सदिशम्<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  प्रति (size_t i = 0; i < Paths.आकार_2(); ++i) {
    ध्रुव स्वचालित &मार्गः = Paths[i];
    ErrorOr<std::unique_ptr<स्मृतिः_बफर>> FileOrErr =
        VFS.get_बफर_For_File(मार्गः);
    यदि (std::error_संहिता EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + मार्गः + "': " + EC.सन्देशः_2()).str();
      प्रत्यागम असत्य;
    }
    std::string ParseError;
    यदि (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + मार्गः + "': " + ParseError).str();
      प्रत्यागम असत्य;
    }
  }
  प्रत्यागम सत्य;
}

तार्किक SpecialCaseList::create_आन्तरिकम्(ध्रुव स्मृतिः_बफर *MB,
                                     std::string &Error) {
  यदि (!parse(0, MB, Error))
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::सङ्कलय_Section(StringRef SectionStr, अचिह्नित FileNo,
                            अचिह्नित पङ्क्तिः_No, तार्किक UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_पृष्ठभागः(SectionStr, FileNo, UseGlobs);
  स्वचालित &Section = Sections.पृष्ठभागः();

  यदि (स्वचालित Err = Section.Impl->SectionMatcher.निवेशय(SectionStr, पङ्क्तिः_No)) {
    प्रत्यागम createStringError(errc::invalid_तर्कः,
                             "malformed section at line " + Twine(पङ्क्तिः_No) +
                                 ": '" + SectionStr +
                                 "': " + गन्तव्यम्_String(std::move(Err)));
  }

  प्रत्यागम &Section;
}

तार्किक SpecialCaseList::parse(अचिह्नित FileIdx, ध्रुव स्मृतिः_बफर *MB,
                            std::string &Error) {
  अचिह्नित दीर्घ दीर्घ संस्करणम् = 2;

  StringRef शीर्षकम् = MB->get_बफर();
  यदि (शीर्षकम्.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(शीर्षकम्, 10, संस्करणम्);

  स्वचालित Min_संस्करणम् = [&](अचिह्नित V) { प्रत्यागम संस्करणम् >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  तार्किक UseGlobs = Min_संस्करणम्(2);
  तार्किक RemoveDotSlash = Min_संस्करणम्(3);
  तार्किक WarnDotSlash = Min_संस्करणम्(4) && !Min_संस्करणम्(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  तार्किक SlashAgnostic = Min_संस्करणम्(4) && llvm::sys::मार्गः_2::is_style_windows(
                                            llvm::sys::मार्गः_2::Style::native);

  स्वचालित ErrOrSection = सङ्कलय_Section("*", FileIdx, 1, सत्य);
  यदि (स्वचालित Err = ErrOrSection.takeError()) {
    Error = गन्तव्यम्_String(std::move(Err));
    प्रत्यागम असत्य;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef मार्गः_Prefixes[] = {"src", "!src", "mainfile", "source"};

  प्रति (पङ्क्तिः_iterator पङ्क्तिः_It(*MB, /*SkipBlanks=*/सत्य, /*CommentMarker=*/'#');
       !पङ्क्तिः_It.is_at_eof(); पङ्क्तिः_It++) {
    अचिह्नित पङ्क्तिः_No = पङ्क्तिः_It.पङ्क्तिः_सङ्ख्या_2();
    StringRef पङ्क्तिः = पङ्क्तिः_It->trim();
    यदि (पङ्क्तिः.रिक्तानि())
      अनुवर्त;

    // Save section names
    यदि (पङ्क्तिः.starts_with("[")) {
      यदि (!पङ्क्तिः.अन्ताः_with("]")) {
        Error =
            ("malformed section header on line " + Twine(पङ्क्तिः_No) + ": " + पङ्क्तिः)
                .str();
        प्रत्यागम असत्य;
      }

      स्वचालित ErrOrSection =
          सङ्कलय_Section(पङ्क्तिः.drop_front().drop_पृष्ठभागः(), FileIdx, पङ्क्तिः_No, UseGlobs);
      यदि (स्वचालित Err = ErrOrSection.takeError()) {
        Error = गन्तव्यम्_String(std::move(Err));
        प्रत्यागम असत्य;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      अनुवर्त;
    }

    // Get our prefix and unparsed glob.
    स्वचालित [उपसर्गः, Postfix] = पङ्क्तिः.विभज(":");
    यदि (Postfix.रिक्तानि()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(पङ्क्तिः_No) + ": '" + पङ्क्तिः + "'").str();
      प्रत्यागम असत्य;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    यदि (llvm::is_contained(मार्गः_Prefixes, उपसर्गः)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_मेलनम् = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    स्वचालित [आदर्शः, Category] = Postfix.विभज("=");
    स्वचालित [It, _] = CurrentImpl->Entries[उपसर्गः].try_emplace(Category, QOpts);
    आदर्शः = आदर्शः.copy(StrAlloc);
    यदि (स्वचालित Err = It->second.निवेशय(आदर्शः, पङ्क्तिः_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(पङ्क्तिः_No) + ": '" + आदर्शः + "': " + गन्तव्यम्_String(std::move(Err)))
              .str();
      प्रत्यागम असत्य;
    }
  }

  प्रत्यागम सत्य;
}

SpecialCaseList::~SpecialCaseList() = स्वभाव;

तार्किक SpecialCaseList::अन्तर्_Section(StringRef Section, StringRef उपसर्गः,
                                StringRef Query, StringRef Category) ध्रुव {
  स्वचालित [FileIdx, पङ्क्तिः_No] = अन्तर्_Section_Blame(Section, उपसर्गः, Query, Category);
  प्रत्यागम पङ्क्तिः_No;
}

std::pair<अचिह्नित, अचिह्नित>
SpecialCaseList::अन्तर्_Section_Blame(StringRef Section, StringRef उपसर्गः,
                                StringRef Query, StringRef Category) ध्रुव {
  प्रति (ध्रुव स्वचालित &S : reverse(Sections)) {
    यदि (S.Impl->SectionMatcher.मेलनम्_Any(Section)) {
      अचिह्नित Blame = S.get_चरमः_मेलनम्(उपसर्गः, Query, Category);
      यदि (Blame)
        प्रत्यागम {S.FileIdx, Blame};
    }
  }
  प्रत्यागम Not_प्राप्तम्;
}

SpecialCaseList::Section::Section(StringRef Str, अचिह्नित FileIdx,
                                  तार्किक UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/असत्य})) {}

SpecialCaseList::Section::Section(Section &&) = स्वभाव;

SpecialCaseList::Section::~Section() = स्वभाव;

तार्किक SpecialCaseList::Section::मेलनम्_Name(StringRef Name) ध्रुव {
  प्रत्यागम Impl->SectionMatcher.मेलनम्_Any(Name);
}

ध्रुव Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef उपसर्गः,
                                                   StringRef Category) ध्रुव {
  SectionEntries::const_iterator I = Entries.find(उपसर्गः);
  यदि (I == Entries.अन्तः())
    प्रत्यागम शून्यसूचकः;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  यदि (II == I->second.अन्तः())
    प्रत्यागम शून्यसूचकः;

  प्रत्यागम &II->second;
}

अचिह्नित SpecialCaseList::Section::get_चरमः_मेलनम्(StringRef उपसर्गः,
                                                StringRef Query,
                                                StringRef Category) ध्रुव {
  यदि (ध्रुव Matcher *M = Impl->findMatcher(उपसर्गः, Category))
    प्रत्यागम M->मेलनम्(Query);
  प्रत्यागम 0;
}

तार्किक SpecialCaseList::Section::has_उपसर्गः(StringRef उपसर्गः) ध्रुव {
  प्रत्यागम Impl->Entries.अन्तर्भवति(उपसर्गः);
}

} // namespace llvm
