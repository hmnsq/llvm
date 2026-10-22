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

#அந்தர்பாவ "llvm/Support/SpecialCaseList.h"
#அந்தர்பாவ "llvm/ADT/RadixTree.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringMap.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/iterator_range.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/GlobPattern.h"
#அந்தர்பாவ "llvm/Support/LineIterator.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/Regex.h"
#அந்தர்பாவ "llvm/Support/VirtualFileSystem.h"
#அந்தர்பாவ "llvm/Support/WithColor.h"
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <mutex>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string>
#அந்தர்பாவ <system_error>
#அந்தர்பாவ <utility>
#அந்தர்பாவ <variant>
#அந்தர்பாவ <vector>

நாமாகாஶ llvm {

நாமாகாஶ {

// Lagacy v1 matcher.
வகுப்பு RegexMatcher {
பொது:
  Error செருகு(StringRef Pattern, unsigned வரி_எண், தருக்க SlashAgnostic);
  unsigned match(StringRef Query) மாறிலி;
  StringRef findRule(unsigned வரி_No) மாறிலி;

தனிப்பட்ட:
  கட்டமைப்பு Reg {
    Reg(StringRef Name, unsigned வரி_No, Regex &&Rg)
        : Name(Name), வரி_No(வரி_No), Rg(std::move(Rg)) {}
    StringRef Name;
    unsigned வரி_No;
    Regex Rg;
  };

  std::vector<Reg> RegExes;
};

வகுப்பு GlobMatcher {
பொது:
  Error செருகு(StringRef Pattern, unsigned வரி_எண், தருக்க SlashAgnostic);
  unsigned match(StringRef Query) மாறிலி;
  StringRef findRule(unsigned வரி_No) மாறிலி;

தனிப்பட்ட:
  கட்டமைப்பு Glob {
    Glob(StringRef Name, unsigned வரி_No, GlobPattern &&Pattern)
        : Name(Name), வரி_No(வரி_No), Pattern(std::move(Pattern)) {}
    StringRef Name;
    unsigned வரி_No;
    GlobPattern Pattern;
  };

  வெறுமை LazyInit() மாறிலி;

  std::vector<GlobMatcher::Glob> Globs;

  mutable Radix_மரம்<iterator_range<StringRef::const_iterator>,
                    Radix_மரம்<iterator_range<StringRef::const_reverse_iterator>,
                              SmallVector<முழு, 1>>>
      PrefixSuffixToGlob;

  mutable Radix_மரம்<iterator_range<StringRef::const_iterator>,
                    SmallVector<முழு, 1>>
      SubstrToGlob;

  mutable தருக்க Initialized = பொய்;
};

கட்டமைப்பு QueryOptions {
  தருக்க UseGlobs = உண்மை;
  தருக்க RemoveDotSlash = பொய்;
  தருக்க WarnDotSlashMatch = பொய்;
  தருக்க SlashAgnostic = பொய்;
};

/// Represents a set of patterns and their line numbers
வகுப்பு Matcher {
பொது:
  explicit Matcher(QueryOptions QOpts);

  Error செருகு(StringRef Pattern, unsigned வரி_எண்);
  unsigned match(StringRef Query) மாறிலி;

  தருக்க matchAny(StringRef Query) மாறிலி { திரும்பு match(Query); }

தனிப்பட்ட:
  unsigned matchInternal(StringRef Query) மாறிலி;
  StringRef findRule(unsigned வரி_No) மாறிலி;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  mutable std::once_flag Warned;
};

Error RegexMatcher::செருகு(StringRef Pattern, unsigned வரி_எண்,
                           தருக்க SlashAgnostic) {
  எனில் (Pattern.empty())
    திரும்பு createStringError(errc::invalid_argument,
                             "Supplied regex was blank");

  // Replace * with .*
  auto Regexp = Pattern.str();
  க்கு (size_t pos = 0; (pos = Regexp.find('*', pos)) != std::string::npos;
       pos += strlen(".*")) {
    Regexp.replace(pos, strlen("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  எனில் (!CheckRE.isValid(REError))
    திரும்பு createStringError(errc::invalid_argument, REError);

  RegExes.emplace_back(Pattern, வரி_எண், std::move(CheckRE));
  திரும்பு Error::success();
}

unsigned RegexMatcher::match(StringRef Query) மாறிலி {
  க்கு (மாறிலி auto &R : reverse(RegExes))
    எனில் (R.Rg.match(Query))
      திரும்பு R.வரி_No;
  திரும்பு 0;
}

StringRef RegexMatcher::findRule(unsigned வரி_No) மாறிலி {
  க்கு (மாறிலி auto &R : RegExes)
    எனில் (R.வரி_No == வரி_No)
      திரும்பு R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  திரும்பு {};
}

Error GlobMatcher::செருகு(StringRef Pattern, unsigned வரி_எண்,
                          தருக்க SlashAgnostic) {
  எனில் (Pattern.empty())
    திரும்பு createStringError(errc::invalid_argument, "Supplied glob was blank");

  auto Res =
      GlobPattern::create(Pattern, /*MaxSubPatterns=*/1024, SlashAgnostic);
  எனில் (auto Err = Res.takeError())
    திரும்பு Err;
  Globs.emplace_back(Pattern, வரி_எண், std::move(Res.get()));
  திரும்பு Error::success();
}

வெறுமை GlobMatcher::LazyInit() மாறிலி {
  எனில் (LLVM_LIKELY(Initialized))
    திரும்பு;
  Initialized = உண்மை;
  க்கு (மாறிலி auto &[Idx, G] : enumerate(Globs)) {
    StringRef Prefix = G.Pattern.prefix();
    StringRef Suffix = G.Pattern.suffix();

    எனில் (Suffix.empty() && Prefix.empty()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.Pattern.longest_substr();
      எனில் (!Substr.empty()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        auto &V = SubstrToGlob.emplace(Substr).first->second;
        V.emplace_back(Idx);
        தொடர்;
      }
    }

    auto &SToGlob = PrefixSuffixToGlob.emplace(Prefix).first->second;
    auto &V = SToGlob.emplace(reverse(Suffix)).first->second;
    V.emplace_back(Idx);
  }
}

unsigned GlobMatcher::match(StringRef Query) மாறிலி {
  LazyInit();

  முழு Best = -1;
  எனில் (!PrefixSuffixToGlob.empty()) {
    க்கு (மாறிலி auto &[_, SToGlob] : PrefixSuffixToGlob.find_prefixes(Query)) {
      க்கு (மாறிலி auto &[_, V] : SToGlob.find_prefixes(reverse(Query))) {
        க்கு (முழு Idx : reverse(V)) {
          எனில் (Best > Idx)
            நிறுத்து;
          மாறிலி GlobMatcher::Glob &G = Globs[Idx];
          எனில் (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            நிறுத்து;
          }
        }
      }
    }
  }

  எனில் (!SubstrToGlob.empty()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    க்கு (StringRef Q = Query; !Q.empty(); Q = Q.drop_front()) {
      க்கு (மாறிலி auto &[_, V] : SubstrToGlob.find_prefixes(Q)) {
        க்கு (முழு Idx : reverse(V)) {
          எனில் (Best > Idx)
            நிறுத்து;
          மாறிலி GlobMatcher::Glob &G = Globs[Idx];
          எனில் (G.Pattern.match(Query)) {
            Best = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            நிறுத்து;
          }
        }
      }
    }
  }
  திரும்பு Best < 0 ? 0 : Globs[Best].வரி_No;
}

StringRef GlobMatcher::findRule(unsigned வரி_No) மாறிலி {
  க்கு (மாறிலி auto &G : Globs)
    எனில் (G.வரி_No == வரி_No)
      திரும்பு G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  திரும்பு {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  எனில் (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  இல்லையெனில்
    M.emplace<RegexMatcher>();
}

Error Matcher::செருகு(StringRef Pattern, unsigned வரி_எண்) {
  திரும்பு std::visit(
      [&](auto &V) {
        திரும்பு V.செருகு(Pattern, வரி_எண், Options.SlashAgnostic);
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
unsigned Matcher::match(StringRef Query) மாறிலி {
  எனில் (!Options.RemoveDotSlash)
    திரும்பு matchInternal(Query);

  எனில் (!Options.WarnDotSlashMatch)
    திரும்பு matchInternal(llvm::sys::path::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::path::remove_leading_dotslash(Query);
  unsigned FixedMatched = matchInternal(FixedQuery);
  எனில் (FixedQuery == Query)
    திரும்பு FixedMatched;

  unsigned OriginalMatch = matchInternal(Query);
  எனில் (OriginalMatch > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << findRule(OriginalMatch) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  திரும்பு std::max(OriginalMatch, FixedMatched);
}

unsigned Matcher::matchInternal(StringRef Query) மாறிலி {
  திரும்பு std::visit([&](auto &V) -> unsigned { திரும்பு V.match(Query); }, M);
}

StringRef Matcher::findRule(unsigned வரி_No) மாறிலி {
  திரும்பு std::visit([&](auto &V) -> StringRef { திரும்பு V.findRule(வரி_No); },
                    M);
}
} // namespace

வகுப்பு SpecialCaseList::Section::SectionImpl {
பொது:
  மாறிலி Matcher *findMatcher(StringRef Prefix, StringRef Category) மாறிலி;

  using SectionEntries = StringMap<StringMap<Matcher>>;

  explicit SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(மாறிலி std::vector<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(புதிய SpecialCaseList());
  எனில் (SCL->createInternal(Paths, FS, Error))
    திரும்பு SCL;
  திரும்பு nullptr;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(மாறிலி MemoryBuffer *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(புதிய SpecialCaseList());
  எனில் (SCL->createInternal(MB, Error))
    திரும்பு SCL;
  திரும்பு nullptr;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(மாறிலி std::vector<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  எனில் (auto SCL = create(Paths, FS, Error))
    திரும்பு SCL;
  report_fatal_error(Twine(Error));
}

தருக்க SpecialCaseList::createInternal(மாறிலி std::vector<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  க்கு (size_t i = 0; i < Paths.size(); ++i) {
    மாறிலி auto &Path = Paths[i];
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        VFS.getBufferForFile(Path);
    எனில் (std::error_code EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + Path + "': " + EC.message()).str();
      திரும்பு பொய்;
    }
    std::string ParseError;
    எனில் (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + Path + "': " + ParseError).str();
      திரும்பு பொய்;
    }
  }
  திரும்பு உண்மை;
}

தருக்க SpecialCaseList::createInternal(மாறிலி MemoryBuffer *MB,
                                     std::string &Error) {
  எனில் (!parse(0, MB, Error))
    திரும்பு பொய்;
  திரும்பு உண்மை;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::addSection(StringRef SectionStr, unsigned FileNo,
                            unsigned வரி_No, தருக்க UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_back(SectionStr, FileNo, UseGlobs);
  auto &Section = Sections.back();

  எனில் (auto Err = Section.Impl->SectionMatcher.செருகு(SectionStr, வரி_No)) {
    திரும்பு createStringError(errc::invalid_argument,
                             "malformed section at line " + Twine(வரி_No) +
                                 ": '" + SectionStr +
                                 "': " + toString(std::move(Err)));
  }

  திரும்பு &Section;
}

தருக்க SpecialCaseList::parse(unsigned FileIdx, மாறிலி MemoryBuffer *MB,
                            std::string &Error) {
  unsigned long long Version = 2;

  StringRef Header = MB->getBuffer();
  எனில் (Header.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(Header, 10, Version);

  auto MinVersion = [&](unsigned V) { திரும்பு Version >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  தருக்க UseGlobs = MinVersion(2);
  தருக்க RemoveDotSlash = MinVersion(3);
  தருக்க WarnDotSlash = MinVersion(4) && !MinVersion(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  தருக்க SlashAgnostic = MinVersion(4) && llvm::sys::path::is_style_windows(
                                            llvm::sys::path::Style::native);

  auto ErrOrSection = addSection("*", FileIdx, 1, உண்மை);
  எனில் (auto Err = ErrOrSection.takeError()) {
    Error = toString(std::move(Err));
    திரும்பு பொய்;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef PathPrefixes[] = {"src", "!src", "mainfile", "source"};

  க்கு (வரி_iterator வரி_It(*MB, /*SkipBlanks=*/உண்மை, /*CommentMarker=*/'#');
       !வரி_It.is_at_eof(); வரி_It++) {
    unsigned வரி_No = வரி_It.வரி_எண்_2();
    StringRef வரி = வரி_It->trim();
    எனில் (வரி.empty())
      தொடர்;

    // Save section names
    எனில் (வரி.starts_with("[")) {
      எனில் (!வரி.ends_with("]")) {
        Error =
            ("malformed section header on line " + Twine(வரி_No) + ": " + வரி)
                .str();
        திரும்பு பொய்;
      }

      auto ErrOrSection =
          addSection(வரி.drop_front().drop_back(), FileIdx, வரி_No, UseGlobs);
      எனில் (auto Err = ErrOrSection.takeError()) {
        Error = toString(std::move(Err));
        திரும்பு பொய்;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      தொடர்;
    }

    // Get our prefix and unparsed glob.
    auto [Prefix, Postfix] = வரி.split(":");
    எனில் (Postfix.empty()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(வரி_No) + ": '" + வரி + "'").str();
      திரும்பு பொய்;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    எனில் (llvm::is_contained(PathPrefixes, Prefix)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.WarnDotSlashMatch = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    auto [Pattern, Category] = Postfix.split("=");
    auto [It, _] = CurrentImpl->Entries[Prefix].try_emplace(Category, QOpts);
    Pattern = Pattern.copy(StrAlloc);
    எனில் (auto Err = It->second.செருகு(Pattern, வரி_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(வரி_No) + ": '" + Pattern + "': " + toString(std::move(Err)))
              .str();
      திரும்பு பொய்;
    }
  }

  திரும்பு உண்மை;
}

SpecialCaseList::~SpecialCaseList() = இயல்பு;

தருக்க SpecialCaseList::inSection(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) மாறிலி {
  auto [FileIdx, வரி_No] = inSectionBlame(Section, Prefix, Query, Category);
  திரும்பு வரி_No;
}

std::pair<unsigned, unsigned>
SpecialCaseList::inSectionBlame(StringRef Section, StringRef Prefix,
                                StringRef Query, StringRef Category) மாறிலி {
  க்கு (மாறிலி auto &S : reverse(Sections)) {
    எனில் (S.Impl->SectionMatcher.matchAny(Section)) {
      unsigned Blame = S.getLastMatch(Prefix, Query, Category);
      எனில் (Blame)
        திரும்பு {S.FileIdx, Blame};
    }
  }
  திரும்பு NotFound;
}

SpecialCaseList::Section::Section(StringRef Str, unsigned FileIdx,
                                  தருக்க UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/பொய்})) {}

SpecialCaseList::Section::Section(Section &&) = இயல்பு;

SpecialCaseList::Section::~Section() = இயல்பு;

தருக்க SpecialCaseList::Section::matchName(StringRef Name) மாறிலி {
  திரும்பு Impl->SectionMatcher.matchAny(Name);
}

மாறிலி Matcher *
SpecialCaseList::Section::SectionImpl::findMatcher(StringRef Prefix,
                                                   StringRef Category) மாறிலி {
  SectionEntries::const_iterator I = Entries.find(Prefix);
  எனில் (I == Entries.end())
    திரும்பு nullptr;
  StringMap<Matcher>::const_iterator II = I->second.find(Category);
  எனில் (II == I->second.end())
    திரும்பு nullptr;

  திரும்பு &II->second;
}

unsigned SpecialCaseList::Section::getLastMatch(StringRef Prefix,
                                                StringRef Query,
                                                StringRef Category) மாறிலி {
  எனில் (மாறிலி Matcher *M = Impl->findMatcher(Prefix, Category))
    திரும்பு M->match(Query);
  திரும்பு 0;
}

தருக்க SpecialCaseList::Section::hasPrefix(StringRef Prefix) மாறிலி {
  திரும்பு Impl->Entries.contains(Prefix);
}

} // namespace llvm
