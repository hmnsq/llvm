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

#అన్తర్భావ "llvm/Support/SpecialCaseList.h"
#అన్తర్భావ "llvm/ADT/RadixTree.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/iterator_range.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/GlobPattern.h"
#అన్తర్భావ "llvm/Support/LineIterator.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Regex.h"
#అన్తర్భావ "llvm/Support/VirtualFileSystem.h"
#అన్తర్భావ "llvm/Support/WithColor.h"
#అన్తర్భావ <assert.h>
#అన్తర్భావ <memory>
#అన్తర్భావ <mutex>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string>
#అన్తర్భావ <system_error>
#అన్తర్భావ <utility>
#అన్తర్భావ <variant>
#అన్తర్భావ <vector>

నామాకాశ llvm {

నామాకాశ {

// Lagacy v1 matcher.
తరగతి RegexMatcher {
బహిరంగ:
  Error చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య, తార్కిక SlashAgnostic);
  గుర్తులేని సరిపోలిక(StringRef Query) స్థిరరాశి;
  StringRef కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి;

అంతరంగ:
  నిర్మాణం Reg {
    Reg(StringRef Name, గుర్తులేని పంక్తి_No, Regex &&Rg)
        : Name(Name), పంక్తి_No(పంక్తి_No), Rg(std::move(Rg)) {}
    StringRef Name;
    గుర్తులేని పంక్తి_No;
    Regex Rg;
  };

  std::సదిశ<Reg> RegExes;
};

తరగతి GlobMatcher {
బహిరంగ:
  Error చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య, తార్కిక SlashAgnostic);
  గుర్తులేని సరిపోలిక(StringRef Query) స్థిరరాశి;
  StringRef కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి;

అంతరంగ:
  నిర్మాణం Glob {
    Glob(StringRef Name, గుర్తులేని పంక్తి_No, Glob_నమూనా &&నమూనా)
        : Name(Name), పంక్తి_No(పంక్తి_No), నమూనా(std::move(నమూనా)) {}
    StringRef Name;
    గుర్తులేని పంక్తి_No;
    Glob_నమూనా నమూనా;
  };

  శూన్య LazyInit() స్థిరరాశి;

  std::సదిశ<GlobMatcher::Glob> Globs;

  మార్చదగిన Radix_వృక్షం<iterator_range<StringRef::const_iterator>,
                    Radix_వృక్షం<iterator_range<StringRef::const_తిరగేయి_iterator>,
                              Small_సదిశ<పూర్ణాంకం, 1>>>
      ఉపసర్గ_Suffix_గమ్యం_Glob;

  మార్చదగిన Radix_వృక్షం<iterator_range<StringRef::const_iterator>,
                    Small_సదిశ<పూర్ణాంకం, 1>>
      Substr_గమ్యం_Glob;

  మార్చదగిన తార్కిక Initialized = అబద్ధం;
};

నిర్మాణం QueryOptions {
  తార్కిక UseGlobs = నిజం;
  తార్కిక RemoveDotSlash = అబద్ధం;
  తార్కిక Warn_Dot_Slash_సరిపోలిక = అబద్ధం;
  తార్కిక SlashAgnostic = అబద్ధం;
};

/// Represents a set of patterns and their line numbers
తరగతి Matcher {
బహిరంగ:
  స్పష్టమైన Matcher(QueryOptions QOpts);

  Error చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య);
  గుర్తులేని సరిపోలిక(StringRef Query) స్థిరరాశి;

  తార్కిక సరిపోలిక_Any(StringRef Query) స్థిరరాశి { తిరిగి సరిపోలిక(Query); }

అంతరంగ:
  గుర్తులేని సరిపోలిక_అంతర్గత(StringRef Query) స్థిరరాశి;
  StringRef కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి;

  std::variant<RegexMatcher, GlobMatcher> M;
  QueryOptions Options;
  మార్చదగిన std::once_flag Warned;
};

