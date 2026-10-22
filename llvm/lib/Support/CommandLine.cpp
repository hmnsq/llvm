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

#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm-c/Support.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLFunctionalExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallPtrSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LibraryOptionsContext.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/StringSaver.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/TypeSize.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualFileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cstdlib>
#ಅನ್ತರ್ಭಾವ <optional>
#ಅನ್ತರ್ಭಾವ <string>
using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ cl {
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ತಾರ್ಕಿಕ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಪೂರ್ಣಾಙ್ಕ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<long>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ದ್ವಿಗುಣ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಭಿನ್ನ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ವರ್ಣ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<Element_ಎಣಿಕೆ>;

#ಯದಿ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ತಾರ್ಕಿಕ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ವರ್ಣ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ಪೂರ್ಣಾಙ್ಕ>;
template ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
ಶೂನ್ಯ Generic_Option_ಮೌಲ್ಯ::anchor() {}
ಶೂನ್ಯ Option_ಮೌಲ್ಯ<boolOrDefault>::anchor() {}
ಶೂನ್ಯ Option_ಮೌಲ್ಯ<std::string>::anchor() {}
ಶೂನ್ಯ Option::anchor() {}
ಶೂನ್ಯ basic_parser_impl::anchor() {}
ಶೂನ್ಯ parser<ತಾರ್ಕಿಕ>::anchor() {}
ಶೂನ್ಯ parser<boolOrDefault>::anchor() {}
ಶೂನ್ಯ parser<ಪೂರ್ಣಾಙ್ಕ>::anchor() {}
ಶೂನ್ಯ parser<long>::anchor() {}
ಶೂನ್ಯ parser<long long>::anchor() {}
ಶೂನ್ಯ parser<unsigned>::anchor() {}
ಶೂನ್ಯ parser<unsigned long>::anchor() {}
ಶೂನ್ಯ parser<unsigned long long>::anchor() {}
ಶೂನ್ಯ parser<ದ್ವಿಗುಣ>::anchor() {}
ಶೂನ್ಯ parser<ಭಿನ್ನ>::anchor() {}
ಶೂನ್ಯ parser<std::string>::anchor() {}
ಶೂನ್ಯ parser<std::optional<std::string>>::anchor() {}
ಶೂನ್ಯ parser<ವರ್ಣ>::anchor() {}
ಶೂನ್ಯ parser<Element_ಎಣಿಕೆ>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
ಶೂನ್ಯ opt_bool_anchor() { opt<ತಾರ್ಕಿಕ> anchor{""}; }
ಶೂನ್ಯ opt_char_anchor() { opt<ವರ್ಣ> anchor{""}; }
ಶೂನ್ಯ opt_int_anchor() { opt<ಪೂರ್ಣಾಙ್ಕ> anchor{""}; }
ಶೂನ್ಯ opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ಧ್ರುವ ಸ್ಥಿರ size_t DefaultPad = 2;

ಸ್ಥಿರ StringRef ArgPrefix = "-";
ಸ್ಥಿರ StringRef ArgPrefixLong = "--";
ಸ್ಥಿರ StringRef ArgHelpPrefix = " - ";

ಸ್ಥಿರ size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  ಯದಿ (Len == 1)
    ಪ್ರತ್ಯಾಗಮ Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  ಪ್ರತ್ಯಾಗಮ Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

ಸ್ಥಿರ SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  ಪ್ರತಿ (size_t I = 0; I < Pad; ++I) {
    Prefix.ತಳ್ಳು_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  ಪ್ರತ್ಯಾಗಮ Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

ನಾಮಾಕಾಶ {

ವರ್ಗ PrintArg {
  StringRef ArgName;
  size_t Pad;
ಸಾರ್ವಜನಿಕ:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ಧ್ರುವ PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ಧ್ರುವ PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ಪ್ರತ್ಯಾಗಮ OS;
}

ವರ್ಗ Command_ಸಾಲು_Parser {
ಸಾರ್ವಜನಿಕ:
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
  size_t ಅಂಕೆ_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ಸಾಲು_Parser() { registerSubCommand(&SubCommand::get_ತುದಿ_Level()); }

  ಶೂನ್ಯ ResetAllOptionOccurrences();

  ತಾರ್ಕಿಕ Parse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  ಶೂನ್ಯ forEachSubCommand(Option &Opt, function_ref<ಶೂನ್ಯ(SubCommand &)> Action) {
    ಯದಿ (Opt.Subs.empty()) {
      Action(SubCommand::get_ತುದಿ_Level());
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      ಪ್ರತಿ (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಪ್ರತಿ (auto *SC : Opt.Subs) {
      ಪ್ರತಿಜ್ಞಾ(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  ಶೂನ್ಯ addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    ಯದಿ (Opt.hasArgStr())
      ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!SC->OptionsMap.ಸೇರಿಸು(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  ಶೂನ್ಯ addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  ಶೂನ್ಯ addOption(Option *O, SubCommand *SC) {
    ತಾರ್ಕಿಕ HadErrors = ಅಸತ್ಯ;
    ಯದಿ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      ಯದಿ (!SC->OptionsMap.ಸೇರಿಸು(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = ಸತ್ಯ;
      }
    }

    // Remember information about positional options.
    ಯದಿ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ತಳ್ಳು_back(O);
    ಅನ್ಯಥಾ ಯದಿ (O->get_ಅಂಕೆ_Occurrences_Flag() == cl::ConsumeAfter) {
      ಯದಿ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = ಸತ್ಯ;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    ಯದಿ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  ಶೂನ್ಯ addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  ಶೂನ್ಯ removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    ಯದಿ (O->hasArgStr())
      OptionNames.ತಳ್ಳು_back(O->ArgStr);

    SubCommand &Sub = *SC;
    ಪ್ರತಿ (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      ಯದಿ (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    ಯದಿ (O->getFormattingFlag() == cl::Positional)
      ಪ್ರತಿ (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        ಯದಿ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ಭಙ್ಗ;
        }
      }
    ಅನ್ಯಥಾ ಯದಿ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  ಶೂನ್ಯ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  ತಾರ್ಕಿಕ hasOptions(ಧ್ರುವ SubCommand &Sub) ಧ್ರುವ {
    ಪ್ರತ್ಯಾಗಮ (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  ತಾರ್ಕಿಕ hasOptions() ಧ್ರುವ {
    ಪ್ರತಿ (ಧ್ರುವ auto *S : RegisteredSubCommands) {
      ಯದಿ (hasOptions(*S))
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಪ್ರತ್ಯಾಗಮ !Libraries.empty();
  }

  ತಾರ್ಕಿಕ hasNamedSubCommands() ಧ್ರುವ {
    ಪ್ರತಿ (ಧ್ರುವ auto *S : RegisteredSubCommands)
      ಯದಿ (!S->getName().empty())
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  SubCommand *getActiveSubCommand() { ಪ್ರತ್ಯಾಗಮ ActiveSubCommand; }

  ಶೂನ್ಯ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    ಯದಿ (!Sub.OptionsMap.ಸೇರಿಸು(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  ಶೂನ್ಯ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  ಶೂನ್ಯ print_Option_ಮೌಲ್ಯಗಳು();

  ಶೂನ್ಯ indexLibraryOptions() {
    ತಾರ್ಕಿಕ HadErrors = ಅಸತ್ಯ;
    ಪ್ರತಿ (; ಅಂಕೆ_Indexed_Libraries != Libraries.size(); ++ಅಂಕೆ_Indexed_Libraries) {
      LibraryOptions *L = Libraries[ಅಂಕೆ_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        ಯದಿ (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ಪ್ರತ್ಯಾಗಮ SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          ಪ್ರತ್ಯಾಗಮ;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = ಸತ್ಯ;
      });
    }
    ಯದಿ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    ಪ್ರತ್ಯಾಗಮ LibraryIndex.lookup(Name);
  }

  ಶೂನ್ಯ registerCategory(OptionCategory *cat) {
    ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_if(RegisteredOptionCategories,
                    [cat](ಧ್ರುವ OptionCategory *Category) {
             ಪ್ರತ್ಯಾಗಮ cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ಸೇರಿಸು(cat);
  }

  ಶೂನ್ಯ registerSubCommand(SubCommand *sub) {
    ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_if(RegisteredSubCommands,
                    [sub](ಧ್ರುವ SubCommand *Sub) {
                      ಪ್ರತ್ಯಾಗಮ (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ಸೇರಿಸು(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ಪ್ರತಿಜ್ಞಾ(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    ಪ್ರತಿ (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      ಯದಿ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      ಅನ್ಯಥಾ
        addLiteralOption(*O, sub, E.first);
    }
  }

  ಶೂನ್ಯ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ಪ್ರತ್ಯಾಗಮ make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  ಶೂನ್ಯ reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    ಅಂಕೆ_Indexed_Libraries = 0;

    SubCommand::get_ತುದಿ_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_ತುದಿ_Level());
  }

ಗುಪ್ತ:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &ಮೌಲ್ಯ);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ಸ್ಥಿರ Command_ಸಾಲು_Parser &globalParser() {
  ಸ್ಥಿರ ManagedStatic<Command_ಸಾಲು_Parser> GlobalParser;
  ಪ್ರತ್ಯಾಗಮ *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ಸ್ಥಿರ Error
parseLibraryOption(LibraryOptions &L, ಶೂನ್ಯ *Opts, ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ಧ್ರುವ ವರ್ಣ *Saved[2];
  unsigned ಅಂಕೆ_Args = std::min<size_t>(Args.size(), 2);
  ಪ್ರತಿ (unsigned J = 0; J != ಅಂಕೆ_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  ಯದಿ (Error E = L.parse(Opts, ArrayRef(Saved, ಅಂಕೆ_Args), Consumed, Alloc))
    ಪ್ರತ್ಯಾಗಮ E;
  ಯದಿ (Record)
    Record->append(Saved, Saved + Consumed);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

template <typename T, T TrueVal, T FalseVal>
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseBool(Option &O, StringRef ArgName, StringRef Arg, T &ಮೌಲ್ಯ) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  ಯದಿ (!Arg.data() || Arg == "true" || Arg == "1") {
    ಮೌಲ್ಯ = TrueVal;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  ಯದಿ (Arg == "false" || Arg == "0") {
    ಮೌಲ್ಯ = FalseVal;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

ಶೂನ್ಯ cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ತಳ್ಳು_back(Help);
}

Option::Option(ಅಂಕೆ_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : ಅಂಕೆ_Occurrences(0), Occurrences(OccurrencesFlag), ಮೌಲ್ಯ(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(ಅಸತ್ಯ), Position(0) {
  Categories.ತಳ್ಳು_back(&getGeneralCategory());
}

ಶೂನ್ಯ Option::addArgument() {
  globalParser().addOption(ಏತತ್);
  FullyInitialized = ಸತ್ಯ;
}

ಶೂನ್ಯ Option::removeArgument() { globalParser().removeOption(ಏತತ್); }

ಶೂನ್ಯ Option::setArgStr(StringRef S) {
  ಯದಿ (FullyInitialized)
    globalParser().updateArgStr(ಏತತ್, S);
  ಪ್ರತಿಜ್ಞಾ(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

ಶೂನ್ಯ Option::addCategory(OptionCategory &C) {
  ಪ್ರತಿಜ್ಞಾ(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  ಯದಿ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  ಅನ್ಯಥಾ ಯದಿ (!is_contained(Categories, &C))
    Categories.ತಳ್ಳು_back(&C);
}

ಶೂನ್ಯ Option::reset() {
  ಅಂಕೆ_Occurrences = 0;
  setDefault();
}

ಶೂನ್ಯ OptionCategory::registerCategory() {
  globalParser().registerCategory(ಏತತ್);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_ತುದಿ_Level() {
  ಸ್ಥಿರ ManagedStatic<SubCommand> ತುದಿ_Level_Sub_Command;
  ಪ್ರತ್ಯಾಗಮ *ತುದಿ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ಸ್ಥಿರ ManagedStatic<SubCommand> AllSubCommands;
  ಪ್ರತ್ಯಾಗಮ *AllSubCommands;
}

ಶೂನ್ಯ SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(ಏತತ್);
}

ಶೂನ್ಯ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ಏತತ್);
}

ಶೂನ್ಯ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator ತಾರ್ಕಿಕ() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ (globalParser().getActiveSubCommand() == ಏತತ್);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_ಸಾಲು_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &ಮೌಲ್ಯ) {
  // Reject all dashes.
  ಯದಿ (Arg.empty())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತಿಜ್ಞಾ(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  ಯದಿ (EqualPos == StringRef::npos) {
    // Look up the option.
    ಪ್ರತ್ಯಾಗಮ Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  ಯದಿ (I == Sub.OptionsMap.end())
    ಪ್ರತ್ಯಾಗಮ nullptr;

  auto *O = I->second;
  ಯದಿ (O->getFormattingFlag() == cl::AlwaysPrefix)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಮೌಲ್ಯ = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ಪ್ರತ್ಯಾಗಮ I->second;
}

SubCommand *Command_ಸಾಲು_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  ಯದಿ (Name.empty())
    ಪ್ರತ್ಯಾಗಮ &SubCommand::get_ತುದಿ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  ಪ್ರತಿ (auto *S : RegisteredSubCommands) {
    ಪ್ರತಿಜ್ಞಾ(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    ಯದಿ (S->getName().empty())
      ಅನುವರ್ತ;

    ಯದಿ (S->getName() == Name)
      ಪ್ರತ್ಯಾಗಮ S;

    ಯದಿ (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  ಯದಿ (NearestMatch)
    NearestString = NearestMatch->getName();

  ಪ್ರತ್ಯಾಗಮ &SubCommand::get_ತುದಿ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ಸ್ಥಿರ Option *LookupNearestOption(StringRef Arg,
                                   ಧ್ರುವ OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  ಯದಿ (Arg.empty())
    ಪ್ರತ್ಯಾಗಮ nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  ಪ್ರತಿ (ಧ್ರುವ auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    ಯದಿ (O->getOptionHiddenFlag() == ReallyHidden)
      ಅನುವರ್ತ;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    ಯದಿ (O->hasArgStr())
      OptionNames.ತಳ್ಳು_back(O->ArgStr);

    ತಾರ್ಕಿಕ Permit_ಮೌಲ್ಯ = O->get_ಮೌಲ್ಯ_Expected_Flag() != cl::ಮೌಲ್ಯ_Disallowed;
    StringRef Flag = Permit_ಮೌಲ್ಯ ? LHS : Arg;
    ಪ್ರತಿ (ಧ್ರುವ auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/ಸತ್ಯ, /*MaxEditDistance=*/BestDistance);
      ಯದಿ (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        ಯದಿ (RHS.empty() || !Permit_ಮೌಲ್ಯ)
          NearestString = std::string(Name);
        ಅನ್ಯಥಾ
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ಸ್ಥಿರ ತಾರ್ಕಿಕ CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef ಮೌಲ್ಯ) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  ಯದಿ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(ಮೌಲ್ಯ);
    StringRef::size_type Pos = Val.find(',');

    ಯಾವತ್ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      ಯದಿ (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    ಮೌಲ್ಯ = Val;
  }

  ಪ್ರತ್ಯಾಗಮ Handler->addOccurrence(pos, ArgName, ಮೌಲ್ಯ);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ತಾರ್ಕಿಕ ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef ಮೌಲ್ಯ, ಪೂರ್ಣಾಙ್ಕ argc,
                                 ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, ಪೂರ್ಣಾಙ್ಕ &i) {
  // Enforce value requirements
  ವಿಕಲ್ಪ (Handler->get_ಮೌಲ್ಯ_Expected_Flag()) {
  ಪಕ್ಷ ಮೌಲ್ಯ_Required:
    ಯದಿ (!ಮೌಲ್ಯ.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      ಯದಿ (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        ಪ್ರತ್ಯಾಗಮ Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ಪ್ರತಿಜ್ಞಾ(argv && "null check");
      ಮೌಲ್ಯ = StringRef(argv[++i]);
    }
    ಭಙ್ಗ;
  ಪಕ್ಷ ಮೌಲ್ಯ_Disallowed:
    ಯದಿ (ಮೌಲ್ಯ.data())
      ಪ್ರತ್ಯಾಗಮ Handler->error("does not allow a value! '" + Twine(ಮೌಲ್ಯ) +
                            "' specified.");
    ಭಙ್ಗ;
  ಪಕ್ಷ ಮೌಲ್ಯ_Optional:
    ಭಙ್ಗ;
  }

  ಪ್ರತ್ಯಾಗಮ CommaSeparateAndAddOccurrence(Handler, i, ArgName, ಮೌಲ್ಯ);
}

ತಾರ್ಕಿಕ llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, ಪೂರ್ಣಾಙ್ಕ i) {
  ಪೂರ್ಣಾಙ್ಕ Dummy = i;
  ಪ್ರತ್ಯಾಗಮ ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ಸ್ಥಿರ Option *findPrefixOption(StringRef Name, size_t &Length,
                                ಧ್ರುವ OptionsMapTy &OptionsMap) {
  ಪ್ರತಿ (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    ಯದಿ (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      ಪ್ರತ್ಯಾಗಮ O;
    }
  }
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ಸ್ಥಿರ Option *HandlePrefixedOption(StringRef &Arg, StringRef &ಮೌಲ್ಯ,
                                    ಧ್ರುವ OptionsMapTy &OptionsMap) {
  ಯದಿ (Arg.size() == 1)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  ಯದಿ (!POpt)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  StringRef Maybe_ಮೌಲ್ಯ =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  ಯದಿ (POpt->getFormattingFlag() == cl::Prefix && Maybe_ಮೌಲ್ಯ.starts_with("="))
    Maybe_ಮೌಲ್ಯ = Maybe_ಮೌಲ್ಯ.drop_front();
  ಮೌಲ್ಯ = Maybe_ಮೌಲ್ಯ;
  ಪ್ರತ್ಯಾಗಮ POpt;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ Requires_ಮೌಲ್ಯ(ಧ್ರುವ Option *O) {
  ಪ್ರತ್ಯಾಗಮ O->get_ಅಂಕೆ_Occurrences_Flag() == cl::Required ||
         O->get_ಅಂಕೆ_Occurrences_Flag() == cl::OneOrMore;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ Eats_Unbounded_ಸಂಖ್ಯೆ_Of_ಮೌಲ್ಯಗಳು(ಧ್ರುವ Option *O) {
  ಪ್ರತ್ಯಾಗಮ O->get_ಅಂಕೆ_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_ಅಂಕೆ_Occurrences_Flag() == cl::OneOrMore;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isWhitespace(ವರ್ಣ C) {
  ಪ್ರತ್ಯಾಗಮ C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isWhitespaceOrNull(ವರ್ಣ C) {
  ಪ್ರತ್ಯಾಗಮ isWhitespace(C) || C == '\0';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isQuote(ವರ್ಣ C) { ಪ್ರತ್ಯಾಗಮ C == '\"' || C == '\''; }

ಶೂನ್ಯ cl::Tokenize_GNUCommand_ಸಾಲು(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv,
                                ತಾರ್ಕಿಕ MarkEOLs) {
  SmallString<128> Token;
  ತಾರ್ಕಿಕ InToken = ಅಸತ್ಯ;
  ಪ್ರತಿ (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    ಯದಿ (!InToken) {
      ಯಾವತ್ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        ಯದಿ (MarkEOLs && Src[I] == '\n')
          NewArgv.ತಳ್ಳು_back(nullptr);
        ++I;
      }
      ಯದಿ (I == E)
        ಭಙ್ಗ;
      InToken = ಸತ್ಯ;
    }

    ವರ್ಣ C = Src[I];

    // Backslash escapes the next character.
    ಯದಿ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ತಳ್ಳು_back(Src[I]);
      ಅನುವರ್ತ;
    }

    // Consume a quoted string.
    ಯದಿ (isQuote(C)) {
      ++I;
      ಯಾವತ್ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        ಯದಿ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ತಳ್ಳು_back(Src[I]);
        ++I;
      }
      ಯದಿ (I == E)
        ಭಙ್ಗ;
      ಅನುವರ್ತ;
    }

    // End the token if this is whitespace.
    ಯದಿ (isWhitespace(C)) {
      NewArgv.ತಳ್ಳು_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      ಯದಿ (MarkEOLs && C == '\n')
        NewArgv.ತಳ್ಳು_back(nullptr);
      Token.clear();
      InToken = ಅಸತ್ಯ;
      ಅನುವರ್ತ;
    }

    // This is a normal character.  Append it.
    Token.ತಳ್ಳು_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  ಯದಿ (InToken)
    NewArgv.ತಳ್ಳು_back(Saver.save(Token.str()).data());
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
ಸ್ಥಿರ size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  ಪೂರ್ಣಾಙ್ಕ Backslash_ಎಣಿಕೆ = 0;
  // Skip the backslashes.
  ಕುರು {
    ++I;
    ++Backslash_ಎಣಿಕೆ;
  } ಯಾವತ್ (I != E && Src[I] == '\\');

  ತಾರ್ಕಿಕ FollowedByDoubleQuote = (I != E && Src[I] == '"');
  ಯದಿ (FollowedByDoubleQuote) {
    Token.append(Backslash_ಎಣಿಕೆ / 2, '\\');
    ಯದಿ (Backslash_ಎಣಿಕೆ % 2 == 0)
      ಪ್ರತ್ಯಾಗಮ I - 1;
    Token.ತಳ್ಳು_back('"');
    ಪ್ರತ್ಯಾಗಮ I;
  }
  Token.append(Backslash_ಎಣಿಕೆ, '\\');
  ಪ್ರತ್ಯಾಗಮ I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isWindowsSpecialChar(ವರ್ಣ C) {
  ಪ್ರತ್ಯಾಗಮ isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ಸ್ಥಿರ ತಾರ್ಕಿಕ isWindowsSpecialCharInCommandName(ವರ್ಣ C) {
  ಪ್ರತ್ಯಾಗಮ isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ tokenize_Windows_Command_ಸಾಲು_Impl(
    StringRef Src, StringSaver &Saver, function_ref<ಶೂನ್ಯ(StringRef)> AddToken,
    ತಾರ್ಕಿಕ AlwaysCopy, function_ref<ಶೂನ್ಯ()> MarkEOL, ತಾರ್ಕಿಕ InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  ತಾರ್ಕಿಕ CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  ಗಣನಾ { INIT, UNQUOTED, QUOTED } State = INIT;

  ಪ್ರತಿ (size_t I = 0, E = Src.size(); I < E; ++I) {
    ವಿಕಲ್ಪ (State) {
    ಪಕ್ಷ INIT: {
      ಪ್ರತಿಜ್ಞಾ(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      ಯಾವತ್ (I < E && isWhitespaceOrNull(Src[I])) {
        ಯದಿ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      ಯದಿ (I >= E)
        ಭಙ್ಗ;
      size_t Start = I;
      ಯದಿ (CommandName) {
        ಯಾವತ್ (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } ಅನ್ಯಥಾ {
        ಯಾವತ್ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      ಯದಿ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        ಯದಿ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } ಅನ್ಯಥಾ {
          CommandName = ಅಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ ಯದಿ (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } ಅನ್ಯಥಾ ಯದಿ (Src[I] == '\\') {
        ಪ್ರತಿಜ್ಞಾ(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } ಅನ್ಯಥಾ {
        llvm_unreachable("unexpected special character");
      }
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ UNQUOTED:
      ಯದಿ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        ಯದಿ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } ಅನ್ಯಥಾ {
          CommandName = ಅಸತ್ಯ;
        }
        State = INIT;
      } ಅನ್ಯಥಾ ಯದಿ (Src[I] == '\"') {
        State = QUOTED;
      } ಅನ್ಯಥಾ ಯದಿ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ಅನ್ಯಥಾ {
        Token.ತಳ್ಳು_back(Src[I]);
      }
      ಭಙ್ಗ;

    ಪಕ್ಷ QUOTED:
      ಯದಿ (Src[I] == '\"') {
        ಯದಿ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ತಳ್ಳು_back('"');
          ++I;
        } ಅನ್ಯಥಾ {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } ಅನ್ಯಥಾ ಯದಿ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ಅನ್ಯಥಾ {
        Token.ತಳ್ಳು_back(Src[I]);
      }
      ಭಙ್ಗ;
    }
  }

  ಯದಿ (State != INIT)
    AddToken(Saver.save(Token.str()));
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv,
                                    ತಾರ್ಕಿಕ MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_back(Tok.data()); };
  auto OnEOL = [&]() {
    ಯದಿ (MarkEOLs)
      NewArgv.ತಳ್ಳು_back(nullptr);
  };
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/ಸತ್ಯ, OnEOL, ಅಸತ್ಯ);
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/ಅಸತ್ಯ,
                                 OnEOL, ಅಸತ್ಯ);
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv,
                                        ತಾರ್ಕಿಕ MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_back(Tok.data()); };
  auto OnEOL = [&]() {
    ಯದಿ (MarkEOLs)
      NewArgv.ತಳ್ಳು_back(nullptr);
  };
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/ಸತ್ಯ, OnEOL, ಸತ್ಯ);
}

ಶೂನ್ಯ cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv,
                            ತಾರ್ಕಿಕ MarkEOLs) {
  ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> ಸಾಲು;
    // Check for comment line.
    ಯದಿ (isWhitespace(*Cur)) {
      ಯಾವತ್ (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      ಅನುವರ್ತ;
    }
    ಯದಿ (*Cur == '#') {
      ಯಾವತ್ (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      ಅನುವರ್ತ;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ಧ್ರುವ ವರ್ಣ *Start = Cur;
    ಧ್ರುವ ವರ್ಣ *ಸಾಲು_End = nullptr;
    ವರ್ಣ Quote = 0;
    ತಾರ್ಕಿಕ AtTokenStart = ಸತ್ಯ;
    ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ *End = Source.end(); Cur != End; ++Cur) {
      ವರ್ಣ C = *Cur;
      ಯದಿ (C == '\\') {
        ಯದಿ (Cur + 1 != End) {
          ++Cur;
          ಯದಿ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            ಸಾಲು.append(Start, Cur - 1);
            ಯದಿ (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            ಅನುವರ್ತ;
          }
        }
        AtTokenStart = ಅಸತ್ಯ;
        ಅನುವರ್ತ;
      }
      ಯದಿ (C == '\n')
        ಭಙ್ಗ;
      ಯದಿ (Quote) {
        ಯದಿ (C == Quote)
          Quote = 0;
      } ಅನ್ಯಥಾ ಯದಿ (isQuote(C)) {
        Quote = C;
      } ಅನ್ಯಥಾ ಯದಿ (C == '#' && AtTokenStart) {
        ಸಾಲು_End = Cur;
        ಯಾವತ್ (Cur != End && *Cur != '\n')
          ++Cur;
        ಭಙ್ಗ;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    ಸಾಲು.append(Start, ಸಾಲು_End ? ಸಾಲು_End : Cur);
    cl::Tokenize_GNUCommand_ಸಾಲು(ಸಾಲು, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ಸ್ಥಿರ ತಾರ್ಕಿಕ hasUTF8ByteOrderMark(ArrayRef<ವರ್ಣ> S) {
  ಪ್ರತ್ಯಾಗಮ (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ಸ್ಥಿರ ಶೂನ್ಯ ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ಧ್ರುವ ವರ್ಣ *&Arg) {
  ಪ್ರತಿಜ್ಞಾ(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ಧ್ರುವ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  ಪ್ರತಿ (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ಧ್ರುವ StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    ಯದಿ (ResponseFile.empty())
      ResponseFile = LHS;
    ಅನ್ಯಥಾ
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  ಯದಿ (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ಧ್ರುವ StringRef Remaining = ArgString.substr(StartPos);
    ಯದಿ (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv) {
  ಪ್ರತಿಜ್ಞಾ(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  ಯದಿ (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    ಪ್ರತ್ಯಾಗಮ llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<ವರ್ಣ> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  ಯದಿ (hasUTF16ByteOrderMark(BufRef)) {
    ಯದಿ (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      ಪ್ರತ್ಯಾಗಮ llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  ಅನ್ಯಥಾ ಯದಿ (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  ಯದಿ (!RelativeNames && !InConfigFile)
    ಪ್ರತ್ಯಾಗಮ Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ *&Arg : NewArgv) {
    ಯದಿ (!Arg)
      ಅನುವರ್ತ;

    // Substitute <CFGDIR> with the file's base path.
    ಯದಿ (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    ತಾರ್ಕಿಕ ConfigInclusion = ಅಸತ್ಯ;
    ಯದಿ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      ಯದಿ (!llvm::sys::path::is_relative(FileName))
        ಅನುವರ್ತ;
    } ಅನ್ಯಥಾ ಯದಿ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = ಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      ಅನುವರ್ತ;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ತಳ್ಳು_back('@');
    ಯದಿ (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      ಯದಿ (!findConfigFile(FileName, FilePath))
        ಪ್ರತ್ಯಾಗಮ createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } ಅನ್ಯಥಾ {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &Argv) {
  ಸಂರಚನಾ ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_ರಾಶಿ;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ರಾಶಿ.ತಳ್ಳು_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  ಪ್ರತಿ (unsigned I = 0; I != Argv.size();) {
    ಯಾವತ್ (I == File_ರಾಶಿ.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ರಾಶಿ.ತೆಗೆ_back();
    }

    ಧ್ರುವ ವರ್ಣ *Arg = Argv[I];
    // Check if it is an EOL marker
    ಯದಿ (Arg == nullptr) {
      ++I;
      ಅನುವರ್ತ;
    }

    ಯದಿ (Arg[0] != '@') {
      ++I;
      ಅನುವರ್ತ;
    }

    ಧ್ರುವ ವರ್ಣ *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    ಯದಿ (llvm::sys::path::is_relative(FName)) {
      ಯದಿ (CurrentDir.empty()) {
        ಯದಿ (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } ಅನ್ಯಥಾ {
          ಪ್ರತ್ಯಾಗಮ createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } ಅನ್ಯಥಾ {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    ಯದಿ (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      ಯದಿ (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        ಯದಿ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ಅನುವರ್ತ;
        }
      }
      ಯದಿ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ಪ್ರತ್ಯಾಗಮ createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ಧ್ರುವ llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, ಏತತ್](ಧ್ರುವ ResponseFileRecord &RFile) -> ErrorOr<ತಾರ್ಕಿಕ> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      ಯದಿ (!RHS)
        ಪ್ರತ್ಯಾಗಮ RHS.getError();
      ಪ್ರತ್ಯಾಗಮ FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    ಪ್ರತಿ (ಧ್ರುವ auto &F : drop_begin(File_ರಾಶಿ)) {
      ಯದಿ (ErrorOr<ತಾರ್ಕಿಕ> R = IsEquivalent(F)) {
        ಯದಿ (R.get())
          ಪ್ರತ್ಯಾಗಮ createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ಧ್ರುವ ವರ್ಣ *, 0> ExpandedArgv;
    ಯದಿ (Error Err = expandResponseFile(FName, ExpandedArgv))
      ಪ್ರತ್ಯಾಗಮ Err;

    ಪ್ರತಿ (ResponseFileRecord &Record : File_ರಾಶಿ) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_ರಾಶಿ.ತಳ್ಳು_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.ಸೇರಿಸು(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ಪ್ರತಿಜ್ಞಾ(File_ರಾಶಿ.size() > 0 && Argv.size() == File_ರಾಶಿ.back().End);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

ತಾರ್ಕಿಕ cl::expandResponseFiles(ಪೂರ್ಣಾಙ್ಕ Argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *Argv,
                             ಧ್ರುವ ವರ್ಣ *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ಸಾಲು;
#ಅನ್ಯಥಾ
  auto Tokenize = cl::Tokenize_GNUCommand_ಸಾಲು;
#endif
  // The environment variable specifies initial options.
  ಯದಿ (EnvVar)
    ಯದಿ (std::optional<std::string> Env_ಮೌಲ್ಯ = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_ಮೌಲ್ಯ, Saver, NewArgv, /*MarkEOLs=*/ಅಸತ್ಯ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  ಯದಿ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  ಯದಿ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

ತಾರ್ಕಿಕ ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<ವರ್ಣ> &FilePath) {
  SmallString<128> CfgFilePath;
  ಧ್ರುವ auto FileExists = [ಏತತ್](SmallString<128> Path) -> ತಾರ್ಕಿಕ {
    auto Status = FS->status(Path);
    ಪ್ರತ್ಯಾಗಮ Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  ಯದಿ (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    ಯದಿ (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    ಯದಿ (!FileExists(CfgFilePath))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  // Look for the file in search directories.
  ಪ್ರತಿ (ಧ್ರುವ StringRef &Dir : ಹುಡುಕು_Dirs) {
    ಯದಿ (Dir.empty())
      ಅನುವರ್ತ;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    ಯದಿ (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ಧ್ರುವ ವರ್ಣ *> &Argv) {
  SmallString<128> AbsPath;
  ಯದಿ (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    ಯದಿ (std::error_code EC = FS->makeAbsolute(AbsPath))
      ಪ್ರತ್ಯಾಗಮ make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = ಸತ್ಯ;
  RelativeNames = ಸತ್ಯ;
  ಯದಿ (Error Err = expandResponseFile(CfgFile, Argv))
    ಪ್ರತ್ಯಾಗಮ Err;
  ಪ್ರತ್ಯಾಗಮ expandResponseFiles(Argv);
}

ಸ್ಥಿರ ಶೂನ್ಯ initCommonOptions();
ಸ್ಥಿರ ತಾರ್ಕಿಕ parse_Command_ಸಾಲು_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ಧ್ರುವ ವರ್ಣ *EnvVar) {
  initCommonOptions();
  SmallVector<ಧ್ರುವ ವರ್ಣ *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ತಳ್ಳು_back(Args[0]);

  // Parse options from environment variable.
  ಯದಿ (EnvVar) {
    ಯದಿ (std::optional<std::string> Env_ಮೌಲ್ಯ =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ಸಾಲು(*Env_ಮೌಲ್ಯ, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  ಪೂರ್ಣಾಙ್ಕ NewArgc = static_cast<ಪೂರ್ಣಾಙ್ಕ>(NewArgv.size());

  // Parse all options.
  ಪ್ರತ್ಯಾಗಮ globalParser().Parse_Command_ಸಾಲು_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

ತಾರ್ಕಿಕ cl::Parse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ಧ್ರುವ ವರ್ಣ *EnvVar) {
  ಪ್ರತ್ಯಾಗಮ parse_Command_ಸಾಲು_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

ತಾರ್ಕಿಕ cl::parse_Command_ಸಾಲು_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ಧ್ರುವ ವರ್ಣ *EnvVar) {
  ಪ್ರತ್ಯಾಗಮ parse_Command_ಸಾಲು_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
ಶೂನ್ಯ Command_ಸಾಲು_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  ಪ್ರತಿ (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    ಪ್ರತಿ (auto &O : SC->OptionsMap)
      Opts.ತಳ್ಳು_back(O.second);
    ಪ್ರತಿ (Option *O : Opts)
      O->reset();
    ಪ್ರತಿ (Option *O : SC->PositionalOpts)
      O->reset();
    ಯದಿ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  ಪ್ರತಿ (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

ತಾರ್ಕಿಕ Command_ಸಾಲು_Parser::Parse_Command_ಸಾಲು_Options(
    ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ಪ್ರತಿಜ್ಞಾ(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  ತಾರ್ಕಿಕ IgnoreErrors = Errs;
  ಯದಿ (!Errs)
    Errs = &errs();
  ಯದಿ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  ತಾರ್ಕಿಕ ErrorParsing = ಅಸತ್ಯ;

  // Expand response files.
  SmallVector<ಧ್ರುವ ವರ್ಣ *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ಸಾಲು;
#ಅನ್ಯಥಾ
  auto Tokenize = cl::Tokenize_GNUCommand_ಸಾಲು;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  ಯದಿ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  argv = &newArgv[0];
  argc = static_cast<ಪೂರ್ಣಾಙ್ಕ>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned ಅಂಕೆ_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  ತಾರ್ಕಿಕ HasUnlimitedPositionals = ಅಸತ್ಯ;

  ಪೂರ್ಣಾಙ್ಕ FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_ತುದಿ_Level();
  std::string NearestSubCommandString;
  ತಾರ್ಕಿಕ MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  ಯದಿ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    ಯದಿ (ChosenSubCommand != &SubCommand::get_ತುದಿ_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  ಪ್ರತಿಜ್ಞಾ(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  ಯದಿ (ConsumeAfterOpt) {
    ಪ್ರತಿಜ್ಞಾ(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  ಯದಿ (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    ತಾರ್ಕಿಕ UnboundedFound = ಅಸತ್ಯ;
    ಪ್ರತಿ (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      ಯದಿ (Requires_ಮೌಲ್ಯ(Opt))
        ++ಅಂಕೆ_Positional_Required;
      ಅನ್ಯಥಾ ಯದಿ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        ಯದಿ (PositionalOpts.size() > 1) {
          ಯದಿ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = ಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ ಯದಿ (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        ಯದಿ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = ಸತ್ಯ;
      }
      UnboundedFound |= Eats_Unbounded_ಸಂಖ್ಯೆ_Of_ಮೌಲ್ಯಗಳು(Opt);
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
  ತಾರ್ಕಿಕ DashDashFound = ಅಸತ್ಯ; // Have we read '--'?
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef ಮೌಲ್ಯ;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    ಯದಿ (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      ಯದಿ (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ಅನುವರ್ತ; // We are done!
      }

      ಯದಿ (!PositionalOpts.empty()) {
        PositionalVals.ತಳ್ಳು_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        ಯದಿ (PositionalVals.size() >= ಅಂಕೆ_Positional_Required && ConsumeAfterOpt) {
          ಪ್ರತಿ (++i; i < argc; ++i)
            PositionalVals.ತಳ್ಳು_back(std::make_pair(StringRef(argv[i]), i));
          ಭಙ್ಗ; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ಅನುವರ್ತ;
      }
    } ಅನ್ಯಥಾ ಯದಿ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = ಸತ್ಯ; // This is the mythical "--"?
      ಅನುವರ್ತ;             // Don't try to process it as an argument itself.
    } ಅನ್ಯಥಾ ಯದಿ (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ಮೌಲ್ಯ);
      ಯದಿ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ಅನುವರ್ತ; // We are done!
      }
    } ಅನ್ಯಥಾ { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ಮೌಲ್ಯ);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      ಯದಿ (!Handler && ChosenSubCommand != &SubCommand::get_ತುದಿ_Level())
        Handler = LookupOption(SubCommand::get_ತುದಿ_Level(), ArgName, ಮೌಲ್ಯ);

      ಯದಿ (!Handler) {
        ಯದಿ (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ಧ್ರುವ ವರ್ಣ *> Rest(argv + i, argc - i);
          unsigned N;
          ಯದಿ (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = ಸತ್ಯ;
          }
          i += N - 1;
          ಅನುವರ್ತ;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      ಯದಿ (!Handler)
        Handler = HandlePrefixedOption(ArgName, ಮೌಲ್ಯ, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      ಯದಿ (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    ಯದಿ (!Handler) {
      auto ReportUnknownArgument = [&](ತಾರ್ಕಿಕ IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        ಯದಿ (NearestArgumentName.empty())
          ಪ್ರತ್ಯಾಗಮ;

        *Errs << ProgramName << ": Did you mean '";
        ಯದಿ (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        ಅನ್ಯಥಾ
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      ಯದಿ (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/ಸತ್ಯ, NearestHandlerString);
      ಅನ್ಯಥಾ
        ReportUnknownArgument(/*IsArg=*/ಅಸತ್ಯ, NearestSubCommandString);

      ErrorParsing = ಸತ್ಯ;
      ಅನುವರ್ತ;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    ಯದಿ (Handler->getFormattingFlag() == cl::Positional) {
      ಯದಿ ((Handler->getMiscFlags() & PositionalEatsArgs) && !ಮೌಲ್ಯ.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = ಸತ್ಯ;
      }
      ActivePositionalArg = Handler;
    }
    ಅನ್ಯಥಾ
      ErrorParsing |= ProvideOption(Handler, ArgName, ಮೌಲ್ಯ, argc, argv, i);
  }

  // Check and handle positional arguments now...
  ಯದಿ (ಅಂಕೆ_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << ಅಂಕೆ_Positional_Required
             << " positional argument" << (ಅಂಕೆ_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = ಸತ್ಯ;
  } ಅನ್ಯಥಾ ಯದಿ (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = ಸತ್ಯ;

  } ಅನ್ಯಥಾ ಯದಿ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, ಅಂಕೆ_Vals = static_cast<unsigned>(PositionalVals.size());
    ಪ್ರತಿ (Option *Opt : PositionalOpts) {
      ಯದಿ (Requires_ಮೌಲ್ಯ(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --ಅಂಕೆ_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      ತಾರ್ಕಿಕ Done = Opt->get_ಅಂಕೆ_Occurrences_Flag() == cl::Required;
      ಯಾವತ್ (ಅಂಕೆ_Vals - ValNo > ಅಂಕೆ_Positional_Required && !Done) {
        ವಿಕಲ್ಪ (Opt->get_ಅಂಕೆ_Occurrences_Flag()) {
        ಪಕ್ಷ cl::Optional:
          Done = ಸತ್ಯ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        ಪಕ್ಷ cl::ZeroOrMore: // Zero or more will take all they can get...
        ಪಕ್ಷ cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          ಭಙ್ಗ;
        ಸ್ವಭಾವ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(ConsumeAfterOpt && ಅಂಕೆ_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    ಪ್ರತಿ (Option *Opt : PositionalOpts)
      ಯದಿ (Requires_ಮೌಲ್ಯ(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    ಯದಿ (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    ಪ್ರತಿ (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  ಪ್ರತಿ (ಧ್ರುವ auto &Opt : OptionsMap) {
    ವಿಕಲ್ಪ (Opt.second->get_ಅಂಕೆ_Occurrences_Flag()) {
    ಪಕ್ಷ Required:
    ಪಕ್ಷ OneOrMore:
      ಯದಿ (Opt.second->get_ಅಂಕೆ_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = ಸತ್ಯ;
      }
      [[fallthrough]];
    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  ಯದಿ (ErrorParsing) {
    ಯದಿ (!IgnoreErrors)
      exit(1);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

ತಾರ್ಕಿಕ Option::error(ಧ್ರುವ Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  ಯದಿ (!ArgName.data())
    ArgName = ArgStr;
  ಯದಿ (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  ಅನ್ಯಥಾ
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef ಮೌಲ್ಯ) {
  ++ಅಂಕೆ_Occurrences;
  ಪ್ರತ್ಯಾಗಮ handleOccurrence(pos, ArgName, ಮೌಲ್ಯ);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ಸ್ಥಿರ StringRef get_ಮೌಲ್ಯ_Str(ಧ್ರುವ Option &O, StringRef Default_ಸಂದೇಶ) {
  ಯದಿ (O.ಮೌಲ್ಯ_Str.empty())
    ಪ್ರತ್ಯಾಗಮ Default_ಸಂದೇಶ;
  ಪ್ರತ್ಯಾಗಮ O.ಮೌಲ್ಯ_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ argPlusPrefixesSize(ArgStr);
}

ಶೂನ್ಯ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_ಸಾಲು_Indented_By) {
  ಪ್ರತಿಜ್ಞಾ(Indent >= First_ಸಾಲು_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_ಸಾಲು_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  ಯಾವತ್ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

ಶೂನ್ಯ Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_ಸಾಲು_Indented_By) {
  ಧ್ರುವ StringRef ValHelpPrefix = "  ";
  ಪ್ರತಿಜ್ಞಾ(BaseIndent >= First_ಸಾಲು_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_ಸಾಲು_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  ಯಾವತ್ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
ಶೂನ್ಯ alias::printOptionInfo(size_t GlobalWidth) ಧ್ರುವ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(ಧ್ರುವ Option &O) ಧ್ರುವ {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_ಮೌಲ್ಯ_Name();
  ಯದಿ (!ValName.empty()) {
    size_t FormattingLen = 3;
    ಯದಿ (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_ಮೌಲ್ಯ_Str(O, ValName).size() + FormattingLen;
  }

  ಪ್ರತ್ಯಾಗಮ Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ಶೂನ್ಯ basic_parser_impl::printOptionInfo(ಧ್ರುವ Option &O,
                                        size_t GlobalWidth) ಧ್ರುವ {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_ಮೌಲ್ಯ_Name();
  ಯದಿ (!ValName.empty()) {
    ಯದಿ (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_ಮೌಲ್ಯ_Str(O, ValName) << ">...";
    } ಅನ್ಯಥಾ ಯದಿ (O.get_ಮೌಲ್ಯ_Expected_Flag() == ಮೌಲ್ಯ_Optional)
      outs() << "[=<" << get_ಮೌಲ್ಯ_Str(O, ValName) << ">]";
    ಅನ್ಯಥಾ {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_ಮೌಲ್ಯ_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

ಶೂನ್ಯ basic_parser_impl::printOptionName(ಧ್ರುವ Option &O,
                                        size_t GlobalWidth) ಧ್ರುವ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
ತಾರ್ಕಿಕ parser<ತಾರ್ಕಿಕ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ತಾರ್ಕಿಕ &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ parseBool<ತಾರ್ಕಿಕ, ಸತ್ಯ, ಅಸತ್ಯ>(O, ArgName, Arg, ಮೌಲ್ಯ);
}

// parser<boolOrDefault> implementation
//
ತಾರ್ಕಿಕ parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, ಮೌಲ್ಯ);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ಮೌಲ್ಯ_Kind,
                                         FixedOrScalableQuantityT &ಮೌಲ್ಯ) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_ಮೌಲ್ಯ;
  ಯದಿ (!Arg.getAsInteger(0, Min_ಮೌಲ್ಯ)) {
    ಮೌಲ್ಯ = FixedOrScalableQuantityT::getFixed(Min_ಮೌಲ್ಯ);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  StringRef Remainder = Arg;
  ಯದಿ (!Remainder.consume_front("vscale"))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  ಯದಿ (!Remainder.consume_front('x'))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  ಯದಿ (Remainder.getAsInteger(0, Min_ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_Kind +
                   " argument!");

  ಮೌಲ್ಯ = FixedOrScalableQuantityT::getScalable(Min_ಮೌಲ್ಯ);
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<int> implementation
//
ತಾರ್ಕಿಕ parser<ಪೂರ್ಣಾಙ್ಕ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        ಪೂರ್ಣಾಙ್ಕ &ಮೌಲ್ಯ) {
  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for integer argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<long> implementation
//
ತಾರ್ಕಿಕ parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &ಮೌಲ್ಯ) {
  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for long argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<long long> implementation
//
ತಾರ್ಕಿಕ parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &ಮೌಲ್ಯ) {
  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for llong argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<unsigned> implementation
//
ತಾರ್ಕಿಕ parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &ಮೌಲ್ಯ) {

  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for uint argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<unsigned long> implementation
//
ತಾರ್ಕಿಕ parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &ಮೌಲ್ಯ) {

  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for ulong argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<unsigned long long> implementation
//
ತಾರ್ಕಿಕ parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &ಮೌಲ್ಯ) {

  ಯದಿ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for ullong argument!");
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// parser<ElementCount> implementation
//
ತಾರ್ಕಿಕ parser<Element_ಎಣಿಕೆ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ಎಣಿಕೆ &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ parseFixedOrScalableQuantity(O, Arg, get_ಮೌಲ್ಯ_Name(), ಮೌಲ್ಯ);
}

// parser<double>/parser<float> implementation
//
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseDouble(Option &O, StringRef Arg, ದ್ವಿಗುಣ &ಮೌಲ್ಯ) {
  ಯದಿ (to_float(Arg, ಮೌಲ್ಯ))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ O.error("'" + Arg + "' value invalid for floating point argument!");
}

ತಾರ್ಕಿಕ parser<ದ್ವಿಗುಣ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ದ್ವಿಗುಣ &Val) {
  ಪ್ರತ್ಯಾಗಮ parseDouble(O, Arg, Val);
}

ತಾರ್ಕಿಕ parser<ಭಿನ್ನ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ಭಿನ್ನ &Val) {
  ದ್ವಿಗುಣ dVal;
  ಯದಿ (parseDouble(O, Arg, dVal))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  Val = (ಭಿನ್ನ)dVal;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_ಅಂಕೆ_Options();

  ಪ್ರತಿ (unsigned i = 0; i != e; ++i) {
    ಯದಿ (getOption(i) == Name)
      ಪ್ರತ್ಯಾಗಮ i;
  }
  ಪ್ರತ್ಯಾಗಮ e;
}

ಸ್ಥಿರ StringRef Eq_ಮೌಲ್ಯ = "=<value>";
ಸ್ಥಿರ StringRef EmptyOption = "<empty>";
ಸ್ಥಿರ StringRef OptionPrefix = "    =";
ಸ್ಥಿರ size_t getOptionPrefixesSize() {
  ಪ್ರತ್ಯಾಗಮ OptionPrefix.size() + ArgHelpPrefix.size();
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ shouldPrintOption(StringRef Name, StringRef Description,
                              ಧ್ರುವ Option &O) {
  ಪ್ರತ್ಯಾಗಮ O.get_ಮೌಲ್ಯ_Expected_Flag() != ಮೌಲ್ಯ_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ಧ್ರುವ Option &O) ಧ್ರುವ {
  ಯದಿ (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_ಮೌಲ್ಯ.size();
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      ಯದಿ (!shouldPrintOption(Name, getDescription(i), O))
        ಅನುವರ್ತ;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    ಪ್ರತ್ಯಾಗಮ Size;
  } ಅನ್ಯಥಾ {
    size_t BaseSize = 0;
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    ಪ್ರತ್ಯಾಗಮ BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ಶೂನ್ಯ generic_parser_base::printOptionInfo(ಧ್ರುವ Option &O,
                                          size_t GlobalWidth) ಧ್ರುವ {
  ಯದಿ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    ಯದಿ (O.get_ಮೌಲ್ಯ_Expected_Flag() == ಮೌಲ್ಯ_Optional) {
      ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
        ಯದಿ (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          ಭಙ್ಗ;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_ಮೌಲ್ಯ;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_ಮೌಲ್ಯ.size() +
                             argPlusPrefixesSize(O.ArgStr));
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      ಯದಿ (!shouldPrintOption(OptionName, Description, O))
        ಅನುವರ್ತ;
      size_t First_ಸಾಲು_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      ಯದಿ (OptionName.empty()) {
        outs() << EmptyOption;
        ಪ್ರತಿಜ್ಞಾ(First_ಸಾಲು_Indent >= EmptyOption.size());
        First_ಸಾಲು_Indent += EmptyOption.size();
      }
      ಯದಿ (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_ಸಾಲು_Indent);
      ಅನ್ಯಥಾ
        outs() << '\n';
    }
  } ಅನ್ಯಥಾ {
    ಯದಿ (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

ಸ್ಥಿರ ಧ್ರುವ size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
ಶೂನ್ಯ generic_parser_base::printGenericOptionDiff(
    ಧ್ರುವ Option &O, ಧ್ರುವ Generic_Option_ಮೌಲ್ಯ &ಮೌಲ್ಯ,
    ಧ್ರುವ Generic_Option_ಮೌಲ್ಯ &Default, size_t GlobalWidth) ಧ್ರುವ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned ಅಂಕೆ_Opts = get_ಅಂಕೆ_Options();
  ಪ್ರತಿ (unsigned i = 0; i != ಅಂಕೆ_Opts; ++i) {
    ಯದಿ (!ಮೌಲ್ಯ.compare(get_Option_ಮೌಲ್ಯ(i)))
      ಅನುವರ್ತ;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t ಅಂಕೆ_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
    ಪ್ರತಿ (unsigned j = 0; j != ಅಂಕೆ_Opts; ++j) {
      ಯದಿ (!Default.compare(get_Option_ಮೌಲ್ಯ(j)))
        ಅನುವರ್ತ;
      outs() << getOption(j);
      ಭಙ್ಗ;
    }
    outs() << ")\n";
    ಪ್ರತ್ಯಾಗಮ;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ cl {
ಸ್ಥಿರ raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  ಪ್ರತ್ಯಾಗಮ OS << static_cast<ಪೂರ್ಣಾಙ್ಕ>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  ಶೂನ್ಯ parser<T>::printOptionDiff(ಧ್ರುವ Option &O, T V, Option_ಮೌಲ್ಯ<T> D,      \
                                  size_t GlobalWidth) ಧ್ರುವ {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t ಅಂಕೆ_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(ಅಂಕೆ_Spaces) << " (default: ";                                 \
    ಯದಿ (D.has_ಮೌಲ್ಯ())                                                          \
      outs() << D.get_ಮೌಲ್ಯ();                                                  \
    ಅನ್ಯಥಾ                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(ತಾರ್ಕಿಕ)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(ಪೂರ್ಣಾಙ್ಕ)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(ದ್ವಿಗುಣ)
PRINT_OPT_DIFF(ಭಿನ್ನ)
PRINT_OPT_DIFF(ವರ್ಣ)
PRINT_OPT_DIFF(Element_ಎಣಿಕೆ)

ಶೂನ್ಯ parser<std::string>::printOptionDiff(ಧ್ರುವ Option &O, StringRef V,
                                          ಧ್ರುವ Option_ಮೌಲ್ಯ<std::string> &D,
                                          size_t GlobalWidth) ಧ್ರುವ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t ಅಂಕೆ_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
  ಯದಿ (D.has_ಮೌಲ್ಯ())
    outs() << D.get_ಮೌಲ್ಯ();
  ಅನ್ಯಥಾ
    outs() << "*no default*";
  outs() << ")\n";
}

ಶೂನ್ಯ parser<std::optional<std::string>>::printOptionDiff(
    ಧ್ರುವ Option &O, std::optional<StringRef> V,
    ಧ್ರುವ Option_ಮೌಲ್ಯ<std::optional<std::string>> &D,
    size_t GlobalWidth) ಧ್ರುವ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_ಮೌಲ್ಯ_2() ? V.ಮೌಲ್ಯ_2().size() : 0;
  size_t ಅಂಕೆ_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
  ಯದಿ (D.has_ಮೌಲ್ಯ() && D.get_ಮೌಲ್ಯ().has_ಮೌಲ್ಯ_2())
    outs() << D.get_ಮೌಲ್ಯ();
  ಅನ್ಯಥಾ
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
ಶೂನ್ಯ basic_parser_impl::print_Option_No_ಮೌಲ್ಯ(ಧ್ರುವ Option &O,
                                           size_t GlobalWidth) ಧ್ರುವ {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ OptNameCompare(ಧ್ರುವ std::pair<ಧ್ರುವ ವರ್ಣ *, Option *> *LHS,
                          ಧ್ರುವ std::pair<ಧ್ರುವ ವರ್ಣ *, Option *> *RHS) {
  ಪ್ರತ್ಯಾಗಮ strcmp(LHS->first, RHS->first);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SubNameCompare(ಧ್ರುವ std::pair<ಧ್ರುವ ವರ್ಣ *, SubCommand *> *LHS,
                          ಧ್ರುವ std::pair<ಧ್ರುವ ವರ್ಣ *, SubCommand *> *RHS) {
  ಪ್ರತ್ಯಾಗಮ strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
ಸ್ಥಿರ ಶೂನ್ಯ sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ಧ್ರುವ ವರ್ಣ *, Option *>> &Opts,
                     ತಾರ್ಕಿಕ ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  ಪ್ರತಿ (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    ಯದಿ (I->second->getOptionHiddenFlag() == ReallyHidden)
      ಅನುವರ್ತ;

    // Unless showhidden is set, ignore hidden flags.
    ಯದಿ (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ಅನುವರ್ತ;

    // If we've already seen this option, don't add it to the list again.
    ಯದಿ (!OptionSet.ಸೇರಿಸು(I->second).second)
      ಅನುವರ್ತ;

    Opts.ತಳ್ಳು_back(
        std::pair<ಧ್ರುವ ವರ್ಣ *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

ಸ್ಥಿರ ಶೂನ್ಯ
sortSubCommands(ಧ್ರುವ SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ಧ್ರುವ ವರ್ಣ *, SubCommand *>> &Subs) {
  ಪ್ರತಿ (auto *S : SubMap) {
    ಯದಿ (S->getName().empty())
      ಅನುವರ್ತ;
    Subs.ತಳ್ಳು_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ HelpPrinter {
ರಕ್ಷಿತ:
  ಧ್ರುವ ತಾರ್ಕಿಕ ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ಧ್ರುವ ವರ್ಣ *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ಧ್ರುವ ವರ್ಣ *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  ಆಭಾಸ ಶೂನ್ಯ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    ಪ್ರತಿ (ಧ್ರುವ auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  ಶೂನ್ಯ printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    ಪ್ರತಿ (ಧ್ರುವ auto &S : Subs) {
      outs() << "  " << S.first;
      ಯದಿ (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

ಸಾರ್ವಜನಿಕ:
  explicit HelpPrinter(ತಾರ್ಕಿಕ showHidden) : ShowHidden(showHidden) {}
  ಆಭಾಸ ~HelpPrinter() = ಸ್ವಭಾವ;

  // Invoke the printer.
  ಶೂನ್ಯ operator=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
    ಯದಿ (!ಮೌಲ್ಯ)
      ಪ್ರತ್ಯಾಗಮ;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  ಶೂನ್ಯ printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    ಯದಿ (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    ಯದಿ (Sub == &SubCommand::get_ತುದಿ_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      ಯದಿ (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } ಅನ್ಯಥಾ {
      ಯದಿ (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    ಪ್ರತಿ (auto *Opt : PositionalOpts) {
      ಯದಿ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    ಯದಿ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    ಯದಿ (Sub == &SubCommand::get_ತುದಿ_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      ಪ್ರತಿ (ಧ್ರುವ auto &Sub : Subs)
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
    ಪ್ರತಿ (ಧ್ರುವ auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    ಯದಿ (ShowHidden)
      ಪ್ರತಿ (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              ಯದಿ (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    ಪ್ರತಿ (ಧ್ರುವ auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    ಯದಿ (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    ಪ್ರತಿ (ಧ್ರುವ auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    ಪ್ರತಿ (ಧ್ರುವ auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

ವರ್ಗ CategorizedHelpPrinter : ಸಾರ್ವಜನಿಕ HelpPrinter {
ಸಾರ್ವಜನಿಕ:
  explicit CategorizedHelpPrinter(ತಾರ್ಕಿಕ showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ OptionCategoryCompare(OptionCategory *ಧ್ರುವ *A,
                                   OptionCategory *ಧ್ರುವ *B) {
    ಪ್ರತ್ಯಾಗಮ (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

ರಕ್ಷಿತ:
  ಶೂನ್ಯ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ಪ್ರತಿಜ್ಞಾ(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    ಪ್ರತಿ (ಧ್ರುವ auto &I : Opts) {
      Option *Opt = I.second;
      ಪ್ರತಿ (OptionCategory *Cat : Opt->Categories) {
        ಪ್ರತಿಜ್ಞಾ(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ತಳ್ಳು_back(Opt);
      }
    }

    // Now do printing.
    ಪ್ರತಿ (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ಧ್ರುವ auto &CategoryOptions = CategorizedOptions[Category];
      ಯದಿ (CategoryOptions.empty())
        ಅನುವರ್ತ;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      ಯದಿ (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      ಅನ್ಯಥಾ
        outs() << "\n";

      // Loop over the options in the category and print.
      ಪ್ರತಿ (ಧ್ರುವ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
ವರ್ಗ HelpPrinterWrapper {
ಗುಪ್ತ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

ಸಾರ್ವಜನಿಕ:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  ಶೂನ್ಯ operator=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ);
};

} // End anonymous namespace

#ಯದಿ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# ಯದಿ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# ಅನ್ಯಥಾ
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# ಯದಿ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# ಅನ್ಯಥಾ
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#ಅನ್ಯಥಾ
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

ನಾಮಾಕಾಶ {
ವರ್ಗ VersionPrinter {
ಸಾರ್ವಜನಿಕ:
  ಶೂನ್ಯ print(ಧ್ರುವ std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#ಅನ್ಯಥಾ
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#ಯದಿ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#ಅನ್ಯಥಾ
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    ಯದಿ (!ExtraPrinters.empty()) {
      ಪ್ರತಿ (ಧ್ರುವ auto &I : ExtraPrinters)
        I(outs());
    }
  }
  ಶೂನ್ಯ operator=(ತಾರ್ಕಿಕ OptionWasSpecified);
};

ಸಂರಚನಾ Command_ಸಾಲು_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{ಅಸತ್ಯ};
  HelpPrinter UncategorizedHiddenPrinter{ಸತ್ಯ};
  CategorizedHelpPrinter CategorizedNormalPrinter{ಅಸತ್ಯ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{ಸತ್ಯ};
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
  cl::opt<HelpPrinter, ಸತ್ಯ, parser<ತಾರ್ಕಿಕ>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, ಸತ್ಯ, parser<ತಾರ್ಕಿಕ>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, ಸತ್ಯ, parser<ತಾರ್ಕಿಕ>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, ಸತ್ಯ, parser<ತಾರ್ಕಿಕ>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ತಾರ್ಕಿಕ> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(ಅಸತ್ಯ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ತಾರ್ಕಿಕ> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(ಅಸತ್ಯ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, ಸತ್ಯ, parser<ತಾರ್ಕಿಕ>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ಸ್ಥಿರ ManagedStatic<Command_ಸಾಲು_Common_Options> CommonOptions;

ಸ್ಥಿರ ಶೂನ್ಯ initCommonOptions() {
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
  ಸ್ಥಿರ OptionCategory GeneralCategory{"General options"};
  ಪ್ರತ್ಯಾಗಮ GeneralCategory;
}

ಶೂನ್ಯ VersionPrinter::operator=(ತಾರ್ಕಿಕ OptionWasSpecified) {
  ಯದಿ (!OptionWasSpecified)
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

ಶೂನ್ಯ HelpPrinterWrapper::operator=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
  ಯದಿ (!ಮೌಲ್ಯ)
    ಪ್ರತ್ಯಾಗಮ;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  ಯದಿ (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = ಸತ್ಯ; // Invoke categorized printer
  } ಅನ್ಯಥಾ {
    UncategorizedPrinter = ಸತ್ಯ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
ಶೂನ್ಯ cl::Print_Option_ಮೌಲ್ಯಗಳು() { globalParser().print_Option_ಮೌಲ್ಯಗಳು(); }

ಶೂನ್ಯ Command_ಸಾಲು_Parser::print_Option_ಮೌಲ್ಯಗಳು() {
  ಯದಿ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ಪ್ರತ್ಯಾಗಮ;

  SmallVector<std::pair<ಧ್ರುವ ವರ್ಣ *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ ಸತ್ಯ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  ಪ್ರತಿ (ಧ್ರುವ auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  ಪ್ರತಿ (ಧ್ರುವ auto &Opt : Opts)
    Opt.second->print_Option_ಮೌಲ್ಯ(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
ಶೂನ್ಯ cl::PrintHelpMessage(ತಾರ್ಕಿಕ Hidden, ತಾರ್ಕಿಕ Categorized) {
  ಯದಿ (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  ಅನ್ಯಥಾ ಯದಿ (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  ಅನ್ಯಥಾ ಯದಿ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  ಅನ್ಯಥಾ
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ಸ್ಥಿರ ಧ್ರುವ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#ಯದಿ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#ಯದಿ __has_feature(address_sanitizer)
      "+asan",
#endif
#ಯದಿ __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#ಯದಿ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#ಯದಿ __has_feature(memory_sanitizer)
      "+msan",
#endif
#ಯದಿ __has_feature(thread_sanitizer)
      "+tsan",
#endif
#ಯದಿ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  ಪ್ರತ್ಯಾಗಮ ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
ಶೂನ್ಯ cl::printBuildConfig(raw_ostream &OS) {
#ಯದಿ LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
ಶೂನ್ಯ cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

ಶೂನ್ಯ cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

ಶೂನ್ಯ cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.ತಳ್ಳು_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (ಶೂನ್ಯ)Subs;
  ಪ್ರತಿಜ್ಞಾ(Subs.contains(&Sub));
  ಪ್ರತ್ಯಾಗಮ Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ಪ್ರತ್ಯಾಗಮ globalParser().getRegisteredSubcommands();
}

ಶೂನ್ಯ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  ಪ್ರತಿ (auto &I : Sub.OptionsMap) {
    ತಾರ್ಕಿಕ Unrelated = ಸತ್ಯ;
    ಪ್ರತಿ (auto &Cat : I.second->Categories) {
      ಯದಿ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = ಅಸತ್ಯ;
    }
    ಯದಿ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ಶೂನ್ಯ cl::HideUnrelatedOptions(ArrayRef<ಧ್ರುವ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  ಪ್ರತಿ (auto &I : Sub.OptionsMap) {
    ತಾರ್ಕಿಕ Unrelated = ಸತ್ಯ;
    ಪ್ರತಿ (auto &Cat : I.second->Categories) {
      ಯದಿ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = ಅಸತ್ಯ;
    }
    ಯದಿ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ಶೂನ್ಯ cl::Reset_Command_ಸಾಲು_Parser() { globalParser().reset(); }

ಶೂನ್ಯ cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ತಳ್ಳು_back(&L);
}

ಶೂನ್ಯ LibraryOptionsContext::assign(ಧ್ರುವ LibraryOptionsContext &Other) {
  ಪ್ರತಿಜ್ಞಾ(ಏತತ್ != &Other);
  ಮೌಲ್ಯಗಳು.clear();
  SavedArgs.clear();
  ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args = Other.SavedArgs;
  ಪ್ರತಿ (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = ಮೌಲ್ಯಗಳು.try_emplace(L.getDefault());
  ಯದಿ (Inserted)
    It->second = L.copyDefault(Alloc);
  ಪ್ರತ್ಯಾಗಮ parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

ಶೂನ್ಯ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

ಶೂನ್ಯ LLVMParse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
                                 ಧ್ರುವ ವರ್ಣ *Overview) {
  llvm::cl::Parse_Command_ಸಾಲು_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
