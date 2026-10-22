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
template वर्ग LLVM_EXPORT_TEMPLATE basic_parser<Element_मोजणी>;

#यदि !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
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
शून्य Generic_Option_मूल्य::anchor() {}
शून्य Option_मूल्य<boolOrDefault>::anchor() {}
शून्य Option_मूल्य<std::string>::anchor() {}
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
शून्य parser<Element_मोजणी>::anchor() {}

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
  यदि (Len == 1)
    प्रत्यागम Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  प्रत्यागम Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

स्थिर SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  प्रति (size_t I = 0; I < Pad; ++I) {
    Prefix.ढकला_back(' ');
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
  size_t अंक_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ओळ_Parser() { registerSubCommand(&SubCommand::get_शीर्ष_Level()); }

  शून्य ResetAllOptionOccurrences();

  तार्किक Parse_Command_ओळ_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  शून्य forEachSubCommand(Option &Opt, function_ref<शून्य(SubCommand &)> Action) {
    यदि (Opt.Subs.empty()) {
      Action(SubCommand::get_शीर्ष_Level());
      प्रत्यागम;
    }
    यदि (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
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
    यदि (Opt.hasArgStr())
      प्रत्यागम;
    यदि (!SC->OptionsMap.घाला(std::make_pair(Name, &Opt)).second) {
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
    यदि (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      यदि (!SC->OptionsMap.घाला(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = सत्य;
      }
    }

    // Remember information about positional options.
    यदि (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ढकला_back(O);
    अन्यथा यदि (O->get_अंक_Occurrences_Flag() == cl::ConsumeAfter) {
      यदि (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = सत्य;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    यदि (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  शून्य addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  शून्य removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    यदि (O->hasArgStr())
      OptionNames.ढकला_back(O->ArgStr);

    SubCommand &Sub = *SC;
    प्रति (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      यदि (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    यदि (O->getFormattingFlag() == cl::Positional)
      प्रति (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        यदि (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          भङ्ग;
        }
      }
    अन्यथा यदि (O == Sub.ConsumeAfterOpt)
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
      यदि (hasOptions(*S))
        प्रत्यागम सत्य;
    }
    प्रत्यागम !Libraries.empty();
  }

  तार्किक hasNamedSubCommands() ध्रुव {
    प्रति (ध्रुव auto *S : RegisteredSubCommands)
      यदि (!S->getName().empty())
        प्रत्यागम सत्य;
    प्रत्यागम असत्य;
  }

  SubCommand *getActiveSubCommand() { प्रत्यागम ActiveSubCommand; }

  शून्य updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    यदि (!Sub.OptionsMap.घाला(std::make_pair(NewName, O)).second) {
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
    प्रति (; अंक_Indexed_Libraries != Libraries.size(); ++अंक_Indexed_Libraries) {
      LibraryOptions *L = Libraries[अंक_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        यदि (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 प्रत्यागम SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          प्रत्यागम;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = सत्य;
      });
    }
    यदि (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    प्रत्यागम LibraryIndex.lookup(Name);
  }

  शून्य registerCategory(OptionCategory *cat) {
    प्रतिज्ञा(मोजणी_if(RegisteredOptionCategories,
                    [cat](ध्रुव OptionCategory *Category) {
             प्रत्यागम cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.घाला(cat);
  }

  शून्य registerSubCommand(SubCommand *sub) {
    प्रतिज्ञा(मोजणी_if(RegisteredSubCommands,
                    [sub](ध्रुव SubCommand *Sub) {
                      प्रत्यागम (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.घाला(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    प्रतिज्ञा(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    प्रति (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      यदि (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
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
    अंक_Indexed_Libraries = 0;

    SubCommand::get_शीर्ष_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_शीर्ष_Level());
  }

गुप्त:
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
  unsigned अंक_Args = std::min<size_t>(Args.size(), 2);
  प्रति (unsigned J = 0; J != अंक_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  यदि (Error E = L.parse(Opts, ArrayRef(Saved, अंक_Args), Consumed, Alloc))
    प्रत्यागम E;
  यदि (Record)
    Record->append(Saved, Saved + Consumed);
  प्रत्यागम Error::success();
}

template <typename T, T TrueVal, T FalseVal>
स्थिर तार्किक parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मूल्य) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  यदि (!Arg.data() || Arg == "true" || Arg == "1") {
    मूल्य = TrueVal;
    प्रत्यागम असत्य;
  }

  यदि (Arg == "false" || Arg == "0") {
    मूल्य = FalseVal;
    प्रत्यागम असत्य;
  }
  प्रत्यागम O.error("'" + Arg +
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
  globalParser().addOption(एतत्);
  FullyInitialized = सत्य;
}

शून्य Option::removeArgument() { globalParser().removeOption(एतत्); }

शून्य Option::setArgStr(StringRef S) {
  यदि (FullyInitialized)
    globalParser().updateArgStr(एतत्, S);
  प्रतिज्ञा(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

शून्य Option::addCategory(OptionCategory &C) {
  प्रतिज्ञा(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  यदि (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अन्यथा यदि (!is_contained(Categories, &C))
    Categories.ढकला_back(&C);
}

शून्य Option::reset() {
  अंक_Occurrences = 0;
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
SubCommand &SubCommand::get_शीर्ष_Level() {
  स्थिर ManagedStatic<SubCommand> शीर्ष_Level_Sub_Command;
  प्रत्यागम *शीर्ष_Level_Sub_Command;
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
Option *Command_ओळ_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &मूल्य) {
  // Reject all dashes.
  यदि (Arg.empty())
    प्रत्यागम nullptr;
  प्रतिज्ञा(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  यदि (EqualPos == StringRef::npos) {
    // Look up the option.
    प्रत्यागम Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  यदि (I == Sub.OptionsMap.end())
    प्रत्यागम nullptr;

  auto *O = I->second;
  यदि (O->getFormattingFlag() == cl::AlwaysPrefix)
    प्रत्यागम nullptr;

  मूल्य = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  प्रत्यागम I->second;
}

SubCommand *Command_ओळ_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  यदि (Name.empty())
    प्रत्यागम &SubCommand::get_शीर्ष_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  प्रति (auto *S : RegisteredSubCommands) {
    प्रतिज्ञा(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    यदि (S->getName().empty())
      अनुवर्त;

    यदि (S->getName() == Name)
      प्रत्यागम S;

    यदि (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  यदि (NearestMatch)
    NearestString = NearestMatch->getName();

  प्रत्यागम &SubCommand::get_शीर्ष_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
स्थिर Option *LookupNearestOption(StringRef Arg,
                                   ध्रुव OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  यदि (Arg.empty())
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
    यदि (O->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    यदि (O->hasArgStr())
      OptionNames.ढकला_back(O->ArgStr);

    तार्किक Permit_मूल्य = O->get_मूल्य_Expected_Flag() != cl::मूल्य_Disallowed;
    StringRef Flag = Permit_मूल्य ? LHS : Arg;
    प्रति (ध्रुव auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/सत्य, /*MaxEditDistance=*/BestDistance);
      यदि (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        यदि (RHS.empty() || !Permit_मूल्य)
          NearestString = std::string(Name);
        अन्यथा
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  प्रत्यागम Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
स्थिर तार्किक CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef मूल्य) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  यदि (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मूल्य);
    StringRef::size_type Pos = Val.find(',');

    यावत् (Pos != StringRef::npos) {
      // Process the portion before the comma.
      यदि (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        प्रत्यागम सत्य;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मूल्य = Val;
  }

  प्रत्यागम Handler->addOccurrence(pos, ArgName, मूल्य);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
स्थिर अन्तःस्थ तार्किक ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef मूल्य, पूर्णाङ्क argc,
                                 ध्रुव वर्ण *ध्रुव *argv, पूर्णाङ्क &i) {
  // Enforce value requirements
  विकल्प (Handler->get_मूल्य_Expected_Flag()) {
  पक्ष मूल्य_Required:
    यदि (!मूल्य.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      यदि (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        प्रत्यागम Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      प्रतिज्ञा(argv && "null check");
      मूल्य = StringRef(argv[++i]);
    }
    भङ्ग;
  पक्ष मूल्य_Disallowed:
    यदि (मूल्य.data())
      प्रत्यागम Handler->error("does not allow a value! '" + Twine(मूल्य) +
                            "' specified.");
    भङ्ग;
  पक्ष मूल्य_Optional:
    भङ्ग;
  }

  प्रत्यागम CommaSeparateAndAddOccurrence(Handler, i, ArgName, मूल्य);
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
    यदि (O && (O->getFormattingFlag() == cl::Prefix ||
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
स्थिर Option *HandlePrefixedOption(StringRef &Arg, StringRef &मूल्य,
                                    ध्रुव OptionsMapTy &OptionsMap) {
  यदि (Arg.size() == 1)
    प्रत्यागम nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  यदि (!POpt)
    प्रत्यागम nullptr;

  StringRef Maybe_मूल्य =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  यदि (POpt->getFormattingFlag() == cl::Prefix && Maybe_मूल्य.starts_with("="))
    Maybe_मूल्य = Maybe_मूल्य.drop_front();
  मूल्य = Maybe_मूल्य;
  प्रत्यागम POpt;
}

स्थिर तार्किक Requires_मूल्य(ध्रुव Option *O) {
  प्रत्यागम O->get_अंक_Occurrences_Flag() == cl::Required ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक Eats_Unbounded_संख्या_Of_मूल्ये(ध्रुव Option *O) {
  प्रत्यागम O->get_अंक_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_अंक_Occurrences_Flag() == cl::OneOrMore;
}

स्थिर तार्किक isWhitespace(वर्ण C) {
  प्रत्यागम C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

स्थिर तार्किक isWhitespaceOrNull(वर्ण C) {
  प्रत्यागम isWhitespace(C) || C == '\0';
}

स्थिर तार्किक isQuote(वर्ण C) { प्रत्यागम C == '\"' || C == '\''; }

शून्य cl::Tokenize_GNUCommand_ओळ(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                तार्किक MarkEOLs) {
  SmallString<128> Token;
  तार्किक InToken = असत्य;
  प्रति (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    यदि (!InToken) {
      यावत् (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        यदि (MarkEOLs && Src[I] == '\n')
          NewArgv.ढकला_back(nullptr);
        ++I;
      }
      यदि (I == E)
        भङ्ग;
      InToken = सत्य;
    }

    वर्ण C = Src[I];

    // Backslash escapes the next character.
    यदि (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ढकला_back(Src[I]);
      अनुवर्त;
    }

    // Consume a quoted string.
    यदि (isQuote(C)) {
      ++I;
      यावत् (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        यदि (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ढकला_back(Src[I]);
        ++I;
      }
      यदि (I == E)
        भङ्ग;
      अनुवर्त;
    }

    // End the token if this is whitespace.
    यदि (isWhitespace(C)) {
      NewArgv.ढकला_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      यदि (MarkEOLs && C == '\n')
        NewArgv.ढकला_back(nullptr);
      Token.clear();
      InToken = असत्य;
      अनुवर्त;
    }

    // This is a normal character.  Append it.
    Token.ढकला_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  यदि (InToken)
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
स्थिर size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  पूर्णाङ्क Backslash_मोजणी = 0;
  // Skip the backslashes.
  कुरु {
    ++I;
    ++Backslash_मोजणी;
  } यावत् (I != E && Src[I] == '\\');

  तार्किक FollowedByDoubleQuote = (I != E && Src[I] == '"');
  यदि (FollowedByDoubleQuote) {
    Token.append(Backslash_मोजणी / 2, '\\');
    यदि (Backslash_मोजणी % 2 == 0)
      प्रत्यागम I - 1;
    Token.ढकला_back('"');
    प्रत्यागम I;
  }
  Token.append(Backslash_मोजणी, '\\');
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
स्थिर अन्तःस्थ शून्य tokenize_Windows_Command_ओळ_Impl(
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
      यावत् (I < E && isWhitespaceOrNull(Src[I])) {
        यदि (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      यदि (I >= E)
        भङ्ग;
      size_t Start = I;
      यदि (CommandName) {
        यावत् (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } अन्यथा {
        यावत् (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      यदि (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        यदि (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अन्यथा {
          CommandName = असत्य;
        }
      } अन्यथा यदि (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } अन्यथा यदि (Src[I] == '\\') {
        प्रतिज्ञा(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अन्यथा {
        llvm_unreachable("unexpected special character");
      }
      भङ्ग;
    }

    पक्ष UNQUOTED:
      यदि (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        यदि (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अन्यथा {
          CommandName = असत्य;
        }
        State = INIT;
      } अन्यथा यदि (Src[I] == '\"') {
        State = QUOTED;
      } अन्यथा यदि (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.ढकला_back(Src[I]);
      }
      भङ्ग;

    पक्ष QUOTED:
      यदि (Src[I] == '\"') {
        यदि (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ढकला_back('"');
          ++I;
        } अन्यथा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अन्यथा यदि (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.ढकला_back(Src[I]);
      }
      भङ्ग;
    }
  }

  यदि (State != INIT)
    AddToken(Saver.save(Token.str()));
}

शून्य cl::Tokenize_Windows_Command_ओळ(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                    तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok.data()); };
  auto OnEOL = [&]() {
    यदि (MarkEOLs)
      NewArgv.ढकला_back(nullptr);
  };
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सत्य, OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_ओळ_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/असत्य,
                                 OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_ओळ_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                                        तार्किक MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ढकला_back(Tok.data()); };
  auto OnEOL = [&]() {
    यदि (MarkEOLs)
      NewArgv.ढकला_back(nullptr);
  };
  tokenize_Windows_Command_ओळ_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/सत्य, OnEOL, सत्य);
}

शून्य cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ध्रुव वर्ण *> &NewArgv,
                            तार्किक MarkEOLs) {
  प्रति (ध्रुव वर्ण *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> ओळ;
    // Check for comment line.
    यदि (isWhitespace(*Cur)) {
      यावत् (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      अनुवर्त;
    }
    यदि (*Cur == '#') {
      यावत् (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      अनुवर्त;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ध्रुव वर्ण *Start = Cur;
    ध्रुव वर्ण *ओळ_End = nullptr;
    वर्ण Quote = 0;
    तार्किक AtTokenStart = सत्य;
    प्रति (ध्रुव वर्ण *End = Source.end(); Cur != End; ++Cur) {
      वर्ण C = *Cur;
      यदि (C == '\\') {
        यदि (Cur + 1 != End) {
          ++Cur;
          यदि (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            ओळ.append(Start, Cur - 1);
            यदि (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            अनुवर्त;
          }
        }
        AtTokenStart = असत्य;
        अनुवर्त;
      }
      यदि (C == '\n')
        भङ्ग;
      यदि (Quote) {
        यदि (C == Quote)
          Quote = 0;
      } अन्यथा यदि (isQuote(C)) {
        Quote = C;
      } अन्यथा यदि (C == '#' && AtTokenStart) {
        ओळ_End = Cur;
        यावत् (Cur != End && *Cur != '\n')
          ++Cur;
        भङ्ग;
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
    यदि (ResponseFile.empty())
      ResponseFile = LHS;
    अन्यथा
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  यदि (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ध्रुव StringRef Remaining = ArgString.substr(StartPos);
    यदि (!Remaining.empty())
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
  यदि (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    प्रत्यागम llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<वर्ण> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  यदि (hasUTF16ByteOrderMark(BufRef)) {
    यदि (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      प्रत्यागम llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अन्यथा यदि (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  यदि (!RelativeNames && !InConfigFile)
    प्रत्यागम Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  प्रति (ध्रुव वर्ण *&Arg : NewArgv) {
    यदि (!Arg)
      अनुवर्त;

    // Substitute <CFGDIR> with the file's base path.
    यदि (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    तार्किक ConfigInclusion = असत्य;
    यदि (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      यदि (!llvm::sys::path::is_relative(FileName))
        अनुवर्त;
    } अन्यथा यदि (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = सत्य;
    } अन्यथा {
      अनुवर्त;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ढकला_back('@');
    यदि (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      यदि (!findConfigFile(FileName, FilePath))
        प्रत्यागम createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } अन्यथा {
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
  SmallVector<ResponseFileRecord, 3> File_चळत;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_चळत.ढकला_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  प्रति (unsigned I = 0; I != Argv.size();) {
    यावत् (I == File_चळत.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_चळत.काढा_back();
    }

    ध्रुव वर्ण *Arg = Argv[I];
    // Check if it is an EOL marker
    यदि (Arg == nullptr) {
      ++I;
      अनुवर्त;
    }

    यदि (Arg[0] != '@') {
      ++I;
      अनुवर्त;
    }

    ध्रुव वर्ण *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    यदि (llvm::sys::path::is_relative(FName)) {
      यदि (CurrentDir.empty()) {
        यदि (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अन्यथा {
          प्रत्यागम createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अन्यथा {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    यदि (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      यदि (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        यदि (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          अनुवर्त;
        }
      }
      यदि (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      प्रत्यागम createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ध्रुव llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, एतत्](ध्रुव ResponseFileRecord &RFile) -> ErrorOr<तार्किक> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      यदि (!RHS)
        प्रत्यागम RHS.getError();
      प्रत्यागम FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    प्रति (ध्रुव auto &F : drop_begin(File_चळत)) {
      यदि (ErrorOr<तार्किक> R = IsEquivalent(F)) {
        यदि (R.get())
          प्रत्यागम createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अन्यथा {
        प्रत्यागम createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ध्रुव वर्ण *, 0> ExpandedArgv;
    यदि (Error Err = expandResponseFile(FName, ExpandedArgv))
      प्रत्यागम Err;

    प्रति (ResponseFileRecord &Record : File_चळत) {
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
  प्रत्यागम Error::success();
}

तार्किक cl::expandResponseFiles(पूर्णाङ्क Argc, ध्रुव वर्ण *ध्रुव *Argv,
                             ध्रुव वर्ण *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ध्रुव वर्ण *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ओळ;
#अन्यथा
  auto Tokenize = cl::Tokenize_GNUCommand_ओळ;
#endif
  // The environment variable specifies initial options.
  यदि (EnvVar)
    यदि (std::optional<std::string> Env_मूल्य = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मूल्य, Saver, NewArgv, /*MarkEOLs=*/असत्य);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  यदि (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  यदि (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
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
  यदि (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    यदि (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      प्रत्यागम असत्य;
    यदि (!FileExists(CfgFilePath))
      प्रत्यागम असत्य;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    प्रत्यागम सत्य;
  }

  // Look for the file in search directories.
  प्रति (ध्रुव StringRef &Dir : शोध_Dirs) {
    यदि (Dir.empty())
      अनुवर्त;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    यदि (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      प्रत्यागम सत्य;
    }
  }

  प्रत्यागम असत्य;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ध्रुव वर्ण *> &Argv) {
  SmallString<128> AbsPath;
  यदि (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    यदि (std::error_code EC = FS->makeAbsolute(AbsPath))
      प्रत्यागम make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = सत्य;
  RelativeNames = सत्य;
  यदि (Error Err = expandResponseFile(CfgFile, Argv))
    प्रत्यागम Err;
  प्रत्यागम expandResponseFiles(Argv);
}

स्थिर शून्य initCommonOptions();
स्थिर तार्किक parse_Command_ओळ_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ध्रुव वर्ण *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ध्रुव वर्ण *EnvVar) {
  initCommonOptions();
  SmallVector<ध्रुव वर्ण *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ढकला_back(Args[0]);

  // Parse options from environment variable.
  यदि (EnvVar) {
    यदि (std::optional<std::string> Env_मूल्य =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ओळ(*Env_मूल्य, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  पूर्णाङ्क NewArgc = static_cast<पूर्णाङ्क>(NewArgv.size());

  // Parse all options.
  प्रत्यागम globalParser().Parse_Command_ओळ_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

तार्किक cl::Parse_Command_ओळ_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_ओळ_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

तार्किक cl::parse_Command_ओळ_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ध्रुव वर्ण *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_ओळ_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
शून्य Command_ओळ_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  प्रति (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    प्रति (auto &O : SC->OptionsMap)
      Opts.ढकला_back(O.second);
    प्रति (Option *O : Opts)
      O->reset();
    प्रति (Option *O : SC->PositionalOpts)
      O->reset();
    यदि (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  प्रति (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

तार्किक Command_ओळ_Parser::Parse_Command_ओळ_Options(
    पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  प्रतिज्ञा(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  तार्किक IgnoreErrors = Errs;
  यदि (!Errs)
    Errs = &errs();
  यदि (!VFS)
    VFS = vfs::getRealFileSystem().get();
  तार्किक ErrorParsing = असत्य;

  // Expand response files.
  SmallVector<ध्रुव वर्ण *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ओळ;
#अन्यथा
  auto Tokenize = cl::Tokenize_GNUCommand_ओळ;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  यदि (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णाङ्क>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned अंक_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  तार्किक HasUnlimitedPositionals = असत्य;

  पूर्णाङ्क FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शीर्ष_Level();
  std::string NearestSubCommandString;
  तार्किक MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  यदि (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    यदि (ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  प्रतिज्ञा(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  यदि (ConsumeAfterOpt) {
    प्रतिज्ञा(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  यदि (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    तार्किक UnboundedFound = असत्य;
    प्रति (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      यदि (Requires_मूल्य(Opt))
        ++अंक_Positional_Required;
      अन्यथा यदि (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        यदि (PositionalOpts.size() > 1) {
          यदि (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = सत्य;
        }
      } अन्यथा यदि (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        यदि (!IgnoreErrors)
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
  SmallVector<std::pair<StringRef, unsigned>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *ActivePositionalArg = nullptr;

  // Loop over all of the arguments... processing them.
  तार्किक DashDashFound = असत्य; // Have we read '--'?
  प्रति (पूर्णाङ्क i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef मूल्य;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    यदि (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      यदि (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }

      यदि (!PositionalOpts.empty()) {
        PositionalVals.ढकला_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        यदि (PositionalVals.size() >= अंक_Positional_Required && ConsumeAfterOpt) {
          प्रति (++i; i < argc; ++i)
            PositionalVals.ढकला_back(std::make_pair(StringRef(argv[i]), i));
          भङ्ग; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        अनुवर्त;
      }
    } अन्यथा यदि (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = सत्य; // This is the mythical "--"?
      अनुवर्त;             // Don't try to process it as an argument itself.
    } अन्यथा यदि (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);
      यदि (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }
    } अन्यथा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मूल्य);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      यदि (!Handler && ChosenSubCommand != &SubCommand::get_शीर्ष_Level())
        Handler = LookupOption(SubCommand::get_शीर्ष_Level(), ArgName, मूल्य);

      यदि (!Handler) {
        यदि (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ध्रुव वर्ण *> Rest(argv + i, argc - i);
          unsigned N;
          यदि (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = सत्य;
          }
          i += N - 1;
          अनुवर्त;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      यदि (!Handler)
        Handler = HandlePrefixedOption(ArgName, मूल्य, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      यदि (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    यदि (!Handler) {
      auto ReportUnknownArgument = [&](तार्किक IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        यदि (NearestArgumentName.empty())
          प्रत्यागम;

        *Errs << ProgramName << ": Did you mean '";
        यदि (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        अन्यथा
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      यदि (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/सत्य, NearestHandlerString);
      अन्यथा
        ReportUnknownArgument(/*IsArg=*/असत्य, NearestSubCommandString);

      ErrorParsing = सत्य;
      अनुवर्त;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    यदि (Handler->getFormattingFlag() == cl::Positional) {
      यदि ((Handler->getMiscFlags() & PositionalEatsArgs) && !मूल्य.empty()) {
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
  यदि (अंक_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अंक_Positional_Required
             << " positional argument" << (अंक_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = सत्य;
  } अन्यथा यदि (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = सत्य;

  } अन्यथा यदि (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, अंक_Vals = static_cast<unsigned>(PositionalVals.size());
    प्रति (Option *Opt : PositionalOpts) {
      यदि (Requires_मूल्य(Opt)) {
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
      यावत् (अंक_Vals - ValNo > अंक_Positional_Required && !Done) {
        विकल्प (Opt->get_अंक_Occurrences_Flag()) {
        पक्ष cl::Optional:
          Done = सत्य; // Optional arguments want _at most_ one value
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
  } अन्यथा {
    प्रतिज्ञा(ConsumeAfterOpt && अंक_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    प्रति (Option *Opt : PositionalOpts)
      यदि (Requires_मूल्य(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    यदि (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
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
    विकल्प (Opt.second->get_अंक_Occurrences_Flag()) {
    पक्ष Required:
    पक्ष OneOrMore:
      यदि (Opt.second->get_अंक_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = सत्य;
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
  यदि (ErrorParsing) {
    यदि (!IgnoreErrors)
      exit(1);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

तार्किक Option::error(ध्रुव Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  यदि (!ArgName.data())
    ArgName = ArgStr;
  यदि (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  अन्यथा
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  प्रत्यागम सत्य;
}

तार्किक Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef मूल्य) {
  ++अंक_Occurrences;
  प्रत्यागम handleOccurrence(pos, ArgName, मूल्य);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
स्थिर StringRef get_मूल्य_Str(ध्रुव Option &O, StringRef Default_संदेश) {
  यदि (O.मूल्य_Str.empty())
    प्रत्यागम Default_संदेश;
  प्रत्यागम O.मूल्य_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ध्रुव {
  प्रत्यागम argPlusPrefixesSize(ArgStr);
}

शून्य Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_ओळ_Indented_By) {
  प्रतिज्ञा(Indent >= First_ओळ_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_ओळ_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  यावत् (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

शून्य Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_ओळ_Indented_By) {
  ध्रुव StringRef ValHelpPrefix = "  ";
  प्रतिज्ञा(BaseIndent >= First_ओळ_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_ओळ_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  यावत् (!Split.second.empty()) {
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
  auto ValName = get_मूल्य_Name();
  यदि (!ValName.empty()) {
    size_t FormattingLen = 3;
    यदि (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_मूल्य_Str(O, ValName).size() + FormattingLen;
  }

  प्रत्यागम Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य basic_parser_impl::printOptionInfo(ध्रुव Option &O,
                                        size_t GlobalWidth) ध्रुव {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_मूल्य_Name();
  यदि (!ValName.empty()) {
    यदि (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_मूल्य_Str(O, ValName) << ">...";
    } अन्यथा यदि (O.get_मूल्य_Expected_Flag() == मूल्य_Optional)
      outs() << "[=<" << get_मूल्य_Str(O, ValName) << ">]";
    अन्यथा {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_मूल्य_Str(O, ValName)
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
                         तार्किक &मूल्य) {
  प्रत्यागम parseBool<तार्किक, सत्य, असत्य>(O, ArgName, Arg, मूल्य);
}

// parser<boolOrDefault> implementation
//
तार्किक parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मूल्य) {
  प्रत्यागम parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, मूल्य);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
स्थिर तार्किक parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef मूल्य_Kind,
                                         FixedOrScalableQuantityT &मूल्य) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_मूल्य;
  यदि (!Arg.getAsInteger(0, Min_मूल्य)) {
    मूल्य = FixedOrScalableQuantityT::getFixed(Min_मूल्य);
    प्रत्यागम असत्य;
  }

  StringRef Remainder = Arg;
  यदि (!Remainder.consume_front("vscale"))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  यदि (!Remainder.consume_front('x'))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  यदि (Remainder.getAsInteger(0, Min_मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मूल्य_Kind +
                   " argument!");

  मूल्य = FixedOrScalableQuantityT::getScalable(Min_मूल्य);
  प्रत्यागम असत्य;
}

// parser<int> implementation
//
तार्किक parser<पूर्णाङ्क>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णाङ्क &मूल्य) {
  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for integer argument!");
  प्रत्यागम असत्य;
}

// parser<long> implementation
//
तार्किक parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &मूल्य) {
  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for long argument!");
  प्रत्यागम असत्य;
}

// parser<long long> implementation
//
तार्किक parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &मूल्य) {
  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for llong argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned> implementation
//
तार्किक parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &मूल्य) {

  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for uint argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned long> implementation
//
तार्किक parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &मूल्य) {

  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ulong argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned long long> implementation
//
तार्किक parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &मूल्य) {

  यदि (Arg.getAsInteger(0, मूल्य))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ullong argument!");
  प्रत्यागम असत्य;
}

// parser<ElementCount> implementation
//
तार्किक parser<Element_मोजणी>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_मोजणी &मूल्य) {
  प्रत्यागम parseFixedOrScalableQuantity(O, Arg, get_मूल्य_Name(), मूल्य);
}

// parser<double>/parser<float> implementation
//
स्थिर तार्किक parseDouble(Option &O, StringRef Arg, द्विगुण &मूल्य) {
  यदि (to_float(Arg, मूल्य))
    प्रत्यागम असत्य;
  प्रत्यागम O.error("'" + Arg + "' value invalid for floating point argument!");
}

तार्किक parser<द्विगुण>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुण &Val) {
  प्रत्यागम parseDouble(O, Arg, Val);
}

तार्किक parser<भिन्न>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          भिन्न &Val) {
  द्विगुण dVal;
  यदि (parseDouble(O, Arg, dVal))
    प्रत्यागम सत्य;
  Val = (भिन्न)dVal;
  प्रत्यागम असत्य;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_अंक_Options();

  प्रति (unsigned i = 0; i != e; ++i) {
    यदि (getOption(i) == Name)
      प्रत्यागम i;
  }
  प्रत्यागम e;
}

स्थिर StringRef Eq_मूल्य = "=<value>";
स्थिर StringRef EmptyOption = "<empty>";
स्थिर StringRef OptionPrefix = "    =";
स्थिर size_t getOptionPrefixesSize() {
  प्रत्यागम OptionPrefix.size() + ArgHelpPrefix.size();
}

स्थिर तार्किक shouldPrintOption(StringRef Name, StringRef Description,
                              ध्रुव Option &O) {
  प्रत्यागम O.get_मूल्य_Expected_Flag() != मूल्य_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ध्रुव Option &O) ध्रुव {
  यदि (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_मूल्य.size();
    प्रति (unsigned i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      यदि (!shouldPrintOption(Name, getDescription(i), O))
        अनुवर्त;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    प्रत्यागम Size;
  } अन्यथा {
    size_t BaseSize = 0;
    प्रति (unsigned i = 0, e = get_अंक_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    प्रत्यागम BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य generic_parser_base::printOptionInfo(ध्रुव Option &O,
                                          size_t GlobalWidth) ध्रुव {
  यदि (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    यदि (O.get_मूल्य_Expected_Flag() == मूल्य_Optional) {
      प्रति (unsigned i = 0, e = get_अंक_Options(); i != e; ++i) {
        यदि (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          भङ्ग;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_मूल्य;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_मूल्य.size() +
                             argPlusPrefixesSize(O.ArgStr));
    प्रति (unsigned i = 0, e = get_अंक_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      यदि (!shouldPrintOption(OptionName, Description, O))
        अनुवर्त;
      size_t First_ओळ_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      यदि (OptionName.empty()) {
        outs() << EmptyOption;
        प्रतिज्ञा(First_ओळ_Indent >= EmptyOption.size());
        First_ओळ_Indent += EmptyOption.size();
      }
      यदि (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_ओळ_Indent);
      अन्यथा
        outs() << '\n';
    }
  } अन्यथा {
    यदि (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    प्रति (unsigned i = 0, e = get_अंक_Options(); i != e; ++i) {
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
    ध्रुव Option &O, ध्रुव Generic_Option_मूल्य &मूल्य,
    ध्रुव Generic_Option_मूल्य &Default, size_t GlobalWidth) ध्रुव {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned अंक_Opts = get_अंक_Options();
  प्रति (unsigned i = 0; i != अंक_Opts; ++i) {
    यदि (!मूल्य.compare(get_Option_मूल्य(i)))
      अनुवर्त;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t अंक_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(अंक_Spaces) << " (default: ";
    प्रति (unsigned j = 0; j != अंक_Opts; ++j) {
      यदि (!Default.compare(get_Option_मूल्य(j)))
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
  शून्य parser<T>::printOptionDiff(ध्रुव Option &O, T V, Option_मूल्य<T> D,      \
                                  size_t GlobalWidth) ध्रुव {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t अंक_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(अंक_Spaces) << " (default: ";                                 \
    यदि (D.has_मूल्य())                                                          \
      outs() << D.get_मूल्य();                                                  \
    अन्यथा                                                                       \
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
PRINT_OPT_DIFF(Element_मोजणी)

शून्य parser<std::string>::printOptionDiff(ध्रुव Option &O, StringRef V,
                                          ध्रुव Option_मूल्य<std::string> &D,
                                          size_t GlobalWidth) ध्रुव {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t अंक_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(अंक_Spaces) << " (default: ";
  यदि (D.has_मूल्य())
    outs() << D.get_मूल्य();
  अन्यथा
    outs() << "*no default*";
  outs() << ")\n";
}

शून्य parser<std::optional<std::string>>::printOptionDiff(
    ध्रुव Option &O, std::optional<StringRef> V,
    ध्रुव Option_मूल्य<std::optional<std::string>> &D,
    size_t GlobalWidth) ध्रुव {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_मूल्य_2() ? V.मूल्य_2().size() : 0;
  size_t अंक_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(अंक_Spaces) << " (default: ";
  यदि (D.has_मूल्य() && D.get_मूल्य().has_मूल्य_2())
    outs() << D.get_मूल्य();
  अन्यथा
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
शून्य basic_parser_impl::print_Option_No_मूल्य(ध्रुव Option &O,
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
    यदि (I->second->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    // Unless showhidden is set, ignore hidden flags.
    यदि (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      अनुवर्त;

    // If we've already seen this option, don't add it to the list again.
    यदि (!OptionSet.घाला(I->second).second)
      अनुवर्त;

    Opts.ढकला_back(
        std::pair<ध्रुव वर्ण *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

स्थिर शून्य
sortSubCommands(ध्रुव SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ध्रुव वर्ण *, SubCommand *>> &Subs) {
  प्रति (auto *S : SubMap) {
    यदि (S->getName().empty())
      अनुवर्त;
    Subs.ढकला_back(std::make_pair(S->getName().data(), S));
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
      यदि (!S.second->getDescription().empty()) {
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
  शून्य operator=(तार्किक मूल्य) {
    यदि (!मूल्य)
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

    यदि (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    यदि (Sub == &SubCommand::get_शीर्ष_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      यदि (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } अन्यथा {
      यदि (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    प्रति (auto *Opt : PositionalOpts) {
      यदि (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    यदि (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    यदि (Sub == &SubCommand::get_शीर्ष_Level() && !Subs.empty()) {
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
    यदि (ShowHidden)
      प्रति (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              यदि (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    प्रति (ध्रुव auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    यदि (!LibraryOpts.empty())
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
        CategorizedOptions[Cat].ढकला_back(Opt);
      }
    }

    // Now do printing.
    प्रति (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ध्रुव auto &CategoryOptions = CategorizedOptions[Category];
      यदि (CategoryOptions.empty())
        अनुवर्त;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      यदि (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      अन्यथा
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
  शून्य operator=(तार्किक मूल्य);
};

} // End anonymous namespace

#यदि defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# यदि defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#नोचेद्यदि defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# यदि defined(_DEBUG)
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
  शून्य print(ध्रुव std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अन्यथा
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#यदि LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अन्यथा
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    यदि (!ExtraPrinters.empty()) {
      प्रति (ध्रुव auto &I : ExtraPrinters)
        I(outs());
    }
  }
  शून्य operator=(तार्किक OptionWasSpecified);
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
  प्रत्यागम GeneralCategory;
}

शून्य VersionPrinter::operator=(तार्किक OptionWasSpecified) {
  यदि (!OptionWasSpecified)
    प्रत्यागम;

  यदि (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

शून्य HelpPrinterWrapper::operator=(तार्किक मूल्य) {
  यदि (!मूल्य)
    प्रत्यागम;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  यदि (globalParser().RegisteredOptionCategories.size() > 1) {
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
  यदि (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    प्रत्यागम;

  SmallVector<std::pair<ध्रुव वर्ण *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ सत्य);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  प्रति (ध्रुव auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  प्रति (ध्रुव auto &Opt : Opts)
    Opt.second->print_Option_मूल्य(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
शून्य cl::PrintHelpMessage(तार्किक Hidden, तार्किक Categorized) {
  यदि (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  अन्यथा यदि (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  अन्यथा यदि (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अन्यथा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  स्थिर ध्रुव StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#यदि LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#यदि __has_feature(address_sanitizer)
      "+asan",
#endif
#यदि __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#यदि __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#यदि __has_feature(memory_sanitizer)
      "+msan",
#endif
#यदि __has_feature(thread_sanitizer)
      "+tsan",
#endif
#यदि __has_feature(undefined_behavior_sanitizer)
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
#यदि LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
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
    तार्किक Unrelated = सत्य;
    प्रति (auto &Cat : I.second->Categories) {
      यदि (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    यदि (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::HideUnrelatedOptions(ArrayRef<ध्रुव cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  प्रति (auto &I : Sub.OptionsMap) {
    तार्किक Unrelated = सत्य;
    प्रति (auto &Cat : I.second->Categories) {
      यदि (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    यदि (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

शून्य cl::Reset_Command_ओळ_Parser() { globalParser().reset(); }

शून्य cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ढकला_back(&L);
}

शून्य LibraryOptionsContext::assign(ध्रुव LibraryOptionsContext &Other) {
  प्रतिज्ञा(एतत् != &Other);
  मूल्ये.clear();
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
  auto [It, Inserted] = मूल्ये.try_emplace(L.getDefault());
  यदि (Inserted)
    It->second = L.copyDefault(Alloc);
  प्रत्यागम parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

शून्य cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

शून्य LLVMParse_Command_ओळ_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 ध्रुव वर्ण *Overview) {
  llvm::cl::Parse_Command_ओळ_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