Error RegexMatcher::చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య,
                           తార్కిక SlashAgnostic) {
  ఒకవేళ (నమూనా.ఖాళీలు())
    తిరిగి createStringError(errc::invalid_వాదం,
                             "Supplied regex was blank");

  // Replace * with .*
  స్వయంచాలక Regexp = నమూనా.str();
  కొరకు (size_t pos = 0; (pos = Regexp.కనుగొను('*', pos)) != std::string::npos;
       pos += అక్షరశ్రేణి_పొడవు(".*")) {
    Regexp.replace(pos, అక్షరశ్రేణి_పొడవు("*"), ".*");
  }

  Regexp = (Twine("^(") + StringRef(Regexp) + ")$").str();

  // Check that the regexp is valid.
  Regex CheckRE(Regexp);
  std::string REError;
  ఒకవేళ (!CheckRE.isValid(REError))
    తిరిగి createStringError(errc::invalid_వాదం, REError);

  RegExes.emplace_వెనుక(నమూనా, పంక్తి_సంఖ్య, std::move(CheckRE));
  తిరిగి Error::success();
}

గుర్తులేని RegexMatcher::సరిపోలిక(StringRef Query) స్థిరరాశి {
  కొరకు (స్థిరరాశి స్వయంచాలక &R : తిరగేయి(RegExes))
    ఒకవేళ (R.Rg.సరిపోలిక(Query))
      తిరిగి R.పంక్తి_No;
  తిరిగి 0;
}

StringRef RegexMatcher::కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి {
  కొరకు (స్థిరరాశి స్వయంచాలక &R : RegExes)
    ఒకవేళ (R.పంక్తి_No == పంక్తి_No)
      తిరిగి R.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  తిరిగి {};
}

Error GlobMatcher::చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య,
                          తార్కిక SlashAgnostic) {
  ఒకవేళ (నమూనా.ఖాళీలు())
    తిరిగి createStringError(errc::invalid_వాదం, "Supplied glob was blank");

  స్వయంచాలక Res =
      Glob_నమూనా::create(నమూనా, /*MaxSubPatterns=*/1024, SlashAgnostic);
  ఒకవేళ (స్వయంచాలక Err = Res.takeError())
    తిరిగి Err;
  Globs.emplace_వెనుక(నమూనా, పంక్తి_సంఖ్య, std::move(Res.get()));
  తిరిగి Error::success();
}

శూన్య GlobMatcher::LazyInit() స్థిరరాశి {
  ఒకవేళ (LLVM_LIKELY(Initialized))
    తిరిగి;
  Initialized = నిజం;
  కొరకు (స్థిరరాశి స్వయంచాలక &[Idx, G] : enumerate(Globs)) {
    StringRef ఉపసర్గ = G.నమూనా.ఉపసర్గ_2();
    StringRef Suffix = G.నమూనా.suffix();

    ఒకవేళ (Suffix.ఖాళీలు() && ఉపసర్గ.ఖాళీలు()) {
      // If both prefix and suffix are empty put into special tree to search by
      // substring in a middle.
      StringRef Substr = G.నమూనా.longest_substr();
      ఒకవేళ (!Substr.ఖాళీలు()) {
        // But only if substring is not empty. Searching this tree is more
        // expensive.
        స్వయంచాలక &V = Substr_గమ్యం_Glob.emplace(Substr).మొదటి_2->రెండవ_2;
        V.emplace_వెనుక(Idx);
        కొనసాగు;
      }
    }

    స్వయంచాలక &SToGlob = ఉపసర్గ_Suffix_గమ్యం_Glob.emplace(ఉపసర్గ).మొదటి_2->రెండవ_2;
    స్వయంచాలక &V = SToGlob.emplace(తిరగేయి(Suffix)).మొదటి_2->రెండవ_2;
    V.emplace_వెనుక(Idx);
  }
}

