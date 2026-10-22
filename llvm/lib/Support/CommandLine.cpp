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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm-c/Support.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLFunctionalExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallPtrSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LibraryOptionsContext.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/StringSaver.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/TypeSize.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualFileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cstdlib>
#ଅନ୍ତର୍ଭାଵ <optional>
#ଅନ୍ତର୍ଭାଵ <string>
using ନାମାକାଶ llvm;
using ନାମାକାଶ cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
ନାମାକାଶ llvm {
ନାମାକାଶ cl {
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<ତାର୍କିକ>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<long>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<ଦ୍ୱିଗୁଣ>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<ଦଶମିକ>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<ଅକ୍ଷର>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE basic_parser<Element_ଗଣନା>;

#ଯଦି !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE opt<ତାର୍କିକ>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE opt<ଅକ୍ଷର>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE opt<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>;
template ଶ୍ରେଣୀ LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
ଶୂନ୍ୟ Generic_Option_ମୂଲ୍ୟ::anchor() {}
ଶୂନ୍ୟ Option_ମୂଲ୍ୟ<boolOrDefault>::anchor() {}
ଶୂନ୍ୟ Option_ମୂଲ୍ୟ<std::string>::anchor() {}
ଶୂନ୍ୟ Option::anchor() {}
ଶୂନ୍ୟ basic_parser_impl::anchor() {}
ଶୂନ୍ୟ parser<ତାର୍କିକ>::anchor() {}
ଶୂନ୍ୟ parser<boolOrDefault>::anchor() {}
ଶୂନ୍ୟ parser<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>::anchor() {}
ଶୂନ୍ୟ parser<long>::anchor() {}
ଶୂନ୍ୟ parser<long long>::anchor() {}
ଶୂନ୍ୟ parser<unsigned>::anchor() {}
ଶୂନ୍ୟ parser<unsigned long>::anchor() {}
ଶୂନ୍ୟ parser<unsigned long long>::anchor() {}
ଶୂନ୍ୟ parser<ଦ୍ୱିଗୁଣ>::anchor() {}
ଶୂନ୍ୟ parser<ଦଶମିକ>::anchor() {}
ଶୂନ୍ୟ parser<std::string>::anchor() {}
ଶୂନ୍ୟ parser<std::optional<std::string>>::anchor() {}
ଶୂନ୍ୟ parser<ଅକ୍ଷର>::anchor() {}
ଶୂନ୍ୟ parser<Element_ଗଣନା>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
ଶୂନ୍ୟ opt_bool_anchor() { opt<ତାର୍କିକ> anchor{""}; }
ଶୂନ୍ୟ opt_char_anchor() { opt<ଅକ୍ଷର> anchor{""}; }
ଶୂନ୍ୟ opt_int_anchor() { opt<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> anchor{""}; }
ଶୂନ୍ୟ opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

ଅଚଳ ସ୍ଥିର size_t DefaultPad = 2;

ସ୍ଥିର StringRef ArgPrefix = "-";
ସ୍ଥିର StringRef ArgPrefixLong = "--";
ସ୍ଥିର StringRef ArgHelpPrefix = " - ";

ସ୍ଥିର size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  ଯଦି (Len == 1)
    ଫେରନ୍ତୁ Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  ଫେରନ୍ତୁ Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

ସ୍ଥିର SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  ପାଇଁ (size_t I = 0; I < Pad; ++I) {
    Prefix.ଠେଲ_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  ଫେରନ୍ତୁ Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

ନାମାକାଶ {

ଶ୍ରେଣୀ PrintArg {
  StringRef ArgName;
  size_t Pad;
ସର୍ବସାଧାରଣ:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, ଅଚଳ PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, ଅଚଳ PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ଫେରନ୍ତୁ OS;
}

ଶ୍ରେଣୀ Command_ଧାଡ଼ି_Parser {
ସର୍ବସାଧାରଣ:
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
  size_t ଅଙ୍କ_Indexed_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ଧାଡ଼ି_Parser() { registerSubCommand(&SubCommand::get_ଶୀର୍ଷ_Level()); }

  ଶୂନ୍ୟ ResetAllOptionOccurrences();

  ତାର୍କିକ Parse_Command_ଧାଡ଼ି_Options(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  ଶୂନ୍ୟ forEachSubCommand(Option &Opt, function_ref<ଶୂନ୍ୟ(SubCommand &)> Action) {
    ଯଦି (Opt.Subs.empty()) {
      Action(SubCommand::get_ଶୀର୍ଷ_Level());
      ଫେରନ୍ତୁ;
    }
    ଯଦି (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      ପାଇଁ (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ଫେରନ୍ତୁ;
    }
    ପାଇଁ (auto *SC : Opt.Subs) {
      ପ୍ରତିଜ୍ଞା(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  ଶୂନ୍ୟ addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    ଯଦି (Opt.hasArgStr())
      ଫେରନ୍ତୁ;
    ଯଦି (!SC->OptionsMap.ଭର୍ତ୍ତି(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  ଶୂନ୍ୟ addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  ଶୂନ୍ୟ addOption(Option *O, SubCommand *SC) {
    ତାର୍କିକ HadErrors = ମିଥ୍ୟା;
    ଯଦି (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      ଯଦି (!SC->OptionsMap.ଭର୍ତ୍ତି(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = ସତ୍ୟ;
      }
    }

    // Remember information about positional options.
    ଯଦି (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ଠେଲ_back(O);
    ଅନ୍ୟଥା ଯଦି (O->get_ଅଙ୍କ_Occurrences_Flag() == cl::ConsumeAfter) {
      ଯଦି (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = ସତ୍ୟ;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    ଯଦି (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  ଶୂନ୍ୟ addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  ଶୂନ୍ୟ removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    ଯଦି (O->hasArgStr())
      OptionNames.ଠେଲ_back(O->ArgStr);

    SubCommand &Sub = *SC;
    ପାଇଁ (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      ଯଦି (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    ଯଦି (O->getFormattingFlag() == cl::Positional)
      ପାଇଁ (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        ଯଦି (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ଭାଙ୍ଗ;
        }
      }
    ଅନ୍ୟଥା ଯଦି (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  ଶୂନ୍ୟ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  ତାର୍କିକ hasOptions(ଅଚଳ SubCommand &Sub) ଅଚଳ {
    ଫେରନ୍ତୁ (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  ତାର୍କିକ hasOptions() ଅଚଳ {
    ପାଇଁ (ଅଚଳ auto *S : RegisteredSubCommands) {
      ଯଦି (hasOptions(*S))
        ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ଫେରନ୍ତୁ !Libraries.empty();
  }

  ତାର୍କିକ hasNamedSubCommands() ଅଚଳ {
    ପାଇଁ (ଅଚଳ auto *S : RegisteredSubCommands)
      ଯଦି (!S->getName().empty())
        ଫେରନ୍ତୁ ସତ୍ୟ;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  SubCommand *getActiveSubCommand() { ଫେରନ୍ତୁ ActiveSubCommand; }

  ଶୂନ୍ୟ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    ଯଦି (!Sub.OptionsMap.ଭର୍ତ୍ତି(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  ଶୂନ୍ୟ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  ଶୂନ୍ୟ print_Option_ମୂଲ୍ୟଗୁଡ଼ିକ();

  ଶୂନ୍ୟ indexLibraryOptions() {
    ତାର୍କିକ HadErrors = ମିଥ୍ୟା;
    ପାଇଁ (; ଅଙ୍କ_Indexed_Libraries != Libraries.size(); ++ଅଙ୍କ_Indexed_Libraries) {
      LibraryOptions *L = Libraries[ଅଙ୍କ_Indexed_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        ଯଦି (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ଫେରନ୍ତୁ SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          ଫେରନ୍ତୁ;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = ସତ୍ୟ;
      });
    }
    ଯଦି (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    ଫେରନ୍ତୁ LibraryIndex.lookup(Name);
  }

  ଶୂନ୍ୟ registerCategory(OptionCategory *cat) {
    ପ୍ରତିଜ୍ଞା(ଗଣନା_if(RegisteredOptionCategories,
                    [cat](ଅଚଳ OptionCategory *Category) {
             ଫେରନ୍ତୁ cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ଭର୍ତ୍ତି(cat);
  }

  ଶୂନ୍ୟ registerSubCommand(SubCommand *sub) {
    ପ୍ରତିଜ୍ଞା(ଗଣନା_if(RegisteredSubCommands,
                    [sub](ଅଚଳ SubCommand *Sub) {
                      ଫେରନ୍ତୁ (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ଭର୍ତ୍ତି(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ପ୍ରତିଜ୍ଞା(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    ପାଇଁ (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      ଯଦି (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      ଅନ୍ୟଥା
        addLiteralOption(*O, sub, E.first);
    }
  }

  ଶୂନ୍ୟ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ଫେରନ୍ତୁ make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  ଶୂନ୍ୟ reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    ଅଙ୍କ_Indexed_Libraries = 0;

    SubCommand::get_ଶୀର୍ଷ_Level().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::get_ଶୀର୍ଷ_Level());
  }

ବ୍ୟକ୍ତିଗତ:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &ମୂଲ୍ୟ);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ସ୍ଥିର Command_ଧାଡ଼ି_Parser &globalParser() {
  ସ୍ଥିର ManagedStatic<Command_ଧାଡ଼ି_Parser> GlobalParser;
  ଫେରନ୍ତୁ *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ସ୍ଥିର Error
parseLibraryOption(LibraryOptions &L, ଶୂନ୍ୟ *Opts, ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  ଅଚଳ ଅକ୍ଷର *Saved[2];
  unsigned ଅଙ୍କ_Args = std::min<size_t>(Args.size(), 2);
  ପାଇଁ (unsigned J = 0; J != ଅଙ୍କ_Args; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  ଯଦି (Error E = L.parse(Opts, ArrayRef(Saved, ଅଙ୍କ_Args), Consumed, Alloc))
    ଫେରନ୍ତୁ E;
  ଯଦି (Record)
    Record->append(Saved, Saved + Consumed);
  ଫେରନ୍ତୁ Error::success();
}

template <typename T, T TrueVal, T FalseVal>
ସ୍ଥିର ତାର୍କିକ parseBool(Option &O, StringRef ArgName, StringRef Arg, T &ମୂଲ୍ୟ) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  ଯଦି (!Arg.data() || Arg == "true" || Arg == "1") {
    ମୂଲ୍ୟ = TrueVal;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  ଯଦି (Arg == "false" || Arg == "0") {
    ମୂଲ୍ୟ = FalseVal;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

ଶୂନ୍ୟ cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.ଠେଲ_back(Help);
}

Option::Option(ଅଙ୍କ_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : ଅଙ୍କ_Occurrences(0), Occurrences(OccurrencesFlag), ମୂଲ୍ୟ(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(ମିଥ୍ୟା), Position(0) {
  Categories.ଠେଲ_back(&getGeneralCategory());
}

ଶୂନ୍ୟ Option::addArgument() {
  globalParser().addOption(ଏହି);
  FullyInitialized = ସତ୍ୟ;
}

ଶୂନ୍ୟ Option::removeArgument() { globalParser().removeOption(ଏହି); }

ଶୂନ୍ୟ Option::setArgStr(StringRef S) {
  ଯଦି (FullyInitialized)
    globalParser().updateArgStr(ଏହି, S);
  ପ୍ରତିଜ୍ଞା(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

ଶୂନ୍ୟ Option::addCategory(OptionCategory &C) {
  ପ୍ରତିଜ୍ଞା(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  ଯଦି (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  ଅନ୍ୟଥା ଯଦି (!is_contained(Categories, &C))
    Categories.ଠେଲ_back(&C);
}

ଶୂନ୍ୟ Option::reset() {
  ଅଙ୍କ_Occurrences = 0;
  setDefault();
}

ଶୂନ୍ୟ OptionCategory::registerCategory() {
  globalParser().registerCategory(ଏହି);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_ଶୀର୍ଷ_Level() {
  ସ୍ଥିର ManagedStatic<SubCommand> ଶୀର୍ଷ_Level_Sub_Command;
  ଫେରନ୍ତୁ *ଶୀର୍ଷ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ସ୍ଥିର ManagedStatic<SubCommand> AllSubCommands;
  ଫେରନ୍ତୁ *AllSubCommands;
}

ଶୂନ୍ୟ SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(ଏହି);
}

ଶୂନ୍ୟ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ଏହି);
}

ଶୂନ୍ୟ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator ତାର୍କିକ() ଅଚଳ {
  ଫେରନ୍ତୁ (globalParser().getActiveSubCommand() == ଏହି);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_ଧାଡ଼ି_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &ମୂଲ୍ୟ) {
  // Reject all dashes.
  ଯଦି (Arg.empty())
    ଫେରନ୍ତୁ nullptr;
  ପ୍ରତିଜ୍ଞା(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  ଯଦି (EqualPos == StringRef::npos) {
    // Look up the option.
    ଫେରନ୍ତୁ Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  ଯଦି (I == Sub.OptionsMap.end())
    ଫେରନ୍ତୁ nullptr;

  auto *O = I->second;
  ଯଦି (O->getFormattingFlag() == cl::AlwaysPrefix)
    ଫେରନ୍ତୁ nullptr;

  ମୂଲ୍ୟ = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ଫେରନ୍ତୁ I->second;
}

SubCommand *Command_ଧାଡ଼ି_Parser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  ଯଦି (Name.empty())
    ଫେରନ୍ତୁ &SubCommand::get_ଶୀର୍ଷ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  ପାଇଁ (auto *S : RegisteredSubCommands) {
    ପ୍ରତିଜ୍ଞା(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    ଯଦି (S->getName().empty())
      ଜାରି;

    ଯଦି (S->getName() == Name)
      ଫେରନ୍ତୁ S;

    ଯଦି (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  ଯଦି (NearestMatch)
    NearestString = NearestMatch->getName();

  ଫେରନ୍ତୁ &SubCommand::get_ଶୀର୍ଷ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ସ୍ଥିର Option *LookupNearestOption(StringRef Arg,
                                   ଅଚଳ OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  ଯଦି (Arg.empty())
    ଫେରନ୍ତୁ nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  ପାଇଁ (ଅଚଳ auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    ଯଦି (O->getOptionHiddenFlag() == ReallyHidden)
      ଜାରି;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    ଯଦି (O->hasArgStr())
      OptionNames.ଠେଲ_back(O->ArgStr);

    ତାର୍କିକ Permit_ମୂଲ୍ୟ = O->get_ମୂଲ୍ୟ_Expected_Flag() != cl::ମୂଲ୍ୟ_Disallowed;
    StringRef Flag = Permit_ମୂଲ୍ୟ ? LHS : Arg;
    ପାଇଁ (ଅଚଳ auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/ସତ୍ୟ, /*MaxEditDistance=*/BestDistance);
      ଯଦି (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        ଯଦି (RHS.empty() || !Permit_ମୂଲ୍ୟ)
          NearestString = std::string(Name);
        ଅନ୍ୟଥା
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ଫେରନ୍ତୁ Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ସ୍ଥିର ତାର୍କିକ CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef ମୂଲ୍ୟ) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  ଯଦି (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(ମୂଲ୍ୟ);
    StringRef::size_type Pos = Val.find(',');

    ଯେତେବେଳେ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      ଯଦି (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        ଫେରନ୍ତୁ ସତ୍ୟ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    ମୂଲ୍ୟ = Val;
  }

  ଫେରନ୍ତୁ Handler->addOccurrence(pos, ArgName, ମୂଲ୍ୟ);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ତାର୍କିକ ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef ମୂଲ୍ୟ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
                                 ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &i) {
  // Enforce value requirements
  ବାଛ (Handler->get_ମୂଲ୍ୟ_Expected_Flag()) {
  ସ୍ଥିତି ମୂଲ୍ୟ_Required:
    ଯଦି (!ମୂଲ୍ୟ.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      ଯଦି (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        ଫେରନ୍ତୁ Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ପ୍ରତିଜ୍ଞା(argv && "null check");
      ମୂଲ୍ୟ = StringRef(argv[++i]);
    }
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ମୂଲ୍ୟ_Disallowed:
    ଯଦି (ମୂଲ୍ୟ.data())
      ଫେରନ୍ତୁ Handler->error("does not allow a value! '" + Twine(ମୂଲ୍ୟ) +
                            "' specified.");
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ମୂଲ୍ୟ_Optional:
    ଭାଙ୍ଗ;
  }

  ଫେରନ୍ତୁ CommaSeparateAndAddOccurrence(Handler, i, ArgName, ମୂଲ୍ୟ);
}

ତାର୍କିକ llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Dummy = i;
  ଫେରନ୍ତୁ ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ସ୍ଥିର Option *findPrefixOption(StringRef Name, size_t &Length,
                                ଅଚଳ OptionsMapTy &OptionsMap) {
  ପାଇଁ (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    ଯଦି (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      ଫେରନ୍ତୁ O;
    }
  }
  ଫେରନ୍ତୁ nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ସ୍ଥିର Option *HandlePrefixedOption(StringRef &Arg, StringRef &ମୂଲ୍ୟ,
                                    ଅଚଳ OptionsMapTy &OptionsMap) {
  ଯଦି (Arg.size() == 1)
    ଫେରନ୍ତୁ nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  ଯଦି (!POpt)
    ଫେରନ୍ତୁ nullptr;

  StringRef Maybe_ମୂଲ୍ୟ =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  ଯଦି (POpt->getFormattingFlag() == cl::Prefix && Maybe_ମୂଲ୍ୟ.starts_with("="))
    Maybe_ମୂଲ୍ୟ = Maybe_ମୂଲ୍ୟ.drop_front();
  ମୂଲ୍ୟ = Maybe_ମୂଲ୍ୟ;
  ଫେରନ୍ତୁ POpt;
}

ସ୍ଥିର ତାର୍କିକ Requires_ମୂଲ୍ୟ(ଅଚଳ Option *O) {
  ଫେରନ୍ତୁ O->get_ଅଙ୍କ_Occurrences_Flag() == cl::Required ||
         O->get_ଅଙ୍କ_Occurrences_Flag() == cl::OneOrMore;
}

ସ୍ଥିର ତାର୍କିକ Eats_Unbounded_ସଂଖ୍ୟା_Of_ମୂଲ୍ୟଗୁଡ଼ିକ(ଅଚଳ Option *O) {
  ଫେରନ୍ତୁ O->get_ଅଙ୍କ_Occurrences_Flag() == cl::ZeroOrMore ||
         O->get_ଅଙ୍କ_Occurrences_Flag() == cl::OneOrMore;
}

ସ୍ଥିର ତାର୍କିକ isWhitespace(ଅକ୍ଷର C) {
  ଫେରନ୍ତୁ C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ସ୍ଥିର ତାର୍କିକ isWhitespaceOrNull(ଅକ୍ଷର C) {
  ଫେରନ୍ତୁ isWhitespace(C) || C == '\0';
}

ସ୍ଥିର ତାର୍କିକ isQuote(ଅକ୍ଷର C) { ଫେରନ୍ତୁ C == '\"' || C == '\''; }

ଶୂନ୍ୟ cl::Tokenize_GNUCommand_ଧାଡ଼ି(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv,
                                ତାର୍କିକ MarkEOLs) {
  SmallString<128> Token;
  ତାର୍କିକ InToken = ମିଥ୍ୟା;
  ପାଇଁ (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    ଯଦି (!InToken) {
      ଯେତେବେଳେ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        ଯଦି (MarkEOLs && Src[I] == '\n')
          NewArgv.ଠେଲ_back(nullptr);
        ++I;
      }
      ଯଦି (I == E)
        ଭାଙ୍ଗ;
      InToken = ସତ୍ୟ;
    }

    ଅକ୍ଷର C = Src[I];

    // Backslash escapes the next character.
    ଯଦି (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ଠେଲ_back(Src[I]);
      ଜାରି;
    }

    // Consume a quoted string.
    ଯଦି (isQuote(C)) {
      ++I;
      ଯେତେବେଳେ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        ଯଦି (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ଠେଲ_back(Src[I]);
        ++I;
      }
      ଯଦି (I == E)
        ଭାଙ୍ଗ;
      ଜାରି;
    }

    // End the token if this is whitespace.
    ଯଦି (isWhitespace(C)) {
      NewArgv.ଠେଲ_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      ଯଦି (MarkEOLs && C == '\n')
        NewArgv.ଠେଲ_back(nullptr);
      Token.clear();
      InToken = ମିଥ୍ୟା;
      ଜାରି;
    }

    // This is a normal character.  Append it.
    Token.ଠେଲ_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  ଯଦି (InToken)
    NewArgv.ଠେଲ_back(Saver.save(Token.str()).data());
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
ସ୍ଥିର size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Backslash_ଗଣନା = 0;
  // Skip the backslashes.
  କର {
    ++I;
    ++Backslash_ଗଣନା;
  } ଯେତେବେଳେ (I != E && Src[I] == '\\');

  ତାର୍କିକ FollowedByDoubleQuote = (I != E && Src[I] == '"');
  ଯଦି (FollowedByDoubleQuote) {
    Token.append(Backslash_ଗଣନା / 2, '\\');
    ଯଦି (Backslash_ଗଣନା % 2 == 0)
      ଫେରନ୍ତୁ I - 1;
    Token.ଠେଲ_back('"');
    ଫେରନ୍ତୁ I;
  }
  Token.append(Backslash_ଗଣନା, '\\');
  ଫେରନ୍ତୁ I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ସ୍ଥିର ତାର୍କିକ isWindowsSpecialChar(ଅକ୍ଷର C) {
  ଫେରନ୍ତୁ isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ସ୍ଥିର ତାର୍କିକ isWindowsSpecialCharInCommandName(ଅକ୍ଷର C) {
  ଫେରନ୍ତୁ isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ tokenize_Windows_Command_ଧାଡ଼ି_Impl(
    StringRef Src, StringSaver &Saver, function_ref<ଶୂନ୍ୟ(StringRef)> AddToken,
    ତାର୍କିକ AlwaysCopy, function_ref<ଶୂନ୍ୟ()> MarkEOL, ତାର୍କିକ InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  ତାର୍କିକ CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  ଗଣନା { INIT, UNQUOTED, QUOTED } State = INIT;

  ପାଇଁ (size_t I = 0, E = Src.size(); I < E; ++I) {
    ବାଛ (State) {
    ସ୍ଥିତି INIT: {
      ପ୍ରତିଜ୍ଞା(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      ଯେତେବେଳେ (I < E && isWhitespaceOrNull(Src[I])) {
        ଯଦି (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      ଯଦି (I >= E)
        ଭାଙ୍ଗ;
      size_t Start = I;
      ଯଦି (CommandName) {
        ଯେତେବେଳେ (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } ଅନ୍ୟଥା {
        ଯେତେବେଳେ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      ଯଦି (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        ଯଦି (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } ଅନ୍ୟଥା {
          CommandName = ମିଥ୍ୟା;
        }
      } ଅନ୍ୟଥା ଯଦି (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } ଅନ୍ୟଥା ଯଦି (Src[I] == '\\') {
        ପ୍ରତିଜ୍ଞା(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } ଅନ୍ୟଥା {
        llvm_unreachable("unexpected special character");
      }
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି UNQUOTED:
      ଯଦି (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        ଯଦି (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } ଅନ୍ୟଥା {
          CommandName = ମିଥ୍ୟା;
        }
        State = INIT;
      } ଅନ୍ୟଥା ଯଦି (Src[I] == '\"') {
        State = QUOTED;
      } ଅନ୍ୟଥା ଯଦି (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ଅନ୍ୟଥା {
        Token.ଠେଲ_back(Src[I]);
      }
      ଭାଙ୍ଗ;

    ସ୍ଥିତି QUOTED:
      ଯଦି (Src[I] == '\"') {
        ଯଦି (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ଠେଲ_back('"');
          ++I;
        } ଅନ୍ୟଥା {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } ଅନ୍ୟଥା ଯଦି (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ଅନ୍ୟଥା {
        Token.ଠେଲ_back(Src[I]);
      }
      ଭାଙ୍ଗ;
    }
  }

  ଯଦି (State != INIT)
    AddToken(Saver.save(Token.str()));
}

ଶୂନ୍ୟ cl::Tokenize_Windows_Command_ଧାଡ଼ି(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv,
                                    ତାର୍କିକ MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ଠେଲ_back(Tok.data()); };
  auto OnEOL = [&]() {
    ଯଦି (MarkEOLs)
      NewArgv.ଠେଲ_back(nullptr);
  };
  tokenize_Windows_Command_ଧାଡ଼ି_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/ସତ୍ୟ, OnEOL, ମିଥ୍ୟା);
}

ଶୂନ୍ୟ cl::Tokenize_Windows_Command_ଧାଡ଼ି_No_Copy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ଠେଲ_back(Tok); };
  auto OnEOL = []() {};
  tokenize_Windows_Command_ଧାଡ଼ି_Impl(Src, Saver, AddToken, /*AlwaysCopy=*/ମିଥ୍ୟା,
                                 OnEOL, ମିଥ୍ୟା);
}

ଶୂନ୍ୟ cl::Tokenize_Windows_Command_ଧାଡ଼ି_Full(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv,
                                        ତାର୍କିକ MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.ଠେଲ_back(Tok.data()); };
  auto OnEOL = [&]() {
    ଯଦି (MarkEOLs)
      NewArgv.ଠେଲ_back(nullptr);
  };
  tokenize_Windows_Command_ଧାଡ଼ି_Impl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/ସତ୍ୟ, OnEOL, ସତ୍ୟ);
}

ଶୂନ୍ୟ cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv,
                            ତାର୍କିକ MarkEOLs) {
  ପାଇଁ (ଅଚଳ ଅକ୍ଷର *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> ଧାଡ଼ି;
    // Check for comment line.
    ଯଦି (isWhitespace(*Cur)) {
      ଯେତେବେଳେ (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      ଜାରି;
    }
    ଯଦି (*Cur == '#') {
      ଯେତେବେଳେ (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      ଜାରି;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ଅଚଳ ଅକ୍ଷର *Start = Cur;
    ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି_End = nullptr;
    ଅକ୍ଷର Quote = 0;
    ତାର୍କିକ AtTokenStart = ସତ୍ୟ;
    ପାଇଁ (ଅଚଳ ଅକ୍ଷର *End = Source.end(); Cur != End; ++Cur) {
      ଅକ୍ଷର C = *Cur;
      ଯଦି (C == '\\') {
        ଯଦି (Cur + 1 != End) {
          ++Cur;
          ଯଦି (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            ଧାଡ଼ି.append(Start, Cur - 1);
            ଯଦି (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            ଜାରି;
          }
        }
        AtTokenStart = ମିଥ୍ୟା;
        ଜାରି;
      }
      ଯଦି (C == '\n')
        ଭାଙ୍ଗ;
      ଯଦି (Quote) {
        ଯଦି (C == Quote)
          Quote = 0;
      } ଅନ୍ୟଥା ଯଦି (isQuote(C)) {
        Quote = C;
      } ଅନ୍ୟଥା ଯଦି (C == '#' && AtTokenStart) {
        ଧାଡ଼ି_End = Cur;
        ଯେତେବେଳେ (Cur != End && *Cur != '\n')
          ++Cur;
        ଭାଙ୍ଗ;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    ଧାଡ଼ି.append(Start, ଧାଡ଼ି_End ? ଧାଡ଼ି_End : Cur);
    cl::Tokenize_GNUCommand_ଧାଡ଼ି(ଧାଡ଼ି, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ସ୍ଥିର ତାର୍କିକ hasUTF8ByteOrderMark(ArrayRef<ଅକ୍ଷର> S) {
  ଫେରନ୍ତୁ (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ସ୍ଥିର ଶୂନ୍ୟ ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            ଅଚଳ ଅକ୍ଷର *&Arg) {
  ପ୍ରତିଜ୍ଞା(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  ଅଚଳ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  ପାଇଁ (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ଅଚଳ StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    ଯଦି (ResponseFile.empty())
      ResponseFile = LHS;
    ଅନ୍ୟଥା
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  ଯଦି (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ଅଚଳ StringRef Remaining = ArgString.substr(StartPos);
    ଯଦି (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv) {
  ପ୍ରତିଜ୍ଞା(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  ଯଦି (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    ଫେରନ୍ତୁ llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<ଅକ୍ଷର> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  ଯଦି (hasUTF16ByteOrderMark(BufRef)) {
    ଯଦି (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      ଫେରନ୍ତୁ llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  ଅନ୍ୟଥା ଯଦି (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  ଯଦି (!RelativeNames && !InConfigFile)
    ଫେରନ୍ତୁ Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  ପାଇଁ (ଅଚଳ ଅକ୍ଷର *&Arg : NewArgv) {
    ଯଦି (!Arg)
      ଜାରି;

    // Substitute <CFGDIR> with the file's base path.
    ଯଦି (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    ତାର୍କିକ ConfigInclusion = ମିଥ୍ୟା;
    ଯଦି (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      ଯଦି (!llvm::sys::path::is_relative(FileName))
        ଜାରି;
    } ଅନ୍ୟଥା ଯଦି (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = ସତ୍ୟ;
    } ଅନ୍ୟଥା {
      ଜାରି;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ଠେଲ_back('@');
    ଯଦି (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      ଯଦି (!findConfigFile(FileName, FilePath))
        ଫେରନ୍ତୁ createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } ଅନ୍ୟଥା {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  ଫେରନ୍ତୁ Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &Argv) {
  ସଂରଚନା ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> File_ଗଦା;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ଗଦା.ଠେଲ_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  ପାଇଁ (unsigned I = 0; I != Argv.size();) {
    ଯେତେବେଳେ (I == File_ଗଦା.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ଗଦା.କାଢ଼_back();
    }

    ଅଚଳ ଅକ୍ଷର *Arg = Argv[I];
    // Check if it is an EOL marker
    ଯଦି (Arg == nullptr) {
      ++I;
      ଜାରି;
    }

    ଯଦି (Arg[0] != '@') {
      ++I;
      ଜାରି;
    }

    ଅଚଳ ଅକ୍ଷର *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    ଯଦି (llvm::sys::path::is_relative(FName)) {
      ଯଦି (CurrentDir.empty()) {
        ଯଦି (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } ଅନ୍ୟଥା {
          ଫେରନ୍ତୁ createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } ଅନ୍ୟଥା {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    ଯଦି (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      ଯଦି (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        ଯଦି (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ଜାରି;
        }
      }
      ଯଦି (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ଫେରନ୍ତୁ createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    ଅଚଳ llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, ଏହି](ଅଚଳ ResponseFileRecord &RFile) -> ErrorOr<ତାର୍କିକ> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      ଯଦି (!RHS)
        ଫେରନ୍ତୁ RHS.getError();
      ଫେରନ୍ତୁ FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    ପାଇଁ (ଅଚଳ auto &F : drop_begin(File_ଗଦା)) {
      ଯଦି (ErrorOr<ତାର୍କିକ> R = IsEquivalent(F)) {
        ଯଦି (R.get())
          ଫେରନ୍ତୁ createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<ଅଚଳ ଅକ୍ଷର *, 0> ExpandedArgv;
    ଯଦି (Error Err = expandResponseFile(FName, ExpandedArgv))
      ଫେରନ୍ତୁ Err;

    ପାଇଁ (ResponseFileRecord &Record : File_ଗଦା) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    File_ଗଦା.ଠେଲ_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.ଭର୍ତ୍ତି(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ପ୍ରତିଜ୍ଞା(File_ଗଦା.size() > 0 && Argv.size() == File_ଗଦା.back().End);
  ଫେରନ୍ତୁ Error::success();
}

ତାର୍କିକ cl::expandResponseFiles(ପୂର୍ଣ୍ଣସଂଖ୍ୟା Argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *Argv,
                             ଅଚଳ ଅକ୍ଷର *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ଧାଡ଼ି;
#ଅନ୍ୟଥା
  auto Tokenize = cl::Tokenize_GNUCommand_ଧାଡ଼ି;
#endif
  // The environment variable specifies initial options.
  ଯଦି (EnvVar)
    ଯଦି (std::optional<std::string> Env_ମୂଲ୍ୟ = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_ମୂଲ୍ୟ, Saver, NewArgv, /*MarkEOLs=*/ମିଥ୍ୟା);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  ଯଦି (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  ଯଦି (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

ତାର୍କିକ ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<ଅକ୍ଷର> &FilePath) {
  SmallString<128> CfgFilePath;
  ଅଚଳ auto FileExists = [ଏହି](SmallString<128> Path) -> ତାର୍କିକ {
    auto Status = FS->status(Path);
    ଫେରନ୍ତୁ Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  ଯଦି (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    ଯଦି (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    ଯଦି (!FileExists(CfgFilePath))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  // Look for the file in search directories.
  ପାଇଁ (ଅଚଳ StringRef &Dir : ସନ୍ଧାନ_Dirs) {
    ଯଦି (Dir.empty())
      ଜାରି;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    ଯଦି (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<ଅଚଳ ଅକ୍ଷର *> &Argv) {
  SmallString<128> AbsPath;
  ଯଦି (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    ଯଦି (std::error_code EC = FS->makeAbsolute(AbsPath))
      ଫେରନ୍ତୁ make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = ସତ୍ୟ;
  RelativeNames = ସତ୍ୟ;
  ଯଦି (Error Err = expandResponseFile(CfgFile, Argv))
    ଫେରନ୍ତୁ Err;
  ଫେରନ୍ତୁ expandResponseFiles(Argv);
}

ସ୍ଥିର ଶୂନ୍ୟ initCommonOptions();
ସ୍ଥିର ତାର୍କିକ parse_Command_ଧାଡ଼ି_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ଅଚଳ ଅକ୍ଷର *EnvVar) {
  initCommonOptions();
  SmallVector<ଅଚଳ ଅକ୍ଷର *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ଠେଲ_back(Args[0]);

  // Parse options from environment variable.
  ଯଦି (EnvVar) {
    ଯଦି (std::optional<std::string> Env_ମୂଲ୍ୟ =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ଧାଡ଼ି(*Env_ମୂଲ୍ୟ, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NewArgc = static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(NewArgv.size());

  // Parse all options.
  ଫେରନ୍ତୁ globalParser().Parse_Command_ଧାଡ଼ି_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

ତାର୍କିକ cl::Parse_Command_ଧାଡ଼ି_Options(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ଅଚଳ ଅକ୍ଷର *EnvVar) {
  ଫେରନ୍ତୁ parse_Command_ଧାଡ଼ି_Options_Impl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

ତାର୍କିକ cl::parse_Command_ଧାଡ଼ି_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ଅଚଳ ଅକ୍ଷର *EnvVar) {
  ଫେରନ୍ତୁ parse_Command_ଧାଡ଼ି_Options_Impl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
ଶୂନ୍ୟ Command_ଧାଡ଼ି_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  ପାଇଁ (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    ପାଇଁ (auto &O : SC->OptionsMap)
      Opts.ଠେଲ_back(O.second);
    ପାଇଁ (Option *O : Opts)
      O->reset();
    ପାଇଁ (Option *O : SC->PositionalOpts)
      O->reset();
    ଯଦି (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  ପାଇଁ (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

ତାର୍କିକ Command_ଧାଡ଼ି_Parser::Parse_Command_ଧାଡ଼ି_Options(
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ପ୍ରତିଜ୍ଞା(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  ତାର୍କିକ IgnoreErrors = Errs;
  ଯଦି (!Errs)
    Errs = &errs();
  ଯଦି (!VFS)
    VFS = vfs::getRealFileSystem().get();
  ତାର୍କିକ ErrorParsing = ମିଥ୍ୟା;

  // Expand response files.
  SmallVector<ଅଚଳ ଅକ୍ଷର *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::Tokenize_Windows_Command_ଧାଡ଼ି;
#ଅନ୍ୟଥା
  auto Tokenize = cl::Tokenize_GNUCommand_ଧାଡ଼ି;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  ଯଦି (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  argv = &newArgv[0];
  argc = static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned ଅଙ୍କ_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  ତାର୍କିକ HasUnlimitedPositionals = ମିଥ୍ୟା;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_ଶୀର୍ଷ_Level();
  std::string NearestSubCommandString;
  ତାର୍କିକ MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  ଯଦି (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    ଯଦି (ChosenSubCommand != &SubCommand::get_ଶୀର୍ଷ_Level())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  ପ୍ରତିଜ୍ଞା(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  ଯଦି (ConsumeAfterOpt) {
    ପ୍ରତିଜ୍ଞା(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  ଯଦି (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    ତାର୍କିକ UnboundedFound = ମିଥ୍ୟା;
    ପାଇଁ (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      ଯଦି (Requires_ମୂଲ୍ୟ(Opt))
        ++ଅଙ୍କ_Positional_Required;
      ଅନ୍ୟଥା ଯଦି (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        ଯଦି (PositionalOpts.size() > 1) {
          ଯଦି (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = ସତ୍ୟ;
        }
      } ଅନ୍ୟଥା ଯଦି (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        ଯଦି (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = ସତ୍ୟ;
      }
      UnboundedFound |= Eats_Unbounded_ସଂଖ୍ୟା_Of_ମୂଲ୍ୟଗୁଡ଼ିକ(Opt);
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
  ତାର୍କିକ DashDashFound = ମିଥ୍ୟା; // Have we read '--'?
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef ମୂଲ୍ୟ;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    ଯଦି (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      ଯଦି (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ଜାରି; // We are done!
      }

      ଯଦି (!PositionalOpts.empty()) {
        PositionalVals.ଠେଲ_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        ଯଦି (PositionalVals.size() >= ଅଙ୍କ_Positional_Required && ConsumeAfterOpt) {
          ପାଇଁ (++i; i < argc; ++i)
            PositionalVals.ଠେଲ_back(std::make_pair(StringRef(argv[i]), i));
          ଭାଙ୍ଗ; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ଜାରି;
      }
    } ଅନ୍ୟଥା ଯଦି (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = ସତ୍ୟ; // This is the mythical "--"?
      ଜାରି;             // Don't try to process it as an argument itself.
    } ଅନ୍ୟଥା ଯଦି (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ମୂଲ୍ୟ);
      ଯଦି (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        ଜାରି; // We are done!
      }
    } ଅନ୍ୟଥା { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ମୂଲ୍ୟ);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      ଯଦି (!Handler && ChosenSubCommand != &SubCommand::get_ଶୀର୍ଷ_Level())
        Handler = LookupOption(SubCommand::get_ଶୀର୍ଷ_Level(), ArgName, ମୂଲ୍ୟ);

      ଯଦି (!Handler) {
        ଯଦି (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<ଅଚଳ ଅକ୍ଷର *> Rest(argv + i, argc - i);
          unsigned N;
          ଯଦି (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = ସତ୍ୟ;
          }
          i += N - 1;
          ଜାରି;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      ଯଦି (!Handler)
        Handler = HandlePrefixedOption(ArgName, ମୂଲ୍ୟ, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      ଯଦି (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    ଯଦି (!Handler) {
      auto ReportUnknownArgument = [&](ତାର୍କିକ IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        ଯଦି (NearestArgumentName.empty())
          ଫେରନ୍ତୁ;

        *Errs << ProgramName << ": Did you mean '";
        ଯଦି (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        ଅନ୍ୟଥା
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      ଯଦି (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/ସତ୍ୟ, NearestHandlerString);
      ଅନ୍ୟଥା
        ReportUnknownArgument(/*IsArg=*/ମିଥ୍ୟା, NearestSubCommandString);

      ErrorParsing = ସତ୍ୟ;
      ଜାରି;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    ଯଦି (Handler->getFormattingFlag() == cl::Positional) {
      ଯଦି ((Handler->getMiscFlags() & PositionalEatsArgs) && !ମୂଲ୍ୟ.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = ସତ୍ୟ;
      }
      ActivePositionalArg = Handler;
    }
    ଅନ୍ୟଥା
      ErrorParsing |= ProvideOption(Handler, ArgName, ମୂଲ୍ୟ, argc, argv, i);
  }

  // Check and handle positional arguments now...
  ଯଦି (ଅଙ୍କ_Positional_Required > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << ଅଙ୍କ_Positional_Required
             << " positional argument" << (ଅଙ୍କ_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = ସତ୍ୟ;
  } ଅନ୍ୟଥା ଯଦି (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = ସତ୍ୟ;

  } ଅନ୍ୟଥା ଯଦି (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, ଅଙ୍କ_Vals = static_cast<unsigned>(PositionalVals.size());
    ପାଇଁ (Option *Opt : PositionalOpts) {
      ଯଦି (Requires_ମୂଲ୍ୟ(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --ଅଙ୍କ_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      ତାର୍କିକ Done = Opt->get_ଅଙ୍କ_Occurrences_Flag() == cl::Required;
      ଯେତେବେଳେ (ଅଙ୍କ_Vals - ValNo > ଅଙ୍କ_Positional_Required && !Done) {
        ବାଛ (Opt->get_ଅଙ୍କ_Occurrences_Flag()) {
        ସ୍ଥିତି cl::Optional:
          Done = ସତ୍ୟ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        ସ୍ଥିତି cl::ZeroOrMore: // Zero or more will take all they can get...
        ସ୍ଥିତି cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(ConsumeAfterOpt && ଅଙ୍କ_Positional_Required <= PositionalVals.size());
    unsigned ValNo = 0;
    ପାଇଁ (Option *Opt : PositionalOpts)
      ଯଦି (Requires_ମୂଲ୍ୟ(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    ଯଦି (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    ପାଇଁ (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  ପାଇଁ (ଅଚଳ auto &Opt : OptionsMap) {
    ବାଛ (Opt.second->get_ଅଙ୍କ_Occurrences_Flag()) {
    ସ୍ଥିତି Required:
    ସ୍ଥିତି OneOrMore:
      ଯଦି (Opt.second->get_ଅଙ୍କ_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = ସତ୍ୟ;
      }
      [[fallthrough]];
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  ଯଦି (ErrorParsing) {
    ଯଦି (!IgnoreErrors)
      exit(1);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

ତାର୍କିକ Option::error(ଅଚଳ Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  ଯଦି (!ArgName.data())
    ArgName = ArgStr;
  ଯଦି (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  ଅନ୍ୟଥା
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef ମୂଲ୍ୟ) {
  ++ଅଙ୍କ_Occurrences;
  ଫେରନ୍ତୁ handleOccurrence(pos, ArgName, ମୂଲ୍ୟ);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ସ୍ଥିର StringRef get_ମୂଲ୍ୟ_Str(ଅଚଳ Option &O, StringRef Default_ସନ୍ଦେଶ) {
  ଯଦି (O.ମୂଲ୍ୟ_Str.empty())
    ଫେରନ୍ତୁ Default_ସନ୍ଦେଶ;
  ଫେରନ୍ତୁ O.ମୂଲ୍ୟ_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() ଅଚଳ {
  ଫେରନ୍ତୁ argPlusPrefixesSize(ArgStr);
}

ଶୂନ୍ୟ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_ଧାଡ଼ି_Indented_By) {
  ପ୍ରତିଜ୍ଞା(Indent >= First_ଧାଡ଼ି_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - First_ଧାଡ଼ି_Indented_By)
      << ArgHelpPrefix << Split.first << "\n";
  ଯେତେବେଳେ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

ଶୂନ୍ୟ Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t First_ଧାଡ଼ି_Indented_By) {
  ଅଚଳ StringRef ValHelpPrefix = "  ";
  ପ୍ରତିଜ୍ଞା(BaseIndent >= First_ଧାଡ଼ି_Indented_By);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - First_ଧାଡ଼ି_Indented_By)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  ଯେତେବେଳେ (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
ଶୂନ୍ୟ alias::printOptionInfo(size_t GlobalWidth) ଅଚଳ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(ଅଚଳ Option &O) ଅଚଳ {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = get_ମୂଲ୍ୟ_Name();
  ଯଦି (!ValName.empty()) {
    size_t FormattingLen = 3;
    ଯଦି (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += get_ମୂଲ୍ୟ_Str(O, ValName).size() + FormattingLen;
  }

  ଫେରନ୍ତୁ Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ଶୂନ୍ୟ basic_parser_impl::printOptionInfo(ଅଚଳ Option &O,
                                        size_t GlobalWidth) ଅଚଳ {
  outs() << PrintArg(O.ArgStr);

  auto ValName = get_ମୂଲ୍ୟ_Name();
  ଯଦି (!ValName.empty()) {
    ଯଦି (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << get_ମୂଲ୍ୟ_Str(O, ValName) << ">...";
    } ଅନ୍ୟଥା ଯଦି (O.get_ମୂଲ୍ୟ_Expected_Flag() == ମୂଲ୍ୟ_Optional)
      outs() << "[=<" << get_ମୂଲ୍ୟ_Str(O, ValName) << ">]";
    ଅନ୍ୟଥା {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << get_ମୂଲ୍ୟ_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

ଶୂନ୍ୟ basic_parser_impl::printOptionName(ଅଚଳ Option &O,
                                        size_t GlobalWidth) ଅଚଳ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
ତାର୍କିକ parser<ତାର୍କିକ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ତାର୍କିକ &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ parseBool<ତାର୍କିକ, ସତ୍ୟ, ମିଥ୍ୟା>(O, ArgName, Arg, ମୂଲ୍ୟ);
}

// parser<boolOrDefault> implementation
//
ତାର୍କିକ parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, ମୂଲ୍ୟ);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
ସ୍ଥିର ତାର୍କିକ parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ମୂଲ୍ୟ_Kind,
                                         FixedOrScalableQuantityT &ମୂଲ୍ୟ) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_ମୂଲ୍ୟ;
  ଯଦି (!Arg.getAsInteger(0, Min_ମୂଲ୍ୟ)) {
    ମୂଲ୍ୟ = FixedOrScalableQuantityT::getFixed(Min_ମୂଲ୍ୟ);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  StringRef Remainder = Arg;
  ଯଦି (!Remainder.consume_front("vscale"))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for " + ମୂଲ୍ୟ_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  ଯଦି (!Remainder.consume_front('x'))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for " + ମୂଲ୍ୟ_Kind +
                   " argument!");

  Remainder = Remainder.ltrim();
  ଯଦି (Remainder.getAsInteger(0, Min_ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for " + ମୂଲ୍ୟ_Kind +
                   " argument!");

  ମୂଲ୍ୟ = FixedOrScalableQuantityT::getScalable(Min_ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<int> implementation
//
ତାର୍କିକ parser<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ମୂଲ୍ୟ) {
  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for integer argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<long> implementation
//
ତାର୍କିକ parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &ମୂଲ୍ୟ) {
  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for long argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<long long> implementation
//
ତାର୍କିକ parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &ମୂଲ୍ୟ) {
  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for llong argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<unsigned> implementation
//
ତାର୍କିକ parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &ମୂଲ୍ୟ) {

  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for uint argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<unsigned long> implementation
//
ତାର୍କିକ parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &ମୂଲ୍ୟ) {

  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for ulong argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<unsigned long long> implementation
//
ତାର୍କିକ parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &ମୂଲ୍ୟ) {

  ଯଦି (Arg.getAsInteger(0, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for ullong argument!");
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// parser<ElementCount> implementation
//
ତାର୍କିକ parser<Element_ଗଣନା>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ଗଣନା &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ parseFixedOrScalableQuantity(O, Arg, get_ମୂଲ୍ୟ_Name(), ମୂଲ୍ୟ);
}

// parser<double>/parser<float> implementation
//
ସ୍ଥିର ତାର୍କିକ parseDouble(Option &O, StringRef Arg, ଦ୍ୱିଗୁଣ &ମୂଲ୍ୟ) {
  ଯଦି (to_float(Arg, ମୂଲ୍ୟ))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ O.error("'" + Arg + "' value invalid for floating point argument!");
}

ତାର୍କିକ parser<ଦ୍ୱିଗୁଣ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ଦ୍ୱିଗୁଣ &Val) {
  ଫେରନ୍ତୁ parseDouble(O, Arg, Val);
}

ତାର୍କିକ parser<ଦଶମିକ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ଦଶମିକ &Val) {
  ଦ୍ୱିଗୁଣ dVal;
  ଯଦି (parseDouble(O, Arg, dVal))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  Val = (ଦଶମିକ)dVal;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = get_ଅଙ୍କ_Options();

  ପାଇଁ (unsigned i = 0; i != e; ++i) {
    ଯଦି (getOption(i) == Name)
      ଫେରନ୍ତୁ i;
  }
  ଫେରନ୍ତୁ e;
}

ସ୍ଥିର StringRef Eq_ମୂଲ୍ୟ = "=<value>";
ସ୍ଥିର StringRef EmptyOption = "<empty>";
ସ୍ଥିର StringRef OptionPrefix = "    =";
ସ୍ଥିର size_t getOptionPrefixesSize() {
  ଫେରନ୍ତୁ OptionPrefix.size() + ArgHelpPrefix.size();
}

ସ୍ଥିର ତାର୍କିକ shouldPrintOption(StringRef Name, StringRef Description,
                              ଅଚଳ Option &O) {
  ଫେରନ୍ତୁ O.get_ମୂଲ୍ୟ_Expected_Flag() != ମୂଲ୍ୟ_Optional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(ଅଚଳ Option &O) ଅଚଳ {
  ଯଦି (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + Eq_ମୂଲ୍ୟ.size();
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      ଯଦି (!shouldPrintOption(Name, getDescription(i), O))
        ଜାରି;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    ଫେରନ୍ତୁ Size;
  } ଅନ୍ୟଥା {
    size_t BaseSize = 0;
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Options(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    ଫେରନ୍ତୁ BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ଶୂନ୍ୟ generic_parser_base::printOptionInfo(ଅଚଳ Option &O,
                                          size_t GlobalWidth) ଅଚଳ {
  ଯଦି (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    ଯଦି (O.get_ମୂଲ୍ୟ_Expected_Flag() == ମୂଲ୍ୟ_Optional) {
      ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Options(); i != e; ++i) {
        ଯଦି (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          ଭାଙ୍ଗ;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_ମୂଲ୍ୟ;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         Eq_ମୂଲ୍ୟ.size() +
                             argPlusPrefixesSize(O.ArgStr));
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      ଯଦି (!shouldPrintOption(OptionName, Description, O))
        ଜାରି;
      size_t First_ଧାଡ଼ି_Indent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      ଯଦି (OptionName.empty()) {
        outs() << EmptyOption;
        ପ୍ରତିଜ୍ଞା(First_ଧାଡ଼ି_Indent >= EmptyOption.size());
        First_ଧାଡ଼ି_Indent += EmptyOption.size();
      }
      ଯଦି (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, First_ଧାଡ଼ି_Indent);
      ଅନ୍ୟଥା
        outs() << '\n';
    }
  } ଅନ୍ୟଥା {
    ଯଦି (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

ସ୍ଥିର ଅଚଳ size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
ଶୂନ୍ୟ generic_parser_base::printGenericOptionDiff(
    ଅଚଳ Option &O, ଅଚଳ Generic_Option_ମୂଲ୍ୟ &ମୂଲ୍ୟ,
    ଅଚଳ Generic_Option_ମୂଲ୍ୟ &Default, size_t GlobalWidth) ଅଚଳ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned ଅଙ୍କ_Opts = get_ଅଙ୍କ_Options();
  ପାଇଁ (unsigned i = 0; i != ଅଙ୍କ_Opts; ++i) {
    ଯଦି (!ମୂଲ୍ୟ.compare(get_Option_ମୂଲ୍ୟ(i)))
      ଜାରି;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t ଅଙ୍କ_Spaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(ଅଙ୍କ_Spaces) << " (default: ";
    ପାଇଁ (unsigned j = 0; j != ଅଙ୍କ_Opts; ++j) {
      ଯଦି (!Default.compare(get_Option_ମୂଲ୍ୟ(j)))
        ଜାରି;
      outs() << getOption(j);
      ଭାଙ୍ଗ;
    }
    outs() << ")\n";
    ଫେରନ୍ତୁ;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
ନାମାକାଶ llvm {
ନାମାକାଶ cl {
ସ୍ଥିର raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  ଫେରନ୍ତୁ OS << static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  ଶୂନ୍ୟ parser<T>::printOptionDiff(ଅଚଳ Option &O, T V, Option_ମୂଲ୍ୟ<T> D,      \
                                  size_t GlobalWidth) ଅଚଳ {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t ଅଙ୍କ_Spaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(ଅଙ୍କ_Spaces) << " (default: ";                                 \
    ଯଦି (D.has_ମୂଲ୍ୟ())                                                          \
      outs() << D.get_ମୂଲ୍ୟ();                                                  \
    ଅନ୍ୟଥା                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(ତାର୍କିକ)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(ଦ୍ୱିଗୁଣ)
PRINT_OPT_DIFF(ଦଶମିକ)
PRINT_OPT_DIFF(ଅକ୍ଷର)
PRINT_OPT_DIFF(Element_ଗଣନା)

ଶୂନ୍ୟ parser<std::string>::printOptionDiff(ଅଚଳ Option &O, StringRef V,
                                          ଅଚଳ Option_ମୂଲ୍ୟ<std::string> &D,
                                          size_t GlobalWidth) ଅଚଳ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t ଅଙ୍କ_Spaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(ଅଙ୍କ_Spaces) << " (default: ";
  ଯଦି (D.has_ମୂଲ୍ୟ())
    outs() << D.get_ମୂଲ୍ୟ();
  ଅନ୍ୟଥା
    outs() << "*no default*";
  outs() << ")\n";
}

ଶୂନ୍ୟ parser<std::optional<std::string>>::printOptionDiff(
    ଅଚଳ Option &O, std::optional<StringRef> V,
    ଅଚଳ Option_ମୂଲ୍ୟ<std::optional<std::string>> &D,
    size_t GlobalWidth) ଅଚଳ {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_ମୂଲ୍ୟ_2() ? V.ମୂଲ୍ୟ_2().size() : 0;
  size_t ଅଙ୍କ_Spaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(ଅଙ୍କ_Spaces) << " (default: ";
  ଯଦି (D.has_ମୂଲ୍ୟ() && D.get_ମୂଲ୍ୟ().has_ମୂଲ୍ୟ_2())
    outs() << D.get_ମୂଲ୍ୟ();
  ଅନ୍ୟଥା
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
ଶୂନ୍ୟ basic_parser_impl::print_Option_No_ମୂଲ୍ୟ(ଅଚଳ Option &O,
                                           size_t GlobalWidth) ଅଚଳ {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା OptNameCompare(ଅଚଳ std::pair<ଅଚଳ ଅକ୍ଷର *, Option *> *LHS,
                          ଅଚଳ std::pair<ଅଚଳ ଅକ୍ଷର *, Option *> *RHS) {
  ଫେରନ୍ତୁ strcmp(LHS->first, RHS->first);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SubNameCompare(ଅଚଳ std::pair<ଅଚଳ ଅକ୍ଷର *, SubCommand *> *LHS,
                          ଅଚଳ std::pair<ଅଚଳ ଅକ୍ଷର *, SubCommand *> *RHS) {
  ଫେରନ୍ତୁ strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
ସ୍ଥିର ଶୂନ୍ୟ sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<ଅଚଳ ଅକ୍ଷର *, Option *>> &Opts,
                     ତାର୍କିକ ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  ପାଇଁ (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    ଯଦି (I->second->getOptionHiddenFlag() == ReallyHidden)
      ଜାରି;

    // Unless showhidden is set, ignore hidden flags.
    ଯଦି (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ଜାରି;

    // If we've already seen this option, don't add it to the list again.
    ଯଦି (!OptionSet.ଭର୍ତ୍ତି(I->second).second)
      ଜାରି;

    Opts.ଠେଲ_back(
        std::pair<ଅଚଳ ଅକ୍ଷର *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

ସ୍ଥିର ଶୂନ୍ୟ
sortSubCommands(ଅଚଳ SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<ଅଚଳ ଅକ୍ଷର *, SubCommand *>> &Subs) {
  ପାଇଁ (auto *S : SubMap) {
    ଯଦି (S->getName().empty())
      ଜାରି;
    Subs.ଠେଲ_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

ନାମାକାଶ {

ଶ୍ରେଣୀ HelpPrinter {
ସୁରକ୍ଷିତ:
  ଅଚଳ ତାର୍କିକ ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<ଅଚଳ ଅକ୍ଷର *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<ଅଚଳ ଅକ୍ଷର *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  ଆଭାସୀ ଶୂନ୍ୟ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    ପାଇଁ (ଅଚଳ auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  ଶୂନ୍ୟ printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    ପାଇଁ (ଅଚଳ auto &S : Subs) {
      outs() << "  " << S.first;
      ଯଦି (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

ସର୍ବସାଧାରଣ:
  explicit HelpPrinter(ତାର୍କିକ showHidden) : ShowHidden(showHidden) {}
  ଆଭାସୀ ~HelpPrinter() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

  // Invoke the printer.
  ଶୂନ୍ୟ operator=(ତାର୍କିକ ମୂଲ୍ୟ) {
    ଯଦି (!ମୂଲ୍ୟ)
      ଫେରନ୍ତୁ;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  ଶୂନ୍ୟ printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    ଯଦି (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    ଯଦି (Sub == &SubCommand::get_ଶୀର୍ଷ_Level()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      ଯଦି (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } ଅନ୍ୟଥା {
      ଯଦି (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    ପାଇଁ (auto *Opt : PositionalOpts) {
      ଯଦି (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    ଯଦି (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    ଯଦି (Sub == &SubCommand::get_ଶୀର୍ଷ_Level() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      ପାଇଁ (ଅଚଳ auto &Sub : Subs)
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
    ପାଇଁ (ଅଚଳ auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    ଯଦି (ShowHidden)
      ପାଇଁ (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              ଯଦି (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    ପାଇଁ (ଅଚଳ auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    ଯଦି (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    ପାଇଁ (ଅଚଳ auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    ପାଇଁ (ଅଚଳ auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

ଶ୍ରେଣୀ CategorizedHelpPrinter : ସର୍ବସାଧାରଣ HelpPrinter {
ସର୍ବସାଧାରଣ:
  explicit CategorizedHelpPrinter(ତାର୍କିକ showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା OptionCategoryCompare(OptionCategory *ଅଚଳ *A,
                                   OptionCategory *ଅଚଳ *B) {
    ଫେରନ୍ତୁ (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

ସୁରକ୍ଷିତ:
  ଶୂନ୍ୟ printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ପ୍ରତିଜ୍ଞା(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    ପାଇଁ (ଅଚଳ auto &I : Opts) {
      Option *Opt = I.second;
      ପାଇଁ (OptionCategory *Cat : Opt->Categories) {
        ପ୍ରତିଜ୍ଞା(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ଠେଲ_back(Opt);
      }
    }

    // Now do printing.
    ପାଇଁ (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ଅଚଳ auto &CategoryOptions = CategorizedOptions[Category];
      ଯଦି (CategoryOptions.empty())
        ଜାରି;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      ଯଦି (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      ଅନ୍ୟଥା
        outs() << "\n";

      // Loop over the options in the category and print.
      ପାଇଁ (ଅଚଳ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
ଶ୍ରେଣୀ HelpPrinterWrapper {
ବ୍ୟକ୍ତିଗତ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

ସର୍ବସାଧାରଣ:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  ଶୂନ୍ୟ operator=(ତାର୍କିକ ମୂଲ୍ୟ);
};

} // End anonymous namespace

#ଯଦି defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# ଯଦି defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# ଅନ୍ୟଥା
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#ନଚେତଯଦି defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# ଯଦି defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# ଅନ୍ୟଥା
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#ଅନ୍ୟଥା
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

ନାମାକାଶ {
ଶ୍ରେଣୀ VersionPrinter {
ସର୍ବସାଧାରଣ:
  ଶୂନ୍ୟ print(ଅଚଳ std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#ଅନ୍ୟଥା
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#ଯଦି LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#ଅନ୍ୟଥା
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    ଯଦି (!ExtraPrinters.empty()) {
      ପାଇଁ (ଅଚଳ auto &I : ExtraPrinters)
        I(outs());
    }
  }
  ଶୂନ୍ୟ operator=(ତାର୍କିକ OptionWasSpecified);
};

ସଂରଚନା Command_ଧାଡ଼ି_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{ମିଥ୍ୟା};
  HelpPrinter UncategorizedHiddenPrinter{ସତ୍ୟ};
  CategorizedHelpPrinter CategorizedNormalPrinter{ମିଥ୍ୟା};
  CategorizedHelpPrinter CategorizedHiddenPrinter{ସତ୍ୟ};
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
  cl::opt<HelpPrinter, ସତ୍ୟ, parser<ତାର୍କିକ>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::ମୂଲ୍ୟ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, ସତ୍ୟ, parser<ତାର୍କିକ>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ମୂଲ୍ୟ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, ସତ୍ୟ, parser<ତାର୍କିକ>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::ମୂଲ୍ୟ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, ସତ୍ୟ, parser<ତାର୍କିକ>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ମୂଲ୍ୟ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ତାର୍କିକ> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(ମିଥ୍ୟା),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ତାର୍କିକ> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(ମିଥ୍ୟା),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, ସତ୍ୟ, parser<ତାର୍କିକ>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::ମୂଲ୍ୟ_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ସ୍ଥିର ManagedStatic<Command_ଧାଡ଼ି_Common_Options> CommonOptions;

ସ୍ଥିର ଶୂନ୍ୟ initCommonOptions() {
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
  ସ୍ଥିର OptionCategory GeneralCategory{"General options"};
  ଫେରନ୍ତୁ GeneralCategory;
}

ଶୂନ୍ୟ VersionPrinter::operator=(ତାର୍କିକ OptionWasSpecified) {
  ଯଦି (!OptionWasSpecified)
    ଫେରନ୍ତୁ;

  ଯଦି (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

ଶୂନ୍ୟ HelpPrinterWrapper::operator=(ତାର୍କିକ ମୂଲ୍ୟ) {
  ଯଦି (!ମୂଲ୍ୟ)
    ଫେରନ୍ତୁ;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  ଯଦି (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = ସତ୍ୟ; // Invoke categorized printer
  } ଅନ୍ୟଥା {
    UncategorizedPrinter = ସତ୍ୟ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
ଶୂନ୍ୟ cl::Print_Option_ମୂଲ୍ୟଗୁଡ଼ିକ() { globalParser().print_Option_ମୂଲ୍ୟଗୁଡ଼ିକ(); }

ଶୂନ୍ୟ Command_ଧାଡ଼ି_Parser::print_Option_ମୂଲ୍ୟଗୁଡ଼ିକ() {
  ଯଦି (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ଫେରନ୍ତୁ;

  SmallVector<std::pair<ଅଚଳ ଅକ୍ଷର *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ ସତ୍ୟ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  ପାଇଁ (ଅଚଳ auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  ପାଇଁ (ଅଚଳ auto &Opt : Opts)
    Opt.second->print_Option_ମୂଲ୍ୟ(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
ଶୂନ୍ୟ cl::PrintHelpMessage(ତାର୍କିକ Hidden, ତାର୍କିକ Categorized) {
  ଯଦି (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  ଅନ୍ୟଥା ଯଦି (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  ଅନ୍ୟଥା ଯଦି (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  ଅନ୍ୟଥା
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ସ୍ଥିର ଅଚଳ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#ଯଦି LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#ଯଦି __has_feature(address_sanitizer)
      "+asan",
#endif
#ଯଦି __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#ଯଦି __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#ଯଦି __has_feature(memory_sanitizer)
      "+msan",
#endif
#ଯଦି __has_feature(thread_sanitizer)
      "+tsan",
#endif
#ଯଦି __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  ଫେରନ୍ତୁ ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
ଶୂନ୍ୟ cl::printBuildConfig(raw_ostream &OS) {
#ଯଦି LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
ଶୂନ୍ୟ cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

ଶୂନ୍ୟ cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

ଶୂନ୍ୟ cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.ଠେଲ_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (ଶୂନ୍ୟ)Subs;
  ପ୍ରତିଜ୍ଞା(Subs.contains(&Sub));
  ଫେରନ୍ତୁ Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ଫେରନ୍ତୁ globalParser().getRegisteredSubcommands();
}

ଶୂନ୍ୟ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  ପାଇଁ (auto &I : Sub.OptionsMap) {
    ତାର୍କିକ Unrelated = ସତ୍ୟ;
    ପାଇଁ (auto &Cat : I.second->Categories) {
      ଯଦି (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = ମିଥ୍ୟା;
    }
    ଯଦି (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ଶୂନ୍ୟ cl::HideUnrelatedOptions(ArrayRef<ଅଚଳ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  ପାଇଁ (auto &I : Sub.OptionsMap) {
    ତାର୍କିକ Unrelated = ସତ୍ୟ;
    ପାଇଁ (auto &Cat : I.second->Categories) {
      ଯଦି (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = ମିଥ୍ୟା;
    }
    ଯଦି (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

ଶୂନ୍ୟ cl::Reset_Command_ଧାଡ଼ି_Parser() { globalParser().reset(); }

ଶୂନ୍ୟ cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.ଠେଲ_back(&L);
}

ଶୂନ୍ୟ LibraryOptionsContext::assign(ଅଚଳ LibraryOptionsContext &Other) {
  ପ୍ରତିଜ୍ଞା(ଏହି != &Other);
  ମୂଲ୍ୟଗୁଡ଼ିକ.clear();
  SavedArgs.clear();
  ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args = Other.SavedArgs;
  ପାଇଁ (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = ମୂଲ୍ୟଗୁଡ଼ିକ.try_emplace(L.getDefault());
  ଯଦି (Inserted)
    It->second = L.copyDefault(Alloc);
  ଫେରନ୍ତୁ parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

ଶୂନ୍ୟ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

ଶୂନ୍ୟ LLVMParse_Command_ଧାଡ଼ି_Options(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
                                 ଅଚଳ ଅକ୍ଷର *Overview) {
  llvm::cl::Parse_Command_ଧାଡ଼ି_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
