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

#অন্তর্ভাৱ "llvm/Support/CommandLine.h"

#অন্তর্ভাৱ "DebugOptions.h"

#অন্তর্ভাৱ "llvm-c/Support.h"
#অন্তর্ভাৱ "llvm/ADT/ArrayRef.h"
#অন্তর্ভাৱ "llvm/ADT/STLFunctionalExtras.h"
#অন্তর্ভাৱ "llvm/ADT/SmallPtrSet.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringMap.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/ConvertUTF.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/Error.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/LibraryOptionsContext.h"
#অন্তর্ভাৱ "llvm/Support/ManagedStatic.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/Process.h"
#অন্তর্ভাৱ "llvm/Support/StringSaver.h"
#অন্তর্ভাৱ "llvm/Support/TypeSize.h"
#অন্তর্ভাৱ "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <cstdlib>
#অন্তর্ভাৱ <optional>
#অন্তর্ভাৱ <string>
using নামাকাশ llvm;
using নামাকাশ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
নামাকাশ llvm {
নামাকাশ cl {
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<তার্কিক>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<পূর্ণাঙ্ক>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<long>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<দ্ৱিগুণ>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<ভিন্ন>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<ৱর্ণ>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE basic_parser<Element_গুনতি>;

#যদি !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template শ্ৰেণী LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template শ্ৰেণী LLVM_EXPORT_TEMPLATE opt<তার্কিক>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE opt<ৱর্ণ>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE opt<পূর্ণাঙ্ক>;
template শ্ৰেণী LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
শূন্য Generic_Option_মান::anchor() {}
শূন্য Option_মান<boolOrDefault>::anchor() {}
শূন্য Option_মান<std::string>::anchor() {}
শূন্য Option::anchor() {}
শূন্য basic_parser_impl::anchor() {}
শূন্য parser<তার্কিক>::anchor() {}
শূন্য parser<boolOrDefault>::anchor() {}
শূন্য parser<পূর্ণাঙ্ক>::anchor() {}
শূন্য parser<long>::anchor() {}
শূন্য parser<long long>::anchor() {}
শূন্য parser<unsigned>::anchor() {}
শূন্য parser<unsigned long>::anchor() {}
শূন্য parser<unsigned long long>::anchor() {}
শূন্য parser<দ্ৱিগুণ>::anchor() {}
শূন্য parser<ভিন্ন>::anchor() {}
শূন্য parser<std::string>::anchor() {}
শূন্য parser<std::optional<std::string>>::anchor() {}
শূন্য parser<ৱর্ণ>::anchor() {}
শূন্য parser<Element_গুনতি>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
শূন্য opt_bool_anchor() { opt<তার্কিক> anchor{""}; }
শূন্য opt_char_anchor() { opt<ৱর্ণ> anchor{""}; }
শূন্য opt_int_anchor() { opt<পূর্ণাঙ্ক> anchor{""}; }
শূন্য opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ধ্রুৱ স্থির size_t DefaultPad = 2;

স্থির StringRef ArgPrefix = "-";
স্থির StringRef ArgPrefixLong = "--";
স্থির StringRef ArgHelpPrefix = " - ";

স্থির size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  যদি (Len == 1)
    ঘূৰাই Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  ঘূৰাই Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

স্থির SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  বাবে (size_t I = 0; I < Pad; ++I) {
    Prefix.ঠেলো_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  ঘূৰাই Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

নামাকাশ {

শ্ৰেণী PrintArg {
  StringRef ArgName;
  size_t Pad;
সার্ৱজনিক:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ধ্রুৱ PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ধ্রুৱ PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ঘূৰাই OS;
}

শ্ৰেণী Command_সাৰি_Parser {
সার্ৱজনিক:
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
  size_t অঙ্ক_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_সাৰি_Parser() { registerSubCommand(&SubCommand::get_শীৰ্ষ_Level()); }

  শূন্য ResetAllOptionOccurrences();

  তার্কিক Parse_Command_সাৰি_Options(পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  শূন্য forEachSubCommand(Option &Opt, function_ref<শূন্য(SubCommand &)> Action) {
    যদি (Opt.Subs.empty()) {
      Action(SubCommand::get_শীৰ্ষ_Level());
      ঘূৰাই;
    }
    যদি (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      বাবে (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ঘূৰাই;
    }
    বাবে (auto *SC : Opt.Subs) {
      প্রতিজ্ঞা(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  শূন্য addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    যদি (Opt.hasArgStr())
      ঘূৰাই;
    যদি (!SC->OptionsMap.ঢোকাও(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  শূন্য addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  শূন্য addOption(Option *O, SubCommand *SC) {
    তার্কিক HadErrors = মিছা;
    যদি (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      যদি (!SC->OptionsMap.ঢোকাও(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = সঁচা;
      }
    }

    // Remember information about positional options.
    যদি (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ঠেলো_back(O);
    অন্যথা যদি (O->get_অঙ্ক_Occurrences_Flag() == cl::ConsumeAfter) {
      যদি (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = সঁচা;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    যদি (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  শূন্য addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  শূন্য removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    যদি (O->hasArgStr())
      OptionNames.ঠেলো_back(O->ArgStr);

    SubCommand &Sub = *SC;
    বাবে (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      যদি (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    যদি (O->getFormattingFlag() == cl::Positional)
      বাবে (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        যদি (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ভঙ্গ;
        }
      }
    অন্যথা যদি (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  শূন্য removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  তার্কিক hasOptions(ধ্রুৱ SubCommand &Sub) ধ্রুৱ {
    ঘূৰাই (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  তার্কিক hasOptions() ধ্রুৱ {
    বাবে (ধ্রুৱ auto *S : RegisteredSubCommands) {
      যদি (hasOptions(*S))
        ঘূৰাই সঁচা;
    }
    ঘূৰাই !Libraries.empty();
  }

  তার্কিক hasNamedSubCommands() ধ্রুৱ {
    বাবে (ধ্রুৱ auto *S : RegisteredSubCommands)
      যদি (!S->getName().empty())
        ঘূৰাই সঁচা;
    ঘূৰাই মিছা;
  }

  SubCommand *getActiveSubCommand() { ঘূৰাই ActiveSubCommand; }

  শূন্য updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    যদি (!Sub.OptionsMap.ঢোকাও(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  শূন্য updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  শূন্য print_Option_মানসমূহ();

  শূন্য indexLibraryOptions() {
    তার্কিক HadErrors = মিছা;
    বাবে (; অঙ্ক_Indexed_Libraries != Libraries.size(); ++অঙ্ক_Indexed_Libraries) {
      LibraryOptions *L = Libraries[অঙ্ক_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        যদি (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ঘূৰাই SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          ঘূৰাই;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = সঁচা;
      });
    }
    যদি (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    ঘূৰাই LibraryIndex.lookup(Name);
  }

  শূন্য registerCategory(OptionCategory *cat) {
    প্রতিজ্ঞা(গুনতি_if(RegisteredOptionCategories,
                    [cat](ধ্রুৱ OptionCategory *Category) {
             ঘূৰাই cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ঢোকাও(cat);
  }

  শূন্য registerSubCommand(SubCommand *sub) {
    প্রতিজ্ঞা(গুনতি_if(RegisteredSubCommands,
                    [sub](ধ্রুৱ SubCommand *Sub) {
                      ঘূৰাই (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ঢোকাও(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    প্রতিজ্ঞা(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    বাবে (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      যদি (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      অন্যথা
        addLiteralOption(*O, sub, E.first);
    }
  }

  শূন্য unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ঘূৰাই make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  শূন্য reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    অঙ্ক_Indexed_Libraries = 0;

    SubCommand::get_শীৰ্ষ_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_শীৰ্ষ_Level());
  }

গুপ্ত:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &মান);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
স্থির Command_সাৰি_Parser &globalParser() {
  স্থির ManagedStatic<Command_সাৰি_Parser> GlobalParser;
  ঘূৰাই *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
স্থির Error
parseLibraryOption(LibraryOptions &L, শূন্য *Opts, ArrayRef<ধ্রুৱ ৱর্ণ *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ধ্রুৱ ৱর্ণ *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ধ্রুৱ ৱর্ণ *Saved[2];
  unsigned অঙ্ক_Args = std::min<size_t>(Args.size(), 2);
  বাবে (unsigned J = 0; J != অঙ্ক_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  যদি (Error E = L.parse(Opts, ArrayRef(Saved, অঙ্ক_Args), Consumed, Alloc))
    ঘূৰাই E;
  যদি (Record)
    Record->append(Saved, Saved + Consumed);
  ঘূৰাই Error::success();
}

template <typename T, T TrueVal, T FalseVal>
স্থির তার্কিক parseBool(Option &O, StringRef ArgName, StringRef Arg, T &মান) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  যদি (!Arg.data() || Arg == "true" || Arg == "1") {
    মান = TrueVal;
    ঘূৰাই মিছা;
  }

  যদি (Arg == "false" || Arg == "0") {
    মান = FalseVal;
    ঘূৰাই মিছা;
  }
  ঘূৰাই O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

শূন্য cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ঠেলো_back(Help);
}

Option::Option(অঙ্ক_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : অঙ্ক_Occurrences(0), Occurrences(OccurrencesFlag), মান(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(মিছা), Position(0) {
  Categories.ঠেলো_back(&getGeneralCategory());
}

শূন্য Option::addArgument() {
  globalParser().addOption(এতত্);
  FullyInitialized = সঁচা;
}

শূন্য Option::removeArgument() { globalParser().removeOption(এতত্); }

শূন্য Option::setArgStr(StringRef S) {
  যদি (FullyInitialized)
    globalParser().updateArgStr(এতত্, S);
  প্রতিজ্ঞা(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

শূন্য Option::addCategory(OptionCategory &C) {
  প্রতিজ্ঞা(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  যদি (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  অন্যথা যদি (!is_contained(Categories, &C))
    Categories.ঠেলো_back(&C);
}

শূন্য Option::reset() {
  অঙ্ক_Occurrences = 0;
  setDefault();
}

শূন্য OptionCategory::registerCategory() {
  globalParser().registerCategory(এতত্);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_শীৰ্ষ_Level() {
  স্থির ManagedStatic<SubCommand> শীৰ্ষ_Level_Sub_Command;
  ঘূৰাই *শীৰ্ষ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  স্থির ManagedStatic<SubCommand> AllSubCommands;
  ঘূৰাই *AllSubCommands;
}

শূন্য SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(এতত্);
}

শূন্য SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(এতত্);
}

শূন্য SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator তার্কিক() ধ্রুৱ {
  ঘূৰাই (globalParser().getActiveSubCommand() == এতত্);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_সাৰি_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &মান) {
  // Reject all dashes.
  যদি (Arg.empty())
    ঘূৰাই nullptr;
  প্রতিজ্ঞা(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  যদি (EqualPos == StringRef::npos) {
    // Look up the option.
    ঘূৰাই Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  যদি (I == Sub.OptionsMap.end())
    ঘূৰাই nullptr;

  auto *O = I->second;
  যদি (O->getFormattingFlag() == cl::AlwaysPrefix)
    ঘূৰাই nullptr;

  মান = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ঘূৰাই I->second;
}

SubCommand *Command_সাৰি_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  যদি (Name.empty())
    ঘূৰাই &SubCommand::get_শীৰ্ষ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  বাবে (auto *S : RegisteredSubCommands) {
    প্রতিজ্ঞা(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    যদি (S->getName().empty())
      অনুৱর্ত;

    যদি (S->getName() == Name)
      ঘূৰাই S;

    যদি (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  যদি (NearestMatch)
    NearestString = NearestMatch->getName();

  ঘূৰাই &SubCommand::get_শীৰ্ষ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
স্থির Option *LookupNearestOption(StringRef Arg,
                                   ধ্রুৱ OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  যদি (Arg.empty())
    ঘূৰাই nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  বাবে (ধ্রুৱ auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    যদি (O->getOptionHiddenFlag() == ReallyHidden)
      অনুৱর্ত;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    যদি (O->hasArgStr())
      OptionNames.ঠেলো_back(O->ArgStr);

    তার্কিক Permit_মান = O->get_মান_Expected_Flag() != cl::মান_Disallowed;
    StringRef Flag = Permit_মান ? LHS : Arg;
    বাবে (ধ্রুৱ auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/সঁচা, /*MaxEditDistance=*/BestDistance);
      যদি (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        যদি (RHS.empty() || !Permit_মান)
          NearestString = std::string(Name);
        অন্যথা
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ঘূৰাই Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
স্থির তার্কিক CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef মান) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  যদি (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(মান);
    StringRef::size_type Pos = Val.find(',');

    যেতিয়ালৈ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      যদি (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        ঘূৰাই সঁচা;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    মান = Val;
  }

  ঘূৰাই Handler->addOccurrence(pos, ArgName, মান);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
স্থির অন্তঃস্থ তার্কিক ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef মান, পূর্ণাঙ্ক argc,
                                 ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, পূর্ণাঙ্ক &i) {
  // Enforce value requirements
  ৱিকল্প (Handler->get_মান_Expected_Flag()) {
  পক্ষ মান_Required:
    যদি (!মান.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      যদি (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        ঘূৰাই Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      প্রতিজ্ঞা(argv && "null check");
      মান = StringRef(argv[++i]);
    }
    ভঙ্গ;
  পক্ষ মান_Disallowed:
    যদি (মান.data())
      ঘূৰাই Handler->error("does not allow a value! '" + Twine(মান) +
                            "' specified.");
    ভঙ্গ;
  পক্ষ মান_Optional:
    ভঙ্গ;
  }

  ঘূৰাই CommaSeparateAndAddOccurrence(Handler, i, ArgName, মান);
}

তার্কিক llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, পূর্ণাঙ্ক i) {
  পূর্ণাঙ্ক Dummy = i;
  ঘূৰাই ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
স্থির Option *findPrefixOption(StringRef Name, size_t &Length,
                                ধ্রুৱ OptionsMapTy &OptionsMap) {
  বাবে (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    যদি (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      ঘূৰাই O;
    }
  }
  ঘূৰাই nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
স্থির Option *HandlePrefixedOption(StringRef &Arg, StringRef &মান,
                                    ধ্রুৱ OptionsMapTy &OptionsMap) {
  যদি (Arg.size() == 1)
    ঘূৰাই nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  যদি (!POpt)
    ঘূৰাই nullptr;

  StringRef Maybe_মান =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  যদি (POpt->getFormattingFlag() == cl::Prefix && Maybe_মান.starts_with("="))
    Maybe_মান = Maybe_মান.drop_front();
  মান = Maybe_মান;
  ঘূৰাই POpt;
}

স্থির তার্কিক Requires_মান(ধ্রুৱ Option *O) {
  ঘূৰাই O->get_অঙ্ক_Occurrences_Flag() == cl::Required ||
         O->get_অঙ্ক_Occurrences_Flag() == cl::OneOrMore;
}

স্থির তার্কিক Eats_Unbounded_সংখ্যা_Of_মানসমূহ(ধ্রুৱ Option *O) {
  ঘূৰাই O->get_অঙ্ক_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_অঙ্ক_Occurrences_Flag() == cl::OneOrMore;
}

স্থির তার্কিক isWhitespace(ৱর্ণ C) {
  ঘূৰাই C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

স্থির তার্কিক isWhitespaceOrNull(ৱর্ণ C) {
  ঘূৰাই isWhitespace(C) || C == '\0';
}

স্থির তার্কিক isQuote(ৱর্ণ C) { ঘূৰাই C == '\"' || C == '\''; }

শূন্য cl::Tokenize_GNUCommand_সাৰি(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv,
                                তার্কিক MarkEOLs) {
  SmallString<128> Token;
  তার্কিক InToken = মিছা;
  বাবে (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    যদি (!InToken) {
      যেতিয়ালৈ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        যদি (MarkEOLs && Src[I] == '\n')
          NewArgv.ঠেলো_back(nullptr);
        ++I;
      }
      যদি (I == E)
        ভঙ্গ;
      InToken = সঁচা;
    }

    ৱর্ণ C = Src[I];

    // Backslash escapes the next character.
    যদি (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ঠেলো_back(Src[I]);
      অনুৱর্ত;
    }

    // Consume a quoted string.
    যদি (isQuote(C)) {
      ++I;
      যেতিয়ালৈ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        যদি (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ঠেলো_back(Src[I]);
        ++I;
      }
      যদি (I == E)
        ভঙ্গ;
      অনুৱর্ত;
    }

    // End the token if this is whitespace.
    যদি (isWhitespace(C)) {
      NewArgv.ঠেলো_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      যদি (MarkEOLs && C == '\n')
        NewArgv.ঠেলো_back(nullptr);
      Token.clear();
      InToken = মিছা;
      অনুৱর্ত;
    }

    // This is a normal character.  Append it.
    Token.ঠেলো_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  যদি (InToken)
    NewArgv.ঠেলো_back(Saver.save(Token.str()).data());
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
স্থির size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  পূর্ণাঙ্ক Backslash_গুনতি = 0;
  // Skip the backslashes.
  কৰা {
    ++I;
    ++Backslash_গুনতি;
  } যেতিয়ালৈ (I != E && Src[I] == '\\');

  তার্কিক FollowedByDoubleQuote = (I != E && Src[I] == '"');
  যদি (FollowedByDoubleQuote) {
    Token.append(Backslash_গুনতি / 2, '\\');
    যদি (Backslash_গুনতি % 2 == 0)
      ঘূৰাই I - 1;
    Token.ঠেলো_back('"');
    ঘূৰাই I;
  }
  Token.append(Backslash_গুনতি, '\\');
  ঘূৰাই I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
স্থির তার্কিক isWindowsSpecialChar(ৱর্ণ C) {
  ঘূৰাই isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
স্থির তার্কিক isWindowsSpecialCharInCommandName(ৱর্ণ C) {
  ঘূৰাই isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
স্থির অন্তঃস্থ শূন্য tokenize_Windows_Command_সাৰি_Impl(
    StringRef Src, StringSaver &Saver, function_ref<শূন্য(StringRef)> AddToken,
    তার্কিক AlwaysCopy, function_ref<শূন্য()> MarkEOL, তার্কিক InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  তার্কিক CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  গণনা { INIT, UNQUOTED, QUOTED } State = INIT;

  বাবে (size_t I = 0, E = Src.size(); I < E; ++I) {
    ৱিকল্প (State) {
    পক্ষ INIT: {
      প্রতিজ্ঞা(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      যেতিয়ালৈ (I < E && isWhitespaceOrNull(Src[I])) {
        যদি (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      যদি (I >= E)
        ভঙ্গ;
      size_t Start = I;
      যদি (CommandName) {
        যেতিয়ালৈ (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } অন্যথা {
        যেতিয়ালৈ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      যদি (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        যদি (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } অন্যথা {
          CommandName = মিছা;
        }
      } অন্যথা যদি (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } অন্যথা যদি (Src[I] == '\\') {
        প্রতিজ্ঞা(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } অন্যথা {
        llvm_unreachable("unexpected special character");
      }
      ভঙ্গ;
    }

    পক্ষ UNQUOTED:
      যদি (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        যদি (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } অন্যথা {
          CommandName = মিছা;
        }
        State = INIT;
      } অন্যথা যদি (Src[I] == '\"') {
        State = QUOTED;
      } অন্যথা যদি (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } অন্যথা {
        Token.ঠেলো_back(Src[I]);
      }
      ভঙ্গ;

    পক্ষ QUOTED:
      যদি (Src[I] == '\"') {
        যদি (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ঠেলো_back('"');
          ++I;
        } অন্যথা {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } অন্যথা যদি (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } অন্যথা {
        Token.ঠেলো_back(Src[I]);
      }
      ভঙ্গ;
    }
  }

  যদি (State != INIT)
    AddToken(Saver.save(Token.str()));
}

শূন্য cl::Tokenize_Windows_Command_সাৰি(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv,
                                    তার্কিক MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ঠেলো_back(Tok.data()); };
  auto OnEOL = [&]() {
    যদি (MarkEOLs)
      NewArgv.ঠেলো_back(nullptr);
  };
  tokenize_Windows_Command_সাৰি_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/সঁচা, OnEOL, মিছা);
}

শূন্য cl::Tokenize_Windows_Command_সাৰি_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ঠেলো_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_সাৰি_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/মিছা,
                                 OnEOL, মিছা);
}

শূন্য cl::Tokenize_Windows_Command_সাৰি_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv,
                                        তার্কিক MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ঠেলো_back(Tok.data()); };
  auto OnEOL = [&]() {
    যদি (MarkEOLs)
      NewArgv.ঠেলো_back(nullptr);
  };
  tokenize_Windows_Command_সাৰি_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/সঁচা, OnEOL, সঁচা);
}

শূন্য cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv,
                            তার্কিক MarkEOLs) {
  বাবে (ধ্রুৱ ৱর্ণ *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> সাৰি;
    // Check for comment line.
    যদি (isWhitespace(*Cur)) {
      যেতিয়ালৈ (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      অনুৱর্ত;
    }
    যদি (*Cur == '#') {
      যেতিয়ালৈ (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      অনুৱর্ত;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ধ্রুৱ ৱর্ণ *Start = Cur;
    ধ্রুৱ ৱর্ণ *সাৰি_End = nullptr;
    ৱর্ণ Quote = 0;
    তার্কিক AtTokenStart = সঁচা;
    বাবে (ধ্রুৱ ৱর্ণ *End = Source.end(); Cur != End; ++Cur) {
      ৱর্ণ C = *Cur;
      যদি (C == '\\') {
        যদি (Cur + 1 != End) {
          ++Cur;
          যদি (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            সাৰি.append(Start, Cur - 1);
            যদি (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            অনুৱর্ত;
          }
        }
        AtTokenStart = মিছা;
        অনুৱর্ত;
      }
      যদি (C == '\n')
        ভঙ্গ;
      যদি (Quote) {
        যদি (C == Quote)
          Quote = 0;
      } অন্যথা যদি (isQuote(C)) {
        Quote = C;
      } অন্যথা যদি (C == '#' && AtTokenStart) {
        সাৰি_End = Cur;
        যেতিয়ালৈ (Cur != End && *Cur != '\n')
          ++Cur;
        ভঙ্গ;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    সাৰি.append(Start, সাৰি_End ? সাৰি_End : Cur);
    cl::Tokenize_GNUCommand_সাৰি(সাৰি, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
স্থির তার্কিক hasUTF8ByteOrderMark(ArrayRef<ৱর্ণ> S) {
  ঘূৰাই (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
স্থির শূন্য ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ধ্রুৱ ৱর্ণ *&Arg) {
  প্রতিজ্ঞা(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ধ্রুৱ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  বাবে (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ধ্রুৱ StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    যদি (ResponseFile.empty())
      ResponseFile = LHS;
    অন্যথা
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  যদি (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ধ্রুৱ StringRef Remaining = ArgString.substr(StartPos);
    যদি (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv) {
  প্রতিজ্ঞা(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  যদি (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    ঘূৰাই llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<ৱর্ণ> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  যদি (hasUTF16ByteOrderMark(BufRef)) {
    যদি (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      ঘূৰাই llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  অন্যথা যদি (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  যদি (!RelativeNames && !InConfigFile)
    ঘূৰাই Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  বাবে (ধ্রুৱ ৱর্ণ *&Arg : NewArgv) {
    যদি (!Arg)
      অনুৱর্ত;

    // Substitute <CFGDIR> with the file's base path.
    যদি (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    তার্কিক ConfigInclusion = মিছা;
    যদি (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      যদি (!llvm::sys::path::is_relative(FileName))
        অনুৱর্ত;
    } অন্যথা যদি (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = সঁচা;
    } অন্যথা {
      অনুৱর্ত;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ঠেলো_back('@');
    যদি (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      যদি (!findConfigFile(FileName, FilePath))
        ঘূৰাই createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } অন্যথা {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  ঘূৰাই Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &Argv) {
  সংরচনা ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_স্তূপ;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_স্তূপ.ঠেলো_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  বাবে (unsigned I = 0; I != Argv.size();) {
    যেতিয়ালৈ (I == File_স্তূপ.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_স্তূপ.বেৰ_কৰো_back();
    }

    ধ্রুৱ ৱর্ণ *Arg = Argv[I];
    // Check if it is an EOL marker
    যদি (Arg == nullptr) {
      ++I;
      অনুৱর্ত;
    }

    যদি (Arg[0] != '@') {
      ++I;
      অনুৱর্ত;
    }

    ধ্রুৱ ৱর্ণ *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    যদি (llvm::sys::path::is_relative(FName)) {
      যদি (CurrentDir.empty()) {
        যদি (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } অন্যথা {
          ঘূৰাই createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } অন্যথা {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    যদি (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      যদি (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        যদি (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          অনুৱর্ত;
        }
      }
      যদি (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ঘূৰাই createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ধ্রুৱ llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, এতত্](ধ্রুৱ ResponseFileRecord &RFile) -> ErrorOr<তার্কিক> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      যদি (!RHS)
        ঘূৰাই RHS.getError();
      ঘূৰাই FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    বাবে (ধ্রুৱ auto &F : drop_begin(File_স্তূপ)) {
      যদি (ErrorOr<তার্কিক> R = IsEquivalent(F)) {
        যদি (R.get())
          ঘূৰাই createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } অন্যথা {
        ঘূৰাই createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ধ্রুৱ ৱর্ণ *, 0> ExpandedArgv;
    যদি (Error Err = expandResponseFile(FName, ExpandedArgv))
      ঘূৰাই Err;

    বাবে (ResponseFileRecord &Record : File_স্তূপ) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_স্তূপ.ঠেলো_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.ঢোকাও(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  প্রতিজ্ঞা(File_স্তূপ.size() > 0 && Argv.size() == File_স্তূপ.back().End);
  ঘূৰাই Error::success();
}

তার্কিক cl::expandResponseFiles(পূর্ণাঙ্ক Argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ *Argv,
                             ধ্রুৱ ৱর্ণ *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_সাৰি;
#অন্যথা
  auto Tokenize = cl::Tokenize_GNUCommand_সাৰি;
#endif
  // The environment variable specifies initial options.
  যদি (EnvVar)
    যদি (std::optional<std::string> Env_মান = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_মান, Saver, NewArgv, /*MarkEOLs=*/মিছা);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  যদি (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

তার্কিক cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  যদি (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

তার্কিক ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<ৱর্ণ> &FilePath) {
  SmallString<128> CfgFilePath;
  ধ্রুৱ auto FileExists = [এতত্](SmallString<128> Path) -> তার্কিক {
    auto Status = FS->status(Path);
    ঘূৰাই Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  যদি (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    যদি (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      ঘূৰাই মিছা;
    যদি (!FileExists(CfgFilePath))
      ঘূৰাই মিছা;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    ঘূৰাই সঁচা;
  }

  // Look for the file in search directories.
  বাবে (ধ্রুৱ StringRef &Dir : অনুসন্ধান_Dirs) {
    যদি (Dir.empty())
      অনুৱর্ত;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    যদি (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      ঘূৰাই সঁচা;
    }
  }

  ঘূৰাই মিছা;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ধ্রুৱ ৱর্ণ *> &Argv) {
  SmallString<128> AbsPath;
  যদি (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    যদি (std::error_code EC = FS->makeAbsolute(AbsPath))
      ঘূৰাই make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = সঁচা;
  RelativeNames = সঁচা;
  যদি (Error Err = expandResponseFile(CfgFile, Argv))
    ঘূৰাই Err;
  ঘূৰাই expandResponseFiles(Argv);
}

স্থির শূন্য initCommonOptions();
স্থির তার্কিক parse_Command_সাৰি_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ধ্রুৱ ৱর্ণ *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ধ্রুৱ ৱর্ণ *EnvVar) {
  initCommonOptions();
  SmallVector<ধ্রুৱ ৱর্ণ *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ঠেলো_back(Args[0]);

  // Parse options from environment variable.
  যদি (EnvVar) {
    যদি (std::optional<std::string> Env_মান =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_সাৰি(*Env_মান, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  পূর্ণাঙ্ক NewArgc = static_cast<পূর্ণাঙ্ক>(NewArgv.size());

  // Parse all options.
  ঘূৰাই globalParser().Parse_Command_সাৰি_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

তার্কিক cl::Parse_Command_সাৰি_Options(পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ধ্রুৱ ৱর্ণ *EnvVar) {
  ঘূৰাই parse_Command_সাৰি_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

তার্কিক cl::parse_Command_সাৰি_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ধ্রুৱ ৱর্ণ *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ধ্রুৱ ৱর্ণ *EnvVar) {
  ঘূৰাই parse_Command_সাৰি_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
শূন্য Command_সাৰি_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  বাবে (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    বাবে (auto &O : SC->OptionsMap)
      Opts.ঠেলো_back(O.second);
    বাবে (Option *O : Opts)
      O->reset();
    বাবে (Option *O : SC->PositionalOpts)
      O->reset();
    যদি (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  বাবে (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

তার্কিক Command_সাৰি_Parser::Parse_Command_সাৰি_Options(
    পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  প্রতিজ্ঞা(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  তার্কিক IgnoreErrors = Errs;
  যদি (!Errs)
    Errs = &errs();
  যদি (!VFS)
    VFS = vfs::getRealFileSystem().get();
  তার্কিক ErrorParsing = মিছা;

  // Expand response files.
  SmallVector<ধ্রুৱ ৱর্ণ *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_সাৰি;
#অন্যথা
  auto Tokenize = cl::Tokenize_GNUCommand_সাৰি;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  যদি (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    ঘূৰাই মিছা;
  }
  argv = &newArgv[0];
  argc = static_cast<পূর্ণাঙ্ক>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned অঙ্ক_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  তার্কিক HasUnlimitedPositionals = মিছা;

  পূর্ণাঙ্ক FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_শীৰ্ষ_Level();
  std::string NearestSubCommandString;
  তার্কিক MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  যদি (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    যদি (ChosenSubCommand != &SubCommand::get_শীৰ্ষ_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  প্রতিজ্ঞা(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  যদি (ConsumeAfterOpt) {
    প্রতিজ্ঞা(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  যদি (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    তার্কিক UnboundedFound = মিছা;
    বাবে (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      যদি (Requires_মান(Opt))
        ++অঙ্ক_Positional_Required;
      অন্যথা যদি (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        যদি (PositionalOpts.size() > 1) {
          যদি (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = সঁচা;
        }
      } অন্যথা যদি (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        যদি (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = সঁচা;
      }
      UnboundedFound |= Eats_Unbounded_সংখ্যা_Of_মানসমূহ(Opt);
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
  তার্কিক DashDashFound = মিছা; // Have we read '--'?
  বাবে (পূর্ণাঙ্ক i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef মান;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    যদি (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      যদি (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        অনুৱর্ত; // We are done!
      }

      যদি (!PositionalOpts.empty()) {
        PositionalVals.ঠেলো_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        যদি (PositionalVals.size() >= অঙ্ক_Positional_Required && ConsumeAfterOpt) {
          বাবে (++i; i < argc; ++i)
            PositionalVals.ঠেলো_back(std::make_pair(StringRef(argv[i]), i));
          ভঙ্গ; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        অনুৱর্ত;
      }
    } অন্যথা যদি (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = সঁচা; // This is the mythical "--"?
      অনুৱর্ত;             // Don't try to process it as an argument itself.
    } অন্যথা যদি (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, মান);
      যদি (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        অনুৱর্ত; // We are done!
      }
    } অন্যথা { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, মান);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      যদি (!Handler && ChosenSubCommand != &SubCommand::get_শীৰ্ষ_Level())
        Handler = LookupOption(SubCommand::get_শীৰ্ষ_Level(), ArgName, মান);

      যদি (!Handler) {
        যদি (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ধ্রুৱ ৱর্ণ *> Rest(argv + i, argc - i);
          unsigned N;
          যদি (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = সঁচা;
          }
          i += N - 1;
          অনুৱর্ত;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      যদি (!Handler)
        Handler = HandlePrefixedOption(ArgName, মান, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      যদি (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    যদি (!Handler) {
      auto ReportUnknownArgument = [&](তার্কিক IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        যদি (NearestArgumentName.empty())
          ঘূৰাই;

        *Errs << ProgramName << ": Did you mean '";
        যদি (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        অন্যথা
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      যদি (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/সঁচা, NearestHandlerString);
      অন্যথা
        ReportUnknownArgument(/*IsArg=*/মিছা, NearestSubCommandString);

      ErrorParsing = সঁচা;
      অনুৱর্ত;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    যদি (Handler->getFormattingFlag() == cl::Positional) {
      যদি ((Handler->getMiscFlags() & PositionalEatsArgs) && !মান.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = সঁচা;
      }
      ActivePositionalArg = Handler;
    }
    অন্যথা
      ErrorParsing |= ProvideOption(Handler, ArgName, মান, argc, argv, i);
  }

  // Check and handle positional arguments now...
  যদি (অঙ্ক_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << অঙ্ক_Positional_Required
             << " positional argument" << (অঙ্ক_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = সঁচা;
  } অন্যথা যদি (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = সঁচা;

  } অন্যথা যদি (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, অঙ্ক_Vals = static_cast<unsigned>(PositionalVals.size());
    বাবে (Option *Opt : PositionalOpts) {
      যদি (Requires_মান(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --অঙ্ক_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      তার্কিক Done = Opt->get_অঙ্ক_Occurrences_Flag() == cl::Required;
      যেতিয়ালৈ (অঙ্ক_Vals - ValNo > অঙ্ক_Positional_Required && !Done) {
        ৱিকল্প (Opt->get_অঙ্ক_Occurrences_Flag()) {
        পক্ষ cl::Optional:
          Done = সঁচা; // Optional arguments want _at most_ one value
          [[fallthrough]];
        পক্ষ cl::ZeroOrMore: // Zero or more will take all they can get...
        পক্ষ cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          ভঙ্গ;
        স্ৱভাৱ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } অন্যথা {
    প্রতিজ্ঞা(ConsumeAfterOpt && অঙ্ক_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    বাবে (Option *Opt : PositionalOpts)
      যদি (Requires_মান(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    যদি (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    বাবে (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  বাবে (ধ্রুৱ auto &Opt : OptionsMap) {
    ৱিকল্প (Opt.second->get_অঙ্ক_Occurrences_Flag()) {
    পক্ষ Required:
    পক্ষ OneOrMore:
      যদি (Opt.second->get_অঙ্ক_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = সঁচা;
      }
      [[fallthrough]];
    স্ৱভাৱ:
      ভঙ্গ;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             বাবে (পূর্ণাঙ্ক i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  যদি (ErrorParsing) {
    যদি (!IgnoreErrors)
      exit(1);
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

তার্কিক Option::error(ধ্রুৱ Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  যদি (!ArgName.data())
    ArgName = ArgStr;
  যদি (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  অন্যথা
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  ঘূৰাই সঁচা;
}

তার্কিক Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef মান) {
  ++অঙ্ক_Occurrences;
  ঘূৰাই handleOccurrence(pos, ArgName, মান);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
স্থির StringRef get_মান_Str(ধ্রুৱ Option &O, StringRef Default_বাৰ্তা) {
  যদি (O.মান_Str.empty())
    ঘূৰাই Default_বাৰ্তা;
  ঘূৰাই O.মান_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ধ্রুৱ {
  ঘূৰাই argPlusPrefixesSize(ArgStr);
}

শূন্য Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_সাৰি_Indented_By) {
  প্রতিজ্ঞা(Indent >= First_সাৰি_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_সাৰি_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  যেতিয়ালৈ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

শূন্য Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_সাৰি_Indented_By) {
  ধ্রুৱ StringRef ValHelpPrefix = "  ";
  প্রতিজ্ঞা(BaseIndent >= First_সাৰি_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_সাৰি_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  যেতিয়ালৈ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
শূন্য alias::printOptionInfo(size_t GlobalWidth) ধ্রুৱ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(ধ্রুৱ Option &O) ধ্রুৱ {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_মান_Name();
  যদি (!ValName.empty()) {
    size_t FormattingLen = 3;
    যদি (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_মান_Str(O, ValName).size() + FormattingLen;
  }

  ঘূৰাই Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
শূন্য basic_parser_impl::printOptionInfo(ধ্রুৱ Option &O,
                                        size_t GlobalWidth) ধ্রুৱ {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_মান_Name();
  যদি (!ValName.empty()) {
    যদি (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_মান_Str(O, ValName) << ">...";
    } অন্যথা যদি (O.get_মান_Expected_Flag() == মান_Optional)
      outs() << "[=<" << get_মান_Str(O, ValName) << ">]";
    অন্যথা {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_মান_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

শূন্য basic_parser_impl::printOptionName(ধ্রুৱ Option &O,
                                        size_t GlobalWidth) ধ্রুৱ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
তার্কিক parser<তার্কিক>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         তার্কিক &মান) {
  ঘূৰাই parseBool<তার্কিক, সঁচা, মিছা>(O, ArgName, Arg, মান);
}

// parser<boolOrDefault> implementation
//
তার্কিক parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &মান) {
  ঘূৰাই parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, মান);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
স্থির তার্কিক parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef মান_Kind,
                                         FixedOrScalableQuantityT &মান) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_মান;
  যদি (!Arg.getAsInteger(0, Min_মান)) {
    মান = FixedOrScalableQuantityT::getFixed(Min_মান);
    ঘূৰাই মিছা;
  }

  StringRef Remainder = Arg;
  যদি (!Remainder.consume_front("vscale"))
    ঘূৰাই O.error("'" + Arg + "' value invalid for " + মান_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  যদি (!Remainder.consume_front('x'))
    ঘূৰাই O.error("'" + Arg + "' value invalid for " + মান_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  যদি (Remainder.getAsInteger(0, Min_মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for " + মান_Kind +
                   " argument!");

  মান = FixedOrScalableQuantityT::getScalable(Min_মান);
  ঘূৰাই মিছা;
}

// parser<int> implementation
//
তার্কিক parser<পূর্ণাঙ্ক>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        পূর্ণাঙ্ক &মান) {
  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for integer argument!");
  ঘূৰাই মিছা;
}

// parser<long> implementation
//
তার্কিক parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &মান) {
  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for long argument!");
  ঘূৰাই মিছা;
}

// parser<long long> implementation
//
তার্কিক parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &মান) {
  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for llong argument!");
  ঘূৰাই মিছা;
}

// parser<unsigned> implementation
//
তার্কিক parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &মান) {

  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for uint argument!");
  ঘূৰাই মিছা;
}

// parser<unsigned long> implementation
//
তার্কিক parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &মান) {

  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for ulong argument!");
  ঘূৰাই মিছা;
}

// parser<unsigned long long> implementation
//
তার্কিক parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &মান) {

  যদি (Arg.getAsInteger(0, মান))
    ঘূৰাই O.error("'" + Arg + "' value invalid for ullong argument!");
  ঘূৰাই মিছা;
}

// parser<ElementCount> implementation
//
তার্কিক parser<Element_গুনতি>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_গুনতি &মান) {
  ঘূৰাই parseFixedOrScalableQuantity(O, Arg, get_মান_Name(), মান);
}

// parser<double>/parser<float> implementation
//
স্থির তার্কিক parseDouble(Option &O, StringRef Arg, দ্ৱিগুণ &মান) {
  যদি (to_float(Arg, মান))
    ঘূৰাই মিছা;
  ঘূৰাই O.error("'" + Arg + "' value invalid for floating point argument!");
}

তার্কিক parser<দ্ৱিগুণ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           দ্ৱিগুণ &Val) {
  ঘূৰাই parseDouble(O, Arg, Val);
}

তার্কিক parser<ভিন্ন>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ভিন্ন &Val) {
  দ্ৱিগুণ dVal;
  যদি (parseDouble(O, Arg, dVal))
    ঘূৰাই সঁচা;
  Val = (ভিন্ন)dVal;
  ঘূৰাই মিছা;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_অঙ্ক_Options();

  বাবে (unsigned i = 0; i != e; ++i) {
    যদি (getOption(i) == Name)
      ঘূৰাই i;
  }
  ঘূৰাই e;
}

স্থির StringRef Eq_মান = "=<value>";
স্থির StringRef EmptyOption = "<empty>";
স্থির StringRef OptionPrefix = "    =";
স্থির size_t getOptionPrefixesSize() {
  ঘূৰাই OptionPrefix.size() + ArgHelpPrefix.size();
}

স্থির তার্কিক shouldPrintOption(StringRef Name, StringRef Description,
                              ধ্রুৱ Option &O) {
  ঘূৰাই O.get_মান_Expected_Flag() != মান_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ধ্রুৱ Option &O) ধ্রুৱ {
  যদি (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_মান.size();
    বাবে (unsigned i = 0, e = get_অঙ্ক_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      যদি (!shouldPrintOption(Name, getDescription(i), O))
        অনুৱর্ত;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    ঘূৰাই Size;
  } অন্যথা {
    size_t BaseSize = 0;
    বাবে (unsigned i = 0, e = get_অঙ্ক_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    ঘূৰাই BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
শূন্য generic_parser_base::printOptionInfo(ধ্রুৱ Option &O,
                                          size_t GlobalWidth) ধ্রুৱ {
  যদি (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    যদি (O.get_মান_Expected_Flag() == মান_Optional) {
      বাবে (unsigned i = 0, e = get_অঙ্ক_Options(); i != e; ++i) {
        যদি (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          ভঙ্গ;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_মান;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_মান.size() +
                             argPlusPrefixesSize(O.ArgStr));
    বাবে (unsigned i = 0, e = get_অঙ্ক_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      যদি (!shouldPrintOption(OptionName, Description, O))
        অনুৱর্ত;
      size_t First_সাৰি_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      যদি (OptionName.empty()) {
        outs() << EmptyOption;
        প্রতিজ্ঞা(First_সাৰি_Indent >= EmptyOption.size());
        First_সাৰি_Indent += EmptyOption.size();
      }
      যদি (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_সাৰি_Indent);
      অন্যথা
        outs() << '\n';
    }
  } অন্যথা {
    যদি (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    বাবে (unsigned i = 0, e = get_অঙ্ক_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

স্থির ধ্রুৱ size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
শূন্য generic_parser_base::printGenericOptionDiff(
    ধ্রুৱ Option &O, ধ্রুৱ Generic_Option_মান &মান,
    ধ্রুৱ Generic_Option_মান &Default, size_t GlobalWidth) ধ্রুৱ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned অঙ্ক_Opts = get_অঙ্ক_Options();
  বাবে (unsigned i = 0; i != অঙ্ক_Opts; ++i) {
    যদি (!মান.compare(get_Option_মান(i)))
      অনুৱর্ত;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t অঙ্ক_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(অঙ্ক_Spaces) << " (default: ";
    বাবে (unsigned j = 0; j != অঙ্ক_Opts; ++j) {
      যদি (!Default.compare(get_Option_মান(j)))
        অনুৱর্ত;
      outs() << getOption(j);
      ভঙ্গ;
    }
    outs() << ")\n";
    ঘূৰাই;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
নামাকাশ llvm {
নামাকাশ cl {
স্থির raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  ঘূৰাই OS << static_cast<পূর্ণাঙ্ক>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  শূন্য parser<T>::printOptionDiff(ধ্রুৱ Option &O, T V, Option_মান<T> D,      \
                                  size_t GlobalWidth) ধ্রুৱ {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t অঙ্ক_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(অঙ্ক_Spaces) << " (default: ";                                 \
    যদি (D.has_মান())                                                          \
      outs() << D.get_মান();                                                  \
    অন্যথা                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(তার্কিক)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(পূর্ণাঙ্ক)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(দ্ৱিগুণ)
PRINT_OPT_DIFF(ভিন্ন)
PRINT_OPT_DIFF(ৱর্ণ)
PRINT_OPT_DIFF(Element_গুনতি)

শূন্য parser<std::string>::printOptionDiff(ধ্রুৱ Option &O, StringRef V,
                                          ধ্রুৱ Option_মান<std::string> &D,
                                          size_t GlobalWidth) ধ্রুৱ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t অঙ্ক_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(অঙ্ক_Spaces) << " (default: ";
  যদি (D.has_মান())
    outs() << D.get_মান();
  অন্যথা
    outs() << "*no default*";
  outs() << ")\n";
}

শূন্য parser<std::optional<std::string>>::printOptionDiff(
    ধ্রুৱ Option &O, std::optional<StringRef> V,
    ধ্রুৱ Option_মান<std::optional<std::string>> &D,
    size_t GlobalWidth) ধ্রুৱ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_মান_2() ? V.মান_2().size() : 0;
  size_t অঙ্ক_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(অঙ্ক_Spaces) << " (default: ";
  যদি (D.has_মান() && D.get_মান().has_মান_2())
    outs() << D.get_মান();
  অন্যথা
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
শূন্য basic_parser_impl::print_Option_No_মান(ধ্রুৱ Option &O,
                                           size_t GlobalWidth) ধ্রুৱ {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

স্থির পূর্ণাঙ্ক OptNameCompare(ধ্রুৱ std::pair<ধ্রুৱ ৱর্ণ *, Option *> *LHS,
                          ধ্রুৱ std::pair<ধ্রুৱ ৱর্ণ *, Option *> *RHS) {
  ঘূৰাই strcmp(LHS->first, RHS->first);
}

স্থির পূর্ণাঙ্ক SubNameCompare(ধ্রুৱ std::pair<ধ্রুৱ ৱর্ণ *, SubCommand *> *LHS,
                          ধ্রুৱ std::pair<ধ্রুৱ ৱর্ণ *, SubCommand *> *RHS) {
  ঘূৰাই strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
স্থির শূন্য sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ধ্রুৱ ৱর্ণ *, Option *>> &Opts,
                     তার্কিক ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  বাবে (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    যদি (I->second->getOptionHiddenFlag() == ReallyHidden)
      অনুৱর্ত;

    // Unless showhidden is set, ignore hidden flags.
    যদি (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      অনুৱর্ত;

    // If we've already seen this option, don't add it to the list again.
    যদি (!OptionSet.ঢোকাও(I->second).second)
      অনুৱর্ত;

    Opts.ঠেলো_back(
        std::pair<ধ্রুৱ ৱর্ণ *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

স্থির শূন্য
sortSubCommands(ধ্রুৱ SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ধ্রুৱ ৱর্ণ *, SubCommand *>> &Subs) {
  বাবে (auto *S : SubMap) {
    যদি (S->getName().empty())
      অনুৱর্ত;
    Subs.ঠেলো_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

নামাকাশ {

শ্ৰেণী HelpPrinter {
রক্ষিত:
  ধ্রুৱ তার্কিক ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ধ্রুৱ ৱর্ণ *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ধ্রুৱ ৱর্ণ *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  আভাস শূন্য printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    বাবে (ধ্রুৱ auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  শূন্য printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    বাবে (ধ্রুৱ auto &S : Subs) {
      outs() << "  " << S.first;
      যদি (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

সার্ৱজনিক:
  explicit HelpPrinter(তার্কিক showHidden) : ShowHidden(showHidden) {}
  আভাস ~HelpPrinter() = স্ৱভাৱ;

  // Invoke the printer.
  শূন্য operator=(তার্কিক মান) {
    যদি (!মান)
      ঘূৰাই;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  শূন্য printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    যদি (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    যদি (Sub == &SubCommand::get_শীৰ্ষ_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      যদি (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } অন্যথা {
      যদি (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    বাবে (auto *Opt : PositionalOpts) {
      যদি (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    যদি (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    যদি (Sub == &SubCommand::get_শীৰ্ষ_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      বাবে (ধ্রুৱ auto &Sub : Subs)
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
    বাবে (ধ্রুৱ auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    যদি (ShowHidden)
      বাবে (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              যদি (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    বাবে (ধ্রুৱ auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    যদি (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    বাবে (ধ্রুৱ auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    বাবে (ধ্রুৱ auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

শ্ৰেণী CategorizedHelpPrinter : সার্ৱজনিক HelpPrinter {
সার্ৱজনিক:
  explicit CategorizedHelpPrinter(তার্কিক showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  স্থির পূর্ণাঙ্ক OptionCategoryCompare(OptionCategory *ধ্রুৱ *A,
                                   OptionCategory *ধ্রুৱ *B) {
    ঘূৰাই (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

রক্ষিত:
  শূন্য printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    প্রতিজ্ঞা(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    বাবে (ধ্রুৱ auto &I : Opts) {
      Option *Opt = I.second;
      বাবে (OptionCategory *Cat : Opt->Categories) {
        প্রতিজ্ঞা(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ঠেলো_back(Opt);
      }
    }

    // Now do printing.
    বাবে (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ধ্রুৱ auto &CategoryOptions = CategorizedOptions[Category];
      যদি (CategoryOptions.empty())
        অনুৱর্ত;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      যদি (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      অন্যথা
        outs() << "\n";

      // Loop over the options in the category and print.
      বাবে (ধ্রুৱ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
শ্ৰেণী HelpPrinterWrapper {
গুপ্ত:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

সার্ৱজনিক:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  শূন্য operator=(তার্কিক মান);
};

} // End anonymous namespace

#যদি defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# যদি defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# অন্যথা
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#নোচেদ্যদি defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# যদি defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# অন্যথা
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#অন্যথা
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

নামাকাশ {
শ্ৰেণী VersionPrinter {
সার্ৱজনিক:
  শূন্য print(ধ্রুৱ std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#অন্যথা
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#যদি LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#অন্যথা
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    যদি (!ExtraPrinters.empty()) {
      বাবে (ধ্রুৱ auto &I : ExtraPrinters)
        I(outs());
    }
  }
  শূন্য operator=(তার্কিক OptionWasSpecified);
};

সংরচনা Command_সাৰি_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{মিছা};
  HelpPrinter UncategorizedHiddenPrinter{সঁচা};
  CategorizedHelpPrinter CategorizedNormalPrinter{মিছা};
  CategorizedHelpPrinter CategorizedHiddenPrinter{সঁচা};
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
  cl::opt<HelpPrinter, সঁচা, parser<তার্কিক>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::মান_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, সঁচা, parser<তার্কিক>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::মান_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, সঁচা, parser<তার্কিক>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::মান_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, সঁচা, parser<তার্কিক>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::মান_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<তার্কিক> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(মিছা),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<তার্কিক> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(মিছা),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, সঁচা, parser<তার্কিক>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::মান_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
স্থির ManagedStatic<Command_সাৰি_Common_Options> CommonOptions;

স্থির শূন্য initCommonOptions() {
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
  স্থির OptionCategory GeneralCategory{"General options"};
  ঘূৰাই GeneralCategory;
}

শূন্য VersionPrinter::operator=(তার্কিক OptionWasSpecified) {
  যদি (!OptionWasSpecified)
    ঘূৰাই;

  যদি (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

শূন্য HelpPrinterWrapper::operator=(তার্কিক মান) {
  যদি (!মান)
    ঘূৰাই;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  যদি (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = সঁচা; // Invoke categorized printer
  } অন্যথা {
    UncategorizedPrinter = সঁচা; // Invoke uncategorized printer
  }
}

// Print the value of each option.
শূন্য cl::Print_Option_মানসমূহ() { globalParser().print_Option_মানসমূহ(); }

শূন্য Command_সাৰি_Parser::print_Option_মানসমূহ() {
  যদি (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ঘূৰাই;

  SmallVector<std::pair<ধ্রুৱ ৱর্ণ *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ সঁচা);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  বাবে (ধ্রুৱ auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  বাবে (ধ্রুৱ auto &Opt : Opts)
    Opt.second->print_Option_মান(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
শূন্য cl::PrintHelpMessage(তার্কিক Hidden, তার্কিক Categorized) {
  যদি (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  অন্যথা যদি (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  অন্যথা যদি (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  অন্যথা
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  স্থির ধ্রুৱ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#যদি LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#যদি __has_feature(address_sanitizer)
      "+asan",
#endif
#যদি __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#যদি __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#যদি __has_feature(memory_sanitizer)
      "+msan",
#endif
#যদি __has_feature(thread_sanitizer)
      "+tsan",
#endif
#যদি __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  ঘূৰাই ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
শূন্য cl::printBuildConfig(raw_ostream &OS) {
#যদি LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
শূন্য cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

শূন্য cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

শূন্য cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.ঠেলো_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (শূন্য)Subs;
  প্রতিজ্ঞা(Subs.contains(&Sub));
  ঘূৰাই Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ঘূৰাই globalParser().getRegisteredSubcommands();
}

শূন্য cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  বাবে (auto &I : Sub.OptionsMap) {
    তার্কিক Unrelated = সঁচা;
    বাবে (auto &Cat : I.second->Categories) {
      যদি (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = মিছা;
    }
    যদি (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

শূন্য cl::HideUnrelatedOptions(ArrayRef<ধ্রুৱ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  বাবে (auto &I : Sub.OptionsMap) {
    তার্কিক Unrelated = সঁচা;
    বাবে (auto &Cat : I.second->Categories) {
      যদি (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = মিছা;
    }
    যদি (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

শূন্য cl::Reset_Command_সাৰি_Parser() { globalParser().reset(); }

শূন্য cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ঠেলো_back(&L);
}

শূন্য LibraryOptionsContext::assign(ধ্রুৱ LibraryOptionsContext &Other) {
  প্রতিজ্ঞা(এতত্ != &Other);
  মানসমূহ.clear();
  SavedArgs.clear();
  ArrayRef<ধ্রুৱ ৱর্ণ *> Args = Other.SavedArgs;
  বাবে (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<ধ্রুৱ ৱর্ণ *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = মানসমূহ.try_emplace(L.getDefault());
  যদি (Inserted)
    It->second = L.copyDefault(Alloc);
  ঘূৰাই parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

শূন্য cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

শূন্য LLVMParse_Command_সাৰি_Options(পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
                                 ধ্রুৱ ৱর্ণ *Overview) {
  llvm::cl::Parse_Command_সাৰি_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
