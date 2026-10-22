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
استفاده نطاق llvm;
استفاده نطاق cl;

#تعریف_کن DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
نطاق llvm {
نطاق cl {
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<منطقي>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<صحيح>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<دراز>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<دراز دراز>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<بیعلامت>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<بیعلامت دراز>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<بیعلامت دراز دراز>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<مزدوج>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<عائم>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<حرف>;
قالب صنف LLVM_EXPORT_TEMPLATE basic_parser<Element_شمار>;

#إذا !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
قالب صنف LLVM_EXPORT_TEMPLATE opt<std::string>;
#پایان_اگر

قالب صنف LLVM_EXPORT_TEMPLATE opt<منطقي>;
قالب صنف LLVM_EXPORT_TEMPLATE opt<حرف>;
قالب صنف LLVM_EXPORT_TEMPLATE opt<صحيح>;
قالب صنف LLVM_EXPORT_TEMPLATE opt<بیعلامت>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
فراغ Generic_Option_مقدار::anchor() {}
فراغ Option_مقدار<boolOrDefault>::anchor() {}
فراغ Option_مقدار<std::string>::anchor() {}
فراغ Option::anchor() {}
فراغ basic_parser_impl::anchor() {}
فراغ parser<منطقي>::anchor() {}
فراغ parser<boolOrDefault>::anchor() {}
فراغ parser<صحيح>::anchor() {}
فراغ parser<دراز>::anchor() {}
فراغ parser<دراز دراز>::anchor() {}
فراغ parser<بیعلامت>::anchor() {}
فراغ parser<بیعلامت دراز>::anchor() {}
فراغ parser<بیعلامت دراز دراز>::anchor() {}
فراغ parser<مزدوج>::anchor() {}
فراغ parser<عائم>::anchor() {}
فراغ parser<std::string>::anchor() {}
فراغ parser<std::optional<std::string>>::anchor() {}
فراغ parser<حرف>::anchor() {}
فراغ parser<Element_شمار>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
فراغ opt_bool_anchor() { opt<منطقي> anchor{""}; }
فراغ opt_char_anchor() { opt<حرف> anchor{""}; }
فراغ opt_int_anchor() { opt<صحيح> anchor{""}; }
فراغ opt_unsigned_anchor() { opt<بیعلامت> anchor{""}; }

//===----------------------------------------------------------------------===//

ثابت ساكن size_t DefaultPad = 2;

ساكن StringRef Arg_پیشوند = "-";
ساكن StringRef Arg_پیشوند_Long = "--";
ساكن StringRef Arg_Help_پیشوند = " - ";

ساكن size_t arg_Plus_Prefixes_اندازه(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.اندازه_2();
  إذا (Len == 1)
    أرجع Len + Pad + Arg_پیشوند.اندازه_2() + Arg_Help_پیشوند.اندازه_2();
  أرجع Len + Pad + Arg_پیشوند_Long.اندازه_2() + Arg_Help_پیشوند.اندازه_2();
}

ساكن SmallString<8> arg_پیشوند(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> پیشوند;
  لكل (size_t I = 0; I < Pad; ++I) {
    پیشوند.افزودن_پشت(' ');
  }
  پیشوند.append(ArgName.اندازه_2() > 1 ? Arg_پیشوند_Long : Arg_پیشوند);
  أرجع پیشوند;
}

استفاده OptionsMapTy = DenseMap<StringRef, Option *>;

نطاق {

صنف PrintArg {
  StringRef ArgName;
  size_t Pad;
عام:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  دوست raw_ostream &عملگر<<(raw_ostream &OS, ثابت PrintArg &);
};

raw_ostream &عملگر<<(raw_ostream &OS, ثابت PrintArg& Arg) {
  OS << arg_پیشوند(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  أرجع OS;
}

صنف Command_سطر_Parser {
عام:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string برنامه_Name;
  StringRef برنامه_Overview;

  // This collects additional help to be printed.
  std::وکتور<StringRef> بیشتر_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_مجموعه<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_مجموعه<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_وکتور<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_شاخص;
  size_t رقم_نمایهای_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_سطر_Parser() { ثبات_Sub_Command(&SubCommand::get_بالاترین_Level()); }

  فراغ ResetAllOptionOccurrences();

  منطقي Parse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                               StringRef Overview, raw_ostream *Errs = اشاره_تهی,
                               vfs::FileSystem *VFS = اشاره_تهی,
                               LibraryOptionsContext *Context = اشاره_تهی);

  فراغ forEachSubCommand(Option &Opt, function_ref<فراغ(SubCommand &)> Action) {
    إذا (Opt.Subs.خالی()) {
      Action(SubCommand::get_بالاترین_Level());
      أرجع;
    }
    إذا (Opt.Subs.اندازه_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
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

  فراغ جمع_کن_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    إذا (Opt.hasArgStr())
      أرجع;
    إذا (!SC->OptionsMap.درج(std::make_pair(Name, &Opt)).دومی_2) {
      errs() << برنامه_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  فراغ جمع_کن_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { جمع_کن_Literal_Option(Opt, &SC, Name); });
  }

  فراغ جمع_کن_Option(Option *O, SubCommand *SC) {
    منطقي HadErrors = خطأ;
    إذا (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      إذا (!SC->OptionsMap.درج(std::make_pair(O->ArgStr, O)).دومی_2 ||
          (RegisteredSubCommands.شامل_است(SC) &&
           Library_شاخص.شامل_است(O->ArgStr))) {
        errs() << برنامه_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = درست;
      }
    }

    // Remember information about positional options.
    إذا (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.افزودن_پشت(O);
    وإلا إذا (O->get_رقم_Occurrences_Flag() == cl::ConsumeAfter) {
      إذا (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = درست;
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

  فراغ جمع_کن_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { جمع_کن_Option(O, &SC); });
  }

  فراغ removeOption(Option *O, SubCommand *SC) {
    Small_وکتور<StringRef, 16> Option_نامها;
    O->get_Extra_Option_نامها(Option_نامها);
    إذا (O->hasArgStr())
      Option_نامها.افزودن_پشت(O->ArgStr);

    SubCommand &Sub = *SC;
    لكل (خودکار Name : Option_نامها) {
      خودکار I = Sub.OptionsMap.پیدا_کن(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      إذا (I != Sub.OptionsMap.انتها() && I->دومی_2 == O)
        Sub.OptionsMap.erase(I);
    }

    إذا (O->getFormattingFlag() == cl::Positional)
      لكل (خودکار *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.انتها(); ++Opt) {
        إذا (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          اكسر;
        }
      }
    وإلا إذا (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = اشاره_تهی;
  }

  فراغ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  منطقي hasOptions(ثابت SubCommand &Sub) ثابت {
    أرجع (!Sub.OptionsMap.خالی() || !Sub.PositionalOpts.خالی() ||
            اشاره_تهی != Sub.ConsumeAfterOpt);
  }

  منطقي hasOptions() ثابت {
    لكل (ثابت خودکار *S : RegisteredSubCommands) {
      إذا (hasOptions(*S))
        أرجع درست;
    }
    أرجع !Libraries.خالی();
  }

  منطقي hasNamedSubCommands() ثابت {
    لكل (ثابت خودکار *S : RegisteredSubCommands)
      إذا (!S->getName().خالی())
        أرجع درست;
    أرجع خطأ;
  }

  SubCommand *get_فعال_Sub_Command() { أرجع فعال_Sub_Command; }

  فراغ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    إذا (!Sub.OptionsMap.درج(std::make_pair(NewName, O)).دومی_2) {
      errs() << برنامه_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  فراغ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  فراغ print_Option_مقادیر();

  فراغ شاخص_Library_Options() {
    منطقي HadErrors = خطأ;
    لكل (; رقم_نمایهای_Libraries != Libraries.اندازه_2(); ++رقم_نمایهای_Libraries) {
      LibraryOptions *L = Libraries[رقم_نمایهای_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        خودکار [It, Inserted] = Library_شاخص.try_emplace(Name, L);
        إذا (Inserted ? none_از(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 أرجع SC->OptionsMap.شامل_است(Name);
                               })
                     : It->دومی_2 == L)
          أرجع;
        errs() << برنامه_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = درست;
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

  فراغ ثبات_Category(OptionCategory *cat) {
    أكد(شمار_if(RegisteredOptionCategories,
                    [cat](ثابت OptionCategory *Category) {
             أرجع cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.درج(cat);
  }

  فراغ ثبات_Sub_Command(SubCommand *sub) {
    أكد(شمار_if(RegisteredSubCommands,
                    [sub](ثابت SubCommand *Sub) {
                      أرجع (!sub->getName().خالی()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.درج(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    أكد(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    لكل (خودکار &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.دومی_2;
      إذا (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        جمع_کن_Option(O, sub);
      وإلا
        جمع_کن_Literal_Option(*O, sub, E.اولی_2);
    }
  }

  فراغ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_مجموعه<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    أرجع make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.انتها());
  }

  فراغ reset() {
    فعال_Sub_Command = اشاره_تهی;
    برنامه_Name.clear();
    برنامه_Overview = StringRef();

    بیشتر_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_شاخص.clear();
    رقم_نمایهای_Libraries = 0;

    SubCommand::get_بالاترین_Level().reset();
    SubCommand::getAll().reset();
    ثبات_Sub_Command(&SubCommand::get_بالاترین_Level());
  }

خاص:
  SubCommand *فعال_Sub_Command = اشاره_تهی;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &مقدار);
  SubCommand *LookupSubCommand(StringRef Name, std::string &نزدیکترین_String);
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
parseLibraryOption(LibraryOptions &L, فراغ *Opts, ArrayRef<ثابت حرف *> آرگومانها,
                   بیعلامت &Consumed, BumpPtrAllocator &Alloc,
                   Small_وکتور_Impl<ثابت حرف *> *رکورد = اشاره_تهی) {
  StringSaver Saver(Alloc);
  ثابت حرف *Saved[2];
  بیعلامت رقم_آرگومانها = std::min<size_t>(آرگومانها.اندازه_2(), 2);
  لكل (بیعلامت J = 0; J != رقم_آرگومانها; ++J)
    Saved[J] = Saver.save(آرگومانها[J]).داده();
  Consumed = 1;
  إذا (Error E = L.parse(Opts, ArrayRef(Saved, رقم_آرگومانها), Consumed, Alloc))
    أرجع E;
  إذا (رکورد)
    رکورد->append(Saved, Saved + Consumed);
  أرجع Error::success();
}

قالب <نام_نوع T, T TrueVal, T FalseVal>
ساكن منطقي parseBool(Option &O, StringRef ArgName, StringRef Arg, T &مقدار) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  إذا (!Arg.داده() || Arg == "true" || Arg == "1") {
    مقدار = TrueVal;
    أرجع خطأ;
  }

  إذا (Arg == "false" || Arg == "0") {
    مقدار = FalseVal;
    أرجع خطأ;
  }
  أرجع O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

فراغ cl::جمع_کن_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().جمع_کن_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().بیشتر_Help.افزودن_پشت(Help);
}

Option::Option(رقم_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : رقم_Occurrences(0), Occurrences(OccurrencesFlag), مقدار(0),
      HiddenFlag(Hidden), Formatting(نرمال_Formatting), Misc(0),
      FullyInitialized(خطأ), Position(0) {
  Categories.افزودن_پشت(&getGeneralCategory());
}

فراغ Option::جمع_کن_آرگومان() {
  globalParser().جمع_کن_Option(هذا);
  FullyInitialized = درست;
}

فراغ Option::remove_آرگومان() { globalParser().removeOption(هذا); }

فراغ Option::مجموعه_Arg_Str(StringRef S) {
  إذا (FullyInitialized)
    globalParser().updateArgStr(هذا, S);
  أكد(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

فراغ Option::جمع_کن_Category(OptionCategory &C) {
  أكد(!Categories.خالی() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  إذا (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  وإلا إذا (!is_contained(Categories, &C))
    Categories.افزودن_پشت(&C);
}

فراغ Option::reset() {
  رقم_Occurrences = 0;
  مجموعه_Default();
}

فراغ OptionCategory::ثبات_Category() {
  globalParser().ثبات_Category(هذا);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_بالاترین_Level() {
  ساكن ManagedStatic<SubCommand> بالاترین_Level_Sub_Command;
  أرجع *بالاترین_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ساكن ManagedStatic<SubCommand> AllSubCommands;
  أرجع *AllSubCommands;
}

فراغ SubCommand::ثبات_Sub_Command() {
  globalParser().ثبات_Sub_Command(هذا);
}

فراغ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(هذا);
}

فراغ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = اشاره_تهی;
}

SubCommand::عملگر منطقي() ثابت {
  أرجع (globalParser().get_فعال_Sub_Command() == هذا);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_سطر_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &مقدار) {
  // Reject all dashes.
  إذا (Arg.خالی())
    أرجع اشاره_تهی;
  أكد(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.پیدا_کن('=');

  // If we have an equals sign, remember the value.
  إذا (EqualPos == StringRef::npos) {
    // Look up the option.
    أرجع Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  خودکار I = Sub.OptionsMap.پیدا_کن(Arg.substr(0, EqualPos));
  إذا (I == Sub.OptionsMap.انتها())
    أرجع اشاره_تهی;

  خودکار *O = I->دومی_2;
  إذا (O->getFormattingFlag() == cl::Always_پیشوند)
    أرجع اشاره_تهی;

  مقدار = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  أرجع I->دومی_2;
}

SubCommand *Command_سطر_Parser::LookupSubCommand(StringRef Name,
                                                std::string &نزدیکترین_String) {
  إذا (Name.خالی())
    أرجع &SubCommand::get_بالاترین_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *نزدیکترین_تطابق = اشاره_تهی;
  لكل (خودکار *S : RegisteredSubCommands) {
    أكد(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    إذا (S->getName().خالی())
      استمر;

    إذا (S->getName() == Name)
      أرجع S;

    إذا (!نزدیکترین_تطابق && S->getName().edit_فاصله(Name) < 2)
      نزدیکترین_تطابق = S;
  }

  إذا (نزدیکترین_تطابق)
    نزدیکترین_String = نزدیکترین_تطابق->getName();

  أرجع &SubCommand::get_بالاترین_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ساكن Option *Lookup_نزدیکترین_Option(StringRef Arg,
                                   ثابت OptionsMapTy &OptionsMap,
                                   std::string &نزدیکترین_String) {
  // Reject all dashes.
  إذا (Arg.خالی())
    أرجع اشاره_تهی;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> تقسیم_Arg = Arg.تقسیم('=');
  StringRef &LHS = تقسیم_Arg.اولی_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = تقسیم_Arg.دومی_2;

  // Find the closest match.
  Option *بهترین = اشاره_تهی;
  بیعلامت بهترین_فاصله = 0;
  لكل (ثابت خودکار &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    إذا (O->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    Small_وکتور<StringRef, 16> Option_نامها;
    O->get_Extra_Option_نامها(Option_نامها);
    إذا (O->hasArgStr())
      Option_نامها.افزودن_پشت(O->ArgStr);

    منطقي Permit_مقدار = O->get_مقدار_Expected_Flag() != cl::مقدار_Disallowed;
    StringRef Flag = Permit_مقدار ? LHS : Arg;
    لكل (ثابت خودکار &Name : Option_نامها) {
      بیعلامت فاصله_2 = StringRef(Name).edit_فاصله(
          Flag, /*AllowReplacements=*/درست, /*MaxEditDistance=*/بهترین_فاصله);
      إذا (!بهترین || فاصله_2 < بهترین_فاصله) {
        بهترین = O;
        بهترین_فاصله = فاصله_2;
        إذا (RHS.خالی() || !Permit_مقدار)
          نزدیکترین_String = std::string(Name);
        وإلا
          نزدیکترین_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  أرجع بهترین;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ساكن منطقي Comma_Separate_And_جمع_کن_Occurrence(Option *Handler, بیعلامت pos,
                                          StringRef ArgName, StringRef مقدار) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  إذا (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(مقدار);
    StringRef::اندازه_type Pos = Val.پیدا_کن(',');

    طالما (Pos != StringRef::npos) {
      // Process the portion before the comma.
      إذا (Handler->جمع_کن_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        أرجع درست;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.پیدا_کن(',');
    }

    مقدار = Val;
  }

  أرجع Handler->جمع_کن_Occurrence(pos, ArgName, مقدار);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ساكن مضمن منطقي ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef مقدار, صحيح argc,
                                 ثابت حرف *ثابت *argv, صحيح &i) {
  // Enforce value requirements
  بدل (Handler->get_مقدار_Expected_Flag()) {
  حالة مقدار_Required:
    إذا (!مقدار.داده()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      إذا (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_پیشوند)
        أرجع Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      أكد(argv && "null check");
      مقدار = StringRef(argv[++i]);
    }
    اكسر;
  حالة مقدار_Disallowed:
    إذا (مقدار.داده())
      أرجع Handler->error("does not allow a value! '" + Twine(مقدار) +
                            "' specified.");
    اكسر;
  حالة مقدار_Optional:
    اكسر;
  }

  أرجع Comma_Separate_And_جمع_کن_Occurrence(Handler, i, ArgName, مقدار);
}

منطقي llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, صحيح i) {
  صحيح Dummy = i;
  أرجع ProvideOption(Handler, Handler->ArgStr, Arg, 0, اشاره_تهی, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ساكن Option *پیدا_کن_پیشوند_Option(StringRef Name, size_t &درازی,
                                ثابت OptionsMapTy &OptionsMap) {
  لكل (; !Name.خالی(); Name = Name.drop_پشت()) {
    Option *O = OptionsMap.lookup(Name);
    إذا (O && (O->getFormattingFlag() == cl::پیشوند ||
              O->getFormattingFlag() == cl::Always_پیشوند)) {
      درازی = Name.اندازه_2();
      أرجع O;
    }
  }
  أرجع اشاره_تهی;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ساكن Option *HandlePrefixedOption(StringRef &Arg, StringRef &مقدار,
                                    ثابت OptionsMapTy &OptionsMap) {
  إذا (Arg.اندازه_2() == 1)
    أرجع اشاره_تهی;

  size_t درازی = 0;
  Option *POpt = پیدا_کن_پیشوند_Option(Arg, درازی, OptionsMap);
  إذا (!POpt)
    أرجع اشاره_تهی;

  StringRef Maybe_مقدار =
      (درازی < Arg.اندازه_2()) ? Arg.substr(درازی) : StringRef();
  Arg = Arg.substr(0, درازی);

  // cl::Prefix options do not preserve '=' when used separately.
  إذا (POpt->getFormattingFlag() == cl::پیشوند && Maybe_مقدار.starts_with("="))
    Maybe_مقدار = Maybe_مقدار.drop_front();
  مقدار = Maybe_مقدار;
  أرجع POpt;
}

ساكن منطقي Requires_مقدار(ثابت Option *O) {
  أرجع O->get_رقم_Occurrences_Flag() == cl::Required ||
         O->get_رقم_Occurrences_Flag() == cl::One_Or_بیشتر;
}

ساكن منطقي Eats_Unbounded_عدد_از_مقادیر(ثابت Option *O) {
  أرجع O->get_رقم_Occurrences_Flag() == cl::Zero_Or_بیشتر ||
         O->get_رقم_Occurrences_Flag() == cl::One_Or_بیشتر;
}

ساكن منطقي isWhitespace(حرف C) {
  أرجع C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ساكن منطقي isWhitespaceOrNull(حرف C) {
  أرجع isWhitespace(C) || C == '\0';
}

ساكن منطقي isQuote(حرف C) { أرجع C == '\"' || C == '\''; }

فراغ cl::Tokenize_GNUCommand_سطر(StringRef Src, StringSaver &Saver,
                                Small_وکتور_Impl<ثابت حرف *> &NewArgv,
                                منطقي MarkEOLs) {
  SmallString<128> Token;
  منطقي درون_Token = خطأ;
  لكل (size_t I = 0, E = Src.اندازه_2(); I != E; ++I) {
    // Consume runs of whitespace.
    إذا (!درون_Token) {
      طالما (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        إذا (MarkEOLs && Src[I] == '\n')
          NewArgv.افزودن_پشت(اشاره_تهی);
        ++I;
      }
      إذا (I == E)
        اكسر;
      درون_Token = درست;
    }

    حرف C = Src[I];

    // Backslash escapes the next character.
    إذا (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.افزودن_پشت(Src[I]);
      استمر;
    }

    // Consume a quoted string.
    إذا (isQuote(C)) {
      ++I;
      طالما (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        إذا (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.افزودن_پشت(Src[I]);
        ++I;
      }
      إذا (I == E)
        اكسر;
      استمر;
    }

    // End the token if this is whitespace.
    إذا (isWhitespace(C)) {
      NewArgv.افزودن_پشت(Saver.save(Token.str()).داده());
      // Mark the end of lines in response files.
      إذا (MarkEOLs && C == '\n')
        NewArgv.افزودن_پشت(اشاره_تهی);
      Token.clear();
      درون_Token = خطأ;
      استمر;
    }

    // This is a normal character.  Append it.
    Token.افزودن_پشت(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  إذا (درون_Token)
    NewArgv.افزودن_پشت(Saver.save(Token.str()).داده());
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
  صحيح Backslash_شمار = 0;
  // Skip the backslashes.
  افعل {
    ++I;
    ++Backslash_شمار;
  } طالما (I != E && Src[I] == '\\');

  منطقي FollowedByDoubleQuote = (I != E && Src[I] == '"');
  إذا (FollowedByDoubleQuote) {
    Token.append(Backslash_شمار / 2, '\\');
    إذا (Backslash_شمار % 2 == 0)
      أرجع I - 1;
    Token.افزودن_پشت('"');
    أرجع I;
  }
  Token.append(Backslash_شمار, '\\');
  أرجع I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ساكن منطقي isWindowsSpecialChar(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ساكن منطقي is_Windows_Special_Char_درون_Command_Name(حرف C) {
  أرجع isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ساكن مضمن فراغ tokenize_Windows_Command_سطر_Impl(
    StringRef Src, StringSaver &Saver, function_ref<فراغ(StringRef)> جمع_کن_Token,
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
      أكد(Token.خالی() && "token should be empty in initial state");
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
        طالما (I < E && !is_Windows_Special_Char_درون_Command_Name(Src[I]))
          ++I;
      } وإلا {
        طالما (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef نرمال_Chars = Src.برش(شروع, I);
      إذا (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        جمع_کن_Token(AlwaysCopy ? Saver.save(نرمال_Chars) : نرمال_Chars);
        إذا (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } وإلا {
          CommandName = خطأ;
        }
      } وإلا إذا (Src[I] == '\"') {
        Token += نرمال_Chars;
        State = QUOTED;
      } وإلا إذا (Src[I] == '\\') {
        أكد(!CommandName && "or else we'd have treated it as a normal char");
        Token += نرمال_Chars;
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
        جمع_کن_Token(Saver.save(Token.str()));
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
        Token.افزودن_پشت(Src[I]);
      }
      اكسر;

    حالة QUOTED:
      إذا (Src[I] == '\"') {
        إذا (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.افزودن_پشت('"');
          ++I;
        } وإلا {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } وإلا إذا (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } وإلا {
        Token.افزودن_پشت(Src[I]);
      }
      اكسر;
    }
  }

  إذا (State != INIT)
    جمع_کن_Token(Saver.save(Token.str()));
}

فراغ cl::Tokenize_Windows_Command_سطر(StringRef Src, StringSaver &Saver,
                                    Small_وکتور_Impl<ثابت حرف *> &NewArgv,
                                    منطقي MarkEOLs) {
  خودکار جمع_کن_Token = [&](StringRef Tok) { NewArgv.افزودن_پشت(Tok.داده()); };
  خودکار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.افزودن_پشت(اشاره_تهی);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جمع_کن_Token,
                                 /*AlwaysCopy=*/درست, OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سطر_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_وکتور_Impl<StringRef> &NewArgv) {
  خودکار جمع_کن_Token = [&](StringRef Tok) { NewArgv.افزودن_پشت(Tok); };
  خودکار OnEOL = []() {};
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جمع_کن_Token, /*AlwaysCopy=*/خطأ,
                                 OnEOL, خطأ);
}

فراغ cl::Tokenize_Windows_Command_سطر_پر(StringRef Src, StringSaver &Saver,
                                        Small_وکتور_Impl<ثابت حرف *> &NewArgv,
                                        منطقي MarkEOLs) {
  خودکار جمع_کن_Token = [&](StringRef Tok) { NewArgv.افزودن_پشت(Tok.داده()); };
  خودکار OnEOL = [&]() {
    إذا (MarkEOLs)
      NewArgv.افزودن_پشت(اشاره_تهی);
  };
  tokenize_Windows_Command_سطر_Impl(Src, Saver, جمع_کن_Token,
                                 /*AlwaysCopy=*/درست, OnEOL, درست);
}

فراغ cl::tokenizeConfigFile(StringRef منبع, StringSaver &Saver,
                            Small_وکتور_Impl<ثابت حرف *> &NewArgv,
                            منطقي MarkEOLs) {
  لكل (ثابت حرف *Cur = منبع.begin(); Cur != منبع.انتها();) {
    SmallString<128> سطر;
    // Check for comment line.
    إذا (isWhitespace(*Cur)) {
      طالما (Cur != منبع.انتها() && isWhitespace(*Cur))
        ++Cur;
      استمر;
    }
    إذا (*Cur == '#') {
      طالما (Cur != منبع.انتها() && *Cur != '\n')
        ++Cur;
      استمر;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ثابت حرف *شروع = Cur;
    ثابت حرف *سطر_انتها = اشاره_تهی;
    حرف Quote = 0;
    منطقي At_Token_شروع = درست;
    لكل (ثابت حرف *انتها_2 = منبع.انتها(); Cur != انتها_2; ++Cur) {
      حرف C = *Cur;
      إذا (C == '\\') {
        إذا (Cur + 1 != انتها_2) {
          ++Cur;
          إذا (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != انتها_2) && Cur[1] == '\n')) {
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
        سطر_انتها = Cur;
        طالما (Cur != انتها_2 && *Cur != '\n')
          ++Cur;
        اكسر;
      }
      At_Token_شروع = isWhitespace(C);
    }
    // Tokenize line.
    سطر.append(شروع, سطر_انتها ? سطر_انتها : Cur);
    cl::Tokenize_GNUCommand_سطر(سطر, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ساكن منطقي has_UTF8_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.اندازه_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ساكن فراغ Expand_پایه_Paths(StringRef پایه_مسیر, StringSaver &Saver,
                            ثابت حرف *&Arg) {
  أكد(sys::مسیر_2::is_absolute(پایه_مسیر));
  constexpr StringLiteral Token("<CFGDIR>");
  ثابت StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::اندازه_type شروع_Pos = 0;
  لكل (StringRef::اندازه_type TokenPos = ArgString.پیدا_کن(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.پیدا_کن(Token, شروع_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ثابت StringRef LHS = ArgString.substr(شروع_Pos, TokenPos - شروع_Pos);
    إذا (ResponseFile.خالی())
      ResponseFile = LHS;
    وإلا
      llvm::sys::مسیر_2::append(ResponseFile, LHS);
    ResponseFile.append(پایه_مسیر);
    شروع_Pos = TokenPos + Token.اندازه_2();
  }

  إذا (!ResponseFile.خالی()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ثابت StringRef مانده = ArgString.substr(شروع_Pos);
    إذا (!مانده.خالی())
      llvm::sys::مسیر_2::append(ResponseFile, مانده);
    Arg = Saver.save(ResponseFile.str()).داده();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_وکتور_Impl<ثابت حرف *> &NewArgv) {
  أكد(sys::مسیر_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<حافظه_میانگیر>> MemBufOrErr =
      FS->get_میانگیر_For_File(FName);
  إذا (!MemBufOrErr) {
    std::error_کد EC = MemBufOrErr.getError();
    أرجع llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.پیام_2());
  }
  حافظه_میانگیر &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_میانگیر_شروع(), MemBuf.get_میانگیر_اندازه());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<حرف> BufRef(MemBuf.get_میانگیر_شروع(), MemBuf.get_میانگیر_انتها());
  std::string UTF8Buf;
  إذا (has_UTF16_Byte_ترتیب_Mark(BufRef)) {
    إذا (!convert_UTF16_مقصد_UTF8_String(BufRef, UTF8Buf))
      أرجع llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  وإلا إذا (has_UTF8_Byte_ترتیب_Mark(BufRef))
    Str = StringRef(BufRef.داده() + 3, BufRef.اندازه_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  إذا (!Relative_نامها && !درون_Config_File)
    أرجع Error::success();

  StringRef پایه_مسیر = llvm::sys::مسیر_2::والد_مسیر(FName);
  لكل (ثابت حرف *&Arg : NewArgv) {
    إذا (!Arg)
      استمر;

    // Substitute <CFGDIR> with the file's base path.
    إذا (درون_Config_File)
      Expand_پایه_Paths(پایه_مسیر, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    منطقي ConfigInclusion = خطأ;
    إذا (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      إذا (!llvm::sys::مسیر_2::is_relative(FileName))
        استمر;
    } وإلا إذا (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = درست;
    } وإلا {
      استمر;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.افزودن_پشت('@');
    إذا (ConfigInclusion && !llvm::sys::مسیر_2::has_والد_مسیر(FileName)) {
      SmallString<128> File_مسیر;
      إذا (!پیدا_کن_Config_File(FileName, File_مسیر))
        أرجع createStringError(
            std::make_error_کد(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_مسیر);
    } وإلا {
      ResponseFile.append(پایه_مسیر);
      llvm::sys::مسیر_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).داده();
  }
  أرجع Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_وکتور_Impl<ثابت حرف *> &Argv) {
  بنية Response_File_رکورد {
    std::string File;
    size_t انتها_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_وکتور<Response_File_رکورد, 3> File_پشته;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_پشته.افزودن_پشت({"", Argv.اندازه_2()});

  // Don't cache Argv.size() because it can change.
  لكل (بیعلامت I = 0; I != Argv.اندازه_2();) {
    طالما (I == File_پشته.پشت().انتها_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_پشته.برداشت_پشت();
    }

    ثابت حرف *Arg = Argv[I];
    // Check if it is an EOL marker
    إذا (Arg == اشاره_تهی) {
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
    إذا (llvm::sys::مسیر_2::is_relative(FName)) {
      إذا (CurrentDir.خالی()) {
        إذا (خودکار CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } وإلا {
          أرجع createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } وإلا {
        CurrDir = CurrentDir;
      }
      llvm::sys::مسیر_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::وضعیت> Res = FS->وضعیت_2(FName);
    إذا (!Res || !Res->exists()) {
      std::error_کد EC = Res.getError();
      إذا (!درون_Config_File) {
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
                                       "': " + EC.پیام_2());
    }
    ثابت llvm::vfs::وضعیت &File_وضعیت = Res.get();

    خودکار IsEquivalent =
        [File_وضعیت, هذا](ثابت Response_File_رکورد &RFile) -> ErrorOr<منطقي> {
      ErrorOr<llvm::vfs::وضعیت> RHS = FS->وضعیت_2(RFile.File);
      إذا (!RHS)
        أرجع RHS.getError();
      أرجع File_وضعیت.equivalent(*RHS);
    };

    // Check for recursive response files.
    لكل (ثابت خودکار &F : drop_begin(File_پشته)) {
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
    Small_وکتور<ثابت حرف *, 0> ExpandedArgv;
    إذا (Error Err = expandResponseFile(FName, ExpandedArgv))
      أرجع Err;

    لكل (Response_File_رکورد &رکورد : File_پشته) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      رکورد.انتها_2 += ExpandedArgv.اندازه_2() - 1;
    }

    File_پشته.افزودن_پشت({FName, I + ExpandedArgv.اندازه_2()});
    Argv.erase(Argv.begin() + I);
    Argv.درج(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.انتها());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  أكد(File_پشته.اندازه_2() > 0 && Argv.اندازه_2() == File_پشته.پشت().انتها_2);
  أرجع Error::success();
}

منطقي cl::expandResponseFiles(صحيح Argc, ثابت حرف *ثابت *Argv,
                             ثابت حرف *EnvVar, StringSaver &Saver,
                             Small_وکتور_Impl<ثابت حرف *> &NewArgv) {
#اگر_تعریف_شده _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#وإلا
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#پایان_اگر
  // The environment variable specifies initial options.
  إذا (EnvVar)
    إذا (std::optional<std::string> Env_مقدار = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_مقدار, Saver, NewArgv, /*MarkEOLs=*/خطأ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  إذا (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << مقصد_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع درست;
}

منطقي cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_وکتور_Impl<ثابت حرف *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  إذا (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << مقصد_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  أرجع درست;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

منطقي ExpansionContext::پیدا_کن_Config_File(StringRef FileName,
                                      Small_وکتور_Impl<حرف> &File_مسیر) {
  SmallString<128> Cfg_File_مسیر;
  ثابت خودکار FileExists = [هذا](SmallString<128> مسیر) -> منطقي {
    خودکار وضعیت = FS->وضعیت_2(مسیر);
    أرجع وضعیت &&
           وضعیت->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  إذا (llvm::sys::مسیر_2::has_والد_مسیر(FileName)) {
    Cfg_File_مسیر = FileName;
    إذا (llvm::sys::مسیر_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_مسیر))
      أرجع خطأ;
    إذا (!FileExists(Cfg_File_مسیر))
      أرجع خطأ;
    File_مسیر.تخصیص_بده(Cfg_File_مسیر.begin(), Cfg_File_مسیر.انتها());
    أرجع درست;
  }

  // Look for the file in search directories.
  لكل (ثابت StringRef &Dir : جستجو_Dirs) {
    إذا (Dir.خالی())
      استمر;
    Cfg_File_مسیر.تخصیص_بده(Dir);
    llvm::sys::مسیر_2::append(Cfg_File_مسیر, FileName);
    llvm::sys::مسیر_2::native(Cfg_File_مسیر);
    إذا (FileExists(Cfg_File_مسیر)) {
      File_مسیر.تخصیص_بده(Cfg_File_مسیر.begin(), Cfg_File_مسیر.انتها());
      أرجع درست;
    }
  }

  أرجع خطأ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_وکتور_Impl<ثابت حرف *> &Argv) {
  SmallString<128> Abs_مسیر;
  إذا (sys::مسیر_2::is_relative(CfgFile)) {
    Abs_مسیر.تخصیص_بده(CfgFile);
    إذا (std::error_کد EC = FS->makeAbsolute(Abs_مسیر))
      أرجع make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_مسیر.str();
  }
  درون_Config_File = درست;
  Relative_نامها = درست;
  إذا (Error Err = expandResponseFile(CfgFile, Argv))
    أرجع Err;
  أرجع expandResponseFiles(Argv);
}

ساكن فراغ initCommonOptions();
ساكن منطقي parse_Command_سطر_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ثابت حرف *> آرگومانها,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ثابت حرف *EnvVar) {
  initCommonOptions();
  Small_وکتور<ثابت حرف *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.افزودن_پشت(آرگومانها[0]);

  // Parse options from environment variable.
  إذا (EnvVar) {
    إذا (std::optional<std::string> Env_مقدار =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_سطر(*Env_مقدار, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, آرگومانها.drop_front());
  صحيح NewArgc = static_cast<صحيح>(NewArgv.اندازه_2());

  // Parse all options.
  أرجع globalParser().Parse_Command_سطر_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

منطقي cl::Parse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سطر_Options_Impl(اشاره_تهی, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

منطقي cl::parse_Command_سطر_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ثابت حرف *> آرگومانها,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ثابت حرف *EnvVar) {
  أرجع parse_Command_سطر_Options_Impl(&Context, آرگومانها, Overview, Errs, VFS,
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
    Small_وکتور<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.اندازه_2());
    لكل (خودکار &O : SC->OptionsMap)
      Opts.افزودن_پشت(O.دومی_2);
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

  برنامه_Overview = Overview;
  منطقي IgnoreErrors = Errs;
  إذا (!Errs)
    Errs = &errs();
  إذا (!VFS)
    VFS = vfs::getRealFileSystem().get();
  منطقي ErrorParsing = خطأ;

  // Expand response files.
  Small_وکتور<ثابت حرف *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#اگر_تعریف_شده _WIN32
  خودکار Tokenize = cl::Tokenize_Windows_Command_سطر;
#وإلا
  خودکار Tokenize = cl::Tokenize_GNUCommand_سطر;
#پایان_اگر
  ExpansionContext ECtx(A, Tokenize, VFS);
  إذا (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << مقصد_String(std::move(Err)) << '\n';
    أرجع خطأ;
  }
  argv = &newArgv[0];
  argc = static_cast<صحيح>(newArgv.اندازه_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  برنامه_Name = std::string(sys::مسیر_2::filename(StringRef(argv[0])));
  شاخص_Library_Options();

  // Check out the positional arguments to collect information about them.
  بیعلامت رقم_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  منطقي HasUnlimitedPositionals = خطأ;

  صحيح اولی_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_بالاترین_Level();
  std::string نزدیکترین_Sub_Command_String;
  منطقي MaybeNamedSubCommand =
      argc >= 2 && argv[اولی_Arg][0] != '-' && hasNamedSubCommands();
  إذا (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[اولی_Arg]), نزدیکترین_Sub_Command_String);
    إذا (ChosenSubCommand != &SubCommand::get_بالاترین_Level())
      اولی_Arg = 2;
  }
  globalParser().فعال_Sub_Command = ChosenSubCommand;

  أكد(ChosenSubCommand);
  خودکار &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  خودکار &PositionalOpts = ChosenSubCommand->PositionalOpts;
  خودکار &OptionsMap = ChosenSubCommand->OptionsMap;

  إذا (ConsumeAfterOpt) {
    أكد(PositionalOpts.اندازه_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  إذا (!PositionalOpts.خالی()) {

    // Calculate how many positional values are _required_.
    منطقي Unbounded_یافت = خطأ;
    لكل (size_t i = 0, e = PositionalOpts.اندازه_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      إذا (Requires_مقدار(Opt))
        ++رقم_Positional_Required;
      وإلا إذا (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        إذا (PositionalOpts.اندازه_2() > 1) {
          إذا (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = درست;
        }
      } وإلا إذا (Unbounded_یافت && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        إذا (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << برنامه_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.اندازه_2();
        ErrorParsing = درست;
      }
      Unbounded_یافت |= Eats_Unbounded_عدد_از_مقادیر(Opt);
    }
    HasUnlimitedPositionals = Unbounded_یافت || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_وکتور<std::pair<StringRef, بیعلامت>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *فعال_Positional_Arg = اشاره_تهی;

  // Loop over all of the arguments... processing them.
  منطقي Dash_Dash_یافت = خطأ; // Have we read '--'?
  لكل (صحيح i = اولی_Arg; i < argc; ++i) {
    Option *Handler = اشاره_تهی;
    std::string نزدیکترین_Handler_String;
    StringRef مقدار;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    إذا (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_یافت) {
      // Positional argument!
      إذا (فعال_Positional_Arg) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }

      إذا (!PositionalOpts.خالی()) {
        PositionalVals.افزودن_پشت(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        إذا (PositionalVals.اندازه_2() >= رقم_Positional_Required && ConsumeAfterOpt) {
          لكل (++i; i < argc; ++i)
            PositionalVals.افزودن_پشت(std::make_pair(StringRef(argv[i]), i));
          اكسر; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        استمر;
      }
    } وإلا إذا (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_یافت) {
      Dash_Dash_یافت = درست; // This is the mythical "--"?
      استمر;             // Don't try to process it as an argument itself.
    } وإلا إذا (فعال_Positional_Arg &&
               (فعال_Positional_Arg->getMiscFlags() & Positional_Eats_آرگومانها)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, مقدار);
      إذا (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(فعال_Positional_Arg, StringRef(argv[i]), i);
        استمر; // We are done!
      }
    } وإلا { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, مقدار);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      إذا (!Handler && ChosenSubCommand != &SubCommand::get_بالاترین_Level())
        Handler = LookupOption(SubCommand::get_بالاترین_Level(), ArgName, مقدار);

      إذا (!Handler) {
        إذا (LibraryOptions *L = lookupLibraryOption(ArgName.تقسیم('=').اولی_2)) {
          ArrayRef<ثابت حرف *> Rest(argv + i, argc - i);
          بیعلامت N;
          إذا (Error E = Context && L->is_در_هر_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << برنامه_Name << ": " << مقصد_String(std::move(E)) << '\n';
            ErrorParsing = درست;
          }
          i += N - 1;
          استمر;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      إذا (!Handler)
        Handler = HandlePrefixedOption(ArgName, مقدار, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      إذا (!Handler)
        Lookup_نزدیکترین_Option(ArgName, OptionsMap, نزدیکترین_Handler_String);
    }

    إذا (!Handler) {
      خودکار Report_Unknown_آرگومان = [&](منطقي IsArg,
                                       StringRef نزدیکترین_آرگومان_Name) {
        *Errs << برنامه_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        إذا (نزدیکترین_آرگومان_Name.خالی())
          أرجع;

        *Errs << برنامه_Name << ": Did you mean '";
        إذا (IsArg)
          *Errs << PrintArg(نزدیکترین_آرگومان_Name, 0);
        وإلا
          *Errs << نزدیکترین_آرگومان_Name;
        *Errs << "'?\n";
      };

      إذا (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_آرگومان(/*IsArg=*/درست, نزدیکترین_Handler_String);
      وإلا
        Report_Unknown_آرگومان(/*IsArg=*/خطأ, نزدیکترین_Sub_Command_String);

      ErrorParsing = درست;
      استمر;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    إذا (Handler->getFormattingFlag() == cl::Positional) {
      إذا ((Handler->getMiscFlags() & Positional_Eats_آرگومانها) && !مقدار.خالی()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = درست;
      }
      فعال_Positional_Arg = Handler;
    }
    وإلا
      ErrorParsing |= ProvideOption(Handler, ArgName, مقدار, argc, argv, i);
  }

  // Check and handle positional arguments now...
  إذا (رقم_Positional_Required > PositionalVals.اندازه_2()) {
      *Errs << برنامه_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << رقم_Positional_Required
             << " positional argument" << (رقم_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = درست;
  } وإلا إذا (!HasUnlimitedPositionals &&
             PositionalVals.اندازه_2() > PositionalOpts.اندازه_2()) {
    *Errs << برنامه_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.اندازه_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = درست;

  } وإلا إذا (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    بیعلامت ValNo = 0, رقم_Vals = static_cast<بیعلامت>(PositionalVals.اندازه_2());
    لكل (Option *Opt : PositionalOpts) {
      إذا (Requires_مقدار(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].اولی_2,
                                PositionalVals[ValNo].دومی_2);
        ValNo++;
        --رقم_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      منطقي انجامشده = Opt->get_رقم_Occurrences_Flag() == cl::Required;
      طالما (رقم_Vals - ValNo > رقم_Positional_Required && !انجامشده) {
        بدل (Opt->get_رقم_Occurrences_Flag()) {
        حالة cl::Optional:
          انجامشده = درست; // Optional arguments want _at most_ one value
          [[fallthrough]];
        حالة cl::Zero_Or_بیشتر: // Zero or more will take all they can get...
        حالة cl::One_Or_بیشتر:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].اولی_2,
                                  PositionalVals[ValNo].دومی_2);
          ValNo++;
          اكسر;
        افتراضي:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } وإلا {
    أكد(ConsumeAfterOpt && رقم_Positional_Required <= PositionalVals.اندازه_2());
    بیعلامت ValNo = 0;
    لكل (Option *Opt : PositionalOpts)
      إذا (Requires_مقدار(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].اولی_2, PositionalVals[ValNo].دومی_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    إذا (PositionalOpts.اندازه_2() == 1 && ValNo == 0 && !PositionalVals.خالی()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].اولی_2,
                                              PositionalVals[ValNo].دومی_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    لكل (; ValNo != PositionalVals.اندازه_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].اولی_2,
                                  PositionalVals[ValNo].دومی_2);
  }

  // Loop over args and make sure all required args are specified!
  لكل (ثابت خودکار &Opt : OptionsMap) {
    بدل (Opt.دومی_2->get_رقم_Occurrences_Flag()) {
    حالة Required:
    حالة One_Or_بیشتر:
      إذا (Opt.دومی_2->get_رقم_Occurrences() == 0) {
        Opt.دومی_2->error("must be specified at least once!");
        ErrorParsing = درست;
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
  بیشتر_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  إذا (ErrorParsing) {
    إذا (!IgnoreErrors)
      خروج(1);
    أرجع خطأ;
  }
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

منطقي Option::error(ثابت Twine &پیام_3, StringRef ArgName, raw_ostream &Errs) {
  إذا (!ArgName.داده())
    ArgName = ArgStr;
  إذا (ArgName.خالی())
    Errs << HelpStr; // Be nice for positional arguments
  وإلا
    Errs << globalParser().برنامه_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << پیام_3 << "\n";
  أرجع درست;
}

منطقي Option::جمع_کن_Occurrence(بیعلامت pos, StringRef ArgName, StringRef مقدار) {
  ++رقم_Occurrences;
  أرجع handleOccurrence(pos, ArgName, مقدار);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ساكن StringRef get_مقدار_Str(ثابت Option &O, StringRef Default_پیام) {
  إذا (O.مقدار_Str.خالی())
    أرجع Default_پیام;
  أرجع O.مقدار_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_عرض() ثابت {
  أرجع arg_Plus_Prefixes_اندازه(ArgStr);
}

فراغ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t اولی_سطر_Indented_By) {
  أكد(Indent >= اولی_سطر_Indented_By);
  std::pair<StringRef, StringRef> تقسیم_2 = HelpStr.تقسیم('\n');
  outs().indent(Indent - اولی_سطر_Indented_By)
      << Arg_Help_پیشوند << تقسیم_2.اولی_2 << "\n";
  طالما (!تقسیم_2.دومی_2.خالی()) {
    تقسیم_2 = تقسیم_2.دومی_2.تقسیم('\n');
    outs().indent(Indent) << تقسیم_2.اولی_2 << "\n";
  }
}

فراغ Option::printEnumValHelpStr(StringRef HelpStr, size_t پایه_Indent,
                                 size_t اولی_سطر_Indented_By) {
  ثابت StringRef Val_Help_پیشوند = "  ";
  أكد(پایه_Indent >= اولی_سطر_Indented_By);
  std::pair<StringRef, StringRef> تقسیم_2 = HelpStr.تقسیم('\n');
  outs().indent(پایه_Indent - اولی_سطر_Indented_By)
      << Arg_Help_پیشوند << Val_Help_پیشوند << تقسیم_2.اولی_2 << "\n";
  طالما (!تقسیم_2.دومی_2.خالی()) {
    تقسیم_2 = تقسیم_2.دومی_2.تقسیم('\n');
    outs().indent(پایه_Indent + Val_Help_پیشوند.اندازه_2()) << تقسیم_2.اولی_2 << "\n";
  }
}

// Print out the option for the alias.
فراغ alias::printOptionInfo(size_t Global_عرض) ثابت {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_عرض, arg_Plus_Prefixes_اندازه(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_عرض(ثابت Option &O) ثابت {
  size_t Len = arg_Plus_Prefixes_اندازه(O.ArgStr);
  خودکار ValName = get_مقدار_Name();
  إذا (!ValName.خالی()) {
    size_t FormattingLen = 3;
    إذا (O.getMiscFlags() & Positional_Eats_آرگومانها)
      FormattingLen = 6;
    Len += get_مقدار_Str(O, ValName).اندازه_2() + FormattingLen;
  }

  أرجع Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ basic_parser_impl::printOptionInfo(ثابت Option &O,
                                        size_t Global_عرض) ثابت {
  outs() << PrintArg(O.ArgStr);

  خودکار ValName = get_مقدار_Name();
  إذا (!ValName.خالی()) {
    إذا (O.getMiscFlags() & Positional_Eats_آرگومانها) {
      outs() << " <" << get_مقدار_Str(O, ValName) << ">...";
    } وإلا إذا (O.get_مقدار_Expected_Flag() == مقدار_Optional)
      outs() << "[=<" << get_مقدار_Str(O, ValName) << ">]";
    وإلا {
      outs() << (O.ArgStr.اندازه_2() == 1 ? " <" : "=<") << get_مقدار_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_عرض, get_Option_عرض(O));
}

فراغ basic_parser_impl::printOptionName(ثابت Option &O,
                                        size_t Global_عرض) ثابت {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_عرض - O.ArgStr.اندازه_2());
}

// parser<bool> implementation
//
منطقي parser<منطقي>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         منطقي &مقدار) {
  أرجع parseBool<منطقي, درست, خطأ>(O, ArgName, Arg, مقدار);
}

// parser<boolOrDefault> implementation
//
منطقي parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &مقدار) {
  أرجع parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, مقدار);
}

// parser<FixedOrScalableQuantity> implementation
//
قالب <نام_نوع FixedOrScalableQuantityT>
ساكن منطقي parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef مقدار_نوع,
                                         FixedOrScalableQuantityT &مقدار) {
  استفاده ScalarTy = نام_نوع FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_مقدار;
  إذا (!Arg.getAsInteger(0, Min_مقدار)) {
    مقدار = FixedOrScalableQuantityT::getFixed(Min_مقدار);
    أرجع خطأ;
  }

  StringRef Remainder = Arg;
  إذا (!Remainder.consume_front("vscale"))
    أرجع O.error("'" + Arg + "' value invalid for " + مقدار_نوع +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (!Remainder.consume_front('x'))
    أرجع O.error("'" + Arg + "' value invalid for " + مقدار_نوع +
                   " argument!");

  Remainder = Remainder.ltrim();
  إذا (Remainder.getAsInteger(0, Min_مقدار))
    أرجع O.error("'" + Arg + "' value invalid for " + مقدار_نوع +
                   " argument!");

  مقدار = FixedOrScalableQuantityT::getScalable(Min_مقدار);
  أرجع خطأ;
}

// parser<int> implementation
//
منطقي parser<صحيح>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        صحيح &مقدار) {
  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for integer argument!");
  أرجع خطأ;
}

// parser<long> implementation
//
منطقي parser<دراز>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         دراز &مقدار) {
  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for long argument!");
  أرجع خطأ;
}

// parser<long long> implementation
//
منطقي parser<دراز دراز>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              دراز دراز &مقدار) {
  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for llong argument!");
  أرجع خطأ;
}

// parser<unsigned> implementation
//
منطقي parser<بیعلامت>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             بیعلامت &مقدار) {

  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for uint argument!");
  أرجع خطأ;
}

// parser<unsigned long> implementation
//
منطقي parser<بیعلامت دراز>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  بیعلامت دراز &مقدار) {

  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for ulong argument!");
  أرجع خطأ;
}

// parser<unsigned long long> implementation
//
منطقي parser<بیعلامت دراز دراز>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       بیعلامت دراز دراز &مقدار) {

  إذا (Arg.getAsInteger(0, مقدار))
    أرجع O.error("'" + Arg + "' value invalid for ullong argument!");
  أرجع خطأ;
}

// parser<ElementCount> implementation
//
منطقي parser<Element_شمار>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_شمار &مقدار) {
  أرجع parseFixedOrScalableQuantity(O, Arg, get_مقدار_Name(), مقدار);
}

// parser<double>/parser<float> implementation
//
ساكن منطقي parseDouble(Option &O, StringRef Arg, مزدوج &مقدار) {
  إذا (مقصد_float(Arg, مقدار))
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
    أرجع درست;
  Val = (عائم)dVal;
  أرجع خطأ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
بیعلامت generic_parser_پایه::پیدا_کن_Option(StringRef Name) {
  بیعلامت e = get_رقم_Options();

  لكل (بیعلامت i = 0; i != e; ++i) {
    إذا (getOption(i) == Name)
      أرجع i;
  }
  أرجع e;
}

ساكن StringRef Eq_مقدار = "=<value>";
ساكن StringRef خالی_Option = "<empty>";
ساكن StringRef Option_پیشوند = "    =";
ساكن size_t get_Option_Prefixes_اندازه() {
  أرجع Option_پیشوند.اندازه_2() + Arg_Help_پیشوند.اندازه_2();
}

ساكن منطقي shouldPrintOption(StringRef Name, StringRef Description,
                              ثابت Option &O) {
  أرجع O.get_مقدار_Expected_Flag() != مقدار_Optional || !Name.خالی() ||
         !Description.خالی();
}

// Return the width of the option tag for printing...
size_t generic_parser_پایه::get_Option_عرض(ثابت Option &O) ثابت {
  إذا (O.hasArgStr()) {
    size_t اندازه =
        arg_Plus_Prefixes_اندازه(O.ArgStr) + Eq_مقدار.اندازه_2();
    لكل (بیعلامت i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      إذا (!shouldPrintOption(Name, getDescription(i), O))
        استمر;
      size_t Name_اندازه = Name.خالی() ? خالی_Option.اندازه_2() : Name.اندازه_2();
      اندازه = std::max(اندازه, Name_اندازه + get_Option_Prefixes_اندازه());
    }
    أرجع اندازه;
  } وإلا {
    size_t پایه_اندازه = 0;
    لكل (بیعلامت i = 0, e = get_رقم_Options(); i != e; ++i)
      پایه_اندازه = std::max(پایه_اندازه, getOption(i).اندازه_2() + 8);
    أرجع پایه_اندازه;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
فراغ generic_parser_پایه::printOptionInfo(ثابت Option &O,
                                          size_t Global_عرض) ثابت {
  إذا (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    إذا (O.get_مقدار_Expected_Flag() == مقدار_Optional) {
      لكل (بیعلامت i = 0, e = get_رقم_Options(); i != e; ++i) {
        إذا (getOption(i).خالی()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_عرض,
                               arg_Plus_Prefixes_اندازه(O.ArgStr));
          اكسر;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_مقدار;
    Option::printHelpStr(O.HelpStr, Global_عرض,
                         Eq_مقدار.اندازه_2() +
                             arg_Plus_Prefixes_اندازه(O.ArgStr));
    لكل (بیعلامت i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      إذا (!shouldPrintOption(OptionName, Description, O))
        استمر;
      size_t اولی_سطر_Indent = OptionName.اندازه_2() + get_Option_Prefixes_اندازه();
      outs() << Option_پیشوند << OptionName;
      إذا (OptionName.خالی()) {
        outs() << خالی_Option;
        أكد(اولی_سطر_Indent >= خالی_Option.اندازه_2());
        اولی_سطر_Indent += خالی_Option.اندازه_2();
      }
      إذا (!Description.خالی())
        Option::printEnumValHelpStr(Description, Global_عرض, اولی_سطر_Indent);
      وإلا
        outs() << '\n';
    }
  } وإلا {
    إذا (!O.HelpStr.خالی())
      outs() << "  " << O.HelpStr << '\n';
    لكل (بیعلامت i = 0, e = get_رقم_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_عرض, Option.اندازه_2() + 8);
    }
  }
}

ساكن ثابت size_t Max_Opt_عرض = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
فراغ generic_parser_پایه::printGenericOptionDiff(
    ثابت Option &O, ثابت Generic_Option_مقدار &مقدار,
    ثابت Generic_Option_مقدار &Default, size_t Global_عرض) ثابت {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_عرض - O.ArgStr.اندازه_2());

  بیعلامت رقم_Opts = get_رقم_Options();
  لكل (بیعلامت i = 0; i != رقم_Opts; ++i) {
    إذا (!مقدار.compare(get_Option_مقدار(i)))
      استمر;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).اندازه_2();
    size_t رقم_Spaces = Max_Opt_عرض > L ? Max_Opt_عرض - L : 0;
    outs().indent(رقم_Spaces) << " (default: ";
    لكل (بیعلامت j = 0; j != رقم_Opts; ++j) {
      إذا (!Default.compare(get_Option_مقدار(j)))
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
ساكن raw_ostream &عملگر<<(raw_ostream &OS, boolOrDefault V) {
  أرجع OS << static_cast<صحيح>(V);
}
} // namespace cl
} // namespace llvm

#تعریف_کن PRINT_OPT_DIFF(T)                                                      \
  فراغ parser<T>::printOptionDiff(ثابت Option &O, T V, Option_مقدار<T> D,      \
                                  size_t Global_عرض) ثابت {                  \
    printOptionName(O, Global_عرض);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t رقم_Spaces =                                                         \
        Max_Opt_عرض > Str.اندازه_2() ? Max_Opt_عرض - Str.اندازه_2() : 0;               \
    outs().indent(رقم_Spaces) << " (default: ";                                 \
    إذا (D.has_مقدار())                                                          \
      outs() << D.get_مقدار();                                                  \
    وإلا                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(منطقي)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(صحيح)
PRINT_OPT_DIFF(دراز)
PRINT_OPT_DIFF(دراز دراز)
PRINT_OPT_DIFF(بیعلامت)
PRINT_OPT_DIFF(بیعلامت دراز)
PRINT_OPT_DIFF(بیعلامت دراز دراز)
PRINT_OPT_DIFF(مزدوج)
PRINT_OPT_DIFF(عائم)
PRINT_OPT_DIFF(حرف)
PRINT_OPT_DIFF(Element_شمار)

فراغ parser<std::string>::printOptionDiff(ثابت Option &O, StringRef V,
                                          ثابت Option_مقدار<std::string> &D,
                                          size_t Global_عرض) ثابت {
  printOptionName(O, Global_عرض);
  outs() << "= " << V;
  size_t رقم_Spaces = Max_Opt_عرض > V.اندازه_2() ? Max_Opt_عرض - V.اندازه_2() : 0;
  outs().indent(رقم_Spaces) << " (default: ";
  إذا (D.has_مقدار())
    outs() << D.get_مقدار();
  وإلا
    outs() << "*no default*";
  outs() << ")\n";
}

فراغ parser<std::optional<std::string>>::printOptionDiff(
    ثابت Option &O, std::optional<StringRef> V,
    ثابت Option_مقدار<std::optional<std::string>> &D,
    size_t Global_عرض) ثابت {
  printOptionName(O, Global_عرض);
  outs() << "= " << V;
  size_t VSize = V.has_مقدار_2() ? V.مقدار_2().اندازه_2() : 0;
  size_t رقم_Spaces = Max_Opt_عرض > VSize ? Max_Opt_عرض - VSize : 0;
  outs().indent(رقم_Spaces) << " (default: ";
  إذا (D.has_مقدار() && D.get_مقدار().has_مقدار_2())
    outs() << D.get_مقدار();
  وإلا
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
فراغ basic_parser_impl::print_Option_No_مقدار(ثابت Option &O,
                                           size_t Global_عرض) ثابت {
  printOptionName(O, Global_عرض);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ساكن صحيح OptNameCompare(ثابت std::pair<ثابت حرف *, Option *> *LHS,
                          ثابت std::pair<ثابت حرف *, Option *> *RHS) {
  أرجع strcmp(LHS->اولی_2, RHS->اولی_2);
}

ساكن صحيح SubNameCompare(ثابت std::pair<ثابت حرف *, SubCommand *> *LHS,
                          ثابت std::pair<ثابت حرف *, SubCommand *> *RHS) {
  أرجع strcmp(LHS->اولی_2, RHS->اولی_2);
}

// Copy Options into a vector so we can sort them as we like.
ساكن فراغ مرتب_سازی_Opts(OptionsMapTy &OptMap,
                     Small_وکتور_Impl<std::pair<ثابت حرف *, Option *>> &Opts,
                     منطقي ShowHidden) {
  Small_Ptr_مجموعه<Option *, 32> Option_مجموعه; // Duplicate option detection.

  لكل (خودکار I = OptMap.begin(), E = OptMap.انتها(); I != E; ++I) {
    // Ignore really-hidden options.
    إذا (I->دومی_2->getOptionHiddenFlag() == ReallyHidden)
      استمر;

    // Unless showhidden is set, ignore hidden flags.
    إذا (I->دومی_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      استمر;

    // If we've already seen this option, don't add it to the list again.
    إذا (!Option_مجموعه.درج(I->دومی_2).دومی_2)
      استمر;

    Opts.افزودن_پشت(
        std::pair<ثابت حرف *, Option *>(I->اولی_2.داده(), I->دومی_2));
  }

  // Sort the options list alphabetically.
  array_pod_مرتب_سازی(Opts.begin(), Opts.انتها(), OptNameCompare);
}

ساكن فراغ
مرتب_سازی_Sub_Commands(ثابت Small_Ptr_مجموعه_Impl<SubCommand *> &SubMap,
                Small_وکتور_Impl<std::pair<ثابت حرف *, SubCommand *>> &Subs) {
  لكل (خودکار *S : SubMap) {
    إذا (S->getName().خالی())
      استمر;
    Subs.افزودن_پشت(std::make_pair(S->getName().داده(), S));
  }
  array_pod_مرتب_سازی(Subs.begin(), Subs.انتها(), SubNameCompare);
}

نطاق {

صنف HelpPrinter {
محمي:
  ثابت منطقي ShowHidden;
  استفاده Str_Option_Pair_وکتور =
      Small_وکتور<std::pair<ثابت حرف *, Option *>, 128>;
  استفاده Str_Sub_Command_Pair_وکتور =
      Small_وکتور<std::pair<ثابت حرف *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  مجازی فراغ printOptions(Str_Option_Pair_وکتور &Opts, size_t MaxArgLen) {
    لكل (ثابت خودکار &Opt : Opts)
      Opt.دومی_2->printOptionInfo(MaxArgLen);
  }

  فراغ printSubCommands(Str_Sub_Command_Pair_وکتور &Subs, size_t MaxSubLen) {
    لكل (ثابت خودکار &S : Subs) {
      outs() << "  " << S.اولی_2;
      إذا (!S.دومی_2->getDescription().خالی()) {
        outs().indent(MaxSubLen - طول_رشته(S.اولی_2));
        outs() << " - " << S.دومی_2->getDescription();
      }
      outs() << "\n";
    }
  }

عام:
  صریح HelpPrinter(منطقي showHidden) : ShowHidden(showHidden) {}
  مجازی ~HelpPrinter() = افتراضي;

  // Invoke the printer.
  فراغ عملگر=(منطقي مقدار) {
    إذا (!مقدار)
      أرجع;
    printHelp();

    // Halt the program since help information was printed
    خروج(0);
  }

  فراغ printHelp() {
    SubCommand *Sub = globalParser().get_فعال_Sub_Command();
    خودکار &OptionsMap = Sub->OptionsMap;
    خودکار &PositionalOpts = Sub->PositionalOpts;
    خودکار &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_وکتور Opts;
    مرتب_سازی_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_وکتور Subs;
    مرتب_سازی_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    إذا (!globalParser().برنامه_Overview.خالی())
      outs() << "OVERVIEW: " << globalParser().برنامه_Overview << "\n";

    إذا (Sub == &SubCommand::get_بالاترین_Level()) {
      outs() << "USAGE: " << globalParser().برنامه_Name;
      إذا (!Subs.خالی())
        outs() << " [subcommand]";
      outs() << " [options]";
    } وإلا {
      إذا (!Sub->getDescription().خالی()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().برنامه_Name << " " << Sub->getName()
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

    إذا (Sub == &SubCommand::get_بالاترین_Level() && !Subs.خالی()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      لكل (ثابت خودکار &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, طول_رشته(Sub.اولی_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().برنامه_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    لكل (ثابت خودکار &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.دومی_2->get_Option_عرض());

    Small_وکتور<std::pair<std::string, StringRef>, 0> LibraryOpts;
    إذا (ShowHidden)
      لكل (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              إذا (!Help.خالی())
                LibraryOpts.emplace_پشت((Spelling + MetaVar).str(), Help);
            });
    llvm::مرتب_سازی(LibraryOpts);
    لكل (ثابت خودکار &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_اندازه(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    إذا (!LibraryOpts.خالی())
      outs() << "\nLibrary options:\n\n";
    لكل (ثابت خودکار &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_اندازه(Name));
    }

    // Print any extra help the user has declared.
    لكل (ثابت خودکار &I : globalParser().بیشتر_Help)
      outs() << I;
    globalParser().بیشتر_Help.clear();
  }
};

صنف CategorizedHelpPrinter : عام HelpPrinter {
عام:
  صریح CategorizedHelpPrinter(منطقي showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ساكن صحيح OptionCategoryCompare(OptionCategory *ثابت *A,
                                   OptionCategory *ثابت *B) {
    أرجع (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  استفاده HelpPrinter::عملگر=;

محمي:
  فراغ printOptions(Str_Option_Pair_وکتور &Opts, size_t MaxArgLen) override {
    std::وکتور<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::وکتور<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    أكد(SortedCategories.اندازه_2() > 0 && "No option categories registered!");
    array_pod_مرتب_سازی(SortedCategories.begin(), SortedCategories.انتها(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    لكل (ثابت خودکار &I : Opts) {
      Option *Opt = I.دومی_2;
      لكل (OptionCategory *Cat : Opt->Categories) {
        أكد(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].افزودن_پشت(Opt);
      }
    }

    // Now do printing.
    لكل (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ثابت خودکار &CategoryOptions = CategorizedOptions[Category];
      إذا (CategoryOptions.خالی())
        استمر;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      إذا (!Category->getDescription().خالی())
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
  صریح HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  فراغ عملگر=(منطقي مقدار);
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
#پایان_اگر

نطاق {
صنف نسخه_Printer {
عام:
  فراغ print(ثابت std::وکتور<نسخه_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#اگر_تعریف_شده PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#وإلا
    OS << "LLVM (http://llvm.org/):\n  ";
#پایان_اگر
    OS << PACKAGE_NAME << " version " << PACKAGE_نسخه << "\n  ";
#إذا LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#وإلا
    OS << "Optimized build";
#پایان_اگر
#اگر_تعریف_نشده NDEBUG
    OS << " with assertions";
#پایان_اگر
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    إذا (!ExtraPrinters.خالی()) {
      لكل (ثابت خودکار &I : ExtraPrinters)
        I(outs());
    }
  }
  فراغ عملگر=(منطقي OptionWasSpecified);
};

بنية Command_سطر_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_نرمال_Printer{خطأ};
  HelpPrinter UncategorizedHiddenPrinter{درست};
  CategorizedHelpPrinter Categorized_نرمال_Printer{خطأ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{درست};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_نرمال_Printer{Uncategorized_نرمال_Printer,
                                          Categorized_نرمال_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, درست, parser<منطقي>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_نرمال_Printer),
      cl::Hidden,
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, درست, parser<منطقي>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, درست, parser<منطقي>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_نرمال_Printer),
      cl::مقدار_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, درست, parser<منطقي>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::مقدار_Disallowed,
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

  نسخه_Printer_Ty Override_نسخه_Printer = اشاره_تهی;

  std::وکتور<نسخه_Printer_Ty> Extra_نسخه_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  نسخه_Printer نسخه_Printer_Instance;

  cl::opt<نسخه_Printer, درست, parser<منطقي>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(نسخه_Printer_Instance), cl::مقدار_Disallowed,
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
  init_سیگنالها_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_دانه_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ساكن OptionCategory GeneralCategory{"General options"};
  أرجع GeneralCategory;
}

فراغ نسخه_Printer::عملگر=(منطقي OptionWasSpecified) {
  إذا (!OptionWasSpecified)
    أرجع;

  إذا (CommonOptions->Override_نسخه_Printer != اشاره_تهی) {
    CommonOptions->Override_نسخه_Printer(outs());
    خروج(0);
  }
  print(CommonOptions->Extra_نسخه_Printers);

  خروج(0);
}

فراغ HelpPrinterWrapper::عملگر=(منطقي مقدار) {
  إذا (!مقدار)
    أرجع;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  إذا (globalParser().RegisteredOptionCategories.اندازه_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.مجموعه_Hidden_Flag(NotHidden);

    CategorizedPrinter = درست; // Invoke categorized printer
  } وإلا {
    UncategorizedPrinter = درست; // Invoke uncategorized printer
  }
}

// Print the value of each option.
فراغ cl::Print_Option_مقادیر() { globalParser().print_Option_مقادیر(); }

فراغ Command_سطر_Parser::print_Option_مقادیر() {
  إذا (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    أرجع;

  Small_وکتور<std::pair<ثابت حرف *, Option *>, 128> Opts;
  مرتب_سازی_Opts(فعال_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ درست);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  لكل (ثابت خودکار &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.دومی_2->get_Option_عرض());

  لكل (ثابت خودکار &Opt : Opts)
    Opt.دومی_2->print_Option_مقدار(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
فراغ cl::Print_Help_پیام(منطقي Hidden, منطقي Categorized) {
  إذا (!Hidden && !Categorized)
    CommonOptions->Uncategorized_نرمال_Printer.printHelp();
  وإلا إذا (!Hidden && Categorized)
    CommonOptions->Categorized_نرمال_Printer.printHelp();
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
#پایان_اگر
#اگر_تعریف_نشده NDEBUG
      "+assertions",
#پایان_اگر
#اگر_تعریف_شده EXPENSIVE_CHECKS
      "+expensive-checks",
#پایان_اگر
#إذا __has_feature(آدرس_sanitizer)
      "+asan",
#پایان_اگر
#إذا __has_feature(dataflow_sanitizer)
      "+dfsan",
#پایان_اگر
#إذا __has_feature(hwaddress_sanitizer)
      "+hwasan",
#پایان_اگر
#إذا __has_feature(حافظه_sanitizer)
      "+msan",
#پایان_اگر
#إذا __has_feature(thread_sanitizer)
      "+tsan",
#پایان_اگر
#إذا __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#پایان_اگر
#اگر_تعریف_شده LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#پایان_اگر
  };
  أرجع ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
فراغ cl::printBuildConfig(raw_ostream &OS) {
#إذا LLVM_نسخه_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#پایان_اگر
}

/// Utility function for printing version number.
فراغ cl::Print_نسخه_پیام() {
  CommonOptions->نسخه_Printer_Instance.print(CommonOptions->Extra_نسخه_Printers);
}

فراغ cl::مجموعه_نسخه_Printer(نسخه_Printer_Ty func) {
  CommonOptions->Override_نسخه_Printer = func;
}

فراغ cl::جمع_کن_Extra_نسخه_Printer(نسخه_Printer_Ty func) {
  CommonOptions->Extra_نسخه_Printers.افزودن_پشت(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  خودکار &Subs = globalParser().RegisteredSubCommands;
  (فراغ)Subs;
  أكد(Subs.شامل_است(&Sub));
  أرجع Sub.OptionsMap;
}

iterator_range<Small_Ptr_مجموعه<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  أرجع globalParser().getRegisteredSubcommands();
}

فراغ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  لكل (خودکار &I : Sub.OptionsMap) {
    منطقي Unrelated = درست;
    لكل (خودکار &Cat : I.دومی_2->Categories) {
      إذا (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دومی_2->مجموعه_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::HideUnrelatedOptions(ArrayRef<ثابت cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  لكل (خودکار &I : Sub.OptionsMap) {
    منطقي Unrelated = درست;
    لكل (خودکار &Cat : I.دومی_2->Categories) {
      إذا (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = خطأ;
    }
    إذا (Unrelated)
      I.دومی_2->مجموعه_Hidden_Flag(cl::ReallyHidden);
  }
}

فراغ cl::Reset_Command_سطر_Parser() { globalParser().reset(); }

فراغ cl::جمع_کن_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.افزودن_پشت(&L);
}

فراغ LibraryOptionsContext::تخصیص_بده(ثابت LibraryOptionsContext &Other) {
  أكد(هذا != &Other);
  مقادیر.clear();
  Saved_آرگومانها.clear();
  ArrayRef<ثابت حرف *> آرگومانها = Other.Saved_آرگومانها;
  لكل (size_t I = 0, E = آرگومانها.اندازه_2(); I != E;) {
    StringRef Name = آرگومانها[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.تقسیم('=').اولی_2);
    بیعلامت N;
    cantFail(parse(*L, آرگومانها.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ثابت حرف *> آرگومانها,
                                   بیعلامت &Consumed) {
  خودکار [It, Inserted] = مقادیر.try_emplace(L.getDefault());
  إذا (Inserted)
    It->دومی_2 = L.copyDefault(Alloc);
  أرجع parseLibraryOption(L, It->دومی_2, آرگومانها, Consumed, Alloc, &Saved_آرگومانها);
}

فراغ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

فراغ LLVMParse_Command_سطر_Options(صحيح argc, ثابت حرف *ثابت *argv,
                                 ثابت حرف *Overview) {
  llvm::cl::Parse_Command_سطر_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
