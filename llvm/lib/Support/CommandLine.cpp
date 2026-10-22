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
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<तार्किक>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<पूर्णांक>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ दीर्घ>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित दीर्घ>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिन्हित दीर्घ दीर्घ>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<द्विगुण>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<दशांश>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अक्षर>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE basic_parser<Element_मोजणी>;

#जर !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

ढाँचा वर्ग LLVM_EXPORT_TEMPLATE opt<तार्किक>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE opt<अक्षर>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE opt<पूर्णांक>;
ढाँचा वर्ग LLVM_EXPORT_TEMPLATE opt<अचिन्हित>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
शून्य Generic_Option_मूल्य::anchor() {}
शून्य Option_मूल्य<boolOrDefault>::anchor() {}
शून्य Option_मूल्य<std::string>::anchor() {}
शून्य Option::anchor() {}
शून्य basic_parser_impl::anchor() {}
शून्य parser<तार्किक>::anchor() {}
शून्य parser<boolOrDefault>::anchor() {}
शून्य parser<पूर्णांक>::anchor() {}
शून्य parser<दीर्घ>::anchor() {}
शून्य parser<दीर्घ दीर्घ>::anchor() {}
शून्य parser<अचिन्हित>::anchor() {}
शून्य parser<अचिन्हित दीर्घ>::anchor() {}
शून्य parser<अचिन्हित दीर्घ दीर्घ>::anchor() {}
शून्य parser<द्विगुण>::anchor() {}
शून्य parser<दशांश>::anchor() {}
शून्य parser<std::string>::anchor() {}
शून्य parser<std::optional<std::string>>::anchor() {}
शून्य parser<अक्षर>::anchor() {}
शून्य parser<Element_मोजणी>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
शून्य opt_bool_anchor() { opt<तार्किक> anchor{""}; }
शून्य opt_char_anchor() { opt<अक्षर> anchor{""}; }
शून्य opt_int_anchor() { opt<पूर्णांक> anchor{""}; }
शून्य opt_unsigned_anchor() { opt<अचिन्हित> anchor{""}; }

//===----------------------------------------------------------------------===//

स्थिरांक स्थिर माप_प्रकार DefaultPad = 2;

स्थिर StringRef ArgPrefix = "-";
स्थिर StringRef ArgPrefixLong = "--";
स्थिर StringRef ArgHelpPrefix = " - ";

