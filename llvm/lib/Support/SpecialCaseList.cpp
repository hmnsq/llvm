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
  Error घाला(StringRef Pattern, अचिन्हित ओळ_संख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) स्थिरांक;
  StringRef findRule(अचिन्हित ओळ_No) स्थिरांक;

खाजगी:
  संरचना Reg {
    Reg(StringRef Name, अचिन्हित ओळ_No, Regex &&Rg)
        : Name(Name), ओळ_No(ओळ_No), Rg(std::move(Rg)) {}
    StringRef Name;
    अचिन्हित ओळ_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

वर्ग GlobMatcher {
सार्वजनिक:
  Error घाला(StringRef Pattern, अचिन्हित ओळ_संख्या, तार्किक SlashAgnostic);
  अचिन्हित match(StringRef Query) स्थिरांक;
  StringRef findRule(अचिन्हित ओळ_No) स्थिरांक;

खाजगी:
  संरचना Glob {
    Glob(StringRef Name, अचिन्हित ओळ_No, GlobPattern &&Pattern)
        : Name(Name), ओळ_No(ओळ_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    अचिन्हित ओळ_No;
    GlobPattern Pattern;
  };

  शून्य LazyInit() स्थिरांक;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
                    Radix_वृक्ष<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<पूर्णांक, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_वृक्ष<iterator_range<StringRef::const_iterator>,
                    SmallVector<पूर्णांक, 1>>
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

  Error घाला(StringRef Pattern, अचिन्हित ओळ_संख्या);
  अचिन्हित match(StringRef Query) स्थिरांक;

  तार्किक matchAny(StringRef Query) स्थिरांक { परत match(Query); }

खाजगी:
  अचिन्हित matchInternal(StringRef Query) स्थिरांक;
  StringRef findRule(अचिन्हित ओळ_No) स्थिरांक;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::घाला(StringRef Pattern, अचिन्हित ओळ_संख्या,
                           तार्किक SlashAgnostic) {
  जर (Pattern.empty())
    परत createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  स्वतः Regexp = Pattern.str();
  साठी (माप_प्रकार pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += म_माप(".*")) {
    Regexp.replace(pos, म_माप("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  जर (!CheckRE.isValid(REError))
    परत createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, ओळ_संख्या, std::move(CheckRE));
  परत Error::success();
}

अचिन्हित RegexMatcher::match(StringRef Query) स्थिरांक {
  साठी (स्थिरांक स्वतः &R : reverse(RegExes))
    जर (R.Rg.match(Query))
      परत R.ओळ_No;
  परत 0;
}

StringRef RegexMatcher::findRule(अचिन्हित ओळ_No) स्थिरांक {
  साठी (स्थिरांक स्वतः &R : RegExes)
    जर (R.ओळ_No == ओळ_No)
      परत R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  परत {};
}

Error GlobMatcher::घाला(StringRef Pattern, अचिन्हित ओळ_संख्या,
                          तार्किक SlashAgnostic) {
  जर (Pattern.empty())
    परत createStringError(errc::invalid_argument, "Supplied glob was blank");

  स्वतः Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  जर (स्वतः Err = Res.takeError())
    परत Err;
  Globs.emplace_back(Pattern, ओळ_संख्या, std::move(Res.get()));
  परत Error::success();
}

शून्य GlobMatcher::LazyInit() स्थिरांक {
  जर (LLVM_LIKELY(Initialized))
    परत;
  Initialized = सत्य;
  साठी (स्थिरांक स्वतः &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    जर (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      जर (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        स्वतः &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        सुरूठेवा;
      }
    }

    स्वतः &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    स्वतः &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

अचिन्हित GlobMatcher::match(StringRef Query) स्थिरांक {
  LazyInit();

  पूर्णांक Best = -1;
  जर (!PrefixSuffixToGlob.empty()) {
    साठी (स्थिरांक स्वतः &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      साठी (स्थिरांक स्वतः &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        साठी (पूर्णांक Idx : reverse(V)) {
          जर (Best > Idx)
            थांबा;
          स्थिरांक GlobMatcher::Glob &G = Globs[Idx];
          जर (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            थांबा;
          }
        }
      }
    }
  }

  जर (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    साठी (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      साठी (स्थिरांक स्वतः &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        साठी (पूर्णांक Idx : reverse(V)) {
          जर (Best > Idx)
            थांबा;
          स्थिरांक GlobMatcher::Glob &G = Globs[Idx];
          जर (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            थांबा;
          }
        }
      }
    }
  }
  परत Best < 0 ? 0 : Globs[Best].ओळ_No;
}

StringRef GlobMatcher::findRule(अचिन्हित ओळ_No) स्थिरांक {
  साठी (स्थिरांक स्वतः &G : Globs)
    जर (G.ओळ_No == ओळ_No)
      परत G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  परत {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  जर (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  अन्यथा
    M.emplace<RegexMatcher>();
}

Error Matcher::घाला(StringRef Pattern, अचिन्हित ओळ_संख्या) {
  परत std::visit(
      [&](स्वतः &V) {
        परत V.घाला(Pattern, ओळ_संख्या, Options.SlashAgnostic);
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
अचिन्हित Matcher::match(StringRef Query) स्थिरांक {
  जर (!Options.RemoveDotSlash)
    परत matchInternal(Query);

  जर (!Options.WarnDotSlashMatch)
    परत matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  अचिन्हित FixedMatched = matchInternal(FixedQuery);
  जर (FixedQuery == Query)
    परत FixedMatched;

  अचिन्हित OriginalMatch = matchInternal(Query);
  जर (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  परत std::max(OriginalMatch, FixedMatched);
}

अचिन्हित Matcher::matchInternal(StringRef Query) स्थिरांक {
  परत std::visit([&](स्वतः &V) -> अचिन्हित { परत V.match(Query); }, M);
}

StringRef Matcher::findRule(अचिन्हित ओळ_No) स्थिरांक {
  परत std::visit([&](स्वतः &V) -> StringRef { परत V.findRule(ओळ_No); },
                    M);
}
} // namespace

वर्ग SpecialCaseList::Section::SectionImpl {
सार्वजनिक:
  स्थिरांक Matcher *findMatcher(StringRef Prefix, StringRef Category) स्थिरांक;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(स्थिरांक std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नवीन SpecialCaseList());
  जर (SCL->createInternal(Paths, FS, Error))
    परत SCL;
  परत nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(स्थिरांक MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(नवीन SpecialCaseList());
  जर (SCL->createInternal(MB, Error))
    परत SCL;
  परत nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(स्थिरांक std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  जर (स्वतः SCL = create(Paths, FS, Error))
    परत SCL;
  report_fatal_error(Twine(Error));
}

तार्किक SpecialCaseList::createInternal(स्थिरांक std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  साठी (माप_प्रकार i = 0; i < Paths.size(); ++i) {
    स्थिरांक स्वतः &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    जर (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      परत असत्य;
    }
    std::string ParseError;
    जर (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      परत असत्य;
    }
  }
  परत सत्य;
}

तार्किक SpecialCaseList::createInternal(स्थिरांक MemoryBuffer *MB,
                                     std::string &Error) {
  जर (!parse(0, MB, Error))
    परत असत्य;
  परत सत्य;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, अचिन्हित FileNo,
                            अचिन्हित ओळ_No, तार्किक UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  स्वतः &Section = Sections.back();

  जर (स्वतः Err = Section.Impl->SectionMatcher.घाला(SectionStr, ओळ_No)) {
    परत createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(ओळ_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  परत &Section;
}

तार्किक SpecialCaseList::parse(अचिन्हित FileIdx, स्थिरांक MemoryBuffer *MB,
                            std::string &Error) {
  अचिन्हित दीर्घ दीर्घ Version = 2;

  StringRef Header = MB->getBuffer();
  जर (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  स्वतः MinVersion = [&](अचिन्हित V) { परत Version >= V; };

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
  जर (स्वतः Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    परत असत्य;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  साठी (ओळ_iterator ओळ_It(*MB, /*SkipBlanks=*/सत्य, /*CommentMarker=*/'#');
       !ओळ_It.is_at_eof(); ओळ_It++) {
    अचिन्हित ओळ_No = ओळ_It.ओळ_संख्या_2();
    StringRef ओळ = ओळ_It->trim();
    जर (ओळ.empty())
      सुरूठेवा;

    // Save section names
    जर (ओळ.starts_with("[")) {
      जर (!ओळ.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(ओळ_No) + ": " + ओळ)
                .str();
        परत असत्य;
      }

      स्वतः ErrOrSection =
          addSection(ओळ.drop_front().drop_back(), FileIdx, ओळ_No, UseGlobs);
      जर (स्वतः Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        परत असत्य;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      सुरूठेवा;
    }

    // Get our prefix and unparsed glob.
    स्वतः [Prefix, Postfix] = ओळ.split(":");
    जर (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(ओळ_No) + ": '" + ओळ + "'").str();
      परत असत्य;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    जर (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    स्वतः [Pattern, Category] = Postfix.split("=");
    स्वतः [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    जर (स्वतः Err = It->second.घाला(Pattern, ओळ_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(ओळ_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      परत असत्य;
    }
  }

  परत सत्य;
}

SpecialCaseList::~SpecialCaseList() = पूर्वनिर्धारित;

तार्किक SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) स्थिरांक {
  स्वतः [FileIdx, ओळ_No] = inSectionBlame(Section, Prefix, Query, Category);
  परत ओळ_No;
}

std::pair<अचिन्हित, अचिन्हित>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) स्थिरांक {
  साठी (स्थिरांक स्वतः &S : reverse(Sections)) {
    जर (S.Impl->SectionMatcher.matchAny(Section)) {
      अचिन्हित Blame = S.getLastMatch(Prefix, Query, Category);
      जर (Blame)
        परत {S.FileIdx, Blame};
    }
  }
  परत NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, अचिन्हित FileIdx,
                                  तार्किक UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/असत्य})) {}

SpecialCaseList::Section::Section(Section &&) = पूर्वनिर्धारित;

SpecialCaseList::Section::~Section() = पूर्वनिर्धारित;

तार्किक SpecialCaseList::Section::matchName(StringRef Name) स्थिरांक {
  परत Impl->SectionMatcher.matchAny(Name);
}

स्थिरांक Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) स्थिरांक {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  जर (I == Entries.end())
    परत nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  जर (II == I->second.end())
    परत nullptr;

  परत &II->second;
}

अचिन्हित SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) स्थिरांक {
  जर (स्थिरांक Matcher *M = Impl->findMatcher(Prefix, Category))
    परत M->match(Query);
  परत 0;
}

तार्किक SpecialCaseList::Section::hasPrefix(StringRef Prefix) स्थिरांक {
  परत Impl->Entries.contains(Prefix);
}

} // namespace llvm
