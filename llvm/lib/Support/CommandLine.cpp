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

#समावेश "llvm/Support/CommandLine.h"

#समावेश "DebugOptions.h"

#समावेश "llvm-c/Support.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/STLFunctionalExtras.h"
#समावेश "llvm/ADT/SmallPtrSet.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringMap.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/LibraryOptionsContext.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/StringSaver.h"
#समावेश "llvm/Support/TypeSize.h"
#समावेश "llvm/Support/VirtualFileSystem.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cstdlib>
#समावेश <optional>
#समावेश <string>
using namespace llvm;
using namespace cl;

#घोषणा DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
namespace llvm {
namespace cl {
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<bool>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<पूर्णांक>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ दीर्घ>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित दीर्घ>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित दीर्घ दीर्घ>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<द्विगुन>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<भग्न>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<अक्षर>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE basic_parser<Element_गिनती>;

#अगर !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE opt<std::string>;
#पूर्ण_अगर

ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE opt<bool>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE opt<अक्षर>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE opt<पूर्णांक>;
ढाँचा श्रेणी LLVM_EXPORT_TEMPLATE opt<अचिन्हित>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
व्योम Generic_Option_मूल्य::anchor() {}
व्योम Option_मूल्य<boolOrDefault>::anchor() {}
व्योम Option_मूल्य<std::string>::anchor() {}
व्योम Option::anchor() {}
व्योम basic_parser_impl::anchor() {}
व्योम parser<bool>::anchor() {}
व्योम parser<boolOrDefault>::anchor() {}
व्योम parser<पूर्णांक>::anchor() {}
व्योम parser<दीर्घ>::anchor() {}
व्योम parser<दीर्घ दीर्घ>::anchor() {}
व्योम parser<अचिन्हित>::anchor() {}
व्योम parser<अचिन्हित दीर्घ>::anchor() {}
व्योम parser<अचिन्हित दीर्घ दीर्घ>::anchor() {}
व्योम parser<द्विगुन>::anchor() {}
व्योम parser<भग्न>::anchor() {}
व्योम parser<std::string>::anchor() {}
व्योम parser<std::optional<std::string>>::anchor() {}
व्योम parser<अक्षर>::anchor() {}
व्योम parser<Element_गिनती>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
व्योम opt_bool_anchor() { opt<bool> anchor{""}; }
व्योम opt_char_anchor() { opt<अक्षर> anchor{""}; }
व्योम opt_int_anchor() { opt<पूर्णांक> anchor{""}; }
व्योम opt_unsigned_anchor() { opt<अचिन्हित> anchor{""}; }

//===----------------------------------------------------------------------===//

स्थिर अटल माप_प्रकार DefaultPad = 2;

अटल StringRef ArgPrefix = "-";
अटल StringRef ArgPrefixLong = "--";
अटल StringRef ArgHelpPrefix = " - ";

अटल माप_प्रकार arg_Plus_Prefixes_आकार(StringRef ArgName, माप_प्रकार Pad = DefaultPad) {
  माप_प्रकार Len = ArgName.आकार_2();
  अगर (Len == 1)
    वापस Len + Pad + ArgPrefix.आकार_2() + ArgHelpPrefix.आकार_2();
  वापस Len + Pad + ArgPrefixLong.आकार_2() + ArgHelpPrefix.आकार_2();
}

अटल SmallString<8> argPrefix(StringRef ArgName, माप_प्रकार Pad = DefaultPad) {
  SmallString<8> Prefix;
  क्रम (माप_प्रकार I = 0; I < Pad; ++I) {
    Prefix.धकेलो_back(' ');
  }
  Prefix.append(ArgName.आकार_2() > 1 ? ArgPrefixLong : ArgPrefix);
  वापस Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

namespace {

श्रेणी PrintArg {
  StringRef ArgName;
  माप_प्रकार Pad;
खुला:
  PrintArg(StringRef ArgName, माप_प्रकार Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  मित्र raw_ostream &चालक<<(raw_ostream &OS, स्थिर PrintArg &);
};

raw_ostream &चालक<<(raw_ostream &OS, स्थिर PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  वापस OS;
}

श्रेणी Command_पंक्ति_Parser {
खुला:
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
  DenseMap<StringRef, LibraryOptions *> Library_सूचकांक;
  माप_प्रकार अंक_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_पंक्ति_Parser() { registerSubCommand(&SubCommand::get_शीर्ष_Level()); }

  व्योम ResetAllOptionOccurrences();

  bool Parse_Command_पंक्ति_Options(पूर्णांक argc, स्थिर अक्षर *स्थिर *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  व्योम forEachSubCommand(Option &Opt, function_ref<व्योम(SubCommand &)> Action) {
    अगर (Opt.Subs.empty()) {
      Action(SubCommand::get_शीर्ष_Level());
      वापस;
    }
    अगर (Opt.Subs.आकार_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      क्रम (स्वतः *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      वापस;
    }
    क्रम (स्वतः *SC : Opt.Subs) {
      निश्चित(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  व्योम addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    अगर (Opt.hasArgStr())
      वापस;
    अगर (!SC->OptionsMap.जोड़ो(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  व्योम addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  व्योम addOption(Option *O, SubCommand *SC) {
    bool HadErrors = false;
    अगर (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      अगर (!SC->OptionsMap.जोड़ो(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           Library_सूचकांक.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = true;
      }
    }

    // Remember information about positional options.
    अगर (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.धकेलो_back(O);
    अन्यथा अगर (O->get_अंक_Occurrences_Flag() == cl::ConsumeAfter) {
      अगर (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = true;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    अगर (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  व्योम addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  व्योम removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    अगर (O->hasArgStr())
      OptionNames.धकेलो_back(O->ArgStr);

    SubCommand &Sub = *SC;
    क्रम (स्वतः Name : OptionNames) {
      स्वतः I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      अगर (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    अगर (O->getFormattingFlag() == cl::Positional)
      क्रम (स्वतः *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        अगर (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          अवरोध;
        }
      }
    अन्यथा अगर (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  व्योम removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  bool hasOptions(स्थिर SubCommand &Sub) स्थिर {
    वापस (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  bool hasOptions() स्थिर {
    क्रम (स्थिर स्वतः *S : RegisteredSubCommands) {
      अगर (hasOptions(*S))
        वापस true;
    }
    वापस !Libraries.empty();
  }

  bool hasNamedSubCommands() स्थिर {
    क्रम (स्थिर स्वतः *S : RegisteredSubCommands)
      अगर (!S->getName().empty())
        वापस true;
    वापस false;
  }

  SubCommand *getActiveSubCommand() { वापस ActiveSubCommand; }

  व्योम updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    अगर (!Sub.OptionsMap.जोड़ो(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  व्योम updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  व्योम print_Option_मूल्यावली();

  व्योम सूचकांक_Library_Options() {
    bool HadErrors = false;
    क्रम (; अंक_Indexed_Libraries != Libraries.आकार_2(); ++अंक_Indexed_Libraries) {
      LibraryOptions *L = Libraries[अंक_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        स्वतः [It, Inserted] = Library_सूचकांक.try_emplace(Name, L);
        अगर (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 वापस SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          वापस;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = true;
      });
    }
    अगर (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    सूचकांक_Library_Options();
    वापस Library_सूचकांक.lookup(Name);
  }

  व्योम registerCategory(OptionCategory *cat) {
    निश्चित(गिनती_if(RegisteredOptionCategories,
                    [cat](स्थिर OptionCategory *Category) {
             वापस cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.जोड़ो(cat);
  }

  व्योम registerSubCommand(SubCommand *sub) {
    निश्चित(गिनती_if(RegisteredSubCommands,
                    [sub](स्थिर SubCommand *Sub) {
                      वापस (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.जोड़ो(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    निश्चित(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    क्रम (स्वतः &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      अगर (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      अन्यथा
        addLiteralOption(*O, sub, E.first);
    }
  }

  व्योम unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    वापस make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  व्योम reset() {
    ActiveSubCommand = nullptr;
    ProgramName.सफा();
    ProgramOverview = StringRef();

    MoreHelp.सफा();
    RegisteredOptionCategories.सफा();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.सफा();
    Libraries.सफा();
    Library_सूचकांक.सफा();
    अंक_Indexed_Libraries = 0;

    SubCommand::get_शीर्ष_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_शीर्ष_Level());
  }

निजी:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &मूल्य);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
अटल Command_पंक्ति_Parser &globalParser() {
  अटल ManagedStatic<Command_पंक्ति_Parser> GlobalParser;
  वापस *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
अटल Error
parseLibraryOption(LibraryOptions &L, व्योम *Opts, ArrayRef<स्थिर अक्षर *> Args,
                   अचिन्हित &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<स्थिर अक्षर *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  स्थिर अक्षर *Saved[2];
  अचिन्हित अंक_Args = std::min<माप_प्रकार>(Args.आकार_2(), 2);
  क्रम (अचिन्हित J = 0; J != अंक_Args; ++J)
    Saved[J] = Saver.save(Args[J]).तथ्य();
  Consumed = 1;
  अगर (Error E = L.parse(Opts, ArrayRef(Saved, अंक_Args), Consumed, Alloc))
    वापस E;
  अगर (Record)
    Record->append(Saved, Saved + Consumed);
  वापस Error::success();
}

ढाँचा <typename T, T TrueVal, T FalseVal>
अटल bool parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मूल्य) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  अगर (!Arg.तथ्य() || Arg == "true" || Arg == "1") {
    मूल्य = TrueVal;
    वापस false;
  }

  अगर (Arg == "false" || Arg == "0") {
    मूल्य = FalseVal;
    वापस false;
  }
  वापस O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

व्योम cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.धकेलो_back(Help);
}

Option::Option(अंक_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : अंक_Occurrences(0), Occurrences(OccurrencesFlag), मूल्य(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(false), Position(0) {
  Categories.धकेलो_back(&getGeneralCategory());
}

व्योम Option::addArgument() {
  globalParser().addOption(यह);
  FullyInitialized = true;
}

व्योम Option::removeArgument() { globalParser().removeOption(यह); }

व्योम Option::setArgStr(StringRef S) {
  अगर (FullyInitialized)
    globalParser().updateArgStr(यह, S);
  निश्चित(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

व्योम Option::addCategory(OptionCategory &C) {
  निश्चित(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  अगर (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अन्यथा अगर (!is_contained(Categories, &C))
    Categories.धकेलो_back(&C);
}

व्योम Option::reset() {
  अंक_Occurrences = 0;
  setDefault();
}

व्योम OptionCategory::registerCategory() {
  globalParser().registerCategory(यह);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_शीर्ष_Level() {
  अटल ManagedStatic<SubCommand> शीर्ष_Level_Sub_Command;
  वापस *शीर्ष_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  अटल ManagedStatic<SubCommand> AllSubCommands;
  वापस *AllSubCommands;
}

व्योम SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(यह);
}

व्योम SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(यह);
}

व्योम SubCommand::reset() {
  PositionalOpts.सफा();
  OptionsMap.सफा();

  ConsumeAfterOpt = nullptr;
}

SubCommand::चालक bool() स्थिर {
  वापस (globalParser().getActiveSubCommand() == यह);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_पंक्ति_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &मूल्य) {
  // Reject all dashes.
  अगर (Arg.empty())
    वापस nullptr;
  निश्चित(&Sub != &SubCommand::getAll());

  माप_प्रकार EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  अगर (EqualPos == StringRef::npos) {
    // Look up the option.
    वापस Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  स्वतः I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  अगर (I == Sub.OptionsMap.end())
    वापस nullptr;

  स्वतः *O = I->second;
  अगर (O->getFormattingFlag() == cl::AlwaysPrefix)
    वापस nullptr;

  मूल्य = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  वापस I->second;
}

SubCommand *Command_पंक्ति_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  अगर (Name.empty())
    वापस &SubCommand::get_शीर्ष_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  क्रम (स्वतः *S : RegisteredSubCommands) {
    निश्चित(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    अगर (S->getName().empty())
      जारी;

    अगर (S->getName() == Name)
      वापस S;

    अगर (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  अगर (NearestMatch)
    NearestString = NearestMatch->getName();

  वापस &SubCommand::get_शीर्ष_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
अटल Option *LookupNearestOption(StringRef Arg,
                                   स्थिर OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  अगर (Arg.empty())
    वापस nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  अचिन्हित BestDistance = 0;
  क्रम (स्थिर स्वतः &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    अगर (O->getOptionHiddenFlag() == ReallyHidden)
      जारी;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    अगर (O->hasArgStr())
      OptionNames.धकेलो_back(O->ArgStr);

    bool Permit_मूल्य = O->get_मूल्य_Expected_Flag() != cl::मूल्य_Disallowed;
    StringRef Flag = Permit_मूल्य ? LHS : Arg;
    क्रम (स्थिर स्वतः &Name : OptionNames) {
      अचिन्हित Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/true, /*MaxEditDistance=*/BestDistance);
      अगर (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        अगर (RHS.empty() || !Permit_मूल्य)
          NearestString = std::string(Name);
        अन्यथा
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  वापस Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
अटल bool CommaSeparateAndAddOccurrence(Option *Handler, अचिन्हित pos,
                                          StringRef ArgName, StringRef मूल्य) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  अगर (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मूल्य);
    StringRef::आकार_type Pos = Val.find(',');

    जबतक (Pos != StringRef::npos) {
      // Process the portion before the comma.
      अगर (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        वापस true;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मूल्य = Val;
  }

  वापस Handler->addOccurrence(pos, ArgName, मूल्य);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
अटल अंतरभूत bool ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef मूल्य, पूर्णांक argc,
                                 स्थिर अक्षर *स्थिर *argv, पूर्णांक &i) {
  // Enforce value requirements
  चयन (Handler->get_मूल्य_Expected_Flag()) {
  हाल मूल्य_Required:
    अगर (!मूल्य.तथ्य()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      अगर (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        वापस Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      निश्चित(argv && "null check");
      मूल्य = StringRef(argv[++i]);
    }
    अवरोध;
  हाल मूल्य_Disallowed:
    अगर (मूल्य.तथ्य())
      वापस Handler->error("does not allow a value! '" + Twine(मूल्य) +
                            "' specified.");
    अवरोध;
  हाल मूल्य_Optional:
    अवरोध;
  }

  वापस CommaSeparateAndAddOccurrence(Handler, i, ArgName, मूल्य);
}

bool llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, पूर्णांक i) {
  पूर्णांक Dummy = i;
  वापस ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
अटल Option *findPrefixOption(StringRef Name, माप_प्रकार &लंबाई,
                                स्थिर OptionsMapTy &OptionsMap) {
  क्रम (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    अगर (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      लंबाई = Name.आकार_2();
      वापस O;
    }
  }
  वापस nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
अटल Option *HandlePrefixedOption(StringRef &Arg, StringRef &मूल्य,
                                    स्थिर OptionsMapTy &OptionsMap) {
  अगर (Arg.आकार_2() == 1)
    वापस nullptr;

  माप_प्रकार लंबाई = 0;
  Option *POpt = findPrefixOption(Arg, लंबाई, OptionsMap);
  अगर (!POpt)
    वापस nullptr;

  StringRef Maybe_मूल्य =
      (लंबाई < Arg.आकार_2()) ? Arg.substr(लंबाई) : StringRef();
  Arg = Arg.substr(0, लंबाई);

  // cl::Prefix options do not preserve '=' when used separately.
  अगर (POpt->getFormattingFlag() == cl::Prefix && Maybe_मूल्य.starts_with("="))
    Maybe_मूल्य = Maybe_मूल्य.drop_front();
  मूल्य = Maybe_मूल्य;
  वापस POpt;
}

अटल bool Requires_मूल्य(स्थिर Option *O) {
  वापस O->get_अंक_Occurrences_Flag() == cl::Required ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

अटल bool Eats_Unbounded_संख्या_Of_मूल्यावली(स्थिर Option *O) {
  वापस O->get_अंक_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

अटल bool isWhitespace(अक्षर C) {
  वापस C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

अटल bool isWhitespaceOrNull(अक्षर C) {
  वापस isWhitespace(C) || C == '\0';
}

अटल bool isQuote(अक्षर C) { वापस C == '\"' || C == '\''; }

व्योम cl::Tokenize_GNUCommand_पंक्ति(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<स्थिर अक्षर *> &NewArgv,
                                bool MarkEOLs) {
  SmallString<128> Token;
  bool InToken = false;
  क्रम (माप_प्रकार I = 0, E = Src.आकार_2(); I != E; ++I) {
    // Consume runs of whitespace.
    अगर (!InToken) {
      जबतक (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        अगर (MarkEOLs && Src[I] == '\n')
          NewArgv.धकेलो_back(nullptr);
        ++I;
      }
      अगर (I == E)
        अवरोध;
      InToken = true;
    }

    अक्षर C = Src[I];

    // Backslash escapes the next character.
    अगर (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.धकेलो_back(Src[I]);
      जारी;
    }

    // Consume a quoted string.
    अगर (isQuote(C)) {
      ++I;
      जबतक (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        अगर (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.धकेलो_back(Src[I]);
        ++I;
      }
      अगर (I == E)
        अवरोध;
      जारी;
    }

    // End the token if this is whitespace.
    अगर (isWhitespace(C)) {
      NewArgv.धकेलो_back(Saver.save(Token.str()).तथ्य());
      // Mark the end of lines in response files.
      अगर (MarkEOLs && C == '\n')
        NewArgv.धकेलो_back(nullptr);
      Token.सफा();
      InToken = false;
      जारी;
    }

    // This is a normal character.  Append it.
    Token.धकेलो_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  अगर (InToken)
    NewArgv.धकेलो_back(Saver.save(Token.str()).तथ्य());
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
अटल माप_प्रकार parseBackslash(StringRef Src, माप_प्रकार I, SmallString<128> &Token) {
  माप_प्रकार E = Src.आकार_2();
  पूर्णांक Backslash_गिनती = 0;
  // Skip the backslashes.
  करो {
    ++I;
    ++Backslash_गिनती;
  } जबतक (I != E && Src[I] == '\\');

  bool FollowedByDoubleQuote = (I != E && Src[I] == '"');
  अगर (FollowedByDoubleQuote) {
    Token.append(Backslash_गिनती / 2, '\\');
    अगर (Backslash_गिनती % 2 == 0)
      वापस I - 1;
    Token.धकेलो_back('"');
    वापस I;
  }
  Token.append(Backslash_गिनती, '\\');
  वापस I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
अटल bool isWindowsSpecialChar(अक्षर C) {
  वापस isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
अटल bool isWindowsSpecialCharInCommandName(अक्षर C) {
  वापस isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
अटल अंतरभूत व्योम tokenize_Windows_Command_पंक्ति_Impl(
    StringRef Src, StringSaver &Saver, function_ref<व्योम(StringRef)> AddToken,
    bool AlwaysCopy, function_ref<व्योम()> MarkEOL, bool InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  bool CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  क्रमागत { INIT, UNQUOTED, QUOTED } State = INIT;

  क्रम (माप_प्रकार I = 0, E = Src.आकार_2(); I < E; ++I) {
    चयन (State) {
    हाल INIT: {
      निश्चित(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      जबतक (I < E && isWhitespaceOrNull(Src[I])) {
        अगर (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      अगर (I >= E)
        अवरोध;
      माप_प्रकार Start = I;
      अगर (CommandName) {
        जबतक (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } अन्यथा {
        जबतक (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      अगर (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        अगर (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अन्यथा {
          CommandName = false;
        }
      } अन्यथा अगर (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } अन्यथा अगर (Src[I] == '\\') {
        निश्चित(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अन्यथा {
        llvm_unreachable("unexpected special character");
      }
      अवरोध;
    }

    हाल UNQUOTED:
      अगर (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.सफा();
        अगर (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अन्यथा {
          CommandName = false;
        }
        State = INIT;
      } अन्यथा अगर (Src[I] == '\"') {
        State = QUOTED;
      } अन्यथा अगर (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.धकेलो_back(Src[I]);
      }
      अवरोध;

    हाल QUOTED:
      अगर (Src[I] == '\"') {
        अगर (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.धकेलो_back('"');
          ++I;
        } अन्यथा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अन्यथा अगर (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.धकेलो_back(Src[I]);
      }
      अवरोध;
    }
  }

  अगर (State != INIT)
    AddToken(Saver.save(Token.str()));
}

व्योम cl::Tokenize_Windows_Command_पंक्ति(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<स्थिर अक्षर *> &NewArgv,
                                    bool MarkEOLs) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.धकेलो_back(Tok.तथ्य()); };
  स्वतः OnEOL = [&]() {
    अगर (MarkEOLs)
      NewArgv.धकेलो_back(nullptr);
  };
  tokenize_Windows_Command_पंक्ति_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, false);
}

व्योम cl::Tokenize_Windows_Command_पंक्ति_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.धकेलो_back(Tok); };
  स्वतः OnEOL = []() {};
  tokenize_Windows_Command_पंक्ति_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/false,
                                 OnEOL, false);
}

व्योम cl::Tokenize_Windows_Command_पंक्ति_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<स्थिर अक्षर *> &NewArgv,
                                        bool MarkEOLs) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.धकेलो_back(Tok.तथ्य()); };
  स्वतः OnEOL = [&]() {
    अगर (MarkEOLs)
      NewArgv.धकेलो_back(nullptr);
  };
  tokenize_Windows_Command_पंक्ति_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, true);
}

व्योम cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<स्थिर अक्षर *> &NewArgv,
                            bool MarkEOLs) {
  क्रम (स्थिर अक्षर *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> पंक्ति_2;
    // Check for comment line.
    अगर (isWhitespace(*Cur)) {
      जबतक (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      जारी;
    }
    अगर (*Cur == '#') {
      जबतक (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      जारी;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    स्थिर अक्षर *Start = Cur;
    स्थिर अक्षर *पंक्ति_End = nullptr;
    अक्षर Quote = 0;
    bool AtTokenStart = true;
    क्रम (स्थिर अक्षर *End = Source.end(); Cur != End; ++Cur) {
      अक्षर C = *Cur;
      अगर (C == '\\') {
        अगर (Cur + 1 != End) {
          ++Cur;
          अगर (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            पंक्ति_2.append(Start, Cur - 1);
            अगर (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            जारी;
          }
        }
        AtTokenStart = false;
        जारी;
      }
      अगर (C == '\n')
        अवरोध;
      अगर (Quote) {
        अगर (C == Quote)
          Quote = 0;
      } अन्यथा अगर (isQuote(C)) {
        Quote = C;
      } अन्यथा अगर (C == '#' && AtTokenStart) {
        पंक्ति_End = Cur;
        जबतक (Cur != End && *Cur != '\n')
          ++Cur;
        अवरोध;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    पंक्ति_2.append(Start, पंक्ति_End ? पंक्ति_End : Cur);
    cl::Tokenize_GNUCommand_पंक्ति(पंक्ति_2, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
अटल bool hasUTF8ByteOrderMark(ArrayRef<अक्षर> S) {
  वापस (S.आकार_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
अटल व्योम ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            स्थिर अक्षर *&Arg) {
  निश्चित(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  स्थिर StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::आकार_type StartPos = 0;
  क्रम (StringRef::आकार_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    स्थिर StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    अगर (ResponseFile.empty())
      ResponseFile = LHS;
    अन्यथा
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.आकार_2();
  }

  अगर (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    स्थिर StringRef Remaining = ArgString.substr(StartPos);
    अगर (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).तथ्य();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<स्थिर अक्षर *> &NewArgv) {
  निश्चित(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<Memory_बफर>> MemBufOrErr =
      FS->get_बफर_For_File(FName);
  अगर (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    वापस llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  Memory_बफर &MemBuf = *MemBufOrErr.लो();
  StringRef Str(MemBuf.get_बफर_Start(), MemBuf.get_बफर_आकार());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<अक्षर> BufRef(MemBuf.get_बफर_Start(), MemBuf.get_बफर_End());
  std::string UTF8Buf;
  अगर (hasUTF16ByteOrderMark(BufRef)) {
    अगर (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      वापस llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अन्यथा अगर (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.तथ्य() + 3, BufRef.आकार_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  अगर (!RelativeNames && !InConfigFile)
    वापस Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  क्रम (स्थिर अक्षर *&Arg : NewArgv) {
    अगर (!Arg)
      जारी;

    // Substitute <CFGDIR> with the file's base path.
    अगर (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    bool ConfigInclusion = false;
    अगर (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      अगर (!llvm::sys::path::is_relative(FileName))
        जारी;
    } अन्यथा अगर (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = true;
    } अन्यथा {
      जारी;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.धकेलो_back('@');
    अगर (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      अगर (!findConfigFile(FileName, FilePath))
        वापस createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } अन्यथा {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).तथ्य();
  }
  वापस Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<स्थिर अक्षर *> &Argv) {
  काष्ठा ResponseFileRecord {
    std::string File;
    माप_प्रकार End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_ढेर;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ढेर.धकेलो_back({"", Argv.आकार_2()});

  // Don't cache Argv.size() because it can change.
  क्रम (अचिन्हित I = 0; I != Argv.आकार_2();) {
    जबतक (I == File_ढेर.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ढेर.निकालो_back();
    }

    स्थिर अक्षर *Arg = Argv[I];
    // Check if it is an EOL marker
    अगर (Arg == nullptr) {
      ++I;
      जारी;
    }

    अगर (Arg[0] != '@') {
      ++I;
      जारी;
    }

    स्थिर अक्षर *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    अगर (llvm::sys::path::is_relative(FName)) {
      अगर (CurrentDir.empty()) {
        अगर (स्वतः CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अन्यथा {
          वापस createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अन्यथा {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    अगर (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      अगर (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        अगर (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          जारी;
        }
      }
      अगर (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      वापस createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    स्थिर llvm::vfs::Status &FileStatus = Res.लो();

    स्वतः IsEquivalent =
        [FileStatus, यह](स्थिर ResponseFileRecord &RFile) -> ErrorOr<bool> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      अगर (!RHS)
        वापस RHS.getError();
      वापस FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    क्रम (स्थिर स्वतः &F : drop_begin(File_ढेर)) {
      अगर (ErrorOr<bool> R = IsEquivalent(F)) {
        अगर (R.लो())
          वापस createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अन्यथा {
        वापस createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<स्थिर अक्षर *, 0> ExpandedArgv;
    अगर (Error Err = expandResponseFile(FName, ExpandedArgv))
      वापस Err;

    क्रम (ResponseFileRecord &Record : File_ढेर) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.आकार_2() - 1;
    }

    File_ढेर.धकेलो_back({FName, I + ExpandedArgv.आकार_2()});
    Argv.erase(Argv.begin() + I);
    Argv.जोड़ो(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  निश्चित(File_ढेर.आकार_2() > 0 && Argv.आकार_2() == File_ढेर.back().End);
  वापस Error::success();
}

bool cl::expandResponseFiles(पूर्णांक Argc, स्थिर अक्षर *स्थिर *Argv,
                             स्थिर अक्षर *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<स्थिर अक्षर *> &NewArgv) {
#अगर_घोषित _WIN32
  स्वतः Tokenize = cl::Tokenize_Windows_Command_पंक्ति;
#अन्यथा
  स्वतः Tokenize = cl::Tokenize_GNUCommand_पंक्ति;
#पूर्ण_अगर
  // The environment variable specifies initial options.
  अगर (EnvVar)
    अगर (std::optional<std::string> Env_मूल्य = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मूल्य, Saver, NewArgv, /*MarkEOLs=*/false);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  अगर (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    वापस false;
  }
  वापस true;
}

bool cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<स्थिर अक्षर *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  अगर (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    वापस false;
  }
  वापस true;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().लो()) {}

bool ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<अक्षर> &FilePath) {
  SmallString<128> CfgFilePath;
  स्थिर स्वतः FileExists = [यह](SmallString<128> Path) -> bool {
    स्वतः Status = FS->status(Path);
    वापस Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  अगर (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    अगर (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      वापस false;
    अगर (!FileExists(CfgFilePath))
      वापस false;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    वापस true;
  }

  // Look for the file in search directories.
  क्रम (स्थिर StringRef &Dir : खोज_Dirs) {
    अगर (Dir.empty())
      जारी;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    अगर (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      वापस true;
    }
  }

  वापस false;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<स्थिर अक्षर *> &Argv) {
  SmallString<128> AbsPath;
  अगर (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    अगर (std::error_code EC = FS->makeAbsolute(AbsPath))
      वापस make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = true;
  RelativeNames = true;
  अगर (Error Err = expandResponseFile(CfgFile, Argv))
    वापस Err;
  वापस expandResponseFiles(Argv);
}

अटल व्योम initCommonOptions();
अटल bool parse_Command_पंक्ति_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<स्थिर अक्षर *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        स्थिर अक्षर *EnvVar) {
  initCommonOptions();
  SmallVector<स्थिर अक्षर *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.धकेलो_back(Args[0]);

  // Parse options from environment variable.
  अगर (EnvVar) {
    अगर (std::optional<std::string> Env_मूल्य =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_पंक्ति(*Env_मूल्य, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  पूर्णांक NewArgc = static_cast<पूर्णांक>(NewArgv.आकार_2());

  // Parse all options.
  वापस globalParser().Parse_Command_पंक्ति_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

bool cl::Parse_Command_पंक्ति_Options(पूर्णांक argc, स्थिर अक्षर *स्थिर *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, स्थिर अक्षर *EnvVar) {
  वापस parse_Command_पंक्ति_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

bool cl::parse_Command_पंक्ति_Options(LibraryOptionsContext &Context,
                                 ArrayRef<स्थिर अक्षर *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, स्थिर अक्षर *EnvVar) {
  वापस parse_Command_पंक्ति_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
व्योम Command_पंक्ति_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  क्रम (स्वतः *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.आकार_2());
    क्रम (स्वतः &O : SC->OptionsMap)
      Opts.धकेलो_back(O.second);
    क्रम (Option *O : Opts)
      O->reset();
    क्रम (Option *O : SC->PositionalOpts)
      O->reset();
    अगर (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  क्रम (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

bool Command_पंक्ति_Parser::Parse_Command_पंक्ति_Options(
    पूर्णांक argc, स्थिर अक्षर *स्थिर *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  निश्चित(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  bool IgnoreErrors = Errs;
  अगर (!Errs)
    Errs = &errs();
  अगर (!VFS)
    VFS = vfs::getRealFileSystem().लो();
  bool ErrorParsing = false;

  // Expand response files.
  SmallVector<स्थिर अक्षर *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#अगर_घोषित _WIN32
  स्वतः Tokenize = cl::Tokenize_Windows_Command_पंक्ति;
#अन्यथा
  स्वतः Tokenize = cl::Tokenize_GNUCommand_पंक्ति;
#पूर्ण_अगर
  ExpansionContext ECtx(A, Tokenize, VFS);
  अगर (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    वापस false;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णांक>(newArgv.आकार_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  सूचकांक_Library_Options();

  // Check out the positional arguments to collect information about them.
  अचिन्हित अंक_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  bool HasUnlimitedPositionals = false;

  पूर्णांक FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शीर्ष_Level();
  std::string NearestSubCommandString;
  bool MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  अगर (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    अगर (ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  निश्चित(ChosenSubCommand);
  स्वतः &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  स्वतः &PositionalOpts = ChosenSubCommand->PositionalOpts;
  स्वतः &OptionsMap = ChosenSubCommand->OptionsMap;

  अगर (ConsumeAfterOpt) {
    निश्चित(PositionalOpts.आकार_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  अगर (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    bool UnboundedFound = false;
    क्रम (माप_प्रकार i = 0, e = PositionalOpts.आकार_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      अगर (Requires_मूल्य(Opt))
        ++अंक_Positional_Required;
      अन्यथा अगर (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        अगर (PositionalOpts.आकार_2() > 1) {
          अगर (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = true;
        }
      } अन्यथा अगर (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        अगर (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.आकार_2();
        ErrorParsing = true;
      }
      UnboundedFound |= Eats_Unbounded_संख्या_Of_मूल्यावली(Opt);
    }
    HasUnlimitedPositionals = UnboundedFound || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  SmallVector<std::pair<StringRef, अचिन्हित>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *ActivePositionalArg = nullptr;

  // Loop over all of the arguments... processing them.
  bool DashDashFound = false; // Have we read '--'?
  क्रम (पूर्णांक i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef मूल्य;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    अगर (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      अगर (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        जारी; // We are done!
      }

      अगर (!PositionalOpts.empty()) {
        PositionalVals.धकेलो_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        अगर (PositionalVals.आकार_2() >= अंक_Positional_Required && ConsumeAfterOpt) {
          क्रम (++i; i < argc; ++i)
            PositionalVals.धकेलो_back(std::make_pair(StringRef(argv[i]), i));
          अवरोध; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        जारी;
      }
    } अन्यथा अगर (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = true; // This is the mythical "--"?
      जारी;             // Don't try to process it as an argument itself.
    } अन्यथा अगर (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);
      अगर (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        जारी; // We are done!
      }
    } अन्यथा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      अगर (!Handler && ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
        Handler = LookupOption(SubCommand::get_शीर्ष_Level(), ArgName, मूल्य);

      अगर (!Handler) {
        अगर (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<स्थिर अक्षर *> Rest(argv + i, argc - i);
          अचिन्हित N;
          अगर (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = true;
          }
          i += N - 1;
          जारी;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      अगर (!Handler)
        Handler = HandlePrefixedOption(ArgName, मूल्य, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      अगर (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    अगर (!Handler) {
      स्वतः ReportUnknownArgument = [&](bool IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        अगर (NearestArgumentName.empty())
          वापस;

        *Errs << ProgramName << ": Did you mean '";
        अगर (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        अन्यथा
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      अगर (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/true, NearestHandlerString);
      अन्यथा
        ReportUnknownArgument(/*IsArg=*/false, NearestSubCommandString);

      ErrorParsing = true;
      जारी;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    अगर (Handler->getFormattingFlag() == cl::Positional) {
      अगर ((Handler->getMiscFlags() & PositionalEatsArgs) && !मूल्य.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = true;
      }
      ActivePositionalArg = Handler;
    }
    अन्यथा
      ErrorParsing |= ProvideOption(Handler, ArgName, मूल्य, argc, argv, i);
  }

  // Check and handle positional arguments now...
  अगर (अंक_Positional_Required > PositionalVals.आकार_2()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अंक_Positional_Required
             << " positional argument" << (अंक_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = true;
  } अन्यथा अगर (!HasUnlimitedPositionals &&
             PositionalVals.आकार_2() > PositionalOpts.आकार_2()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.आकार_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = true;

  } अन्यथा अगर (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    अचिन्हित ValNo = 0, अंक_Vals = static_cast<अचिन्हित>(PositionalVals.आकार_2());
    क्रम (Option *Opt : PositionalOpts) {
      अगर (Requires_मूल्य(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --अंक_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      bool Done = Opt->get_अंक_Occurrences_Flag() == cl::Required;
      जबतक (अंक_Vals - ValNo > अंक_Positional_Required && !Done) {
        चयन (Opt->get_अंक_Occurrences_Flag()) {
        हाल cl::Optional:
          Done = true; // Optional arguments want _at most_ one value
          [[fallthrough]];
        हाल cl::ZeroOrMore: // Zero or more will take all they can get...
        हाल cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          अवरोध;
        शेष:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } अन्यथा {
    निश्चित(ConsumeAfterOpt && अंक_Positional_Required <= PositionalVals.आकार_2());
    अचिन्हित ValNo = 0;
    क्रम (Option *Opt : PositionalOpts)
      अगर (Requires_मूल्य(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    अगर (PositionalOpts.आकार_2() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    क्रम (; ValNo != PositionalVals.आकार_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  क्रम (स्थिर स्वतः &Opt : OptionsMap) {
    चयन (Opt.second->get_अंक_Occurrences_Flag()) {
    हाल Required:
    हाल OneOrMore:
      अगर (Opt.second->get_अंक_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = true;
      }
      [[fallthrough]];
    शेष:
      अवरोध;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             क्रम (पूर्णांक i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.सफा();

  // If we had an error processing our arguments, don't let the program execute
  अगर (ErrorParsing) {
    अगर (!IgnoreErrors)
      निकास(1);
    वापस false;
  }
  वापस true;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

bool Option::error(स्थिर Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  अगर (!ArgName.तथ्य())
    ArgName = ArgStr;
  अगर (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  अन्यथा
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  वापस true;
}

bool Option::addOccurrence(अचिन्हित pos, StringRef ArgName, StringRef मूल्य) {
  ++अंक_Occurrences;
  वापस handleOccurrence(pos, ArgName, मूल्य);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
अटल StringRef get_मूल्य_Str(स्थिर Option &O, StringRef Default_संदेश) {
  अगर (O.मूल्य_Str.empty())
    वापस Default_संदेश;
  वापस O.मूल्य_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
माप_प्रकार alias::getOptionWidth() स्थिर {
  वापस arg_Plus_Prefixes_आकार(ArgStr);
}

व्योम Option::printHelpStr(StringRef HelpStr, माप_प्रकार Indent,
                          माप_प्रकार First_पंक्ति_Indented_By) {
  निश्चित(Indent >= First_पंक्ति_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  म_दो().indent(Indent - First_पंक्ति_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  जबतक (!Split.second.empty()) {
    Split = Split.second.split('\n');
    म_दो().indent(Indent) << Split.first << "\n";
  }
}

व्योम Option::printEnumValHelpStr(StringRef HelpStr, माप_प्रकार BaseIndent,
                                 माप_प्रकार First_पंक्ति_Indented_By) {
  स्थिर StringRef ValHelpPrefix = "  ";
  निश्चित(BaseIndent >= First_पंक्ति_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  म_दो().indent(BaseIndent - First_पंक्ति_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  जबतक (!Split.second.empty()) {
    Split = Split.second.split('\n');
    म_दो().indent(BaseIndent + ValHelpPrefix.आकार_2()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
व्योम alias::printOptionInfo(माप_प्रकार GlobalWidth) स्थिर {
  म_दो() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, arg_Plus_Prefixes_आकार(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
माप_प्रकार basic_parser_impl::getOptionWidth(स्थिर Option &O) स्थिर {
  माप_प्रकार Len = arg_Plus_Prefixes_आकार(O.ArgStr);
  स्वतः ValName = get_मूल्य_Name();
  अगर (!ValName.empty()) {
    माप_प्रकार FormattingLen = 3;
    अगर (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_मूल्य_Str(O, ValName).आकार_2() + FormattingLen;
  }

  वापस Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
व्योम basic_parser_impl::printOptionInfo(स्थिर Option &O,
                                        माप_प्रकार GlobalWidth) स्थिर {
  म_दो() << PrintArg(O.ArgStr);

  स्वतः ValName = get_मूल्य_Name();
  अगर (!ValName.empty()) {
    अगर (O.getMiscFlags() & PositionalEatsArgs) {
      म_दो() << " <" << get_मूल्य_Str(O, ValName) << ">...";
    } अन्यथा अगर (O.get_मूल्य_Expected_Flag() == मूल्य_Optional)
      म_दो() << "[=<" << get_मूल्य_Str(O, ValName) << ">]";
    अन्यथा {
      म_दो() << (O.ArgStr.आकार_2() == 1 ? " <" : "=<") << get_मूल्य_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

व्योम basic_parser_impl::printOptionName(स्थिर Option &O,
                                        माप_प्रकार GlobalWidth) स्थिर {
  म_दो() << PrintArg(O.ArgStr);
  म_दो().indent(GlobalWidth - O.ArgStr.आकार_2());
}

// parser<bool> implementation
//
bool parser<bool>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         bool &मूल्य) {
  वापस parseBool<bool, true, false>(O, ArgName, Arg, मूल्य);
}

// parser<boolOrDefault> implementation
//
bool parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मूल्य) {
  वापस parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, मूल्य);
}

// parser<FixedOrScalableQuantity> implementation
//
ढाँचा <typename FixedOrScalableQuantityT>
अटल bool parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef मूल्य_Kind,
                                         FixedOrScalableQuantityT &मूल्य) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_मूल्य;
  अगर (!Arg.getAsInteger(0, Min_मूल्य)) {
    मूल्य = FixedOrScalableQuantityT::getFixed(Min_मूल्य);
    वापस false;
  }

  StringRef Remainder = Arg;
  अगर (!Remainder.consume_front("vscale"))
    वापस O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  अगर (!Remainder.consume_front('x'))
    वापस O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  अगर (Remainder.getAsInteger(0, Min_मूल्य))
    वापस O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  मूल्य = FixedOrScalableQuantityT::getScalable(Min_मूल्य);
  वापस false;
}

// parser<int> implementation
//
bool parser<पूर्णांक>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णांक &मूल्य) {
  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for integer argument!");
  वापस false;
}

// parser<long> implementation
//
bool parser<दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         दीर्घ &मूल्य) {
  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for long argument!");
  वापस false;
}

// parser<long long> implementation
//
bool parser<दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              दीर्घ दीर्घ &मूल्य) {
  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for llong argument!");
  वापस false;
}

// parser<unsigned> implementation
//
bool parser<अचिन्हित>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             अचिन्हित &मूल्य) {

  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for uint argument!");
  वापस false;
}

// parser<unsigned long> implementation
//
bool parser<अचिन्हित दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  अचिन्हित दीर्घ &मूल्य) {

  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for ulong argument!");
  वापस false;
}

// parser<unsigned long long> implementation
//
bool parser<अचिन्हित दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       अचिन्हित दीर्घ दीर्घ &मूल्य) {

  अगर (Arg.getAsInteger(0, मूल्य))
    वापस O.error("'" + Arg + "' value invalid for ullong argument!");
  वापस false;
}

// parser<ElementCount> implementation
//
bool parser<Element_गिनती>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_गिनती &मूल्य) {
  वापस parseFixedOrScalableQuantity(O, Arg, get_मूल्य_Name(), मूल्य);
}

// parser<double>/parser<float> implementation
//
अटल bool parseDouble(Option &O, StringRef Arg, द्विगुन &मूल्य) {
  अगर (to_float(Arg, मूल्य))
    वापस false;
  वापस O.error("'" + Arg + "' value invalid for floating point argument!");
}

bool parser<द्विगुन>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुन &Val) {
  वापस parseDouble(O, Arg, Val);
}

bool parser<भग्न>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          भग्न &Val) {
  द्विगुन dVal;
  अगर (parseDouble(O, Arg, dVal))
    वापस true;
  Val = (भग्न)dVal;
  वापस false;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
अचिन्हित generic_parser_base::findOption(StringRef Name) {
  अचिन्हित e = get_अंक_Options();

  क्रम (अचिन्हित i = 0; i != e; ++i) {
    अगर (getOption(i) == Name)
      वापस i;
  }
  वापस e;
}

अटल StringRef Eq_मूल्य = "=<value>";
अटल StringRef EmptyOption = "<empty>";
अटल StringRef OptionPrefix = "    =";
अटल माप_प्रकार get_Option_Prefixes_आकार() {
  वापस OptionPrefix.आकार_2() + ArgHelpPrefix.आकार_2();
}

अटल bool shouldPrintOption(StringRef Name, StringRef Description,
                              स्थिर Option &O) {
  वापस O.get_मूल्य_Expected_Flag() != मूल्य_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
माप_प्रकार generic_parser_base::getOptionWidth(स्थिर Option &O) स्थिर {
  अगर (O.hasArgStr()) {
    माप_प्रकार आकार =
        arg_Plus_Prefixes_आकार(O.ArgStr) + Eq_मूल्य.आकार_2();
    क्रम (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      अगर (!shouldPrintOption(Name, getDescription(i), O))
        जारी;
      माप_प्रकार Name_आकार = Name.empty() ? EmptyOption.आकार_2() : Name.आकार_2();
      आकार = std::max(आकार, Name_आकार + get_Option_Prefixes_आकार());
    }
    वापस आकार;
  } अन्यथा {
    माप_प्रकार Base_आकार = 0;
    क्रम (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i)
      Base_आकार = std::max(Base_आकार, getOption(i).आकार_2() + 8);
    वापस Base_आकार;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
व्योम generic_parser_base::printOptionInfo(स्थिर Option &O,
                                          माप_प्रकार GlobalWidth) स्थिर {
  अगर (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    अगर (O.get_मूल्य_Expected_Flag() == मूल्य_Optional) {
      क्रम (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
        अगर (getOption(i).empty()) {
          म_दो() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               arg_Plus_Prefixes_आकार(O.ArgStr));
          अवरोध;
        }
      }
    }

    म_दो() << PrintArg(O.ArgStr) << Eq_मूल्य;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_मूल्य.आकार_2() +
                             arg_Plus_Prefixes_आकार(O.ArgStr));
    क्रम (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      अगर (!shouldPrintOption(OptionName, Description, O))
        जारी;
      माप_प्रकार First_पंक्ति_Indent = OptionName.आकार_2() + get_Option_Prefixes_आकार();
      म_दो() << OptionPrefix << OptionName;
      अगर (OptionName.empty()) {
        म_दो() << EmptyOption;
        निश्चित(First_पंक्ति_Indent >= EmptyOption.आकार_2());
        First_पंक्ति_Indent += EmptyOption.आकार_2();
      }
      अगर (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_पंक्ति_Indent);
      अन्यथा
        म_दो() << '\n';
    }
  } अन्यथा {
    अगर (!O.HelpStr.empty())
      म_दो() << "  " << O.HelpStr << '\n';
    क्रम (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      म_दो() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.आकार_2() + 8);
    }
  }
}

अटल स्थिर माप_प्रकार MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
व्योम generic_parser_base::printGenericOptionDiff(
    स्थिर Option &O, स्थिर Generic_Option_मूल्य &मूल्य,
    स्थिर Generic_Option_मूल्य &Default, माप_प्रकार GlobalWidth) स्थिर {
  म_दो() << "  " << PrintArg(O.ArgStr);
  म_दो().indent(GlobalWidth - O.ArgStr.आकार_2());

  अचिन्हित अंक_Opts = get_अंक_Options();
  क्रम (अचिन्हित i = 0; i != अंक_Opts; ++i) {
    अगर (!मूल्य.compare(get_Option_मूल्य(i)))
      जारी;

    म_दो() << "= " << getOption(i);
    माप_प्रकार L = getOption(i).आकार_2();
    माप_प्रकार अंक_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    म_दो().indent(अंक_Spaces) << " (default: ";
    क्रम (अचिन्हित j = 0; j != अंक_Opts; ++j) {
      अगर (!Default.compare(get_Option_मूल्य(j)))
        जारी;
      म_दो() << getOption(j);
      अवरोध;
    }
    म_दो() << ")\n";
    वापस;
  }
  म_दो() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
namespace llvm {
namespace cl {
अटल raw_ostream &चालक<<(raw_ostream &OS, boolOrDefault V) {
  वापस OS << static_cast<पूर्णांक>(V);
}
} // namespace cl
} // namespace llvm

#घोषणा PRINT_OPT_DIFF(T)                                                      \
  व्योम parser<T>::printOptionDiff(स्थिर Option &O, T V, Option_मूल्य<T> D,      \
                                  माप_प्रकार GlobalWidth) स्थिर {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    म_दो() << "= " << Str;                                                     \
    माप_प्रकार अंक_Spaces =                                                         \
        MaxOptWidth > Str.आकार_2() ? MaxOptWidth - Str.आकार_2() : 0;               \
    म_दो().indent(अंक_Spaces) << " (default: ";                                 \
    अगर (D.has_मूल्य())                                                          \
      म_दो() << D.get_मूल्य();                                                  \
    अन्यथा                                                                       \
      म_दो() << "*no default*";                                                \
    म_दो() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(bool)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(पूर्णांक)
PRINT_OPT_DIFF(दीर्घ)
PRINT_OPT_DIFF(दीर्घ दीर्घ)
PRINT_OPT_DIFF(अचिन्हित)
PRINT_OPT_DIFF(अचिन्हित दीर्घ)
PRINT_OPT_DIFF(अचिन्हित दीर्घ दीर्घ)
PRINT_OPT_DIFF(द्विगुन)
PRINT_OPT_DIFF(भग्न)
PRINT_OPT_DIFF(अक्षर)
PRINT_OPT_DIFF(Element_गिनती)

व्योम parser<std::string>::printOptionDiff(स्थिर Option &O, StringRef V,
                                          स्थिर Option_मूल्य<std::string> &D,
                                          माप_प्रकार GlobalWidth) स्थिर {
  printOptionName(O, GlobalWidth);
  म_दो() << "= " << V;
  माप_प्रकार अंक_Spaces = MaxOptWidth > V.आकार_2() ? MaxOptWidth - V.आकार_2() : 0;
  म_दो().indent(अंक_Spaces) << " (default: ";
  अगर (D.has_मूल्य())
    म_दो() << D.get_मूल्य();
  अन्यथा
    म_दो() << "*no default*";
  म_दो() << ")\n";
}

व्योम parser<std::optional<std::string>>::printOptionDiff(
    स्थिर Option &O, std::optional<StringRef> V,
    स्थिर Option_मूल्य<std::optional<std::string>> &D,
    माप_प्रकार GlobalWidth) स्थिर {
  printOptionName(O, GlobalWidth);
  म_दो() << "= " << V;
  माप_प्रकार VSize = V.has_मूल्य_2() ? V.मूल्य_2().आकार_2() : 0;
  माप_प्रकार अंक_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  म_दो().indent(अंक_Spaces) << " (default: ";
  अगर (D.has_मूल्य() && D.get_मूल्य().has_मूल्य_2())
    म_दो() << D.get_मूल्य();
  अन्यथा
    म_दो() << "*no value*";
  म_दो() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
व्योम basic_parser_impl::print_Option_No_मूल्य(स्थिर Option &O,
                                           माप_प्रकार GlobalWidth) स्थिर {
  printOptionName(O, GlobalWidth);
  म_दो() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

अटल पूर्णांक OptNameCompare(स्थिर std::pair<स्थिर अक्षर *, Option *> *LHS,
                          स्थिर std::pair<स्थिर अक्षर *, Option *> *RHS) {
  वापस म_भेद(LHS->first, RHS->first);
}

अटल पूर्णांक SubNameCompare(स्थिर std::pair<स्थिर अक्षर *, SubCommand *> *LHS,
                          स्थिर std::pair<स्थिर अक्षर *, SubCommand *> *RHS) {
  वापस म_भेद(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
अटल व्योम sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<स्थिर अक्षर *, Option *>> &Opts,
                     bool ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  क्रम (स्वतः I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    अगर (I->second->getOptionHiddenFlag() == ReallyHidden)
      जारी;

    // Unless showhidden is set, ignore hidden flags.
    अगर (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      जारी;

    // If we've already seen this option, don't add it to the list again.
    अगर (!OptionSet.जोड़ो(I->second).second)
      जारी;

    Opts.धकेलो_back(
        std::pair<स्थिर अक्षर *, Option *>(I->first.तथ्य(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

अटल व्योम
sortSubCommands(स्थिर SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<स्थिर अक्षर *, SubCommand *>> &Subs) {
  क्रम (स्वतः *S : SubMap) {
    अगर (S->getName().empty())
      जारी;
    Subs.धकेलो_back(std::make_pair(S->getName().तथ्य(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

namespace {

श्रेणी HelpPrinter {
रक्षित:
  स्थिर bool ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<स्थिर अक्षर *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<स्थिर अक्षर *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  भव व्योम printOptions(StrOptionPairVector &Opts, माप_प्रकार MaxArgLen) {
    क्रम (स्थिर स्वतः &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  व्योम printSubCommands(StrSubCommandPairVector &Subs, माप_प्रकार MaxSubLen) {
    क्रम (स्थिर स्वतः &S : Subs) {
      म_दो() << "  " << S.first;
      अगर (!S.second->getDescription().empty()) {
        म_दो().indent(MaxSubLen - म_माप(S.first));
        म_दो() << " - " << S.second->getDescription();
      }
      म_दो() << "\n";
    }
  }

खुला:
  explicit HelpPrinter(bool showHidden) : ShowHidden(showHidden) {}
  भव ~HelpPrinter() = शेष;

  // Invoke the printer.
  व्योम चालक=(bool मूल्य) {
    अगर (!मूल्य)
      वापस;
    printHelp();

    // Halt the program since help information was printed
    निकास(0);
  }

  व्योम printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    स्वतः &OptionsMap = Sub->OptionsMap;
    स्वतः &PositionalOpts = Sub->PositionalOpts;
    स्वतः &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    अगर (!globalParser().ProgramOverview.empty())
      म_दो() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    अगर (Sub == &SubCommand::get_शीर्ष_Level()) {
      म_दो() << "USAGE: " << globalParser().ProgramName;
      अगर (!Subs.empty())
        म_दो() << " [subcommand]";
      म_दो() << " [options]";
    } अन्यथा {
      अगर (!Sub->getDescription().empty()) {
        म_दो() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      म_दो() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    क्रम (स्वतः *Opt : PositionalOpts) {
      अगर (Opt->hasArgStr())
        म_दो() << " --" << Opt->ArgStr;
      म_दो() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    अगर (ConsumeAfterOpt)
      म_दो() << " " << ConsumeAfterOpt->HelpStr;

    अगर (Sub == &SubCommand::get_शीर्ष_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      माप_प्रकार MaxSubLen = 0;
      क्रम (स्थिर स्वतः &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, म_माप(Sub.first));

      म_दो() << "\n\n";
      म_दो() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      म_दो() << "\n";
      म_दो() << "  Type \"" << globalParser().ProgramName
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    म_दो() << "\n\n";

    // Compute the maximum argument length...
    माप_प्रकार MaxArgLen = 0;
    क्रम (स्थिर स्वतः &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    अगर (ShowHidden)
      क्रम (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              अगर (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    क्रम (स्थिर स्वतः &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_आकार(Name));

    म_दो() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    अगर (!LibraryOpts.empty())
      म_दो() << "\nLibrary options:\n\n";
    क्रम (स्थिर स्वतः &[Name, Help] : LibraryOpts) {
      म_दो() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_आकार(Name));
    }

    // Print any extra help the user has declared.
    क्रम (स्थिर स्वतः &I : globalParser().MoreHelp)
      म_दो() << I;
    globalParser().MoreHelp.सफा();
  }
};

श्रेणी CategorizedHelpPrinter : खुला HelpPrinter {
खुला:
  explicit CategorizedHelpPrinter(bool showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  अटल पूर्णांक OptionCategoryCompare(OptionCategory *स्थिर *A,
                                   OptionCategory *स्थिर *B) {
    वापस (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::चालक=;

रक्षित:
  व्योम printOptions(StrOptionPairVector &Opts, माप_प्रकार MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    निश्चित(SortedCategories.आकार_2() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    क्रम (स्थिर स्वतः &I : Opts) {
      Option *Opt = I.second;
      क्रम (OptionCategory *Cat : Opt->Categories) {
        निश्चित(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].धकेलो_back(Opt);
      }
    }

    // Now do printing.
    क्रम (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      स्थिर स्वतः &CategoryOptions = CategorizedOptions[Category];
      अगर (CategoryOptions.empty())
        जारी;

      // Print category information.
      म_दो() << "\n";
      म_दो() << Category->getName() << ":\n";

      // Check if description is set.
      अगर (!Category->getDescription().empty())
        म_दो() << Category->getDescription() << "\n\n";
      अन्यथा
        म_दो() << "\n";

      // Loop over the options in the category and print.
      क्रम (स्थिर Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
श्रेणी HelpPrinterWrapper {
निजी:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

खुला:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  व्योम चालक=(bool मूल्य);
};

} // End anonymous namespace

#अगर defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# अगर defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#या_अगर defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# अगर defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#अन्यथा
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#पूर्ण_अगर

namespace {
श्रेणी VersionPrinter {
खुला:
  व्योम print(स्थिर std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = म_दो();
#अगर_घोषित PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अन्यथा
    OS << "LLVM (http://llvm.org/):\n  ";
#पूर्ण_अगर
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#अगर LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अन्यथा
    OS << "Optimized build";
#पूर्ण_अगर
#अगर_अघोषित न_संशोधन
    OS << " with assertions";
#पूर्ण_अगर
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    अगर (!ExtraPrinters.empty()) {
      क्रम (स्थिर स्वतः &I : ExtraPrinters)
        I(म_दो());
    }
  }
  व्योम चालक=(bool OptionWasSpecified);
};

काष्ठा Command_पंक्ति_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{false};
  HelpPrinter UncategorizedHiddenPrinter{true};
  CategorizedHelpPrinter CategorizedNormalPrinter{false};
  CategorizedHelpPrinter CategorizedHiddenPrinter{true};
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
  cl::opt<HelpPrinter, true, parser<bool>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, true, parser<bool>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, true, parser<bool>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, true, parser<bool>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<bool> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(false),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<bool> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(false),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, true, parser<bool>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::मूल्य_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
अटल ManagedStatic<Command_पंक्ति_Common_Options> CommonOptions;

अटल व्योम initCommonOptions() {
  *CommonOptions;
  init_Debug_गणक_Options();
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
  अटल OptionCategory GeneralCategory{"General options"};
  वापस GeneralCategory;
}

व्योम VersionPrinter::चालक=(bool OptionWasSpecified) {
  अगर (!OptionWasSpecified)
    वापस;

  अगर (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(म_दो());
    निकास(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  निकास(0);
}

व्योम HelpPrinterWrapper::चालक=(bool मूल्य) {
  अगर (!मूल्य)
    वापस;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  अगर (globalParser().RegisteredOptionCategories.आकार_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = true; // Invoke categorized printer
  } अन्यथा {
    UncategorizedPrinter = true; // Invoke uncategorized printer
  }
}

// Print the value of each option.
व्योम cl::Print_Option_मूल्यावली() { globalParser().print_Option_मूल्यावली(); }

व्योम Command_पंक्ति_Parser::print_Option_मूल्यावली() {
  अगर (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    वापस;

  SmallVector<std::pair<स्थिर अक्षर *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ true);

  // Compute the maximum argument length...
  माप_प्रकार MaxArgLen = 0;
  क्रम (स्थिर स्वतः &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  क्रम (स्थिर स्वतः &Opt : Opts)
    Opt.second->print_Option_मूल्य(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
व्योम cl::PrintHelpMessage(bool Hidden, bool Categorized) {
  अगर (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  अन्यथा अगर (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  अन्यथा अगर (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अन्यथा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  अटल स्थिर StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#अगर LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#पूर्ण_अगर
#अगर_अघोषित न_संशोधन
      "+assertions",
#पूर्ण_अगर
#अगर_घोषित EXPENSIVE_CHECKS
      "+expensive-checks",
#पूर्ण_अगर
#अगर __has_feature(address_sanitizer)
      "+asan",
#पूर्ण_अगर
#अगर __has_feature(dataflow_sanitizer)
      "+dfsan",
#पूर्ण_अगर
#अगर __has_feature(hwaddress_sanitizer)
      "+hwasan",
#पूर्ण_अगर
#अगर __has_feature(memory_sanitizer)
      "+msan",
#पूर्ण_अगर
#अगर __has_feature(thread_sanitizer)
      "+tsan",
#पूर्ण_अगर
#अगर __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#पूर्ण_अगर
#अगर_घोषित LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#पूर्ण_अगर
  };
  वापस ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
व्योम cl::printBuildConfig(raw_ostream &OS) {
#अगर LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#पूर्ण_अगर
}

/// Utility function for printing version number.
व्योम cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

व्योम cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

व्योम cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.धकेलो_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  स्वतः &Subs = globalParser().RegisteredSubCommands;
  (व्योम)Subs;
  निश्चित(Subs.contains(&Sub));
  वापस Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  वापस globalParser().getRegisteredSubcommands();
}

व्योम cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  क्रम (स्वतः &I : Sub.OptionsMap) {
    bool Unrelated = true;
    क्रम (स्वतः &Cat : I.second->Categories) {
      अगर (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    अगर (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

व्योम cl::HideUnrelatedOptions(ArrayRef<स्थिर cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  क्रम (स्वतः &I : Sub.OptionsMap) {
    bool Unrelated = true;
    क्रम (स्वतः &Cat : I.second->Categories) {
      अगर (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    अगर (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

व्योम cl::Reset_Command_पंक्ति_Parser() { globalParser().reset(); }

व्योम cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.धकेलो_back(&L);
}

व्योम LibraryOptionsContext::assign(स्थिर LibraryOptionsContext &Other) {
  निश्चित(यह != &Other);
  मूल्यावली.सफा();
  SavedArgs.सफा();
  ArrayRef<स्थिर अक्षर *> Args = Other.SavedArgs;
  क्रम (माप_प्रकार I = 0, E = Args.आकार_2(); I != E;) {
    StringRef Name = Args[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.split('=').first);
    अचिन्हित N;
    cantFail(parse(*L, Args.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<स्थिर अक्षर *> Args,
                                   अचिन्हित &Consumed) {
  स्वतः [It, Inserted] = मूल्यावली.try_emplace(L.getDefault());
  अगर (Inserted)
    It->second = L.copyDefault(Alloc);
  वापस parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

व्योम cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

व्योम LLVMParse_Command_पंक्ति_Options(पूर्णांक argc, स्थिर अक्षर *स्थिर *argv,
                                 स्थिर अक्षर *Overview) {
  llvm::cl::Parse_Command_पंक्ति_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
