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
استعمال نطاق llvm;
استعمال نطاق cl;

#وصف_ڪريو DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نطاق llvm {
نطاق cl {
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<ڊگهو>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<ڊگهو ڊگهو>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشان>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشان ڊگهو>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشان ڊگهو ڊگهو>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
سانچو صنف LLVM_EXPORT_TEMPLATE basic_parser<Element_ڳڻپ>;

#إذا !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
سانچو صنف LLVM_EXPORT_TEMPLATE opt<std::string>;
#جيڪڏهن_پورو

سانچو صنف LLVM_EXPORT_TEMPLATE opt<منطقي>;
سانچو صنف LLVM_EXPORT_TEMPLATE opt<حرف>;
سانچو صنف LLVM_EXPORT_TEMPLATE opt<صحيح>;
سانچو صنف LLVM_EXPORT_TEMPLATE opt<بنانشان>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
فراغ Generic_Option_قدر::anchor() {}
فراغ Option_قدر<boolOrDefault>::anchor() {}
فراغ Option_قدر<std::string>::anchor() {}
فراغ Option::anchor() {}
فراغ basic_parser_impl::anchor() {}
فراغ parser<منطقي>::anchor() {}
فراغ parser<boolOrDefault>::anchor() {}
فراغ parser<صحيح>::anchor() {}
فراغ parser<ڊگهو>::anchor() {}
فراغ parser<ڊگهو ڊگهو>::anchor() {}
فراغ parser<بنانشان>::anchor() {}
فراغ parser<بنانشان ڊگهو>::anchor() {}
فراغ parser<بنانشان ڊگهو ڊگهو>::anchor() {}
فراغ parser<مزدوج>::anchor() {}
فراغ parser<عائم>::anchor() {}
فراغ parser<std::string>::anchor() {}
فراغ parser<std::optional<std::string>>::anchor() {}
فراغ parser<حرف>::anchor() {}
فراغ parser<Element_ڳڻپ>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
فراغ opt_bool_anchor() { opt<منطقي> anchor{""}; }
فراغ opt_char_anchor() { opt<حرف> anchor{""}; }
فراغ opt_int_anchor() { opt<صحيح> anchor{""}; }
فراغ opt_unsigned_anchor() { opt<بنانشان> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساكن size_t DefaultPad = 2;

ساكن StringRef Arg_اڳياڙي = "-";
ساكن StringRef Arg_اڳياڙي_Long = "--";
ساكن StringRef Arg_Help_اڳياڙي = " - ";

ساكن size_t arg_Plus_Prefixes_سائيز(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.سائيز_2();
  إذا (Len == 1)
    أرجع Len + Pad + Arg_اڳياڙي.سائيز_2() + Arg_Help_اڳياڙي.سائيز_2();
  أرجع Len + Pad + Arg_اڳياڙي_Long.سائيز_2() + Arg_Help_اڳياڙي.سائيز_2();
}

ساكن SmallString<8> arg_اڳياڙي(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> اڳياڙي;
  لكل (size_t I = 0; I < Pad; ++I) {
    اڳياڙي.ڌڪو_پٺ(' ');
  }
  اڳياڙي.append(ArgName.سائيز_2() > 1 ? Arg_اڳياڙي_Long : Arg_اڳياڙي);
  أرجع اڳياڙي;
}

استعمال OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

صنف PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  دوست raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << arg_اڳياڙي(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  أرجع OS;
}

صنف Command_سٽ_Parser {
عام:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string پروگرام_Name;
  StringRef پروگرام_Overview;

  // This collects additional help to be printed.
  std::ويڪٽر<StringRef> وڌيڪ_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_سيٽ<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_سيٽ<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_ويڪٽر<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_اشاريو;
  size_t انگ_اشاريي_وارو_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_سٽ_Parser() { رجسٽر_Sub_Command(&SubCommand::get_چوٽي_Level()); }

  فراغ ResetAllOptionOccurrences();

  منطقي Parse_Command_سٽ_Options(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = خالي_اشارو,
                               vfs::FileSystem *VFS = خالي_اشارو,
                               LibraryOptionsContext *Context = خالي_اشارو);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    إذا (Opt.Subs.خالي_جايون()) {
      Action(SubCommand::get_چوٽي_Level());
      أرجع;
    }
    إذا (Opt.Subs.سائيز_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      لكل (خودڪار *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      أرجع;
    }
    لكل (خودڪار *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  فراغ جوڙيو_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    إذا (Opt.hasArgStr())
      أرجع;
    إذا (!SC->OptionsMap.وجهو(std::make_pair(Name, &Opt)).ٻيو_2) {
      errs() << پروگرام_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  فراغ جوڙيو_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { جوڙيو_Literal_Option(Opt, &SC, Name); });
  }

  فراغ جوڙيو_Option(Option *O, SubCommand *SC) {
    منطقي HadErrors = خطأ;
    إذا (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      إذا (!SC->OptionsMap.وجهو(std::make_pair(O->ArgStr, O)).ٻيو_2 ||
          (RegisteredSubCommands.شامل_آهي(SC) &&
           Library_اشاريو.شامل_آهي(O->ArgStr))) {
        errs() << پروگرام_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = سچ;
      }
    }

    // Remember information about positional options.
    إذا (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ڌڪو_پٺ(O);
    وإلا إذا (O->get_انگ_Occurrences_Flag() == cl::ConsumeAfter) {
      إذا (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = سچ;
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

  فراغ جوڙيو_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { جوڙيو_Option(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    Small_ويڪٽر<StringRef, 16> Option_نالا;
    O->get_Extra_Option_نالا(Option_نالا);
    إذا (O->hasArgStr())
      Option_نالا.ڌڪو_پٺ(O->ArgStr);

    SubCommand &Sub = *SC;
    لكل (خودڪار Name : Option_نالا) {
      خودڪار I = Sub.OptionsMap.ڳوليو(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      إذا (I != Sub.OptionsMap.پڇاڙي() && I->ٻيو_2 == O)
        Sub.OptionsMap.erase(I);
    }

    إذا (O->getFormattingFlag() == cl::Positional)
      لكل (خودڪار *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.پڇاڙي(); ++Opt) {
        إذا (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    وإلا إذا (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = خالي_اشارو;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    أرجع (!Sub.OptionsMap.خالي_جايون() || !Sub.PositionalOpts.خالي_جايون() ||
            خالي_اشارو != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    لكل (ثابت خودڪار *S : RegisteredSubCommands) {
      إذا (hasOptions(*S))
        أرجع سچ;
    }
    أرجع !Libraries.خالي_جايون();
  }

  منطقي hasNamedSubCommands() ثابت {
    لكل (ثابت خودڪار *S : RegisteredSubCommands)
      إذا (!S->getName().خالي_جايون())
        أرجع سچ;
    أرجع خطأ;
  }

  SubCommand *get_سرگرم_Sub_Command() { أرجع سرگرم_Sub_Command; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    إذا (!Sub.OptionsMap.وجهو(std::make_pair(NewName, O)).ٻيو_2) {
      errs() << پروگرام_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  فراغ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  فراغ print_Option_قدرون();

  فراغ اشاريو_Library_Options() {
    منطقي HadErrors = خطأ;
    لكل (; انگ_اشاريي_وارو_Libraries != Libraries.سائيز_2(); ++انگ_اشاريي_وارو_Libraries) {
      LibraryOptions *L = Libraries[انگ_اشاريي_وارو_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        خودڪار [It, Inserted] = Library_اشاريو.try_emplace(Name, L);
        إذا (Inserted ? none_جو(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 أرجع SC->OptionsMap.شامل_آهي(Name);
                               })
                     : It->ٻيو_2 == L)
          أرجع;
        errs() << پروگرام_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = سچ;
      });
    }
    إذا (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    اشاريو_Library_Options();
    أرجع Library_اشاريو.lookup(Name);
  }

  فراغ رجسٽر_Category(OptionCategory *cat) {
    أكد(ڳڻپ_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             أرجع cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.وجهو(cat);
  }

  فراغ رجسٽر_Sub_Command(SubCommand *sub) {
    أكد(ڳڻپ_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      أرجع (!sub->getName().خالي_جايون()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.وجهو(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    لكل (خودڪار &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.ٻيو_2;
      إذا (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        جوڙيو_Option(O, sub);
      وإلا
        جوڙيو_Literal_Option(*O, sub, E.پهريون_2);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_سيٽ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    أرجع make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.پڇاڙي());
  }

  فراغ reset() {
    سرگرم_Sub_Command = خالي_اشارو;
    پروگرام_Name.clear();
    پروگرام_Overview = StringRef();

    وڌيڪ_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_اشاريو.clear();
    انگ_اشاريي_وارو_Libraries = 0;

    SubCommand::get_چوٽي_Level().reset();
    SubCommand::getAll().reset();
    رجسٽر_Sub_Command(&SubCommand::get_چوٽي_Level());
  }

خاص:
  SubCommand *سرگرم_Sub_Command = خالي_اشارو;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &قدر);
  SubCommand *LookupSubCommand(StringRef Name, std::string &سڀ_کان_ويجهو_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساكن Command_سٽ_Parser &globalParser() {
  ساكن ManagedStatic<Command_سٽ_Parser> GlobalParser;
  أرجع *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ساكن Error
parseLibraryOption(LibraryOptions &L, فراغ *Opts, ArrayRef<ثابت حرف *> دليلون,
                   بنانشان &Consumed, BumpPtrAllocator &Alloc,
                   Small_ويڪٽر_Impl<ثابت حرف *> *رڪارڊ = خالي_اشارو) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  بنانشان انگ_دليلون = std::min<size_t>(دليلون.سائيز_2(), 2);
  لكل (بنانشان J = 0; J != انگ_دليلون; ++J)
    Saved[J] = Saver.save(دليلون[J]).معلومات();
  Consumed = 1;
  إذا (Error E = L.parse(Opts, ArrayRef(Saved, انگ_دليلون), Consumed, Alloc))
    أرجع E;
  إذا (رڪارڊ)
    رڪارڊ->append(Saved, Saved + Consumed);
  أرجع Error::success();
}

سانچو <قسم_نالو T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &قدر) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  إذا (!Arg.معلومات() || Arg == "true" || Arg == "1") {
    قدر = TrueVal;
    أرجع خطأ;
  }

  إذا (Arg == "false" || Arg == "0") {
    قدر = FalseVal;
    أرجع خطأ;
  }
  أرجع O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

فراغ cl::جوڙيو_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().جوڙيو_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().وڌيڪ_Help.ڌڪو_پٺ(Help);
}

Option::Option(انگ_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : انگ_Occurrences(0), Occurrences(OccurrencesFlag), قدر(0),
      HiddenFlag(Hidden), Formatting(عام_Formatting), Misc(0),
      FullyInitialized(خطأ), Position(0) {
  Categories.ڌڪو_پٺ(&getGeneralCategory());
}

فراغ Option::جوڙيو_دليل() {
  globalParser().جوڙيو_Option(هذا);
  FullyInitialized = سچ;
}

فراغ Option::remove_دليل() { globalParser().removeOption(هذا); }

فراغ Option::سيٽ_Arg_Str(StringRef S) {
  إذا (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::جوڙيو_Category(OptionCategory &C) {
  أكد(!Categories.خالي_جايون() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  إذا (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وإلا إذا (!is_contained(Categories, &C))
    Categories.ڌڪو_پٺ(&C);
}

فراغ Option::reset() {
  انگ_Occurrences = 0;
  سيٽ_Default();
}

فراغ OptionCategory::رجسٽر_Category() {
  globalParser().رجسٽر_Category(هذا);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_چوٽي_Level() {
  ساكن ManagedStatic<SubCommand> چوٽي_Level_Sub_Command;
  أرجع *چوٽي_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  أرجع *AllSubCommands;
}

فراغ SubCommand::رجسٽر_Sub_Command() {
  globalParser().رجسٽر_Sub_Command(هذا);
}

فراغ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(هذا);
}

فراغ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = خالي_اشارو;
}

SubCommand::عامل منطقي() ثابت {
  أرجع (globalParser().get_سرگرم_Sub_Command() == هذا);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_سٽ_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &قدر) {
  // Reject all dashes.
  إذا (Arg.خالي_جايون())
    أرجع خالي_اشارو;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.ڳوليو('=');

  // If we have an equals sign, remember the value.
  إذا (EqualPos == StringRef::npos) {
    // Look up the option.
    أرجع Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  خودڪار I = Sub.OptionsMap.ڳوليو(Arg.substr(0, EqualPos));
  إذا (I == Sub.OptionsMap.پڇاڙي())
    أرجع خالي_اشارو;

  خودڪار *O = I->ٻيو_2;
  إذا (O->getFormattingFlag() == cl::Always_اڳياڙي)
    أرجع خالي_اشارو;

  قدر = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  أرجع I->ٻيو_2;
}

SubCommand *Command_سٽ_Parser::LookupSubCommand(StringRef Name,
                                                std::string &سڀ_کان_ويجهو_String) {
  إذا (Name.خالي_جايون())
    أرجع &SubCommand::get_چوٽي_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *سڀ_کان_ويجهو_ميلاپ = خالي_اشارو;
  لكل (خودڪار *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    إذا (S->getName().خالي_جايون())
      استمر;

    إذا (S->getName() == Name)
      أرجع S;

    إذا (!سڀ_کان_ويجهو_ميلاپ && S->getName().edit_مفاصلو(Name) < 2)
      سڀ_کان_ويجهو_ميلاپ = S;
  }

  إذا (سڀ_کان_ويجهو_ميلاپ)
    سڀ_کان_ويجهو_String = سڀ_کان_ويجهو_ميلاپ->getName();

  أرجع &SubCommand::get_چوٽي_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *Lookup_سڀ_کان_ويجهو_Option(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &سڀ_کان_ويجهو_String) {
  // Reject all dashes.
  إذا (Arg.خالي_جايون())
    أرجع خالي_اشارو;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> ورهايو_Arg = Arg.ورهايو('=');
  StringRef &LHS = ورهايو_Arg.پهريون_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = ورهايو_Arg.ٻيو_2;

  // Find the closest match.
  Option *بهترين = خالي_اشارو;
  بنانشان بهترين_مفاصلو = 0;
  لكل (ثابت خودڪار &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    إذا (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    Small_ويڪٽر<StringRef, 16> Option_نالا;
    O->get_Extra_Option_نالا(Option_نالا);
    إذا (O->hasArgStr())
      Option_نالا.ڌڪو_پٺ(O->ArgStr);

    منطقي Permit_قدر = O->get_قدر_Expected_Flag() != cl::قدر_Disallowed;
    StringRef Flag = Permit_قدر ? LHS : Arg;
    لكل (ثابت خودڪار &Name : Option_نالا) {
      بنانشان مفاصلو_2 = StringRef(Name).edit_مفاصلو(
          Flag, /*AllowReplacements=*/سچ, /*MaxEditDistance=*/بهترين_مفاصلو);
      إذا (!بهترين || مفاصلو_2 < بهترين_مفاصلو) {
        بهترين = O;
        بهترين_مفاصلو = مفاصلو_2;
        إذا (RHS.خالي_جايون() || !Permit_قدر)
          سڀ_کان_ويجهو_String = std::string(Name);
        وإلا
          سڀ_کان_ويجهو_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  أرجع بهترين;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي Comma_Separate_And_جوڙيو_Occurrence(Option *Handler, بنانشان pos,
                                          StringRef ArgName, StringRef قدر) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  إذا (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(قدر);
    StringRef::سائيز_type Pos = Val.ڳوليو(',');

    طالما (Pos != StringRef::npos) {
      // Process the portion before the comma.
      إذا (Handler->جوڙيو_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        أرجع سچ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.ڳوليو(',');
    }

    قدر = Val;
  }

  أرجع Handler->جوڙيو_Occurrence(pos, ArgName, قدر);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساكن مضمن منطقي ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef قدر, صحيح argc,
                                 ثابت حرف *ثابت *argv, صحيح &i) {
  // Enforce value requirements
  بدل (Handler->get_قدر_Expected_Flag()) {
  حالة قدر_Required:
    إذا (!قدر.معلومات()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      إذا (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_اڳياڙي)
        أرجع Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      قدر = StringRef(argv[++i]);
    }
    اكسر;
  حالة قدر_Disallowed:
    إذا (قدر.معلومات())
      أرجع Handler->error("does not allow a value! '" + Twine(قدر) +
                            "' specified.");
    اكسر;
  حالة قدر_Optional:
    اكسر;
  }

  أرجع Comma_Separate_And_جوڙيو_Occurrence(Handler, i, ArgName, قدر);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  أرجع ProvideOption(Handler, Handler->ArgStr, Arg, 0, خالي_اشارو, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *ڳوليو_اڳياڙي_Option(StringRef Name, size_t &ڊيگهائي,
                                ثابت OptionsMapTy &OptionsMap) {
  لكل (; !Name.خالي_جايون(); Name = Name.drop_پٺ()) {
    Option *O = OptionsMap.lookup(Name);
    إذا (O && (O->getFormattingFlag() == cl::اڳياڙي ||
              O->getFormattingFlag() == cl::Always_اڳياڙي)) {
      ڊيگهائي = Name.سائيز_2();
      أرجع O;
    }
  }
  أرجع خالي_اشارو;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &قدر,
                                    ثابت OptionsMapTy &OptionsMap) {
  إذا (Arg.سائيز_2() == 1)
    أرجع خالي_اشارو;

  size_t ڊيگهائي = 0;
  Option *POpt = ڳوليو_اڳياڙي_Option(Arg, ڊيگهائي, OptionsMap);
  إذا (!POpt)
    أرجع خالي_اشارو;

  StringRef Maybe_قدر =
      (ڊيگهائي < Arg.سائيز_2()) ? Arg.substr(ڊيگهائي) : StringRef();
  Arg = Arg.substr(0, ڊيگهائي);

  // cl::Prefix options do not preserve '=' when used separately.
  إذا (POpt->getFormattingFlag() == cl::اڳياڙي && Maybe_قدر.starts_with("="))
    Maybe_قدر = Maybe_قدر.drop_front();
  قدر = Maybe_قدر;
  أرجع POpt;
}

ساكن منطقي Requires_قدر(ثابت Option *O) {
  أرجع O->get_انگ_Occurrences_Flag() == cl::Required ||
         O->get_انگ_Occurrences_Flag() == cl::One_Or_وڌيڪ;
}

ساكن منطقي Eats_Unbounded_عدد_جو_قدرون(ثابت Option *O) {
  أرجع O->get_انگ_Occurrences_Flag() == cl::Zero_Or_وڌيڪ ||
         O->get_انگ_Occurrences_Flag() == cl::One_Or_وڌيڪ;
}

ساكن منطقي isWhitespace(حرف C) {
  أرجع C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  أرجع isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { أرجع C == '\"' || C == '\''; }

فراغ cl::Tokenize_GNUCommand_سٽ(StringRef Src, StringSaver &Saver,
                                Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي اندريون_Token = خطأ;
  لكل (size_t I = 0, E = Src.سائيز_2(); I != E; ++I) {
    // Consume runs of whitespace.
    إذا (!اندريون_Token) {
      طالما (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        إذا (MarkEOLs && Src[I] == '\n')
          NewArgv.ڌڪو_پٺ(خالي_اشارو);
        ++I;
      }
      إذا (I == E)
        اكسر;
      اندريون_Token = سچ;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    إذا (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ڌڪو_پٺ(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    إذا (isQuote(C)) {
      ++I;
      طالما (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        إذا (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ڌڪو_پٺ(Src[I]);
        ++I;
      }
      إذا (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    إذا (isWhitespace(C)) {
      NewArgv.ڌڪو_پٺ(Saver.save(Token.str()).معلومات());
      // Mark the end of lines in response files.
      إذا (MarkEOLs && C == '\n')
        NewArgv.ڌڪو_پٺ(خالي_اشارو);
      Token.clear();
      اندريون_Token = خطأ;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.ڌڪو_پٺ(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  إذا (اندريون_Token)
    NewArgv.ڌڪو_پٺ(Saver.save(Token.str()).معلومات());
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
  size_t E = Src.سائيز_2();
  صحيح Backslash_ڳڻپ = 0;
  // Skip the backslashes.
  افعل {
    ++I;
    ++Backslash_ڳڻپ;
  } طالما (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  إذا (FollowedByDoubleQuote) {
    Token.append(Backslash_ڳڻپ / 2, '\\');
    إذا (Backslash_ڳڻپ % 2 == 0)
      أرجع I - 1;
    Token.ڌڪو_پٺ('"');
    أرجع I;
  }
  Token.append(Backslash_ڳڻپ, '\\');
  أرجع I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي is_Windows_Special_Char_اندريون_Command_Name(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenize_Windows_Command_سٽ_Impl(
    StringRef Src, StringSaver &Saver, function_ref<فراغ(StringRef)> جوڙيو_Token,
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

  لكل (size_t I = 0, E = Src.سائيز_2(); I < E; ++I) {
    بدل (State) {
    حالة INIT: {
      أكد(Token.خالي_جايون() && "token should be empty in initial state");
      // Eat whitespace before a token.
      طالما (I < E && isWhitespaceOrNull(Src[I])) {
        إذا (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      إذا (I >= E)
        اكسر;
      size_t شروعات = I;
      إذا (CommandName) {
        طالما (I < E && !is_Windows_Special_Char_اندريون_Command_Name(Src[I]))
          ++I;
      } وإلا {
        طالما (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef عام_Chars = Src.ٽڪرو(شروعات, I);
      إذا (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        جوڙيو_Token(AlwaysCopy ? Saver.save(عام_Chars) : عام_Chars);
        إذا (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } وإلا {
          CommandName = خطأ;
        }
      } وإلا إذا (Src[I] == '\"') {
        Token += عام_Chars;
        State = QUOTED;
      } وإلا إذا (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += عام_Chars;
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
        جوڙيو_Token(Saver.save(Token.str()));
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
        Token.ڌڪو_پٺ(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      إذا (Src[I] == '\"') {
        إذا (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ڌڪو_پٺ('"');
          ++I;
        } وإلا {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.ڌڪو_پٺ(Src[I]);
      }
      اكسر;
    }
  }

  إذا (State != INIT)
    جوڙيو_Token(Saver.save(Token.str()));
}

فراغ cl::Tokenize_Windows_Command_سٽ(StringRef Src, StringSaver &Saver,
                                    Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  خودڪار جوڙيو_Token = [&](StringRef Tok) { NewArgv.ڌڪو_پٺ(Tok.معلومات()); };
  خودڪار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ڌڪو_پٺ(خالي_اشارو);
  };
  tokenize_Windows_Command_سٽ_Impl(Src, Saver, جوڙيو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سٽ_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_ويڪٽر_Impl<StringRef> &NewArgv) {
  خودڪار جوڙيو_Token = [&](StringRef Tok) { NewArgv.ڌڪو_پٺ(Tok); };
  خودڪار OnEOL = []() {};
  tokenize_Windows_Command_سٽ_Impl(Src, Saver, جوڙيو_Token, /*AlwaysCopy=*/خطأ,
                                 OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سٽ_ڀريل(StringRef Src, StringSaver &Saver,
                                        Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  خودڪار جوڙيو_Token = [&](StringRef Tok) { NewArgv.ڌڪو_پٺ(Tok.معلومات()); };
  خودڪار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ڌڪو_پٺ(خالي_اشارو);
  };
  tokenize_Windows_Command_سٽ_Impl(Src, Saver, جوڙيو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, سچ);
}

فراغ cl::tokenizeConfigFile(StringRef ذريعو, StringSaver &Saver,
                            Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  لكل (ثابت حرف *Cur = ذريعو.begin(); Cur != ذريعو.پڇاڙي();) {
    SmallString<128> سٽ;
    // Check for comment line.
    إذا (isWhitespace(*Cur)) {
      طالما (Cur != ذريعو.پڇاڙي() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    إذا (*Cur == '#') {
      طالما (Cur != ذريعو.پڇاڙي() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *شروعات = Cur;
    ثابت حرف *سٽ_پڇاڙي = خالي_اشارو;
    حرف Quote = 0;
    منطقي At_Token_شروعات = سچ;
    لكل (ثابت حرف *پڇاڙي_2 = ذريعو.پڇاڙي(); Cur != پڇاڙي_2; ++Cur) {
      حرف C = *Cur;
      إذا (C == '\\') {
        إذا (Cur + 1 != پڇاڙي_2) {
          ++Cur;
          إذا (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != پڇاڙي_2) && Cur[1] == '\n')) {
            سٽ.append(شروعات, Cur - 1);
            إذا (*Cur == '\r')
              ++Cur;
            شروعات = Cur + 1;
            // Splicing does not introduce a token boundary.
            استمر;
          }
        }
        At_Token_شروعات = خطأ;
        استمر;
      }
      إذا (C == '\n')
        اكسر;
      إذا (Quote) {
        إذا (C == Quote)
          Quote = 0;
      } وإلا إذا (isQuote(C)) {
        Quote = C;
      } وإلا إذا (C == '#' && At_Token_شروعات) {
        سٽ_پڇاڙي = Cur;
        طالما (Cur != پڇاڙي_2 && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      At_Token_شروعات = isWhitespace(C);
    }
    // Tokenize line.
    سٽ.append(شروعات, سٽ_پڇاڙي ? سٽ_پڇاڙي : Cur);
    cl::Tokenize_GNUCommand_سٽ(سٽ, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي has_UTF8_Byte_ترتيب_Mark(ArrayRef<حرف> S) {
  أرجع (S.سائيز_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ Expand_بنياد_Paths(StringRef بنياد_رستو, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::رستو_2::is_absolute(بنياد_رستو));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::سائيز_type شروعات_Pos = 0;
  لكل (StringRef::سائيز_type TokenPos = ArgString.ڳوليو(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.ڳوليو(Token, شروعات_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(شروعات_Pos, TokenPos - شروعات_Pos);
    إذا (ResponseFile.خالي_جايون())
      ResponseFile = LHS;
    وإلا
      llvm::sys::رستو_2::append(ResponseFile, LHS);
    ResponseFile.append(بنياد_رستو);
    شروعات_Pos = TokenPos + Token.سائيز_2();
  }

  إذا (!ResponseFile.خالي_جايون()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef بچيل = ArgString.substr(شروعات_Pos);
    إذا (!بچيل.خالي_جايون())
      llvm::sys::رستو_2::append(ResponseFile, بچيل);
    Arg = Saver.save(ResponseFile.str()).معلومات();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv) {
  أكد(sys::رستو_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<يادگيري_بفر>> MemBufOrErr =
      FS->get_بفر_For_File(FName);
  إذا (!MemBufOrErr) {
    std::error_ڪوڊ EC = MemBufOrErr.getError();
    أرجع llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.پيغام());
  }
  يادگيري_بفر &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_بفر_شروعات(), MemBuf.get_بفر_سائيز());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.get_بفر_شروعات(), MemBuf.get_بفر_پڇاڙي());
  std::string UTF8Buf;
  إذا (has_UTF16_Byte_ترتيب_Mark(BufRef)) {
    إذا (!convert_UTF16_منزل_UTF8_String(BufRef, UTF8Buf))
      أرجع llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  وإلا إذا (has_UTF8_Byte_ترتيب_Mark(BufRef))
    Str = StringRef(BufRef.معلومات() + 3, BufRef.سائيز_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  إذا (!Relative_نالا && !اندريون_Config_File)
    أرجع Error::success();

  StringRef بنياد_رستو = llvm::sys::رستو_2::والد_رستو(FName);
  لكل (ثابت حرف *&Arg : NewArgv) {
    إذا (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    إذا (اندريون_Config_File)
      Expand_بنياد_Paths(بنياد_رستو, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = خطأ;
    إذا (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      إذا (!llvm::sys::رستو_2::is_relative(FileName))
        استمر;
    } وإلا إذا (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = سچ;
    } وإلا {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ڌڪو_پٺ('@');
    إذا (ConfigInclusion && !llvm::sys::رستو_2::has_والد_رستو(FileName)) {
      SmallString<128> File_رستو;
      إذا (!ڳوليو_Config_File(FileName, File_رستو))
        أرجع createStringError(
            std::make_error_ڪوڊ(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_رستو);
    } وإلا {
      ResponseFile.append(بنياد_رستو);
      llvm::sys::رستو_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).معلومات();
  }
  أرجع Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_ويڪٽر_Impl<ثابت حرف *> &Argv) {
  بنية Response_File_رڪارڊ {
    std::string File;
    size_t پڇاڙي_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_ويڪٽر<Response_File_رڪارڊ, 3> File_ڍير;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ڍير.ڌڪو_پٺ({"", Argv.سائيز_2()});

  // Don't cache Argv.size() because it can change.
  لكل (بنانشان I = 0; I != Argv.سائيز_2();) {
    طالما (I == File_ڍير.پٺ().پڇاڙي_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ڍير.ڪڍو_پٺ();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    إذا (Arg == خالي_اشارو) {
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
    إذا (llvm::sys::رستو_2::is_relative(FName)) {
      إذا (CurrentDir.خالي_جايون()) {
        إذا (خودڪار CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وإلا {
          أرجع createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وإلا {
        CurrDir = CurrentDir;
      }
      llvm::sys::رستو_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::حالت> Res = FS->حالت_2(FName);
    إذا (!Res || !Res->exists()) {
      std::error_ڪوڊ EC = Res.getError();
      إذا (!اندريون_Config_File) {
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
                                       "': " + EC.پيغام());
    }
    ثابت llvm::vfs::حالت &File_حالت = Res.get();

    خودڪار IsEquivalent =
        [File_حالت, هذا](ثابت Response_File_رڪارڊ &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::حالت> RHS = FS->حالت_2(RFile.File);
      إذا (!RHS)
        أرجع RHS.getError();
      أرجع File_حالت.equivalent(*RHS);
    };

    // Check for recursive response files.
    لكل (ثابت خودڪار &F : drop_begin(File_ڍير)) {
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
    Small_ويڪٽر<ثابت حرف *, 0> ExpandedArgv;
    إذا (Error Err = expandResponseFile(FName, ExpandedArgv))
      أرجع Err;

    لكل (Response_File_رڪارڊ &رڪارڊ : File_ڍير) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      رڪارڊ.پڇاڙي_2 += ExpandedArgv.سائيز_2() - 1;
    }

    File_ڍير.ڌڪو_پٺ({FName, I + ExpandedArgv.سائيز_2()});
    Argv.erase(Argv.begin() + I);
    Argv.وجهو(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.پڇاڙي());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_ڍير.سائيز_2() > 0 && Argv.سائيز_2() == File_ڍير.پٺ().پڇاڙي_2);
  أرجع Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             Small_ويڪٽر_Impl<ثابت حرف *> &NewArgv) {
#جيڪڏهن_وصف_ٿيل _WIN32
  خودڪار Tokenize = cl::Tokenize_Windows_Command_سٽ;
#وإلا
  خودڪار Tokenize = cl::Tokenize_GNUCommand_سٽ;
#جيڪڏهن_پورو
  // The environment variable specifies initial options.
  إذا (EnvVar)
    إذا (std::optional<std::string> Env_قدر = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_قدر, Saver, NewArgv, /*MarkEOLs=*/خطأ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  إذا (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع سچ;
}

منطقي cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_ويڪٽر_Impl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  إذا (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع سچ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقي ExpansionContext::ڳوليو_Config_File(StringRef FileName,
                                      Small_ويڪٽر_Impl<حرف> &File_رستو) {
  SmallString<128> Cfg_File_رستو;
  ثابت خودڪار FileExists = [هذا](SmallString<128> رستو) -> منطقي {
    خودڪار حالت = FS->حالت_2(رستو);
    أرجع حالت &&
           حالت->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  إذا (llvm::sys::رستو_2::has_والد_رستو(FileName)) {
    Cfg_File_رستو = FileName;
    إذا (llvm::sys::رستو_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_رستو))
      أرجع خطأ;
    إذا (!FileExists(Cfg_File_رستو))
      أرجع خطأ;
    File_رستو.سونپيو(Cfg_File_رستو.begin(), Cfg_File_رستو.پڇاڙي());
    أرجع سچ;
  }

  // Look for the file in search directories.
  لكل (ثابت StringRef &Dir : ڳولا_Dirs) {
    إذا (Dir.خالي_جايون())
      استمر;
    Cfg_File_رستو.سونپيو(Dir);
    llvm::sys::رستو_2::append(Cfg_File_رستو, FileName);
    llvm::sys::رستو_2::native(Cfg_File_رستو);
    إذا (FileExists(Cfg_File_رستو)) {
      File_رستو.سونپيو(Cfg_File_رستو.begin(), Cfg_File_رستو.پڇاڙي());
      أرجع سچ;
    }
  }

  أرجع خطأ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_ويڪٽر_Impl<ثابت حرف *> &Argv) {
  SmallString<128> Abs_رستو;
  إذا (sys::رستو_2::is_relative(CfgFile)) {
    Abs_رستو.سونپيو(CfgFile);
    إذا (std::error_ڪوڊ EC = FS->makeAbsolute(Abs_رستو))
      أرجع make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_رستو.str();
  }
  اندريون_Config_File = سچ;
  Relative_نالا = سچ;
  إذا (Error Err = expandResponseFile(CfgFile, Argv))
    أرجع Err;
  أرجع expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parse_Command_سٽ_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> دليلون,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  Small_ويڪٽر<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ڌڪو_پٺ(دليلون[0]);

  // Parse options from environment variable.
  إذا (EnvVar) {
    إذا (std::optional<std::string> Env_قدر =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_سٽ(*Env_قدر, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, دليلون.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.سائيز_2());

  // Parse all options.
  أرجع globalParser().Parse_Command_سٽ_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::Parse_Command_سٽ_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سٽ_Options_Impl(خالي_اشارو, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parse_Command_سٽ_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> دليلون,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سٽ_Options_Impl(&Context, دليلون, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
فراغ Command_سٽ_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  لكل (خودڪار *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_ويڪٽر<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.سائيز_2());
    لكل (خودڪار &O : SC->OptionsMap)
      Opts.ڌڪو_پٺ(O.ٻيو_2);
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

منطقي Command_سٽ_Parser::Parse_Command_سٽ_Options(
    صحيح argc, ثابت حرف *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  پروگرام_Overview = Overview;
  منطقي IgnoreErrors = Errs;
  إذا (!Errs)
    Errs = &errs();
  إذا (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقي ErrorParsing = خطأ;

  // Expand response files.
  Small_ويڪٽر<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#جيڪڏهن_وصف_ٿيل _WIN32
  خودڪار Tokenize = cl::Tokenize_Windows_Command_سٽ;
#وإلا
  خودڪار Tokenize = cl::Tokenize_GNUCommand_سٽ;
#جيڪڏهن_پورو
  ExpansionContext ECtx(A, Tokenize, VFS);
  إذا (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.سائيز_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  پروگرام_Name = std::string(sys::رستو_2::filename(StringRef(argv[0])));
  اشاريو_Library_Options();

  // Check out the positional arguments to collect information about them.
  بنانشان انگ_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = خطأ;

  صحيح پهريون_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_چوٽي_Level();
  std::string سڀ_کان_ويجهو_Sub_Command_String;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[پهريون_Arg][0] != '-' && hasNamedSubCommands();
  إذا (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[پهريون_Arg]), سڀ_کان_ويجهو_Sub_Command_String);
    إذا (ChosenSubCommand != &SubCommand::get_چوٽي_Level())
      پهريون_Arg = 2;
  }
  globalParser().سرگرم_Sub_Command = ChosenSubCommand;

  أكد(ChosenSubCommand);
  خودڪار &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  خودڪار &PositionalOpts = ChosenSubCommand->PositionalOpts;
  خودڪار &OptionsMap = ChosenSubCommand->OptionsMap;

  إذا (ConsumeAfterOpt) {
    أكد(PositionalOpts.سائيز_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  إذا (!PositionalOpts.خالي_جايون()) {

    // Calculate how many positional values are _required_.
    منطقي Unbounded_مليو = خطأ;
    لكل (size_t i = 0, e = PositionalOpts.سائيز_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      إذا (Requires_قدر(Opt))
        ++انگ_Positional_Required;
      وإلا إذا (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        إذا (PositionalOpts.سائيز_2() > 1) {
          إذا (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = سچ;
        }
      } وإلا إذا (Unbounded_مليو && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        إذا (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << پروگرام_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.سائيز_2();
        ErrorParsing = سچ;
      }
      Unbounded_مليو |= Eats_Unbounded_عدد_جو_قدرون(Opt);
    }
    HasUnlimitedPositionals = Unbounded_مليو || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_ويڪٽر<std::pair<StringRef, بنانشان>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *سرگرم_Positional_Arg = خالي_اشارو;

  // Loop over all of the arguments... processing them.
  منطقي Dash_Dash_مليو = خطأ; // Have we read '--'?
  لكل (صحيح i = پهريون_Arg; i < argc; ++i) {
    Option *Handler = خالي_اشارو;
    std::string سڀ_کان_ويجهو_Handler_String;
    StringRef قدر;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    إذا (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_مليو) {
      // Positional argument!
      إذا (سرگرم_Positional_Arg) {
        ProvidePositionalOption(سرگرم_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      إذا (!PositionalOpts.خالي_جايون()) {
        PositionalVals.ڌڪو_پٺ(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        إذا (PositionalVals.سائيز_2() >= انگ_Positional_Required && ConsumeAfterOpt) {
          لكل (++i; i < argc; ++i)
            PositionalVals.ڌڪو_پٺ(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } وإلا إذا (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_مليو) {
      Dash_Dash_مليو = سچ; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } وإلا إذا (سرگرم_Positional_Arg &&
               (سرگرم_Positional_Arg->getMiscFlags() & Positional_Eats_دليلون)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قدر);
      إذا (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(سرگرم_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }
    } وإلا { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قدر);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      إذا (!Handler && ChosenSubCommand != &SubCommand::get_چوٽي_Level())
        Handler = LookupOption(SubCommand::get_چوٽي_Level(), ArgName, قدر);

      إذا (!Handler) {
        إذا (LibraryOptions *L = lookupLibraryOption(ArgName.ورهايو('=').پهريون_2)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          بنانشان N;
          إذا (Error E = Context && L->is_في_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << پروگرام_Name << ": " << منزل_String(std::move(E)) << '\n';
            ErrorParsing = سچ;
          }
          i += N - 1;
          استمر;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      إذا (!Handler)
        Handler = HandlePrefixedOption(ArgName, قدر, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      إذا (!Handler)
        Lookup_سڀ_کان_ويجهو_Option(ArgName, OptionsMap, سڀ_کان_ويجهو_Handler_String);
    }

    إذا (!Handler) {
      خودڪار Report_Unknown_دليل = [&](منطقي IsArg,
                                       StringRef سڀ_کان_ويجهو_دليل_Name) {
        *Errs << پروگرام_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        إذا (سڀ_کان_ويجهو_دليل_Name.خالي_جايون())
          أرجع;

        *Errs << پروگرام_Name << ": Did you mean '";
        إذا (IsArg)
          *Errs << PrintArg(سڀ_کان_ويجهو_دليل_Name, 0);
        وإلا
          *Errs << سڀ_کان_ويجهو_دليل_Name;
        *Errs << "'?\n";
      };

      إذا (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_دليل(/*IsArg=*/سچ, سڀ_کان_ويجهو_Handler_String);
      وإلا
        Report_Unknown_دليل(/*IsArg=*/خطأ, سڀ_کان_ويجهو_Sub_Command_String);

      ErrorParsing = سچ;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    إذا (Handler->getFormattingFlag() == cl::Positional) {
      إذا ((Handler->getMiscFlags() & Positional_Eats_دليلون) && !قدر.خالي_جايون()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = سچ;
      }
      سرگرم_Positional_Arg = Handler;
    }
    وإلا
      ErrorParsing |= ProvideOption(Handler, ArgName, قدر, argc, argv, i);
  }

  // Check and handle positional arguments now...
  إذا (انگ_Positional_Required > PositionalVals.سائيز_2()) {
      *Errs << پروگرام_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << انگ_Positional_Required
             << " positional argument" << (انگ_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = سچ;
  } وإلا إذا (!HasUnlimitedPositionals &&
             PositionalVals.سائيز_2() > PositionalOpts.سائيز_2()) {
    *Errs << پروگرام_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.سائيز_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = سچ;

  } وإلا إذا (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    بنانشان ValNo = 0, انگ_Vals = static_cast<بنانشان>(PositionalVals.سائيز_2());
    لكل (Option *Opt : PositionalOpts) {
      إذا (Requires_قدر(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].پهريون_2,
                                PositionalVals[ValNo].ٻيو_2);
        ValNo++;
        --انگ_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي پورو = Opt->get_انگ_Occurrences_Flag() == cl::Required;
      طالما (انگ_Vals - ValNo > انگ_Positional_Required && !پورو) {
        بدل (Opt->get_انگ_Occurrences_Flag()) {
        حالة cl::Optional:
          پورو = سچ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالة cl::Zero_Or_وڌيڪ: // Zero or more will take all they can get...
        حالة cl::One_Or_وڌيڪ:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].پهريون_2,
                                  PositionalVals[ValNo].ٻيو_2);
          ValNo++;
          اكسر;
        افتراضي:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وإلا {
    أكد(ConsumeAfterOpt && انگ_Positional_Required <= PositionalVals.سائيز_2());
    بنانشان ValNo = 0;
    لكل (Option *Opt : PositionalOpts)
      إذا (Requires_قدر(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].پهريون_2, PositionalVals[ValNo].ٻيو_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    إذا (PositionalOpts.سائيز_2() == 1 && ValNo == 0 && !PositionalVals.خالي_جايون()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].پهريون_2,
                                              PositionalVals[ValNo].ٻيو_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    لكل (; ValNo != PositionalVals.سائيز_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].پهريون_2,
                                  PositionalVals[ValNo].ٻيو_2);
  }

  // Loop over args and make sure all required args are specified!
  لكل (ثابت خودڪار &Opt : OptionsMap) {
    بدل (Opt.ٻيو_2->get_انگ_Occurrences_Flag()) {
    حالة Required:
    حالة One_Or_وڌيڪ:
      إذا (Opt.ٻيو_2->get_انگ_Occurrences() == 0) {
        Opt.ٻيو_2->error("must be specified at least once!");
        ErrorParsing = سچ;
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
  وڌيڪ_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  إذا (ErrorParsing) {
    إذا (!IgnoreErrors)
      ٻاهر_نڪرو(1);
    أرجع خطأ;
  }
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &پيغام_2, StringRef ArgName, raw_ostream &Errs) {
  إذا (!ArgName.معلومات())
    ArgName = ArgStr;
  إذا (ArgName.خالي_جايون())
    Errs << HelpStr; // Be nice for positional arguments
  وإلا
    Errs << globalParser().پروگرام_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << پيغام_2 << "\n";
  أرجع سچ;
}

منطقي Option::جوڙيو_Occurrence(بنانشان pos, StringRef ArgName, StringRef قدر) {
  ++انگ_Occurrences;
  أرجع handleOccurrence(pos, ArgName, قدر);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef get_قدر_Str(ثابت Option &O, StringRef Default_نياپو) {
  إذا (O.قدر_Str.خالي_جايون())
    أرجع Default_نياپو;
  أرجع O.قدر_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_ويڪر() ثابت {
  أرجع arg_Plus_Prefixes_سائيز(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t پهريون_سٽ_Indented_By) {
  أكد(Indent >= پهريون_سٽ_Indented_By);
  std::pair<StringRef, StringRef> ورهايو_2 = HelpStr.ورهايو('\n');
  outs().indent(Indent - پهريون_سٽ_Indented_By)
      << Arg_Help_اڳياڙي << ورهايو_2.پهريون_2 << "\n";
  طالما (!ورهايو_2.ٻيو_2.خالي_جايون()) {
    ورهايو_2 = ورهايو_2.ٻيو_2.ورهايو('\n');
    outs().indent(Indent) << ورهايو_2.پهريون_2 << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t بنياد_Indent,
                                 size_t پهريون_سٽ_Indented_By) {
  ثابت StringRef Val_Help_اڳياڙي = "  ";
  أكد(بنياد_Indent >= پهريون_سٽ_Indented_By);
  std::pair<StringRef, StringRef> ورهايو_2 = HelpStr.ورهايو('\n');
  outs().indent(بنياد_Indent - پهريون_سٽ_Indented_By)
      << Arg_Help_اڳياڙي << Val_Help_اڳياڙي << ورهايو_2.پهريون_2 << "\n";
  طالما (!ورهايو_2.ٻيو_2.خالي_جايون()) {
    ورهايو_2 = ورهايو_2.ٻيو_2.ورهايو('\n');
    outs().indent(بنياد_Indent + Val_Help_اڳياڙي.سائيز_2()) << ورهايو_2.پهريون_2 << "\n";
  }
}

// Print out the option for the alias.
فراغ alias::printOptionInfo(size_t Global_ويڪر) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_ويڪر, arg_Plus_Prefixes_سائيز(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_ويڪر(ثابت Option &O) ثابت {
  size_t Len = arg_Plus_Prefixes_سائيز(O.ArgStr);
  خودڪار ValName = get_قدر_Name();
  إذا (!ValName.خالي_جايون()) {
    size_t FormattingLen = 3;
    إذا (O.getMiscFlags() & Positional_Eats_دليلون)
      FormattingLen = 6;
    Len += get_قدر_Str(O, ValName).سائيز_2() + FormattingLen;
  }

  أرجع Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t Global_ويڪر) ثابت {
  outs() << PrintArg(O.ArgStr);

  خودڪار ValName = get_قدر_Name();
  إذا (!ValName.خالي_جايون()) {
    إذا (O.getMiscFlags() & Positional_Eats_دليلون) {
      outs() << " <" << get_قدر_Str(O, ValName) << ">...";
    } وإلا إذا (O.get_قدر_Expected_Flag() == قدر_Optional)
      outs() << "[=<" << get_قدر_Str(O, ValName) << ">]";
    وإلا {
      outs() << (O.ArgStr.سائيز_2() == 1 ? " <" : "=<") << get_قدر_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_ويڪر, get_Option_ويڪر(O));
}

فراغ basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t Global_ويڪر) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_ويڪر - O.ArgStr.سائيز_2());
}

// parser<bool> implementation
//
منطقي parser<منطقي>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقي &قدر) {
  أرجع parseBool<منطقي, سچ, خطأ>(O, ArgName, Arg, قدر);
}

// parser<boolOrDefault> implementation
//
منطقي parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &قدر) {
  أرجع parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, قدر);
}

// parser<FixedOrScalableQuantity> implementation
//
سانچو <قسم_نالو FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef قدر_قسم,
                                         FixedOrScalableQuantityT &قدر) {
  استعمال ScalarTy = قسم_نالو FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_قدر;
  إذا (!Arg.getAsInteger(0, Min_قدر)) {
    قدر = FixedOrScalableQuantityT::getFixed(Min_قدر);
    أرجع خطأ;
  }

  StringRef Remainder = Arg;
  إذا (!Remainder.consume_front("vscale"))
    أرجع O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (!Remainder.consume_front('x'))
    أرجع O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (Remainder.getAsInteger(0, Min_قدر))
    أرجع O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  قدر = FixedOrScalableQuantityT::getScalable(Min_قدر);
  أرجع خطأ;
}

// parser<int> implementation
//
منطقي parser<صحيح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحيح &قدر) {
  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for integer argument!");
  أرجع خطأ;
}

// parser<long> implementation
//
منطقي parser<ڊگهو>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ڊگهو &قدر) {
  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for long argument!");
  أرجع خطأ;
}

// parser<long long> implementation
//
منطقي parser<ڊگهو ڊگهو>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              ڊگهو ڊگهو &قدر) {
  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for llong argument!");
  أرجع خطأ;
}

// parser<unsigned> implementation
//
منطقي parser<بنانشان>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             بنانشان &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for uint argument!");
  أرجع خطأ;
}

// parser<unsigned long> implementation
//
منطقي parser<بنانشان ڊگهو>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  بنانشان ڊگهو &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for ulong argument!");
  أرجع خطأ;
}

// parser<unsigned long long> implementation
//
منطقي parser<بنانشان ڊگهو ڊگهو>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       بنانشان ڊگهو ڊگهو &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for ullong argument!");
  أرجع خطأ;
}

// parser<ElementCount> implementation
//
منطقي parser<Element_ڳڻپ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ڳڻپ &قدر) {
  أرجع parseFixedOrScalableQuantity(O, Arg, get_قدر_Name(), قدر);
}

// parser<double>/parser<float> implementation
//
ساكن منطقي parseDouble(Option &O, StringRef Arg, مزدوج &قدر) {
  إذا (منزل_float(Arg, قدر))
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
    أرجع سچ;
  Val = (عائم)dVal;
  أرجع خطأ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
بنانشان generic_parser_بنياد::ڳوليو_Option(StringRef Name) {
  بنانشان e = get_انگ_Options();

  لكل (بنانشان i = 0; i != e; ++i) {
    إذا (getOption(i) == Name)
      أرجع i;
  }
  أرجع e;
}

ساكن StringRef Eq_قدر = "=<value>";
ساكن StringRef خالي_جايون_Option = "<empty>";
ساكن StringRef Option_اڳياڙي = "    =";
ساكن size_t get_Option_Prefixes_سائيز() {
  أرجع Option_اڳياڙي.سائيز_2() + Arg_Help_اڳياڙي.سائيز_2();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  أرجع O.get_قدر_Expected_Flag() != قدر_Optional || !Name.خالي_جايون() ||
         !Description.خالي_جايون();
}

// Return the width of the option tag for printing...
size_t generic_parser_بنياد::get_Option_ويڪر(ثابت Option &O) ثابت {
  إذا (O.hasArgStr()) {
    size_t سائيز =
        arg_Plus_Prefixes_سائيز(O.ArgStr) + Eq_قدر.سائيز_2();
    لكل (بنانشان i = 0, e = get_انگ_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      إذا (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t Name_سائيز = Name.خالي_جايون() ? خالي_جايون_Option.سائيز_2() : Name.سائيز_2();
      سائيز = std::max(سائيز, Name_سائيز + get_Option_Prefixes_سائيز());
    }
    أرجع سائيز;
  } وإلا {
    size_t بنياد_سائيز = 0;
    لكل (بنانشان i = 0, e = get_انگ_Options(); i != e; ++i)
      بنياد_سائيز = std::max(بنياد_سائيز, getOption(i).سائيز_2() + 8);
    أرجع بنياد_سائيز;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_بنياد::printOptionInfo(ثابت Option &O,
                                          size_t Global_ويڪر) ثابت {
  إذا (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    إذا (O.get_قدر_Expected_Flag() == قدر_Optional) {
      لكل (بنانشان i = 0, e = get_انگ_Options(); i != e; ++i) {
        إذا (getOption(i).خالي_جايون()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_ويڪر,
                               arg_Plus_Prefixes_سائيز(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_قدر;
    Option::printHelpStr(O.HelpStr, Global_ويڪر,
                         Eq_قدر.سائيز_2() +
                             arg_Plus_Prefixes_سائيز(O.ArgStr));
    لكل (بنانشان i = 0, e = get_انگ_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      إذا (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t پهريون_سٽ_Indent = OptionName.سائيز_2() + get_Option_Prefixes_سائيز();
      outs() << Option_اڳياڙي << OptionName;
      إذا (OptionName.خالي_جايون()) {
        outs() << خالي_جايون_Option;
        أكد(پهريون_سٽ_Indent >= خالي_جايون_Option.سائيز_2());
        پهريون_سٽ_Indent += خالي_جايون_Option.سائيز_2();
      }
      إذا (!Description.خالي_جايون())
        Option::printEnumValHelpStr(Description, Global_ويڪر, پهريون_سٽ_Indent);
      وإلا
        outs() << '\n';
    }
  } وإلا {
    إذا (!O.HelpStr.خالي_جايون())
      outs() << "  " << O.HelpStr << '\n';
    لكل (بنانشان i = 0, e = get_انگ_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_ويڪر, Option.سائيز_2() + 8);
    }
  }
}

ساكن ثابت size_t Max_Opt_ويڪر = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
فراغ generic_parser_بنياد::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_قدر &قدر,
    ثابت Generic_Option_قدر &Default, size_t Global_ويڪر) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_ويڪر - O.ArgStr.سائيز_2());

  بنانشان انگ_Opts = get_انگ_Options();
  لكل (بنانشان i = 0; i != انگ_Opts; ++i) {
    إذا (!قدر.compare(get_Option_قدر(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).سائيز_2();
    size_t انگ_Spaces = Max_Opt_ويڪر > L ? Max_Opt_ويڪر - L : 0;
    outs().indent(انگ_Spaces) << " (default: ";
    لكل (بنانشان j = 0; j != انگ_Opts; ++j) {
      إذا (!Default.compare(get_Option_قدر(j)))
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
ساكن raw_ostream &عامل<<(raw_ostream &OS, boolOrDefault V) {
  أرجع OS << static_cast<صحيح>(V);
}
} // namespace cl
} // namespace llvm

#وصف_ڪريو PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, Option_قدر<T> D,      \
                                  size_t Global_ويڪر) ثابت {                  \
    printOptionName(O, Global_ويڪر);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t انگ_Spaces =                                                         \
        Max_Opt_ويڪر > Str.سائيز_2() ? Max_Opt_ويڪر - Str.سائيز_2() : 0;               \
    outs().indent(انگ_Spaces) << " (default: ";                                 \
    إذا (D.has_قدر())                                                          \
      outs() << D.get_قدر();                                                  \
    وإلا                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقي)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحيح)
PRINT_OPT_DIFF(ڊگهو)
PRINT_OPT_DIFF(ڊگهو ڊگهو)
PRINT_OPT_DIFF(بنانشان)
PRINT_OPT_DIFF(بنانشان ڊگهو)
PRINT_OPT_DIFF(بنانشان ڊگهو ڊگهو)
PRINT_OPT_DIFF(مزدوج)
PRINT_OPT_DIFF(عائم)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_ڳڻپ)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_قدر<std::string> &D,
                                          size_t Global_ويڪر) ثابت {
  printOptionName(O, Global_ويڪر);
  outs() << "= " << V;
  size_t انگ_Spaces = Max_Opt_ويڪر > V.سائيز_2() ? Max_Opt_ويڪر - V.سائيز_2() : 0;
  outs().indent(انگ_Spaces) << " (default: ";
  إذا (D.has_قدر())
    outs() << D.get_قدر();
  وإلا
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_قدر<std::optional<std::string>> &D,
    size_t Global_ويڪر) ثابت {
  printOptionName(O, Global_ويڪر);
  outs() << "= " << V;
  size_t VSize = V.has_قدر_2() ? V.قدر_2().سائيز_2() : 0;
  size_t انگ_Spaces = Max_Opt_ويڪر > VSize ? Max_Opt_ويڪر - VSize : 0;
  outs().indent(انگ_Spaces) << " (default: ";
  إذا (D.has_قدر() && D.get_قدر().has_قدر_2())
    outs() << D.get_قدر();
  وإلا
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::print_Option_No_قدر(ثابت Option &O,
                                           size_t Global_ويڪر) ثابت {
  printOptionName(O, Global_ويڪر);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  أرجع strcmp(LHS->پهريون_2, RHS->پهريون_2);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  أرجع strcmp(LHS->پهريون_2, RHS->پهريون_2);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ ترتيب_ڏيو_Opts(OptionsMapTy &OptMap,
                     Small_ويڪٽر_Impl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  Small_Ptr_سيٽ<Option *, 32> Option_سيٽ; // Duplicate option detection.

  لكل (خودڪار I = OptMap.begin(), E = OptMap.پڇاڙي(); I != E; ++I) {
    // Ignore really-hidden options.
    إذا (I->ٻيو_2->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    إذا (I->ٻيو_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    إذا (!Option_سيٽ.وجهو(I->ٻيو_2).ٻيو_2)
      استمر;

    Opts.ڌڪو_پٺ(
        std::pair<ثابت حرف *, Option *>(I->پهريون_2.معلومات(), I->ٻيو_2));
  }

  // Sort the options list alphabetically.
  array_pod_ترتيب_ڏيو(Opts.begin(), Opts.پڇاڙي(), OptNameCompare);
}

ساكن فراغ
ترتيب_ڏيو_Sub_Commands(ثابت Small_Ptr_سيٽ_Impl<SubCommand *> &SubMap,
                Small_ويڪٽر_Impl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  لكل (خودڪار *S : SubMap) {
    إذا (S->getName().خالي_جايون())
      استمر;
    Subs.ڌڪو_پٺ(std::make_pair(S->getName().معلومات(), S));
  }
  array_pod_ترتيب_ڏيو(Subs.begin(), Subs.پڇاڙي(), SubNameCompare);
}

نطاق {

صنف HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  استعمال Str_Option_Pair_ويڪٽر =
      Small_ويڪٽر<std::pair<ثابت حرف *, Option *>, 128>;
  استعمال Str_Sub_Command_Pair_ويڪٽر =
      Small_ويڪٽر<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازي فراغ printOptions(Str_Option_Pair_ويڪٽر &Opts, size_t MaxArgLen) {
    لكل (ثابت خودڪار &Opt : Opts)
      Opt.ٻيو_2->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(Str_Sub_Command_Pair_ويڪٽر &Subs, size_t MaxSubLen) {
    لكل (ثابت خودڪار &S : Subs) {
      outs() << "  " << S.پهريون_2;
      إذا (!S.ٻيو_2->getDescription().خالي_جايون()) {
        outs().indent(MaxSubLen - لڙي_ڊيگهه(S.پهريون_2));
        outs() << " - " << S.ٻيو_2->getDescription();
      }
      outs() << "\n";
    }
  }

عام:
  واضح HelpPrinter(منطقي showHidden) : ShowHidden(showHidden) {}
  مجازي ~HelpPrinter() = افتراضي;

  // Invoke the printer.
  فراغ عامل=(منطقي قدر) {
    إذا (!قدر)
      أرجع;
    printHelp();

    // Halt the program since help information was printed
    ٻاهر_نڪرو(0);
  }

  فراغ printHelp() {
    SubCommand *Sub = globalParser().get_سرگرم_Sub_Command();
    خودڪار &OptionsMap = Sub->OptionsMap;
    خودڪار &PositionalOpts = Sub->PositionalOpts;
    خودڪار &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_ويڪٽر Opts;
    ترتيب_ڏيو_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_ويڪٽر Subs;
    ترتيب_ڏيو_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    إذا (!globalParser().پروگرام_Overview.خالي_جايون())
      outs() << "OVERVIEW: " << globalParser().پروگرام_Overview << "\n";

    إذا (Sub == &SubCommand::get_چوٽي_Level()) {
      outs() << "USAGE: " << globalParser().پروگرام_Name;
      إذا (!Subs.خالي_جايون())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وإلا {
      إذا (!Sub->getDescription().خالي_جايون()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().پروگرام_Name << " " << Sub->getName()
             << " [options]";
    }

    لكل (خودڪار *Opt : PositionalOpts) {
      إذا (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    إذا (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    إذا (Sub == &SubCommand::get_چوٽي_Level() && !Subs.خالي_جايون()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      لكل (ثابت خودڪار &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, لڙي_ڊيگهه(Sub.پهريون_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().پروگرام_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    لكل (ثابت خودڪار &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.ٻيو_2->get_Option_ويڪر());

    Small_ويڪٽر<std::pair<std::string, StringRef>, 0> LibraryOpts;
    إذا (ShowHidden)
      لكل (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              إذا (!Help.خالي_جايون())
                LibraryOpts.emplace_پٺ((Spelling + MetaVar).str(), Help);
            });
    llvm::ترتيب_ڏيو(LibraryOpts);
    لكل (ثابت خودڪار &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_سائيز(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    إذا (!LibraryOpts.خالي_جايون())
      outs() << "\nLibrary options:\n\n";
    لكل (ثابت خودڪار &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_سائيز(Name));
    }

    // Print any extra help the user has declared.
    لكل (ثابت خودڪار &I : globalParser().وڌيڪ_Help)
      outs() << I;
    globalParser().وڌيڪ_Help.clear();
  }
};

صنف CategorizedHelpPrinter : عام HelpPrinter {
عام:
  واضح CategorizedHelpPrinter(منطقي showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساكن صحيح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    أرجع (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  استعمال HelpPrinter::عامل=;

محمي:
  فراغ printOptions(Str_Option_Pair_ويڪٽر &Opts, size_t MaxArgLen) override {
    std::ويڪٽر<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::ويڪٽر<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.سائيز_2() > 0 && "No option categories registered!");
    array_pod_ترتيب_ڏيو(SortedCategories.begin(), SortedCategories.پڇاڙي(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    لكل (ثابت خودڪار &I : Opts) {
      Option *Opt = I.ٻيو_2;
      لكل (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ڌڪو_پٺ(Opt);
      }
    }

    // Now do printing.
    لكل (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت خودڪار &CategoryOptions = CategorizedOptions[Category];
      إذا (CategoryOptions.خالي_جايون())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      إذا (!Category->getDescription().خالي_جايون())
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
  واضح HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  فراغ عامل=(منطقي قدر);
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
#جيڪڏهن_پورو

نطاق {
صنف نسخو_Printer {
عام:
  فراغ print(ثابت std::ويڪٽر<نسخو_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#جيڪڏهن_وصف_ٿيل PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وإلا
    OS << "LLVM (http://llvm.org/):\n  ";
#جيڪڏهن_پورو
    OS << PACKAGE_NAME << " version " << PACKAGE_نسخو << "\n  ";
#إذا LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وإلا
    OS << "Optimized build";
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
    OS << " with assertions";
#جيڪڏهن_پورو
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    إذا (!ExtraPrinters.خالي_جايون()) {
      لكل (ثابت خودڪار &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ عامل=(منطقي OptionWasSpecified);
};

بنية Command_سٽ_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_عام_Printer{خطأ};
  HelpPrinter UncategorizedHiddenPrinter{سچ};
  CategorizedHelpPrinter Categorized_عام_Printer{خطأ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{سچ};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_عام_Printer{Uncategorized_عام_Printer,
                                          Categorized_عام_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, سچ, parser<منطقي>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_عام_Printer),
      cl::Hidden,
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, سچ, parser<منطقي>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, سچ, parser<منطقي>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_عام_Printer),
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, سچ, parser<منطقي>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::قدر_Disallowed,
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

  نسخو_Printer_Ty Override_نسخو_Printer = خالي_اشارو;

  std::ويڪٽر<نسخو_Printer_Ty> Extra_نسخو_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  نسخو_Printer نسخو_Printer_Instance;

  cl::opt<نسخو_Printer, سچ, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(نسخو_Printer_Instance), cl::قدر_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساكن ManagedStatic<Command_سٽ_Common_Options> CommonOptions;

ساكن فراغ initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_گراف_Writer_Options();
  init_سگنل_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_ٻج_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ساكن OptionCategory GeneralCategory{"General options"};
  أرجع GeneralCategory;
}

فراغ نسخو_Printer::عامل=(منطقي OptionWasSpecified) {
  إذا (!OptionWasSpecified)
    أرجع;

  إذا (CommonOptions->Override_نسخو_Printer != خالي_اشارو) {
    CommonOptions->Override_نسخو_Printer(outs());
    ٻاهر_نڪرو(0);
  }
  print(CommonOptions->Extra_نسخو_Printers);

  ٻاهر_نڪرو(0);
}

فراغ HelpPrinterWrapper::عامل=(منطقي قدر) {
  إذا (!قدر)
    أرجع;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  إذا (globalParser().RegisteredOptionCategories.سائيز_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.سيٽ_Hidden_Flag(NotHidden);

    CategorizedPrinter = سچ; // Invoke categorized printer
  } وإلا {
    UncategorizedPrinter = سچ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::Print_Option_قدرون() { globalParser().print_Option_قدرون(); }

فراغ Command_سٽ_Parser::print_Option_قدرون() {
  إذا (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    أرجع;

  Small_ويڪٽر<std::pair<ثابت حرف *, Option *>, 128> Opts;
  ترتيب_ڏيو_Opts(سرگرم_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ سچ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  لكل (ثابت خودڪار &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.ٻيو_2->get_Option_ويڪر());

  لكل (ثابت خودڪار &Opt : Opts)
    Opt.ٻيو_2->print_Option_قدر(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::Print_Help_پيغام(منطقي Hidden, منطقي Categorized) {
  إذا (!Hidden && !Categorized)
    CommonOptions->Uncategorized_عام_Printer.printHelp();
  وإلا إذا (!Hidden && Categorized)
    CommonOptions->Categorized_عام_Printer.printHelp();
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
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
      "+assertions",
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل EXPENSIVE_CHECKS
      "+expensive-checks",
#جيڪڏهن_پورو
#إذا __has_feature(پتو_sanitizer)
      "+asan",
#جيڪڏهن_پورو
#إذا __has_feature(dataflow_sanitizer)
      "+dfsan",
#جيڪڏهن_پورو
#إذا __has_feature(hwaddress_sanitizer)
      "+hwasan",
#جيڪڏهن_پورو
#إذا __has_feature(يادگيري_sanitizer)
      "+msan",
#جيڪڏهن_پورو
#إذا __has_feature(thread_sanitizer)
      "+tsan",
#جيڪڏهن_پورو
#إذا __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#جيڪڏهن_پورو
  };
  أرجع ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#إذا LLVM_نسخو_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#جيڪڏهن_پورو
}

/// Utility function for printing version number.
فراغ cl::Print_نسخو_پيغام() {
  CommonOptions->نسخو_Printer_Instance.print(CommonOptions->Extra_نسخو_Printers);
}

فراغ cl::سيٽ_نسخو_Printer(نسخو_Printer_Ty func) {
  CommonOptions->Override_نسخو_Printer = func;
}

فراغ cl::جوڙيو_Extra_نسخو_Printer(نسخو_Printer_Ty func) {
  CommonOptions->Extra_نسخو_Printers.ڌڪو_پٺ(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  خودڪار &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.شامل_آهي(&Sub));
  أرجع Sub.OptionsMap;
}

iterator_range<Small_Ptr_سيٽ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  أرجع globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  لكل (خودڪار &I : Sub.OptionsMap) {
    منطقي Unrelated = سچ;
    لكل (خودڪار &Cat : I.ٻيو_2->Categories) {
      إذا (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.ٻيو_2->سيٽ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  لكل (خودڪار &I : Sub.OptionsMap) {
    منطقي Unrelated = سچ;
    لكل (خودڪار &Cat : I.ٻيو_2->Categories) {
      إذا (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.ٻيو_2->سيٽ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::Reset_Command_سٽ_Parser() { globalParser().reset(); }

فراغ cl::جوڙيو_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ڌڪو_پٺ(&L);
}

فراغ LibraryOptionsContext::سونپيو(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  قدرون.clear();
  Saved_دليلون.clear();
  ArrayRef<ثابت حرف *> دليلون = Other.Saved_دليلون;
  لكل (size_t I = 0, E = دليلون.سائيز_2(); I != E;) {
    StringRef Name = دليلون[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.ورهايو('=').پهريون_2);
    بنانشان N;
    cantFail(parse(*L, دليلون.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ثابت حرف *> دليلون,
                                   بنانشان &Consumed) {
  خودڪار [It, Inserted] = قدرون.try_emplace(L.getDefault());
  إذا (Inserted)
    It->ٻيو_2 = L.copyDefault(Alloc);
  أرجع parseLibraryOption(L, It->ٻيو_2, دليلون, Consumed, Alloc, &Saved_دليلون);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParse_Command_سٽ_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_سٽ_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
