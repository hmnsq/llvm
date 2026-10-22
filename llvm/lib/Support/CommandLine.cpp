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

#அந்தர்பாவ "llvm/Support/CommandLine.h"

#அந்தர்பாவ "DebugOptions.h"

#அந்தர்பாவ "llvm-c/Support.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/STLFunctionalExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallPtrSet.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringMap.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/LibraryOptionsContext.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/StringSaver.h"
#அந்தர்பாவ "llvm/Support/TypeSize.h"
#அந்தர்பாவ "llvm/Support/VirtualFileSystem.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cstdlib>
#அந்தர்பாவ <optional>
#அந்தர்பாவ <string>
using நாமாகாஶ llvm;
using நாமாகாஶ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
நாமாகாஶ llvm {
நாமாகாஶ cl {
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<தருக்க>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<முழு>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<long>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<இரட்டை>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<தசம>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<எழுத்து>;
template வகுப்பு LLVM_EXPORT_TEMPLATE basic_parser<Element_மொத்தம்>;

#எனில் !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template வகுப்பு LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template வகுப்பு LLVM_EXPORT_TEMPLATE opt<தருக்க>;
template வகுப்பு LLVM_EXPORT_TEMPLATE opt<எழுத்து>;
template வகுப்பு LLVM_EXPORT_TEMPLATE opt<முழு>;
template வகுப்பு LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
வெறுமை Generic_Option_மதிப்பு::anchor() {}
வெறுமை Option_மதிப்பு<boolOrDefault>::anchor() {}
வெறுமை Option_மதிப்பு<std::string>::anchor() {}
வெறுமை Option::anchor() {}
வெறுமை basic_parser_impl::anchor() {}
வெறுமை parser<தருக்க>::anchor() {}
வெறுமை parser<boolOrDefault>::anchor() {}
வெறுமை parser<முழு>::anchor() {}
வெறுமை parser<long>::anchor() {}
வெறுமை parser<long long>::anchor() {}
வெறுமை parser<unsigned>::anchor() {}
வெறுமை parser<unsigned long>::anchor() {}
வெறுமை parser<unsigned long long>::anchor() {}
வெறுமை parser<இரட்டை>::anchor() {}
வெறுமை parser<தசம>::anchor() {}
வெறுமை parser<std::string>::anchor() {}
வெறுமை parser<std::optional<std::string>>::anchor() {}
வெறுமை parser<எழுத்து>::anchor() {}
வெறுமை parser<Element_மொத்தம்>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
வெறுமை opt_bool_anchor() { opt<தருக்க> anchor{""}; }
வெறுமை opt_char_anchor() { opt<எழுத்து> anchor{""}; }
வெறுமை opt_int_anchor() { opt<முழு> anchor{""}; }
வெறுமை opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

மாறிலி நிலையான size_t DefaultPad = 2;

நிலையான StringRef ArgPrefix = "-";
நிலையான StringRef ArgPrefixLong = "--";
நிலையான StringRef ArgHelpPrefix = " - ";

நிலையான size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  எனில் (Len == 1)
    திரும்பு Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  திரும்பு Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

நிலையான SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  க்கு (size_t I = 0; I < Pad; ++I) {
    Prefix.தள்ளு_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  திரும்பு Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

நாமாகாஶ {

வகுப்பு PrintArg {
  StringRef ArgName;
  size_t Pad;
பொது:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, மாறிலி PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, மாறிலி PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  திரும்பு OS;
}

வகுப்பு Command_வரி_Parser {
பொது:
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
  size_t இலக்கம்_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_வரி_Parser() { registerSubCommand(&SubCommand::get_உச்சி_Level()); }

  வெறுமை ResetAllOptionOccurrences();

  தருக்க Parse_Command_வரி_Options(முழு argc, மாறிலி எழுத்து *மாறிலி *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  வெறுமை forEachSubCommand(Option &Opt, function_ref<வெறுமை(SubCommand &)> Action) {
    எனில் (Opt.Subs.empty()) {
      Action(SubCommand::get_உச்சி_Level());
      திரும்பு;
    }
    எனில் (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      க்கு (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      திரும்பு;
    }
    க்கு (auto *SC : Opt.Subs) {
      ப்ரதிஜ்ஞா(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  வெறுமை addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    எனில் (Opt.hasArgStr())
      திரும்பு;
    எனில் (!SC->OptionsMap.செருகு(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  வெறுமை addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  வெறுமை addOption(Option *O, SubCommand *SC) {
    தருக்க HadErrors = பொய்;
    எனில் (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      எனில் (!SC->OptionsMap.செருகு(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = உண்மை;
      }
    }

    // Remember information about positional options.
    எனில் (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.தள்ளு_back(O);
    இல்லையெனில் எனில் (O->get_இலக்கம்_Occurrences_Flag() == cl::ConsumeAfter) {
      எனில் (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = உண்மை;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    எனில் (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  வெறுமை addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  வெறுமை removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    எனில் (O->hasArgStr())
      OptionNames.தள்ளு_back(O->ArgStr);

    SubCommand &Sub = *SC;
    க்கு (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      எனில் (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    எனில் (O->getFormattingFlag() == cl::Positional)
      க்கு (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        எனில் (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          நிறுத்து;
        }
      }
    இல்லையெனில் எனில் (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  வெறுமை removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  தருக்க hasOptions(மாறிலி SubCommand &Sub) மாறிலி {
    திரும்பு (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  தருக்க hasOptions() மாறிலி {
    க்கு (மாறிலி auto *S : RegisteredSubCommands) {
      எனில் (hasOptions(*S))
        திரும்பு உண்மை;
    }
    திரும்பு !Libraries.empty();
  }

  தருக்க hasNamedSubCommands() மாறிலி {
    க்கு (மாறிலி auto *S : RegisteredSubCommands)
      எனில் (!S->getName().empty())
        திரும்பு உண்மை;
    திரும்பு பொய்;
  }

  SubCommand *getActiveSubCommand() { திரும்பு ActiveSubCommand; }

  வெறுமை updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    எனில் (!Sub.OptionsMap.செருகு(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  வெறுமை updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  வெறுமை print_Option_மதிப்புகள்();

  வெறுமை indexLibraryOptions() {
    தருக்க HadErrors = பொய்;
    க்கு (; இலக்கம்_Indexed_Libraries != Libraries.size(); ++இலக்கம்_Indexed_Libraries) {
      LibraryOptions *L = Libraries[இலக்கம்_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        எனில் (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 திரும்பு SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          திரும்பு;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = உண்மை;
      });
    }
    எனில் (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    திரும்பு LibraryIndex.lookup(Name);
  }

  வெறுமை registerCategory(OptionCategory *cat) {
    ப்ரதிஜ்ஞா(மொத்தம்_if(RegisteredOptionCategories,
                    [cat](மாறிலி OptionCategory *Category) {
             திரும்பு cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.செருகு(cat);
  }

  வெறுமை registerSubCommand(SubCommand *sub) {
    ப்ரதிஜ்ஞா(மொத்தம்_if(RegisteredSubCommands,
                    [sub](மாறிலி SubCommand *Sub) {
                      திரும்பு (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.செருகு(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ப்ரதிஜ்ஞா(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    க்கு (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      எனில் (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      இல்லையெனில்
        addLiteralOption(*O, sub, E.first);
    }
  }

  வெறுமை unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    திரும்பு make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  வெறுமை reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    இலக்கம்_Indexed_Libraries = 0;

    SubCommand::get_உச்சி_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_உச்சி_Level());
  }

தனிப்பட்ட:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &மதிப்பு);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
நிலையான Command_வரி_Parser &globalParser() {
  நிலையான ManagedStatic<Command_வரி_Parser> GlobalParser;
  திரும்பு *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
நிலையான Error
parseLibraryOption(LibraryOptions &L, வெறுமை *Opts, ArrayRef<மாறிலி எழுத்து *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<மாறிலி எழுத்து *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  மாறிலி எழுத்து *Saved[2];
  unsigned இலக்கம்_Args = std::min<size_t>(Args.size(), 2);
  க்கு (unsigned J = 0; J != இலக்கம்_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  எனில் (Error E = L.parse(Opts, ArrayRef(Saved, இலக்கம்_Args), Consumed, Alloc))
    திரும்பு E;
  எனில் (Record)
    Record->append(Saved, Saved + Consumed);
  திரும்பு Error::success();
}

template <typename T, T TrueVal, T FalseVal>
நிலையான தருக்க parseBool(Option &O, StringRef ArgName, StringRef Arg, T &மதிப்பு) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  எனில் (!Arg.data() || Arg == "true" || Arg == "1") {
    மதிப்பு = TrueVal;
    திரும்பு பொய்;
  }

  எனில் (Arg == "false" || Arg == "0") {
    மதிப்பு = FalseVal;
    திரும்பு பொய்;
  }
  திரும்பு O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

வெறுமை cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.தள்ளு_back(Help);
}

Option::Option(இலக்கம்_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : இலக்கம்_Occurrences(0), Occurrences(OccurrencesFlag), மதிப்பு(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(பொய்), Position(0) {
  Categories.தள்ளு_back(&getGeneralCategory());
}

வெறுமை Option::addArgument() {
  globalParser().addOption(இது);
  FullyInitialized = உண்மை;
}

வெறுமை Option::removeArgument() { globalParser().removeOption(இது); }

வெறுமை Option::setArgStr(StringRef S) {
  எனில் (FullyInitialized)
    globalParser().updateArgStr(இது, S);
  ப்ரதிஜ்ஞா(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

வெறுமை Option::addCategory(OptionCategory &C) {
  ப்ரதிஜ்ஞா(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  எனில் (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  இல்லையெனில் எனில் (!is_contained(Categories, &C))
    Categories.தள்ளு_back(&C);
}

வெறுமை Option::reset() {
  இலக்கம்_Occurrences = 0;
  setDefault();
}

வெறுமை OptionCategory::registerCategory() {
  globalParser().registerCategory(இது);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_உச்சி_Level() {
  நிலையான ManagedStatic<SubCommand> உச்சி_Level_Sub_Command;
  திரும்பு *உச்சி_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  நிலையான ManagedStatic<SubCommand> AllSubCommands;
  திரும்பு *AllSubCommands;
}

வெறுமை SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(இது);
}

வெறுமை SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(இது);
}

வெறுமை SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator தருக்க() மாறிலி {
  திரும்பு (globalParser().getActiveSubCommand() == இது);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_வரி_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &மதிப்பு) {
  // Reject all dashes.
  எனில் (Arg.empty())
    திரும்பு nullptr;
  ப்ரதிஜ்ஞா(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  எனில் (EqualPos == StringRef::npos) {
    // Look up the option.
    திரும்பு Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  எனில் (I == Sub.OptionsMap.end())
    திரும்பு nullptr;

  auto *O = I->second;
  எனில் (O->getFormattingFlag() == cl::AlwaysPrefix)
    திரும்பு nullptr;

  மதிப்பு = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  திரும்பு I->second;
}

SubCommand *Command_வரி_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  எனில் (Name.empty())
    திரும்பு &SubCommand::get_உச்சி_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  க்கு (auto *S : RegisteredSubCommands) {
    ப்ரதிஜ்ஞா(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    எனில் (S->getName().empty())
      தொடர்;

    எனில் (S->getName() == Name)
      திரும்பு S;

    எனில் (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  எனில் (NearestMatch)
    NearestString = NearestMatch->getName();

  திரும்பு &SubCommand::get_உச்சி_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
நிலையான Option *LookupNearestOption(StringRef Arg,
                                   மாறிலி OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  எனில் (Arg.empty())
    திரும்பு nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  க்கு (மாறிலி auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    எனில் (O->getOptionHiddenFlag() == ReallyHidden)
      தொடர்;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    எனில் (O->hasArgStr())
      OptionNames.தள்ளு_back(O->ArgStr);

    தருக்க Permit_மதிப்பு = O->get_மதிப்பு_Expected_Flag() != cl::மதிப்பு_Disallowed;
    StringRef Flag = Permit_மதிப்பு ? LHS : Arg;
    க்கு (மாறிலி auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/உண்மை, /*MaxEditDistance=*/BestDistance);
      எனில் (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        எனில் (RHS.empty() || !Permit_மதிப்பு)
          NearestString = std::string(Name);
        இல்லையெனில்
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  திரும்பு Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
நிலையான தருக்க CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef மதிப்பு) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  எனில் (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(மதிப்பு);
    StringRef::size_type Pos = Val.find(',');

    போது (Pos != StringRef::npos) {
      // Process the portion before the comma.
      எனில் (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        திரும்பு உண்மை;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    மதிப்பு = Val;
  }

  திரும்பு Handler->addOccurrence(pos, ArgName, மதிப்பு);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
நிலையான உள்வரி தருக்க ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef மதிப்பு, முழு argc,
                                 மாறிலி எழுத்து *மாறிலி *argv, முழு &i) {
  // Enforce value requirements
  தேர்வு (Handler->get_மதிப்பு_Expected_Flag()) {
  நிலை மதிப்பு_Required:
    எனில் (!மதிப்பு.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      எனில் (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        திரும்பு Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ப்ரதிஜ்ஞா(argv && "null check");
      மதிப்பு = StringRef(argv[++i]);
    }
    நிறுத்து;
  நிலை மதிப்பு_Disallowed:
    எனில் (மதிப்பு.data())
      திரும்பு Handler->error("does not allow a value! '" + Twine(மதிப்பு) +
                            "' specified.");
    நிறுத்து;
  நிலை மதிப்பு_Optional:
    நிறுத்து;
  }

  திரும்பு CommaSeparateAndAddOccurrence(Handler, i, ArgName, மதிப்பு);
}

தருக்க llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, முழு i) {
  முழு Dummy = i;
  திரும்பு ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
நிலையான Option *findPrefixOption(StringRef Name, size_t &Length,
                                மாறிலி OptionsMapTy &OptionsMap) {
  க்கு (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    எனில் (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      திரும்பு O;
    }
  }
  திரும்பு nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
நிலையான Option *HandlePrefixedOption(StringRef &Arg, StringRef &மதிப்பு,
                                    மாறிலி OptionsMapTy &OptionsMap) {
  எனில் (Arg.size() == 1)
    திரும்பு nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  எனில் (!POpt)
    திரும்பு nullptr;

  StringRef Maybe_மதிப்பு =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  எனில் (POpt->getFormattingFlag() == cl::Prefix && Maybe_மதிப்பு.starts_with("="))
    Maybe_மதிப்பு = Maybe_மதிப்பு.drop_front();
  மதிப்பு = Maybe_மதிப்பு;
  திரும்பு POpt;
}

நிலையான தருக்க Requires_மதிப்பு(மாறிலி Option *O) {
  திரும்பு O->get_இலக்கம்_Occurrences_Flag() == cl::Required ||
         O->get_இலக்கம்_Occurrences_Flag() == cl::OneOrMore;
}

நிலையான தருக்க Eats_Unbounded_எண்_Of_மதிப்புகள்(மாறிலி Option *O) {
  திரும்பு O->get_இலக்கம்_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_இலக்கம்_Occurrences_Flag() == cl::OneOrMore;
}

நிலையான தருக்க isWhitespace(எழுத்து C) {
  திரும்பு C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

நிலையான தருக்க isWhitespaceOrNull(எழுத்து C) {
  திரும்பு isWhitespace(C) || C == '\0';
}

நிலையான தருக்க isQuote(எழுத்து C) { திரும்பு C == '\"' || C == '\''; }

வெறுமை cl::Tokenize_GNUCommand_வரி(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv,
                                தருக்க MarkEOLs) {
  SmallString<128> Token;
  தருக்க InToken = பொய்;
  க்கு (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    எனில் (!InToken) {
      போது (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        எனில் (MarkEOLs && Src[I] == '\n')
          NewArgv.தள்ளு_back(nullptr);
        ++I;
      }
      எனில் (I == E)
        நிறுத்து;
      InToken = உண்மை;
    }

    எழுத்து C = Src[I];

    // Backslash escapes the next character.
    எனில் (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.தள்ளு_back(Src[I]);
      தொடர்;
    }

    // Consume a quoted string.
    எனில் (isQuote(C)) {
      ++I;
      போது (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        எனில் (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.தள்ளு_back(Src[I]);
        ++I;
      }
      எனில் (I == E)
        நிறுத்து;
      தொடர்;
    }

    // End the token if this is whitespace.
    எனில் (isWhitespace(C)) {
      NewArgv.தள்ளு_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      எனில் (MarkEOLs && C == '\n')
        NewArgv.தள்ளு_back(nullptr);
      Token.clear();
      InToken = பொய்;
      தொடர்;
    }

    // This is a normal character.  Append it.
    Token.தள்ளு_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  எனில் (InToken)
    NewArgv.தள்ளு_back(Saver.save(Token.str()).data());
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
நிலையான size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  முழு Backslash_மொத்தம் = 0;
  // Skip the backslashes.
  செய் {
    ++I;
    ++Backslash_மொத்தம்;
  } போது (I != E && Src[I] == '\\');

  தருக்க FollowedByDoubleQuote = (I != E && Src[I] == '"');
  எனில் (FollowedByDoubleQuote) {
    Token.append(Backslash_மொத்தம் / 2, '\\');
    எனில் (Backslash_மொத்தம் % 2 == 0)
      திரும்பு I - 1;
    Token.தள்ளு_back('"');
    திரும்பு I;
  }
  Token.append(Backslash_மொத்தம், '\\');
  திரும்பு I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
நிலையான தருக்க isWindowsSpecialChar(எழுத்து C) {
  திரும்பு isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
நிலையான தருக்க isWindowsSpecialCharInCommandName(எழுத்து C) {
  திரும்பு isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
நிலையான உள்வரி வெறுமை tokenize_Windows_Command_வரி_Impl(
    StringRef Src, StringSaver &Saver, function_ref<வெறுமை(StringRef)> AddToken,
    தருக்க AlwaysCopy, function_ref<வெறுமை()> MarkEOL, தருக்க InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  தருக்க CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  எண்ணிக்கை { INIT, UNQUOTED, QUOTED } State = INIT;

  க்கு (size_t I = 0, E = Src.size(); I < E; ++I) {
    தேர்வு (State) {
    நிலை INIT: {
      ப்ரதிஜ்ஞா(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      போது (I < E && isWhitespaceOrNull(Src[I])) {
        எனில் (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      எனில் (I >= E)
        நிறுத்து;
      size_t Start = I;
      எனில் (CommandName) {
        போது (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } இல்லையெனில் {
        போது (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      எனில் (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        எனில் (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } இல்லையெனில் {
          CommandName = பொய்;
        }
      } இல்லையெனில் எனில் (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } இல்லையெனில் எனில் (Src[I] == '\\') {
        ப்ரதிஜ்ஞா(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } இல்லையெனில் {
        llvm_unreachable("unexpected special character");
      }
      நிறுத்து;
    }

    நிலை UNQUOTED:
      எனில் (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        எனில் (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } இல்லையெனில் {
          CommandName = பொய்;
        }
        State = INIT;
      } இல்லையெனில் எனில் (Src[I] == '\"') {
        State = QUOTED;
      } இல்லையெனில் எனில் (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } இல்லையெனில் {
        Token.தள்ளு_back(Src[I]);
      }
      நிறுத்து;

    நிலை QUOTED:
      எனில் (Src[I] == '\"') {
        எனில் (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.தள்ளு_back('"');
          ++I;
        } இல்லையெனில் {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } இல்லையெனில் எனில் (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } இல்லையெனில் {
        Token.தள்ளு_back(Src[I]);
      }
      நிறுத்து;
    }
  }

  எனில் (State != INIT)
    AddToken(Saver.save(Token.str()));
}

வெறுமை cl::Tokenize_Windows_Command_வரி(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv,
                                    தருக்க MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.தள்ளு_back(Tok.data()); };
  auto OnEOL = [&]() {
    எனில் (MarkEOLs)
      NewArgv.தள்ளு_back(nullptr);
  };
  tokenize_Windows_Command_வரி_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/உண்மை, OnEOL, பொய்);
}

வெறுமை cl::Tokenize_Windows_Command_வரி_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.தள்ளு_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_வரி_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/பொய்,
                                 OnEOL, பொய்);
}

வெறுமை cl::Tokenize_Windows_Command_வரி_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv,
                                        தருக்க MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.தள்ளு_back(Tok.data()); };
  auto OnEOL = [&]() {
    எனில் (MarkEOLs)
      NewArgv.தள்ளு_back(nullptr);
  };
  tokenize_Windows_Command_வரி_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/உண்மை, OnEOL, உண்மை);
}

வெறுமை cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv,
                            தருக்க MarkEOLs) {
  க்கு (மாறிலி எழுத்து *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> வரி;
    // Check for comment line.
    எனில் (isWhitespace(*Cur)) {
      போது (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      தொடர்;
    }
    எனில் (*Cur == '#') {
      போது (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      தொடர்;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    மாறிலி எழுத்து *Start = Cur;
    மாறிலி எழுத்து *வரி_End = nullptr;
    எழுத்து Quote = 0;
    தருக்க AtTokenStart = உண்மை;
    க்கு (மாறிலி எழுத்து *End = Source.end(); Cur != End; ++Cur) {
      எழுத்து C = *Cur;
      எனில் (C == '\\') {
        எனில் (Cur + 1 != End) {
          ++Cur;
          எனில் (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            வரி.append(Start, Cur - 1);
            எனில் (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            தொடர்;
          }
        }
        AtTokenStart = பொய்;
        தொடர்;
      }
      எனில் (C == '\n')
        நிறுத்து;
      எனில் (Quote) {
        எனில் (C == Quote)
          Quote = 0;
      } இல்லையெனில் எனில் (isQuote(C)) {
        Quote = C;
      } இல்லையெனில் எனில் (C == '#' && AtTokenStart) {
        வரி_End = Cur;
        போது (Cur != End && *Cur != '\n')
          ++Cur;
        நிறுத்து;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    வரி.append(Start, வரி_End ? வரி_End : Cur);
    cl::Tokenize_GNUCommand_வரி(வரி, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
நிலையான தருக்க hasUTF8ByteOrderMark(ArrayRef<எழுத்து> S) {
  திரும்பு (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
நிலையான வெறுமை ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            மாறிலி எழுத்து *&Arg) {
  ப்ரதிஜ்ஞா(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  மாறிலி StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  க்கு (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    மாறிலி StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    எனில் (ResponseFile.empty())
      ResponseFile = LHS;
    இல்லையெனில்
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  எனில் (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    மாறிலி StringRef Remaining = ArgString.substr(StartPos);
    எனில் (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv) {
  ப்ரதிஜ்ஞா(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  எனில் (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    திரும்பு llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<எழுத்து> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  எனில் (hasUTF16ByteOrderMark(BufRef)) {
    எனில் (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      திரும்பு llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  இல்லையெனில் எனில் (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  எனில் (!RelativeNames && !InConfigFile)
    திரும்பு Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  க்கு (மாறிலி எழுத்து *&Arg : NewArgv) {
    எனில் (!Arg)
      தொடர்;

    // Substitute <CFGDIR> with the file's base path.
    எனில் (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    தருக்க ConfigInclusion = பொய்;
    எனில் (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      எனில் (!llvm::sys::path::is_relative(FileName))
        தொடர்;
    } இல்லையெனில் எனில் (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = உண்மை;
    } இல்லையெனில் {
      தொடர்;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.தள்ளு_back('@');
    எனில் (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      எனில் (!findConfigFile(FileName, FilePath))
        திரும்பு createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } இல்லையெனில் {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  திரும்பு Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<மாறிலி எழுத்து *> &Argv) {
  கட்டமைப்பு ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_அடுக்கு;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_அடுக்கு.தள்ளு_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  க்கு (unsigned I = 0; I != Argv.size();) {
    போது (I == File_அடுக்கு.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_அடுக்கு.எடு_back();
    }

    மாறிலி எழுத்து *Arg = Argv[I];
    // Check if it is an EOL marker
    எனில் (Arg == nullptr) {
      ++I;
      தொடர்;
    }

    எனில் (Arg[0] != '@') {
      ++I;
      தொடர்;
    }

    மாறிலி எழுத்து *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    எனில் (llvm::sys::path::is_relative(FName)) {
      எனில் (CurrentDir.empty()) {
        எனில் (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } இல்லையெனில் {
          திரும்பு createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } இல்லையெனில் {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    எனில் (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      எனில் (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        எனில் (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          தொடர்;
        }
      }
      எனில் (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      திரும்பு createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    மாறிலி llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, இது](மாறிலி ResponseFileRecord &RFile) -> ErrorOr<தருக்க> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      எனில் (!RHS)
        திரும்பு RHS.getError();
      திரும்பு FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    க்கு (மாறிலி auto &F : drop_begin(File_அடுக்கு)) {
      எனில் (ErrorOr<தருக்க> R = IsEquivalent(F)) {
        எனில் (R.get())
          திரும்பு createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } இல்லையெனில் {
        திரும்பு createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<மாறிலி எழுத்து *, 0> ExpandedArgv;
    எனில் (Error Err = expandResponseFile(FName, ExpandedArgv))
      திரும்பு Err;

    க்கு (ResponseFileRecord &Record : File_அடுக்கு) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_அடுக்கு.தள்ளு_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.செருகு(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ப்ரதிஜ்ஞா(File_அடுக்கு.size() > 0 && Argv.size() == File_அடுக்கு.back().End);
  திரும்பு Error::success();
}

தருக்க cl::expandResponseFiles(முழு Argc, மாறிலி எழுத்து *மாறிலி *Argv,
                             மாறிலி எழுத்து *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<மாறிலி எழுத்து *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_வரி;
#இல்லையெனில்
  auto Tokenize = cl::Tokenize_GNUCommand_வரி;
#endif
  // The environment variable specifies initial options.
  எனில் (EnvVar)
    எனில் (std::optional<std::string> Env_மதிப்பு = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_மதிப்பு, Saver, NewArgv, /*MarkEOLs=*/பொய்);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  எனில் (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

தருக்க cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<மாறிலி எழுத்து *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  எனில் (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

தருக்க ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<எழுத்து> &FilePath) {
  SmallString<128> CfgFilePath;
  மாறிலி auto FileExists = [இது](SmallString<128> Path) -> தருக்க {
    auto Status = FS->status(Path);
    திரும்பு Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  எனில் (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    எனில் (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      திரும்பு பொய்;
    எனில் (!FileExists(CfgFilePath))
      திரும்பு பொய்;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    திரும்பு உண்மை;
  }

  // Look for the file in search directories.
  க்கு (மாறிலி StringRef &Dir : தேடல்_Dirs) {
    எனில் (Dir.empty())
      தொடர்;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    எனில் (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      திரும்பு உண்மை;
    }
  }

  திரும்பு பொய்;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<மாறிலி எழுத்து *> &Argv) {
  SmallString<128> AbsPath;
  எனில் (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    எனில் (std::error_code EC = FS->makeAbsolute(AbsPath))
      திரும்பு make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = உண்மை;
  RelativeNames = உண்மை;
  எனில் (Error Err = expandResponseFile(CfgFile, Argv))
    திரும்பு Err;
  திரும்பு expandResponseFiles(Argv);
}

நிலையான வெறுமை initCommonOptions();
நிலையான தருக்க parse_Command_வரி_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<மாறிலி எழுத்து *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        மாறிலி எழுத்து *EnvVar) {
  initCommonOptions();
  SmallVector<மாறிலி எழுத்து *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.தள்ளு_back(Args[0]);

  // Parse options from environment variable.
  எனில் (EnvVar) {
    எனில் (std::optional<std::string> Env_மதிப்பு =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_வரி(*Env_மதிப்பு, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  முழு NewArgc = static_cast<முழு>(NewArgv.size());

  // Parse all options.
  திரும்பு globalParser().Parse_Command_வரி_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

தருக்க cl::Parse_Command_வரி_Options(முழு argc, மாறிலி எழுத்து *மாறிலி *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, மாறிலி எழுத்து *EnvVar) {
  திரும்பு parse_Command_வரி_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

தருக்க cl::parse_Command_வரி_Options(LibraryOptionsContext &Context,
                                 ArrayRef<மாறிலி எழுத்து *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, மாறிலி எழுத்து *EnvVar) {
  திரும்பு parse_Command_வரி_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
வெறுமை Command_வரி_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  க்கு (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    க்கு (auto &O : SC->OptionsMap)
      Opts.தள்ளு_back(O.second);
    க்கு (Option *O : Opts)
      O->reset();
    க்கு (Option *O : SC->PositionalOpts)
      O->reset();
    எனில் (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  க்கு (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

தருக்க Command_வரி_Parser::Parse_Command_வரி_Options(
    முழு argc, மாறிலி எழுத்து *மாறிலி *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ப்ரதிஜ்ஞா(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  தருக்க IgnoreErrors = Errs;
  எனில் (!Errs)
    Errs = &errs();
  எனில் (!VFS)
    VFS = vfs::getRealFileSystem().get();
  தருக்க ErrorParsing = பொய்;

  // Expand response files.
  SmallVector<மாறிலி எழுத்து *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_வரி;
#இல்லையெனில்
  auto Tokenize = cl::Tokenize_GNUCommand_வரி;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  எனில் (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    திரும்பு பொய்;
  }
  argv = &newArgv[0];
  argc = static_cast<முழு>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned இலக்கம்_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  தருக்க HasUnlimitedPositionals = பொய்;

  முழு FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_உச்சி_Level();
  std::string NearestSubCommandString;
  தருக்க MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  எனில் (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    எனில் (ChosenSubCommand != &SubCommand::get_உச்சி_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  ப்ரதிஜ்ஞா(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  எனில் (ConsumeAfterOpt) {
    ப்ரதிஜ்ஞா(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  எனில் (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    தருக்க UnboundedFound = பொய்;
    க்கு (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      எனில் (Requires_மதிப்பு(Opt))
        ++இலக்கம்_Positional_Required;
      இல்லையெனில் எனில் (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        எனில் (PositionalOpts.size() > 1) {
          எனில் (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = உண்மை;
        }
      } இல்லையெனில் எனில் (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        எனில் (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = உண்மை;
      }
      UnboundedFound |= Eats_Unbounded_எண்_Of_மதிப்புகள்(Opt);
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
  தருக்க DashDashFound = பொய்; // Have we read '--'?
  க்கு (முழு i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef மதிப்பு;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    எனில் (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      எனில் (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        தொடர்; // We are done!
      }

      எனில் (!PositionalOpts.empty()) {
        PositionalVals.தள்ளு_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        எனில் (PositionalVals.size() >= இலக்கம்_Positional_Required && ConsumeAfterOpt) {
          க்கு (++i; i < argc; ++i)
            PositionalVals.தள்ளு_back(std::make_pair(StringRef(argv[i]), i));
          நிறுத்து; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        தொடர்;
      }
    } இல்லையெனில் எனில் (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = உண்மை; // This is the mythical "--"?
      தொடர்;             // Don't try to process it as an argument itself.
    } இல்லையெனில் எனில் (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, மதிப்பு);
      எனில் (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        தொடர்; // We are done!
      }
    } இல்லையெனில் { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, மதிப்பு);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      எனில் (!Handler && ChosenSubCommand != &SubCommand::get_உச்சி_Level())
        Handler = LookupOption(SubCommand::get_உச்சி_Level(), ArgName, மதிப்பு);

      எனில் (!Handler) {
        எனில் (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<மாறிலி எழுத்து *> Rest(argv + i, argc - i);
          unsigned N;
          எனில் (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = உண்மை;
          }
          i += N - 1;
          தொடர்;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      எனில் (!Handler)
        Handler = HandlePrefixedOption(ArgName, மதிப்பு, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      எனில் (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    எனில் (!Handler) {
      auto ReportUnknownArgument = [&](தருக்க IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        எனில் (NearestArgumentName.empty())
          திரும்பு;

        *Errs << ProgramName << ": Did you mean '";
        எனில் (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        இல்லையெனில்
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      எனில் (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/உண்மை, NearestHandlerString);
      இல்லையெனில்
        ReportUnknownArgument(/*IsArg=*/பொய், NearestSubCommandString);

      ErrorParsing = உண்மை;
      தொடர்;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    எனில் (Handler->getFormattingFlag() == cl::Positional) {
      எனில் ((Handler->getMiscFlags() & PositionalEatsArgs) && !மதிப்பு.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = உண்மை;
      }
      ActivePositionalArg = Handler;
    }
    இல்லையெனில்
      ErrorParsing |= ProvideOption(Handler, ArgName, மதிப்பு, argc, argv, i);
  }

  // Check and handle positional arguments now...
  எனில் (இலக்கம்_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << இலக்கம்_Positional_Required
             << " positional argument" << (இலக்கம்_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = உண்மை;
  } இல்லையெனில் எனில் (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = உண்மை;

  } இல்லையெனில் எனில் (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, இலக்கம்_Vals = static_cast<unsigned>(PositionalVals.size());
    க்கு (Option *Opt : PositionalOpts) {
      எனில் (Requires_மதிப்பு(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --இலக்கம்_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      தருக்க Done = Opt->get_இலக்கம்_Occurrences_Flag() == cl::Required;
      போது (இலக்கம்_Vals - ValNo > இலக்கம்_Positional_Required && !Done) {
        தேர்வு (Opt->get_இலக்கம்_Occurrences_Flag()) {
        நிலை cl::Optional:
          Done = உண்மை; // Optional arguments want _at most_ one value
          [[fallthrough]];
        நிலை cl::ZeroOrMore: // Zero or more will take all they can get...
        நிலை cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          நிறுத்து;
        இயல்பு:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } இல்லையெனில் {
    ப்ரதிஜ்ஞா(ConsumeAfterOpt && இலக்கம்_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    க்கு (Option *Opt : PositionalOpts)
      எனில் (Requires_மதிப்பு(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    எனில் (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    க்கு (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  க்கு (மாறிலி auto &Opt : OptionsMap) {
    தேர்வு (Opt.second->get_இலக்கம்_Occurrences_Flag()) {
    நிலை Required:
    நிலை OneOrMore:
      எனில் (Opt.second->get_இலக்கம்_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = உண்மை;
      }
      [[fallthrough]];
    இயல்பு:
      நிறுத்து;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             க்கு (முழு i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  எனில் (ErrorParsing) {
    எனில் (!IgnoreErrors)
      exit(1);
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

தருக்க Option::error(மாறிலி Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  எனில் (!ArgName.data())
    ArgName = ArgStr;
  எனில் (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  இல்லையெனில்
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  திரும்பு உண்மை;
}

தருக்க Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef மதிப்பு) {
  ++இலக்கம்_Occurrences;
  திரும்பு handleOccurrence(pos, ArgName, மதிப்பு);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
நிலையான StringRef get_மதிப்பு_Str(மாறிலி Option &O, StringRef Default_செய்தி) {
  எனில் (O.மதிப்பு_Str.empty())
    திரும்பு Default_செய்தி;
  திரும்பு O.மதிப்பு_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() மாறிலி {
  திரும்பு argPlusPrefixesSize(ArgStr);
}

வெறுமை Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_வரி_Indented_By) {
  ப்ரதிஜ்ஞா(Indent >= First_வரி_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_வரி_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  போது (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

வெறுமை Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_வரி_Indented_By) {
  மாறிலி StringRef ValHelpPrefix = "  ";
  ப்ரதிஜ்ஞா(BaseIndent >= First_வரி_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_வரி_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  போது (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
வெறுமை alias::printOptionInfo(size_t GlobalWidth) மாறிலி {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(மாறிலி Option &O) மாறிலி {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_மதிப்பு_Name();
  எனில் (!ValName.empty()) {
    size_t FormattingLen = 3;
    எனில் (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_மதிப்பு_Str(O, ValName).size() + FormattingLen;
  }

  திரும்பு Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
வெறுமை basic_parser_impl::printOptionInfo(மாறிலி Option &O,
                                        size_t GlobalWidth) மாறிலி {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_மதிப்பு_Name();
  எனில் (!ValName.empty()) {
    எனில் (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_மதிப்பு_Str(O, ValName) << ">...";
    } இல்லையெனில் எனில் (O.get_மதிப்பு_Expected_Flag() == மதிப்பு_Optional)
      outs() << "[=<" << get_மதிப்பு_Str(O, ValName) << ">]";
    இல்லையெனில் {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_மதிப்பு_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

வெறுமை basic_parser_impl::printOptionName(மாறிலி Option &O,
                                        size_t GlobalWidth) மாறிலி {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
தருக்க parser<தருக்க>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         தருக்க &மதிப்பு) {
  திரும்பு parseBool<தருக்க, உண்மை, பொய்>(O, ArgName, Arg, மதிப்பு);
}

// parser<boolOrDefault> implementation
//
தருக்க parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &மதிப்பு) {
  திரும்பு parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, மதிப்பு);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
நிலையான தருக்க parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef மதிப்பு_Kind,
                                         FixedOrScalableQuantityT &மதிப்பு) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_மதிப்பு;
  எனில் (!Arg.getAsInteger(0, Min_மதிப்பு)) {
    மதிப்பு = FixedOrScalableQuantityT::getFixed(Min_மதிப்பு);
    திரும்பு பொய்;
  }

  StringRef Remainder = Arg;
  எனில் (!Remainder.consume_front("vscale"))
    திரும்பு O.error("'" + Arg + "' value invalid for " + மதிப்பு_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  எனில் (!Remainder.consume_front('x'))
    திரும்பு O.error("'" + Arg + "' value invalid for " + மதிப்பு_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  எனில் (Remainder.getAsInteger(0, Min_மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for " + மதிப்பு_Kind +
                   " argument!");

  மதிப்பு = FixedOrScalableQuantityT::getScalable(Min_மதிப்பு);
  திரும்பு பொய்;
}

// parser<int> implementation
//
தருக்க parser<முழு>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        முழு &மதிப்பு) {
  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for integer argument!");
  திரும்பு பொய்;
}

// parser<long> implementation
//
தருக்க parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &மதிப்பு) {
  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for long argument!");
  திரும்பு பொய்;
}

// parser<long long> implementation
//
தருக்க parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &மதிப்பு) {
  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for llong argument!");
  திரும்பு பொய்;
}

// parser<unsigned> implementation
//
தருக்க parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &மதிப்பு) {

  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for uint argument!");
  திரும்பு பொய்;
}

// parser<unsigned long> implementation
//
தருக்க parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &மதிப்பு) {

  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for ulong argument!");
  திரும்பு பொய்;
}

// parser<unsigned long long> implementation
//
தருக்க parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &மதிப்பு) {

  எனில் (Arg.getAsInteger(0, மதிப்பு))
    திரும்பு O.error("'" + Arg + "' value invalid for ullong argument!");
  திரும்பு பொய்;
}

// parser<ElementCount> implementation
//
தருக்க parser<Element_மொத்தம்>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_மொத்தம் &மதிப்பு) {
  திரும்பு parseFixedOrScalableQuantity(O, Arg, get_மதிப்பு_Name(), மதிப்பு);
}

// parser<double>/parser<float> implementation
//
நிலையான தருக்க parseDouble(Option &O, StringRef Arg, இரட்டை &மதிப்பு) {
  எனில் (to_float(Arg, மதிப்பு))
    திரும்பு பொய்;
  திரும்பு O.error("'" + Arg + "' value invalid for floating point argument!");
}

தருக்க parser<இரட்டை>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           இரட்டை &Val) {
  திரும்பு parseDouble(O, Arg, Val);
}

தருக்க parser<தசம>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          தசம &Val) {
  இரட்டை dVal;
  எனில் (parseDouble(O, Arg, dVal))
    திரும்பு உண்மை;
  Val = (தசம)dVal;
  திரும்பு பொய்;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_இலக்கம்_Options();

  க்கு (unsigned i = 0; i != e; ++i) {
    எனில் (getOption(i) == Name)
      திரும்பு i;
  }
  திரும்பு e;
}

நிலையான StringRef Eq_மதிப்பு = "=<value>";
நிலையான StringRef EmptyOption = "<empty>";
நிலையான StringRef OptionPrefix = "    =";
நிலையான size_t getOptionPrefixesSize() {
  திரும்பு OptionPrefix.size() + ArgHelpPrefix.size();
}

நிலையான தருக்க shouldPrintOption(StringRef Name, StringRef Description,
                              மாறிலி Option &O) {
  திரும்பு O.get_மதிப்பு_Expected_Flag() != மதிப்பு_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(மாறிலி Option &O) மாறிலி {
  எனில் (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_மதிப்பு.size();
    க்கு (unsigned i = 0, e = get_இலக்கம்_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      எனில் (!shouldPrintOption(Name, getDescription(i), O))
        தொடர்;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    திரும்பு Size;
  } இல்லையெனில் {
    size_t BaseSize = 0;
    க்கு (unsigned i = 0, e = get_இலக்கம்_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    திரும்பு BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
வெறுமை generic_parser_base::printOptionInfo(மாறிலி Option &O,
                                          size_t GlobalWidth) மாறிலி {
  எனில் (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    எனில் (O.get_மதிப்பு_Expected_Flag() == மதிப்பு_Optional) {
      க்கு (unsigned i = 0, e = get_இலக்கம்_Options(); i != e; ++i) {
        எனில் (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          நிறுத்து;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_மதிப்பு;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_மதிப்பு.size() +
                             argPlusPrefixesSize(O.ArgStr));
    க்கு (unsigned i = 0, e = get_இலக்கம்_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      எனில் (!shouldPrintOption(OptionName, Description, O))
        தொடர்;
      size_t First_வரி_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      எனில் (OptionName.empty()) {
        outs() << EmptyOption;
        ப்ரதிஜ்ஞா(First_வரி_Indent >= EmptyOption.size());
        First_வரி_Indent += EmptyOption.size();
      }
      எனில் (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_வரி_Indent);
      இல்லையெனில்
        outs() << '\n';
    }
  } இல்லையெனில் {
    எனில் (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    க்கு (unsigned i = 0, e = get_இலக்கம்_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

நிலையான மாறிலி size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
வெறுமை generic_parser_base::printGenericOptionDiff(
    மாறிலி Option &O, மாறிலி Generic_Option_மதிப்பு &மதிப்பு,
    மாறிலி Generic_Option_மதிப்பு &Default, size_t GlobalWidth) மாறிலி {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned இலக்கம்_Opts = get_இலக்கம்_Options();
  க்கு (unsigned i = 0; i != இலக்கம்_Opts; ++i) {
    எனில் (!மதிப்பு.compare(get_Option_மதிப்பு(i)))
      தொடர்;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t இலக்கம்_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(இலக்கம்_Spaces) << " (default: ";
    க்கு (unsigned j = 0; j != இலக்கம்_Opts; ++j) {
      எனில் (!Default.compare(get_Option_மதிப்பு(j)))
        தொடர்;
      outs() << getOption(j);
      நிறுத்து;
    }
    outs() << ")\n";
    திரும்பு;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
நாமாகாஶ llvm {
நாமாகாஶ cl {
நிலையான raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  திரும்பு OS << static_cast<முழு>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  வெறுமை parser<T>::printOptionDiff(மாறிலி Option &O, T V, Option_மதிப்பு<T> D,      \
                                  size_t GlobalWidth) மாறிலி {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t இலக்கம்_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(இலக்கம்_Spaces) << " (default: ";                                 \
    எனில் (D.has_மதிப்பு())                                                          \
      outs() << D.get_மதிப்பு();                                                  \
    இல்லையெனில்                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(தருக்க)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(முழு)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(இரட்டை)
PRINT_OPT_DIFF(தசம)
PRINT_OPT_DIFF(எழுத்து)
PRINT_OPT_DIFF(Element_மொத்தம்)

வெறுமை parser<std::string>::printOptionDiff(மாறிலி Option &O, StringRef V,
                                          மாறிலி Option_மதிப்பு<std::string> &D,
                                          size_t GlobalWidth) மாறிலி {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t இலக்கம்_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(இலக்கம்_Spaces) << " (default: ";
  எனில் (D.has_மதிப்பு())
    outs() << D.get_மதிப்பு();
  இல்லையெனில்
    outs() << "*no default*";
  outs() << ")\n";
}

வெறுமை parser<std::optional<std::string>>::printOptionDiff(
    மாறிலி Option &O, std::optional<StringRef> V,
    மாறிலி Option_மதிப்பு<std::optional<std::string>> &D,
    size_t GlobalWidth) மாறிலி {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_மதிப்பு_2() ? V.மதிப்பு_2().size() : 0;
  size_t இலக்கம்_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(இலக்கம்_Spaces) << " (default: ";
  எனில் (D.has_மதிப்பு() && D.get_மதிப்பு().has_மதிப்பு_2())
    outs() << D.get_மதிப்பு();
  இல்லையெனில்
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
வெறுமை basic_parser_impl::print_Option_No_மதிப்பு(மாறிலி Option &O,
                                           size_t GlobalWidth) மாறிலி {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

நிலையான முழு OptNameCompare(மாறிலி std::pair<மாறிலி எழுத்து *, Option *> *LHS,
                          மாறிலி std::pair<மாறிலி எழுத்து *, Option *> *RHS) {
  திரும்பு strcmp(LHS->first, RHS->first);
}

நிலையான முழு SubNameCompare(மாறிலி std::pair<மாறிலி எழுத்து *, SubCommand *> *LHS,
                          மாறிலி std::pair<மாறிலி எழுத்து *, SubCommand *> *RHS) {
  திரும்பு strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
நிலையான வெறுமை sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<மாறிலி எழுத்து *, Option *>> &Opts,
                     தருக்க ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  க்கு (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    எனில் (I->second->getOptionHiddenFlag() == ReallyHidden)
      தொடர்;

    // Unless showhidden is set, ignore hidden flags.
    எனில் (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      தொடர்;

    // If we've already seen this option, don't add it to the list again.
    எனில் (!OptionSet.செருகு(I->second).second)
      தொடர்;

    Opts.தள்ளு_back(
        std::pair<மாறிலி எழுத்து *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

நிலையான வெறுமை
sortSubCommands(மாறிலி SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<மாறிலி எழுத்து *, SubCommand *>> &Subs) {
  க்கு (auto *S : SubMap) {
    எனில் (S->getName().empty())
      தொடர்;
    Subs.தள்ளு_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

நாமாகாஶ {

வகுப்பு HelpPrinter {
பாதுகாக்கப்பட்ட:
  மாறிலி தருக்க ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<மாறிலி எழுத்து *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<மாறிலி எழுத்து *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  மெய்நிகர் வெறுமை printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    க்கு (மாறிலி auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  வெறுமை printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    க்கு (மாறிலி auto &S : Subs) {
      outs() << "  " << S.first;
      எனில் (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

பொது:
  explicit HelpPrinter(தருக்க showHidden) : ShowHidden(showHidden) {}
  மெய்நிகர் ~HelpPrinter() = இயல்பு;

  // Invoke the printer.
  வெறுமை operator=(தருக்க மதிப்பு) {
    எனில் (!மதிப்பு)
      திரும்பு;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  வெறுமை printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    எனில் (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    எனில் (Sub == &SubCommand::get_உச்சி_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      எனில் (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } இல்லையெனில் {
      எனில் (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    க்கு (auto *Opt : PositionalOpts) {
      எனில் (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    எனில் (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    எனில் (Sub == &SubCommand::get_உச்சி_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      க்கு (மாறிலி auto &Sub : Subs)
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
    க்கு (மாறிலி auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    எனில் (ShowHidden)
      க்கு (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              எனில் (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    க்கு (மாறிலி auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    எனில் (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    க்கு (மாறிலி auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    க்கு (மாறிலி auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

வகுப்பு CategorizedHelpPrinter : பொது HelpPrinter {
பொது:
  explicit CategorizedHelpPrinter(தருக்க showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  நிலையான முழு OptionCategoryCompare(OptionCategory *மாறிலி *A,
                                   OptionCategory *மாறிலி *B) {
    திரும்பு (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

பாதுகாக்கப்பட்ட:
  வெறுமை printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ப்ரதிஜ்ஞா(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    க்கு (மாறிலி auto &I : Opts) {
      Option *Opt = I.second;
      க்கு (OptionCategory *Cat : Opt->Categories) {
        ப்ரதிஜ்ஞா(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].தள்ளு_back(Opt);
      }
    }

    // Now do printing.
    க்கு (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      மாறிலி auto &CategoryOptions = CategorizedOptions[Category];
      எனில் (CategoryOptions.empty())
        தொடர்;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      எனில் (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      இல்லையெனில்
        outs() << "\n";

      // Loop over the options in the category and print.
      க்கு (மாறிலி Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
வகுப்பு HelpPrinterWrapper {
தனிப்பட்ட:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

பொது:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  வெறுமை operator=(தருக்க மதிப்பு);
};

} // End anonymous namespace

#எனில் defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# எனில் defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# இல்லையெனில்
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#அல்லதெனில் defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# எனில் defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# இல்லையெனில்
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#இல்லையெனில்
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

நாமாகாஶ {
வகுப்பு VersionPrinter {
பொது:
  வெறுமை print(மாறிலி std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#இல்லையெனில்
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#எனில் LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#இல்லையெனில்
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    எனில் (!ExtraPrinters.empty()) {
      க்கு (மாறிலி auto &I : ExtraPrinters)
        I(outs());
    }
  }
  வெறுமை operator=(தருக்க OptionWasSpecified);
};

கட்டமைப்பு Command_வரி_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{பொய்};
  HelpPrinter UncategorizedHiddenPrinter{உண்மை};
  CategorizedHelpPrinter CategorizedNormalPrinter{பொய்};
  CategorizedHelpPrinter CategorizedHiddenPrinter{உண்மை};
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
  cl::opt<HelpPrinter, உண்மை, parser<தருக்க>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::மதிப்பு_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, உண்மை, parser<தருக்க>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::மதிப்பு_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, உண்மை, parser<தருக்க>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::மதிப்பு_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, உண்மை, parser<தருக்க>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::மதிப்பு_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<தருக்க> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(பொய்),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<தருக்க> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(பொய்),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, உண்மை, parser<தருக்க>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::மதிப்பு_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
நிலையான ManagedStatic<Command_வரி_Common_Options> CommonOptions;

நிலையான வெறுமை initCommonOptions() {
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
  நிலையான OptionCategory GeneralCategory{"General options"};
  திரும்பு GeneralCategory;
}

வெறுமை VersionPrinter::operator=(தருக்க OptionWasSpecified) {
  எனில் (!OptionWasSpecified)
    திரும்பு;

  எனில் (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

வெறுமை HelpPrinterWrapper::operator=(தருக்க மதிப்பு) {
  எனில் (!மதிப்பு)
    திரும்பு;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  எனில் (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = உண்மை; // Invoke categorized printer
  } இல்லையெனில் {
    UncategorizedPrinter = உண்மை; // Invoke uncategorized printer
  }
}

// Print the value of each option.
வெறுமை cl::Print_Option_மதிப்புகள்() { globalParser().print_Option_மதிப்புகள்(); }

வெறுமை Command_வரி_Parser::print_Option_மதிப்புகள்() {
  எனில் (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    திரும்பு;

  SmallVector<std::pair<மாறிலி எழுத்து *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ உண்மை);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  க்கு (மாறிலி auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  க்கு (மாறிலி auto &Opt : Opts)
    Opt.second->print_Option_மதிப்பு(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
வெறுமை cl::PrintHelpMessage(தருக்க Hidden, தருக்க Categorized) {
  எனில் (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  இல்லையெனில் எனில் (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  இல்லையெனில் எனில் (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  இல்லையெனில்
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  நிலையான மாறிலி StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#எனில் LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#எனில் __has_feature(address_sanitizer)
      "+asan",
#endif
#எனில் __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#எனில் __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#எனில் __has_feature(memory_sanitizer)
      "+msan",
#endif
#எனில் __has_feature(thread_sanitizer)
      "+tsan",
#endif
#எனில் __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  திரும்பு ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
வெறுமை cl::printBuildConfig(raw_ostream &OS) {
#எனில் LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
வெறுமை cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

வெறுமை cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

வெறுமை cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.தள்ளு_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (வெறுமை)Subs;
  ப்ரதிஜ்ஞா(Subs.contains(&Sub));
  திரும்பு Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  திரும்பு globalParser().getRegisteredSubcommands();
}

வெறுமை cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  க்கு (auto &I : Sub.OptionsMap) {
    தருக்க Unrelated = உண்மை;
    க்கு (auto &Cat : I.second->Categories) {
      எனில் (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = பொய்;
    }
    எனில் (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

வெறுமை cl::HideUnrelatedOptions(ArrayRef<மாறிலி cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  க்கு (auto &I : Sub.OptionsMap) {
    தருக்க Unrelated = உண்மை;
    க்கு (auto &Cat : I.second->Categories) {
      எனில் (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = பொய்;
    }
    எனில் (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

வெறுமை cl::Reset_Command_வரி_Parser() { globalParser().reset(); }

வெறுமை cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.தள்ளு_back(&L);
}

வெறுமை LibraryOptionsContext::assign(மாறிலி LibraryOptionsContext &Other) {
  ப்ரதிஜ்ஞா(இது != &Other);
  மதிப்புகள்.clear();
  SavedArgs.clear();
  ArrayRef<மாறிலி எழுத்து *> Args = Other.SavedArgs;
  க்கு (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<மாறிலி எழுத்து *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = மதிப்புகள்.try_emplace(L.getDefault());
  எனில் (Inserted)
    It->second = L.copyDefault(Alloc);
  திரும்பு parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

வெறுமை cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

வெறுமை LLVMParse_Command_வரி_Options(முழு argc, மாறிலி எழுத்து *மாறிலி *argv,
                                 மாறிலி எழுத்து *Overview) {
  llvm::cl::Parse_Command_வரி_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
