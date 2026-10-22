//===-- CommandLine.cpp - Command line parser implementation --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements a command line argument processor that is useful when
// creating a tool.  It provides a simple, minimalistic interface that is easily
// extensible and supports nonlocal (library) command line options.
//
// Note that rather than trying to figure out what this code does, you could try
// reading the library documentation located in docs/CommandLine.html
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CommandLine.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm-c/Support.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/STLFunctionalExtras.h"
#अन्तर्भाव "llvm/ADT/SmallPtrSet.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/LibraryOptionsContext.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/StringSaver.h"
#अन्तर्भाव "llvm/Support/TypeSize.h"
#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <optional>
#अन्तर्भाव <string>
using नामाकाश llvm;
using नामाकाश cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
नामाकाश llvm {
नामाकाश cl {
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<तार्किक>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<पूर्णाङ्क>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<long>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<द्विगुण>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<भिन्न>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<वर्ण>;
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<Element_गणनम्>;

#जइ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template वर्ग LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template वर्ग LLVM_EXPORT_TEMPLATE opt<तार्किक>;
template वर्ग LLVM_EXPORT_TEMPLATE opt<वर्ण>;
template वर्ग LLVM_EXPORT_TEMPLATE opt<पूर्णाङ्क>;
template वर्ग LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
शून्य Generic_Option_मानम्::anchor() {}
शून्य Option_मानम्<boolOrDefault>::anchor() {}
शून्य Option_मानम्<std::string>::anchor() {}
शून्य Option::anchor() {}
शून्य basic_parser_impl::anchor() {}
शून्य parser<तार्किक>::anchor() {}
शून्य parser<boolOrDefault>::anchor() {}
शून्य parser<पूर्णाङ्क>::anchor() {}
शून्य parser<long>::anchor() {}
शून्य parser<long long>::anchor() {}
शून्य parser<unsigned>::anchor() {}
शून्य parser<unsigned long>::anchor() {}
शून्य parser<unsigned long long>::anchor() {}
शून्य parser<द्विगुण>::anchor() {}
शून्य parser<भिन्न>::anchor() {}
शून्य parser<std::string>::anchor() {}
शून्य parser<std::optional<std::string>>::anchor() {}
शून्य parser<वर्ण>::anchor() {}
शून्य parser<Element_गणनम्>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
शून्य opt_bool_anchor() { opt<तार्किक> anchor{""}; }
शून्य opt_char_anchor() { opt<वर्ण> anchor{""}; }
शून्य opt_int_anchor() { opt<पूर्णाङ्क> anchor{""}; }
शून्य opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ध्रुव स्थिर size_t DefaultPad = 2;

स्थिर StringRef ArgPrefix = "-";
स्थिर StringRef ArgPrefixLong = "--";
स्थिर StringRef ArgHelpPrefix = " - ";

स्थिर size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  जइ (Len == 1)
    प्रत्यागम Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  प्रत्यागम Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

स्थिर SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  प्रति (size_t I = 0; I < Pad; ++I) {
    Prefix.प्रक्षिप_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  प्रत्यागम Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

नामाकाश {

वर्ग PrintArg {
  StringRef ArgName;
  size_t Pad;
सार्वजनिक:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ध्रुव PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ध्रुव PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  प्रत्यागम OS;
}

वर्ग Command_पङ्क्तिः_Parser {
सार्वजनिक:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string ProgramName;
  StringRef ProgramOverview;

  // This collects additional help to be printed.
  std::vector<StringRef> MoreHelp;

  // This collects the different option categories that have been registered.
  SmallPtrSet<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  SmallPtrSet<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  SmallVector<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> LibraryIndex;
  size_t अङ्कः_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_पङ्क्तिः_Parser() { registerSubCommand(&SubCommand::get_शिखरम्_Level()); }

  शून्य ResetAllOptionOccurrences();

  तार्किक Parse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  शून्य forEachSubCommand(Option &Opt, function_ref<शून्य(SubCommand &)> Action) {
    जइ (Opt.Subs.empty()) {
      Action(SubCommand::get_शिखरम्_Level());
      प्रत्यागम;
    }
    जइ (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      प्रति (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      प्रत्यागम;
    }
    प्रति (auto *SC : Opt.Subs) {
      प्रतिज्ञा(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  शून्य addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    जइ (Opt.hasArgStr())
      प्रत्यागम;
    जइ (!SC->OptionsMap.निवेशय(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  शून्य addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  शून्य addOption(Option *O, SubCommand *SC) {
    तार्किक HadErrors = मिच्छ;
    जइ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      जइ (!SC->OptionsMap.निवेशय(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = सच्च;
      }
    }

    // Remember information about positional options.
    जइ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.प्रक्षिप_back(O);
    अण्णहा जइ (O->get_अङ्कः_Occurrences_Flag() == cl::ConsumeAfter) {
      जइ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = सच्च;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    जइ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  शून्य addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  शून्य removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    जइ (O->hasArgStr())
      OptionNames.प्रक्षिप_back(O->ArgStr);

    SubCommand &Sub = *SC;
    प्रति (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      जइ (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    जइ (O->getFormattingFlag() == cl::Positional)
      प्रति (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        जइ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          भङ्ग;
        }
      }
    अण्णहा जइ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  शून्य removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  तार्किक hasOptions(ध्रुव SubCommand &Sub) ध्रुव {
    प्रत्यागम (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  तार्किक hasOptions() ध्रुव {
    प्रति (ध्रुव auto *S : RegisteredSubCommands) {
      जइ (hasOptions(*S))
        प्रत्यागम सच्च;
    }
    प्रत्यागम !Libraries.empty();
  }

  तार्किक hasNamedSubCommands() ध्रुव {
    प्रति (ध्रुव auto *S : RegisteredSubCommands)
      जइ (!S->getName().empty())
        प्रत्यागम सच्च;
    प्रत्यागम मिच्छ;
  }

  SubCommand *getActiveSubCommand() { प्रत्यागम ActiveSubCommand; }

  शून्य updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    जइ (!Sub.OptionsMap.निवेशय(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  शून्य updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  शून्य print_Option_मानानि();

  शून्य indexLibraryOptions() {
    तार्किक HadErrors = मिच्छ;
    प्रति (; अङ्कः_Indexed_Libraries != Libraries.size(); ++अङ्कः_Indexed_Libraries) {
      LibraryOptions *L = Libraries[अङ्कः_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        जइ (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 प्रत्यागम SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          प्रत्यागम;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = सच्च;
      });
    }
    जइ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    प्रत्यागम LibraryIndex.lookup(Name);
  }

  शून्य registerCategory(OptionCategory *cat) {
    प्रतिज्ञा(गणनम्_if(RegisteredOptionCategories,
                    [cat](ध्रुव OptionCategory *Category) {
             प्रत्यागम cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.निवेशय(cat);
  }

  शून्य registerSubCommand(SubCommand *sub) {
    प्रतिज्ञा(गणनम्_if(RegisteredSubCommands,
                    [sub](ध्रुव SubCommand *Sub) {
                      प्रत्यागम (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.निवेशय(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    प्रतिज्ञा(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    प्रति (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      जइ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      अण्णहा
        addLiteralOption(*O, sub, E.first);
    }
  }

  शून्य unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    प्रत्यागम make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  शून्य reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    अङ्कः_Indexed_Libraries = 0;

    SubCommand::get_शिखरम्_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_शिखरम्_Level());
  }

गुप्त:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &मानम्);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
स्थिर Command_पङ्क्तिः_Parser &globalParser() {
  स्थिर ManagedStatic<Command_पङ्क्तिः_Parser> GlobalParser;
  प्रत्यागम *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
स्थिर Error
parseLibraryOption(LibraryOptions &L, शून्य *Opts, ArrayRef<ध्रुव वर्ण *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ध्रुव वर्ण *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ध्रुव वर्ण *Saved[2];
  unsigned अङ्कः_Args = std::min<size_t>(Args.size(), 2);
  प्रति (unsigned J = 0; J != अङ्कः_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  जइ (Error E = L.parse(Opts, ArrayRef(Saved, अङ्कः_Args), Consumed, Alloc))
    प्रत्यागम E;
  जइ (Record)
    Record->append(Saved, Saved + Consumed);
  प्रत्यागम Error::success();
}

template <typename T, T TrueVal, T FalseVal>
स्थिर तार्किक parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मानम्) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  जइ (!Arg.data() || Arg == "true" || Arg == "1") {
    मानम् = TrueVal;
    प्रत्यागम मिच्छ;
  }

  जइ (Arg == "false" || Arg == "0") {
    मानम् = FalseVal;
    प्रत्यागम मिच्छ;
  }
  प्रत्यागम O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

शून्य cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.प्रक्षिप_back(Help);
}

Option::Option(अङ्कः_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : अङ्कः_Occurrences(0), Occurrences(OccurrencesFlag), मानम्(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(मिच्छ), Position(0) {
  Categories.प्रक्षिप_back(&getGeneralCategory());
}

शून्य Option::addArgument() {
  globalParser().addOption(एतत्);
  FullyInitialized = सच्च;
}

शून्य Option::removeArgument() { globalParser().removeOption(एतत्); }

शून्य Option::setArgStr(StringRef S) {
  जइ (FullyInitialized)
    globalParser().updateArgStr(एतत्, S);
  प्रतिज्ञा(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

शून्य Option::addCategory(OptionCategory &C) {
  प्रतिज्ञा(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  जइ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अण्णहा जइ (!is_contained(Categories, &C))
    Categories.प्रक्षिप_back(&C);
}

शून्य Option::reset() {
  अङ्कः_Occurrences = 0;
  setDefault();
}

शून्य OptionCategory::registerCategory() {
  globalParser().registerCategory(एतत्);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_शिखरम्_Level() {
  स्थिर ManagedStatic<SubCommand> शिखरम्_Level_Sub_Command;
  प्रत्यागम *शिखरम्_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  स्थिर ManagedStatic<SubCommand> AllSubCommands;
  प्रत्यागम *AllSubCommands;
}

शून्य SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(एतत्);
}

शून्य SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(एतत्);
}

शून्य SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator तार्किक() ध्रुव {
  प्रत्यागम (globalParser().getActiveSubCommand() == एतत्);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_पङ्क्तिः_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &मानम्) {
  // Reject all dashes.
  जइ (Arg.empty())
    प्रत्यागम nullptr;
  प्रतिज्ञा(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  जइ (EqualPos == StringRef::npos) {
    // Look up the option.
    प्रत्यागम Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  जइ (I == Sub.OptionsMap.end())
    प्रत्यागम nullptr;

  auto *O = I->second;
  जइ (O->getFormattingFlag() == cl::AlwaysPrefix)
    प्रत्यागम nullptr;

  मानम् = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  प्रत्यागम I->second;
}

SubCommand *Command_पङ्क्तिः_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  जइ (Name.empty())
    प्रत्यागम &SubCommand::get_शिखरम्_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  प्रति (auto *S : RegisteredSubCommands) {
    प्रतिज्ञा(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    जइ (S->getName().empty())
      अनुवर्त;

    जइ (S->getName() == Name)
      प्रत्यागम S;

    जइ (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  जइ (NearestMatch)
    NearestString = NearestMatch->getName();

  प्रत्यागम &SubCommand::get_शिखरम्_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
स्थिर Option *LookupNearestOption(StringRef Arg,
                                   ध्रुव OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  जइ (Arg.empty())
    प्रत्यागम nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  प्रति (ध्रुव auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    जइ (O->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    जइ (O->hasArgStr())
      OptionNames.प्रक्षिप_back(O->ArgStr);

    तार्किक Permit_मानम् = O->get_मानम्_Expected_Flag() != cl::मानम्_Disallowed;
    StringRef Flag = Permit_मानम् ? LHS : Arg;
    प्रति (ध्रुव auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/सच्च, /*MaxEditDistance=*/BestDistance);
      जइ (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        जइ (RHS.empty() || !Permit_मानम्)
          NearestString = std::string(Name);
        अण्णहा
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  प्रत्यागम Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
स्थिर तार्किक CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef मानम्) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  जइ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मानम्);
    StringRef::size_type Pos = Val.find(',');

    जाव (Pos != StringRef::npos) {
      // Process the portion before the comma.
      जइ (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        प्रत्यागम सच्च;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मानम् = Val;
  }

  प्रत्यागम Handler->addOccurrence(pos, ArgName, मानम्);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
स्थिर अन्तःस्थ तार्किक ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef मानम्, पूर्णाङ्क argc,
                                 ध्रुव वर्ण *ध्रुव *argv, पूर्णाङ्क &i) {
  // Enforce value requirements
  विकल्प (Handler->get_मानम्_Expected_Flag()) {
  पक्ष मानम्_Required:
    जइ (!मानम्.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      जइ (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        प्रत्यागम Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      प्रतिज्ञा(argv && "null check");
      मानम् = StringRef(argv[++i]);
    }
    भङ्ग;
  पक्ष मानम्_Disallowed:
    जइ (मानम्.data())
      प्रत्यागम Handler->error("does not allow a value! '" + Twine(मानम्) +
                            "' specified.");
    भङ्ग;
  पक्ष मानम्_Optional:
    भङ्ग;
  }

  प्रत्यागम CommaSeparateAndAddOccurrence(Handler, i, ArgName, मानम्);
}

तार्किक llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, पूर्णाङ्क i) {
  पूर्णाङ्क Dummy = i;
  प्रत्यागम ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
स्थिर Option *findPrefixOption(StringRef Name, size_t &Length,
                                ध्रुव OptionsMapTy &OptionsMap) {
  प्रति (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    जइ (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      प्रत्यागम O;
    }
  }
  प्रत्यागम nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
स्थिर Option *HandlePrefixedOption(StringRef &Arg, StringRef &मानम्,
                                    ध्रुव OptionsMapTy &OptionsMap) {
  जइ (Arg.size() == 1)
    प्रत्यागम nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  जइ (!POpt)
    प्रत्यागम nullptr;

  StringRef Maybe_मानम् =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  जइ (POpt->getFormattingFlag() == cl::Prefix && Maybe_मानम्.starts_with("="))
    Maybe_मानम् = Maybe_मानम्.drop_front();
  मानम् = Maybe_मानम्;
  प्रत्यागम POpt;
}

स्थिर तार्किक Requires_मानम्(ध्रुव Option *O) {
  प्रत्यागम O->get_अङ्कः_Occurrences_Flag() == cl::Required ||
         O->get_अङ्कः_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक Eats_Unbounded_सङ्ख्या_Of_मानानि(ध्रुव Option *O) {
  प्रत्यागम O->get_अङ्कः_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_अङ्कः_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक isWhitespace(वर्ण C) {
  प्रत्यागम C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

स्थिर तार्किक isWhitespaceOrNull(वर्ण C) {
  प्रत्यागम isWhitespace(C) || C == '\0';
}

स्थिर तार्किक isQuote(वर्ण C) { प्रत्यागम C == '\"' || C == '\''; }

शून्य cl::Tokenize_GNUCommand_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                तार्किक MarkEOLs) {
  SmallString<128> Token;
  तार्किक InToken = मिच्छ;
  प्रति (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    जइ (!InToken) {
      जाव (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        जइ (MarkEOLs && Src[I] == '\n')
          NewArgv.प्रक्षिप_back(nullptr);
        ++I;
      }
      जइ (I == E)
        भङ्ग;
      InToken = सच्च;
    }

    वर्ण C = Src[I];

    // Backslash escapes the next character.
    जइ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.प्रक्षिप_back(Src[I]);
      अनुवर्त;
    }

    // Consume a quoted string.
    जइ (isQuote(C)) {
      ++I;
      जाव (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        जइ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.प्रक्षिप_back(Src[I]);
        ++I;
      }
      जइ (I == E)
        भङ्ग;
      अनुवर्त;
    }

    // End the token if this is whitespace.
    जइ (isWhitespace(C)) {
      NewArgv.प्रक्षिप_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      जइ (MarkEOLs && C == '\n')
        NewArgv.प्रक्षिप_back(nullptr);
      Token.clear();
      InToken = मिच्छ;
      अनुवर्त;
    }

    // This is a normal character.  Append it.
    Token.प्रक्षिप_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  जइ (InToken)
    NewArgv.प्रक्षिप_back(Saver.save(Token.str()).data());
}

/// Backslashes are interpreted in a rather complicated way in the Windows-style
/// command line, because backslashes are used both to separate path and to
/// escape double quote. This method consumes runs of backslashes as well as the
/// following double quote if it's escaped.
///
///  * If an even number of backslashes is followed by a double quote, one
///    backslash is output for every pair of backslashes, and the last double
///    quote remains unconsumed. The double quote will later be interpreted as
///    the start or end of a quoted string in the main loop outside of this
///    function.
///
///  * If an odd number of backslashes is followed by a double quote, one
///    backslash is output for every pair of backslashes, and a double quote is
///    output for the last pair of backslash-double quote. The double quote is
///    consumed in this case.
///
///  * Otherwise, backslashes are interpreted literally.
स्थिर size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  पूर्णाङ्क Backslash_गणनम् = 0;
  // Skip the backslashes.
  करेहि {
    ++I;
    ++Backslash_गणनम्;
  } जाव (I != E && Src[I] == '\\');

  तार्किक FollowedByDoubleQuote = (I != E && Src[I] == '"');
  जइ (FollowedByDoubleQuote) {
    Token.append(Backslash_गणनम् / 2, '\\');
    जइ (Backslash_गणनम् % 2 == 0)
      प्रत्यागम I - 1;
    Token.प्रक्षिप_back('"');
    प्रत्यागम I;
  }
  Token.append(Backslash_गणनम्, '\\');
  प्रत्यागम I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
स्थिर तार्किक isWindowsSpecialChar(वर्ण C) {
  प्रत्यागम isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
स्थिर तार्किक isWindowsSpecialCharInCommandName(वर्ण C) {
  प्रत्यागम isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
स्थिर अन्तःस्थ शून्य tokenize_Windows_Command_पङ्क्तिः_Impl(
    StringRef Src, StringSaver &Saver, function_ref<शून्य(StringRef)> AddToken,
    तार्किक AlwaysCopy, function_ref<शून्य()> MarkEOL, तार्किक InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  तार्किक CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  गणना { INIT, UNQUOTED, QUOTED } State = INIT;

  प्रति (size_t I = 0, E = Src.size(); I < E; ++I) {
    विकल्प (State) {
    पक्ष INIT: {
      प्रतिज्ञा(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      जाव (I < E && isWhitespaceOrNull(Src[I])) {
        जइ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      जइ (I >= E)
        भङ्ग;
      size_t Start = I;
      जइ (CommandName) {
        जाव (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } अण्णहा {
        जाव (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      जइ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        जइ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अण्णहा {
          CommandName = मिच्छ;
        }
      } अण्णहा जइ (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } अण्णहा जइ (Src[I] == '\\') {
        प्रतिज्ञा(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अण्णहा {
        llvm_unreachable("unexpected special character");
      }
      भङ्ग;
    }

    पक्ष UNQUOTED:
      जइ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        जइ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अण्णहा {
          CommandName = मिच्छ;
        }
        State = INIT;
      } अण्णहा जइ (Src[I] == '\"') {
        State = QUOTED;
      } अण्णहा जइ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अण्णहा {
        Token.प्रक्षिप_back(Src[I]);
      }
      भङ्ग;

    पक्ष QUOTED:
      जइ (Src[I] == '\"') {
        जइ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.प्रक्षिप_back('"');
          ++I;
        } अण्णहा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अण्णहा जइ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अण्णहा {
        Token.प्रक्षिप_back(Src[I]);
      }
      भङ्ग;
    }
  }

  जइ (State != INIT)
    AddToken(Saver.save(Token.str()));
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                    तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok.data()); };
  auto OnEOL = [&]() {
    जइ (MarkEOLs)
      NewArgv.प्रक्षिप_back(nullptr);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सच्च, OnEOL, मिच्छ);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/मिच्छ,
                                 OnEOL, मिच्छ);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                        तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok.data()); };
  auto OnEOL = [&]() {
    जइ (MarkEOLs)
      NewArgv.प्रक्षिप_back(nullptr);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सच्च, OnEOL, सच्च);
}

शून्य cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                            तार्किक MarkEOLs) {
  प्रति (ध्रुव वर्ण *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> पङ्क्तिः;
    // Check for comment line.
    जइ (isWhitespace(*Cur)) {
      जाव (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      अनुवर्त;
    }
    जइ (*Cur == '#') {
      जाव (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      अनुवर्त;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ध्रुव वर्ण *Start = Cur;
    ध्रुव वर्ण *पङ्क्तिः_End = nullptr;
    वर्ण Quote = 0;
    तार्किक AtTokenStart = सच्च;
    प्रति (ध्रुव वर्ण *End = Source.end(); Cur != End; ++Cur) {
      वर्ण C = *Cur;
      जइ (C == '\\') {
        जइ (Cur + 1 != End) {
          ++Cur;
          जइ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            पङ्क्तिः.append(Start, Cur - 1);
            जइ (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            अनुवर्त;
          }
        }
        AtTokenStart = मिच्छ;
        अनुवर्त;
      }
      जइ (C == '\n')
        भङ्ग;
      जइ (Quote) {
        जइ (C == Quote)
          Quote = 0;
      } अण्णहा जइ (isQuote(C)) {
        Quote = C;
      } अण्णहा जइ (C == '#' && AtTokenStart) {
        पङ्क्तिः_End = Cur;
        जाव (Cur != End && *Cur != '\n')
          ++Cur;
        भङ्ग;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    पङ्क्तिः.append(Start, पङ्क्तिः_End ? पङ्क्तिः_End : Cur);
    cl::Tokenize_GNUCommand_पङ्क्तिः(पङ्क्तिः, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
स्थिर तार्किक hasUTF8ByteOrderMark(ArrayRef<वर्ण> S) {
  प्रत्यागम (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
स्थिर शून्य ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ध्रुव वर्ण *&Arg) {
  प्रतिज्ञा(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ध्रुव StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  प्रति (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ध्रुव StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    जइ (ResponseFile.empty())
      ResponseFile = LHS;
    अण्णहा
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  जइ (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ध्रुव StringRef Remaining = ArgString.substr(StartPos);
    जइ (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ध्रुव वर्ण *> &NewArgv) {
  प्रतिज्ञा(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  जइ (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    प्रत्यागम llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<वर्ण> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  जइ (hasUTF16ByteOrderMark(BufRef)) {
    जइ (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      प्रत्यागम llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अण्णहा जइ (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  जइ (!RelativeNames && !InConfigFile)
    प्रत्यागम Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  प्रति (ध्रुव वर्ण *&Arg : NewArgv) {
    जइ (!Arg)
      अनुवर्त;

    // Substitute <CFGDIR> with the file's base path.
    जइ (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    तार्किक ConfigInclusion = मिच्छ;
    जइ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      जइ (!llvm::sys::path::is_relative(FileName))
        अनुवर्त;
    } अण्णहा जइ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = सच्च;
    } अण्णहा {
      अनुवर्त;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.प्रक्षिप_back('@');
    जइ (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      जइ (!findConfigFile(FileName, FilePath))
        प्रत्यागम createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } अण्णहा {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  प्रत्यागम Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  संरचना ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_राशिः;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_राशिः.प्रक्षिप_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  प्रति (unsigned I = 0; I != Argv.size();) {
    जाव (I == File_राशिः.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_राशिः.निष्कासय_back();
    }

    ध्रुव वर्ण *Arg = Argv[I];
    // Check if it is an EOL marker
    जइ (Arg == nullptr) {
      ++I;
      अनुवर्त;
    }

    जइ (Arg[0] != '@') {
      ++I;
      अनुवर्त;
    }

    ध्रुव वर्ण *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    जइ (llvm::sys::path::is_relative(FName)) {
      जइ (CurrentDir.empty()) {
        जइ (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अण्णहा {
          प्रत्यागम createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अण्णहा {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    जइ (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      जइ (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        जइ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          अनुवर्त;
        }
      }
      जइ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      प्रत्यागम createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ध्रुव llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, एतत्](ध्रुव ResponseFileRecord &RFile) -> ErrorOr<तार्किक> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      जइ (!RHS)
        प्रत्यागम RHS.getError();
      प्रत्यागम FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    प्रति (ध्रुव auto &F : drop_begin(File_राशिः)) {
      जइ (ErrorOr<तार्किक> R = IsEquivalent(F)) {
        जइ (R.get())
          प्रत्यागम createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अण्णहा {
        प्रत्यागम createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ध्रुव वर्ण *, 0> ExpandedArgv;
    जइ (Error Err = expandResponseFile(FName, ExpandedArgv))
      प्रत्यागम Err;

    प्रति (ResponseFileRecord &Record : File_राशिः) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_राशिः.प्रक्षिप_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.निवेशय(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  प्रतिज्ञा(File_राशिः.size() > 0 && Argv.size() == File_राशिः.back().End);
  प्रत्यागम Error::success();
}

तार्किक cl::expandResponseFiles(पूर्णाङ्क Argc, ध्रुव वर्ण *ध्रुव *Argv,
                             ध्रुव वर्ण *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ध्रुव वर्ण *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अण्णहा
  auto Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#endif
  // The environment variable specifies initial options.
  जइ (EnvVar)
    जइ (std::optional<std::string> Env_मानम् = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मानम्, Saver, NewArgv, /*MarkEOLs=*/मिच्छ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  जइ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    प्रत्यागम मिच्छ;
  }
  प्रत्यागम सच्च;
}

तार्किक cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  जइ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    प्रत्यागम मिच्छ;
  }
  प्रत्यागम सच्च;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

तार्किक ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<वर्ण> &FilePath) {
  SmallString<128> CfgFilePath;
  ध्रुव auto FileExists = [एतत्](SmallString<128> Path) -> तार्किक {
    auto Status = FS->status(Path);
    प्रत्यागम Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  जइ (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    जइ (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      प्रत्यागम मिच्छ;
    जइ (!FileExists(CfgFilePath))
      प्रत्यागम मिच्छ;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    प्रत्यागम सच्च;
  }

  // Look for the file in search directories.
  प्रति (ध्रुव StringRef &Dir : अन्वेषणम्_Dirs) {
    जइ (Dir.empty())
      अनुवर्त;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    जइ (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      प्रत्यागम सच्च;
    }
  }

  प्रत्यागम मिच्छ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  SmallString<128> AbsPath;
  जइ (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    जइ (std::error_code EC = FS->makeAbsolute(AbsPath))
      प्रत्यागम make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = सच्च;
  RelativeNames = सच्च;
  जइ (Error Err = expandResponseFile(CfgFile, Argv))
    प्रत्यागम Err;
  प्रत्यागम expandResponseFiles(Argv);
}

स्थिर शून्य initCommonOptions();
स्थिर तार्किक parse_Command_पङ्क्तिः_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ध्रुव वर्ण *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ध्रुव वर्ण *EnvVar) {
  initCommonOptions();
  SmallVector<ध्रुव वर्ण *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.प्रक्षिप_back(Args[0]);

  // Parse options from environment variable.
  जइ (EnvVar) {
    जइ (std::optional<std::string> Env_मानम् =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_पङ्क्तिः(*Env_मानम्, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  पूर्णाङ्क NewArgc = static_cast<पूर्णाङ्क>(NewArgv.size());

  // Parse all options.
  प्रत्यागम globalParser().Parse_Command_पङ्क्तिः_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

तार्किक cl::Parse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_पङ्क्तिः_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

तार्किक cl::parse_Command_पङ्क्तिः_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ध्रुव वर्ण *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_पङ्क्तिः_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
शून्य Command_पङ्क्तिः_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  प्रति (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    प्रति (auto &O : SC->OptionsMap)
      Opts.प्रक्षिप_back(O.second);
    प्रति (Option *O : Opts)
      O->reset();
    प्रति (Option *O : SC->PositionalOpts)
      O->reset();
    जइ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  प्रति (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

तार्किक Command_पङ्क्तिः_Parser::Parse_Command_पङ्क्तिः_Options(
    पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  प्रतिज्ञा(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  तार्किक IgnoreErrors = Errs;
  जइ (!Errs)
    Errs = &errs();
  जइ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  तार्किक ErrorParsing = मिच्छ;

  // Expand response files.
  SmallVector<ध्रुव वर्ण *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अण्णहा
  auto Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  जइ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    प्रत्यागम मिच्छ;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णाङ्क>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned अङ्कः_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  तार्किक HasUnlimitedPositionals = मिच्छ;

  पूर्णाङ्क FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शिखरम्_Level();
  std::string NearestSubCommandString;
  तार्किक MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  जइ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    जइ (ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  प्रतिज्ञा(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  जइ (ConsumeAfterOpt) {
    प्रतिज्ञा(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  जइ (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    तार्किक UnboundedFound = मिच्छ;
    प्रति (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      जइ (Requires_मानम्(Opt))
        ++अङ्कः_Positional_Required;
      अण्णहा जइ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        जइ (PositionalOpts.size() > 1) {
          जइ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = सच्च;
        }
      } अण्णहा जइ (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        जइ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = सच्च;
      }
      UnboundedFound |= Eats_Unbounded_सङ्ख्या_Of_मानानि(Opt);
    }
    HasUnlimitedPositionals = UnboundedFound || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  SmallVector<std::pair<StringRef, unsigned>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *ActivePositionalArg = nullptr;

  // Loop over all of the arguments... processing them.
  तार्किक DashDashFound = मिच्छ; // Have we read '--'?
  प्रति (पूर्णाङ्क i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef मानम्;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    जइ (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      जइ (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }

      जइ (!PositionalOpts.empty()) {
        PositionalVals.प्रक्षिप_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        जइ (PositionalVals.size() >= अङ्कः_Positional_Required && ConsumeAfterOpt) {
          प्रति (++i; i < argc; ++i)
            PositionalVals.प्रक्षिप_back(std::make_pair(StringRef(argv[i]), i));
          भङ्ग; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        अनुवर्त;
      }
    } अण्णहा जइ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = सच्च; // This is the mythical "--"?
      अनुवर्त;             // Don't try to process it as an argument itself.
    } अण्णहा जइ (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);
      जइ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }
    } अण्णहा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      जइ (!Handler && ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
        Handler = LookupOption(SubCommand::get_शिखरम्_Level(), ArgName, मानम्);

      जइ (!Handler) {
        जइ (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ध्रुव वर्ण *> Rest(argv + i, argc - i);
          unsigned N;
          जइ (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = सच्च;
          }
          i += N - 1;
          अनुवर्त;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      जइ (!Handler)
        Handler = HandlePrefixedOption(ArgName, मानम्, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      जइ (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    जइ (!Handler) {
      auto ReportUnknownArgument = [&](तार्किक IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        जइ (NearestArgumentName.empty())
          प्रत्यागम;

        *Errs << ProgramName << ": Did you mean '";
        जइ (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        अण्णहा
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      जइ (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/सच्च, NearestHandlerString);
      अण्णहा
        ReportUnknownArgument(/*IsArg=*/मिच्छ, NearestSubCommandString);

      ErrorParsing = सच्च;
      अनुवर्त;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    जइ (Handler->getFormattingFlag() == cl::Positional) {
      जइ ((Handler->getMiscFlags() & PositionalEatsArgs) && !मानम्.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = सच्च;
      }
      ActivePositionalArg = Handler;
    }
    अण्णहा
      ErrorParsing |= ProvideOption(Handler, ArgName, मानम्, argc, argv, i);
  }

  // Check and handle positional arguments now...
  जइ (अङ्कः_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अङ्कः_Positional_Required
             << " positional argument" << (अङ्कः_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = सच्च;
  } अण्णहा जइ (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = सच्च;

  } अण्णहा जइ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, अङ्कः_Vals = static_cast<unsigned>(PositionalVals.size());
    प्रति (Option *Opt : PositionalOpts) {
      जइ (Requires_मानम्(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --अङ्कः_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      तार्किक Done = Opt->get_अङ्कः_Occurrences_Flag() == cl::Required;
      जाव (अङ्कः_Vals - ValNo > अङ्कः_Positional_Required && !Done) {
        विकल्प (Opt->get_अङ्कः_Occurrences_Flag()) {
        पक्ष cl::Optional:
          Done = सच्च; // Optional arguments want _at most_ one value
          [[fallthrough]];
        पक्ष cl::ZeroOrMore: // Zero or more will take all they can get...
        पक्ष cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          भङ्ग;
        स्वभाव:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } अण्णहा {
    प्रतिज्ञा(ConsumeAfterOpt && अङ्कः_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    प्रति (Option *Opt : PositionalOpts)
      जइ (Requires_मानम्(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    जइ (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    प्रति (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  प्रति (ध्रुव auto &Opt : OptionsMap) {
    विकल्प (Opt.second->get_अङ्कः_Occurrences_Flag()) {
    पक्ष Required:
    पक्ष OneOrMore:
      जइ (Opt.second->get_अङ्कः_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = सच्च;
      }
      [[fallthrough]];
    स्वभाव:
      भङ्ग;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             प्रति (पूर्णाङ्क i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  जइ (ErrorParsing) {
    जइ (!IgnoreErrors)
      exit(1);
    प्रत्यागम मिच्छ;
  }
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

तार्किक Option::error(ध्रुव Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  जइ (!ArgName.data())
    ArgName = ArgStr;
  जइ (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  अण्णहा
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  प्रत्यागम सच्च;
}

तार्किक Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef मानम्) {
  ++अङ्कः_Occurrences;
  प्रत्यागम handleOccurrence(pos, ArgName, मानम्);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
स्थिर StringRef get_मानम्_Str(ध्रुव Option &O, StringRef Default_सन्देशः) {
  जइ (O.मानम्_Str.empty())
    प्रत्यागम Default_सन्देशः;
  प्रत्यागम O.मानम्_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ध्रुव {
  प्रत्यागम argPlusPrefixesSize(ArgStr);
}

शून्य Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_पङ्क्तिः_Indented_By) {
  प्रतिज्ञा(Indent >= First_पङ्क्तिः_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_पङ्क्तिः_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  जाव (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

शून्य Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_पङ्क्तिः_Indented_By) {
  ध्रुव StringRef ValHelpPrefix = "  ";
  प्रतिज्ञा(BaseIndent >= First_पङ्क्तिः_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_पङ्क्तिः_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  जाव (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
शून्य alias::printOptionInfo(size_t GlobalWidth) ध्रुव {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(ध्रुव Option &O) ध्रुव {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_मानम्_Name();
  जइ (!ValName.empty()) {
    size_t FormattingLen = 3;
    जइ (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_मानम्_Str(O, ValName).size() + FormattingLen;
  }

  प्रत्यागम Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य basic_parser_impl::printOptionInfo(ध्रुव Option &O,
                                        size_t GlobalWidth) ध्रुव {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_मानम्_Name();
  जइ (!ValName.empty()) {
    जइ (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_मानम्_Str(O, ValName) << ">...";
    } अण्णहा जइ (O.get_मानम्_Expected_Flag() == मानम्_Optional)
      outs() << "[=<" << get_मानम्_Str(O, ValName) << ">]";
    अण्णहा {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_मानम्_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

शून्य basic_parser_impl::printOptionName(ध्रुव Option &O,
                                        size_t GlobalWidth) ध्रुव {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
तार्किक parser<तार्किक>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         तार्किक &मानम्) {
  प्रत्यागम parseBool<तार्किक, सच्च, मिच्छ>(O, ArgName, Arg, मानम्);
}

// parser<boolOrDefault> implementation
//
तार्किक parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मानम्) {
  प्रत्यागम parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, मानम्);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
स्थिर तार्किक parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef मानम्_Kind,
                                         FixedOrScalableQuantityT &मानम्) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_मानम्;
  जइ (!Arg.getAsInteger(0, Min_मानम्)) {
    मानम् = FixedOrScalableQuantityT::getFixed(Min_मानम्);
    प्रत्यागम मिच्छ;
  }

  StringRef Remainder = Arg;
  जइ (!Remainder.consume_front("vscale"))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  जइ (!Remainder.consume_front('x'))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  जइ (Remainder.getAsInteger(0, Min_मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  मानम् = FixedOrScalableQuantityT::getScalable(Min_मानम्);
  प्रत्यागम मिच्छ;
}

// parser<int> implementation
//
तार्किक parser<पूर्णाङ्क>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णाङ्क &मानम्) {
  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for integer argument!");
  प्रत्यागम मिच्छ;
}

// parser<long> implementation
//
तार्किक parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &मानम्) {
  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for long argument!");
  प्रत्यागम मिच्छ;
}

// parser<long long> implementation
//
तार्किक parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &मानम्) {
  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for llong argument!");
  प्रत्यागम मिच्छ;
}

// parser<unsigned> implementation
//
तार्किक parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &मानम्) {

  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for uint argument!");
  प्रत्यागम मिच्छ;
}

// parser<unsigned long> implementation
//
तार्किक parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &मानम्) {

  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ulong argument!");
  प्रत्यागम मिच्छ;
}

// parser<unsigned long long> implementation
//
तार्किक parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &मानम्) {

  जइ (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ullong argument!");
  प्रत्यागम मिच्छ;
}

// parser<ElementCount> implementation
//
तार्किक parser<Element_गणनम्>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_गणनम् &मानम्) {
  प्रत्यागम parseFixedOrScalableQuantity(O, Arg, get_मानम्_Name(), मानम्);
}

// parser<double>/parser<float> implementation
//
स्थिर तार्किक parseDouble(Option &O, StringRef Arg, द्विगुण &मानम्) {
  जइ (to_float(Arg, मानम्))
    प्रत्यागम मिच्छ;
  प्रत्यागम O.error("'" + Arg + "' value invalid for floating point argument!");
}

तार्किक parser<द्विगुण>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुण &Val) {
  प्रत्यागम parseDouble(O, Arg, Val);
}

तार्किक parser<भिन्न>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          भिन्न &Val) {
  द्विगुण dVal;
  जइ (parseDouble(O, Arg, dVal))
    प्रत्यागम सच्च;
  Val = (भिन्न)dVal;
  प्रत्यागम मिच्छ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_अङ्कः_Options();

  प्रति (unsigned i = 0; i != e; ++i) {
    जइ (getOption(i) == Name)
      प्रत्यागम i;
  }
  प्रत्यागम e;
}

स्थिर StringRef Eq_मानम् = "=<value>";
स्थिर StringRef EmptyOption = "<empty>";
स्थिर StringRef OptionPrefix = "    =";
स्थिर size_t getOptionPrefixesSize() {
  प्रत्यागम OptionPrefix.size() + ArgHelpPrefix.size();
}

स्थिर तार्किक shouldPrintOption(StringRef Name, StringRef Description,
                              ध्रुव Option &O) {
  प्रत्यागम O.get_मानम्_Expected_Flag() != मानम्_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ध्रुव Option &O) ध्रुव {
  जइ (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_मानम्.size();
    प्रति (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      जइ (!shouldPrintOption(Name, getDescription(i), O))
        अनुवर्त;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    प्रत्यागम Size;
  } अण्णहा {
    size_t BaseSize = 0;
    प्रति (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    प्रत्यागम BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य generic_parser_base::printOptionInfo(ध्रुव Option &O,
                                          size_t GlobalWidth) ध्रुव {
  जइ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    जइ (O.get_मानम्_Expected_Flag() == मानम्_Optional) {
      प्रति (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
        जइ (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          भङ्ग;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_मानम्;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_मानम्.size() +
                             argPlusPrefixesSize(O.ArgStr));
    प्रति (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      जइ (!shouldPrintOption(OptionName, Description, O))
        अनुवर्त;
      size_t First_पङ्क्तिः_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      जइ (OptionName.empty()) {
        outs() << EmptyOption;
        प्रतिज्ञा(First_पङ्क्तिः_Indent >= EmptyOption.size());
        First_पङ्क्तिः_Indent += EmptyOption.size();
      }
      जइ (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_पङ्क्तिः_Indent);
      अण्णहा
        outs() << '\n';
    }
  } अण्णहा {
    जइ (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    प्रति (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

स्थिर ध्रुव size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
शून्य generic_parser_base::printGenericOptionDiff(
    ध्रुव Option &O, ध्रुव Generic_Option_मानम् &मानम्,
    ध्रुव Generic_Option_मानम् &Default, size_t GlobalWidth) ध्रुव {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned अङ्कः_Opts = get_अङ्कः_Options();
  प्रति (unsigned i = 0; i != अङ्कः_Opts; ++i) {
    जइ (!मानम्.compare(get_Option_मानम्(i)))
      अनुवर्त;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t अङ्कः_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(अङ्कः_Spaces) << " (default: ";
    प्रति (unsigned j = 0; j != अङ्कः_Opts; ++j) {
      जइ (!Default.compare(get_Option_मानम्(j)))
        अनुवर्त;
      outs() << getOption(j);
      भङ्ग;
    }
    outs() << ")\n";
    प्रत्यागम;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
नामाकाश llvm {
नामाकाश cl {
स्थिर raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  प्रत्यागम OS << static_cast<पूर्णाङ्क>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  शून्य parser<T>::printOptionDiff(ध्रुव Option &O, T V, Option_मानम्<T> D,      \
                                  size_t GlobalWidth) ध्रुव {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t अङ्कः_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(अङ्कः_Spaces) << " (default: ";                                 \
    जइ (D.has_मानम्())                                                          \
      outs() << D.get_मानम्();                                                  \
    अण्णहा                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(तार्किक)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(पूर्णाङ्क)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(द्विगुण)
PRINT_OPT_DIFF(भिन्न)
PRINT_OPT_DIFF(वर्ण)
PRINT_OPT_DIFF(Element_गणनम्)

शून्य parser<std::string>::printOptionDiff(ध्रुव Option &O, StringRef V,
                                          ध्रुव Option_मानम्<std::string> &D,
                                          size_t GlobalWidth) ध्रुव {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t अङ्कः_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(अङ्कः_Spaces) << " (default: ";
  जइ (D.has_मानम्())
    outs() << D.get_मानम्();
  अण्णहा
    outs() << "*no default*";
  outs() << ")\n";
}

शून्य parser<std::optional<std::string>>::printOptionDiff(
    ध्रुव Option &O, std::optional<StringRef> V,
    ध्रुव Option_मानम्<std::optional<std::string>> &D,
    size_t GlobalWidth) ध्रुव {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_मानम्_2() ? V.मानम्_2().size() : 0;
  size_t अङ्कः_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(अङ्कः_Spaces) << " (default: ";
  जइ (D.has_मानम्() && D.get_मानम्().has_मानम्_2())
    outs() << D.get_मानम्();
  अण्णहा
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
शून्य basic_parser_impl::print_Option_No_मानम्(ध्रुव Option &O,
                                           size_t GlobalWidth) ध्रुव {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

स्थिर पूर्णाङ्क OptNameCompare(ध्रुव std::pair<ध्रुव वर्ण *, Option *> *LHS,
                          ध्रुव std::pair<ध्रुव वर्ण *, Option *> *RHS) {
  प्रत्यागम strcmp(LHS->first, RHS->first);
}

स्थिर पूर्णाङ्क SubNameCompare(ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *LHS,
                          ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *RHS) {
  प्रत्यागम strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
स्थिर शून्य sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ध्रुव वर्ण *, Option *>> &Opts,
                     तार्किक ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  प्रति (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    जइ (I->second->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    // Unless showhidden is set, ignore hidden flags.
    जइ (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      अनुवर्त;

    // If we've already seen this option, don't add it to the list again.
    जइ (!OptionSet.निवेशय(I->second).second)
      अनुवर्त;

    Opts.प्रक्षिप_back(
        std::pair<ध्रुव वर्ण *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

स्थिर शून्य
sortSubCommands(ध्रुव SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ध्रुव वर्ण *, SubCommand *>> &Subs) {
  प्रति (auto *S : SubMap) {
    जइ (S->getName().empty())
      अनुवर्त;
    Subs.प्रक्षिप_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

नामाकाश {

वर्ग HelpPrinter {
रक्षित:
  ध्रुव तार्किक ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ध्रुव वर्ण *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ध्रुव वर्ण *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  आभास शून्य printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    प्रति (ध्रुव auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  शून्य printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    प्रति (ध्रुव auto &S : Subs) {
      outs() << "  " << S.first;
      जइ (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

सार्वजनिक:
  explicit HelpPrinter(तार्किक showHidden) : ShowHidden(showHidden) {}
  आभास ~HelpPrinter() = स्वभाव;

  // Invoke the printer.
  शून्य operator=(तार्किक मानम्) {
    जइ (!मानम्)
      प्रत्यागम;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  शून्य printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    जइ (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    जइ (Sub == &SubCommand::get_शिखरम्_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      जइ (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } अण्णहा {
      जइ (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    प्रति (auto *Opt : PositionalOpts) {
      जइ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    जइ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    जइ (Sub == &SubCommand::get_शिखरम्_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      प्रति (ध्रुव auto &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, strlen(Sub.first));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().ProgramName
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    प्रति (ध्रुव auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    जइ (ShowHidden)
      प्रति (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              जइ (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    प्रति (ध्रुव auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    जइ (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    प्रति (ध्रुव auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    प्रति (ध्रुव auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

वर्ग CategorizedHelpPrinter : सार्वजनिक HelpPrinter {
सार्वजनिक:
  explicit CategorizedHelpPrinter(तार्किक showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  स्थिर पूर्णाङ्क OptionCategoryCompare(OptionCategory *ध्रुव *A,
                                   OptionCategory *ध्रुव *B) {
    प्रत्यागम (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

रक्षित:
  शून्य printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    प्रतिज्ञा(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    प्रति (ध्रुव auto &I : Opts) {
      Option *Opt = I.second;
      प्रति (OptionCategory *Cat : Opt->Categories) {
        प्रतिज्ञा(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].प्रक्षिप_back(Opt);
      }
    }

    // Now do printing.
    प्रति (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ध्रुव auto &CategoryOptions = CategorizedOptions[Category];
      जइ (CategoryOptions.empty())
        अनुवर्त;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      जइ (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      अण्णहा
        outs() << "\n";

      // Loop over the options in the category and print.
      प्रति (ध्रुव Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
वर्ग HelpPrinterWrapper {
गुप्त:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

सार्वजनिक:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  शून्य operator=(तार्किक मानम्);
};

} // End anonymous namespace

#जइ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# जइ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अण्णहा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#नोचेद्यदि defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# जइ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# अण्णहा
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#अण्णहा
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

नामाकाश {
वर्ग VersionPrinter {
सार्वजनिक:
  शून्य print(ध्रुव std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अण्णहा
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#जइ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अण्णहा
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    जइ (!ExtraPrinters.empty()) {
      प्रति (ध्रुव auto &I : ExtraPrinters)
        I(outs());
    }
  }
  शून्य operator=(तार्किक OptionWasSpecified);
};

संरचना Command_पङ्क्तिः_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{मिच्छ};
  HelpPrinter UncategorizedHiddenPrinter{सच्च};
  CategorizedHelpPrinter CategorizedNormalPrinter{मिच्छ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{सच्च};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper WrappedNormalPrinter{UncategorizedNormalPrinter,
                                          CategorizedNormalPrinter};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, सच्च, parser<तार्किक>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, सच्च, parser<तार्किक>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, सच्च, parser<तार्किक>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, सच्च, parser<तार्किक>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(मिच्छ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(मिच्छ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, सच्च, parser<तार्किक>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::मानम्_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
स्थिर ManagedStatic<Command_पङ्क्तिः_Common_Options> CommonOptions;

स्थिर शून्य initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  initGraphWriterOptions();
  initSignalsOptions();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  initRandomSeedOptions();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  स्थिर OptionCategory GeneralCategory{"General options"};
  प्रत्यागम GeneralCategory;
}

शून्य VersionPrinter::operator=(तार्किक OptionWasSpecified) {
  जइ (!OptionWasSpecified)
    प्रत्यागम;

  जइ (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

शून्य HelpPrinterWrapper::operator=(तार्किक मानम्) {
  जइ (!मानम्)
    प्रत्यागम;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  जइ (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = सच्च; // Invoke categorized printer
  } अण्णहा {
    UncategorizedPrinter = सच्च; // Invoke uncategorized printer
  }
}

// Print the value of each option.
शून्य cl::Print_Option_मानानि() { globalParser().print_Option_मानानि(); }

शून्य Command_पङ्क्तिः_Parser::print_Option_मानानि() {
  जइ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    प्रत्यागम;

  SmallVector<std::pair<ध्रुव वर्ण *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ सच्च);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  प्रति (ध्रुव auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  प्रति (ध्रुव auto &Opt : Opts)
    Opt.second->print_Option_मानम्(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
शून्य cl::PrintHelpMessage(तार्किक Hidden, तार्किक Categorized) {
  जइ (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  अण्णहा जइ (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  अण्णहा जइ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अण्णहा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  स्थिर ध्रुव StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#जइ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#जइ __has_feature(address_sanitizer)
      "+asan",
#endif
#जइ __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#जइ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#जइ __has_feature(memory_sanitizer)
      "+msan",
#endif
#जइ __has_feature(thread_sanitizer)
      "+tsan",
#endif
#जइ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  प्रत्यागम ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
शून्य cl::printBuildConfig(raw_ostream &OS) {
#जइ LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
शून्य cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

शून्य cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

शून्य cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.प्रक्षिप_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (शून्य)Subs;
  प्रतिज्ञा(Subs.contains(&Sub));
  प्रत्यागम Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  प्रत्यागम globalParser().getRegisteredSubcommands();
}

शून्य cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  प्रति (auto &I : Sub.OptionsMap) {
    तार्किक Unrelated = सच्च;
    प्रति (auto &Cat : I.second->Categories) {
      जइ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = मिच्छ;
    }
    जइ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::HideUnrelatedOptions(ArrayRef<ध्रुव cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  प्रति (auto &I : Sub.OptionsMap) {
    तार्किक Unrelated = सच्च;
    प्रति (auto &Cat : I.second->Categories) {
      जइ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = मिच्छ;
    }
    जइ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::Reset_Command_पङ्क्तिः_Parser() { globalParser().reset(); }

शून्य cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.प्रक्षिप_back(&L);
}

शून्य LibraryOptionsContext::assign(ध्रुव LibraryOptionsContext &Other) {
  प्रतिज्ञा(एतत् != &Other);
  मानानि.clear();
  SavedArgs.clear();
  ArrayRef<ध्रुव वर्ण *> Args = Other.SavedArgs;
  प्रति (size_t I = 0, E = Args.size(); I != E;) {
    StringRef Name = Args[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.split('=').first);
    unsigned N;
    cantFail(parse(*L, Args.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ध्रुव वर्ण *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = मानानि.try_emplace(L.getDefault());
  जइ (Inserted)
    It->second = L.copyDefault(Alloc);
  प्रत्यागम parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

शून्य cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

शून्य LLVMParse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 ध्रुव वर्ण *Overview) {
  llvm::cl::Parse_Command_पङ्क्तिः_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
