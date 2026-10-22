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

#अन्तर्भाव "llvm/Support/CommandLine.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm-c/Support.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/STLFunctionalExtras.h"
#अन्तर्भाव "llvm/ADT/SmallPtrSet.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/LibraryOptionsContext.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/StringSaver.h"
#अन्तर्भाव "llvm/Support/TypeSize.h"
#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <optional>
#अन्तर्भाव <string>
उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश cl;

#परिभाषय DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
नामाकाश llvm {
नामाकाश cl {
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<तार्किक>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<पूर्णाङ्क>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<दीर्घ दीर्घ>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिह्नित>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिह्नित दीर्घ>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<अचिह्नित दीर्घ दीर्घ>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<द्विगुण>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<भिन्न>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<वर्ण>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE basic_parser<Element_गणनम्>;

#यदि !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE opt<std::string>;
#यदि_अन्तः

प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE opt<तार्किक>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE opt<वर्ण>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE opt<पूर्णाङ्क>;
प्रतिरूपम् वर्ग LLVM_EXPORT_TEMPLATE opt<अचिह्नित>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
शून्य Generic_Option_मानम्::anchor() {}
शून्य Option_मानम्<boolOrDefault>::anchor() {}
शून्य Option_मानम्<std::string>::anchor() {}
शून्य Option::anchor() {}
शून्य basic_parser_impl::anchor() {}
शून्य parser<तार्किक>::anchor() {}
शून्य parser<boolOrDefault>::anchor() {}
शून्य parser<पूर्णाङ्क>::anchor() {}
शून्य parser<दीर्घ>::anchor() {}
शून्य parser<दीर्घ दीर्घ>::anchor() {}
शून्य parser<अचिह्नित>::anchor() {}
शून्य parser<अचिह्नित दीर्घ>::anchor() {}
शून्य parser<अचिह्नित दीर्घ दीर्घ>::anchor() {}
शून्य parser<द्विगुण>::anchor() {}
शून्य parser<भिन्न>::anchor() {}
शून्य parser<std::string>::anchor() {}
शून्य parser<std::optional<std::string>>::anchor() {}
शून्य parser<वर्ण>::anchor() {}
शून्य parser<Element_गणनम्>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
शून्य opt_bool_anchor() { opt<तार्किक> anchor{""}; }
शून्य opt_char_anchor() { opt<वर्ण> anchor{""}; }
शून्य opt_int_anchor() { opt<पूर्णाङ्क> anchor{""}; }
शून्य opt_unsigned_anchor() { opt<अचिह्नित> anchor{""}; }

//===----------------------------------------------------------------------===//

ध्रुव स्थिर size_t DefaultPad = 2;

स्थिर StringRef Arg_उपसर्गः = "-";
स्थिर StringRef Arg_उपसर्गः_Long = "--";
स्थिर StringRef Arg_Help_उपसर्गः = " - ";

स्थिर size_t arg_Plus_Prefixes_आकार(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.आकार_2();
  यदि (Len == 1)
    प्रत्यागम Len + Pad + Arg_उपसर्गः.आकार_2() + Arg_Help_उपसर्गः.आकार_2();
  प्रत्यागम Len + Pad + Arg_उपसर्गः_Long.आकार_2() + Arg_Help_उपसर्गः.आकार_2();
}

स्थिर SmallString<8> arg_उपसर्गः(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> उपसर्गः;
  प्रति (size_t I = 0; I < Pad; ++I) {
    उपसर्गः.प्रक्षिप_पृष्ठभागः(' ');
  }
  उपसर्गः.append(ArgName.आकार_2() > 1 ? Arg_उपसर्गः_Long : Arg_उपसर्गः);
  प्रत्यागम उपसर्गः;
}

उपयुज्य OptionsMapTy = DenseMap<StringRef, Option *>;

नामाकाश {

वर्ग PrintArg {
  StringRef ArgName;
  size_t Pad;
सार्वजनिक:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  मित्रम् raw_ostream &संक्रियकः<<(raw_ostream &OS, ध्रुव PrintArg &);
};

raw_ostream &संक्रियकः<<(raw_ostream &OS, ध्रुव PrintArg& Arg) {
  OS << arg_उपसर्गः(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  प्रत्यागम OS;
}

वर्ग Command_पङ्क्तिः_Parser {
सार्वजनिक:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string क्रमादेशः_Name;
  StringRef क्रमादेशः_Overview;

  // This collects additional help to be printed.
  std::सदिशम्<StringRef> अधिकम्_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_समुच्चयः<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_समुच्चयः<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_सदिशम्<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_सूचकांक;
  size_t अङ्कः_अनुक्रमितम्_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_पङ्क्तिः_Parser() { पञ्जिकाकोशः_Sub_Command(&SubCommand::get_शिखरम्_Level()); }

  शून्य ResetAllOptionOccurrences();

  तार्किक Parse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                               StringRef Overview, raw_ostream *Errs = शून्यसूचकः,
                               vfs::FileSystem *VFS = शून्यसूचकः,
                               LibraryOptionsContext *Context = शून्यसूचकः);

  शून्य forEachSubCommand(Option &Opt, function_ref<शून्य(SubCommand &)> Action) {
    यदि (Opt.Subs.रिक्तानि()) {
      Action(SubCommand::get_शिखरम्_Level());
      प्रत्यागम;
    }
    यदि (Opt.Subs.आकार_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      प्रति (स्वचालित *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      प्रत्यागम;
    }
    प्रति (स्वचालित *SC : Opt.Subs) {
      प्रतिज्ञा(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  शून्य सङ्कलय_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    यदि (Opt.hasArgStr())
      प्रत्यागम;
    यदि (!SC->OptionsMap.निवेशय(std::make_pair(Name, &Opt)).second) {
      errs() << क्रमादेशः_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  शून्य सङ्कलय_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { सङ्कलय_Literal_Option(Opt, &SC, Name); });
  }

  शून्य सङ्कलय_Option(Option *O, SubCommand *SC) {
    तार्किक HadErrors = असत्य;
    यदि (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      यदि (!SC->OptionsMap.निवेशय(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.अन्तर्भवति(SC) &&
           Library_सूचकांक.अन्तर्भवति(O->ArgStr))) {
        errs() << क्रमादेशः_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = सत्य;
      }
    }

    // Remember information about positional options.
    यदि (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.प्रक्षिप_पृष्ठभागः(O);
    अन्यथा यदि (O->get_अङ्कः_Occurrences_Flag() == cl::ConsumeAfter) {
      यदि (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = सत्य;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    यदि (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  शून्य सङ्कलय_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { सङ्कलय_Option(O, &SC); });
  }

  शून्य removeOption(Option *O, SubCommand *SC) {
    Small_सदिशम्<StringRef, 16> Option_नामानि;
    O->get_Extra_Option_नामानि(Option_नामानि);
    यदि (O->hasArgStr())
      Option_नामानि.प्रक्षिप_पृष्ठभागः(O->ArgStr);

    SubCommand &Sub = *SC;
    प्रति (स्वचालित Name : Option_नामानि) {
      स्वचालित I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      यदि (I != Sub.OptionsMap.अन्तः() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    यदि (O->getFormattingFlag() == cl::Positional)
      प्रति (स्वचालित *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.अन्तः(); ++Opt) {
        यदि (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          भङ्ग;
        }
      }
    अन्यथा यदि (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = शून्यसूचकः;
  }

  शून्य removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  तार्किक hasOptions(ध्रुव SubCommand &Sub) ध्रुव {
    प्रत्यागम (!Sub.OptionsMap.रिक्तानि() || !Sub.PositionalOpts.रिक्तानि() ||
            शून्यसूचकः != Sub.ConsumeAfterOpt);
  }

  तार्किक hasOptions() ध्रुव {
    प्रति (ध्रुव स्वचालित *S : RegisteredSubCommands) {
      यदि (hasOptions(*S))
        प्रत्यागम सत्य;
    }
    प्रत्यागम !Libraries.रिक्तानि();
  }

  तार्किक hasNamedSubCommands() ध्रुव {
    प्रति (ध्रुव स्वचालित *S : RegisteredSubCommands)
      यदि (!S->getName().रिक्तानि())
        प्रत्यागम सत्य;
    प्रत्यागम असत्य;
  }

  SubCommand *get_सक्रिय_Sub_Command() { प्रत्यागम सक्रिय_Sub_Command; }

  शून्य updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    यदि (!Sub.OptionsMap.निवेशय(std::make_pair(NewName, O)).second) {
      errs() << क्रमादेशः_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  शून्य updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  शून्य print_Option_मानानि();

  शून्य सूचकांक_Library_Options() {
    तार्किक HadErrors = असत्य;
    प्रति (; अङ्कः_अनुक्रमितम्_Libraries != Libraries.आकार_2(); ++अङ्कः_अनुक्रमितम्_Libraries) {
      LibraryOptions *L = Libraries[अङ्कः_अनुक्रमितम्_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        स्वचालित [It, Inserted] = Library_सूचकांक.try_emplace(Name, L);
        यदि (Inserted ? none_सम्बन्धि(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 प्रत्यागम SC->OptionsMap.अन्तर्भवति(Name);
                               })
                     : It->second == L)
          प्रत्यागम;
        errs() << क्रमादेशः_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = सत्य;
      });
    }
    यदि (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    सूचकांक_Library_Options();
    प्रत्यागम Library_सूचकांक.lookup(Name);
  }

  शून्य पञ्जिकाकोशः_Category(OptionCategory *cat) {
    प्रतिज्ञा(गणनम्_if(RegisteredOptionCategories,
                    [cat](ध्रुव OptionCategory *Category) {
             प्रत्यागम cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.निवेशय(cat);
  }

  शून्य पञ्जिकाकोशः_Sub_Command(SubCommand *sub) {
    प्रतिज्ञा(गणनम्_if(RegisteredSubCommands,
                    [sub](ध्रुव SubCommand *Sub) {
                      प्रत्यागम (!sub->getName().रिक्तानि()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.निवेशय(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    प्रतिज्ञा(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    प्रति (स्वचालित &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      यदि (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        सङ्कलय_Option(O, sub);
      अन्यथा
        सङ्कलय_Literal_Option(*O, sub, E.first);
    }
  }

  शून्य unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_समुच्चयः<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    प्रत्यागम make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.अन्तः());
  }

  शून्य reset() {
    सक्रिय_Sub_Command = शून्यसूचकः;
    क्रमादेशः_Name.clear();
    क्रमादेशः_Overview = StringRef();

    अधिकम्_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_सूचकांक.clear();
    अङ्कः_अनुक्रमितम्_Libraries = 0;

    SubCommand::get_शिखरम्_Level().reset();
    SubCommand::getAll().reset();
    पञ्जिकाकोशः_Sub_Command(&SubCommand::get_शिखरम्_Level());
  }

गुप्त:
  SubCommand *सक्रिय_Sub_Command = शून्यसूचकः;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &मानम्);
  SubCommand *LookupSubCommand(StringRef Name, std::string &निकटतमः_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
स्थिर Command_पङ्क्तिः_Parser &globalParser() {
  स्थिर ManagedStatic<Command_पङ्क्तिः_Parser> GlobalParser;
  प्रत्यागम *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
स्थिर Error
parseLibraryOption(LibraryOptions &L, शून्य *Opts, ArrayRef<ध्रुव वर्ण *> तर्काः,
                   अचिह्नित &Consumed, BumpPtrAllocator &Alloc,
                   Small_सदिशम्_Impl<ध्रुव वर्ण *> *लेखः = शून्यसूचकः) {
  StringSaver Saver(Alloc);
  ध्रुव वर्ण *Saved[2];
  अचिह्नित अङ्कः_तर्काः = std::min<size_t>(तर्काः.आकार_2(), 2);
  प्रति (अचिह्नित J = 0; J != अङ्कः_तर्काः; ++J)
    Saved[J] = Saver.save(तर्काः[J]).दत्त();
  Consumed = 1;
  यदि (Error E = L.parse(Opts, ArrayRef(Saved, अङ्कः_तर्काः), Consumed, Alloc))
    प्रत्यागम E;
  यदि (लेखः)
    लेखः->append(Saved, Saved + Consumed);
  प्रत्यागम Error::success();
}

प्रतिरूपम् <प्रकारनाम T, T TrueVal, T FalseVal>
स्थिर तार्किक parseBool(Option &O, StringRef ArgName, StringRef Arg, T &मानम्) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  यदि (!Arg.दत्त() || Arg == "true" || Arg == "1") {
    मानम् = TrueVal;
    प्रत्यागम असत्य;
  }

  यदि (Arg == "false" || Arg == "0") {
    मानम् = FalseVal;
    प्रत्यागम असत्य;
  }
  प्रत्यागम O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

शून्य cl::सङ्कलय_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().सङ्कलय_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().अधिकम्_Help.प्रक्षिप_पृष्ठभागः(Help);
}

Option::Option(अङ्कः_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : अङ्कः_Occurrences(0), Occurrences(OccurrencesFlag), मानम्(0),
      HiddenFlag(Hidden), Formatting(प्रसामान्यम्_Formatting), Misc(0),
      FullyInitialized(असत्य), Position(0) {
  Categories.प्रक्षिप_पृष्ठभागः(&getGeneralCategory());
}

शून्य Option::सङ्कलय_तर्कः() {
  globalParser().सङ्कलय_Option(एतत्);
  FullyInitialized = सत्य;
}

शून्य Option::remove_तर्कः() { globalParser().removeOption(एतत्); }

शून्य Option::समुच्चयः_Arg_Str(StringRef S) {
  यदि (FullyInitialized)
    globalParser().updateArgStr(एतत्, S);
  प्रतिज्ञा(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

शून्य Option::सङ्कलय_Category(OptionCategory &C) {
  प्रतिज्ञा(!Categories.रिक्तानि() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  यदि (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  अन्यथा यदि (!is_contained(Categories, &C))
    Categories.प्रक्षिप_पृष्ठभागः(&C);
}

शून्य Option::reset() {
  अङ्कः_Occurrences = 0;
  समुच्चयः_Default();
}

शून्य OptionCategory::पञ्जिकाकोशः_Category() {
  globalParser().पञ्जिकाकोशः_Category(एतत्);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_शिखरम्_Level() {
  स्थिर ManagedStatic<SubCommand> शिखरम्_Level_Sub_Command;
  प्रत्यागम *शिखरम्_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  स्थिर ManagedStatic<SubCommand> AllSubCommands;
  प्रत्यागम *AllSubCommands;
}

शून्य SubCommand::पञ्जिकाकोशः_Sub_Command() {
  globalParser().पञ्जिकाकोशः_Sub_Command(एतत्);
}

शून्य SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(एतत्);
}

शून्य SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = शून्यसूचकः;
}

SubCommand::संक्रियकः तार्किक() ध्रुव {
  प्रत्यागम (globalParser().get_सक्रिय_Sub_Command() == एतत्);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_पङ्क्तिः_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &मानम्) {
  // Reject all dashes.
  यदि (Arg.रिक्तानि())
    प्रत्यागम शून्यसूचकः;
  प्रतिज्ञा(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  यदि (EqualPos == StringRef::npos) {
    // Look up the option.
    प्रत्यागम Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  स्वचालित I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  यदि (I == Sub.OptionsMap.अन्तः())
    प्रत्यागम शून्यसूचकः;

  स्वचालित *O = I->second;
  यदि (O->getFormattingFlag() == cl::Always_उपसर्गः)
    प्रत्यागम शून्यसूचकः;

  मानम् = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  प्रत्यागम I->second;
}

SubCommand *Command_पङ्क्तिः_Parser::LookupSubCommand(StringRef Name,
                                                std::string &निकटतमः_String) {
  यदि (Name.रिक्तानि())
    प्रत्यागम &SubCommand::get_शिखरम्_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *निकटतमः_मेलनम् = शून्यसूचकः;
  प्रति (स्वचालित *S : RegisteredSubCommands) {
    प्रतिज्ञा(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    यदि (S->getName().रिक्तानि())
      अनुवर्त;

    यदि (S->getName() == Name)
      प्रत्यागम S;

    यदि (!निकटतमः_मेलनम् && S->getName().edit_distance(Name) < 2)
      निकटतमः_मेलनम् = S;
  }

  यदि (निकटतमः_मेलनम्)
    निकटतमः_String = निकटतमः_मेलनम्->getName();

  प्रत्यागम &SubCommand::get_शिखरम्_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
स्थिर Option *Lookup_निकटतमः_Option(StringRef Arg,
                                   ध्रुव OptionsMapTy &OptionsMap,
                                   std::string &निकटतमः_String) {
  // Reject all dashes.
  यदि (Arg.रिक्तानि())
    प्रत्यागम शून्यसूचकः;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> विभज_Arg = Arg.विभज('=');
  StringRef &LHS = विभज_Arg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = विभज_Arg.second;

  // Find the closest match.
  Option *Best = शून्यसूचकः;
  अचिह्नित BestDistance = 0;
  प्रति (ध्रुव स्वचालित &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    यदि (O->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    Small_सदिशम्<StringRef, 16> Option_नामानि;
    O->get_Extra_Option_नामानि(Option_नामानि);
    यदि (O->hasArgStr())
      Option_नामानि.प्रक्षिप_पृष्ठभागः(O->ArgStr);

    तार्किक Permit_मानम् = O->get_मानम्_Expected_Flag() != cl::मानम्_Disallowed;
    StringRef Flag = Permit_मानम् ? LHS : Arg;
    प्रति (ध्रुव स्वचालित &Name : Option_नामानि) {
      अचिह्नित Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/सत्य, /*MaxEditDistance=*/BestDistance);
      यदि (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        यदि (RHS.रिक्तानि() || !Permit_मानम्)
          निकटतमः_String = std::string(Name);
        अन्यथा
          निकटतमः_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  प्रत्यागम Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
स्थिर तार्किक Comma_Separate_And_सङ्कलय_Occurrence(Option *Handler, अचिह्नित pos,
                                          StringRef ArgName, StringRef मानम्) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  यदि (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(मानम्);
    StringRef::आकार_type Pos = Val.find(',');

    यावत् (Pos != StringRef::npos) {
      // Process the portion before the comma.
      यदि (Handler->सङ्कलय_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        प्रत्यागम सत्य;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    मानम् = Val;
  }

  प्रत्यागम Handler->सङ्कलय_Occurrence(pos, ArgName, मानम्);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
स्थिर अन्तःस्थ तार्किक ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef मानम्, पूर्णाङ्क argc,
                                 ध्रुव वर्ण *ध्रुव *argv, पूर्णाङ्क &i) {
  // Enforce value requirements
  विकल्प (Handler->get_मानम्_Expected_Flag()) {
  पक्ष मानम्_Required:
    यदि (!मानम्.दत्त()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      यदि (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_उपसर्गः)
        प्रत्यागम Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      प्रतिज्ञा(argv && "null check");
      मानम् = StringRef(argv[++i]);
    }
    भङ्ग;
  पक्ष मानम्_Disallowed:
    यदि (मानम्.दत्त())
      प्रत्यागम Handler->error("does not allow a value! '" + Twine(मानम्) +
                            "' specified.");
    भङ्ग;
  पक्ष मानम्_Optional:
    भङ्ग;
  }

  प्रत्यागम Comma_Separate_And_सङ्कलय_Occurrence(Handler, i, ArgName, मानम्);
}

तार्किक llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, पूर्णाङ्क i) {
  पूर्णाङ्क Dummy = i;
  प्रत्यागम ProvideOption(Handler, Handler->ArgStr, Arg, 0, शून्यसूचकः, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
स्थिर Option *find_उपसर्गः_Option(StringRef Name, size_t &दीर्घता,
                                ध्रुव OptionsMapTy &OptionsMap) {
  प्रति (; !Name.रिक्तानि(); Name = Name.drop_पृष्ठभागः()) {
    Option *O = OptionsMap.lookup(Name);
    यदि (O && (O->getFormattingFlag() == cl::उपसर्गः ||
              O->getFormattingFlag() == cl::Always_उपसर्गः)) {
      दीर्घता = Name.आकार_2();
      प्रत्यागम O;
    }
  }
  प्रत्यागम शून्यसूचकः;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
स्थिर Option *HandlePrefixedOption(StringRef &Arg, StringRef &मानम्,
                                    ध्रुव OptionsMapTy &OptionsMap) {
  यदि (Arg.आकार_2() == 1)
    प्रत्यागम शून्यसूचकः;

  size_t दीर्घता = 0;
  Option *POpt = find_उपसर्गः_Option(Arg, दीर्घता, OptionsMap);
  यदि (!POpt)
    प्रत्यागम शून्यसूचकः;

  StringRef Maybe_मानम् =
      (दीर्घता < Arg.आकार_2()) ? Arg.substr(दीर्घता) : StringRef();
  Arg = Arg.substr(0, दीर्घता);

  // cl::Prefix options do not preserve '=' when used separately.
  यदि (POpt->getFormattingFlag() == cl::उपसर्गः && Maybe_मानम्.starts_with("="))
    Maybe_मानम् = Maybe_मानम्.drop_front();
  मानम् = Maybe_मानम्;
  प्रत्यागम POpt;
}

स्थिर तार्किक Requires_मानम्(ध्रुव Option *O) {
  प्रत्यागम O->get_अङ्कः_Occurrences_Flag() == cl::Required ||
         O->get_अङ्कः_Occurrences_Flag() == cl::One_Or_अधिकम्;
}

स्थिर तार्किक Eats_Unbounded_सङ्ख्या_सम्बन्धि_मानानि(ध्रुव Option *O) {
  प्रत्यागम O->get_अङ्कः_Occurrences_Flag() == cl::Zero_Or_अधिकम् ||
         O->get_अङ्कः_Occurrences_Flag() == cl::One_Or_अधिकम्;
}

स्थिर तार्किक isWhitespace(वर्ण C) {
  प्रत्यागम C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

स्थिर तार्किक isWhitespaceOrNull(वर्ण C) {
  प्रत्यागम isWhitespace(C) || C == '\0';
}

स्थिर तार्किक isQuote(वर्ण C) { प्रत्यागम C == '\"' || C == '\''; }

शून्य cl::Tokenize_GNUCommand_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv,
                                तार्किक MarkEOLs) {
  SmallString<128> Token;
  तार्किक अन्तर्_Token = असत्य;
  प्रति (size_t I = 0, E = Src.आकार_2(); I != E; ++I) {
    // Consume runs of whitespace.
    यदि (!अन्तर्_Token) {
      यावत् (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        यदि (MarkEOLs && Src[I] == '\n')
          NewArgv.प्रक्षिप_पृष्ठभागः(शून्यसूचकः);
        ++I;
      }
      यदि (I == E)
        भङ्ग;
      अन्तर्_Token = सत्य;
    }

    वर्ण C = Src[I];

    // Backslash escapes the next character.
    यदि (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.प्रक्षिप_पृष्ठभागः(Src[I]);
      अनुवर्त;
    }

    // Consume a quoted string.
    यदि (isQuote(C)) {
      ++I;
      यावत् (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        यदि (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.प्रक्षिप_पृष्ठभागः(Src[I]);
        ++I;
      }
      यदि (I == E)
        भङ्ग;
      अनुवर्त;
    }

    // End the token if this is whitespace.
    यदि (isWhitespace(C)) {
      NewArgv.प्रक्षिप_पृष्ठभागः(Saver.save(Token.str()).दत्त());
      // Mark the end of lines in response files.
      यदि (MarkEOLs && C == '\n')
        NewArgv.प्रक्षिप_पृष्ठभागः(शून्यसूचकः);
      Token.clear();
      अन्तर्_Token = असत्य;
      अनुवर्त;
    }

    // This is a normal character.  Append it.
    Token.प्रक्षिप_पृष्ठभागः(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  यदि (अन्तर्_Token)
    NewArgv.प्रक्षिप_पृष्ठभागः(Saver.save(Token.str()).दत्त());
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
स्थिर size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.आकार_2();
  पूर्णाङ्क Backslash_गणनम् = 0;
  // Skip the backslashes.
  कुरु {
    ++I;
    ++Backslash_गणनम्;
  } यावत् (I != E && Src[I] == '\\');

  तार्किक FollowedByDoubleQuote = (I != E && Src[I] == '"');
  यदि (FollowedByDoubleQuote) {
    Token.append(Backslash_गणनम् / 2, '\\');
    यदि (Backslash_गणनम् % 2 == 0)
      प्रत्यागम I - 1;
    Token.प्रक्षिप_पृष्ठभागः('"');
    प्रत्यागम I;
  }
  Token.append(Backslash_गणनम्, '\\');
  प्रत्यागम I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
स्थिर तार्किक isWindowsSpecialChar(वर्ण C) {
  प्रत्यागम isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
स्थिर तार्किक is_Windows_Special_Char_अन्तर्_Command_Name(वर्ण C) {
  प्रत्यागम isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
स्थिर अन्तःस्थ शून्य tokenize_Windows_Command_पङ्क्तिः_Impl(
    StringRef Src, StringSaver &Saver, function_ref<शून्य(StringRef)> सङ्कलय_Token,
    तार्किक AlwaysCopy, function_ref<शून्य()> MarkEOL, तार्किक InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  तार्किक CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  गणना { INIT, UNQUOTED, QUOTED } State = INIT;

  प्रति (size_t I = 0, E = Src.आकार_2(); I < E; ++I) {
    विकल्प (State) {
    पक्ष INIT: {
      प्रतिज्ञा(Token.रिक्तानि() && "token should be empty in initial state");
      // Eat whitespace before a token.
      यावत् (I < E && isWhitespaceOrNull(Src[I])) {
        यदि (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      यदि (I >= E)
        भङ्ग;
      size_t Start = I;
      यदि (CommandName) {
        यावत् (I < E && !is_Windows_Special_Char_अन्तर्_Command_Name(Src[I]))
          ++I;
      } अन्यथा {
        यावत् (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef प्रसामान्यम्_Chars = Src.खण्डिका(Start, I);
      यदि (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        सङ्कलय_Token(AlwaysCopy ? Saver.save(प्रसामान्यम्_Chars) : प्रसामान्यम्_Chars);
        यदि (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } अन्यथा {
          CommandName = असत्य;
        }
      } अन्यथा यदि (Src[I] == '\"') {
        Token += प्रसामान्यम्_Chars;
        State = QUOTED;
      } अन्यथा यदि (Src[I] == '\\') {
        प्रतिज्ञा(!CommandName && "or else we'd have treated it as a normal char");
        Token += प्रसामान्यम्_Chars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } अन्यथा {
        llvm_unreachable("unexpected special character");
      }
      भङ्ग;
    }

    पक्ष UNQUOTED:
      यदि (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        सङ्कलय_Token(Saver.save(Token.str()));
        Token.clear();
        यदि (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } अन्यथा {
          CommandName = असत्य;
        }
        State = INIT;
      } अन्यथा यदि (Src[I] == '\"') {
        State = QUOTED;
      } अन्यथा यदि (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.प्रक्षिप_पृष्ठभागः(Src[I]);
      }
      भङ्ग;

    पक्ष QUOTED:
      यदि (Src[I] == '\"') {
        यदि (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.प्रक्षिप_पृष्ठभागः('"');
          ++I;
        } अन्यथा {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } अन्यथा यदि (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } अन्यथा {
        Token.प्रक्षिप_पृष्ठभागः(Src[I]);
      }
      भङ्ग;
    }
  }

  यदि (State != INIT)
    सङ्कलय_Token(Saver.save(Token.str()));
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः(StringRef Src, StringSaver &Saver,
                                    Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv,
                                    तार्किक MarkEOLs) {
  स्वचालित सङ्कलय_Token = [&](StringRef Tok) { NewArgv.प्रक्षिप_पृष्ठभागः(Tok.दत्त()); };
  स्वचालित OnEOL = [&]() {
    यदि (MarkEOLs)
      NewArgv.प्रक्षिप_पृष्ठभागः(शून्यसूचकः);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, सङ्कलय_Token,
                                 /*AlwaysCopy=*/सत्य, OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_सदिशम्_Impl<StringRef> &NewArgv) {
  स्वचालित सङ्कलय_Token = [&](StringRef Tok) { NewArgv.प्रक्षिप_पृष्ठभागः(Tok); };
  स्वचालित OnEOL = []() {};
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, सङ्कलय_Token, /*AlwaysCopy=*/असत्य,
                                 OnEOL, असत्य);
}

शून्य cl::Tokenize_Windows_Command_पङ्क्तिः_पूर्णम्(StringRef Src, StringSaver &Saver,
                                        Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv,
                                        तार्किक MarkEOLs) {
  स्वचालित सङ्कलय_Token = [&](StringRef Tok) { NewArgv.प्रक्षिप_पृष्ठभागः(Tok.दत्त()); };
  स्वचालित OnEOL = [&]() {
    यदि (MarkEOLs)
      NewArgv.प्रक्षिप_पृष्ठभागः(शून्यसूचकः);
  };
  tokenize_Windows_Command_पङ्क्तिः_Impl(Src, Saver, सङ्कलय_Token,
                                 /*AlwaysCopy=*/सत्य, OnEOL, सत्य);
}

शून्य cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv,
                            तार्किक MarkEOLs) {
  प्रति (ध्रुव वर्ण *Cur = Source.begin(); Cur != Source.अन्तः();) {
    SmallString<128> पङ्क्तिः;
    // Check for comment line.
    यदि (isWhitespace(*Cur)) {
      यावत् (Cur != Source.अन्तः() && isWhitespace(*Cur))
        ++Cur;
      अनुवर्त;
    }
    यदि (*Cur == '#') {
      यावत् (Cur != Source.अन्तः() && *Cur != '\n')
        ++Cur;
      अनुवर्त;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ध्रुव वर्ण *Start = Cur;
    ध्रुव वर्ण *पङ्क्तिः_अन्तः = शून्यसूचकः;
    वर्ण Quote = 0;
    तार्किक AtTokenStart = सत्य;
    प्रति (ध्रुव वर्ण *अन्तः_2 = Source.अन्तः(); Cur != अन्तः_2; ++Cur) {
      वर्ण C = *Cur;
      यदि (C == '\\') {
        यदि (Cur + 1 != अन्तः_2) {
          ++Cur;
          यदि (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != अन्तः_2) && Cur[1] == '\n')) {
            पङ्क्तिः.append(Start, Cur - 1);
            यदि (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            अनुवर्त;
          }
        }
        AtTokenStart = असत्य;
        अनुवर्त;
      }
      यदि (C == '\n')
        भङ्ग;
      यदि (Quote) {
        यदि (C == Quote)
          Quote = 0;
      } अन्यथा यदि (isQuote(C)) {
        Quote = C;
      } अन्यथा यदि (C == '#' && AtTokenStart) {
        पङ्क्तिः_अन्तः = Cur;
        यावत् (Cur != अन्तः_2 && *Cur != '\n')
          ++Cur;
        भङ्ग;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    पङ्क्तिः.append(Start, पङ्क्तिः_अन्तः ? पङ्क्तिः_अन्तः : Cur);
    cl::Tokenize_GNUCommand_पङ्क्तिः(पङ्क्तिः, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
स्थिर तार्किक has_UTF8_Byte_क्रमः_Mark(ArrayRef<वर्ण> S) {
  प्रत्यागम (S.आकार_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
स्थिर शून्य Expand_आधारः_Paths(StringRef आधारः_मार्गः, StringSaver &Saver,
                            ध्रुव वर्ण *&Arg) {
  प्रतिज्ञा(sys::मार्गः_2::is_absolute(आधारः_मार्गः));
  constexpr StringLiteral Token("<CFGDIR>");
  ध्रुव StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::आकार_type StartPos = 0;
  प्रति (StringRef::आकार_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ध्रुव StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    यदि (ResponseFile.रिक्तानि())
      ResponseFile = LHS;
    अन्यथा
      llvm::sys::मार्गः_2::append(ResponseFile, LHS);
    ResponseFile.append(आधारः_मार्गः);
    StartPos = TokenPos + Token.आकार_2();
  }

  यदि (!ResponseFile.रिक्तानि()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ध्रुव StringRef Remaining = ArgString.substr(StartPos);
    यदि (!Remaining.रिक्तानि())
      llvm::sys::मार्गः_2::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).दत्त();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv) {
  प्रतिज्ञा(sys::मार्गः_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<स्मृतिः_बफर>> MemBufOrErr =
      FS->get_बफर_For_File(FName);
  यदि (!MemBufOrErr) {
    std::error_संहिता EC = MemBufOrErr.getError();
    प्रत्यागम llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.सन्देशः_2());
  }
  स्मृतिः_बफर &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_बफर_Start(), MemBuf.get_बफर_आकार());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<वर्ण> BufRef(MemBuf.get_बफर_Start(), MemBuf.get_बफर_अन्तः());
  std::string UTF8Buf;
  यदि (has_UTF16_Byte_क्रमः_Mark(BufRef)) {
    यदि (!convert_UTF16_गन्तव्यम्_UTF8_String(BufRef, UTF8Buf))
      प्रत्यागम llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  अन्यथा यदि (has_UTF8_Byte_क्रमः_Mark(BufRef))
    Str = StringRef(BufRef.दत्त() + 3, BufRef.आकार_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  यदि (!Relative_नामानि && !अन्तर्_Config_File)
    प्रत्यागम Error::success();

  StringRef आधारः_मार्गः = llvm::sys::मार्गः_2::parent_मार्गः(FName);
  प्रति (ध्रुव वर्ण *&Arg : NewArgv) {
    यदि (!Arg)
      अनुवर्त;

    // Substitute <CFGDIR> with the file's base path.
    यदि (अन्तर्_Config_File)
      Expand_आधारः_Paths(आधारः_मार्गः, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    तार्किक ConfigInclusion = असत्य;
    यदि (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      यदि (!llvm::sys::मार्गः_2::is_relative(FileName))
        अनुवर्त;
    } अन्यथा यदि (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = सत्य;
    } अन्यथा {
      अनुवर्त;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.प्रक्षिप_पृष्ठभागः('@');
    यदि (ConfigInclusion && !llvm::sys::मार्गः_2::has_parent_मार्गः(FileName)) {
      SmallString<128> File_मार्गः;
      यदि (!findConfigFile(FileName, File_मार्गः))
        प्रत्यागम createStringError(
            std::make_error_संहिता(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_मार्गः);
    } अन्यथा {
      ResponseFile.append(आधारः_मार्गः);
      llvm::sys::मार्गः_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).दत्त();
  }
  प्रत्यागम Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_सदिशम्_Impl<ध्रुव वर्ण *> &Argv) {
  संरचना Response_File_लेखः {
    std::string File;
    size_t अन्तः_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_सदिशम्<Response_File_लेखः, 3> File_राशिः;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_राशिः.प्रक्षिप_पृष्ठभागः({"", Argv.आकार_2()});

  // Don't cache Argv.size() because it can change.
  प्रति (अचिह्नित I = 0; I != Argv.आकार_2();) {
    यावत् (I == File_राशिः.पृष्ठभागः().अन्तः_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_राशिः.निष्कासय_पृष्ठभागः();
    }

    ध्रुव वर्ण *Arg = Argv[I];
    // Check if it is an EOL marker
    यदि (Arg == शून्यसूचकः) {
      ++I;
      अनुवर्त;
    }

    यदि (Arg[0] != '@') {
      ++I;
      अनुवर्त;
    }

    ध्रुव वर्ण *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    यदि (llvm::sys::मार्गः_2::is_relative(FName)) {
      यदि (CurrentDir.रिक्तानि()) {
        यदि (स्वचालित CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } अन्यथा {
          प्रत्यागम createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } अन्यथा {
        CurrDir = CurrentDir;
      }
      llvm::sys::मार्गः_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::स्थितिः> Res = FS->स्थितिः_2(FName);
    यदि (!Res || !Res->exists()) {
      std::error_संहिता EC = Res.getError();
      यदि (!अन्तर्_Config_File) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        यदि (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          अनुवर्त;
        }
      }
      यदि (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      प्रत्यागम createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.सन्देशः_2());
    }
    ध्रुव llvm::vfs::स्थितिः &File_स्थितिः = Res.get();

    स्वचालित IsEquivalent =
        [File_स्थितिः, एतत्](ध्रुव Response_File_लेखः &RFile) -> ErrorOr<तार्किक> {
      ErrorOr<llvm::vfs::स्थितिः> RHS = FS->स्थितिः_2(RFile.File);
      यदि (!RHS)
        प्रत्यागम RHS.getError();
      प्रत्यागम File_स्थितिः.equivalent(*RHS);
    };

    // Check for recursive response files.
    प्रति (ध्रुव स्वचालित &F : drop_begin(File_राशिः)) {
      यदि (ErrorOr<तार्किक> R = IsEquivalent(F)) {
        यदि (R.get())
          प्रत्यागम createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } अन्यथा {
        प्रत्यागम createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    Small_सदिशम्<ध्रुव वर्ण *, 0> ExpandedArgv;
    यदि (Error Err = expandResponseFile(FName, ExpandedArgv))
      प्रत्यागम Err;

    प्रति (Response_File_लेखः &लेखः : File_राशिः) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      लेखः.अन्तः_2 += ExpandedArgv.आकार_2() - 1;
    }

    File_राशिः.प्रक्षिप_पृष्ठभागः({FName, I + ExpandedArgv.आकार_2()});
    Argv.erase(Argv.begin() + I);
    Argv.निवेशय(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.अन्तः());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  प्रतिज्ञा(File_राशिः.आकार_2() > 0 && Argv.आकार_2() == File_राशिः.पृष्ठभागः().अन्तः_2);
  प्रत्यागम Error::success();
}

तार्किक cl::expandResponseFiles(पूर्णाङ्क Argc, ध्रुव वर्ण *ध्रुव *Argv,
                             ध्रुव वर्ण *EnvVar, StringSaver &Saver,
                             Small_सदिशम्_Impl<ध्रुव वर्ण *> &NewArgv) {
#यदि_परिभाषितम् _WIN32
  स्वचालित Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अन्यथा
  स्वचालित Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#यदि_अन्तः
  // The environment variable specifies initial options.
  यदि (EnvVar)
    यदि (std::optional<std::string> Env_मानम् = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_मानम्, Saver, NewArgv, /*MarkEOLs=*/असत्य);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  यदि (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << गन्तव्यम्_String(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_सदिशम्_Impl<ध्रुव वर्ण *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  यदि (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << गन्तव्यम्_String(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

तार्किक ExpansionContext::findConfigFile(StringRef FileName,
                                      Small_सदिशम्_Impl<वर्ण> &File_मार्गः) {
  SmallString<128> Cfg_File_मार्गः;
  ध्रुव स्वचालित FileExists = [एतत्](SmallString<128> मार्गः) -> तार्किक {
    स्वचालित स्थितिः = FS->स्थितिः_2(मार्गः);
    प्रत्यागम स्थितिः &&
           स्थितिः->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  यदि (llvm::sys::मार्गः_2::has_parent_मार्गः(FileName)) {
    Cfg_File_मार्गः = FileName;
    यदि (llvm::sys::मार्गः_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_मार्गः))
      प्रत्यागम असत्य;
    यदि (!FileExists(Cfg_File_मार्गः))
      प्रत्यागम असत्य;
    File_मार्गः.नियोजय(Cfg_File_मार्गः.begin(), Cfg_File_मार्गः.अन्तः());
    प्रत्यागम सत्य;
  }

  // Look for the file in search directories.
  प्रति (ध्रुव StringRef &Dir : अन्वेषणम्_Dirs) {
    यदि (Dir.रिक्तानि())
      अनुवर्त;
    Cfg_File_मार्गः.नियोजय(Dir);
    llvm::sys::मार्गः_2::append(Cfg_File_मार्गः, FileName);
    llvm::sys::मार्गः_2::native(Cfg_File_मार्गः);
    यदि (FileExists(Cfg_File_मार्गः)) {
      File_मार्गः.नियोजय(Cfg_File_मार्गः.begin(), Cfg_File_मार्गः.अन्तः());
      प्रत्यागम सत्य;
    }
  }

  प्रत्यागम असत्य;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_सदिशम्_Impl<ध्रुव वर्ण *> &Argv) {
  SmallString<128> Abs_मार्गः;
  यदि (sys::मार्गः_2::is_relative(CfgFile)) {
    Abs_मार्गः.नियोजय(CfgFile);
    यदि (std::error_संहिता EC = FS->makeAbsolute(Abs_मार्गः))
      प्रत्यागम make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_मार्गः.str();
  }
  अन्तर्_Config_File = सत्य;
  Relative_नामानि = सत्य;
  यदि (Error Err = expandResponseFile(CfgFile, Argv))
    प्रत्यागम Err;
  प्रत्यागम expandResponseFiles(Argv);
}

स्थिर शून्य initCommonOptions();
स्थिर तार्किक parse_Command_पङ्क्तिः_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ध्रुव वर्ण *> तर्काः,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ध्रुव वर्ण *EnvVar) {
  initCommonOptions();
  Small_सदिशम्<ध्रुव वर्ण *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.प्रक्षिप_पृष्ठभागः(तर्काः[0]);

  // Parse options from environment variable.
  यदि (EnvVar) {
    यदि (std::optional<std::string> Env_मानम् =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_पङ्क्तिः(*Env_मानम्, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, तर्काः.drop_front());
  पूर्णाङ्क NewArgc = static_cast<पूर्णाङ्क>(NewArgv.आकार_2());

  // Parse all options.
  प्रत्यागम globalParser().Parse_Command_पङ्क्तिः_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

तार्किक cl::Parse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_पङ्क्तिः_Options_Impl(शून्यसूचकः, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

तार्किक cl::parse_Command_पङ्क्तिः_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ध्रुव वर्ण *> तर्काः,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ध्रुव वर्ण *EnvVar) {
  प्रत्यागम parse_Command_पङ्क्तिः_Options_Impl(&Context, तर्काः, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
शून्य Command_पङ्क्तिः_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  प्रति (स्वचालित *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_सदिशम्<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.आकार_2());
    प्रति (स्वचालित &O : SC->OptionsMap)
      Opts.प्रक्षिप_पृष्ठभागः(O.second);
    प्रति (Option *O : Opts)
      O->reset();
    प्रति (Option *O : SC->PositionalOpts)
      O->reset();
    यदि (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  प्रति (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

तार्किक Command_पङ्क्तिः_Parser::Parse_Command_पङ्क्तिः_Options(
    पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  प्रतिज्ञा(hasOptions() && "No options specified!");

  क्रमादेशः_Overview = Overview;
  तार्किक IgnoreErrors = Errs;
  यदि (!Errs)
    Errs = &errs();
  यदि (!VFS)
    VFS = vfs::getRealFileSystem().get();
  तार्किक ErrorParsing = असत्य;

  // Expand response files.
  Small_सदिशम्<ध्रुव वर्ण *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#यदि_परिभाषितम् _WIN32
  स्वचालित Tokenize = cl::Tokenize_Windows_Command_पङ्क्तिः;
#अन्यथा
  स्वचालित Tokenize = cl::Tokenize_GNUCommand_पङ्क्तिः;
#यदि_अन्तः
  ExpansionContext ECtx(A, Tokenize, VFS);
  यदि (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << गन्तव्यम्_String(std::move(Err)) << '\n';
    प्रत्यागम असत्य;
  }
  argv = &newArgv[0];
  argc = static_cast<पूर्णाङ्क>(newArgv.आकार_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  क्रमादेशः_Name = std::string(sys::मार्गः_2::filename(StringRef(argv[0])));
  सूचकांक_Library_Options();

  // Check out the positional arguments to collect information about them.
  अचिह्नित अङ्कः_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  तार्किक HasUnlimitedPositionals = असत्य;

  पूर्णाङ्क FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_शिखरम्_Level();
  std::string निकटतमः_Sub_Command_String;
  तार्किक MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  यदि (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), निकटतमः_Sub_Command_String);
    यदि (ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
      FirstArg = 2;
  }
  globalParser().सक्रिय_Sub_Command = ChosenSubCommand;

  प्रतिज्ञा(ChosenSubCommand);
  स्वचालित &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  स्वचालित &PositionalOpts = ChosenSubCommand->PositionalOpts;
  स्वचालित &OptionsMap = ChosenSubCommand->OptionsMap;

  यदि (ConsumeAfterOpt) {
    प्रतिज्ञा(PositionalOpts.आकार_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  यदि (!PositionalOpts.रिक्तानि()) {

    // Calculate how many positional values are _required_.
    तार्किक Unbounded_प्राप्तम् = असत्य;
    प्रति (size_t i = 0, e = PositionalOpts.आकार_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      यदि (Requires_मानम्(Opt))
        ++अङ्कः_Positional_Required;
      अन्यथा यदि (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        यदि (PositionalOpts.आकार_2() > 1) {
          यदि (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = सत्य;
        }
      } अन्यथा यदि (Unbounded_प्राप्तम् && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        यदि (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << क्रमादेशः_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.आकार_2();
        ErrorParsing = सत्य;
      }
      Unbounded_प्राप्तम् |= Eats_Unbounded_सङ्ख्या_सम्बन्धि_मानानि(Opt);
    }
    HasUnlimitedPositionals = Unbounded_प्राप्तम् || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_सदिशम्<std::pair<StringRef, अचिह्नित>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *सक्रिय_Positional_Arg = शून्यसूचकः;

  // Loop over all of the arguments... processing them.
  तार्किक Dash_Dash_प्राप्तम् = असत्य; // Have we read '--'?
  प्रति (पूर्णाङ्क i = FirstArg; i < argc; ++i) {
    Option *Handler = शून्यसूचकः;
    std::string निकटतमः_Handler_String;
    StringRef मानम्;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    यदि (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_प्राप्तम्) {
      // Positional argument!
      यदि (सक्रिय_Positional_Arg) {
        ProvidePositionalOption(सक्रिय_Positional_Arg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }

      यदि (!PositionalOpts.रिक्तानि()) {
        PositionalVals.प्रक्षिप_पृष्ठभागः(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        यदि (PositionalVals.आकार_2() >= अङ्कः_Positional_Required && ConsumeAfterOpt) {
          प्रति (++i; i < argc; ++i)
            PositionalVals.प्रक्षिप_पृष्ठभागः(std::make_pair(StringRef(argv[i]), i));
          भङ्ग; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        अनुवर्त;
      }
    } अन्यथा यदि (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_प्राप्तम्) {
      Dash_Dash_प्राप्तम् = सत्य; // This is the mythical "--"?
      अनुवर्त;             // Don't try to process it as an argument itself.
    } अन्यथा यदि (सक्रिय_Positional_Arg &&
               (सक्रिय_Positional_Arg->getMiscFlags() & Positional_Eats_तर्काः)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);
      यदि (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(सक्रिय_Positional_Arg, StringRef(argv[i]), i);
        अनुवर्त; // We are done!
      }
    } अन्यथा { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, मानम्);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      यदि (!Handler && ChosenSubCommand != &SubCommand::get_शिखरम्_Level())
        Handler = LookupOption(SubCommand::get_शिखरम्_Level(), ArgName, मानम्);

      यदि (!Handler) {
        यदि (LibraryOptions *L = lookupLibraryOption(ArgName.विभज('=').first)) {
          ArrayRef<ध्रुव वर्ण *> Rest(argv + i, argc - i);
          अचिह्नित N;
          यदि (Error E = Context && L->is_प्रत्येकम्_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << क्रमादेशः_Name << ": " << गन्तव्यम्_String(std::move(E)) << '\n';
            ErrorParsing = सत्य;
          }
          i += N - 1;
          अनुवर्त;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      यदि (!Handler)
        Handler = HandlePrefixedOption(ArgName, मानम्, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      यदि (!Handler)
        Lookup_निकटतमः_Option(ArgName, OptionsMap, निकटतमः_Handler_String);
    }

    यदि (!Handler) {
      स्वचालित Report_Unknown_तर्कः = [&](तार्किक IsArg,
                                       StringRef निकटतमः_तर्कः_Name) {
        *Errs << क्रमादेशः_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        यदि (निकटतमः_तर्कः_Name.रिक्तानि())
          प्रत्यागम;

        *Errs << क्रमादेशः_Name << ": Did you mean '";
        यदि (IsArg)
          *Errs << PrintArg(निकटतमः_तर्कः_Name, 0);
        अन्यथा
          *Errs << निकटतमः_तर्कः_Name;
        *Errs << "'?\n";
      };

      यदि (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_तर्कः(/*IsArg=*/सत्य, निकटतमः_Handler_String);
      अन्यथा
        Report_Unknown_तर्कः(/*IsArg=*/असत्य, निकटतमः_Sub_Command_String);

      ErrorParsing = सत्य;
      अनुवर्त;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    यदि (Handler->getFormattingFlag() == cl::Positional) {
      यदि ((Handler->getMiscFlags() & Positional_Eats_तर्काः) && !मानम्.रिक्तानि()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = सत्य;
      }
      सक्रिय_Positional_Arg = Handler;
    }
    अन्यथा
      ErrorParsing |= ProvideOption(Handler, ArgName, मानम्, argc, argv, i);
  }

  // Check and handle positional arguments now...
  यदि (अङ्कः_Positional_Required > PositionalVals.आकार_2()) {
      *Errs << क्रमादेशः_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << अङ्कः_Positional_Required
             << " positional argument" << (अङ्कः_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = सत्य;
  } अन्यथा यदि (!HasUnlimitedPositionals &&
             PositionalVals.आकार_2() > PositionalOpts.आकार_2()) {
    *Errs << क्रमादेशः_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.आकार_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = सत्य;

  } अन्यथा यदि (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    अचिह्नित ValNo = 0, अङ्कः_Vals = static_cast<अचिह्नित>(PositionalVals.आकार_2());
    प्रति (Option *Opt : PositionalOpts) {
      यदि (Requires_मानम्(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --अङ्कः_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      तार्किक समाप्तम् = Opt->get_अङ्कः_Occurrences_Flag() == cl::Required;
      यावत् (अङ्कः_Vals - ValNo > अङ्कः_Positional_Required && !समाप्तम्) {
        विकल्प (Opt->get_अङ्कः_Occurrences_Flag()) {
        पक्ष cl::Optional:
          समाप्तम् = सत्य; // Optional arguments want _at most_ one value
          [[fallthrough]];
        पक्ष cl::Zero_Or_अधिकम्: // Zero or more will take all they can get...
        पक्ष cl::One_Or_अधिकम्:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          भङ्ग;
        स्वभाव:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } अन्यथा {
    प्रतिज्ञा(ConsumeAfterOpt && अङ्कः_Positional_Required <= PositionalVals.आकार_2());
    अचिह्नित ValNo = 0;
    प्रति (Option *Opt : PositionalOpts)
      यदि (Requires_मानम्(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    यदि (PositionalOpts.आकार_2() == 1 && ValNo == 0 && !PositionalVals.रिक्तानि()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    प्रति (; ValNo != PositionalVals.आकार_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  प्रति (ध्रुव स्वचालित &Opt : OptionsMap) {
    विकल्प (Opt.second->get_अङ्कः_Occurrences_Flag()) {
    पक्ष Required:
    पक्ष One_Or_अधिकम्:
      यदि (Opt.second->get_अङ्कः_Occurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = सत्य;
      }
      [[fallthrough]];
    स्वभाव:
      भङ्ग;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             प्रति (पूर्णाङ्क i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  अधिकम्_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  यदि (ErrorParsing) {
    यदि (!IgnoreErrors)
      निर्गच्छ(1);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

तार्किक Option::error(ध्रुव Twine &सन्देशः_3, StringRef ArgName, raw_ostream &Errs) {
  यदि (!ArgName.दत्त())
    ArgName = ArgStr;
  यदि (ArgName.रिक्तानि())
    Errs << HelpStr; // Be nice for positional arguments
  अन्यथा
    Errs << globalParser().क्रमादेशः_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << सन्देशः_3 << "\n";
  प्रत्यागम सत्य;
}

तार्किक Option::सङ्कलय_Occurrence(अचिह्नित pos, StringRef ArgName, StringRef मानम्) {
  ++अङ्कः_Occurrences;
  प्रत्यागम handleOccurrence(pos, ArgName, मानम्);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
स्थिर StringRef get_मानम्_Str(ध्रुव Option &O, StringRef Default_सन्देशः) {
  यदि (O.मानम्_Str.रिक्तानि())
    प्रत्यागम Default_सन्देशः;
  प्रत्यागम O.मानम्_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_विस्तारः() ध्रुव {
  प्रत्यागम arg_Plus_Prefixes_आकार(ArgStr);
}

शून्य Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t First_पङ्क्तिः_Indented_By) {
  प्रतिज्ञा(Indent >= First_पङ्क्तिः_Indented_By);
  std::pair<StringRef, StringRef> विभज_2 = HelpStr.विभज('\n');
  outs().indent(Indent - First_पङ्क्तिः_Indented_By)
      << Arg_Help_उपसर्गः << विभज_2.first << "\n";
  यावत् (!विभज_2.second.रिक्तानि()) {
    विभज_2 = विभज_2.second.विभज('\n');
    outs().indent(Indent) << विभज_2.first << "\n";
  }
}

शून्य Option::printEnumValHelpStr(StringRef HelpStr, size_t आधारः_Indent,
                                 size_t First_पङ्क्तिः_Indented_By) {
  ध्रुव StringRef Val_Help_उपसर्गः = "  ";
  प्रतिज्ञा(आधारः_Indent >= First_पङ्क्तिः_Indented_By);
  std::pair<StringRef, StringRef> विभज_2 = HelpStr.विभज('\n');
  outs().indent(आधारः_Indent - First_पङ्क्तिः_Indented_By)
      << Arg_Help_उपसर्गः << Val_Help_उपसर्गः << विभज_2.first << "\n";
  यावत् (!विभज_2.second.रिक्तानि()) {
    विभज_2 = विभज_2.second.विभज('\n');
    outs().indent(आधारः_Indent + Val_Help_उपसर्गः.आकार_2()) << विभज_2.first << "\n";
  }
}

// Print out the option for the alias.
शून्य alias::printOptionInfo(size_t Global_विस्तारः) ध्रुव {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_विस्तारः, arg_Plus_Prefixes_आकार(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_विस्तारः(ध्रुव Option &O) ध्रुव {
  size_t Len = arg_Plus_Prefixes_आकार(O.ArgStr);
  स्वचालित ValName = get_मानम्_Name();
  यदि (!ValName.रिक्तानि()) {
    size_t FormattingLen = 3;
    यदि (O.getMiscFlags() & Positional_Eats_तर्काः)
      FormattingLen = 6;
    Len += get_मानम्_Str(O, ValName).आकार_2() + FormattingLen;
  }

  प्रत्यागम Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य basic_parser_impl::printOptionInfo(ध्रुव Option &O,
                                        size_t Global_विस्तारः) ध्रुव {
  outs() << PrintArg(O.ArgStr);

  स्वचालित ValName = get_मानम्_Name();
  यदि (!ValName.रिक्तानि()) {
    यदि (O.getMiscFlags() & Positional_Eats_तर्काः) {
      outs() << " <" << get_मानम्_Str(O, ValName) << ">...";
    } अन्यथा यदि (O.get_मानम्_Expected_Flag() == मानम्_Optional)
      outs() << "[=<" << get_मानम्_Str(O, ValName) << ">]";
    अन्यथा {
      outs() << (O.ArgStr.आकार_2() == 1 ? " <" : "=<") << get_मानम्_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_विस्तारः, get_Option_विस्तारः(O));
}

शून्य basic_parser_impl::printOptionName(ध्रुव Option &O,
                                        size_t Global_विस्तारः) ध्रुव {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_विस्तारः - O.ArgStr.आकार_2());
}

// parser<bool> implementation
//
तार्किक parser<तार्किक>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         तार्किक &मानम्) {
  प्रत्यागम parseBool<तार्किक, सत्य, असत्य>(O, ArgName, Arg, मानम्);
}

// parser<boolOrDefault> implementation
//
तार्किक parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &मानम्) {
  प्रत्यागम parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, मानम्);
}

// parser<FixedOrScalableQuantity> implementation
//
प्रतिरूपम् <प्रकारनाम FixedOrScalableQuantityT>
स्थिर तार्किक parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef मानम्_विधा,
                                         FixedOrScalableQuantityT &मानम्) {
  उपयुज्य ScalarTy = प्रकारनाम FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_मानम्;
  यदि (!Arg.getAsInteger(0, Min_मानम्)) {
    मानम् = FixedOrScalableQuantityT::getFixed(Min_मानम्);
    प्रत्यागम असत्य;
  }

  StringRef Remainder = Arg;
  यदि (!Remainder.consume_front("vscale"))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_विधा +
                   " argument!");

  Remainder = Remainder.ltrim();
  यदि (!Remainder.consume_front('x'))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_विधा +
                   " argument!");

  Remainder = Remainder.ltrim();
  यदि (Remainder.getAsInteger(0, Min_मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for " + मानम्_विधा +
                   " argument!");

  मानम् = FixedOrScalableQuantityT::getScalable(Min_मानम्);
  प्रत्यागम असत्य;
}

// parser<int> implementation
//
तार्किक parser<पूर्णाङ्क>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        पूर्णाङ्क &मानम्) {
  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for integer argument!");
  प्रत्यागम असत्य;
}

// parser<long> implementation
//
तार्किक parser<दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         दीर्घ &मानम्) {
  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for long argument!");
  प्रत्यागम असत्य;
}

// parser<long long> implementation
//
तार्किक parser<दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              दीर्घ दीर्घ &मानम्) {
  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for llong argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned> implementation
//
तार्किक parser<अचिह्नित>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             अचिह्नित &मानम्) {

  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for uint argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned long> implementation
//
तार्किक parser<अचिह्नित दीर्घ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  अचिह्नित दीर्घ &मानम्) {

  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ulong argument!");
  प्रत्यागम असत्य;
}

// parser<unsigned long long> implementation
//
तार्किक parser<अचिह्नित दीर्घ दीर्घ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       अचिह्नित दीर्घ दीर्घ &मानम्) {

  यदि (Arg.getAsInteger(0, मानम्))
    प्रत्यागम O.error("'" + Arg + "' value invalid for ullong argument!");
  प्रत्यागम असत्य;
}

// parser<ElementCount> implementation
//
तार्किक parser<Element_गणनम्>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_गणनम् &मानम्) {
  प्रत्यागम parseFixedOrScalableQuantity(O, Arg, get_मानम्_Name(), मानम्);
}

// parser<double>/parser<float> implementation
//
स्थिर तार्किक parseDouble(Option &O, StringRef Arg, द्विगुण &मानम्) {
  यदि (गन्तव्यम्_float(Arg, मानम्))
    प्रत्यागम असत्य;
  प्रत्यागम O.error("'" + Arg + "' value invalid for floating point argument!");
}

तार्किक parser<द्विगुण>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           द्विगुण &Val) {
  प्रत्यागम parseDouble(O, Arg, Val);
}

तार्किक parser<भिन्न>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          भिन्न &Val) {
  द्विगुण dVal;
  यदि (parseDouble(O, Arg, dVal))
    प्रत्यागम सत्य;
  Val = (भिन्न)dVal;
  प्रत्यागम असत्य;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
अचिह्नित generic_parser_आधारः::findOption(StringRef Name) {
  अचिह्नित e = get_अङ्कः_Options();

  प्रति (अचिह्नित i = 0; i != e; ++i) {
    यदि (getOption(i) == Name)
      प्रत्यागम i;
  }
  प्रत्यागम e;
}

स्थिर StringRef Eq_मानम् = "=<value>";
स्थिर StringRef रिक्तानि_Option = "<empty>";
स्थिर StringRef Option_उपसर्गः = "    =";
स्थिर size_t get_Option_Prefixes_आकार() {
  प्रत्यागम Option_उपसर्गः.आकार_2() + Arg_Help_उपसर्गः.आकार_2();
}

स्थिर तार्किक shouldPrintOption(StringRef Name, StringRef Description,
                              ध्रुव Option &O) {
  प्रत्यागम O.get_मानम्_Expected_Flag() != मानम्_Optional || !Name.रिक्तानि() ||
         !Description.रिक्तानि();
}

// Return the width of the option tag for printing...
size_t generic_parser_आधारः::get_Option_विस्तारः(ध्रुव Option &O) ध्रुव {
  यदि (O.hasArgStr()) {
    size_t आकार =
        arg_Plus_Prefixes_आकार(O.ArgStr) + Eq_मानम्.आकार_2();
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      यदि (!shouldPrintOption(Name, getDescription(i), O))
        अनुवर्त;
      size_t Name_आकार = Name.रिक्तानि() ? रिक्तानि_Option.आकार_2() : Name.आकार_2();
      आकार = std::max(आकार, Name_आकार + get_Option_Prefixes_आकार());
    }
    प्रत्यागम आकार;
  } अन्यथा {
    size_t आधारः_आकार = 0;
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Options(); i != e; ++i)
      आधारः_आकार = std::max(आधारः_आकार, getOption(i).आकार_2() + 8);
    प्रत्यागम आधारः_आकार;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
शून्य generic_parser_आधारः::printOptionInfo(ध्रुव Option &O,
                                          size_t Global_विस्तारः) ध्रुव {
  यदि (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    यदि (O.get_मानम्_Expected_Flag() == मानम्_Optional) {
      प्रति (अचिह्नित i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
        यदि (getOption(i).रिक्तानि()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_विस्तारः,
                               arg_Plus_Prefixes_आकार(O.ArgStr));
          भङ्ग;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_मानम्;
    Option::printHelpStr(O.HelpStr, Global_विस्तारः,
                         Eq_मानम्.आकार_2() +
                             arg_Plus_Prefixes_आकार(O.ArgStr));
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      यदि (!shouldPrintOption(OptionName, Description, O))
        अनुवर्त;
      size_t First_पङ्क्तिः_Indent = OptionName.आकार_2() + get_Option_Prefixes_आकार();
      outs() << Option_उपसर्गः << OptionName;
      यदि (OptionName.रिक्तानि()) {
        outs() << रिक्तानि_Option;
        प्रतिज्ञा(First_पङ्क्तिः_Indent >= रिक्तानि_Option.आकार_2());
        First_पङ्क्तिः_Indent += रिक्तानि_Option.आकार_2();
      }
      यदि (!Description.रिक्तानि())
        Option::printEnumValHelpStr(Description, Global_विस्तारः, First_पङ्क्तिः_Indent);
      अन्यथा
        outs() << '\n';
    }
  } अन्यथा {
    यदि (!O.HelpStr.रिक्तानि())
      outs() << "  " << O.HelpStr << '\n';
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_विस्तारः, Option.आकार_2() + 8);
    }
  }
}

स्थिर ध्रुव size_t Max_Opt_विस्तारः = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
शून्य generic_parser_आधारः::printGenericOptionDiff(
    ध्रुव Option &O, ध्रुव Generic_Option_मानम् &मानम्,
    ध्रुव Generic_Option_मानम् &Default, size_t Global_विस्तारः) ध्रुव {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_विस्तारः - O.ArgStr.आकार_2());

  अचिह्नित अङ्कः_Opts = get_अङ्कः_Options();
  प्रति (अचिह्नित i = 0; i != अङ्कः_Opts; ++i) {
    यदि (!मानम्.compare(get_Option_मानम्(i)))
      अनुवर्त;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).आकार_2();
    size_t अङ्कः_Spaces = Max_Opt_विस्तारः > L ? Max_Opt_विस्तारः - L : 0;
    outs().indent(अङ्कः_Spaces) << " (default: ";
    प्रति (अचिह्नित j = 0; j != अङ्कः_Opts; ++j) {
      यदि (!Default.compare(get_Option_मानम्(j)))
        अनुवर्त;
      outs() << getOption(j);
      भङ्ग;
    }
    outs() << ")\n";
    प्रत्यागम;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
नामाकाश llvm {
नामाकाश cl {
स्थिर raw_ostream &संक्रियकः<<(raw_ostream &OS, boolOrDefault V) {
  प्रत्यागम OS << static_cast<पूर्णाङ्क>(V);
}
} // namespace cl
} // namespace llvm

#परिभाषय PRINT_OPT_DIFF(T)                                                      \
  शून्य parser<T>::printOptionDiff(ध्रुव Option &O, T V, Option_मानम्<T> D,      \
                                  size_t Global_विस्तारः) ध्रुव {                  \
    printOptionName(O, Global_विस्तारः);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t अङ्कः_Spaces =                                                         \
        Max_Opt_विस्तारः > Str.आकार_2() ? Max_Opt_विस्तारः - Str.आकार_2() : 0;               \
    outs().indent(अङ्कः_Spaces) << " (default: ";                                 \
    यदि (D.has_मानम्())                                                          \
      outs() << D.get_मानम्();                                                  \
    अन्यथा                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(तार्किक)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(पूर्णाङ्क)
PRINT_OPT_DIFF(दीर्घ)
PRINT_OPT_DIFF(दीर्घ दीर्घ)
PRINT_OPT_DIFF(अचिह्नित)
PRINT_OPT_DIFF(अचिह्नित दीर्घ)
PRINT_OPT_DIFF(अचिह्नित दीर्घ दीर्घ)
PRINT_OPT_DIFF(द्विगुण)
PRINT_OPT_DIFF(भिन्न)
PRINT_OPT_DIFF(वर्ण)
PRINT_OPT_DIFF(Element_गणनम्)

शून्य parser<std::string>::printOptionDiff(ध्रुव Option &O, StringRef V,
                                          ध्रुव Option_मानम्<std::string> &D,
                                          size_t Global_विस्तारः) ध्रुव {
  printOptionName(O, Global_विस्तारः);
  outs() << "= " << V;
  size_t अङ्कः_Spaces = Max_Opt_विस्तारः > V.आकार_2() ? Max_Opt_विस्तारः - V.आकार_2() : 0;
  outs().indent(अङ्कः_Spaces) << " (default: ";
  यदि (D.has_मानम्())
    outs() << D.get_मानम्();
  अन्यथा
    outs() << "*no default*";
  outs() << ")\n";
}

शून्य parser<std::optional<std::string>>::printOptionDiff(
    ध्रुव Option &O, std::optional<StringRef> V,
    ध्रुव Option_मानम्<std::optional<std::string>> &D,
    size_t Global_विस्तारः) ध्रुव {
  printOptionName(O, Global_विस्तारः);
  outs() << "= " << V;
  size_t VSize = V.has_मानम्_2() ? V.मानम्_2().आकार_2() : 0;
  size_t अङ्कः_Spaces = Max_Opt_विस्तारः > VSize ? Max_Opt_विस्तारः - VSize : 0;
  outs().indent(अङ्कः_Spaces) << " (default: ";
  यदि (D.has_मानम्() && D.get_मानम्().has_मानम्_2())
    outs() << D.get_मानम्();
  अन्यथा
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
शून्य basic_parser_impl::print_Option_No_मानम्(ध्रुव Option &O,
                                           size_t Global_विस्तारः) ध्रुव {
  printOptionName(O, Global_विस्तारः);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

स्थिर पूर्णाङ्क OptNameCompare(ध्रुव std::pair<ध्रुव वर्ण *, Option *> *LHS,
                          ध्रुव std::pair<ध्रुव वर्ण *, Option *> *RHS) {
  प्रत्यागम वाक्यं_तुलय(LHS->first, RHS->first);
}

स्थिर पूर्णाङ्क SubNameCompare(ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *LHS,
                          ध्रुव std::pair<ध्रुव वर्ण *, SubCommand *> *RHS) {
  प्रत्यागम वाक्यं_तुलय(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
स्थिर शून्य क्रमय_Opts(OptionsMapTy &OptMap,
                     Small_सदिशम्_Impl<std::pair<ध्रुव वर्ण *, Option *>> &Opts,
                     तार्किक ShowHidden) {
  Small_Ptr_समुच्चयः<Option *, 32> Option_समुच्चयः; // Duplicate option detection.

  प्रति (स्वचालित I = OptMap.begin(), E = OptMap.अन्तः(); I != E; ++I) {
    // Ignore really-hidden options.
    यदि (I->second->getOptionHiddenFlag() == ReallyHidden)
      अनुवर्त;

    // Unless showhidden is set, ignore hidden flags.
    यदि (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      अनुवर्त;

    // If we've already seen this option, don't add it to the list again.
    यदि (!Option_समुच्चयः.निवेशय(I->second).second)
      अनुवर्त;

    Opts.प्रक्षिप_पृष्ठभागः(
        std::pair<ध्रुव वर्ण *, Option *>(I->first.दत्त(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_क्रमय(Opts.begin(), Opts.अन्तः(), OptNameCompare);
}

स्थिर शून्य
क्रमय_Sub_Commands(ध्रुव Small_Ptr_समुच्चयः_Impl<SubCommand *> &SubMap,
                Small_सदिशम्_Impl<std::pair<ध्रुव वर्ण *, SubCommand *>> &Subs) {
  प्रति (स्वचालित *S : SubMap) {
    यदि (S->getName().रिक्तानि())
      अनुवर्त;
    Subs.प्रक्षिप_पृष्ठभागः(std::make_pair(S->getName().दत्त(), S));
  }
  array_pod_क्रमय(Subs.begin(), Subs.अन्तः(), SubNameCompare);
}

नामाकाश {

वर्ग HelpPrinter {
रक्षित:
  ध्रुव तार्किक ShowHidden;
  उपयुज्य Str_Option_Pair_सदिशम् =
      Small_सदिशम्<std::pair<ध्रुव वर्ण *, Option *>, 128>;
  उपयुज्य Str_Sub_Command_Pair_सदिशम् =
      Small_सदिशम्<std::pair<ध्रुव वर्ण *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  आभास शून्य printOptions(Str_Option_Pair_सदिशम् &Opts, size_t MaxArgLen) {
    प्रति (ध्रुव स्वचालित &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  शून्य printSubCommands(Str_Sub_Command_Pair_सदिशम् &Subs, size_t MaxSubLen) {
    प्रति (ध्रुव स्वचालित &S : Subs) {
      outs() << "  " << S.first;
      यदि (!S.second->getDescription().रिक्तानि()) {
        outs().indent(MaxSubLen - वाक्यदैर्घ्यम्(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

सार्वजनिक:
  स्पष्ट HelpPrinter(तार्किक showHidden) : ShowHidden(showHidden) {}
  आभास ~HelpPrinter() = स्वभाव;

  // Invoke the printer.
  शून्य संक्रियकः=(तार्किक मानम्) {
    यदि (!मानम्)
      प्रत्यागम;
    printHelp();

    // Halt the program since help information was printed
    निर्गच्छ(0);
  }

  शून्य printHelp() {
    SubCommand *Sub = globalParser().get_सक्रिय_Sub_Command();
    स्वचालित &OptionsMap = Sub->OptionsMap;
    स्वचालित &PositionalOpts = Sub->PositionalOpts;
    स्वचालित &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_सदिशम् Opts;
    क्रमय_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_सदिशम् Subs;
    क्रमय_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    यदि (!globalParser().क्रमादेशः_Overview.रिक्तानि())
      outs() << "OVERVIEW: " << globalParser().क्रमादेशः_Overview << "\n";

    यदि (Sub == &SubCommand::get_शिखरम्_Level()) {
      outs() << "USAGE: " << globalParser().क्रमादेशः_Name;
      यदि (!Subs.रिक्तानि())
        outs() << " [subcommand]";
      outs() << " [options]";
    } अन्यथा {
      यदि (!Sub->getDescription().रिक्तानि()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().क्रमादेशः_Name << " " << Sub->getName()
             << " [options]";
    }

    प्रति (स्वचालित *Opt : PositionalOpts) {
      यदि (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    यदि (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    यदि (Sub == &SubCommand::get_शिखरम्_Level() && !Subs.रिक्तानि()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      प्रति (ध्रुव स्वचालित &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, वाक्यदैर्घ्यम्(Sub.first));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().क्रमादेशः_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    प्रति (ध्रुव स्वचालित &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->get_Option_विस्तारः());

    Small_सदिशम्<std::pair<std::string, StringRef>, 0> LibraryOpts;
    यदि (ShowHidden)
      प्रति (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              यदि (!Help.रिक्तानि())
                LibraryOpts.emplace_पृष्ठभागः((Spelling + MetaVar).str(), Help);
            });
    llvm::क्रमय(LibraryOpts);
    प्रति (ध्रुव स्वचालित &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_आकार(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    यदि (!LibraryOpts.रिक्तानि())
      outs() << "\nLibrary options:\n\n";
    प्रति (ध्रुव स्वचालित &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_आकार(Name));
    }

    // Print any extra help the user has declared.
    प्रति (ध्रुव स्वचालित &I : globalParser().अधिकम्_Help)
      outs() << I;
    globalParser().अधिकम्_Help.clear();
  }
};

वर्ग CategorizedHelpPrinter : सार्वजनिक HelpPrinter {
सार्वजनिक:
  स्पष्ट CategorizedHelpPrinter(तार्किक showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  स्थिर पूर्णाङ्क OptionCategoryCompare(OptionCategory *ध्रुव *A,
                                   OptionCategory *ध्रुव *B) {
    प्रत्यागम (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  उपयुज्य HelpPrinter::संक्रियकः=;

रक्षित:
  शून्य printOptions(Str_Option_Pair_सदिशम् &Opts, size_t MaxArgLen) override {
    std::सदिशम्<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::सदिशम्<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    प्रतिज्ञा(SortedCategories.आकार_2() > 0 && "No option categories registered!");
    array_pod_क्रमय(SortedCategories.begin(), SortedCategories.अन्तः(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    प्रति (ध्रुव स्वचालित &I : Opts) {
      Option *Opt = I.second;
      प्रति (OptionCategory *Cat : Opt->Categories) {
        प्रतिज्ञा(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].प्रक्षिप_पृष्ठभागः(Opt);
      }
    }

    // Now do printing.
    प्रति (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ध्रुव स्वचालित &CategoryOptions = CategorizedOptions[Category];
      यदि (CategoryOptions.रिक्तानि())
        अनुवर्त;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      यदि (!Category->getDescription().रिक्तानि())
        outs() << Category->getDescription() << "\n\n";
      अन्यथा
        outs() << "\n";

      // Loop over the options in the category and print.
      प्रति (ध्रुव Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
वर्ग HelpPrinterWrapper {
गुप्त:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

सार्वजनिक:
  स्पष्ट HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  शून्य संक्रियकः=(तार्किक मानम्);
};

} // End anonymous namespace

#यदि defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# यदि defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#नोचेद्यदि defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# यदि defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# अन्यथा
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#अन्यथा
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#यदि_अन्तः

नामाकाश {
वर्ग संस्करणम्_Printer {
सार्वजनिक:
  शून्य print(ध्रुव std::सदिशम्<संस्करणम्_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#यदि_परिभाषितम् PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#अन्यथा
    OS << "LLVM (http://llvm.org/):\n  ";
#यदि_अन्तः
    OS << PACKAGE_NAME << " version " << PACKAGE_संस्करणम् << "\n  ";
#यदि LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#अन्यथा
    OS << "Optimized build";
#यदि_अन्तः
#यदि_अपरिभाषितम् NDEBUG
    OS << " with assertions";
#यदि_अन्तः
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    यदि (!ExtraPrinters.रिक्तानि()) {
      प्रति (ध्रुव स्वचालित &I : ExtraPrinters)
        I(outs());
    }
  }
  शून्य संक्रियकः=(तार्किक OptionWasSpecified);
};

संरचना Command_पङ्क्तिः_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_प्रसामान्यम्_Printer{असत्य};
  HelpPrinter UncategorizedHiddenPrinter{सत्य};
  CategorizedHelpPrinter Categorized_प्रसामान्यम्_Printer{असत्य};
  CategorizedHelpPrinter CategorizedHiddenPrinter{सत्य};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_प्रसामान्यम्_Printer{Uncategorized_प्रसामान्यम्_Printer,
                                          Categorized_प्रसामान्यम्_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, सत्य, parser<तार्किक>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_प्रसामान्यम्_Printer),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, सत्य, parser<तार्किक>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, सत्य, parser<तार्किक>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_प्रसामान्यम्_Printer),
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, सत्य, parser<तार्किक>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::मानम्_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(असत्य),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<तार्किक> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(असत्य),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  संस्करणम्_Printer_Ty Override_संस्करणम्_Printer = शून्यसूचकः;

  std::सदिशम्<संस्करणम्_Printer_Ty> Extra_संस्करणम्_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  संस्करणम्_Printer संस्करणम्_Printer_Instance;

  cl::opt<संस्करणम्_Printer, सत्य, parser<तार्किक>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(संस्करणम्_Printer_Instance), cl::मानम्_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
स्थिर ManagedStatic<Command_पङ्क्तिः_Common_Options> CommonOptions;

स्थिर शून्य initCommonOptions() {
  *CommonOptions;
  init_Debug_गणक_Options();
  initGraphWriterOptions();
  init_सङ्केताः_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_बीजम्_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  स्थिर OptionCategory GeneralCategory{"General options"};
  प्रत्यागम GeneralCategory;
}

शून्य संस्करणम्_Printer::संक्रियकः=(तार्किक OptionWasSpecified) {
  यदि (!OptionWasSpecified)
    प्रत्यागम;

  यदि (CommonOptions->Override_संस्करणम्_Printer != शून्यसूचकः) {
    CommonOptions->Override_संस्करणम्_Printer(outs());
    निर्गच्छ(0);
  }
  print(CommonOptions->Extra_संस्करणम्_Printers);

  निर्गच्छ(0);
}

शून्य HelpPrinterWrapper::संक्रियकः=(तार्किक मानम्) {
  यदि (!मानम्)
    प्रत्यागम;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  यदि (globalParser().RegisteredOptionCategories.आकार_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.समुच्चयः_Hidden_Flag(NotHidden);

    CategorizedPrinter = सत्य; // Invoke categorized printer
  } अन्यथा {
    UncategorizedPrinter = सत्य; // Invoke uncategorized printer
  }
}

// Print the value of each option.
शून्य cl::Print_Option_मानानि() { globalParser().print_Option_मानानि(); }

शून्य Command_पङ्क्तिः_Parser::print_Option_मानानि() {
  यदि (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    प्रत्यागम;

  Small_सदिशम्<std::pair<ध्रुव वर्ण *, Option *>, 128> Opts;
  क्रमय_Opts(सक्रिय_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ सत्य);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  प्रति (ध्रुव स्वचालित &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->get_Option_विस्तारः());

  प्रति (ध्रुव स्वचालित &Opt : Opts)
    Opt.second->print_Option_मानम्(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
शून्य cl::Print_Help_सन्देशः(तार्किक Hidden, तार्किक Categorized) {
  यदि (!Hidden && !Categorized)
    CommonOptions->Uncategorized_प्रसामान्यम्_Printer.printHelp();
  अन्यथा यदि (!Hidden && Categorized)
    CommonOptions->Categorized_प्रसामान्यम्_Printer.printHelp();
  अन्यथा यदि (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  अन्यथा
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  स्थिर ध्रुव StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#यदि LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#यदि_अन्तः
#यदि_अपरिभाषितम् NDEBUG
      "+assertions",
#यदि_अन्तः
#यदि_परिभाषितम् EXPENSIVE_CHECKS
      "+expensive-checks",
#यदि_अन्तः
#यदि __has_feature(स्थानाङ्कः_sanitizer)
      "+asan",
#यदि_अन्तः
#यदि __has_feature(dataflow_sanitizer)
      "+dfsan",
#यदि_अन्तः
#यदि __has_feature(hwaddress_sanitizer)
      "+hwasan",
#यदि_अन्तः
#यदि __has_feature(स्मृतिः_sanitizer)
      "+msan",
#यदि_अन्तः
#यदि __has_feature(thread_sanitizer)
      "+tsan",
#यदि_अन्तः
#यदि __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#यदि_अन्तः
#यदि_परिभाषितम् LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#यदि_अन्तः
  };
  प्रत्यागम ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
शून्य cl::printBuildConfig(raw_ostream &OS) {
#यदि LLVM_संस्करणम्_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#यदि_अन्तः
}

/// Utility function for printing version number.
शून्य cl::Print_संस्करणम्_सन्देशः() {
  CommonOptions->संस्करणम्_Printer_Instance.print(CommonOptions->Extra_संस्करणम्_Printers);
}

शून्य cl::समुच्चयः_संस्करणम्_Printer(संस्करणम्_Printer_Ty func) {
  CommonOptions->Override_संस्करणम्_Printer = func;
}

शून्य cl::सङ्कलय_Extra_संस्करणम्_Printer(संस्करणम्_Printer_Ty func) {
  CommonOptions->Extra_संस्करणम्_Printers.प्रक्षिप_पृष्ठभागः(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  स्वचालित &Subs = globalParser().RegisteredSubCommands;
  (शून्य)Subs;
  प्रतिज्ञा(Subs.अन्तर्भवति(&Sub));
  प्रत्यागम Sub.OptionsMap;
}

iterator_range<Small_Ptr_समुच्चयः<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  प्रत्यागम globalParser().getRegisteredSubcommands();
}

शून्य cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  प्रति (स्वचालित &I : Sub.OptionsMap) {
    तार्किक Unrelated = सत्य;
    प्रति (स्वचालित &Cat : I.second->Categories) {
      यदि (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    यदि (Unrelated)
      I.second->समुच्चयः_Hidden_Flag(cl::ReallyHidden);
  }
}

शून्य cl::HideUnrelatedOptions(ArrayRef<ध्रुव cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  प्रति (स्वचालित &I : Sub.OptionsMap) {
    तार्किक Unrelated = सत्य;
    प्रति (स्वचालित &Cat : I.second->Categories) {
      यदि (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = असत्य;
    }
    यदि (Unrelated)
      I.second->समुच्चयः_Hidden_Flag(cl::ReallyHidden);
  }
}

शून्य cl::Reset_Command_पङ्क्तिः_Parser() { globalParser().reset(); }

शून्य cl::सङ्कलय_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.प्रक्षिप_पृष्ठभागः(&L);
}

शून्य LibraryOptionsContext::नियोजय(ध्रुव LibraryOptionsContext &Other) {
  प्रतिज्ञा(एतत् != &Other);
  मानानि.clear();
  Saved_तर्काः.clear();
  ArrayRef<ध्रुव वर्ण *> तर्काः = Other.Saved_तर्काः;
  प्रति (size_t I = 0, E = तर्काः.आकार_2(); I != E;) {
    StringRef Name = तर्काः[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.विभज('=').first);
    अचिह्नित N;
    cantFail(parse(*L, तर्काः.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ध्रुव वर्ण *> तर्काः,
                                   अचिह्नित &Consumed) {
  स्वचालित [It, Inserted] = मानानि.try_emplace(L.getDefault());
  यदि (Inserted)
    It->second = L.copyDefault(Alloc);
  प्रत्यागम parseLibraryOption(L, It->second, तर्काः, Consumed, Alloc, &Saved_तर्काः);
}

शून्य cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

शून्य LLVMParse_Command_पङ्क्तिः_Options(पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव *argv,
                                 ध्रुव वर्ण *Overview) {
  llvm::cl::Parse_Command_पङ्क्तिः_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
