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
کارول نطاق llvm;
کارول نطاق cl;

#تعریف_کړه DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نطاق llvm {
نطاق cl {
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<اوږد>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<اوږد اوږد>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<بینښه>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<بینښه اوږد>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<بینښه اوږد اوږد>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE basic_parser<Element_شمېر>;

#إذا !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
کينډۍ صنف LLVM_EXPORT_TEMPLATE opt<std::string>;
#د_که_پای

کينډۍ صنف LLVM_EXPORT_TEMPLATE opt<منطقي>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE opt<حرف>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE opt<صحيح>;
کينډۍ صنف LLVM_EXPORT_TEMPLATE opt<بینښه>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
فراغ Generic_Option_ارزښت::anchor() {}
فراغ Option_ارزښت<boolOrDefault>::anchor() {}
فراغ Option_ارزښت<std::string>::anchor() {}
فراغ Option::anchor() {}
فراغ basic_parser_impl::anchor() {}
فراغ parser<منطقي>::anchor() {}
فراغ parser<boolOrDefault>::anchor() {}
فراغ parser<صحيح>::anchor() {}
فراغ parser<اوږد>::anchor() {}
فراغ parser<اوږد اوږد>::anchor() {}
فراغ parser<بینښه>::anchor() {}
فراغ parser<بینښه اوږد>::anchor() {}
فراغ parser<بینښه اوږد اوږد>::anchor() {}
فراغ parser<مزدوج>::anchor() {}
فراغ parser<عائم>::anchor() {}
فراغ parser<std::string>::anchor() {}
فراغ parser<std::optional<std::string>>::anchor() {}
فراغ parser<حرف>::anchor() {}
فراغ parser<Element_شمېر>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
فراغ opt_bool_anchor() { opt<منطقي> anchor{""}; }
فراغ opt_char_anchor() { opt<حرف> anchor{""}; }
فراغ opt_int_anchor() { opt<صحيح> anchor{""}; }
فراغ opt_unsigned_anchor() { opt<بینښه> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساكن size_t DefaultPad = 2;

ساكن StringRef Arg_مختاړی = "-";
ساكن StringRef Arg_مختاړی_Long = "--";
ساكن StringRef Arg_Help_مختاړی = " - ";

ساكن size_t arg_Plus_Prefixes_اندازه(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.اندازه_2();
  إذا (Len == 1)
    أرجع Len + Pad + Arg_مختاړی.اندازه_2() + Arg_Help_مختاړی.اندازه_2();
  أرجع Len + Pad + Arg_مختاړی_Long.اندازه_2() + Arg_Help_مختاړی.اندازه_2();
}

ساكن SmallString<8> arg_مختاړی(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> مختاړی;
  لكل (size_t I = 0; I < Pad; ++I) {
    مختاړی.ټېل_شا(' ');
  }
  مختاړی.append(ArgName.اندازه_2() > 1 ? Arg_مختاړی_Long : Arg_مختاړی);
  أرجع مختاړی;
}

کارول OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

صنف PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  ملګری raw_ostream &چلوونکی<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &چلوونکی<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << arg_مختاړی(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  أرجع OS;
}

صنف Command_کرښه_Parser {
عام:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string پروګرام_Name;
  StringRef پروګرام_Overview;

  // This collects additional help to be printed.
  std::ویکټور<StringRef> نور_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_سېټ<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_سېټ<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_ویکټور<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_شاخص;
  size_t عدد_شاخصي_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_کرښه_Parser() { ثبتوونکی_Sub_Command(&SubCommand::get_سر_Level()); }

  فراغ ResetAllOptionOccurrences();

  منطقي Parse_Command_کرښه_Options(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = تش_اشاره,
                               vfs::FileSystem *VFS = تش_اشاره,
                               LibraryOptionsContext *Context = تش_اشاره);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    إذا (Opt.Subs.تشه()) {
      Action(SubCommand::get_سر_Level());
      أرجع;
    }
    إذا (Opt.Subs.اندازه_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      لكل (خپلکاری *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      أرجع;
    }
    لكل (خپلکاری *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  فراغ جمع_کړه_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    إذا (Opt.hasArgStr())
      أرجع;
    إذا (!SC->OptionsMap.ورننباسه(std::make_pair(Name, &Opt)).دوهم_2) {
      errs() << پروګرام_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  فراغ جمع_کړه_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { جمع_کړه_Literal_Option(Opt, &SC, Name); });
  }

  فراغ جمع_کړه_Option(Option *O, SubCommand *SC) {
    منطقي HadErrors = خطأ;
    إذا (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      إذا (!SC->OptionsMap.ورننباسه(std::make_pair(O->ArgStr, O)).دوهم_2 ||
          (RegisteredSubCommands.لري(SC) &&
           Library_شاخص.لري(O->ArgStr))) {
        errs() << پروګرام_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = ریښتیا;
      }
    }

    // Remember information about positional options.
    إذا (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ټېل_شا(O);
    وإلا إذا (O->get_عدد_Occurrences_Flag() == cl::ConsumeAfter) {
      إذا (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = ریښتیا;
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

  فراغ جمع_کړه_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { جمع_کړه_Option(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    Small_ویکټور<StringRef, 16> Option_نومونه;
    O->get_Extra_Option_نومونه(Option_نومونه);
    إذا (O->hasArgStr())
      Option_نومونه.ټېل_شا(O->ArgStr);

    SubCommand &Sub = *SC;
    لكل (خپلکاری Name : Option_نومونه) {
      خپلکاری I = Sub.OptionsMap.پیدا_کړه(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      إذا (I != Sub.OptionsMap.پای_ټکی() && I->دوهم_2 == O)
        Sub.OptionsMap.erase(I);
    }

    إذا (O->getFormattingFlag() == cl::Positional)
      لكل (خپلکاری *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.پای_ټکی(); ++Opt) {
        إذا (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    وإلا إذا (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = تش_اشاره;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    أرجع (!Sub.OptionsMap.تشه() || !Sub.PositionalOpts.تشه() ||
            تش_اشاره != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    لكل (ثابت خپلکاری *S : RegisteredSubCommands) {
      إذا (hasOptions(*S))
        أرجع ریښتیا;
    }
    أرجع !Libraries.تشه();
  }

  منطقي hasNamedSubCommands() ثابت {
    لكل (ثابت خپلکاری *S : RegisteredSubCommands)
      إذا (!S->getName().تشه())
        أرجع ریښتیا;
    أرجع خطأ;
  }

  SubCommand *get_فعال_Sub_Command() { أرجع فعال_Sub_Command; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    إذا (!Sub.OptionsMap.ورننباسه(std::make_pair(NewName, O)).دوهم_2) {
      errs() << پروګرام_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  فراغ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  فراغ print_Option_ارزښتونه();

  فراغ شاخص_Library_Options() {
    منطقي HadErrors = خطأ;
    لكل (; عدد_شاخصي_Libraries != Libraries.اندازه_2(); ++عدد_شاخصي_Libraries) {
      LibraryOptions *L = Libraries[عدد_شاخصي_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        خپلکاری [It, Inserted] = Library_شاخص.try_emplace(Name, L);
        إذا (Inserted ? none_د(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 أرجع SC->OptionsMap.لري(Name);
                               })
                     : It->دوهم_2 == L)
          أرجع;
        errs() << پروګرام_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = ریښتیا;
      });
    }
    إذا (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    شاخص_Library_Options();
    أرجع Library_شاخص.lookup(Name);
  }

  فراغ ثبتوونکی_Category(OptionCategory *cat) {
    أكد(شمېر_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             أرجع cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ورننباسه(cat);
  }

  فراغ ثبتوونکی_Sub_Command(SubCommand *sub) {
    أكد(شمېر_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      أرجع (!sub->getName().تشه()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ورننباسه(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    لكل (خپلکاری &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.دوهم_2;
      إذا (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        جمع_کړه_Option(O, sub);
      وإلا
        جمع_کړه_Literal_Option(*O, sub, E.لومړی_2);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_سېټ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    أرجع make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.پای_ټکی());
  }

  فراغ reset() {
    فعال_Sub_Command = تش_اشاره;
    پروګرام_Name.clear();
    پروګرام_Overview = StringRef();

    نور_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_شاخص.clear();
    عدد_شاخصي_Libraries = 0;

    SubCommand::get_سر_Level().reset();
    SubCommand::getAll().reset();
    ثبتوونکی_Sub_Command(&SubCommand::get_سر_Level());
  }

خاص:
  SubCommand *فعال_Sub_Command = تش_اشاره;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &ارزښت);
  SubCommand *LookupSubCommand(StringRef Name, std::string &تر_ټولو_نږدې_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساكن Command_کرښه_Parser &globalParser() {
  ساكن ManagedStatic<Command_کرښه_Parser> GlobalParser;
  أرجع *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ساكن Error
parseLibraryOption(LibraryOptions &L, فراغ *Opts, ArrayRef<ثابت حرف *> دلیلونه,
                   بینښه &Consumed, BumpPtrAllocator &Alloc,
                   Small_ویکټور_Impl<ثابت حرف *> *ریکارډ = تش_اشاره) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  بینښه عدد_دلیلونه = std::min<size_t>(دلیلونه.اندازه_2(), 2);
  لكل (بینښه J = 0; J != عدد_دلیلونه; ++J)
    Saved[J] = Saver.save(دلیلونه[J]).معلومات();
  Consumed = 1;
  إذا (Error E = L.parse(Opts, ArrayRef(Saved, عدد_دلیلونه), Consumed, Alloc))
    أرجع E;
  إذا (ریکارډ)
    ریکارډ->append(Saved, Saved + Consumed);
  أرجع Error::success();
}

کينډۍ <ډول_نوم T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &ارزښت) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  إذا (!Arg.معلومات() || Arg == "true" || Arg == "1") {
    ارزښت = TrueVal;
    أرجع خطأ;
  }

  إذا (Arg == "false" || Arg == "0") {
    ارزښت = FalseVal;
    أرجع خطأ;
  }
  أرجع O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

فراغ cl::جمع_کړه_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().جمع_کړه_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().نور_Help.ټېل_شا(Help);
}

Option::Option(عدد_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : عدد_Occurrences(0), Occurrences(OccurrencesFlag), ارزښت(0),
      HiddenFlag(Hidden), Formatting(نورمال_Formatting), Misc(0),
      FullyInitialized(خطأ), Position(0) {
  Categories.ټېل_شا(&getGeneralCategory());
}

فراغ Option::جمع_کړه_دلیل() {
  globalParser().جمع_کړه_Option(هذا);
  FullyInitialized = ریښتیا;
}

فراغ Option::remove_دلیل() { globalParser().removeOption(هذا); }

فراغ Option::سېټ_Arg_Str(StringRef S) {
  إذا (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::جمع_کړه_Category(OptionCategory &C) {
  أكد(!Categories.تشه() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  إذا (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وإلا إذا (!is_contained(Categories, &C))
    Categories.ټېل_شا(&C);
}

فراغ Option::reset() {
  عدد_Occurrences = 0;
  سېټ_Default();
}

فراغ OptionCategory::ثبتوونکی_Category() {
  globalParser().ثبتوونکی_Category(هذا);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_سر_Level() {
  ساكن ManagedStatic<SubCommand> سر_Level_Sub_Command;
  أرجع *سر_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  أرجع *AllSubCommands;
}

فراغ SubCommand::ثبتوونکی_Sub_Command() {
  globalParser().ثبتوونکی_Sub_Command(هذا);
}

فراغ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(هذا);
}

فراغ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = تش_اشاره;
}

SubCommand::چلوونکی منطقي() ثابت {
  أرجع (globalParser().get_فعال_Sub_Command() == هذا);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_کرښه_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &ارزښت) {
  // Reject all dashes.
  إذا (Arg.تشه())
    أرجع تش_اشاره;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.پیدا_کړه('=');

  // If we have an equals sign, remember the value.
  إذا (EqualPos == StringRef::npos) {
    // Look up the option.
    أرجع Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  خپلکاری I = Sub.OptionsMap.پیدا_کړه(Arg.substr(0, EqualPos));
  إذا (I == Sub.OptionsMap.پای_ټکی())
    أرجع تش_اشاره;

  خپلکاری *O = I->دوهم_2;
  إذا (O->getFormattingFlag() == cl::Always_مختاړی)
    أرجع تش_اشاره;

  ارزښت = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  أرجع I->دوهم_2;
}

SubCommand *Command_کرښه_Parser::LookupSubCommand(StringRef Name,
                                                std::string &تر_ټولو_نږدې_String) {
  إذا (Name.تشه())
    أرجع &SubCommand::get_سر_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *تر_ټولو_نږدې_سمون = تش_اشاره;
  لكل (خپلکاری *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    إذا (S->getName().تشه())
      استمر;

    إذا (S->getName() == Name)
      أرجع S;

    إذا (!تر_ټولو_نږدې_سمون && S->getName().edit_واټن(Name) < 2)
      تر_ټولو_نږدې_سمون = S;
  }

  إذا (تر_ټولو_نږدې_سمون)
    تر_ټولو_نږدې_String = تر_ټولو_نږدې_سمون->getName();

  أرجع &SubCommand::get_سر_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *Lookup_تر_ټولو_نږدې_Option(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &تر_ټولو_نږدې_String) {
  // Reject all dashes.
  إذا (Arg.تشه())
    أرجع تش_اشاره;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> وېشل_Arg = Arg.وېشل('=');
  StringRef &LHS = وېشل_Arg.لومړی_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = وېشل_Arg.دوهم_2;

  // Find the closest match.
  Option *غوره = تش_اشاره;
  بینښه غوره_واټن = 0;
  لكل (ثابت خپلکاری &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    إذا (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    Small_ویکټور<StringRef, 16> Option_نومونه;
    O->get_Extra_Option_نومونه(Option_نومونه);
    إذا (O->hasArgStr())
      Option_نومونه.ټېل_شا(O->ArgStr);

    منطقي Permit_ارزښت = O->get_ارزښت_Expected_Flag() != cl::ارزښت_Disallowed;
    StringRef Flag = Permit_ارزښت ? LHS : Arg;
    لكل (ثابت خپلکاری &Name : Option_نومونه) {
      بینښه واټن_2 = StringRef(Name).edit_واټن(
          Flag, /*AllowReplacements=*/ریښتیا, /*MaxEditDistance=*/غوره_واټن);
      إذا (!غوره || واټن_2 < غوره_واټن) {
        غوره = O;
        غوره_واټن = واټن_2;
        إذا (RHS.تشه() || !Permit_ارزښت)
          تر_ټولو_نږدې_String = std::string(Name);
        وإلا
          تر_ټولو_نږدې_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  أرجع غوره;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي Comma_Separate_And_جمع_کړه_Occurrence(Option *Handler, بینښه pos,
                                          StringRef ArgName, StringRef ارزښت) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  إذا (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(ارزښت);
    StringRef::اندازه_type Pos = Val.پیدا_کړه(',');

    طالما (Pos != StringRef::npos) {
      // Process the portion before the comma.
      إذا (Handler->جمع_کړه_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        أرجع ریښتیا;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.پیدا_کړه(',');
    }

    ارزښت = Val;
  }

  أرجع Handler->جمع_کړه_Occurrence(pos, ArgName, ارزښت);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساكن مضمن منطقي ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef ارزښت, صحيح argc,
                                 ثابت حرف *ثابت *argv, صحيح &i) {
  // Enforce value requirements
  بدل (Handler->get_ارزښت_Expected_Flag()) {
  حالة ارزښت_Required:
    إذا (!ارزښت.معلومات()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      إذا (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_مختاړی)
        أرجع Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      ارزښت = StringRef(argv[++i]);
    }
    اكسر;
  حالة ارزښت_Disallowed:
    إذا (ارزښت.معلومات())
      أرجع Handler->error("does not allow a value! '" + Twine(ارزښت) +
                            "' specified.");
    اكسر;
  حالة ارزښت_Optional:
    اكسر;
  }

  أرجع Comma_Separate_And_جمع_کړه_Occurrence(Handler, i, ArgName, ارزښت);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  أرجع ProvideOption(Handler, Handler->ArgStr, Arg, 0, تش_اشاره, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *پیدا_کړه_مختاړی_Option(StringRef Name, size_t &اوږدتیا,
                                ثابت OptionsMapTy &OptionsMap) {
  لكل (; !Name.تشه(); Name = Name.drop_شا()) {
    Option *O = OptionsMap.lookup(Name);
    إذا (O && (O->getFormattingFlag() == cl::مختاړی ||
              O->getFormattingFlag() == cl::Always_مختاړی)) {
      اوږدتیا = Name.اندازه_2();
      أرجع O;
    }
  }
  أرجع تش_اشاره;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &ارزښت,
                                    ثابت OptionsMapTy &OptionsMap) {
  إذا (Arg.اندازه_2() == 1)
    أرجع تش_اشاره;

  size_t اوږدتیا = 0;
  Option *POpt = پیدا_کړه_مختاړی_Option(Arg, اوږدتیا, OptionsMap);
  إذا (!POpt)
    أرجع تش_اشاره;

  StringRef Maybe_ارزښت =
      (اوږدتیا < Arg.اندازه_2()) ? Arg.substr(اوږدتیا) : StringRef();
  Arg = Arg.substr(0, اوږدتیا);

  // cl::Prefix options do not preserve '=' when used separately.
  إذا (POpt->getFormattingFlag() == cl::مختاړی && Maybe_ارزښت.starts_with("="))
    Maybe_ارزښت = Maybe_ارزښت.drop_front();
  ارزښت = Maybe_ارزښت;
  أرجع POpt;
}

ساكن منطقي Requires_ارزښت(ثابت Option *O) {
  أرجع O->get_عدد_Occurrences_Flag() == cl::Required ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_نور;
}

ساكن منطقي Eats_Unbounded_شمېره_د_ارزښتونه(ثابت Option *O) {
  أرجع O->get_عدد_Occurrences_Flag() == cl::Zero_Or_نور ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_نور;
}

ساكن منطقي isWhitespace(حرف C) {
  أرجع C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  أرجع isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { أرجع C == '\"' || C == '\''; }

فراغ cl::Tokenize_GNUCommand_کرښه(StringRef Src, StringSaver &Saver,
                                Small_ویکټور_Impl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي دننه_Token = خطأ;
  لكل (size_t I = 0, E = Src.اندازه_2(); I != E; ++I) {
    // Consume runs of whitespace.
    إذا (!دننه_Token) {
      طالما (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        إذا (MarkEOLs && Src[I] == '\n')
          NewArgv.ټېل_شا(تش_اشاره);
        ++I;
      }
      إذا (I == E)
        اكسر;
      دننه_Token = ریښتیا;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    إذا (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ټېل_شا(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    إذا (isQuote(C)) {
      ++I;
      طالما (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        إذا (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ټېل_شا(Src[I]);
        ++I;
      }
      إذا (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    إذا (isWhitespace(C)) {
      NewArgv.ټېل_شا(Saver.save(Token.str()).معلومات());
      // Mark the end of lines in response files.
      إذا (MarkEOLs && C == '\n')
        NewArgv.ټېل_شا(تش_اشاره);
      Token.clear();
      دننه_Token = خطأ;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.ټېل_شا(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  إذا (دننه_Token)
    NewArgv.ټېل_شا(Saver.save(Token.str()).معلومات());
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
  size_t E = Src.اندازه_2();
  صحيح Backslash_شمېر = 0;
  // Skip the backslashes.
  افعل {
    ++I;
    ++Backslash_شمېر;
  } طالما (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  إذا (FollowedByDoubleQuote) {
    Token.append(Backslash_شمېر / 2, '\\');
    إذا (Backslash_شمېر % 2 == 0)
      أرجع I - 1;
    Token.ټېل_شا('"');
    أرجع I;
  }
  Token.append(Backslash_شمېر, '\\');
  أرجع I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي is_Windows_Special_Char_دننه_Command_Name(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenize_Windows_Command_کرښه_Impl(
    StringRef Src, StringSaver &Saver, function_ref<فراغ(StringRef)> جمع_کړه_Token,
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

  لكل (size_t I = 0, E = Src.اندازه_2(); I < E; ++I) {
    بدل (State) {
    حالة INIT: {
      أكد(Token.تشه() && "token should be empty in initial state");
      // Eat whitespace before a token.
      طالما (I < E && isWhitespaceOrNull(Src[I])) {
        إذا (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      إذا (I >= E)
        اكسر;
      size_t پیل = I;
      إذا (CommandName) {
        طالما (I < E && !is_Windows_Special_Char_دننه_Command_Name(Src[I]))
          ++I;
      } وإلا {
        طالما (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef نورمال_Chars = Src.ټوټه(پیل, I);
      إذا (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        جمع_کړه_Token(AlwaysCopy ? Saver.save(نورمال_Chars) : نورمال_Chars);
        إذا (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } وإلا {
          CommandName = خطأ;
        }
      } وإلا إذا (Src[I] == '\"') {
        Token += نورمال_Chars;
        State = QUOTED;
      } وإلا إذا (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += نورمال_Chars;
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
        جمع_کړه_Token(Saver.save(Token.str()));
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
        Token.ټېل_شا(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      إذا (Src[I] == '\"') {
        إذا (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ټېل_شا('"');
          ++I;
        } وإلا {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.ټېل_شا(Src[I]);
      }
      اكسر;
    }
  }

  إذا (State != INIT)
    جمع_کړه_Token(Saver.save(Token.str()));
}

فراغ cl::Tokenize_Windows_Command_کرښه(StringRef Src, StringSaver &Saver,
                                    Small_ویکټور_Impl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  خپلکاری جمع_کړه_Token = [&](StringRef Tok) { NewArgv.ټېل_شا(Tok.معلومات()); };
  خپلکاری OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ټېل_شا(تش_اشاره);
  };
  tokenize_Windows_Command_کرښه_Impl(Src, Saver, جمع_کړه_Token,
                                 /*AlwaysCopy=*/ریښتیا, OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_کرښه_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_ویکټور_Impl<StringRef> &NewArgv) {
  خپلکاری جمع_کړه_Token = [&](StringRef Tok) { NewArgv.ټېل_شا(Tok); };
  خپلکاری OnEOL = []() {};
  tokenize_Windows_Command_کرښه_Impl(Src, Saver, جمع_کړه_Token, /*AlwaysCopy=*/خطأ,
                                 OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_کرښه_ډک(StringRef Src, StringSaver &Saver,
                                        Small_ویکټور_Impl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  خپلکاری جمع_کړه_Token = [&](StringRef Tok) { NewArgv.ټېل_شا(Tok.معلومات()); };
  خپلکاری OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ټېل_شا(تش_اشاره);
  };
  tokenize_Windows_Command_کرښه_Impl(Src, Saver, جمع_کړه_Token,
                                 /*AlwaysCopy=*/ریښتیا, OnEOL, ریښتیا);
}

فراغ cl::tokenizeConfigFile(StringRef سرچینه, StringSaver &Saver,
                            Small_ویکټور_Impl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  لكل (ثابت حرف *Cur = سرچینه.begin(); Cur != سرچینه.پای_ټکی();) {
    SmallString<128> کرښه;
    // Check for comment line.
    إذا (isWhitespace(*Cur)) {
      طالما (Cur != سرچینه.پای_ټکی() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    إذا (*Cur == '#') {
      طالما (Cur != سرچینه.پای_ټکی() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *پیل = Cur;
    ثابت حرف *کرښه_پای_ټکی = تش_اشاره;
    حرف Quote = 0;
    منطقي At_Token_پیل = ریښتیا;
    لكل (ثابت حرف *پای_ټکی_2 = سرچینه.پای_ټکی(); Cur != پای_ټکی_2; ++Cur) {
      حرف C = *Cur;
      إذا (C == '\\') {
        إذا (Cur + 1 != پای_ټکی_2) {
          ++Cur;
          إذا (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != پای_ټکی_2) && Cur[1] == '\n')) {
            کرښه.append(پیل, Cur - 1);
            إذا (*Cur == '\r')
              ++Cur;
            پیل = Cur + 1;
            // Splicing does not introduce a token boundary.
            استمر;
          }
        }
        At_Token_پیل = خطأ;
        استمر;
      }
      إذا (C == '\n')
        اكسر;
      إذا (Quote) {
        إذا (C == Quote)
          Quote = 0;
      } وإلا إذا (isQuote(C)) {
        Quote = C;
      } وإلا إذا (C == '#' && At_Token_پیل) {
        کرښه_پای_ټکی = Cur;
        طالما (Cur != پای_ټکی_2 && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      At_Token_پیل = isWhitespace(C);
    }
    // Tokenize line.
    کرښه.append(پیل, کرښه_پای_ټکی ? کرښه_پای_ټکی : Cur);
    cl::Tokenize_GNUCommand_کرښه(کرښه, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي has_UTF8_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.اندازه_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ Expand_بنسټ_Paths(StringRef بنسټ_لار, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::لار_2::is_absolute(بنسټ_لار));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::اندازه_type پیل_Pos = 0;
  لكل (StringRef::اندازه_type TokenPos = ArgString.پیدا_کړه(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.پیدا_کړه(Token, پیل_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(پیل_Pos, TokenPos - پیل_Pos);
    إذا (ResponseFile.تشه())
      ResponseFile = LHS;
    وإلا
      llvm::sys::لار_2::append(ResponseFile, LHS);
    ResponseFile.append(بنسټ_لار);
    پیل_Pos = TokenPos + Token.اندازه_2();
  }

  إذا (!ResponseFile.تشه()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef پاتې_شوی = ArgString.substr(پیل_Pos);
    إذا (!پاتې_شوی.تشه())
      llvm::sys::لار_2::append(ResponseFile, پاتې_شوی);
    Arg = Saver.save(ResponseFile.str()).معلومات();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_ویکټور_Impl<ثابت حرف *> &NewArgv) {
  أكد(sys::لار_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<حافظه_بفر>> MemBufOrErr =
      FS->get_بفر_For_File(FName);
  إذا (!MemBufOrErr) {
    std::error_کوډ EC = MemBufOrErr.getError();
    أرجع llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.پیغام_2());
  }
  حافظه_بفر &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_بفر_پیل(), MemBuf.get_بفر_اندازه());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.get_بفر_پیل(), MemBuf.get_بفر_پای_ټکی());
  std::string UTF8Buf;
  إذا (has_UTF16_Byte_ترتیب_Mark(BufRef)) {
    إذا (!convert_UTF16_منزل_UTF8_String(BufRef, UTF8Buf))
      أرجع llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  وإلا إذا (has_UTF8_Byte_ترتیب_Mark(BufRef))
    Str = StringRef(BufRef.معلومات() + 3, BufRef.اندازه_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  إذا (!Relative_نومونه && !دننه_Config_File)
    أرجع Error::success();

  StringRef بنسټ_لار = llvm::sys::لار_2::پلار_لار(FName);
  لكل (ثابت حرف *&Arg : NewArgv) {
    إذا (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    إذا (دننه_Config_File)
      Expand_بنسټ_Paths(بنسټ_لار, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = خطأ;
    إذا (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      إذا (!llvm::sys::لار_2::is_relative(FileName))
        استمر;
    } وإلا إذا (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = ریښتیا;
    } وإلا {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ټېل_شا('@');
    إذا (ConfigInclusion && !llvm::sys::لار_2::has_پلار_لار(FileName)) {
      SmallString<128> File_لار;
      إذا (!پیدا_کړه_Config_File(FileName, File_لار))
        أرجع createStringError(
            std::make_error_کوډ(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_لار);
    } وإلا {
      ResponseFile.append(بنسټ_لار);
      llvm::sys::لار_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).معلومات();
  }
  أرجع Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_ویکټور_Impl<ثابت حرف *> &Argv) {
  بنية Response_File_ریکارډ {
    std::string File;
    size_t پای_ټکی_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_ویکټور<Response_File_ریکارډ, 3> File_ډېری;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ډېری.ټېل_شا({"", Argv.اندازه_2()});

  // Don't cache Argv.size() because it can change.
  لكل (بینښه I = 0; I != Argv.اندازه_2();) {
    طالما (I == File_ډېری.شا().پای_ټکی_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ډېری.راوباسه_شا();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    إذا (Arg == تش_اشاره) {
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
    إذا (llvm::sys::لار_2::is_relative(FName)) {
      إذا (CurrentDir.تشه()) {
        إذا (خپلکاری CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وإلا {
          أرجع createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وإلا {
        CurrDir = CurrentDir;
      }
      llvm::sys::لار_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::حالت> Res = FS->حالت_2(FName);
    إذا (!Res || !Res->exists()) {
      std::error_کوډ EC = Res.getError();
      إذا (!دننه_Config_File) {
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
                                       "': " + EC.پیغام_2());
    }
    ثابت llvm::vfs::حالت &File_حالت = Res.get();

    خپلکاری IsEquivalent =
        [File_حالت, هذا](ثابت Response_File_ریکارډ &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::حالت> RHS = FS->حالت_2(RFile.File);
      إذا (!RHS)
        أرجع RHS.getError();
      أرجع File_حالت.equivalent(*RHS);
    };

    // Check for recursive response files.
    لكل (ثابت خپلکاری &F : drop_begin(File_ډېری)) {
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
    Small_ویکټور<ثابت حرف *, 0> ExpandedArgv;
    إذا (Error Err = expandResponseFile(FName, ExpandedArgv))
      أرجع Err;

    لكل (Response_File_ریکارډ &ریکارډ : File_ډېری) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      ریکارډ.پای_ټکی_2 += ExpandedArgv.اندازه_2() - 1;
    }

    File_ډېری.ټېل_شا({FName, I + ExpandedArgv.اندازه_2()});
    Argv.erase(Argv.begin() + I);
    Argv.ورننباسه(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.پای_ټکی());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_ډېری.اندازه_2() > 0 && Argv.اندازه_2() == File_ډېری.شا().پای_ټکی_2);
  أرجع Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             Small_ویکټور_Impl<ثابت حرف *> &NewArgv) {
#که_تعریف_شوی _WIN32
  خپلکاری Tokenize = cl::Tokenize_Windows_Command_کرښه;
#وإلا
  خپلکاری Tokenize = cl::Tokenize_GNUCommand_کرښه;
#د_که_پای
  // The environment variable specifies initial options.
  إذا (EnvVar)
    إذا (std::optional<std::string> Env_ارزښت = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_ارزښت, Saver, NewArgv, /*MarkEOLs=*/خطأ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  إذا (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع ریښتیا;
}

منطقي cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_ویکټور_Impl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  إذا (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع ریښتیا;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقي ExpansionContext::پیدا_کړه_Config_File(StringRef FileName,
                                      Small_ویکټور_Impl<حرف> &File_لار) {
  SmallString<128> Cfg_File_لار;
  ثابت خپلکاری FileExists = [هذا](SmallString<128> لار) -> منطقي {
    خپلکاری حالت = FS->حالت_2(لار);
    أرجع حالت &&
           حالت->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  إذا (llvm::sys::لار_2::has_پلار_لار(FileName)) {
    Cfg_File_لار = FileName;
    إذا (llvm::sys::لار_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_لار))
      أرجع خطأ;
    إذا (!FileExists(Cfg_File_لار))
      أرجع خطأ;
    File_لار.ټاکل(Cfg_File_لار.begin(), Cfg_File_لار.پای_ټکی());
    أرجع ریښتیا;
  }

  // Look for the file in search directories.
  لكل (ثابت StringRef &Dir : پلټنه_Dirs) {
    إذا (Dir.تشه())
      استمر;
    Cfg_File_لار.ټاکل(Dir);
    llvm::sys::لار_2::append(Cfg_File_لار, FileName);
    llvm::sys::لار_2::native(Cfg_File_لار);
    إذا (FileExists(Cfg_File_لار)) {
      File_لار.ټاکل(Cfg_File_لار.begin(), Cfg_File_لار.پای_ټکی());
      أرجع ریښتیا;
    }
  }

  أرجع خطأ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_ویکټور_Impl<ثابت حرف *> &Argv) {
  SmallString<128> Abs_لار;
  إذا (sys::لار_2::is_relative(CfgFile)) {
    Abs_لار.ټاکل(CfgFile);
    إذا (std::error_کوډ EC = FS->makeAbsolute(Abs_لار))
      أرجع make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_لار.str();
  }
  دننه_Config_File = ریښتیا;
  Relative_نومونه = ریښتیا;
  إذا (Error Err = expandResponseFile(CfgFile, Argv))
    أرجع Err;
  أرجع expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parse_Command_کرښه_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> دلیلونه,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  Small_ویکټور<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ټېل_شا(دلیلونه[0]);

  // Parse options from environment variable.
  إذا (EnvVar) {
    إذا (std::optional<std::string> Env_ارزښت =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_کرښه(*Env_ارزښت, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, دلیلونه.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.اندازه_2());

  // Parse all options.
  أرجع globalParser().Parse_Command_کرښه_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::Parse_Command_کرښه_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_کرښه_Options_Impl(تش_اشاره, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parse_Command_کرښه_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> دلیلونه,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_کرښه_Options_Impl(&Context, دلیلونه, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
فراغ Command_کرښه_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  لكل (خپلکاری *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_ویکټور<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.اندازه_2());
    لكل (خپلکاری &O : SC->OptionsMap)
      Opts.ټېل_شا(O.دوهم_2);
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

منطقي Command_کرښه_Parser::Parse_Command_کرښه_Options(
    صحيح argc, ثابت حرف *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  پروګرام_Overview = Overview;
  منطقي IgnoreErrors = Errs;
  إذا (!Errs)
    Errs = &errs();
  إذا (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقي ErrorParsing = خطأ;

  // Expand response files.
  Small_ویکټور<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#که_تعریف_شوی _WIN32
  خپلکاری Tokenize = cl::Tokenize_Windows_Command_کرښه;
#وإلا
  خپلکاری Tokenize = cl::Tokenize_GNUCommand_کرښه;
#د_که_پای
  ExpansionContext ECtx(A, Tokenize, VFS);
  إذا (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.اندازه_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  پروګرام_Name = std::string(sys::لار_2::filename(StringRef(argv[0])));
  شاخص_Library_Options();

  // Check out the positional arguments to collect information about them.
  بینښه عدد_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = خطأ;

  صحيح لومړی_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_سر_Level();
  std::string تر_ټولو_نږدې_Sub_Command_String;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[لومړی_Arg][0] != '-' && hasNamedSubCommands();
  إذا (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[لومړی_Arg]), تر_ټولو_نږدې_Sub_Command_String);
    إذا (ChosenSubCommand != &SubCommand::get_سر_Level())
      لومړی_Arg = 2;
  }
  globalParser().فعال_Sub_Command = ChosenSubCommand;

  أكد(ChosenSubCommand);
  خپلکاری &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  خپلکاری &PositionalOpts = ChosenSubCommand->PositionalOpts;
  خپلکاری &OptionsMap = ChosenSubCommand->OptionsMap;

  إذا (ConsumeAfterOpt) {
    أكد(PositionalOpts.اندازه_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  إذا (!PositionalOpts.تشه()) {

    // Calculate how many positional values are _required_.
    منطقي Unbounded_وموندل = خطأ;
    لكل (size_t i = 0, e = PositionalOpts.اندازه_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      إذا (Requires_ارزښت(Opt))
        ++عدد_Positional_Required;
      وإلا إذا (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        إذا (PositionalOpts.اندازه_2() > 1) {
          إذا (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = ریښتیا;
        }
      } وإلا إذا (Unbounded_وموندل && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        إذا (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << پروګرام_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.اندازه_2();
        ErrorParsing = ریښتیا;
      }
      Unbounded_وموندل |= Eats_Unbounded_شمېره_د_ارزښتونه(Opt);
    }
    HasUnlimitedPositionals = Unbounded_وموندل || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_ویکټور<std::pair<StringRef, بینښه>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *فعال_Positional_Arg = تش_اشاره;

  // Loop over all of the arguments... processing them.
  منطقي Dash_Dash_وموندل = خطأ; // Have we read '--'?
  لكل (صحيح i = لومړی_Arg; i < argc; ++i) {
    Option *Handler = تش_اشاره;
    std::string تر_ټولو_نږدې_Handler_String;
    StringRef ارزښت;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    إذا (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_وموندل) {
      // Positional argument!
      إذا (فعال_Positional_Arg) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      إذا (!PositionalOpts.تشه()) {
        PositionalVals.ټېل_شا(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        إذا (PositionalVals.اندازه_2() >= عدد_Positional_Required && ConsumeAfterOpt) {
          لكل (++i; i < argc; ++i)
            PositionalVals.ټېل_شا(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } وإلا إذا (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_وموندل) {
      Dash_Dash_وموندل = ریښتیا; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } وإلا إذا (فعال_Positional_Arg &&
               (فعال_Positional_Arg->getMiscFlags() & Positional_Eats_دلیلونه)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ارزښت);
      إذا (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }
    } وإلا { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ارزښت);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      إذا (!Handler && ChosenSubCommand != &SubCommand::get_سر_Level())
        Handler = LookupOption(SubCommand::get_سر_Level(), ArgName, ارزښت);

      إذا (!Handler) {
        إذا (LibraryOptions *L = lookupLibraryOption(ArgName.وېشل('=').لومړی_2)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          بینښه N;
          إذا (Error E = Context && L->is_فی_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << پروګرام_Name << ": " << منزل_String(std::move(E)) << '\n';
            ErrorParsing = ریښتیا;
          }
          i += N - 1;
          استمر;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      إذا (!Handler)
        Handler = HandlePrefixedOption(ArgName, ارزښت, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      إذا (!Handler)
        Lookup_تر_ټولو_نږدې_Option(ArgName, OptionsMap, تر_ټولو_نږدې_Handler_String);
    }

    إذا (!Handler) {
      خپلکاری Report_Unknown_دلیل = [&](منطقي IsArg,
                                       StringRef تر_ټولو_نږدې_دلیل_Name) {
        *Errs << پروګرام_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        إذا (تر_ټولو_نږدې_دلیل_Name.تشه())
          أرجع;

        *Errs << پروګرام_Name << ": Did you mean '";
        إذا (IsArg)
          *Errs << PrintArg(تر_ټولو_نږدې_دلیل_Name, 0);
        وإلا
          *Errs << تر_ټولو_نږدې_دلیل_Name;
        *Errs << "'?\n";
      };

      إذا (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_دلیل(/*IsArg=*/ریښتیا, تر_ټولو_نږدې_Handler_String);
      وإلا
        Report_Unknown_دلیل(/*IsArg=*/خطأ, تر_ټولو_نږدې_Sub_Command_String);

      ErrorParsing = ریښتیا;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    إذا (Handler->getFormattingFlag() == cl::Positional) {
      إذا ((Handler->getMiscFlags() & Positional_Eats_دلیلونه) && !ارزښت.تشه()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = ریښتیا;
      }
      فعال_Positional_Arg = Handler;
    }
    وإلا
      ErrorParsing |= ProvideOption(Handler, ArgName, ارزښت, argc, argv, i);
  }

  // Check and handle positional arguments now...
  إذا (عدد_Positional_Required > PositionalVals.اندازه_2()) {
      *Errs << پروګرام_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << عدد_Positional_Required
             << " positional argument" << (عدد_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = ریښتیا;
  } وإلا إذا (!HasUnlimitedPositionals &&
             PositionalVals.اندازه_2() > PositionalOpts.اندازه_2()) {
    *Errs << پروګرام_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.اندازه_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = ریښتیا;

  } وإلا إذا (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    بینښه ValNo = 0, عدد_Vals = static_cast<بینښه>(PositionalVals.اندازه_2());
    لكل (Option *Opt : PositionalOpts) {
      إذا (Requires_ارزښت(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].لومړی_2,
                                PositionalVals[ValNo].دوهم_2);
        ValNo++;
        --عدد_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي بشپړ = Opt->get_عدد_Occurrences_Flag() == cl::Required;
      طالما (عدد_Vals - ValNo > عدد_Positional_Required && !بشپړ) {
        بدل (Opt->get_عدد_Occurrences_Flag()) {
        حالة cl::Optional:
          بشپړ = ریښتیا; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالة cl::Zero_Or_نور: // Zero or more will take all they can get...
        حالة cl::One_Or_نور:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].لومړی_2,
                                  PositionalVals[ValNo].دوهم_2);
          ValNo++;
          اكسر;
        افتراضي:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وإلا {
    أكد(ConsumeAfterOpt && عدد_Positional_Required <= PositionalVals.اندازه_2());
    بینښه ValNo = 0;
    لكل (Option *Opt : PositionalOpts)
      إذا (Requires_ارزښت(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].لومړی_2, PositionalVals[ValNo].دوهم_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    إذا (PositionalOpts.اندازه_2() == 1 && ValNo == 0 && !PositionalVals.تشه()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].لومړی_2,
                                              PositionalVals[ValNo].دوهم_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    لكل (; ValNo != PositionalVals.اندازه_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].لومړی_2,
                                  PositionalVals[ValNo].دوهم_2);
  }

  // Loop over args and make sure all required args are specified!
  لكل (ثابت خپلکاری &Opt : OptionsMap) {
    بدل (Opt.دوهم_2->get_عدد_Occurrences_Flag()) {
    حالة Required:
    حالة One_Or_نور:
      إذا (Opt.دوهم_2->get_عدد_Occurrences() == 0) {
        Opt.دوهم_2->error("must be specified at least once!");
        ErrorParsing = ریښتیا;
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
  نور_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  إذا (ErrorParsing) {
    إذا (!IgnoreErrors)
      وتل(1);
    أرجع خطأ;
  }
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &پیغام_3, StringRef ArgName, raw_ostream &Errs) {
  إذا (!ArgName.معلومات())
    ArgName = ArgStr;
  إذا (ArgName.تشه())
    Errs << HelpStr; // Be nice for positional arguments
  وإلا
    Errs << globalParser().پروګرام_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << پیغام_3 << "\n";
  أرجع ریښتیا;
}

منطقي Option::جمع_کړه_Occurrence(بینښه pos, StringRef ArgName, StringRef ارزښت) {
  ++عدد_Occurrences;
  أرجع handleOccurrence(pos, ArgName, ارزښت);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef get_ارزښت_Str(ثابت Option &O, StringRef Default_پیغام) {
  إذا (O.ارزښت_Str.تشه())
    أرجع Default_پیغام;
  أرجع O.ارزښت_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_پلنوالی() ثابت {
  أرجع arg_Plus_Prefixes_اندازه(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t لومړی_کرښه_Indented_By) {
  أكد(Indent >= لومړی_کرښه_Indented_By);
  std::pair<StringRef, StringRef> وېشل_2 = HelpStr.وېشل('\n');
  outs().indent(Indent - لومړی_کرښه_Indented_By)
      << Arg_Help_مختاړی << وېشل_2.لومړی_2 << "\n";
  طالما (!وېشل_2.دوهم_2.تشه()) {
    وېشل_2 = وېشل_2.دوهم_2.وېشل('\n');
    outs().indent(Indent) << وېشل_2.لومړی_2 << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t بنسټ_Indent,
                                 size_t لومړی_کرښه_Indented_By) {
  ثابت StringRef Val_Help_مختاړی = "  ";
  أكد(بنسټ_Indent >= لومړی_کرښه_Indented_By);
  std::pair<StringRef, StringRef> وېشل_2 = HelpStr.وېشل('\n');
  outs().indent(بنسټ_Indent - لومړی_کرښه_Indented_By)
      << Arg_Help_مختاړی << Val_Help_مختاړی << وېشل_2.لومړی_2 << "\n";
  طالما (!وېشل_2.دوهم_2.تشه()) {
    وېشل_2 = وېشل_2.دوهم_2.وېشل('\n');
    outs().indent(بنسټ_Indent + Val_Help_مختاړی.اندازه_2()) << وېشل_2.لومړی_2 << "\n";
  }
}

// Print out the option for the alias.
فراغ alias::printOptionInfo(size_t Global_پلنوالی) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_پلنوالی, arg_Plus_Prefixes_اندازه(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_پلنوالی(ثابت Option &O) ثابت {
  size_t Len = arg_Plus_Prefixes_اندازه(O.ArgStr);
  خپلکاری ValName = get_ارزښت_Name();
  إذا (!ValName.تشه()) {
    size_t FormattingLen = 3;
    إذا (O.getMiscFlags() & Positional_Eats_دلیلونه)
      FormattingLen = 6;
    Len += get_ارزښت_Str(O, ValName).اندازه_2() + FormattingLen;
  }

  أرجع Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t Global_پلنوالی) ثابت {
  outs() << PrintArg(O.ArgStr);

  خپلکاری ValName = get_ارزښت_Name();
  إذا (!ValName.تشه()) {
    إذا (O.getMiscFlags() & Positional_Eats_دلیلونه) {
      outs() << " <" << get_ارزښت_Str(O, ValName) << ">...";
    } وإلا إذا (O.get_ارزښت_Expected_Flag() == ارزښت_Optional)
      outs() << "[=<" << get_ارزښت_Str(O, ValName) << ">]";
    وإلا {
      outs() << (O.ArgStr.اندازه_2() == 1 ? " <" : "=<") << get_ارزښت_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_پلنوالی, get_Option_پلنوالی(O));
}

فراغ basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t Global_پلنوالی) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_پلنوالی - O.ArgStr.اندازه_2());
}

// parser<bool> implementation
//
منطقي parser<منطقي>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقي &ارزښت) {
  أرجع parseBool<منطقي, ریښتیا, خطأ>(O, ArgName, Arg, ارزښت);
}

// parser<boolOrDefault> implementation
//
منطقي parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &ارزښت) {
  أرجع parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, ارزښت);
}

// parser<FixedOrScalableQuantity> implementation
//
کينډۍ <ډول_نوم FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ارزښت_ډول,
                                         FixedOrScalableQuantityT &ارزښت) {
  کارول ScalarTy = ډول_نوم FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_ارزښت;
  إذا (!Arg.getAsInteger(0, Min_ارزښت)) {
    ارزښت = FixedOrScalableQuantityT::getFixed(Min_ارزښت);
    أرجع خطأ;
  }

  StringRef Remainder = Arg;
  إذا (!Remainder.consume_front("vscale"))
    أرجع O.error("'" + Arg + "' value invalid for " + ارزښت_ډول +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (!Remainder.consume_front('x'))
    أرجع O.error("'" + Arg + "' value invalid for " + ارزښت_ډول +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (Remainder.getAsInteger(0, Min_ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for " + ارزښت_ډول +
                   " argument!");

  ارزښت = FixedOrScalableQuantityT::getScalable(Min_ارزښت);
  أرجع خطأ;
}

// parser<int> implementation
//
منطقي parser<صحيح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحيح &ارزښت) {
  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for integer argument!");
  أرجع خطأ;
}

// parser<long> implementation
//
منطقي parser<اوږد>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         اوږد &ارزښت) {
  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for long argument!");
  أرجع خطأ;
}

// parser<long long> implementation
//
منطقي parser<اوږد اوږد>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              اوږد اوږد &ارزښت) {
  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for llong argument!");
  أرجع خطأ;
}

// parser<unsigned> implementation
//
منطقي parser<بینښه>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             بینښه &ارزښت) {

  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for uint argument!");
  أرجع خطأ;
}

// parser<unsigned long> implementation
//
منطقي parser<بینښه اوږد>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  بینښه اوږد &ارزښت) {

  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for ulong argument!");
  أرجع خطأ;
}

// parser<unsigned long long> implementation
//
منطقي parser<بینښه اوږد اوږد>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       بینښه اوږد اوږد &ارزښت) {

  إذا (Arg.getAsInteger(0, ارزښت))
    أرجع O.error("'" + Arg + "' value invalid for ullong argument!");
  أرجع خطأ;
}

// parser<ElementCount> implementation
//
منطقي parser<Element_شمېر>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_شمېر &ارزښت) {
  أرجع parseFixedOrScalableQuantity(O, Arg, get_ارزښت_Name(), ارزښت);
}

// parser<double>/parser<float> implementation
//
ساكن منطقي parseDouble(Option &O, StringRef Arg, مزدوج &ارزښت) {
  إذا (منزل_float(Arg, ارزښت))
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
    أرجع ریښتیا;
  Val = (عائم)dVal;
  أرجع خطأ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
بینښه generic_parser_بنسټ::پیدا_کړه_Option(StringRef Name) {
  بینښه e = get_عدد_Options();

  لكل (بینښه i = 0; i != e; ++i) {
    إذا (getOption(i) == Name)
      أرجع i;
  }
  أرجع e;
}

ساكن StringRef Eq_ارزښت = "=<value>";
ساكن StringRef تشه_Option = "<empty>";
ساكن StringRef Option_مختاړی = "    =";
ساكن size_t get_Option_Prefixes_اندازه() {
  أرجع Option_مختاړی.اندازه_2() + Arg_Help_مختاړی.اندازه_2();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  أرجع O.get_ارزښت_Expected_Flag() != ارزښت_Optional || !Name.تشه() ||
         !Description.تشه();
}

// Return the width of the option tag for printing...
size_t generic_parser_بنسټ::get_Option_پلنوالی(ثابت Option &O) ثابت {
  إذا (O.hasArgStr()) {
    size_t اندازه =
        arg_Plus_Prefixes_اندازه(O.ArgStr) + Eq_ارزښت.اندازه_2();
    لكل (بینښه i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      إذا (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t Name_اندازه = Name.تشه() ? تشه_Option.اندازه_2() : Name.اندازه_2();
      اندازه = std::max(اندازه, Name_اندازه + get_Option_Prefixes_اندازه());
    }
    أرجع اندازه;
  } وإلا {
    size_t بنسټ_اندازه = 0;
    لكل (بینښه i = 0, e = get_عدد_Options(); i != e; ++i)
      بنسټ_اندازه = std::max(بنسټ_اندازه, getOption(i).اندازه_2() + 8);
    أرجع بنسټ_اندازه;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_بنسټ::printOptionInfo(ثابت Option &O,
                                          size_t Global_پلنوالی) ثابت {
  إذا (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    إذا (O.get_ارزښت_Expected_Flag() == ارزښت_Optional) {
      لكل (بینښه i = 0, e = get_عدد_Options(); i != e; ++i) {
        إذا (getOption(i).تشه()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_پلنوالی,
                               arg_Plus_Prefixes_اندازه(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_ارزښت;
    Option::printHelpStr(O.HelpStr, Global_پلنوالی,
                         Eq_ارزښت.اندازه_2() +
                             arg_Plus_Prefixes_اندازه(O.ArgStr));
    لكل (بینښه i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      إذا (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t لومړی_کرښه_Indent = OptionName.اندازه_2() + get_Option_Prefixes_اندازه();
      outs() << Option_مختاړی << OptionName;
      إذا (OptionName.تشه()) {
        outs() << تشه_Option;
        أكد(لومړی_کرښه_Indent >= تشه_Option.اندازه_2());
        لومړی_کرښه_Indent += تشه_Option.اندازه_2();
      }
      إذا (!Description.تشه())
        Option::printEnumValHelpStr(Description, Global_پلنوالی, لومړی_کرښه_Indent);
      وإلا
        outs() << '\n';
    }
  } وإلا {
    إذا (!O.HelpStr.تشه())
      outs() << "  " << O.HelpStr << '\n';
    لكل (بینښه i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_پلنوالی, Option.اندازه_2() + 8);
    }
  }
}

ساكن ثابت size_t Max_Opt_پلنوالی = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
فراغ generic_parser_بنسټ::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_ارزښت &ارزښت,
    ثابت Generic_Option_ارزښت &Default, size_t Global_پلنوالی) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_پلنوالی - O.ArgStr.اندازه_2());

  بینښه عدد_Opts = get_عدد_Options();
  لكل (بینښه i = 0; i != عدد_Opts; ++i) {
    إذا (!ارزښت.compare(get_Option_ارزښت(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).اندازه_2();
    size_t عدد_Spaces = Max_Opt_پلنوالی > L ? Max_Opt_پلنوالی - L : 0;
    outs().indent(عدد_Spaces) << " (default: ";
    لكل (بینښه j = 0; j != عدد_Opts; ++j) {
      إذا (!Default.compare(get_Option_ارزښت(j)))
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
ساكن raw_ostream &چلوونکی<<(raw_ostream &OS, boolOrDefault V) {
  أرجع OS << static_cast<صحيح>(V);
}
} // namespace cl
} // namespace llvm

#تعریف_کړه PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, Option_ارزښت<T> D,      \
                                  size_t Global_پلنوالی) ثابت {                  \
    printOptionName(O, Global_پلنوالی);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t عدد_Spaces =                                                         \
        Max_Opt_پلنوالی > Str.اندازه_2() ? Max_Opt_پلنوالی - Str.اندازه_2() : 0;               \
    outs().indent(عدد_Spaces) << " (default: ";                                 \
    إذا (D.has_ارزښت())                                                          \
      outs() << D.get_ارزښت();                                                  \
    وإلا                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقي)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحيح)
PRINT_OPT_DIFF(اوږد)
PRINT_OPT_DIFF(اوږد اوږد)
PRINT_OPT_DIFF(بینښه)
PRINT_OPT_DIFF(بینښه اوږد)
PRINT_OPT_DIFF(بینښه اوږد اوږد)
PRINT_OPT_DIFF(مزدوج)
PRINT_OPT_DIFF(عائم)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_شمېر)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_ارزښت<std::string> &D,
                                          size_t Global_پلنوالی) ثابت {
  printOptionName(O, Global_پلنوالی);
  outs() << "= " << V;
  size_t عدد_Spaces = Max_Opt_پلنوالی > V.اندازه_2() ? Max_Opt_پلنوالی - V.اندازه_2() : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  إذا (D.has_ارزښت())
    outs() << D.get_ارزښت();
  وإلا
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_ارزښت<std::optional<std::string>> &D,
    size_t Global_پلنوالی) ثابت {
  printOptionName(O, Global_پلنوالی);
  outs() << "= " << V;
  size_t VSize = V.has_ارزښت_2() ? V.ارزښت_2().اندازه_2() : 0;
  size_t عدد_Spaces = Max_Opt_پلنوالی > VSize ? Max_Opt_پلنوالی - VSize : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  إذا (D.has_ارزښت() && D.get_ارزښت().has_ارزښت_2())
    outs() << D.get_ارزښت();
  وإلا
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::print_Option_No_ارزښت(ثابت Option &O,
                                           size_t Global_پلنوالی) ثابت {
  printOptionName(O, Global_پلنوالی);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  أرجع strcmp(LHS->لومړی_2, RHS->لومړی_2);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  أرجع strcmp(LHS->لومړی_2, RHS->لومړی_2);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ ترتیبول_Opts(OptionsMapTy &OptMap,
                     Small_ویکټور_Impl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  Small_Ptr_سېټ<Option *, 32> Option_سېټ; // Duplicate option detection.

  لكل (خپلکاری I = OptMap.begin(), E = OptMap.پای_ټکی(); I != E; ++I) {
    // Ignore really-hidden options.
    إذا (I->دوهم_2->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    إذا (I->دوهم_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    إذا (!Option_سېټ.ورننباسه(I->دوهم_2).دوهم_2)
      استمر;

    Opts.ټېل_شا(
        std::pair<ثابت حرف *, Option *>(I->لومړی_2.معلومات(), I->دوهم_2));
  }

  // Sort the options list alphabetically.
  array_pod_ترتیبول(Opts.begin(), Opts.پای_ټکی(), OptNameCompare);
}

ساكن فراغ
ترتیبول_Sub_Commands(ثابت Small_Ptr_سېټ_Impl<SubCommand *> &SubMap,
                Small_ویکټور_Impl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  لكل (خپلکاری *S : SubMap) {
    إذا (S->getName().تشه())
      استمر;
    Subs.ټېل_شا(std::make_pair(S->getName().معلومات(), S));
  }
  array_pod_ترتیبول(Subs.begin(), Subs.پای_ټکی(), SubNameCompare);
}

نطاق {

صنف HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  کارول Str_Option_Pair_ویکټور =
      Small_ویکټور<std::pair<ثابت حرف *, Option *>, 128>;
  کارول Str_Sub_Command_Pair_ویکټور =
      Small_ویکټور<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازي فراغ printOptions(Str_Option_Pair_ویکټور &Opts, size_t MaxArgLen) {
    لكل (ثابت خپلکاری &Opt : Opts)
      Opt.دوهم_2->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(Str_Sub_Command_Pair_ویکټور &Subs, size_t MaxSubLen) {
    لكل (ثابت خپلکاری &S : Subs) {
      outs() << "  " << S.لومړی_2;
      إذا (!S.دوهم_2->getDescription().تشه()) {
        outs().indent(MaxSubLen - مزي_اوږدوالی(S.لومړی_2));
        outs() << " - " << S.دوهم_2->getDescription();
      }
      outs() << "\n";
    }
  }

عام:
  څرګند HelpPrinter(منطقي showHidden) : ShowHidden(showHidden) {}
  مجازي ~HelpPrinter() = افتراضي;

  // Invoke the printer.
  فراغ چلوونکی=(منطقي ارزښت) {
    إذا (!ارزښت)
      أرجع;
    printHelp();

    // Halt the program since help information was printed
    وتل(0);
  }

  فراغ printHelp() {
    SubCommand *Sub = globalParser().get_فعال_Sub_Command();
    خپلکاری &OptionsMap = Sub->OptionsMap;
    خپلکاری &PositionalOpts = Sub->PositionalOpts;
    خپلکاری &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_ویکټور Opts;
    ترتیبول_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_ویکټور Subs;
    ترتیبول_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    إذا (!globalParser().پروګرام_Overview.تشه())
      outs() << "OVERVIEW: " << globalParser().پروګرام_Overview << "\n";

    إذا (Sub == &SubCommand::get_سر_Level()) {
      outs() << "USAGE: " << globalParser().پروګرام_Name;
      إذا (!Subs.تشه())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وإلا {
      إذا (!Sub->getDescription().تشه()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().پروګرام_Name << " " << Sub->getName()
             << " [options]";
    }

    لكل (خپلکاری *Opt : PositionalOpts) {
      إذا (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    إذا (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    إذا (Sub == &SubCommand::get_سر_Level() && !Subs.تشه()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      لكل (ثابت خپلکاری &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, مزي_اوږدوالی(Sub.لومړی_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().پروګرام_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    لكل (ثابت خپلکاری &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.دوهم_2->get_Option_پلنوالی());

    Small_ویکټور<std::pair<std::string, StringRef>, 0> LibraryOpts;
    إذا (ShowHidden)
      لكل (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              إذا (!Help.تشه())
                LibraryOpts.emplace_شا((Spelling + MetaVar).str(), Help);
            });
    llvm::ترتیبول(LibraryOpts);
    لكل (ثابت خپلکاری &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_اندازه(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    إذا (!LibraryOpts.تشه())
      outs() << "\nLibrary options:\n\n";
    لكل (ثابت خپلکاری &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_اندازه(Name));
    }

    // Print any extra help the user has declared.
    لكل (ثابت خپلکاری &I : globalParser().نور_Help)
      outs() << I;
    globalParser().نور_Help.clear();
  }
};

صنف CategorizedHelpPrinter : عام HelpPrinter {
عام:
  څرګند CategorizedHelpPrinter(منطقي showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساكن صحيح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    أرجع (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  کارول HelpPrinter::چلوونکی=;

محمي:
  فراغ printOptions(Str_Option_Pair_ویکټور &Opts, size_t MaxArgLen) override {
    std::ویکټور<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::ویکټور<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.اندازه_2() > 0 && "No option categories registered!");
    array_pod_ترتیبول(SortedCategories.begin(), SortedCategories.پای_ټکی(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    لكل (ثابت خپلکاری &I : Opts) {
      Option *Opt = I.دوهم_2;
      لكل (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ټېل_شا(Opt);
      }
    }

    // Now do printing.
    لكل (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت خپلکاری &CategoryOptions = CategorizedOptions[Category];
      إذا (CategoryOptions.تشه())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      إذا (!Category->getDescription().تشه())
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
  څرګند HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  فراغ چلوونکی=(منطقي ارزښت);
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
#د_که_پای

نطاق {
صنف نسخه_Printer {
عام:
  فراغ print(ثابت std::ویکټور<نسخه_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#که_تعریف_شوی PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وإلا
    OS << "LLVM (http://llvm.org/):\n  ";
#د_که_پای
    OS << PACKAGE_NAME << " version " << PACKAGE_نسخه << "\n  ";
#إذا LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وإلا
    OS << "Optimized build";
#د_که_پای
#که_نه_تعریف_شوی NDEBUG
    OS << " with assertions";
#د_که_پای
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    إذا (!ExtraPrinters.تشه()) {
      لكل (ثابت خپلکاری &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ چلوونکی=(منطقي OptionWasSpecified);
};

بنية Command_کرښه_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_نورمال_Printer{خطأ};
  HelpPrinter UncategorizedHiddenPrinter{ریښتیا};
  CategorizedHelpPrinter Categorized_نورمال_Printer{خطأ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{ریښتیا};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_نورمال_Printer{Uncategorized_نورمال_Printer,
                                          Categorized_نورمال_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, ریښتیا, parser<منطقي>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_نورمال_Printer),
      cl::Hidden,
      cl::ارزښت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, ریښتیا, parser<منطقي>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ارزښت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, ریښتیا, parser<منطقي>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_نورمال_Printer),
      cl::ارزښت_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, ریښتیا, parser<منطقي>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ارزښت_Disallowed,
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

  نسخه_Printer_Ty Override_نسخه_Printer = تش_اشاره;

  std::ویکټور<نسخه_Printer_Ty> Extra_نسخه_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  نسخه_Printer نسخه_Printer_Instance;

  cl::opt<نسخه_Printer, ریښتیا, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(نسخه_Printer_Instance), cl::ارزښت_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساكن ManagedStatic<Command_کرښه_Common_Options> CommonOptions;

ساكن فراغ initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_ګراف_Writer_Options();
  init_سیګنالونه_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_تخم_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ساكن OptionCategory GeneralCategory{"General options"};
  أرجع GeneralCategory;
}

فراغ نسخه_Printer::چلوونکی=(منطقي OptionWasSpecified) {
  إذا (!OptionWasSpecified)
    أرجع;

  إذا (CommonOptions->Override_نسخه_Printer != تش_اشاره) {
    CommonOptions->Override_نسخه_Printer(outs());
    وتل(0);
  }
  print(CommonOptions->Extra_نسخه_Printers);

  وتل(0);
}

فراغ HelpPrinterWrapper::چلوونکی=(منطقي ارزښت) {
  إذا (!ارزښت)
    أرجع;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  إذا (globalParser().RegisteredOptionCategories.اندازه_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.سېټ_Hidden_Flag(NotHidden);

    CategorizedPrinter = ریښتیا; // Invoke categorized printer
  } وإلا {
    UncategorizedPrinter = ریښتیا; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::Print_Option_ارزښتونه() { globalParser().print_Option_ارزښتونه(); }

فراغ Command_کرښه_Parser::print_Option_ارزښتونه() {
  إذا (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    أرجع;

  Small_ویکټور<std::pair<ثابت حرف *, Option *>, 128> Opts;
  ترتیبول_Opts(فعال_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ ریښتیا);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  لكل (ثابت خپلکاری &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.دوهم_2->get_Option_پلنوالی());

  لكل (ثابت خپلکاری &Opt : Opts)
    Opt.دوهم_2->print_Option_ارزښت(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::Print_Help_پیغام(منطقي Hidden, منطقي Categorized) {
  إذا (!Hidden && !Categorized)
    CommonOptions->Uncategorized_نورمال_Printer.printHelp();
  وإلا إذا (!Hidden && Categorized)
    CommonOptions->Categorized_نورمال_Printer.printHelp();
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
#د_که_پای
#که_نه_تعریف_شوی NDEBUG
      "+assertions",
#د_که_پای
#که_تعریف_شوی EXPENSIVE_CHECKS
      "+expensive-checks",
#د_که_پای
#إذا __has_feature(پته_sanitizer)
      "+asan",
#د_که_پای
#إذا __has_feature(dataflow_sanitizer)
      "+dfsan",
#د_که_پای
#إذا __has_feature(hwaddress_sanitizer)
      "+hwasan",
#د_که_پای
#إذا __has_feature(حافظه_sanitizer)
      "+msan",
#د_که_پای
#إذا __has_feature(thread_sanitizer)
      "+tsan",
#د_که_پای
#إذا __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#د_که_پای
#که_تعریف_شوی LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#د_که_پای
  };
  أرجع ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#إذا LLVM_نسخه_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#د_که_پای
}

/// Utility function for printing version number.
فراغ cl::Print_نسخه_پیغام() {
  CommonOptions->نسخه_Printer_Instance.print(CommonOptions->Extra_نسخه_Printers);
}

فراغ cl::سېټ_نسخه_Printer(نسخه_Printer_Ty func) {
  CommonOptions->Override_نسخه_Printer = func;
}

فراغ cl::جمع_کړه_Extra_نسخه_Printer(نسخه_Printer_Ty func) {
  CommonOptions->Extra_نسخه_Printers.ټېل_شا(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  خپلکاری &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.لري(&Sub));
  أرجع Sub.OptionsMap;
}

iterator_range<Small_Ptr_سېټ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  أرجع globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  لكل (خپلکاری &I : Sub.OptionsMap) {
    منطقي Unrelated = ریښتیا;
    لكل (خپلکاری &Cat : I.دوهم_2->Categories) {
      إذا (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دوهم_2->سېټ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  لكل (خپلکاری &I : Sub.OptionsMap) {
    منطقي Unrelated = ریښتیا;
    لكل (خپلکاری &Cat : I.دوهم_2->Categories) {
      إذا (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دوهم_2->سېټ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::Reset_Command_کرښه_Parser() { globalParser().reset(); }

فراغ cl::جمع_کړه_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ټېل_شا(&L);
}

فراغ LibraryOptionsContext::ټاکل(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  ارزښتونه.clear();
  Saved_دلیلونه.clear();
  ArrayRef<ثابت حرف *> دلیلونه = Other.Saved_دلیلونه;
  لكل (size_t I = 0, E = دلیلونه.اندازه_2(); I != E;) {
    StringRef Name = دلیلونه[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.وېشل('=').لومړی_2);
    بینښه N;
    cantFail(parse(*L, دلیلونه.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ثابت حرف *> دلیلونه,
                                   بینښه &Consumed) {
  خپلکاری [It, Inserted] = ارزښتونه.try_emplace(L.getDefault());
  إذا (Inserted)
    It->دوهم_2 = L.copyDefault(Alloc);
  أرجع parseLibraryOption(L, It->دوهم_2, دلیلونه, Consumed, Alloc, &Saved_دلیلونه);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParse_Command_کرښه_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_کرښه_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
