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
ورتوں نطاق llvm;
ورتوں نطاق cl;

#تعریف_کرو DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نطاق llvm {
نطاق cl {
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<لما>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<لما لما>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشانی>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشانی لما>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<بنانشانی لما لما>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
سانچا صنف LLVM_EXPORT_TEMPLATE basic_parser<Element_گنتی>;

#إذا !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
سانچا صنف LLVM_EXPORT_TEMPLATE opt<std::string>;
#جے_انت

سانچا صنف LLVM_EXPORT_TEMPLATE opt<منطقي>;
سانچا صنف LLVM_EXPORT_TEMPLATE opt<حرف>;
سانچا صنف LLVM_EXPORT_TEMPLATE opt<صحيح>;
سانچا صنف LLVM_EXPORT_TEMPLATE opt<بنانشانی>;

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
فراغ parser<لما>::anchor() {}
فراغ parser<لما لما>::anchor() {}
فراغ parser<بنانشانی>::anchor() {}
فراغ parser<بنانشانی لما>::anchor() {}
فراغ parser<بنانشانی لما لما>::anchor() {}
فراغ parser<مزدوج>::anchor() {}
فراغ parser<عائم>::anchor() {}
فراغ parser<std::string>::anchor() {}
فراغ parser<std::optional<std::string>>::anchor() {}
فراغ parser<حرف>::anchor() {}
فراغ parser<Element_گنتی>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
فراغ opt_bool_anchor() { opt<منطقي> anchor{""}; }
فراغ opt_char_anchor() { opt<حرف> anchor{""}; }
فراغ opt_int_anchor() { opt<صحيح> anchor{""}; }
فراغ opt_unsigned_anchor() { opt<بنانشانی> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساكن size_t DefaultPad = 2;

ساكن StringRef Arg_اگیتر = "-";
ساكن StringRef Arg_اگیتر_Long = "--";
ساكن StringRef Arg_Help_اگیتر = " - ";

ساكن size_t arg_Plus_Prefixes_ناپ(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.ناپ_2();
  إذا (Len == 1)
    أرجع Len + Pad + Arg_اگیتر.ناپ_2() + Arg_Help_اگیتر.ناپ_2();
  أرجع Len + Pad + Arg_اگیتر_Long.ناپ_2() + Arg_Help_اگیتر.ناپ_2();
}

ساكن SmallString<8> arg_اگیتر(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> اگیتر;
  لكل (size_t I = 0; I < Pad; ++I) {
    اگیتر.ڈالو_پٹھ(' ');
  }
  اگیتر.append(ArgName.ناپ_2() > 1 ? Arg_اگیتر_Long : Arg_اگیتر);
  أرجع اگیتر;
}

ورتوں OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

صنف PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  یار raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << arg_اگیتر(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  أرجع OS;
}

صنف Command_سطر_Parser {
عام:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string پروگرام_Name;
  StringRef پروگرام_Overview;

  // This collects additional help to be printed.
  std::سمتیہ<StringRef> ہور_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_سیٹ<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_سیٹ<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_سمتیہ<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_اشاریہ;
  size_t عدد_اشاریہ_دار_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_سطر_Parser() { رجسٹر_Sub_Command(&SubCommand::get_چوٹی_Level()); }

  فراغ ResetAllOptionOccurrences();

  منطقي Parse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = خالی_سوچک,
                               vfs::FileSystem *VFS = خالی_سوچک,
                               LibraryOptionsContext *Context = خالی_سوچک);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    إذا (Opt.Subs.خالی_تھاواں()) {
      Action(SubCommand::get_چوٹی_Level());
      أرجع;
    }
    إذا (Opt.Subs.ناپ_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      لكل (خودکار *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      أرجع;
    }
    لكل (خودکار *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  فراغ جوڑو_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    إذا (Opt.hasArgStr())
      أرجع;
    إذا (!SC->OptionsMap.داخل_کرو(std::make_pair(Name, &Opt)).دوجا_2) {
      errs() << پروگرام_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  فراغ جوڑو_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { جوڑو_Literal_Option(Opt, &SC, Name); });
  }

  فراغ جوڑو_Option(Option *O, SubCommand *SC) {
    منطقي HadErrors = خطأ;
    إذا (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      إذا (!SC->OptionsMap.داخل_کرو(std::make_pair(O->ArgStr, O)).دوجا_2 ||
          (RegisteredSubCommands.شامل_اے(SC) &&
           Library_اشاریہ.شامل_اے(O->ArgStr))) {
        errs() << پروگرام_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = سچ;
      }
    }

    // Remember information about positional options.
    إذا (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ڈالو_پٹھ(O);
    وإلا إذا (O->get_عدد_Occurrences_Flag() == cl::ConsumeAfter) {
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

  فراغ جوڑو_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { جوڑو_Option(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    Small_سمتیہ<StringRef, 16> Option_ناں;
    O->get_Extra_Option_ناں(Option_ناں);
    إذا (O->hasArgStr())
      Option_ناں.ڈالو_پٹھ(O->ArgStr);

    SubCommand &Sub = *SC;
    لكل (خودکار Name : Option_ناں) {
      خودکار I = Sub.OptionsMap.لبھو(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      إذا (I != Sub.OptionsMap.سرا() && I->دوجا_2 == O)
        Sub.OptionsMap.erase(I);
    }

    إذا (O->getFormattingFlag() == cl::Positional)
      لكل (خودکار *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.سرا(); ++Opt) {
        إذا (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    وإلا إذا (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = خالی_سوچک;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    أرجع (!Sub.OptionsMap.خالی_تھاواں() || !Sub.PositionalOpts.خالی_تھاواں() ||
            خالی_سوچک != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    لكل (ثابت خودکار *S : RegisteredSubCommands) {
      إذا (hasOptions(*S))
        أرجع سچ;
    }
    أرجع !Libraries.خالی_تھاواں();
  }

  منطقي hasNamedSubCommands() ثابت {
    لكل (ثابت خودکار *S : RegisteredSubCommands)
      إذا (!S->getName().خالی_تھاواں())
        أرجع سچ;
    أرجع خطأ;
  }

  SubCommand *get_سرگرم_Sub_Command() { أرجع سرگرم_Sub_Command; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    إذا (!Sub.OptionsMap.داخل_کرو(std::make_pair(NewName, O)).دوجا_2) {
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

  فراغ print_Option_اقدار();

  فراغ اشاریہ_Library_Options() {
    منطقي HadErrors = خطأ;
    لكل (; عدد_اشاریہ_دار_Libraries != Libraries.ناپ_2(); ++عدد_اشاریہ_دار_Libraries) {
      LibraryOptions *L = Libraries[عدد_اشاریہ_دار_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        خودکار [It, Inserted] = Library_اشاریہ.try_emplace(Name, L);
        إذا (Inserted ? none_دا(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 أرجع SC->OptionsMap.شامل_اے(Name);
                               })
                     : It->دوجا_2 == L)
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
    اشاریہ_Library_Options();
    أرجع Library_اشاریہ.lookup(Name);
  }

  فراغ رجسٹر_Category(OptionCategory *cat) {
    أكد(گنتی_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             أرجع cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.داخل_کرو(cat);
  }

  فراغ رجسٹر_Sub_Command(SubCommand *sub) {
    أكد(گنتی_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      أرجع (!sub->getName().خالی_تھاواں()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.داخل_کرو(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    لكل (خودکار &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.دوجا_2;
      إذا (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        جوڑو_Option(O, sub);
      وإلا
        جوڑو_Literal_Option(*O, sub, E.پہلا_2);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_سیٹ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    أرجع make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.سرا());
  }

  فراغ reset() {
    سرگرم_Sub_Command = خالی_سوچک;
    پروگرام_Name.clear();
    پروگرام_Overview = StringRef();

    ہور_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_اشاریہ.clear();
    عدد_اشاریہ_دار_Libraries = 0;

    SubCommand::get_چوٹی_Level().reset();
    SubCommand::getAll().reset();
    رجسٹر_Sub_Command(&SubCommand::get_چوٹی_Level());
  }

خاص:
  SubCommand *سرگرم_Sub_Command = خالی_سوچک;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &قدر);
  SubCommand *LookupSubCommand(StringRef Name, std::string &سبھ_توں_نیڑے_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساكن Command_سطر_Parser &globalParser() {
  ساكن ManagedStatic<Command_سطر_Parser> GlobalParser;
  أرجع *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ساكن Error
parseLibraryOption(LibraryOptions &L, فراغ *Opts, ArrayRef<ثابت حرف *> دلیلاں,
                   بنانشانی &Consumed, BumpPtrAllocator &Alloc,
                   Small_سمتیہ_Impl<ثابت حرف *> *ریکارڈ = خالی_سوچک) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  بنانشانی عدد_دلیلاں = std::min<size_t>(دلیلاں.ناپ_2(), 2);
  لكل (بنانشانی J = 0; J != عدد_دلیلاں; ++J)
    Saved[J] = Saver.save(دلیلاں[J]).انکڑے();
  Consumed = 1;
  إذا (Error E = L.parse(Opts, ArrayRef(Saved, عدد_دلیلاں), Consumed, Alloc))
    أرجع E;
  إذا (ریکارڈ)
    ریکارڈ->append(Saved, Saved + Consumed);
  أرجع Error::success();
}

سانچا <قسم_ناں T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &قدر) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  إذا (!Arg.انکڑے() || Arg == "true" || Arg == "1") {
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

فراغ cl::جوڑو_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().جوڑو_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().ہور_Help.ڈالو_پٹھ(Help);
}

Option::Option(عدد_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : عدد_Occurrences(0), Occurrences(OccurrencesFlag), قدر(0),
      HiddenFlag(Hidden), Formatting(عام_Formatting), Misc(0),
      FullyInitialized(خطأ), Position(0) {
  Categories.ڈالو_پٹھ(&getGeneralCategory());
}

فراغ Option::جوڑو_دلیل() {
  globalParser().جوڑو_Option(هذا);
  FullyInitialized = سچ;
}

فراغ Option::remove_دلیل() { globalParser().removeOption(هذا); }

فراغ Option::سیٹ_Arg_Str(StringRef S) {
  إذا (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::جوڑو_Category(OptionCategory &C) {
  أكد(!Categories.خالی_تھاواں() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  إذا (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وإلا إذا (!is_contained(Categories, &C))
    Categories.ڈالو_پٹھ(&C);
}

فراغ Option::reset() {
  عدد_Occurrences = 0;
  سیٹ_Default();
}

فراغ OptionCategory::رجسٹر_Category() {
  globalParser().رجسٹر_Category(هذا);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_چوٹی_Level() {
  ساكن ManagedStatic<SubCommand> چوٹی_Level_Sub_Command;
  أرجع *چوٹی_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  أرجع *AllSubCommands;
}

فراغ SubCommand::رجسٹر_Sub_Command() {
  globalParser().رجسٹر_Sub_Command(هذا);
}

فراغ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(هذا);
}

فراغ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = خالی_سوچک;
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
Option *Command_سطر_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &قدر) {
  // Reject all dashes.
  إذا (Arg.خالی_تھاواں())
    أرجع خالی_سوچک;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.لبھو('=');

  // If we have an equals sign, remember the value.
  إذا (EqualPos == StringRef::npos) {
    // Look up the option.
    أرجع Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  خودکار I = Sub.OptionsMap.لبھو(Arg.substr(0, EqualPos));
  إذا (I == Sub.OptionsMap.سرا())
    أرجع خالی_سوچک;

  خودکار *O = I->دوجا_2;
  إذا (O->getFormattingFlag() == cl::Always_اگیتر)
    أرجع خالی_سوچک;

  قدر = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  أرجع I->دوجا_2;
}

SubCommand *Command_سطر_Parser::LookupSubCommand(StringRef Name,
                                                std::string &سبھ_توں_نیڑے_String) {
  إذا (Name.خالی_تھاواں())
    أرجع &SubCommand::get_چوٹی_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *سبھ_توں_نیڑے_میل = خالی_سوچک;
  لكل (خودکار *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    إذا (S->getName().خالی_تھاواں())
      استمر;

    إذا (S->getName() == Name)
      أرجع S;

    إذا (!سبھ_توں_نیڑے_میل && S->getName().edit_فاصلہ(Name) < 2)
      سبھ_توں_نیڑے_میل = S;
  }

  إذا (سبھ_توں_نیڑے_میل)
    سبھ_توں_نیڑے_String = سبھ_توں_نیڑے_میل->getName();

  أرجع &SubCommand::get_چوٹی_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *Lookup_سبھ_توں_نیڑے_Option(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &سبھ_توں_نیڑے_String) {
  // Reject all dashes.
  إذا (Arg.خالی_تھاواں())
    أرجع خالی_سوچک;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> ونڈو_Arg = Arg.ونڈو('=');
  StringRef &LHS = ونڈو_Arg.پہلا_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = ونڈو_Arg.دوجا_2;

  // Find the closest match.
  Option *بہترین = خالی_سوچک;
  بنانشانی بہترین_فاصلہ = 0;
  لكل (ثابت خودکار &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    إذا (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    Small_سمتیہ<StringRef, 16> Option_ناں;
    O->get_Extra_Option_ناں(Option_ناں);
    إذا (O->hasArgStr())
      Option_ناں.ڈالو_پٹھ(O->ArgStr);

    منطقي Permit_قدر = O->get_قدر_Expected_Flag() != cl::قدر_Disallowed;
    StringRef Flag = Permit_قدر ? LHS : Arg;
    لكل (ثابت خودکار &Name : Option_ناں) {
      بنانشانی فاصلہ_2 = StringRef(Name).edit_فاصلہ(
          Flag, /*AllowReplacements=*/سچ, /*MaxEditDistance=*/بہترین_فاصلہ);
      إذا (!بہترین || فاصلہ_2 < بہترین_فاصلہ) {
        بہترین = O;
        بہترین_فاصلہ = فاصلہ_2;
        إذا (RHS.خالی_تھاواں() || !Permit_قدر)
          سبھ_توں_نیڑے_String = std::string(Name);
        وإلا
          سبھ_توں_نیڑے_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  أرجع بہترین;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي Comma_Separate_And_جوڑو_Occurrence(Option *Handler, بنانشانی pos,
                                          StringRef ArgName, StringRef قدر) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  إذا (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(قدر);
    StringRef::ناپ_type Pos = Val.لبھو(',');

    طالما (Pos != StringRef::npos) {
      // Process the portion before the comma.
      إذا (Handler->جوڑو_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        أرجع سچ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.لبھو(',');
    }

    قدر = Val;
  }

  أرجع Handler->جوڑو_Occurrence(pos, ArgName, قدر);
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
    إذا (!قدر.انکڑے()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      إذا (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_اگیتر)
        أرجع Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      قدر = StringRef(argv[++i]);
    }
    اكسر;
  حالة قدر_Disallowed:
    إذا (قدر.انکڑے())
      أرجع Handler->error("does not allow a value! '" + Twine(قدر) +
                            "' specified.");
    اكسر;
  حالة قدر_Optional:
    اكسر;
  }

  أرجع Comma_Separate_And_جوڑو_Occurrence(Handler, i, ArgName, قدر);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  أرجع ProvideOption(Handler, Handler->ArgStr, Arg, 0, خالی_سوچک, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *لبھو_اگیتر_Option(StringRef Name, size_t &لمائی,
                                ثابت OptionsMapTy &OptionsMap) {
  لكل (; !Name.خالی_تھاواں(); Name = Name.drop_پٹھ()) {
    Option *O = OptionsMap.lookup(Name);
    إذا (O && (O->getFormattingFlag() == cl::اگیتر ||
              O->getFormattingFlag() == cl::Always_اگیتر)) {
      لمائی = Name.ناپ_2();
      أرجع O;
    }
  }
  أرجع خالی_سوچک;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &قدر,
                                    ثابت OptionsMapTy &OptionsMap) {
  إذا (Arg.ناپ_2() == 1)
    أرجع خالی_سوچک;

  size_t لمائی = 0;
  Option *POpt = لبھو_اگیتر_Option(Arg, لمائی, OptionsMap);
  إذا (!POpt)
    أرجع خالی_سوچک;

  StringRef Maybe_قدر =
      (لمائی < Arg.ناپ_2()) ? Arg.substr(لمائی) : StringRef();
  Arg = Arg.substr(0, لمائی);

  // cl::Prefix options do not preserve '=' when used separately.
  إذا (POpt->getFormattingFlag() == cl::اگیتر && Maybe_قدر.starts_with("="))
    Maybe_قدر = Maybe_قدر.drop_front();
  قدر = Maybe_قدر;
  أرجع POpt;
}

ساكن منطقي Requires_قدر(ثابت Option *O) {
  أرجع O->get_عدد_Occurrences_Flag() == cl::Required ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_ہور;
}

ساكن منطقي Eats_Unbounded_نمبر_دا_اقدار(ثابت Option *O) {
  أرجع O->get_عدد_Occurrences_Flag() == cl::Zero_Or_ہور ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_ہور;
}

ساكن منطقي isWhitespace(حرف C) {
  أرجع C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  أرجع isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { أرجع C == '\"' || C == '\''; }

فراغ cl::Tokenize_GNUCommand_سطر(StringRef Src, StringSaver &Saver,
                                Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي وچ_Token = خطأ;
  لكل (size_t I = 0, E = Src.ناپ_2(); I != E; ++I) {
    // Consume runs of whitespace.
    إذا (!وچ_Token) {
      طالما (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        إذا (MarkEOLs && Src[I] == '\n')
          NewArgv.ڈالو_پٹھ(خالی_سوچک);
        ++I;
      }
      إذا (I == E)
        اكسر;
      وچ_Token = سچ;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    إذا (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ڈالو_پٹھ(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    إذا (isQuote(C)) {
      ++I;
      طالما (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        إذا (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ڈالو_پٹھ(Src[I]);
        ++I;
      }
      إذا (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    إذا (isWhitespace(C)) {
      NewArgv.ڈالو_پٹھ(Saver.save(Token.str()).انکڑے());
      // Mark the end of lines in response files.
      إذا (MarkEOLs && C == '\n')
        NewArgv.ڈالو_پٹھ(خالی_سوچک);
      Token.clear();
      وچ_Token = خطأ;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.ڈالو_پٹھ(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  إذا (وچ_Token)
    NewArgv.ڈالو_پٹھ(Saver.save(Token.str()).انکڑے());
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
  size_t E = Src.ناپ_2();
  صحيح Backslash_گنتی = 0;
  // Skip the backslashes.
  افعل {
    ++I;
    ++Backslash_گنتی;
  } طالما (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  إذا (FollowedByDoubleQuote) {
    Token.append(Backslash_گنتی / 2, '\\');
    إذا (Backslash_گنتی % 2 == 0)
      أرجع I - 1;
    Token.ڈالو_پٹھ('"');
    أرجع I;
  }
  Token.append(Backslash_گنتی, '\\');
  أرجع I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي is_Windows_Special_Char_وچ_Command_Name(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenize_Windows_Command_سطر_Impl(
    StringRef Src, StringSaver &Saver, function_ref<فراغ(StringRef)> جوڑو_Token,
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

  لكل (size_t I = 0, E = Src.ناپ_2(); I < E; ++I) {
    بدل (State) {
    حالة INIT: {
      أكد(Token.خالی_تھاواں() && "token should be empty in initial state");
      // Eat whitespace before a token.
      طالما (I < E && isWhitespaceOrNull(Src[I])) {
        إذا (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      إذا (I >= E)
        اكسر;
      size_t شروع = I;
      إذا (CommandName) {
        طالما (I < E && !is_Windows_Special_Char_وچ_Command_Name(Src[I]))
          ++I;
      } وإلا {
        طالما (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef عام_Chars = Src.ٹکڑا(شروع, I);
      إذا (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        جوڑو_Token(AlwaysCopy ? Saver.save(عام_Chars) : عام_Chars);
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
        جوڑو_Token(Saver.save(Token.str()));
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
        Token.ڈالو_پٹھ(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      إذا (Src[I] == '\"') {
        إذا (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ڈالو_پٹھ('"');
          ++I;
        } وإلا {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.ڈالو_پٹھ(Src[I]);
      }
      اكسر;
    }
  }

  إذا (State != INIT)
    جوڑو_Token(Saver.save(Token.str()));
}

فراغ cl::Tokenize_Windows_Command_سطر(StringRef Src, StringSaver &Saver,
                                    Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_پٹھ(Tok.انکڑے()); };
  خودکار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ڈالو_پٹھ(خالی_سوچک);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سطر_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_سمتیہ_Impl<StringRef> &NewArgv) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_پٹھ(Tok); };
  خودکار OnEOL = []() {};
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token, /*AlwaysCopy=*/خطأ,
                                 OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سطر_بھریا(StringRef Src, StringSaver &Saver,
                                        Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_پٹھ(Tok.انکڑے()); };
  خودکار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.ڈالو_پٹھ(خالی_سوچک);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, سچ);
}

فراغ cl::tokenizeConfigFile(StringRef سروت, StringSaver &Saver,
                            Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  لكل (ثابت حرف *Cur = سروت.begin(); Cur != سروت.سرا();) {
    SmallString<128> سطر;
    // Check for comment line.
    إذا (isWhitespace(*Cur)) {
      طالما (Cur != سروت.سرا() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    إذا (*Cur == '#') {
      طالما (Cur != سروت.سرا() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *شروع = Cur;
    ثابت حرف *سطر_سرا = خالی_سوچک;
    حرف Quote = 0;
    منطقي At_Token_شروع = سچ;
    لكل (ثابت حرف *سرا_2 = سروت.سرا(); Cur != سرا_2; ++Cur) {
      حرف C = *Cur;
      إذا (C == '\\') {
        إذا (Cur + 1 != سرا_2) {
          ++Cur;
          إذا (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != سرا_2) && Cur[1] == '\n')) {
            سطر.append(شروع, Cur - 1);
            إذا (*Cur == '\r')
              ++Cur;
            شروع = Cur + 1;
            // Splicing does not introduce a token boundary.
            استمر;
          }
        }
        At_Token_شروع = خطأ;
        استمر;
      }
      إذا (C == '\n')
        اكسر;
      إذا (Quote) {
        إذا (C == Quote)
          Quote = 0;
      } وإلا إذا (isQuote(C)) {
        Quote = C;
      } وإلا إذا (C == '#' && At_Token_شروع) {
        سطر_سرا = Cur;
        طالما (Cur != سرا_2 && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      At_Token_شروع = isWhitespace(C);
    }
    // Tokenize line.
    سطر.append(شروع, سطر_سرا ? سطر_سرا : Cur);
    cl::Tokenize_GNUCommand_سطر(سطر, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي has_UTF8_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.ناپ_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ Expand_ادھار_Paths(StringRef ادھار_رستہ, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::رستہ_2::is_absolute(ادھار_رستہ));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::ناپ_type شروع_Pos = 0;
  لكل (StringRef::ناپ_type TokenPos = ArgString.لبھو(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.لبھو(Token, شروع_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(شروع_Pos, TokenPos - شروع_Pos);
    إذا (ResponseFile.خالی_تھاواں())
      ResponseFile = LHS;
    وإلا
      llvm::sys::رستہ_2::append(ResponseFile, LHS);
    ResponseFile.append(ادھار_رستہ);
    شروع_Pos = TokenPos + Token.ناپ_2();
  }

  إذا (!ResponseFile.خالی_تھاواں()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef بچیا = ArgString.substr(شروع_Pos);
    إذا (!بچیا.خالی_تھاواں())
      llvm::sys::رستہ_2::append(ResponseFile, بچیا);
    Arg = Saver.save(ResponseFile.str()).انکڑے();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_سمتیہ_Impl<ثابت حرف *> &NewArgv) {
  أكد(sys::رستہ_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<حافظہ_بفر>> MemBufOrErr =
      FS->get_بفر_For_File(FName);
  إذا (!MemBufOrErr) {
    std::error_کوڈ EC = MemBufOrErr.getError();
    أرجع llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.سنیہا());
  }
  حافظہ_بفر &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_بفر_شروع(), MemBuf.get_بفر_ناپ());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.get_بفر_شروع(), MemBuf.get_بفر_سرا());
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
    Str = StringRef(BufRef.انکڑے() + 3, BufRef.ناپ_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  إذا (!Relative_ناں && !وچ_Config_File)
    أرجع Error::success();

  StringRef ادھار_رستہ = llvm::sys::رستہ_2::ماپا_رستہ(FName);
  لكل (ثابت حرف *&Arg : NewArgv) {
    إذا (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    إذا (وچ_Config_File)
      Expand_ادھار_Paths(ادھار_رستہ, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = خطأ;
    إذا (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      إذا (!llvm::sys::رستہ_2::is_relative(FileName))
        استمر;
    } وإلا إذا (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = سچ;
    } وإلا {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ڈالو_پٹھ('@');
    إذا (ConfigInclusion && !llvm::sys::رستہ_2::has_ماپا_رستہ(FileName)) {
      SmallString<128> File_رستہ;
      إذا (!لبھو_Config_File(FileName, File_رستہ))
        أرجع createStringError(
            std::make_error_کوڈ(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_رستہ);
    } وإلا {
      ResponseFile.append(ادھار_رستہ);
      llvm::sys::رستہ_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).انکڑے();
  }
  أرجع Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
  بنية Response_File_ریکارڈ {
    std::string File;
    size_t سرا_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_سمتیہ<Response_File_ریکارڈ, 3> File_ڈھیر;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ڈھیر.ڈالو_پٹھ({"", Argv.ناپ_2()});

  // Don't cache Argv.size() because it can change.
  لكل (بنانشانی I = 0; I != Argv.ناپ_2();) {
    طالما (I == File_ڈھیر.پٹھ().سرا_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ڈھیر.نکالو_پٹھ();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    إذا (Arg == خالی_سوچک) {
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
    إذا (llvm::sys::رستہ_2::is_relative(FName)) {
      إذا (CurrentDir.خالی_تھاواں()) {
        إذا (خودکار CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وإلا {
          أرجع createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وإلا {
        CurrDir = CurrentDir;
      }
      llvm::sys::رستہ_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::حالت> Res = FS->حالت_2(FName);
    إذا (!Res || !Res->exists()) {
      std::error_کوڈ EC = Res.getError();
      إذا (!وچ_Config_File) {
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
                                       "': " + EC.سنیہا());
    }
    ثابت llvm::vfs::حالت &File_حالت = Res.get();

    خودکار IsEquivalent =
        [File_حالت, هذا](ثابت Response_File_ریکارڈ &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::حالت> RHS = FS->حالت_2(RFile.File);
      إذا (!RHS)
        أرجع RHS.getError();
      أرجع File_حالت.equivalent(*RHS);
    };

    // Check for recursive response files.
    لكل (ثابت خودکار &F : drop_begin(File_ڈھیر)) {
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
    Small_سمتیہ<ثابت حرف *, 0> ExpandedArgv;
    إذا (Error Err = expandResponseFile(FName, ExpandedArgv))
      أرجع Err;

    لكل (Response_File_ریکارڈ &ریکارڈ : File_ڈھیر) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      ریکارڈ.سرا_2 += ExpandedArgv.ناپ_2() - 1;
    }

    File_ڈھیر.ڈالو_پٹھ({FName, I + ExpandedArgv.ناپ_2()});
    Argv.erase(Argv.begin() + I);
    Argv.داخل_کرو(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.سرا());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_ڈھیر.ناپ_2() > 0 && Argv.ناپ_2() == File_ڈھیر.پٹھ().سرا_2);
  أرجع Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             Small_سمتیہ_Impl<ثابت حرف *> &NewArgv) {
#جے_تعریف_شدہ _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#وإلا
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#جے_انت
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
                             Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
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

منطقي ExpansionContext::لبھو_Config_File(StringRef FileName,
                                      Small_سمتیہ_Impl<حرف> &File_رستہ) {
  SmallString<128> Cfg_File_رستہ;
  ثابت خودکار FileExists = [هذا](SmallString<128> رستہ) -> منطقي {
    خودکار حالت = FS->حالت_2(رستہ);
    أرجع حالت &&
           حالت->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  إذا (llvm::sys::رستہ_2::has_ماپا_رستہ(FileName)) {
    Cfg_File_رستہ = FileName;
    إذا (llvm::sys::رستہ_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_رستہ))
      أرجع خطأ;
    إذا (!FileExists(Cfg_File_رستہ))
      أرجع خطأ;
    File_رستہ.سونپو(Cfg_File_رستہ.begin(), Cfg_File_رستہ.سرا());
    أرجع سچ;
  }

  // Look for the file in search directories.
  لكل (ثابت StringRef &Dir : تلاش_Dirs) {
    إذا (Dir.خالی_تھاواں())
      استمر;
    Cfg_File_رستہ.سونپو(Dir);
    llvm::sys::رستہ_2::append(Cfg_File_رستہ, FileName);
    llvm::sys::رستہ_2::native(Cfg_File_رستہ);
    إذا (FileExists(Cfg_File_رستہ)) {
      File_رستہ.سونپو(Cfg_File_رستہ.begin(), Cfg_File_رستہ.سرا());
      أرجع سچ;
    }
  }

  أرجع خطأ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
  SmallString<128> Abs_رستہ;
  إذا (sys::رستہ_2::is_relative(CfgFile)) {
    Abs_رستہ.سونپو(CfgFile);
    إذا (std::error_کوڈ EC = FS->makeAbsolute(Abs_رستہ))
      أرجع make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_رستہ.str();
  }
  وچ_Config_File = سچ;
  Relative_ناں = سچ;
  إذا (Error Err = expandResponseFile(CfgFile, Argv))
    أرجع Err;
  أرجع expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parse_Command_سطر_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> دلیلاں,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  Small_سمتیہ<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ڈالو_پٹھ(دلیلاں[0]);

  // Parse options from environment variable.
  إذا (EnvVar) {
    إذا (std::optional<std::string> Env_قدر =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_سطر(*Env_قدر, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, دلیلاں.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.ناپ_2());

  // Parse all options.
  أرجع globalParser().Parse_Command_سطر_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::Parse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سطر_Options_Impl(خالی_سوچک, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parse_Command_سطر_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> دلیلاں,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سطر_Options_Impl(&Context, دلیلاں, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
فراغ Command_سطر_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  لكل (خودکار *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_سمتیہ<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.ناپ_2());
    لكل (خودکار &O : SC->OptionsMap)
      Opts.ڈالو_پٹھ(O.دوجا_2);
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

منطقي Command_سطر_Parser::Parse_Command_سطر_Options(
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
  Small_سمتیہ<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#جے_تعریف_شدہ _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#وإلا
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#جے_انت
  ExpansionContext ECtx(A, Tokenize, VFS);
  إذا (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << منزل_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.ناپ_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  پروگرام_Name = std::string(sys::رستہ_2::filename(StringRef(argv[0])));
  اشاریہ_Library_Options();

  // Check out the positional arguments to collect information about them.
  بنانشانی عدد_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = خطأ;

  صحيح پہلا_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_چوٹی_Level();
  std::string سبھ_توں_نیڑے_Sub_Command_String;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[پہلا_Arg][0] != '-' && hasNamedSubCommands();
  إذا (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[پہلا_Arg]), سبھ_توں_نیڑے_Sub_Command_String);
    إذا (ChosenSubCommand != &SubCommand::get_چوٹی_Level())
      پہلا_Arg = 2;
  }
  globalParser().سرگرم_Sub_Command = ChosenSubCommand;

  أكد(ChosenSubCommand);
  خودکار &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  خودکار &PositionalOpts = ChosenSubCommand->PositionalOpts;
  خودکار &OptionsMap = ChosenSubCommand->OptionsMap;

  إذا (ConsumeAfterOpt) {
    أكد(PositionalOpts.ناپ_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  إذا (!PositionalOpts.خالی_تھاواں()) {

    // Calculate how many positional values are _required_.
    منطقي Unbounded_لبھیا = خطأ;
    لكل (size_t i = 0, e = PositionalOpts.ناپ_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      إذا (Requires_قدر(Opt))
        ++عدد_Positional_Required;
      وإلا إذا (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        إذا (PositionalOpts.ناپ_2() > 1) {
          إذا (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = سچ;
        }
      } وإلا إذا (Unbounded_لبھیا && !Opt->hasArgStr()) {
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
        *Errs << PositionalOpts.ناپ_2();
        ErrorParsing = سچ;
      }
      Unbounded_لبھیا |= Eats_Unbounded_نمبر_دا_اقدار(Opt);
    }
    HasUnlimitedPositionals = Unbounded_لبھیا || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_سمتیہ<std::pair<StringRef, بنانشانی>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *سرگرم_Positional_Arg = خالی_سوچک;

  // Loop over all of the arguments... processing them.
  منطقي Dash_Dash_لبھیا = خطأ; // Have we read '--'?
  لكل (صحيح i = پہلا_Arg; i < argc; ++i) {
    Option *Handler = خالی_سوچک;
    std::string سبھ_توں_نیڑے_Handler_String;
    StringRef قدر;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    إذا (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_لبھیا) {
      // Positional argument!
      إذا (سرگرم_Positional_Arg) {
        ProvidePositionalOption(سرگرم_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      إذا (!PositionalOpts.خالی_تھاواں()) {
        PositionalVals.ڈالو_پٹھ(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        إذا (PositionalVals.ناپ_2() >= عدد_Positional_Required && ConsumeAfterOpt) {
          لكل (++i; i < argc; ++i)
            PositionalVals.ڈالو_پٹھ(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } وإلا إذا (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_لبھیا) {
      Dash_Dash_لبھیا = سچ; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } وإلا إذا (سرگرم_Positional_Arg &&
               (سرگرم_Positional_Arg->getMiscFlags() & Positional_Eats_دلیلاں)) {
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
      إذا (!Handler && ChosenSubCommand != &SubCommand::get_چوٹی_Level())
        Handler = LookupOption(SubCommand::get_چوٹی_Level(), ArgName, قدر);

      إذا (!Handler) {
        إذا (LibraryOptions *L = lookupLibraryOption(ArgName.ونڈو('=').پہلا_2)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          بنانشانی N;
          إذا (Error E = Context && L->is_فی_Context()
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
        Lookup_سبھ_توں_نیڑے_Option(ArgName, OptionsMap, سبھ_توں_نیڑے_Handler_String);
    }

    إذا (!Handler) {
      خودکار Report_Unknown_دلیل = [&](منطقي IsArg,
                                       StringRef سبھ_توں_نیڑے_دلیل_Name) {
        *Errs << پروگرام_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        إذا (سبھ_توں_نیڑے_دلیل_Name.خالی_تھاواں())
          أرجع;

        *Errs << پروگرام_Name << ": Did you mean '";
        إذا (IsArg)
          *Errs << PrintArg(سبھ_توں_نیڑے_دلیل_Name, 0);
        وإلا
          *Errs << سبھ_توں_نیڑے_دلیل_Name;
        *Errs << "'?\n";
      };

      إذا (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_دلیل(/*IsArg=*/سچ, سبھ_توں_نیڑے_Handler_String);
      وإلا
        Report_Unknown_دلیل(/*IsArg=*/خطأ, سبھ_توں_نیڑے_Sub_Command_String);

      ErrorParsing = سچ;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    إذا (Handler->getFormattingFlag() == cl::Positional) {
      إذا ((Handler->getMiscFlags() & Positional_Eats_دلیلاں) && !قدر.خالی_تھاواں()) {
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
  إذا (عدد_Positional_Required > PositionalVals.ناپ_2()) {
      *Errs << پروگرام_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << عدد_Positional_Required
             << " positional argument" << (عدد_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = سچ;
  } وإلا إذا (!HasUnlimitedPositionals &&
             PositionalVals.ناپ_2() > PositionalOpts.ناپ_2()) {
    *Errs << پروگرام_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.ناپ_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = سچ;

  } وإلا إذا (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    بنانشانی ValNo = 0, عدد_Vals = static_cast<بنانشانی>(PositionalVals.ناپ_2());
    لكل (Option *Opt : PositionalOpts) {
      إذا (Requires_قدر(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].پہلا_2,
                                PositionalVals[ValNo].دوجا_2);
        ValNo++;
        --عدد_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي پورا = Opt->get_عدد_Occurrences_Flag() == cl::Required;
      طالما (عدد_Vals - ValNo > عدد_Positional_Required && !پورا) {
        بدل (Opt->get_عدد_Occurrences_Flag()) {
        حالة cl::Optional:
          پورا = سچ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالة cl::Zero_Or_ہور: // Zero or more will take all they can get...
        حالة cl::One_Or_ہور:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].پہلا_2,
                                  PositionalVals[ValNo].دوجا_2);
          ValNo++;
          اكسر;
        افتراضي:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وإلا {
    أكد(ConsumeAfterOpt && عدد_Positional_Required <= PositionalVals.ناپ_2());
    بنانشانی ValNo = 0;
    لكل (Option *Opt : PositionalOpts)
      إذا (Requires_قدر(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].پہلا_2, PositionalVals[ValNo].دوجا_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    إذا (PositionalOpts.ناپ_2() == 1 && ValNo == 0 && !PositionalVals.خالی_تھاواں()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].پہلا_2,
                                              PositionalVals[ValNo].دوجا_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    لكل (; ValNo != PositionalVals.ناپ_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].پہلا_2,
                                  PositionalVals[ValNo].دوجا_2);
  }

  // Loop over args and make sure all required args are specified!
  لكل (ثابت خودکار &Opt : OptionsMap) {
    بدل (Opt.دوجا_2->get_عدد_Occurrences_Flag()) {
    حالة Required:
    حالة One_Or_ہور:
      إذا (Opt.دوجا_2->get_عدد_Occurrences() == 0) {
        Opt.دوجا_2->error("must be specified at least once!");
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
  ہور_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  إذا (ErrorParsing) {
    إذا (!IgnoreErrors)
      باہر_نکلو(1);
    أرجع خطأ;
  }
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &سنیہا_2, StringRef ArgName, raw_ostream &Errs) {
  إذا (!ArgName.انکڑے())
    ArgName = ArgStr;
  إذا (ArgName.خالی_تھاواں())
    Errs << HelpStr; // Be nice for positional arguments
  وإلا
    Errs << globalParser().پروگرام_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << سنیہا_2 << "\n";
  أرجع سچ;
}

منطقي Option::جوڑو_Occurrence(بنانشانی pos, StringRef ArgName, StringRef قدر) {
  ++عدد_Occurrences;
  أرجع handleOccurrence(pos, ArgName, قدر);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef get_قدر_Str(ثابت Option &O, StringRef Default_پیغام) {
  إذا (O.قدر_Str.خالی_تھاواں())
    أرجع Default_پیغام;
  أرجع O.قدر_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_چوڑائی() ثابت {
  أرجع arg_Plus_Prefixes_ناپ(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t پہلا_سطر_Indented_By) {
  أكد(Indent >= پہلا_سطر_Indented_By);
  std::pair<StringRef, StringRef> ونڈو_2 = HelpStr.ونڈو('\n');
  outs().indent(Indent - پہلا_سطر_Indented_By)
      << Arg_Help_اگیتر << ونڈو_2.پہلا_2 << "\n";
  طالما (!ونڈو_2.دوجا_2.خالی_تھاواں()) {
    ونڈو_2 = ونڈو_2.دوجا_2.ونڈو('\n');
    outs().indent(Indent) << ونڈو_2.پہلا_2 << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t ادھار_Indent,
                                 size_t پہلا_سطر_Indented_By) {
  ثابت StringRef Val_Help_اگیتر = "  ";
  أكد(ادھار_Indent >= پہلا_سطر_Indented_By);
  std::pair<StringRef, StringRef> ونڈو_2 = HelpStr.ونڈو('\n');
  outs().indent(ادھار_Indent - پہلا_سطر_Indented_By)
      << Arg_Help_اگیتر << Val_Help_اگیتر << ونڈو_2.پہلا_2 << "\n";
  طالما (!ونڈو_2.دوجا_2.خالی_تھاواں()) {
    ونڈو_2 = ونڈو_2.دوجا_2.ونڈو('\n');
    outs().indent(ادھار_Indent + Val_Help_اگیتر.ناپ_2()) << ونڈو_2.پہلا_2 << "\n";
  }
}

// Print out the option for the alias.
فراغ alias::printOptionInfo(size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_چوڑائی, arg_Plus_Prefixes_ناپ(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_چوڑائی(ثابت Option &O) ثابت {
  size_t Len = arg_Plus_Prefixes_ناپ(O.ArgStr);
  خودکار ValName = get_قدر_Name();
  إذا (!ValName.خالی_تھاواں()) {
    size_t FormattingLen = 3;
    إذا (O.getMiscFlags() & Positional_Eats_دلیلاں)
      FormattingLen = 6;
    Len += get_قدر_Str(O, ValName).ناپ_2() + FormattingLen;
  }

  أرجع Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(O.ArgStr);

  خودکار ValName = get_قدر_Name();
  إذا (!ValName.خالی_تھاواں()) {
    إذا (O.getMiscFlags() & Positional_Eats_دلیلاں) {
      outs() << " <" << get_قدر_Str(O, ValName) << ">...";
    } وإلا إذا (O.get_قدر_Expected_Flag() == قدر_Optional)
      outs() << "[=<" << get_قدر_Str(O, ValName) << ">]";
    وإلا {
      outs() << (O.ArgStr.ناپ_2() == 1 ? " <" : "=<") << get_قدر_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_چوڑائی, get_Option_چوڑائی(O));
}

فراغ basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_چوڑائی - O.ArgStr.ناپ_2());
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
سانچا <قسم_ناں FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef قدر_قسم,
                                         FixedOrScalableQuantityT &قدر) {
  ورتوں ScalarTy = قسم_ناں FixedOrScalableQuantityT::ScalarTy;

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
منطقي parser<لما>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         لما &قدر) {
  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for long argument!");
  أرجع خطأ;
}

// parser<long long> implementation
//
منطقي parser<لما لما>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              لما لما &قدر) {
  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for llong argument!");
  أرجع خطأ;
}

// parser<unsigned> implementation
//
منطقي parser<بنانشانی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             بنانشانی &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for uint argument!");
  أرجع خطأ;
}

// parser<unsigned long> implementation
//
منطقي parser<بنانشانی لما>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  بنانشانی لما &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for ulong argument!");
  أرجع خطأ;
}

// parser<unsigned long long> implementation
//
منطقي parser<بنانشانی لما لما>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       بنانشانی لما لما &قدر) {

  إذا (Arg.getAsInteger(0, قدر))
    أرجع O.error("'" + Arg + "' value invalid for ullong argument!");
  أرجع خطأ;
}

// parser<ElementCount> implementation
//
منطقي parser<Element_گنتی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_گنتی &قدر) {
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
بنانشانی generic_parser_ادھار::لبھو_Option(StringRef Name) {
  بنانشانی e = get_عدد_Options();

  لكل (بنانشانی i = 0; i != e; ++i) {
    إذا (getOption(i) == Name)
      أرجع i;
  }
  أرجع e;
}

ساكن StringRef Eq_قدر = "=<value>";
ساكن StringRef خالی_تھاواں_Option = "<empty>";
ساكن StringRef Option_اگیتر = "    =";
ساكن size_t get_Option_Prefixes_ناپ() {
  أرجع Option_اگیتر.ناپ_2() + Arg_Help_اگیتر.ناپ_2();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  أرجع O.get_قدر_Expected_Flag() != قدر_Optional || !Name.خالی_تھاواں() ||
         !Description.خالی_تھاواں();
}

// Return the width of the option tag for printing...
size_t generic_parser_ادھار::get_Option_چوڑائی(ثابت Option &O) ثابت {
  إذا (O.hasArgStr()) {
    size_t ناپ =
        arg_Plus_Prefixes_ناپ(O.ArgStr) + Eq_قدر.ناپ_2();
    لكل (بنانشانی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      إذا (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t Name_ناپ = Name.خالی_تھاواں() ? خالی_تھاواں_Option.ناپ_2() : Name.ناپ_2();
      ناپ = std::max(ناپ, Name_ناپ + get_Option_Prefixes_ناپ());
    }
    أرجع ناپ;
  } وإلا {
    size_t ادھار_ناپ = 0;
    لكل (بنانشانی i = 0, e = get_عدد_Options(); i != e; ++i)
      ادھار_ناپ = std::max(ادھار_ناپ, getOption(i).ناپ_2() + 8);
    أرجع ادھار_ناپ;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_ادھار::printOptionInfo(ثابت Option &O,
                                          size_t Global_چوڑائی) ثابت {
  إذا (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    إذا (O.get_قدر_Expected_Flag() == قدر_Optional) {
      لكل (بنانشانی i = 0, e = get_عدد_Options(); i != e; ++i) {
        إذا (getOption(i).خالی_تھاواں()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_چوڑائی,
                               arg_Plus_Prefixes_ناپ(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_قدر;
    Option::printHelpStr(O.HelpStr, Global_چوڑائی,
                         Eq_قدر.ناپ_2() +
                             arg_Plus_Prefixes_ناپ(O.ArgStr));
    لكل (بنانشانی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      إذا (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t پہلا_سطر_Indent = OptionName.ناپ_2() + get_Option_Prefixes_ناپ();
      outs() << Option_اگیتر << OptionName;
      إذا (OptionName.خالی_تھاواں()) {
        outs() << خالی_تھاواں_Option;
        أكد(پہلا_سطر_Indent >= خالی_تھاواں_Option.ناپ_2());
        پہلا_سطر_Indent += خالی_تھاواں_Option.ناپ_2();
      }
      إذا (!Description.خالی_تھاواں())
        Option::printEnumValHelpStr(Description, Global_چوڑائی, پہلا_سطر_Indent);
      وإلا
        outs() << '\n';
    }
  } وإلا {
    إذا (!O.HelpStr.خالی_تھاواں())
      outs() << "  " << O.HelpStr << '\n';
    لكل (بنانشانی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_چوڑائی, Option.ناپ_2() + 8);
    }
  }
}

ساكن ثابت size_t Max_Opt_چوڑائی = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
فراغ generic_parser_ادھار::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_قدر &قدر,
    ثابت Generic_Option_قدر &Default, size_t Global_چوڑائی) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_چوڑائی - O.ArgStr.ناپ_2());

  بنانشانی عدد_Opts = get_عدد_Options();
  لكل (بنانشانی i = 0; i != عدد_Opts; ++i) {
    إذا (!قدر.compare(get_Option_قدر(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).ناپ_2();
    size_t عدد_Spaces = Max_Opt_چوڑائی > L ? Max_Opt_چوڑائی - L : 0;
    outs().indent(عدد_Spaces) << " (default: ";
    لكل (بنانشانی j = 0; j != عدد_Opts; ++j) {
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

#تعریف_کرو PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, Option_قدر<T> D,      \
                                  size_t Global_چوڑائی) ثابت {                  \
    printOptionName(O, Global_چوڑائی);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t عدد_Spaces =                                                         \
        Max_Opt_چوڑائی > Str.ناپ_2() ? Max_Opt_چوڑائی - Str.ناپ_2() : 0;               \
    outs().indent(عدد_Spaces) << " (default: ";                                 \
    إذا (D.has_قدر())                                                          \
      outs() << D.get_قدر();                                                  \
    وإلا                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقي)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحيح)
PRINT_OPT_DIFF(لما)
PRINT_OPT_DIFF(لما لما)
PRINT_OPT_DIFF(بنانشانی)
PRINT_OPT_DIFF(بنانشانی لما)
PRINT_OPT_DIFF(بنانشانی لما لما)
PRINT_OPT_DIFF(مزدوج)
PRINT_OPT_DIFF(عائم)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_گنتی)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_قدر<std::string> &D,
                                          size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= " << V;
  size_t عدد_Spaces = Max_Opt_چوڑائی > V.ناپ_2() ? Max_Opt_چوڑائی - V.ناپ_2() : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  إذا (D.has_قدر())
    outs() << D.get_قدر();
  وإلا
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_قدر<std::optional<std::string>> &D,
    size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= " << V;
  size_t VSize = V.has_قدر_2() ? V.قدر_2().ناپ_2() : 0;
  size_t عدد_Spaces = Max_Opt_چوڑائی > VSize ? Max_Opt_چوڑائی - VSize : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  إذا (D.has_قدر() && D.get_قدر().has_قدر_2())
    outs() << D.get_قدر();
  وإلا
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::print_Option_No_قدر(ثابت Option &O,
                                           size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  أرجع strcmp(LHS->پہلا_2, RHS->پہلا_2);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  أرجع strcmp(LHS->پہلا_2, RHS->پہلا_2);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ ترتیب_دیو_Opts(OptionsMapTy &OptMap,
                     Small_سمتیہ_Impl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  Small_Ptr_سیٹ<Option *, 32> Option_سیٹ; // Duplicate option detection.

  لكل (خودکار I = OptMap.begin(), E = OptMap.سرا(); I != E; ++I) {
    // Ignore really-hidden options.
    إذا (I->دوجا_2->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    إذا (I->دوجا_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    إذا (!Option_سیٹ.داخل_کرو(I->دوجا_2).دوجا_2)
      استمر;

    Opts.ڈالو_پٹھ(
        std::pair<ثابت حرف *, Option *>(I->پہلا_2.انکڑے(), I->دوجا_2));
  }

  // Sort the options list alphabetically.
  array_pod_ترتیب_دیو(Opts.begin(), Opts.سرا(), OptNameCompare);
}

ساكن فراغ
ترتیب_دیو_Sub_Commands(ثابت Small_Ptr_سیٹ_Impl<SubCommand *> &SubMap,
                Small_سمتیہ_Impl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  لكل (خودکار *S : SubMap) {
    إذا (S->getName().خالی_تھاواں())
      استمر;
    Subs.ڈالو_پٹھ(std::make_pair(S->getName().انکڑے(), S));
  }
  array_pod_ترتیب_دیو(Subs.begin(), Subs.سرا(), SubNameCompare);
}

نطاق {

صنف HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  ورتوں Str_Option_Pair_سمتیہ =
      Small_سمتیہ<std::pair<ثابت حرف *, Option *>, 128>;
  ورتوں Str_Sub_Command_Pair_سمتیہ =
      Small_سمتیہ<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازی فراغ printOptions(Str_Option_Pair_سمتیہ &Opts, size_t MaxArgLen) {
    لكل (ثابت خودکار &Opt : Opts)
      Opt.دوجا_2->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(Str_Sub_Command_Pair_سمتیہ &Subs, size_t MaxSubLen) {
    لكل (ثابت خودکار &S : Subs) {
      outs() << "  " << S.پہلا_2;
      إذا (!S.دوجا_2->getDescription().خالی_تھاواں()) {
        outs().indent(MaxSubLen - لڑی_لمبائی(S.پہلا_2));
        outs() << " - " << S.دوجا_2->getDescription();
      }
      outs() << "\n";
    }
  }

عام:
  واضح HelpPrinter(منطقي showHidden) : ShowHidden(showHidden) {}
  مجازی ~HelpPrinter() = افتراضي;

  // Invoke the printer.
  فراغ عامل=(منطقي قدر) {
    إذا (!قدر)
      أرجع;
    printHelp();

    // Halt the program since help information was printed
    باہر_نکلو(0);
  }

  فراغ printHelp() {
    SubCommand *Sub = globalParser().get_سرگرم_Sub_Command();
    خودکار &OptionsMap = Sub->OptionsMap;
    خودکار &PositionalOpts = Sub->PositionalOpts;
    خودکار &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_سمتیہ Opts;
    ترتیب_دیو_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_سمتیہ Subs;
    ترتیب_دیو_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    إذا (!globalParser().پروگرام_Overview.خالی_تھاواں())
      outs() << "OVERVIEW: " << globalParser().پروگرام_Overview << "\n";

    إذا (Sub == &SubCommand::get_چوٹی_Level()) {
      outs() << "USAGE: " << globalParser().پروگرام_Name;
      إذا (!Subs.خالی_تھاواں())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وإلا {
      إذا (!Sub->getDescription().خالی_تھاواں()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().پروگرام_Name << " " << Sub->getName()
             << " [options]";
    }

    لكل (خودکار *Opt : PositionalOpts) {
      إذا (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    إذا (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    إذا (Sub == &SubCommand::get_چوٹی_Level() && !Subs.خالی_تھاواں()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      لكل (ثابت خودکار &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, لڑی_لمبائی(Sub.پہلا_2));

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
    لكل (ثابت خودکار &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.دوجا_2->get_Option_چوڑائی());

    Small_سمتیہ<std::pair<std::string, StringRef>, 0> LibraryOpts;
    إذا (ShowHidden)
      لكل (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              إذا (!Help.خالی_تھاواں())
                LibraryOpts.emplace_پٹھ((Spelling + MetaVar).str(), Help);
            });
    llvm::ترتیب_دیو(LibraryOpts);
    لكل (ثابت خودکار &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_ناپ(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    إذا (!LibraryOpts.خالی_تھاواں())
      outs() << "\nLibrary options:\n\n";
    لكل (ثابت خودکار &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_ناپ(Name));
    }

    // Print any extra help the user has declared.
    لكل (ثابت خودکار &I : globalParser().ہور_Help)
      outs() << I;
    globalParser().ہور_Help.clear();
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
  ورتوں HelpPrinter::عامل=;

محمي:
  فراغ printOptions(Str_Option_Pair_سمتیہ &Opts, size_t MaxArgLen) override {
    std::سمتیہ<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::سمتیہ<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.ناپ_2() > 0 && "No option categories registered!");
    array_pod_ترتیب_دیو(SortedCategories.begin(), SortedCategories.سرا(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    لكل (ثابت خودکار &I : Opts) {
      Option *Opt = I.دوجا_2;
      لكل (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ڈالو_پٹھ(Opt);
      }
    }

    // Now do printing.
    لكل (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت خودکار &CategoryOptions = CategorizedOptions[Category];
      إذا (CategoryOptions.خالی_تھاواں())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      إذا (!Category->getDescription().خالی_تھاواں())
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
#جے_انت

نطاق {
صنف نسخہ_Printer {
عام:
  فراغ print(ثابت std::سمتیہ<نسخہ_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#جے_تعریف_شدہ PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وإلا
    OS << "LLVM (http://llvm.org/):\n  ";
#جے_انت
    OS << PACKAGE_NAME << " version " << PACKAGE_نسخہ << "\n  ";
#إذا LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وإلا
    OS << "Optimized build";
#جے_انت
#جے_بنا_تعریف NDEBUG
    OS << " with assertions";
#جے_انت
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    إذا (!ExtraPrinters.خالی_تھاواں()) {
      لكل (ثابت خودکار &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ عامل=(منطقي OptionWasSpecified);
};

بنية Command_سطر_Common_Options {
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

  نسخہ_Printer_Ty Override_نسخہ_Printer = خالی_سوچک;

  std::سمتیہ<نسخہ_Printer_Ty> Extra_نسخہ_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  نسخہ_Printer نسخہ_Printer_Instance;

  cl::opt<نسخہ_Printer, سچ, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(نسخہ_Printer_Instance), cl::قدر_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساكن ManagedStatic<Command_سطر_Common_Options> CommonOptions;

ساكن فراغ initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_گراف_Writer_Options();
  init_اشارے_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_بیج_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ساكن OptionCategory GeneralCategory{"General options"};
  أرجع GeneralCategory;
}

فراغ نسخہ_Printer::عامل=(منطقي OptionWasSpecified) {
  إذا (!OptionWasSpecified)
    أرجع;

  إذا (CommonOptions->Override_نسخہ_Printer != خالی_سوچک) {
    CommonOptions->Override_نسخہ_Printer(outs());
    باہر_نکلو(0);
  }
  print(CommonOptions->Extra_نسخہ_Printers);

  باہر_نکلو(0);
}

فراغ HelpPrinterWrapper::عامل=(منطقي قدر) {
  إذا (!قدر)
    أرجع;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  إذا (globalParser().RegisteredOptionCategories.ناپ_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.سیٹ_Hidden_Flag(NotHidden);

    CategorizedPrinter = سچ; // Invoke categorized printer
  } وإلا {
    UncategorizedPrinter = سچ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::Print_Option_اقدار() { globalParser().print_Option_اقدار(); }

فراغ Command_سطر_Parser::print_Option_اقدار() {
  إذا (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    أرجع;

  Small_سمتیہ<std::pair<ثابت حرف *, Option *>, 128> Opts;
  ترتیب_دیو_Opts(سرگرم_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ سچ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  لكل (ثابت خودکار &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.دوجا_2->get_Option_چوڑائی());

  لكل (ثابت خودکار &Opt : Opts)
    Opt.دوجا_2->print_Option_قدر(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::Print_Help_سنیہا(منطقي Hidden, منطقي Categorized) {
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
#جے_انت
#جے_بنا_تعریف NDEBUG
      "+assertions",
#جے_انت
#جے_تعریف_شدہ EXPENSIVE_CHECKS
      "+expensive-checks",
#جے_انت
#إذا __has_feature(پتہ_sanitizer)
      "+asan",
#جے_انت
#إذا __has_feature(dataflow_sanitizer)
      "+dfsan",
#جے_انت
#إذا __has_feature(hwaddress_sanitizer)
      "+hwasan",
#جے_انت
#إذا __has_feature(حافظہ_sanitizer)
      "+msan",
#جے_انت
#إذا __has_feature(thread_sanitizer)
      "+tsan",
#جے_انت
#إذا __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#جے_انت
#جے_تعریف_شدہ LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#جے_انت
  };
  أرجع ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#إذا LLVM_نسخہ_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#جے_انت
}

/// Utility function for printing version number.
فراغ cl::Print_نسخہ_سنیہا() {
  CommonOptions->نسخہ_Printer_Instance.print(CommonOptions->Extra_نسخہ_Printers);
}

فراغ cl::سیٹ_نسخہ_Printer(نسخہ_Printer_Ty func) {
  CommonOptions->Override_نسخہ_Printer = func;
}

فراغ cl::جوڑو_Extra_نسخہ_Printer(نسخہ_Printer_Ty func) {
  CommonOptions->Extra_نسخہ_Printers.ڈالو_پٹھ(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  خودکار &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.شامل_اے(&Sub));
  أرجع Sub.OptionsMap;
}

iterator_range<Small_Ptr_سیٹ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  أرجع globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  لكل (خودکار &I : Sub.OptionsMap) {
    منطقي Unrelated = سچ;
    لكل (خودکار &Cat : I.دوجا_2->Categories) {
      إذا (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دوجا_2->سیٹ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  لكل (خودکار &I : Sub.OptionsMap) {
    منطقي Unrelated = سچ;
    لكل (خودکار &Cat : I.دوجا_2->Categories) {
      إذا (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دوجا_2->سیٹ_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::Reset_Command_سطر_Parser() { globalParser().reset(); }

فراغ cl::جوڑو_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ڈالو_پٹھ(&L);
}

فراغ LibraryOptionsContext::سونپو(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  اقدار.clear();
  Saved_دلیلاں.clear();
  ArrayRef<ثابت حرف *> دلیلاں = Other.Saved_دلیلاں;
  لكل (size_t I = 0, E = دلیلاں.ناپ_2(); I != E;) {
    StringRef Name = دلیلاں[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.ونڈو('=').پہلا_2);
    بنانشانی N;
    cantFail(parse(*L, دلیلاں.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ثابت حرف *> دلیلاں,
                                   بنانشانی &Consumed) {
  خودکار [It, Inserted] = اقدار.try_emplace(L.getDefault());
  إذا (Inserted)
    It->دوجا_2 = L.copyDefault(Alloc);
  أرجع parseLibraryOption(L, It->دوجا_2, دلیلاں, Consumed, Alloc, &Saved_دلیلاں);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_سطر_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
