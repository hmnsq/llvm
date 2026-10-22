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
  Error घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) अचर;
  StringRef findRule(अचिन्हित पङ्क्ति_No) अचर;

गुप्त:
  संरचना Reg {
    Reg(StringRef Name, अचिन्हित पङ्क्ति_No, Regex &&Rg)
        : Name(Name), पङ्क्ति_No(पङ्क्ति_No), Rg(std::move(Rg)) {}
    StringRef Name;
    अचिन्हित पङ्क्ति_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

वर्ग GlobMatcher {
सार्वजनिक:
  Error घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) अचर;
  StringRef findRule(अचिन्हित पङ्क्ति_No) अचर;

गुप्त:
  संरचना Glob {
    Glob(StringRef Name, अचिन्हित पङ्क्ति_No, GlobPattern &&Pattern)
        : Name(Name), पङ्क्ति_No(पङ्क्ति_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    अचिन्हित पङ्क्ति_No;
    GlobPattern Pattern;
  };

  शून्य LazyInit() अचर;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_रूख<iterator_range<StringRef::const_iterator>,
                    Radix_रूख<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<पूर्णाङ्क, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_रूख<iterator_range<StringRef::const_iterator>,
                    SmallVector<पूर्णाङ्क, 1>>
      SubstrToGlob;

  mutable तार्किक Initialized = असत्य;
};

संरचना QueryOptions {
  तार्किक UseGlobs = सत्य;
  तार्किक RemoveDotSlash = असत्य;
  तार्किक WarnDotSlashMatch = असत्य;
  तार्किक SlashAgnostic = असत्य;
};

/// Represents a set of patterns and their line numbers
वर्ग Matcher {
सार्वजनिक:
  explicit Matcher(QueryOptions QOpts);

  Error घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या);
  अचिन्हित match(StringRef Query) अचर;

  तार्किक matchAny(StringRef Query) अचर { फर्काऊ match(Query); }

गुप्त:
  अचिन्हित matchInternal(StringRef Query) अचर;
  StringRef findRule(अचिन्हित पङ्क्ति_No) अचर;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या,
                           तार्किक SlashAgnostic) {
  यदि (Pattern.empty())
    फर्काऊ createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  स्वतः Regexp = Pattern.str();
  लागि (माप_प्रकार pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += म_माप(".*")) {
    Regexp.replace(pos, म_माप("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  यदि (!CheckRE.isValid(REError))
    फर्काऊ createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, पङ्क्ति_सङ्ख्या, std::move(CheckRE));
  फर्काऊ Error::success();
}

अचिन्हित RegexMatcher::match(StringRef Query) अचर {
  लागि (अचर स्वतः &R : reverse(RegExes))
    यदि (R.Rg.match(Query))
      फर्काऊ R.पङ्क्ति_No;
  फर्काऊ 0;
}

StringRef RegexMatcher::findRule(अचिन्हित पङ्क्ति_No) अचर {
  लागि (अचर स्वतः &R : RegExes)
    यदि (R.पङ्क्ति_No == पङ्क्ति_No)
      फर्काऊ R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  फर्काऊ {};
}

Error GlobMatcher::घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या,
                          तार्किक SlashAgnostic) {
  यदि (Pattern.empty())
    फर्काऊ createStringError(errc::invalid_argument, "Supplied glob was blank");

  स्वतः Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  यदि (स्वतः Err = Res.takeError())
    फर्काऊ Err;
  Globs.emplace_back(Pattern, पङ्क्ति_सङ्ख्या, std::move(Res.get()));
  फर्काऊ Error::success();
}

शून्य GlobMatcher::LazyInit() अचर {
  यदि (LLVM_LIKELY(Initialized))
    फर्काऊ;
  Initialized = सत्य;
  लागि (अचर स्वतः &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    यदि (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      यदि (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        स्वतः &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        जारी;
      }
    }

    स्वतः &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    स्वतः &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

अचिन्हित GlobMatcher::match(StringRef Query) अचर {
  LazyInit();

  पूर्णाङ्क Best = -1;
  यदि (!PrefixSuffixToGlob.empty()) {
    लागि (अचर स्वतः &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      लागि (अचर स्वतः &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        लागि (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            रोक;
          अचर GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            रोक;
          }
        }
      }
    }
  }

  यदि (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    लागि (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      लागि (अचर स्वतः &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        लागि (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            रोक;
          अचर GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            रोक;
          }
        }
      }
    }
  }
  फर्काऊ Best < 0 ? 0 : Globs[Best].पङ्क्ति_No;
}

StringRef GlobMatcher::findRule(अचिन्हित पङ्क्ति_No) अचर {
  लागि (अचर स्वतः &G : Globs)
    यदि (G.पङ्क्ति_No == पङ्क्ति_No)
      फर्काऊ G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  फर्काऊ {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  यदि (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  अन्यथा
    M.emplace<RegexMatcher>();
}

Error Matcher::घुसाउ(StringRef Pattern, अचिन्हित पङ्क्ति_सङ्ख्या) {
  फर्काऊ std::visit(
      [&](स्वतः &V) {
        फर्काऊ V.घुसाउ(Pattern, पङ्क्ति_सङ्ख्या, Options.SlashAgnostic);
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
अचिन्हित Matcher::match(StringRef Query) अचर {
  यदि (!Options.RemoveDotSlash)
    फर्काऊ matchInternal(Query);

  यदि (!Options.WarnDotSlashMatch)
    फर्काऊ matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  अचिन्हित FixedMatched = matchInternal(FixedQuery);
  यदि (FixedQuery == Query)
    फर्काऊ FixedMatched;

  अचिन्हित OriginalMatch = matchInternal(Query);
  यदि (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  फर्काऊ std::max(OriginalMatch, FixedMatched);
}

अचिन्हित Matcher::matchInternal(StringRef Query) अचर {
  फर्काऊ std::visit([&](स्वतः &V) -> अचिन्हित { फर्काऊ V.match(Query); }, M);
}

StringRef Matcher::findRule(अचिन्हित पङ्क्ति_No) अचर {
  फर्काऊ std::visit([&](स्वतः &V) -> StringRef { फर्काऊ V.findRule(पङ्क्ति_No); },
                    M);
}
} // namespace

वर्ग SpecialCaseList::Section::SectionImpl {
सार्वजनिक:
  अचर Matcher *findMatcher(StringRef Prefix, StringRef Category) अचर;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(अचर std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->createInternal(Paths, FS, Error))
    फर्काऊ SCL;
  फर्काऊ nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(अचर MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->createInternal(MB, Error))
    फर्काऊ SCL;
  फर्काऊ nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(अचर std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  यदि (स्वतः SCL = create(Paths, FS, Error))
    फर्काऊ SCL;
  report_fatal_error(Twine(Error));
}

तार्किक SpecialCaseList::createInternal(अचर std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  लागि (माप_प्रकार i = 0; i < Paths.size(); ++i) {
    अचर स्वतः &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    यदि (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      फर्काऊ असत्य;
    }
    std::string ParseError;
    यदि (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      फर्काऊ असत्य;
    }
  }
  फर्काऊ सत्य;
}

तार्किक SpecialCaseList::createInternal(अचर MemoryBuffer *MB,
                                     std::string &Error) {
  यदि (!parse(0, MB, Error))
    फर्काऊ असत्य;
  फर्काऊ सत्य;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, अचिन्हित FileNo,
                            अचिन्हित पङ्क्ति_No, तार्किक UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  स्वतः &Section = Sections.back();

  यदि (स्वतः Err = Section.Impl->SectionMatcher.घुसाउ(SectionStr, पङ्क्ति_No)) {
    फर्काऊ createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(पङ्क्ति_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  फर्काऊ &Section;
}

तार्किक SpecialCaseList::parse(अचिन्हित FileIdx, अचर MemoryBuffer *MB,
                            std::string &Error) {
  अचिन्हित दीर्घ दीर्घ Version = 2;

  StringRef Header = MB->getBuffer();
  यदि (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  स्वतः MinVersion = [&](अचिन्हित V) { फर्काऊ Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  तार्किक UseGlobs = MinVersion(2);
  तार्किक RemoveDotSlash = MinVersion(3);
  तार्किक WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  तार्किक SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  स्वतः ErrOrSection = addSection("*", FileIdx, 1, सत्य);
  यदि (स्वतः Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    फर्काऊ असत्य;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  लागि (पङ्क्ति_iterator पङ्क्ति_It(*MB, /*SkipBlanks=*/सत्य, /*CommentMarker=*/'#');
       !पङ्क्ति_It.is_at_eof(); पङ्क्ति_It++) {
    अचिन्हित पङ्क्ति_No = पङ्क्ति_It.पङ्क्ति_सङ्ख्या_2();
    StringRef पङ्क्ति = पङ्क्ति_It->trim();
    यदि (पङ्क्ति.empty())
      जारी;

    // Save section names
    यदि (पङ्क्ति.starts_with("[")) {
      यदि (!पङ्क्ति.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(पङ्क्ति_No) + ": " + पङ्क्ति)
                .str();
        फर्काऊ असत्य;
      }

      स्वतः ErrOrSection =
          addSection(पङ्क्ति.drop_front().drop_back(), FileIdx, पङ्क्ति_No, UseGlobs);
      यदि (स्वतः Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        फर्काऊ असत्य;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      जारी;
    }

    // Get our prefix and unparsed glob.
    स्वतः [Prefix, Postfix] = पङ्क्ति.split(":");
    यदि (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(पङ्क्ति_No) + ": '" + पङ्क्ति + "'").str();
      फर्काऊ असत्य;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    यदि (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    स्वतः [Pattern, Category] = Postfix.split("=");
    स्वतः [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    यदि (स्वतः Err = It->second.घुसाउ(Pattern, पङ्क्ति_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(पङ्क्ति_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      फर्काऊ असत्य;
    }
  }

  फर्काऊ सत्य;
}

SpecialCaseList::~SpecialCaseList() = स्वभाव;

तार्किक SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) अचर {
  स्वतः [FileIdx, पङ्क्ति_No] = inSectionBlame(Section, Prefix, Query, Category);
  फर्काऊ पङ्क्ति_No;
}

std::pair<अचिन्हित, अचिन्हित>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) अचर {
  लागि (अचर स्वतः &S : reverse(Sections)) {
    यदि (S.Impl->SectionMatcher.matchAny(Section)) {
      अचिन्हित Blame = S.getLastMatch(Prefix, Query, Category);
      यदि (Blame)
        फर्काऊ {S.FileIdx, Blame};
    }
  }
  फर्काऊ NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, अचिन्हित FileIdx,
                                  तार्किक UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/असत्य})) {}

SpecialCaseList::Section::Section(Section &&) = स्वभाव;

SpecialCaseList::Section::~Section() = स्वभाव;

तार्किक SpecialCaseList::Section::matchName(StringRef Name) अचर {
  फर्काऊ Impl->SectionMatcher.matchAny(Name);
}

अचर Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) अचर {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  यदि (I == Entries.end())
    फर्काऊ nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  यदि (II == I->second.end())
    फर्काऊ nullptr;

  फर्काऊ &II->second;
}

अचिन्हित SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) अचर {
  यदि (अचर Matcher *M = Impl->findMatcher(Prefix, Category))
    फर्काऊ M->match(Query);
  फर्काऊ 0;
}

तार्किक SpecialCaseList::Section::hasPrefix(StringRef Prefix) अचर {
  फर्काऊ Impl->Entries.contains(Prefix);
}

} // namespace llvm
