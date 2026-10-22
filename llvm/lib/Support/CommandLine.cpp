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

#અન્તર્ભાવ "llvm/Support/CommandLine.h"

#અન્તર્ભાવ "DebugOptions.h"

#અન્તર્ભાવ "llvm-c/Support.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/STLFunctionalExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallPtrSet.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringMap.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/ConvertUTF.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/LibraryOptionsContext.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/StringSaver.h"
#અન્તર્ભાવ "llvm/Support/TypeSize.h"
#અન્તર્ભાવ "llvm/Support/VirtualFileSystem.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cstdlib>
#અન્તર્ભાવ <optional>
#અન્તર્ભાવ <string>
using નામાકાશ llvm;
using નામાકાશ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
નામાકાશ llvm {
નામાકાશ cl {
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<તાર્કિક>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<પૂર્ણાંક>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<long>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<દ્વિગુણ>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<દશાંશ>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<અક્ષર>;
template વર્ગ LLVM_EXPORT_TEMPLATE basic_parser<Element_ગણતરી>;

#જો !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template વર્ગ LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template વર્ગ LLVM_EXPORT_TEMPLATE opt<તાર્કિક>;
template વર્ગ LLVM_EXPORT_TEMPLATE opt<અક્ષર>;
template વર્ગ LLVM_EXPORT_TEMPLATE opt<પૂર્ણાંક>;
template વર્ગ LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
શૂન્ય Generic_Option_કિંમત::anchor() {}
શૂન્ય Option_કિંમત<boolOrDefault>::anchor() {}
શૂન્ય Option_કિંમત<std::string>::anchor() {}
શૂન્ય Option::anchor() {}
શૂન્ય basic_parser_impl::anchor() {}
શૂન્ય parser<તાર્કિક>::anchor() {}
શૂન્ય parser<boolOrDefault>::anchor() {}
શૂન્ય parser<પૂર્ણાંક>::anchor() {}
શૂન્ય parser<long>::anchor() {}
શૂન્ય parser<long long>::anchor() {}
શૂન્ય parser<unsigned>::anchor() {}
શૂન્ય parser<unsigned long>::anchor() {}
શૂન્ય parser<unsigned long long>::anchor() {}
શૂન્ય parser<દ્વિગુણ>::anchor() {}
શૂન્ય parser<દશાંશ>::anchor() {}
શૂન્ય parser<std::string>::anchor() {}
શૂન્ય parser<std::optional<std::string>>::anchor() {}
શૂન્ય parser<અક્ષર>::anchor() {}
શૂન્ય parser<Element_ગણતરી>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
શૂન્ય opt_bool_anchor() { opt<તાર્કિક> anchor{""}; }
શૂન્ય opt_char_anchor() { opt<અક્ષર> anchor{""}; }
શૂન્ય opt_int_anchor() { opt<પૂર્ણાંક> anchor{""}; }
શૂન્ય opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

અચલ સ્થિર size_t DefaultPad = 2;

સ્થિર StringRef ArgPrefix = "-";
સ્થિર StringRef ArgPrefixLong = "--";
સ્થિર StringRef ArgHelpPrefix = " - ";

સ્થિર size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  જો (Len == 1)
    પાછા Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  પાછા Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

સ્થિર SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  માટે (size_t I = 0; I < Pad; ++I) {
    Prefix.ધકેલો_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  પાછા Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

નામાકાશ {

વર્ગ PrintArg {
  StringRef ArgName;
  size_t Pad;
જાહેર:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, અચલ PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, અચલ PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  પાછા OS;
}

વર્ગ Command_લીટી_Parser {
જાહેર:
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
  size_t અંક_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_લીટી_Parser() { registerSubCommand(&SubCommand::get_ટોચ_Level()); }

  શૂન્ય ResetAllOptionOccurrences();

  તાર્કિક Parse_Command_લીટી_Options(પૂર્ણાંક argc, અચલ અક્ષર *અચલ *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  શૂન્ય forEachSubCommand(Option &Opt, function_ref<શૂન્ય(SubCommand &)> Action) {
    જો (Opt.Subs.empty()) {
      Action(SubCommand::get_ટોચ_Level());
      પાછા;
    }
    જો (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      માટે (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      પાછા;
    }
    માટે (auto *SC : Opt.Subs) {
      પ્રતિજ્ઞા(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  શૂન્ય addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    જો (Opt.hasArgStr())
      પાછા;
    જો (!SC->OptionsMap.ઉમેરો(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  શૂન્ય addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  શૂન્ય addOption(Option *O, SubCommand *SC) {
    તાર્કિક HadErrors = ખોટું;
    જો (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      જો (!SC->OptionsMap.ઉમેરો(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = સાચું;
      }
    }

    // Remember information about positional options.
    જો (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ધકેલો_back(O);
    નહીંતર જો (O->get_અંક_Occurrences_Flag() == cl::ConsumeAfter) {
      જો (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = સાચું;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    જો (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  શૂન્ય addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  શૂન્ય removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    જો (O->hasArgStr())
      OptionNames.ધકેલો_back(O->ArgStr);

    SubCommand &Sub = *SC;
    માટે (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      જો (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    જો (O->getFormattingFlag() == cl::Positional)
      માટે (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        જો (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          તોડો;
        }
      }
    નહીંતર જો (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  શૂન્ય removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  તાર્કિક hasOptions(અચલ SubCommand &Sub) અચલ {
    પાછા (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  તાર્કિક hasOptions() અચલ {
    માટે (અચલ auto *S : RegisteredSubCommands) {
      જો (hasOptions(*S))
        પાછા સાચું;
    }
    પાછા !Libraries.empty();
  }

  તાર્કિક hasNamedSubCommands() અચલ {
    માટે (અચલ auto *S : RegisteredSubCommands)
      જો (!S->getName().empty())
        પાછા સાચું;
    પાછા ખોટું;
  }

  SubCommand *getActiveSubCommand() { પાછા ActiveSubCommand; }

  શૂન્ય updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    જો (!Sub.OptionsMap.ઉમેરો(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  શૂન્ય updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  શૂન્ય print_Option_કિંમતો();

  શૂન્ય indexLibraryOptions() {
    તાર્કિક HadErrors = ખોટું;
    માટે (; અંક_Indexed_Libraries != Libraries.size(); ++અંક_Indexed_Libraries) {
      LibraryOptions *L = Libraries[અંક_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        જો (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 પાછા SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          પાછા;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = સાચું;
      });
    }
    જો (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    પાછા LibraryIndex.lookup(Name);
  }

  શૂન્ય registerCategory(OptionCategory *cat) {
    પ્રતિજ્ઞા(ગણતરી_if(RegisteredOptionCategories,
                    [cat](અચલ OptionCategory *Category) {
             પાછા cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ઉમેરો(cat);
  }

  શૂન્ય registerSubCommand(SubCommand *sub) {
    પ્રતિજ્ઞા(ગણતરી_if(RegisteredSubCommands,
                    [sub](અચલ SubCommand *Sub) {
                      પાછા (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ઉમેરો(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    પ્રતિજ્ઞા(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    માટે (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      જો (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      નહીંતર
        addLiteralOption(*O, sub, E.first);
    }
  }

  શૂન્ય unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    પાછા make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  શૂન્ય reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    અંક_Indexed_Libraries = 0;

    SubCommand::get_ટોચ_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_ટોચ_Level());
  }

ખાનગી:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &કિંમત);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
સ્થિર Command_લીટી_Parser &globalParser() {
  સ્થિર ManagedStatic<Command_લીટી_Parser> GlobalParser;
  પાછા *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
સ્થિર Error
parseLibraryOption(LibraryOptions &L, શૂન્ય *Opts, ArrayRef<અચલ અક્ષર *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<અચલ અક્ષર *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  અચલ અક્ષર *Saved[2];
  unsigned અંક_Args = std::min<size_t>(Args.size(), 2);
  માટે (unsigned J = 0; J != અંક_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  જો (Error E = L.parse(Opts, ArrayRef(Saved, અંક_Args), Consumed, Alloc))
    પાછા E;
  જો (Record)
    Record->append(Saved, Saved + Consumed);
  પાછા Error::success();
}

template <typename T, T TrueVal, T FalseVal>
સ્થિર તાર્કિક parseBool(Option &O, StringRef ArgName, StringRef Arg, T &કિંમત) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  જો (!Arg.data() || Arg == "true" || Arg == "1") {
    કિંમત = TrueVal;
    પાછા ખોટું;
  }

  જો (Arg == "false" || Arg == "0") {
    કિંમત = FalseVal;
    પાછા ખોટું;
  }
  પાછા O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

શૂન્ય cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ધકેલો_back(Help);
}

Option::Option(અંક_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : અંક_Occurrences(0), Occurrences(OccurrencesFlag), કિંમત(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(ખોટું), Position(0) {
  Categories.ધકેલો_back(&getGeneralCategory());
}

શૂન્ય Option::addArgument() {
  globalParser().addOption(આ);
  FullyInitialized = સાચું;
}

શૂન્ય Option::removeArgument() { globalParser().removeOption(આ); }

શૂન્ય Option::setArgStr(StringRef S) {
  જો (FullyInitialized)
    globalParser().updateArgStr(આ, S);
  પ્રતિજ્ઞા(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

શૂન્ય Option::addCategory(OptionCategory &C) {
  પ્રતિજ્ઞા(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  જો (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  નહીંતર જો (!is_contained(Categories, &C))
    Categories.ધકેલો_back(&C);
}

શૂન્ય Option::reset() {
  અંક_Occurrences = 0;
  setDefault();
}

શૂન્ય OptionCategory::registerCategory() {
  globalParser().registerCategory(આ);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_ટોચ_Level() {
  સ્થિર ManagedStatic<SubCommand> ટોચ_Level_Sub_Command;
  પાછા *ટોચ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  સ્થિર ManagedStatic<SubCommand> AllSubCommands;
  પાછા *AllSubCommands;
}

શૂન્ય SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(આ);
}

શૂન્ય SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(આ);
}

શૂન્ય SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator તાર્કિક() અચલ {
  પાછા (globalParser().getActiveSubCommand() == આ);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_લીટી_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &કિંમત) {
  // Reject all dashes.
  જો (Arg.empty())
    પાછા nullptr;
  પ્રતિજ્ઞા(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  જો (EqualPos == StringRef::npos) {
    // Look up the option.
    પાછા Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  જો (I == Sub.OptionsMap.end())
    પાછા nullptr;

  auto *O = I->second;
  જો (O->getFormattingFlag() == cl::AlwaysPrefix)
    પાછા nullptr;

  કિંમત = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  પાછા I->second;
}

SubCommand *Command_લીટી_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  જો (Name.empty())
    પાછા &SubCommand::get_ટોચ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  માટે (auto *S : RegisteredSubCommands) {
    પ્રતિજ્ઞા(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    જો (S->getName().empty())
      ચાલુ;

    જો (S->getName() == Name)
      પાછા S;

    જો (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  જો (NearestMatch)
    NearestString = NearestMatch->getName();

  પાછા &SubCommand::get_ટોચ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
સ્થિર Option *LookupNearestOption(StringRef Arg,
                                   અચલ OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  જો (Arg.empty())
    પાછા nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  માટે (અચલ auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    જો (O->getOptionHiddenFlag() == ReallyHidden)
      ચાલુ;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    જો (O->hasArgStr())
      OptionNames.ધકેલો_back(O->ArgStr);

    તાર્કિક Permit_કિંમત = O->get_કિંમત_Expected_Flag() != cl::કિંમત_Disallowed;
    StringRef Flag = Permit_કિંમત ? LHS : Arg;
    માટે (અચલ auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/સાચું, /*MaxEditDistance=*/BestDistance);
      જો (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        જો (RHS.empty() || !Permit_કિંમત)
          NearestString = std::string(Name);
        નહીંતર
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  પાછા Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
સ્થિર તાર્કિક CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef કિંમત) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  જો (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(કિંમત);
    StringRef::size_type Pos = Val.find(',');

    જ્યારે (Pos != StringRef::npos) {
      // Process the portion before the comma.
      જો (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        પાછા સાચું;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    કિંમત = Val;
  }

  પાછા Handler->addOccurrence(pos, ArgName, કિંમત);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
સ્થિર અંતઃરેખ તાર્કિક ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef કિંમત, પૂર્ણાંક argc,
                                 અચલ અક્ષર *અચલ *argv, પૂર્ણાંક &i) {
  // Enforce value requirements
  પસંદગી (Handler->get_કિંમત_Expected_Flag()) {
  સ્થિતિ કિંમત_Required:
    જો (!કિંમત.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      જો (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        પાછા Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      પ્રતિજ્ઞા(argv && "null check");
      કિંમત = StringRef(argv[++i]);
    }
    તોડો;
  સ્થિતિ કિંમત_Disallowed:
    જો (કિંમત.data())
      પાછા Handler->error("does not allow a value! '" + Twine(કિંમત) +
                            "' specified.");
    તોડો;
  સ્થિતિ કિંમત_Optional:
    તોડો;
  }

  પાછા CommaSeparateAndAddOccurrence(Handler, i, ArgName, કિંમત);
}

તાર્કિક llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, પૂર્ણાંક i) {
  પૂર્ણાંક Dummy = i;
  પાછા ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
સ્થિર Option *findPrefixOption(StringRef Name, size_t &Length,
                                અચલ OptionsMapTy &OptionsMap) {
  માટે (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    જો (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      પાછા O;
    }
  }
  પાછા nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
સ્થિર Option *HandlePrefixedOption(StringRef &Arg, StringRef &કિંમત,
                                    અચલ OptionsMapTy &OptionsMap) {
  જો (Arg.size() == 1)
    પાછા nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  જો (!POpt)
    પાછા nullptr;

  StringRef Maybe_કિંમત =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  જો (POpt->getFormattingFlag() == cl::Prefix && Maybe_કિંમત.starts_with("="))
    Maybe_કિંમત = Maybe_કિંમત.drop_front();
  કિંમત = Maybe_કિંમત;
  પાછા POpt;
}

સ્થિર તાર્કિક Requires_કિંમત(અચલ Option *O) {
  પાછા O->get_અંક_Occurrences_Flag() == cl::Required ||
         O->get_અંક_Occurrences_Flag() == cl::OneOrMore;
}

સ્થિર તાર્કિક Eats_Unbounded_સંખ્યા_Of_કિંમતો(અચલ Option *O) {
  પાછા O->get_અંક_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_અંક_Occurrences_Flag() == cl::OneOrMore;
}

સ્થિર તાર્કિક isWhitespace(અક્ષર C) {
  પાછા C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

સ્થિર તાર્કિક isWhitespaceOrNull(અક્ષર C) {
  પાછા isWhitespace(C) || C == '\0';
}

સ્થિર તાર્કિક isQuote(અક્ષર C) { પાછા C == '\"' || C == '\''; }

શૂન્ય cl::Tokenize_GNUCommand_લીટી(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<અચલ અક્ષર *> &NewArgv,
                                તાર્કિક MarkEOLs) {
  SmallString<128> Token;
  તાર્કિક InToken = ખોટું;
  માટે (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    જો (!InToken) {
      જ્યારે (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        જો (MarkEOLs && Src[I] == '\n')
          NewArgv.ધકેલો_back(nullptr);
        ++I;
      }
      જો (I == E)
        તોડો;
      InToken = સાચું;
    }

    અક્ષર C = Src[I];

    // Backslash escapes the next character.
    જો (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ધકેલો_back(Src[I]);
      ચાલુ;
    }

    // Consume a quoted string.
    જો (isQuote(C)) {
      ++I;
      જ્યારે (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        જો (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ધકેલો_back(Src[I]);
        ++I;
      }
      જો (I == E)
        તોડો;
      ચાલુ;
    }

    // End the token if this is whitespace.
    જો (isWhitespace(C)) {
      NewArgv.ધકેલો_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      જો (MarkEOLs && C == '\n')
        NewArgv.ધકેલો_back(nullptr);
      Token.clear();
      InToken = ખોટું;
      ચાલુ;
    }

    // This is a normal character.  Append it.
    Token.ધકેલો_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  જો (InToken)
    NewArgv.ધકેલો_back(Saver.save(Token.str()).data());
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
સ્થિર size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  પૂર્ણાંક Backslash_ગણતરી = 0;
  // Skip the backslashes.
  કરો {
    ++I;
    ++Backslash_ગણતરી;
  } જ્યારે (I != E && Src[I] == '\\');

  તાર્કિક FollowedByDoubleQuote = (I != E && Src[I] == '"');
  જો (FollowedByDoubleQuote) {
    Token.append(Backslash_ગણતરી / 2, '\\');
    જો (Backslash_ગણતરી % 2 == 0)
      પાછા I - 1;
    Token.ધકેલો_back('"');
    પાછા I;
  }
  Token.append(Backslash_ગણતરી, '\\');
  પાછા I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
સ્થિર તાર્કિક isWindowsSpecialChar(અક્ષર C) {
  પાછા isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
સ્થિર તાર્કિક isWindowsSpecialCharInCommandName(અક્ષર C) {
  પાછા isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
સ્થિર અંતઃરેખ શૂન્ય tokenize_Windows_Command_લીટી_Impl(
    StringRef Src, StringSaver &Saver, function_ref<શૂન્ય(StringRef)> AddToken,
    તાર્કિક AlwaysCopy, function_ref<શૂન્ય()> MarkEOL, તાર્કિક InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  તાર્કિક CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  ગણના { INIT, UNQUOTED, QUOTED } State = INIT;

  માટે (size_t I = 0, E = Src.size(); I < E; ++I) {
    પસંદગી (State) {
    સ્થિતિ INIT: {
      પ્રતિજ્ઞા(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      જ્યારે (I < E && isWhitespaceOrNull(Src[I])) {
        જો (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      જો (I >= E)
        તોડો;
      size_t Start = I;
      જો (CommandName) {
        જ્યારે (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } નહીંતર {
        જ્યારે (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      જો (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        જો (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } નહીંતર {
          CommandName = ખોટું;
        }
      } નહીંતર જો (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } નહીંતર જો (Src[I] == '\\') {
        પ્રતિજ્ઞા(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } નહીંતર {
        llvm_unreachable("unexpected special character");
      }
      તોડો;
    }

    સ્થિતિ UNQUOTED:
      જો (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        જો (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } નહીંતર {
          CommandName = ખોટું;
        }
        State = INIT;
      } નહીંતર જો (Src[I] == '\"') {
        State = QUOTED;
      } નહીંતર જો (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } નહીંતર {
        Token.ધકેલો_back(Src[I]);
      }
      તોડો;

    સ્થિતિ QUOTED:
      જો (Src[I] == '\"') {
        જો (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ધકેલો_back('"');
          ++I;
        } નહીંતર {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } નહીંતર જો (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } નહીંતર {
        Token.ધકેલો_back(Src[I]);
      }
      તોડો;
    }
  }

  જો (State != INIT)
    AddToken(Saver.save(Token.str()));
}

શૂન્ય cl::Tokenize_Windows_Command_લીટી(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<અચલ અક્ષર *> &NewArgv,
                                    તાર્કિક MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ધકેલો_back(Tok.data()); };
  auto OnEOL = [&]() {
    જો (MarkEOLs)
      NewArgv.ધકેલો_back(nullptr);
  };
  tokenize_Windows_Command_લીટી_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/સાચું, OnEOL, ખોટું);
}

શૂન્ય cl::Tokenize_Windows_Command_લીટી_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ધકેલો_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_લીટી_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/ખોટું,
                                 OnEOL, ખોટું);
}

શૂન્ય cl::Tokenize_Windows_Command_લીટી_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<અચલ અક્ષર *> &NewArgv,
                                        તાર્કિક MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ધકેલો_back(Tok.data()); };
  auto OnEOL = [&]() {
    જો (MarkEOLs)
      NewArgv.ધકેલો_back(nullptr);
  };
  tokenize_Windows_Command_લીટી_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/સાચું, OnEOL, સાચું);
}

શૂન્ય cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<અચલ અક્ષર *> &NewArgv,
                            તાર્કિક MarkEOLs) {
  માટે (અચલ અક્ષર *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> લીટી;
    // Check for comment line.
    જો (isWhitespace(*Cur)) {
      જ્યારે (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      ચાલુ;
    }
    જો (*Cur == '#') {
      જ્યારે (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      ચાલુ;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    અચલ અક્ષર *Start = Cur;
    અચલ અક્ષર *લીટી_End = nullptr;
    અક્ષર Quote = 0;
    તાર્કિક AtTokenStart = સાચું;
    માટે (અચલ અક્ષર *End = Source.end(); Cur != End; ++Cur) {
      અક્ષર C = *Cur;
      જો (C == '\\') {
        જો (Cur + 1 != End) {
          ++Cur;
          જો (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            લીટી.append(Start, Cur - 1);
            જો (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            ચાલુ;
          }
        }
        AtTokenStart = ખોટું;
        ચાલુ;
      }
      જો (C == '\n')
        તોડો;
      જો (Quote) {
        જો (C == Quote)
          Quote = 0;
      } નહીંતર જો (isQuote(C)) {
        Quote = C;
      } નહીંતર જો (C == '#' && AtTokenStart) {
        લીટી_End = Cur;
        જ્યારે (Cur != End && *Cur != '\n')
          ++Cur;
        તોડો;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    લીટી.append(Start, લીટી_End ? લીટી_End : Cur);
    cl::Tokenize_GNUCommand_લીટી(લીટી, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
સ્થિર તાર્કિક hasUTF8ByteOrderMark(ArrayRef<અક્ષર> S) {
  પાછા (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
સ્થિર શૂન્ય ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            અચલ અક્ષર *&Arg) {
  પ્રતિજ્ઞા(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  અચલ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  માટે (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    અચલ StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    જો (ResponseFile.empty())
      ResponseFile = LHS;
    નહીંતર
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  જો (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    અચલ StringRef Remaining = ArgString.substr(StartPos);
    જો (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<અચલ અક્ષર *> &NewArgv) {
  પ્રતિજ્ઞા(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  જો (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    પાછા llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<અક્ષર> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  જો (hasUTF16ByteOrderMark(BufRef)) {
    જો (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      પાછા llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  નહીંતર જો (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  જો (!RelativeNames && !InConfigFile)
    પાછા Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  માટે (અચલ અક્ષર *&Arg : NewArgv) {
    જો (!Arg)
      ચાલુ;

    // Substitute <CFGDIR> with the file's base path.
    જો (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    તાર્કિક ConfigInclusion = ખોટું;
    જો (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      જો (!llvm::sys::path::is_relative(FileName))
        ચાલુ;
    } નહીંતર જો (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = સાચું;
    } નહીંતર {
      ચાલુ;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ધકેલો_back('@');
    જો (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      જો (!findConfigFile(FileName, FilePath))
        પાછા createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } નહીંતર {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  પાછા Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<અચલ અક્ષર *> &Argv) {
  સંરચના ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_ઢગલો;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ઢગલો.ધકેલો_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  માટે (unsigned I = 0; I != Argv.size();) {
    જ્યારે (I == File_ઢગલો.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ઢગલો.કાઢો_back();
    }

    અચલ અક્ષર *Arg = Argv[I];
    // Check if it is an EOL marker
    જો (Arg == nullptr) {
      ++I;
      ચાલુ;
    }

    જો (Arg[0] != '@') {
      ++I;
      ચાલુ;
    }

    અચલ અક્ષર *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    જો (llvm::sys::path::is_relative(FName)) {
      જો (CurrentDir.empty()) {
        જો (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } નહીંતર {
          પાછા createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } નહીંતર {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    જો (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      જો (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        જો (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ચાલુ;
        }
      }
      જો (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      પાછા createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    અચલ llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, આ](અચલ ResponseFileRecord &RFile) -> ErrorOr<તાર્કિક> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      જો (!RHS)
        પાછા RHS.getError();
      પાછા FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    માટે (અચલ auto &F : drop_begin(File_ઢગલો)) {
      જો (ErrorOr<તાર્કિક> R = IsEquivalent(F)) {
        જો (R.get())
          પાછા createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } નહીંતર {
        પાછા createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<અચલ અક્ષર *, 0> ExpandedArgv;
    જો (Error Err = expandResponseFile(FName, ExpandedArgv))
      પાછા Err;

    માટે (ResponseFileRecord &Record : File_ઢગલો) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_ઢગલો.ધકેલો_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.ઉમેરો(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  પ્રતિજ્ઞા(File_ઢગલો.size() > 0 && Argv.size() == File_ઢગલો.back().End);
  પાછા Error::success();
}

તાર્કિક cl::expandResponseFiles(પૂર્ણાંક Argc, અચલ અક્ષર *અચલ *Argv,
                             અચલ અક્ષર *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<અચલ અક્ષર *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_લીટી;
#નહીંતર
  auto Tokenize = cl::Tokenize_GNUCommand_લીટી;
#endif
  // The environment variable specifies initial options.
  જો (EnvVar)
    જો (std::optional<std::string> Env_કિંમત = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_કિંમત, Saver, NewArgv, /*MarkEOLs=*/ખોટું);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  જો (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    પાછા ખોટું;
  }
  પાછા સાચું;
}

તાર્કિક cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<અચલ અક્ષર *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  જો (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    પાછા ખોટું;
  }
  પાછા સાચું;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

તાર્કિક ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<અક્ષર> &FilePath) {
  SmallString<128> CfgFilePath;
  અચલ auto FileExists = [આ](SmallString<128> Path) -> તાર્કિક {
    auto Status = FS->status(Path);
    પાછા Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  જો (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    જો (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      પાછા ખોટું;
    જો (!FileExists(CfgFilePath))
      પાછા ખોટું;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    પાછા સાચું;
  }

  // Look for the file in search directories.
  માટે (અચલ StringRef &Dir : શોધ_Dirs) {
    જો (Dir.empty())
      ચાલુ;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    જો (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      પાછા સાચું;
    }
  }

  પાછા ખોટું;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<અચલ અક્ષર *> &Argv) {
  SmallString<128> AbsPath;
  જો (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    જો (std::error_code EC = FS->makeAbsolute(AbsPath))
      પાછા make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = સાચું;
  RelativeNames = સાચું;
  જો (Error Err = expandResponseFile(CfgFile, Argv))
    પાછા Err;
  પાછા expandResponseFiles(Argv);
}

સ્થિર શૂન્ય initCommonOptions();
સ્થિર તાર્કિક parse_Command_લીટી_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<અચલ અક્ષર *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        અચલ અક્ષર *EnvVar) {
  initCommonOptions();
  SmallVector<અચલ અક્ષર *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ધકેલો_back(Args[0]);

  // Parse options from environment variable.
  જો (EnvVar) {
    જો (std::optional<std::string> Env_કિંમત =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_લીટી(*Env_કિંમત, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  પૂર્ણાંક NewArgc = static_cast<પૂર્ણાંક>(NewArgv.size());

  // Parse all options.
  પાછા globalParser().Parse_Command_લીટી_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

તાર્કિક cl::Parse_Command_લીટી_Options(પૂર્ણાંક argc, અચલ અક્ષર *અચલ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, અચલ અક્ષર *EnvVar) {
  પાછા parse_Command_લીટી_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

તાર્કિક cl::parse_Command_લીટી_Options(LibraryOptionsContext &Context,
                                 ArrayRef<અચલ અક્ષર *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, અચલ અક્ષર *EnvVar) {
  પાછા parse_Command_લીટી_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
શૂન્ય Command_લીટી_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  માટે (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    માટે (auto &O : SC->OptionsMap)
      Opts.ધકેલો_back(O.second);
    માટે (Option *O : Opts)
      O->reset();
    માટે (Option *O : SC->PositionalOpts)
      O->reset();
    જો (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  માટે (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

તાર્કિક Command_લીટી_Parser::Parse_Command_લીટી_Options(
    પૂર્ણાંક argc, અચલ અક્ષર *અચલ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  પ્રતિજ્ઞા(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  તાર્કિક IgnoreErrors = Errs;
  જો (!Errs)
    Errs = &errs();
  જો (!VFS)
    VFS = vfs::getRealFileSystem().get();
  તાર્કિક ErrorParsing = ખોટું;

  // Expand response files.
  SmallVector<અચલ અક્ષર *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_લીટી;
#નહીંતર
  auto Tokenize = cl::Tokenize_GNUCommand_લીટી;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  જો (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    પાછા ખોટું;
  }
  argv = &newArgv[0];
  argc = static_cast<પૂર્ણાંક>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned અંક_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  તાર્કિક HasUnlimitedPositionals = ખોટું;

  પૂર્ણાંક FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_ટોચ_Level();
  std::string NearestSubCommandString;
  તાર્કિક MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  જો (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    જો (ChosenSubCommand != &SubCommand::get_ટોચ_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  પ્રતિજ્ઞા(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  જો (ConsumeAfterOpt) {
    પ્રતિજ્ઞા(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  જો (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    તાર્કિક UnboundedFound = ખોટું;
    માટે (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      જો (Requires_કિંમત(Opt))
        ++અંક_Positional_Required;
      નહીંતર જો (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        જો (PositionalOpts.size() > 1) {
          જો (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = સાચું;
        }
      } નહીંતર જો (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        જો (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = સાચું;
      }
      UnboundedFound |= Eats_Unbounded_સંખ્યા_Of_કિંમતો(Opt);
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
  તાર્કિક DashDashFound = ખોટું; // Have we read '--'?
  માટે (પૂર્ણાંક i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef કિંમત;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    જો (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      જો (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ચાલુ; // We are done!
      }

      જો (!PositionalOpts.empty()) {
        PositionalVals.ધકેલો_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        જો (PositionalVals.size() >= અંક_Positional_Required && ConsumeAfterOpt) {
          માટે (++i; i < argc; ++i)
            PositionalVals.ધકેલો_back(std::make_pair(StringRef(argv[i]), i));
          તોડો; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ચાલુ;
      }
    } નહીંતર જો (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = સાચું; // This is the mythical "--"?
      ચાલુ;             // Don't try to process it as an argument itself.
    } નહીંતર જો (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, કિંમત);
      જો (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ચાલુ; // We are done!
      }
    } નહીંતર { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, કિંમત);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      જો (!Handler && ChosenSubCommand != &SubCommand::get_ટોચ_Level())
        Handler = LookupOption(SubCommand::get_ટોચ_Level(), ArgName, કિંમત);

      જો (!Handler) {
        જો (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<અચલ અક્ષર *> Rest(argv + i, argc - i);
          unsigned N;
          જો (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = સાચું;
          }
          i += N - 1;
          ચાલુ;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      જો (!Handler)
        Handler = HandlePrefixedOption(ArgName, કિંમત, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      જો (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    જો (!Handler) {
      auto ReportUnknownArgument = [&](તાર્કિક IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        જો (NearestArgumentName.empty())
          પાછા;

        *Errs << ProgramName << ": Did you mean '";
        જો (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        નહીંતર
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      જો (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/સાચું, NearestHandlerString);
      નહીંતર
        ReportUnknownArgument(/*IsArg=*/ખોટું, NearestSubCommandString);

      ErrorParsing = સાચું;
      ચાલુ;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    જો (Handler->getFormattingFlag() == cl::Positional) {
      જો ((Handler->getMiscFlags() & PositionalEatsArgs) && !કિંમત.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = સાચું;
      }
      ActivePositionalArg = Handler;
    }
    નહીંતર
      ErrorParsing |= ProvideOption(Handler, ArgName, કિંમત, argc, argv, i);
  }

  // Check and handle positional arguments now...
  જો (અંક_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << અંક_Positional_Required
             << " positional argument" << (અંક_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = સાચું;
  } નહીંતર જો (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = સાચું;

  } નહીંતર જો (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, અંક_Vals = static_cast<unsigned>(PositionalVals.size());
    માટે (Option *Opt : PositionalOpts) {
      જો (Requires_કિંમત(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --અંક_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      તાર્કિક Done = Opt->get_અંક_Occurrences_Flag() == cl::Required;
      જ્યારે (અંક_Vals - ValNo > અંક_Positional_Required && !Done) {
        પસંદગી (Opt->get_અંક_Occurrences_Flag()) {
        સ્થિતિ cl::Optional:
          Done = સાચું; // Optional arguments want _at most_ one value
          [[fallthrough]];
        સ્થિતિ cl::ZeroOrMore: // Zero or more will take all they can get...
        સ્થિતિ cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          તોડો;
        મૂળભૂત:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } નહીંતર {
    પ્રતિજ્ઞા(ConsumeAfterOpt && અંક_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    માટે (Option *Opt : PositionalOpts)
      જો (Requires_કિંમત(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    જો (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    માટે (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  માટે (અચલ auto &Opt : OptionsMap) {
    પસંદગી (Opt.second->get_અંક_Occurrences_Flag()) {
    સ્થિતિ Required:
    સ્થિતિ OneOrMore:
      જો (Opt.second->get_અંક_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = સાચું;
      }
      [[fallthrough]];
    મૂળભૂત:
      તોડો;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             માટે (પૂર્ણાંક i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  જો (ErrorParsing) {
    જો (!IgnoreErrors)
      exit(1);
    પાછા ખોટું;
  }
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

તાર્કિક Option::error(અચલ Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  જો (!ArgName.data())
    ArgName = ArgStr;
  જો (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  નહીંતર
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  પાછા સાચું;
}

તાર્કિક Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef કિંમત) {
  ++અંક_Occurrences;
  પાછા handleOccurrence(pos, ArgName, કિંમત);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
સ્થિર StringRef get_કિંમત_Str(અચલ Option &O, StringRef Default_સંદેશ) {
  જો (O.કિંમત_Str.empty())
    પાછા Default_સંદેશ;
  પાછા O.કિંમત_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() અચલ {
  પાછા argPlusPrefixesSize(ArgStr);
}

શૂન્ય Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_લીટી_Indented_By) {
  પ્રતિજ્ઞા(Indent >= First_લીટી_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_લીટી_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  જ્યારે (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

શૂન્ય Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_લીટી_Indented_By) {
  અચલ StringRef ValHelpPrefix = "  ";
  પ્રતિજ્ઞા(BaseIndent >= First_લીટી_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_લીટી_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  જ્યારે (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
શૂન્ય alias::printOptionInfo(size_t GlobalWidth) અચલ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(અચલ Option &O) અચલ {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_કિંમત_Name();
  જો (!ValName.empty()) {
    size_t FormattingLen = 3;
    જો (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_કિંમત_Str(O, ValName).size() + FormattingLen;
  }

  પાછા Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
શૂન્ય basic_parser_impl::printOptionInfo(અચલ Option &O,
                                        size_t GlobalWidth) અચલ {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_કિંમત_Name();
  જો (!ValName.empty()) {
    જો (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_કિંમત_Str(O, ValName) << ">...";
    } નહીંતર જો (O.get_કિંમત_Expected_Flag() == કિંમત_Optional)
      outs() << "[=<" << get_કિંમત_Str(O, ValName) << ">]";
    નહીંતર {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_કિંમત_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

શૂન્ય basic_parser_impl::printOptionName(અચલ Option &O,
                                        size_t GlobalWidth) અચલ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
તાર્કિક parser<તાર્કિક>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         તાર્કિક &કિંમત) {
  પાછા parseBool<તાર્કિક, સાચું, ખોટું>(O, ArgName, Arg, કિંમત);
}

// parser<boolOrDefault> implementation
//
તાર્કિક parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &કિંમત) {
  પાછા parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, કિંમત);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
સ્થિર તાર્કિક parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef કિંમત_Kind,
                                         FixedOrScalableQuantityT &કિંમત) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_કિંમત;
  જો (!Arg.getAsInteger(0, Min_કિંમત)) {
    કિંમત = FixedOrScalableQuantityT::getFixed(Min_કિંમત);
    પાછા ખોટું;
  }

  StringRef Remainder = Arg;
  જો (!Remainder.consume_front("vscale"))
    પાછા O.error("'" + Arg + "' value invalid for " + કિંમત_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  જો (!Remainder.consume_front('x'))
    પાછા O.error("'" + Arg + "' value invalid for " + કિંમત_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  જો (Remainder.getAsInteger(0, Min_કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for " + કિંમત_Kind +
                   " argument!");

  કિંમત = FixedOrScalableQuantityT::getScalable(Min_કિંમત);
  પાછા ખોટું;
}

// parser<int> implementation
//
તાર્કિક parser<પૂર્ણાંક>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        પૂર્ણાંક &કિંમત) {
  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for integer argument!");
  પાછા ખોટું;
}

// parser<long> implementation
//
તાર્કિક parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &કિંમત) {
  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for long argument!");
  પાછા ખોટું;
}

// parser<long long> implementation
//
તાર્કિક parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &કિંમત) {
  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for llong argument!");
  પાછા ખોટું;
}

// parser<unsigned> implementation
//
તાર્કિક parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &કિંમત) {

  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for uint argument!");
  પાછા ખોટું;
}

// parser<unsigned long> implementation
//
તાર્કિક parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &કિંમત) {

  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for ulong argument!");
  પાછા ખોટું;
}

// parser<unsigned long long> implementation
//
તાર્કિક parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &કિંમત) {

  જો (Arg.getAsInteger(0, કિંમત))
    પાછા O.error("'" + Arg + "' value invalid for ullong argument!");
  પાછા ખોટું;
}

// parser<ElementCount> implementation
//
તાર્કિક parser<Element_ગણતરી>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ગણતરી &કિંમત) {
  પાછા parseFixedOrScalableQuantity(O, Arg, get_કિંમત_Name(), કિંમત);
}

// parser<double>/parser<float> implementation
//
સ્થિર તાર્કિક parseDouble(Option &O, StringRef Arg, દ્વિગુણ &કિંમત) {
  જો (to_float(Arg, કિંમત))
    પાછા ખોટું;
  પાછા O.error("'" + Arg + "' value invalid for floating point argument!");
}

તાર્કિક parser<દ્વિગુણ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           દ્વિગુણ &Val) {
  પાછા parseDouble(O, Arg, Val);
}

તાર્કિક parser<દશાંશ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          દશાંશ &Val) {
  દ્વિગુણ dVal;
  જો (parseDouble(O, Arg, dVal))
    પાછા સાચું;
  Val = (દશાંશ)dVal;
  પાછા ખોટું;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_અંક_Options();

  માટે (unsigned i = 0; i != e; ++i) {
    જો (getOption(i) == Name)
      પાછા i;
  }
  પાછા e;
}

સ્થિર StringRef Eq_કિંમત = "=<value>";
સ્થિર StringRef EmptyOption = "<empty>";
સ્થિર StringRef OptionPrefix = "    =";
સ્થિર size_t getOptionPrefixesSize() {
  પાછા OptionPrefix.size() + ArgHelpPrefix.size();
}

સ્થિર તાર્કિક shouldPrintOption(StringRef Name, StringRef Description,
                              અચલ Option &O) {
  પાછા O.get_કિંમત_Expected_Flag() != કિંમત_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(અચલ Option &O) અચલ {
  જો (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_કિંમત.size();
    માટે (unsigned i = 0, e = get_અંક_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      જો (!shouldPrintOption(Name, getDescription(i), O))
        ચાલુ;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    પાછા Size;
  } નહીંતર {
    size_t BaseSize = 0;
    માટે (unsigned i = 0, e = get_અંક_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    પાછા BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
શૂન્ય generic_parser_base::printOptionInfo(અચલ Option &O,
                                          size_t GlobalWidth) અચલ {
  જો (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    જો (O.get_કિંમત_Expected_Flag() == કિંમત_Optional) {
      માટે (unsigned i = 0, e = get_અંક_Options(); i != e; ++i) {
        જો (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          તોડો;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_કિંમત;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_કિંમત.size() +
                             argPlusPrefixesSize(O.ArgStr));
    માટે (unsigned i = 0, e = get_અંક_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      જો (!shouldPrintOption(OptionName, Description, O))
        ચાલુ;
      size_t First_લીટી_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      જો (OptionName.empty()) {
        outs() << EmptyOption;
        પ્રતિજ્ઞા(First_લીટી_Indent >= EmptyOption.size());
        First_લીટી_Indent += EmptyOption.size();
      }
      જો (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_લીટી_Indent);
      નહીંતર
        outs() << '\n';
    }
  } નહીંતર {
    જો (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    માટે (unsigned i = 0, e = get_અંક_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

સ્થિર અચલ size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
શૂન્ય generic_parser_base::printGenericOptionDiff(
    અચલ Option &O, અચલ Generic_Option_કિંમત &કિંમત,
    અચલ Generic_Option_કિંમત &Default, size_t GlobalWidth) અચલ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned અંક_Opts = get_અંક_Options();
  માટે (unsigned i = 0; i != અંક_Opts; ++i) {
    જો (!કિંમત.compare(get_Option_કિંમત(i)))
      ચાલુ;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t અંક_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(અંક_Spaces) << " (default: ";
    માટે (unsigned j = 0; j != અંક_Opts; ++j) {
      જો (!Default.compare(get_Option_કિંમત(j)))
        ચાલુ;
      outs() << getOption(j);
      તોડો;
    }
    outs() << ")\n";
    પાછા;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
નામાકાશ llvm {
નામાકાશ cl {
સ્થિર raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  પાછા OS << static_cast<પૂર્ણાંક>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  શૂન્ય parser<T>::printOptionDiff(અચલ Option &O, T V, Option_કિંમત<T> D,      \
                                  size_t GlobalWidth) અચલ {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t અંક_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(અંક_Spaces) << " (default: ";                                 \
    જો (D.has_કિંમત())                                                          \
      outs() << D.get_કિંમત();                                                  \
    નહીંતર                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(તાર્કિક)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(પૂર્ણાંક)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(દ્વિગુણ)
PRINT_OPT_DIFF(દશાંશ)
PRINT_OPT_DIFF(અક્ષર)
PRINT_OPT_DIFF(Element_ગણતરી)

શૂન્ય parser<std::string>::printOptionDiff(અચલ Option &O, StringRef V,
                                          અચલ Option_કિંમત<std::string> &D,
                                          size_t GlobalWidth) અચલ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t અંક_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(અંક_Spaces) << " (default: ";
  જો (D.has_કિંમત())
    outs() << D.get_કિંમત();
  નહીંતર
    outs() << "*no default*";
  outs() << ")\n";
}

શૂન્ય parser<std::optional<std::string>>::printOptionDiff(
    અચલ Option &O, std::optional<StringRef> V,
    અચલ Option_કિંમત<std::optional<std::string>> &D,
    size_t GlobalWidth) અચલ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_કિંમત_2() ? V.કિંમત_2().size() : 0;
  size_t અંક_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(અંક_Spaces) << " (default: ";
  જો (D.has_કિંમત() && D.get_કિંમત().has_કિંમત_2())
    outs() << D.get_કિંમત();
  નહીંતર
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
શૂન્ય basic_parser_impl::print_Option_No_કિંમત(અચલ Option &O,
                                           size_t GlobalWidth) અચલ {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

સ્થિર પૂર્ણાંક OptNameCompare(અચલ std::pair<અચલ અક્ષર *, Option *> *LHS,
                          અચલ std::pair<અચલ અક્ષર *, Option *> *RHS) {
  પાછા strcmp(LHS->first, RHS->first);
}

સ્થિર પૂર્ણાંક SubNameCompare(અચલ std::pair<અચલ અક્ષર *, SubCommand *> *LHS,
                          અચલ std::pair<અચલ અક્ષર *, SubCommand *> *RHS) {
  પાછા strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
સ્થિર શૂન્ય sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<અચલ અક્ષર *, Option *>> &Opts,
                     તાર્કિક ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  માટે (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    જો (I->second->getOptionHiddenFlag() == ReallyHidden)
      ચાલુ;

    // Unless showhidden is set, ignore hidden flags.
    જો (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ચાલુ;

    // If we've already seen this option, don't add it to the list again.
    જો (!OptionSet.ઉમેરો(I->second).second)
      ચાલુ;

    Opts.ધકેલો_back(
        std::pair<અચલ અક્ષર *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

સ્થિર શૂન્ય
sortSubCommands(અચલ SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<અચલ અક્ષર *, SubCommand *>> &Subs) {
  માટે (auto *S : SubMap) {
    જો (S->getName().empty())
      ચાલુ;
    Subs.ધકેલો_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

નામાકાશ {

વર્ગ HelpPrinter {
સુરક્ષિત:
  અચલ તાર્કિક ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<અચલ અક્ષર *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<અચલ અક્ષર *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  આભાસી શૂન્ય printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    માટે (અચલ auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  શૂન્ય printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    માટે (અચલ auto &S : Subs) {
      outs() << "  " << S.first;
      જો (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

જાહેર:
  explicit HelpPrinter(તાર્કિક showHidden) : ShowHidden(showHidden) {}
  આભાસી ~HelpPrinter() = મૂળભૂત;

  // Invoke the printer.
  શૂન્ય operator=(તાર્કિક કિંમત) {
    જો (!કિંમત)
      પાછા;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  શૂન્ય printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    જો (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    જો (Sub == &SubCommand::get_ટોચ_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      જો (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } નહીંતર {
      જો (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    માટે (auto *Opt : PositionalOpts) {
      જો (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    જો (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    જો (Sub == &SubCommand::get_ટોચ_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      માટે (અચલ auto &Sub : Subs)
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
    માટે (અચલ auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    જો (ShowHidden)
      માટે (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              જો (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    માટે (અચલ auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    જો (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    માટે (અચલ auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    માટે (અચલ auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

વર્ગ CategorizedHelpPrinter : જાહેર HelpPrinter {
જાહેર:
  explicit CategorizedHelpPrinter(તાર્કિક showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  સ્થિર પૂર્ણાંક OptionCategoryCompare(OptionCategory *અચલ *A,
                                   OptionCategory *અચલ *B) {
    પાછા (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

સુરક્ષિત:
  શૂન્ય printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    પ્રતિજ્ઞા(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    માટે (અચલ auto &I : Opts) {
      Option *Opt = I.second;
      માટે (OptionCategory *Cat : Opt->Categories) {
        પ્રતિજ્ઞા(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ધકેલો_back(Opt);
      }
    }

    // Now do printing.
    માટે (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      અચલ auto &CategoryOptions = CategorizedOptions[Category];
      જો (CategoryOptions.empty())
        ચાલુ;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      જો (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      નહીંતર
        outs() << "\n";

      // Loop over the options in the category and print.
      માટે (અચલ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
વર્ગ HelpPrinterWrapper {
ખાનગી:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

જાહેર:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  શૂન્ય operator=(તાર્કિક કિંમત);
};

} // End anonymous namespace

#જો defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# જો defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# નહીંતર
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#અથવાજો defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# જો defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# નહીંતર
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#નહીંતર
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

નામાકાશ {
વર્ગ VersionPrinter {
જાહેર:
  શૂન્ય print(અચલ std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#નહીંતર
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#જો LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#નહીંતર
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    જો (!ExtraPrinters.empty()) {
      માટે (અચલ auto &I : ExtraPrinters)
        I(outs());
    }
  }
  શૂન્ય operator=(તાર્કિક OptionWasSpecified);
};

સંરચના Command_લીટી_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{ખોટું};
  HelpPrinter UncategorizedHiddenPrinter{સાચું};
  CategorizedHelpPrinter CategorizedNormalPrinter{ખોટું};
  CategorizedHelpPrinter CategorizedHiddenPrinter{સાચું};
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
  cl::opt<HelpPrinter, સાચું, parser<તાર્કિક>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::કિંમત_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, સાચું, parser<તાર્કિક>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::કિંમત_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, સાચું, parser<તાર્કિક>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::કિંમત_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, સાચું, parser<તાર્કિક>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::કિંમત_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<તાર્કિક> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(ખોટું),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<તાર્કિક> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(ખોટું),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, સાચું, parser<તાર્કિક>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::કિંમત_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
સ્થિર ManagedStatic<Command_લીટી_Common_Options> CommonOptions;

સ્થિર શૂન્ય initCommonOptions() {
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
  સ્થિર OptionCategory GeneralCategory{"General options"};
  પાછા GeneralCategory;
}

શૂન્ય VersionPrinter::operator=(તાર્કિક OptionWasSpecified) {
  જો (!OptionWasSpecified)
    પાછા;

  જો (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

શૂન્ય HelpPrinterWrapper::operator=(તાર્કિક કિંમત) {
  જો (!કિંમત)
    પાછા;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  જો (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = સાચું; // Invoke categorized printer
  } નહીંતર {
    UncategorizedPrinter = સાચું; // Invoke uncategorized printer
  }
}

// Print the value of each option.
શૂન્ય cl::Print_Option_કિંમતો() { globalParser().print_Option_કિંમતો(); }

શૂન્ય Command_લીટી_Parser::print_Option_કિંમતો() {
  જો (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    પાછા;

  SmallVector<std::pair<અચલ અક્ષર *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ સાચું);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  માટે (અચલ auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  માટે (અચલ auto &Opt : Opts)
    Opt.second->print_Option_કિંમત(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
શૂન્ય cl::PrintHelpMessage(તાર્કિક Hidden, તાર્કિક Categorized) {
  જો (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  નહીંતર જો (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  નહીંતર જો (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  નહીંતર
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  સ્થિર અચલ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#જો LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#જો __has_feature(address_sanitizer)
      "+asan",
#endif
#જો __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#જો __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#જો __has_feature(memory_sanitizer)
      "+msan",
#endif
#જો __has_feature(thread_sanitizer)
      "+tsan",
#endif
#જો __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  પાછા ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
શૂન્ય cl::printBuildConfig(raw_ostream &OS) {
#જો LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
શૂન્ય cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

શૂન્ય cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

શૂન્ય cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.ધકેલો_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (શૂન્ય)Subs;
  પ્રતિજ્ઞા(Subs.contains(&Sub));
  પાછા Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  પાછા globalParser().getRegisteredSubcommands();
}

શૂન્ય cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  માટે (auto &I : Sub.OptionsMap) {
    તાર્કિક Unrelated = સાચું;
    માટે (auto &Cat : I.second->Categories) {
      જો (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = ખોટું;
    }
    જો (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

શૂન્ય cl::HideUnrelatedOptions(ArrayRef<અચલ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  માટે (auto &I : Sub.OptionsMap) {
    તાર્કિક Unrelated = સાચું;
    માટે (auto &Cat : I.second->Categories) {
      જો (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = ખોટું;
    }
    જો (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

શૂન્ય cl::Reset_Command_લીટી_Parser() { globalParser().reset(); }

શૂન્ય cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ધકેલો_back(&L);
}

શૂન્ય LibraryOptionsContext::assign(અચલ LibraryOptionsContext &Other) {
  પ્રતિજ્ઞા(આ != &Other);
  કિંમતો.clear();
  SavedArgs.clear();
  ArrayRef<અચલ અક્ષર *> Args = Other.SavedArgs;
  માટે (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<અચલ અક્ષર *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = કિંમતો.try_emplace(L.getDefault());
  જો (Inserted)
    It->second = L.copyDefault(Alloc);
  પાછા parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

શૂન્ય cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

શૂન્ય LLVMParse_Command_લીટી_Options(પૂર્ણાંક argc, અચલ અક્ષર *અચલ *argv,
                                 અચલ અક્ષર *Overview) {
  llvm::cl::Parse_Command_લીટી_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
