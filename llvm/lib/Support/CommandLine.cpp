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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm-c/Support.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLFunctionalExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallPtrSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LibraryOptionsContext.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/StringSaver.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TypeSize.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualFileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ cl;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ cl {
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਤਾਰਕਿਕ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਪੂਰਨ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਲੰਮਾ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਲੰਮਾ ਲੰਮਾ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਬਿਨਾਨਿਸ਼ਾਨੀ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਦੁੱਗਣਾ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਦਸ਼ਮਲਵ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<ਅੱਖਰ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE basic_parser<Element_ਗਿਣਤੀ>;

#ਜੇ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE opt<std::string>;
#ਜੇ_ਅੰਤ

ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE opt<ਤਾਰਕਿਕ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE opt<ਅੱਖਰ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE opt<ਪੂਰਨ>;
ਸਾਂਚਾ ਵਰਗ LLVM_EXPORT_TEMPLATE opt<ਬਿਨਾਨਿਸ਼ਾਨੀ>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
ਖਾਲੀ Generic_Option_ਮੁੱਲ::anchor() {}
ਖਾਲੀ Option_ਮੁੱਲ<boolOrDefault>::anchor() {}
ਖਾਲੀ Option_ਮੁੱਲ<std::string>::anchor() {}
ਖਾਲੀ Option::anchor() {}
ਖਾਲੀ basic_parser_impl::anchor() {}
ਖਾਲੀ parser<ਤਾਰਕਿਕ>::anchor() {}
ਖਾਲੀ parser<boolOrDefault>::anchor() {}
ਖਾਲੀ parser<ਪੂਰਨ>::anchor() {}
ਖਾਲੀ parser<ਲੰਮਾ>::anchor() {}
ਖਾਲੀ parser<ਲੰਮਾ ਲੰਮਾ>::anchor() {}
ਖਾਲੀ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ>::anchor() {}
ਖਾਲੀ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ>::anchor() {}
ਖਾਲੀ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ>::anchor() {}
ਖਾਲੀ parser<ਦੁੱਗਣਾ>::anchor() {}
ਖਾਲੀ parser<ਦਸ਼ਮਲਵ>::anchor() {}
ਖਾਲੀ parser<std::string>::anchor() {}
ਖਾਲੀ parser<std::optional<std::string>>::anchor() {}
ਖਾਲੀ parser<ਅੱਖਰ>::anchor() {}
ਖਾਲੀ parser<Element_ਗਿਣਤੀ>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
ਖਾਲੀ opt_bool_anchor() { opt<ਤਾਰਕਿਕ> anchor{""}; }
ਖਾਲੀ opt_char_anchor() { opt<ਅੱਖਰ> anchor{""}; }
ਖਾਲੀ opt_int_anchor() { opt<ਪੂਰਨ> anchor{""}; }
ਖਾਲੀ opt_unsigned_anchor() { opt<ਬਿਨਾਨਿਸ਼ਾਨੀ> anchor{""}; }

//===----------------------------------------------------------------------===//

ਅਚਲ ਸਥਿਰ size_t DefaultPad = 2;

ਸਥਿਰ StringRef Arg_ਅਗੇਤਰ = "-";
ਸਥਿਰ StringRef Arg_ਅਗੇਤਰ_Long = "--";
ਸਥਿਰ StringRef Arg_Help_ਅਗੇਤਰ = " - ";

ਸਥਿਰ size_t arg_Plus_Prefixes_ਮਾਪ(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.ਮਾਪ_2();
  ਜੇ (Len == 1)
    ਵਾਪਸ Len + Pad + Arg_ਅਗੇਤਰ.ਮਾਪ_2() + Arg_Help_ਅਗੇਤਰ.ਮਾਪ_2();
  ਵਾਪਸ Len + Pad + Arg_ਅਗੇਤਰ_Long.ਮਾਪ_2() + Arg_Help_ਅਗੇਤਰ.ਮਾਪ_2();
}

ਸਥਿਰ SmallString<8> arg_ਅਗੇਤਰ(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> ਅਗੇਤਰ;
  ਲਈ (size_t I = 0; I < Pad; ++I) {
    ਅਗੇਤਰ.ਧੱਕੋ_ਪਿੱਠ(' ');
  }
  ਅਗੇਤਰ.append(ArgName.ਮਾਪ_2() > 1 ? Arg_ਅਗੇਤਰ_Long : Arg_ਅਗੇਤਰ);
  ਵਾਪਸ ਅਗੇਤਰ;
}

ਵਰਤੋਂ OptionsMapTy = DenseMap<StringRef, Option *>;

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ PrintArg {
  StringRef ArgName;
  size_t Pad;
ਜਨਤਕ:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  ਮਿੱਤਰ raw_ostream &ਸੰਚਾਲਕ<<(raw_ostream &OS, ਅਚਲ PrintArg &);
};

raw_ostream &ਸੰਚਾਲਕ<<(raw_ostream &OS, ਅਚਲ PrintArg& Arg) {
  OS << arg_ਅਗੇਤਰ(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ਵਾਪਸ OS;
}

ਵਰਗ Command_ਸਤਰ_Parser {
ਜਨਤਕ:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string ਪ੍ਰੋਗਰਾਮ_Name;
  StringRef ਪ੍ਰੋਗਰਾਮ_Overview;

  // This collects additional help to be printed.
  std::ਸਦਿਸ਼<StringRef> ਹੋਰ_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_ਸੈੱਟ<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_ਸੈੱਟ<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_ਸਦਿਸ਼<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_ਸੂਚਕਾਂਕ;
  size_t ਅੰਕ_ਸੂਚਕਾਂਕਿਤ_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ਸਤਰ_Parser() { ਰਜਿਸਟਰ_Sub_Command(&SubCommand::get_ਸਿਖਰ_Level()); }

  ਖਾਲੀ ResetAllOptionOccurrences();

  ਤਾਰਕਿਕ Parse_Command_ਸਤਰ_Options(ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
                               StringRef Overview, raw_ostream *Errs = ਖਾਲੀ_ਸੂਚਕ,
                               vfs::FileSystem *VFS = ਖਾਲੀ_ਸੂਚਕ,
                               LibraryOptionsContext *Context = ਖਾਲੀ_ਸੂਚਕ);

  ਖਾਲੀ forEachSubCommand(Option &Opt, function_ref<ਖਾਲੀ(SubCommand &)> Action) {
    ਜੇ (Opt.Subs.ਖਾਲੀ_ਥਾਵਾਂ()) {
      Action(SubCommand::get_ਸਿਖਰ_Level());
      ਵਾਪਸ;
    }
    ਜੇ (Opt.Subs.ਮਾਪ_2() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      ਲਈ (ਸਵੈਚਾਲਿਤ *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ਵਾਪਸ;
    }
    ਲਈ (ਸਵੈਚਾਲਿਤ *SC : Opt.Subs) {
      ਪ੍ਰਤਿਜ੍ਞਾ(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  ਖਾਲੀ ਜੋੜੋ_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    ਜੇ (Opt.hasArgStr())
      ਵਾਪਸ;
    ਜੇ (!SC->OptionsMap.ਪਾਓ(std::make_pair(Name, &Opt)).ਦੂਜਾ_2) {
      errs() << ਪ੍ਰੋਗਰਾਮ_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  ਖਾਲੀ ਜੋੜੋ_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { ਜੋੜੋ_Literal_Option(Opt, &SC, Name); });
  }

  ਖਾਲੀ ਜੋੜੋ_Option(Option *O, SubCommand *SC) {
    ਤਾਰਕਿਕ HadErrors = ਝੂਠ;
    ਜੇ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      ਜੇ (!SC->OptionsMap.ਪਾਓ(std::make_pair(O->ArgStr, O)).ਦੂਜਾ_2 ||
          (RegisteredSubCommands.ਸ਼ਾਮਲ_ਹੈ(SC) &&
           Library_ਸੂਚਕਾਂਕ.ਸ਼ਾਮਲ_ਹੈ(O->ArgStr))) {
        errs() << ਪ੍ਰੋਗਰਾਮ_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = ਸੱਚ;
      }
    }

    // Remember information about positional options.
    ਜੇ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ਧੱਕੋ_ਪਿੱਠ(O);
    ਨਹੀਂਤਾਂ ਜੇ (O->get_ਅੰਕ_Occurrences_Flag() == cl::ConsumeAfter) {
      ਜੇ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = ਸੱਚ;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    ਜੇ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  ਖਾਲੀ ਜੋੜੋ_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { ਜੋੜੋ_Option(O, &SC); });
  }

  ਖਾਲੀ removeOption(Option *O, SubCommand *SC) {
    Small_ਸਦਿਸ਼<StringRef, 16> Option_ਨਾਂ;
    O->get_Extra_Option_ਨਾਂ(Option_ਨਾਂ);
    ਜੇ (O->hasArgStr())
      Option_ਨਾਂ.ਧੱਕੋ_ਪਿੱਠ(O->ArgStr);

    SubCommand &Sub = *SC;
    ਲਈ (ਸਵੈਚਾਲਿਤ Name : Option_ਨਾਂ) {
      ਸਵੈਚਾਲਿਤ I = Sub.OptionsMap.ਲੱਭੋ(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      ਜੇ (I != Sub.OptionsMap.ਸਿਰਾ() && I->ਦੂਜਾ_2 == O)
        Sub.OptionsMap.erase(I);
    }

    ਜੇ (O->getFormattingFlag() == cl::Positional)
      ਲਈ (ਸਵੈਚਾਲਿਤ *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.ਸਿਰਾ(); ++Opt) {
        ਜੇ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ਰੋਕੋ;
        }
      }
    ਨਹੀਂਤਾਂ ਜੇ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = ਖਾਲੀ_ਸੂਚਕ;
  }

  ਖਾਲੀ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  ਤਾਰਕਿਕ hasOptions(ਅਚਲ SubCommand &Sub) ਅਚਲ {
    ਵਾਪਸ (!Sub.OptionsMap.ਖਾਲੀ_ਥਾਵਾਂ() || !Sub.PositionalOpts.ਖਾਲੀ_ਥਾਵਾਂ() ||
            ਖਾਲੀ_ਸੂਚਕ != Sub.ConsumeAfterOpt);
  }

  ਤਾਰਕਿਕ hasOptions() ਅਚਲ {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ *S : RegisteredSubCommands) {
      ਜੇ (hasOptions(*S))
        ਵਾਪਸ ਸੱਚ;
    }
    ਵਾਪਸ !Libraries.ਖਾਲੀ_ਥਾਵਾਂ();
  }

  ਤਾਰਕਿਕ hasNamedSubCommands() ਅਚਲ {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ *S : RegisteredSubCommands)
      ਜੇ (!S->getName().ਖਾਲੀ_ਥਾਵਾਂ())
        ਵਾਪਸ ਸੱਚ;
    ਵਾਪਸ ਝੂਠ;
  }

  SubCommand *get_ਸਰਗਰਮ_Sub_Command() { ਵਾਪਸ ਸਰਗਰਮ_Sub_Command; }

  ਖਾਲੀ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    ਜੇ (!Sub.OptionsMap.ਪਾਓ(std::make_pair(NewName, O)).ਦੂਜਾ_2) {
      errs() << ਪ੍ਰੋਗਰਾਮ_Name << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  ਖਾਲੀ updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  ਖਾਲੀ print_Option_ਮੁੱਲਾਂ();

  ਖਾਲੀ ਸੂਚਕਾਂਕ_Library_Options() {
    ਤਾਰਕਿਕ HadErrors = ਝੂਠ;
    ਲਈ (; ਅੰਕ_ਸੂਚਕਾਂਕਿਤ_Libraries != Libraries.ਮਾਪ_2(); ++ਅੰਕ_ਸੂਚਕਾਂਕਿਤ_Libraries) {
      LibraryOptions *L = Libraries[ਅੰਕ_ਸੂਚਕਾਂਕਿਤ_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        ਸਵੈਚਾਲਿਤ [It, Inserted] = Library_ਸੂਚਕਾਂਕ.try_emplace(Name, L);
        ਜੇ (Inserted ? none_ਦਾ(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ਵਾਪਸ SC->OptionsMap.ਸ਼ਾਮਲ_ਹੈ(Name);
                               })
                     : It->ਦੂਜਾ_2 == L)
          ਵਾਪਸ;
        errs() << ਪ੍ਰੋਗਰਾਮ_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = ਸੱਚ;
      });
    }
    ਜੇ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    ਸੂਚਕਾਂਕ_Library_Options();
    ਵਾਪਸ Library_ਸੂਚਕਾਂਕ.lookup(Name);
  }

  ਖਾਲੀ ਰਜਿਸਟਰ_Category(OptionCategory *cat) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਗਿਣਤੀ_if(RegisteredOptionCategories,
                    [cat](ਅਚਲ OptionCategory *Category) {
             ਵਾਪਸ cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ਪਾਓ(cat);
  }

  ਖਾਲੀ ਰਜਿਸਟਰ_Sub_Command(SubCommand *sub) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਗਿਣਤੀ_if(RegisteredSubCommands,
                    [sub](ਅਚਲ SubCommand *Sub) {
                      ਵਾਪਸ (!sub->getName().ਖਾਲੀ_ਥਾਵਾਂ()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ਪਾਓ(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ਪ੍ਰਤਿਜ੍ਞਾ(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    ਲਈ (ਸਵੈਚਾਲਿਤ &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.ਦੂਜਾ_2;
      ਜੇ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        ਜੋੜੋ_Option(O, sub);
      ਨਹੀਂਤਾਂ
        ਜੋੜੋ_Literal_Option(*O, sub, E.ਪਹਿਲਾ_2);
    }
  }

  ਖਾਲੀ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_ਸੈੱਟ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ਵਾਪਸ make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.ਸਿਰਾ());
  }

  ਖਾਲੀ reset() {
    ਸਰਗਰਮ_Sub_Command = ਖਾਲੀ_ਸੂਚਕ;
    ਪ੍ਰੋਗਰਾਮ_Name.clear();
    ਪ੍ਰੋਗਰਾਮ_Overview = StringRef();

    ਹੋਰ_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_ਸੂਚਕਾਂਕ.clear();
    ਅੰਕ_ਸੂਚਕਾਂਕਿਤ_Libraries = 0;

    SubCommand::get_ਸਿਖਰ_Level().reset();
    SubCommand::getAll().reset();
    ਰਜਿਸਟਰ_Sub_Command(&SubCommand::get_ਸਿਖਰ_Level());
  }

ਨਿਜੀ:
  SubCommand *ਸਰਗਰਮ_Sub_Command = ਖਾਲੀ_ਸੂਚਕ;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &ਮੁੱਲ);
  SubCommand *LookupSubCommand(StringRef Name, std::string &ਸਭ_ਤੋਂ_ਨੇੜੇ_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ਸਥਿਰ Command_ਸਤਰ_Parser &globalParser() {
  ਸਥਿਰ ManagedStatic<Command_ਸਤਰ_Parser> GlobalParser;
  ਵਾਪਸ *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ਸਥਿਰ Error
parseLibraryOption(LibraryOptions &L, ਖਾਲੀ *Opts, ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ,
                   ਬਿਨਾਨਿਸ਼ਾਨੀ &Consumed, BumpPtrAllocator &Alloc,
                   Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> *ਰਿਕਾਰਡ = ਖਾਲੀ_ਸੂਚਕ) {
  StringSaver Saver(Alloc);
  ਅਚਲ ਅੱਖਰ *Saved[2];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਦਲੀਲਾਂ = std::min<size_t>(ਦਲੀਲਾਂ.ਮਾਪ_2(), 2);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ J = 0; J != ਅੰਕ_ਦਲੀਲਾਂ; ++J)
    Saved[J] = Saver.save(ਦਲੀਲਾਂ[J]).ਅੰਕੜੇ();
  Consumed = 1;
  ਜੇ (Error E = L.parse(Opts, ArrayRef(Saved, ਅੰਕ_ਦਲੀਲਾਂ), Consumed, Alloc))
    ਵਾਪਸ E;
  ਜੇ (ਰਿਕਾਰਡ)
    ਰਿਕਾਰਡ->append(Saved, Saved + Consumed);
  ਵਾਪਸ Error::success();
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T, T TrueVal, T FalseVal>
ਸਥਿਰ ਤਾਰਕਿਕ parseBool(Option &O, StringRef ArgName, StringRef Arg, T &ਮੁੱਲ) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  ਜੇ (!Arg.ਅੰਕੜੇ() || Arg == "true" || Arg == "1") {
    ਮੁੱਲ = TrueVal;
    ਵਾਪਸ ਝੂਠ;
  }

  ਜੇ (Arg == "false" || Arg == "0") {
    ਮੁੱਲ = FalseVal;
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

ਖਾਲੀ cl::ਜੋੜੋ_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().ਜੋੜੋ_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().ਹੋਰ_Help.ਧੱਕੋ_ਪਿੱਠ(Help);
}

Option::Option(ਅੰਕ_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : ਅੰਕ_Occurrences(0), Occurrences(OccurrencesFlag), ਮੁੱਲ(0),
      HiddenFlag(Hidden), Formatting(ਸਾਧਾਰਨ_Formatting), Misc(0),
      FullyInitialized(ਝੂਠ), Position(0) {
  Categories.ਧੱਕੋ_ਪਿੱਠ(&getGeneralCategory());
}

ਖਾਲੀ Option::ਜੋੜੋ_ਦਲੀਲ() {
  globalParser().ਜੋੜੋ_Option(ਇਹ);
  FullyInitialized = ਸੱਚ;
}

ਖਾਲੀ Option::remove_ਦਲੀਲ() { globalParser().removeOption(ਇਹ); }

ਖਾਲੀ Option::ਸੈੱਟ_Arg_Str(StringRef S) {
  ਜੇ (FullyInitialized)
    globalParser().updateArgStr(ਇਹ, S);
  ਪ੍ਰਤਿਜ੍ਞਾ(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

ਖਾਲੀ Option::ਜੋੜੋ_Category(OptionCategory &C) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Categories.ਖਾਲੀ_ਥਾਵਾਂ() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  ਜੇ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  ਨਹੀਂਤਾਂ ਜੇ (!is_contained(Categories, &C))
    Categories.ਧੱਕੋ_ਪਿੱਠ(&C);
}

ਖਾਲੀ Option::reset() {
  ਅੰਕ_Occurrences = 0;
  ਸੈੱਟ_Default();
}

ਖਾਲੀ OptionCategory::ਰਜਿਸਟਰ_Category() {
  globalParser().ਰਜਿਸਟਰ_Category(ਇਹ);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_ਸਿਖਰ_Level() {
  ਸਥਿਰ ManagedStatic<SubCommand> ਸਿਖਰ_Level_Sub_Command;
  ਵਾਪਸ *ਸਿਖਰ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ਸਥਿਰ ManagedStatic<SubCommand> AllSubCommands;
  ਵਾਪਸ *AllSubCommands;
}

ਖਾਲੀ SubCommand::ਰਜਿਸਟਰ_Sub_Command() {
  globalParser().ਰਜਿਸਟਰ_Sub_Command(ਇਹ);
}

ਖਾਲੀ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ਇਹ);
}

ਖਾਲੀ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = ਖਾਲੀ_ਸੂਚਕ;
}

SubCommand::ਸੰਚਾਲਕ ਤਾਰਕਿਕ() ਅਚਲ {
  ਵਾਪਸ (globalParser().get_ਸਰਗਰਮ_Sub_Command() == ਇਹ);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *Command_ਸਤਰ_Parser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &ਮੁੱਲ) {
  // Reject all dashes.
  ਜੇ (Arg.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.ਲੱਭੋ('=');

  // If we have an equals sign, remember the value.
  ਜੇ (EqualPos == StringRef::npos) {
    // Look up the option.
    ਵਾਪਸ Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  ਸਵੈਚਾਲਿਤ I = Sub.OptionsMap.ਲੱਭੋ(Arg.substr(0, EqualPos));
  ਜੇ (I == Sub.OptionsMap.ਸਿਰਾ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਸਵੈਚਾਲਿਤ *O = I->ਦੂਜਾ_2;
  ਜੇ (O->getFormattingFlag() == cl::Always_ਅਗੇਤਰ)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਮੁੱਲ = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ਵਾਪਸ I->ਦੂਜਾ_2;
}

SubCommand *Command_ਸਤਰ_Parser::LookupSubCommand(StringRef Name,
                                                std::string &ਸਭ_ਤੋਂ_ਨੇੜੇ_String) {
  ਜੇ (Name.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ &SubCommand::get_ਸਿਖਰ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *ਸਭ_ਤੋਂ_ਨੇੜੇ_ਮੇਲ = ਖਾਲੀ_ਸੂਚਕ;
  ਲਈ (ਸਵੈਚਾਲਿਤ *S : RegisteredSubCommands) {
    ਪ੍ਰਤਿਜ੍ਞਾ(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    ਜੇ (S->getName().ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;

    ਜੇ (S->getName() == Name)
      ਵਾਪਸ S;

    ਜੇ (!ਸਭ_ਤੋਂ_ਨੇੜੇ_ਮੇਲ && S->getName().edit_ਦੂਰੀ(Name) < 2)
      ਸਭ_ਤੋਂ_ਨੇੜੇ_ਮੇਲ = S;
  }

  ਜੇ (ਸਭ_ਤੋਂ_ਨੇੜੇ_ਮੇਲ)
    ਸਭ_ਤੋਂ_ਨੇੜੇ_String = ਸਭ_ਤੋਂ_ਨੇੜੇ_ਮੇਲ->getName();

  ਵਾਪਸ &SubCommand::get_ਸਿਖਰ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ਸਥਿਰ Option *Lookup_ਸਭ_ਤੋਂ_ਨੇੜੇ_Option(StringRef Arg,
                                   ਅਚਲ OptionsMapTy &OptionsMap,
                                   std::string &ਸਭ_ਤੋਂ_ਨੇੜੇ_String) {
  // Reject all dashes.
  ਜੇ (Arg.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> ਵੰਡੋ_Arg = Arg.ਵੰਡੋ('=');
  StringRef &LHS = ਵੰਡੋ_Arg.ਪਹਿਲਾ_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = ਵੰਡੋ_Arg.ਦੂਜਾ_2;

  // Find the closest match.
  Option *ਉੱਤਮ = ਖਾਲੀ_ਸੂਚਕ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਉੱਤਮ_ਦੂਰੀ = 0;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    ਜੇ (O->getOptionHiddenFlag() == ReallyHidden)
      ਜਾਰੀ;

    Small_ਸਦਿਸ਼<StringRef, 16> Option_ਨਾਂ;
    O->get_Extra_Option_ਨਾਂ(Option_ਨਾਂ);
    ਜੇ (O->hasArgStr())
      Option_ਨਾਂ.ਧੱਕੋ_ਪਿੱਠ(O->ArgStr);

    ਤਾਰਕਿਕ Permit_ਮੁੱਲ = O->get_ਮੁੱਲ_Expected_Flag() != cl::ਮੁੱਲ_Disallowed;
    StringRef Flag = Permit_ਮੁੱਲ ? LHS : Arg;
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Name : Option_ਨਾਂ) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੂਰੀ_2 = StringRef(Name).edit_ਦੂਰੀ(
          Flag, /*AllowReplacements=*/ਸੱਚ, /*MaxEditDistance=*/ਉੱਤਮ_ਦੂਰੀ);
      ਜੇ (!ਉੱਤਮ || ਦੂਰੀ_2 < ਉੱਤਮ_ਦੂਰੀ) {
        ਉੱਤਮ = O;
        ਉੱਤਮ_ਦੂਰੀ = ਦੂਰੀ_2;
        ਜੇ (RHS.ਖਾਲੀ_ਥਾਵਾਂ() || !Permit_ਮੁੱਲ)
          ਸਭ_ਤੋਂ_ਨੇੜੇ_String = std::string(Name);
        ਨਹੀਂਤਾਂ
          ਸਭ_ਤੋਂ_ਨੇੜੇ_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ਵਾਪਸ ਉੱਤਮ;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ਸਥਿਰ ਤਾਰਕਿਕ Comma_Separate_And_ਜੋੜੋ_Occurrence(Option *Handler, ਬਿਨਾਨਿਸ਼ਾਨੀ pos,
                                          StringRef ArgName, StringRef ਮੁੱਲ) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  ਜੇ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(ਮੁੱਲ);
    StringRef::ਮਾਪ_type Pos = Val.ਲੱਭੋ(',');

    ਜਦੋਂ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      ਜੇ (Handler->ਜੋੜੋ_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        ਵਾਪਸ ਸੱਚ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.ਲੱਭੋ(',');
    }

    ਮੁੱਲ = Val;
  }

  ਵਾਪਸ Handler->ਜੋੜੋ_Occurrence(pos, ArgName, ਮੁੱਲ);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ਸਥਿਰ ਸਤਰਅੰਦਰ ਤਾਰਕਿਕ ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef ਮੁੱਲ, ਪੂਰਨ argc,
                                 ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, ਪੂਰਨ &i) {
  // Enforce value requirements
  ਚੋਣ (Handler->get_ਮੁੱਲ_Expected_Flag()) {
  ਹਾਲਤ ਮੁੱਲ_Required:
    ਜੇ (!ਮੁੱਲ.ਅੰਕੜੇ()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      ਜੇ (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_ਅਗੇਤਰ)
        ਵਾਪਸ Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ਪ੍ਰਤਿਜ੍ਞਾ(argv && "null check");
      ਮੁੱਲ = StringRef(argv[++i]);
    }
    ਰੋਕੋ;
  ਹਾਲਤ ਮੁੱਲ_Disallowed:
    ਜੇ (ਮੁੱਲ.ਅੰਕੜੇ())
      ਵਾਪਸ Handler->error("does not allow a value! '" + Twine(ਮੁੱਲ) +
                            "' specified.");
    ਰੋਕੋ;
  ਹਾਲਤ ਮੁੱਲ_Optional:
    ਰੋਕੋ;
  }

  ਵਾਪਸ Comma_Separate_And_ਜੋੜੋ_Occurrence(Handler, i, ArgName, ਮੁੱਲ);
}

ਤਾਰਕਿਕ llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, ਪੂਰਨ i) {
  ਪੂਰਨ Dummy = i;
  ਵਾਪਸ ProvideOption(Handler, Handler->ArgStr, Arg, 0, ਖਾਲੀ_ਸੂਚਕ, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ਸਥਿਰ Option *ਲੱਭੋ_ਅਗੇਤਰ_Option(StringRef Name, size_t &ਲੰਮਾਈ,
                                ਅਚਲ OptionsMapTy &OptionsMap) {
  ਲਈ (; !Name.ਖਾਲੀ_ਥਾਵਾਂ(); Name = Name.drop_ਪਿੱਠ()) {
    Option *O = OptionsMap.lookup(Name);
    ਜੇ (O && (O->getFormattingFlag() == cl::ਅਗੇਤਰ ||
              O->getFormattingFlag() == cl::Always_ਅਗੇਤਰ)) {
      ਲੰਮਾਈ = Name.ਮਾਪ_2();
      ਵਾਪਸ O;
    }
  }
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ਸਥਿਰ Option *HandlePrefixedOption(StringRef &Arg, StringRef &ਮੁੱਲ,
                                    ਅਚਲ OptionsMapTy &OptionsMap) {
  ਜੇ (Arg.ਮਾਪ_2() == 1)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  size_t ਲੰਮਾਈ = 0;
  Option *POpt = ਲੱਭੋ_ਅਗੇਤਰ_Option(Arg, ਲੰਮਾਈ, OptionsMap);
  ਜੇ (!POpt)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  StringRef Maybe_ਮੁੱਲ =
      (ਲੰਮਾਈ < Arg.ਮਾਪ_2()) ? Arg.substr(ਲੰਮਾਈ) : StringRef();
  Arg = Arg.substr(0, ਲੰਮਾਈ);

  // cl::Prefix options do not preserve '=' when used separately.
  ਜੇ (POpt->getFormattingFlag() == cl::ਅਗੇਤਰ && Maybe_ਮੁੱਲ.starts_with("="))
    Maybe_ਮੁੱਲ = Maybe_ਮੁੱਲ.drop_front();
  ਮੁੱਲ = Maybe_ਮੁੱਲ;
  ਵਾਪਸ POpt;
}

ਸਥਿਰ ਤਾਰਕਿਕ Requires_ਮੁੱਲ(ਅਚਲ Option *O) {
  ਵਾਪਸ O->get_ਅੰਕ_Occurrences_Flag() == cl::Required ||
         O->get_ਅੰਕ_Occurrences_Flag() == cl::One_Or_ਹੋਰ;
}

ਸਥਿਰ ਤਾਰਕਿਕ Eats_Unbounded_ਸੰਖਿਆ_ਦਾ_ਮੁੱਲਾਂ(ਅਚਲ Option *O) {
  ਵਾਪਸ O->get_ਅੰਕ_Occurrences_Flag() == cl::Zero_Or_ਹੋਰ ||
         O->get_ਅੰਕ_Occurrences_Flag() == cl::One_Or_ਹੋਰ;
}

ਸਥਿਰ ਤਾਰਕਿਕ isWhitespace(ਅੱਖਰ C) {
  ਵਾਪਸ C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ਸਥਿਰ ਤਾਰਕਿਕ isWhitespaceOrNull(ਅੱਖਰ C) {
  ਵਾਪਸ isWhitespace(C) || C == '\0';
}

ਸਥਿਰ ਤਾਰਕਿਕ isQuote(ਅੱਖਰ C) { ਵਾਪਸ C == '\"' || C == '\''; }

ਖਾਲੀ cl::Tokenize_GNUCommand_ਸਤਰ(StringRef Src, StringSaver &Saver,
                                Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv,
                                ਤਾਰਕਿਕ MarkEOLs) {
  SmallString<128> Token;
  ਤਾਰਕਿਕ ਵਿੱਚ_Token = ਝੂਠ;
  ਲਈ (size_t I = 0, E = Src.ਮਾਪ_2(); I != E; ++I) {
    // Consume runs of whitespace.
    ਜੇ (!ਵਿੱਚ_Token) {
      ਜਦੋਂ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        ਜੇ (MarkEOLs && Src[I] == '\n')
          NewArgv.ਧੱਕੋ_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
        ++I;
      }
      ਜੇ (I == E)
        ਰੋਕੋ;
      ਵਿੱਚ_Token = ਸੱਚ;
    }

    ਅੱਖਰ C = Src[I];

    // Backslash escapes the next character.
    ਜੇ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ਧੱਕੋ_ਪਿੱਠ(Src[I]);
      ਜਾਰੀ;
    }

    // Consume a quoted string.
    ਜੇ (isQuote(C)) {
      ++I;
      ਜਦੋਂ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        ਜੇ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ਧੱਕੋ_ਪਿੱਠ(Src[I]);
        ++I;
      }
      ਜੇ (I == E)
        ਰੋਕੋ;
      ਜਾਰੀ;
    }

    // End the token if this is whitespace.
    ਜੇ (isWhitespace(C)) {
      NewArgv.ਧੱਕੋ_ਪਿੱਠ(Saver.save(Token.str()).ਅੰਕੜੇ());
      // Mark the end of lines in response files.
      ਜੇ (MarkEOLs && C == '\n')
        NewArgv.ਧੱਕੋ_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
      Token.clear();
      ਵਿੱਚ_Token = ਝੂਠ;
      ਜਾਰੀ;
    }

    // This is a normal character.  Append it.
    Token.ਧੱਕੋ_ਪਿੱਠ(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  ਜੇ (ਵਿੱਚ_Token)
    NewArgv.ਧੱਕੋ_ਪਿੱਠ(Saver.save(Token.str()).ਅੰਕੜੇ());
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
ਸਥਿਰ size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.ਮਾਪ_2();
  ਪੂਰਨ Backslash_ਗਿਣਤੀ = 0;
  // Skip the backslashes.
  ਕਰੋ {
    ++I;
    ++Backslash_ਗਿਣਤੀ;
  } ਜਦੋਂ (I != E && Src[I] == '\\');

  ਤਾਰਕਿਕ FollowedByDoubleQuote = (I != E && Src[I] == '"');
  ਜੇ (FollowedByDoubleQuote) {
    Token.append(Backslash_ਗਿਣਤੀ / 2, '\\');
    ਜੇ (Backslash_ਗਿਣਤੀ % 2 == 0)
      ਵਾਪਸ I - 1;
    Token.ਧੱਕੋ_ਪਿੱਠ('"');
    ਵਾਪਸ I;
  }
  Token.append(Backslash_ਗਿਣਤੀ, '\\');
  ਵਾਪਸ I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ਸਥਿਰ ਤਾਰਕਿਕ isWindowsSpecialChar(ਅੱਖਰ C) {
  ਵਾਪਸ isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ਸਥਿਰ ਤਾਰਕਿਕ is_Windows_Special_Char_ਵਿੱਚ_Command_Name(ਅੱਖਰ C) {
  ਵਾਪਸ isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ tokenize_Windows_Command_ਸਤਰ_Impl(
    StringRef Src, StringSaver &Saver, function_ref<ਖਾਲੀ(StringRef)> ਜੋੜੋ_Token,
    ਤਾਰਕਿਕ AlwaysCopy, function_ref<ਖਾਲੀ()> MarkEOL, ਤਾਰਕਿਕ InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  ਤਾਰਕਿਕ CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  ਗਿਣਤੀ { INIT, UNQUOTED, QUOTED } State = INIT;

  ਲਈ (size_t I = 0, E = Src.ਮਾਪ_2(); I < E; ++I) {
    ਚੋਣ (State) {
    ਹਾਲਤ INIT: {
      ਪ੍ਰਤਿਜ੍ਞਾ(Token.ਖਾਲੀ_ਥਾਵਾਂ() && "token should be empty in initial state");
      // Eat whitespace before a token.
      ਜਦੋਂ (I < E && isWhitespaceOrNull(Src[I])) {
        ਜੇ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      ਜੇ (I >= E)
        ਰੋਕੋ;
      size_t ਸ਼ੁਰੂ = I;
      ਜੇ (CommandName) {
        ਜਦੋਂ (I < E && !is_Windows_Special_Char_ਵਿੱਚ_Command_Name(Src[I]))
          ++I;
      } ਨਹੀਂਤਾਂ {
        ਜਦੋਂ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef ਸਾਧਾਰਨ_Chars = Src.ਟੁਕੜਾ(ਸ਼ੁਰੂ, I);
      ਜੇ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        ਜੋੜੋ_Token(AlwaysCopy ? Saver.save(ਸਾਧਾਰਨ_Chars) : ਸਾਧਾਰਨ_Chars);
        ਜੇ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } ਨਹੀਂਤਾਂ {
          CommandName = ਝੂਠ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (Src[I] == '\"') {
        Token += ਸਾਧਾਰਨ_Chars;
        State = QUOTED;
      } ਨਹੀਂਤਾਂ ਜੇ (Src[I] == '\\') {
        ਪ੍ਰਤਿਜ੍ਞਾ(!CommandName && "or else we'd have treated it as a normal char");
        Token += ਸਾਧਾਰਨ_Chars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } ਨਹੀਂਤਾਂ {
        llvm_unreachable("unexpected special character");
      }
      ਰੋਕੋ;
    }

    ਹਾਲਤ UNQUOTED:
      ਜੇ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        ਜੋੜੋ_Token(Saver.save(Token.str()));
        Token.clear();
        ਜੇ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } ਨਹੀਂਤਾਂ {
          CommandName = ਝੂਠ;
        }
        State = INIT;
      } ਨਹੀਂਤਾਂ ਜੇ (Src[I] == '\"') {
        State = QUOTED;
      } ਨਹੀਂਤਾਂ ਜੇ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ਨਹੀਂਤਾਂ {
        Token.ਧੱਕੋ_ਪਿੱਠ(Src[I]);
      }
      ਰੋਕੋ;

    ਹਾਲਤ QUOTED:
      ਜੇ (Src[I] == '\"') {
        ਜੇ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ਧੱਕੋ_ਪਿੱਠ('"');
          ++I;
        } ਨਹੀਂਤਾਂ {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ਨਹੀਂਤਾਂ {
        Token.ਧੱਕੋ_ਪਿੱਠ(Src[I]);
      }
      ਰੋਕੋ;
    }
  }

  ਜੇ (State != INIT)
    ਜੋੜੋ_Token(Saver.save(Token.str()));
}

ਖਾਲੀ cl::Tokenize_Windows_Command_ਸਤਰ(StringRef Src, StringSaver &Saver,
                                    Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv,
                                    ਤਾਰਕਿਕ MarkEOLs) {
  ਸਵੈਚਾਲਿਤ ਜੋੜੋ_Token = [&](StringRef Tok) { NewArgv.ਧੱਕੋ_ਪਿੱਠ(Tok.ਅੰਕੜੇ()); };
  ਸਵੈਚਾਲਿਤ OnEOL = [&]() {
    ਜੇ (MarkEOLs)
      NewArgv.ਧੱਕੋ_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
  };
  tokenize_Windows_Command_ਸਤਰ_Impl(Src, Saver, ਜੋੜੋ_Token,
                                 /*AlwaysCopy=*/ਸੱਚ, OnEOL, ਝੂਠ);
}

ਖਾਲੀ cl::Tokenize_Windows_Command_ਸਤਰ_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_ਸਦਿਸ਼_Impl<StringRef> &NewArgv) {
  ਸਵੈਚਾਲਿਤ ਜੋੜੋ_Token = [&](StringRef Tok) { NewArgv.ਧੱਕੋ_ਪਿੱਠ(Tok); };
  ਸਵੈਚਾਲਿਤ OnEOL = []() {};
  tokenize_Windows_Command_ਸਤਰ_Impl(Src, Saver, ਜੋੜੋ_Token, /*AlwaysCopy=*/ਝੂਠ,
                                 OnEOL, ਝੂਠ);
}

ਖਾਲੀ cl::Tokenize_Windows_Command_ਸਤਰ_ਭਰਿਆ(StringRef Src, StringSaver &Saver,
                                        Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv,
                                        ਤਾਰਕਿਕ MarkEOLs) {
  ਸਵੈਚਾਲਿਤ ਜੋੜੋ_Token = [&](StringRef Tok) { NewArgv.ਧੱਕੋ_ਪਿੱਠ(Tok.ਅੰਕੜੇ()); };
  ਸਵੈਚਾਲਿਤ OnEOL = [&]() {
    ਜੇ (MarkEOLs)
      NewArgv.ਧੱਕੋ_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
  };
  tokenize_Windows_Command_ਸਤਰ_Impl(Src, Saver, ਜੋੜੋ_Token,
                                 /*AlwaysCopy=*/ਸੱਚ, OnEOL, ਸੱਚ);
}

ਖਾਲੀ cl::tokenizeConfigFile(StringRef ਸਰੋਤ, StringSaver &Saver,
                            Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv,
                            ਤਾਰਕਿਕ MarkEOLs) {
  ਲਈ (ਅਚਲ ਅੱਖਰ *Cur = ਸਰੋਤ.begin(); Cur != ਸਰੋਤ.ਸਿਰਾ();) {
    SmallString<128> ਸਤਰ;
    // Check for comment line.
    ਜੇ (isWhitespace(*Cur)) {
      ਜਦੋਂ (Cur != ਸਰੋਤ.ਸਿਰਾ() && isWhitespace(*Cur))
        ++Cur;
      ਜਾਰੀ;
    }
    ਜੇ (*Cur == '#') {
      ਜਦੋਂ (Cur != ਸਰੋਤ.ਸਿਰਾ() && *Cur != '\n')
        ++Cur;
      ਜਾਰੀ;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = Cur;
    ਅਚਲ ਅੱਖਰ *ਸਤਰ_ਸਿਰਾ = ਖਾਲੀ_ਸੂਚਕ;
    ਅੱਖਰ Quote = 0;
    ਤਾਰਕਿਕ At_Token_ਸ਼ੁਰੂ = ਸੱਚ;
    ਲਈ (ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_2 = ਸਰੋਤ.ਸਿਰਾ(); Cur != ਸਿਰਾ_2; ++Cur) {
      ਅੱਖਰ C = *Cur;
      ਜੇ (C == '\\') {
        ਜੇ (Cur + 1 != ਸਿਰਾ_2) {
          ++Cur;
          ਜੇ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != ਸਿਰਾ_2) && Cur[1] == '\n')) {
            ਸਤਰ.append(ਸ਼ੁਰੂ, Cur - 1);
            ਜੇ (*Cur == '\r')
              ++Cur;
            ਸ਼ੁਰੂ = Cur + 1;
            // Splicing does not introduce a token boundary.
            ਜਾਰੀ;
          }
        }
        At_Token_ਸ਼ੁਰੂ = ਝੂਠ;
        ਜਾਰੀ;
      }
      ਜੇ (C == '\n')
        ਰੋਕੋ;
      ਜੇ (Quote) {
        ਜੇ (C == Quote)
          Quote = 0;
      } ਨਹੀਂਤਾਂ ਜੇ (isQuote(C)) {
        Quote = C;
      } ਨਹੀਂਤਾਂ ਜੇ (C == '#' && At_Token_ਸ਼ੁਰੂ) {
        ਸਤਰ_ਸਿਰਾ = Cur;
        ਜਦੋਂ (Cur != ਸਿਰਾ_2 && *Cur != '\n')
          ++Cur;
        ਰੋਕੋ;
      }
      At_Token_ਸ਼ੁਰੂ = isWhitespace(C);
    }
    // Tokenize line.
    ਸਤਰ.append(ਸ਼ੁਰੂ, ਸਤਰ_ਸਿਰਾ ? ਸਤਰ_ਸਿਰਾ : Cur);
    cl::Tokenize_GNUCommand_ਸਤਰ(ਸਤਰ, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ਸਥਿਰ ਤਾਰਕਿਕ has_UTF8_Byte_ਕ੍ਰਮ_Mark(ArrayRef<ਅੱਖਰ> S) {
  ਵਾਪਸ (S.ਮਾਪ_2() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ਸਥਿਰ ਖਾਲੀ Expand_ਅਧਾਰ_Paths(StringRef ਅਧਾਰ_ਰਸਤਾ, StringSaver &Saver,
                            ਅਚਲ ਅੱਖਰ *&Arg) {
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(ਅਧਾਰ_ਰਸਤਾ));
  constexpr StringLiteral Token("<CFGDIR>");
  ਅਚਲ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::ਮਾਪ_type ਸ਼ੁਰੂ_Pos = 0;
  ਲਈ (StringRef::ਮਾਪ_type TokenPos = ArgString.ਲੱਭੋ(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.ਲੱਭੋ(Token, ਸ਼ੁਰੂ_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ਅਚਲ StringRef LHS = ArgString.substr(ਸ਼ੁਰੂ_Pos, TokenPos - ਸ਼ੁਰੂ_Pos);
    ਜੇ (ResponseFile.ਖਾਲੀ_ਥਾਵਾਂ())
      ResponseFile = LHS;
    ਨਹੀਂਤਾਂ
      llvm::sys::ਰਸਤਾ_2::append(ResponseFile, LHS);
    ResponseFile.append(ਅਧਾਰ_ਰਸਤਾ);
    ਸ਼ੁਰੂ_Pos = TokenPos + Token.ਮਾਪ_2();
  }

  ਜੇ (!ResponseFile.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ਅਚਲ StringRef ਬਚਿਆ = ArgString.substr(ਸ਼ੁਰੂ_Pos);
    ਜੇ (!ਬਚਿਆ.ਖਾਲੀ_ਥਾਵਾਂ())
      llvm::sys::ਰਸਤਾ_2::append(ResponseFile, ਬਚਿਆ);
    Arg = Saver.save(ResponseFile.str()).ਅੰਕੜੇ();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv) {
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> MemBufOrErr =
      FS->get_ਬਫਰ_For_File(FName);
  ਜੇ (!MemBufOrErr) {
    std::error_ਕੋਡ EC = MemBufOrErr.getError();
    ਵਾਪਸ llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.ਸੁਨੇਹਾ_2());
  }
  ਸਮ੍ਰਿਤੀ_ਬਫਰ &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_ਬਫਰ_ਸ਼ੁਰੂ(), MemBuf.get_ਬਫਰ_ਮਾਪ());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<ਅੱਖਰ> BufRef(MemBuf.get_ਬਫਰ_ਸ਼ੁਰੂ(), MemBuf.get_ਬਫਰ_ਸਿਰਾ());
  std::string UTF8Buf;
  ਜੇ (has_UTF16_Byte_ਕ੍ਰਮ_Mark(BufRef)) {
    ਜੇ (!convert_UTF16_ਮੰਜ਼ਿਲ_UTF8_String(BufRef, UTF8Buf))
      ਵਾਪਸ llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  ਨਹੀਂਤਾਂ ਜੇ (has_UTF8_Byte_ਕ੍ਰਮ_Mark(BufRef))
    Str = StringRef(BufRef.ਅੰਕੜੇ() + 3, BufRef.ਮਾਪ_2() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  ਜੇ (!Relative_ਨਾਂ && !ਵਿੱਚ_Config_File)
    ਵਾਪਸ Error::success();

  StringRef ਅਧਾਰ_ਰਸਤਾ = llvm::sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(FName);
  ਲਈ (ਅਚਲ ਅੱਖਰ *&Arg : NewArgv) {
    ਜੇ (!Arg)
      ਜਾਰੀ;

    // Substitute <CFGDIR> with the file's base path.
    ਜੇ (ਵਿੱਚ_Config_File)
      Expand_ਅਧਾਰ_Paths(ਅਧਾਰ_ਰਸਤਾ, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    ਤਾਰਕਿਕ ConfigInclusion = ਝੂਠ;
    ਜੇ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      ਜੇ (!llvm::sys::ਰਸਤਾ_2::is_relative(FileName))
        ਜਾਰੀ;
    } ਨਹੀਂਤਾਂ ਜੇ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = ਸੱਚ;
    } ਨਹੀਂਤਾਂ {
      ਜਾਰੀ;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ਧੱਕੋ_ਪਿੱਠ('@');
    ਜੇ (ConfigInclusion && !llvm::sys::ਰਸਤਾ_2::has_ਮਾਪਾ_ਰਸਤਾ(FileName)) {
      SmallString<128> File_ਰਸਤਾ;
      ਜੇ (!ਲੱਭੋ_Config_File(FileName, File_ਰਸਤਾ))
        ਵਾਪਸ createStringError(
            std::make_error_ਕੋਡ(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_ਰਸਤਾ);
    } ਨਹੀਂਤਾਂ {
      ResponseFile.append(ਅਧਾਰ_ਰਸਤਾ);
      llvm::sys::ਰਸਤਾ_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).ਅੰਕੜੇ();
  }
  ਵਾਪਸ Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &Argv) {
  ਬਣਤਰ Response_File_ਰਿਕਾਰਡ {
    std::string File;
    size_t ਸਿਰਾ_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_ਸਦਿਸ਼<Response_File_ਰਿਕਾਰਡ, 3> File_ਢੇਰ;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ({"", Argv.ਮਾਪ_2()});

  // Don't cache Argv.size() because it can change.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I != Argv.ਮਾਪ_2();) {
    ਜਦੋਂ (I == File_ਢੇਰ.ਪਿੱਠ().ਸਿਰਾ_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    }

    ਅਚਲ ਅੱਖਰ *Arg = Argv[I];
    // Check if it is an EOL marker
    ਜੇ (Arg == ਖਾਲੀ_ਸੂਚਕ) {
      ++I;
      ਜਾਰੀ;
    }

    ਜੇ (Arg[0] != '@') {
      ++I;
      ਜਾਰੀ;
    }

    ਅਚਲ ਅੱਖਰ *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    ਜੇ (llvm::sys::ਰਸਤਾ_2::is_relative(FName)) {
      ਜੇ (CurrentDir.ਖਾਲੀ_ਥਾਵਾਂ()) {
        ਜੇ (ਸਵੈਚਾਲਿਤ CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } ਨਹੀਂਤਾਂ {
          ਵਾਪਸ createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } ਨਹੀਂਤਾਂ {
        CurrDir = CurrentDir;
      }
      llvm::sys::ਰਸਤਾ_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::ਸਥਿਤੀ> Res = FS->ਸਥਿਤੀ_2(FName);
    ਜੇ (!Res || !Res->exists()) {
      std::error_ਕੋਡ EC = Res.getError();
      ਜੇ (!ਵਿੱਚ_Config_File) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        ਜੇ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ਜਾਰੀ;
        }
      }
      ਜੇ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ਵਾਪਸ createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.ਸੁਨੇਹਾ_2());
    }
    ਅਚਲ llvm::vfs::ਸਥਿਤੀ &File_ਸਥਿਤੀ = Res.get();

    ਸਵੈਚਾਲਿਤ IsEquivalent =
        [File_ਸਥਿਤੀ, ਇਹ](ਅਚਲ Response_File_ਰਿਕਾਰਡ &RFile) -> ErrorOr<ਤਾਰਕਿਕ> {
      ErrorOr<llvm::vfs::ਸਥਿਤੀ> RHS = FS->ਸਥਿਤੀ_2(RFile.File);
      ਜੇ (!RHS)
        ਵਾਪਸ RHS.getError();
      ਵਾਪਸ File_ਸਥਿਤੀ.equivalent(*RHS);
    };

    // Check for recursive response files.
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &F : drop_begin(File_ਢੇਰ)) {
      ਜੇ (ErrorOr<ਤਾਰਕਿਕ> R = IsEquivalent(F)) {
        ਜੇ (R.get())
          ਵਾਪਸ createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    Small_ਸਦਿਸ਼<ਅਚਲ ਅੱਖਰ *, 0> ExpandedArgv;
    ਜੇ (Error Err = expandResponseFile(FName, ExpandedArgv))
      ਵਾਪਸ Err;

    ਲਈ (Response_File_ਰਿਕਾਰਡ &ਰਿਕਾਰਡ : File_ਢੇਰ) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      ਰਿਕਾਰਡ.ਸਿਰਾ_2 += ExpandedArgv.ਮਾਪ_2() - 1;
    }

    File_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ({FName, I + ExpandedArgv.ਮਾਪ_2()});
    Argv.erase(Argv.begin() + I);
    Argv.ਪਾਓ(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.ਸਿਰਾ());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ਪ੍ਰਤਿਜ੍ਞਾ(File_ਢੇਰ.ਮਾਪ_2() > 0 && Argv.ਮਾਪ_2() == File_ਢੇਰ.ਪਿੱਠ().ਸਿਰਾ_2);
  ਵਾਪਸ Error::success();
}

ਤਾਰਕਿਕ cl::expandResponseFiles(ਪੂਰਨ Argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ *Argv,
                             ਅਚਲ ਅੱਖਰ *EnvVar, StringSaver &Saver,
                             Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &NewArgv) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਸਵੈਚਾਲਿਤ Tokenize = cl::Tokenize_Windows_Command_ਸਤਰ;
#ਨਹੀਂਤਾਂ
  ਸਵੈਚਾਲਿਤ Tokenize = cl::Tokenize_GNUCommand_ਸਤਰ;
#ਜੇ_ਅੰਤ
  // The environment variable specifies initial options.
  ਜੇ (EnvVar)
    ਜੇ (std::optional<std::string> Env_ਮੁੱਲ = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_ਮੁੱਲ, Saver, NewArgv, /*MarkEOLs=*/ਝੂਠ);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  ਜੇ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << ਮੰਜ਼ਿਲ_String(std::move(Err)) << '\n';
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  ਜੇ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << ਮੰਜ਼ਿਲ_String(std::move(Err)) << '\n';
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

ਤਾਰਕਿਕ ExpansionContext::ਲੱਭੋ_Config_File(StringRef FileName,
                                      Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &File_ਰਸਤਾ) {
  SmallString<128> Cfg_File_ਰਸਤਾ;
  ਅਚਲ ਸਵੈਚਾਲਿਤ FileExists = [ਇਹ](SmallString<128> ਰਸਤਾ) -> ਤਾਰਕਿਕ {
    ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ = FS->ਸਥਿਤੀ_2(ਰਸਤਾ);
    ਵਾਪਸ ਸਥਿਤੀ &&
           ਸਥਿਤੀ->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  ਜੇ (llvm::sys::ਰਸਤਾ_2::has_ਮਾਪਾ_ਰਸਤਾ(FileName)) {
    Cfg_File_ਰਸਤਾ = FileName;
    ਜੇ (llvm::sys::ਰਸਤਾ_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_ਰਸਤਾ))
      ਵਾਪਸ ਝੂਠ;
    ਜੇ (!FileExists(Cfg_File_ਰਸਤਾ))
      ਵਾਪਸ ਝੂਠ;
    File_ਰਸਤਾ.ਸੌਂਪੋ(Cfg_File_ਰਸਤਾ.begin(), Cfg_File_ਰਸਤਾ.ਸਿਰਾ());
    ਵਾਪਸ ਸੱਚ;
  }

  // Look for the file in search directories.
  ਲਈ (ਅਚਲ StringRef &Dir : ਖੋਜ_Dirs) {
    ਜੇ (Dir.ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;
    Cfg_File_ਰਸਤਾ.ਸੌਂਪੋ(Dir);
    llvm::sys::ਰਸਤਾ_2::append(Cfg_File_ਰਸਤਾ, FileName);
    llvm::sys::ਰਸਤਾ_2::native(Cfg_File_ਰਸਤਾ);
    ਜੇ (FileExists(Cfg_File_ਰਸਤਾ)) {
      File_ਰਸਤਾ.ਸੌਂਪੋ(Cfg_File_ਰਸਤਾ.begin(), Cfg_File_ਰਸਤਾ.ਸਿਰਾ());
      ਵਾਪਸ ਸੱਚ;
    }
  }

  ਵਾਪਸ ਝੂਠ;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_ਸਦਿਸ਼_Impl<ਅਚਲ ਅੱਖਰ *> &Argv) {
  SmallString<128> Abs_ਰਸਤਾ;
  ਜੇ (sys::ਰਸਤਾ_2::is_relative(CfgFile)) {
    Abs_ਰਸਤਾ.ਸੌਂਪੋ(CfgFile);
    ਜੇ (std::error_ਕੋਡ EC = FS->makeAbsolute(Abs_ਰਸਤਾ))
      ਵਾਪਸ make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_ਰਸਤਾ.str();
  }
  ਵਿੱਚ_Config_File = ਸੱਚ;
  Relative_ਨਾਂ = ਸੱਚ;
  ਜੇ (Error Err = expandResponseFile(CfgFile, Argv))
    ਵਾਪਸ Err;
  ਵਾਪਸ expandResponseFiles(Argv);
}

ਸਥਿਰ ਖਾਲੀ initCommonOptions();
ਸਥਿਰ ਤਾਰਕਿਕ parse_Command_ਸਤਰ_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ਅਚਲ ਅੱਖਰ *EnvVar) {
  initCommonOptions();
  Small_ਸਦਿਸ਼<ਅਚਲ ਅੱਖਰ *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ਧੱਕੋ_ਪਿੱਠ(ਦਲੀਲਾਂ[0]);

  // Parse options from environment variable.
  ਜੇ (EnvVar) {
    ਜੇ (std::optional<std::string> Env_ਮੁੱਲ =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ਸਤਰ(*Env_ਮੁੱਲ, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, ਦਲੀਲਾਂ.drop_front());
  ਪੂਰਨ NewArgc = static_cast<ਪੂਰਨ>(NewArgv.ਮਾਪ_2());

  // Parse all options.
  ਵਾਪਸ globalParser().Parse_Command_ਸਤਰ_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

ਤਾਰਕਿਕ cl::Parse_Command_ਸਤਰ_Options(ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ਅਚਲ ਅੱਖਰ *EnvVar) {
  ਵਾਪਸ parse_Command_ਸਤਰ_Options_Impl(ਖਾਲੀ_ਸੂਚਕ, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

ਤਾਰਕਿਕ cl::parse_Command_ਸਤਰ_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ਅਚਲ ਅੱਖਰ *EnvVar) {
  ਵਾਪਸ parse_Command_ਸਤਰ_Options_Impl(&Context, ਦਲੀਲਾਂ, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
ਖਾਲੀ Command_ਸਤਰ_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  ਲਈ (ਸਵੈਚਾਲਿਤ *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_ਸਦਿਸ਼<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.ਮਾਪ_2());
    ਲਈ (ਸਵੈਚਾਲਿਤ &O : SC->OptionsMap)
      Opts.ਧੱਕੋ_ਪਿੱਠ(O.ਦੂਜਾ_2);
    ਲਈ (Option *O : Opts)
      O->reset();
    ਲਈ (Option *O : SC->PositionalOpts)
      O->reset();
    ਜੇ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  ਲਈ (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

ਤਾਰਕਿਕ Command_ਸਤਰ_Parser::Parse_Command_ਸਤਰ_Options(
    ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ਪ੍ਰਤਿਜ੍ਞਾ(hasOptions() && "No options specified!");

  ਪ੍ਰੋਗਰਾਮ_Overview = Overview;
  ਤਾਰਕਿਕ IgnoreErrors = Errs;
  ਜੇ (!Errs)
    Errs = &errs();
  ਜੇ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  ਤਾਰਕਿਕ ErrorParsing = ਝੂਠ;

  // Expand response files.
  Small_ਸਦਿਸ਼<ਅਚਲ ਅੱਖਰ *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਸਵੈਚਾਲਿਤ Tokenize = cl::Tokenize_Windows_Command_ਸਤਰ;
#ਨਹੀਂਤਾਂ
  ਸਵੈਚਾਲਿਤ Tokenize = cl::Tokenize_GNUCommand_ਸਤਰ;
#ਜੇ_ਅੰਤ
  ExpansionContext ECtx(A, Tokenize, VFS);
  ਜੇ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << ਮੰਜ਼ਿਲ_String(std::move(Err)) << '\n';
    ਵਾਪਸ ਝੂਠ;
  }
  argv = &newArgv[0];
  argc = static_cast<ਪੂਰਨ>(newArgv.ਮਾਪ_2());

  // Copy the program name into ProgName, making sure not to overflow it.
  ਪ੍ਰੋਗਰਾਮ_Name = std::string(sys::ਰਸਤਾ_2::filename(StringRef(argv[0])));
  ਸੂਚਕਾਂਕ_Library_Options();

  // Check out the positional arguments to collect information about them.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  ਤਾਰਕਿਕ HasUnlimitedPositionals = ਝੂਠ;

  ਪੂਰਨ ਪਹਿਲਾ_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_ਸਿਖਰ_Level();
  std::string ਸਭ_ਤੋਂ_ਨੇੜੇ_Sub_Command_String;
  ਤਾਰਕਿਕ MaybeNamedSubCommand =
      argc >= 2 && argv[ਪਹਿਲਾ_Arg][0] != '-' && hasNamedSubCommands();
  ਜੇ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[ਪਹਿਲਾ_Arg]), ਸਭ_ਤੋਂ_ਨੇੜੇ_Sub_Command_String);
    ਜੇ (ChosenSubCommand != &SubCommand::get_ਸਿਖਰ_Level())
      ਪਹਿਲਾ_Arg = 2;
  }
  globalParser().ਸਰਗਰਮ_Sub_Command = ChosenSubCommand;

  ਪ੍ਰਤਿਜ੍ਞਾ(ChosenSubCommand);
  ਸਵੈਚਾਲਿਤ &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  ਸਵੈਚਾਲਿਤ &PositionalOpts = ChosenSubCommand->PositionalOpts;
  ਸਵੈਚਾਲਿਤ &OptionsMap = ChosenSubCommand->OptionsMap;

  ਜੇ (ConsumeAfterOpt) {
    ਪ੍ਰਤਿਜ੍ਞਾ(PositionalOpts.ਮਾਪ_2() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  ਜੇ (!PositionalOpts.ਖਾਲੀ_ਥਾਵਾਂ()) {

    // Calculate how many positional values are _required_.
    ਤਾਰਕਿਕ Unbounded_ਮਿਲਿਆ = ਝੂਠ;
    ਲਈ (size_t i = 0, e = PositionalOpts.ਮਾਪ_2(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      ਜੇ (Requires_ਮੁੱਲ(Opt))
        ++ਅੰਕ_Positional_Required;
      ਨਹੀਂਤਾਂ ਜੇ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        ਜੇ (PositionalOpts.ਮਾਪ_2() > 1) {
          ਜੇ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = ਸੱਚ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (Unbounded_ਮਿਲਿਆ && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        ਜੇ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ਪ੍ਰੋਗਰਾਮ_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.ਮਾਪ_2();
        ErrorParsing = ਸੱਚ;
      }
      Unbounded_ਮਿਲਿਆ |= Eats_Unbounded_ਸੰਖਿਆ_ਦਾ_ਮੁੱਲਾਂ(Opt);
    }
    HasUnlimitedPositionals = Unbounded_ਮਿਲਿਆ || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_ਸਦਿਸ਼<std::pair<StringRef, ਬਿਨਾਨਿਸ਼ਾਨੀ>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *ਸਰਗਰਮ_Positional_Arg = ਖਾਲੀ_ਸੂਚਕ;

  // Loop over all of the arguments... processing them.
  ਤਾਰਕਿਕ Dash_Dash_ਮਿਲਿਆ = ਝੂਠ; // Have we read '--'?
  ਲਈ (ਪੂਰਨ i = ਪਹਿਲਾ_Arg; i < argc; ++i) {
    Option *Handler = ਖਾਲੀ_ਸੂਚਕ;
    std::string ਸਭ_ਤੋਂ_ਨੇੜੇ_Handler_String;
    StringRef ਮੁੱਲ;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    ਜੇ (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_ਮਿਲਿਆ) {
      // Positional argument!
      ਜੇ (ਸਰਗਰਮ_Positional_Arg) {
        ProvidePositionalOption(ਸਰਗਰਮ_Positional_Arg, StringRef(argv[i]), i);
        ਜਾਰੀ; // We are done!
      }

      ਜੇ (!PositionalOpts.ਖਾਲੀ_ਥਾਵਾਂ()) {
        PositionalVals.ਧੱਕੋ_ਪਿੱਠ(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        ਜੇ (PositionalVals.ਮਾਪ_2() >= ਅੰਕ_Positional_Required && ConsumeAfterOpt) {
          ਲਈ (++i; i < argc; ++i)
            PositionalVals.ਧੱਕੋ_ਪਿੱਠ(std::make_pair(StringRef(argv[i]), i));
          ਰੋਕੋ; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ਜਾਰੀ;
      }
    } ਨਹੀਂਤਾਂ ਜੇ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_ਮਿਲਿਆ) {
      Dash_Dash_ਮਿਲਿਆ = ਸੱਚ; // This is the mythical "--"?
      ਜਾਰੀ;             // Don't try to process it as an argument itself.
    } ਨਹੀਂਤਾਂ ਜੇ (ਸਰਗਰਮ_Positional_Arg &&
               (ਸਰਗਰਮ_Positional_Arg->getMiscFlags() & Positional_Eats_ਦਲੀਲਾਂ)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ਮੁੱਲ);
      ਜੇ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ਸਰਗਰਮ_Positional_Arg, StringRef(argv[i]), i);
        ਜਾਰੀ; // We are done!
      }
    } ਨਹੀਂਤਾਂ { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ਮੁੱਲ);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      ਜੇ (!Handler && ChosenSubCommand != &SubCommand::get_ਸਿਖਰ_Level())
        Handler = LookupOption(SubCommand::get_ਸਿਖਰ_Level(), ArgName, ਮੁੱਲ);

      ਜੇ (!Handler) {
        ਜੇ (LibraryOptions *L = lookupLibraryOption(ArgName.ਵੰਡੋ('=').ਪਹਿਲਾ_2)) {
          ArrayRef<ਅਚਲ ਅੱਖਰ *> Rest(argv + i, argc - i);
          ਬਿਨਾਨਿਸ਼ਾਨੀ N;
          ਜੇ (Error E = Context && L->is_ਪ੍ਰਤੀ_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ਪ੍ਰੋਗਰਾਮ_Name << ": " << ਮੰਜ਼ਿਲ_String(std::move(E)) << '\n';
            ErrorParsing = ਸੱਚ;
          }
          i += N - 1;
          ਜਾਰੀ;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      ਜੇ (!Handler)
        Handler = HandlePrefixedOption(ArgName, ਮੁੱਲ, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      ਜੇ (!Handler)
        Lookup_ਸਭ_ਤੋਂ_ਨੇੜੇ_Option(ArgName, OptionsMap, ਸਭ_ਤੋਂ_ਨੇੜੇ_Handler_String);
    }

    ਜੇ (!Handler) {
      ਸਵੈਚਾਲਿਤ Report_Unknown_ਦਲੀਲ = [&](ਤਾਰਕਿਕ IsArg,
                                       StringRef ਸਭ_ਤੋਂ_ਨੇੜੇ_ਦਲੀਲ_Name) {
        *Errs << ਪ੍ਰੋਗਰਾਮ_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        ਜੇ (ਸਭ_ਤੋਂ_ਨੇੜੇ_ਦਲੀਲ_Name.ਖਾਲੀ_ਥਾਵਾਂ())
          ਵਾਪਸ;

        *Errs << ਪ੍ਰੋਗਰਾਮ_Name << ": Did you mean '";
        ਜੇ (IsArg)
          *Errs << PrintArg(ਸਭ_ਤੋਂ_ਨੇੜੇ_ਦਲੀਲ_Name, 0);
        ਨਹੀਂਤਾਂ
          *Errs << ਸਭ_ਤੋਂ_ਨੇੜੇ_ਦਲੀਲ_Name;
        *Errs << "'?\n";
      };

      ਜੇ (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_ਦਲੀਲ(/*IsArg=*/ਸੱਚ, ਸਭ_ਤੋਂ_ਨੇੜੇ_Handler_String);
      ਨਹੀਂਤਾਂ
        Report_Unknown_ਦਲੀਲ(/*IsArg=*/ਝੂਠ, ਸਭ_ਤੋਂ_ਨੇੜੇ_Sub_Command_String);

      ErrorParsing = ਸੱਚ;
      ਜਾਰੀ;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    ਜੇ (Handler->getFormattingFlag() == cl::Positional) {
      ਜੇ ((Handler->getMiscFlags() & Positional_Eats_ਦਲੀਲਾਂ) && !ਮੁੱਲ.ਖਾਲੀ_ਥਾਵਾਂ()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = ਸੱਚ;
      }
      ਸਰਗਰਮ_Positional_Arg = Handler;
    }
    ਨਹੀਂਤਾਂ
      ErrorParsing |= ProvideOption(Handler, ArgName, ਮੁੱਲ, argc, argv, i);
  }

  // Check and handle positional arguments now...
  ਜੇ (ਅੰਕ_Positional_Required > PositionalVals.ਮਾਪ_2()) {
      *Errs << ਪ੍ਰੋਗਰਾਮ_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << ਅੰਕ_Positional_Required
             << " positional argument" << (ਅੰਕ_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = ਸੱਚ;
  } ਨਹੀਂਤਾਂ ਜੇ (!HasUnlimitedPositionals &&
             PositionalVals.ਮਾਪ_2() > PositionalOpts.ਮਾਪ_2()) {
    *Errs << ਪ੍ਰੋਗਰਾਮ_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.ਮਾਪ_2()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = ਸੱਚ;

  } ਨਹੀਂਤਾਂ ਜੇ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ValNo = 0, ਅੰਕ_Vals = static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(PositionalVals.ਮਾਪ_2());
    ਲਈ (Option *Opt : PositionalOpts) {
      ਜੇ (Requires_ਮੁੱਲ(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].ਪਹਿਲਾ_2,
                                PositionalVals[ValNo].ਦੂਜਾ_2);
        ValNo++;
        --ਅੰਕ_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      ਤਾਰਕਿਕ ਪੂਰਾ = Opt->get_ਅੰਕ_Occurrences_Flag() == cl::Required;
      ਜਦੋਂ (ਅੰਕ_Vals - ValNo > ਅੰਕ_Positional_Required && !ਪੂਰਾ) {
        ਚੋਣ (Opt->get_ਅੰਕ_Occurrences_Flag()) {
        ਹਾਲਤ cl::Optional:
          ਪੂਰਾ = ਸੱਚ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        ਹਾਲਤ cl::Zero_Or_ਹੋਰ: // Zero or more will take all they can get...
        ਹਾਲਤ cl::One_Or_ਹੋਰ:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].ਪਹਿਲਾ_2,
                                  PositionalVals[ValNo].ਦੂਜਾ_2);
          ValNo++;
          ਰੋਕੋ;
        ਮੂਲ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(ConsumeAfterOpt && ਅੰਕ_Positional_Required <= PositionalVals.ਮਾਪ_2());
    ਬਿਨਾਨਿਸ਼ਾਨੀ ValNo = 0;
    ਲਈ (Option *Opt : PositionalOpts)
      ਜੇ (Requires_ਮੁੱਲ(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].ਪਹਿਲਾ_2, PositionalVals[ValNo].ਦੂਜਾ_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    ਜੇ (PositionalOpts.ਮਾਪ_2() == 1 && ValNo == 0 && !PositionalVals.ਖਾਲੀ_ਥਾਵਾਂ()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].ਪਹਿਲਾ_2,
                                              PositionalVals[ValNo].ਦੂਜਾ_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    ਲਈ (; ValNo != PositionalVals.ਮਾਪ_2(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].ਪਹਿਲਾ_2,
                                  PositionalVals[ValNo].ਦੂਜਾ_2);
  }

  // Loop over args and make sure all required args are specified!
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Opt : OptionsMap) {
    ਚੋਣ (Opt.ਦੂਜਾ_2->get_ਅੰਕ_Occurrences_Flag()) {
    ਹਾਲਤ Required:
    ਹਾਲਤ One_Or_ਹੋਰ:
      ਜੇ (Opt.ਦੂਜਾ_2->get_ਅੰਕ_Occurrences() == 0) {
        Opt.ਦੂਜਾ_2->error("must be specified at least once!");
        ErrorParsing = ਸੱਚ;
      }
      [[fallthrough]];
    ਮੂਲ:
      ਰੋਕੋ;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             ਲਈ (ਪੂਰਨ i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  ਹੋਰ_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  ਜੇ (ErrorParsing) {
    ਜੇ (!IgnoreErrors)
      ਬਾਹਰ_ਨਿਕਲੋ(1);
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

ਤਾਰਕਿਕ Option::error(ਅਚਲ Twine &ਸੁਨੇਹਾ_3, StringRef ArgName, raw_ostream &Errs) {
  ਜੇ (!ArgName.ਅੰਕੜੇ())
    ArgName = ArgStr;
  ਜੇ (ArgName.ਖਾਲੀ_ਥਾਵਾਂ())
    Errs << HelpStr; // Be nice for positional arguments
  ਨਹੀਂਤਾਂ
    Errs << globalParser().ਪ੍ਰੋਗਰਾਮ_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << ਸੁਨੇਹਾ_3 << "\n";
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Option::ਜੋੜੋ_Occurrence(ਬਿਨਾਨਿਸ਼ਾਨੀ pos, StringRef ArgName, StringRef ਮੁੱਲ) {
  ++ਅੰਕ_Occurrences;
  ਵਾਪਸ handleOccurrence(pos, ArgName, ਮੁੱਲ);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ਸਥਿਰ StringRef get_ਮੁੱਲ_Str(ਅਚਲ Option &O, StringRef Default_ਸੁਨੇਹਾ) {
  ਜੇ (O.ਮੁੱਲ_Str.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ Default_ਸੁਨੇਹਾ;
  ਵਾਪਸ O.ਮੁੱਲ_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_ਚੌੜਾਈ() ਅਚਲ {
  ਵਾਪਸ arg_Plus_Prefixes_ਮਾਪ(ArgStr);
}

ਖਾਲੀ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t ਪਹਿਲਾ_ਸਤਰ_Indented_By) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Indent >= ਪਹਿਲਾ_ਸਤਰ_Indented_By);
  std::pair<StringRef, StringRef> ਵੰਡੋ_2 = HelpStr.ਵੰਡੋ('\n');
  outs().indent(Indent - ਪਹਿਲਾ_ਸਤਰ_Indented_By)
      << Arg_Help_ਅਗੇਤਰ << ਵੰਡੋ_2.ਪਹਿਲਾ_2 << "\n";
  ਜਦੋਂ (!ਵੰਡੋ_2.ਦੂਜਾ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਵੰਡੋ_2 = ਵੰਡੋ_2.ਦੂਜਾ_2.ਵੰਡੋ('\n');
    outs().indent(Indent) << ਵੰਡੋ_2.ਪਹਿਲਾ_2 << "\n";
  }
}

ਖਾਲੀ Option::printEnumValHelpStr(StringRef HelpStr, size_t ਅਧਾਰ_Indent,
                                 size_t ਪਹਿਲਾ_ਸਤਰ_Indented_By) {
  ਅਚਲ StringRef Val_Help_ਅਗੇਤਰ = "  ";
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅਧਾਰ_Indent >= ਪਹਿਲਾ_ਸਤਰ_Indented_By);
  std::pair<StringRef, StringRef> ਵੰਡੋ_2 = HelpStr.ਵੰਡੋ('\n');
  outs().indent(ਅਧਾਰ_Indent - ਪਹਿਲਾ_ਸਤਰ_Indented_By)
      << Arg_Help_ਅਗੇਤਰ << Val_Help_ਅਗੇਤਰ << ਵੰਡੋ_2.ਪਹਿਲਾ_2 << "\n";
  ਜਦੋਂ (!ਵੰਡੋ_2.ਦੂਜਾ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਵੰਡੋ_2 = ਵੰਡੋ_2.ਦੂਜਾ_2.ਵੰਡੋ('\n');
    outs().indent(ਅਧਾਰ_Indent + Val_Help_ਅਗੇਤਰ.ਮਾਪ_2()) << ਵੰਡੋ_2.ਪਹਿਲਾ_2 << "\n";
  }
}

// Print out the option for the alias.
ਖਾਲੀ alias::printOptionInfo(size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_ਚੌੜਾਈ, arg_Plus_Prefixes_ਮਾਪ(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_ਚੌੜਾਈ(ਅਚਲ Option &O) ਅਚਲ {
  size_t Len = arg_Plus_Prefixes_ਮਾਪ(O.ArgStr);
  ਸਵੈਚਾਲਿਤ ValName = get_ਮੁੱਲ_Name();
  ਜੇ (!ValName.ਖਾਲੀ_ਥਾਵਾਂ()) {
    size_t FormattingLen = 3;
    ਜੇ (O.getMiscFlags() & Positional_Eats_ਦਲੀਲਾਂ)
      FormattingLen = 6;
    Len += get_ਮੁੱਲ_Str(O, ValName).ਮਾਪ_2() + FormattingLen;
  }

  ਵਾਪਸ Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ਖਾਲੀ basic_parser_impl::printOptionInfo(ਅਚਲ Option &O,
                                        size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  outs() << PrintArg(O.ArgStr);

  ਸਵੈਚਾਲਿਤ ValName = get_ਮੁੱਲ_Name();
  ਜੇ (!ValName.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਜੇ (O.getMiscFlags() & Positional_Eats_ਦਲੀਲਾਂ) {
      outs() << " <" << get_ਮੁੱਲ_Str(O, ValName) << ">...";
    } ਨਹੀਂਤਾਂ ਜੇ (O.get_ਮੁੱਲ_Expected_Flag() == ਮੁੱਲ_Optional)
      outs() << "[=<" << get_ਮੁੱਲ_Str(O, ValName) << ">]";
    ਨਹੀਂਤਾਂ {
      outs() << (O.ArgStr.ਮਾਪ_2() == 1 ? " <" : "=<") << get_ਮੁੱਲ_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_ਚੌੜਾਈ, get_Option_ਚੌੜਾਈ(O));
}

ਖਾਲੀ basic_parser_impl::printOptionName(ਅਚਲ Option &O,
                                        size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_ਚੌੜਾਈ - O.ArgStr.ਮਾਪ_2());
}

// parser<bool> implementation
//
ਤਾਰਕਿਕ parser<ਤਾਰਕਿਕ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ਤਾਰਕਿਕ &ਮੁੱਲ) {
  ਵਾਪਸ parseBool<ਤਾਰਕਿਕ, ਸੱਚ, ਝੂਠ>(O, ArgName, Arg, ਮੁੱਲ);
}

// parser<boolOrDefault> implementation
//
ਤਾਰਕਿਕ parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &ਮੁੱਲ) {
  ਵਾਪਸ parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, ਮੁੱਲ);
}

// parser<FixedOrScalableQuantity> implementation
//
ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ FixedOrScalableQuantityT>
ਸਥਿਰ ਤਾਰਕਿਕ parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ਮੁੱਲ_ਕਿਸਮ,
                                         FixedOrScalableQuantityT &ਮੁੱਲ) {
  ਵਰਤੋਂ ScalarTy = ਕਿਸਮ_ਨਾਂ FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_ਮੁੱਲ;
  ਜੇ (!Arg.getAsInteger(0, Min_ਮੁੱਲ)) {
    ਮੁੱਲ = FixedOrScalableQuantityT::getFixed(Min_ਮੁੱਲ);
    ਵਾਪਸ ਝੂਠ;
  }

  StringRef Remainder = Arg;
  ਜੇ (!Remainder.consume_front("vscale"))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for " + ਮੁੱਲ_ਕਿਸਮ +
                   " argument!");

  Remainder = Remainder.ltrim();
  ਜੇ (!Remainder.consume_front('x'))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for " + ਮੁੱਲ_ਕਿਸਮ +
                   " argument!");

  Remainder = Remainder.ltrim();
  ਜੇ (Remainder.getAsInteger(0, Min_ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for " + ਮੁੱਲ_ਕਿਸਮ +
                   " argument!");

  ਮੁੱਲ = FixedOrScalableQuantityT::getScalable(Min_ਮੁੱਲ);
  ਵਾਪਸ ਝੂਠ;
}

// parser<int> implementation
//
ਤਾਰਕਿਕ parser<ਪੂਰਨ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        ਪੂਰਨ &ਮੁੱਲ) {
  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for integer argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<long> implementation
//
ਤਾਰਕਿਕ parser<ਲੰਮਾ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ਲੰਮਾ &ਮੁੱਲ) {
  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for long argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<long long> implementation
//
ਤਾਰਕਿਕ parser<ਲੰਮਾ ਲੰਮਾ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              ਲੰਮਾ ਲੰਮਾ &ਮੁੱਲ) {
  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for llong argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<unsigned> implementation
//
ਤਾਰਕਿਕ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ &ਮੁੱਲ) {

  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for uint argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<unsigned long> implementation
//
ਤਾਰਕਿਕ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ &ਮੁੱਲ) {

  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for ulong argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<unsigned long long> implementation
//
ਤਾਰਕਿਕ parser<ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ &ਮੁੱਲ) {

  ਜੇ (Arg.getAsInteger(0, ਮੁੱਲ))
    ਵਾਪਸ O.error("'" + Arg + "' value invalid for ullong argument!");
  ਵਾਪਸ ਝੂਠ;
}

// parser<ElementCount> implementation
//
ਤਾਰਕਿਕ parser<Element_ਗਿਣਤੀ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ਗਿਣਤੀ &ਮੁੱਲ) {
  ਵਾਪਸ parseFixedOrScalableQuantity(O, Arg, get_ਮੁੱਲ_Name(), ਮੁੱਲ);
}

// parser<double>/parser<float> implementation
//
ਸਥਿਰ ਤਾਰਕਿਕ parseDouble(Option &O, StringRef Arg, ਦੁੱਗਣਾ &ਮੁੱਲ) {
  ਜੇ (ਮੰਜ਼ਿਲ_float(Arg, ਮੁੱਲ))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ O.error("'" + Arg + "' value invalid for floating point argument!");
}

ਤਾਰਕਿਕ parser<ਦੁੱਗਣਾ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ਦੁੱਗਣਾ &Val) {
  ਵਾਪਸ parseDouble(O, Arg, Val);
}

ਤਾਰਕਿਕ parser<ਦਸ਼ਮਲਵ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ਦਸ਼ਮਲਵ &Val) {
  ਦੁੱਗਣਾ dVal;
  ਜੇ (parseDouble(O, Arg, dVal))
    ਵਾਪਸ ਸੱਚ;
  Val = (ਦਸ਼ਮਲਵ)dVal;
  ਵਾਪਸ ਝੂਠ;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
ਬਿਨਾਨਿਸ਼ਾਨੀ generic_parser_ਅਧਾਰ::ਲੱਭੋ_Option(StringRef Name) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ e = get_ਅੰਕ_Options();

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != e; ++i) {
    ਜੇ (getOption(i) == Name)
      ਵਾਪਸ i;
  }
  ਵਾਪਸ e;
}

ਸਥਿਰ StringRef Eq_ਮੁੱਲ = "=<value>";
ਸਥਿਰ StringRef ਖਾਲੀ_ਥਾਵਾਂ_Option = "<empty>";
ਸਥਿਰ StringRef Option_ਅਗੇਤਰ = "    =";
ਸਥਿਰ size_t get_Option_Prefixes_ਮਾਪ() {
  ਵਾਪਸ Option_ਅਗੇਤਰ.ਮਾਪ_2() + Arg_Help_ਅਗੇਤਰ.ਮਾਪ_2();
}

ਸਥਿਰ ਤਾਰਕਿਕ shouldPrintOption(StringRef Name, StringRef Description,
                              ਅਚਲ Option &O) {
  ਵਾਪਸ O.get_ਮੁੱਲ_Expected_Flag() != ਮੁੱਲ_Optional || !Name.ਖਾਲੀ_ਥਾਵਾਂ() ||
         !Description.ਖਾਲੀ_ਥਾਵਾਂ();
}

// Return the width of the option tag for printing...
size_t generic_parser_ਅਧਾਰ::get_Option_ਚੌੜਾਈ(ਅਚਲ Option &O) ਅਚਲ {
  ਜੇ (O.hasArgStr()) {
    size_t ਮਾਪ =
        arg_Plus_Prefixes_ਮਾਪ(O.ArgStr) + Eq_ਮੁੱਲ.ਮਾਪ_2();
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      ਜੇ (!shouldPrintOption(Name, getDescription(i), O))
        ਜਾਰੀ;
      size_t Name_ਮਾਪ = Name.ਖਾਲੀ_ਥਾਵਾਂ() ? ਖਾਲੀ_ਥਾਵਾਂ_Option.ਮਾਪ_2() : Name.ਮਾਪ_2();
      ਮਾਪ = std::max(ਮਾਪ, Name_ਮਾਪ + get_Option_Prefixes_ਮਾਪ());
    }
    ਵਾਪਸ ਮਾਪ;
  } ਨਹੀਂਤਾਂ {
    size_t ਅਧਾਰ_ਮਾਪ = 0;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Options(); i != e; ++i)
      ਅਧਾਰ_ਮਾਪ = std::max(ਅਧਾਰ_ਮਾਪ, getOption(i).ਮਾਪ_2() + 8);
    ਵਾਪਸ ਅਧਾਰ_ਮਾਪ;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ਖਾਲੀ generic_parser_ਅਧਾਰ::printOptionInfo(ਅਚਲ Option &O,
                                          size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  ਜੇ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    ਜੇ (O.get_ਮੁੱਲ_Expected_Flag() == ਮੁੱਲ_Optional) {
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Options(); i != e; ++i) {
        ਜੇ (getOption(i).ਖਾਲੀ_ਥਾਵਾਂ()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_ਚੌੜਾਈ,
                               arg_Plus_Prefixes_ਮਾਪ(O.ArgStr));
          ਰੋਕੋ;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_ਮੁੱਲ;
    Option::printHelpStr(O.HelpStr, Global_ਚੌੜਾਈ,
                         Eq_ਮੁੱਲ.ਮਾਪ_2() +
                             arg_Plus_Prefixes_ਮਾਪ(O.ArgStr));
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      ਜੇ (!shouldPrintOption(OptionName, Description, O))
        ਜਾਰੀ;
      size_t ਪਹਿਲਾ_ਸਤਰ_Indent = OptionName.ਮਾਪ_2() + get_Option_Prefixes_ਮਾਪ();
      outs() << Option_ਅਗੇਤਰ << OptionName;
      ਜੇ (OptionName.ਖਾਲੀ_ਥਾਵਾਂ()) {
        outs() << ਖਾਲੀ_ਥਾਵਾਂ_Option;
        ਪ੍ਰਤਿਜ੍ਞਾ(ਪਹਿਲਾ_ਸਤਰ_Indent >= ਖਾਲੀ_ਥਾਵਾਂ_Option.ਮਾਪ_2());
        ਪਹਿਲਾ_ਸਤਰ_Indent += ਖਾਲੀ_ਥਾਵਾਂ_Option.ਮਾਪ_2();
      }
      ਜੇ (!Description.ਖਾਲੀ_ਥਾਵਾਂ())
        Option::printEnumValHelpStr(Description, Global_ਚੌੜਾਈ, ਪਹਿਲਾ_ਸਤਰ_Indent);
      ਨਹੀਂਤਾਂ
        outs() << '\n';
    }
  } ਨਹੀਂਤਾਂ {
    ਜੇ (!O.HelpStr.ਖਾਲੀ_ਥਾਵਾਂ())
      outs() << "  " << O.HelpStr << '\n';
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_ਚੌੜਾਈ, Option.ਮਾਪ_2() + 8);
    }
  }
}

ਸਥਿਰ ਅਚਲ size_t Max_Opt_ਚੌੜਾਈ = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
ਖਾਲੀ generic_parser_ਅਧਾਰ::printGenericOptionDiff(
    ਅਚਲ Option &O, ਅਚਲ Generic_Option_ਮੁੱਲ &ਮੁੱਲ,
    ਅਚਲ Generic_Option_ਮੁੱਲ &Default, size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_ਚੌੜਾਈ - O.ArgStr.ਮਾਪ_2());

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Opts = get_ਅੰਕ_Options();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਅੰਕ_Opts; ++i) {
    ਜੇ (!ਮੁੱਲ.compare(get_Option_ਮੁੱਲ(i)))
      ਜਾਰੀ;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).ਮਾਪ_2();
    size_t ਅੰਕ_Spaces = Max_Opt_ਚੌੜਾਈ > L ? Max_Opt_ਚੌੜਾਈ - L : 0;
    outs().indent(ਅੰਕ_Spaces) << " (default: ";
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ j = 0; j != ਅੰਕ_Opts; ++j) {
      ਜੇ (!Default.compare(get_Option_ਮੁੱਲ(j)))
        ਜਾਰੀ;
      outs() << getOption(j);
      ਰੋਕੋ;
    }
    outs() << ")\n";
    ਵਾਪਸ;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ cl {
ਸਥਿਰ raw_ostream &ਸੰਚਾਲਕ<<(raw_ostream &OS, boolOrDefault V) {
  ਵਾਪਸ OS << static_cast<ਪੂਰਨ>(V);
}
} // namespace cl
} // namespace llvm

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRINT_OPT_DIFF(T)                                                      \
  ਖਾਲੀ parser<T>::printOptionDiff(ਅਚਲ Option &O, T V, Option_ਮੁੱਲ<T> D,      \
                                  size_t Global_ਚੌੜਾਈ) ਅਚਲ {                  \
    printOptionName(O, Global_ਚੌੜਾਈ);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t ਅੰਕ_Spaces =                                                         \
        Max_Opt_ਚੌੜਾਈ > Str.ਮਾਪ_2() ? Max_Opt_ਚੌੜਾਈ - Str.ਮਾਪ_2() : 0;               \
    outs().indent(ਅੰਕ_Spaces) << " (default: ";                                 \
    ਜੇ (D.has_ਮੁੱਲ())                                                          \
      outs() << D.get_ਮੁੱਲ();                                                  \
    ਨਹੀਂਤਾਂ                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(ਤਾਰਕਿਕ)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(ਪੂਰਨ)
PRINT_OPT_DIFF(ਲੰਮਾ)
PRINT_OPT_DIFF(ਲੰਮਾ ਲੰਮਾ)
PRINT_OPT_DIFF(ਬਿਨਾਨਿਸ਼ਾਨੀ)
PRINT_OPT_DIFF(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)
PRINT_OPT_DIFF(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)
PRINT_OPT_DIFF(ਦੁੱਗਣਾ)
PRINT_OPT_DIFF(ਦਸ਼ਮਲਵ)
PRINT_OPT_DIFF(ਅੱਖਰ)
PRINT_OPT_DIFF(Element_ਗਿਣਤੀ)

ਖਾਲੀ parser<std::string>::printOptionDiff(ਅਚਲ Option &O, StringRef V,
                                          ਅਚਲ Option_ਮੁੱਲ<std::string> &D,
                                          size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  printOptionName(O, Global_ਚੌੜਾਈ);
  outs() << "= " << V;
  size_t ਅੰਕ_Spaces = Max_Opt_ਚੌੜਾਈ > V.ਮਾਪ_2() ? Max_Opt_ਚੌੜਾਈ - V.ਮਾਪ_2() : 0;
  outs().indent(ਅੰਕ_Spaces) << " (default: ";
  ਜੇ (D.has_ਮੁੱਲ())
    outs() << D.get_ਮੁੱਲ();
  ਨਹੀਂਤਾਂ
    outs() << "*no default*";
  outs() << ")\n";
}

ਖਾਲੀ parser<std::optional<std::string>>::printOptionDiff(
    ਅਚਲ Option &O, std::optional<StringRef> V,
    ਅਚਲ Option_ਮੁੱਲ<std::optional<std::string>> &D,
    size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  printOptionName(O, Global_ਚੌੜਾਈ);
  outs() << "= " << V;
  size_t VSize = V.has_ਮੁੱਲ_2() ? V.ਮੁੱਲ_2().ਮਾਪ_2() : 0;
  size_t ਅੰਕ_Spaces = Max_Opt_ਚੌੜਾਈ > VSize ? Max_Opt_ਚੌੜਾਈ - VSize : 0;
  outs().indent(ਅੰਕ_Spaces) << " (default: ";
  ਜੇ (D.has_ਮੁੱਲ() && D.get_ਮੁੱਲ().has_ਮੁੱਲ_2())
    outs() << D.get_ਮੁੱਲ();
  ਨਹੀਂਤਾਂ
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
ਖਾਲੀ basic_parser_impl::print_Option_No_ਮੁੱਲ(ਅਚਲ Option &O,
                                           size_t Global_ਚੌੜਾਈ) ਅਚਲ {
  printOptionName(O, Global_ਚੌੜਾਈ);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ਸਥਿਰ ਪੂਰਨ OptNameCompare(ਅਚਲ std::pair<ਅਚਲ ਅੱਖਰ *, Option *> *LHS,
                          ਅਚਲ std::pair<ਅਚਲ ਅੱਖਰ *, Option *> *RHS) {
  ਵਾਪਸ strcmp(LHS->ਪਹਿਲਾ_2, RHS->ਪਹਿਲਾ_2);
}

ਸਥਿਰ ਪੂਰਨ SubNameCompare(ਅਚਲ std::pair<ਅਚਲ ਅੱਖਰ *, SubCommand *> *LHS,
                          ਅਚਲ std::pair<ਅਚਲ ਅੱਖਰ *, SubCommand *> *RHS) {
  ਵਾਪਸ strcmp(LHS->ਪਹਿਲਾ_2, RHS->ਪਹਿਲਾ_2);
}

// Copy Options into a vector so we can sort them as we like.
ਸਥਿਰ ਖਾਲੀ ਛਾਂਟੋ_Opts(OptionsMapTy &OptMap,
                     Small_ਸਦਿਸ਼_Impl<std::pair<ਅਚਲ ਅੱਖਰ *, Option *>> &Opts,
                     ਤਾਰਕਿਕ ShowHidden) {
  Small_Ptr_ਸੈੱਟ<Option *, 32> Option_ਸੈੱਟ; // Duplicate option detection.

  ਲਈ (ਸਵੈਚਾਲਿਤ I = OptMap.begin(), E = OptMap.ਸਿਰਾ(); I != E; ++I) {
    // Ignore really-hidden options.
    ਜੇ (I->ਦੂਜਾ_2->getOptionHiddenFlag() == ReallyHidden)
      ਜਾਰੀ;

    // Unless showhidden is set, ignore hidden flags.
    ਜੇ (I->ਦੂਜਾ_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ਜਾਰੀ;

    // If we've already seen this option, don't add it to the list again.
    ਜੇ (!Option_ਸੈੱਟ.ਪਾਓ(I->ਦੂਜਾ_2).ਦੂਜਾ_2)
      ਜਾਰੀ;

    Opts.ਧੱਕੋ_ਪਿੱਠ(
        std::pair<ਅਚਲ ਅੱਖਰ *, Option *>(I->ਪਹਿਲਾ_2.ਅੰਕੜੇ(), I->ਦੂਜਾ_2));
  }

  // Sort the options list alphabetically.
  array_pod_ਛਾਂਟੋ(Opts.begin(), Opts.ਸਿਰਾ(), OptNameCompare);
}

ਸਥਿਰ ਖਾਲੀ
ਛਾਂਟੋ_Sub_Commands(ਅਚਲ Small_Ptr_ਸੈੱਟ_Impl<SubCommand *> &SubMap,
                Small_ਸਦਿਸ਼_Impl<std::pair<ਅਚਲ ਅੱਖਰ *, SubCommand *>> &Subs) {
  ਲਈ (ਸਵੈਚਾਲਿਤ *S : SubMap) {
    ਜੇ (S->getName().ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;
    Subs.ਧੱਕੋ_ਪਿੱਠ(std::make_pair(S->getName().ਅੰਕੜੇ(), S));
  }
  array_pod_ਛਾਂਟੋ(Subs.begin(), Subs.ਸਿਰਾ(), SubNameCompare);
}

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ HelpPrinter {
ਸੁਰੱਖਿਅਤ:
  ਅਚਲ ਤਾਰਕਿਕ ShowHidden;
  ਵਰਤੋਂ Str_Option_Pair_ਸਦਿਸ਼ =
      Small_ਸਦਿਸ਼<std::pair<ਅਚਲ ਅੱਖਰ *, Option *>, 128>;
  ਵਰਤੋਂ Str_Sub_Command_Pair_ਸਦਿਸ਼ =
      Small_ਸਦਿਸ਼<std::pair<ਅਚਲ ਅੱਖਰ *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  ਆਭਾਸੀ ਖਾਲੀ printOptions(Str_Option_Pair_ਸਦਿਸ਼ &Opts, size_t MaxArgLen) {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Opt : Opts)
      Opt.ਦੂਜਾ_2->printOptionInfo(MaxArgLen);
  }

  ਖਾਲੀ printSubCommands(Str_Sub_Command_Pair_ਸਦਿਸ਼ &Subs, size_t MaxSubLen) {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &S : Subs) {
      outs() << "  " << S.ਪਹਿਲਾ_2;
      ਜੇ (!S.ਦੂਜਾ_2->getDescription().ਖਾਲੀ_ਥਾਵਾਂ()) {
        outs().indent(MaxSubLen - ਲੜੀ_ਲੰਬਾਈ(S.ਪਹਿਲਾ_2));
        outs() << " - " << S.ਦੂਜਾ_2->getDescription();
      }
      outs() << "\n";
    }
  }

ਜਨਤਕ:
  ਸਪਸ਼ਟ HelpPrinter(ਤਾਰਕਿਕ showHidden) : ShowHidden(showHidden) {}
  ਆਭਾਸੀ ~HelpPrinter() = ਮੂਲ;

  // Invoke the printer.
  ਖਾਲੀ ਸੰਚਾਲਕ=(ਤਾਰਕਿਕ ਮੁੱਲ) {
    ਜੇ (!ਮੁੱਲ)
      ਵਾਪਸ;
    printHelp();

    // Halt the program since help information was printed
    ਬਾਹਰ_ਨਿਕਲੋ(0);
  }

  ਖਾਲੀ printHelp() {
    SubCommand *Sub = globalParser().get_ਸਰਗਰਮ_Sub_Command();
    ਸਵੈਚਾਲਿਤ &OptionsMap = Sub->OptionsMap;
    ਸਵੈਚਾਲਿਤ &PositionalOpts = Sub->PositionalOpts;
    ਸਵੈਚਾਲਿਤ &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_ਸਦਿਸ਼ Opts;
    ਛਾਂਟੋ_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_ਸਦਿਸ਼ Subs;
    ਛਾਂਟੋ_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    ਜੇ (!globalParser().ਪ੍ਰੋਗਰਾਮ_Overview.ਖਾਲੀ_ਥਾਵਾਂ())
      outs() << "OVERVIEW: " << globalParser().ਪ੍ਰੋਗਰਾਮ_Overview << "\n";

    ਜੇ (Sub == &SubCommand::get_ਸਿਖਰ_Level()) {
      outs() << "USAGE: " << globalParser().ਪ੍ਰੋਗਰਾਮ_Name;
      ਜੇ (!Subs.ਖਾਲੀ_ਥਾਵਾਂ())
        outs() << " [subcommand]";
      outs() << " [options]";
    } ਨਹੀਂਤਾਂ {
      ਜੇ (!Sub->getDescription().ਖਾਲੀ_ਥਾਵਾਂ()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ਪ੍ਰੋਗਰਾਮ_Name << " " << Sub->getName()
             << " [options]";
    }

    ਲਈ (ਸਵੈਚਾਲਿਤ *Opt : PositionalOpts) {
      ਜੇ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    ਜੇ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    ਜੇ (Sub == &SubCommand::get_ਸਿਖਰ_Level() && !Subs.ਖਾਲੀ_ਥਾਵਾਂ()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, ਲੜੀ_ਲੰਬਾਈ(Sub.ਪਹਿਲਾ_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().ਪ੍ਰੋਗਰਾਮ_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.ਦੂਜਾ_2->get_Option_ਚੌੜਾਈ());

    Small_ਸਦਿਸ਼<std::pair<std::string, StringRef>, 0> LibraryOpts;
    ਜੇ (ShowHidden)
      ਲਈ (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              ਜੇ (!Help.ਖਾਲੀ_ਥਾਵਾਂ())
                LibraryOpts.emplace_ਪਿੱਠ((Spelling + MetaVar).str(), Help);
            });
    llvm::ਛਾਂਟੋ(LibraryOpts);
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_ਮਾਪ(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    ਜੇ (!LibraryOpts.ਖਾਲੀ_ਥਾਵਾਂ())
      outs() << "\nLibrary options:\n\n";
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_ਮਾਪ(Name));
    }

    // Print any extra help the user has declared.
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &I : globalParser().ਹੋਰ_Help)
      outs() << I;
    globalParser().ਹੋਰ_Help.clear();
  }
};

ਵਰਗ CategorizedHelpPrinter : ਜਨਤਕ HelpPrinter {
ਜਨਤਕ:
  ਸਪਸ਼ਟ CategorizedHelpPrinter(ਤਾਰਕਿਕ showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ਸਥਿਰ ਪੂਰਨ OptionCategoryCompare(OptionCategory *ਅਚਲ *A,
                                   OptionCategory *ਅਚਲ *B) {
    ਵਾਪਸ (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  ਵਰਤੋਂ HelpPrinter::ਸੰਚਾਲਕ=;

ਸੁਰੱਖਿਅਤ:
  ਖਾਲੀ printOptions(Str_Option_Pair_ਸਦਿਸ਼ &Opts, size_t MaxArgLen) override {
    std::ਸਦਿਸ਼<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::ਸਦਿਸ਼<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ਪ੍ਰਤਿਜ੍ਞਾ(SortedCategories.ਮਾਪ_2() > 0 && "No option categories registered!");
    array_pod_ਛਾਂਟੋ(SortedCategories.begin(), SortedCategories.ਸਿਰਾ(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &I : Opts) {
      Option *Opt = I.ਦੂਜਾ_2;
      ਲਈ (OptionCategory *Cat : Opt->Categories) {
        ਪ੍ਰਤਿਜ੍ਞਾ(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ਧੱਕੋ_ਪਿੱਠ(Opt);
      }
    }

    // Now do printing.
    ਲਈ (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ਅਚਲ ਸਵੈਚਾਲਿਤ &CategoryOptions = CategorizedOptions[Category];
      ਜੇ (CategoryOptions.ਖਾਲੀ_ਥਾਵਾਂ())
        ਜਾਰੀ;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      ਜੇ (!Category->getDescription().ਖਾਲੀ_ਥਾਵਾਂ())
        outs() << Category->getDescription() << "\n\n";
      ਨਹੀਂਤਾਂ
        outs() << "\n";

      // Loop over the options in the category and print.
      ਲਈ (ਅਚਲ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
ਵਰਗ HelpPrinterWrapper {
ਨਿਜੀ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

ਜਨਤਕ:
  ਸਪਸ਼ਟ HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  ਖਾਲੀ ਸੰਚਾਲਕ=(ਤਾਰਕਿਕ ਮੁੱਲ);
};

} // End anonymous namespace

#ਜੇ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# ਜੇ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# ਨਹੀਂਤਾਂ
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#ਜਾਂਜੇ defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# ਜੇ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# ਨਹੀਂਤਾਂ
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#ਨਹੀਂਤਾਂ
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#ਜੇ_ਅੰਤ

ਨਾਮਾਕਾਸ਼ {
ਵਰਗ ਸੰਸਕਰਣ_Printer {
ਜਨਤਕ:
  ਖਾਲੀ print(ਅਚਲ std::ਸਦਿਸ਼<ਸੰਸਕਰਣ_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#ਨਹੀਂਤਾਂ
    OS << "LLVM (http://llvm.org/):\n  ";
#ਜੇ_ਅੰਤ
    OS << PACKAGE_NAME << " version " << PACKAGE_ਸੰਸਕਰਣ << "\n  ";
#ਜੇ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#ਨਹੀਂਤਾਂ
    OS << "Optimized build";
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
    OS << " with assertions";
#ਜੇ_ਅੰਤ
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    ਜੇ (!ExtraPrinters.ਖਾਲੀ_ਥਾਵਾਂ()) {
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &I : ExtraPrinters)
        I(outs());
    }
  }
  ਖਾਲੀ ਸੰਚਾਲਕ=(ਤਾਰਕਿਕ OptionWasSpecified);
};

ਬਣਤਰ Command_ਸਤਰ_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_ਸਾਧਾਰਨ_Printer{ਝੂਠ};
  HelpPrinter UncategorizedHiddenPrinter{ਸੱਚ};
  CategorizedHelpPrinter Categorized_ਸਾਧਾਰਨ_Printer{ਝੂਠ};
  CategorizedHelpPrinter CategorizedHiddenPrinter{ਸੱਚ};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_ਸਾਧਾਰਨ_Printer{Uncategorized_ਸਾਧਾਰਨ_Printer,
                                          Categorized_ਸਾਧਾਰਨ_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, ਸੱਚ, parser<ਤਾਰਕਿਕ>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_ਸਾਧਾਰਨ_Printer),
      cl::Hidden,
      cl::ਮੁੱਲ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, ਸੱਚ, parser<ਤਾਰਕਿਕ>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ਮੁੱਲ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, ਸੱਚ, parser<ਤਾਰਕਿਕ>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_ਸਾਧਾਰਨ_Printer),
      cl::ਮੁੱਲ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, ਸੱਚ, parser<ਤਾਰਕਿਕ>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ਮੁੱਲ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ਤਾਰਕਿਕ> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(ਝੂਠ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ਤਾਰਕਿਕ> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(ਝੂਠ),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  ਸੰਸਕਰਣ_Printer_Ty Override_ਸੰਸਕਰਣ_Printer = ਖਾਲੀ_ਸੂਚਕ;

  std::ਸਦਿਸ਼<ਸੰਸਕਰਣ_Printer_Ty> Extra_ਸੰਸਕਰਣ_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  ਸੰਸਕਰਣ_Printer ਸੰਸਕਰਣ_Printer_Instance;

  cl::opt<ਸੰਸਕਰਣ_Printer, ਸੱਚ, parser<ਤਾਰਕਿਕ>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(ਸੰਸਕਰਣ_Printer_Instance), cl::ਮੁੱਲ_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ਸਥਿਰ ManagedStatic<Command_ਸਤਰ_Common_Options> CommonOptions;

ਸਥਿਰ ਖਾਲੀ initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_ਗ੍ਰਾਫ_Writer_Options();
  init_ਸੰਕੇਤ_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_ਬੀਜ_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ਸਥਿਰ OptionCategory GeneralCategory{"General options"};
  ਵਾਪਸ GeneralCategory;
}

ਖਾਲੀ ਸੰਸਕਰਣ_Printer::ਸੰਚਾਲਕ=(ਤਾਰਕਿਕ OptionWasSpecified) {
  ਜੇ (!OptionWasSpecified)
    ਵਾਪਸ;

  ਜੇ (CommonOptions->Override_ਸੰਸਕਰਣ_Printer != ਖਾਲੀ_ਸੂਚਕ) {
    CommonOptions->Override_ਸੰਸਕਰਣ_Printer(outs());
    ਬਾਹਰ_ਨਿਕਲੋ(0);
  }
  print(CommonOptions->Extra_ਸੰਸਕਰਣ_Printers);

  ਬਾਹਰ_ਨਿਕਲੋ(0);
}

ਖਾਲੀ HelpPrinterWrapper::ਸੰਚਾਲਕ=(ਤਾਰਕਿਕ ਮੁੱਲ) {
  ਜੇ (!ਮੁੱਲ)
    ਵਾਪਸ;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  ਜੇ (globalParser().RegisteredOptionCategories.ਮਾਪ_2() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.ਸੈੱਟ_Hidden_Flag(NotHidden);

    CategorizedPrinter = ਸੱਚ; // Invoke categorized printer
  } ਨਹੀਂਤਾਂ {
    UncategorizedPrinter = ਸੱਚ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
ਖਾਲੀ cl::Print_Option_ਮੁੱਲਾਂ() { globalParser().print_Option_ਮੁੱਲਾਂ(); }

ਖਾਲੀ Command_ਸਤਰ_Parser::print_Option_ਮੁੱਲਾਂ() {
  ਜੇ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ਵਾਪਸ;

  Small_ਸਦਿਸ਼<std::pair<ਅਚਲ ਅੱਖਰ *, Option *>, 128> Opts;
  ਛਾਂਟੋ_Opts(ਸਰਗਰਮ_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ ਸੱਚ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.ਦੂਜਾ_2->get_Option_ਚੌੜਾਈ());

  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Opt : Opts)
    Opt.ਦੂਜਾ_2->print_Option_ਮੁੱਲ(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
ਖਾਲੀ cl::Print_Help_ਸੁਨੇਹਾ(ਤਾਰਕਿਕ Hidden, ਤਾਰਕਿਕ Categorized) {
  ਜੇ (!Hidden && !Categorized)
    CommonOptions->Uncategorized_ਸਾਧਾਰਨ_Printer.printHelp();
  ਨਹੀਂਤਾਂ ਜੇ (!Hidden && Categorized)
    CommonOptions->Categorized_ਸਾਧਾਰਨ_Printer.printHelp();
  ਨਹੀਂਤਾਂ ਜੇ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  ਨਹੀਂਤਾਂ
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ਸਥਿਰ ਅਚਲ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#ਜੇ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
      "+assertions",
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EXPENSIVE_CHECKS
      "+expensive-checks",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(ਪਤਾ_sanitizer)
      "+asan",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(dataflow_sanitizer)
      "+dfsan",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(ਸਮ੍ਰਿਤੀ_sanitizer)
      "+msan",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(thread_sanitizer)
      "+tsan",
#ਜੇ_ਅੰਤ
#ਜੇ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#ਜੇ_ਅੰਤ
  };
  ਵਾਪਸ ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
ਖਾਲੀ cl::printBuildConfig(raw_ostream &OS) {
#ਜੇ LLVM_ਸੰਸਕਰਣ_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#ਜੇ_ਅੰਤ
}

/// Utility function for printing version number.
ਖਾਲੀ cl::Print_ਸੰਸਕਰਣ_ਸੁਨੇਹਾ() {
  CommonOptions->ਸੰਸਕਰਣ_Printer_Instance.print(CommonOptions->Extra_ਸੰਸਕਰਣ_Printers);
}

ਖਾਲੀ cl::ਸੈੱਟ_ਸੰਸਕਰਣ_Printer(ਸੰਸਕਰਣ_Printer_Ty func) {
  CommonOptions->Override_ਸੰਸਕਰਣ_Printer = func;
}

ਖਾਲੀ cl::ਜੋੜੋ_Extra_ਸੰਸਕਰਣ_Printer(ਸੰਸਕਰਣ_Printer_Ty func) {
  CommonOptions->Extra_ਸੰਸਕਰਣ_Printers.ਧੱਕੋ_ਪਿੱਠ(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  ਸਵੈਚਾਲਿਤ &Subs = globalParser().RegisteredSubCommands;
  (ਖਾਲੀ)Subs;
  ਪ੍ਰਤਿਜ੍ਞਾ(Subs.ਸ਼ਾਮਲ_ਹੈ(&Sub));
  ਵਾਪਸ Sub.OptionsMap;
}

iterator_range<Small_Ptr_ਸੈੱਟ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ਵਾਪਸ globalParser().getRegisteredSubcommands();
}

ਖਾਲੀ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  ਲਈ (ਸਵੈਚਾਲਿਤ &I : Sub.OptionsMap) {
    ਤਾਰਕਿਕ Unrelated = ਸੱਚ;
    ਲਈ (ਸਵੈਚਾਲਿਤ &Cat : I.ਦੂਜਾ_2->Categories) {
      ਜੇ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = ਝੂਠ;
    }
    ਜੇ (Unrelated)
      I.ਦੂਜਾ_2->ਸੈੱਟ_Hidden_Flag(cl::ReallyHidden);
  }
}

ਖਾਲੀ cl::HideUnrelatedOptions(ArrayRef<ਅਚਲ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  ਲਈ (ਸਵੈਚਾਲਿਤ &I : Sub.OptionsMap) {
    ਤਾਰਕਿਕ Unrelated = ਸੱਚ;
    ਲਈ (ਸਵੈਚਾਲਿਤ &Cat : I.ਦੂਜਾ_2->Categories) {
      ਜੇ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = ਝੂਠ;
    }
    ਜੇ (Unrelated)
      I.ਦੂਜਾ_2->ਸੈੱਟ_Hidden_Flag(cl::ReallyHidden);
  }
}

ਖਾਲੀ cl::Reset_Command_ਸਤਰ_Parser() { globalParser().reset(); }

ਖਾਲੀ cl::ਜੋੜੋ_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ਧੱਕੋ_ਪਿੱਠ(&L);
}

ਖਾਲੀ LibraryOptionsContext::ਸੌਂਪੋ(ਅਚਲ LibraryOptionsContext &Other) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ != &Other);
  ਮੁੱਲਾਂ.clear();
  Saved_ਦਲੀਲਾਂ.clear();
  ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ = Other.Saved_ਦਲੀਲਾਂ;
  ਲਈ (size_t I = 0, E = ਦਲੀਲਾਂ.ਮਾਪ_2(); I != E;) {
    StringRef Name = ਦਲੀਲਾਂ[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.ਵੰਡੋ('=').ਪਹਿਲਾ_2);
    ਬਿਨਾਨਿਸ਼ਾਨੀ N;
    cantFail(parse(*L, ਦਲੀਲਾਂ.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ,
                                   ਬਿਨਾਨਿਸ਼ਾਨੀ &Consumed) {
  ਸਵੈਚਾਲਿਤ [It, Inserted] = ਮੁੱਲਾਂ.try_emplace(L.getDefault());
  ਜੇ (Inserted)
    It->ਦੂਜਾ_2 = L.copyDefault(Alloc);
  ਵਾਪਸ parseLibraryOption(L, It->ਦੂਜਾ_2, ਦਲੀਲਾਂ, Consumed, Alloc, &Saved_ਦਲੀਲਾਂ);
}

ਖਾਲੀ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

ਖਾਲੀ LLVMParse_Command_ਸਤਰ_Options(ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
                                 ਅਚਲ ਅੱਖਰ *Overview) {
  llvm::cl::Parse_Command_ਸਤਰ_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
