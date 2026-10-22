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
استعمال نامخلا llvm;
استعمال نامخلا cl;

#تعریف_کرو DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نامخلا llvm {
نامخلا cl {
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<منطقی>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<صحیح>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<طویل>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<طویل طویل>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<غیرعلامتی>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<غیرعلامتی طویل>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<غیرعلامتی طویل طویل>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<دوہرا>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<اعشاری>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE basic_parser<Element_گنتی>;

#اگر !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
سانچہ جماعت LLVM_EXPORT_TEMPLATE opt<std::string>;
#اگر_ختم

سانچہ جماعت LLVM_EXPORT_TEMPLATE opt<منطقی>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE opt<حرف>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE opt<صحیح>;
سانچہ جماعت LLVM_EXPORT_TEMPLATE opt<غیرعلامتی>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
خالی Generic_Option_قدر::anchor() {}
خالی Option_قدر<boolOrDefault>::anchor() {}
خالی Option_قدر<std::string>::anchor() {}
خالی Option::anchor() {}
خالی basic_parser_impl::anchor() {}
خالی parser<منطقی>::anchor() {}
خالی parser<boolOrDefault>::anchor() {}
خالی parser<صحیح>::anchor() {}
خالی parser<طویل>::anchor() {}
خالی parser<طویل طویل>::anchor() {}
خالی parser<غیرعلامتی>::anchor() {}
خالی parser<غیرعلامتی طویل>::anchor() {}
خالی parser<غیرعلامتی طویل طویل>::anchor() {}
خالی parser<دوہرا>::anchor() {}
خالی parser<اعشاری>::anchor() {}
خالی parser<std::string>::anchor() {}
خالی parser<std::optional<std::string>>::anchor() {}
خالی parser<حرف>::anchor() {}
خالی parser<Element_گنتی>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
خالی opt_bool_anchor() { opt<منطقی> anchor{""}; }
خالی opt_char_anchor() { opt<حرف> anchor{""}; }
خالی opt_int_anchor() { opt<صحیح> anchor{""}; }
خالی opt_unsigned_anchor() { opt<غیرعلامتی> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساکن size_t DefaultPad = 2;

ساکن StringRef Arg_سابقہ = "-";
ساکن StringRef Arg_سابقہ_Long = "--";
ساکن StringRef Arg_Help_سابقہ = " - ";

ساکن size_t arg_Plus_Prefixes_سائز(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.سائز_2();
  اگر (Len == 1)
    واپس Len + Pad + Arg_سابقہ.سائز_2() + Arg_Help_سابقہ.سائز_2();
  واپس Len + Pad + Arg_سابقہ_Long.سائز_2() + Arg_Help_سابقہ.سائز_2();
}

ساکن SmallString<8> arg_سابقہ(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> سابقہ;
  برائے (size_t I = 0; I < Pad; ++I) {
    سابقہ.ڈالو_عقب(' ');
  }
  سابقہ.append(ArgName.سائز_2() > 1 ? Arg_سابقہ_Long : Arg_سابقہ);
  واپس سابقہ;
}

استعمال OptionsMapTy = DenseMap<StringRef, Option *>;

نامخلا {

جماعت PrintArg {
  StringRef ArgName;
  size_t Pad;
عوامی:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  دوست raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &عامل<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << arg_سابقہ(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  واپس OS;
}

جماعت Command_سطر_Parser {
عوامی:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string پروگرام_Name;
  StringRef پروگرام_Overview;

  // This collects additional help to be printed.
  std::سمتیہ<StringRef> مزید_Help;

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

  خالی ResetAllOptionOccurrences();

  منطقی Parse_Command_سطر_Options(صحیح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = خالی_اشاریہ,
                               vfs::FileSystem *VFS = خالی_اشاریہ,
                               LibraryOptionsContext *Context = خالی_اشاریہ);

  خالی forEachSubCommand(Option &Opt, function_ref<خالی(SubCommand &)> Action) {
    اگر (Opt.Subs.خالی_خانے()) {
      Action(SubCommand::get_چوٹی_Level());
      واپس;
    }
    اگر (Opt.Subs.سائز_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      برائے (خودکار *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      واپس;
    }
    برائے (خودکار *SC : Opt.Subs) {
      أكد(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  خالی جوڑو_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    اگر (Opt.hasArgStr())
      واپس;
    اگر (!SC->OptionsMap.داخل_کرو(std::make_pair(Name, &Opt)).دوسرا_2) {
      errs() << پروگرام_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  خالی جوڑو_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { جوڑو_Literal_Option(Opt, &SC, Name); });
  }

  خالی جوڑو_Option(Option *O, SubCommand *SC) {
    منطقی HadErrors = جھوٹ;
    اگر (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      اگر (!SC->OptionsMap.داخل_کرو(std::make_pair(O->ArgStr, O)).دوسرا_2 ||
          (RegisteredSubCommands.شامل_ہے(SC) &&
           Library_اشاریہ.شامل_ہے(O->ArgStr))) {
        errs() << پروگرام_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = سچ;
      }
    }

    // Remember information about positional options.
    اگر (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ڈالو_عقب(O);
    ورنہ اگر (O->get_عدد_Occurrences_Flag() == cl::ConsumeAfter) {
      اگر (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = سچ;
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

  خالی جوڑو_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { جوڑو_Option(O, &SC); });
  }

  خالی removeOption(Option *O, SubCommand *SC) {
    Small_سمتیہ<StringRef, 16> Option_نام;
    O->get_Extra_Option_نام(Option_نام);
    اگر (O->hasArgStr())
      Option_نام.ڈالو_عقب(O->ArgStr);

    SubCommand &Sub = *SC;
    برائے (خودکار Name : Option_نام) {
      خودکار I = Sub.OptionsMap.ڈھونڈو(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      اگر (I != Sub.OptionsMap.انتہا() && I->دوسرا_2 == O)
        Sub.OptionsMap.erase(I);
    }

    اگر (O->getFormattingFlag() == cl::Positional)
      برائے (خودکار *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.انتہا(); ++Opt) {
        اگر (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          توڑو;
        }
      }
    ورنہ اگر (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = خالی_اشاریہ;
  }

  خالی removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقی hasOptions(ثابت SubCommand &Sub) ثابت {
    واپس (!Sub.OptionsMap.خالی_خانے() || !Sub.PositionalOpts.خالی_خانے() ||
            خالی_اشاریہ != Sub.ConsumeAfterOpt);
  }

  منطقی hasOptions() ثابت {
    برائے (ثابت خودکار *S : RegisteredSubCommands) {
      اگر (hasOptions(*S))
        واپس سچ;
    }
    واپس !Libraries.خالی_خانے();
  }

  منطقی hasNamedSubCommands() ثابت {
    برائے (ثابت خودکار *S : RegisteredSubCommands)
      اگر (!S->getName().خالی_خانے())
        واپس سچ;
    واپس جھوٹ;
  }

  SubCommand *get_فعال_Sub_Command() { واپس فعال_Sub_Command; }

  خالی updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    اگر (!Sub.OptionsMap.داخل_کرو(std::make_pair(NewName, O)).دوسرا_2) {
      errs() << پروگرام_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  خالی updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  خالی print_Option_اقدار();

  خالی اشاریہ_Library_Options() {
    منطقی HadErrors = جھوٹ;
    برائے (; عدد_اشاریہ_دار_Libraries != Libraries.سائز_2(); ++عدد_اشاریہ_دار_Libraries) {
      LibraryOptions *L = Libraries[عدد_اشاریہ_دار_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        خودکار [It, Inserted] = Library_اشاریہ.try_emplace(Name, L);
        اگر (Inserted ? none_کا(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 واپس SC->OptionsMap.شامل_ہے(Name);
                               })
                     : It->دوسرا_2 == L)
          واپس;
        errs() << پروگرام_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = سچ;
      });
    }
    اگر (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    اشاریہ_Library_Options();
    واپس Library_اشاریہ.lookup(Name);
  }

  خالی رجسٹر_Category(OptionCategory *cat) {
    أكد(گنتی_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             واپس cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.داخل_کرو(cat);
  }

  خالی رجسٹر_Sub_Command(SubCommand *sub) {
    أكد(گنتی_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      واپس (!sub->getName().خالی_خانے()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.داخل_کرو(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    برائے (خودکار &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.دوسرا_2;
      اگر (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        جوڑو_Option(O, sub);
      ورنہ
        جوڑو_Literal_Option(*O, sub, E.پہلا_2);
    }
  }

  خالی unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_سیٹ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    واپس make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.انتہا());
  }

  خالی reset() {
    فعال_Sub_Command = خالی_اشاریہ;
    پروگرام_Name.clear();
    پروگرام_Overview = StringRef();

    مزید_Help.clear();
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

نجی:
  SubCommand *فعال_Sub_Command = خالی_اشاریہ;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &قدر);
  SubCommand *LookupSubCommand(StringRef Name, std::string &قریب_ترین_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ساکن Command_سطر_Parser &globalParser() {
  ساکن ManagedStatic<Command_سطر_Parser> GlobalParser;
  واپس *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ساکن Error
parseLibraryOption(LibraryOptions &L, خالی *Opts, ArrayRef<ثابت حرف *> دلائل,
                   غیرعلامتی &Consumed, BumpPtrAllocator &Alloc,
                   Small_سمتیہ_Impl<ثابت حرف *> *ریکارڈ = خالی_اشاریہ) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  غیرعلامتی عدد_دلائل = std::min<size_t>(دلائل.سائز_2(), 2);
  برائے (غیرعلامتی J = 0; J != عدد_دلائل; ++J)
    Saved[J] = Saver.save(دلائل[J]).ڈیٹا();
  Consumed = 1;
  اگر (Error E = L.parse(Opts, ArrayRef(Saved, عدد_دلائل), Consumed, Alloc))
    واپس E;
  اگر (ریکارڈ)
    ریکارڈ->append(Saved, Saved + Consumed);
  واپس Error::success();
}

سانچہ <قسم_نام T, T TrueVal, T FalseVal>
ساکن منطقی parseBool(Option &O, StringRef ArgName, StringRef Arg, T &قدر) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  اگر (!Arg.ڈیٹا() || Arg == "true" || Arg == "1") {
    قدر = TrueVal;
    واپس جھوٹ;
  }

  اگر (Arg == "false" || Arg == "0") {
    قدر = FalseVal;
    واپس جھوٹ;
  }
  واپس O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

خالی cl::جوڑو_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().جوڑو_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().مزید_Help.ڈالو_عقب(Help);
}

Option::Option(عدد_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : عدد_Occurrences(0), Occurrences(OccurrencesFlag), قدر(0),
      HiddenFlag(Hidden), Formatting(معمول_Formatting), Misc(0),
      FullyInitialized(جھوٹ), Position(0) {
  Categories.ڈالو_عقب(&getGeneralCategory());
}

خالی Option::جوڑو_دلیل() {
  globalParser().جوڑو_Option(یہ);
  FullyInitialized = سچ;
}

خالی Option::remove_دلیل() { globalParser().removeOption(یہ); }

خالی Option::سیٹ_Arg_Str(StringRef S) {
  اگر (FullyInitialized)
    globalParser().updateArgStr(یہ, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

خالی Option::جوڑو_Category(OptionCategory &C) {
  أكد(!Categories.خالی_خانے() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  اگر (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  ورنہ اگر (!is_contained(Categories, &C))
    Categories.ڈالو_عقب(&C);
}

خالی Option::reset() {
  عدد_Occurrences = 0;
  سیٹ_Default();
}

خالی OptionCategory::رجسٹر_Category() {
  globalParser().رجسٹر_Category(یہ);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_چوٹی_Level() {
  ساکن ManagedStatic<SubCommand> چوٹی_Level_Sub_Command;
  واپس *چوٹی_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساکن ManagedStatic<SubCommand> AllSubCommands;
  واپس *AllSubCommands;
}

خالی SubCommand::رجسٹر_Sub_Command() {
  globalParser().رجسٹر_Sub_Command(یہ);
}

خالی SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(یہ);
}

خالی SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = خالی_اشاریہ;
}

SubCommand::عامل منطقی() ثابت {
  واپس (globalParser().get_فعال_Sub_Command() == یہ);
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
  اگر (Arg.خالی_خانے())
    واپس خالی_اشاریہ;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.ڈھونڈو('=');

  // If we have an equals sign, remember the value.
  اگر (EqualPos == StringRef::npos) {
    // Look up the option.
    واپس Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  خودکار I = Sub.OptionsMap.ڈھونڈو(Arg.substr(0, EqualPos));
  اگر (I == Sub.OptionsMap.انتہا())
    واپس خالی_اشاریہ;

  خودکار *O = I->دوسرا_2;
  اگر (O->getFormattingFlag() == cl::Always_سابقہ)
    واپس خالی_اشاریہ;

  قدر = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  واپس I->دوسرا_2;
}

SubCommand *Command_سطر_Parser::LookupSubCommand(StringRef Name,
                                                std::string &قریب_ترین_String) {
  اگر (Name.خالی_خانے())
    واپس &SubCommand::get_چوٹی_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *قریب_ترین_مطابقت = خالی_اشاریہ;
  برائے (خودکار *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    اگر (S->getName().خالی_خانے())
      جاری;

    اگر (S->getName() == Name)
      واپس S;

    اگر (!قریب_ترین_مطابقت && S->getName().edit_فاصلہ(Name) < 2)
      قریب_ترین_مطابقت = S;
  }

  اگر (قریب_ترین_مطابقت)
    قریب_ترین_String = قریب_ترین_مطابقت->getName();

  واپس &SubCommand::get_چوٹی_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساکن Option *Lookup_قریب_ترین_Option(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &قریب_ترین_String) {
  // Reject all dashes.
  اگر (Arg.خالی_خانے())
    واپس خالی_اشاریہ;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> بانٹو_Arg = Arg.بانٹو('=');
  StringRef &LHS = بانٹو_Arg.پہلا_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = بانٹو_Arg.دوسرا_2;

  // Find the closest match.
  Option *بہترین = خالی_اشاریہ;
  غیرعلامتی بہترین_فاصلہ = 0;
  برائے (ثابت خودکار &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    اگر (O->getOptionHiddenFlag() == ReallyHidden)
      جاری;

    Small_سمتیہ<StringRef, 16> Option_نام;
    O->get_Extra_Option_نام(Option_نام);
    اگر (O->hasArgStr())
      Option_نام.ڈالو_عقب(O->ArgStr);

    منطقی Permit_قدر = O->get_قدر_Expected_Flag() != cl::قدر_Disallowed;
    StringRef Flag = Permit_قدر ? LHS : Arg;
    برائے (ثابت خودکار &Name : Option_نام) {
      غیرعلامتی فاصلہ_2 = StringRef(Name).edit_فاصلہ(
          Flag, /*AllowReplacements=*/سچ, /*MaxEditDistance=*/بہترین_فاصلہ);
      اگر (!بہترین || فاصلہ_2 < بہترین_فاصلہ) {
        بہترین = O;
        بہترین_فاصلہ = فاصلہ_2;
        اگر (RHS.خالی_خانے() || !Permit_قدر)
          قریب_ترین_String = std::string(Name);
        ورنہ
          قریب_ترین_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  واپس بہترین;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساکن منطقی Comma_Separate_And_جوڑو_Occurrence(Option *Handler, غیرعلامتی pos,
                                          StringRef ArgName, StringRef قدر) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  اگر (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(قدر);
    StringRef::سائز_type Pos = Val.ڈھونڈو(',');

    جب (Pos != StringRef::npos) {
      // Process the portion before the comma.
      اگر (Handler->جوڑو_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        واپس سچ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.ڈھونڈو(',');
    }

    قدر = Val;
  }

  واپس Handler->جوڑو_Occurrence(pos, ArgName, قدر);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساکن اندرونی منطقی ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef قدر, صحیح argc,
                                 ثابت حرف *ثابت *argv, صحیح &i) {
  // Enforce value requirements
  انتخاب (Handler->get_قدر_Expected_Flag()) {
  حالت قدر_Required:
    اگر (!قدر.ڈیٹا()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      اگر (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_سابقہ)
        واپس Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      قدر = StringRef(argv[++i]);
    }
    توڑو;
  حالت قدر_Disallowed:
    اگر (قدر.ڈیٹا())
      واپس Handler->error("does not allow a value! '" + Twine(قدر) +
                            "' specified.");
    توڑو;
  حالت قدر_Optional:
    توڑو;
  }

  واپس Comma_Separate_And_جوڑو_Occurrence(Handler, i, ArgName, قدر);
}

منطقی llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحیح i) {
  صحیح Dummy = i;
  واپس ProvideOption(Handler, Handler->ArgStr, Arg, 0, خالی_اشاریہ, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساکن Option *ڈھونڈو_سابقہ_Option(StringRef Name, size_t &لمبائی,
                                ثابت OptionsMapTy &OptionsMap) {
  برائے (; !Name.خالی_خانے(); Name = Name.drop_عقب()) {
    Option *O = OptionsMap.lookup(Name);
    اگر (O && (O->getFormattingFlag() == cl::سابقہ ||
              O->getFormattingFlag() == cl::Always_سابقہ)) {
      لمبائی = Name.سائز_2();
      واپس O;
    }
  }
  واپس خالی_اشاریہ;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساکن Option *HandlePrefixedOption(StringRef &Arg, StringRef &قدر,
                                    ثابت OptionsMapTy &OptionsMap) {
  اگر (Arg.سائز_2() == 1)
    واپس خالی_اشاریہ;

  size_t لمبائی = 0;
  Option *POpt = ڈھونڈو_سابقہ_Option(Arg, لمبائی, OptionsMap);
  اگر (!POpt)
    واپس خالی_اشاریہ;

  StringRef Maybe_قدر =
      (لمبائی < Arg.سائز_2()) ? Arg.substr(لمبائی) : StringRef();
  Arg = Arg.substr(0, لمبائی);

  // cl::Prefix options do not preserve '=' when used separately.
  اگر (POpt->getFormattingFlag() == cl::سابقہ && Maybe_قدر.starts_with("="))
    Maybe_قدر = Maybe_قدر.drop_front();
  قدر = Maybe_قدر;
  واپس POpt;
}

ساکن منطقی Requires_قدر(ثابت Option *O) {
  واپس O->get_عدد_Occurrences_Flag() == cl::Required ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_مزید;
}

ساکن منطقی Eats_Unbounded_نمبر_کا_اقدار(ثابت Option *O) {
  واپس O->get_عدد_Occurrences_Flag() == cl::Zero_Or_مزید ||
         O->get_عدد_Occurrences_Flag() == cl::One_Or_مزید;
}

ساکن منطقی isWhitespace(حرف C) {
  واپس C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساکن منطقی isWhitespaceOrNull(حرف C) {
  واپس isWhitespace(C) || C == '\0';
}

ساکن منطقی isQuote(حرف C) { واپس C == '\"' || C == '\''; }

خالی cl::Tokenize_GNUCommand_سطر(StringRef Src, StringSaver &Saver,
                                Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                منطقی MarkEOLs) {
  SmallString<128> Token;
  منطقی اندر_Token = جھوٹ;
  برائے (size_t I = 0, E = Src.سائز_2(); I != E; ++I) {
    // Consume runs of whitespace.
    اگر (!اندر_Token) {
      جب (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        اگر (MarkEOLs && Src[I] == '\n')
          NewArgv.ڈالو_عقب(خالی_اشاریہ);
        ++I;
      }
      اگر (I == E)
        توڑو;
      اندر_Token = سچ;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    اگر (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ڈالو_عقب(Src[I]);
      جاری;
    }

    // Consume a quoted string.
    اگر (isQuote(C)) {
      ++I;
      جب (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        اگر (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ڈالو_عقب(Src[I]);
        ++I;
      }
      اگر (I == E)
        توڑو;
      جاری;
    }

    // End the token if this is whitespace.
    اگر (isWhitespace(C)) {
      NewArgv.ڈالو_عقب(Saver.save(Token.str()).ڈیٹا());
      // Mark the end of lines in response files.
      اگر (MarkEOLs && C == '\n')
        NewArgv.ڈالو_عقب(خالی_اشاریہ);
      Token.clear();
      اندر_Token = جھوٹ;
      جاری;
    }

    // This is a normal character.  Append it.
    Token.ڈالو_عقب(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  اگر (اندر_Token)
    NewArgv.ڈالو_عقب(Saver.save(Token.str()).ڈیٹا());
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
ساکن size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.سائز_2();
  صحیح Backslash_گنتی = 0;
  // Skip the backslashes.
  کرو {
    ++I;
    ++Backslash_گنتی;
  } جب (I != E && Src[I] == '\\');

  منطقی FollowedByDoubleQuote = (I != E && Src[I] == '"');
  اگر (FollowedByDoubleQuote) {
    Token.append(Backslash_گنتی / 2, '\\');
    اگر (Backslash_گنتی % 2 == 0)
      واپس I - 1;
    Token.ڈالو_عقب('"');
    واپس I;
  }
  Token.append(Backslash_گنتی, '\\');
  واپس I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساکن منطقی isWindowsSpecialChar(حرف C) {
  واپس isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساکن منطقی is_Windows_Special_Char_اندر_Command_Name(حرف C) {
  واپس isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساکن اندرونی خالی tokenize_Windows_Command_سطر_Impl(
    StringRef Src, StringSaver &Saver, function_ref<خالی(StringRef)> جوڑو_Token,
    منطقی AlwaysCopy, function_ref<خالی()> MarkEOL, منطقی InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  منطقی CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  شمار { INIT, UNQUOTED, QUOTED } State = INIT;

  برائے (size_t I = 0, E = Src.سائز_2(); I < E; ++I) {
    انتخاب (State) {
    حالت INIT: {
      أكد(Token.خالی_خانے() && "token should be empty in initial state");
      // Eat whitespace before a token.
      جب (I < E && isWhitespaceOrNull(Src[I])) {
        اگر (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      اگر (I >= E)
        توڑو;
      size_t آغاز = I;
      اگر (CommandName) {
        جب (I < E && !is_Windows_Special_Char_اندر_Command_Name(Src[I]))
          ++I;
      } ورنہ {
        جب (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef معمول_Chars = Src.قاش(آغاز, I);
      اگر (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        جوڑو_Token(AlwaysCopy ? Saver.save(معمول_Chars) : معمول_Chars);
        اگر (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } ورنہ {
          CommandName = جھوٹ;
        }
      } ورنہ اگر (Src[I] == '\"') {
        Token += معمول_Chars;
        State = QUOTED;
      } ورنہ اگر (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += معمول_Chars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } ورنہ {
        llvm_unreachable("unexpected special character");
      }
      توڑو;
    }

    حالت UNQUOTED:
      اگر (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        جوڑو_Token(Saver.save(Token.str()));
        Token.clear();
        اگر (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } ورنہ {
          CommandName = جھوٹ;
        }
        State = INIT;
      } ورنہ اگر (Src[I] == '\"') {
        State = QUOTED;
      } ورنہ اگر (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ورنہ {
        Token.ڈالو_عقب(Src[I]);
      }
      توڑو;

    حالت QUOTED:
      اگر (Src[I] == '\"') {
        اگر (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ڈالو_عقب('"');
          ++I;
        } ورنہ {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } ورنہ اگر (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ورنہ {
        Token.ڈالو_عقب(Src[I]);
      }
      توڑو;
    }
  }

  اگر (State != INIT)
    جوڑو_Token(Saver.save(Token.str()));
}

خالی cl::Tokenize_Windows_Command_سطر(StringRef Src, StringSaver &Saver,
                                    Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                    منطقی MarkEOLs) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_عقب(Tok.ڈیٹا()); };
  خودکار OnEOL = [&]() {
    اگر (MarkEOLs)
      NewArgv.ڈالو_عقب(خالی_اشاریہ);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, جھوٹ);
}

خالی cl::Tokenize_Windows_Command_سطر_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_سمتیہ_Impl<StringRef> &NewArgv) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_عقب(Tok); };
  خودکار OnEOL = []() {};
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token, /*AlwaysCopy=*/جھوٹ,
                                 OnEOL, جھوٹ);
}

خالی cl::Tokenize_Windows_Command_سطر_بھرا(StringRef Src, StringSaver &Saver,
                                        Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                                        منطقی MarkEOLs) {
  خودکار جوڑو_Token = [&](StringRef Tok) { NewArgv.ڈالو_عقب(Tok.ڈیٹا()); };
  خودکار OnEOL = [&]() {
    اگر (MarkEOLs)
      NewArgv.ڈالو_عقب(خالی_اشاریہ);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جوڑو_Token,
                                 /*AlwaysCopy=*/سچ, OnEOL, سچ);
}

خالی cl::tokenizeConfigFile(StringRef ماخذ, StringSaver &Saver,
                            Small_سمتیہ_Impl<ثابت حرف *> &NewArgv,
                            منطقی MarkEOLs) {
  برائے (ثابت حرف *Cur = ماخذ.begin(); Cur != ماخذ.انتہا();) {
    SmallString<128> سطر;
    // Check for comment line.
    اگر (isWhitespace(*Cur)) {
      جب (Cur != ماخذ.انتہا() && isWhitespace(*Cur))
        ++Cur;
      جاری;
    }
    اگر (*Cur == '#') {
      جب (Cur != ماخذ.انتہا() && *Cur != '\n')
        ++Cur;
      جاری;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *آغاز = Cur;
    ثابت حرف *سطر_انتہا = خالی_اشاریہ;
    حرف Quote = 0;
    منطقی At_Token_آغاز = سچ;
    برائے (ثابت حرف *انتہا_2 = ماخذ.انتہا(); Cur != انتہا_2; ++Cur) {
      حرف C = *Cur;
      اگر (C == '\\') {
        اگر (Cur + 1 != انتہا_2) {
          ++Cur;
          اگر (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != انتہا_2) && Cur[1] == '\n')) {
            سطر.append(آغاز, Cur - 1);
            اگر (*Cur == '\r')
              ++Cur;
            آغاز = Cur + 1;
            // Splicing does not introduce a token boundary.
            جاری;
          }
        }
        At_Token_آغاز = جھوٹ;
        جاری;
      }
      اگر (C == '\n')
        توڑو;
      اگر (Quote) {
        اگر (C == Quote)
          Quote = 0;
      } ورنہ اگر (isQuote(C)) {
        Quote = C;
      } ورنہ اگر (C == '#' && At_Token_آغاز) {
        سطر_انتہا = Cur;
        جب (Cur != انتہا_2 && *Cur != '\n')
          ++Cur;
        توڑو;
      }
      At_Token_آغاز = isWhitespace(C);
    }
    // Tokenize line.
    سطر.append(آغاز, سطر_انتہا ? سطر_انتہا : Cur);
    cl::Tokenize_GNUCommand_سطر(سطر, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساکن منطقی has_UTF8_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  واپس (S.سائز_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساکن خالی Expand_بنیاد_Paths(StringRef بنیاد_راستہ, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::راستہ_2::is_absolute(بنیاد_راستہ));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::سائز_type آغاز_Pos = 0;
  برائے (StringRef::سائز_type TokenPos = ArgString.ڈھونڈو(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.ڈھونڈو(Token, آغاز_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(آغاز_Pos, TokenPos - آغاز_Pos);
    اگر (ResponseFile.خالی_خانے())
      ResponseFile = LHS;
    ورنہ
      llvm::sys::راستہ_2::append(ResponseFile, LHS);
    ResponseFile.append(بنیاد_راستہ);
    آغاز_Pos = TokenPos + Token.سائز_2();
  }

  اگر (!ResponseFile.خالی_خانے()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef بقایا = ArgString.substr(آغاز_Pos);
    اگر (!بقایا.خالی_خانے())
      llvm::sys::راستہ_2::append(ResponseFile, بقایا);
    Arg = Saver.save(ResponseFile.str()).ڈیٹا();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_سمتیہ_Impl<ثابت حرف *> &NewArgv) {
  أكد(sys::راستہ_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<حافظہ_بفر>> MemBufOrErr =
      FS->get_بفر_For_File(FName);
  اگر (!MemBufOrErr) {
    std::error_رمز EC = MemBufOrErr.getError();
    واپس llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.پیغام_2());
  }
  حافظہ_بفر &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_بفر_آغاز(), MemBuf.get_بفر_سائز());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.get_بفر_آغاز(), MemBuf.get_بفر_انتہا());
  std::string UTF8Buf;
  اگر (has_UTF16_Byte_ترتیب_Mark(BufRef)) {
    اگر (!convert_UTF16_مقصد_UTF8_String(BufRef, UTF8Buf))
      واپس llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  ورنہ اگر (has_UTF8_Byte_ترتیب_Mark(BufRef))
    Str = StringRef(BufRef.ڈیٹا() + 3, BufRef.سائز_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  اگر (!Relative_نام && !اندر_Config_File)
    واپس Error::success();

  StringRef بنیاد_راستہ = llvm::sys::راستہ_2::والد_راستہ(FName);
  برائے (ثابت حرف *&Arg : NewArgv) {
    اگر (!Arg)
      جاری;

    // Substitute <CFGDIR> with the file's base path.
    اگر (اندر_Config_File)
      Expand_بنیاد_Paths(بنیاد_راستہ, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقی ConfigInclusion = جھوٹ;
    اگر (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      اگر (!llvm::sys::راستہ_2::is_relative(FileName))
        جاری;
    } ورنہ اگر (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = سچ;
    } ورنہ {
      جاری;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ڈالو_عقب('@');
    اگر (ConfigInclusion && !llvm::sys::راستہ_2::has_والد_راستہ(FileName)) {
      SmallString<128> File_راستہ;
      اگر (!ڈھونڈو_Config_File(FileName, File_راستہ))
        واپس createStringError(
            std::make_error_رمز(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_راستہ);
    } ورنہ {
      ResponseFile.append(بنیاد_راستہ);
      llvm::sys::راستہ_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).ڈیٹا();
  }
  واپس Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
  ساخت Response_File_ریکارڈ {
    std::string File;
    size_t انتہا_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_سمتیہ<Response_File_ریکارڈ, 3> File_ڈھیر;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ڈھیر.ڈالو_عقب({"", Argv.سائز_2()});

  // Don't cache Argv.size() because it can change.
  برائے (غیرعلامتی I = 0; I != Argv.سائز_2();) {
    جب (I == File_ڈھیر.عقب().انتہا_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ڈھیر.نکالو_عقب();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    اگر (Arg == خالی_اشاریہ) {
      ++I;
      جاری;
    }

    اگر (Arg[0] != '@') {
      ++I;
      جاری;
    }

    ثابت حرف *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    اگر (llvm::sys::راستہ_2::is_relative(FName)) {
      اگر (CurrentDir.خالی_خانے()) {
        اگر (خودکار CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } ورنہ {
          واپس createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } ورنہ {
        CurrDir = CurrentDir;
      }
      llvm::sys::راستہ_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::حالت_2> Res = FS->حالت_3(FName);
    اگر (!Res || !Res->exists()) {
      std::error_رمز EC = Res.getError();
      اگر (!اندر_Config_File) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        اگر (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          جاری;
        }
      }
      اگر (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      واپس createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.پیغام_2());
    }
    ثابت llvm::vfs::حالت_2 &File_حالت = Res.get();

    خودکار IsEquivalent =
        [File_حالت, یہ](ثابت Response_File_ریکارڈ &RFile) -> ErrorOr<منطقی> {
      ErrorOr<llvm::vfs::حالت_2> RHS = FS->حالت_3(RFile.File);
      اگر (!RHS)
        واپس RHS.getError();
      واپس File_حالت.equivalent(*RHS);
    };

    // Check for recursive response files.
    برائے (ثابت خودکار &F : drop_begin(File_ڈھیر)) {
      اگر (ErrorOr<منطقی> R = IsEquivalent(F)) {
        اگر (R.get())
          واپس createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } ورنہ {
        واپس createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    Small_سمتیہ<ثابت حرف *, 0> ExpandedArgv;
    اگر (Error Err = expandResponseFile(FName, ExpandedArgv))
      واپس Err;

    برائے (Response_File_ریکارڈ &ریکارڈ : File_ڈھیر) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      ریکارڈ.انتہا_2 += ExpandedArgv.سائز_2() - 1;
    }

    File_ڈھیر.ڈالو_عقب({FName, I + ExpandedArgv.سائز_2()});
    Argv.erase(Argv.begin() + I);
    Argv.داخل_کرو(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.انتہا());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_ڈھیر.سائز_2() > 0 && Argv.سائز_2() == File_ڈھیر.عقب().انتہا_2);
  واپس Error::success();
}

منطقی cl::expandResponseFiles(صحیح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             Small_سمتیہ_Impl<ثابت حرف *> &NewArgv) {
#اگر_تعریف_شدہ _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#ورنہ
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#اگر_ختم
  // The environment variable specifies initial options.
  اگر (EnvVar)
    اگر (std::optional<std::string> Env_قدر = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_قدر, Saver, NewArgv, /*MarkEOLs=*/جھوٹ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  اگر (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << مقصد_String(std::move(Err)) << '\n';
    واپس جھوٹ;
  }
  واپس سچ;
}

منطقی cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  اگر (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << مقصد_String(std::move(Err)) << '\n';
    واپس جھوٹ;
  }
  واپس سچ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقی ExpansionContext::ڈھونڈو_Config_File(StringRef FileName,
                                      Small_سمتیہ_Impl<حرف> &File_راستہ) {
  SmallString<128> Cfg_File_راستہ;
  ثابت خودکار FileExists = [یہ](SmallString<128> راستہ) -> منطقی {
    خودکار حالت_2 = FS->حالت_3(راستہ);
    واپس حالت_2 &&
           حالت_2->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  اگر (llvm::sys::راستہ_2::has_والد_راستہ(FileName)) {
    Cfg_File_راستہ = FileName;
    اگر (llvm::sys::راستہ_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_راستہ))
      واپس جھوٹ;
    اگر (!FileExists(Cfg_File_راستہ))
      واپس جھوٹ;
    File_راستہ.تفویض(Cfg_File_راستہ.begin(), Cfg_File_راستہ.انتہا());
    واپس سچ;
  }

  // Look for the file in search directories.
  برائے (ثابت StringRef &Dir : تلاش_Dirs) {
    اگر (Dir.خالی_خانے())
      جاری;
    Cfg_File_راستہ.تفویض(Dir);
    llvm::sys::راستہ_2::append(Cfg_File_راستہ, FileName);
    llvm::sys::راستہ_2::native(Cfg_File_راستہ);
    اگر (FileExists(Cfg_File_راستہ)) {
      File_راستہ.تفویض(Cfg_File_راستہ.begin(), Cfg_File_راستہ.انتہا());
      واپس سچ;
    }
  }

  واپس جھوٹ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_سمتیہ_Impl<ثابت حرف *> &Argv) {
  SmallString<128> Abs_راستہ;
  اگر (sys::راستہ_2::is_relative(CfgFile)) {
    Abs_راستہ.تفویض(CfgFile);
    اگر (std::error_رمز EC = FS->makeAbsolute(Abs_راستہ))
      واپس make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_راستہ.str();
  }
  اندر_Config_File = سچ;
  Relative_نام = سچ;
  اگر (Error Err = expandResponseFile(CfgFile, Argv))
    واپس Err;
  واپس expandResponseFiles(Argv);
}

ساکن خالی initCommonOptions();
ساکن منطقی parse_Command_سطر_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> دلائل,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  Small_سمتیہ<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ڈالو_عقب(دلائل[0]);

  // Parse options from environment variable.
  اگر (EnvVar) {
    اگر (std::optional<std::string> Env_قدر =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_سطر(*Env_قدر, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, دلائل.drop_front());
  صحیح NewArgc = static_cast<صحیح>(NewArgv.سائز_2());

  // Parse all options.
  واپس globalParser().Parse_Command_سطر_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقی cl::Parse_Command_سطر_Options(صحیح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  واپس parse_Command_سطر_Options_Impl(خالی_اشاریہ, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقی cl::parse_Command_سطر_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> دلائل,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  واپس parse_Command_سطر_Options_Impl(&Context, دلائل, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
خالی Command_سطر_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  برائے (خودکار *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_سمتیہ<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.سائز_2());
    برائے (خودکار &O : SC->OptionsMap)
      Opts.ڈالو_عقب(O.دوسرا_2);
    برائے (Option *O : Opts)
      O->reset();
    برائے (Option *O : SC->PositionalOpts)
      O->reset();
    اگر (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  برائے (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

منطقی Command_سطر_Parser::Parse_Command_سطر_Options(
    صحیح argc, ثابت حرف *ثابت *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  أكد(hasOptions() && "No options specified!");

  پروگرام_Overview = Overview;
  منطقی IgnoreErrors = Errs;
  اگر (!Errs)
    Errs = &errs();
  اگر (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقی ErrorParsing = جھوٹ;

  // Expand response files.
  Small_سمتیہ<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#اگر_تعریف_شدہ _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#ورنہ
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#اگر_ختم
  ExpansionContext ECtx(A, Tokenize, VFS);
  اگر (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << مقصد_String(std::move(Err)) << '\n';
    واپس جھوٹ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحیح>(newArgv.سائز_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  پروگرام_Name = std::string(sys::راستہ_2::filename(StringRef(argv[0])));
  اشاریہ_Library_Options();

  // Check out the positional arguments to collect information about them.
  غیرعلامتی عدد_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقی HasUnlimitedPositionals = جھوٹ;

  صحیح پہلا_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_چوٹی_Level();
  std::string قریب_ترین_Sub_Command_String;
  منطقی MaybeNamedSubCommand =
      argc >= 2 && argv[پہلا_Arg][0] != '-' && hasNamedSubCommands();
  اگر (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[پہلا_Arg]), قریب_ترین_Sub_Command_String);
    اگر (ChosenSubCommand != &SubCommand::get_چوٹی_Level())
      پہلا_Arg = 2;
  }
  globalParser().فعال_Sub_Command = ChosenSubCommand;

  أكد(ChosenSubCommand);
  خودکار &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  خودکار &PositionalOpts = ChosenSubCommand->PositionalOpts;
  خودکار &OptionsMap = ChosenSubCommand->OptionsMap;

  اگر (ConsumeAfterOpt) {
    أكد(PositionalOpts.سائز_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  اگر (!PositionalOpts.خالی_خانے()) {

    // Calculate how many positional values are _required_.
    منطقی Unbounded_ملا = جھوٹ;
    برائے (size_t i = 0, e = PositionalOpts.سائز_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      اگر (Requires_قدر(Opt))
        ++عدد_Positional_Required;
      ورنہ اگر (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        اگر (PositionalOpts.سائز_2() > 1) {
          اگر (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = سچ;
        }
      } ورنہ اگر (Unbounded_ملا && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        اگر (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << پروگرام_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.سائز_2();
        ErrorParsing = سچ;
      }
      Unbounded_ملا |= Eats_Unbounded_نمبر_کا_اقدار(Opt);
    }
    HasUnlimitedPositionals = Unbounded_ملا || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_سمتیہ<std::pair<StringRef, غیرعلامتی>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *فعال_Positional_Arg = خالی_اشاریہ;

  // Loop over all of the arguments... processing them.
  منطقی Dash_Dash_ملا = جھوٹ; // Have we read '--'?
  برائے (صحیح i = پہلا_Arg; i < argc; ++i) {
    Option *Handler = خالی_اشاریہ;
    std::string قریب_ترین_Handler_String;
    StringRef قدر;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    اگر (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_ملا) {
      // Positional argument!
      اگر (فعال_Positional_Arg) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        جاری; // We are done!
      }

      اگر (!PositionalOpts.خالی_خانے()) {
        PositionalVals.ڈالو_عقب(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        اگر (PositionalVals.سائز_2() >= عدد_Positional_Required && ConsumeAfterOpt) {
          برائے (++i; i < argc; ++i)
            PositionalVals.ڈالو_عقب(std::make_pair(StringRef(argv[i]), i));
          توڑو; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        جاری;
      }
    } ورنہ اگر (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_ملا) {
      Dash_Dash_ملا = سچ; // This is the mythical "--"?
      جاری;             // Don't try to process it as an argument itself.
    } ورنہ اگر (فعال_Positional_Arg &&
               (فعال_Positional_Arg->getMiscFlags() & Positional_Eats_دلائل)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قدر);
      اگر (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        جاری; // We are done!
      }
    } ورنہ { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, قدر);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      اگر (!Handler && ChosenSubCommand != &SubCommand::get_چوٹی_Level())
        Handler = LookupOption(SubCommand::get_چوٹی_Level(), ArgName, قدر);

      اگر (!Handler) {
        اگر (LibraryOptions *L = lookupLibraryOption(ArgName.بانٹو('=').پہلا_2)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          غیرعلامتی N;
          اگر (Error E = Context && L->is_فی_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << پروگرام_Name << ": " << مقصد_String(std::move(E)) << '\n';
            ErrorParsing = سچ;
          }
          i += N - 1;
          جاری;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      اگر (!Handler)
        Handler = HandlePrefixedOption(ArgName, قدر, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      اگر (!Handler)
        Lookup_قریب_ترین_Option(ArgName, OptionsMap, قریب_ترین_Handler_String);
    }

    اگر (!Handler) {
      خودکار Report_Unknown_دلیل = [&](منطقی IsArg,
                                       StringRef قریب_ترین_دلیل_Name) {
        *Errs << پروگرام_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        اگر (قریب_ترین_دلیل_Name.خالی_خانے())
          واپس;

        *Errs << پروگرام_Name << ": Did you mean '";
        اگر (IsArg)
          *Errs << PrintArg(قریب_ترین_دلیل_Name, 0);
        ورنہ
          *Errs << قریب_ترین_دلیل_Name;
        *Errs << "'?\n";
      };

      اگر (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_دلیل(/*IsArg=*/سچ, قریب_ترین_Handler_String);
      ورنہ
        Report_Unknown_دلیل(/*IsArg=*/جھوٹ, قریب_ترین_Sub_Command_String);

      ErrorParsing = سچ;
      جاری;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    اگر (Handler->getFormattingFlag() == cl::Positional) {
      اگر ((Handler->getMiscFlags() & Positional_Eats_دلائل) && !قدر.خالی_خانے()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = سچ;
      }
      فعال_Positional_Arg = Handler;
    }
    ورنہ
      ErrorParsing |= ProvideOption(Handler, ArgName, قدر, argc, argv, i);
  }

  // Check and handle positional arguments now...
  اگر (عدد_Positional_Required > PositionalVals.سائز_2()) {
      *Errs << پروگرام_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << عدد_Positional_Required
             << " positional argument" << (عدد_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = سچ;
  } ورنہ اگر (!HasUnlimitedPositionals &&
             PositionalVals.سائز_2() > PositionalOpts.سائز_2()) {
    *Errs << پروگرام_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.سائز_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = سچ;

  } ورنہ اگر (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    غیرعلامتی ValNo = 0, عدد_Vals = static_cast<غیرعلامتی>(PositionalVals.سائز_2());
    برائے (Option *Opt : PositionalOpts) {
      اگر (Requires_قدر(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].پہلا_2,
                                PositionalVals[ValNo].دوسرا_2);
        ValNo++;
        --عدد_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقی مکمل = Opt->get_عدد_Occurrences_Flag() == cl::Required;
      جب (عدد_Vals - ValNo > عدد_Positional_Required && !مکمل) {
        انتخاب (Opt->get_عدد_Occurrences_Flag()) {
        حالت cl::Optional:
          مکمل = سچ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالت cl::Zero_Or_مزید: // Zero or more will take all they can get...
        حالت cl::One_Or_مزید:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].پہلا_2,
                                  PositionalVals[ValNo].دوسرا_2);
          ValNo++;
          توڑو;
        طےشدہ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } ورنہ {
    أكد(ConsumeAfterOpt && عدد_Positional_Required <= PositionalVals.سائز_2());
    غیرعلامتی ValNo = 0;
    برائے (Option *Opt : PositionalOpts)
      اگر (Requires_قدر(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].پہلا_2, PositionalVals[ValNo].دوسرا_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    اگر (PositionalOpts.سائز_2() == 1 && ValNo == 0 && !PositionalVals.خالی_خانے()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].پہلا_2,
                                              PositionalVals[ValNo].دوسرا_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    برائے (; ValNo != PositionalVals.سائز_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].پہلا_2,
                                  PositionalVals[ValNo].دوسرا_2);
  }

  // Loop over args and make sure all required args are specified!
  برائے (ثابت خودکار &Opt : OptionsMap) {
    انتخاب (Opt.دوسرا_2->get_عدد_Occurrences_Flag()) {
    حالت Required:
    حالت One_Or_مزید:
      اگر (Opt.دوسرا_2->get_عدد_Occurrences() == 0) {
        Opt.دوسرا_2->error("must be specified at least once!");
        ErrorParsing = سچ;
      }
      [[fallthrough]];
    طےشدہ:
      توڑو;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             برائے (صحیح i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  مزید_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  اگر (ErrorParsing) {
    اگر (!IgnoreErrors)
      باہر_نکلو(1);
    واپس جھوٹ;
  }
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقی Option::error(ثابت Twine &پیغام_3, StringRef ArgName, raw_ostream &Errs) {
  اگر (!ArgName.ڈیٹا())
    ArgName = ArgStr;
  اگر (ArgName.خالی_خانے())
    Errs << HelpStr; // Be nice for positional arguments
  ورنہ
    Errs << globalParser().پروگرام_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << پیغام_3 << "\n";
  واپس سچ;
}

منطقی Option::جوڑو_Occurrence(غیرعلامتی pos, StringRef ArgName, StringRef قدر) {
  ++عدد_Occurrences;
  واپس handleOccurrence(pos, ArgName, قدر);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساکن StringRef get_قدر_Str(ثابت Option &O, StringRef Default_پیغام) {
  اگر (O.قدر_Str.خالی_خانے())
    واپس Default_پیغام;
  واپس O.قدر_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_چوڑائی() ثابت {
  واپس arg_Plus_Prefixes_سائز(ArgStr);
}

خالی Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t پہلا_سطر_Indented_By) {
  أكد(Indent >= پہلا_سطر_Indented_By);
  std::pair<StringRef, StringRef> بانٹو_2 = HelpStr.بانٹو('\n');
  outs().indent(Indent - پہلا_سطر_Indented_By)
      << Arg_Help_سابقہ << بانٹو_2.پہلا_2 << "\n";
  جب (!بانٹو_2.دوسرا_2.خالی_خانے()) {
    بانٹو_2 = بانٹو_2.دوسرا_2.بانٹو('\n');
    outs().indent(Indent) << بانٹو_2.پہلا_2 << "\n";
  }
}

خالی Option::printEnumValHelpStr(StringRef HelpStr, size_t بنیاد_Indent,
                                 size_t پہلا_سطر_Indented_By) {
  ثابت StringRef Val_Help_سابقہ = "  ";
  أكد(بنیاد_Indent >= پہلا_سطر_Indented_By);
  std::pair<StringRef, StringRef> بانٹو_2 = HelpStr.بانٹو('\n');
  outs().indent(بنیاد_Indent - پہلا_سطر_Indented_By)
      << Arg_Help_سابقہ << Val_Help_سابقہ << بانٹو_2.پہلا_2 << "\n";
  جب (!بانٹو_2.دوسرا_2.خالی_خانے()) {
    بانٹو_2 = بانٹو_2.دوسرا_2.بانٹو('\n');
    outs().indent(بنیاد_Indent + Val_Help_سابقہ.سائز_2()) << بانٹو_2.پہلا_2 << "\n";
  }
}

// Print out the option for the alias.
خالی alias::printOptionInfo(size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_چوڑائی, arg_Plus_Prefixes_سائز(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_چوڑائی(ثابت Option &O) ثابت {
  size_t Len = arg_Plus_Prefixes_سائز(O.ArgStr);
  خودکار ValName = get_قدر_Name();
  اگر (!ValName.خالی_خانے()) {
    size_t FormattingLen = 3;
    اگر (O.getMiscFlags() & Positional_Eats_دلائل)
      FormattingLen = 6;
    Len += get_قدر_Str(O, ValName).سائز_2() + FormattingLen;
  }

  واپس Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
خالی basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(O.ArgStr);

  خودکار ValName = get_قدر_Name();
  اگر (!ValName.خالی_خانے()) {
    اگر (O.getMiscFlags() & Positional_Eats_دلائل) {
      outs() << " <" << get_قدر_Str(O, ValName) << ">...";
    } ورنہ اگر (O.get_قدر_Expected_Flag() == قدر_Optional)
      outs() << "[=<" << get_قدر_Str(O, ValName) << ">]";
    ورنہ {
      outs() << (O.ArgStr.سائز_2() == 1 ? " <" : "=<") << get_قدر_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_چوڑائی, get_Option_چوڑائی(O));
}

خالی basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t Global_چوڑائی) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_چوڑائی - O.ArgStr.سائز_2());
}

// parser<bool> implementation
//
منطقی parser<منطقی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقی &قدر) {
  واپس parseBool<منطقی, سچ, جھوٹ>(O, ArgName, Arg, قدر);
}

// parser<boolOrDefault> implementation
//
منطقی parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &قدر) {
  واپس parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, قدر);
}

// parser<FixedOrScalableQuantity> implementation
//
سانچہ <قسم_نام FixedOrScalableQuantityT>
ساکن منطقی parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef قدر_قسم,
                                         FixedOrScalableQuantityT &قدر) {
  استعمال ScalarTy = قسم_نام FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_قدر;
  اگر (!Arg.getAsInteger(0, Min_قدر)) {
    قدر = FixedOrScalableQuantityT::getFixed(Min_قدر);
    واپس جھوٹ;
  }

  StringRef Remainder = Arg;
  اگر (!Remainder.consume_front("vscale"))
    واپس O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  Remainder = Remainder.ltrim();
  اگر (!Remainder.consume_front('x'))
    واپس O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  Remainder = Remainder.ltrim();
  اگر (Remainder.getAsInteger(0, Min_قدر))
    واپس O.error("'" + Arg + "' value invalid for " + قدر_قسم +
                   " argument!");

  قدر = FixedOrScalableQuantityT::getScalable(Min_قدر);
  واپس جھوٹ;
}

// parser<int> implementation
//
منطقی parser<صحیح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحیح &قدر) {
  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for integer argument!");
  واپس جھوٹ;
}

// parser<long> implementation
//
منطقی parser<طویل>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         طویل &قدر) {
  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for long argument!");
  واپس جھوٹ;
}

// parser<long long> implementation
//
منطقی parser<طویل طویل>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              طویل طویل &قدر) {
  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for llong argument!");
  واپس جھوٹ;
}

// parser<unsigned> implementation
//
منطقی parser<غیرعلامتی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             غیرعلامتی &قدر) {

  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for uint argument!");
  واپس جھوٹ;
}

// parser<unsigned long> implementation
//
منطقی parser<غیرعلامتی طویل>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  غیرعلامتی طویل &قدر) {

  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for ulong argument!");
  واپس جھوٹ;
}

// parser<unsigned long long> implementation
//
منطقی parser<غیرعلامتی طویل طویل>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       غیرعلامتی طویل طویل &قدر) {

  اگر (Arg.getAsInteger(0, قدر))
    واپس O.error("'" + Arg + "' value invalid for ullong argument!");
  واپس جھوٹ;
}

// parser<ElementCount> implementation
//
منطقی parser<Element_گنتی>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_گنتی &قدر) {
  واپس parseFixedOrScalableQuantity(O, Arg, get_قدر_Name(), قدر);
}

// parser<double>/parser<float> implementation
//
ساکن منطقی parseDouble(Option &O, StringRef Arg, دوہرا &قدر) {
  اگر (مقصد_float(Arg, قدر))
    واپس جھوٹ;
  واپس O.error("'" + Arg + "' value invalid for floating point argument!");
}

منطقی parser<دوہرا>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           دوہرا &Val) {
  واپس parseDouble(O, Arg, Val);
}

منطقی parser<اعشاری>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          اعشاری &Val) {
  دوہرا dVal;
  اگر (parseDouble(O, Arg, dVal))
    واپس سچ;
  Val = (اعشاری)dVal;
  واپس جھوٹ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
غیرعلامتی generic_parser_بنیاد::ڈھونڈو_Option(StringRef Name) {
  غیرعلامتی e = get_عدد_Options();

  برائے (غیرعلامتی i = 0; i != e; ++i) {
    اگر (getOption(i) == Name)
      واپس i;
  }
  واپس e;
}

ساکن StringRef Eq_قدر = "=<value>";
ساکن StringRef خالی_خانے_Option = "<empty>";
ساکن StringRef Option_سابقہ = "    =";
ساکن size_t get_Option_Prefixes_سائز() {
  واپس Option_سابقہ.سائز_2() + Arg_Help_سابقہ.سائز_2();
}

ساکن منطقی shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  واپس O.get_قدر_Expected_Flag() != قدر_Optional || !Name.خالی_خانے() ||
         !Description.خالی_خانے();
}

// Return the width of the option tag for printing...
size_t generic_parser_بنیاد::get_Option_چوڑائی(ثابت Option &O) ثابت {
  اگر (O.hasArgStr()) {
    size_t سائز =
        arg_Plus_Prefixes_سائز(O.ArgStr) + Eq_قدر.سائز_2();
    برائے (غیرعلامتی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      اگر (!shouldPrintOption(Name, getDescription(i), O))
        جاری;
      size_t Name_سائز = Name.خالی_خانے() ? خالی_خانے_Option.سائز_2() : Name.سائز_2();
      سائز = std::max(سائز, Name_سائز + get_Option_Prefixes_سائز());
    }
    واپس سائز;
  } ورنہ {
    size_t بنیاد_سائز = 0;
    برائے (غیرعلامتی i = 0, e = get_عدد_Options(); i != e; ++i)
      بنیاد_سائز = std::max(بنیاد_سائز, getOption(i).سائز_2() + 8);
    واپس بنیاد_سائز;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
خالی generic_parser_بنیاد::printOptionInfo(ثابت Option &O,
                                          size_t Global_چوڑائی) ثابت {
  اگر (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    اگر (O.get_قدر_Expected_Flag() == قدر_Optional) {
      برائے (غیرعلامتی i = 0, e = get_عدد_Options(); i != e; ++i) {
        اگر (getOption(i).خالی_خانے()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_چوڑائی,
                               arg_Plus_Prefixes_سائز(O.ArgStr));
          توڑو;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_قدر;
    Option::printHelpStr(O.HelpStr, Global_چوڑائی,
                         Eq_قدر.سائز_2() +
                             arg_Plus_Prefixes_سائز(O.ArgStr));
    برائے (غیرعلامتی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      اگر (!shouldPrintOption(OptionName, Description, O))
        جاری;
      size_t پہلا_سطر_Indent = OptionName.سائز_2() + get_Option_Prefixes_سائز();
      outs() << Option_سابقہ << OptionName;
      اگر (OptionName.خالی_خانے()) {
        outs() << خالی_خانے_Option;
        أكد(پہلا_سطر_Indent >= خالی_خانے_Option.سائز_2());
        پہلا_سطر_Indent += خالی_خانے_Option.سائز_2();
      }
      اگر (!Description.خالی_خانے())
        Option::printEnumValHelpStr(Description, Global_چوڑائی, پہلا_سطر_Indent);
      ورنہ
        outs() << '\n';
    }
  } ورنہ {
    اگر (!O.HelpStr.خالی_خانے())
      outs() << "  " << O.HelpStr << '\n';
    برائے (غیرعلامتی i = 0, e = get_عدد_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_چوڑائی, Option.سائز_2() + 8);
    }
  }
}

ساکن ثابت size_t Max_Opt_چوڑائی = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
خالی generic_parser_بنیاد::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_قدر &قدر,
    ثابت Generic_Option_قدر &Default, size_t Global_چوڑائی) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_چوڑائی - O.ArgStr.سائز_2());

  غیرعلامتی عدد_Opts = get_عدد_Options();
  برائے (غیرعلامتی i = 0; i != عدد_Opts; ++i) {
    اگر (!قدر.compare(get_Option_قدر(i)))
      جاری;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).سائز_2();
    size_t عدد_Spaces = Max_Opt_چوڑائی > L ? Max_Opt_چوڑائی - L : 0;
    outs().indent(عدد_Spaces) << " (default: ";
    برائے (غیرعلامتی j = 0; j != عدد_Opts; ++j) {
      اگر (!Default.compare(get_Option_قدر(j)))
        جاری;
      outs() << getOption(j);
      توڑو;
    }
    outs() << ")\n";
    واپس;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
نامخلا llvm {
نامخلا cl {
ساکن raw_ostream &عامل<<(raw_ostream &OS, boolOrDefault V) {
  واپس OS << static_cast<صحیح>(V);
}
} // namespace cl
} // namespace llvm

#تعریف_کرو PRINT_OPT_DIFF(T)                                                      \
  خالی parser<T>::printOptionDiff(ثابت Option &O, T V, Option_قدر<T> D,      \
                                  size_t Global_چوڑائی) ثابت {                  \
    printOptionName(O, Global_چوڑائی);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t عدد_Spaces =                                                         \
        Max_Opt_چوڑائی > Str.سائز_2() ? Max_Opt_چوڑائی - Str.سائز_2() : 0;               \
    outs().indent(عدد_Spaces) << " (default: ";                                 \
    اگر (D.has_قدر())                                                          \
      outs() << D.get_قدر();                                                  \
    ورنہ                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقی)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحیح)
PRINT_OPT_DIFF(طویل)
PRINT_OPT_DIFF(طویل طویل)
PRINT_OPT_DIFF(غیرعلامتی)
PRINT_OPT_DIFF(غیرعلامتی طویل)
PRINT_OPT_DIFF(غیرعلامتی طویل طویل)
PRINT_OPT_DIFF(دوہرا)
PRINT_OPT_DIFF(اعشاری)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_گنتی)

خالی parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_قدر<std::string> &D,
                                          size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= " << V;
  size_t عدد_Spaces = Max_Opt_چوڑائی > V.سائز_2() ? Max_Opt_چوڑائی - V.سائز_2() : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  اگر (D.has_قدر())
    outs() << D.get_قدر();
  ورنہ
    outs() << "*no default*";
  outs() << ")\n";
}

خالی parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_قدر<std::optional<std::string>> &D,
    size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= " << V;
  size_t VSize = V.has_قدر_2() ? V.قدر_2().سائز_2() : 0;
  size_t عدد_Spaces = Max_Opt_چوڑائی > VSize ? Max_Opt_چوڑائی - VSize : 0;
  outs().indent(عدد_Spaces) << " (default: ";
  اگر (D.has_قدر() && D.get_قدر().has_قدر_2())
    outs() << D.get_قدر();
  ورنہ
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
خالی basic_parser_impl::print_Option_No_قدر(ثابت Option &O,
                                           size_t Global_چوڑائی) ثابت {
  printOptionName(O, Global_چوڑائی);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساکن صحیح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  واپس strcmp(LHS->پہلا_2, RHS->پہلا_2);
}

ساکن صحیح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  واپس strcmp(LHS->پہلا_2, RHS->پہلا_2);
}

// Copy Options into a vector so we can sort them as we like.
ساکن خالی ترتیب_دو_Opts(OptionsMapTy &OptMap,
                     Small_سمتیہ_Impl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقی ShowHidden) {
  Small_Ptr_سیٹ<Option *, 32> Option_سیٹ; // Duplicate option detection.

  برائے (خودکار I = OptMap.begin(), E = OptMap.انتہا(); I != E; ++I) {
    // Ignore really-hidden options.
    اگر (I->دوسرا_2->getOptionHiddenFlag() == ReallyHidden)
      جاری;

    // Unless showhidden is set, ignore hidden flags.
    اگر (I->دوسرا_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      جاری;

    // If we've already seen this option, don't add it to the list again.
    اگر (!Option_سیٹ.داخل_کرو(I->دوسرا_2).دوسرا_2)
      جاری;

    Opts.ڈالو_عقب(
        std::pair<ثابت حرف *, Option *>(I->پہلا_2.ڈیٹا(), I->دوسرا_2));
  }

  // Sort the options list alphabetically.
  array_pod_ترتیب_دو(Opts.begin(), Opts.انتہا(), OptNameCompare);
}

ساکن خالی
ترتیب_دو_Sub_Commands(ثابت Small_Ptr_سیٹ_Impl<SubCommand *> &SubMap,
                Small_سمتیہ_Impl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  برائے (خودکار *S : SubMap) {
    اگر (S->getName().خالی_خانے())
      جاری;
    Subs.ڈالو_عقب(std::make_pair(S->getName().ڈیٹا(), S));
  }
  array_pod_ترتیب_دو(Subs.begin(), Subs.انتہا(), SubNameCompare);
}

نامخلا {

جماعت HelpPrinter {
محفوظ:
  ثابت منطقی ShowHidden;
  استعمال Str_Option_Pair_سمتیہ =
      Small_سمتیہ<std::pair<ثابت حرف *, Option *>, 128>;
  استعمال Str_Sub_Command_Pair_سمتیہ =
      Small_سمتیہ<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازی خالی printOptions(Str_Option_Pair_سمتیہ &Opts, size_t MaxArgLen) {
    برائے (ثابت خودکار &Opt : Opts)
      Opt.دوسرا_2->printOptionInfo(MaxArgLen);
  }

  خالی printSubCommands(Str_Sub_Command_Pair_سمتیہ &Subs, size_t MaxSubLen) {
    برائے (ثابت خودکار &S : Subs) {
      outs() << "  " << S.پہلا_2;
      اگر (!S.دوسرا_2->getDescription().خالی_خانے()) {
        outs().indent(MaxSubLen - لڑی_لمبائی(S.پہلا_2));
        outs() << " - " << S.دوسرا_2->getDescription();
      }
      outs() << "\n";
    }
  }

عوامی:
  صریح HelpPrinter(منطقی showHidden) : ShowHidden(showHidden) {}
  مجازی ~HelpPrinter() = طےشدہ;

  // Invoke the printer.
  خالی عامل=(منطقی قدر) {
    اگر (!قدر)
      واپس;
    printHelp();

    // Halt the program since help information was printed
    باہر_نکلو(0);
  }

  خالی printHelp() {
    SubCommand *Sub = globalParser().get_فعال_Sub_Command();
    خودکار &OptionsMap = Sub->OptionsMap;
    خودکار &PositionalOpts = Sub->PositionalOpts;
    خودکار &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_سمتیہ Opts;
    ترتیب_دو_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_سمتیہ Subs;
    ترتیب_دو_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    اگر (!globalParser().پروگرام_Overview.خالی_خانے())
      outs() << "OVERVIEW: " << globalParser().پروگرام_Overview << "\n";

    اگر (Sub == &SubCommand::get_چوٹی_Level()) {
      outs() << "USAGE: " << globalParser().پروگرام_Name;
      اگر (!Subs.خالی_خانے())
        outs() << " [subcommand]";
      outs() << " [options]";
    } ورنہ {
      اگر (!Sub->getDescription().خالی_خانے()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().پروگرام_Name << " " << Sub->getName()
             << " [options]";
    }

    برائے (خودکار *Opt : PositionalOpts) {
      اگر (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    اگر (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    اگر (Sub == &SubCommand::get_چوٹی_Level() && !Subs.خالی_خانے()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      برائے (ثابت خودکار &Sub : Subs)
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
    برائے (ثابت خودکار &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.دوسرا_2->get_Option_چوڑائی());

    Small_سمتیہ<std::pair<std::string, StringRef>, 0> LibraryOpts;
    اگر (ShowHidden)
      برائے (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              اگر (!Help.خالی_خانے())
                LibraryOpts.emplace_عقب((Spelling + MetaVar).str(), Help);
            });
    llvm::ترتیب_دو(LibraryOpts);
    برائے (ثابت خودکار &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_سائز(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    اگر (!LibraryOpts.خالی_خانے())
      outs() << "\nLibrary options:\n\n";
    برائے (ثابت خودکار &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_سائز(Name));
    }

    // Print any extra help the user has declared.
    برائے (ثابت خودکار &I : globalParser().مزید_Help)
      outs() << I;
    globalParser().مزید_Help.clear();
  }
};

جماعت CategorizedHelpPrinter : عوامی HelpPrinter {
عوامی:
  صریح CategorizedHelpPrinter(منطقی showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساکن صحیح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    واپس (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  استعمال HelpPrinter::عامل=;

محفوظ:
  خالی printOptions(Str_Option_Pair_سمتیہ &Opts, size_t MaxArgLen) override {
    std::سمتیہ<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::سمتیہ<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.سائز_2() > 0 && "No option categories registered!");
    array_pod_ترتیب_دو(SortedCategories.begin(), SortedCategories.انتہا(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    برائے (ثابت خودکار &I : Opts) {
      Option *Opt = I.دوسرا_2;
      برائے (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ڈالو_عقب(Opt);
      }
    }

    // Now do printing.
    برائے (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت خودکار &CategoryOptions = CategorizedOptions[Category];
      اگر (CategoryOptions.خالی_خانے())
        جاری;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      اگر (!Category->getDescription().خالی_خانے())
        outs() << Category->getDescription() << "\n\n";
      ورنہ
        outs() << "\n";

      // Loop over the options in the category and print.
      برائے (ثابت Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
جماعت HelpPrinterWrapper {
نجی:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

عوامی:
  صریح HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  خالی عامل=(منطقی قدر);
};

} // End anonymous namespace

#اگر defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# اگر defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# ورنہ
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#وگرنہ_اگر defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# اگر defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# ورنہ
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#ورنہ
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#اگر_ختم

نامخلا {
جماعت نسخہ_Printer {
عوامی:
  خالی print(ثابت std::سمتیہ<نسخہ_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#اگر_تعریف_شدہ PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#ورنہ
    OS << "LLVM (http://llvm.org/):\n  ";
#اگر_ختم
    OS << PACKAGE_NAME << " version " << PACKAGE_نسخہ << "\n  ";
#اگر LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#ورنہ
    OS << "Optimized build";
#اگر_ختم
#اگر_غیر_تعریف_شدہ NDEBUG
    OS << " with assertions";
#اگر_ختم
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    اگر (!ExtraPrinters.خالی_خانے()) {
      برائے (ثابت خودکار &I : ExtraPrinters)
        I(outs());
    }
  }
  خالی عامل=(منطقی OptionWasSpecified);
};

ساخت Command_سطر_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_معمول_Printer{جھوٹ};
  HelpPrinter UncategorizedHiddenPrinter{سچ};
  CategorizedHelpPrinter Categorized_معمول_Printer{جھوٹ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{سچ};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_معمول_Printer{Uncategorized_معمول_Printer,
                                          Categorized_معمول_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, سچ, parser<منطقی>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_معمول_Printer),
      cl::Hidden,
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, سچ, parser<منطقی>> HLHOp{
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
  cl::opt<HelpPrinterWrapper, سچ, parser<منطقی>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_معمول_Printer),
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, سچ, parser<منطقی>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::قدر_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقی> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(جھوٹ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<منطقی> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(جھوٹ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  نسخہ_Printer_Ty Override_نسخہ_Printer = خالی_اشاریہ;

  std::سمتیہ<نسخہ_Printer_Ty> Extra_نسخہ_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  نسخہ_Printer نسخہ_Printer_Instance;

  cl::opt<نسخہ_Printer, سچ, parser<منطقی>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(نسخہ_Printer_Instance), cl::قدر_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ساکن ManagedStatic<Command_سطر_Common_Options> CommonOptions;

ساکن خالی initCommonOptions() {
  *CommonOptions;
  init_Debug_شمار_Options();
  init_گراف_Writer_Options();
  init_اشارات_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_بیج_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ساکن OptionCategory GeneralCategory{"General options"};
  واپس GeneralCategory;
}

خالی نسخہ_Printer::عامل=(منطقی OptionWasSpecified) {
  اگر (!OptionWasSpecified)
    واپس;

  اگر (CommonOptions->Override_نسخہ_Printer != خالی_اشاریہ) {
    CommonOptions->Override_نسخہ_Printer(outs());
    باہر_نکلو(0);
  }
  print(CommonOptions->Extra_نسخہ_Printers);

  باہر_نکلو(0);
}

خالی HelpPrinterWrapper::عامل=(منطقی قدر) {
  اگر (!قدر)
    واپس;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  اگر (globalParser().RegisteredOptionCategories.سائز_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.سیٹ_Hidden_Flag(NotHidden);

    CategorizedPrinter = سچ; // Invoke categorized printer
  } ورنہ {
    UncategorizedPrinter = سچ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
خالی cl::Print_Option_اقدار() { globalParser().print_Option_اقدار(); }

خالی Command_سطر_Parser::print_Option_اقدار() {
  اگر (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    واپس;

  Small_سمتیہ<std::pair<ثابت حرف *, Option *>, 128> Opts;
  ترتیب_دو_Opts(فعال_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ سچ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  برائے (ثابت خودکار &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.دوسرا_2->get_Option_چوڑائی());

  برائے (ثابت خودکار &Opt : Opts)
    Opt.دوسرا_2->print_Option_قدر(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
خالی cl::Print_Help_پیغام(منطقی Hidden, منطقی Categorized) {
  اگر (!Hidden && !Categorized)
    CommonOptions->Uncategorized_معمول_Printer.printHelp();
  ورنہ اگر (!Hidden && Categorized)
    CommonOptions->Categorized_معمول_Printer.printHelp();
  ورنہ اگر (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  ورنہ
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ساکن ثابت StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#اگر LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#اگر_ختم
#اگر_غیر_تعریف_شدہ NDEBUG
      "+assertions",
#اگر_ختم
#اگر_تعریف_شدہ EXPENSIVE_CHECKS
      "+expensive-checks",
#اگر_ختم
#اگر __has_feature(پتہ_sanitizer)
      "+asan",
#اگر_ختم
#اگر __has_feature(dataflow_sanitizer)
      "+dfsan",
#اگر_ختم
#اگر __has_feature(hwaddress_sanitizer)
      "+hwasan",
#اگر_ختم
#اگر __has_feature(حافظہ_sanitizer)
      "+msan",
#اگر_ختم
#اگر __has_feature(thread_sanitizer)
      "+tsan",
#اگر_ختم
#اگر __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#اگر_ختم
#اگر_تعریف_شدہ LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#اگر_ختم
  };
  واپس ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
خالی cl::printBuildConfig(raw_ostream &OS) {
#اگر LLVM_نسخہ_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#اگر_ختم
}

/// Utility function for printing version number.
خالی cl::Print_نسخہ_پیغام() {
  CommonOptions->نسخہ_Printer_Instance.print(CommonOptions->Extra_نسخہ_Printers);
}

خالی cl::سیٹ_نسخہ_Printer(نسخہ_Printer_Ty func) {
  CommonOptions->Override_نسخہ_Printer = func;
}

خالی cl::جوڑو_Extra_نسخہ_Printer(نسخہ_Printer_Ty func) {
  CommonOptions->Extra_نسخہ_Printers.ڈالو_عقب(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  خودکار &Subs = globalParser().RegisteredSubCommands;
  (خالی)Subs;
  أكد(Subs.شامل_ہے(&Sub));
  واپس Sub.OptionsMap;
}

iterator_range<Small_Ptr_سیٹ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  واپس globalParser().getRegisteredSubcommands();
}

خالی cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  برائے (خودکار &I : Sub.OptionsMap) {
    منطقی Unrelated = سچ;
    برائے (خودکار &Cat : I.دوسرا_2->Categories) {
      اگر (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = جھوٹ;
    }
    اگر (Unrelated)
      I.دوسرا_2->سیٹ_Hidden_Flag(cl::ReallyHidden);
  }
}

خالی cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  برائے (خودکار &I : Sub.OptionsMap) {
    منطقی Unrelated = سچ;
    برائے (خودکار &Cat : I.دوسرا_2->Categories) {
      اگر (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = جھوٹ;
    }
    اگر (Unrelated)
      I.دوسرا_2->سیٹ_Hidden_Flag(cl::ReallyHidden);
  }
}

خالی cl::Reset_Command_سطر_Parser() { globalParser().reset(); }

خالی cl::جوڑو_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ڈالو_عقب(&L);
}

خالی LibraryOptionsContext::تفویض(ثابت LibraryOptionsContext &Other) {
  أكد(یہ != &Other);
  اقدار.clear();
  Saved_دلائل.clear();
  ArrayRef<ثابت حرف *> دلائل = Other.Saved_دلائل;
  برائے (size_t I = 0, E = دلائل.سائز_2(); I != E;) {
    StringRef Name = دلائل[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.بانٹو('=').پہلا_2);
    غیرعلامتی N;
    cantFail(parse(*L, دلائل.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ثابت حرف *> دلائل,
                                   غیرعلامتی &Consumed) {
  خودکار [It, Inserted] = اقدار.try_emplace(L.getDefault());
  اگر (Inserted)
    It->دوسرا_2 = L.copyDefault(Alloc);
  واپس parseLibraryOption(L, It->دوسرا_2, دلائل, Consumed, Alloc, &Saved_دلائل);
}

خالی cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

خالی LLVMParse_Command_سطر_Options(صحیح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_سطر_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