గుర్తులేని GlobMatcher::సరిపోలిక(StringRef Query) స్థిరరాశి {
  LazyInit();

  పూర్ణాంకం ఉత్తమ = -1;
  ఒకవేళ (!ఉపసర్గ_Suffix_గమ్యం_Glob.ఖాళీలు()) {
    కొరకు (స్థిరరాశి స్వయంచాలక &[_, SToGlob] : ఉపసర్గ_Suffix_గమ్యం_Glob.కనుగొను_prefixes(Query)) {
      కొరకు (స్థిరరాశి స్వయంచాలక &[_, V] : SToGlob.కనుగొను_prefixes(తిరగేయి(Query))) {
        కొరకు (పూర్ణాంకం Idx : తిరగేయి(V)) {
          ఒకవేళ (ఉత్తమ > Idx)
            ఆపు;
          స్థిరరాశి GlobMatcher::Glob &G = Globs[Idx];
          ఒకవేళ (G.నమూనా.సరిపోలిక(Query)) {
            ఉత్తమ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ఆపు;
          }
        }
      }
    }
  }

  ఒకవేళ (!Substr_గమ్యం_Glob.ఖాళీలు()) {
    // As we don't know when substring exactly starts, we will try all
    // possibilities. In most cases search will fail on first characters.
    కొరకు (StringRef Q = Query; !Q.ఖాళీలు(); Q = Q.drop_front()) {
      కొరకు (స్థిరరాశి స్వయంచాలక &[_, V] : Substr_గమ్యం_Glob.కనుగొను_prefixes(Q)) {
        కొరకు (పూర్ణాంకం Idx : తిరగేయి(V)) {
          ఒకవేళ (ఉత్తమ > Idx)
            ఆపు;
          స్థిరరాశి GlobMatcher::Glob &G = Globs[Idx];
          ఒకవేళ (G.నమూనా.సరిపోలిక(Query)) {
            ఉత్తమ = Idx;
            // As soon as we find a match in the vector, we can break for this
            // vector, since the globs are already sorted by priority within the
            // prefix group. However, we continue searching other prefix groups
            // in the map, as they may contain a better match overall.
            ఆపు;
          }
        }
      }
    }
  }
  తిరిగి ఉత్తమ < 0 ? 0 : Globs[ఉత్తమ].పంక్తి_No;
}

StringRef GlobMatcher::కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి {
  కొరకు (స్థిరరాశి స్వయంచాలక &G : Globs)
    ఒకవేళ (G.పంక్తి_No == పంక్తి_No)
      తిరిగి G.Name;
  llvm_unreachable("`findRule` should be called only with correct `LineNo`");
  తిరిగి {};
}

Matcher::Matcher(QueryOptions QOpts) : Options(QOpts) {
  ఒకవేళ (Options.UseGlobs)
    M.emplace<GlobMatcher>();
  లేకపోతే
    M.emplace<RegexMatcher>();
}

