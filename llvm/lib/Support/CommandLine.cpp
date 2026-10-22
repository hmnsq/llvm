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

#అన్తర్భావ "llvm/Support/CommandLine.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm-c/Support.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/STLFunctionalExtras.h"
#అన్తర్భావ "llvm/ADT/SmallPtrSet.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/LibraryOptionsContext.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/StringSaver.h"
#అన్తర్భావ "llvm/Support/TypeSize.h"
#అన్తర్భావ "llvm/Support/VirtualFileSystem.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cstdlib>
#అన్తర్భావ <optional>
#అన్తర్భావ <string>
ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ cl;

#నిర్వచించు DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
నామాకాశ llvm {
నామాకాశ cl {
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<తార్కిక>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<పూర్ణాంకం>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<దీర్ఘ>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<దీర్ఘ దీర్ఘ>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<గుర్తులేని>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<గుర్తులేని దీర్ఘ>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<గుర్తులేని దీర్ఘ దీర్ఘ>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<ద్విగుణ>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<దశాంశం>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<అక్షరం>;
మూస తరగతి LLVM_EXPORT_TEMPLATE basic_parser<Element_లెక్క>;

#ఒకవేళ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
మూస తరగతి LLVM_EXPORT_TEMPLATE opt<std::string>;
#అయితే_ముగింపు

మూస తరగతి LLVM_EXPORT_TEMPLATE opt<తార్కిక>;
మూస తరగతి LLVM_EXPORT_TEMPLATE opt<అక్షరం>;
మూస తరగతి LLVM_EXPORT_TEMPLATE opt<పూర్ణాంకం>;
మూస తరగతి LLVM_EXPORT_TEMPLATE opt<గుర్తులేని>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
శూన్య Generic_Option_విలువ::anchor() {}
శూన్య Option_విలువ<boolOrDefault>::anchor() {}
శూన్య Option_విలువ<std::string>::anchor() {}
శూన్య Option::anchor() {}
శూన్య basic_parser_impl::anchor() {}
శూన్య parser<తార్కిక>::anchor() {}
శూన్య parser<boolOrDefault>::anchor() {}
శూన్య parser<పూర్ణాంకం>::anchor() {}
శూన్య parser<దీర్ఘ>::anchor() {}
శూన్య parser<దీర్ఘ దీర్ఘ>::anchor() {}
శూన్య parser<గుర్తులేని>::anchor() {}
శూన్య parser<గుర్తులేని దీర్ఘ>::anchor() {}
శూన్య parser<గుర్తులేని దీర్ఘ దీర్ఘ>::anchor() {}
శూన్య parser<ద్విగుణ>::anchor() {}
శూన్య parser<దశాంశం>::anchor() {}
శూన్య parser<std::string>::anchor() {}
శూన్య parser<std::optional<std::string>>::anchor() {}
శూన్య parser<అక్షరం>::anchor() {}
శూన్య parser<Element_లెక్క>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
శూన్య opt_bool_anchor() { opt<తార్కిక> anchor{""}; }
శూన్య opt_char_anchor() { opt<అక్షరం> anchor{""}; }
శూన్య opt_int_anchor() { opt<పూర్ణాంకం> anchor{""}; }
శూన్య opt_unsigned_anchor() { opt<గుర్తులేని> anchor{""}; }

//===----------------------------------------------------------------------===//

స్థిరరాశి స్థిర size_t DefaultPad = 2;

స్థిర StringRef Arg_ఉపసర్గ = "-";
స్థిర StringRef Arg_ఉపసర్గ_Long = "--";
స్థిర StringRef Arg_Help_ఉపసర్గ = " - ";

స్థిర size_t arg_Plus_Prefixes_కొలత(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.కొలత_2();
  ఒకవేళ (Len == 1)
    తిరిగి Len + Pad + Arg_ఉపసర్గ.కొలత_2() + Arg_Help_ఉపసర్గ.కొలత_2();
  తిరిగి Len + Pad + Arg_ఉపసర్గ_Long.కొలత_2() + Arg_Help_ఉపసర్గ.కొలత_2();
}

స్థిర SmallString<8> arg_ఉపసర్గ(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> ఉపసర్గ;
  కొరకు (size_t I = 0; I < Pad; ++I) {
    ఉపసర్గ.నెట్టు_వెనుక(' ');
  }
  ఉపసర్గ.append(ArgName.కొలత_2() > 1 ? Arg_ఉపసర్గ_Long : Arg_ఉపసర్గ);
  తిరిగి ఉపసర్గ;
}

ఉపయోగించి OptionsMapTy = DenseMap<StringRef, Option *>;

నామాకాశ {

తరగతి PrintArg {
  StringRef ArgName;
  size_t Pad;
బహిరంగ:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  మిత్ర raw_ostream &క్రియాకారకం<<(raw_ostream &OS, స్థిరరాశి PrintArg &);
};

raw_ostream &క్రియాకారకం<<(raw_ostream &OS, స్థిరరాశి PrintArg& Arg) {
  OS << arg_ఉపసర్గ(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  తిరిగి OS;
}

తరగతి Command_పంక్తి_Parser {
బహిరంగ:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string కార్యక్రమం_Name;
  StringRef కార్యక్రమం_Overview;

  // This collects additional help to be printed.
  std::సదిశ<StringRef> మరిన్ని_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_సమితి<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_సమితి<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_సదిశ<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_సూచీ;
  size_t అంకె_సూచీకృత_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_పంక్తి_Parser() { రిజిస్టరు_Sub_Command(&SubCommand::get_శిఖరం_Level()); }

  శూన్య ResetAllOptionOccurrences();

  తార్కిక Parse_Command_పంక్తి_Options(పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
                               StringRef Overview, raw_ostream *Errs = శూన్యసూచిక,
                               vfs::FileSystem *VFS = శూన్యసూచిక,
                               LibraryOptionsContext *Context = శూన్యసూచిక);

  శూన్య forEachSubCommand(Option &Opt, function_ref<శూన్య(SubCommand &)> Action) {
    ఒకవేళ (Opt.Subs.ఖాళీలు()) {
      Action(SubCommand::get_శిఖరం_Level());
      తిరిగి;
    }
    ఒకవేళ (Opt.Subs.కొలత_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      కొరకు (స్వయంచాలక *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      తిరిగి;
    }
    కొరకు (స్వయంచాలక *SC : Opt.Subs) {
      ప్రతిజ్ఞా(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  శూన్య కూడు_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    ఒకవేళ (Opt.hasArgStr())
      తిరిగి;
    ఒకవేళ (!SC->OptionsMap.చొప్పించు(std::make_pair(Name, &Opt)).రెండవ_2) {
      errs() << కార్యక్రమం_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  శూన్య కూడు_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { కూడు_Literal_Option(Opt, &SC, Name); });
  }

  శూన్య కూడు_Option(Option *O, SubCommand *SC) {
    తార్కిక HadErrors = అబద్ధం;
    ఒకవేళ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      ఒకవేళ (!SC->OptionsMap.చొప్పించు(std::make_pair(O->ArgStr, O)).రెండవ_2 ||
          (RegisteredSubCommands.కలిగియుంది(SC) &&
           Library_సూచీ.కలిగియుంది(O->ArgStr))) {
        errs() << కార్యక్రమం_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = నిజం;
      }
    }

    // Remember information about positional options.
    ఒకవేళ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.నెట్టు_వెనుక(O);
    లేకపోతే ఒకవేళ (O->get_అంకె_Occurrences_Flag() == cl::ConsumeAfter) {
      ఒకవేళ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = నిజం;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    ఒకవేళ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  శూన్య కూడు_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { కూడు_Option(O, &SC); });
  }

  శూన్య removeOption(Option *O, SubCommand *SC) {
    Small_సదిశ<StringRef, 16> Option_పేర్లు;
    O->get_Extra_Option_పేర్లు(Option_పేర్లు);
    ఒకవేళ (O->hasArgStr())
      Option_పేర్లు.నెట్టు_వెనుక(O->ArgStr);

    SubCommand &Sub = *SC;
    కొరకు (స్వయంచాలక Name : Option_పేర్లు) {
      స్వయంచాలక I = Sub.OptionsMap.కనుగొను(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      ఒకవేళ (I != Sub.OptionsMap.చివర() && I->రెండవ_2 == O)
        Sub.OptionsMap.erase(I);
    }

    ఒకవేళ (O->getFormattingFlag() == cl::Positional)
      కొరకు (స్వయంచాలక *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.చివర(); ++Opt) {
        ఒకవేళ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ఆపు;
        }
      }
    లేకపోతే ఒకవేళ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = శూన్యసూచిక;
  }

  శూన్య removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  తార్కిక hasOptions(స్థిరరాశి SubCommand &Sub) స్థిరరాశి {
    తిరిగి (!Sub.OptionsMap.ఖాళీలు() || !Sub.PositionalOpts.ఖాళీలు() ||
            శూన్యసూచిక != Sub.ConsumeAfterOpt);
  }

  తార్కిక hasOptions() స్థిరరాశి {
    కొరకు (స్థిరరాశి స్వయంచాలక *S : RegisteredSubCommands) {
      ఒకవేళ (hasOptions(*S))
        తిరిగి నిజం;
    }
    తిరిగి !Libraries.ఖాళీలు();
  }

  తార్కిక hasNamedSubCommands() స్థిరరాశి {
    కొరకు (స్థిరరాశి స్వయంచాలక *S : RegisteredSubCommands)
      ఒకవేళ (!S->getName().ఖాళీలు())
        తిరిగి నిజం;
    తిరిగి అబద్ధం;
  }

  SubCommand *get_చురుకైన_Sub_Command() { తిరిగి చురుకైన_Sub_Command; }

  శూన్య updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    ఒకవేళ (!Sub.OptionsMap.చొప్పించు(std::make_pair(NewName, O)).రెండవ_2) {
      errs() << కార్యక్రమం_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  శూన్య updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  శూన్య print_Option_విలువలు();

  శూన్య సూచీ_Library_Options() {
    తార్కిక HadErrors = అబద్ధం;
    కొరకు (; అంకె_సూచీకృత_Libraries != Libraries.కొలత_2(); ++అంకె_సూచీకృత_Libraries) {
      LibraryOptions *L = Libraries[అంకె_సూచీకృత_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        స్వయంచాలక [It, Inserted] = Library_సూచీ.try_emplace(Name, L);
        ఒకవేళ (Inserted ? none_యొక్క(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 తిరిగి SC->OptionsMap.కలిగియుంది(Name);
                               })
                     : It->రెండవ_2 == L)
          తిరిగి;
        errs() << కార్యక్రమం_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = నిజం;
      });
    }
    ఒకవేళ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    సూచీ_Library_Options();
    తిరిగి Library_సూచీ.lookup(Name);
  }

  శూన్య రిజిస్టరు_Category(OptionCategory *cat) {
    ప్రతిజ్ఞా(లెక్క_if(RegisteredOptionCategories,
                    [cat](స్థిరరాశి OptionCategory *Category) {
             తిరిగి cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.చొప్పించు(cat);
  }

  శూన్య రిజిస్టరు_Sub_Command(SubCommand *sub) {
    ప్రతిజ్ఞా(లెక్క_if(RegisteredSubCommands,
                    [sub](స్థిరరాశి SubCommand *Sub) {
                      తిరిగి (!sub->getName().ఖాళీలు()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.చొప్పించు(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ప్రతిజ్ఞా(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    కొరకు (స్వయంచాలక &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.రెండవ_2;
      ఒకవేళ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        కూడు_Option(O, sub);
      లేకపోతే
        కూడు_Literal_Option(*O, sub, E.మొదటి_2);
    }
  }

  శూన్య unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_సమితి<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    తిరిగి make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.చివర());
  }

  శూన్య reset() {
    చురుకైన_Sub_Command = శూన్యసూచిక;
    కార్యక్రమం_Name.clear();
    కార్యక్రమం_Overview = StringRef();

    మరిన్ని_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_సూచీ.clear();
    అంకె_సూచీకృత_Libraries = 0;

    SubCommand::get_శిఖరం_Level().reset();
    SubCommand::getAll().reset();
    రిజిస్టరు_Sub_Command(&SubCommand::get_శిఖరం_Level());
  }

అంతరంగ:
  SubCommand *చురుకైన_Sub_Command = శూన్యసూచిక;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &విలువ);
  SubCommand *LookupSubCommand(StringRef Name, std::string &సమీప_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
స్థిర Command_పంక్తి_Parser &globalParser() {
  స్థిర ManagedStatic<Command_పంక్తి_Parser> GlobalParser;
  తిరిగి *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
స్థిర Error
parseLibraryOption(LibraryOptions &L, శూన్య *Opts, ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు,
                   గుర్తులేని &Consumed, BumpPtrAllocator &Alloc,
                   Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> *రికార్డు = శూన్యసూచిక) {
  StringSaver Saver(Alloc);
  స్థిరరాశి అక్షరం *Saved[2];
  గుర్తులేని అంకె_వాదనలు = std::min<size_t>(వాదనలు.కొలత_2(), 2);
  కొరకు (గుర్తులేని J = 0; J != అంకె_వాదనలు; ++J)
    Saved[J] = Saver.save(వాదనలు[J]).దత్తాంశం();
  Consumed = 1;
  ఒకవేళ (Error E = L.parse(Opts, ArrayRef(Saved, అంకె_వాదనలు), Consumed, Alloc))
    తిరిగి E;
  ఒకవేళ (రికార్డు)
    రికార్డు->append(Saved, Saved + Consumed);
  తిరిగి Error::success();
}

మూస <రకం_పేరు T, T TrueVal, T FalseVal>
స్థిర తార్కిక parseBool(Option &O, StringRef ArgName, StringRef Arg, T &విలువ) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  ఒకవేళ (!Arg.దత్తాంశం() || Arg == "true" || Arg == "1") {
    విలువ = TrueVal;
    తిరిగి అబద్ధం;
  }

  ఒకవేళ (Arg == "false" || Arg == "0") {
    విలువ = FalseVal;
    తిరిగి అబద్ధం;
  }
  తిరిగి O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

శూన్య cl::కూడు_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().కూడు_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().మరిన్ని_Help.నెట్టు_వెనుక(Help);
}

Option::Option(అంకె_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : అంకె_Occurrences(0), Occurrences(OccurrencesFlag), విలువ(0),
      HiddenFlag(Hidden), Formatting(సాధారణ_Formatting), Misc(0),
      FullyInitialized(అబద్ధం), Position(0) {
  Categories.నెట్టు_వెనుక(&getGeneralCategory());
}

శూన్య Option::కూడు_వాదం() {
  globalParser().కూడు_Option(ఇది);
  FullyInitialized = నిజం;
}

శూన్య Option::remove_వాదం() { globalParser().removeOption(ఇది); }

శూన్య Option::సమితి_Arg_Str(StringRef S) {
  ఒకవేళ (FullyInitialized)
    globalParser().updateArgStr(ఇది, S);
  ప్రతిజ్ఞా(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

శూన్య Option::కూడు_Category(OptionCategory &C) {
  ప్రతిజ్ఞా(!Categories.ఖాళీలు() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  ఒకవేళ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  లేకపోతే ఒకవేళ (!is_contained(Categories, &C))
    Categories.నెట్టు_వెనుక(&C);
}

శూన్య Option::reset() {
  అంకె_Occurrences = 0;
  సమితి_Default();
}

శూన్య OptionCategory::రిజిస్టరు_Category() {
  globalParser().రిజిస్టరు_Category(ఇది);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_శిఖరం_Level() {
  స్థిర ManagedStatic<SubCommand> శిఖరం_Level_Sub_Command;
  తిరిగి *శిఖరం_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  స్థిర ManagedStatic<SubCommand> AllSubCommands;
  తిరిగి *AllSubCommands;
}

శూన్య SubCommand::రిజిస్టరు_Sub_Command() {
  globalParser().రిజిస్టరు_Sub_Command(ఇది);
}

శూన్య SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ఇది);
}

శూన్య SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = శూన్యసూచిక;
}

SubCommand::క్రియాకారకం తార్కిక() స్థిరరాశి {
  తిరిగి (globalParser().get_చురుకైన_Sub_Command() == ఇది);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_పంక్తి_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &విలువ) {
  // Reject all dashes.
  ఒకవేళ (Arg.ఖాళీలు())
    తిరిగి శూన్యసూచిక;
  ప్రతిజ్ఞా(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.కనుగొను('=');

  // If we have an equals sign, remember the value.
  ఒకవేళ (EqualPos == StringRef::npos) {
    // Look up the option.
    తిరిగి Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  స్వయంచాలక I = Sub.OptionsMap.కనుగొను(Arg.substr(0, EqualPos));
  ఒకవేళ (I == Sub.OptionsMap.చివర())
    తిరిగి శూన్యసూచిక;

  స్వయంచాలక *O = I->రెండవ_2;
  ఒకవేళ (O->getFormattingFlag() == cl::Always_ఉపసర్గ)
    తిరిగి శూన్యసూచిక;

  విలువ = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  తిరిగి I->రెండవ_2;
}

SubCommand *Command_పంక్తి_Parser::LookupSubCommand(StringRef Name,
                                                std::string &సమీప_String) {
  ఒకవేళ (Name.ఖాళీలు())
    తిరిగి &SubCommand::get_శిఖరం_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *సమీప_సరిపోలిక = శూన్యసూచిక;
  కొరకు (స్వయంచాలక *S : RegisteredSubCommands) {
    ప్రతిజ్ఞా(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    ఒకవేళ (S->getName().ఖాళీలు())
      కొనసాగు;

    ఒకవేళ (S->getName() == Name)
      తిరిగి S;

    ఒకవేళ (!సమీప_సరిపోలిక && S->getName().edit_దూరం(Name) < 2)
      సమీప_సరిపోలిక = S;
  }

  ఒకవేళ (సమీప_సరిపోలిక)
    సమీప_String = సమీప_సరిపోలిక->getName();

  తిరిగి &SubCommand::get_శిఖరం_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
స్థిర Option *Lookup_సమీప_Option(StringRef Arg,
                                   స్థిరరాశి OptionsMapTy &OptionsMap,
                                   std::string &సమీప_String) {
  // Reject all dashes.
  ఒకవేళ (Arg.ఖాళీలు())
    తిరిగి శూన్యసూచిక;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> విభజించు_Arg = Arg.విభజించు('=');
  StringRef &LHS = విభజించు_Arg.మొదటి_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = విభజించు_Arg.రెండవ_2;

  // Find the closest match.
  Option *ఉత్తమ = శూన్యసూచిక;
  గుర్తులేని ఉత్తమ_దూరం = 0;
  కొరకు (స్థిరరాశి స్వయంచాలక &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    ఒకవేళ (O->getOptionHiddenFlag() == ReallyHidden)
      కొనసాగు;

    Small_సదిశ<StringRef, 16> Option_పేర్లు;
    O->get_Extra_Option_పేర్లు(Option_పేర్లు);
    ఒకవేళ (O->hasArgStr())
      Option_పేర్లు.నెట్టు_వెనుక(O->ArgStr);

    తార్కిక Permit_విలువ = O->get_విలువ_Expected_Flag() != cl::విలువ_Disallowed;
    StringRef Flag = Permit_విలువ ? LHS : Arg;
    కొరకు (స్థిరరాశి స్వయంచాలక &Name : Option_పేర్లు) {
      గుర్తులేని దూరం_2 = StringRef(Name).edit_దూరం(
          Flag, /*AllowReplacements=*/నిజం, /*MaxEditDistance=*/ఉత్తమ_దూరం);
      ఒకవేళ (!ఉత్తమ || దూరం_2 < ఉత్తమ_దూరం) {
        ఉత్తమ = O;
        ఉత్తమ_దూరం = దూరం_2;
        ఒకవేళ (RHS.ఖాళీలు() || !Permit_విలువ)
          సమీప_String = std::string(Name);
        లేకపోతే
          సమీప_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  తిరిగి ఉత్తమ;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
స్థిర తార్కిక Comma_Separate_And_కూడు_Occurrence(Option *Handler, గుర్తులేని pos,
                                          StringRef ArgName, StringRef విలువ) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  ఒకవేళ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(విలువ);
    StringRef::కొలత_type Pos = Val.కనుగొను(',');

    అయితే (Pos != StringRef::npos) {
      // Process the portion before the comma.
      ఒకవేళ (Handler->కూడు_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        తిరిగి నిజం;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.కనుగొను(',');
    }

    విలువ = Val;
  }

  తిరిగి Handler->కూడు_Occurrence(pos, ArgName, విలువ);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
స్థిర అంతఃవరుస తార్కిక ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef విలువ, పూర్ణాంకం argc,
                                 స్థిరరాశి అక్షరం *స్థిరరాశి *argv, పూర్ణాంకం &i) {
  // Enforce value requirements
  ఎంపిక (Handler->get_విలువ_Expected_Flag()) {
  స్థితి విలువ_Required:
    ఒకవేళ (!విలువ.దత్తాంశం()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      ఒకవేళ (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_ఉపసర్గ)
        తిరిగి Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ప్రతిజ్ఞా(argv && "null check");
      విలువ = StringRef(argv[++i]);
    }
    ఆపు;
  స్థితి విలువ_Disallowed:
    ఒకవేళ (విలువ.దత్తాంశం())
      తిరిగి Handler->error("does not allow a value! '" + Twine(విలువ) +
                            "' specified.");
    ఆపు;
  స్థితి విలువ_Optional:
    ఆపు;
  }

  తిరిగి Comma_Separate_And_కూడు_Occurrence(Handler, i, ArgName, విలువ);
}

తార్కిక llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, పూర్ణాంకం i) {
  పూర్ణాంకం Dummy = i;
  తిరిగి ProvideOption(Handler, Handler->ArgStr, Arg, 0, శూన్యసూచిక, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
స్థిర Option *కనుగొను_ఉపసర్గ_Option(StringRef Name, size_t &నిడివి,
                                స్థిరరాశి OptionsMapTy &OptionsMap) {
  కొరకు (; !Name.ఖాళీలు(); Name = Name.drop_వెనుక()) {
    Option *O = OptionsMap.lookup(Name);
    ఒకవేళ (O && (O->getFormattingFlag() == cl::ఉపసర్గ ||
              O->getFormattingFlag() == cl::Always_ఉపసర్గ)) {
      నిడివి = Name.కొలత_2();
      తిరిగి O;
    }
  }
  తిరిగి శూన్యసూచిక;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
స్థిర Option *HandlePrefixedOption(StringRef &Arg, StringRef &విలువ,
                                    స్థిరరాశి OptionsMapTy &OptionsMap) {
  ఒకవేళ (Arg.కొలత_2() == 1)
    తిరిగి శూన్యసూచిక;

  size_t నిడివి = 0;
  Option *POpt = కనుగొను_ఉపసర్గ_Option(Arg, నిడివి, OptionsMap);
  ఒకవేళ (!POpt)
    తిరిగి శూన్యసూచిక;

  StringRef Maybe_విలువ =
      (నిడివి < Arg.కొలత_2()) ? Arg.substr(నిడివి) : StringRef();
  Arg = Arg.substr(0, నిడివి);

  // cl::Prefix options do not preserve '=' when used separately.
  ఒకవేళ (POpt->getFormattingFlag() == cl::ఉపసర్గ && Maybe_విలువ.starts_with("="))
    Maybe_విలువ = Maybe_విలువ.drop_front();
  విలువ = Maybe_విలువ;
  తిరిగి POpt;
}

స్థిర తార్కిక Requires_విలువ(స్థిరరాశి Option *O) {
  తిరిగి O->get_అంకె_Occurrences_Flag() == cl::Required ||
         O->get_అంకె_Occurrences_Flag() == cl::One_Or_మరిన్ని;
}

స్థిర తార్కిక Eats_Unbounded_సంఖ్య_యొక్క_విలువలు(స్థిరరాశి Option *O) {
  తిరిగి O->get_అంకె_Occurrences_Flag() == cl::Zero_Or_మరిన్ని ||
         O->get_అంకె_Occurrences_Flag() == cl::One_Or_మరిన్ని;
}

స్థిర తార్కిక isWhitespace(అక్షరం C) {
  తిరిగి C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

స్థిర తార్కిక isWhitespaceOrNull(అక్షరం C) {
  తిరిగి isWhitespace(C) || C == '\0';
}

స్థిర తార్కిక isQuote(అక్షరం C) { తిరిగి C == '\"' || C == '\''; }

శూన్య cl::Tokenize_GNUCommand_పంక్తి(StringRef Src, StringSaver &Saver,
                                Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv,
                                తార్కిక MarkEOLs) {
  SmallString<128> Token;
  తార్కిక లోపల_Token = అబద్ధం;
  కొరకు (size_t I = 0, E = Src.కొలత_2(); I != E; ++I) {
    // Consume runs of whitespace.
    ఒకవేళ (!లోపల_Token) {
      అయితే (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        ఒకవేళ (MarkEOLs && Src[I] == '\n')
          NewArgv.నెట్టు_వెనుక(శూన్యసూచిక);
        ++I;
      }
      ఒకవేళ (I == E)
        ఆపు;
      లోపల_Token = నిజం;
    }

    అక్షరం C = Src[I];

    // Backslash escapes the next character.
    ఒకవేళ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.నెట్టు_వెనుక(Src[I]);
      కొనసాగు;
    }

    // Consume a quoted string.
    ఒకవేళ (isQuote(C)) {
      ++I;
      అయితే (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        ఒకవేళ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.నెట్టు_వెనుక(Src[I]);
        ++I;
      }
      ఒకవేళ (I == E)
        ఆపు;
      కొనసాగు;
    }

    // End the token if this is whitespace.
    ఒకవేళ (isWhitespace(C)) {
      NewArgv.నెట్టు_వెనుక(Saver.save(Token.str()).దత్తాంశం());
      // Mark the end of lines in response files.
      ఒకవేళ (MarkEOLs && C == '\n')
        NewArgv.నెట్టు_వెనుక(శూన్యసూచిక);
      Token.clear();
      లోపల_Token = అబద్ధం;
      కొనసాగు;
    }

    // This is a normal character.  Append it.
    Token.నెట్టు_వెనుక(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  ఒకవేళ (లోపల_Token)
    NewArgv.నెట్టు_వెనుక(Saver.save(Token.str()).దత్తాంశం());
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
స్థిర size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.కొలత_2();
  పూర్ణాంకం Backslash_లెక్క = 0;
  // Skip the backslashes.
  చేయి {
    ++I;
    ++Backslash_లెక్క;
  } అయితే (I != E && Src[I] == '\\');

  తార్కిక FollowedByDoubleQuote = (I != E && Src[I] == '"');
  ఒకవేళ (FollowedByDoubleQuote) {
    Token.append(Backslash_లెక్క / 2, '\\');
    ఒకవేళ (Backslash_లెక్క % 2 == 0)
      తిరిగి I - 1;
    Token.నెట్టు_వెనుక('"');
    తిరిగి I;
  }
  Token.append(Backslash_లెక్క, '\\');
  తిరిగి I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
స్థిర తార్కిక isWindowsSpecialChar(అక్షరం C) {
  తిరిగి isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
స్థిర తార్కిక is_Windows_Special_Char_లోపల_Command_Name(అక్షరం C) {
  తిరిగి isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
స్థిర అంతఃవరుస శూన్య tokenize_Windows_Command_పంక్తి_Impl(
    StringRef Src, StringSaver &Saver, function_ref<శూన్య(StringRef)> కూడు_Token,
    తార్కిక AlwaysCopy, function_ref<శూన్య()> MarkEOL, తార్కిక InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  తార్కిక CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  గణన { INIT, UNQUOTED, QUOTED } State = INIT;

  కొరకు (size_t I = 0, E = Src.కొలత_2(); I < E; ++I) {
    ఎంపిక (State) {
    స్థితి INIT: {
      ప్రతిజ్ఞా(Token.ఖాళీలు() && "token should be empty in initial state");
      // Eat whitespace before a token.
      అయితే (I < E && isWhitespaceOrNull(Src[I])) {
        ఒకవేళ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      ఒకవేళ (I >= E)
        ఆపు;
      size_t ప్రారంభం = I;
      ఒకవేళ (CommandName) {
        అయితే (I < E && !is_Windows_Special_Char_లోపల_Command_Name(Src[I]))
          ++I;
      } లేకపోతే {
        అయితే (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef సాధారణ_Chars = Src.ముక్క(ప్రారంభం, I);
      ఒకవేళ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        కూడు_Token(AlwaysCopy ? Saver.save(సాధారణ_Chars) : సాధారణ_Chars);
        ఒకవేళ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } లేకపోతే {
          CommandName = అబద్ధం;
        }
      } లేకపోతే ఒకవేళ (Src[I] == '\"') {
        Token += సాధారణ_Chars;
        State = QUOTED;
      } లేకపోతే ఒకవేళ (Src[I] == '\\') {
        ప్రతిజ్ఞా(!CommandName && "or else we'd have treated it as a normal char");
        Token += సాధారణ_Chars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } లేకపోతే {
        llvm_unreachable("unexpected special character");
      }
      ఆపు;
    }

    స్థితి UNQUOTED:
      ఒకవేళ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        కూడు_Token(Saver.save(Token.str()));
        Token.clear();
        ఒకవేళ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } లేకపోతే {
          CommandName = అబద్ధం;
        }
        State = INIT;
      } లేకపోతే ఒకవేళ (Src[I] == '\"') {
        State = QUOTED;
      } లేకపోతే ఒకవేళ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } లేకపోతే {
        Token.నెట్టు_వెనుక(Src[I]);
      }
      ఆపు;

    స్థితి QUOTED:
      ఒకవేళ (Src[I] == '\"') {
        ఒకవేళ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.నెట్టు_వెనుక('"');
          ++I;
        } లేకపోతే {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } లేకపోతే ఒకవేళ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } లేకపోతే {
        Token.నెట్టు_వెనుక(Src[I]);
      }
      ఆపు;
    }
  }

  ఒకవేళ (State != INIT)
    కూడు_Token(Saver.save(Token.str()));
}

శూన్య cl::Tokenize_Windows_Command_పంక్తి(StringRef Src, StringSaver &Saver,
                                    Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv,
                                    తార్కిక MarkEOLs) {
  స్వయంచాలక కూడు_Token = [&](StringRef Tok) { NewArgv.నెట్టు_వెనుక(Tok.దత్తాంశం()); };
  స్వయంచాలక OnEOL = [&]() {
    ఒకవేళ (MarkEOLs)
      NewArgv.నెట్టు_వెనుక(శూన్యసూచిక);
  };
  tokenize_Windows_Command_పంక్తి_Impl(Src, Saver, కూడు_Token,
                                 /*AlwaysCopy=*/నిజం, OnEOL, అబద్ధం);
}

శూన్య cl::Tokenize_Windows_Command_పంక్తి_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_సదిశ_Impl<StringRef> &NewArgv) {
  స్వయంచాలక కూడు_Token = [&](StringRef Tok) { NewArgv.నెట్టు_వెనుక(Tok); };
  స్వయంచాలక OnEOL = []() {};
  tokenize_Windows_Command_పంక్తి_Impl(Src, Saver, కూడు_Token, /*AlwaysCopy=*/అబద్ధం,
                                 OnEOL, అబద్ధం);
}

శూన్య cl::Tokenize_Windows_Command_పంక్తి_నిండిన(StringRef Src, StringSaver &Saver,
                                        Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv,
                                        తార్కిక MarkEOLs) {
  స్వయంచాలక కూడు_Token = [&](StringRef Tok) { NewArgv.నెట్టు_వెనుక(Tok.దత్తాంశం()); };
  స్వయంచాలక OnEOL = [&]() {
    ఒకవేళ (MarkEOLs)
      NewArgv.నెట్టు_వెనుక(శూన్యసూచిక);
  };
  tokenize_Windows_Command_పంక్తి_Impl(Src, Saver, కూడు_Token,
                                 /*AlwaysCopy=*/నిజం, OnEOL, నిజం);
}

శూన్య cl::tokenizeConfigFile(StringRef ఆధారం, StringSaver &Saver,
                            Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv,
                            తార్కిక MarkEOLs) {
  కొరకు (స్థిరరాశి అక్షరం *Cur = ఆధారం.begin(); Cur != ఆధారం.చివర();) {
    SmallString<128> పంక్తి;
    // Check for comment line.
    ఒకవేళ (isWhitespace(*Cur)) {
      అయితే (Cur != ఆధారం.చివర() && isWhitespace(*Cur))
        ++Cur;
      కొనసాగు;
    }
    ఒకవేళ (*Cur == '#') {
      అయితే (Cur != ఆధారం.చివర() && *Cur != '\n')
        ++Cur;
      కొనసాగు;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    స్థిరరాశి అక్షరం *ప్రారంభం = Cur;
    స్థిరరాశి అక్షరం *పంక్తి_చివర = శూన్యసూచిక;
    అక్షరం Quote = 0;
    తార్కిక At_Token_ప్రారంభం = నిజం;
    కొరకు (స్థిరరాశి అక్షరం *చివర_2 = ఆధారం.చివర(); Cur != చివర_2; ++Cur) {
      అక్షరం C = *Cur;
      ఒకవేళ (C == '\\') {
        ఒకవేళ (Cur + 1 != చివర_2) {
          ++Cur;
          ఒకవేళ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != చివర_2) && Cur[1] == '\n')) {
            పంక్తి.append(ప్రారంభం, Cur - 1);
            ఒకవేళ (*Cur == '\r')
              ++Cur;
            ప్రారంభం = Cur + 1;
            // Splicing does not introduce a token boundary.
            కొనసాగు;
          }
        }
        At_Token_ప్రారంభం = అబద్ధం;
        కొనసాగు;
      }
      ఒకవేళ (C == '\n')
        ఆపు;
      ఒకవేళ (Quote) {
        ఒకవేళ (C == Quote)
          Quote = 0;
      } లేకపోతే ఒకవేళ (isQuote(C)) {
        Quote = C;
      } లేకపోతే ఒకవేళ (C == '#' && At_Token_ప్రారంభం) {
        పంక్తి_చివర = Cur;
        అయితే (Cur != చివర_2 && *Cur != '\n')
          ++Cur;
        ఆపు;
      }
      At_Token_ప్రారంభం = isWhitespace(C);
    }
    // Tokenize line.
    పంక్తి.append(ప్రారంభం, పంక్తి_చివర ? పంక్తి_చివర : Cur);
    cl::Tokenize_GNUCommand_పంక్తి(పంక్తి, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
స్థిర తార్కిక has_UTF8_Byte_క్రమం_Mark(ArrayRef<అక్షరం> S) {
  తిరిగి (S.కొలత_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
స్థిర శూన్య Expand_పునాది_Paths(StringRef పునాది_మార్గం, StringSaver &Saver,
                            స్థిరరాశి అక్షరం *&Arg) {
  ప్రతిజ్ఞా(sys::మార్గం_2::is_absolute(పునాది_మార్గం));
  constexpr StringLiteral Token("<CFGDIR>");
  స్థిరరాశి StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::కొలత_type ప్రారంభం_Pos = 0;
  కొరకు (StringRef::కొలత_type TokenPos = ArgString.కనుగొను(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.కనుగొను(Token, ప్రారంభం_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    స్థిరరాశి StringRef LHS = ArgString.substr(ప్రారంభం_Pos, TokenPos - ప్రారంభం_Pos);
    ఒకవేళ (ResponseFile.ఖాళీలు())
      ResponseFile = LHS;
    లేకపోతే
      llvm::sys::మార్గం_2::append(ResponseFile, LHS);
    ResponseFile.append(పునాది_మార్గం);
    ప్రారంభం_Pos = TokenPos + Token.కొలత_2();
  }

  ఒకవేళ (!ResponseFile.ఖాళీలు()) {
    // Path-append the remaining arg substring if at least one token appeared.
    స్థిరరాశి StringRef మిగిలిన = ArgString.substr(ప్రారంభం_Pos);
    ఒకవేళ (!మిగిలిన.ఖాళీలు())
      llvm::sys::మార్గం_2::append(ResponseFile, మిగిలిన);
    Arg = Saver.save(ResponseFile.str()).దత్తాంశం();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv) {
  ప్రతిజ్ఞా(sys::మార్గం_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> MemBufOrErr =
      FS->get_బఫరు_For_File(FName);
  ఒకవేళ (!MemBufOrErr) {
    std::error_సంకేతం EC = MemBufOrErr.getError();
    తిరిగి llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.సందేశం_2());
  }
  జ్ఞాపకం_బఫరు &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_బఫరు_ప్రారంభం(), MemBuf.get_బఫరు_కొలత());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<అక్షరం> BufRef(MemBuf.get_బఫరు_ప్రారంభం(), MemBuf.get_బఫరు_చివర());
  std::string UTF8Buf;
  ఒకవేళ (has_UTF16_Byte_క్రమం_Mark(BufRef)) {
    ఒకవేళ (!convert_UTF16_గమ్యం_UTF8_String(BufRef, UTF8Buf))
      తిరిగి llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  లేకపోతే ఒకవేళ (has_UTF8_Byte_క్రమం_Mark(BufRef))
    Str = StringRef(BufRef.దత్తాంశం() + 3, BufRef.కొలత_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  ఒకవేళ (!Relative_పేర్లు && !లోపల_Config_File)
    తిరిగి Error::success();

  StringRef పునాది_మార్గం = llvm::sys::మార్గం_2::మాతృ_మార్గం(FName);
  కొరకు (స్థిరరాశి అక్షరం *&Arg : NewArgv) {
    ఒకవేళ (!Arg)
      కొనసాగు;

    // Substitute <CFGDIR> with the file's base path.
    ఒకవేళ (లోపల_Config_File)
      Expand_పునాది_Paths(పునాది_మార్గం, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    తార్కిక ConfigInclusion = అబద్ధం;
    ఒకవేళ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      ఒకవేళ (!llvm::sys::మార్గం_2::is_relative(FileName))
        కొనసాగు;
    } లేకపోతే ఒకవేళ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = నిజం;
    } లేకపోతే {
      కొనసాగు;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.నెట్టు_వెనుక('@');
    ఒకవేళ (ConfigInclusion && !llvm::sys::మార్గం_2::has_మాతృ_మార్గం(FileName)) {
      SmallString<128> File_మార్గం;
      ఒకవేళ (!కనుగొను_Config_File(FileName, File_మార్గం))
        తిరిగి createStringError(
            std::make_error_సంకేతం(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_మార్గం);
    } లేకపోతే {
      ResponseFile.append(పునాది_మార్గం);
      llvm::sys::మార్గం_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).దత్తాంశం();
  }
  తిరిగి Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &Argv) {
  నిర్మాణం Response_File_రికార్డు {
    std::string File;
    size_t చివర_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_సదిశ<Response_File_రికార్డు, 3> File_దొంతర;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_దొంతర.నెట్టు_వెనుక({"", Argv.కొలత_2()});

  // Don't cache Argv.size() because it can change.
  కొరకు (గుర్తులేని I = 0; I != Argv.కొలత_2();) {
    అయితే (I == File_దొంతర.వెనుక().చివర_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_దొంతర.తీయి_వెనుక();
    }

    స్థిరరాశి అక్షరం *Arg = Argv[I];
    // Check if it is an EOL marker
    ఒకవేళ (Arg == శూన్యసూచిక) {
      ++I;
      కొనసాగు;
    }

    ఒకవేళ (Arg[0] != '@') {
      ++I;
      కొనసాగు;
    }

    స్థిరరాశి అక్షరం *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    ఒకవేళ (llvm::sys::మార్గం_2::is_relative(FName)) {
      ఒకవేళ (CurrentDir.ఖాళీలు()) {
        ఒకవేళ (స్వయంచాలక CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } లేకపోతే {
          తిరిగి createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } లేకపోతే {
        CurrDir = CurrentDir;
      }
      llvm::sys::మార్గం_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::స్థితి_2> Res = FS->స్థితి_3(FName);
    ఒకవేళ (!Res || !Res->exists()) {
      std::error_సంకేతం EC = Res.getError();
      ఒకవేళ (!లోపల_Config_File) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        ఒకవేళ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          కొనసాగు;
        }
      }
      ఒకవేళ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      తిరిగి createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.సందేశం_2());
    }
    స్థిరరాశి llvm::vfs::స్థితి_2 &File_స్థితి = Res.get();

    స్వయంచాలక IsEquivalent =
        [File_స్థితి, ఇది](స్థిరరాశి Response_File_రికార్డు &RFile) -> ErrorOr<తార్కిక> {
      ErrorOr<llvm::vfs::స్థితి_2> RHS = FS->స్థితి_3(RFile.File);
      ఒకవేళ (!RHS)
        తిరిగి RHS.getError();
      తిరిగి File_స్థితి.equivalent(*RHS);
    };

    // Check for recursive response files.
    కొరకు (స్థిరరాశి స్వయంచాలక &F : drop_begin(File_దొంతర)) {
      ఒకవేళ (ErrorOr<తార్కిక> R = IsEquivalent(F)) {
        ఒకవేళ (R.get())
          తిరిగి createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } లేకపోతే {
        తిరిగి createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    Small_సదిశ<స్థిరరాశి అక్షరం *, 0> ExpandedArgv;
    ఒకవేళ (Error Err = expandResponseFile(FName, ExpandedArgv))
      తిరిగి Err;

    కొరకు (Response_File_రికార్డు &రికార్డు : File_దొంతర) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      రికార్డు.చివర_2 += ExpandedArgv.కొలత_2() - 1;
    }

    File_దొంతర.నెట్టు_వెనుక({FName, I + ExpandedArgv.కొలత_2()});
    Argv.erase(Argv.begin() + I);
    Argv.చొప్పించు(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.చివర());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ప్రతిజ్ఞా(File_దొంతర.కొలత_2() > 0 && Argv.కొలత_2() == File_దొంతర.వెనుక().చివర_2);
  తిరిగి Error::success();
}

తార్కిక cl::expandResponseFiles(పూర్ణాంకం Argc, స్థిరరాశి అక్షరం *స్థిరరాశి *Argv,
                             స్థిరరాశి అక్షరం *EnvVar, StringSaver &Saver,
                             Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &NewArgv) {
#నిర్వచితమైతే _WIN32
  స్వయంచాలక Tokenize = cl::Tokenize_Windows_Command_పంక్తి;
#లేకపోతే
  స్వయంచాలక Tokenize = cl::Tokenize_GNUCommand_పంక్తి;
#అయితే_ముగింపు
  // The environment variable specifies initial options.
  ఒకవేళ (EnvVar)
    ఒకవేళ (std::optional<std::string> Env_విలువ = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_విలువ, Saver, NewArgv, /*MarkEOLs=*/అబద్ధం);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  ఒకవేళ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << గమ్యం_String(std::move(Err)) << '\n';
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

తార్కిక cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  ఒకవేళ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << గమ్యం_String(std::move(Err)) << '\n';
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

తార్కిక ExpansionContext::కనుగొను_Config_File(StringRef FileName,
                                      Small_సదిశ_Impl<అక్షరం> &File_మార్గం) {
  SmallString<128> Cfg_File_మార్గం;
  స్థిరరాశి స్వయంచాలక FileExists = [ఇది](SmallString<128> మార్గం) -> తార్కిక {
    స్వయంచాలక స్థితి_2 = FS->స్థితి_3(మార్గం);
    తిరిగి స్థితి_2 &&
           స్థితి_2->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  ఒకవేళ (llvm::sys::మార్గం_2::has_మాతృ_మార్గం(FileName)) {
    Cfg_File_మార్గం = FileName;
    ఒకవేళ (llvm::sys::మార్గం_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_మార్గం))
      తిరిగి అబద్ధం;
    ఒకవేళ (!FileExists(Cfg_File_మార్గం))
      తిరిగి అబద్ధం;
    File_మార్గం.కేటాయించు(Cfg_File_మార్గం.begin(), Cfg_File_మార్గం.చివర());
    తిరిగి నిజం;
  }

  // Look for the file in search directories.
  కొరకు (స్థిరరాశి StringRef &Dir : శోధన_Dirs) {
    ఒకవేళ (Dir.ఖాళీలు())
      కొనసాగు;
    Cfg_File_మార్గం.కేటాయించు(Dir);
    llvm::sys::మార్గం_2::append(Cfg_File_మార్గం, FileName);
    llvm::sys::మార్గం_2::native(Cfg_File_మార్గం);
    ఒకవేళ (FileExists(Cfg_File_మార్గం)) {
      File_మార్గం.కేటాయించు(Cfg_File_మార్గం.begin(), Cfg_File_మార్గం.చివర());
      తిరిగి నిజం;
    }
  }

  తిరిగి అబద్ధం;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_సదిశ_Impl<స్థిరరాశి అక్షరం *> &Argv) {
  SmallString<128> Abs_మార్గం;
  ఒకవేళ (sys::మార్గం_2::is_relative(CfgFile)) {
    Abs_మార్గం.కేటాయించు(CfgFile);
    ఒకవేళ (std::error_సంకేతం EC = FS->makeAbsolute(Abs_మార్గం))
      తిరిగి make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_మార్గం.str();
  }
  లోపల_Config_File = నిజం;
  Relative_పేర్లు = నిజం;
  ఒకవేళ (Error Err = expandResponseFile(CfgFile, Argv))
    తిరిగి Err;
  తిరిగి expandResponseFiles(Argv);
}

స్థిర శూన్య initCommonOptions();
స్థిర తార్కిక parse_Command_పంక్తి_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        స్థిరరాశి అక్షరం *EnvVar) {
  initCommonOptions();
  Small_సదిశ<స్థిరరాశి అక్షరం *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.నెట్టు_వెనుక(వాదనలు[0]);

  // Parse options from environment variable.
  ఒకవేళ (EnvVar) {
    ఒకవేళ (std::optional<std::string> Env_విలువ =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_పంక్తి(*Env_విలువ, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, వాదనలు.drop_front());
  పూర్ణాంకం NewArgc = static_cast<పూర్ణాంకం>(NewArgv.కొలత_2());

  // Parse all options.
  తిరిగి globalParser().Parse_Command_పంక్తి_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

తార్కిక cl::Parse_Command_పంక్తి_Options(పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, స్థిరరాశి అక్షరం *EnvVar) {
  తిరిగి parse_Command_పంక్తి_Options_Impl(శూన్యసూచిక, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

తార్కిక cl::parse_Command_పంక్తి_Options(LibraryOptionsContext &Context,
                                 ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, స్థిరరాశి అక్షరం *EnvVar) {
  తిరిగి parse_Command_పంక్తి_Options_Impl(&Context, వాదనలు, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
శూన్య Command_పంక్తి_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  కొరకు (స్వయంచాలక *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_సదిశ<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.కొలత_2());
    కొరకు (స్వయంచాలక &O : SC->OptionsMap)
      Opts.నెట్టు_వెనుక(O.రెండవ_2);
    కొరకు (Option *O : Opts)
      O->reset();
    కొరకు (Option *O : SC->PositionalOpts)
      O->reset();
    ఒకవేళ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  కొరకు (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

తార్కిక Command_పంక్తి_Parser::Parse_Command_పంక్తి_Options(
    పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ప్రతిజ్ఞా(hasOptions() && "No options specified!");

  కార్యక్రమం_Overview = Overview;
  తార్కిక IgnoreErrors = Errs;
  ఒకవేళ (!Errs)
    Errs = &errs();
  ఒకవేళ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  తార్కిక ErrorParsing = అబద్ధం;

  // Expand response files.
  Small_సదిశ<స్థిరరాశి అక్షరం *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#నిర్వచితమైతే _WIN32
  స్వయంచాలక Tokenize = cl::Tokenize_Windows_Command_పంక్తి;
#లేకపోతే
  స్వయంచాలక Tokenize = cl::Tokenize_GNUCommand_పంక్తి;
#అయితే_ముగింపు
  ExpansionContext ECtx(A, Tokenize, VFS);
  ఒకవేళ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << గమ్యం_String(std::move(Err)) << '\n';
    తిరిగి అబద్ధం;
  }
  argv = &newArgv[0];
  argc = static_cast<పూర్ణాంకం>(newArgv.కొలత_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  కార్యక్రమం_Name = std::string(sys::మార్గం_2::filename(StringRef(argv[0])));
  సూచీ_Library_Options();

  // Check out the positional arguments to collect information about them.
  గుర్తులేని అంకె_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  తార్కిక HasUnlimitedPositionals = అబద్ధం;

  పూర్ణాంకం మొదటి_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_శిఖరం_Level();
  std::string సమీప_Sub_Command_String;
  తార్కిక MaybeNamedSubCommand =
      argc >= 2 && argv[మొదటి_Arg][0] != '-' && hasNamedSubCommands();
  ఒకవేళ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[మొదటి_Arg]), సమీప_Sub_Command_String);
    ఒకవేళ (ChosenSubCommand != &SubCommand::get_శిఖరం_Level())
      మొదటి_Arg = 2;
  }
  globalParser().చురుకైన_Sub_Command = ChosenSubCommand;

  ప్రతిజ్ఞా(ChosenSubCommand);
  స్వయంచాలక &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  స్వయంచాలక &PositionalOpts = ChosenSubCommand->PositionalOpts;
  స్వయంచాలక &OptionsMap = ChosenSubCommand->OptionsMap;

  ఒకవేళ (ConsumeAfterOpt) {
    ప్రతిజ్ఞా(PositionalOpts.కొలత_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  ఒకవేళ (!PositionalOpts.ఖాళీలు()) {

    // Calculate how many positional values are _required_.
    తార్కిక Unbounded_దొరికింది = అబద్ధం;
    కొరకు (size_t i = 0, e = PositionalOpts.కొలత_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      ఒకవేళ (Requires_విలువ(Opt))
        ++అంకె_Positional_Required;
      లేకపోతే ఒకవేళ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        ఒకవేళ (PositionalOpts.కొలత_2() > 1) {
          ఒకవేళ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = నిజం;
        }
      } లేకపోతే ఒకవేళ (Unbounded_దొరికింది && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        ఒకవేళ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << కార్యక్రమం_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.కొలత_2();
        ErrorParsing = నిజం;
      }
      Unbounded_దొరికింది |= Eats_Unbounded_సంఖ్య_యొక్క_విలువలు(Opt);
    }
    HasUnlimitedPositionals = Unbounded_దొరికింది || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_సదిశ<std::pair<StringRef, గుర్తులేని>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *చురుకైన_Positional_Arg = శూన్యసూచిక;

  // Loop over all of the arguments... processing them.
  తార్కిక Dash_Dash_దొరికింది = అబద్ధం; // Have we read '--'?
  కొరకు (పూర్ణాంకం i = మొదటి_Arg; i < argc; ++i) {
    Option *Handler = శూన్యసూచిక;
    std::string సమీప_Handler_String;
    StringRef విలువ;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    ఒకవేళ (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_దొరికింది) {
      // Positional argument!
      ఒకవేళ (చురుకైన_Positional_Arg) {
        ProvidePositionalOption(చురుకైన_Positional_Arg, StringRef(argv[i]), i);
        కొనసాగు; // We are done!
      }

      ఒకవేళ (!PositionalOpts.ఖాళీలు()) {
        PositionalVals.నెట్టు_వెనుక(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        ఒకవేళ (PositionalVals.కొలత_2() >= అంకె_Positional_Required && ConsumeAfterOpt) {
          కొరకు (++i; i < argc; ++i)
            PositionalVals.నెట్టు_వెనుక(std::make_pair(StringRef(argv[i]), i));
          ఆపు; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        కొనసాగు;
      }
    } లేకపోతే ఒకవేళ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_దొరికింది) {
      Dash_Dash_దొరికింది = నిజం; // This is the mythical "--"?
      కొనసాగు;             // Don't try to process it as an argument itself.
    } లేకపోతే ఒకవేళ (చురుకైన_Positional_Arg &&
               (చురుకైన_Positional_Arg->getMiscFlags() & Positional_Eats_వాదనలు)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, విలువ);
      ఒకవేళ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(చురుకైన_Positional_Arg, StringRef(argv[i]), i);
        కొనసాగు; // We are done!
      }
    } లేకపోతే { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, విలువ);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      ఒకవేళ (!Handler && ChosenSubCommand != &SubCommand::get_శిఖరం_Level())
        Handler = LookupOption(SubCommand::get_శిఖరం_Level(), ArgName, విలువ);

      ఒకవేళ (!Handler) {
        ఒకవేళ (LibraryOptions *L = lookupLibraryOption(ArgName.విభజించు('=').మొదటి_2)) {
          ArrayRef<స్థిరరాశి అక్షరం *> Rest(argv + i, argc - i);
          గుర్తులేని N;
          ఒకవేళ (Error E = Context && L->is_చొప్పున_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << కార్యక్రమం_Name << ": " << గమ్యం_String(std::move(E)) << '\n';
            ErrorParsing = నిజం;
          }
          i += N - 1;
          కొనసాగు;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      ఒకవేళ (!Handler)
        Handler = HandlePrefixedOption(ArgName, విలువ, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      ఒకవేళ (!Handler)
        Lookup_సమీప_Option(ArgName, OptionsMap, సమీప_Handler_String);
    }

    ఒకవేళ (!Handler) {
      స్వయంచాలక Report_Unknown_వాదం = [&](తార్కిక IsArg,
                                       StringRef సమీప_వాదం_Name) {
        *Errs << కార్యక్రమం_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        ఒకవేళ (సమీప_వాదం_Name.ఖాళీలు())
          తిరిగి;

        *Errs << కార్యక్రమం_Name << ": Did you mean '";
        ఒకవేళ (IsArg)
          *Errs << PrintArg(సమీప_వాదం_Name, 0);
        లేకపోతే
          *Errs << సమీప_వాదం_Name;
        *Errs << "'?\n";
      };

      ఒకవేళ (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_వాదం(/*IsArg=*/నిజం, సమీప_Handler_String);
      లేకపోతే
        Report_Unknown_వాదం(/*IsArg=*/అబద్ధం, సమీప_Sub_Command_String);

      ErrorParsing = నిజం;
      కొనసాగు;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    ఒకవేళ (Handler->getFormattingFlag() == cl::Positional) {
      ఒకవేళ ((Handler->getMiscFlags() & Positional_Eats_వాదనలు) && !విలువ.ఖాళీలు()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = నిజం;
      }
      చురుకైన_Positional_Arg = Handler;
    }
    లేకపోతే
      ErrorParsing |= ProvideOption(Handler, ArgName, విలువ, argc, argv, i);
  }

  // Check and handle positional arguments now...
  ఒకవేళ (అంకె_Positional_Required > PositionalVals.కొలత_2()) {
      *Errs << కార్యక్రమం_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << అంకె_Positional_Required
             << " positional argument" << (అంకె_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = నిజం;
  } లేకపోతే ఒకవేళ (!HasUnlimitedPositionals &&
             PositionalVals.కొలత_2() > PositionalOpts.కొలత_2()) {
    *Errs << కార్యక్రమం_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.కొలత_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = నిజం;

  } లేకపోతే ఒకవేళ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    గుర్తులేని ValNo = 0, అంకె_Vals = static_cast<గుర్తులేని>(PositionalVals.కొలత_2());
    కొరకు (Option *Opt : PositionalOpts) {
      ఒకవేళ (Requires_విలువ(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].మొదటి_2,
                                PositionalVals[ValNo].రెండవ_2);
        ValNo++;
        --అంకె_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      తార్కిక పూర్తి = Opt->get_అంకె_Occurrences_Flag() == cl::Required;
      అయితే (అంకె_Vals - ValNo > అంకె_Positional_Required && !పూర్తి) {
        ఎంపిక (Opt->get_అంకె_Occurrences_Flag()) {
        స్థితి cl::Optional:
          పూర్తి = నిజం; // Optional arguments want _at most_ one value
          [[fallthrough]];
        స్థితి cl::Zero_Or_మరిన్ని: // Zero or more will take all they can get...
        స్థితి cl::One_Or_మరిన్ని:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].మొదటి_2,
                                  PositionalVals[ValNo].రెండవ_2);
          ValNo++;
          ఆపు;
        అప్రమేయ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } లేకపోతే {
    ప్రతిజ్ఞా(ConsumeAfterOpt && అంకె_Positional_Required <= PositionalVals.కొలత_2());
    గుర్తులేని ValNo = 0;
    కొరకు (Option *Opt : PositionalOpts)
      ఒకవేళ (Requires_విలువ(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].మొదటి_2, PositionalVals[ValNo].రెండవ_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    ఒకవేళ (PositionalOpts.కొలత_2() == 1 && ValNo == 0 && !PositionalVals.ఖాళీలు()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].మొదటి_2,
                                              PositionalVals[ValNo].రెండవ_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    కొరకు (; ValNo != PositionalVals.కొలత_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].మొదటి_2,
                                  PositionalVals[ValNo].రెండవ_2);
  }

  // Loop over args and make sure all required args are specified!
  కొరకు (స్థిరరాశి స్వయంచాలక &Opt : OptionsMap) {
    ఎంపిక (Opt.రెండవ_2->get_అంకె_Occurrences_Flag()) {
    స్థితి Required:
    స్థితి One_Or_మరిన్ని:
      ఒకవేళ (Opt.రెండవ_2->get_అంకె_Occurrences() == 0) {
        Opt.రెండవ_2->error("must be specified at least once!");
        ErrorParsing = నిజం;
      }
      [[fallthrough]];
    అప్రమేయ:
      ఆపు;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             కొరకు (పూర్ణాంకం i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  మరిన్ని_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  ఒకవేళ (ErrorParsing) {
    ఒకవేళ (!IgnoreErrors)
      నిష్క్రమించు(1);
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

తార్కిక Option::error(స్థిరరాశి Twine &సందేశం_3, StringRef ArgName, raw_ostream &Errs) {
  ఒకవేళ (!ArgName.దత్తాంశం())
    ArgName = ArgStr;
  ఒకవేళ (ArgName.ఖాళీలు())
    Errs << HelpStr; // Be nice for positional arguments
  లేకపోతే
    Errs << globalParser().కార్యక్రమం_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << సందేశం_3 << "\n";
  తిరిగి నిజం;
}

తార్కిక Option::కూడు_Occurrence(గుర్తులేని pos, StringRef ArgName, StringRef విలువ) {
  ++అంకె_Occurrences;
  తిరిగి handleOccurrence(pos, ArgName, విలువ);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
స్థిర StringRef get_విలువ_Str(స్థిరరాశి Option &O, StringRef Default_సందేశం) {
  ఒకవేళ (O.విలువ_Str.ఖాళీలు())
    తిరిగి Default_సందేశం;
  తిరిగి O.విలువ_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_వెడల్పు() స్థిరరాశి {
  తిరిగి arg_Plus_Prefixes_కొలత(ArgStr);
}

శూన్య Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t మొదటి_పంక్తి_Indented_By) {
  ప్రతిజ్ఞా(Indent >= మొదటి_పంక్తి_Indented_By);
  std::pair<StringRef, StringRef> విభజించు_2 = HelpStr.విభజించు('\n');
  outs().indent(Indent - మొదటి_పంక్తి_Indented_By)
      << Arg_Help_ఉపసర్గ << విభజించు_2.మొదటి_2 << "\n";
  అయితే (!విభజించు_2.రెండవ_2.ఖాళీలు()) {
    విభజించు_2 = విభజించు_2.రెండవ_2.విభజించు('\n');
    outs().indent(Indent) << విభజించు_2.మొదటి_2 << "\n";
  }
}

శూన్య Option::printEnumValHelpStr(StringRef HelpStr, size_t పునాది_Indent,
                                 size_t మొదటి_పంక్తి_Indented_By) {
  స్థిరరాశి StringRef Val_Help_ఉపసర్గ = "  ";
  ప్రతిజ్ఞా(పునాది_Indent >= మొదటి_పంక్తి_Indented_By);
  std::pair<StringRef, StringRef> విభజించు_2 = HelpStr.విభజించు('\n');
  outs().indent(పునాది_Indent - మొదటి_పంక్తి_Indented_By)
      << Arg_Help_ఉపసర్గ << Val_Help_ఉపసర్గ << విభజించు_2.మొదటి_2 << "\n";
  అయితే (!విభజించు_2.రెండవ_2.ఖాళీలు()) {
    విభజించు_2 = విభజించు_2.రెండవ_2.విభజించు('\n');
    outs().indent(పునాది_Indent + Val_Help_ఉపసర్గ.కొలత_2()) << విభజించు_2.మొదటి_2 << "\n";
  }
}

// Print out the option for the alias.
శూన్య alias::printOptionInfo(size_t Global_వెడల్పు) స్థిరరాశి {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_వెడల్పు, arg_Plus_Prefixes_కొలత(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_వెడల్పు(స్థిరరాశి Option &O) స్థిరరాశి {
  size_t Len = arg_Plus_Prefixes_కొలత(O.ArgStr);
  స్వయంచాలక ValName = get_విలువ_Name();
  ఒకవేళ (!ValName.ఖాళీలు()) {
    size_t FormattingLen = 3;
    ఒకవేళ (O.getMiscFlags() & Positional_Eats_వాదనలు)
      FormattingLen = 6;
    Len += get_విలువ_Str(O, ValName).కొలత_2() + FormattingLen;
  }

  తిరిగి Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
శూన్య basic_parser_impl::printOptionInfo(స్థిరరాశి Option &O,
                                        size_t Global_వెడల్పు) స్థిరరాశి {
  outs() << PrintArg(O.ArgStr);

  స్వయంచాలక ValName = get_విలువ_Name();
  ఒకవేళ (!ValName.ఖాళీలు()) {
    ఒకవేళ (O.getMiscFlags() & Positional_Eats_వాదనలు) {
      outs() << " <" << get_విలువ_Str(O, ValName) << ">...";
    } లేకపోతే ఒకవేళ (O.get_విలువ_Expected_Flag() == విలువ_Optional)
      outs() << "[=<" << get_విలువ_Str(O, ValName) << ">]";
    లేకపోతే {
      outs() << (O.ArgStr.కొలత_2() == 1 ? " <" : "=<") << get_విలువ_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_వెడల్పు, get_Option_వెడల్పు(O));
}

శూన్య basic_parser_impl::printOptionName(స్థిరరాశి Option &O,
                                        size_t Global_వెడల్పు) స్థిరరాశి {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_వెడల్పు - O.ArgStr.కొలత_2());
}

// parser<bool> implementation
//
తార్కిక parser<తార్కిక>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         తార్కిక &విలువ) {
  తిరిగి parseBool<తార్కిక, నిజం, అబద్ధం>(O, ArgName, Arg, విలువ);
}

// parser<boolOrDefault> implementation
//
తార్కిక parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &విలువ) {
  తిరిగి parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, విలువ);
}

// parser<FixedOrScalableQuantity> implementation
//
మూస <రకం_పేరు FixedOrScalableQuantityT>
స్థిర తార్కిక parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef విలువ_రకం,
                                         FixedOrScalableQuantityT &విలువ) {
  ఉపయోగించి ScalarTy = రకం_పేరు FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_విలువ;
  ఒకవేళ (!Arg.getAsInteger(0, Min_విలువ)) {
    విలువ = FixedOrScalableQuantityT::getFixed(Min_విలువ);
    తిరిగి అబద్ధం;
  }

  StringRef Remainder = Arg;
  ఒకవేళ (!Remainder.consume_front("vscale"))
    తిరిగి O.error("'" + Arg + "' value invalid for " + విలువ_రకం +
                   " argument!");

  Remainder = Remainder.ltrim();
  ఒకవేళ (!Remainder.consume_front('x'))
    తిరిగి O.error("'" + Arg + "' value invalid for " + విలువ_రకం +
                   " argument!");

  Remainder = Remainder.ltrim();
  ఒకవేళ (Remainder.getAsInteger(0, Min_విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for " + విలువ_రకం +
                   " argument!");

  విలువ = FixedOrScalableQuantityT::getScalable(Min_విలువ);
  తిరిగి అబద్ధం;
}

// parser<int> implementation
//
తార్కిక parser<పూర్ణాంకం>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        పూర్ణాంకం &విలువ) {
  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for integer argument!");
  తిరిగి అబద్ధం;
}

// parser<long> implementation
//
తార్కిక parser<దీర్ఘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         దీర్ఘ &విలువ) {
  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for long argument!");
  తిరిగి అబద్ధం;
}

// parser<long long> implementation
//
తార్కిక parser<దీర్ఘ దీర్ఘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              దీర్ఘ దీర్ఘ &విలువ) {
  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for llong argument!");
  తిరిగి అబద్ధం;
}

// parser<unsigned> implementation
//
తార్కిక parser<గుర్తులేని>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             గుర్తులేని &విలువ) {

  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for uint argument!");
  తిరిగి అబద్ధం;
}

// parser<unsigned long> implementation
//
తార్కిక parser<గుర్తులేని దీర్ఘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  గుర్తులేని దీర్ఘ &విలువ) {

  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for ulong argument!");
  తిరిగి అబద్ధం;
}

// parser<unsigned long long> implementation
//
తార్కిక parser<గుర్తులేని దీర్ఘ దీర్ఘ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       గుర్తులేని దీర్ఘ దీర్ఘ &విలువ) {

  ఒకవేళ (Arg.getAsInteger(0, విలువ))
    తిరిగి O.error("'" + Arg + "' value invalid for ullong argument!");
  తిరిగి అబద్ధం;
}

// parser<ElementCount> implementation
//
తార్కిక parser<Element_లెక్క>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_లెక్క &విలువ) {
  తిరిగి parseFixedOrScalableQuantity(O, Arg, get_విలువ_Name(), విలువ);
}

// parser<double>/parser<float> implementation
//
స్థిర తార్కిక parseDouble(Option &O, StringRef Arg, ద్విగుణ &విలువ) {
  ఒకవేళ (గమ్యం_float(Arg, విలువ))
    తిరిగి అబద్ధం;
  తిరిగి O.error("'" + Arg + "' value invalid for floating point argument!");
}

తార్కిక parser<ద్విగుణ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ద్విగుణ &Val) {
  తిరిగి parseDouble(O, Arg, Val);
}

తార్కిక parser<దశాంశం>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          దశాంశం &Val) {
  ద్విగుణ dVal;
  ఒకవేళ (parseDouble(O, Arg, dVal))
    తిరిగి నిజం;
  Val = (దశాంశం)dVal;
  తిరిగి అబద్ధం;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
గుర్తులేని generic_parser_పునాది::కనుగొను_Option(StringRef Name) {
  గుర్తులేని e = get_అంకె_Options();

  కొరకు (గుర్తులేని i = 0; i != e; ++i) {
    ఒకవేళ (getOption(i) == Name)
      తిరిగి i;
  }
  తిరిగి e;
}

స్థిర StringRef Eq_విలువ = "=<value>";
స్థిర StringRef ఖాళీలు_Option = "<empty>";
స్థిర StringRef Option_ఉపసర్గ = "    =";
స్థిర size_t get_Option_Prefixes_కొలత() {
  తిరిగి Option_ఉపసర్గ.కొలత_2() + Arg_Help_ఉపసర్గ.కొలత_2();
}

స్థిర తార్కిక shouldPrintOption(StringRef Name, StringRef Description,
                              స్థిరరాశి Option &O) {
  తిరిగి O.get_విలువ_Expected_Flag() != విలువ_Optional || !Name.ఖాళీలు() ||
         !Description.ఖాళీలు();
}

// Return the width of the option tag for printing...
size_t generic_parser_పునాది::get_Option_వెడల్పు(స్థిరరాశి Option &O) స్థిరరాశి {
  ఒకవేళ (O.hasArgStr()) {
    size_t కొలత =
        arg_Plus_Prefixes_కొలత(O.ArgStr) + Eq_విలువ.కొలత_2();
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      ఒకవేళ (!shouldPrintOption(Name, getDescription(i), O))
        కొనసాగు;
      size_t Name_కొలత = Name.ఖాళీలు() ? ఖాళీలు_Option.కొలత_2() : Name.కొలత_2();
      కొలత = std::max(కొలత, Name_కొలత + get_Option_Prefixes_కొలత());
    }
    తిరిగి కొలత;
  } లేకపోతే {
    size_t పునాది_కొలత = 0;
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Options(); i != e; ++i)
      పునాది_కొలత = std::max(పునాది_కొలత, getOption(i).కొలత_2() + 8);
    తిరిగి పునాది_కొలత;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
శూన్య generic_parser_పునాది::printOptionInfo(స్థిరరాశి Option &O,
                                          size_t Global_వెడల్పు) స్థిరరాశి {
  ఒకవేళ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    ఒకవేళ (O.get_విలువ_Expected_Flag() == విలువ_Optional) {
      కొరకు (గుర్తులేని i = 0, e = get_అంకె_Options(); i != e; ++i) {
        ఒకవేళ (getOption(i).ఖాళీలు()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_వెడల్పు,
                               arg_Plus_Prefixes_కొలత(O.ArgStr));
          ఆపు;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_విలువ;
    Option::printHelpStr(O.HelpStr, Global_వెడల్పు,
                         Eq_విలువ.కొలత_2() +
                             arg_Plus_Prefixes_కొలత(O.ArgStr));
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      ఒకవేళ (!shouldPrintOption(OptionName, Description, O))
        కొనసాగు;
      size_t మొదటి_పంక్తి_Indent = OptionName.కొలత_2() + get_Option_Prefixes_కొలత();
      outs() << Option_ఉపసర్గ << OptionName;
      ఒకవేళ (OptionName.ఖాళీలు()) {
        outs() << ఖాళీలు_Option;
        ప్రతిజ్ఞా(మొదటి_పంక్తి_Indent >= ఖాళీలు_Option.కొలత_2());
        మొదటి_పంక్తి_Indent += ఖాళీలు_Option.కొలత_2();
      }
      ఒకవేళ (!Description.ఖాళీలు())
        Option::printEnumValHelpStr(Description, Global_వెడల్పు, మొదటి_పంక్తి_Indent);
      లేకపోతే
        outs() << '\n';
    }
  } లేకపోతే {
    ఒకవేళ (!O.HelpStr.ఖాళీలు())
      outs() << "  " << O.HelpStr << '\n';
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_వెడల్పు, Option.కొలత_2() + 8);
    }
  }
}

స్థిర స్థిరరాశి size_t Max_Opt_వెడల్పు = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
శూన్య generic_parser_పునాది::printGenericOptionDiff(
    స్థిరరాశి Option &O, స్థిరరాశి Generic_Option_విలువ &విలువ,
    స్థిరరాశి Generic_Option_విలువ &Default, size_t Global_వెడల్పు) స్థిరరాశి {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_వెడల్పు - O.ArgStr.కొలత_2());

  గుర్తులేని అంకె_Opts = get_అంకె_Options();
  కొరకు (గుర్తులేని i = 0; i != అంకె_Opts; ++i) {
    ఒకవేళ (!విలువ.compare(get_Option_విలువ(i)))
      కొనసాగు;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).కొలత_2();
    size_t అంకె_Spaces = Max_Opt_వెడల్పు > L ? Max_Opt_వెడల్పు - L : 0;
    outs().indent(అంకె_Spaces) << " (default: ";
    కొరకు (గుర్తులేని j = 0; j != అంకె_Opts; ++j) {
      ఒకవేళ (!Default.compare(get_Option_విలువ(j)))
        కొనసాగు;
      outs() << getOption(j);
      ఆపు;
    }
    outs() << ")\n";
    తిరిగి;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
నామాకాశ llvm {
నామాకాశ cl {
స్థిర raw_ostream &క్రియాకారకం<<(raw_ostream &OS, boolOrDefault V) {
  తిరిగి OS << static_cast<పూర్ణాంకం>(V);
}
} // namespace cl
} // namespace llvm

#నిర్వచించు PRINT_OPT_DIFF(T)                                                      \
  శూన్య parser<T>::printOptionDiff(స్థిరరాశి Option &O, T V, Option_విలువ<T> D,      \
                                  size_t Global_వెడల్పు) స్థిరరాశి {                  \
    printOptionName(O, Global_వెడల్పు);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t అంకె_Spaces =                                                         \
        Max_Opt_వెడల్పు > Str.కొలత_2() ? Max_Opt_వెడల్పు - Str.కొలత_2() : 0;               \
    outs().indent(అంకె_Spaces) << " (default: ";                                 \
    ఒకవేళ (D.has_విలువ())                                                          \
      outs() << D.get_విలువ();                                                  \
    లేకపోతే                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(తార్కిక)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(పూర్ణాంకం)
PRINT_OPT_DIFF(దీర్ఘ)
PRINT_OPT_DIFF(దీర్ఘ దీర్ఘ)
PRINT_OPT_DIFF(గుర్తులేని)
PRINT_OPT_DIFF(గుర్తులేని దీర్ఘ)
PRINT_OPT_DIFF(గుర్తులేని దీర్ఘ దీర్ఘ)
PRINT_OPT_DIFF(ద్విగుణ)
PRINT_OPT_DIFF(దశాంశం)
PRINT_OPT_DIFF(అక్షరం)
PRINT_OPT_DIFF(Element_లెక్క)

శూన్య parser<std::string>::printOptionDiff(స్థిరరాశి Option &O, StringRef V,
                                          స్థిరరాశి Option_విలువ<std::string> &D,
                                          size_t Global_వెడల్పు) స్థిరరాశి {
  printOptionName(O, Global_వెడల్పు);
  outs() << "= " << V;
  size_t అంకె_Spaces = Max_Opt_వెడల్పు > V.కొలత_2() ? Max_Opt_వెడల్పు - V.కొలత_2() : 0;
  outs().indent(అంకె_Spaces) << " (default: ";
  ఒకవేళ (D.has_విలువ())
    outs() << D.get_విలువ();
  లేకపోతే
    outs() << "*no default*";
  outs() << ")\n";
}

శూన్య parser<std::optional<std::string>>::printOptionDiff(
    స్థిరరాశి Option &O, std::optional<StringRef> V,
    స్థిరరాశి Option_విలువ<std::optional<std::string>> &D,
    size_t Global_వెడల్పు) స్థిరరాశి {
  printOptionName(O, Global_వెడల్పు);
  outs() << "= " << V;
  size_t VSize = V.has_విలువ_2() ? V.విలువ_2().కొలత_2() : 0;
  size_t అంకె_Spaces = Max_Opt_వెడల్పు > VSize ? Max_Opt_వెడల్పు - VSize : 0;
  outs().indent(అంకె_Spaces) << " (default: ";
  ఒకవేళ (D.has_విలువ() && D.get_విలువ().has_విలువ_2())
    outs() << D.get_విలువ();
  లేకపోతే
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
శూన్య basic_parser_impl::print_Option_No_విలువ(స్థిరరాశి Option &O,
                                           size_t Global_వెడల్పు) స్థిరరాశి {
  printOptionName(O, Global_వెడల్పు);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

స్థిర పూర్ణాంకం OptNameCompare(స్థిరరాశి std::pair<స్థిరరాశి అక్షరం *, Option *> *LHS,
                          స్థిరరాశి std::pair<స్థిరరాశి అక్షరం *, Option *> *RHS) {
  తిరిగి strcmp(LHS->మొదటి_2, RHS->మొదటి_2);
}

స్థిర పూర్ణాంకం SubNameCompare(స్థిరరాశి std::pair<స్థిరరాశి అక్షరం *, SubCommand *> *LHS,
                          స్థిరరాశి std::pair<స్థిరరాశి అక్షరం *, SubCommand *> *RHS) {
  తిరిగి strcmp(LHS->మొదటి_2, RHS->మొదటి_2);
}

// Copy Options into a vector so we can sort them as we like.
స్థిర శూన్య క్రమీకరించు_Opts(OptionsMapTy &OptMap,
                     Small_సదిశ_Impl<std::pair<స్థిరరాశి అక్షరం *, Option *>> &Opts,
                     తార్కిక ShowHidden) {
  Small_Ptr_సమితి<Option *, 32> Option_సమితి; // Duplicate option detection.

  కొరకు (స్వయంచాలక I = OptMap.begin(), E = OptMap.చివర(); I != E; ++I) {
    // Ignore really-hidden options.
    ఒకవేళ (I->రెండవ_2->getOptionHiddenFlag() == ReallyHidden)
      కొనసాగు;

    // Unless showhidden is set, ignore hidden flags.
    ఒకవేళ (I->రెండవ_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      కొనసాగు;

    // If we've already seen this option, don't add it to the list again.
    ఒకవేళ (!Option_సమితి.చొప్పించు(I->రెండవ_2).రెండవ_2)
      కొనసాగు;

    Opts.నెట్టు_వెనుక(
        std::pair<స్థిరరాశి అక్షరం *, Option *>(I->మొదటి_2.దత్తాంశం(), I->రెండవ_2));
  }

  // Sort the options list alphabetically.
  array_pod_క్రమీకరించు(Opts.begin(), Opts.చివర(), OptNameCompare);
}

స్థిర శూన్య
క్రమీకరించు_Sub_Commands(స్థిరరాశి Small_Ptr_సమితి_Impl<SubCommand *> &SubMap,
                Small_సదిశ_Impl<std::pair<స్థిరరాశి అక్షరం *, SubCommand *>> &Subs) {
  కొరకు (స్వయంచాలక *S : SubMap) {
    ఒకవేళ (S->getName().ఖాళీలు())
      కొనసాగు;
    Subs.నెట్టు_వెనుక(std::make_pair(S->getName().దత్తాంశం(), S));
  }
  array_pod_క్రమీకరించు(Subs.begin(), Subs.చివర(), SubNameCompare);
}

నామాకాశ {

తరగతి HelpPrinter {
రక్షిత:
  స్థిరరాశి తార్కిక ShowHidden;
  ఉపయోగించి Str_Option_Pair_సదిశ =
      Small_సదిశ<std::pair<స్థిరరాశి అక్షరం *, Option *>, 128>;
  ఉపయోగించి Str_Sub_Command_Pair_సదిశ =
      Small_సదిశ<std::pair<స్థిరరాశి అక్షరం *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  మాయా శూన్య printOptions(Str_Option_Pair_సదిశ &Opts, size_t MaxArgLen) {
    కొరకు (స్థిరరాశి స్వయంచాలక &Opt : Opts)
      Opt.రెండవ_2->printOptionInfo(MaxArgLen);
  }

  శూన్య printSubCommands(Str_Sub_Command_Pair_సదిశ &Subs, size_t MaxSubLen) {
    కొరకు (స్థిరరాశి స్వయంచాలక &S : Subs) {
      outs() << "  " << S.మొదటి_2;
      ఒకవేళ (!S.రెండవ_2->getDescription().ఖాళీలు()) {
        outs().indent(MaxSubLen - అక్షరశ్రేణి_పొడవు(S.మొదటి_2));
        outs() << " - " << S.రెండవ_2->getDescription();
      }
      outs() << "\n";
    }
  }

బహిరంగ:
  స్పష్టమైన HelpPrinter(తార్కిక showHidden) : ShowHidden(showHidden) {}
  మాయా ~HelpPrinter() = అప్రమేయ;

  // Invoke the printer.
  శూన్య క్రియాకారకం=(తార్కిక విలువ) {
    ఒకవేళ (!విలువ)
      తిరిగి;
    printHelp();

    // Halt the program since help information was printed
    నిష్క్రమించు(0);
  }

  శూన్య printHelp() {
    SubCommand *Sub = globalParser().get_చురుకైన_Sub_Command();
    స్వయంచాలక &OptionsMap = Sub->OptionsMap;
    స్వయంచాలక &PositionalOpts = Sub->PositionalOpts;
    స్వయంచాలక &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_సదిశ Opts;
    క్రమీకరించు_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_సదిశ Subs;
    క్రమీకరించు_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    ఒకవేళ (!globalParser().కార్యక్రమం_Overview.ఖాళీలు())
      outs() << "OVERVIEW: " << globalParser().కార్యక్రమం_Overview << "\n";

    ఒకవేళ (Sub == &SubCommand::get_శిఖరం_Level()) {
      outs() << "USAGE: " << globalParser().కార్యక్రమం_Name;
      ఒకవేళ (!Subs.ఖాళీలు())
        outs() << " [subcommand]";
      outs() << " [options]";
    } లేకపోతే {
      ఒకవేళ (!Sub->getDescription().ఖాళీలు()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().కార్యక్రమం_Name << " " << Sub->getName()
             << " [options]";
    }

    కొరకు (స్వయంచాలక *Opt : PositionalOpts) {
      ఒకవేళ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    ఒకవేళ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    ఒకవేళ (Sub == &SubCommand::get_శిఖరం_Level() && !Subs.ఖాళీలు()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      కొరకు (స్థిరరాశి స్వయంచాలక &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, అక్షరశ్రేణి_పొడవు(Sub.మొదటి_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().కార్యక్రమం_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    కొరకు (స్థిరరాశి స్వయంచాలక &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.రెండవ_2->get_Option_వెడల్పు());

    Small_సదిశ<std::pair<std::string, StringRef>, 0> LibraryOpts;
    ఒకవేళ (ShowHidden)
      కొరకు (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              ఒకవేళ (!Help.ఖాళీలు())
                LibraryOpts.emplace_వెనుక((Spelling + MetaVar).str(), Help);
            });
    llvm::క్రమీకరించు(LibraryOpts);
    కొరకు (స్థిరరాశి స్వయంచాలక &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_కొలత(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    ఒకవేళ (!LibraryOpts.ఖాళీలు())
      outs() << "\nLibrary options:\n\n";
    కొరకు (స్థిరరాశి స్వయంచాలక &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_కొలత(Name));
    }

    // Print any extra help the user has declared.
    కొరకు (స్థిరరాశి స్వయంచాలక &I : globalParser().మరిన్ని_Help)
      outs() << I;
    globalParser().మరిన్ని_Help.clear();
  }
};

తరగతి CategorizedHelpPrinter : బహిరంగ HelpPrinter {
బహిరంగ:
  స్పష్టమైన CategorizedHelpPrinter(తార్కిక showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  స్థిర పూర్ణాంకం OptionCategoryCompare(OptionCategory *స్థిరరాశి *A,
                                   OptionCategory *స్థిరరాశి *B) {
    తిరిగి (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  ఉపయోగించి HelpPrinter::క్రియాకారకం=;

రక్షిత:
  శూన్య printOptions(Str_Option_Pair_సదిశ &Opts, size_t MaxArgLen) override {
    std::సదిశ<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::సదిశ<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ప్రతిజ్ఞా(SortedCategories.కొలత_2() > 0 && "No option categories registered!");
    array_pod_క్రమీకరించు(SortedCategories.begin(), SortedCategories.చివర(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    కొరకు (స్థిరరాశి స్వయంచాలక &I : Opts) {
      Option *Opt = I.రెండవ_2;
      కొరకు (OptionCategory *Cat : Opt->Categories) {
        ప్రతిజ్ఞా(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].నెట్టు_వెనుక(Opt);
      }
    }

    // Now do printing.
    కొరకు (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      స్థిరరాశి స్వయంచాలక &CategoryOptions = CategorizedOptions[Category];
      ఒకవేళ (CategoryOptions.ఖాళీలు())
        కొనసాగు;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      ఒకవేళ (!Category->getDescription().ఖాళీలు())
        outs() << Category->getDescription() << "\n\n";
      లేకపోతే
        outs() << "\n";

      // Loop over the options in the category and print.
      కొరకు (స్థిరరాశి Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
తరగతి HelpPrinterWrapper {
అంతరంగ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

బహిరంగ:
  స్పష్టమైన HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  శూన్య క్రియాకారకం=(తార్కిక విలువ);
};

} // End anonymous namespace

#ఒకవేళ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# ఒకవేళ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# లేకపోతే
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#లేదావేళ defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# ఒకవేళ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# లేకపోతే
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#లేకపోతే
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#అయితే_ముగింపు

నామాకాశ {
తరగతి సంస్కరణ_Printer {
బహిరంగ:
  శూన్య print(స్థిరరాశి std::సదిశ<సంస్కరణ_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#నిర్వచితమైతే PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#లేకపోతే
    OS << "LLVM (http://llvm.org/):\n  ";
#అయితే_ముగింపు
    OS << PACKAGE_NAME << " version " << PACKAGE_సంస్కరణ << "\n  ";
#ఒకవేళ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#లేకపోతే
    OS << "Optimized build";
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే NDEBUG
    OS << " with assertions";
#అయితే_ముగింపు
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    ఒకవేళ (!ExtraPrinters.ఖాళీలు()) {
      కొరకు (స్థిరరాశి స్వయంచాలక &I : ExtraPrinters)
        I(outs());
    }
  }
  శూన్య క్రియాకారకం=(తార్కిక OptionWasSpecified);
};

నిర్మాణం Command_పంక్తి_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_సాధారణ_Printer{అబద్ధం};
  HelpPrinter UncategorizedHiddenPrinter{నిజం};
  CategorizedHelpPrinter Categorized_సాధారణ_Printer{అబద్ధం};
  CategorizedHelpPrinter CategorizedHiddenPrinter{నిజం};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_సాధారణ_Printer{Uncategorized_సాధారణ_Printer,
                                          Categorized_సాధారణ_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, నిజం, parser<తార్కిక>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_సాధారణ_Printer),
      cl::Hidden,
      cl::విలువ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, నిజం, parser<తార్కిక>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::విలువ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, నిజం, parser<తార్కిక>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_సాధారణ_Printer),
      cl::విలువ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, నిజం, parser<తార్కిక>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::విలువ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<తార్కిక> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(అబద్ధం),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<తార్కిక> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(అబద్ధం),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  సంస్కరణ_Printer_Ty Override_సంస్కరణ_Printer = శూన్యసూచిక;

  std::సదిశ<సంస్కరణ_Printer_Ty> Extra_సంస్కరణ_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  సంస్కరణ_Printer సంస్కరణ_Printer_Instance;

  cl::opt<సంస్కరణ_Printer, నిజం, parser<తార్కిక>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(సంస్కరణ_Printer_Instance), cl::విలువ_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
స్థిర ManagedStatic<Command_పంక్తి_Common_Options> CommonOptions;

స్థిర శూన్య initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_లేఖాచిత్రం_Writer_Options();
  init_సంకేతాలు_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_విత్తనం_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  స్థిర OptionCategory GeneralCategory{"General options"};
  తిరిగి GeneralCategory;
}

శూన్య సంస్కరణ_Printer::క్రియాకారకం=(తార్కిక OptionWasSpecified) {
  ఒకవేళ (!OptionWasSpecified)
    తిరిగి;

  ఒకవేళ (CommonOptions->Override_సంస్కరణ_Printer != శూన్యసూచిక) {
    CommonOptions->Override_సంస్కరణ_Printer(outs());
    నిష్క్రమించు(0);
  }
  print(CommonOptions->Extra_సంస్కరణ_Printers);

  నిష్క్రమించు(0);
}

శూన్య HelpPrinterWrapper::క్రియాకారకం=(తార్కిక విలువ) {
  ఒకవేళ (!విలువ)
    తిరిగి;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  ఒకవేళ (globalParser().RegisteredOptionCategories.కొలత_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.సమితి_Hidden_Flag(NotHidden);

    CategorizedPrinter = నిజం; // Invoke categorized printer
  } లేకపోతే {
    UncategorizedPrinter = నిజం; // Invoke uncategorized printer
  }
}

// Print the value of each option.
శూన్య cl::Print_Option_విలువలు() { globalParser().print_Option_విలువలు(); }

శూన్య Command_పంక్తి_Parser::print_Option_విలువలు() {
  ఒకవేళ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    తిరిగి;

  Small_సదిశ<std::pair<స్థిరరాశి అక్షరం *, Option *>, 128> Opts;
  క్రమీకరించు_Opts(చురుకైన_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ నిజం);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  కొరకు (స్థిరరాశి స్వయంచాలక &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.రెండవ_2->get_Option_వెడల్పు());

  కొరకు (స్థిరరాశి స్వయంచాలక &Opt : Opts)
    Opt.రెండవ_2->print_Option_విలువ(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
శూన్య cl::Print_Help_సందేశం(తార్కిక Hidden, తార్కిక Categorized) {
  ఒకవేళ (!Hidden && !Categorized)
    CommonOptions->Uncategorized_సాధారణ_Printer.printHelp();
  లేకపోతే ఒకవేళ (!Hidden && Categorized)
    CommonOptions->Categorized_సాధారణ_Printer.printHelp();
  లేకపోతే ఒకవేళ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  లేకపోతే
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  స్థిర స్థిరరాశి StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#ఒకవేళ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే NDEBUG
      "+assertions",
#అయితే_ముగింపు
#నిర్వచితమైతే EXPENSIVE_CHECKS
      "+expensive-checks",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(చిరునామా_sanitizer)
      "+asan",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(dataflow_sanitizer)
      "+dfsan",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(జ్ఞాపకం_sanitizer)
      "+msan",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(thread_sanitizer)
      "+tsan",
#అయితే_ముగింపు
#ఒకవేళ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#అయితే_ముగింపు
#నిర్వచితమైతే LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#అయితే_ముగింపు
  };
  తిరిగి ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
శూన్య cl::printBuildConfig(raw_ostream &OS) {
#ఒకవేళ LLVM_సంస్కరణ_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#అయితే_ముగింపు
}

/// Utility function for printing version number.
శూన్య cl::Print_సంస్కరణ_సందేశం() {
  CommonOptions->సంస్కరణ_Printer_Instance.print(CommonOptions->Extra_సంస్కరణ_Printers);
}

శూన్య cl::సమితి_సంస్కరణ_Printer(సంస్కరణ_Printer_Ty func) {
  CommonOptions->Override_సంస్కరణ_Printer = func;
}

శూన్య cl::కూడు_Extra_సంస్కరణ_Printer(సంస్కరణ_Printer_Ty func) {
  CommonOptions->Extra_సంస్కరణ_Printers.నెట్టు_వెనుక(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  స్వయంచాలక &Subs = globalParser().RegisteredSubCommands;
  (శూన్య)Subs;
  ప్రతిజ్ఞా(Subs.కలిగియుంది(&Sub));
  తిరిగి Sub.OptionsMap;
}

iterator_range<Small_Ptr_సమితి<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  తిరిగి globalParser().getRegisteredSubcommands();
}

శూన్య cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  కొరకు (స్వయంచాలక &I : Sub.OptionsMap) {
    తార్కిక Unrelated = నిజం;
    కొరకు (స్వయంచాలక &Cat : I.రెండవ_2->Categories) {
      ఒకవేళ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = అబద్ధం;
    }
    ఒకవేళ (Unrelated)
      I.రెండవ_2->సమితి_Hidden_Flag(cl::ReallyHidden);
  }
}

శూన్య cl::HideUnrelatedOptions(ArrayRef<స్థిరరాశి cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  కొరకు (స్వయంచాలక &I : Sub.OptionsMap) {
    తార్కిక Unrelated = నిజం;
    కొరకు (స్వయంచాలక &Cat : I.రెండవ_2->Categories) {
      ఒకవేళ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = అబద్ధం;
    }
    ఒకవేళ (Unrelated)
      I.రెండవ_2->సమితి_Hidden_Flag(cl::ReallyHidden);
  }
}

శూన్య cl::Reset_Command_పంక్తి_Parser() { globalParser().reset(); }

శూన్య cl::కూడు_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.నెట్టు_వెనుక(&L);
}

శూన్య LibraryOptionsContext::కేటాయించు(స్థిరరాశి LibraryOptionsContext &Other) {
  ప్రతిజ్ఞా(ఇది != &Other);
  విలువలు.clear();
  Saved_వాదనలు.clear();
  ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు = Other.Saved_వాదనలు;
  కొరకు (size_t I = 0, E = వాదనలు.కొలత_2(); I != E;) {
    StringRef Name = వాదనలు[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.విభజించు('=').మొదటి_2);
    గుర్తులేని N;
    cantFail(parse(*L, వాదనలు.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు,
                                   గుర్తులేని &Consumed) {
  స్వయంచాలక [It, Inserted] = విలువలు.try_emplace(L.getDefault());
  ఒకవేళ (Inserted)
    It->రెండవ_2 = L.copyDefault(Alloc);
  తిరిగి parseLibraryOption(L, It->రెండవ_2, వాదనలు, Consumed, Alloc, &Saved_వాదనలు);
}

శూన్య cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

శూన్య LLVMParse_Command_పంక్తి_Options(పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
                                 స్థిరరాశి అక్షరం *Overview) {
  llvm::cl::Parse_Command_పంక్తి_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
