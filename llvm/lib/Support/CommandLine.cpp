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
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<long>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
template מחלקה LLVM_EXPORT_TEMPLATE basic_parser<ElementCount>;

#אם !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template מחלקה LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template מחלקה LLVM_EXPORT_TEMPLATE opt<منطقي>;
template מחלקה LLVM_EXPORT_TEMPLATE opt<حرف>;
template מחלקה LLVM_EXPORT_TEMPLATE opt<صحيح>;
template מחלקה LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
فراغ GenericOptionValue::anchor() {}
فراغ OptionValue<boolOrDefault>::anchor() {}
فراغ OptionValue<std::string>::anchor() {}
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
فراغ parser<ElementCount>::anchor() {}

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
  אם (Len == 1)
    החזר Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  החזר Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

ساكن SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  עבור (size_t I = 0; I < Pad; ++I) {
    Prefix.push_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  החזר Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

מחלקה PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  החזר OS;
}

מחלקה CommandLineParser {
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
  size_t NumIndexedLibraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  CommandLineParser() { registerSubCommand(&SubCommand::getTopLevel()); }

  فراغ ResetAllOptionOccurrences();

  منطقي ParseCommandLineOptions(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    אם (Opt.Subs.empty()) {
      Action(SubCommand::getTopLevel());
      החזר;
    }
    אם (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      עבור (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      החזר;
    }
    עבור (auto *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  فراغ addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    אם (Opt.hasArgStr())
      החזר;
    אם (!SC->OptionsMap.insert(std::make_pair(Name, &Opt)).second) {
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
    منطقي HadErrors = שקר;
    אם (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      אם (!SC->OptionsMap.insert(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = אמת;
      }
    }

    // Remember information about positional options.
    אם (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.push_back(O);
    אחרת אם (O->getNumOccurrencesFlag() == cl::ConsumeAfter) {
      אם (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = אמת;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    אם (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  فراغ addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    אם (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    SubCommand &Sub = *SC;
    עבור (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      אם (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    אם (O->getFormattingFlag() == cl::Positional)
      עבור (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        אם (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    אחרת אם (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    החזר (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    עבור (ثابت auto *S : RegisteredSubCommands) {
      אם (hasOptions(*S))
        החזר אמת;
    }
    החזר !Libraries.empty();
  }

  منطقي hasNamedSubCommands() ثابت {
    עבור (ثابت auto *S : RegisteredSubCommands)
      אם (!S->getName().empty())
        החזר אמת;
    החזר שקר;
  }

  SubCommand *getActiveSubCommand() { החזר ActiveSubCommand; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    אם (!Sub.OptionsMap.insert(std::make_pair(NewName, O)).second) {
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

  فراغ printOptionValues();

  فراغ indexLibraryOptions() {
    منطقي HadErrors = שקר;
    עבור (; NumIndexedLibraries != Libraries.size(); ++NumIndexedLibraries) {
      LibraryOptions *L = Libraries[NumIndexedLibraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        אם (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 החזר SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          החזר;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = אמת;
      });
    }
    אם (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    החזר LibraryIndex.lookup(Name);
  }

  فراغ registerCategory(OptionCategory *cat) {
    أكد(count_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             החזר cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.insert(cat);
  }

  فراغ registerSubCommand(SubCommand *sub) {
    أكد(count_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      החזר (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.insert(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    עבור (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      אם (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      אחרת
        addLiteralOption(*O, sub, E.first);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    החזר make_range(RegisteredSubCommands.begin(),
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
    NumIndexedLibraries = 0;

    SubCommand::getTopLevel().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::getTopLevel());
  }

خاص:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &Value);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساكن CommandLineParser &globalParser() {
  ساكن ManagedStatic<CommandLineParser> GlobalParser;
  החזר *GlobalParser;
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
  unsigned NumArgs = std::min<size_t>(Args.size(), 2);
  עבור (unsigned J = 0; J != NumArgs; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  אם (Error E = L.parse(Opts, ArrayRef(Saved, NumArgs), Consumed, Alloc))
    החזר E;
  אם (Record)
    Record->append(Saved, Saved + Consumed);
  החזר Error::success();
}

template <typename T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &Value) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  אם (!Arg.data() || Arg == "true" || Arg == "1") {
    Value = TrueVal;
    החזר שקר;
  }

  אם (Arg == "false" || Arg == "0") {
    Value = FalseVal;
    החזר שקר;
  }
  החזר O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

فراغ cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.push_back(Help);
}

Option::Option(NumOccurrencesFlag OccurrencesFlag, OptionHidden Hidden)
    : NumOccurrences(0), Occurrences(OccurrencesFlag), Value(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(שקר), Position(0) {
  Categories.push_back(&getGeneralCategory());
}

فراغ Option::addArgument() {
  globalParser().addOption(هذا);
  FullyInitialized = אמת;
}

فراغ Option::removeArgument() { globalParser().removeOption(هذا); }

فراغ Option::setArgStr(StringRef S) {
  אם (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::addCategory(OptionCategory &C) {
  أكد(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  אם (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  אחרת אם (!is_contained(Categories, &C))
    Categories.push_back(&C);
}

فراغ Option::reset() {
  NumOccurrences = 0;
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
SubCommand &SubCommand::getTopLevel() {
  ساكن ManagedStatic<SubCommand> TopLevelSubCommand;
  החזר *TopLevelSubCommand;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  החזר *AllSubCommands;
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
  החזר (globalParser().getActiveSubCommand() == هذا);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *CommandLineParser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &Value) {
  // Reject all dashes.
  אם (Arg.empty())
    החזר nullptr;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  אם (EqualPos == StringRef::npos) {
    // Look up the option.
    החזר Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  אם (I == Sub.OptionsMap.end())
    החזר nullptr;

  auto *O = I->second;
  אם (O->getFormattingFlag() == cl::AlwaysPrefix)
    החזר nullptr;

  Value = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  החזר I->second;
}

SubCommand *CommandLineParser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  אם (Name.empty())
    החזר &SubCommand::getTopLevel();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  עבור (auto *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    אם (S->getName().empty())
      استمر;

    אם (S->getName() == Name)
      החזר S;

    אם (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  אם (NearestMatch)
    NearestString = NearestMatch->getName();

  החזר &SubCommand::getTopLevel();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *LookupNearestOption(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  אם (Arg.empty())
    החזר nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  עבור (ثابت auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    אם (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    אם (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    منطقي PermitValue = O->getValueExpectedFlag() != cl::ValueDisallowed;
    StringRef Flag = PermitValue ? LHS : Arg;
    עבור (ثابت auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/אמת, /*MaxEditDistance=*/BestDistance);
      אם (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        אם (RHS.empty() || !PermitValue)
          NearestString = std::string(Name);
        אחרת
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  החזר Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef Value) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  אם (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(Value);
    StringRef::size_type Pos = Val.find(',');

    כלעוד (Pos != StringRef::npos) {
      // Process the portion before the comma.
      אם (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        החזר אמת;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    Value = Val;
  }

  החזר Handler->addOccurrence(pos, ArgName, Value);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساكن مضمن منطقي ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef Value, صحيح argc,
                                 ثابت حرف *ثابت *argv, صحيح &i) {
  // Enforce value requirements
  بدل (Handler->getValueExpectedFlag()) {
  حالة ValueRequired:
    אם (!Value.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      אם (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        החזר Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      Value = StringRef(argv[++i]);
    }
    اكسر;
  حالة ValueDisallowed:
    אם (Value.data())
      החזר Handler->error("does not allow a value! '" + Twine(Value) +
                            "' specified.");
    اكسر;
  حالة ValueOptional:
    اكسر;
  }

  החזר CommaSeparateAndAddOccurrence(Handler, i, ArgName, Value);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  החזר ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *findPrefixOption(StringRef Name, size_t &Length,
                                ثابت OptionsMapTy &OptionsMap) {
  עבור (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    אם (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      החזר O;
    }
  }
  החזר nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &Value,
                                    ثابت OptionsMapTy &OptionsMap) {
  אם (Arg.size() == 1)
    החזר nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  אם (!POpt)
    החזר nullptr;

  StringRef MaybeValue =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  אם (POpt->getFormattingFlag() == cl::Prefix && MaybeValue.starts_with("="))
    MaybeValue = MaybeValue.drop_front();
  Value = MaybeValue;
  החזר POpt;
}

ساكن منطقي RequiresValue(ثابت Option *O) {
  החזר O->getNumOccurrencesFlag() == cl::Required ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

ساكن منطقي EatsUnboundedNumberOfValues(ثابت Option *O) {
  החזר O->getNumOccurrencesFlag() == cl::ZeroOrMore ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

ساكن منطقي isWhitespace(حرف C) {
  החזר C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  החזר isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { החזר C == '\"' || C == '\''; }

فراغ cl::TokenizeGNUCommandLine(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي InToken = שקר;
  עבור (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    אם (!InToken) {
      כלעוד (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        אם (MarkEOLs && Src[I] == '\n')
          NewArgv.push_back(nullptr);
        ++I;
      }
      אם (I == E)
        اكسر;
      InToken = אמת;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    אם (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.push_back(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    אם (isQuote(C)) {
      ++I;
      כלעוד (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        אם (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.push_back(Src[I]);
        ++I;
      }
      אם (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    אם (isWhitespace(C)) {
      NewArgv.push_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      אם (MarkEOLs && C == '\n')
        NewArgv.push_back(nullptr);
      Token.clear();
      InToken = שקר;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.push_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  אם (InToken)
    NewArgv.push_back(Saver.save(Token.str()).data());
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
  صحيح BackslashCount = 0;
  // Skip the backslashes.
  עשה {
    ++I;
    ++BackslashCount;
  } כלעוד (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  אם (FollowedByDoubleQuote) {
    Token.append(BackslashCount / 2, '\\');
    אם (BackslashCount % 2 == 0)
      החזר I - 1;
    Token.push_back('"');
    החזר I;
  }
  Token.append(BackslashCount, '\\');
  החזר I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  החזר isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي isWindowsSpecialCharInCommandName(حرف C) {
  החזר isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenizeWindowsCommandLineImpl(
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

  עבור (size_t I = 0, E = Src.size(); I < E; ++I) {
    بدل (State) {
    حالة INIT: {
      أكد(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      כלעוד (I < E && isWhitespaceOrNull(Src[I])) {
        אם (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      אם (I >= E)
        اكسر;
      size_t Start = I;
      אם (CommandName) {
        כלעוד (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } אחרת {
        כלעוד (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      אם (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        אם (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } אחרת {
          CommandName = שקר;
        }
      } אחרת אם (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } אחרת אם (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } אחרת {
        llvm_unreachable("unexpected special character");
      }
      اكسر;
    }

    حالة UNQUOTED:
      אם (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        אם (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } אחרת {
          CommandName = שקר;
        }
        State = INIT;
      } אחרת אם (Src[I] == '\"') {
        State = QUOTED;
      } אחרת אם (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } אחרת {
        Token.push_back(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      אם (Src[I] == '\"') {
        אם (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.push_back('"');
          ++I;
        } אחרת {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } אחרת אם (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } אחרת {
        Token.push_back(Src[I]);
      }
      اكسر;
    }
  }

  אם (State != INIT)
    AddToken(Saver.save(Token.str()));
}

فراغ cl::TokenizeWindowsCommandLine(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    אם (MarkEOLs)
      NewArgv.push_back(nullptr);
  };
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/אמת, OnEOL, שקר);
}

فراغ cl::TokenizeWindowsCommandLineNoCopy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok); };
  auto OnEOL = []() {};
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken, /*AlwaysCopy=*/שקר,
                                 OnEOL, שקר);
}

فراغ cl::TokenizeWindowsCommandLineFull(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    אם (MarkEOLs)
      NewArgv.push_back(nullptr);
  };
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/אמת, OnEOL, אמת);
}

فراغ cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  עבור (ثابت حرف *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> Line;
    // Check for comment line.
    אם (isWhitespace(*Cur)) {
      כלעוד (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    אם (*Cur == '#') {
      כלעוד (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *Start = Cur;
    ثابت حرف *LineEnd = nullptr;
    حرف Quote = 0;
    منطقي AtTokenStart = אמת;
    עבור (ثابت حرف *End = Source.end(); Cur != End; ++Cur) {
      حرف C = *Cur;
      אם (C == '\\') {
        אם (Cur + 1 != End) {
          ++Cur;
          אם (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            Line.append(Start, Cur - 1);
            אם (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            استمر;
          }
        }
        AtTokenStart = שקר;
        استمر;
      }
      אם (C == '\n')
        اكسر;
      אם (Quote) {
        אם (C == Quote)
          Quote = 0;
      } אחרת אם (isQuote(C)) {
        Quote = C;
      } אחרת אם (C == '#' && AtTokenStart) {
        LineEnd = Cur;
        כלעוד (Cur != End && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    Line.append(Start, LineEnd ? LineEnd : Cur);
    cl::TokenizeGNUCommandLine(Line, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي hasUTF8ByteOrderMark(ArrayRef<حرف> S) {
  החזר (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  עבור (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    אם (ResponseFile.empty())
      ResponseFile = LHS;
    אחרת
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  אם (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef Remaining = ArgString.substr(StartPos);
    אם (!Remaining.empty())
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
  אם (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    החזר llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  אם (hasUTF16ByteOrderMark(BufRef)) {
    אם (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      החזר llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  אחרת אם (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  אם (!RelativeNames && !InConfigFile)
    החזר Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  עבור (ثابت حرف *&Arg : NewArgv) {
    אם (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    אם (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = שקר;
    אם (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      אם (!llvm::sys::path::is_relative(FileName))
        استمر;
    } אחרת אם (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = אמת;
    } אחרת {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.push_back('@');
    אם (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      אם (!findConfigFile(FileName, FilePath))
        החזר createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } אחרת {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  החזר Error::success();
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
  SmallVector<ResponseFileRecord, 3> FileStack;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  FileStack.push_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  עבור (unsigned I = 0; I != Argv.size();) {
    כלעוד (I == FileStack.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      FileStack.pop_back();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    אם (Arg == nullptr) {
      ++I;
      استمر;
    }

    אם (Arg[0] != '@') {
      ++I;
      استمر;
    }

    ثابت حرف *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    אם (llvm::sys::path::is_relative(FName)) {
      אם (CurrentDir.empty()) {
        אם (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } אחרת {
          החזר createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } אחרת {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    אם (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      אם (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        אם (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          استمر;
        }
      }
      אם (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      החזר createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ثابت llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, هذا](ثابت ResponseFileRecord &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      אם (!RHS)
        החזר RHS.getError();
      החזר FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    עבור (ثابت auto &F : drop_begin(FileStack)) {
      אם (ErrorOr<منطقي> R = IsEquivalent(F)) {
        אם (R.get())
          החזר createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } אחרת {
        החזר createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ثابت حرف *, 0> ExpandedArgv;
    אם (Error Err = expandResponseFile(FName, ExpandedArgv))
      החזר Err;

    עבור (ResponseFileRecord &Record : FileStack) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    FileStack.push_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.insert(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(FileStack.size() > 0 && Argv.size() == FileStack.back().End);
  החזר Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ثابت حرف *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#אחרת
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  // The environment variable specifies initial options.
  אם (EnvVar)
    אם (std::optional<std::string> EnvValue = sys::Process::GetEnv(EnvVar))
      Tokenize(*EnvValue, Saver, NewArgv, /*MarkEOLs=*/שקר);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  אם (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    החזר שקר;
  }
  החזר אמת;
}

منطقي cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  אם (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    החזר שקר;
  }
  החזר אמת;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقي ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<حرف> &FilePath) {
  SmallString<128> CfgFilePath;
  ثابت auto FileExists = [هذا](SmallString<128> Path) -> منطقي {
    auto Status = FS->status(Path);
    החזר Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  אם (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    אם (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      החזר שקר;
    אם (!FileExists(CfgFilePath))
      החזר שקר;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    החזר אמת;
  }

  // Look for the file in search directories.
  עבור (ثابت StringRef &Dir : SearchDirs) {
    אם (Dir.empty())
      استمر;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    אם (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      החזר אמת;
    }
  }

  החזר שקר;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ثابت حرف *> &Argv) {
  SmallString<128> AbsPath;
  אם (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    אם (std::error_code EC = FS->makeAbsolute(AbsPath))
      החזר make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = אמת;
  RelativeNames = אמת;
  אם (Error Err = expandResponseFile(CfgFile, Argv))
    החזר Err;
  החזר expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parseCommandLineOptionsImpl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  SmallVector<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.push_back(Args[0]);

  // Parse options from environment variable.
  אם (EnvVar) {
    אם (std::optional<std::string> EnvValue =
            sys::Process::GetEnv(StringRef(EnvVar)))
      TokenizeGNUCommandLine(*EnvValue, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.size());

  // Parse all options.
  החזר globalParser().ParseCommandLineOptions(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::ParseCommandLineOptions(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  החזר parseCommandLineOptionsImpl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parseCommandLineOptions(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  החזר parseCommandLineOptionsImpl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
فراغ CommandLineParser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  עבור (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    עבור (auto &O : SC->OptionsMap)
      Opts.push_back(O.second);
    עבור (Option *O : Opts)
      O->reset();
    עבור (Option *O : SC->PositionalOpts)
      O->reset();
    אם (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  עבור (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

منطقي CommandLineParser::ParseCommandLineOptions(
    صحيح argc, ثابت حرف *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  منطقي IgnoreErrors = Errs;
  אם (!Errs)
    Errs = &errs();
  אם (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقي ErrorParsing = שקר;

  // Expand response files.
  SmallVector<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#אחרת
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  אם (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    החזר שקר;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned NumPositionalRequired = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = שקר;

  صحيح FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::getTopLevel();
  std::string NearestSubCommandString;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  אם (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    אם (ChosenSubCommand != &SubCommand::getTopLevel())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  أكد(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  אם (ConsumeAfterOpt) {
    أكد(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  אם (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    منطقي UnboundedFound = שקר;
    עבור (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      אם (RequiresValue(Opt))
        ++NumPositionalRequired;
      אחרת אם (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        אם (PositionalOpts.size() > 1) {
          אם (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = אמת;
        }
      } אחרת אם (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        אם (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = אמת;
      }
      UnboundedFound |= EatsUnboundedNumberOfValues(Opt);
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
  منطقي DashDashFound = שקר; // Have we read '--'?
  עבור (صحيح i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef Value;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    אם (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      אם (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      אם (!PositionalOpts.empty()) {
        PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        אם (PositionalVals.size() >= NumPositionalRequired && ConsumeAfterOpt) {
          עבור (++i; i < argc; ++i)
            PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } אחרת אם (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = אמת; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } אחרת אם (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);
      אם (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        استمر; // We are done!
      }
    } אחרת { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      אם (!Handler && ChosenSubCommand != &SubCommand::getTopLevel())
        Handler = LookupOption(SubCommand::getTopLevel(), ArgName, Value);

      אם (!Handler) {
        אם (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          unsigned N;
          אם (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = אמת;
          }
          i += N - 1;
          استمر;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      אם (!Handler)
        Handler = HandlePrefixedOption(ArgName, Value, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      אם (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    אם (!Handler) {
      auto ReportUnknownArgument = [&](منطقي IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        אם (NearestArgumentName.empty())
          החזר;

        *Errs << ProgramName << ": Did you mean '";
        אם (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        אחרת
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      אם (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/אמת, NearestHandlerString);
      אחרת
        ReportUnknownArgument(/*IsArg=*/שקר, NearestSubCommandString);

      ErrorParsing = אמת;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    אם (Handler->getFormattingFlag() == cl::Positional) {
      אם ((Handler->getMiscFlags() & PositionalEatsArgs) && !Value.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = אמת;
      }
      ActivePositionalArg = Handler;
    }
    אחרת
      ErrorParsing |= ProvideOption(Handler, ArgName, Value, argc, argv, i);
  }

  // Check and handle positional arguments now...
  אם (NumPositionalRequired > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << NumPositionalRequired
             << " positional argument" << (NumPositionalRequired > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = אמת;
  } אחרת אם (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = אמת;

  } אחרת אם (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, NumVals = static_cast<unsigned>(PositionalVals.size());
    עבור (Option *Opt : PositionalOpts) {
      אם (RequiresValue(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --NumPositionalRequired; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي Done = Opt->getNumOccurrencesFlag() == cl::Required;
      כלעוד (NumVals - ValNo > NumPositionalRequired && !Done) {
        بدل (Opt->getNumOccurrencesFlag()) {
        حالة cl::Optional:
          Done = אמת; // Optional arguments want _at most_ one value
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
  } אחרת {
    أكد(ConsumeAfterOpt && NumPositionalRequired <= PositionalVals.size());
    unsigned ValNo = 0;
    עבור (Option *Opt : PositionalOpts)
      אם (RequiresValue(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    אם (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    עבור (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  עבור (ثابت auto &Opt : OptionsMap) {
    بدل (Opt.second->getNumOccurrencesFlag()) {
    حالة Required:
    حالة OneOrMore:
      אם (Opt.second->getNumOccurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = אמת;
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
             עבור (صحيح i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  אם (ErrorParsing) {
    אם (!IgnoreErrors)
      exit(1);
    החזר שקר;
  }
  החזר אמת;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  אם (!ArgName.data())
    ArgName = ArgStr;
  אם (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  אחרת
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  החזר אמת;
}

منطقي Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef Value) {
  ++NumOccurrences;
  החזר handleOccurrence(pos, ArgName, Value);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef getValueStr(ثابت Option &O, StringRef DefaultMsg) {
  אם (O.ValueStr.empty())
    החזר DefaultMsg;
  החזר O.ValueStr;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ثابت {
  החזר argPlusPrefixesSize(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t FirstLineIndentedBy) {
  أكد(Indent >= FirstLineIndentedBy);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - FirstLineIndentedBy)
      << ArgHelpPrefix << Split.first << "\n";
  כלעוד (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t FirstLineIndentedBy) {
  ثابت StringRef ValHelpPrefix = "  ";
  أكد(BaseIndent >= FirstLineIndentedBy);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - FirstLineIndentedBy)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  כלעוד (!Split.second.empty()) {
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
  auto ValName = getValueName();
  אם (!ValName.empty()) {
    size_t FormattingLen = 3;
    אם (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += getValueStr(O, ValName).size() + FormattingLen;
  }

  החזר Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t GlobalWidth) ثابت {
  outs() << PrintArg(O.ArgStr);

  auto ValName = getValueName();
  אם (!ValName.empty()) {
    אם (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << getValueStr(O, ValName) << ">...";
    } אחרת אם (O.getValueExpectedFlag() == ValueOptional)
      outs() << "[=<" << getValueStr(O, ValName) << ">]";
    אחרת {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << getValueStr(O, ValName)
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
                         منطقي &Value) {
  החזר parseBool<منطقي, אמת, שקר>(O, ArgName, Arg, Value);
}

// parser<boolOrDefault> implementation
//
منطقي parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &Value) {
  החזר parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, Value);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ValueKind,
                                         FixedOrScalableQuantityT &Value) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy MinValue;
  אם (!Arg.getAsInteger(0, MinValue)) {
    Value = FixedOrScalableQuantityT::getFixed(MinValue);
    החזר שקר;
  }

  StringRef Remainder = Arg;
  אם (!Remainder.consume_front("vscale"))
    החזר O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  אם (!Remainder.consume_front('x'))
    החזר O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  אם (Remainder.getAsInteger(0, MinValue))
    החזר O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Value = FixedOrScalableQuantityT::getScalable(MinValue);
  החזר שקר;
}

// parser<int> implementation
//
منطقي parser<صحيح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحيح &Value) {
  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for integer argument!");
  החזר שקר;
}

// parser<long> implementation
//
منطقي parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &Value) {
  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for long argument!");
  החזר שקר;
}

// parser<long long> implementation
//
منطقي parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &Value) {
  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for llong argument!");
  החזר שקר;
}

// parser<unsigned> implementation
//
منطقي parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &Value) {

  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for uint argument!");
  החזר שקר;
}

// parser<unsigned long> implementation
//
منطقي parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &Value) {

  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for ulong argument!");
  החזר שקר;
}

// parser<unsigned long long> implementation
//
منطقي parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &Value) {

  אם (Arg.getAsInteger(0, Value))
    החזר O.error("'" + Arg + "' value invalid for ullong argument!");
  החזר שקר;
}

// parser<ElementCount> implementation
//
منطقي parser<ElementCount>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 ElementCount &Value) {
  החזר parseFixedOrScalableQuantity(O, Arg, getValueName(), Value);
}

// parser<double>/parser<float> implementation
//
ساكن منطقي parseDouble(Option &O, StringRef Arg, مزدوج &Value) {
  אם (to_float(Arg, Value))
    החזר שקר;
  החזר O.error("'" + Arg + "' value invalid for floating point argument!");
}

منطقي parser<مزدوج>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           مزدوج &Val) {
  החזר parseDouble(O, Arg, Val);
}

منطقي parser<عائم>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          عائم &Val) {
  مزدوج dVal;
  אם (parseDouble(O, Arg, dVal))
    החזר אמת;
  Val = (عائم)dVal;
  החזר שקר;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = getNumOptions();

  עבור (unsigned i = 0; i != e; ++i) {
    אם (getOption(i) == Name)
      החזר i;
  }
  החזר e;
}

ساكن StringRef EqValue = "=<value>";
ساكن StringRef EmptyOption = "<empty>";
ساكن StringRef OptionPrefix = "    =";
ساكن size_t getOptionPrefixesSize() {
  החזר OptionPrefix.size() + ArgHelpPrefix.size();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  החזר O.getValueExpectedFlag() != ValueOptional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ثابت Option &O) ثابت {
  אם (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + EqValue.size();
    עבור (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef Name = getOption(i);
      אם (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    החזר Size;
  } אחרת {
    size_t BaseSize = 0;
    עבור (unsigned i = 0, e = getNumOptions(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    החזר BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_base::printOptionInfo(ثابت Option &O,
                                          size_t GlobalWidth) ثابت {
  אם (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    אם (O.getValueExpectedFlag() == ValueOptional) {
      עבור (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
        אם (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << EqValue;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         EqValue.size() +
                             argPlusPrefixesSize(O.ArgStr));
    עבור (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      אם (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t FirstLineIndent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      אם (OptionName.empty()) {
        outs() << EmptyOption;
        أكد(FirstLineIndent >= EmptyOption.size());
        FirstLineIndent += EmptyOption.size();
      }
      אם (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, FirstLineIndent);
      אחרת
        outs() << '\n';
    }
  } אחרת {
    אם (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    עבור (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
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
    ثابت Option &O, ثابت GenericOptionValue &Value,
    ثابت GenericOptionValue &Default, size_t GlobalWidth) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned NumOpts = getNumOptions();
  עבור (unsigned i = 0; i != NumOpts; ++i) {
    אם (!Value.compare(getOptionValue(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t NumSpaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(NumSpaces) << " (default: ";
    עבור (unsigned j = 0; j != NumOpts; ++j) {
      אם (!Default.compare(getOptionValue(j)))
        استمر;
      outs() << getOption(j);
      اكسر;
    }
    outs() << ")\n";
    החזר;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
نطاق llvm {
نطاق cl {
ساكن raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  החזר OS << static_cast<صحيح>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, OptionValue<T> D,      \
                                  size_t GlobalWidth) ثابت {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t NumSpaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(NumSpaces) << " (default: ";                                 \
    אם (D.hasValue())                                                          \
      outs() << D.getValue();                                                  \
    אחרת                                                                       \
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
PRINT_OPT_DIFF(ElementCount)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت OptionValue<std::string> &D,
                                          size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t NumSpaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(NumSpaces) << " (default: ";
  אם (D.hasValue())
    outs() << D.getValue();
  אחרת
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت OptionValue<std::optional<std::string>> &D,
    size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_value() ? V.value().size() : 0;
  size_t NumSpaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(NumSpaces) << " (default: ";
  אם (D.hasValue() && D.getValue().has_value())
    outs() << D.getValue();
  אחרת
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::printOptionNoValue(ثابت Option &O,
                                           size_t GlobalWidth) ثابت {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  החזר strcmp(LHS->first, RHS->first);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  החזר strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  עבור (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    אם (I->second->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    אם (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    אם (!OptionSet.insert(I->second).second)
      استمر;

    Opts.push_back(
        std::pair<ثابت حرف *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

ساكن فراغ
sortSubCommands(ثابت SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  עבור (auto *S : SubMap) {
    אם (S->getName().empty())
      استمر;
    Subs.push_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

نطاق {

מחלקה HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ثابت حرف *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  virtual فراغ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    עבור (ثابت auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    עבור (ثابت auto &S : Subs) {
      outs() << "  " << S.first;
      אם (!S.second->getDescription().empty()) {
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
  فراغ operator=(منطقي Value) {
    אם (!Value)
      החזר;
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

    אם (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    אם (Sub == &SubCommand::getTopLevel()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      אם (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } אחרת {
      אם (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    עבור (auto *Opt : PositionalOpts) {
      אם (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    אם (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    אם (Sub == &SubCommand::getTopLevel() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      עבור (ثابت auto &Sub : Subs)
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
    עבור (ثابت auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    אם (ShowHidden)
      עבור (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              אם (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    עבור (ثابت auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    אם (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    עבור (ثابت auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    עבור (ثابت auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

מחלקה CategorizedHelpPrinter : عام HelpPrinter {
عام:
  explicit CategorizedHelpPrinter(منطقي showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساكن صحيح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    החזר (*A)->getName().compare((*B)->getName());
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
    עבור (ثابت auto &I : Opts) {
      Option *Opt = I.second;
      עבור (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].push_back(Opt);
      }
    }

    // Now do printing.
    עבור (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت auto &CategoryOptions = CategorizedOptions[Category];
      אם (CategoryOptions.empty())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      אם (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      אחרת
        outs() << "\n";

      // Loop over the options in the category and print.
      עבור (ثابت Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
מחלקה HelpPrinterWrapper {
خاص:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

عام:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  فراغ operator=(منطقي Value);
};

} // End anonymous namespace

#אם defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# אם defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# אחרת
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#وإلاإذا defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# אם defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# אחרת
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#אחרת
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

نطاق {
מחלקה VersionPrinter {
عام:
  فراغ print(ثابت std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#אחרת
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#אם LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#אחרת
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    אם (!ExtraPrinters.empty()) {
      עבור (ثابت auto &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ operator=(منطقي OptionWasSpecified);
};

بنية CommandLineCommonOptions {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{שקר};
  HelpPrinter UncategorizedHiddenPrinter{אמת};
  CategorizedHelpPrinter CategorizedNormalPrinter{שקר};
  CategorizedHelpPrinter CategorizedHiddenPrinter{אמת};
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
  cl::opt<HelpPrinter, אמת, parser<منطقي>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, אמת, parser<منطقي>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, אמת, parser<منطقي>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, אמת, parser<منطقي>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقي> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(שקר),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقي> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(שקר),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, אמת, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::ValueDisallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساكن ManagedStatic<CommandLineCommonOptions> CommonOptions;

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
  החזר GeneralCategory;
}

فراغ VersionPrinter::operator=(منطقي OptionWasSpecified) {
  אם (!OptionWasSpecified)
    החזר;

  אם (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

فراغ HelpPrinterWrapper::operator=(منطقي Value) {
  אם (!Value)
    החזר;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  אם (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = אמת; // Invoke categorized printer
  } אחרת {
    UncategorizedPrinter = אמת; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::PrintOptionValues() { globalParser().printOptionValues(); }

فراغ CommandLineParser::printOptionValues() {
  אם (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    החזר;

  SmallVector<std::pair<ثابت حرف *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ אמת);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  עבור (ثابت auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  עבור (ثابت auto &Opt : Opts)
    Opt.second->printOptionValue(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::PrintHelpMessage(منطقي Hidden, منطقي Categorized) {
  אם (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  אחרת אם (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  אחרת אם (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  אחרת
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ساكن ثابت StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#אם LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#אם __has_feature(address_sanitizer)
      "+asan",
#endif
#אם __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#אם __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#אם __has_feature(memory_sanitizer)
      "+msan",
#endif
#אם __has_feature(thread_sanitizer)
      "+tsan",
#endif
#אם __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  החזר ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#אם LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
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
  CommonOptions->ExtraVersionPrinters.push_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.contains(&Sub));
  החזר Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  החזר globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  עבור (auto &I : Sub.OptionsMap) {
    منطقي Unrelated = אמת;
    עבור (auto &Cat : I.second->Categories) {
      אם (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = שקר;
    }
    אם (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  עבור (auto &I : Sub.OptionsMap) {
    منطقي Unrelated = אמת;
    עבור (auto &Cat : I.second->Categories) {
      אם (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = שקר;
    }
    אם (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

فراغ cl::ResetCommandLineParser() { globalParser().reset(); }

فراغ cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.push_back(&L);
}

فراغ LibraryOptionsContext::assign(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  Values.clear();
  SavedArgs.clear();
  ArrayRef<ثابت حرف *> Args = Other.SavedArgs;
  עבור (size_t I = 0, E = Args.size(); I != E;) {
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
  auto [It, Inserted] = Values.try_emplace(L.getDefault());
  אם (Inserted)
    It->second = L.copyDefault(Alloc);
  החזר parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParseCommandLineOptions(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::ParseCommandLineOptions(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