स्थिर माप_प्रकार argPlusPrefixesSize(StringRef ArgName, माप_प्रकार Pad = DefaultPad) {
  माप_प्रकार Len = ArgName.size();
  जर (Len == 1)
    परत Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  परत Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

स्थिर SmallString<8> argPrefix(StringRef ArgName, माप_प्रकार Pad = DefaultPad) {
  SmallString<8> Prefix;
  साठी (माप_प्रकार I = 0; I < Pad; ++I) {
    Prefix.ढकला_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  परत Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

नामाकाश {

वर्ग PrintArg {
  StringRef ArgName;
  माप_प्रकार Pad;
सार्वजनिक:
  PrintArg(StringRef ArgName, माप_प्रकार Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  मित्र raw_ostream &चालक<<(raw_ostream &OS, स्थिरांक PrintArg &);
};

raw_ostream &चालक<<(raw_ostream &OS, स्थिरांक PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  परत OS;
}

वर्ग Command_ओळ_Parser {
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
  माप_प्रकार अंक_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ओळ_Parser() { registerSubCommand(&SubCommand::get_शीर्ष_Level()); }

  शून्य ResetAllOptionOccurrences();

  तार्किक Parse_Command_ओळ_Options(पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  शून्य forEachSubCommand(Option &Opt, function_ref<शून्य(SubCommand &)> Action) {
    जर (Opt.Subs.empty()) {
      Action(SubCommand::get_शीर्ष_Level());
      परत;
    }
    जर (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      साठी (स्वतः *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      परत;
    }
    साठी (स्वतः *SC : Opt.Subs) {
      प्रतिज्ञा(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  शून्य addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    जर (Opt.hasArgStr())
      परत;
    जर (!SC->OptionsMap.घाला(std::make_pair(Name, &Opt)).second) {
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
    तार्किक HadErrors = असत्य;
    जर (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      जर (!SC->OptionsMap.घाला(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = सत्य;
      }
    }

    // Remember information about positional options.
    जर (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ढकला_back(O);
    अन्यथा जर (O->get_अंक_Occurrences_Flag() == cl::ConsumeAfter) {
      जर (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = सत्य;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    जर (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  शून्य addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  शून्य removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    जर (O->hasArgStr())
      OptionNames.ढकला_back(O->ArgStr);

    SubCommand &Sub = *SC;
    साठी (स्वतः Name : OptionNames) {
      स्वतः I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      जर (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    जर (O->getFormattingFlag() == cl::Positional)
      साठी (स्वतः *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        जर (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          थांबा;
        }
      }
    अन्यथा जर (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  शून्य removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  तार्किक hasOptions(स्थिरांक SubCommand &Sub) स्थिरांक {
    परत (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  तार्किक hasOptions() स्थिरांक {
    साठी (स्थिरांक स्वतः *S : RegisteredSubCommands) {
      जर (hasOptions(*S))
        परत सत्य;
    }
    परत !Libraries.empty();
  }

  तार्किक hasNamedSubCommands() स्थिरांक {
    साठी (स्थिरांक स्वतः *S : RegisteredSubCommands)
      जर (!S->getName().empty())
        परत सत्य;
    परत असत्य;
  }

  SubCommand *getActiveSubCommand() { परत ActiveSubCommand; }

  शून्य updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    जर (!Sub.OptionsMap.घाला(std::make_pair(NewName, O)).second) {
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

  शून्य print_Option_मूल्ये();

  शून्य indexLibraryOptions() {
    तार्किक HadErrors = असत्य;
    साठी (; अंक_Indexed_Libraries != Libraries.size(); ++अंक_Indexed_Libraries) {
      LibraryOptions *L = Libraries[अंक_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        स्वतः [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        जर (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 परत SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          परत;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = सत्य;
      });
    }
    जर (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    परत LibraryIndex.lookup(Name);
  }

  शून्य registerCategory(OptionCategory *cat) {
    प्रतिज्ञा(मोजणी_if(RegisteredOptionCategories,
                    [cat](स्थिरांक OptionCategory *Category) {
             परत cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.घाला(cat);
  }

  शून्य registerSubCommand(SubCommand *sub) {
    प्रतिज्ञा(मोजणी_if(RegisteredSubCommands,
                    [sub](स्थिरांक SubCommand *Sub) {
                      परत (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.घाला(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    प्रतिज्ञा(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    साठी (स्वतः &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      जर (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      अन्यथा
        addLiteralOption(*O, sub, E.first);
    }
  }

  शून्य unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    परत make_range(RegisteredSubCommands.begin(),
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
    अंक_Indexed_Libraries = 0;

    SubCommand::get_शीर्ष_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_शीर्ष_Level());
  }

खाजगी:
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
स्थिर Command_ओळ_Parser &globalParser() {
  स्थिर ManagedStatic<Command_ओळ_Parser> GlobalParser;
  परत *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
स्थिर Error
parseLibraryOption(LibraryOptions &L, शून्य *Opts, ArrayRef<स्थिरांक अक्षर *> Args,
                   अचिन्हित &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<स्थिरांक अक्षर *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  स्थिरांक अक्षर *Saved[2];
  अचिन्हित अंक_Args = std::min<माप_प्रकार>(Args.size(), 2);
  साठी (अचिन्हित J = 0; J != अंक_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  जर (Error E = L.parse(Opts, ArrayRef(Saved, अंक_Args), Consumed, Alloc))
    परत E;
  जर (Record)
    Record->append(Saved, Saved + Consumed);
  परत Error::success();
}

ढाँचा <typename T, T TrueVal, T FalseVal>
स्थिर तार्किक parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मूल्य) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  जर (!Arg.data() || Arg == "true" || Arg == "1") {
    मूल्य = TrueVal;
    परत असत्य;
  }

  जर (Arg == "false" || Arg == "0") {
    मूल्य = FalseVal;
    परत असत्य;
  }
  परत O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

शून्य cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ढकला_back(Help);
}

Option::Option(अंक_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : अंक_Occurrences(0), Occurrences(OccurrencesFlag), मूल्य(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(असत्य), Position(0) {
  Categories.ढकला_back(&getGeneralCategory());
}

शून्य Option::addArgument() {
  globalParser().addOption(हे);
  FullyInitialized = सत्य;
}

शून्य Option::removeArgument() { globalParser().removeOption(हे); }

शून्य Option::setArgStr(StringRef S) {
  जर (FullyInitialized)
    globalParser().updateArgStr(हे, S);
  प्रतिज्ञा(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

शून्य Option::addCategory(OptionCategory &C) {
  प्रतिज्ञा(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  जर (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अन्यथा जर (!is_contained(Categories, &C))
    Categories.ढकला_back(&C);
}

शून्य Option::reset() {
  अंक_Occurrences = 0;
  setDefault();
}

शून्य OptionCategory::registerCategory() {
  globalParser().registerCategory(हे);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_शीर्ष_Level() {
  स्थिर ManagedStatic<SubCommand> शीर्ष_Level_Sub_Command;
  परत *शीर्ष_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  स्थिर ManagedStatic<SubCommand> AllSubCommands;
  परत *AllSubCommands;
}

शून्य SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(हे);
}

शून्य SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(हे);
}

शून्य SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::चालक तार्किक() स्थिरांक {
  परत (globalParser().getActiveSubCommand() == हे);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_ओळ_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &मूल्य) {
  // Reject all dashes.
  जर (Arg.empty())
    परत nullptr;
  प्रतिज्ञा(&Sub != &SubCommand::getAll());

  माप_प्रकार EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  जर (EqualPos == StringRef::npos) {
    // Look up the option.
    परत Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  स्वतः I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  जर (I == Sub.OptionsMap.end())
    परत nullptr;

  स्वतः *O = I->second;
  जर (O->getFormattingFlag() == cl::AlwaysPrefix)
    परत nullptr;

  मूल्य = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  परत I->second;
}

SubCommand *Command_ओळ_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  जर (Name.empty())
    परत &SubCommand::get_शीर्ष_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  साठी (स्वतः *S : RegisteredSubCommands) {
    प्रतिज्ञा(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    जर (S->getName().empty())
      सुरूठेवा;

    जर (S->getName() == Name)
      परत S;

    जर (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  जर (NearestMatch)
    NearestString = NearestMatch->getName();

  परत &SubCommand::get_शीर्ष_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
स्थिर Option *LookupNearestOption(StringRef Arg,
                                   स्थिरांक OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  जर (Arg.empty())
    परत nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  अचिन्हित BestDistance = 0;
  साठी (स्थिरांक स्वतः &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    जर (O->getOptionHiddenFlag() == ReallyHidden)
      सुरूठेवा;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    जर (O->hasArgStr())
      OptionNames.ढकला_back(O->ArgStr);

    तार्किक Permit_मूल्य = O->get_मूल्य_Expected_Flag() != cl::मूल्य_Disallowed;
    StringRef Flag = Permit_मूल्य ? LHS : Arg;
    साठी (स्थिरांक स्वतः &Name : OptionNames) {
      अचिन्हित Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/सत्य, /*MaxEditDistance=*/BestDistance);
      जर (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        जर (RHS.empty() || !Permit_मूल्य)
          NearestString = std::string(Name);
        अन्यथा
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  परत Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
स्थिर तार्किक CommaSeparateAndAddOccurrence(Option *Handler, अचिन्हित pos,
                                          StringRef ArgName, StringRef मूल्य) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  जर (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मूल्य);
    StringRef::size_type Pos = Val.find(',');

    पर्यंत (Pos != StringRef::npos) {
      // Process the portion before the comma.
      जर (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        परत सत्य;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मूल्य = Val;
  }

  परत Handler->addOccurrence(pos, ArgName, मूल्य);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
स्थिर ओळीत तार्किक ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef मूल्य, पूर्णांक argc,
                                 स्थिरांक अक्षर *स्थिरांक *argv, पूर्णांक &i) {
  // Enforce value requirements
  निवड (Handler->get_मूल्य_Expected_Flag()) {
  स्थिती मूल्य_Required:
    जर (!मूल्य.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      जर (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        परत Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      प्रतिज्ञा(argv && "null check");
      मूल्य = StringRef(argv[++i]);
    }
    थांबा;
  स्थिती मूल्य_Disallowed:
    जर (मूल्य.data())
      परत Handler->error("does not allow a value! '" + Twine(मूल्य) +
                            "' specified.");
    थांबा;
  स्थिती मूल्य_Optional:
    थांबा;
  }

  परत CommaSeparateAndAddOccurrence(Handler, i, ArgName, मूल्य);
}

तार्किक llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, पूर्णांक i) {
  पूर्णांक Dummy = i;
  परत ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
स्थिर Option *findPrefixOption(StringRef Name, माप_प्रकार &Length,
                                स्थिरांक OptionsMapTy &OptionsMap) {
  साठी (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    जर (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      परत O;
    }
  }
  परत nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
स्थिर Option *HandlePrefixedOption(StringRef &Arg, StringRef &मूल्य,
                                    स्थिरांक OptionsMapTy &OptionsMap) {
  जर (Arg.size() == 1)
    परत nullptr;

  माप_प्रकार Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  जर (!POpt)
    परत nullptr;

  StringRef Maybe_मूल्य =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  जर (POpt->getFormattingFlag() == cl::Prefix && Maybe_मूल्य.starts_with("="))
    Maybe_मूल्य = Maybe_मूल्य.drop_front();
  मूल्य = Maybe_मूल्य;
  परत POpt;
}

स्थिर तार्किक Requires_मूल्य(स्थिरांक Option *O) {
  परत O->get_अंक_Occurrences_Flag() == cl::Required ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक Eats_Unbounded_संख्या_Of_मूल्ये(स्थिरांक Option *O) {
  परत O->get_अंक_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक isWhitespace(अक्षर C) {
  परत C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

स्थिर तार्किक isWhitespaceOrNull(अक्षर C) {
  परत isWhitespace(C) || C == '\0';
}

स्थिर तार्किक isQuote(अक्षर C) { परत C == '\"' || C == '\''; }

शून्य cl::Tokenize_GNUCommand_ओळ(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv,
                                तार्किक MarkEOLs) {
  SmallString<128> Token;
  तार्किक InToken = असत्य;
  साठी (माप_प्रकार I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    जर (!InToken) {
      पर्यंत (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        जर (MarkEOLs && Src[I] == '\n')
          NewArgv.ढकला_back(nullptr);
        ++I;
      }
      जर (I == E)
        थांबा;
      InToken = सत्य;
    }

    अक्षर C = Src[I];

    // Backslash escapes the next character.
    जर (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ढकला_back(Src[I]);
      सुरूठेवा;
    }

    // Consume a quoted string.
    जर (isQuote(C)) {
      ++I;
      पर्यंत (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        जर (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ढकला_back(Src[I]);
        ++I;
      }
      जर (I == E)
        थांबा;
      सुरूठेवा;
    }

    // End the token if this is whitespace.
    जर (isWhitespace(C)) {
      NewArgv.ढकला_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      जर (MarkEOLs && C == '\n')
        NewArgv.ढकला_back(nullptr);
      Token.clear();
      InToken = असत्य;
      सुरूठेवा;
    }

    // This is a normal character.  Append it.
    Token.ढकला_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  जर (InToken)
    NewArgv.ढकला_back(Saver.save(Token.str()).data());
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
स्थिर माप_प्रकार parseBackslash(StringRef Src, माप_प्रकार I, SmallString<128> &Token) {
  माप_प्रकार E = Src.size();
  पूर्णांक Backslash_मोजणी = 0;
  // Skip the backslashes.
  करा {
    ++I;
    ++Backslash_मोजणी;
  } पर्यंत (I != E && Src[I] == '\\');

  तार्किक FollowedByDoubleQuote = (I != E && Src[I] == '"');
  जर (FollowedByDoubleQuote) {
    Token.append(Backslash_मोजणी / 2, '\\');
    जर (Backslash_मोजणी % 2 == 0)
      परत I - 1;
    Token.ढकला_back('"');
    परत I;
  }
  Token.append(Backslash_मोजणी, '\\');
  परत I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
स्थिर तार्किक isWindowsSpecialChar(अक्षर C) {
  परत isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
स्थिर तार्किक isWindowsSpecialCharInCommandName(अक्षर C) {
  परत isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
स्थिर ओळीत शून्य tokenize_Windows_Command_ओळ_Impl(
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

  साठी (माप_प्रकार I = 0, E = Src.size(); I < E; ++I) {
    निवड (State) {
    स्थिती INIT: {
      प्रतिज्ञा(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      पर्यंत (I < E && isWhitespaceOrNull(Src[I])) {
        जर (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      जर (I >= E)
        थांबा;
      माप_प्रकार Start = I;
      जर (CommandName) {
        पर्यंत (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } अन्यथा {
        पर्यंत (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      जर (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        जर (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अन्यथा {
          CommandName = असत्य;
        }
      } अन्यथा जर (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } अन्यथा जर (Src[I] == '\\') {
        प्रतिज्ञा(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अन्यथा {
        llvm_unreachable("unexpected special character");
      }
      थांबा;
    }

    स्थिती UNQUOTED:
      जर (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        जर (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अन्यथा {
          CommandName = असत्य;
        }
        State = INIT;
      } अन्यथा जर (Src[I] == '\"') {
        State = QUOTED;
      } अन्यथा जर (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.ढकला_back(Src[I]);
      }
      थांबा;

    स्थिती QUOTED:
      जर (Src[I] == '\"') {
        जर (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ढकला_back('"');
          ++I;
        } अन्यथा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अन्यथा जर (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.ढकला_back(Src[I]);
      }
      थांबा;
    }
  }

  जर (State != INIT)
    AddToken(Saver.save(Token.str()));
}

शून्य cl::Tokenize_Windows_Command_ओळ(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv,
                                    तार्किक MarkEOLs) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok.data()); };
  स्वतः OnEOL = [&]() {
    जर (MarkEOLs)
      NewArgv.ढकला_back(nullptr);
  };
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सत्य, OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_ओळ_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok); };
  स्वतः OnEOL = []() {};
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/असत्य,
                                 OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_ओळ_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv,
                                        तार्किक MarkEOLs) {
  स्वतः AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok.data()); };
  स्वतः OnEOL = [&]() {
    जर (MarkEOLs)
      NewArgv.ढकला_back(nullptr);
  };
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सत्य, OnEOL, सत्य);
}

शून्य cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv,
                            तार्किक MarkEOLs) {
  साठी (स्थिरांक अक्षर *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> ओळ;
    // Check for comment line.
    जर (isWhitespace(*Cur)) {
      पर्यंत (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      सुरूठेवा;
    }
    जर (*Cur == '#') {
      पर्यंत (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      सुरूठेवा;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    स्थिरांक अक्षर *Start = Cur;
    स्थिरांक अक्षर *ओळ_End = nullptr;
    अक्षर Quote = 0;
    तार्किक AtTokenStart = सत्य;
    साठी (स्थिरांक अक्षर *End = Source.end(); Cur != End; ++Cur) {
      अक्षर C = *Cur;
      जर (C == '\\') {
        जर (Cur + 1 != End) {
          ++Cur;
          जर (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            ओळ.append(Start, Cur - 1);
            जर (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            सुरूठेवा;
          }
        }
        AtTokenStart = असत्य;
        सुरूठेवा;
      }
      जर (C == '\n')
        थांबा;
      जर (Quote) {
        जर (C == Quote)
          Quote = 0;
      } अन्यथा जर (isQuote(C)) {
        Quote = C;
      } अन्यथा जर (C == '#' && AtTokenStart) {
        ओळ_End = Cur;
        पर्यंत (Cur != End && *Cur != '\n')
          ++Cur;
        थांबा;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    ओळ.append(Start, ओळ_End ? ओळ_End : Cur);
    cl::Tokenize_GNUCommand_ओळ(ओळ, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
स्थिर तार्किक hasUTF8ByteOrderMark(ArrayRef<अक्षर> S) {
  परत (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
स्थिर शून्य ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            स्थिरांक अक्षर *&Arg) {
  प्रतिज्ञा(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  स्थिरांक StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  साठी (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    स्थिरांक StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    जर (ResponseFile.empty())
      ResponseFile = LHS;
    अन्यथा
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  जर (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    स्थिरांक StringRef Remaining = ArgString.substr(StartPos);
    जर (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv) {
  प्रतिज्ञा(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  जर (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    परत llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<अक्षर> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  जर (hasUTF16ByteOrderMark(BufRef)) {
    जर (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      परत llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अन्यथा जर (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  जर (!RelativeNames && !InConfigFile)
    परत Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  साठी (स्थिरांक अक्षर *&Arg : NewArgv) {
    जर (!Arg)
      सुरूठेवा;

    // Substitute <CFGDIR> with the file's base path.
    जर (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    तार्किक ConfigInclusion = असत्य;
    जर (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      जर (!llvm::sys::path::is_relative(FileName))
        सुरूठेवा;
    } अन्यथा जर (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = सत्य;
    } अन्यथा {
      सुरूठेवा;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ढकला_back('@');
    जर (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      जर (!findConfigFile(FileName, FilePath))
        परत createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } अन्यथा {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  परत Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<स्थिरांक अक्षर *> &Argv) {
  संरचना ResponseFileRecord {
    std::string File;
    माप_प्रकार End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_चळत;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_चळत.ढकला_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  साठी (अचिन्हित I = 0; I != Argv.size();) {
    पर्यंत (I == File_चळत.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_चळत.काढा_back();
    }

    स्थिरांक अक्षर *Arg = Argv[I];
    // Check if it is an EOL marker
    जर (Arg == nullptr) {
      ++I;
      सुरूठेवा;
    }

    जर (Arg[0] != '@') {
      ++I;
      सुरूठेवा;
    }

    स्थिरांक अक्षर *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    जर (llvm::sys::path::is_relative(FName)) {
      जर (CurrentDir.empty()) {
        जर (स्वतः CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अन्यथा {
          परत createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अन्यथा {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    जर (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      जर (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        जर (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          सुरूठेवा;
        }
      }
      जर (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      परत createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    स्थिरांक llvm::vfs::Status &FileStatus = Res.get();

    स्वतः IsEquivalent =
        [FileStatus, हे](स्थिरांक ResponseFileRecord &RFile) -> ErrorOr<तार्किक> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      जर (!RHS)
        परत RHS.getError();
      परत FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    साठी (स्थिरांक स्वतः &F : drop_begin(File_चळत)) {
      जर (ErrorOr<तार्किक> R = IsEquivalent(F)) {
        जर (R.get())
          परत createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अन्यथा {
        परत createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<स्थिरांक अक्षर *, 0> ExpandedArgv;
    जर (Error Err = expandResponseFile(FName, ExpandedArgv))
      परत Err;

    साठी (ResponseFileRecord &Record : File_चळत) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_चळत.ढकला_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.घाला(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  प्रतिज्ञा(File_चळत.size() > 0 && Argv.size() == File_चळत.back().End);
  परत Error::success();
}

तार्किक cl::expandResponseFiles(पूर्णांक Argc, स्थिरांक अक्षर *स्थिरांक *Argv,
                             स्थिरांक अक्षर *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<स्थिरांक अक्षर *> &NewArgv) {
#ifdef _WIN32
  स्वतः Tokenize = cl::Tokenize_Windows_Command_ओळ;
#अन्यथा
  स्वतः Tokenize = cl::Tokenize_GNUCommand_ओळ;
#endif
  // The environment variable specifies initial options.
  जर (EnvVar)
    जर (std::optional<std::string> Env_मूल्य = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मूल्य, Saver, NewArgv, /*MarkEOLs=*/असत्य);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  जर (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    परत असत्य;
  }
  परत सत्य;
}

तार्किक cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<स्थिरांक अक्षर *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  जर (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    परत असत्य;
  }
  परत सत्य;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

तार्किक ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<अक्षर> &FilePath) {
  SmallString<128> CfgFilePath;
  स्थिरांक स्वतः FileExists = [हे](SmallString<128> Path) -> तार्किक {
    स्वतः Status = FS->status(Path);
    परत Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  जर (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    जर (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      परत असत्य;
    जर (!FileExists(CfgFilePath))
      परत असत्य;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    परत सत्य;
  }

  // Look for the file in search directories.
  साठी (स्थिरांक StringRef &Dir : शोध_Dirs) {
    जर (Dir.empty())
      सुरूठेवा;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    जर (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      परत सत्य;
    }
  }

  परत असत्य;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<स्थिरांक अक्षर *> &Argv) {
  SmallString<128> AbsPath;
  जर (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    जर (std::error_code EC = FS->makeAbsolute(AbsPath))
      परत make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = सत्य;
  RelativeNames = सत्य;
  जर (Error Err = expandResponseFile(CfgFile, Argv))
    परत Err;
  परत expandResponseFiles(Argv);
}

स्थिर शून्य initCommonOptions();
स्थिर तार्किक parse_Command_ओळ_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<स्थिरांक अक्षर *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        स्थिरांक अक्षर *EnvVar) {
  initCommonOptions();
  SmallVector<स्थिरांक अक्षर *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ढकला_back(Args[0]);

  // Parse options from environment variable.
  जर (EnvVar) {
    जर (std::optional<std::string> Env_मूल्य =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ओळ(*Env_मूल्य, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  पूर्णांक NewArgc = static_cast<पूर्णांक>(NewArgv.size());

  // Parse all options.
  परत globalParser().Parse_Command_ओळ_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

तार्किक cl::Parse_Command_ओळ_Options(पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, स्थिरांक अक्षर *EnvVar) {
  परत parse_Command_ओळ_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

तार्किक cl::parse_Command_ओळ_Options(LibraryOptionsContext &Context,
                                 ArrayRef<स्थिरांक अक्षर *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, स्थिरांक अक्षर *EnvVar) {
  परत parse_Command_ओळ_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
शून्य Command_ओळ_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  साठी (स्वतः *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    साठी (स्वतः &O : SC->OptionsMap)
      Opts.ढकला_back(O.second);
    साठी (Option *O : Opts)
      O->reset();
    साठी (Option *O : SC->PositionalOpts)
      O->reset();
    जर (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  साठी (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

तार्किक Command_ओळ_Parser::Parse_Command_ओळ_Options(
    पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  प्रतिज्ञा(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  तार्किक IgnoreErrors = Errs;
  जर (!Errs)
    Errs = &errs();
  जर (!VFS)
    VFS = vfs::getRealFileSystem().get();
  तार्किक ErrorParsing = असत्य;

  // Expand response files.
  SmallVector<स्थिरांक अक्षर *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  स्वतः Tokenize = cl::Tokenize_Windows_Command_ओळ;
#अन्यथा
  स्वतः Tokenize = cl::Tokenize_GNUCommand_ओळ;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  जर (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    परत असत्य;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णांक>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  अचिन्हित अंक_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  तार्किक HasUnlimitedPositionals = असत्य;

  पूर्णांक FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शीर्ष_Level();
  std::string NearestSubCommandString;
  तार्किक MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  जर (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    जर (ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  प्रतिज्ञा(ChosenSubCommand);
  स्वतः &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  स्वतः &PositionalOpts = ChosenSubCommand->PositionalOpts;
  स्वतः &OptionsMap = ChosenSubCommand->OptionsMap;

  जर (ConsumeAfterOpt) {
    प्रतिज्ञा(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  जर (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    तार्किक UnboundedFound = असत्य;
    साठी (माप_प्रकार i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      जर (Requires_मूल्य(Opt))
        ++अंक_Positional_Required;
      अन्यथा जर (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        जर (PositionalOpts.size() > 1) {
          जर (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = सत्य;
        }
      } अन्यथा जर (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        जर (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = सत्य;
      }
      UnboundedFound |= Eats_Unbounded_संख्या_Of_मूल्ये(Opt);
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
  तार्किक DashDashFound = असत्य; // Have we read '--'?
  साठी (पूर्णांक i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef मूल्य;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    जर (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      जर (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        सुरूठेवा; // We are done!
      }

      जर (!PositionalOpts.empty()) {
        PositionalVals.ढकला_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        जर (PositionalVals.size() >= अंक_Positional_Required && ConsumeAfterOpt) {
          साठी (++i; i < argc; ++i)
            PositionalVals.ढकला_back(std::make_pair(StringRef(argv[i]), i));
          थांबा; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        सुरूठेवा;
      }
    } अन्यथा जर (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = सत्य; // This is the mythical "--"?
      सुरूठेवा;             // Don't try to process it as an argument itself.
    } अन्यथा जर (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);
      जर (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        सुरूठेवा; // We are done!
      }
    } अन्यथा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      जर (!Handler && ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
        Handler = LookupOption(SubCommand::get_शीर्ष_Level(), ArgName, मूल्य);

      जर (!Handler) {
        जर (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<स्थिरांक अक्षर *> Rest(argv + i, argc - i);
          अचिन्हित N;
          जर (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = सत्य;
          }
          i += N - 1;
          सुरूठेवा;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      जर (!Handler)
        Handler = HandlePrefixedOption(ArgName, मूल्य, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      जर (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    जर (!Handler) {
      स्वतः ReportUnknownArgument = [&](तार्किक IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        जर (NearestArgumentName.empty())
          परत;

        *Errs << ProgramName << ": Did you mean '";
        जर (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        अन्यथा
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      जर (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/सत्य, NearestHandlerString);
      अन्यथा
        ReportUnknownArgument(/*IsArg=*/असत्य, NearestSubCommandString);

      ErrorParsing = सत्य;
      सुरूठेवा;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    जर (Handler->getFormattingFlag() == cl::Positional) {
      जर ((Handler->getMiscFlags() & PositionalEatsArgs) && !मूल्य.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = सत्य;
      }
      ActivePositionalArg = Handler;
    }
    अन्यथा
      ErrorParsing |= ProvideOption(Handler, ArgName, मूल्य, argc, argv, i);
  }

  // Check and handle positional arguments now...
  जर (अंक_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अंक_Positional_Required
             << " positional argument" << (अंक_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = सत्य;
  } अन्यथा जर (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = सत्य;

  } अन्यथा जर (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    अचिन्हित ValNo = 0, अंक_Vals = static_cast<अचिन्हित>(PositionalVals.size());
    साठी (Option *Opt : PositionalOpts) {
      जर (Requires_मूल्य(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --अंक_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      तार्किक Done = Opt->get_अंक_Occurrences_Flag() == cl::Required;
      पर्यंत (अंक_Vals - ValNo > अंक_Positional_Required && !Done) {
        निवड (Opt->get_अंक_Occurrences_Flag()) {
        स्थिती cl::Optional:
          Done = सत्य; // Optional arguments want _at most_ one value
          [[fallthrough]];
        स्थिती cl::ZeroOrMore: // Zero or more will take all they can get...
        स्थिती cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          थांबा;
        पूर्वनिर्धारित:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } अन्यथा {
    प्रतिज्ञा(ConsumeAfterOpt && अंक_Positional_Required <= PositionalVals.size());
    अचिन्हित ValNo = 0;
    साठी (Option *Opt : PositionalOpts)
      जर (Requires_मूल्य(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    जर (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    साठी (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  साठी (स्थिरांक स्वतः &Opt : OptionsMap) {
    निवड (Opt.second->get_अंक_Occurrences_Flag()) {
    स्थिती Required:
    स्थिती OneOrMore:
      जर (Opt.second->get_अंक_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = सत्य;
      }
      [[fallthrough]];
    पूर्वनिर्धारित:
      थांबा;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             साठी (पूर्णांक i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  जर (ErrorParsing) {
    जर (!IgnoreErrors)
      निकास(1);
    परत असत्य;
  }
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

तार्किक Option::error(स्थिरांक Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  जर (!ArgName.data())
    ArgName = ArgStr;
  जर (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  अन्यथा
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  परत सत्य;
}

तार्किक Option::addOccurrence(अचिन्हित pos, StringRef ArgName, StringRef मूल्य) {
  ++अंक_Occurrences;
  परत handleOccurrence(pos, ArgName, मूल्य);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
स्थिर StringRef get_मूल्य_Str(स्थिरांक Option &O, StringRef Default_संदेश) {
  जर (O.मूल्य_Str.empty())
    परत Default_संदेश;
  परत O.मूल्य_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
माप_प्रकार alias::getOptionWidth() स्थिरांक {
  परत argPlusPrefixesSize(ArgStr);
}

शून्य Option::printHelpStr(StringRef HelpStr, माप_प्रकार Indent,
                          माप_प्रकार First_ओळ_Indented_By) {
  प्रतिज्ञा(Indent >= First_ओळ_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_ओळ_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  पर्यंत (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

शून्य Option::printEnumValHelpStr(StringRef HelpStr, माप_प्रकार BaseIndent,
                                 माप_प्रकार First_ओळ_Indented_By) {
  स्थिरांक StringRef ValHelpPrefix = "  ";
  प्रतिज्ञा(BaseIndent >= First_ओळ_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_ओळ_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  पर्यंत (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
शून्य alias::printOptionInfo(माप_प्रकार GlobalWidth) स्थिरांक {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
माप_प्रकार basic_parser_impl::getOptionWidth(स्थिरांक Option &O) स्थिरांक {
  माप_प्रकार Len = argPlusPrefixesSize(O.ArgStr);
  स्वतः ValName = get_मूल्य_Name();
  जर (!ValName.empty()) {
    माप_प्रकार FormattingLen = 3;
    जर (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_मूल्य_Str(O, ValName).size() + FormattingLen;
  }

  परत Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य basic_parser_impl::printOptionInfo(स्थिरांक Option &O,
                                        माप_प्रकार GlobalWidth) स्थिरांक {
  outs() << PrintArg(O.ArgStr);

  स्वतः ValName = get_मूल्य_Name();
  जर (!ValName.empty()) {
    जर (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_मूल्य_Str(O, ValName) << ">...";
    } अन्यथा जर (O.get_मूल्य_Expected_Flag() == मूल्य_Optional)
      outs() << "[=<" << get_मूल्य_Str(O, ValName) << ">]";
    अन्यथा {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_मूल्य_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

शून्य basic_parser_impl::printOptionName(स्थिरांक Option &O,
                                        माप_प्रकार GlobalWidth) स्थिरांक {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
तार्किक parser<तार्किक>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         तार्किक &मूल्य) {
  परत parseBool<तार्किक, सत्य, असत्य>(O, ArgName, Arg, मूल्य);
}

// parser<boolOrDefault> implementation
//
तार्किक parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मूल्य) {
  परत parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, मूल्य);
}

// parser<FixedOrScalableQuantity> implementation
//
ढाँचा <typename FixedOrScalableQuantityT>
स्थिर तार्किक parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef मूल्य_Kind,
                                         FixedOrScalableQuantityT &मूल्य) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_मूल्य;
  जर (!Arg.getAsInteger(0, Min_मूल्य)) {
    मूल्य = FixedOrScalableQuantityT::getFixed(Min_मूल्य);
    परत असत्य;
  }

  StringRef Remainder = Arg;
  जर (!Remainder.consume_front("vscale"))
    परत O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  जर (!Remainder.consume_front('x'))
    परत O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  जर (Remainder.getAsInteger(0, Min_मूल्य))
    परत O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  मूल्य = FixedOrScalableQuantityT::getScalable(Min_मूल्य);
  परत असत्य;
}

// parser<int> implementation
//
तार्किक parser<पूर्णांक>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णांक &मूल्य) {
  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for integer argument!");
  परत असत्य;
}

// parser<long> implementation
//
तार्किक parser<दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         दीर्घ &मूल्य) {
  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for long argument!");
  परत असत्य;
}

// parser<long long> implementation
//
तार्किक parser<दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              दीर्घ दीर्घ &मूल्य) {
  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for llong argument!");
  परत असत्य;
}

// parser<unsigned> implementation
//
तार्किक parser<अचिन्हित>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             अचिन्हित &मूल्य) {

  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for uint argument!");
  परत असत्य;
}

// parser<unsigned long> implementation
//
तार्किक parser<अचिन्हित दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  अचिन्हित दीर्घ &मूल्य) {

  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for ulong argument!");
  परत असत्य;
}

// parser<unsigned long long> implementation
//
तार्किक parser<अचिन्हित दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       अचिन्हित दीर्घ दीर्घ &मूल्य) {

  जर (Arg.getAsInteger(0, मूल्य))
    परत O.error("'" + Arg + "' value invalid for ullong argument!");
  परत असत्य;
}

// parser<ElementCount> implementation
//
तार्किक parser<Element_मोजणी>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_मोजणी &मूल्य) {
  परत parseFixedOrScalableQuantity(O, Arg, get_मूल्य_Name(), मूल्य);
}

// parser<double>/parser<float> implementation
//
स्थिर तार्किक parseDouble(Option &O, StringRef Arg, द्विगुण &मूल्य) {
  जर (to_float(Arg, मूल्य))
    परत असत्य;
  परत O.error("'" + Arg + "' value invalid for floating point argument!");
}

तार्किक parser<द्विगुण>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुण &Val) {
  परत parseDouble(O, Arg, Val);
}

तार्किक parser<दशांश>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          दशांश &Val) {
  द्विगुण dVal;
  जर (parseDouble(O, Arg, dVal))
    परत सत्य;
  Val = (दशांश)dVal;
  परत असत्य;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
अचिन्हित generic_parser_base::findOption(StringRef Name) {
  अचिन्हित e = get_अंक_Options();

  साठी (अचिन्हित i = 0; i != e; ++i) {
    जर (getOption(i) == Name)
      परत i;
  }
  परत e;
}

स्थिर StringRef Eq_मूल्य = "=<value>";
स्थिर StringRef EmptyOption = "<empty>";
स्थिर StringRef OptionPrefix = "    =";
स्थिर माप_प्रकार getOptionPrefixesSize() {
  परत OptionPrefix.size() + ArgHelpPrefix.size();
}

स्थिर तार्किक shouldPrintOption(StringRef Name, StringRef Description,
                              स्थिरांक Option &O) {
  परत O.get_मूल्य_Expected_Flag() != मूल्य_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
माप_प्रकार generic_parser_base::getOptionWidth(स्थिरांक Option &O) स्थिरांक {
  जर (O.hasArgStr()) {
    माप_प्रकार Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_मूल्य.size();
    साठी (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      जर (!shouldPrintOption(Name, getDescription(i), O))
        सुरूठेवा;
      माप_प्रकार NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    परत Size;
  } अन्यथा {
    माप_प्रकार BaseSize = 0;
    साठी (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    परत BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य generic_parser_base::printOptionInfo(स्थिरांक Option &O,
                                          माप_प्रकार GlobalWidth) स्थिरांक {
  जर (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    जर (O.get_मूल्य_Expected_Flag() == मूल्य_Optional) {
      साठी (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
        जर (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          थांबा;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_मूल्य;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_मूल्य.size() +
                             argPlusPrefixesSize(O.ArgStr));
    साठी (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      जर (!shouldPrintOption(OptionName, Description, O))
        सुरूठेवा;
      माप_प्रकार First_ओळ_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      जर (OptionName.empty()) {
        outs() << EmptyOption;
        प्रतिज्ञा(First_ओळ_Indent >= EmptyOption.size());
        First_ओळ_Indent += EmptyOption.size();
      }
      जर (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_ओळ_Indent);
      अन्यथा
        outs() << '\n';
    }
  } अन्यथा {
    जर (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    साठी (अचिन्हित i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

स्थिर स्थिरांक माप_प्रकार MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
शून्य generic_parser_base::printGenericOptionDiff(
    स्थिरांक Option &O, स्थिरांक Generic_Option_मूल्य &मूल्य,
    स्थिरांक Generic_Option_मूल्य &Default, माप_प्रकार GlobalWidth) स्थिरांक {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  अचिन्हित अंक_Opts = get_अंक_Options();
  साठी (अचिन्हित i = 0; i != अंक_Opts; ++i) {
    जर (!मूल्य.compare(get_Option_मूल्य(i)))
      सुरूठेवा;

    outs() << "= " << getOption(i);
    माप_प्रकार L = getOption(i).size();
    माप_प्रकार अंक_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(अंक_Spaces) << " (default: ";
    साठी (अचिन्हित j = 0; j != अंक_Opts; ++j) {
      जर (!Default.compare(get_Option_मूल्य(j)))
        सुरूठेवा;
      outs() << getOption(j);
      थांबा;
    }
    outs() << ")\n";
    परत;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
नामाकाश llvm {
नामाकाश cl {
स्थिर raw_ostream &चालक<<(raw_ostream &OS, boolOrDefault V) {
  परत OS << static_cast<पूर्णांक>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  शून्य parser<T>::printOptionDiff(स्थिरांक Option &O, T V, Option_मूल्य<T> D,      \
                                  माप_प्रकार GlobalWidth) स्थिरांक {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    माप_प्रकार अंक_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(अंक_Spaces) << " (default: ";                                 \
    जर (D.has_मूल्य())                                                          \
      outs() << D.get_मूल्य();                                                  \
    अन्यथा                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(तार्किक)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(पूर्णांक)
PRINT_OPT_DIFF(दीर्घ)
PRINT_OPT_DIFF(दीर्घ दीर्घ)
PRINT_OPT_DIFF(अचिन्हित)
PRINT_OPT_DIFF(अचिन्हित दीर्घ)
PRINT_OPT_DIFF(अचिन्हित दीर्घ दीर्घ)
PRINT_OPT_DIFF(द्विगुण)
PRINT_OPT_DIFF(दशांश)
PRINT_OPT_DIFF(अक्षर)
PRINT_OPT_DIFF(Element_मोजणी)

शून्य parser<std::string>::printOptionDiff(स्थिरांक Option &O, StringRef V,
                                          स्थिरांक Option_मूल्य<std::string> &D,
                                          माप_प्रकार GlobalWidth) स्थिरांक {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  माप_प्रकार अंक_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(अंक_Spaces) << " (default: ";
  जर (D.has_मूल्य())
    outs() << D.get_मूल्य();
  अन्यथा
    outs() << "*no default*";
  outs() << ")\n";
}

शून्य parser<std::optional<std::string>>::printOptionDiff(
    स्थिरांक Option &O, std::optional<StringRef> V,
    स्थिरांक Option_मूल्य<std::optional<std::string>> &D,
    माप_प्रकार GlobalWidth) स्थिरांक {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  माप_प्रकार VSize = V.has_मूल्य_2() ? V.मूल्य_2().size() : 0;
  माप_प्रकार अंक_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(अंक_Spaces) << " (default: ";
  जर (D.has_मूल्य() && D.get_मूल्य().has_मूल्य_2())
    outs() << D.get_मूल्य();
  अन्यथा
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
शून्य basic_parser_impl::print_Option_No_मूल्य(स्थिरांक Option &O,
                                           माप_प्रकार GlobalWidth) स्थिरांक {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

स्थिर पूर्णांक OptNameCompare(स्थिरांक std::pair<स्थिरांक अक्षर *, Option *> *LHS,
                          स्थिरांक std::pair<स्थिरांक अक्षर *, Option *> *RHS) {
  परत म_भेद(LHS->first, RHS->first);
}

स्थिर पूर्णांक SubNameCompare(स्थिरांक std::pair<स्थिरांक अक्षर *, SubCommand *> *LHS,
                          स्थिरांक std::pair<स्थिरांक अक्षर *, SubCommand *> *RHS) {
  परत म_भेद(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
स्थिर शून्य sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<स्थिरांक अक्षर *, Option *>> &Opts,
                     तार्किक ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  साठी (स्वतः I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    जर (I->second->getOptionHiddenFlag() == ReallyHidden)
      सुरूठेवा;

    // Unless showhidden is set, ignore hidden flags.
    जर (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      सुरूठेवा;

    // If we've already seen this option, don't add it to the list again.
    जर (!OptionSet.घाला(I->second).second)
      सुरूठेवा;

    Opts.ढकला_back(
        std::pair<स्थिरांक अक्षर *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

स्थिर शून्य
sortSubCommands(स्थिरांक SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<स्थिरांक अक्षर *, SubCommand *>> &Subs) {
  साठी (स्वतः *S : SubMap) {
    जर (S->getName().empty())
      सुरूठेवा;
    Subs.ढकला_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

नामाकाश {

वर्ग HelpPrinter {
संरक्षित:
  स्थिरांक तार्किक ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<स्थिरांक अक्षर *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<स्थिरांक अक्षर *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  आभासी शून्य printOptions(StrOptionPairVector &Opts, माप_प्रकार MaxArgLen) {
    साठी (स्थिरांक स्वतः &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  शून्य printSubCommands(StrSubCommandPairVector &Subs, माप_प्रकार MaxSubLen) {
    साठी (स्थिरांक स्वतः &S : Subs) {
      outs() << "  " << S.first;
      जर (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - म_माप(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

सार्वजनिक:
  explicit HelpPrinter(तार्किक showHidden) : ShowHidden(showHidden) {}
  आभासी ~HelpPrinter() = पूर्वनिर्धारित;

  // Invoke the printer.
  शून्य चालक=(तार्किक मूल्य) {
    जर (!मूल्य)
      परत;
    printHelp();

    // Halt the program since help information was printed
    निकास(0);
  }

  शून्य printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    स्वतः &OptionsMap = Sub->OptionsMap;
    स्वतः &PositionalOpts = Sub->PositionalOpts;
    स्वतः &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    जर (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    जर (Sub == &SubCommand::get_शीर्ष_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      जर (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } अन्यथा {
      जर (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    साठी (स्वतः *Opt : PositionalOpts) {
      जर (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    जर (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    जर (Sub == &SubCommand::get_शीर्ष_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      माप_प्रकार MaxSubLen = 0;
      साठी (स्थिरांक स्वतः &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, म_माप(Sub.first));

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
    माप_प्रकार MaxArgLen = 0;
    साठी (स्थिरांक स्वतः &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    जर (ShowHidden)
      साठी (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              जर (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    साठी (स्थिरांक स्वतः &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    जर (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    साठी (स्थिरांक स्वतः &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    साठी (स्थिरांक स्वतः &I : globalParser().MoreHelp)
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
  स्थिर पूर्णांक OptionCategoryCompare(OptionCategory *स्थिरांक *A,
                                   OptionCategory *स्थिरांक *B) {
    परत (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::चालक=;

संरक्षित:
  शून्य printOptions(StrOptionPairVector &Opts, माप_प्रकार MaxArgLen) override {
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
    साठी (स्थिरांक स्वतः &I : Opts) {
      Option *Opt = I.second;
      साठी (OptionCategory *Cat : Opt->Categories) {
        प्रतिज्ञा(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ढकला_back(Opt);
      }
    }

    // Now do printing.
    साठी (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      स्थिरांक स्वतः &CategoryOptions = CategorizedOptions[Category];
      जर (CategoryOptions.empty())
        सुरूठेवा;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      जर (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      अन्यथा
        outs() << "\n";

      // Loop over the options in the category and print.
      साठी (स्थिरांक Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
वर्ग HelpPrinterWrapper {
खाजगी:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

सार्वजनिक:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  शून्य चालक=(तार्किक मूल्य);
};

} // End anonymous namespace

#जर defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# जर defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#किंवाजर defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# जर defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#अन्यथा
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

नामाकाश {
वर्ग VersionPrinter {
सार्वजनिक:
  शून्य print(स्थिरांक std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अन्यथा
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#जर LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अन्यथा
    OS << "Optimized build";
#endif
#ifndef न_संशोधन
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    जर (!ExtraPrinters.empty()) {
      साठी (स्थिरांक स्वतः &I : ExtraPrinters)
        I(outs());
    }
  }
  शून्य चालक=(तार्किक OptionWasSpecified);
};

संरचना Command_ओळ_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{असत्य};
  HelpPrinter UncategorizedHiddenPrinter{सत्य};
  CategorizedHelpPrinter CategorizedNormalPrinter{असत्य};
  CategorizedHelpPrinter CategorizedHiddenPrinter{सत्य};
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
  cl::opt<HelpPrinter, सत्य, parser<तार्किक>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, सत्य, parser<तार्किक>> HLHOp{
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
  cl::opt<HelpPrinterWrapper, सत्य, parser<तार्किक>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, सत्य, parser<तार्किक>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::मूल्य_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(असत्य),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(असत्य),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, सत्य, parser<तार्किक>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::मूल्य_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
स्थिर ManagedStatic<Command_ओळ_Common_Options> CommonOptions;

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
  परत GeneralCategory;
}

शून्य VersionPrinter::चालक=(तार्किक OptionWasSpecified) {
  जर (!OptionWasSpecified)
    परत;

  जर (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    निकास(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  निकास(0);
}

शून्य HelpPrinterWrapper::चालक=(तार्किक मूल्य) {
  जर (!मूल्य)
    परत;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  जर (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = सत्य; // Invoke categorized printer
  } अन्यथा {
    UncategorizedPrinter = सत्य; // Invoke uncategorized printer
  }
}

// Print the value of each option.
शून्य cl::Print_Option_मूल्ये() { globalParser().print_Option_मूल्ये(); }

शून्य Command_ओळ_Parser::print_Option_मूल्ये() {
  जर (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    परत;

  SmallVector<std::pair<स्थिरांक अक्षर *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ सत्य);

  // Compute the maximum argument length...
  माप_प्रकार MaxArgLen = 0;
  साठी (स्थिरांक स्वतः &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  साठी (स्थिरांक स्वतः &Opt : Opts)
    Opt.second->print_Option_मूल्य(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
शून्य cl::PrintHelpMessage(तार्किक Hidden, तार्किक Categorized) {
  जर (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  अन्यथा जर (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  अन्यथा जर (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अन्यथा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  स्थिर स्थिरांक StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#जर LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef न_संशोधन
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#जर __has_feature(address_sanitizer)
      "+asan",
#endif
#जर __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#जर __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#जर __has_feature(memory_sanitizer)
      "+msan",
#endif
#जर __has_feature(thread_sanitizer)
      "+tsan",
#endif
#जर __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  परत ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
शून्य cl::printBuildConfig(raw_ostream &OS) {
#जर LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
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
  CommonOptions->ExtraVersionPrinters.ढकला_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  स्वतः &Subs = globalParser().RegisteredSubCommands;
  (शून्य)Subs;
  प्रतिज्ञा(Subs.contains(&Sub));
  परत Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  परत globalParser().getRegisteredSubcommands();
}

शून्य cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  साठी (स्वतः &I : Sub.OptionsMap) {
    तार्किक Unrelated = सत्य;
    साठी (स्वतः &Cat : I.second->Categories) {
      जर (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    जर (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::HideUnrelatedOptions(ArrayRef<स्थिरांक cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  साठी (स्वतः &I : Sub.OptionsMap) {
    तार्किक Unrelated = सत्य;
    साठी (स्वतः &Cat : I.second->Categories) {
      जर (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    जर (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::Reset_Command_ओळ_Parser() { globalParser().reset(); }

शून्य cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ढकला_back(&L);
}

शून्य LibraryOptionsContext::assign(स्थिरांक LibraryOptionsContext &Other) {
  प्रतिज्ञा(हे != &Other);
  मूल्ये.clear();
  SavedArgs.clear();
  ArrayRef<स्थिरांक अक्षर *> Args = Other.SavedArgs;
  साठी (माप_प्रकार I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<स्थिरांक अक्षर *> Args,
                                   अचिन्हित &Consumed) {
  स्वतः [It, Inserted] = मूल्ये.try_emplace(L.getDefault());
  जर (Inserted)
    It->second = L.copyDefault(Alloc);
  परत parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

शून्य cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

शून्य LLVMParse_Command_ओळ_Options(पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक *argv,
                                 स्थिरांक अक्षर *Overview) {
  llvm::cl::Parse_Command_ओळ_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
