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
  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) ध्रुव;
  StringRef findRule(अचिन्हित पंक्ति_No) ध्रुव;

गुप्त:
  संरचना Reg {
    Reg(StringRef Name, अचिन्हित पंक्ति_No, Regex &&Rg)
        : Name(Name), पंक्ति_No(पंक्ति_No), Rg(std::move(Rg)) {}
    StringRef Name;
    अचिन्हित पंक्ति_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

वर्ग GlobMatcher {
सार्वजनिक:
  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) ध्रुव;
  StringRef findRule(अचिन्हित पंक्ति_No) ध्रुव;

गुप्त:
  संरचना Glob {
    Glob(StringRef Name, अचिन्हित पंक्ति_No, GlobPattern &&Pattern)
        : Name(Name), पंक्ति_No(पंक्ति_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    अचिन्हित पंक्ति_No;
    GlobPattern Pattern;
  };

  शून्य LazyInit() ध्रुव;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
                    Radix_वृक्ष<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<पूर्णाङ्क, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
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

  Error जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या);
  अचिन्हित match(StringRef Query) ध्रुव;

  तार्किक matchAny(StringRef Query) ध्रुव { प्रत्यागम match(Query); }

गुप्त:
  अचिन्हित matchInternal(StringRef Query) ध्रुव;
  StringRef findRule(अचिन्हित पंक्ति_No) ध्रुव;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या,
                           तार्किक SlashAgnostic) {
  यदि (Pattern.empty())
    प्रत्यागम createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  स्वतः Regexp = Pattern.str();
  प्रति (माप_प्रकार pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += म_माप(".*")) {
    Regexp.replace(pos, म_माप("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  यदि (!CheckRE.isValid(REError))
    प्रत्यागम createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, पंक्ति_संख्या, std::move(CheckRE));
  प्रत्यागम Error::success();
}

अचिन्हित RegexMatcher::match(StringRef Query) ध्रुव {
  प्रति (ध्रुव स्वतः &R : reverse(RegExes))
    यदि (R.Rg.match(Query))
      प्रत्यागम R.पंक्ति_No;
  प्रत्यागम 0;
}

StringRef RegexMatcher::findRule(अचिन्हित पंक्ति_No) ध्रुव {
  प्रति (ध्रुव स्वतः &R : RegExes)
    यदि (R.पंक्ति_No == पंक्ति_No)
      प्रत्यागम R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  प्रत्यागम {};
}

Error GlobMatcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या,
                          तार्किक SlashAgnostic) {
  यदि (Pattern.empty())
    प्रत्यागम createStringError(errc::invalid_argument, "Supplied glob was blank");

  स्वतः Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  यदि (स्वतः Err = Res.takeError())
    प्रत्यागम Err;
  Globs.emplace_back(Pattern, पंक्ति_संख्या, std::move(Res.get()));
  प्रत्यागम Error::success();
}

शून्य GlobMatcher::LazyInit() ध्रुव {
  यदि (LLVM_LIKELY(Initialized))
    प्रत्यागम;
  Initialized = सत्य;
  प्रति (ध्रुव स्वतः &[Idx, G] : enumerate(Globs)) {
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
        अनुवर्त;
      }
    }

    स्वतः &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    स्वतः &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

अचिन्हित GlobMatcher::match(StringRef Query) ध्रुव {
  LazyInit();

  पूर्णाङ्क Best = -1;
  यदि (!PrefixSuffixToGlob.empty()) {
    प्रति (ध्रुव स्वतः &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      प्रति (ध्रुव स्वतः &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.Pattern.match(Query)) {
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

  यदि (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    प्रति (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      प्रति (ध्रुव स्वतः &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        प्रति (पूर्णाङ्क Idx : reverse(V)) {
          यदि (Best > Idx)
            भङ्ग;
          ध्रुव GlobMatcher::Glob &G = Globs[Idx];
          यदि (G.Pattern.match(Query)) {
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
  प्रत्यागम Best < 0 ? 0 : Globs[Best].पंक्ति_No;
}

StringRef GlobMatcher::findRule(अचिन्हित पंक्ति_No) ध्रुव {
  प्रति (ध्रुव स्वतः &G : Globs)
    यदि (G.पंक्ति_No == पंक्ति_No)
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

Error Matcher::जोड़ो(StringRef Pattern, अचिन्हित पंक्ति_संख्या) {
  प्रत्यागम std::visit(
      [&](स्वतः &V) {
        प्रत्यागम V.जोड़ो(Pattern, पंक्ति_संख्या, Options.SlashAgnostic);
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
अचिन्हित Matcher::match(StringRef Query) ध्रुव {
  यदि (!Options.RemoveDotSlash)
    प्रत्यागम matchInternal(Query);

  यदि (!Options.WarnDotSlashMatch)
    प्रत्यागम matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  अचिन्हित FixedMatched = matchInternal(FixedQuery);
  यदि (FixedQuery == Query)
    प्रत्यागम FixedMatched;

  अचिन्हित OriginalMatch = matchInternal(Query);
  यदि (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  प्रत्यागम std::max(OriginalMatch, FixedMatched);
}

अचिन्हित Matcher::matchInternal(StringRef Query) ध्रुव {
  प्रत्यागम std::visit([&](स्वतः &V) -> अचिन्हित { प्रत्यागम V.match(Query); }, M);
}

StringRef Matcher::findRule(अचिन्हित पंक्ति_No) ध्रुव {
  प्रत्यागम std::visit([&](स्वतः &V) -> StringRef { प्रत्यागम V.findRule(पंक्ति_No); },
                    M);
}
} // namespace

वर्ग SpecialCaseList::Section::SectionImpl {
सार्वजनिक:
  ध्रुव Matcher *findMatcher(StringRef Prefix, StringRef Category) ध्रुव;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(ध्रुव std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->createInternal(Paths, FS, Error))
    प्रत्यागम SCL;
  प्रत्यागम nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(ध्रुव MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नूतन SpecialCaseList());
  यदि (SCL->createInternal(MB, Error))
    प्रत्यागम SCL;
  प्रत्यागम nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(ध्रुव std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  यदि (स्वतः SCL = create(Paths, FS, Error))
    प्रत्यागम SCL;
  report_fatal_error(Twine(Error));
}

तार्किक SpecialCaseList::createInternal(ध्रुव std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  प्रति (माप_प्रकार i = 0; i < Paths.size(); ++i) {
    ध्रुव स्वतः &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    यदि (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      प्रत्यागम असत्य;
    }
    std::string ParseError;
    यदि (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      प्रत्यागम असत्य;
    }
  }
  प्रत्यागम सत्य;
}

तार्किक SpecialCaseList::createInternal(ध्रुव MemoryBuffer *MB,
                                     std::string &Error) {
  यदि (!parse(0, MB, Error))
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, अचिन्हित FileNo,
                            अचिन्हित पंक्ति_No, तार्किक UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  स्वतः &Section = Sections.back();

  यदि (स्वतः Err = Section.Impl->SectionMatcher.जोड़ो(SectionStr, पंक्ति_No)) {
    प्रत्यागम createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(पंक्ति_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  प्रत्यागम &Section;
}

तार्किक SpecialCaseList::parse(अचिन्हित FileIdx, ध्रुव MemoryBuffer *MB,
                            std::string &Error) {
  अचिन्हित दीर्घ दीर्घ Version = 2;

  StringRef Header = MB->getBuffer();
  यदि (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  स्वतः MinVersion = [&](अचिन्हित V) { प्रत्यागम Version >= V; };

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
    प्रत्यागम असत्य;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  प्रति (पंक्ति_iterator पंक्ति_It(*MB, /*SkipBlanks=*/सत्य, /*CommentMarker=*/'#');
       !पंक्ति_It.is_at_eof(); पंक्ति_It++) {
    अचिन्हित पंक्ति_No = पंक्ति_It.पंक्ति_संख्या_2();
    StringRef पंक्ति = पंक्ति_It->trim();
    यदि (पंक्ति.empty())
      अनुवर्त;

    // Save section names
    यदि (पंक्ति.starts_with("[")) {
      यदि (!पंक्ति.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(पंक्ति_No) + ": " + पंक्ति)
                .str();
        प्रत्यागम असत्य;
      }

      स्वतः ErrOrSection =
          addSection(पंक्ति.drop_front().drop_back(), FileIdx, पंक्ति_No, UseGlobs);
      यदि (स्वतः Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        प्रत्यागम असत्य;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      अनुवर्त;
    }

    // Get our prefix and unparsed glob.
    स्वतः [Prefix, Postfix] = पंक्ति.split(":");
    यदि (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(पंक्ति_No) + ": '" + पंक्ति + "'").str();
      प्रत्यागम असत्य;
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
    यदि (स्वतः Err = It->second.जोड़ो(Pattern, पंक्ति_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(पंक्ति_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      प्रत्यागम असत्य;
    }
  }

  प्रत्यागम सत्य;
}

SpecialCaseList::~SpecialCaseList() = स्वभाव;

तार्किक SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ध्रुव {
  स्वतः [FileIdx, पंक्ति_No] = inSectionBlame(Section, Prefix, Query, Category);
  प्रत्यागम पंक्ति_No;
}

std::pair<अचिन्हित, अचिन्हित>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) ध्रुव {
  प्रति (ध्रुव स्वतः &S : reverse(Sections)) {
    यदि (S.Impl->SectionMatcher.matchAny(Section)) {
      अचिन्हित Blame = S.getLastMatch(Prefix, Query, Category);
      यदि (Blame)
        प्रत्यागम {S.FileIdx, Blame};
    }
  }
  प्रत्यागम NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, अचिन्हित FileIdx,
                                  तार्किक UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/असत्य})) {}

SpecialCaseList::Section::Section(Section &&) = स्वभाव;

SpecialCaseList::Section::~Section() = स्वभाव;

तार्किक SpecialCaseList::Section::matchName(StringRef Name) ध्रुव {
  प्रत्यागम Impl->SectionMatcher.matchAny(Name);
}

ध्रुव Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) ध्रुव {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  यदि (I == Entries.end())
    प्रत्यागम nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  यदि (II == I->second.end())
    प्रत्यागम nullptr;

  प्रत्यागम &II->second;
}

अचिन्हित SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) ध्रुव {
  यदि (ध्रुव Matcher *M = Impl->findMatcher(Prefix, Category))
    प्रत्यागम M->match(Query);
  प्रत्यागम 0;
}

तार्किक SpecialCaseList::Section::hasPrefix(StringRef Prefix) ध्रुव {
  प्रत्यागम Impl->Entries.contains(Prefix);
}

} // namespace llvm
