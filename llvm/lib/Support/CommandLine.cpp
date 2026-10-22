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

#അന്തര്ഭാവ "llvm/Support/CommandLine.h"

#അന്തര്ഭാവ "DebugOptions.h"

#അന്തര്ഭാവ "llvm-c/Support.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/STLFunctionalExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallPtrSet.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/LibraryOptionsContext.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/StringSaver.h"
#അന്തര്ഭാവ "llvm/Support/TypeSize.h"
#അന്തര്ഭാവ "llvm/Support/VirtualFileSystem.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cstdlib>
#അന്തര്ഭാവ <optional>
#അന്തര്ഭാവ <string>
using നാമാകാശ llvm;
using നാമാകാശ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
നാമാകാശ llvm {
നാമാകാശ cl {
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<തർക്കം>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<പൂർണ്ണസംഖ്യ>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<long>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<ഇരട്ട>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<ദശാംശം>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<അക്ഷരം>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE basic_parser<Element_എണ്ണം>;

#എങ്കിൽ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template വർഗ്ഗം LLVM_EXPORT_TEMPLATE opt<തർക്കം>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE opt<അക്ഷരം>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE opt<പൂർണ്ണസംഖ്യ>;
template വർഗ്ഗം LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
ശൂന്യം Generic_Option_മൂല്യം::anchor() {}
ശൂന്യം Option_മൂല്യം<boolOrDefault>::anchor() {}
ശൂന്യം Option_മൂല്യം<std::string>::anchor() {}
ശൂന്യം Option::anchor() {}
ശൂന്യം basic_parser_impl::anchor() {}
ശൂന്യം parser<തർക്കം>::anchor() {}
ശൂന്യം parser<boolOrDefault>::anchor() {}
ശൂന്യം parser<പൂർണ്ണസംഖ്യ>::anchor() {}
ശൂന്യം parser<long>::anchor() {}
ശൂന്യം parser<long long>::anchor() {}
ശൂന്യം parser<unsigned>::anchor() {}
ശൂന്യം parser<unsigned long>::anchor() {}
ശൂന്യം parser<unsigned long long>::anchor() {}
ശൂന്യം parser<ഇരട്ട>::anchor() {}
ശൂന്യം parser<ദശാംശം>::anchor() {}
ശൂന്യം parser<std::string>::anchor() {}
ശൂന്യം parser<std::optional<std::string>>::anchor() {}
ശൂന്യം parser<അക്ഷരം>::anchor() {}
ശൂന്യം parser<Element_എണ്ണം>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
ശൂന്യം opt_bool_anchor() { opt<തർക്കം> anchor{""}; }
ശൂന്യം opt_char_anchor() { opt<അക്ഷരം> anchor{""}; }
ശൂന്യം opt_int_anchor() { opt<പൂർണ്ണസംഖ്യ> anchor{""}; }
ശൂന്യം opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

സ്ഥിരം സ്ഥിതം size_t DefaultPad = 2;

സ്ഥിതം StringRef ArgPrefix = "-";
സ്ഥിതം StringRef ArgPrefixLong = "--";
സ്ഥിതം StringRef ArgHelpPrefix = " - ";

സ്ഥിതം size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  എങ്കിൽ (Len == 1)
    തിരികെ Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  തിരികെ Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

സ്ഥിതം SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  വേണ്ടി (size_t I = 0; I < Pad; ++I) {
    Prefix.തള്ളുക_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  തിരികെ Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

നാമാകാശ {

വർഗ്ഗം PrintArg {
  StringRef ArgName;
  size_t Pad;
പൊതു:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, സ്ഥിരം PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, സ്ഥിരം PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  തിരികെ OS;
}

വർഗ്ഗം Command_വരി_Parser {
പൊതു:
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
  size_t അക്കം_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_വരി_Parser() { registerSubCommand(&SubCommand::get_മുകൾ_Level()); }

  ശൂന്യം ResetAllOptionOccurrences();

  തർക്കം Parse_Command_വരി_Options(പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  ശൂന്യം forEachSubCommand(Option &Opt, function_ref<ശൂന്യം(SubCommand &)> Action) {
    എങ്കിൽ (Opt.Subs.empty()) {
      Action(SubCommand::get_മുകൾ_Level());
      തിരികെ;
    }
    എങ്കിൽ (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      വേണ്ടി (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      തിരികെ;
    }
    വേണ്ടി (auto *SC : Opt.Subs) {
      പ്രതിജ്ഞാ(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  ശൂന്യം addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    എങ്കിൽ (Opt.hasArgStr())
      തിരികെ;
    എങ്കിൽ (!SC->OptionsMap.തിരുകുക(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  ശൂന്യം addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  ശൂന്യം addOption(Option *O, SubCommand *SC) {
    തർക്കം HadErrors = അസത്യം;
    എങ്കിൽ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      എങ്കിൽ (!SC->OptionsMap.തിരുകുക(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = സത്യം;
      }
    }

    // Remember information about positional options.
    എങ്കിൽ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.തള്ളുക_back(O);
    അല്ലെങ്കിൽ എങ്കിൽ (O->get_അക്കം_Occurrences_Flag() == cl::ConsumeAfter) {
      എങ്കിൽ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = സത്യം;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    എങ്കിൽ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  ശൂന്യം addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  ശൂന്യം removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    എങ്കിൽ (O->hasArgStr())
      OptionNames.തള്ളുക_back(O->ArgStr);

    SubCommand &Sub = *SC;
    വേണ്ടി (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      എങ്കിൽ (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    എങ്കിൽ (O->getFormattingFlag() == cl::Positional)
      വേണ്ടി (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        എങ്കിൽ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          നിർത്തുക;
        }
      }
    അല്ലെങ്കിൽ എങ്കിൽ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  ശൂന്യം removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  തർക്കം hasOptions(സ്ഥിരം SubCommand &Sub) സ്ഥിരം {
    തിരികെ (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  തർക്കം hasOptions() സ്ഥിരം {
    വേണ്ടി (സ്ഥിരം auto *S : RegisteredSubCommands) {
      എങ്കിൽ (hasOptions(*S))
        തിരികെ സത്യം;
    }
    തിരികെ !Libraries.empty();
  }

  തർക്കം hasNamedSubCommands() സ്ഥിരം {
    വേണ്ടി (സ്ഥിരം auto *S : RegisteredSubCommands)
      എങ്കിൽ (!S->getName().empty())
        തിരികെ സത്യം;
    തിരികെ അസത്യം;
  }

  SubCommand *getActiveSubCommand() { തിരികെ ActiveSubCommand; }

  ശൂന്യം updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    എങ്കിൽ (!Sub.OptionsMap.തിരുകുക(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  ശൂന്യം updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  ശൂന്യം print_Option_മൂല്യങ്ങൾ();

  ശൂന്യം indexLibraryOptions() {
    തർക്കം HadErrors = അസത്യം;
    വേണ്ടി (; അക്കം_Indexed_Libraries != Libraries.size(); ++അക്കം_Indexed_Libraries) {
      LibraryOptions *L = Libraries[അക്കം_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        എങ്കിൽ (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 തിരികെ SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          തിരികെ;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = സത്യം;
      });
    }
    എങ്കിൽ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    തിരികെ LibraryIndex.lookup(Name);
  }

  ശൂന്യം registerCategory(OptionCategory *cat) {
    പ്രതിജ്ഞാ(എണ്ണം_if(RegisteredOptionCategories,
                    [cat](സ്ഥിരം OptionCategory *Category) {
             തിരികെ cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.തിരുകുക(cat);
  }

  ശൂന്യം registerSubCommand(SubCommand *sub) {
    പ്രതിജ്ഞാ(എണ്ണം_if(RegisteredSubCommands,
                    [sub](സ്ഥിരം SubCommand *Sub) {
                      തിരികെ (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.തിരുകുക(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    പ്രതിജ്ഞാ(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    വേണ്ടി (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      എങ്കിൽ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      അല്ലെങ്കിൽ
        addLiteralOption(*O, sub, E.first);
    }
  }

  ശൂന്യം unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    തിരികെ make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  ശൂന്യം reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    അക്കം_Indexed_Libraries = 0;

    SubCommand::get_മുകൾ_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_മുകൾ_Level());
  }

സ്വകാര്യ:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &മൂല്യം);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
സ്ഥിതം Command_വരി_Parser &globalParser() {
  സ്ഥിതം ManagedStatic<Command_വരി_Parser> GlobalParser;
  തിരികെ *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
സ്ഥിതം Error
parseLibraryOption(LibraryOptions &L, ശൂന്യം *Opts, ArrayRef<സ്ഥിരം അക്ഷരം *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<സ്ഥിരം അക്ഷരം *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  സ്ഥിരം അക്ഷരം *Saved[2];
  unsigned അക്കം_Args = std::min<size_t>(Args.size(), 2);
  വേണ്ടി (unsigned J = 0; J != അക്കം_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  എങ്കിൽ (Error E = L.parse(Opts, ArrayRef(Saved, അക്കം_Args), Consumed, Alloc))
    തിരികെ E;
  എങ്കിൽ (Record)
    Record->append(Saved, Saved + Consumed);
  തിരികെ Error::success();
}

template <typename T, T TrueVal, T FalseVal>
സ്ഥിതം തർക്കം parseBool(Option &O, StringRef ArgName, StringRef Arg, T &മൂല്യം) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  എങ്കിൽ (!Arg.data() || Arg == "true" || Arg == "1") {
    മൂല്യം = TrueVal;
    തിരികെ അസത്യം;
  }

  എങ്കിൽ (Arg == "false" || Arg == "0") {
    മൂല്യം = FalseVal;
    തിരികെ അസത്യം;
  }
  തിരികെ O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

ശൂന്യം cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.തള്ളുക_back(Help);
}

Option::Option(അക്കം_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : അക്കം_Occurrences(0), Occurrences(OccurrencesFlag), മൂല്യം(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(അസത്യം), Position(0) {
  Categories.തള്ളുക_back(&getGeneralCategory());
}

ശൂന്യം Option::addArgument() {
  globalParser().addOption(ഇത്);
  FullyInitialized = സത്യം;
}

ശൂന്യം Option::removeArgument() { globalParser().removeOption(ഇത്); }

ശൂന്യം Option::setArgStr(StringRef S) {
  എങ്കിൽ (FullyInitialized)
    globalParser().updateArgStr(ഇത്, S);
  പ്രതിജ്ഞാ(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

ശൂന്യം Option::addCategory(OptionCategory &C) {
  പ്രതിജ്ഞാ(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  എങ്കിൽ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  അല്ലെങ്കിൽ എങ്കിൽ (!is_contained(Categories, &C))
    Categories.തള്ളുക_back(&C);
}

ശൂന്യം Option::reset() {
  അക്കം_Occurrences = 0;
  setDefault();
}

ശൂന്യം OptionCategory::registerCategory() {
  globalParser().registerCategory(ഇത്);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_മുകൾ_Level() {
  സ്ഥിതം ManagedStatic<SubCommand> മുകൾ_Level_Sub_Command;
  തിരികെ *മുകൾ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  സ്ഥിതം ManagedStatic<SubCommand> AllSubCommands;
  തിരികെ *AllSubCommands;
}

ശൂന്യം SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(ഇത്);
}

ശൂന്യം SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ഇത്);
}

ശൂന്യം SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator തർക്കം() സ്ഥിരം {
  തിരികെ (globalParser().getActiveSubCommand() == ഇത്);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_വരി_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &മൂല്യം) {
  // Reject all dashes.
  എങ്കിൽ (Arg.empty())
    തിരികെ nullptr;
  പ്രതിജ്ഞാ(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  എങ്കിൽ (EqualPos == StringRef::npos) {
    // Look up the option.
    തിരികെ Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  എങ്കിൽ (I == Sub.OptionsMap.end())
    തിരികെ nullptr;

  auto *O = I->second;
  എങ്കിൽ (O->getFormattingFlag() == cl::AlwaysPrefix)
    തിരികെ nullptr;

  മൂല്യം = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  തിരികെ I->second;
}

SubCommand *Command_വരി_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  എങ്കിൽ (Name.empty())
    തിരികെ &SubCommand::get_മുകൾ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  വേണ്ടി (auto *S : RegisteredSubCommands) {
    പ്രതിജ്ഞാ(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    എങ്കിൽ (S->getName().empty())
      തുടരുക;

    എങ്കിൽ (S->getName() == Name)
      തിരികെ S;

    എങ്കിൽ (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  എങ്കിൽ (NearestMatch)
    NearestString = NearestMatch->getName();

  തിരികെ &SubCommand::get_മുകൾ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
സ്ഥിതം Option *LookupNearestOption(StringRef Arg,
                                   സ്ഥിരം OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  എങ്കിൽ (Arg.empty())
    തിരികെ nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  വേണ്ടി (സ്ഥിരം auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    എങ്കിൽ (O->getOptionHiddenFlag() == ReallyHidden)
      തുടരുക;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    എങ്കിൽ (O->hasArgStr())
      OptionNames.തള്ളുക_back(O->ArgStr);

    തർക്കം Permit_മൂല്യം = O->get_മൂല്യം_Expected_Flag() != cl::മൂല്യം_Disallowed;
    StringRef Flag = Permit_മൂല്യം ? LHS : Arg;
    വേണ്ടി (സ്ഥിരം auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/സത്യം, /*MaxEditDistance=*/BestDistance);
      എങ്കിൽ (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        എങ്കിൽ (RHS.empty() || !Permit_മൂല്യം)
          NearestString = std::string(Name);
        അല്ലെങ്കിൽ
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  തിരികെ Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
സ്ഥിതം തർക്കം CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef മൂല്യം) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  എങ്കിൽ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(മൂല്യം);
    StringRef::size_type Pos = Val.find(',');

    ആകുമ്പോൾ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      എങ്കിൽ (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        തിരികെ സത്യം;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    മൂല്യം = Val;
  }

  തിരികെ Handler->addOccurrence(pos, ArgName, മൂല്യം);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
സ്ഥിതം വരിയിൽ തർക്കം ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef മൂല്യം, പൂർണ്ണസംഖ്യ argc,
                                 സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, പൂർണ്ണസംഖ്യ &i) {
  // Enforce value requirements
  തിരഞ്ഞെടുപ്പ് (Handler->get_മൂല്യം_Expected_Flag()) {
  അവസ്ഥ മൂല്യം_Required:
    എങ്കിൽ (!മൂല്യം.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      എങ്കിൽ (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        തിരികെ Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      പ്രതിജ്ഞാ(argv && "null check");
      മൂല്യം = StringRef(argv[++i]);
    }
    നിർത്തുക;
  അവസ്ഥ മൂല്യം_Disallowed:
    എങ്കിൽ (മൂല്യം.data())
      തിരികെ Handler->error("does not allow a value! '" + Twine(മൂല്യം) +
                            "' specified.");
    നിർത്തുക;
  അവസ്ഥ മൂല്യം_Optional:
    നിർത്തുക;
  }

  തിരികെ CommaSeparateAndAddOccurrence(Handler, i, ArgName, മൂല്യം);
}

തർക്കം llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, പൂർണ്ണസംഖ്യ i) {
  പൂർണ്ണസംഖ്യ Dummy = i;
  തിരികെ ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
സ്ഥിതം Option *findPrefixOption(StringRef Name, size_t &Length,
                                സ്ഥിരം OptionsMapTy &OptionsMap) {
  വേണ്ടി (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    എങ്കിൽ (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      തിരികെ O;
    }
  }
  തിരികെ nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
സ്ഥിതം Option *HandlePrefixedOption(StringRef &Arg, StringRef &മൂല്യം,
                                    സ്ഥിരം OptionsMapTy &OptionsMap) {
  എങ്കിൽ (Arg.size() == 1)
    തിരികെ nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  എങ്കിൽ (!POpt)
    തിരികെ nullptr;

  StringRef Maybe_മൂല്യം =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  എങ്കിൽ (POpt->getFormattingFlag() == cl::Prefix && Maybe_മൂല്യം.starts_with("="))
    Maybe_മൂല്യം = Maybe_മൂല്യം.drop_front();
  മൂല്യം = Maybe_മൂല്യം;
  തിരികെ POpt;
}

സ്ഥിതം തർക്കം Requires_മൂല്യം(സ്ഥിരം Option *O) {
  തിരികെ O->get_അക്കം_Occurrences_Flag() == cl::Required ||
         O->get_അക്കം_Occurrences_Flag() == cl::OneOrMore;
}

സ്ഥിതം തർക്കം Eats_Unbounded_സംഖ്യ_Of_മൂല്യങ്ങൾ(സ്ഥിരം Option *O) {
  തിരികെ O->get_അക്കം_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_അക്കം_Occurrences_Flag() == cl::OneOrMore;
}

സ്ഥിതം തർക്കം isWhitespace(അക്ഷരം C) {
  തിരികെ C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

സ്ഥിതം തർക്കം isWhitespaceOrNull(അക്ഷരം C) {
  തിരികെ isWhitespace(C) || C == '\0';
}

സ്ഥിതം തർക്കം isQuote(അക്ഷരം C) { തിരികെ C == '\"' || C == '\''; }

ശൂന്യം cl::Tokenize_GNUCommand_വരി(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv,
                                തർക്കം MarkEOLs) {
  SmallString<128> Token;
  തർക്കം InToken = അസത്യം;
  വേണ്ടി (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    എങ്കിൽ (!InToken) {
      ആകുമ്പോൾ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        എങ്കിൽ (MarkEOLs && Src[I] == '\n')
          NewArgv.തള്ളുക_back(nullptr);
        ++I;
      }
      എങ്കിൽ (I == E)
        നിർത്തുക;
      InToken = സത്യം;
    }

    അക്ഷരം C = Src[I];

    // Backslash escapes the next character.
    എങ്കിൽ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.തള്ളുക_back(Src[I]);
      തുടരുക;
    }

    // Consume a quoted string.
    എങ്കിൽ (isQuote(C)) {
      ++I;
      ആകുമ്പോൾ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        എങ്കിൽ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.തള്ളുക_back(Src[I]);
        ++I;
      }
      എങ്കിൽ (I == E)
        നിർത്തുക;
      തുടരുക;
    }

    // End the token if this is whitespace.
    എങ്കിൽ (isWhitespace(C)) {
      NewArgv.തള്ളുക_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      എങ്കിൽ (MarkEOLs && C == '\n')
        NewArgv.തള്ളുക_back(nullptr);
      Token.clear();
      InToken = അസത്യം;
      തുടരുക;
    }

    // This is a normal character.  Append it.
    Token.തള്ളുക_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  എങ്കിൽ (InToken)
    NewArgv.തള്ളുക_back(Saver.save(Token.str()).data());
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
സ്ഥിതം size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  പൂർണ്ണസംഖ്യ Backslash_എണ്ണം = 0;
  // Skip the backslashes.
  ചെയ്യുക {
    ++I;
    ++Backslash_എണ്ണം;
  } ആകുമ്പോൾ (I != E && Src[I] == '\\');

  തർക്കം FollowedByDoubleQuote = (I != E && Src[I] == '"');
  എങ്കിൽ (FollowedByDoubleQuote) {
    Token.append(Backslash_എണ്ണം / 2, '\\');
    എങ്കിൽ (Backslash_എണ്ണം % 2 == 0)
      തിരികെ I - 1;
    Token.തള്ളുക_back('"');
    തിരികെ I;
  }
  Token.append(Backslash_എണ്ണം, '\\');
  തിരികെ I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
സ്ഥിതം തർക്കം isWindowsSpecialChar(അക്ഷരം C) {
  തിരികെ isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
സ്ഥിതം തർക്കം isWindowsSpecialCharInCommandName(അക്ഷരം C) {
  തിരികെ isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
സ്ഥിതം വരിയിൽ ശൂന്യം tokenize_Windows_Command_വരി_Impl(
    StringRef Src, StringSaver &Saver, function_ref<ശൂന്യം(StringRef)> AddToken,
    തർക്കം AlwaysCopy, function_ref<ശൂന്യം()> MarkEOL, തർക്കം InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  തർക്കം CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  എണ്ണൽ { INIT, UNQUOTED, QUOTED } State = INIT;

  വേണ്ടി (size_t I = 0, E = Src.size(); I < E; ++I) {
    തിരഞ്ഞെടുപ്പ് (State) {
    അവസ്ഥ INIT: {
      പ്രതിജ്ഞാ(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      ആകുമ്പോൾ (I < E && isWhitespaceOrNull(Src[I])) {
        എങ്കിൽ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      എങ്കിൽ (I >= E)
        നിർത്തുക;
      size_t Start = I;
      എങ്കിൽ (CommandName) {
        ആകുമ്പോൾ (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } അല്ലെങ്കിൽ {
        ആകുമ്പോൾ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      എങ്കിൽ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        എങ്കിൽ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } അല്ലെങ്കിൽ {
          CommandName = അസത്യം;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } അല്ലെങ്കിൽ എങ്കിൽ (Src[I] == '\\') {
        പ്രതിജ്ഞാ(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } അല്ലെങ്കിൽ {
        llvm_unreachable("unexpected special character");
      }
      നിർത്തുക;
    }

    അവസ്ഥ UNQUOTED:
      എങ്കിൽ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        എങ്കിൽ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } അല്ലെങ്കിൽ {
          CommandName = അസത്യം;
        }
        State = INIT;
      } അല്ലെങ്കിൽ എങ്കിൽ (Src[I] == '\"') {
        State = QUOTED;
      } അല്ലെങ്കിൽ എങ്കിൽ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } അല്ലെങ്കിൽ {
        Token.തള്ളുക_back(Src[I]);
      }
      നിർത്തുക;

    അവസ്ഥ QUOTED:
      എങ്കിൽ (Src[I] == '\"') {
        എങ്കിൽ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.തള്ളുക_back('"');
          ++I;
        } അല്ലെങ്കിൽ {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } അല്ലെങ്കിൽ {
        Token.തള്ളുക_back(Src[I]);
      }
      നിർത്തുക;
    }
  }

  എങ്കിൽ (State != INIT)
    AddToken(Saver.save(Token.str()));
}

ശൂന്യം cl::Tokenize_Windows_Command_വരി(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv,
                                    തർക്കം MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.തള്ളുക_back(Tok.data()); };
  auto OnEOL = [&]() {
    എങ്കിൽ (MarkEOLs)
      NewArgv.തള്ളുക_back(nullptr);
  };
  tokenize_Windows_Command_വരി_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/സത്യം, OnEOL, അസത്യം);
}

ശൂന്യം cl::Tokenize_Windows_Command_വരി_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.തള്ളുക_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_വരി_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/അസത്യം,
                                 OnEOL, അസത്യം);
}

ശൂന്യം cl::Tokenize_Windows_Command_വരി_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv,
                                        തർക്കം MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.തള്ളുക_back(Tok.data()); };
  auto OnEOL = [&]() {
    എങ്കിൽ (MarkEOLs)
      NewArgv.തള്ളുക_back(nullptr);
  };
  tokenize_Windows_Command_വരി_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/സത്യം, OnEOL, സത്യം);
}

ശൂന്യം cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv,
                            തർക്കം MarkEOLs) {
  വേണ്ടി (സ്ഥിരം അക്ഷരം *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> വരി;
    // Check for comment line.
    എങ്കിൽ (isWhitespace(*Cur)) {
      ആകുമ്പോൾ (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      തുടരുക;
    }
    എങ്കിൽ (*Cur == '#') {
      ആകുമ്പോൾ (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      തുടരുക;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    സ്ഥിരം അക്ഷരം *Start = Cur;
    സ്ഥിരം അക്ഷരം *വരി_End = nullptr;
    അക്ഷരം Quote = 0;
    തർക്കം AtTokenStart = സത്യം;
    വേണ്ടി (സ്ഥിരം അക്ഷരം *End = Source.end(); Cur != End; ++Cur) {
      അക്ഷരം C = *Cur;
      എങ്കിൽ (C == '\\') {
        എങ്കിൽ (Cur + 1 != End) {
          ++Cur;
          എങ്കിൽ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            വരി.append(Start, Cur - 1);
            എങ്കിൽ (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            തുടരുക;
          }
        }
        AtTokenStart = അസത്യം;
        തുടരുക;
      }
      എങ്കിൽ (C == '\n')
        നിർത്തുക;
      എങ്കിൽ (Quote) {
        എങ്കിൽ (C == Quote)
          Quote = 0;
      } അല്ലെങ്കിൽ എങ്കിൽ (isQuote(C)) {
        Quote = C;
      } അല്ലെങ്കിൽ എങ്കിൽ (C == '#' && AtTokenStart) {
        വരി_End = Cur;
        ആകുമ്പോൾ (Cur != End && *Cur != '\n')
          ++Cur;
        നിർത്തുക;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    വരി.append(Start, വരി_End ? വരി_End : Cur);
    cl::Tokenize_GNUCommand_വരി(വരി, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
സ്ഥിതം തർക്കം hasUTF8ByteOrderMark(ArrayRef<അക്ഷരം> S) {
  തിരികെ (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
സ്ഥിതം ശൂന്യം ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            സ്ഥിരം അക്ഷരം *&Arg) {
  പ്രതിജ്ഞാ(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  സ്ഥിരം StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  വേണ്ടി (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    സ്ഥിരം StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    എങ്കിൽ (ResponseFile.empty())
      ResponseFile = LHS;
    അല്ലെങ്കിൽ
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  എങ്കിൽ (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    സ്ഥിരം StringRef Remaining = ArgString.substr(StartPos);
    എങ്കിൽ (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv) {
  പ്രതിജ്ഞാ(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  എങ്കിൽ (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    തിരികെ llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<അക്ഷരം> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  എങ്കിൽ (hasUTF16ByteOrderMark(BufRef)) {
    എങ്കിൽ (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      തിരികെ llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  അല്ലെങ്കിൽ എങ്കിൽ (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  എങ്കിൽ (!RelativeNames && !InConfigFile)
    തിരികെ Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  വേണ്ടി (സ്ഥിരം അക്ഷരം *&Arg : NewArgv) {
    എങ്കിൽ (!Arg)
      തുടരുക;

    // Substitute <CFGDIR> with the file's base path.
    എങ്കിൽ (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    തർക്കം ConfigInclusion = അസത്യം;
    എങ്കിൽ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      എങ്കിൽ (!llvm::sys::path::is_relative(FileName))
        തുടരുക;
    } അല്ലെങ്കിൽ എങ്കിൽ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = സത്യം;
    } അല്ലെങ്കിൽ {
      തുടരുക;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.തള്ളുക_back('@');
    എങ്കിൽ (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      എങ്കിൽ (!findConfigFile(FileName, FilePath))
        തിരികെ createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } അല്ലെങ്കിൽ {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  തിരികെ Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &Argv) {
  ഘടന ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_അട്ടി;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_അട്ടി.തള്ളുക_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  വേണ്ടി (unsigned I = 0; I != Argv.size();) {
    ആകുമ്പോൾ (I == File_അട്ടി.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_അട്ടി.എടുക്കുക_back();
    }

    സ്ഥിരം അക്ഷരം *Arg = Argv[I];
    // Check if it is an EOL marker
    എങ്കിൽ (Arg == nullptr) {
      ++I;
      തുടരുക;
    }

    എങ്കിൽ (Arg[0] != '@') {
      ++I;
      തുടരുക;
    }

    സ്ഥിരം അക്ഷരം *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    എങ്കിൽ (llvm::sys::path::is_relative(FName)) {
      എങ്കിൽ (CurrentDir.empty()) {
        എങ്കിൽ (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } അല്ലെങ്കിൽ {
          തിരികെ createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } അല്ലെങ്കിൽ {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    എങ്കിൽ (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      എങ്കിൽ (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        എങ്കിൽ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          തുടരുക;
        }
      }
      എങ്കിൽ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      തിരികെ createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    സ്ഥിരം llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, ഇത്](സ്ഥിരം ResponseFileRecord &RFile) -> ErrorOr<തർക്കം> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      എങ്കിൽ (!RHS)
        തിരികെ RHS.getError();
      തിരികെ FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    വേണ്ടി (സ്ഥിരം auto &F : drop_begin(File_അട്ടി)) {
      എങ്കിൽ (ErrorOr<തർക്കം> R = IsEquivalent(F)) {
        എങ്കിൽ (R.get())
          തിരികെ createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } അല്ലെങ്കിൽ {
        തിരികെ createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<സ്ഥിരം അക്ഷരം *, 0> ExpandedArgv;
    എങ്കിൽ (Error Err = expandResponseFile(FName, ExpandedArgv))
      തിരികെ Err;

    വേണ്ടി (ResponseFileRecord &Record : File_അട്ടി) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_അട്ടി.തള്ളുക_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.തിരുകുക(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  പ്രതിജ്ഞാ(File_അട്ടി.size() > 0 && Argv.size() == File_അട്ടി.back().End);
  തിരികെ Error::success();
}

തർക്കം cl::expandResponseFiles(പൂർണ്ണസംഖ്യ Argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം *Argv,
                             സ്ഥിരം അക്ഷരം *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_വരി;
#അല്ലെങ്കിൽ
  auto Tokenize = cl::Tokenize_GNUCommand_വരി;
#endif
  // The environment variable specifies initial options.
  എങ്കിൽ (EnvVar)
    എങ്കിൽ (std::optional<std::string> Env_മൂല്യം = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_മൂല്യം, Saver, NewArgv, /*MarkEOLs=*/അസത്യം);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  എങ്കിൽ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

തർക്കം cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  എങ്കിൽ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

തർക്കം ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<അക്ഷരം> &FilePath) {
  SmallString<128> CfgFilePath;
  സ്ഥിരം auto FileExists = [ഇത്](SmallString<128> Path) -> തർക്കം {
    auto Status = FS->status(Path);
    തിരികെ Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  എങ്കിൽ (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    എങ്കിൽ (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      തിരികെ അസത്യം;
    എങ്കിൽ (!FileExists(CfgFilePath))
      തിരികെ അസത്യം;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    തിരികെ സത്യം;
  }

  // Look for the file in search directories.
  വേണ്ടി (സ്ഥിരം StringRef &Dir : തിരയൽ_Dirs) {
    എങ്കിൽ (Dir.empty())
      തുടരുക;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    എങ്കിൽ (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      തിരികെ സത്യം;
    }
  }

  തിരികെ അസത്യം;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<സ്ഥിരം അക്ഷരം *> &Argv) {
  SmallString<128> AbsPath;
  എങ്കിൽ (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    എങ്കിൽ (std::error_code EC = FS->makeAbsolute(AbsPath))
      തിരികെ make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = സത്യം;
  RelativeNames = സത്യം;
  എങ്കിൽ (Error Err = expandResponseFile(CfgFile, Argv))
    തിരികെ Err;
  തിരികെ expandResponseFiles(Argv);
}

സ്ഥിതം ശൂന്യം initCommonOptions();
സ്ഥിതം തർക്കം parse_Command_വരി_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<സ്ഥിരം അക്ഷരം *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        സ്ഥിരം അക്ഷരം *EnvVar) {
  initCommonOptions();
  SmallVector<സ്ഥിരം അക്ഷരം *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.തള്ളുക_back(Args[0]);

  // Parse options from environment variable.
  എങ്കിൽ (EnvVar) {
    എങ്കിൽ (std::optional<std::string> Env_മൂല്യം =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_വരി(*Env_മൂല്യം, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  പൂർണ്ണസംഖ്യ NewArgc = static_cast<പൂർണ്ണസംഖ്യ>(NewArgv.size());

  // Parse all options.
  തിരികെ globalParser().Parse_Command_വരി_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

തർക്കം cl::Parse_Command_വരി_Options(പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, സ്ഥിരം അക്ഷരം *EnvVar) {
  തിരികെ parse_Command_വരി_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

തർക്കം cl::parse_Command_വരി_Options(LibraryOptionsContext &Context,
                                 ArrayRef<സ്ഥിരം അക്ഷരം *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, സ്ഥിരം അക്ഷരം *EnvVar) {
  തിരികെ parse_Command_വരി_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
ശൂന്യം Command_വരി_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  വേണ്ടി (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    വേണ്ടി (auto &O : SC->OptionsMap)
      Opts.തള്ളുക_back(O.second);
    വേണ്ടി (Option *O : Opts)
      O->reset();
    വേണ്ടി (Option *O : SC->PositionalOpts)
      O->reset();
    എങ്കിൽ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  വേണ്ടി (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

തർക്കം Command_വരി_Parser::Parse_Command_വരി_Options(
    പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  പ്രതിജ്ഞാ(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  തർക്കം IgnoreErrors = Errs;
  എങ്കിൽ (!Errs)
    Errs = &errs();
  എങ്കിൽ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  തർക്കം ErrorParsing = അസത്യം;

  // Expand response files.
  SmallVector<സ്ഥിരം അക്ഷരം *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_വരി;
#അല്ലെങ്കിൽ
  auto Tokenize = cl::Tokenize_GNUCommand_വരി;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  എങ്കിൽ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    തിരികെ അസത്യം;
  }
  argv = &newArgv[0];
  argc = static_cast<പൂർണ്ണസംഖ്യ>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned അക്കം_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  തർക്കം HasUnlimitedPositionals = അസത്യം;

  പൂർണ്ണസംഖ്യ FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_മുകൾ_Level();
  std::string NearestSubCommandString;
  തർക്കം MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  എങ്കിൽ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    എങ്കിൽ (ChosenSubCommand != &SubCommand::get_മുകൾ_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  പ്രതിജ്ഞാ(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  എങ്കിൽ (ConsumeAfterOpt) {
    പ്രതിജ്ഞാ(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  എങ്കിൽ (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    തർക്കം UnboundedFound = അസത്യം;
    വേണ്ടി (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      എങ്കിൽ (Requires_മൂല്യം(Opt))
        ++അക്കം_Positional_Required;
      അല്ലെങ്കിൽ എങ്കിൽ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        എങ്കിൽ (PositionalOpts.size() > 1) {
          എങ്കിൽ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = സത്യം;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        എങ്കിൽ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = സത്യം;
      }
      UnboundedFound |= Eats_Unbounded_സംഖ്യ_Of_മൂല്യങ്ങൾ(Opt);
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
  തർക്കം DashDashFound = അസത്യം; // Have we read '--'?
  വേണ്ടി (പൂർണ്ണസംഖ്യ i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef മൂല്യം;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    എങ്കിൽ (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      എങ്കിൽ (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        തുടരുക; // We are done!
      }

      എങ്കിൽ (!PositionalOpts.empty()) {
        PositionalVals.തള്ളുക_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        എങ്കിൽ (PositionalVals.size() >= അക്കം_Positional_Required && ConsumeAfterOpt) {
          വേണ്ടി (++i; i < argc; ++i)
            PositionalVals.തള്ളുക_back(std::make_pair(StringRef(argv[i]), i));
          നിർത്തുക; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        തുടരുക;
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = സത്യം; // This is the mythical "--"?
      തുടരുക;             // Don't try to process it as an argument itself.
    } അല്ലെങ്കിൽ എങ്കിൽ (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, മൂല്യം);
      എങ്കിൽ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        തുടരുക; // We are done!
      }
    } അല്ലെങ്കിൽ { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, മൂല്യം);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      എങ്കിൽ (!Handler && ChosenSubCommand != &SubCommand::get_മുകൾ_Level())
        Handler = LookupOption(SubCommand::get_മുകൾ_Level(), ArgName, മൂല്യം);

      എങ്കിൽ (!Handler) {
        എങ്കിൽ (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<സ്ഥിരം അക്ഷരം *> Rest(argv + i, argc - i);
          unsigned N;
          എങ്കിൽ (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = സത്യം;
          }
          i += N - 1;
          തുടരുക;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      എങ്കിൽ (!Handler)
        Handler = HandlePrefixedOption(ArgName, മൂല്യം, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      എങ്കിൽ (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    എങ്കിൽ (!Handler) {
      auto ReportUnknownArgument = [&](തർക്കം IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        എങ്കിൽ (NearestArgumentName.empty())
          തിരികെ;

        *Errs << ProgramName << ": Did you mean '";
        എങ്കിൽ (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        അല്ലെങ്കിൽ
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      എങ്കിൽ (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/സത്യം, NearestHandlerString);
      അല്ലെങ്കിൽ
        ReportUnknownArgument(/*IsArg=*/അസത്യം, NearestSubCommandString);

      ErrorParsing = സത്യം;
      തുടരുക;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    എങ്കിൽ (Handler->getFormattingFlag() == cl::Positional) {
      എങ്കിൽ ((Handler->getMiscFlags() & PositionalEatsArgs) && !മൂല്യം.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = സത്യം;
      }
      ActivePositionalArg = Handler;
    }
    അല്ലെങ്കിൽ
      ErrorParsing |= ProvideOption(Handler, ArgName, മൂല്യം, argc, argv, i);
  }

  // Check and handle positional arguments now...
  എങ്കിൽ (അക്കം_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << അക്കം_Positional_Required
             << " positional argument" << (അക്കം_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = സത്യം;
  } അല്ലെങ്കിൽ എങ്കിൽ (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = സത്യം;

  } അല്ലെങ്കിൽ എങ്കിൽ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, അക്കം_Vals = static_cast<unsigned>(PositionalVals.size());
    വേണ്ടി (Option *Opt : PositionalOpts) {
      എങ്കിൽ (Requires_മൂല്യം(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --അക്കം_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      തർക്കം Done = Opt->get_അക്കം_Occurrences_Flag() == cl::Required;
      ആകുമ്പോൾ (അക്കം_Vals - ValNo > അക്കം_Positional_Required && !Done) {
        തിരഞ്ഞെടുപ്പ് (Opt->get_അക്കം_Occurrences_Flag()) {
        അവസ്ഥ cl::Optional:
          Done = സത്യം; // Optional arguments want _at most_ one value
          [[fallthrough]];
        അവസ്ഥ cl::ZeroOrMore: // Zero or more will take all they can get...
        അവസ്ഥ cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(ConsumeAfterOpt && അക്കം_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    വേണ്ടി (Option *Opt : PositionalOpts)
      എങ്കിൽ (Requires_മൂല്യം(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    എങ്കിൽ (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    വേണ്ടി (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  വേണ്ടി (സ്ഥിരം auto &Opt : OptionsMap) {
    തിരഞ്ഞെടുപ്പ് (Opt.second->get_അക്കം_Occurrences_Flag()) {
    അവസ്ഥ Required:
    അവസ്ഥ OneOrMore:
      എങ്കിൽ (Opt.second->get_അക്കം_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = സത്യം;
      }
      [[fallthrough]];
    സ്ഥിരസ്ഥിതി:
      നിർത്തുക;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  എങ്കിൽ (ErrorParsing) {
    എങ്കിൽ (!IgnoreErrors)
      exit(1);
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

തർക്കം Option::error(സ്ഥിരം Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  എങ്കിൽ (!ArgName.data())
    ArgName = ArgStr;
  എങ്കിൽ (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  അല്ലെങ്കിൽ
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  തിരികെ സത്യം;
}

തർക്കം Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef മൂല്യം) {
  ++അക്കം_Occurrences;
  തിരികെ handleOccurrence(pos, ArgName, മൂല്യം);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
സ്ഥിതം StringRef get_മൂല്യം_Str(സ്ഥിരം Option &O, StringRef Default_സന്ദേശം) {
  എങ്കിൽ (O.മൂല്യം_Str.empty())
    തിരികെ Default_സന്ദേശം;
  തിരികെ O.മൂല്യം_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() സ്ഥിരം {
  തിരികെ argPlusPrefixesSize(ArgStr);
}

ശൂന്യം Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_വരി_Indented_By) {
  പ്രതിജ്ഞാ(Indent >= First_വരി_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_വരി_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  ആകുമ്പോൾ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

ശൂന്യം Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_വരി_Indented_By) {
  സ്ഥിരം StringRef ValHelpPrefix = "  ";
  പ്രതിജ്ഞാ(BaseIndent >= First_വരി_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_വരി_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  ആകുമ്പോൾ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
ശൂന്യം alias::printOptionInfo(size_t GlobalWidth) സ്ഥിരം {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(സ്ഥിരം Option &O) സ്ഥിരം {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_മൂല്യം_Name();
  എങ്കിൽ (!ValName.empty()) {
    size_t FormattingLen = 3;
    എങ്കിൽ (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_മൂല്യം_Str(O, ValName).size() + FormattingLen;
  }

  തിരികെ Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ശൂന്യം basic_parser_impl::printOptionInfo(സ്ഥിരം Option &O,
                                        size_t GlobalWidth) സ്ഥിരം {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_മൂല്യം_Name();
  എങ്കിൽ (!ValName.empty()) {
    എങ്കിൽ (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_മൂല്യം_Str(O, ValName) << ">...";
    } അല്ലെങ്കിൽ എങ്കിൽ (O.get_മൂല്യം_Expected_Flag() == മൂല്യം_Optional)
      outs() << "[=<" << get_മൂല്യം_Str(O, ValName) << ">]";
    അല്ലെങ്കിൽ {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_മൂല്യം_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

ശൂന്യം basic_parser_impl::printOptionName(സ്ഥിരം Option &O,
                                        size_t GlobalWidth) സ്ഥിരം {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
തർക്കം parser<തർക്കം>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         തർക്കം &മൂല്യം) {
  തിരികെ parseBool<തർക്കം, സത്യം, അസത്യം>(O, ArgName, Arg, മൂല്യം);
}

// parser<boolOrDefault> implementation
//
തർക്കം parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &മൂല്യം) {
  തിരികെ parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, മൂല്യം);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
സ്ഥിതം തർക്കം parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef മൂല്യം_Kind,
                                         FixedOrScalableQuantityT &മൂല്യം) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_മൂല്യം;
  എങ്കിൽ (!Arg.getAsInteger(0, Min_മൂല്യം)) {
    മൂല്യം = FixedOrScalableQuantityT::getFixed(Min_മൂല്യം);
    തിരികെ അസത്യം;
  }

  StringRef Remainder = Arg;
  എങ്കിൽ (!Remainder.consume_front("vscale"))
    തിരികെ O.error("'" + Arg + "' value invalid for " + മൂല്യം_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  എങ്കിൽ (!Remainder.consume_front('x'))
    തിരികെ O.error("'" + Arg + "' value invalid for " + മൂല്യം_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  എങ്കിൽ (Remainder.getAsInteger(0, Min_മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for " + മൂല്യം_Kind +
                   " argument!");

  മൂല്യം = FixedOrScalableQuantityT::getScalable(Min_മൂല്യം);
  തിരികെ അസത്യം;
}

// parser<int> implementation
//
തർക്കം parser<പൂർണ്ണസംഖ്യ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        പൂർണ്ണസംഖ്യ &മൂല്യം) {
  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for integer argument!");
  തിരികെ അസത്യം;
}

// parser<long> implementation
//
തർക്കം parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &മൂല്യം) {
  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for long argument!");
  തിരികെ അസത്യം;
}

// parser<long long> implementation
//
തർക്കം parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &മൂല്യം) {
  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for llong argument!");
  തിരികെ അസത്യം;
}

// parser<unsigned> implementation
//
തർക്കം parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &മൂല്യം) {

  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for uint argument!");
  തിരികെ അസത്യം;
}

// parser<unsigned long> implementation
//
തർക്കം parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &മൂല്യം) {

  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for ulong argument!");
  തിരികെ അസത്യം;
}

// parser<unsigned long long> implementation
//
തർക്കം parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &മൂല്യം) {

  എങ്കിൽ (Arg.getAsInteger(0, മൂല്യം))
    തിരികെ O.error("'" + Arg + "' value invalid for ullong argument!");
  തിരികെ അസത്യം;
}

// parser<ElementCount> implementation
//
തർക്കം parser<Element_എണ്ണം>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_എണ്ണം &മൂല്യം) {
  തിരികെ parseFixedOrScalableQuantity(O, Arg, get_മൂല്യം_Name(), മൂല്യം);
}

// parser<double>/parser<float> implementation
//
സ്ഥിതം തർക്കം parseDouble(Option &O, StringRef Arg, ഇരട്ട &മൂല്യം) {
  എങ്കിൽ (to_float(Arg, മൂല്യം))
    തിരികെ അസത്യം;
  തിരികെ O.error("'" + Arg + "' value invalid for floating point argument!");
}

തർക്കം parser<ഇരട്ട>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ഇരട്ട &Val) {
  തിരികെ parseDouble(O, Arg, Val);
}

തർക്കം parser<ദശാംശം>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ദശാംശം &Val) {
  ഇരട്ട dVal;
  എങ്കിൽ (parseDouble(O, Arg, dVal))
    തിരികെ സത്യം;
  Val = (ദശാംശം)dVal;
  തിരികെ അസത്യം;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_അക്കം_Options();

  വേണ്ടി (unsigned i = 0; i != e; ++i) {
    എങ്കിൽ (getOption(i) == Name)
      തിരികെ i;
  }
  തിരികെ e;
}

സ്ഥിതം StringRef Eq_മൂല്യം = "=<value>";
സ്ഥിതം StringRef EmptyOption = "<empty>";
സ്ഥിതം StringRef OptionPrefix = "    =";
സ്ഥിതം size_t getOptionPrefixesSize() {
  തിരികെ OptionPrefix.size() + ArgHelpPrefix.size();
}

സ്ഥിതം തർക്കം shouldPrintOption(StringRef Name, StringRef Description,
                              സ്ഥിരം Option &O) {
  തിരികെ O.get_മൂല്യം_Expected_Flag() != മൂല്യം_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(സ്ഥിരം Option &O) സ്ഥിരം {
  എങ്കിൽ (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_മൂല്യം.size();
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      എങ്കിൽ (!shouldPrintOption(Name, getDescription(i), O))
        തുടരുക;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    തിരികെ Size;
  } അല്ലെങ്കിൽ {
    size_t BaseSize = 0;
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    തിരികെ BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ശൂന്യം generic_parser_base::printOptionInfo(സ്ഥിരം Option &O,
                                          size_t GlobalWidth) സ്ഥിരം {
  എങ്കിൽ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    എങ്കിൽ (O.get_മൂല്യം_Expected_Flag() == മൂല്യം_Optional) {
      വേണ്ടി (unsigned i = 0, e = get_അക്കം_Options(); i != e; ++i) {
        എങ്കിൽ (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          നിർത്തുക;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_മൂല്യം;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_മൂല്യം.size() +
                             argPlusPrefixesSize(O.ArgStr));
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      എങ്കിൽ (!shouldPrintOption(OptionName, Description, O))
        തുടരുക;
      size_t First_വരി_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      എങ്കിൽ (OptionName.empty()) {
        outs() << EmptyOption;
        പ്രതിജ്ഞാ(First_വരി_Indent >= EmptyOption.size());
        First_വരി_Indent += EmptyOption.size();
      }
      എങ്കിൽ (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_വരി_Indent);
      അല്ലെങ്കിൽ
        outs() << '\n';
    }
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

സ്ഥിതം സ്ഥിരം size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
ശൂന്യം generic_parser_base::printGenericOptionDiff(
    സ്ഥിരം Option &O, സ്ഥിരം Generic_Option_മൂല്യം &മൂല്യം,
    സ്ഥിരം Generic_Option_മൂല്യം &Default, size_t GlobalWidth) സ്ഥിരം {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned അക്കം_Opts = get_അക്കം_Options();
  വേണ്ടി (unsigned i = 0; i != അക്കം_Opts; ++i) {
    എങ്കിൽ (!മൂല്യം.compare(get_Option_മൂല്യം(i)))
      തുടരുക;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t അക്കം_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(അക്കം_Spaces) << " (default: ";
    വേണ്ടി (unsigned j = 0; j != അക്കം_Opts; ++j) {
      എങ്കിൽ (!Default.compare(get_Option_മൂല്യം(j)))
        തുടരുക;
      outs() << getOption(j);
      നിർത്തുക;
    }
    outs() << ")\n";
    തിരികെ;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
നാമാകാശ llvm {
നാമാകാശ cl {
സ്ഥിതം raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  തിരികെ OS << static_cast<പൂർണ്ണസംഖ്യ>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  ശൂന്യം parser<T>::printOptionDiff(സ്ഥിരം Option &O, T V, Option_മൂല്യം<T> D,      \
                                  size_t GlobalWidth) സ്ഥിരം {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t അക്കം_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(അക്കം_Spaces) << " (default: ";                                 \
    എങ്കിൽ (D.has_മൂല്യം())                                                          \
      outs() << D.get_മൂല്യം();                                                  \
    അല്ലെങ്കിൽ                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(തർക്കം)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(പൂർണ്ണസംഖ്യ)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(ഇരട്ട)
PRINT_OPT_DIFF(ദശാംശം)
PRINT_OPT_DIFF(അക്ഷരം)
PRINT_OPT_DIFF(Element_എണ്ണം)

ശൂന്യം parser<std::string>::printOptionDiff(സ്ഥിരം Option &O, StringRef V,
                                          സ്ഥിരം Option_മൂല്യം<std::string> &D,
                                          size_t GlobalWidth) സ്ഥിരം {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t അക്കം_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(അക്കം_Spaces) << " (default: ";
  എങ്കിൽ (D.has_മൂല്യം())
    outs() << D.get_മൂല്യം();
  അല്ലെങ്കിൽ
    outs() << "*no default*";
  outs() << ")\n";
}

ശൂന്യം parser<std::optional<std::string>>::printOptionDiff(
    സ്ഥിരം Option &O, std::optional<StringRef> V,
    സ്ഥിരം Option_മൂല്യം<std::optional<std::string>> &D,
    size_t GlobalWidth) സ്ഥിരം {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_മൂല്യം_2() ? V.മൂല്യം_2().size() : 0;
  size_t അക്കം_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(അക്കം_Spaces) << " (default: ";
  എങ്കിൽ (D.has_മൂല്യം() && D.get_മൂല്യം().has_മൂല്യം_2())
    outs() << D.get_മൂല്യം();
  അല്ലെങ്കിൽ
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
ശൂന്യം basic_parser_impl::print_Option_No_മൂല്യം(സ്ഥിരം Option &O,
                                           size_t GlobalWidth) സ്ഥിരം {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

സ്ഥിതം പൂർണ്ണസംഖ്യ OptNameCompare(സ്ഥിരം std::pair<സ്ഥിരം അക്ഷരം *, Option *> *LHS,
                          സ്ഥിരം std::pair<സ്ഥിരം അക്ഷരം *, Option *> *RHS) {
  തിരികെ strcmp(LHS->first, RHS->first);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SubNameCompare(സ്ഥിരം std::pair<സ്ഥിരം അക്ഷരം *, SubCommand *> *LHS,
                          സ്ഥിരം std::pair<സ്ഥിരം അക്ഷരം *, SubCommand *> *RHS) {
  തിരികെ strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
സ്ഥിതം ശൂന്യം sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<സ്ഥിരം അക്ഷരം *, Option *>> &Opts,
                     തർക്കം ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  വേണ്ടി (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    എങ്കിൽ (I->second->getOptionHiddenFlag() == ReallyHidden)
      തുടരുക;

    // Unless showhidden is set, ignore hidden flags.
    എങ്കിൽ (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      തുടരുക;

    // If we've already seen this option, don't add it to the list again.
    എങ്കിൽ (!OptionSet.തിരുകുക(I->second).second)
      തുടരുക;

    Opts.തള്ളുക_back(
        std::pair<സ്ഥിരം അക്ഷരം *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

സ്ഥിതം ശൂന്യം
sortSubCommands(സ്ഥിരം SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<സ്ഥിരം അക്ഷരം *, SubCommand *>> &Subs) {
  വേണ്ടി (auto *S : SubMap) {
    എങ്കിൽ (S->getName().empty())
      തുടരുക;
    Subs.തള്ളുക_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

നാമാകാശ {

വർഗ്ഗം HelpPrinter {
സംരക്ഷിത:
  സ്ഥിരം തർക്കം ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<സ്ഥിരം അക്ഷരം *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<സ്ഥിരം അക്ഷരം *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  മായാ ശൂന്യം printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    വേണ്ടി (സ്ഥിരം auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  ശൂന്യം printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    വേണ്ടി (സ്ഥിരം auto &S : Subs) {
      outs() << "  " << S.first;
      എങ്കിൽ (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

പൊതു:
  explicit HelpPrinter(തർക്കം showHidden) : ShowHidden(showHidden) {}
  മായാ ~HelpPrinter() = സ്ഥിരസ്ഥിതി;

  // Invoke the printer.
  ശൂന്യം operator=(തർക്കം മൂല്യം) {
    എങ്കിൽ (!മൂല്യം)
      തിരികെ;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  ശൂന്യം printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    എങ്കിൽ (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    എങ്കിൽ (Sub == &SubCommand::get_മുകൾ_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      എങ്കിൽ (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } അല്ലെങ്കിൽ {
      എങ്കിൽ (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    വേണ്ടി (auto *Opt : PositionalOpts) {
      എങ്കിൽ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    എങ്കിൽ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    എങ്കിൽ (Sub == &SubCommand::get_മുകൾ_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      വേണ്ടി (സ്ഥിരം auto &Sub : Subs)
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
    വേണ്ടി (സ്ഥിരം auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    എങ്കിൽ (ShowHidden)
      വേണ്ടി (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              എങ്കിൽ (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    വേണ്ടി (സ്ഥിരം auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    എങ്കിൽ (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    വേണ്ടി (സ്ഥിരം auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    വേണ്ടി (സ്ഥിരം auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

വർഗ്ഗം CategorizedHelpPrinter : പൊതു HelpPrinter {
പൊതു:
  explicit CategorizedHelpPrinter(തർക്കം showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  സ്ഥിതം പൂർണ്ണസംഖ്യ OptionCategoryCompare(OptionCategory *സ്ഥിരം *A,
                                   OptionCategory *സ്ഥിരം *B) {
    തിരികെ (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

സംരക്ഷിത:
  ശൂന്യം printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    പ്രതിജ്ഞാ(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    വേണ്ടി (സ്ഥിരം auto &I : Opts) {
      Option *Opt = I.second;
      വേണ്ടി (OptionCategory *Cat : Opt->Categories) {
        പ്രതിജ്ഞാ(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].തള്ളുക_back(Opt);
      }
    }

    // Now do printing.
    വേണ്ടി (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      സ്ഥിരം auto &CategoryOptions = CategorizedOptions[Category];
      എങ്കിൽ (CategoryOptions.empty())
        തുടരുക;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      എങ്കിൽ (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      അല്ലെങ്കിൽ
        outs() << "\n";

      // Loop over the options in the category and print.
      വേണ്ടി (സ്ഥിരം Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
വർഗ്ഗം HelpPrinterWrapper {
സ്വകാര്യ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

പൊതു:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  ശൂന്യം operator=(തർക്കം മൂല്യം);
};

} // End anonymous namespace

#എങ്കിൽ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# എങ്കിൽ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# അല്ലെങ്കിൽ
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# എങ്കിൽ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# അല്ലെങ്കിൽ
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#അല്ലെങ്കിൽ
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

നാമാകാശ {
വർഗ്ഗം VersionPrinter {
പൊതു:
  ശൂന്യം print(സ്ഥിരം std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#അല്ലെങ്കിൽ
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#എങ്കിൽ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#അല്ലെങ്കിൽ
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    എങ്കിൽ (!ExtraPrinters.empty()) {
      വേണ്ടി (സ്ഥിരം auto &I : ExtraPrinters)
        I(outs());
    }
  }
  ശൂന്യം operator=(തർക്കം OptionWasSpecified);
};

ഘടന Command_വരി_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{അസത്യം};
  HelpPrinter UncategorizedHiddenPrinter{സത്യം};
  CategorizedHelpPrinter CategorizedNormalPrinter{അസത്യം};
  CategorizedHelpPrinter CategorizedHiddenPrinter{സത്യം};
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
  cl::opt<HelpPrinter, സത്യം, parser<തർക്കം>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::മൂല്യം_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, സത്യം, parser<തർക്കം>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::മൂല്യം_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, സത്യം, parser<തർക്കം>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::മൂല്യം_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, സത്യം, parser<തർക്കം>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::മൂല്യം_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<തർക്കം> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(അസത്യം),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<തർക്കം> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(അസത്യം),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, സത്യം, parser<തർക്കം>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::മൂല്യം_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
സ്ഥിതം ManagedStatic<Command_വരി_Common_Options> CommonOptions;

സ്ഥിതം ശൂന്യം initCommonOptions() {
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
  സ്ഥിതം OptionCategory GeneralCategory{"General options"};
  തിരികെ GeneralCategory;
}

ശൂന്യം VersionPrinter::operator=(തർക്കം OptionWasSpecified) {
  എങ്കിൽ (!OptionWasSpecified)
    തിരികെ;

  എങ്കിൽ (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

ശൂന്യം HelpPrinterWrapper::operator=(തർക്കം മൂല്യം) {
  എങ്കിൽ (!മൂല്യം)
    തിരികെ;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  എങ്കിൽ (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = സത്യം; // Invoke categorized printer
  } അല്ലെങ്കിൽ {
    UncategorizedPrinter = സത്യം; // Invoke uncategorized printer
  }
}

// Print the value of each option.
ശൂന്യം cl::Print_Option_മൂല്യങ്ങൾ() { globalParser().print_Option_മൂല്യങ്ങൾ(); }

ശൂന്യം Command_വരി_Parser::print_Option_മൂല്യങ്ങൾ() {
  എങ്കിൽ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    തിരികെ;

  SmallVector<std::pair<സ്ഥിരം അക്ഷരം *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ സത്യം);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  വേണ്ടി (സ്ഥിരം auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  വേണ്ടി (സ്ഥിരം auto &Opt : Opts)
    Opt.second->print_Option_മൂല്യം(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
ശൂന്യം cl::PrintHelpMessage(തർക്കം Hidden, തർക്കം Categorized) {
  എങ്കിൽ (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  അല്ലെങ്കിൽ എങ്കിൽ (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  അല്ലെങ്കിൽ എങ്കിൽ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  അല്ലെങ്കിൽ
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  സ്ഥിതം സ്ഥിരം StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#എങ്കിൽ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#എങ്കിൽ __has_feature(address_sanitizer)
      "+asan",
#endif
#എങ്കിൽ __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#എങ്കിൽ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#എങ്കിൽ __has_feature(memory_sanitizer)
      "+msan",
#endif
#എങ്കിൽ __has_feature(thread_sanitizer)
      "+tsan",
#endif
#എങ്കിൽ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  തിരികെ ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
ശൂന്യം cl::printBuildConfig(raw_ostream &OS) {
#എങ്കിൽ LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
ശൂന്യം cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

ശൂന്യം cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

ശൂന്യം cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.തള്ളുക_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (ശൂന്യം)Subs;
  പ്രതിജ്ഞാ(Subs.contains(&Sub));
  തിരികെ Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  തിരികെ globalParser().getRegisteredSubcommands();
}

ശൂന്യം cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  വേണ്ടി (auto &I : Sub.OptionsMap) {
    തർക്കം Unrelated = സത്യം;
    വേണ്ടി (auto &Cat : I.second->Categories) {
      എങ്കിൽ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = അസത്യം;
    }
    എങ്കിൽ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ശൂന്യം cl::HideUnrelatedOptions(ArrayRef<സ്ഥിരം cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  വേണ്ടി (auto &I : Sub.OptionsMap) {
    തർക്കം Unrelated = സത്യം;
    വേണ്ടി (auto &Cat : I.second->Categories) {
      എങ്കിൽ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = അസത്യം;
    }
    എങ്കിൽ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ശൂന്യം cl::Reset_Command_വരി_Parser() { globalParser().reset(); }

ശൂന്യം cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.തള്ളുക_back(&L);
}

ശൂന്യം LibraryOptionsContext::assign(സ്ഥിരം LibraryOptionsContext &Other) {
  പ്രതിജ്ഞാ(ഇത് != &Other);
  മൂല്യങ്ങൾ.clear();
  SavedArgs.clear();
  ArrayRef<സ്ഥിരം അക്ഷരം *> Args = Other.SavedArgs;
  വേണ്ടി (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<സ്ഥിരം അക്ഷരം *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = മൂല്യങ്ങൾ.try_emplace(L.getDefault());
  എങ്കിൽ (Inserted)
    It->second = L.copyDefault(Alloc);
  തിരികെ parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

ശൂന്യം cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

ശൂന്യം LLVMParse_Command_വരി_Options(പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം *argv,
                                 സ്ഥിരം അക്ഷരം *Overview) {
  llvm::cl::Parse_Command_വരി_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