Error Matcher::చొప్పించు(StringRef నమూనా, గుర్తులేని పంక్తి_సంఖ్య) {
  తిరిగి std::సందర్శించు(
      [&](స్వయంచాలక &V) {
        తిరిగి V.చొప్పించు(నమూనా, పంక్తి_సంఖ్య, Options.SlashAgnostic);
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
గుర్తులేని Matcher::సరిపోలిక(StringRef Query) స్థిరరాశి {
  ఒకవేళ (!Options.RemoveDotSlash)
    తిరిగి సరిపోలిక_అంతర్గత(Query);

  ఒకవేళ (!Options.Warn_Dot_Slash_సరిపోలిక)
    తిరిగి సరిపోలిక_అంతర్గత(llvm::sys::మార్గం_2::remove_leading_dotslash(Query));

  StringRef FixedQuery = llvm::sys::మార్గం_2::remove_leading_dotslash(Query);
  గుర్తులేని FixedMatched = సరిపోలిక_అంతర్గత(FixedQuery);
  ఒకవేళ (FixedQuery == Query)
    తిరిగి FixedMatched;

  గుర్తులేని Original_సరిపోలిక = సరిపోలిక_అంతర్గత(Query);
  ఒకవేళ (Original_సరిపోలిక > FixedMatched) {
    std::call_once(Warned, [&]() {
      WithColor::warning() << "Deprecated behaviour: pattern '"
                           << కనుగొను_నియమం(Original_సరిపోలిక) << "' matches '" << Query
                           << "', update it to match '" << FixedQuery
                           << "' instead (further warnings suppressed).\n";
    });
  }
  తిరిగి std::max(Original_సరిపోలిక, FixedMatched);
}

గుర్తులేని Matcher::సరిపోలిక_అంతర్గత(StringRef Query) స్థిరరాశి {
  తిరిగి std::సందర్శించు([&](స్వయంచాలక &V) -> గుర్తులేని { తిరిగి V.సరిపోలిక(Query); }, M);
}

StringRef Matcher::కనుగొను_నియమం(గుర్తులేని పంక్తి_No) స్థిరరాశి {
  తిరిగి std::సందర్శించు([&](స్వయంచాలక &V) -> StringRef { తిరిగి V.కనుగొను_నియమం(పంక్తి_No); },
                    M);
}
} // namespace

తరగతి SpecialCaseList::Section::SectionImpl {
బహిరంగ:
  స్థిరరాశి Matcher *కనుగొను_Matcher(StringRef ఉపసర్గ, StringRef Category) స్థిరరాశి;

  ఉపయోగించి SectionEntries = StringMap<StringMap<Matcher>>;

  స్పష్టమైన SectionImpl(QueryOptions QOpts) : SectionMatcher(QOpts) {}

  Matcher SectionMatcher;
  SectionEntries Entries;
};

// TODO: Refactor this to return Expected<...>
std::unique_ptr<SpecialCaseList>
SpecialCaseList::create(స్థిరరాశి std::సదిశ<std::string> &Paths,
                        llvm::vfs::FileSystem &FS, std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(కొత్త SpecialCaseList());
  ఒకవేళ (SCL->create_అంతర్గత(Paths, FS, Error))
    తిరిగి SCL;
  తిరిగి శూన్యసూచిక;
}

std::unique_ptr<SpecialCaseList> SpecialCaseList::create(స్థిరరాశి జ్ఞాపకం_బఫరు *MB,
                                                         std::string &Error) {
  std::unique_ptr<SpecialCaseList> SCL(కొత్త SpecialCaseList());
  ఒకవేళ (SCL->create_అంతర్గత(MB, Error))
    తిరిగి SCL;
  తిరిగి శూన్యసూచిక;
}

std::unique_ptr<SpecialCaseList>
SpecialCaseList::createOrDie(స్థిరరాశి std::సదిశ<std::string> &Paths,
                             llvm::vfs::FileSystem &FS) {
  std::string Error;
  ఒకవేళ (స్వయంచాలక SCL = create(Paths, FS, Error))
    తిరిగి SCL;
  report_fatal_error(Twine(Error));
}

తార్కిక SpecialCaseList::create_అంతర్గత(స్థిరరాశి std::సదిశ<std::string> &Paths,
                                     vfs::FileSystem &VFS, std::string &Error) {
  కొరకు (size_t i = 0; i < Paths.కొలత_2(); ++i) {
    స్థిరరాశి స్వయంచాలక &మార్గం = Paths[i];
    ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> FileOrErr =
        VFS.get_బఫరు_For_File(మార్గం);
    ఒకవేళ (std::error_సంకేతం EC = FileOrErr.getError()) {
      Error = (Twine("can't open file '") + మార్గం + "': " + EC.సందేశం_2()).str();
      తిరిగి అబద్ధం;
    }
    std::string ParseError;
    ఒకవేళ (!parse(i, FileOrErr.get().get(), ParseError)) {
      Error = (Twine("error parsing file '") + మార్గం + "': " + ParseError).str();
      తిరిగి అబద్ధం;
    }
  }
  తిరిగి నిజం;
}

తార్కిక SpecialCaseList::create_అంతర్గత(స్థిరరాశి జ్ఞాపకం_బఫరు *MB,
                                     std::string &Error) {
  ఒకవేళ (!parse(0, MB, Error))
    తిరిగి అబద్ధం;
  తిరిగి నిజం;
}

Expected<SpecialCaseList::Section *>
SpecialCaseList::కూడు_Section(StringRef SectionStr, గుర్తులేని FileNo,
                            గుర్తులేని పంక్తి_No, తార్కిక UseGlobs) {
  SectionStr = SectionStr.copy(StrAlloc);
  Sections.emplace_వెనుక(SectionStr, FileNo, UseGlobs);
  స్వయంచాలక &Section = Sections.వెనుక();

  ఒకవేళ (స్వయంచాలక Err = Section.Impl->SectionMatcher.చొప్పించు(SectionStr, పంక్తి_No)) {
    తిరిగి createStringError(errc::invalid_వాదం,
                             "malformed section at line " + Twine(పంక్తి_No) +
                                 ": '" + SectionStr +
                                 "': " + గమ్యం_String(std::move(Err)));
  }

  తిరిగి &Section;
}

తార్కిక SpecialCaseList::parse(గుర్తులేని FileIdx, స్థిరరాశి జ్ఞాపకం_బఫరు *MB,
                            std::string &Error) {
  గుర్తులేని దీర్ఘ దీర్ఘ సంస్కరణ = 2;

  StringRef శీర్షిక = MB->get_బఫరు();
  ఒకవేళ (శీర్షిక.consume_front("#!special-case-list-v"))
    consumeUnsignedInteger(శీర్షిక, 10, సంస్కరణ);

  స్వయంచాలక Min_సంస్కరణ = [&](గుర్తులేని V) { తిరిగి సంస్కరణ >= V; };

  // In https://reviews.llvm.org/D154014 we added glob support and planned
  // to remove regex support in patterns. We temporarily support the
  // original behavior using regexes if "#!special-case-list-v1" is the
  // first line of the file. For more details, see
  // https://discourse.llvm.org/t/use-glob-instead-of-regex-for-specialcaselists/71666
  తార్కిక UseGlobs = Min_సంస్కరణ(2);
  తార్కిక RemoveDotSlash = Min_సంస్కరణ(3);
  తార్కిక WarnDotSlash = Min_సంస్కరణ(4) && !Min_సంస్కరణ(5);
  // TODO: Improve efficiency on Windows.
  // `SlashAgnostic` makes `GlobMatcher` lookup inefficient by reducing the part
  // of the pattern handled by the RadixTree. This was already the case even
  // before `SlashAgnostic` because `GlobMatcher` pessimizes on escape sequences
  // needed to represent Windows backslashes. A possible, but not unique,
  // solution is to assume (or convert Windows query) backslashes, and
  // preprocess the Glob pattern to use different escape sequences.
  తార్కిక SlashAgnostic = Min_సంస్కరణ(4) && llvm::sys::మార్గం_2::is_style_windows(
                                            llvm::sys::మార్గం_2::Style::native);

  స్వయంచాలక ErrOrSection = కూడు_Section("*", FileIdx, 1, నిజం);
  ఒకవేళ (స్వయంచాలక Err = ErrOrSection.takeError()) {
    Error = గమ్యం_String(std::move(Err));
    తిరిగి అబద్ధం;
  }
  Section::SectionImpl *CurrentImpl = ErrOrSection.get()->Impl.get();

  // This is the current list of prefixes for all existing users matching file
  // path. We may need parametrization in constructor in future.
  constexpr StringRef మార్గం_Prefixes[] = {"src", "!src", "mainfile", "source"};

  కొరకు (పంక్తి_iterator పంక్తి_It(*MB, /*SkipBlanks=*/నిజం, /*CommentMarker=*/'#');
       !పంక్తి_It.is_at_eof(); పంక్తి_It++) {
    గుర్తులేని పంక్తి_No = పంక్తి_It.పంక్తి_సంఖ్య_2();
    StringRef పంక్తి = పంక్తి_It->trim();
    ఒకవేళ (పంక్తి.ఖాళీలు())
      కొనసాగు;

    // Save section names
    ఒకవేళ (పంక్తి.starts_with("[")) {
      ఒకవేళ (!పంక్తి.చివర్లు_with("]")) {
        Error =
            ("malformed section header on line " + Twine(పంక్తి_No) + ": " + పంక్తి)
                .str();
        తిరిగి అబద్ధం;
      }

      స్వయంచాలక ErrOrSection =
          కూడు_Section(పంక్తి.drop_front().drop_వెనుక(), FileIdx, పంక్తి_No, UseGlobs);
      ఒకవేళ (స్వయంచాలక Err = ErrOrSection.takeError()) {
        Error = గమ్యం_String(std::move(Err));
        తిరిగి అబద్ధం;
      }
      CurrentImpl = ErrOrSection.get()->Impl.get();
      కొనసాగు;
    }

    // Get our prefix and unparsed glob.
    స్వయంచాలక [ఉపసర్గ, Postfix] = పంక్తి.విభజించు(":");
    ఒకవేళ (Postfix.ఖాళీలు()) {
      // Missing ':' in the line.
      Error = ("malformed line " + Twine(పంక్తి_No) + ": '" + పంక్తి + "'").str();
      తిరిగి అబద్ధం;
    }

    QueryOptions QOpts;
    QOpts.UseGlobs = UseGlobs;
    ఒకవేళ (llvm::is_contained(మార్గం_Prefixes, ఉపసర్గ)) {
      QOpts.RemoveDotSlash = RemoveDotSlash;
      QOpts.Warn_Dot_Slash_సరిపోలిక = WarnDotSlash;
      QOpts.SlashAgnostic = SlashAgnostic;
    }

    స్వయంచాలక [నమూనా, Category] = Postfix.విభజించు("=");
    స్వయంచాలక [It, _] = CurrentImpl->Entries[ఉపసర్గ].try_emplace(Category, QOpts);
    నమూనా = నమూనా.copy(StrAlloc);
    ఒకవేళ (స్వయంచాలక Err = It->రెండవ_2.చొప్పించు(నమూనా, పంక్తి_No)) {
      Error =
          (Twine("malformed ") + (UseGlobs ? "glob" : "regex") + " in line " +
           Twine(పంక్తి_No) + ": '" + నమూనా + "': " + గమ్యం_String(std::move(Err)))
              .str();
      తిరిగి అబద్ధం;
    }
  }

  తిరిగి నిజం;
}

SpecialCaseList::~SpecialCaseList() = అప్రమేయ;

తార్కిక SpecialCaseList::లోపల_Section(StringRef Section, StringRef ఉపసర్గ,
                                StringRef Query, StringRef Category) స్థిరరాశి {
  స్వయంచాలక [FileIdx, పంక్తి_No] = లోపల_Section_Blame(Section, ఉపసర్గ, Query, Category);
  తిరిగి పంక్తి_No;
}

std::pair<గుర్తులేని, గుర్తులేని>
SpecialCaseList::లోపల_Section_Blame(StringRef Section, StringRef ఉపసర్గ,
                                StringRef Query, StringRef Category) స్థిరరాశి {
  కొరకు (స్థిరరాశి స్వయంచాలక &S : తిరగేయి(Sections)) {
    ఒకవేళ (S.Impl->SectionMatcher.సరిపోలిక_Any(Section)) {
      గుర్తులేని Blame = S.get_చివరి_సరిపోలిక(ఉపసర్గ, Query, Category);
      ఒకవేళ (Blame)
        తిరిగి {S.FileIdx, Blame};
    }
  }
  తిరిగి Not_దొరికింది;
}

SpecialCaseList::Section::Section(StringRef Str, గుర్తులేని FileIdx,
                                  తార్కిక UseGlobs)
    : Name(Str), FileIdx(FileIdx),
      Impl(std::make_unique<SectionImpl>(
          QueryOptions{UseGlobs, /*RemoveDotSlash=*/అబద్ధం})) {}

SpecialCaseList::Section::Section(Section &&) = అప్రమేయ;

SpecialCaseList::Section::~Section() = అప్రమేయ;

తార్కిక SpecialCaseList::Section::సరిపోలిక_Name(StringRef Name) స్థిరరాశి {
  తిరిగి Impl->SectionMatcher.సరిపోలిక_Any(Name);
}

స్థిరరాశి Matcher *
SpecialCaseList::Section::SectionImpl::కనుగొను_Matcher(StringRef ఉపసర్గ,
                                                   StringRef Category) స్థిరరాశి {
  SectionEntries::const_iterator I = Entries.కనుగొను(ఉపసర్గ);
  ఒకవేళ (I == Entries.చివర())
    తిరిగి శూన్యసూచిక;
  StringMap<Matcher>::const_iterator II = I->రెండవ_2.కనుగొను(Category);
  ఒకవేళ (II == I->రెండవ_2.చివర())
    తిరిగి శూన్యసూచిక;

  తిరిగి &II->రెండవ_2;
}

గుర్తులేని SpecialCaseList::Section::get_చివరి_సరిపోలిక(StringRef ఉపసర్గ,
                                                StringRef Query,
                                                StringRef Category) స్థిరరాశి {
  ఒకవేళ (స్థిరరాశి Matcher *M = Impl->కనుగొను_Matcher(ఉపసర్గ, Category))
    తిరిగి M->సరిపోలిక(Query);
  తిరిగి 0;
}

తార్కిక SpecialCaseList::Section::has_ఉపసర్గ(StringRef ఉపసర్గ) స్థిరరాశి {
  తిరిగి Impl->Entries.కలిగియుంది(ఉపసర్గ);
}

} // namespace llvm
