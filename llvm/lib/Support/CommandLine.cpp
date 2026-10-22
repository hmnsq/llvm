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
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<तार्किक>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<पूर्णाङ्क>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<long>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<द्विगुण>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<भिन्न>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<वर्ण>;
template वग्ग LLVM_EXPORT_TEMPLATE basic_parser<Element_गणनम्>;

#सचे !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template वग्ग LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template वग्ग LLVM_EXPORT_TEMPLATE opt<तार्किक>;
template वग्ग LLVM_EXPORT_TEMPLATE opt<वर्ण>;
template वग्ग LLVM_EXPORT_TEMPLATE opt<पूर्णाङ्क>;
template वग्ग LLVM_EXPORT_TEMPLATE opt<unsigned>;

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
  सचे (Len == 1)
    पटिनिवत्त Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  पटिनिवत्त Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

स्थिर SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  अत्थाय (size_t I = 0; I < Pad; ++I) {
    Prefix.प्रक्षिप_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  पटिनिवत्त Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

नामाकाश {

वग्ग PrintArg {
  StringRef ArgName;
  size_t Pad;
सार्वजनिक:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ध्रुव PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ध्रुव PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  पटिनिवत्त OS;
}

वग्ग Command_पङ्क्तिः_Parser {
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
    सचे (Opt.Subs.empty()) {
      Action(SubCommand::get_शिखरम्_Level());
      पटिनिवत्त;
    }
    सचे (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      अत्थाय (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      पटिनिवत्त;
    }
    अत्थाय (auto *SC : Opt.Subs) {
      प्रतिज्ञा(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  शून्य addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    सचे (Opt.hasArgStr())
      पटिनिवत्त;
    सचे (!SC->OptionsMap.निवेशय(std::make_pair(Name, &Opt)).second) {
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
    तार्किक HadErrors = मिच्छा;
    सचे (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      सचे (!SC->OptionsMap.निवेशय(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = सच्च;
      }
    }

    // Remember information about positional options.
    सचे (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.प्रक्षिप_back(O);
    अञ्ञथा सचे (O->get_अङ्कः_Occurrences_Flag() == cl::ConsumeAfter) {
      सचे (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = सच्च;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    सचे (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  शून्य addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  शून्य removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    सचे (O->hasArgStr())
      OptionNames.प्रक्षिप_back(O->ArgStr);

    SubCommand &Sub = *SC;
    अत्थाय (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      सचे (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    सचे (O->getFormattingFlag() == cl::Positional)
      अत्थाय (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        सचे (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          भङ्ग;
        }
      }
    अञ्ञथा सचे (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  शून्य removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  तार्किक hasOptions(ध्रुव SubCommand &Sub) ध्रुव {
    पटिनिवत्त (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  तार्किक hasOptions() ध्रुव {
    अत्थाय (ध्रुव auto *S : RegisteredSubCommands) {
      सचे (hasOptions(*S))
        पटिनिवत्त सच्च;
    }
    पटिनिवत्त !Libraries.empty();
  }

  तार्किक hasNamedSubCommands() ध्रुव {
    अत्थाय (ध्रुव auto *S : RegisteredSubCommands)
      सचे (!S->getName().empty())
        पटिनिवत्त सच्च;
    पटिनिवत्त मिच्छा;
  }

  SubCommand *getActiveSubCommand() { पटिनिवत्त ActiveSubCommand; }

  शून्य updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    सचे (!Sub.OptionsMap.निवेशय(std::make_pair(NewName, O)).second) {
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
    तार्किक HadErrors = मिच्छा;
    अत्थाय (; अङ्कः_Indexed_Libraries != Libraries.size(); ++अङ्कः_Indexed_Libraries) {
      LibraryOptions *L = Libraries[अङ्कः_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        सचे (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 पटिनिवत्त SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          पटिनिवत्त;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = सच्च;
      });
    }
    सचे (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    पटिनिवत्त LibraryIndex.lookup(Name);
  }

  शून्य registerCategory(OptionCategory *cat) {
    प्रतिज्ञा(गणनम्_if(RegisteredOptionCategories,
                    [cat](ध्रुव OptionCategory *Category) {
             पटिनिवत्त cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.निवेशय(cat);
  }

  शून्य registerSubCommand(SubCommand *sub) {
    प्रतिज्ञा(गणनम्_if(RegisteredSubCommands,
                    [sub](ध्रुव SubCommand *Sub) {
                      पटिनिवत्त (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.निवेशय(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    प्रतिज्ञा(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    अत्थाय (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      सचे (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      अञ्ञथा
        addLiteralOption(*O, sub, E.first);
    }
  }

  शून्य unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    पटिनिवत्त make_range(RegisteredSubCommands.begin(),
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
  पटिनिवत्त *GlobalParser;
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
  अत्थाय (unsigned J = 0; J != अङ्कः_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  सचे (Error E = L.parse(Opts, ArrayRef(Saved, अङ्कः_Args), Consumed, Alloc))
    पटिनिवत्त E;
  सचे (Record)
    Record->append(Saved, Saved + Consumed);
  पटिनिवत्त Error::success();
}

template <typename T, T TrueVal, T FalseVal>
स्थिर तार्किक parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मानम्) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  सचे (!Arg.data() || Arg == "true" || Arg == "1") {
    मानम् = TrueVal;
    पटिनिवत्त मिच्छा;
  }

  सचे (Arg == "false" || Arg == "0") {
    मानम् = FalseVal;
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त O.error("'" + Arg +
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
      FullyInitialized(मिच्छा), Position(0) {
  Categories.प्रक्षिप_back(&getGeneralCategory());
}

शून्य Option::addArgument() {
  globalParser().addOption(एतत्);
  FullyInitialized = सच्च;
}

शून्य Option::removeArgument() { globalParser().removeOption(एतत्); }

शून्य Option::setArgStr(StringRef S) {
  सचे (FullyInitialized)
    globalParser().updateArgStr(एतत्, S);
  प्रतिज्ञा(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

शून्य Option::addCategory(OptionCategory &C) {
  प्रतिज्ञा(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  सचे (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अञ्ञथा सचे (!is_contained(Categories, &C))
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
  पटिनिवत्त *शिखरम्_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  स्थिर ManagedStatic<SubCommand> AllSubCommands;
  पटिनिवत्त *AllSubCommands;
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
  पटिनिवत्त (globalParser().getActiveSubCommand() == एतत्);
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
  सचे (Arg.empty())
    पटिनिवत्त nullptr;
  प्रतिज्ञा(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  सचे (EqualPos == StringRef::npos) {
    // Look up the option.
    पटिनिवत्त Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  सचे (I == Sub.OptionsMap.end())
    पटिनिवत्त nullptr;

  auto *O = I->second;
  सचे (O->getFormattingFlag() == cl::AlwaysPrefix)
    पटिनिवत्त nullptr;

  मानम् = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  पटिनिवत्त I->second;
}

SubCommand *Command_पङ्क्तिः_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  सचे (Name.empty())
    पटिनिवत्त &SubCommand::get_शिखरम्_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  अत्थाय (auto *S : RegisteredSubCommands) {
    प्रतिज्ञा(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    सचे (S->getName().empty())
      अनुवर्त;

    सचे (S->getName() == Name)
      पटिनिवत्त S;

    सचे (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  सचे (NearestMatch)
    NearestString = NearestMatch->getName();

  पटिनिवत्त &SubCommand::get_शिखरम्_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
स्थिर Option *LookupNearestOption(StringRef Arg,
                                   ध्रुव OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  सचे (Arg.empty())
    पटिनिवत्त nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  अत्थाय (ध्रुव auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    सचे (O->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    सचे (O->hasArgStr())
      OptionNames.प्रक्षिप_back(O->ArgStr);

    तार्किक Permit_मानम् = O->get_मानम्_Expected_Flag() != cl::मानम्_Disallowed;
    StringRef Flag = Permit_मानम् ? LHS : Arg;
    अत्थाय (ध्रुव auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/सच्च, /*MaxEditDistance=*/BestDistance);
      सचे (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        सचे (RHS.empty() || !Permit_मानम्)
          NearestString = std::string(Name);
        अञ्ञथा
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  पटिनिवत्त Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
स्थिर तार्किक CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef मानम्) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  सचे (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मानम्);
    StringRef::size_type Pos = Val.find(',');

    याव (Pos != StringRef::npos) {
      // Process the portion before the comma.
      सचे (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        पटिनिवत्त सच्च;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मानम् = Val;
  }

  पटिनिवत्त Handler->addOccurrence(pos, ArgName, मानम्);
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
    सचे (!मानम्.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      सचे (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        पटिनिवत्त Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      प्रतिज्ञा(argv && "null check");
      मानम् = StringRef(argv[++i]);
    }
    भङ्ग;
  पक्ष मानम्_Disallowed:
    सचे (मानम्.data())
      पटिनिवत्त Handler->error("does not allow a value! '" + Twine(मानम्) +
                            "' specified.");
    भङ्ग;
  पक्ष मानम्_Optional:
    भङ्ग;
  }

  पटिनिवत्त CommaSeparateAndAddOccurrence(Handler, i, ArgName, मानम्);
}

तार्किक llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, पूर्णाङ्क i) {
  पूर्णाङ्क Dummy = i;
  पटिनिवत्त ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
स्थिर Option *findPrefixOption(StringRef Name, size_t &Length,
                                ध्रुव OptionsMapTy &OptionsMap) {
  अत्थाय (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    सचे (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      पटिनिवत्त O;
    }
  }
  पटिनिवत्त nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
स्थिर Option *HandlePrefixedOption(StringRef &Arg, StringRef &मानम्,
                                    ध्रुव OptionsMapTy &OptionsMap) {
  सचे (Arg.size() == 1)
    पटिनिवत्त nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  सचे (!POpt)
    पटिनिवत्त nullptr;

  StringRef Maybe_मानम् =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  सचे (POpt->getFormattingFlag() == cl::Prefix && Maybe_मानम्.starts_with("="))
    Maybe_मानम् = Maybe_मानम्.drop_front();
  मानम् = Maybe_मानम्;
  पटिनिवत्त POpt;
}

स्थिर तार्किक Requires_मानम्(ध्रुव Option *O) {
  पटिनिवत्त O->get_अङ्कः_Occurrences_Flag() == cl::Required ||
         O->get_अङ्कः_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक Eats_Unbounded_सङ्ख्या_Of_मानानि(ध्रुव Option *O) {
  पटिनिवत्त O->get_अङ्कः_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_अङ्कः_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक isWhitespace(वर्ण C) {
  पटिनिवत्त C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

स्थिर तार्किक isWhitespaceOrNull(वर्ण C) {
  पटिनिवत्त isWhitespace(C) || C == '\0';
}

स्थिर तार्किक isQuote(वर्ण C) { पटिनिवत्त C == '\"' || C == '\''; }

शून्य cl::Tokenize_GNUCommand_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                तार्किक MarkEOLs) {
  SmallString<128> Token;
  तार्किक InToken = मिच्छा;
  अत्थाय (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    सचे (!InToken) {
      याव (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        सचे (MarkEOLs && Src[I] == '\n')
          NewArgv.प्रक्षिप_back(nullptr);
        ++I;
      }
      सचे (I == E)
        भङ्ग;
      InToken = सच्च;
    }

    वर्ण C = Src[I];

    // Backslash escapes the next character.
    सचे (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.प्रक्षिप_back(Src[I]);
      अनुवर्त;
    }

    // Consume a quoted string.
    सचे (isQuote(C)) {
      ++I;
      याव (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        सचे (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.प्रक्षिप_back(Src[I]);
        ++I;
      }
      सचे (I == E)
        भङ्ग;
      अनुवर्त;
    }

    // End the token if this is whitespace.
    सचे (isWhitespace(C)) {
      NewArgv.प्रक्षिप_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      सचे (MarkEOLs && C == '\n')
        NewArgv.प्रक्षिप_back(nullptr);
      Token.clear();
      InToken = मिच्छा;
      अनुवर्त;
    }

    // This is a normal character.  Append it.
    Token.प्रक्षिप_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  सचे (InToken)
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
  करोहि {
    ++I;
    ++Backslash_गणनम्;
  } याव (I != E && Src[I] == '\\');

  तार्किक FollowedByDoubleQuote = (I != E && Src[I] == '"');
  सचे (FollowedByDoubleQuote) {
    Token.append(Backslash_गणनम् / 2, '\\');
    सचे (Backslash_गणनम् % 2 == 0)
      पटिनिवत्त I - 1;
    Token.प्रक्षिप_back('"');
    पटिनिवत्त I;
  }
  Token.append(Backslash_गणनम्, '\\');
  पटिनिवत्त I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
स्थिर तार्किक isWindowsSpecialChar(वर्ण C) {
  पटिनिवत्त isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
स्थिर तार्किक isWindowsSpecialCharInCommandName(वर्ण C) {
  पटिनिवत्त isWhitespaceOrNull(C) || C == '\"';
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

  अत्थाय (size_t I = 0, E = Src.size(); I < E; ++I) {
    विकल्प (State) {
    पक्ष INIT: {
      प्रतिज्ञा(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      याव (I < E && isWhitespaceOrNull(Src[I])) {
        सचे (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      सचे (I >= E)
        भङ्ग;
      size_t Start = I;
      सचे (CommandName) {
        याव (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } अञ्ञथा {
        याव (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      सचे (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        सचे (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अञ्ञथा {
          CommandName = मिच्छा;
        }
      } अञ्ञथा सचे (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } अञ्ञथा सचे (Src[I] == '\\') {
        प्रतिज्ञा(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अञ्ञथा {
        llvm_unreachable("unexpected special character");
      }
      भङ्ग;
    }

    पक्ष UNQUOTED:
      सचे (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        सचे (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अञ्ञथा {
          CommandName = मिच्छा;
        }
        State = INIT;
      } अञ्ञथा सचे (Src[I] == '\"') {
        State = QUOTED;
      } अञ्ञथा सचे (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अञ्ञथा {
        Token.प्रक्षिप_back(Src[I]);
      }
      भङ्ग;

    पक्ष QUOTED:
      सचे (Src[I] == '\"') {
        सचे (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.प्रक्षिप_back('"');
          ++I;
        } अञ्ञथा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अञ्ञथा सचे (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अञ्ञथा {
        Token.प्रक्षिप_back(Src[I]);
      }
      भङ्ग;
    }
  }

  सचे (State != INIT)
    AddToken(Saver.save(Token.str()));
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                    तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok.data()); };
  auto OnEOL = [&]() {
    सचे (MarkEOLs)
      NewArgv.प्रक्षिप_back(nullptr);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सच्च, OnEOL, मिच्छा);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/मिच्छा,
                                 OnEOL, मिच्छा);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                        तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.प्रक्षिप_back(Tok.data()); };
  auto OnEOL = [&]() {
    सचे (MarkEOLs)
      NewArgv.प्रक्षिप_back(nullptr);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सच्च, OnEOL, सच्च);
}

शून्य cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                            तार्किक MarkEOLs) {
  अत्थाय (ध्रुव वर्ण *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> पङ्क्तिः;
    // Check for comment line.
    सचे (isWhitespace(*Cur)) {
      याव (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      अनुवर्त;
    }
    सचे (*Cur == '#') {
      याव (Cur != Source.end() && *Cur != '\n')
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
    अत्थाय (ध्रुव वर्ण *End = Source.end(); Cur != End; ++Cur) {
      वर्ण C = *Cur;
      सचे (C == '\\') {
        सचे (Cur + 1 != End) {
          ++Cur;
          सचे (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            पङ्क्तिः.append(Start, Cur - 1);
            सचे (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            अनुवर्त;
          }
        }
        AtTokenStart = मिच्छा;
        अनुवर्त;
      }
      सचे (C == '\n')
        भङ्ग;
      सचे (Quote) {
        सचे (C == Quote)
          Quote = 0;
      } अञ्ञथा सचे (isQuote(C)) {
        Quote = C;
      } अञ्ञथा सचे (C == '#' && AtTokenStart) {
        पङ्क्तिः_End = Cur;
        याव (Cur != End && *Cur != '\n')
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
  पटिनिवत्त (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
स्थिर शून्य ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ध्रुव वर्ण *&Arg) {
  प्रतिज्ञा(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ध्रुव StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  अत्थाय (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ध्रुव StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    सचे (ResponseFile.empty())
      ResponseFile = LHS;
    अञ्ञथा
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  सचे (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ध्रुव StringRef Remaining = ArgString.substr(StartPos);
    सचे (!Remaining.empty())
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
  सचे (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    पटिनिवत्त llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<वर्ण> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  सचे (hasUTF16ByteOrderMark(BufRef)) {
    सचे (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      पटिनिवत्त llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अञ्ञथा सचे (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  सचे (!RelativeNames && !InConfigFile)
    पटिनिवत्त Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  अत्थाय (ध्रुव वर्ण *&Arg : NewArgv) {
    सचे (!Arg)
      अनुवर्त;

    // Substitute <CFGDIR> with the file's base path.
    सचे (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    तार्किक ConfigInclusion = मिच्छा;
    सचे (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      सचे (!llvm::sys::path::is_relative(FileName))
        अनुवर्त;
    } अञ्ञथा सचे (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = सच्च;
    } अञ्ञथा {
      अनुवर्त;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.प्रक्षिप_back('@');
    सचे (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      सचे (!findConfigFile(FileName, FilePath))
        पटिनिवत्त createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } अञ्ञथा {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  पटिनिवत्त Error::success();
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
  अत्थाय (unsigned I = 0; I != Argv.size();) {
    याव (I == File_राशिः.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_राशिः.निष्कासय_back();
    }

    ध्रुव वर्ण *Arg = Argv[I];
    // Check if it is an EOL marker
    सचे (Arg == nullptr) {
      ++I;
      अनुवर्त;
    }

    सचे (Arg[0] != '@') {
      ++I;
      अनुवर्त;
    }

    ध्रुव वर्ण *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    सचे (llvm::sys::path::is_relative(FName)) {
      सचे (CurrentDir.empty()) {
        सचे (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अञ्ञथा {
          पटिनिवत्त createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अञ्ञथा {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    सचे (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      सचे (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        सचे (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          अनुवर्त;
        }
      }
      सचे (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      पटिनिवत्त createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ध्रुव llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, एतत्](ध्रुव ResponseFileRecord &RFile) -> ErrorOr<तार्किक> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      सचे (!RHS)
        पटिनिवत्त RHS.getError();
      पटिनिवत्त FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    अत्थाय (ध्रुव auto &F : drop_begin(File_राशिः)) {
      सचे (ErrorOr<तार्किक> R = IsEquivalent(F)) {
        सचे (R.get())
          पटिनिवत्त createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अञ्ञथा {
        पटिनिवत्त createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ध्रुव वर्ण *, 0> ExpandedArgv;
    सचे (Error Err = expandResponseFile(FName, ExpandedArgv))
      पटिनिवत्त Err;

    अत्थाय (ResponseFileRecord &Record : File_राशिः) {
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
  पटिनिवत्त Error::success();
}

तार्किक cl::expandResponseFiles(पूर्णाङ्क Argc, ध्रुव वर्ण *ध्रुव *Argv,
                             ध्रुव वर्ण *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ध्रुव वर्ण *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अञ्ञथा
  auto Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#endif
  // The environment variable specifies initial options.
  सचे (EnvVar)
    सचे (std::optional<std::string> Env_मानम् = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मानम्, Saver, NewArgv, /*MarkEOLs=*/मिच्छा);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  सचे (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

तार्किक cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  सचे (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

तार्किक ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<वर्ण> &FilePath) {
  SmallString<128> CfgFilePath;
  ध्रुव auto FileExists = [एतत्](SmallString<128> Path) -> तार्किक {
    auto Status = FS->status(Path);
    पटिनिवत्त Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  सचे (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    सचे (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      पटिनिवत्त मिच्छा;
    सचे (!FileExists(CfgFilePath))
      पटिनिवत्त मिच्छा;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    पटिनिवत्त सच्च;
  }

  // Look for the file in search directories.
  अत्थाय (ध्रुव StringRef &Dir : अन्वेषणम्_Dirs) {
    सचे (Dir.empty())
      अनुवर्त;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    सचे (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      पटिनिवत्त सच्च;
    }
  }

  पटिनिवत्त मिच्छा;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  SmallString<128> AbsPath;
  सचे (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    सचे (std::error_code EC = FS->makeAbsolute(AbsPath))
      पटिनिवत्त make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = सच्च;
  RelativeNames = सच्च;
  सचे (Error Err = expandResponseFile(CfgFile, Argv))
    पटिनिवत्त Err;
  पटिनिवत्त expandResponseFiles(Argv);
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
  सचे (EnvVar) {
    सचे (std::optional<std::string> Env_मानम् =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_पङ्क्तिः(*Env_मानम्, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  पूर्णाङ्क NewArgc = static_cast<पूर्णाङ्क>(NewArgv.size());

  // Parse all options.
  पटिनिवत्त globalParser().Parse_Command_पङ्क्तिः_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

तार्किक cl::Parse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  पटिनिवत्त parse_Command_पङ्क्तिः_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

तार्किक cl::parse_Command_पङ्क्तिः_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ध्रुव वर्ण *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  पटिनिवत्त parse_Command_पङ्क्तिः_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
शून्य Command_पङ्क्तिः_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  अत्थाय (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    अत्थाय (auto &O : SC->OptionsMap)
      Opts.प्रक्षिप_back(O.second);
    अत्थाय (Option *O : Opts)
      O->reset();
    अत्थाय (Option *O : SC->PositionalOpts)
      O->reset();
    सचे (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  अत्थाय (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

तार्किक Command_पङ्क्तिः_Parser::Parse_Command_पङ्क्तिः_Options(
    पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  प्रतिज्ञा(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  तार्किक IgnoreErrors = Errs;
  सचे (!Errs)
    Errs = &errs();
  सचे (!VFS)
    VFS = vfs::getRealFileSystem().get();
  तार्किक ErrorParsing = मिच्छा;

  // Expand response files.
  SmallVector<ध्रुव वर्ण *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अञ्ञथा
  auto Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  सचे (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    पटिनिवत्त मिच्छा;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णाङ्क>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned अङ्कः_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  तार्किक HasUnlimitedPositionals = मिच्छा;

  पूर्णाङ्क FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शिखरम्_Level();
  std::string NearestSubCommandString;
  तार्किक MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  सचे (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    सचे (ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  प्रतिज्ञा(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  सचे (ConsumeAfterOpt) {
    प्रतिज्ञा(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  सचे (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    तार्किक UnboundedFound = मिच्छा;
    अत्थाय (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      सचे (Requires_मानम्(Opt))
        ++अङ्कः_Positional_Required;
      अञ्ञथा सचे (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        सचे (PositionalOpts.size() > 1) {
          सचे (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = सच्च;
        }
      } अञ्ञथा सचे (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        सचे (!IgnoreErrors)
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
  तार्किक DashDashFound = मिच्छा; // Have we read '--'?
  अत्थाय (पूर्णाङ्क i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef मानम्;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    सचे (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      सचे (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }

      सचे (!PositionalOpts.empty()) {
        PositionalVals.प्रक्षिप_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        सचे (PositionalVals.size() >= अङ्कः_Positional_Required && ConsumeAfterOpt) {
          अत्थाय (++i; i < argc; ++i)
            PositionalVals.प्रक्षिप_back(std::make_pair(StringRef(argv[i]), i));
          भङ्ग; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        अनुवर्त;
      }
    } अञ्ञथा सचे (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = सच्च; // This is the mythical "--"?
      अनुवर्त;             // Don't try to process it as an argument itself.
    } अञ्ञथा सचे (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);
      सचे (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }
    } अञ्ञथा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      सचे (!Handler && ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
        Handler = LookupOption(SubCommand::get_शिखरम्_Level(), ArgName, मानम्);

      सचे (!Handler) {
        सचे (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ध्रुव वर्ण *> Rest(argv + i, argc - i);
          unsigned N;
          सचे (Error E = Context && L->isPerContext()
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
      सचे (!Handler)
        Handler = HandlePrefixedOption(ArgName, मानम्, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      सचे (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    सचे (!Handler) {
      auto ReportUnknownArgument = [&](तार्किक IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        सचे (NearestArgumentName.empty())
          पटिनिवत्त;

        *Errs << ProgramName << ": Did you mean '";
        सचे (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        अञ्ञथा
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      सचे (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/सच्च, NearestHandlerString);
      अञ्ञथा
        ReportUnknownArgument(/*IsArg=*/मिच्छा, NearestSubCommandString);

      ErrorParsing = सच्च;
      अनुवर्त;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    सचे (Handler->getFormattingFlag() == cl::Positional) {
      सचे ((Handler->getMiscFlags() & PositionalEatsArgs) && !मानम्.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = सच्च;
      }
      ActivePositionalArg = Handler;
    }
    अञ्ञथा
      ErrorParsing |= ProvideOption(Handler, ArgName, मानम्, argc, argv, i);
  }

  // Check and handle positional arguments now...
  सचे (अङ्कः_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अङ्कः_Positional_Required
             << " positional argument" << (अङ्कः_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = सच्च;
  } अञ्ञथा सचे (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = सच्च;

  } अञ्ञथा सचे (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, अङ्कः_Vals = static_cast<unsigned>(PositionalVals.size());
    अत्थाय (Option *Opt : PositionalOpts) {
      सचे (Requires_मानम्(Opt)) {
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
      याव (अङ्कः_Vals - ValNo > अङ्कः_Positional_Required && !Done) {
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
  } अञ्ञथा {
    प्रतिज्ञा(ConsumeAfterOpt && अङ्कः_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    अत्थाय (Option *Opt : PositionalOpts)
      सचे (Requires_मानम्(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    सचे (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    अत्थाय (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  अत्थाय (ध्रुव auto &Opt : OptionsMap) {
    विकल्प (Opt.second->get_अङ्कः_Occurrences_Flag()) {
    पक्ष Required:
    पक्ष OneOrMore:
      सचे (Opt.second->get_अङ्कः_Occurrences() == 0) {
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
             अत्थाय (पूर्णाङ्क i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  सचे (ErrorParsing) {
    सचे (!IgnoreErrors)
      exit(1);
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

तार्किक Option::error(ध्रुव Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  सचे (!ArgName.data())
    ArgName = ArgStr;
  सचे (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  अञ्ञथा
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  पटिनिवत्त सच्च;
}

तार्किक Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef मानम्) {
  ++अङ्कः_Occurrences;
  पटिनिवत्त handleOccurrence(pos, ArgName, मानम्);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
स्थिर StringRef get_मानम्_Str(ध्रुव Option &O, StringRef Default_सन्देशः) {
  सचे (O.मानम्_Str.empty())
    पटिनिवत्त Default_सन्देशः;
  पटिनिवत्त O.मानम्_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ध्रुव {
  पटिनिवत्त argPlusPrefixesSize(ArgStr);
}

शून्य Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_पङ्क्तिः_Indented_By) {
  प्रतिज्ञा(Indent >= First_पङ्क्तिः_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_पङ्क्तिः_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  याव (!Split.second.empty()) {
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
  याव (!Split.second.empty()) {
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
  सचे (!ValName.empty()) {
    size_t FormattingLen = 3;
    सचे (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_मानम्_Str(O, ValName).size() + FormattingLen;
  }

  पटिनिवत्त Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य basic_parser_impl::printOptionInfo(ध्रुव Option &O,
                                        size_t GlobalWidth) ध्रुव {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_मानम्_Name();
  सचे (!ValName.empty()) {
    सचे (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_मानम्_Str(O, ValName) << ">...";
    } अञ्ञथा सचे (O.get_मानम्_Expected_Flag() == मानम्_Optional)
      outs() << "[=<" << get_मानम्_Str(O, ValName) << ">]";
    अञ्ञथा {
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
  पटिनिवत्त parseBool<तार्किक, सच्च, मिच्छा>(O, ArgName, Arg, मानम्);
}

// parser<boolOrDefault> implementation
//
तार्किक parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मानम्) {
  पटिनिवत्त parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
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
  सचे (!Arg.getAsInteger(0, Min_मानम्)) {
    मानम् = FixedOrScalableQuantityT::getFixed(Min_मानम्);
    पटिनिवत्त मिच्छा;
  }

  StringRef Remainder = Arg;
  सचे (!Remainder.consume_front("vscale"))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  सचे (!Remainder.consume_front('x'))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  सचे (Remainder.getAsInteger(0, Min_मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for " + मानम्_Kind +
                   " argument!");

  मानम् = FixedOrScalableQuantityT::getScalable(Min_मानम्);
  पटिनिवत्त मिच्छा;
}

// parser<int> implementation
//
तार्किक parser<पूर्णाङ्क>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णाङ्क &मानम्) {
  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for integer argument!");
  पटिनिवत्त मिच्छा;
}

// parser<long> implementation
//
तार्किक parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &मानम्) {
  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for long argument!");
  पटिनिवत्त मिच्छा;
}

// parser<long long> implementation
//
तार्किक parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &मानम्) {
  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for llong argument!");
  पटिनिवत्त मिच्छा;
}

// parser<unsigned> implementation
//
तार्किक parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &मानम्) {

  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for uint argument!");
  पटिनिवत्त मिच्छा;
}

// parser<unsigned long> implementation
//
तार्किक parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &मानम्) {

  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for ulong argument!");
  पटिनिवत्त मिच्छा;
}

// parser<unsigned long long> implementation
//
तार्किक parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &मानम्) {

  सचे (Arg.getAsInteger(0, मानम्))
    पटिनिवत्त O.error("'" + Arg + "' value invalid for ullong argument!");
  पटिनिवत्त मिच्छा;
}

// parser<ElementCount> implementation
//
तार्किक parser<Element_गणनम्>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_गणनम् &मानम्) {
  पटिनिवत्त parseFixedOrScalableQuantity(O, Arg, get_मानम्_Name(), मानम्);
}

// parser<double>/parser<float> implementation
//
स्थिर तार्किक parseDouble(Option &O, StringRef Arg, द्विगुण &मानम्) {
  सचे (to_float(Arg, मानम्))
    पटिनिवत्त मिच्छा;
  पटिनिवत्त O.error("'" + Arg + "' value invalid for floating point argument!");
}

तार्किक parser<द्विगुण>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुण &Val) {
  पटिनिवत्त parseDouble(O, Arg, Val);
}

तार्किक parser<भिन्न>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          भिन्न &Val) {
  द्विगुण dVal;
  सचे (parseDouble(O, Arg, dVal))
    पटिनिवत्त सच्च;
  Val = (भिन्न)dVal;
  पटिनिवत्त मिच्छा;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_अङ्कः_Options();

  अत्थाय (unsigned i = 0; i != e; ++i) {
    सचे (getOption(i) == Name)
      पटिनिवत्त i;
  }
  पटिनिवत्त e;
}

स्थिर StringRef Eq_मानम् = "=<value>";
स्थिर StringRef EmptyOption = "<empty>";
स्थिर StringRef OptionPrefix = "    =";
स्थिर size_t getOptionPrefixesSize() {
  पटिनिवत्त OptionPrefix.size() + ArgHelpPrefix.size();
}

स्थिर तार्किक shouldPrintOption(StringRef Name, StringRef Description,
                              ध्रुव Option &O) {
  पटिनिवत्त O.get_मानम्_Expected_Flag() != मानम्_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ध्रुव Option &O) ध्रुव {
  सचे (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_मानम्.size();
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      सचे (!shouldPrintOption(Name, getDescription(i), O))
        अनुवर्त;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    पटिनिवत्त Size;
  } अञ्ञथा {
    size_t BaseSize = 0;
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    पटिनिवत्त BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य generic_parser_base::printOptionInfo(ध्रुव Option &O,
                                          size_t GlobalWidth) ध्रुव {
  सचे (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    सचे (O.get_मानम्_Expected_Flag() == मानम्_Optional) {
      अत्थाय (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
        सचे (getOption(i).empty()) {
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
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      सचे (!shouldPrintOption(OptionName, Description, O))
        अनुवर्त;
      size_t First_पङ्क्तिः_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      सचे (OptionName.empty()) {
        outs() << EmptyOption;
        प्रतिज्ञा(First_पङ्क्तिः_Indent >= EmptyOption.size());
        First_पङ्क्तिः_Indent += EmptyOption.size();
      }
      सचे (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_पङ्क्तिः_Indent);
      अञ्ञथा
        outs() << '\n';
    }
  } अञ्ञथा {
    सचे (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
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
  अत्थाय (unsigned i = 0; i != अङ्कः_Opts; ++i) {
    सचे (!मानम्.compare(get_Option_मानम्(i)))
      अनुवर्त;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t अङ्कः_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(अङ्कः_Spaces) << " (default: ";
    अत्थाय (unsigned j = 0; j != अङ्कः_Opts; ++j) {
      सचे (!Default.compare(get_Option_मानम्(j)))
        अनुवर्त;
      outs() << getOption(j);
      भङ्ग;
    }
    outs() << ")\n";
    पटिनिवत्त;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
नामाकाश llvm {
नामाकाश cl {
स्थिर raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  पटिनिवत्त OS << static_cast<पूर्णाङ्क>(V);
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
    सचे (D.has_मानम्())                                                          \
      outs() << D.get_मानम्();                                                  \
    अञ्ञथा                                                                       \
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
  सचे (D.has_मानम्())
    outs() << D.get_मानम्();
  अञ्ञथा
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
  सचे (D.has_मानम्() && D.get_मानम्().has_मानम्_2())
    outs() << D.get_मानम्();
  अञ्ञथा
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
  पटिनिवत्त strcmp(LHS->first, RHS->first);
}

स्थिर पूर्णाङ्क SubNameCompare(ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *LHS,
                          ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *RHS) {
  पटिनिवत्त strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
स्थिर शून्य sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ध्रुव वर्ण *, Option *>> &Opts,
                     तार्किक ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  अत्थाय (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    सचे (I->second->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    // Unless showhidden is set, ignore hidden flags.
    सचे (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      अनुवर्त;

    // If we've already seen this option, don't add it to the list again.
    सचे (!OptionSet.निवेशय(I->second).second)
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
  अत्थाय (auto *S : SubMap) {
    सचे (S->getName().empty())
      अनुवर्त;
    Subs.प्रक्षिप_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

नामाकाश {

वग्ग HelpPrinter {
रक्षित:
  ध्रुव तार्किक ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ध्रुव वर्ण *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ध्रुव वर्ण *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  आभास शून्य printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    अत्थाय (ध्रुव auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  शून्य printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    अत्थाय (ध्रुव auto &S : Subs) {
      outs() << "  " << S.first;
      सचे (!S.second->getDescription().empty()) {
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
    सचे (!मानम्)
      पटिनिवत्त;
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

    सचे (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    सचे (Sub == &SubCommand::get_शिखरम्_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      सचे (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } अञ्ञथा {
      सचे (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    अत्थाय (auto *Opt : PositionalOpts) {
      सचे (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    सचे (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    सचे (Sub == &SubCommand::get_शिखरम्_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      अत्थाय (ध्रुव auto &Sub : Subs)
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
    अत्थाय (ध्रुव auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    सचे (ShowHidden)
      अत्थाय (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              सचे (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    अत्थाय (ध्रुव auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    सचे (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    अत्थाय (ध्रुव auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    अत्थाय (ध्रुव auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

वग्ग CategorizedHelpPrinter : सार्वजनिक HelpPrinter {
सार्वजनिक:
  explicit CategorizedHelpPrinter(तार्किक showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  स्थिर पूर्णाङ्क OptionCategoryCompare(OptionCategory *ध्रुव *A,
                                   OptionCategory *ध्रुव *B) {
    पटिनिवत्त (*A)->getName().compare((*B)->getName());
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
    अत्थाय (ध्रुव auto &I : Opts) {
      Option *Opt = I.second;
      अत्थाय (OptionCategory *Cat : Opt->Categories) {
        प्रतिज्ञा(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].प्रक्षिप_back(Opt);
      }
    }

    // Now do printing.
    अत्थाय (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ध्रुव auto &CategoryOptions = CategorizedOptions[Category];
      सचे (CategoryOptions.empty())
        अनुवर्त;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      सचे (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      अञ्ञथा
        outs() << "\n";

      // Loop over the options in the category and print.
      अत्थाय (ध्रुव Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
वग्ग HelpPrinterWrapper {
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

#सचे defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# सचे defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अञ्ञथा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#नोचेद्यदि defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# सचे defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# अञ्ञथा
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#अञ्ञथा
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

नामाकाश {
वग्ग VersionPrinter {
सार्वजनिक:
  शून्य print(ध्रुव std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अञ्ञथा
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#सचे LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अञ्ञथा
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    सचे (!ExtraPrinters.empty()) {
      अत्थाय (ध्रुव auto &I : ExtraPrinters)
        I(outs());
    }
  }
  शून्य operator=(तार्किक OptionWasSpecified);
};

संरचना Command_पङ्क्तिः_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{मिच्छा};
  HelpPrinter UncategorizedHiddenPrinter{सच्च};
  CategorizedHelpPrinter CategorizedNormalPrinter{मिच्छा};
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
      cl::init(मिच्छा),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(मिच्छा),
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
  पटिनिवत्त GeneralCategory;
}

शून्य VersionPrinter::operator=(तार्किक OptionWasSpecified) {
  सचे (!OptionWasSpecified)
    पटिनिवत्त;

  सचे (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

शून्य HelpPrinterWrapper::operator=(तार्किक मानम्) {
  सचे (!मानम्)
    पटिनिवत्त;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  सचे (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = सच्च; // Invoke categorized printer
  } अञ्ञथा {
    UncategorizedPrinter = सच्च; // Invoke uncategorized printer
  }
}

// Print the value of each option.
शून्य cl::Print_Option_मानानि() { globalParser().print_Option_मानानि(); }

शून्य Command_पङ्क्तिः_Parser::print_Option_मानानि() {
  सचे (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    पटिनिवत्त;

  SmallVector<std::pair<ध्रुव वर्ण *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ सच्च);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  अत्थाय (ध्रुव auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  अत्थाय (ध्रुव auto &Opt : Opts)
    Opt.second->print_Option_मानम्(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
शून्य cl::PrintHelpMessage(तार्किक Hidden, तार्किक Categorized) {
  सचे (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  अञ्ञथा सचे (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  अञ्ञथा सचे (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अञ्ञथा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  स्थिर ध्रुव StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#सचे LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#सचे __has_feature(address_sanitizer)
      "+asan",
#endif
#सचे __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#सचे __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#सचे __has_feature(memory_sanitizer)
      "+msan",
#endif
#सचे __has_feature(thread_sanitizer)
      "+tsan",
#endif
#सचे __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  पटिनिवत्त ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
शून्य cl::printBuildConfig(raw_ostream &OS) {
#सचे LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
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
  पटिनिवत्त Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  पटिनिवत्त globalParser().getRegisteredSubcommands();
}

शून्य cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  अत्थाय (auto &I : Sub.OptionsMap) {
    तार्किक Unrelated = सच्च;
    अत्थाय (auto &Cat : I.second->Categories) {
      सचे (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = मिच्छा;
    }
    सचे (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::HideUnrelatedOptions(ArrayRef<ध्रुव cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  अत्थाय (auto &I : Sub.OptionsMap) {
    तार्किक Unrelated = सच्च;
    अत्थाय (auto &Cat : I.second->Categories) {
      सचे (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = मिच्छा;
    }
    सचे (Unrelated)
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
  अत्थाय (size_t I = 0, E = Args.size(); I != E;) {
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
  सचे (Inserted)
    It->second = L.copyDefault(Alloc);
  पटिनिवत्त parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

शून्य cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

शून्य LLVMParse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 ध्रुव वर्ण *Overview) {
  llvm::cl::Parse_Command_पङ्क्तिः_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
