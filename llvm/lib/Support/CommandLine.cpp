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

#ضمن "llvm/Support/CommandLine.h"

#ضمن "DebugOptions.h"

#ضمن "llvm-c/Support.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/STLFunctionalExtras.h"
#ضمن "llvm/ADT/SmallPtrSet.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/LibraryOptionsContext.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/StringSaver.h"
#ضمن "llvm/Support/TypeSize.h"
#ضمن "llvm/Support/VirtualFileSystem.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cstdlib>
#ضمن <optional>
#ضمن <string>
using نطاق llvm;
using نطاق cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نطاق llvm {
نطاق cl {
template صنف LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<long>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
template صنف LLVM_EXPORT_TEMPLATE basic_parser<Element_سانى>;

#إذا !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template صنف LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template صنف LLVM_EXPORT_TEMPLATE opt<منطقي>;
template صنف LLVM_EXPORT_TEMPLATE opt<حرف>;
template صنف LLVM_EXPORT_TEMPLATE opt<صحيح>;
template صنف LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
فراغ Generic_Option_قىممەت::anchor() {}
فراغ Option_قىممەت<boolOrDefault>::anchor() {}
فراغ Option_قىممەت<std::string>::anchor() {}
فراغ Option::anchor() {}
فراغ basic_parser_impl::anchor() {}
فراغ parser<منطقي>::anchor() {}
فراغ parser<boolOrDefault>::anchor() {}
فراغ parser<صحيح>::anchor() {}
فراغ parser<long>::anchor() {}
فراغ parser<long long>::anchor() {}
فراغ parser<unsigned>::anchor() {}
فراغ parser<unsigned long>::anchor() {}
فراغ parser<unsigned long long>::anchor() {}
فراغ parser<مزدوج>::anchor() {}
فراغ parser<عائم>::anchor() {}
فراغ parser<std::string>::anchor() {}
فراغ parser<std::optional<std::string>>::anchor() {}
فراغ parser<حرف>::anchor() {}
فراغ parser<Element_سانى>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
فراغ opt_bool_anchor() { opt<منطقي> anchor{""}; }
فراغ opt_char_anchor() { opt<حرف> anchor{""}; }
فراغ opt_int_anchor() { opt<صحيح> anchor{""}; }
فراغ opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساكن size_t DefaultPad = 2;

ساكن StringRef ArgPrefix = "-";
ساكن StringRef ArgPrefixLong = "--";
ساكن StringRef ArgHelpPrefix = " - ";

ساكن size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  إذا (Len == 1)
    أرجع Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  أرجع Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

ساكن SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  لكل (size_t I = 0; I < Pad; ++I) {
    Prefix.ئىتتىرىش_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  أرجع Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

صنف PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  أرجع OS;
}

صنف Command_قۇر_Parser {
عام:
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
  size_t رەقەم_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_قۇر_Parser() { registerSubCommand(&SubCommand::get_چوققا_Level()); }

  فراغ ResetAllOptionOccurrences();

  منطقي Parse_Command_قۇر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    إذا (Opt.Subs.empty()) {
      Action(SubCommand::get_چوققا_Level());
      أرجع;
    }
    إذا (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      لكل (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      أرجع;
    }
    لكل (auto *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  فراغ addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    إذا (Opt.hasArgStr())
      أرجع;
    إذا (!SC->OptionsMap.قىستۇرۇش(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  فراغ addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  فراغ addOption(Option *O, SubCommand *SC) {
    منطقي HadErrors = خطأ;
    إذا (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      إذا (!SC->OptionsMap.قىستۇرۇش(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = true;
      }
    }

    // Remember information about positional options.
    إذا (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ئىتتىرىش_back(O);
    وإلا إذا (O->get_رەقەم_Occurrences_Flag() == cl::ConsumeAfter) {
      إذا (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = true;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    إذا (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  فراغ addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    إذا (O->hasArgStr())
      OptionNames.ئىتتىرىش_back(O->ArgStr);

    SubCommand &Sub = *SC;
    لكل (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      إذا (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    إذا (O->getFormattingFlag() == cl::Positional)
      لكل (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        إذا (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    وإلا إذا (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    أرجع (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    لكل (ثابت auto *S : RegisteredSubCommands) {
      إذا (hasOptions(*S))
        أرجع true;
    }
    أرجع !Libraries.empty();
  }

  منطقي hasNamedSubCommands() ثابت {
    لكل (ثابت auto *S : RegisteredSubCommands)
      إذا (!S->getName().empty())
        أرجع true;
    أرجع خطأ;
  }

  SubCommand *getActiveSubCommand() { أرجع ActiveSubCommand; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    إذا (!Sub.OptionsMap.قىستۇرۇش(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  فراغ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  فراغ print_Option_قىممەتلەر();

  فراغ indexLibraryOptions() {
    منطقي HadErrors = خطأ;
    لكل (; رەقەم_Indexed_Libraries != Libraries.size(); ++رەقەم_Indexed_Libraries) {
      LibraryOptions *L = Libraries[رەقەم_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        إذا (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 أرجع SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          أرجع;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = true;
      });
    }
    إذا (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    أرجع LibraryIndex.lookup(Name);
  }

  فراغ registerCategory(OptionCategory *cat) {
    أكد(سانى_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             أرجع cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.قىستۇرۇش(cat);
  }

  فراغ registerSubCommand(SubCommand *sub) {
    أكد(سانى_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      أرجع (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.قىستۇرۇش(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    لكل (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      إذا (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      وإلا
        addLiteralOption(*O, sub, E.first);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    أرجع make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  فراغ reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    رەقەم_Indexed_Libraries = 0;

    SubCommand::get_چوققا_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_چوققا_Level());
  }

خاص:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &قىممەت);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساكن Command_قۇر_Parser &globalParser() {
  ساكن ManagedStatic<Command_قۇر_Parser> GlobalParser;
  أرجع *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ساكن Error
parseLibraryOption(LibraryOptions &L, فراغ *Opts, ArrayRef<ثابت حرف *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ثابت حرف *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  unsigned رەقەم_Args = std::min<size_t>(Args.size(), 2);
  لكل (unsigned J = 0; J != رەقەم_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  إذا (Error E = L.parse(Opts, ArrayRef(Saved, رەقەم_Args), Consumed, Alloc))
    أرجع E;
  إذا (Record)
    Record->append(Saved, Saved + Consumed);
  أرجع Error::success();
}

template <typename T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &قىممەت) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  إذا (!Arg.data() || Arg == "true" || Arg == "1") {
    قىممەت = TrueVal;
    أرجع خطأ;
  }

  إذا (Arg == "false" || Arg == "0") {
    قىممەت = FalseVal;
    أرجع خطأ;
  }
  أرجع O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

فراغ cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ئىتتىرىش_back(Help);
}

Option::Option(رەقەم_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : رەقەم_Occurrences(0), Occurrences(OccurrencesFlag), قىممەت(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(خطأ), Position(0) {
  Categories.ئىتتىرىش_back(&getGeneralCategory());
}

فراغ Option::addArgument() {
  globalParser().addOption(هذا);
  FullyInitialized = true;
}

فراغ Option::removeArgument() { globalParser().removeOption(هذا); }

فراغ Option::setArgStr(StringRef S) {
  إذا (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::addCategory(OptionCategory &C) {
  أكد(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  إذا (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وإلا إذا (!is_contained(Categories, &C))
    Categories.ئىتتىرىش_back(&C);
}

فراغ Option::reset() {
  رەقەم_Occurrences = 0;
  setDefault();
}

فراغ OptionCategory::registerCategory() {
  globalParser().registerCategory(هذا);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_چوققا_Level() {
  ساكن ManagedStatic<SubCommand> چوققا_Level_Sub_Command;
  أرجع *چوققا_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  أرجع *AllSubCommands;
}

فراغ SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(هذا);
}

فراغ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(هذا);
}

فراغ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator منطقي() ثابت {
  أرجع (globalParser().getActiveSubCommand() == هذا);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_قۇر_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &قىممەت) {
  // Reject all dashes.
  إذا (Arg.empty())
    أرجع nullptr;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  إذا (EqualPos == StringRef::npos) {
    // Look up the option.
    أرجع Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  إذا (I == Sub.OptionsMap.end())
    أرجع nullptr;

  auto *O = I->second;
  إذا (O->getFormattingFlag() == cl::AlwaysPrefix)
    أرجع nullptr;

  قىممەت = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  أرجع I->second;
}

SubCommand *Command_قۇر_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  إذا (Name.empty())
    أرجع &SubCommand::get_چوققا_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  لكل (auto *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    إذا (S->getName().empty())
      استمر;

    إذا (S->getName() == Name)
      أرجع S;

    إذا (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  إذا (NearestMatch)
    NearestString = NearestMatch->getName();

  أرجع &SubCommand::get_چوققا_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *LookupNearestOption(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  إذا (Arg.empty())
    أرجع nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  لكل (ثابت auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    إذا (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    إذا (O->hasArgStr())
      OptionNames.ئىتتىرىش_back(O->ArgStr);

    منطقي Permit_قىممەت = O->get_قىممەت_Expected_Flag() != cl::قىممەت_Disallowed;
    StringRef Flag = Permit_قىممەت ? LHS : Arg;
    لكل (ثابت auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/true, /*MaxEditDistance=*/BestDistance);
      إذا (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        إذا (RHS.empty() || !Permit_قىممەت)
          NearestString = std::string(Name);
        وإلا
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  أرجع Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef قىممەت) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  إذا (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(قىممەت);
    StringRef::size_type Pos = Val.find(',');

    طالما (Pos != StringRef::npos) {
      // Process the portion before the comma.
      إذا (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        أرجع true;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    قىممەت = Val;
  }

  أرجع Handler->addOccurrence(pos, ArgName, قىممەت);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساكن مضمن منطقي ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef قىممەت, صحيح argc,
                                 ثابت حرف *ثابت *argv, صحيح &i) {
  // Enforce value requirements
  بدل (Handler->get_قىممەت_Expected_Flag()) {
  حالة قىممەت_Required:
    إذا (!قىممەت.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      إذا (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        أرجع Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      قىممەت = StringRef(argv[++i]);
    }
    اكسر;
  حالة قىممەت_Disallowed:
    إذا (قىممەت.data())
      أرجع Handler->error("does not allow a value! '" + Twine(قىممەت) +
                            "' specified.");
    اكسر;
  حالة قىممەت_Optional:
    اكسر;
  }

  أرجع CommaSeparateAndAddOccurrence(Handler, i, ArgName, قىممەت);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  أرجع ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *findPrefixOption(StringRef Name, size_t &Length,
                                ثابت OptionsMapTy &OptionsMap) {
  لكل (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    إذا (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      أرجع O;
    }
  }
  أرجع nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &قىممەت,
                                    ثابت OptionsMapTy &OptionsMap) {
  إذا (Arg.size() == 1)
    أرجع nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  إذا (!POpt)
    أرجع nullptr;

  StringRef Maybe_قىممەت =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  إذا (POpt->getFormattingFlag() == cl::Prefix && Maybe_قىممەت.starts_with("="))
    Maybe_قىممەت = Maybe_قىممەت.drop_front();
  قىممەت = Maybe_قىممەت;
  أرجع POpt;
}

ساكن منطقي Requires_قىممەت(ثابت Option *O) {
  أرجع O->get_رەقەم_Occurrences_Flag() == cl::Required ||
         O->get_رەقەم_Occurrences_Flag() == cl::OneOrMore;
}

ساكن منطقي Eats_Unbounded_سان_Of_قىممەتلەر(ثابت Option *O) {
  أرجع O->get_رەقەم_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_رەقەم_Occurrences_Flag() == cl::OneOrMore;
}

ساكن منطقي isWhitespace(حرف C) {
  أرجع C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  أرجع isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { أرجع C == '\"' || C == '\''; }

فراغ cl::Tokenize_GNUCommand_قۇر(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي InToken = خطأ;
  لكل (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    إذا (!InToken) {
      طالما (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        إذا (MarkEOLs && Src[I] == '\n')
          NewArgv.ئىتتىرىش_back(nullptr);
        ++I;
      }
      إذا (I == E)
        اكسر;
      InToken = true;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    إذا (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ئىتتىرىش_back(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    إذا (isQuote(C)) {
      ++I;
      طالما (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        إذا (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ئىتتىرىش_back(Src[I]);
        ++I;
      }
      إذا (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    إذا (isWhitespace(C)) {
      NewArgv.ئىتتىرىش_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      إذا (MarkEOLs && C == '\n')
        NewArgv.ئىتتىرىش_back(nullptr);
      Token.clear();
      InToken = خطأ;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.ئىتتىرىش_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  إذا (InToken)
    NewArgv.ئىتتىرىش_back(Saver.save(Token.str()).data());
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
ساكن size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  صحيح Backslash_سانى = 0;
  // Skip the backslashes.
  افعل {
    ++I;
    ++Backslash_سانى;
  } طالما (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  إذا (FollowedByDoubleQuote) {
    Token.append(Backslash_سانى / 2, '\\');
    إذا (Backslash_سانى % 2 == 0)
      أرجع I - 1;
    Token.ئىتتىرىش_back('"');
    أرجع I;
  }
  Token.append(Backslash_سانى, '\\');
  أرجع I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي isWindowsSpecialCharInCommandName(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenize_Windows_Command_قۇر_Impl(
    StringRef Src, StringSaver &Saver, function_ref<فراغ(StringRef)> AddToken,
    منطقي AlwaysCopy, function_ref<فراغ()> MarkEOL, منطقي InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  منطقي CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  تعداد { INIT, UNQUOTED, QUOTED } State = INIT;

  لكل (size_t I = 0, E = Src.size(); I < E; ++I) {
    بدل (State) {
    حالة INIT: {
      أكد(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      طالما (I < E && isWhitespaceOrNull(Src[I])) {
        إذا (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      إذا (I >= E)
        اكسر;
      size_t Start = I;
      إذا (CommandName) {
        طالما (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } وإلا {
        طالما (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      إذا (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        إذا (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } وإلا {
          CommandName = خطأ;
        }
      } وإلا إذا (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } وإلا إذا (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } وإلا {
        llvm_unreachable("unexpected special character");
      }
      اكسر;
    }

    حالة UNQUOTED:
      إذا (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        إذا (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } وإلا {
          CommandName = خطأ;
        }
        State = INIT;
      } وإلا إذا (Src[I] == '\"') {
        State = QUOTED;
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.ئىتتىرىش_back(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      إذا (Src[I] == '\"') {
        إذا (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ئىتتىرىش_back('"');
          ++I;
        } وإلا {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.ئىتتىرىش_back(Src[I]);
      }
      اكسر;
    }
  }

  إذا (State != INIT)
    AddToken(Saver.save(Token.str()));
}

فراغ cl::Tokenize_Windows_Command_قۇر(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ئىتتىرىش_back(Tok.data()); };
  auto OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ئىتتىرىش_back(nullptr);
  };
  tokenize_Windows_Command_قۇر_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_قۇر_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ئىتتىرىش_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_قۇر_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/خطأ,
                                 OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_قۇر_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ئىتتىرىش_back(Tok.data()); };
  auto OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ئىتتىرىش_back(nullptr);
  };
  tokenize_Windows_Command_قۇر_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, true);
}

فراغ cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  لكل (ثابت حرف *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> قۇر;
    // Check for comment line.
    إذا (isWhitespace(*Cur)) {
      طالما (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    إذا (*Cur == '#') {
      طالما (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *Start = Cur;
    ثابت حرف *قۇر_End = nullptr;
    حرف Quote = 0;
    منطقي AtTokenStart = true;
    لكل (ثابت حرف *End = Source.end(); Cur != End; ++Cur) {
      حرف C = *Cur;
      إذا (C == '\\') {
        إذا (Cur + 1 != End) {
          ++Cur;
          إذا (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            قۇر.append(Start, Cur - 1);
            إذا (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            استمر;
          }
        }
        AtTokenStart = خطأ;
        استمر;
      }
      إذا (C == '\n')
        اكسر;
      إذا (Quote) {
        إذا (C == Quote)
          Quote = 0;
      } وإلا إذا (isQuote(C)) {
        Quote = C;
      } وإلا إذا (C == '#' && AtTokenStart) {
        قۇر_End = Cur;
        طالما (Cur != End && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    قۇر.append(Start, قۇر_End ? قۇر_End : Cur);
    cl::Tokenize_GNUCommand_قۇر(قۇر, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي hasUTF8ByteOrderMark(ArrayRef<حرف> S) {
  أرجع (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  لكل (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    إذا (ResponseFile.empty())
      ResponseFile = LHS;
    وإلا
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  إذا (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef Remaining = ArgString.substr(StartPos);
    إذا (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ثابت حرف *> &NewArgv) {
  أكد(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  إذا (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    أرجع llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  إذا (hasUTF16ByteOrderMark(BufRef)) {
    إذا (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      أرجع llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  وإلا إذا (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  إذا (!RelativeNames && !InConfigFile)
    أرجع Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  لكل (ثابت حرف *&Arg : NewArgv) {
    إذا (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    إذا (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = خطأ;
    إذا (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      إذا (!llvm::sys::path::is_relative(FileName))
        استمر;
    } وإلا إذا (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = true;
    } وإلا {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ئىتتىرىش_back('@');
    إذا (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      إذا (!findConfigFile(FileName, FilePath))
        أرجع createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } وإلا {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  أرجع Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ثابت حرف *> &Argv) {
  بنية ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_دۆۋە;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_دۆۋە.ئىتتىرىش_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  لكل (unsigned I = 0; I != Argv.size();) {
    طالما (I == File_دۆۋە.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_دۆۋە.چىقىرىش_back();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    إذا (Arg == nullptr) {
      ++I;
      استمر;
    }

    إذا (Arg[0] != '@') {
      ++I;
      استمر;
    }

    ثابت حرف *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    إذا (llvm::sys::path::is_relative(FName)) {
      إذا (CurrentDir.empty()) {
        إذا (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وإلا {
          أرجع createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وإلا {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    إذا (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      إذا (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        إذا (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          استمر;
        }
      }
      إذا (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      أرجع createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ثابت llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, هذا](ثابت ResponseFileRecord &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      إذا (!RHS)
        أرجع RHS.getError();
      أرجع FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    لكل (ثابت auto &F : drop_begin(File_دۆۋە)) {
      إذا (ErrorOr<منطقي> R = IsEquivalent(F)) {
        إذا (R.get())
          أرجع createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } وإلا {
        أرجع createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ثابت حرف *, 0> ExpandedArgv;
    إذا (Error Err = expandResponseFile(FName, ExpandedArgv))
      أرجع Err;

    لكل (ResponseFileRecord &Record : File_دۆۋە) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_دۆۋە.ئىتتىرىش_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.قىستۇرۇش(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_دۆۋە.size() > 0 && Argv.size() == File_دۆۋە.back().End);
  أرجع Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ثابت حرف *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_قۇر;
#وإلا
  auto Tokenize = cl::Tokenize_GNUCommand_قۇر;
#endif
  // The environment variable specifies initial options.
  إذا (EnvVar)
    إذا (std::optional<std::string> Env_قىممەت = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_قىممەت, Saver, NewArgv, /*MarkEOLs=*/خطأ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  إذا (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع true;
}

منطقي cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  إذا (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع true;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقي ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<حرف> &FilePath) {
  SmallString<128> CfgFilePath;
  ثابت auto FileExists = [هذا](SmallString<128> Path) -> منطقي {
    auto Status = FS->status(Path);
    أرجع Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  إذا (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    إذا (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      أرجع خطأ;
    إذا (!FileExists(CfgFilePath))
      أرجع خطأ;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    أرجع true;
  }

  // Look for the file in search directories.
  لكل (ثابت StringRef &Dir : ئىزدەش_Dirs) {
    إذا (Dir.empty())
      استمر;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    إذا (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      أرجع true;
    }
  }

  أرجع خطأ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ثابت حرف *> &Argv) {
  SmallString<128> AbsPath;
  إذا (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    إذا (std::error_code EC = FS->makeAbsolute(AbsPath))
      أرجع make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = true;
  RelativeNames = true;
  إذا (Error Err = expandResponseFile(CfgFile, Argv))
    أرجع Err;
  أرجع expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parse_Command_قۇر_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  SmallVector<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ئىتتىرىش_back(Args[0]);

  // Parse options from environment variable.
  إذا (EnvVar) {
    إذا (std::optional<std::string> Env_قىممەت =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_قۇر(*Env_قىممەت, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.size());

  // Parse all options.
  أرجع globalParser().Parse_Command_قۇر_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::Parse_Command_قۇر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_قۇر_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parse_Command_قۇر_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_قۇر_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
فراغ Command_قۇر_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  لكل (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    لكل (auto &O : SC->OptionsMap)
      Opts.ئىتتىرىش_back(O.second);
    لكل (Option *O : Opts)
      O->reset();
    لكل (Option *O : SC->PositionalOpts)
      O->reset();
    إذا (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  لكل (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

منطقي Command_قۇر_Parser::Parse_Command_قۇر_Options(
    صحيح argc, ثابت حرف *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  منطقي IgnoreErrors = Errs;
  إذا (!Errs)
    Errs = &errs();
  إذا (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقي ErrorParsing = خطأ;

  // Expand response files.
  SmallVector<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_قۇر;
#وإلا
  auto Tokenize = cl::Tokenize_GNUCommand_قۇر;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  إذا (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned رەقەم_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = خطأ;

  صحيح FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_چوققا_Level();
  std::string NearestSubCommandString;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  إذا (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    إذا (ChosenSubCommand != &SubCommand::get_چوققا_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  أكد(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  إذا (ConsumeAfterOpt) {
    أكد(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  إذا (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    منطقي UnboundedFound = خطأ;
    لكل (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      إذا (Requires_قىممەت(Opt))
        ++رەقەم_Positional_Required;
      وإلا إذا (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        إذا (PositionalOpts.size() > 1) {
          إذا (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = true;
        }
      } وإلا إذا (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        إذا (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = true;
      }
      UnboundedFound |= Eats_Unbounded_سان_Of_قىممەتلەر(Opt);
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
  منطقي DashDashFound = خطأ; // Have we read '--'?
  لكل (صحيح i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef قىممەت;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    إذا (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      إذا (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      إذا (!PositionalOpts.empty()) {
        PositionalVals.ئىتتىرىش_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        إذا (PositionalVals.size() >= رەقەم_Positional_Required && ConsumeAfterOpt) {
          لكل (++i; i < argc; ++i)
            PositionalVals.ئىتتىرىش_back(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } وإلا إذا (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = true; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } وإلا إذا (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قىممەت);
      إذا (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        استمر; // We are done!
      }
    } وإلا { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قىممەت);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      إذا (!Handler && ChosenSubCommand != &SubCommand::get_چوققا_Level())
        Handler = LookupOption(SubCommand::get_چوققا_Level(), ArgName, قىممەت);

      إذا (!Handler) {
        إذا (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          unsigned N;
          إذا (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = true;
          }
          i += N - 1;
          استمر;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      إذا (!Handler)
        Handler = HandlePrefixedOption(ArgName, قىممەت, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      إذا (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    إذا (!Handler) {
      auto ReportUnknownArgument = [&](منطقي IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        إذا (NearestArgumentName.empty())
          أرجع;

        *Errs << ProgramName << ": Did you mean '";
        إذا (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        وإلا
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      إذا (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/true, NearestHandlerString);
      وإلا
        ReportUnknownArgument(/*IsArg=*/خطأ, NearestSubCommandString);

      ErrorParsing = true;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    إذا (Handler->getFormattingFlag() == cl::Positional) {
      إذا ((Handler->getMiscFlags() & PositionalEatsArgs) && !قىممەت.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = true;
      }
      ActivePositionalArg = Handler;
    }
    وإلا
      ErrorParsing |= ProvideOption(Handler, ArgName, قىممەت, argc, argv, i);
  }

  // Check and handle positional arguments now...
  إذا (رەقەم_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << رەقەم_Positional_Required
             << " positional argument" << (رەقەم_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = true;
  } وإلا إذا (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = true;

  } وإلا إذا (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, رەقەم_Vals = static_cast<unsigned>(PositionalVals.size());
    لكل (Option *Opt : PositionalOpts) {
      إذا (Requires_قىممەت(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --رەقەم_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي Done = Opt->get_رەقەم_Occurrences_Flag() == cl::Required;
      طالما (رەقەم_Vals - ValNo > رەقەم_Positional_Required && !Done) {
        بدل (Opt->get_رەقەم_Occurrences_Flag()) {
        حالة cl::Optional:
          Done = true; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالة cl::ZeroOrMore: // Zero or more will take all they can get...
        حالة cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          اكسر;
        افتراضي:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وإلا {
    أكد(ConsumeAfterOpt && رەقەم_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    لكل (Option *Opt : PositionalOpts)
      إذا (Requires_قىممەت(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    إذا (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    لكل (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  لكل (ثابت auto &Opt : OptionsMap) {
    بدل (Opt.second->get_رەقەم_Occurrences_Flag()) {
    حالة Required:
    حالة OneOrMore:
      إذا (Opt.second->get_رەقەم_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = true;
      }
      [[fallthrough]];
    افتراضي:
      اكسر;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             لكل (صحيح i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  إذا (ErrorParsing) {
    إذا (!IgnoreErrors)
      exit(1);
    أرجع خطأ;
  }
  أرجع true;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  إذا (!ArgName.data())
    ArgName = ArgStr;
  إذا (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  وإلا
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  أرجع true;
}

منطقي Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef قىممەت) {
  ++رەقەم_Occurrences;
  أرجع handleOccurrence(pos, ArgName, قىممەت);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef get_قىممەت_Str(ثابت Option &O, StringRef Default_ئۇچۇر) {
  إذا (O.قىممەت_Str.empty())
    أرجع Default_ئۇچۇر;
  أرجع O.قىممەت_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ثابت {
  أرجع argPlusPrefixesSize(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_قۇر_Indented_By) {
  أكد(Indent >= First_قۇر_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_قۇر_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  طالما (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_قۇر_Indented_By) {
  ثابت StringRef ValHelpPrefix = "  ";
  أكد(BaseIndent >= First_قۇر_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_قۇر_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  طالما (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
فراغ alias::printOptionInfo(size_t GlobalWidth) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(ثابت Option &O) ثابت {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_قىممەت_Name();
  إذا (!ValName.empty()) {
    size_t FormattingLen = 3;
    إذا (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_قىممەت_Str(O, ValName).size() + FormattingLen;
  }

  أرجع Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t GlobalWidth) ثابت {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_قىممەت_Name();
  إذا (!ValName.empty()) {
    إذا (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_قىممەت_Str(O, ValName) << ">...";
    } وإلا إذا (O.get_قىممەت_Expected_Flag() == قىممەت_Optional)
      outs() << "[=<" << get_قىممەت_Str(O, ValName) << ">]";
    وإلا {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_قىممەت_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

فراغ basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t GlobalWidth) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
منطقي parser<منطقي>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقي &قىممەت) {
  أرجع parseBool<منطقي, true, خطأ>(O, ArgName, Arg, قىممەت);
}

// parser<boolOrDefault> implementation
//
منطقي parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &قىممەت) {
  أرجع parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, قىممەت);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef قىممەت_Kind,
                                         FixedOrScalableQuantityT &قىممەت) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_قىممەت;
  إذا (!Arg.getAsInteger(0, Min_قىممەت)) {
    قىممەت = FixedOrScalableQuantityT::getFixed(Min_قىممەت);
    أرجع خطأ;
  }

  StringRef Remainder = Arg;
  إذا (!Remainder.consume_front("vscale"))
    أرجع O.error("'" + Arg + "' value invalid for " + قىممەت_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (!Remainder.consume_front('x'))
    أرجع O.error("'" + Arg + "' value invalid for " + قىممەت_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (Remainder.getAsInteger(0, Min_قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for " + قىممەت_Kind +
                   " argument!");

  قىممەت = FixedOrScalableQuantityT::getScalable(Min_قىممەت);
  أرجع خطأ;
}

// parser<int> implementation
//
منطقي parser<صحيح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحيح &قىممەت) {
  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for integer argument!");
  أرجع خطأ;
}

// parser<long> implementation
//
منطقي parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &قىممەت) {
  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for long argument!");
  أرجع خطأ;
}

// parser<long long> implementation
//
منطقي parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &قىممەت) {
  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for llong argument!");
  أرجع خطأ;
}

// parser<unsigned> implementation
//
منطقي parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &قىممەت) {

  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for uint argument!");
  أرجع خطأ;
}

// parser<unsigned long> implementation
//
منطقي parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &قىممەت) {

  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for ulong argument!");
  أرجع خطأ;
}

// parser<unsigned long long> implementation
//
منطقي parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &قىممەت) {

  إذا (Arg.getAsInteger(0, قىممەت))
    أرجع O.error("'" + Arg + "' value invalid for ullong argument!");
  أرجع خطأ;
}

// parser<ElementCount> implementation
//
منطقي parser<Element_سانى>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_سانى &قىممەت) {
  أرجع parseFixedOrScalableQuantity(O, Arg, get_قىممەت_Name(), قىممەت);
}

// parser<double>/parser<float> implementation
//
ساكن منطقي parseDouble(Option &O, StringRef Arg, مزدوج &قىممەت) {
  إذا (to_float(Arg, قىممەت))
    أرجع خطأ;
  أرجع O.error("'" + Arg + "' value invalid for floating point argument!");
}

منطقي parser<مزدوج>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           مزدوج &Val) {
  أرجع parseDouble(O, Arg, Val);
}

منطقي parser<عائم>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          عائم &Val) {
  مزدوج dVal;
  إذا (parseDouble(O, Arg, dVal))
    أرجع true;
  Val = (عائم)dVal;
  أرجع خطأ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_رەقەم_Options();

  لكل (unsigned i = 0; i != e; ++i) {
    إذا (getOption(i) == Name)
      أرجع i;
  }
  أرجع e;
}

ساكن StringRef Eq_قىممەت = "=<value>";
ساكن StringRef EmptyOption = "<empty>";
ساكن StringRef OptionPrefix = "    =";
ساكن size_t getOptionPrefixesSize() {
  أرجع OptionPrefix.size() + ArgHelpPrefix.size();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  أرجع O.get_قىممەت_Expected_Flag() != قىممەت_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ثابت Option &O) ثابت {
  إذا (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_قىممەت.size();
    لكل (unsigned i = 0, e = get_رەقەم_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      إذا (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    أرجع Size;
  } وإلا {
    size_t BaseSize = 0;
    لكل (unsigned i = 0, e = get_رەقەم_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    أرجع BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_base::printOptionInfo(ثابت Option &O,
                                          size_t GlobalWidth) ثابت {
  إذا (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    إذا (O.get_قىممەت_Expected_Flag() == قىممەت_Optional) {
      لكل (unsigned i = 0, e = get_رەقەم_Options(); i != e; ++i) {
        إذا (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_قىممەت;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_قىممەت.size() +
                             argPlusPrefixesSize(O.ArgStr));
    لكل (unsigned i = 0, e = get_رەقەم_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      إذا (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t First_قۇر_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      إذا (OptionName.empty()) {
        outs() << EmptyOption;
        أكد(First_قۇر_Indent >= EmptyOption.size());
        First_قۇر_Indent += EmptyOption.size();
      }
      إذا (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_قۇر_Indent);
      وإلا
        outs() << '\n';
    }
  } وإلا {
    إذا (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    لكل (unsigned i = 0, e = get_رەقەم_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

ساكن ثابت size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
فراغ generic_parser_base::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_قىممەت &قىممەت,
    ثابت Generic_Option_قىممەت &Default, size_t GlobalWidth) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned رەقەم_Opts = get_رەقەم_Options();
  لكل (unsigned i = 0; i != رەقەم_Opts; ++i) {
    إذا (!قىممەت.compare(get_Option_قىممەت(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t رەقەم_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(رەقەم_Spaces) << " (default: ";
    لكل (unsigned j = 0; j != رەقەم_Opts; ++j) {
      إذا (!Default.compare(get_Option_قىممەت(j)))
        استمر;
      outs() << getOption(j);
      اكسر;
    }
    outs() << ")\n";
    أرجع;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
نطاق llvm {
نطاق cl {
ساكن raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  أرجع OS << static_cast<صحيح>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, Option_قىممەت<T> D,      \
                                  size_t GlobalWidth) ثابت {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t رەقەم_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(رەقەم_Spaces) << " (default: ";                                 \
    إذا (D.has_قىممەت())                                                          \
      outs() << D.get_قىممەت();                                                  \
    وإلا                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقي)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحيح)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(مزدوج)
PRINT_OPT_DIFF(عائم)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_سانى)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_قىممەت<std::string> &D,
                                          size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t رەقەم_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(رەقەم_Spaces) << " (default: ";
  إذا (D.has_قىممەت())
    outs() << D.get_قىممەت();
  وإلا
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_قىممەت<std::optional<std::string>> &D,
    size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_قىممەت_2() ? V.قىممەت_2().size() : 0;
  size_t رەقەم_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(رەقەم_Spaces) << " (default: ";
  إذا (D.has_قىممەت() && D.get_قىممەت().has_قىممەت_2())
    outs() << D.get_قىممەت();
  وإلا
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::print_Option_No_قىممەت(ثابت Option &O,
                                           size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  أرجع strcmp(LHS->first, RHS->first);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  أرجع strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  لكل (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    إذا (I->second->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    إذا (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    إذا (!OptionSet.قىستۇرۇش(I->second).second)
      استمر;

    Opts.ئىتتىرىش_back(
        std::pair<ثابت حرف *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

ساكن فراغ
sortSubCommands(ثابت SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  لكل (auto *S : SubMap) {
    إذا (S->getName().empty())
      استمر;
    Subs.ئىتتىرىش_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

نطاق {

صنف HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ثابت حرف *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  virtual فراغ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    لكل (ثابت auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    لكل (ثابت auto &S : Subs) {
      outs() << "  " << S.first;
      إذا (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

عام:
  explicit HelpPrinter(منطقي showHidden) : ShowHidden(showHidden) {}
  virtual ~HelpPrinter() = افتراضي;

  // Invoke the printer.
  فراغ operator=(منطقي قىممەت) {
    إذا (!قىممەت)
      أرجع;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  فراغ printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    إذا (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    إذا (Sub == &SubCommand::get_چوققا_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      إذا (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وإلا {
      إذا (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    لكل (auto *Opt : PositionalOpts) {
      إذا (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    إذا (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    إذا (Sub == &SubCommand::get_چوققا_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      لكل (ثابت auto &Sub : Subs)
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
    لكل (ثابت auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    إذا (ShowHidden)
      لكل (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              إذا (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    لكل (ثابت auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    إذا (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    لكل (ثابت auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    لكل (ثابت auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

صنف CategorizedHelpPrinter : عام HelpPrinter {
عام:
  explicit CategorizedHelpPrinter(منطقي showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساكن صحيح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    أرجع (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

محمي:
  فراغ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    لكل (ثابت auto &I : Opts) {
      Option *Opt = I.second;
      لكل (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ئىتتىرىش_back(Opt);
      }
    }

    // Now do printing.
    لكل (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت auto &CategoryOptions = CategorizedOptions[Category];
      إذا (CategoryOptions.empty())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      إذا (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      وإلا
        outs() << "\n";

      // Loop over the options in the category and print.
      لكل (ثابت Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
صنف HelpPrinterWrapper {
خاص:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

عام:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  فراغ operator=(منطقي قىممەت);
};

} // End anonymous namespace

#إذا defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# إذا defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# وإلا
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#وإلاإذا defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# إذا defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# وإلا
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#وإلا
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

نطاق {
صنف VersionPrinter {
عام:
  فراغ print(ثابت std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وإلا
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#إذا LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وإلا
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    إذا (!ExtraPrinters.empty()) {
      لكل (ثابت auto &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ operator=(منطقي OptionWasSpecified);
};

بنية Command_قۇر_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{خطأ};
  HelpPrinter UncategorizedHiddenPrinter{true};
  CategorizedHelpPrinter CategorizedNormalPrinter{خطأ};
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
  cl::opt<HelpPrinter, true, parser<منطقي>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::قىممەت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, true, parser<منطقي>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::قىممەت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, true, parser<منطقي>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::قىممەت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, true, parser<منطقي>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::قىممەت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقي> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(خطأ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقي> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(خطأ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, true, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::قىممەت_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساكن ManagedStatic<Command_قۇر_Common_Options> CommonOptions;

ساكن فراغ initCommonOptions() {
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
  ساكن OptionCategory GeneralCategory{"General options"};
  أرجع GeneralCategory;
}

فراغ VersionPrinter::operator=(منطقي OptionWasSpecified) {
  إذا (!OptionWasSpecified)
    أرجع;

  إذا (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

فراغ HelpPrinterWrapper::operator=(منطقي قىممەت) {
  إذا (!قىممەت)
    أرجع;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  إذا (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = true; // Invoke categorized printer
  } وإلا {
    UncategorizedPrinter = true; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::Print_Option_قىممەتلەر() { globalParser().print_Option_قىممەتلەر(); }

فراغ Command_قۇر_Parser::print_Option_قىممەتلەر() {
  إذا (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    أرجع;

  SmallVector<std::pair<ثابت حرف *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ true);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  لكل (ثابت auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  لكل (ثابت auto &Opt : Opts)
    Opt.second->print_Option_قىممەت(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::PrintHelpMessage(منطقي Hidden, منطقي Categorized) {
  إذا (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  وإلا إذا (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  وإلا إذا (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  وإلا
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ساكن ثابت StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#إذا LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#إذا __has_feature(address_sanitizer)
      "+asan",
#endif
#إذا __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#إذا __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#إذا __has_feature(memory_sanitizer)
      "+msan",
#endif
#إذا __has_feature(thread_sanitizer)
      "+tsan",
#endif
#إذا __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  أرجع ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#إذا LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
فراغ cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

فراغ cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

فراغ cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.ئىتتىرىش_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.contains(&Sub));
  أرجع Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  أرجع globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  لكل (auto &I : Sub.OptionsMap) {
    منطقي Unrelated = true;
    لكل (auto &Cat : I.second->Categories) {
      إذا (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  لكل (auto &I : Sub.OptionsMap) {
    منطقي Unrelated = true;
    لكل (auto &Cat : I.second->Categories) {
      إذا (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

فراغ cl::Reset_Command_قۇر_Parser() { globalParser().reset(); }

فراغ cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ئىتتىرىش_back(&L);
}

فراغ LibraryOptionsContext::assign(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  قىممەتلەر.clear();
  SavedArgs.clear();
  ArrayRef<ثابت حرف *> Args = Other.SavedArgs;
  لكل (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<ثابت حرف *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = قىممەتلەر.try_emplace(L.getDefault());
  إذا (Inserted)
    It->second = L.copyDefault(Alloc);
  أرجع parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParse_Command_قۇر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_قۇر_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
