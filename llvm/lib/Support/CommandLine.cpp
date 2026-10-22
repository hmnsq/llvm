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

#شامل "llvm/Support/CommandLine.h"

#شامل "DebugOptions.h"

#شامل "llvm-c/Support.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/STLFunctionalExtras.h"
#شامل "llvm/ADT/SmallPtrSet.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/LibraryOptionsContext.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/StringSaver.h"
#شامل "llvm/Support/TypeSize.h"
#شامل "llvm/Support/VirtualFileSystem.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cstdlib>
#شامل <optional>
#شامل <string>
using فضاینام llvm;
using فضاینام cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
فضاینام llvm {
فضاینام cl {
template رده LLVM_EXPORT_TEMPLATE basic_parser<منطقی>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<صحیح>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<long>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<دوگانه>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<اعشاری>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<نویسه>;
template رده LLVM_EXPORT_TEMPLATE basic_parser<Element_شمار>;

#اگر !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template رده LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template رده LLVM_EXPORT_TEMPLATE opt<منطقی>;
template رده LLVM_EXPORT_TEMPLATE opt<نویسه>;
template رده LLVM_EXPORT_TEMPLATE opt<صحیح>;
template رده LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
تهی Generic_Option_مقدار::anchor() {}
تهی Option_مقدار<boolOrDefault>::anchor() {}
تهی Option_مقدار<std::string>::anchor() {}
تهی Option::anchor() {}
تهی basic_parser_impl::anchor() {}
تهی parser<منطقی>::anchor() {}
تهی parser<boolOrDefault>::anchor() {}
تهی parser<صحیح>::anchor() {}
تهی parser<long>::anchor() {}
تهی parser<long long>::anchor() {}
تهی parser<unsigned>::anchor() {}
تهی parser<unsigned long>::anchor() {}
تهی parser<unsigned long long>::anchor() {}
تهی parser<دوگانه>::anchor() {}
تهی parser<اعشاری>::anchor() {}
تهی parser<std::string>::anchor() {}
تهی parser<std::optional<std::string>>::anchor() {}
تهی parser<نویسه>::anchor() {}
تهی parser<Element_شمار>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
تهی opt_bool_anchor() { opt<منطقی> anchor{""}; }
تهی opt_char_anchor() { opt<نویسه> anchor{""}; }
تهی opt_int_anchor() { opt<صحیح> anchor{""}; }
تهی opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ایستا size_t DefaultPad = 2;

ایستا StringRef ArgPrefix = "-";
ایستا StringRef ArgPrefixLong = "--";
ایستا StringRef ArgHelpPrefix = " - ";

ایستا size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  اگر (Len == 1)
    بازگشت Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  بازگشت Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

ایستا SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  برای (size_t I = 0; I < Pad; ++I) {
    Prefix.افزودن_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  بازگشت Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

فضاینام {

رده PrintArg {
  StringRef ArgName;
  size_t Pad;
عمومی:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  بازگشت OS;
}

رده Command_سطر_Parser {
عمومی:
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
  size_t رقم_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_سطر_Parser() { registerSubCommand(&SubCommand::get_بالاترین_Level()); }

  تهی ResetAllOptionOccurrences();

  منطقی Parse_Command_سطر_Options(صحیح argc, ثابت نویسه *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  تهی forEachSubCommand(Option &Opt, function_ref<تهی(SubCommand &)> Action) {
    اگر (Opt.Subs.empty()) {
      Action(SubCommand::get_بالاترین_Level());
      بازگشت;
    }
    اگر (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      برای (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      بازگشت;
    }
    برای (auto *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  تهی addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    اگر (Opt.hasArgStr())
      بازگشت;
    اگر (!SC->OptionsMap.درج(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  تهی addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  تهی addOption(Option *O, SubCommand *SC) {
    منطقی HadErrors = نادرست;
    اگر (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      اگر (!SC->OptionsMap.درج(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = درست;
      }
    }

    // Remember information about positional options.
    اگر (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.افزودن_back(O);
    وگرنه اگر (O->get_رقم_Occurrences_Flag() == cl::ConsumeAfter) {
      اگر (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = درست;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    اگر (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  تهی addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  تهی removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    اگر (O->hasArgStr())
      OptionNames.افزودن_back(O->ArgStr);

    SubCommand &Sub = *SC;
    برای (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      اگر (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    اگر (O->getFormattingFlag() == cl::Positional)
      برای (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        اگر (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          بشکن;
        }
      }
    وگرنه اگر (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  تهی removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقی hasOptions(ثابت SubCommand &Sub) ثابت {
    بازگشت (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  منطقی hasOptions() ثابت {
    برای (ثابت auto *S : RegisteredSubCommands) {
      اگر (hasOptions(*S))
        بازگشت درست;
    }
    بازگشت !Libraries.empty();
  }

  منطقی hasNamedSubCommands() ثابت {
    برای (ثابت auto *S : RegisteredSubCommands)
      اگر (!S->getName().empty())
        بازگشت درست;
    بازگشت نادرست;
  }

  SubCommand *getActiveSubCommand() { بازگشت ActiveSubCommand; }

  تهی updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    اگر (!Sub.OptionsMap.درج(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  تهی updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  تهی print_Option_مقادیر();

  تهی indexLibraryOptions() {
    منطقی HadErrors = نادرست;
    برای (; رقم_Indexed_Libraries != Libraries.size(); ++رقم_Indexed_Libraries) {
      LibraryOptions *L = Libraries[رقم_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        اگر (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 بازگشت SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          بازگشت;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = درست;
      });
    }
    اگر (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    بازگشت LibraryIndex.lookup(Name);
  }

  تهی registerCategory(OptionCategory *cat) {
    أكد(شمار_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             بازگشت cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.درج(cat);
  }

  تهی registerSubCommand(SubCommand *sub) {
    أكد(شمار_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      بازگشت (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.درج(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    برای (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      اگر (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      وگرنه
        addLiteralOption(*O, sub, E.first);
    }
  }

  تهی unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    بازگشت make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  تهی reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    رقم_Indexed_Libraries = 0;

    SubCommand::get_بالاترین_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_بالاترین_Level());
  }

خصوصی:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &مقدار);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ایستا Command_سطر_Parser &globalParser() {
  ایستا ManagedStatic<Command_سطر_Parser> GlobalParser;
  بازگشت *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ایستا Error
parseLibraryOption(LibraryOptions &L, تهی *Opts, ArrayRef<ثابت نویسه *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ثابت نویسه *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ثابت نویسه *Saved[2];
  unsigned رقم_Args = std::min<size_t>(Args.size(), 2);
  برای (unsigned J = 0; J != رقم_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  اگر (Error E = L.parse(Opts, ArrayRef(Saved, رقم_Args), Consumed, Alloc))
    بازگشت E;
  اگر (Record)
    Record->append(Saved, Saved + Consumed);
  بازگشت Error::success();
}

template <typename T, T TrueVal, T FalseVal>
ایستا منطقی parseBool(Option &O, StringRef ArgName, StringRef Arg, T &مقدار) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  اگر (!Arg.data() || Arg == "true" || Arg == "1") {
    مقدار = TrueVal;
    بازگشت نادرست;
  }

  اگر (Arg == "false" || Arg == "0") {
    مقدار = FalseVal;
    بازگشت نادرست;
  }
  بازگشت O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

تهی cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.افزودن_back(Help);
}

Option::Option(رقم_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : رقم_Occurrences(0), Occurrences(OccurrencesFlag), مقدار(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(نادرست), Position(0) {
  Categories.افزودن_back(&getGeneralCategory());
}

تهی Option::addArgument() {
  globalParser().addOption(این);
  FullyInitialized = درست;
}

تهی Option::removeArgument() { globalParser().removeOption(این); }

تهی Option::setArgStr(StringRef S) {
  اگر (FullyInitialized)
    globalParser().updateArgStr(این, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

تهی Option::addCategory(OptionCategory &C) {
  أكد(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  اگر (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وگرنه اگر (!is_contained(Categories, &C))
    Categories.افزودن_back(&C);
}

تهی Option::reset() {
  رقم_Occurrences = 0;
  setDefault();
}

تهی OptionCategory::registerCategory() {
  globalParser().registerCategory(این);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_بالاترین_Level() {
  ایستا ManagedStatic<SubCommand> بالاترین_Level_Sub_Command;
  بازگشت *بالاترین_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ایستا ManagedStatic<SubCommand> AllSubCommands;
  بازگشت *AllSubCommands;
}

تهی SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(این);
}

تهی SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(این);
}

تهی SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator منطقی() ثابت {
  بازگشت (globalParser().getActiveSubCommand() == این);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_سطر_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &مقدار) {
  // Reject all dashes.
  اگر (Arg.empty())
    بازگشت nullptr;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  اگر (EqualPos == StringRef::npos) {
    // Look up the option.
    بازگشت Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  اگر (I == Sub.OptionsMap.end())
    بازگشت nullptr;

  auto *O = I->second;
  اگر (O->getFormattingFlag() == cl::AlwaysPrefix)
    بازگشت nullptr;

  مقدار = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  بازگشت I->second;
}

SubCommand *Command_سطر_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  اگر (Name.empty())
    بازگشت &SubCommand::get_بالاترین_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  برای (auto *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    اگر (S->getName().empty())
      ادامه;

    اگر (S->getName() == Name)
      بازگشت S;

    اگر (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  اگر (NearestMatch)
    NearestString = NearestMatch->getName();

  بازگشت &SubCommand::get_بالاترین_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ایستا Option *LookupNearestOption(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  اگر (Arg.empty())
    بازگشت nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  برای (ثابت auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    اگر (O->getOptionHiddenFlag() == ReallyHidden)
      ادامه;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    اگر (O->hasArgStr())
      OptionNames.افزودن_back(O->ArgStr);

    منطقی Permit_مقدار = O->get_مقدار_Expected_Flag() != cl::مقدار_Disallowed;
    StringRef Flag = Permit_مقدار ? LHS : Arg;
    برای (ثابت auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/درست, /*MaxEditDistance=*/BestDistance);
      اگر (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        اگر (RHS.empty() || !Permit_مقدار)
          NearestString = std::string(Name);
        وگرنه
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  بازگشت Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ایستا منطقی CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef مقدار) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  اگر (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(مقدار);
    StringRef::size_type Pos = Val.find(',');

    وقتی (Pos != StringRef::npos) {
      // Process the portion before the comma.
      اگر (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        بازگشت درست;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    مقدار = Val;
  }

  بازگشت Handler->addOccurrence(pos, ArgName, مقدار);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ایستا درونخطی منطقی ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef مقدار, صحیح argc,
                                 ثابت نویسه *ثابت *argv, صحیح &i) {
  // Enforce value requirements
  انتخاب (Handler->get_مقدار_Expected_Flag()) {
  حالت مقدار_Required:
    اگر (!مقدار.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      اگر (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        بازگشت Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      مقدار = StringRef(argv[++i]);
    }
    بشکن;
  حالت مقدار_Disallowed:
    اگر (مقدار.data())
      بازگشت Handler->error("does not allow a value! '" + Twine(مقدار) +
                            "' specified.");
    بشکن;
  حالت مقدار_Optional:
    بشکن;
  }

  بازگشت CommaSeparateAndAddOccurrence(Handler, i, ArgName, مقدار);
}

منطقی llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحیح i) {
  صحیح Dummy = i;
  بازگشت ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ایستا Option *findPrefixOption(StringRef Name, size_t &Length,
                                ثابت OptionsMapTy &OptionsMap) {
  برای (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    اگر (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      بازگشت O;
    }
  }
  بازگشت nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ایستا Option *HandlePrefixedOption(StringRef &Arg, StringRef &مقدار,
                                    ثابت OptionsMapTy &OptionsMap) {
  اگر (Arg.size() == 1)
    بازگشت nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  اگر (!POpt)
    بازگشت nullptr;

  StringRef Maybe_مقدار =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  اگر (POpt->getFormattingFlag() == cl::Prefix && Maybe_مقدار.starts_with("="))
    Maybe_مقدار = Maybe_مقدار.drop_front();
  مقدار = Maybe_مقدار;
  بازگشت POpt;
}

ایستا منطقی Requires_مقدار(ثابت Option *O) {
  بازگشت O->get_رقم_Occurrences_Flag() == cl::Required ||
         O->get_رقم_Occurrences_Flag() == cl::OneOrMore;
}

ایستا منطقی Eats_Unbounded_عدد_Of_مقادیر(ثابت Option *O) {
  بازگشت O->get_رقم_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_رقم_Occurrences_Flag() == cl::OneOrMore;
}

ایستا منطقی isWhitespace(نویسه C) {
  بازگشت C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ایستا منطقی isWhitespaceOrNull(نویسه C) {
  بازگشت isWhitespace(C) || C == '\0';
}

ایستا منطقی isQuote(نویسه C) { بازگشت C == '\"' || C == '\''; }

تهی cl::Tokenize_GNUCommand_سطر(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ثابت نویسه *> &NewArgv,
                                منطقی MarkEOLs) {
  SmallString<128> Token;
  منطقی InToken = نادرست;
  برای (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    اگر (!InToken) {
      وقتی (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        اگر (MarkEOLs && Src[I] == '\n')
          NewArgv.افزودن_back(nullptr);
        ++I;
      }
      اگر (I == E)
        بشکن;
      InToken = درست;
    }

    نویسه C = Src[I];

    // Backslash escapes the next character.
    اگر (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.افزودن_back(Src[I]);
      ادامه;
    }

    // Consume a quoted string.
    اگر (isQuote(C)) {
      ++I;
      وقتی (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        اگر (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.افزودن_back(Src[I]);
        ++I;
      }
      اگر (I == E)
        بشکن;
      ادامه;
    }

    // End the token if this is whitespace.
    اگر (isWhitespace(C)) {
      NewArgv.افزودن_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      اگر (MarkEOLs && C == '\n')
        NewArgv.افزودن_back(nullptr);
      Token.clear();
      InToken = نادرست;
      ادامه;
    }

    // This is a normal character.  Append it.
    Token.افزودن_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  اگر (InToken)
    NewArgv.افزودن_back(Saver.save(Token.str()).data());
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
ایستا size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  صحیح Backslash_شمار = 0;
  // Skip the backslashes.
  انجامبده {
    ++I;
    ++Backslash_شمار;
  } وقتی (I != E && Src[I] == '\\');

  منطقی FollowedByDoubleQuote = (I != E && Src[I] == '"');
  اگر (FollowedByDoubleQuote) {
    Token.append(Backslash_شمار / 2, '\\');
    اگر (Backslash_شمار % 2 == 0)
      بازگشت I - 1;
    Token.افزودن_back('"');
    بازگشت I;
  }
  Token.append(Backslash_شمار, '\\');
  بازگشت I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ایستا منطقی isWindowsSpecialChar(نویسه C) {
  بازگشت isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ایستا منطقی isWindowsSpecialCharInCommandName(نویسه C) {
  بازگشت isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ایستا درونخطی تهی tokenize_Windows_Command_سطر_Impl(
    StringRef Src, StringSaver &Saver, function_ref<تهی(StringRef)> AddToken,
    منطقی AlwaysCopy, function_ref<تهی()> MarkEOL, منطقی InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  منطقی CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  شمارش { INIT, UNQUOTED, QUOTED } State = INIT;

  برای (size_t I = 0, E = Src.size(); I < E; ++I) {
    انتخاب (State) {
    حالت INIT: {
      أكد(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      وقتی (I < E && isWhitespaceOrNull(Src[I])) {
        اگر (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      اگر (I >= E)
        بشکن;
      size_t Start = I;
      اگر (CommandName) {
        وقتی (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } وگرنه {
        وقتی (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      اگر (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        اگر (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } وگرنه {
          CommandName = نادرست;
        }
      } وگرنه اگر (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } وگرنه اگر (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } وگرنه {
        llvm_unreachable("unexpected special character");
      }
      بشکن;
    }

    حالت UNQUOTED:
      اگر (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        اگر (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } وگرنه {
          CommandName = نادرست;
        }
        State = INIT;
      } وگرنه اگر (Src[I] == '\"') {
        State = QUOTED;
      } وگرنه اگر (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وگرنه {
        Token.افزودن_back(Src[I]);
      }
      بشکن;

    حالت QUOTED:
      اگر (Src[I] == '\"') {
        اگر (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.افزودن_back('"');
          ++I;
        } وگرنه {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وگرنه اگر (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وگرنه {
        Token.افزودن_back(Src[I]);
      }
      بشکن;
    }
  }

  اگر (State != INIT)
    AddToken(Saver.save(Token.str()));
}

تهی cl::Tokenize_Windows_Command_سطر(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ثابت نویسه *> &NewArgv,
                                    منطقی MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.افزودن_back(Tok.data()); };
  auto OnEOL = [&]() {
    اگر (MarkEOLs)
      NewArgv.افزودن_back(nullptr);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/درست, OnEOL, نادرست);
}

تهی cl::Tokenize_Windows_Command_سطر_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.افزودن_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_سطر_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/نادرست,
                                 OnEOL, نادرست);
}

تهی cl::Tokenize_Windows_Command_سطر_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ثابت نویسه *> &NewArgv,
                                        منطقی MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.افزودن_back(Tok.data()); };
  auto OnEOL = [&]() {
    اگر (MarkEOLs)
      NewArgv.افزودن_back(nullptr);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/درست, OnEOL, درست);
}

تهی cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ثابت نویسه *> &NewArgv,
                            منطقی MarkEOLs) {
  برای (ثابت نویسه *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> سطر;
    // Check for comment line.
    اگر (isWhitespace(*Cur)) {
      وقتی (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      ادامه;
    }
    اگر (*Cur == '#') {
      وقتی (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      ادامه;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت نویسه *Start = Cur;
    ثابت نویسه *سطر_End = nullptr;
    نویسه Quote = 0;
    منطقی AtTokenStart = درست;
    برای (ثابت نویسه *End = Source.end(); Cur != End; ++Cur) {
      نویسه C = *Cur;
      اگر (C == '\\') {
        اگر (Cur + 1 != End) {
          ++Cur;
          اگر (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            سطر.append(Start, Cur - 1);
            اگر (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            ادامه;
          }
        }
        AtTokenStart = نادرست;
        ادامه;
      }
      اگر (C == '\n')
        بشکن;
      اگر (Quote) {
        اگر (C == Quote)
          Quote = 0;
      } وگرنه اگر (isQuote(C)) {
        Quote = C;
      } وگرنه اگر (C == '#' && AtTokenStart) {
        سطر_End = Cur;
        وقتی (Cur != End && *Cur != '\n')
          ++Cur;
        بشکن;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    سطر.append(Start, سطر_End ? سطر_End : Cur);
    cl::Tokenize_GNUCommand_سطر(سطر, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ایستا منطقی hasUTF8ByteOrderMark(ArrayRef<نویسه> S) {
  بازگشت (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ایستا تهی ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ثابت نویسه *&Arg) {
  أكد(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  برای (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    اگر (ResponseFile.empty())
      ResponseFile = LHS;
    وگرنه
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  اگر (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef Remaining = ArgString.substr(StartPos);
    اگر (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ثابت نویسه *> &NewArgv) {
  أكد(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  اگر (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    بازگشت llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<نویسه> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  اگر (hasUTF16ByteOrderMark(BufRef)) {
    اگر (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      بازگشت llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  وگرنه اگر (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  اگر (!RelativeNames && !InConfigFile)
    بازگشت Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  برای (ثابت نویسه *&Arg : NewArgv) {
    اگر (!Arg)
      ادامه;

    // Substitute <CFGDIR> with the file's base path.
    اگر (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقی ConfigInclusion = نادرست;
    اگر (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      اگر (!llvm::sys::path::is_relative(FileName))
        ادامه;
    } وگرنه اگر (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = درست;
    } وگرنه {
      ادامه;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.افزودن_back('@');
    اگر (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      اگر (!findConfigFile(FileName, FilePath))
        بازگشت createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } وگرنه {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  بازگشت Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ثابت نویسه *> &Argv) {
  ساختار ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_پشته;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_پشته.افزودن_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  برای (unsigned I = 0; I != Argv.size();) {
    وقتی (I == File_پشته.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_پشته.برداشت_back();
    }

    ثابت نویسه *Arg = Argv[I];
    // Check if it is an EOL marker
    اگر (Arg == nullptr) {
      ++I;
      ادامه;
    }

    اگر (Arg[0] != '@') {
      ++I;
      ادامه;
    }

    ثابت نویسه *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    اگر (llvm::sys::path::is_relative(FName)) {
      اگر (CurrentDir.empty()) {
        اگر (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وگرنه {
          بازگشت createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وگرنه {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    اگر (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      اگر (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        اگر (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ادامه;
        }
      }
      اگر (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      بازگشت createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ثابت llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, این](ثابت ResponseFileRecord &RFile) -> ErrorOr<منطقی> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      اگر (!RHS)
        بازگشت RHS.getError();
      بازگشت FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    برای (ثابت auto &F : drop_begin(File_پشته)) {
      اگر (ErrorOr<منطقی> R = IsEquivalent(F)) {
        اگر (R.get())
          بازگشت createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } وگرنه {
        بازگشت createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ثابت نویسه *, 0> ExpandedArgv;
    اگر (Error Err = expandResponseFile(FName, ExpandedArgv))
      بازگشت Err;

    برای (ResponseFileRecord &Record : File_پشته) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_پشته.افزودن_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.درج(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_پشته.size() > 0 && Argv.size() == File_پشته.back().End);
  بازگشت Error::success();
}

منطقی cl::expandResponseFiles(صحیح Argc, ثابت نویسه *ثابت *Argv,
                             ثابت نویسه *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ثابت نویسه *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_سطر;
#وگرنه
  auto Tokenize = cl::Tokenize_GNUCommand_سطر;
#endif
  // The environment variable specifies initial options.
  اگر (EnvVar)
    اگر (std::optional<std::string> Env_مقدار = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_مقدار, Saver, NewArgv, /*MarkEOLs=*/نادرست);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  اگر (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    بازگشت نادرست;
  }
  بازگشت درست;
}

منطقی cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ثابت نویسه *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  اگر (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    بازگشت نادرست;
  }
  بازگشت درست;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقی ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<نویسه> &FilePath) {
  SmallString<128> CfgFilePath;
  ثابت auto FileExists = [این](SmallString<128> Path) -> منطقی {
    auto Status = FS->status(Path);
    بازگشت Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  اگر (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    اگر (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      بازگشت نادرست;
    اگر (!FileExists(CfgFilePath))
      بازگشت نادرست;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    بازگشت درست;
  }

  // Look for the file in search directories.
  برای (ثابت StringRef &Dir : جستجو_Dirs) {
    اگر (Dir.empty())
      ادامه;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    اگر (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      بازگشت درست;
    }
  }

  بازگشت نادرست;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ثابت نویسه *> &Argv) {
  SmallString<128> AbsPath;
  اگر (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    اگر (std::error_code EC = FS->makeAbsolute(AbsPath))
      بازگشت make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = درست;
  RelativeNames = درست;
  اگر (Error Err = expandResponseFile(CfgFile, Argv))
    بازگشت Err;
  بازگشت expandResponseFiles(Argv);
}

ایستا تهی initCommonOptions();
ایستا منطقی parse_Command_سطر_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت نویسه *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت نویسه *EnvVar) {
  initCommonOptions();
  SmallVector<ثابت نویسه *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.افزودن_back(Args[0]);

  // Parse options from environment variable.
  اگر (EnvVar) {
    اگر (std::optional<std::string> Env_مقدار =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_سطر(*Env_مقدار, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  صحیح NewArgc = static_cast<صحیح>(NewArgv.size());

  // Parse all options.
  بازگشت globalParser().Parse_Command_سطر_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقی cl::Parse_Command_سطر_Options(صحیح argc, ثابت نویسه *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت نویسه *EnvVar) {
  بازگشت parse_Command_سطر_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقی cl::parse_Command_سطر_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت نویسه *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت نویسه *EnvVar) {
  بازگشت parse_Command_سطر_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
تهی Command_سطر_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  برای (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    برای (auto &O : SC->OptionsMap)
      Opts.افزودن_back(O.second);
    برای (Option *O : Opts)
      O->reset();
    برای (Option *O : SC->PositionalOpts)
      O->reset();
    اگر (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  برای (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

منطقی Command_سطر_Parser::Parse_Command_سطر_Options(
    صحیح argc, ثابت نویسه *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  منطقی IgnoreErrors = Errs;
  اگر (!Errs)
    Errs = &errs();
  اگر (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقی ErrorParsing = نادرست;

  // Expand response files.
  SmallVector<ثابت نویسه *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_سطر;
#وگرنه
  auto Tokenize = cl::Tokenize_GNUCommand_سطر;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  اگر (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    بازگشت نادرست;
  }
  argv = &newArgv[0];
  argc = static_cast<صحیح>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned رقم_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقی HasUnlimitedPositionals = نادرست;

  صحیح FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_بالاترین_Level();
  std::string NearestSubCommandString;
  منطقی MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  اگر (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    اگر (ChosenSubCommand != &SubCommand::get_بالاترین_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  أكد(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  اگر (ConsumeAfterOpt) {
    أكد(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  اگر (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    منطقی UnboundedFound = نادرست;
    برای (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      اگر (Requires_مقدار(Opt))
        ++رقم_Positional_Required;
      وگرنه اگر (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        اگر (PositionalOpts.size() > 1) {
          اگر (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = درست;
        }
      } وگرنه اگر (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        اگر (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = درست;
      }
      UnboundedFound |= Eats_Unbounded_عدد_Of_مقادیر(Opt);
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
  منطقی DashDashFound = نادرست; // Have we read '--'?
  برای (صحیح i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef مقدار;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    اگر (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      اگر (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ادامه; // We are done!
      }

      اگر (!PositionalOpts.empty()) {
        PositionalVals.افزودن_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        اگر (PositionalVals.size() >= رقم_Positional_Required && ConsumeAfterOpt) {
          برای (++i; i < argc; ++i)
            PositionalVals.افزودن_back(std::make_pair(StringRef(argv[i]), i));
          بشکن; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ادامه;
      }
    } وگرنه اگر (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = درست; // This is the mythical "--"?
      ادامه;             // Don't try to process it as an argument itself.
    } وگرنه اگر (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, مقدار);
      اگر (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ادامه; // We are done!
      }
    } وگرنه { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, مقدار);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      اگر (!Handler && ChosenSubCommand != &SubCommand::get_بالاترین_Level())
        Handler = LookupOption(SubCommand::get_بالاترین_Level(), ArgName, مقدار);

      اگر (!Handler) {
        اگر (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ثابت نویسه *> Rest(argv + i, argc - i);
          unsigned N;
          اگر (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = درست;
          }
          i += N - 1;
          ادامه;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      اگر (!Handler)
        Handler = HandlePrefixedOption(ArgName, مقدار, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      اگر (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    اگر (!Handler) {
      auto ReportUnknownArgument = [&](منطقی IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        اگر (NearestArgumentName.empty())
          بازگشت;

        *Errs << ProgramName << ": Did you mean '";
        اگر (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        وگرنه
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      اگر (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/درست, NearestHandlerString);
      وگرنه
        ReportUnknownArgument(/*IsArg=*/نادرست, NearestSubCommandString);

      ErrorParsing = درست;
      ادامه;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    اگر (Handler->getFormattingFlag() == cl::Positional) {
      اگر ((Handler->getMiscFlags() & PositionalEatsArgs) && !مقدار.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = درست;
      }
      ActivePositionalArg = Handler;
    }
    وگرنه
      ErrorParsing |= ProvideOption(Handler, ArgName, مقدار, argc, argv, i);
  }

  // Check and handle positional arguments now...
  اگر (رقم_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << رقم_Positional_Required
             << " positional argument" << (رقم_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = درست;
  } وگرنه اگر (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = درست;

  } وگرنه اگر (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, رقم_Vals = static_cast<unsigned>(PositionalVals.size());
    برای (Option *Opt : PositionalOpts) {
      اگر (Requires_مقدار(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --رقم_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقی Done = Opt->get_رقم_Occurrences_Flag() == cl::Required;
      وقتی (رقم_Vals - ValNo > رقم_Positional_Required && !Done) {
        انتخاب (Opt->get_رقم_Occurrences_Flag()) {
        حالت cl::Optional:
          Done = درست; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالت cl::ZeroOrMore: // Zero or more will take all they can get...
        حالت cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          بشکن;
        پیشفرض:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وگرنه {
    أكد(ConsumeAfterOpt && رقم_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    برای (Option *Opt : PositionalOpts)
      اگر (Requires_مقدار(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    اگر (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    برای (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  برای (ثابت auto &Opt : OptionsMap) {
    انتخاب (Opt.second->get_رقم_Occurrences_Flag()) {
    حالت Required:
    حالت OneOrMore:
      اگر (Opt.second->get_رقم_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = درست;
      }
      [[fallthrough]];
    پیشفرض:
      بشکن;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             برای (صحیح i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  اگر (ErrorParsing) {
    اگر (!IgnoreErrors)
      exit(1);
    بازگشت نادرست;
  }
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقی Option::error(ثابت Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  اگر (!ArgName.data())
    ArgName = ArgStr;
  اگر (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  وگرنه
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  بازگشت درست;
}

منطقی Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef مقدار) {
  ++رقم_Occurrences;
  بازگشت handleOccurrence(pos, ArgName, مقدار);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ایستا StringRef get_مقدار_Str(ثابت Option &O, StringRef Default_پیام) {
  اگر (O.مقدار_Str.empty())
    بازگشت Default_پیام;
  بازگشت O.مقدار_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ثابت {
  بازگشت argPlusPrefixesSize(ArgStr);
}

تهی Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_سطر_Indented_By) {
  أكد(Indent >= First_سطر_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_سطر_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  وقتی (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

تهی Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_سطر_Indented_By) {
  ثابت StringRef ValHelpPrefix = "  ";
  أكد(BaseIndent >= First_سطر_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_سطر_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  وقتی (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
تهی alias::printOptionInfo(size_t GlobalWidth) ثابت {
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
  auto ValName = get_مقدار_Name();
  اگر (!ValName.empty()) {
    size_t FormattingLen = 3;
    اگر (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_مقدار_Str(O, ValName).size() + FormattingLen;
  }

  بازگشت Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
تهی basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t GlobalWidth) ثابت {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_مقدار_Name();
  اگر (!ValName.empty()) {
    اگر (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_مقدار_Str(O, ValName) << ">...";
    } وگرنه اگر (O.get_مقدار_Expected_Flag() == مقدار_Optional)
      outs() << "[=<" << get_مقدار_Str(O, ValName) << ">]";
    وگرنه {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_مقدار_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

تهی basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t GlobalWidth) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
منطقی parser<منطقی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقی &مقدار) {
  بازگشت parseBool<منطقی, درست, نادرست>(O, ArgName, Arg, مقدار);
}

// parser<boolOrDefault> implementation
//
منطقی parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &مقدار) {
  بازگشت parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, مقدار);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
ایستا منطقی parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef مقدار_Kind,
                                         FixedOrScalableQuantityT &مقدار) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_مقدار;
  اگر (!Arg.getAsInteger(0, Min_مقدار)) {
    مقدار = FixedOrScalableQuantityT::getFixed(Min_مقدار);
    بازگشت نادرست;
  }

  StringRef Remainder = Arg;
  اگر (!Remainder.consume_front("vscale"))
    بازگشت O.error("'" + Arg + "' value invalid for " + مقدار_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  اگر (!Remainder.consume_front('x'))
    بازگشت O.error("'" + Arg + "' value invalid for " + مقدار_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  اگر (Remainder.getAsInteger(0, Min_مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for " + مقدار_Kind +
                   " argument!");

  مقدار = FixedOrScalableQuantityT::getScalable(Min_مقدار);
  بازگشت نادرست;
}

// parser<int> implementation
//
منطقی parser<صحیح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحیح &مقدار) {
  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for integer argument!");
  بازگشت نادرست;
}

// parser<long> implementation
//
منطقی parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &مقدار) {
  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for long argument!");
  بازگشت نادرست;
}

// parser<long long> implementation
//
منطقی parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &مقدار) {
  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for llong argument!");
  بازگشت نادرست;
}

// parser<unsigned> implementation
//
منطقی parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &مقدار) {

  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for uint argument!");
  بازگشت نادرست;
}

// parser<unsigned long> implementation
//
منطقی parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &مقدار) {

  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for ulong argument!");
  بازگشت نادرست;
}

// parser<unsigned long long> implementation
//
منطقی parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &مقدار) {

  اگر (Arg.getAsInteger(0, مقدار))
    بازگشت O.error("'" + Arg + "' value invalid for ullong argument!");
  بازگشت نادرست;
}

// parser<ElementCount> implementation
//
منطقی parser<Element_شمار>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_شمار &مقدار) {
  بازگشت parseFixedOrScalableQuantity(O, Arg, get_مقدار_Name(), مقدار);
}

// parser<double>/parser<float> implementation
//
ایستا منطقی parseDouble(Option &O, StringRef Arg, دوگانه &مقدار) {
  اگر (to_float(Arg, مقدار))
    بازگشت نادرست;
  بازگشت O.error("'" + Arg + "' value invalid for floating point argument!");
}

منطقی parser<دوگانه>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           دوگانه &Val) {
  بازگشت parseDouble(O, Arg, Val);
}

منطقی parser<اعشاری>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          اعشاری &Val) {
  دوگانه dVal;
  اگر (parseDouble(O, Arg, dVal))
    بازگشت درست;
  Val = (اعشاری)dVal;
  بازگشت نادرست;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_رقم_Options();

  برای (unsigned i = 0; i != e; ++i) {
    اگر (getOption(i) == Name)
      بازگشت i;
  }
  بازگشت e;
}

ایستا StringRef Eq_مقدار = "=<value>";
ایستا StringRef EmptyOption = "<empty>";
ایستا StringRef OptionPrefix = "    =";
ایستا size_t getOptionPrefixesSize() {
  بازگشت OptionPrefix.size() + ArgHelpPrefix.size();
}

ایستا منطقی shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  بازگشت O.get_مقدار_Expected_Flag() != مقدار_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ثابت Option &O) ثابت {
  اگر (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_مقدار.size();
    برای (unsigned i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      اگر (!shouldPrintOption(Name, getDescription(i), O))
        ادامه;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    بازگشت Size;
  } وگرنه {
    size_t BaseSize = 0;
    برای (unsigned i = 0, e = get_رقم_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    بازگشت BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
تهی generic_parser_base::printOptionInfo(ثابت Option &O,
                                          size_t GlobalWidth) ثابت {
  اگر (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    اگر (O.get_مقدار_Expected_Flag() == مقدار_Optional) {
      برای (unsigned i = 0, e = get_رقم_Options(); i != e; ++i) {
        اگر (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          بشکن;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_مقدار;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_مقدار.size() +
                             argPlusPrefixesSize(O.ArgStr));
    برای (unsigned i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      اگر (!shouldPrintOption(OptionName, Description, O))
        ادامه;
      size_t First_سطر_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      اگر (OptionName.empty()) {
        outs() << EmptyOption;
        أكد(First_سطر_Indent >= EmptyOption.size());
        First_سطر_Indent += EmptyOption.size();
      }
      اگر (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_سطر_Indent);
      وگرنه
        outs() << '\n';
    }
  } وگرنه {
    اگر (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    برای (unsigned i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

ایستا ثابت size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
تهی generic_parser_base::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_مقدار &مقدار,
    ثابت Generic_Option_مقدار &Default, size_t GlobalWidth) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned رقم_Opts = get_رقم_Options();
  برای (unsigned i = 0; i != رقم_Opts; ++i) {
    اگر (!مقدار.compare(get_Option_مقدار(i)))
      ادامه;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t رقم_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(رقم_Spaces) << " (default: ";
    برای (unsigned j = 0; j != رقم_Opts; ++j) {
      اگر (!Default.compare(get_Option_مقدار(j)))
        ادامه;
      outs() << getOption(j);
      بشکن;
    }
    outs() << ")\n";
    بازگشت;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
فضاینام llvm {
فضاینام cl {
ایستا raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  بازگشت OS << static_cast<صحیح>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  تهی parser<T>::printOptionDiff(ثابت Option &O, T V, Option_مقدار<T> D,      \
                                  size_t GlobalWidth) ثابت {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t رقم_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(رقم_Spaces) << " (default: ";                                 \
    اگر (D.has_مقدار())                                                          \
      outs() << D.get_مقدار();                                                  \
    وگرنه                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقی)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحیح)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(دوگانه)
PRINT_OPT_DIFF(اعشاری)
PRINT_OPT_DIFF(نویسه)
PRINT_OPT_DIFF(Element_شمار)

تهی parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_مقدار<std::string> &D,
                                          size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t رقم_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(رقم_Spaces) << " (default: ";
  اگر (D.has_مقدار())
    outs() << D.get_مقدار();
  وگرنه
    outs() << "*no default*";
  outs() << ")\n";
}

تهی parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_مقدار<std::optional<std::string>> &D,
    size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_مقدار_2() ? V.مقدار_2().size() : 0;
  size_t رقم_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(رقم_Spaces) << " (default: ";
  اگر (D.has_مقدار() && D.get_مقدار().has_مقدار_2())
    outs() << D.get_مقدار();
  وگرنه
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
تهی basic_parser_impl::print_Option_No_مقدار(ثابت Option &O,
                                           size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ایستا صحیح OptNameCompare(ثابت std::pair<ثابت نویسه *, Option *> *LHS,
                          ثابت std::pair<ثابت نویسه *, Option *> *RHS) {
  بازگشت strcmp(LHS->first, RHS->first);
}

ایستا صحیح SubNameCompare(ثابت std::pair<ثابت نویسه *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت نویسه *, SubCommand *> *RHS) {
  بازگشت strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
ایستا تهی sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ثابت نویسه *, Option *>> &Opts,
                     منطقی ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  برای (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    اگر (I->second->getOptionHiddenFlag() == ReallyHidden)
      ادامه;

    // Unless showhidden is set, ignore hidden flags.
    اگر (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ادامه;

    // If we've already seen this option, don't add it to the list again.
    اگر (!OptionSet.درج(I->second).second)
      ادامه;

    Opts.افزودن_back(
        std::pair<ثابت نویسه *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

ایستا تهی
sortSubCommands(ثابت SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ثابت نویسه *, SubCommand *>> &Subs) {
  برای (auto *S : SubMap) {
    اگر (S->getName().empty())
      ادامه;
    Subs.افزودن_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

فضاینام {

رده HelpPrinter {
محافظتشده:
  ثابت منطقی ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ثابت نویسه *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ثابت نویسه *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازی تهی printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    برای (ثابت auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  تهی printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    برای (ثابت auto &S : Subs) {
      outs() << "  " << S.first;
      اگر (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

عمومی:
  explicit HelpPrinter(منطقی showHidden) : ShowHidden(showHidden) {}
  مجازی ~HelpPrinter() = پیشفرض;

  // Invoke the printer.
  تهی operator=(منطقی مقدار) {
    اگر (!مقدار)
      بازگشت;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  تهی printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    اگر (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    اگر (Sub == &SubCommand::get_بالاترین_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      اگر (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وگرنه {
      اگر (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    برای (auto *Opt : PositionalOpts) {
      اگر (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    اگر (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    اگر (Sub == &SubCommand::get_بالاترین_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      برای (ثابت auto &Sub : Subs)
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
    برای (ثابت auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    اگر (ShowHidden)
      برای (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              اگر (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    برای (ثابت auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    اگر (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    برای (ثابت auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    برای (ثابت auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

رده CategorizedHelpPrinter : عمومی HelpPrinter {
عمومی:
  explicit CategorizedHelpPrinter(منطقی showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ایستا صحیح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    بازگشت (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

محافظتشده:
  تهی printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
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
    برای (ثابت auto &I : Opts) {
      Option *Opt = I.second;
      برای (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].افزودن_back(Opt);
      }
    }

    // Now do printing.
    برای (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت auto &CategoryOptions = CategorizedOptions[Category];
      اگر (CategoryOptions.empty())
        ادامه;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      اگر (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      وگرنه
        outs() << "\n";

      // Loop over the options in the category and print.
      برای (ثابت Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
رده HelpPrinterWrapper {
خصوصی:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

عمومی:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  تهی operator=(منطقی مقدار);
};

} // End anonymous namespace

#اگر defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# اگر defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# وگرنه
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#وگرنهاگر defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# اگر defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# وگرنه
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#وگرنه
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

فضاینام {
رده VersionPrinter {
عمومی:
  تهی print(ثابت std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وگرنه
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#اگر LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وگرنه
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    اگر (!ExtraPrinters.empty()) {
      برای (ثابت auto &I : ExtraPrinters)
        I(outs());
    }
  }
  تهی operator=(منطقی OptionWasSpecified);
};

ساختار Command_سطر_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{نادرست};
  HelpPrinter UncategorizedHiddenPrinter{درست};
  CategorizedHelpPrinter CategorizedNormalPrinter{نادرست};
  CategorizedHelpPrinter CategorizedHiddenPrinter{درست};
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
  cl::opt<HelpPrinter, درست, parser<منطقی>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, درست, parser<منطقی>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, درست, parser<منطقی>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, درست, parser<منطقی>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقی> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(نادرست),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقی> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(نادرست),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, درست, parser<منطقی>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::مقدار_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ایستا ManagedStatic<Command_سطر_Common_Options> CommonOptions;

ایستا تهی initCommonOptions() {
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
  ایستا OptionCategory GeneralCategory{"General options"};
  بازگشت GeneralCategory;
}

تهی VersionPrinter::operator=(منطقی OptionWasSpecified) {
  اگر (!OptionWasSpecified)
    بازگشت;

  اگر (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

تهی HelpPrinterWrapper::operator=(منطقی مقدار) {
  اگر (!مقدار)
    بازگشت;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  اگر (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = درست; // Invoke categorized printer
  } وگرنه {
    UncategorizedPrinter = درست; // Invoke uncategorized printer
  }
}

// Print the value of each option.
تهی cl::Print_Option_مقادیر() { globalParser().print_Option_مقادیر(); }

تهی Command_سطر_Parser::print_Option_مقادیر() {
  اگر (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    بازگشت;

  SmallVector<std::pair<ثابت نویسه *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ درست);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  برای (ثابت auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  برای (ثابت auto &Opt : Opts)
    Opt.second->print_Option_مقدار(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
تهی cl::PrintHelpMessage(منطقی Hidden, منطقی Categorized) {
  اگر (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  وگرنه اگر (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  وگرنه اگر (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  وگرنه
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ایستا ثابت StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#اگر LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#اگر __has_feature(address_sanitizer)
      "+asan",
#endif
#اگر __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#اگر __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#اگر __has_feature(memory_sanitizer)
      "+msan",
#endif
#اگر __has_feature(thread_sanitizer)
      "+tsan",
#endif
#اگر __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  بازگشت ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
تهی cl::printBuildConfig(raw_ostream &OS) {
#اگر LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
تهی cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

تهی cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

تهی cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.افزودن_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (تهی)Subs;
  أكد(Subs.contains(&Sub));
  بازگشت Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  بازگشت globalParser().getRegisteredSubcommands();
}

تهی cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  برای (auto &I : Sub.OptionsMap) {
    منطقی Unrelated = درست;
    برای (auto &Cat : I.second->Categories) {
      اگر (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = نادرست;
    }
    اگر (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

تهی cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  برای (auto &I : Sub.OptionsMap) {
    منطقی Unrelated = درست;
    برای (auto &Cat : I.second->Categories) {
      اگر (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = نادرست;
    }
    اگر (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

تهی cl::Reset_Command_سطر_Parser() { globalParser().reset(); }

تهی cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.افزودن_back(&L);
}

تهی LibraryOptionsContext::assign(ثابت LibraryOptionsContext &Other) {
  أكد(این != &Other);
  مقادیر.clear();
  SavedArgs.clear();
  ArrayRef<ثابت نویسه *> Args = Other.SavedArgs;
  برای (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<ثابت نویسه *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = مقادیر.try_emplace(L.getDefault());
  اگر (Inserted)
    It->second = L.copyDefault(Alloc);
  بازگشت parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

تهی cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

تهی LLVMParse_Command_سطر_Options(صحیح argc, ثابت نویسه *ثابت *argv,
                                 ثابت نویسه *Overview) {
  llvm::cl::Parse_Command_سطر_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
