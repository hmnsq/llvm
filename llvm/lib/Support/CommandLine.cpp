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
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ cl;

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ cl {
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ತಾರ್ಕಿಕ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಪೂರ್ಣಾಂಕ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ದೀರ್ಘ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ದೀರ್ಘ ದೀರ್ಘ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಚಿಹ್ನೆರಹಿತ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ದ್ವಿಗುಣ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ದಶಮಾಂಶ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<ಅಕ್ಷರ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE basic_parser<Element_ಎಣಿಕೆ>;

#ಆದರೆ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<std::string>;
#ಆದರೆ_ಅಂತ್ಯ

ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ತಾರ್ಕಿಕ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ಅಕ್ಷರ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ಪೂರ್ಣಾಂಕ>;
ಮಾದರಿ ವರ್ಗ LLVM_EXPORT_TEMPLATE opt<ಚಿಹ್ನೆರಹಿತ>;

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
ಶೂನ್ಯ parser<ಪೂರ್ಣಾಂಕ>::anchor() {}
ಶೂನ್ಯ parser<ದೀರ್ಘ>::anchor() {}
ಶೂನ್ಯ parser<ದೀರ್ಘ ದೀರ್ಘ>::anchor() {}
ಶೂನ್ಯ parser<ಚಿಹ್ನೆರಹಿತ>::anchor() {}
ಶೂನ್ಯ parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ>::anchor() {}
ಶೂನ್ಯ parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ>::anchor() {}
ಶೂನ್ಯ parser<ದ್ವಿಗುಣ>::anchor() {}
ಶೂನ್ಯ parser<ದಶಮಾಂಶ>::anchor() {}
ಶೂನ್ಯ parser<std::string>::anchor() {}
ಶೂನ್ಯ parser<std::optional<std::string>>::anchor() {}
ಶೂನ್ಯ parser<ಅಕ್ಷರ>::anchor() {}
ಶೂನ್ಯ parser<Element_ಎಣಿಕೆ>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
ಶೂನ್ಯ opt_bool_anchor() { opt<ತಾರ್ಕಿಕ> anchor{""}; }
ಶೂನ್ಯ opt_char_anchor() { opt<ಅಕ್ಷರ> anchor{""}; }
ಶೂನ್ಯ opt_int_anchor() { opt<ಪೂರ್ಣಾಂಕ> anchor{""}; }
ಶೂನ್ಯ opt_unsigned_anchor() { opt<ಚಿಹ್ನೆರಹಿತ> anchor{""}; }

//===----------------------------------------------------------------------===//

ಸ್ಥಿರಾಂಕ ಸ್ಥಿರ size_t DefaultPad = 2;

ಸ್ಥಿರ StringRef Arg_ಪೂರ್ವಪ್ರತ್ಯಯ = "-";
ಸ್ಥಿರ StringRef Arg_ಪೂರ್ವಪ್ರತ್ಯಯ_Long = "--";
ಸ್ಥಿರ StringRef Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ = " - ";

ಸ್ಥಿರ size_t arg_Plus_Prefixes_ಗಾತ್ರ(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.ಗಾತ್ರ_3();
  ಆದರೆ (Len == 1)
    ಹಿಂತಿರುಗಿ Len + Pad + Arg_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3() + Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ Len + Pad + Arg_ಪೂರ್ವಪ್ರತ್ಯಯ_Long.ಗಾತ್ರ_3() + Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3();
}

ಸ್ಥಿರ SmallString<8> arg_ಪೂರ್ವಪ್ರತ್ಯಯ(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> ಪೂರ್ವಪ್ರತ್ಯಯ;
  ಗಾಗಿ (size_t I = 0; I < Pad; ++I) {
    ಪೂರ್ವಪ್ರತ್ಯಯ.ತಳ್ಳು_ಹಿಂಭಾಗ(' ');
  }
  ಪೂರ್ವಪ್ರತ್ಯಯ.append(ArgName.ಗಾತ್ರ_3() > 1 ? Arg_ಪೂರ್ವಪ್ರತ್ಯಯ_Long : Arg_ಪೂರ್ವಪ್ರತ್ಯಯ);
  ಹಿಂತಿರುಗಿ ಪೂರ್ವಪ್ರತ್ಯಯ;
}

ಬಳಸಿ OptionsMapTy = DenseMap<StringRef, Option *>;

ನಾಮಾಕಾಶ {

ವರ್ಗ PrintArg {
  StringRef ArgName;
  size_t Pad;
ಸಾರ್ವಜನಿಕ:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  ಮಿತ್ರ raw_ostream &ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಥಿರಾಂಕ PrintArg &);
};

raw_ostream &ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಥಿರಾಂಕ PrintArg& Arg) {
  OS << arg_ಪೂರ್ವಪ್ರತ್ಯಯ(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ಹಿಂತಿರುಗಿ OS;
}

ವರ್ಗ Command_ಸಾಲು_Parser {
ಸಾರ್ವಜನಿಕ:
  // Globals for name and overview of program.  Program name is not a string to
  // avoid static ctor/dtor issues.
  std::string ಕಾರ್ಯಕ್ರಮ_Name;
  StringRef ಕಾರ್ಯಕ್ರಮ_Overview;

  // This collects additional help to be printed.
  std::ಸದಿಶ<StringRef> ಇನ್ನಷ್ಟು_Help;

  // This collects the different option categories that have been registered.
  Small_Ptr_ಗಣ<OptionCategory *, 16> RegisteredOptionCategories;

  // This collects the different subcommands that have been registered.
  Small_Ptr_ಗಣ<SubCommand *, 4> RegisteredSubCommands;

  // Libraries whose options are declared in TableGen, and an index from their
  // option names. Libraries[NumIndexedLibraries:] are not indexed yet.
  Small_ಸದಿಶ<LibraryOptions *, 0> Libraries;
  DenseMap<StringRef, LibraryOptions *> Library_ಸೂಚ್ಯಂಕ;
  size_t ಅಂಕೆ_ಸೂಚ್ಯಂಕಿತ_Libraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  Command_ಸಾಲು_Parser() { ರಿಜಿಸ್ಟರ್_Sub_Command(&SubCommand::get_ತುದಿ_Level()); }

  ಶೂನ್ಯ ResetAllOptionOccurrences();

  ತಾರ್ಕಿಕ Parse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
                               StringRef Overview, raw_ostream *Errs = ಶೂನ್ಯಸೂಚಕ,
                               vfs::FileSystem *VFS = ಶೂನ್ಯಸೂಚಕ,
                               LibraryOptionsContext *Context = ಶೂನ್ಯಸೂಚಕ);

  ಶೂನ್ಯ forEachSubCommand(Option &Opt, function_ref<ಶೂನ್ಯ(SubCommand &)> Action) {
    ಆದರೆ (Opt.Subs.ಖಾಲಿಗಳು()) {
      Action(SubCommand::get_ತುದಿ_Level());
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (Opt.Subs.ಗಾತ್ರ_3() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ಹಿಂತಿರುಗಿ;
    }
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *SC : Opt.Subs) {
      ಪ್ರತಿಜ್ಞಾ(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  ಶೂನ್ಯ ಕೂಡಿಸು_Literal_Option(Option &Opt, SubCommand *SC, StringRef Name) {
    ಆದರೆ (Opt.hasArgStr())
      ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!SC->OptionsMap.ಸೇರಿಸು(std::make_pair(Name, &Opt)).ಎರಡನೇ_2) {
      errs() << ಕಾರ್ಯಕ್ರಮ_Name << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  ಶೂನ್ಯ ಕೂಡಿಸು_Literal_Option(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { ಕೂಡಿಸು_Literal_Option(Opt, &SC, Name); });
  }

  ಶೂನ್ಯ ಕೂಡಿಸು_Option(Option *O, SubCommand *SC) {
    ತಾರ್ಕಿಕ HadErrors = ಸುಳ್ಳು;
    ಆದರೆ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      ಆದರೆ (!SC->OptionsMap.ಸೇರಿಸು(std::make_pair(O->ArgStr, O)).ಎರಡನೇ_2 ||
          (RegisteredSubCommands.ಒಳಗೊಂಡಿದೆ(SC) &&
           Library_ಸೂಚ್ಯಂಕ.ಒಳಗೊಂಡಿದೆ(O->ArgStr))) {
        errs() << ಕಾರ್ಯಕ್ರಮ_Name << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = ನಿಜ;
      }
    }

    // Remember information about positional options.
    ಆದರೆ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.ತಳ್ಳು_ಹಿಂಭಾಗ(O);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (O->get_ಅಂಕೆ_Occurrences_Flag() == cl::ConsumeAfter) {
      ಆದರೆ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = ನಿಜ;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    ಆದರೆ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  ಶೂನ್ಯ ಕೂಡಿಸು_Option(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { ಕೂಡಿಸು_Option(O, &SC); });
  }

  ಶೂನ್ಯ removeOption(Option *O, SubCommand *SC) {
    Small_ಸದಿಶ<StringRef, 16> Option_ಹೆಸರುಗಳು;
    O->get_Extra_Option_ಹೆಸರುಗಳು(Option_ಹೆಸರುಗಳು);
    ಆದರೆ (O->hasArgStr())
      Option_ಹೆಸರುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(O->ArgStr);

    SubCommand &Sub = *SC;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ Name : Option_ಹೆಸರುಗಳು) {
      ಸ್ವಯಂಚಾಲಿತ I = Sub.OptionsMap.ಕಂಡುಹಿಡಿ(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      ಆದರೆ (I != Sub.OptionsMap.ಕೊನೆ() && I->ಎರಡನೇ_2 == O)
        Sub.OptionsMap.erase(I);
    }

    ಆದರೆ (O->getFormattingFlag() == cl::Positional)
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.ಕೊನೆ(); ++Opt) {
        ಆದರೆ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          ನಿಲ್ಲಿಸು;
        }
      }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = ಶೂನ್ಯಸೂಚಕ;
  }

  ಶೂನ್ಯ removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  ತಾರ್ಕಿಕ hasOptions(ಸ್ಥಿರಾಂಕ SubCommand &Sub) ಸ್ಥಿರಾಂಕ {
    ಹಿಂತಿರುಗಿ (!Sub.OptionsMap.ಖಾಲಿಗಳು() || !Sub.PositionalOpts.ಖಾಲಿಗಳು() ||
            ಶೂನ್ಯಸೂಚಕ != Sub.ConsumeAfterOpt);
  }

  ತಾರ್ಕಿಕ hasOptions() ಸ್ಥಿರಾಂಕ {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *S : RegisteredSubCommands) {
      ಆದರೆ (hasOptions(*S))
        ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಹಿಂತಿರುಗಿ !Libraries.ಖಾಲಿಗಳು();
  }

  ತಾರ್ಕಿಕ hasNamedSubCommands() ಸ್ಥಿರಾಂಕ {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *S : RegisteredSubCommands)
      ಆದರೆ (!S->getName().ಖಾಲಿಗಳು())
        ಹಿಂತಿರುಗಿ ನಿಜ;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  SubCommand *get_ಸಕ್ರಿಯ_Sub_Command() { ಹಿಂತಿರುಗಿ ಸಕ್ರಿಯ_Sub_Command; }

  ಶೂನ್ಯ updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    ಆದರೆ (!Sub.OptionsMap.ಸೇರಿಸು(std::make_pair(NewName, O)).ಎರಡನೇ_2) {
      errs() << ಕಾರ್ಯಕ್ರಮ_Name << ": CommandLine Error: Option '" << O->ArgStr
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

  ಶೂನ್ಯ ಸೂಚ್ಯಂಕ_Library_Options() {
    ತಾರ್ಕಿಕ HadErrors = ಸುಳ್ಳು;
    ಗಾಗಿ (; ಅಂಕೆ_ಸೂಚ್ಯಂಕಿತ_Libraries != Libraries.ಗಾತ್ರ_3(); ++ಅಂಕೆ_ಸೂಚ್ಯಂಕಿತ_Libraries) {
      LibraryOptions *L = Libraries[ಅಂಕೆ_ಸೂಚ್ಯಂಕಿತ_Libraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        ಸ್ವಯಂಚಾಲಿತ [It, Inserted] = Library_ಸೂಚ್ಯಂಕ.try_emplace(Name, L);
        ಆದರೆ (Inserted ? none_ದ(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ಹಿಂತಿರುಗಿ SC->OptionsMap.ಒಳಗೊಂಡಿದೆ(Name);
                               })
                     : It->ಎರಡನೇ_2 == L)
          ಹಿಂತಿರುಗಿ;
        errs() << ಕಾರ್ಯಕ್ರಮ_Name << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = ನಿಜ;
      });
    }
    ಆದರೆ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    ಸೂಚ್ಯಂಕ_Library_Options();
    ಹಿಂತಿರುಗಿ Library_ಸೂಚ್ಯಂಕ.lookup(Name);
  }

  ಶೂನ್ಯ ರಿಜಿಸ್ಟರ್_Category(OptionCategory *cat) {
    ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_if(RegisteredOptionCategories,
                    [cat](ಸ್ಥಿರಾಂಕ OptionCategory *Category) {
             ಹಿಂತಿರುಗಿ cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.ಸೇರಿಸು(cat);
  }

  ಶೂನ್ಯ ರಿಜಿಸ್ಟರ್_Sub_Command(SubCommand *sub) {
    ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_if(RegisteredSubCommands,
                    [sub](ಸ್ಥಿರಾಂಕ SubCommand *Sub) {
                      ಹಿಂತಿರುಗಿ (!sub->getName().ಖಾಲಿಗಳು()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.ಸೇರಿಸು(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    ಪ್ರತಿಜ್ಞಾ(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.ಎರಡನೇ_2;
      ಆದರೆ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        ಕೂಡಿಸು_Option(O, sub);
      ಇಲ್ಲದಿದ್ದರೆ
        ಕೂಡಿಸು_Literal_Option(*O, sub, E.ಮೊದಲ_2);
    }
  }

  ಶೂನ್ಯ unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<Small_Ptr_ಗಣ<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ಹಿಂತಿರುಗಿ make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.ಕೊನೆ());
  }

  ಶೂನ್ಯ reset() {
    ಸಕ್ರಿಯ_Sub_Command = ಶೂನ್ಯಸೂಚಕ;
    ಕಾರ್ಯಕ್ರಮ_Name.clear();
    ಕಾರ್ಯಕ್ರಮ_Overview = StringRef();

    ಇನ್ನಷ್ಟು_Help.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    Library_ಸೂಚ್ಯಂಕ.clear();
    ಅಂಕೆ_ಸೂಚ್ಯಂಕಿತ_Libraries = 0;

    SubCommand::get_ತುದಿ_Level().reset();
    SubCommand::getAll().reset();
    ರಿಜಿಸ್ಟರ್_Sub_Command(&SubCommand::get_ತುದಿ_Level());
  }

ಖಾಸಗಿ:
  SubCommand *ಸಕ್ರಿಯ_Sub_Command = ಶೂನ್ಯಸೂಚಕ;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &ಮೌಲ್ಯ);
  SubCommand *LookupSubCommand(StringRef Name, std::string &ಹತ್ತಿರದ_String);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
ಸ್ಥಿರ Command_ಸಾಲು_Parser &globalParser() {
  ಸ್ಥಿರ ManagedStatic<Command_ಸಾಲು_Parser> GlobalParser;
  ಹಿಂತಿರುಗಿ *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
ಸ್ಥಿರ Error
parseLibraryOption(LibraryOptions &L, ಶೂನ್ಯ *Opts, ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು,
                   ಚಿಹ್ನೆರಹಿತ &Consumed, BumpPtrAllocator &Alloc,
                   Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> *ದಾಖಲೆ = ಶೂನ್ಯಸೂಚಕ) {
  StringSaver Saver(Alloc);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Saved[2];
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ವಾದಗಳು = std::min<size_t>(ವಾದಗಳು.ಗಾತ್ರ_3(), 2);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ J = 0; J != ಅಂಕೆ_ವಾದಗಳು; ++J)
    Saved[J] = Saver.save(ವಾದಗಳು[J]).ದತ್ತಾಂಶ();
  Consumed = 1;
  ಆದರೆ (Error E = L.parse(Opts, ArrayRef(Saved, ಅಂಕೆ_ವಾದಗಳು), Consumed, Alloc))
    ಹಿಂತಿರುಗಿ E;
  ಆದರೆ (ದಾಖಲೆ)
    ದಾಖಲೆ->append(Saved, Saved + Consumed);
  ಹಿಂತಿರುಗಿ Error::success();
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T, T TrueVal, T FalseVal>
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseBool(Option &O, StringRef ArgName, StringRef Arg, T &ಮೌಲ್ಯ) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  ಆದರೆ (!Arg.ದತ್ತಾಂಶ() || Arg == "true" || Arg == "1") {
    ಮೌಲ್ಯ = TrueVal;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಆದರೆ (Arg == "false" || Arg == "0") {
    ಮೌಲ್ಯ = FalseVal;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

ಶೂನ್ಯ cl::ಕೂಡಿಸು_Literal_Option_2(Option &O, StringRef Name) {
  globalParser().ಕೂಡಿಸು_Literal_Option(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().ಇನ್ನಷ್ಟು_Help.ತಳ್ಳು_ಹಿಂಭಾಗ(Help);
}

Option::Option(ಅಂಕೆ_Occurrences_Flag OccurrencesFlag, OptionHidden Hidden)
    : ಅಂಕೆ_Occurrences(0), Occurrences(OccurrencesFlag), ಮೌಲ್ಯ(0),
      HiddenFlag(Hidden), Formatting(ಸಾಮಾನ್ಯ_Formatting), Misc(0),
      FullyInitialized(ಸುಳ್ಳು), Position(0) {
  Categories.ತಳ್ಳು_ಹಿಂಭಾಗ(&getGeneralCategory());
}

ಶೂನ್ಯ Option::ಕೂಡಿಸು_ವಾದ() {
  globalParser().ಕೂಡಿಸು_Option(ಇದು);
  FullyInitialized = ನಿಜ;
}

ಶೂನ್ಯ Option::remove_ವಾದ() { globalParser().removeOption(ಇದು); }

ಶೂನ್ಯ Option::ಗಣ_Arg_Str(StringRef S) {
  ಆದರೆ (FullyInitialized)
    globalParser().updateArgStr(ಇದು, S);
  ಪ್ರತಿಜ್ಞಾ(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

ಶೂನ್ಯ Option::ಕೂಡಿಸು_Category(OptionCategory &C) {
  ಪ್ರತಿಜ್ಞಾ(!Categories.ಖಾಲಿಗಳು() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  ಆದರೆ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!is_contained(Categories, &C))
    Categories.ತಳ್ಳು_ಹಿಂಭಾಗ(&C);
}

ಶೂನ್ಯ Option::reset() {
  ಅಂಕೆ_Occurrences = 0;
  ಗಣ_Default();
}

ಶೂನ್ಯ OptionCategory::ರಿಜಿಸ್ಟರ್_Category() {
  globalParser().ರಿಜಿಸ್ಟರ್_Category(ಇದು);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::get_ತುದಿ_Level() {
  ಸ್ಥಿರ ManagedStatic<SubCommand> ತುದಿ_Level_Sub_Command;
  ಹಿಂತಿರುಗಿ *ತುದಿ_Level_Sub_Command;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  ಸ್ಥಿರ ManagedStatic<SubCommand> AllSubCommands;
  ಹಿಂತಿರುಗಿ *AllSubCommands;
}

ಶೂನ್ಯ SubCommand::ರಿಜಿಸ್ಟರ್_Sub_Command() {
  globalParser().ರಿಜಿಸ್ಟರ್_Sub_Command(ಇದು);
}

ಶೂನ್ಯ SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(ಇದು);
}

ಶೂನ್ಯ SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = ಶೂನ್ಯಸೂಚಕ;
}

SubCommand::ಕ್ರಿಯಾಕಾರಕ ತಾರ್ಕಿಕ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ (globalParser().get_ಸಕ್ರಿಯ_Sub_Command() == ಇದು);
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
  ಆದರೆ (Arg.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಪ್ರತಿಜ್ಞಾ(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.ಕಂಡುಹಿಡಿ('=');

  // If we have an equals sign, remember the value.
  ಆದರೆ (EqualPos == StringRef::npos) {
    // Look up the option.
    ಹಿಂತಿರುಗಿ Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  ಸ್ವಯಂಚಾಲಿತ I = Sub.OptionsMap.ಕಂಡುಹಿಡಿ(Arg.substr(0, EqualPos));
  ಆದರೆ (I == Sub.OptionsMap.ಕೊನೆ())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಸ್ವಯಂಚಾಲಿತ *O = I->ಎರಡನೇ_2;
  ಆದರೆ (O->getFormattingFlag() == cl::Always_ಪೂರ್ವಪ್ರತ್ಯಯ)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಮೌಲ್ಯ = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ಹಿಂತಿರುಗಿ I->ಎರಡನೇ_2;
}

SubCommand *Command_ಸಾಲು_Parser::LookupSubCommand(StringRef Name,
                                                std::string &ಹತ್ತಿರದ_String) {
  ಆದರೆ (Name.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ &SubCommand::get_ತುದಿ_Level();
  // Find a subcommand with the edit distance == 1.
  SubCommand *ಹತ್ತಿರದ_ಹೊಂದಿಕೆ = ಶೂನ್ಯಸೂಚಕ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *S : RegisteredSubCommands) {
    ಪ್ರತಿಜ್ಞಾ(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    ಆದರೆ (S->getName().ಖಾಲಿಗಳು())
      ಮುಂದುವರಿ;

    ಆದರೆ (S->getName() == Name)
      ಹಿಂತಿರುಗಿ S;

    ಆದರೆ (!ಹತ್ತಿರದ_ಹೊಂದಿಕೆ && S->getName().edit_ದೂರ(Name) < 2)
      ಹತ್ತಿರದ_ಹೊಂದಿಕೆ = S;
  }

  ಆದರೆ (ಹತ್ತಿರದ_ಹೊಂದಿಕೆ)
    ಹತ್ತಿರದ_String = ಹತ್ತಿರದ_ಹೊಂದಿಕೆ->getName();

  ಹಿಂತಿರುಗಿ &SubCommand::get_ತುದಿ_Level();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
ಸ್ಥಿರ Option *Lookup_ಹತ್ತಿರದ_Option(StringRef Arg,
                                   ಸ್ಥಿರಾಂಕ OptionsMapTy &OptionsMap,
                                   std::string &ಹತ್ತಿರದ_String) {
  // Reject all dashes.
  ಆದರೆ (Arg.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> ವಿಭಜಿಸು_Arg = Arg.ವಿಭಜಿಸು('=');
  StringRef &LHS = ವಿಭಜಿಸು_Arg.ಮೊದಲ_2; // LHS == Arg when no '=' is present.
  StringRef &RHS = ವಿಭಜಿಸು_Arg.ಎರಡನೇ_2;

  // Find the closest match.
  Option *ಉತ್ತಮ = ಶೂನ್ಯಸೂಚಕ;
  ಚಿಹ್ನೆರಹಿತ ಉತ್ತಮ_ದೂರ = 0;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    ಆದರೆ (O->getOptionHiddenFlag() == ReallyHidden)
      ಮುಂದುವರಿ;

    Small_ಸದಿಶ<StringRef, 16> Option_ಹೆಸರುಗಳು;
    O->get_Extra_Option_ಹೆಸರುಗಳು(Option_ಹೆಸರುಗಳು);
    ಆದರೆ (O->hasArgStr())
      Option_ಹೆಸರುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(O->ArgStr);

    ತಾರ್ಕಿಕ Permit_ಮೌಲ್ಯ = O->get_ಮೌಲ್ಯ_Expected_Flag() != cl::ಮೌಲ್ಯ_Disallowed;
    StringRef Flag = Permit_ಮೌಲ್ಯ ? LHS : Arg;
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Name : Option_ಹೆಸರುಗಳು) {
      ಚಿಹ್ನೆರಹಿತ ದೂರ_2 = StringRef(Name).edit_ದೂರ(
          Flag, /*AllowReplacements=*/ನಿಜ, /*MaxEditDistance=*/ಉತ್ತಮ_ದೂರ);
      ಆದರೆ (!ಉತ್ತಮ || ದೂರ_2 < ಉತ್ತಮ_ದೂರ) {
        ಉತ್ತಮ = O;
        ಉತ್ತಮ_ದೂರ = ದೂರ_2;
        ಆದರೆ (RHS.ಖಾಲಿಗಳು() || !Permit_ಮೌಲ್ಯ)
          ಹತ್ತಿರದ_String = std::string(Name);
        ಇಲ್ಲದಿದ್ದರೆ
          ಹತ್ತಿರದ_String = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ಹಿಂತಿರುಗಿ ಉತ್ತಮ;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
ಸ್ಥಿರ ತಾರ್ಕಿಕ Comma_Separate_And_ಕೂಡಿಸು_Occurrence(Option *Handler, ಚಿಹ್ನೆರಹಿತ pos,
                                          StringRef ArgName, StringRef ಮೌಲ್ಯ) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  ಆದರೆ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(ಮೌಲ್ಯ);
    StringRef::ಗಾತ್ರ_type Pos = Val.ಕಂಡುಹಿಡಿ(',');

    ಆಗ (Pos != StringRef::npos) {
      // Process the portion before the comma.
      ಆದರೆ (Handler->ಕೂಡಿಸು_Occurrence(pos, ArgName, Val.substr(0, Pos)))
        ಹಿಂತಿರುಗಿ ನಿಜ;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.ಕಂಡುಹಿಡಿ(',');
    }

    ಮೌಲ್ಯ = Val;
  }

  ಹಿಂತಿರುಗಿ Handler->ಕೂಡಿಸು_Occurrence(pos, ArgName, ಮೌಲ್ಯ);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ತಾರ್ಕಿಕ ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ argc,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, ಪೂರ್ಣಾಂಕ &i) {
  // Enforce value requirements
  ಆಯ್ಕೆ (Handler->get_ಮೌಲ್ಯ_Expected_Flag()) {
  ಸ್ಥಿತಿ ಮೌಲ್ಯ_Required:
    ಆದರೆ (!ಮೌಲ್ಯ.ದತ್ತಾಂಶ()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      ಆದರೆ (i + 1 >= argc || Handler->getFormattingFlag() == cl::Always_ಪೂರ್ವಪ್ರತ್ಯಯ)
        ಹಿಂತಿರುಗಿ Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      ಪ್ರತಿಜ್ಞಾ(argv && "null check");
      ಮೌಲ್ಯ = StringRef(argv[++i]);
    }
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ_Disallowed:
    ಆದರೆ (ಮೌಲ್ಯ.ದತ್ತಾಂಶ())
      ಹಿಂತಿರುಗಿ Handler->error("does not allow a value! '" + Twine(ಮೌಲ್ಯ) +
                            "' specified.");
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ_Optional:
    ನಿಲ್ಲಿಸು;
  }

  ಹಿಂತಿರುಗಿ Comma_Separate_And_ಕೂಡಿಸು_Occurrence(Handler, i, ArgName, ಮೌಲ್ಯ);
}

ತಾರ್ಕಿಕ llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, ಪೂರ್ಣಾಂಕ i) {
  ಪೂರ್ಣಾಂಕ Dummy = i;
  ಹಿಂತಿರುಗಿ ProvideOption(Handler, Handler->ArgStr, Arg, 0, ಶೂನ್ಯಸೂಚಕ, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
ಸ್ಥಿರ Option *ಕಂಡುಹಿಡಿ_ಪೂರ್ವಪ್ರತ್ಯಯ_Option(StringRef Name, size_t &ಉದ್ದಳತೆ,
                                ಸ್ಥಿರಾಂಕ OptionsMapTy &OptionsMap) {
  ಗಾಗಿ (; !Name.ಖಾಲಿಗಳು(); Name = Name.drop_ಹಿಂಭಾಗ()) {
    Option *O = OptionsMap.lookup(Name);
    ಆದರೆ (O && (O->getFormattingFlag() == cl::ಪೂರ್ವಪ್ರತ್ಯಯ ||
              O->getFormattingFlag() == cl::Always_ಪೂರ್ವಪ್ರತ್ಯಯ)) {
      ಉದ್ದಳತೆ = Name.ಗಾತ್ರ_3();
      ಹಿಂತಿರುಗಿ O;
    }
  }
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
ಸ್ಥಿರ Option *HandlePrefixedOption(StringRef &Arg, StringRef &ಮೌಲ್ಯ,
                                    ಸ್ಥಿರಾಂಕ OptionsMapTy &OptionsMap) {
  ಆದರೆ (Arg.ಗಾತ್ರ_3() == 1)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  size_t ಉದ್ದಳತೆ = 0;
  Option *POpt = ಕಂಡುಹಿಡಿ_ಪೂರ್ವಪ್ರತ್ಯಯ_Option(Arg, ಉದ್ದಳತೆ, OptionsMap);
  ಆದರೆ (!POpt)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  StringRef Maybe_ಮೌಲ್ಯ =
      (ಉದ್ದಳತೆ < Arg.ಗಾತ್ರ_3()) ? Arg.substr(ಉದ್ದಳತೆ) : StringRef();
  Arg = Arg.substr(0, ಉದ್ದಳತೆ);

  // cl::Prefix options do not preserve '=' when used separately.
  ಆದರೆ (POpt->getFormattingFlag() == cl::ಪೂರ್ವಪ್ರತ್ಯಯ && Maybe_ಮೌಲ್ಯ.starts_with("="))
    Maybe_ಮೌಲ್ಯ = Maybe_ಮೌಲ್ಯ.drop_front();
  ಮೌಲ್ಯ = Maybe_ಮೌಲ್ಯ;
  ಹಿಂತಿರುಗಿ POpt;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ Requires_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ Option *O) {
  ಹಿಂತಿರುಗಿ O->get_ಅಂಕೆ_Occurrences_Flag() == cl::Required ||
         O->get_ಅಂಕೆ_Occurrences_Flag() == cl::One_Or_ಇನ್ನಷ್ಟು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ Eats_Unbounded_ಸಂಖ್ಯೆ_ದ_ಮೌಲ್ಯಗಳು(ಸ್ಥಿರಾಂಕ Option *O) {
  ಹಿಂತಿರುಗಿ O->get_ಅಂಕೆ_Occurrences_Flag() == cl::Zero_Or_ಇನ್ನಷ್ಟು ||
         O->get_ಅಂಕೆ_Occurrences_Flag() == cl::One_Or_ಇನ್ನಷ್ಟು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isWhitespace(ಅಕ್ಷರ C) {
  ಹಿಂತಿರುಗಿ C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isWhitespaceOrNull(ಅಕ್ಷರ C) {
  ಹಿಂತಿರುಗಿ isWhitespace(C) || C == '\0';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isQuote(ಅಕ್ಷರ C) { ಹಿಂತಿರುಗಿ C == '\"' || C == '\''; }

ಶೂನ್ಯ cl::Tokenize_GNUCommand_ಸಾಲು(StringRef Src, StringSaver &Saver,
                                Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv,
                                ತಾರ್ಕಿಕ MarkEOLs) {
  SmallString<128> Token;
  ತಾರ್ಕಿಕ ಒಳ_Token = ಸುಳ್ಳು;
  ಗಾಗಿ (size_t I = 0, E = Src.ಗಾತ್ರ_3(); I != E; ++I) {
    // Consume runs of whitespace.
    ಆದರೆ (!ಒಳ_Token) {
      ಆಗ (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        ಆದರೆ (MarkEOLs && Src[I] == '\n')
          NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
        ++I;
      }
      ಆದರೆ (I == E)
        ನಿಲ್ಲಿಸು;
      ಒಳ_Token = ನಿಜ;
    }

    ಅಕ್ಷರ C = Src[I];

    // Backslash escapes the next character.
    ಆದರೆ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.ತಳ್ಳು_ಹಿಂಭಾಗ(Src[I]);
      ಮುಂದುವರಿ;
    }

    // Consume a quoted string.
    ಆದರೆ (isQuote(C)) {
      ++I;
      ಆಗ (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        ಆದರೆ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.ತಳ್ಳು_ಹಿಂಭಾಗ(Src[I]);
        ++I;
      }
      ಆದರೆ (I == E)
        ನಿಲ್ಲಿಸು;
      ಮುಂದುವರಿ;
    }

    // End the token if this is whitespace.
    ಆದರೆ (isWhitespace(C)) {
      NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(Saver.save(Token.str()).ದತ್ತಾಂಶ());
      // Mark the end of lines in response files.
      ಆದರೆ (MarkEOLs && C == '\n')
        NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
      Token.clear();
      ಒಳ_Token = ಸುಳ್ಳು;
      ಮುಂದುವರಿ;
    }

    // This is a normal character.  Append it.
    Token.ತಳ್ಳು_ಹಿಂಭಾಗ(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  ಆದರೆ (ಒಳ_Token)
    NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(Saver.save(Token.str()).ದತ್ತಾಂಶ());
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
  size_t E = Src.ಗಾತ್ರ_3();
  ಪೂರ್ಣಾಂಕ Backslash_ಎಣಿಕೆ = 0;
  // Skip the backslashes.
  ಮಾಡು {
    ++I;
    ++Backslash_ಎಣಿಕೆ;
  } ಆಗ (I != E && Src[I] == '\\');

  ತಾರ್ಕಿಕ FollowedByDoubleQuote = (I != E && Src[I] == '"');
  ಆದರೆ (FollowedByDoubleQuote) {
    Token.append(Backslash_ಎಣಿಕೆ / 2, '\\');
    ಆದರೆ (Backslash_ಎಣಿಕೆ % 2 == 0)
      ಹಿಂತಿರುಗಿ I - 1;
    Token.ತಳ್ಳು_ಹಿಂಭಾಗ('"');
    ಹಿಂತಿರುಗಿ I;
  }
  Token.append(Backslash_ಎಣಿಕೆ, '\\');
  ಹಿಂತಿರುಗಿ I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isWindowsSpecialChar(ಅಕ್ಷರ C) {
  ಹಿಂತಿರುಗಿ isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
ಸ್ಥಿರ ತಾರ್ಕಿಕ is_Windows_Special_Char_ಒಳ_Command_Name(ಅಕ್ಷರ C) {
  ಹಿಂತಿರುಗಿ isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ tokenize_Windows_Command_ಸಾಲು_Impl(
    StringRef Src, StringSaver &Saver, function_ref<ಶೂನ್ಯ(StringRef)> ಕೂಡಿಸು_Token,
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
  ಎಣಿಕೆ { INIT, UNQUOTED, QUOTED } State = INIT;

  ಗಾಗಿ (size_t I = 0, E = Src.ಗಾತ್ರ_3(); I < E; ++I) {
    ಆಯ್ಕೆ (State) {
    ಸ್ಥಿತಿ INIT: {
      ಪ್ರತಿಜ್ಞಾ(Token.ಖಾಲಿಗಳು() && "token should be empty in initial state");
      // Eat whitespace before a token.
      ಆಗ (I < E && isWhitespaceOrNull(Src[I])) {
        ಆದರೆ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      ಆದರೆ (I >= E)
        ನಿಲ್ಲಿಸು;
      size_t ಆರಂಭ = I;
      ಆದರೆ (CommandName) {
        ಆಗ (I < E && !is_Windows_Special_Char_ಒಳ_Command_Name(Src[I]))
          ++I;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆಗ (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef ಸಾಮಾನ್ಯ_Chars = Src.ತುಂಡು(ಆರಂಭ, I);
      ಆದರೆ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        ಕೂಡಿಸು_Token(AlwaysCopy ? Saver.save(ಸಾಮಾನ್ಯ_Chars) : ಸಾಮಾನ್ಯ_Chars);
        ಆದರೆ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } ಇಲ್ಲದಿದ್ದರೆ {
          CommandName = ಸುಳ್ಳು;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src[I] == '\"') {
        Token += ಸಾಮಾನ್ಯ_Chars;
        State = QUOTED;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src[I] == '\\') {
        ಪ್ರತಿಜ್ಞಾ(!CommandName && "or else we'd have treated it as a normal char");
        Token += ಸಾಮಾನ್ಯ_Chars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } ಇಲ್ಲದಿದ್ದರೆ {
        llvm_unreachable("unexpected special character");
      }
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ UNQUOTED:
      ಆದರೆ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        ಕೂಡಿಸು_Token(Saver.save(Token.str()));
        Token.clear();
        ಆದರೆ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } ಇಲ್ಲದಿದ್ದರೆ {
          CommandName = ಸುಳ್ಳು;
        }
        State = INIT;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src[I] == '\"') {
        State = QUOTED;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ಇಲ್ಲದಿದ್ದರೆ {
        Token.ತಳ್ಳು_ಹಿಂಭಾಗ(Src[I]);
      }
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ QUOTED:
      ಆದರೆ (Src[I] == '\"') {
        ಆದರೆ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.ತಳ್ಳು_ಹಿಂಭಾಗ('"');
          ++I;
        } ಇಲ್ಲದಿದ್ದರೆ {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } ಇಲ್ಲದಿದ್ದರೆ {
        Token.ತಳ್ಳು_ಹಿಂಭಾಗ(Src[I]);
      }
      ನಿಲ್ಲಿಸು;
    }
  }

  ಆದರೆ (State != INIT)
    ಕೂಡಿಸು_Token(Saver.save(Token.str()));
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು(StringRef Src, StringSaver &Saver,
                                    Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv,
                                    ತಾರ್ಕಿಕ MarkEOLs) {
  ಸ್ವಯಂಚಾಲಿತ ಕೂಡಿಸು_Token = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(Tok.ದತ್ತಾಂಶ()); };
  ಸ್ವಯಂಚಾಲಿತ OnEOL = [&]() {
    ಆದರೆ (MarkEOLs)
      NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
  };
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, ಕೂಡಿಸು_Token,
                                 /*AlwaysCopy=*/ನಿಜ, OnEOL, ಸುಳ್ಳು);
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು_No_Copy(StringRef Src, StringSaver &Saver,
                                          Small_ಸದಿಶ_Impl<StringRef> &NewArgv) {
  ಸ್ವಯಂಚಾಲಿತ ಕೂಡಿಸು_Token = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(Tok); };
  ಸ್ವಯಂಚಾಲಿತ OnEOL = []() {};
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, ಕೂಡಿಸು_Token, /*AlwaysCopy=*/ಸುಳ್ಳು,
                                 OnEOL, ಸುಳ್ಳು);
}

ಶೂನ್ಯ cl::Tokenize_Windows_Command_ಸಾಲು_ತುಂಬಿದ(StringRef Src, StringSaver &Saver,
                                        Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv,
                                        ತಾರ್ಕಿಕ MarkEOLs) {
  ಸ್ವಯಂಚಾಲಿತ ಕೂಡಿಸು_Token = [&](StringRef Tok) { NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(Tok.ದತ್ತಾಂಶ()); };
  ಸ್ವಯಂಚಾಲಿತ OnEOL = [&]() {
    ಆದರೆ (MarkEOLs)
      NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
  };
  tokenize_Windows_Command_ಸಾಲು_Impl(Src, Saver, ಕೂಡಿಸು_Token,
                                 /*AlwaysCopy=*/ನಿಜ, OnEOL, ನಿಜ);
}

ಶೂನ್ಯ cl::tokenizeConfigFile(StringRef ಮೂಲ, StringSaver &Saver,
                            Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv,
                            ತಾರ್ಕಿಕ MarkEOLs) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Cur = ಮೂಲ.begin(); Cur != ಮೂಲ.ಕೊನೆ();) {
    SmallString<128> ಸಾಲು;
    // Check for comment line.
    ಆದರೆ (isWhitespace(*Cur)) {
      ಆಗ (Cur != ಮೂಲ.ಕೊನೆ() && isWhitespace(*Cur))
        ++Cur;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ (*Cur == '#') {
      ಆಗ (Cur != ಮೂಲ.ಕೊನೆ() && *Cur != '\n')
        ++Cur;
      ಮುಂದುವರಿ;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = Cur;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು_ಕೊನೆ = ಶೂನ್ಯಸೂಚಕ;
    ಅಕ್ಷರ Quote = 0;
    ತಾರ್ಕಿಕ At_Token_ಆರಂಭ = ನಿಜ;
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_2 = ಮೂಲ.ಕೊನೆ(); Cur != ಕೊನೆ_2; ++Cur) {
      ಅಕ್ಷರ C = *Cur;
      ಆದರೆ (C == '\\') {
        ಆದರೆ (Cur + 1 != ಕೊನೆ_2) {
          ++Cur;
          ಆದರೆ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != ಕೊನೆ_2) && Cur[1] == '\n')) {
            ಸಾಲು.append(ಆರಂಭ, Cur - 1);
            ಆದರೆ (*Cur == '\r')
              ++Cur;
            ಆರಂಭ = Cur + 1;
            // Splicing does not introduce a token boundary.
            ಮುಂದುವರಿ;
          }
        }
        At_Token_ಆರಂಭ = ಸುಳ್ಳು;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ (C == '\n')
        ನಿಲ್ಲಿಸು;
      ಆದರೆ (Quote) {
        ಆದರೆ (C == Quote)
          Quote = 0;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isQuote(C)) {
        Quote = C;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (C == '#' && At_Token_ಆರಂಭ) {
        ಸಾಲು_ಕೊನೆ = Cur;
        ಆಗ (Cur != ಕೊನೆ_2 && *Cur != '\n')
          ++Cur;
        ನಿಲ್ಲಿಸು;
      }
      At_Token_ಆರಂಭ = isWhitespace(C);
    }
    // Tokenize line.
    ಸಾಲು.append(ಆರಂಭ, ಸಾಲು_ಕೊನೆ ? ಸಾಲು_ಕೊನೆ : Cur);
    cl::Tokenize_GNUCommand_ಸಾಲು(ಸಾಲು, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
ಸ್ಥಿರ ತಾರ್ಕಿಕ has_UTF8_Byte_ಕ್ರಮ_Mark(ArrayRef<ಅಕ್ಷರ> S) {
  ಹಿಂತಿರುಗಿ (S.ಗಾತ್ರ_3() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
ಸ್ಥಿರ ಶೂನ್ಯ Expand_ತಳ_Paths(StringRef ತಳ_ಮಾರ್ಗ, StringSaver &Saver,
                            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *&Arg) {
  ಪ್ರತಿಜ್ಞಾ(sys::ಮಾರ್ಗ_2::is_absolute(ತಳ_ಮಾರ್ಗ));
  constexpr StringLiteral Token("<CFGDIR>");
  ಸ್ಥಿರಾಂಕ StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::ಗಾತ್ರ_type ಆರಂಭ_Pos = 0;
  ಗಾಗಿ (StringRef::ಗಾತ್ರ_type TokenPos = ArgString.ಕಂಡುಹಿಡಿ(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.ಕಂಡುಹಿಡಿ(Token, ಆರಂಭ_Pos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    ಸ್ಥಿರಾಂಕ StringRef LHS = ArgString.substr(ಆರಂಭ_Pos, TokenPos - ಆರಂಭ_Pos);
    ಆದರೆ (ResponseFile.ಖಾಲಿಗಳು())
      ResponseFile = LHS;
    ಇಲ್ಲದಿದ್ದರೆ
      llvm::sys::ಮಾರ್ಗ_2::append(ResponseFile, LHS);
    ResponseFile.append(ತಳ_ಮಾರ್ಗ);
    ಆರಂಭ_Pos = TokenPos + Token.ಗಾತ್ರ_3();
  }

  ಆದರೆ (!ResponseFile.ಖಾಲಿಗಳು()) {
    // Path-append the remaining arg substring if at least one token appeared.
    ಸ್ಥಿರಾಂಕ StringRef ಉಳಿದ = ArgString.substr(ಆರಂಭ_Pos);
    ಆದರೆ (!ಉಳಿದ.ಖಾಲಿಗಳು())
      llvm::sys::ಮಾರ್ಗ_2::append(ResponseFile, ಉಳಿದ);
    Arg = Saver.save(ResponseFile.str()).ದತ್ತಾಂಶ();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv) {
  ಪ್ರತಿಜ್ಞಾ(sys::ಮಾರ್ಗ_2::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> MemBufOrErr =
      FS->get_ಬಫರ್_For_File(FName);
  ಆದರೆ (!MemBufOrErr) {
    std::error_ಸಂಕೇತ EC = MemBufOrErr.getError();
    ಹಿಂತಿರುಗಿ llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.ಸಂದೇಶ_2());
  }
  ಸ್ಮರಣೆ_ಬಫರ್ &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.get_ಬಫರ್_ಆರಂಭ(), MemBuf.get_ಬಫರ್_ಗಾತ್ರ());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<ಅಕ್ಷರ> BufRef(MemBuf.get_ಬಫರ್_ಆರಂಭ(), MemBuf.get_ಬಫರ್_ಕೊನೆ());
  std::string UTF8Buf;
  ಆದರೆ (has_UTF16_Byte_ಕ್ರಮ_Mark(BufRef)) {
    ಆದರೆ (!convert_UTF16_ಗಮ್ಯ_UTF8_String(BufRef, UTF8Buf))
      ಹಿಂತಿರುಗಿ llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (has_UTF8_Byte_ಕ್ರಮ_Mark(BufRef))
    Str = StringRef(BufRef.ದತ್ತಾಂಶ() + 3, BufRef.ಗಾತ್ರ_3() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  ಆದರೆ (!Relative_ಹೆಸರುಗಳು && !ಒಳ_Config_File)
    ಹಿಂತಿರುಗಿ Error::success();

  StringRef ತಳ_ಮಾರ್ಗ = llvm::sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(FName);
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *&Arg : NewArgv) {
    ಆದರೆ (!Arg)
      ಮುಂದುವರಿ;

    // Substitute <CFGDIR> with the file's base path.
    ಆದರೆ (ಒಳ_Config_File)
      Expand_ತಳ_Paths(ತಳ_ಮಾರ್ಗ, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    ತಾರ್ಕಿಕ ConfigInclusion = ಸುಳ್ಳು;
    ಆದರೆ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      ಆದರೆ (!llvm::sys::ಮಾರ್ಗ_2::is_relative(FileName))
        ಮುಂದುವರಿ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಮುಂದುವರಿ;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.ತಳ್ಳು_ಹಿಂಭಾಗ('@');
    ಆದರೆ (ConfigInclusion && !llvm::sys::ಮಾರ್ಗ_2::has_ಪೋಷಕ_ಮಾರ್ಗ(FileName)) {
      SmallString<128> File_ಮಾರ್ಗ;
      ಆದರೆ (!ಕಂಡುಹಿಡಿ_Config_File(FileName, File_ಮಾರ್ಗ))
        ಹಿಂತಿರುಗಿ createStringError(
            std::make_error_ಸಂಕೇತ(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(File_ಮಾರ್ಗ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ResponseFile.append(ತಳ_ಮಾರ್ಗ);
      llvm::sys::ಮಾರ್ಗ_2::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).ದತ್ತಾಂಶ();
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &Argv) {
  ರಚನೆ Response_File_ದಾಖಲೆ {
    std::string File;
    size_t ಕೊನೆ_2;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  Small_ಸದಿಶ<Response_File_ದಾಖಲೆ, 3> File_ರಾಶಿ;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  File_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ({"", Argv.ಗಾತ್ರ_3()});

  // Don't cache Argv.size() because it can change.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I != Argv.ಗಾತ್ರ_3();) {
    ಆಗ (I == File_ರಾಶಿ.ಹಿಂಭಾಗ().ಕೊನೆ_2) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      File_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    }

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Arg = Argv[I];
    // Check if it is an EOL marker
    ಆದರೆ (Arg == ಶೂನ್ಯಸೂಚಕ) {
      ++I;
      ಮುಂದುವರಿ;
    }

    ಆದರೆ (Arg[0] != '@') {
      ++I;
      ಮುಂದುವರಿ;
    }

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    ಆದರೆ (llvm::sys::ಮಾರ್ಗ_2::is_relative(FName)) {
      ಆದರೆ (CurrentDir.ಖಾಲಿಗಳು()) {
        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } ಇಲ್ಲದಿದ್ದರೆ {
          ಹಿಂತಿರುಗಿ createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        CurrDir = CurrentDir;
      }
      llvm::sys::ಮಾರ್ಗ_2::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::ಸ್ಥಿತಿ_2> Res = FS->ಸ್ಥಿತಿ_3(FName);
    ಆದರೆ (!Res || !Res->exists()) {
      std::error_ಸಂಕೇತ EC = Res.getError();
      ಆದರೆ (!ಒಳ_Config_File) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        ಆದರೆ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          ಮುಂದುವರಿ;
        }
      }
      ಆದರೆ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ಹಿಂತಿರುಗಿ createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.ಸಂದೇಶ_2());
    }
    ಸ್ಥಿರಾಂಕ llvm::vfs::ಸ್ಥಿತಿ_2 &File_ಸ್ಥಿತಿ = Res.get();

    ಸ್ವಯಂಚಾಲಿತ IsEquivalent =
        [File_ಸ್ಥಿತಿ, ಇದು](ಸ್ಥಿರಾಂಕ Response_File_ದಾಖಲೆ &RFile) -> ErrorOr<ತಾರ್ಕಿಕ> {
      ErrorOr<llvm::vfs::ಸ್ಥಿತಿ_2> RHS = FS->ಸ್ಥಿತಿ_3(RFile.File);
      ಆದರೆ (!RHS)
        ಹಿಂತಿರುಗಿ RHS.getError();
      ಹಿಂತಿರುಗಿ File_ಸ್ಥಿತಿ.equivalent(*RHS);
    };

    // Check for recursive response files.
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &F : drop_begin(File_ರಾಶಿ)) {
      ಆದರೆ (ErrorOr<ತಾರ್ಕಿಕ> R = IsEquivalent(F)) {
        ಆದರೆ (R.get())
          ಹಿಂತಿರುಗಿ createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    Small_ಸದಿಶ<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, 0> ExpandedArgv;
    ಆದರೆ (Error Err = expandResponseFile(FName, ExpandedArgv))
      ಹಿಂತಿರುಗಿ Err;

    ಗಾಗಿ (Response_File_ದಾಖಲೆ &ದಾಖಲೆ : File_ರಾಶಿ) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      ದಾಖಲೆ.ಕೊನೆ_2 += ExpandedArgv.ಗಾತ್ರ_3() - 1;
    }

    File_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ({FName, I + ExpandedArgv.ಗಾತ್ರ_3()});
    Argv.erase(Argv.begin() + I);
    Argv.ಸೇರಿಸು(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.ಕೊನೆ());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  ಪ್ರತಿಜ್ಞಾ(File_ರಾಶಿ.ಗಾತ್ರ_3() > 0 && Argv.ಗಾತ್ರ_3() == File_ರಾಶಿ.ಹಿಂಭಾಗ().ಕೊನೆ_2);
  ಹಿಂತಿರುಗಿ Error::success();
}

ತಾರ್ಕಿಕ cl::expandResponseFiles(ಪೂರ್ಣಾಂಕ Argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *Argv,
                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EnvVar, StringSaver &Saver,
                             Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &NewArgv) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಸ್ವಯಂಚಾಲಿತ Tokenize = cl::Tokenize_Windows_Command_ಸಾಲು;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ವಯಂಚಾಲಿತ Tokenize = cl::Tokenize_GNUCommand_ಸಾಲು;
#ಆದರೆ_ಅಂತ್ಯ
  // The environment variable specifies initial options.
  ಆದರೆ (EnvVar)
    ಆದರೆ (std::optional<std::string> Env_ಮೌಲ್ಯ = sys::Process::GetEnv(EnvVar))
      Tokenize(*Env_ಮೌಲ್ಯ, Saver, NewArgv, /*MarkEOLs=*/ಸುಳ್ಳು);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  ಆದರೆ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << ಗಮ್ಯ_String(std::move(Err)) << '\n';
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  ಆದರೆ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << ಗಮ್ಯ_String(std::move(Err)) << '\n';
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

ತಾರ್ಕಿಕ ExpansionContext::ಕಂಡುಹಿಡಿ_Config_File(StringRef FileName,
                                      Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &File_ಮಾರ್ಗ) {
  SmallString<128> Cfg_File_ಮಾರ್ಗ;
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ FileExists = [ಇದು](SmallString<128> ಮಾರ್ಗ) -> ತಾರ್ಕಿಕ {
    ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_2 = FS->ಸ್ಥಿತಿ_3(ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 &&
           ಸ್ಥಿತಿ_2->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  ಆದರೆ (llvm::sys::ಮಾರ್ಗ_2::has_ಪೋಷಕ_ಮಾರ್ಗ(FileName)) {
    Cfg_File_ಮಾರ್ಗ = FileName;
    ಆದರೆ (llvm::sys::ಮಾರ್ಗ_2::is_relative(FileName) && FS->makeAbsolute(Cfg_File_ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಆದರೆ (!FileExists(Cfg_File_ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    File_ಮಾರ್ಗ.ನಿಯೋಜಿಸು(Cfg_File_ಮಾರ್ಗ.begin(), Cfg_File_ಮಾರ್ಗ.ಕೊನೆ());
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  // Look for the file in search directories.
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ StringRef &Dir : ಹುಡುಕು_Dirs) {
    ಆದರೆ (Dir.ಖಾಲಿಗಳು())
      ಮುಂದುವರಿ;
    Cfg_File_ಮಾರ್ಗ.ನಿಯೋಜಿಸು(Dir);
    llvm::sys::ಮಾರ್ಗ_2::append(Cfg_File_ಮಾರ್ಗ, FileName);
    llvm::sys::ಮಾರ್ಗ_2::native(Cfg_File_ಮಾರ್ಗ);
    ಆದರೆ (FileExists(Cfg_File_ಮಾರ್ಗ)) {
      File_ಮಾರ್ಗ.ನಿಯೋಜಿಸು(Cfg_File_ಮಾರ್ಗ.begin(), Cfg_File_ಮಾರ್ಗ.ಕೊನೆ());
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       Small_ಸದಿಶ_Impl<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> &Argv) {
  SmallString<128> Abs_ಮಾರ್ಗ;
  ಆದರೆ (sys::ಮಾರ್ಗ_2::is_relative(CfgFile)) {
    Abs_ಮಾರ್ಗ.ನಿಯೋಜಿಸು(CfgFile);
    ಆದರೆ (std::error_ಸಂಕೇತ EC = FS->makeAbsolute(Abs_ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = Abs_ಮಾರ್ಗ.str();
  }
  ಒಳ_Config_File = ನಿಜ;
  Relative_ಹೆಸರುಗಳು = ನಿಜ;
  ಆದರೆ (Error Err = expandResponseFile(CfgFile, Argv))
    ಹಿಂತಿರುಗಿ Err;
  ಹಿಂತಿರುಗಿ expandResponseFiles(Argv);
}

ಸ್ಥಿರ ಶೂನ್ಯ initCommonOptions();
ಸ್ಥಿರ ತಾರ್ಕಿಕ parse_Command_ಸಾಲು_Options_Impl(LibraryOptionsContext *Context,
                                        ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EnvVar) {
  initCommonOptions();
  Small_ಸದಿಶ<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.ತಳ್ಳು_ಹಿಂಭಾಗ(ವಾದಗಳು[0]);

  // Parse options from environment variable.
  ಆದರೆ (EnvVar) {
    ಆದರೆ (std::optional<std::string> Env_ಮೌಲ್ಯ =
            sys::Process::GetEnv(StringRef(EnvVar)))
      Tokenize_GNUCommand_ಸಾಲು(*Env_ಮೌಲ್ಯ, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, ವಾದಗಳು.drop_front());
  ಪೂರ್ಣಾಂಕ NewArgc = static_cast<ಪೂರ್ಣಾಂಕ>(NewArgv.ಗಾತ್ರ_3());

  // Parse all options.
  ಹಿಂತಿರುಗಿ globalParser().Parse_Command_ಸಾಲು_Options(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

ತಾರ್ಕಿಕ cl::Parse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EnvVar) {
  ಹಿಂತಿರುಗಿ parse_Command_ಸಾಲು_Options_Impl(ಶೂನ್ಯಸೂಚಕ, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

ತಾರ್ಕಿಕ cl::parse_Command_ಸಾಲು_Options(LibraryOptionsContext &Context,
                                 ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *EnvVar) {
  ಹಿಂತಿರುಗಿ parse_Command_ಸಾಲು_Options_Impl(&Context, ವಾದಗಳು, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
ಶೂನ್ಯ Command_ಸಾಲು_Parser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    Small_ಸದಿಶ<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.ಗಾತ್ರ_3());
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &O : SC->OptionsMap)
      Opts.ತಳ್ಳು_ಹಿಂಭಾಗ(O.ಎರಡನೇ_2);
    ಗಾಗಿ (Option *O : Opts)
      O->reset();
    ಗಾಗಿ (Option *O : SC->PositionalOpts)
      O->reset();
    ಆದರೆ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  ಗಾಗಿ (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

ತಾರ್ಕಿಕ Command_ಸಾಲು_Parser::Parse_Command_ಸಾಲು_Options(
    ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  ಪ್ರತಿಜ್ಞಾ(hasOptions() && "No options specified!");

  ಕಾರ್ಯಕ್ರಮ_Overview = Overview;
  ತಾರ್ಕಿಕ IgnoreErrors = Errs;
  ಆದರೆ (!Errs)
    Errs = &errs();
  ಆದರೆ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  ತಾರ್ಕಿಕ ErrorParsing = ಸುಳ್ಳು;

  // Expand response files.
  Small_ಸದಿಶ<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಸ್ವಯಂಚಾಲಿತ Tokenize = cl::Tokenize_Windows_Command_ಸಾಲು;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ವಯಂಚಾಲಿತ Tokenize = cl::Tokenize_GNUCommand_ಸಾಲು;
#ಆದರೆ_ಅಂತ್ಯ
  ExpansionContext ECtx(A, Tokenize, VFS);
  ಆದರೆ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << ಗಮ್ಯ_String(std::move(Err)) << '\n';
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  argv = &newArgv[0];
  argc = static_cast<ಪೂರ್ಣಾಂಕ>(newArgv.ಗಾತ್ರ_3());

  // Copy the program name into ProgName, making sure not to overflow it.
  ಕಾರ್ಯಕ್ರಮ_Name = std::string(sys::ಮಾರ್ಗ_2::filename(StringRef(argv[0])));
  ಸೂಚ್ಯಂಕ_Library_Options();

  // Check out the positional arguments to collect information about them.
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Positional_Required = 0;

  // Determine whether or not there are an unlimited number of positionals
  ತಾರ್ಕಿಕ HasUnlimitedPositionals = ಸುಳ್ಳು;

  ಪೂರ್ಣಾಂಕ ಮೊದಲ_Arg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::get_ತುದಿ_Level();
  std::string ಹತ್ತಿರದ_Sub_Command_String;
  ತಾರ್ಕಿಕ MaybeNamedSubCommand =
      argc >= 2 && argv[ಮೊದಲ_Arg][0] != '-' && hasNamedSubCommands();
  ಆದರೆ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[ಮೊದಲ_Arg]), ಹತ್ತಿರದ_Sub_Command_String);
    ಆದರೆ (ChosenSubCommand != &SubCommand::get_ತುದಿ_Level())
      ಮೊದಲ_Arg = 2;
  }
  globalParser().ಸಕ್ರಿಯ_Sub_Command = ChosenSubCommand;

  ಪ್ರತಿಜ್ಞಾ(ChosenSubCommand);
  ಸ್ವಯಂಚಾಲಿತ &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  ಸ್ವಯಂಚಾಲಿತ &PositionalOpts = ChosenSubCommand->PositionalOpts;
  ಸ್ವಯಂಚಾಲಿತ &OptionsMap = ChosenSubCommand->OptionsMap;

  ಆದರೆ (ConsumeAfterOpt) {
    ಪ್ರತಿಜ್ಞಾ(PositionalOpts.ಗಾತ್ರ_3() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  ಆದರೆ (!PositionalOpts.ಖಾಲಿಗಳು()) {

    // Calculate how many positional values are _required_.
    ತಾರ್ಕಿಕ Unbounded_ಸಿಕ್ಕಿತು = ಸುಳ್ಳು;
    ಗಾಗಿ (size_t i = 0, e = PositionalOpts.ಗಾತ್ರ_3(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      ಆದರೆ (Requires_ಮೌಲ್ಯ(Opt))
        ++ಅಂಕೆ_Positional_Required;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        ಆದರೆ (PositionalOpts.ಗಾತ್ರ_3() > 1) {
          ಆದರೆ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = ನಿಜ;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unbounded_ಸಿಕ್ಕಿತು && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        ಆದರೆ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ಕಾರ್ಯಕ್ರಮ_Name << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.ಗಾತ್ರ_3();
        ErrorParsing = ನಿಜ;
      }
      Unbounded_ಸಿಕ್ಕಿತು |= Eats_Unbounded_ಸಂಖ್ಯೆ_ದ_ಮೌಲ್ಯಗಳು(Opt);
    }
    HasUnlimitedPositionals = Unbounded_ಸಿಕ್ಕಿತು || ConsumeAfterOpt;
  }

  // PositionalVals - A vector of "positional" arguments we accumulate into
  // the process at the end.
  //
  Small_ಸದಿಶ<std::pair<StringRef, ಚಿಹ್ನೆರಹಿತ>, 4> PositionalVals;

  // If the program has named positional arguments, and the name has been run
  // across, keep track of which positional argument was named.  Otherwise put
  // the positional args into the PositionalVals list...
  Option *ಸಕ್ರಿಯ_Positional_Arg = ಶೂನ್ಯಸೂಚಕ;

  // Loop over all of the arguments... processing them.
  ತಾರ್ಕಿಕ Dash_Dash_ಸಿಕ್ಕಿತು = ಸುಳ್ಳು; // Have we read '--'?
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = ಮೊದಲ_Arg; i < argc; ++i) {
    Option *Handler = ಶೂನ್ಯಸೂಚಕ;
    std::string ಹತ್ತಿರದ_Handler_String;
    StringRef ಮೌಲ್ಯ;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    ಆದರೆ (argv[i][0] != '-' || argv[i][1] == 0 || Dash_Dash_ಸಿಕ್ಕಿತು) {
      // Positional argument!
      ಆದರೆ (ಸಕ್ರಿಯ_Positional_Arg) {
        ProvidePositionalOption(ಸಕ್ರಿಯ_Positional_Arg, StringRef(argv[i]), i);
        ಮುಂದುವರಿ; // We are done!
      }

      ಆದರೆ (!PositionalOpts.ಖಾಲಿಗಳು()) {
        PositionalVals.ತಳ್ಳು_ಹಿಂಭಾಗ(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        ಆದರೆ (PositionalVals.ಗಾತ್ರ_3() >= ಅಂಕೆ_Positional_Required && ConsumeAfterOpt) {
          ಗಾಗಿ (++i; i < argc; ++i)
            PositionalVals.ತಳ್ಳು_ಹಿಂಭಾಗ(std::make_pair(StringRef(argv[i]), i));
          ನಿಲ್ಲಿಸು; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        ಮುಂದುವರಿ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !Dash_Dash_ಸಿಕ್ಕಿತು) {
      Dash_Dash_ಸಿಕ್ಕಿತು = ನಿಜ; // This is the mythical "--"?
      ಮುಂದುವರಿ;             // Don't try to process it as an argument itself.
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸಕ್ರಿಯ_Positional_Arg &&
               (ಸಕ್ರಿಯ_Positional_Arg->getMiscFlags() & Positional_Eats_ವಾದಗಳು)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ಮೌಲ್ಯ);
      ಆದರೆ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ಸಕ್ರಿಯ_Positional_Arg, StringRef(argv[i]), i);
        ಮುಂದುವರಿ; // We are done!
      }
    } ಇಲ್ಲದಿದ್ದರೆ { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, ಮೌಲ್ಯ);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      ಆದರೆ (!Handler && ChosenSubCommand != &SubCommand::get_ತುದಿ_Level())
        Handler = LookupOption(SubCommand::get_ತುದಿ_Level(), ArgName, ಮೌಲ್ಯ);

      ಆದರೆ (!Handler) {
        ಆದರೆ (LibraryOptions *L = lookupLibraryOption(ArgName.ವಿಭಜಿಸು('=').ಮೊದಲ_2)) {
          ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> Rest(argv + i, argc - i);
          ಚಿಹ್ನೆರಹಿತ N;
          ಆದರೆ (Error E = Context && L->is_ಪ್ರತಿ_Context()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ಕಾರ್ಯಕ್ರಮ_Name << ": " << ಗಮ್ಯ_String(std::move(E)) << '\n';
            ErrorParsing = ನಿಜ;
          }
          i += N - 1;
          ಮುಂದುವರಿ;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      ಆದರೆ (!Handler)
        Handler = HandlePrefixedOption(ArgName, ಮೌಲ್ಯ, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      ಆದರೆ (!Handler)
        Lookup_ಹತ್ತಿರದ_Option(ArgName, OptionsMap, ಹತ್ತಿರದ_Handler_String);
    }

    ಆದರೆ (!Handler) {
      ಸ್ವಯಂಚಾಲಿತ Report_Unknown_ವಾದ = [&](ತಾರ್ಕಿಕ IsArg,
                                       StringRef ಹತ್ತಿರದ_ವಾದ_Name) {
        *Errs << ಕಾರ್ಯಕ್ರಮ_Name << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        ಆದರೆ (ಹತ್ತಿರದ_ವಾದ_Name.ಖಾಲಿಗಳು())
          ಹಿಂತಿರುಗಿ;

        *Errs << ಕಾರ್ಯಕ್ರಮ_Name << ": Did you mean '";
        ಆದರೆ (IsArg)
          *Errs << PrintArg(ಹತ್ತಿರದ_ವಾದ_Name, 0);
        ಇಲ್ಲದಿದ್ದರೆ
          *Errs << ಹತ್ತಿರದ_ವಾದ_Name;
        *Errs << "'?\n";
      };

      ಆದರೆ (i > 1 || !MaybeNamedSubCommand)
        Report_Unknown_ವಾದ(/*IsArg=*/ನಿಜ, ಹತ್ತಿರದ_Handler_String);
      ಇಲ್ಲದಿದ್ದರೆ
        Report_Unknown_ವಾದ(/*IsArg=*/ಸುಳ್ಳು, ಹತ್ತಿರದ_Sub_Command_String);

      ErrorParsing = ನಿಜ;
      ಮುಂದುವರಿ;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    ಆದರೆ (Handler->getFormattingFlag() == cl::Positional) {
      ಆದರೆ ((Handler->getMiscFlags() & Positional_Eats_ವಾದಗಳು) && !ಮೌಲ್ಯ.ಖಾಲಿಗಳು()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = ನಿಜ;
      }
      ಸಕ್ರಿಯ_Positional_Arg = Handler;
    }
    ಇಲ್ಲದಿದ್ದರೆ
      ErrorParsing |= ProvideOption(Handler, ArgName, ಮೌಲ್ಯ, argc, argv, i);
  }

  // Check and handle positional arguments now...
  ಆದರೆ (ಅಂಕೆ_Positional_Required > PositionalVals.ಗಾತ್ರ_3()) {
      *Errs << ಕಾರ್ಯಕ್ರಮ_Name
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << ಅಂಕೆ_Positional_Required
             << " positional argument" << (ಅಂಕೆ_Positional_Required > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = ನಿಜ;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!HasUnlimitedPositionals &&
             PositionalVals.ಗಾತ್ರ_3() > PositionalOpts.ಗಾತ್ರ_3()) {
    *Errs << ಕಾರ್ಯಕ್ರಮ_Name << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.ಗಾತ್ರ_3()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = ನಿಜ;

  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    ಚಿಹ್ನೆರಹಿತ ValNo = 0, ಅಂಕೆ_Vals = static_cast<ಚಿಹ್ನೆರಹಿತ>(PositionalVals.ಗಾತ್ರ_3());
    ಗಾಗಿ (Option *Opt : PositionalOpts) {
      ಆದರೆ (Requires_ಮೌಲ್ಯ(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].ಮೊದಲ_2,
                                PositionalVals[ValNo].ಎರಡನೇ_2);
        ValNo++;
        --ಅಂಕೆ_Positional_Required; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      ತಾರ್ಕಿಕ ಮುಗಿದ = Opt->get_ಅಂಕೆ_Occurrences_Flag() == cl::Required;
      ಆಗ (ಅಂಕೆ_Vals - ValNo > ಅಂಕೆ_Positional_Required && !ಮುಗಿದ) {
        ಆಯ್ಕೆ (Opt->get_ಅಂಕೆ_Occurrences_Flag()) {
        ಸ್ಥಿತಿ cl::Optional:
          ಮುಗಿದ = ನಿಜ; // Optional arguments want _at most_ one value
          [[fallthrough]];
        ಸ್ಥಿತಿ cl::Zero_Or_ಇನ್ನಷ್ಟು: // Zero or more will take all they can get...
        ಸ್ಥಿತಿ cl::One_Or_ಇನ್ನಷ್ಟು:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].ಮೊದಲ_2,
                                  PositionalVals[ValNo].ಎರಡನೇ_2);
          ValNo++;
          ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(ConsumeAfterOpt && ಅಂಕೆ_Positional_Required <= PositionalVals.ಗಾತ್ರ_3());
    ಚಿಹ್ನೆರಹಿತ ValNo = 0;
    ಗಾಗಿ (Option *Opt : PositionalOpts)
      ಆದರೆ (Requires_ಮೌಲ್ಯ(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].ಮೊದಲ_2, PositionalVals[ValNo].ಎರಡನೇ_2);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    ಆದರೆ (PositionalOpts.ಗಾತ್ರ_3() == 1 && ValNo == 0 && !PositionalVals.ಖಾಲಿಗಳು()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].ಮೊದಲ_2,
                                              PositionalVals[ValNo].ಎರಡನೇ_2);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    ಗಾಗಿ (; ValNo != PositionalVals.ಗಾತ್ರ_3(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].ಮೊದಲ_2,
                                  PositionalVals[ValNo].ಎರಡನೇ_2);
  }

  // Loop over args and make sure all required args are specified!
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Opt : OptionsMap) {
    ಆಯ್ಕೆ (Opt.ಎರಡನೇ_2->get_ಅಂಕೆ_Occurrences_Flag()) {
    ಸ್ಥಿತಿ Required:
    ಸ್ಥಿತಿ One_Or_ಇನ್ನಷ್ಟು:
      ಆದರೆ (Opt.ಎರಡನೇ_2->get_ಅಂಕೆ_Occurrences() == 0) {
        Opt.ಎರಡನೇ_2->error("must be specified at least once!");
        ErrorParsing = ನಿಜ;
      }
      [[fallthrough]];
    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  ಇನ್ನಷ್ಟು_Help.clear();

  // If we had an error processing our arguments, don't let the program execute
  ಆದರೆ (ErrorParsing) {
    ಆದರೆ (!IgnoreErrors)
      ನಿರ್ಗಮಿಸು(1);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

ತಾರ್ಕಿಕ Option::error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_3, StringRef ArgName, raw_ostream &Errs) {
  ಆದರೆ (!ArgName.ದತ್ತಾಂಶ())
    ArgName = ArgStr;
  ಆದರೆ (ArgName.ಖಾಲಿಗಳು())
    Errs << HelpStr; // Be nice for positional arguments
  ಇಲ್ಲದಿದ್ದರೆ
    Errs << globalParser().ಕಾರ್ಯಕ್ರಮ_Name << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << ಸಂದೇಶ_3 << "\n";
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Option::ಕೂಡಿಸು_Occurrence(ಚಿಹ್ನೆರಹಿತ pos, StringRef ArgName, StringRef ಮೌಲ್ಯ) {
  ++ಅಂಕೆ_Occurrences;
  ಹಿಂತಿರುಗಿ handleOccurrence(pos, ArgName, ಮೌಲ್ಯ);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
ಸ್ಥಿರ StringRef get_ಮೌಲ್ಯ_Str(ಸ್ಥಿರಾಂಕ Option &O, StringRef Default_ಸಂದೇಶ) {
  ಆದರೆ (O.ಮೌಲ್ಯ_Str.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ Default_ಸಂದೇಶ;
  ಹಿಂತಿರುಗಿ O.ಮೌಲ್ಯ_Str;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::get_Option_ಅಗಲ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ arg_Plus_Prefixes_ಗಾತ್ರ(ArgStr);
}

ಶೂನ್ಯ Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t ಮೊದಲ_ಸಾಲು_Indented_By) {
  ಪ್ರತಿಜ್ಞಾ(Indent >= ಮೊದಲ_ಸಾಲು_Indented_By);
  std::pair<StringRef, StringRef> ವಿಭಜಿಸು_2 = HelpStr.ವಿಭಜಿಸು('\n');
  outs().indent(Indent - ಮೊದಲ_ಸಾಲು_Indented_By)
      << Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ << ವಿಭಜಿಸು_2.ಮೊದಲ_2 << "\n";
  ಆಗ (!ವಿಭಜಿಸು_2.ಎರಡನೇ_2.ಖಾಲಿಗಳು()) {
    ವಿಭಜಿಸು_2 = ವಿಭಜಿಸು_2.ಎರಡನೇ_2.ವಿಭಜಿಸು('\n');
    outs().indent(Indent) << ವಿಭಜಿಸು_2.ಮೊದಲ_2 << "\n";
  }
}

ಶೂನ್ಯ Option::printEnumValHelpStr(StringRef HelpStr, size_t ತಳ_Indent,
                                 size_t ಮೊದಲ_ಸಾಲು_Indented_By) {
  ಸ್ಥಿರಾಂಕ StringRef Val_Help_ಪೂರ್ವಪ್ರತ್ಯಯ = "  ";
  ಪ್ರತಿಜ್ಞಾ(ತಳ_Indent >= ಮೊದಲ_ಸಾಲು_Indented_By);
  std::pair<StringRef, StringRef> ವಿಭಜಿಸು_2 = HelpStr.ವಿಭಜಿಸು('\n');
  outs().indent(ತಳ_Indent - ಮೊದಲ_ಸಾಲು_Indented_By)
      << Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ << Val_Help_ಪೂರ್ವಪ್ರತ್ಯಯ << ವಿಭಜಿಸು_2.ಮೊದಲ_2 << "\n";
  ಆಗ (!ವಿಭಜಿಸು_2.ಎರಡನೇ_2.ಖಾಲಿಗಳು()) {
    ವಿಭಜಿಸು_2 = ವಿಭಜಿಸು_2.ಎರಡನೇ_2.ವಿಭಜಿಸು('\n');
    outs().indent(ತಳ_Indent + Val_Help_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3()) << ವಿಭಜಿಸು_2.ಮೊದಲ_2 << "\n";
  }
}

// Print out the option for the alias.
ಶೂನ್ಯ alias::printOptionInfo(size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, Global_ಅಗಲ, arg_Plus_Prefixes_ಗಾತ್ರ(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::get_Option_ಅಗಲ(ಸ್ಥಿರಾಂಕ Option &O) ಸ್ಥಿರಾಂಕ {
  size_t Len = arg_Plus_Prefixes_ಗಾತ್ರ(O.ArgStr);
  ಸ್ವಯಂಚಾಲಿತ ValName = get_ಮೌಲ್ಯ_Name();
  ಆದರೆ (!ValName.ಖಾಲಿಗಳು()) {
    size_t FormattingLen = 3;
    ಆದರೆ (O.getMiscFlags() & Positional_Eats_ವಾದಗಳು)
      FormattingLen = 6;
    Len += get_ಮೌಲ್ಯ_Str(O, ValName).ಗಾತ್ರ_3() + FormattingLen;
  }

  ಹಿಂತಿರುಗಿ Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ಶೂನ್ಯ basic_parser_impl::printOptionInfo(ಸ್ಥಿರಾಂಕ Option &O,
                                        size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  outs() << PrintArg(O.ArgStr);

  ಸ್ವಯಂಚಾಲಿತ ValName = get_ಮೌಲ್ಯ_Name();
  ಆದರೆ (!ValName.ಖಾಲಿಗಳು()) {
    ಆದರೆ (O.getMiscFlags() & Positional_Eats_ವಾದಗಳು) {
      outs() << " <" << get_ಮೌಲ್ಯ_Str(O, ValName) << ">...";
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (O.get_ಮೌಲ್ಯ_Expected_Flag() == ಮೌಲ್ಯ_Optional)
      outs() << "[=<" << get_ಮೌಲ್ಯ_Str(O, ValName) << ">]";
    ಇಲ್ಲದಿದ್ದರೆ {
      outs() << (O.ArgStr.ಗಾತ್ರ_3() == 1 ? " <" : "=<") << get_ಮೌಲ್ಯ_Str(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, Global_ಅಗಲ, get_Option_ಅಗಲ(O));
}

ಶೂನ್ಯ basic_parser_impl::printOptionName(ಸ್ಥಿರಾಂಕ Option &O,
                                        size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  outs() << PrintArg(O.ArgStr);
  outs().indent(Global_ಅಗಲ - O.ArgStr.ಗಾತ್ರ_3());
}

// parser<bool> implementation
//
ತಾರ್ಕಿಕ parser<ತಾರ್ಕಿಕ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ತಾರ್ಕಿಕ &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ parseBool<ತಾರ್ಕಿಕ, ನಿಜ, ಸುಳ್ಳು>(O, ArgName, Arg, ಮೌಲ್ಯ);
}

// parser<boolOrDefault> implementation
//
ತಾರ್ಕಿಕ parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, ಮೌಲ್ಯ);
}

// parser<FixedOrScalableQuantity> implementation
//
ಮಾದರಿ <ಬಗೆ_ಹೆಸರು FixedOrScalableQuantityT>
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ಮೌಲ್ಯ_ಬಗೆ,
                                         FixedOrScalableQuantityT &ಮೌಲ್ಯ) {
  ಬಳಸಿ ScalarTy = ಬಗೆ_ಹೆಸರು FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy Min_ಮೌಲ್ಯ;
  ಆದರೆ (!Arg.getAsInteger(0, Min_ಮೌಲ್ಯ)) {
    ಮೌಲ್ಯ = FixedOrScalableQuantityT::getFixed(Min_ಮೌಲ್ಯ);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  StringRef Remainder = Arg;
  ಆದರೆ (!Remainder.consume_front("vscale"))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_ಬಗೆ +
                   " argument!");

  Remainder = Remainder.ltrim();
  ಆದರೆ (!Remainder.consume_front('x'))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_ಬಗೆ +
                   " argument!");

  Remainder = Remainder.ltrim();
  ಆದರೆ (Remainder.getAsInteger(0, Min_ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for " + ಮೌಲ್ಯ_ಬಗೆ +
                   " argument!");

  ಮೌಲ್ಯ = FixedOrScalableQuantityT::getScalable(Min_ಮೌಲ್ಯ);
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<int> implementation
//
ತಾರ್ಕಿಕ parser<ಪೂರ್ಣಾಂಕ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        ಪೂರ್ಣಾಂಕ &ಮೌಲ್ಯ) {
  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for integer argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<long> implementation
//
ತಾರ್ಕಿಕ parser<ದೀರ್ಘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         ದೀರ್ಘ &ಮೌಲ್ಯ) {
  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for long argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<long long> implementation
//
ತಾರ್ಕಿಕ parser<ದೀರ್ಘ ದೀರ್ಘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              ದೀರ್ಘ ದೀರ್ಘ &ಮೌಲ್ಯ) {
  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for llong argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<unsigned> implementation
//
ತಾರ್ಕಿಕ parser<ಚಿಹ್ನೆರಹಿತ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             ಚಿಹ್ನೆರಹಿತ &ಮೌಲ್ಯ) {

  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for uint argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<unsigned long> implementation
//
ತಾರ್ಕಿಕ parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ &ಮೌಲ್ಯ) {

  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for ulong argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<unsigned long long> implementation
//
ತಾರ್ಕಿಕ parser<ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ &ಮೌಲ್ಯ) {

  ಆದರೆ (Arg.getAsInteger(0, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for ullong argument!");
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// parser<ElementCount> implementation
//
ತಾರ್ಕಿಕ parser<Element_ಎಣಿಕೆ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 Element_ಎಣಿಕೆ &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ parseFixedOrScalableQuantity(O, Arg, get_ಮೌಲ್ಯ_Name(), ಮೌಲ್ಯ);
}

// parser<double>/parser<float> implementation
//
ಸ್ಥಿರ ತಾರ್ಕಿಕ parseDouble(Option &O, StringRef Arg, ದ್ವಿಗುಣ &ಮೌಲ್ಯ) {
  ಆದರೆ (ಗಮ್ಯ_float(Arg, ಮೌಲ್ಯ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ O.error("'" + Arg + "' value invalid for floating point argument!");
}

ತಾರ್ಕಿಕ parser<ದ್ವಿಗುಣ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           ದ್ವಿಗುಣ &Val) {
  ಹಿಂತಿರುಗಿ parseDouble(O, Arg, Val);
}

ತಾರ್ಕಿಕ parser<ದಶಮಾಂಶ>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          ದಶಮಾಂಶ &Val) {
  ದ್ವಿಗುಣ dVal;
  ಆದರೆ (parseDouble(O, Arg, dVal))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  Val = (ದಶಮಾಂಶ)dVal;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
ಚಿಹ್ನೆರಹಿತ generic_parser_ತಳ::ಕಂಡುಹಿಡಿ_Option(StringRef Name) {
  ಚಿಹ್ನೆರಹಿತ e = get_ಅಂಕೆ_Options();

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != e; ++i) {
    ಆದರೆ (getOption(i) == Name)
      ಹಿಂತಿರುಗಿ i;
  }
  ಹಿಂತಿರುಗಿ e;
}

ಸ್ಥಿರ StringRef Eq_ಮೌಲ್ಯ = "=<value>";
ಸ್ಥಿರ StringRef ಖಾಲಿಗಳು_Option = "<empty>";
ಸ್ಥಿರ StringRef Option_ಪೂರ್ವಪ್ರತ್ಯಯ = "    =";
ಸ್ಥಿರ size_t get_Option_Prefixes_ಗಾತ್ರ() {
  ಹಿಂತಿರುಗಿ Option_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3() + Arg_Help_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3();
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ shouldPrintOption(StringRef Name, StringRef Description,
                              ಸ್ಥಿರಾಂಕ Option &O) {
  ಹಿಂತಿರುಗಿ O.get_ಮೌಲ್ಯ_Expected_Flag() != ಮೌಲ್ಯ_Optional || !Name.ಖಾಲಿಗಳು() ||
         !Description.ಖಾಲಿಗಳು();
}

// Return the width of the option tag for printing...
size_t generic_parser_ತಳ::get_Option_ಅಗಲ(ಸ್ಥಿರಾಂಕ Option &O) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (O.hasArgStr()) {
    size_t ಗಾತ್ರ_2 =
        arg_Plus_Prefixes_ಗಾತ್ರ(O.ArgStr) + Eq_ಮೌಲ್ಯ.ಗಾತ್ರ_3();
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef Name = getOption(i);
      ಆದರೆ (!shouldPrintOption(Name, getDescription(i), O))
        ಮುಂದುವರಿ;
      size_t Name_ಗಾತ್ರ = Name.ಖಾಲಿಗಳು() ? ಖಾಲಿಗಳು_Option.ಗಾತ್ರ_3() : Name.ಗಾತ್ರ_3();
      ಗಾತ್ರ_2 = std::max(ಗಾತ್ರ_2, Name_ಗಾತ್ರ + get_Option_Prefixes_ಗಾತ್ರ());
    }
    ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
  } ಇಲ್ಲದಿದ್ದರೆ {
    size_t ತಳ_ಗಾತ್ರ = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i)
      ತಳ_ಗಾತ್ರ = std::max(ತಳ_ಗಾತ್ರ, getOption(i).ಗಾತ್ರ_3() + 8);
    ಹಿಂತಿರುಗಿ ತಳ_ಗಾತ್ರ;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
ಶೂನ್ಯ generic_parser_ತಳ::printOptionInfo(ಸ್ಥಿರಾಂಕ Option &O,
                                          size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    ಆದರೆ (O.get_ಮೌಲ್ಯ_Expected_Flag() == ಮೌಲ್ಯ_Optional) {
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
        ಆದರೆ (getOption(i).ಖಾಲಿಗಳು()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, Global_ಅಗಲ,
                               arg_Plus_Prefixes_ಗಾತ್ರ(O.ArgStr));
          ನಿಲ್ಲಿಸು;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << Eq_ಮೌಲ್ಯ;
    Option::printHelpStr(O.HelpStr, Global_ಅಗಲ,
                         Eq_ಮೌಲ್ಯ.ಗಾತ್ರ_3() +
                             arg_Plus_Prefixes_ಗಾತ್ರ(O.ArgStr));
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      ಆದರೆ (!shouldPrintOption(OptionName, Description, O))
        ಮುಂದುವರಿ;
      size_t ಮೊದಲ_ಸಾಲು_Indent = OptionName.ಗಾತ್ರ_3() + get_Option_Prefixes_ಗಾತ್ರ();
      outs() << Option_ಪೂರ್ವಪ್ರತ್ಯಯ << OptionName;
      ಆದರೆ (OptionName.ಖಾಲಿಗಳು()) {
        outs() << ಖಾಲಿಗಳು_Option;
        ಪ್ರತಿಜ್ಞಾ(ಮೊದಲ_ಸಾಲು_Indent >= ಖಾಲಿಗಳು_Option.ಗಾತ್ರ_3());
        ಮೊದಲ_ಸಾಲು_Indent += ಖಾಲಿಗಳು_Option.ಗಾತ್ರ_3();
      }
      ಆದರೆ (!Description.ಖಾಲಿಗಳು())
        Option::printEnumValHelpStr(Description, Global_ಅಗಲ, ಮೊದಲ_ಸಾಲು_Indent);
      ಇಲ್ಲದಿದ್ದರೆ
        outs() << '\n';
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (!O.HelpStr.ಖಾಲಿಗಳು())
      outs() << "  " << O.HelpStr << '\n';
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Options(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), Global_ಅಗಲ, Option.ಗಾತ್ರ_3() + 8);
    }
  }
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ size_t Max_Opt_ಅಗಲ = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
ಶೂನ್ಯ generic_parser_ತಳ::printGenericOptionDiff(
    ಸ್ಥಿರಾಂಕ Option &O, ಸ್ಥಿರಾಂಕ Generic_Option_ಮೌಲ್ಯ &ಮೌಲ್ಯ,
    ಸ್ಥಿರಾಂಕ Generic_Option_ಮೌಲ್ಯ &Default, size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(Global_ಅಗಲ - O.ArgStr.ಗಾತ್ರ_3());

  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Opts = get_ಅಂಕೆ_Options();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಅಂಕೆ_Opts; ++i) {
    ಆದರೆ (!ಮೌಲ್ಯ.compare(get_Option_ಮೌಲ್ಯ(i)))
      ಮುಂದುವರಿ;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).ಗಾತ್ರ_3();
    size_t ಅಂಕೆ_Spaces = Max_Opt_ಅಗಲ > L ? Max_Opt_ಅಗಲ - L : 0;
    outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ j = 0; j != ಅಂಕೆ_Opts; ++j) {
      ಆದರೆ (!Default.compare(get_Option_ಮೌಲ್ಯ(j)))
        ಮುಂದುವರಿ;
      outs() << getOption(j);
      ನಿಲ್ಲಿಸು;
    }
    outs() << ")\n";
    ಹಿಂತಿರುಗಿ;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ cl {
ಸ್ಥಿರ raw_ostream &ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, boolOrDefault V) {
  ಹಿಂತಿರುಗಿ OS << static_cast<ಪೂರ್ಣಾಂಕ>(V);
}
} // namespace cl
} // namespace llvm

#ವ್ಯಾಖ್ಯಾನಿಸು PRINT_OPT_DIFF(T)                                                      \
  ಶೂನ್ಯ parser<T>::printOptionDiff(ಸ್ಥಿರಾಂಕ Option &O, T V, Option_ಮೌಲ್ಯ<T> D,      \
                                  size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {                  \
    printOptionName(O, Global_ಅಗಲ);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t ಅಂಕೆ_Spaces =                                                         \
        Max_Opt_ಅಗಲ > Str.ಗಾತ್ರ_3() ? Max_Opt_ಅಗಲ - Str.ಗಾತ್ರ_3() : 0;               \
    outs().indent(ಅಂಕೆ_Spaces) << " (default: ";                                 \
    ಆದರೆ (D.has_ಮೌಲ್ಯ())                                                          \
      outs() << D.get_ಮೌಲ್ಯ();                                                  \
    ಇಲ್ಲದಿದ್ದರೆ                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(ತಾರ್ಕಿಕ)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(ಪೂರ್ಣಾಂಕ)
PRINT_OPT_DIFF(ದೀರ್ಘ)
PRINT_OPT_DIFF(ದೀರ್ಘ ದೀರ್ಘ)
PRINT_OPT_DIFF(ಚಿಹ್ನೆರಹಿತ)
PRINT_OPT_DIFF(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)
PRINT_OPT_DIFF(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)
PRINT_OPT_DIFF(ದ್ವಿಗುಣ)
PRINT_OPT_DIFF(ದಶಮಾಂಶ)
PRINT_OPT_DIFF(ಅಕ್ಷರ)
PRINT_OPT_DIFF(Element_ಎಣಿಕೆ)

ಶೂನ್ಯ parser<std::string>::printOptionDiff(ಸ್ಥಿರಾಂಕ Option &O, StringRef V,
                                          ಸ್ಥಿರಾಂಕ Option_ಮೌಲ್ಯ<std::string> &D,
                                          size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  printOptionName(O, Global_ಅಗಲ);
  outs() << "= " << V;
  size_t ಅಂಕೆ_Spaces = Max_Opt_ಅಗಲ > V.ಗಾತ್ರ_3() ? Max_Opt_ಅಗಲ - V.ಗಾತ್ರ_3() : 0;
  outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
  ಆದರೆ (D.has_ಮೌಲ್ಯ())
    outs() << D.get_ಮೌಲ್ಯ();
  ಇಲ್ಲದಿದ್ದರೆ
    outs() << "*no default*";
  outs() << ")\n";
}

ಶೂನ್ಯ parser<std::optional<std::string>>::printOptionDiff(
    ಸ್ಥಿರಾಂಕ Option &O, std::optional<StringRef> V,
    ಸ್ಥಿರಾಂಕ Option_ಮೌಲ್ಯ<std::optional<std::string>> &D,
    size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  printOptionName(O, Global_ಅಗಲ);
  outs() << "= " << V;
  size_t VSize = V.has_ಮೌಲ್ಯ_2() ? V.ಮೌಲ್ಯ_2().ಗಾತ್ರ_3() : 0;
  size_t ಅಂಕೆ_Spaces = Max_Opt_ಅಗಲ > VSize ? Max_Opt_ಅಗಲ - VSize : 0;
  outs().indent(ಅಂಕೆ_Spaces) << " (default: ";
  ಆದರೆ (D.has_ಮೌಲ್ಯ() && D.get_ಮೌಲ್ಯ().has_ಮೌಲ್ಯ_2())
    outs() << D.get_ಮೌಲ್ಯ();
  ಇಲ್ಲದಿದ್ದರೆ
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
ಶೂನ್ಯ basic_parser_impl::print_Option_No_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ Option &O,
                                           size_t Global_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  printOptionName(O, Global_ಅಗಲ);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ OptNameCompare(ಸ್ಥಿರಾಂಕ std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *> *LHS,
                          ಸ್ಥಿರಾಂಕ std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *> *RHS) {
  ಹಿಂತಿರುಗಿ strcmp(LHS->ಮೊದಲ_2, RHS->ಮೊದಲ_2);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SubNameCompare(ಸ್ಥಿರಾಂಕ std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, SubCommand *> *LHS,
                          ಸ್ಥಿರಾಂಕ std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, SubCommand *> *RHS) {
  ಹಿಂತಿರುಗಿ strcmp(LHS->ಮೊದಲ_2, RHS->ಮೊದಲ_2);
}

// Copy Options into a vector so we can sort them as we like.
ಸ್ಥಿರ ಶೂನ್ಯ ವಿಂಗಡಿಸು_Opts(OptionsMapTy &OptMap,
                     Small_ಸದಿಶ_Impl<std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *>> &Opts,
                     ತಾರ್ಕಿಕ ShowHidden) {
  Small_Ptr_ಗಣ<Option *, 32> Option_ಗಣ; // Duplicate option detection.

  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = OptMap.begin(), E = OptMap.ಕೊನೆ(); I != E; ++I) {
    // Ignore really-hidden options.
    ಆದರೆ (I->ಎರಡನೇ_2->getOptionHiddenFlag() == ReallyHidden)
      ಮುಂದುವರಿ;

    // Unless showhidden is set, ignore hidden flags.
    ಆದರೆ (I->ಎರಡನೇ_2->getOptionHiddenFlag() == Hidden && !ShowHidden)
      ಮುಂದುವರಿ;

    // If we've already seen this option, don't add it to the list again.
    ಆದರೆ (!Option_ಗಣ.ಸೇರಿಸು(I->ಎರಡನೇ_2).ಎರಡನೇ_2)
      ಮುಂದುವರಿ;

    Opts.ತಳ್ಳು_ಹಿಂಭಾಗ(
        std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *>(I->ಮೊದಲ_2.ದತ್ತಾಂಶ(), I->ಎರಡನೇ_2));
  }

  // Sort the options list alphabetically.
  array_pod_ವಿಂಗಡಿಸು(Opts.begin(), Opts.ಕೊನೆ(), OptNameCompare);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ವಿಂಗಡಿಸು_Sub_Commands(ಸ್ಥಿರಾಂಕ Small_Ptr_ಗಣ_Impl<SubCommand *> &SubMap,
                Small_ಸದಿಶ_Impl<std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, SubCommand *>> &Subs) {
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *S : SubMap) {
    ಆದರೆ (S->getName().ಖಾಲಿಗಳು())
      ಮುಂದುವರಿ;
    Subs.ತಳ್ಳು_ಹಿಂಭಾಗ(std::make_pair(S->getName().ದತ್ತಾಂಶ(), S));
  }
  array_pod_ವಿಂಗಡಿಸು(Subs.begin(), Subs.ಕೊನೆ(), SubNameCompare);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ HelpPrinter {
ಸಂರಕ್ಷಿತ:
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ ShowHidden;
  ಬಳಸಿ Str_Option_Pair_ಸದಿಶ =
      Small_ಸದಿಶ<std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *>, 128>;
  ಬಳಸಿ Str_Sub_Command_Pair_ಸದಿಶ =
      Small_ಸದಿಶ<std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  ವಾಸ್ತವಿಕ ಶೂನ್ಯ printOptions(Str_Option_Pair_ಸದಿಶ &Opts, size_t MaxArgLen) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Opt : Opts)
      Opt.ಎರಡನೇ_2->printOptionInfo(MaxArgLen);
  }

  ಶೂನ್ಯ printSubCommands(Str_Sub_Command_Pair_ಸದಿಶ &Subs, size_t MaxSubLen) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &S : Subs) {
      outs() << "  " << S.ಮೊದಲ_2;
      ಆದರೆ (!S.ಎರಡನೇ_2->getDescription().ಖಾಲಿಗಳು()) {
        outs().indent(MaxSubLen - ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(S.ಮೊದಲ_2));
        outs() << " - " << S.ಎರಡನೇ_2->getDescription();
      }
      outs() << "\n";
    }
  }

ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ HelpPrinter(ತಾರ್ಕಿಕ showHidden) : ShowHidden(showHidden) {}
  ವಾಸ್ತವಿಕ ~HelpPrinter() = ಪೂರ್ವನಿಯೋಜಿತ;

  // Invoke the printer.
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
    ಆದರೆ (!ಮೌಲ್ಯ)
      ಹಿಂತಿರುಗಿ;
    printHelp();

    // Halt the program since help information was printed
    ನಿರ್ಗಮಿಸು(0);
  }

  ಶೂನ್ಯ printHelp() {
    SubCommand *Sub = globalParser().get_ಸಕ್ರಿಯ_Sub_Command();
    ಸ್ವಯಂಚಾಲಿತ &OptionsMap = Sub->OptionsMap;
    ಸ್ವಯಂಚಾಲಿತ &PositionalOpts = Sub->PositionalOpts;
    ಸ್ವಯಂಚಾಲಿತ &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    Str_Option_Pair_ಸದಿಶ Opts;
    ವಿಂಗಡಿಸು_Opts(OptionsMap, Opts, ShowHidden);

    Str_Sub_Command_Pair_ಸದಿಶ Subs;
    ವಿಂಗಡಿಸು_Sub_Commands(globalParser().RegisteredSubCommands, Subs);

    ಆದರೆ (!globalParser().ಕಾರ್ಯಕ್ರಮ_Overview.ಖಾಲಿಗಳು())
      outs() << "OVERVIEW: " << globalParser().ಕಾರ್ಯಕ್ರಮ_Overview << "\n";

    ಆದರೆ (Sub == &SubCommand::get_ತುದಿ_Level()) {
      outs() << "USAGE: " << globalParser().ಕಾರ್ಯಕ್ರಮ_Name;
      ಆದರೆ (!Subs.ಖಾಲಿಗಳು())
        outs() << " [subcommand]";
      outs() << " [options]";
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (!Sub->getDescription().ಖಾಲಿಗಳು()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ಕಾರ್ಯಕ್ರಮ_Name << " " << Sub->getName()
             << " [options]";
    }

    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ *Opt : PositionalOpts) {
      ಆದರೆ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    ಆದರೆ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    ಆದರೆ (Sub == &SubCommand::get_ತುದಿ_Level() && !Subs.ಖಾಲಿಗಳು()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Sub : Subs)
        MaxSubLen = std::max(MaxSubLen, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(Sub.ಮೊದಲ_2));

      outs() << "\n\n";
      outs() << "SUBCOMMANDS:\n\n";
      printSubCommands(Subs, MaxSubLen);
      outs() << "\n";
      outs() << "  Type \"" << globalParser().ಕಾರ್ಯಕ್ರಮ_Name
             << " <subcommand> --help\" to get more help on a specific "
                "subcommand";
    }

    outs() << "\n\n";

    // Compute the maximum argument length...
    size_t MaxArgLen = 0;
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.ಎರಡನೇ_2->get_Option_ಅಗಲ());

    Small_ಸದಿಶ<std::pair<std::string, StringRef>, 0> LibraryOpts;
    ಆದರೆ (ShowHidden)
      ಗಾಗಿ (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              ಆದರೆ (!Help.ಖಾಲಿಗಳು())
                LibraryOpts.emplace_ಹಿಂಭಾಗ((Spelling + MetaVar).str(), Help);
            });
    llvm::ವಿಂಗಡಿಸು(LibraryOpts);
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, arg_Plus_Prefixes_ಗಾತ್ರ(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    ಆದರೆ (!LibraryOpts.ಖಾಲಿಗಳು())
      outs() << "\nLibrary options:\n\n";
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, arg_Plus_Prefixes_ಗಾತ್ರ(Name));
    }

    // Print any extra help the user has declared.
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &I : globalParser().ಇನ್ನಷ್ಟು_Help)
      outs() << I;
    globalParser().ಇನ್ನಷ್ಟು_Help.clear();
  }
};

ವರ್ಗ CategorizedHelpPrinter : ಸಾರ್ವಜನಿಕ HelpPrinter {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ CategorizedHelpPrinter(ತಾರ್ಕಿಕ showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ OptionCategoryCompare(OptionCategory *ಸ್ಥಿರಾಂಕ *A,
                                   OptionCategory *ಸ್ಥಿರಾಂಕ *B) {
    ಹಿಂತಿರುಗಿ (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  ಬಳಸಿ HelpPrinter::ಕ್ರಿಯಾಕಾರಕ=;

ಸಂರಕ್ಷಿತ:
  ಶೂನ್ಯ printOptions(Str_Option_Pair_ಸದಿಶ &Opts, size_t MaxArgLen) override {
    std::ಸದಿಶ<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::ಸದಿಶ<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    ಪ್ರತಿಜ್ಞಾ(SortedCategories.ಗಾತ್ರ_3() > 0 && "No option categories registered!");
    array_pod_ವಿಂಗಡಿಸು(SortedCategories.begin(), SortedCategories.ಕೊನೆ(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &I : Opts) {
      Option *Opt = I.ಎರಡನೇ_2;
      ಗಾಗಿ (OptionCategory *Cat : Opt->Categories) {
        ಪ್ರತಿಜ್ಞಾ(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].ತಳ್ಳು_ಹಿಂಭಾಗ(Opt);
      }
    }

    // Now do printing.
    ಗಾಗಿ (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &CategoryOptions = CategorizedOptions[Category];
      ಆದರೆ (CategoryOptions.ಖಾಲಿಗಳು())
        ಮುಂದುವರಿ;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      ಆದರೆ (!Category->getDescription().ಖಾಲಿಗಳು())
        outs() << Category->getDescription() << "\n\n";
      ಇಲ್ಲದಿದ್ದರೆ
        outs() << "\n";

      // Loop over the options in the category and print.
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
ವರ್ಗ HelpPrinterWrapper {
ಖಾಸಗಿ:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ);
};

} // End anonymous namespace

#ಆದರೆ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# ಆದರೆ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# ಇಲ್ಲದಿದ್ದರೆ
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#ಅಥವಾಆದರೆ defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# ಆದರೆ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# ಇಲ್ಲದಿದ್ದರೆ
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#ಇಲ್ಲದಿದ್ದರೆ
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#ಆದರೆ_ಅಂತ್ಯ

ನಾಮಾಕಾಶ {
ವರ್ಗ ಆವೃತ್ತಿ_Printer {
ಸಾರ್ವಜನಿಕ:
  ಶೂನ್ಯ print(ಸ್ಥಿರಾಂಕ std::ಸದಿಶ<ಆವೃತ್ತಿ_Printer_Ty> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#ಇಲ್ಲದಿದ್ದರೆ
    OS << "LLVM (http://llvm.org/):\n  ";
#ಆದರೆ_ಅಂತ್ಯ
    OS << PACKAGE_NAME << " version " << PACKAGE_ಆವೃತ್ತಿ << "\n  ";
#ಆದರೆ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#ಇಲ್ಲದಿದ್ದರೆ
    OS << "Optimized build";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
    OS << " with assertions";
#ಆದರೆ_ಅಂತ್ಯ
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    ಆದರೆ (!ExtraPrinters.ಖಾಲಿಗಳು()) {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &I : ExtraPrinters)
        I(outs());
    }
  }
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ=(ತಾರ್ಕಿಕ OptionWasSpecified);
};

ರಚನೆ Command_ಸಾಲು_Common_Options {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter Uncategorized_ಸಾಮಾನ್ಯ_Printer{ಸುಳ್ಳು};
  HelpPrinter UncategorizedHiddenPrinter{ನಿಜ};
  CategorizedHelpPrinter Categorized_ಸಾಮಾನ್ಯ_Printer{ಸುಳ್ಳು};
  CategorizedHelpPrinter CategorizedHiddenPrinter{ನಿಜ};
  // Declare HelpPrinter wrappers that will decide whether or not to invoke
  // a categorizing help printer
  HelpPrinterWrapper Wrapped_ಸಾಮಾನ್ಯ_Printer{Uncategorized_ಸಾಮಾನ್ಯ_Printer,
                                          Categorized_ಸಾಮಾನ್ಯ_Printer};
  HelpPrinterWrapper WrappedHiddenPrinter{UncategorizedHiddenPrinter,
                                          CategorizedHiddenPrinter};
  // Define a category for generic options that all tools should have.
  cl::OptionCategory GenericCategory{"Generic Options"};

  // Define uncategorized help printers.
  // --help-list is hidden by default because if Option categories are being
  // used then --help behaves the same as --help-list.
  cl::opt<HelpPrinter, ನಿಜ, parser<ತಾರ್ಕಿಕ>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(Uncategorized_ಸಾಮಾನ್ಯ_Printer),
      cl::Hidden,
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, ನಿಜ, parser<ತಾರ್ಕಿಕ>> HLHOp{
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
  cl::opt<HelpPrinterWrapper, ನಿಜ, parser<ತಾರ್ಕಿಕ>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(Wrapped_ಸಾಮಾನ್ಯ_Printer),
      cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, ನಿಜ, parser<ತಾರ್ಕಿಕ>> HHOp{
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
      cl::init(ಸುಳ್ಳು),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<ತಾರ್ಕಿಕ> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(ಸುಳ್ಳು),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  ಆವೃತ್ತಿ_Printer_Ty Override_ಆವೃತ್ತಿ_Printer = ಶೂನ್ಯಸೂಚಕ;

  std::ಸದಿಶ<ಆವೃತ್ತಿ_Printer_Ty> Extra_ಆವೃತ್ತಿ_Printers;

  // Define the --version option that prints out the LLVM version for the tool
  ಆವೃತ್ತಿ_Printer ಆವೃತ್ತಿ_Printer_Instance;

  cl::opt<ಆವೃತ್ತಿ_Printer, ನಿಜ, parser<ತಾರ್ಕಿಕ>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(ಆವೃತ್ತಿ_Printer_Instance), cl::ಮೌಲ್ಯ_Disallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
ಸ್ಥಿರ ManagedStatic<Command_ಸಾಲು_Common_Options> CommonOptions;

ಸ್ಥಿರ ಶೂನ್ಯ initCommonOptions() {
  *CommonOptions;
  initDebugCounterOptions();
  init_ನಕ್ಷೆ_Writer_Options();
  init_ಸಂಕೇತಗಳು_Options();
  initStatisticOptions();
  initTimerOptions();
  initWithColorOptions();
  initDebugOptions();
  init_Random_ಬೀಜ_Options();
}

OptionCategory &cl::getGeneralCategory() {
  // Initialise the general option category.
  ಸ್ಥಿರ OptionCategory GeneralCategory{"General options"};
  ಹಿಂತಿರುಗಿ GeneralCategory;
}

ಶೂನ್ಯ ಆವೃತ್ತಿ_Printer::ಕ್ರಿಯಾಕಾರಕ=(ತಾರ್ಕಿಕ OptionWasSpecified) {
  ಆದರೆ (!OptionWasSpecified)
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ (CommonOptions->Override_ಆವೃತ್ತಿ_Printer != ಶೂನ್ಯಸೂಚಕ) {
    CommonOptions->Override_ಆವೃತ್ತಿ_Printer(outs());
    ನಿರ್ಗಮಿಸು(0);
  }
  print(CommonOptions->Extra_ಆವೃತ್ತಿ_Printers);

  ನಿರ್ಗಮಿಸು(0);
}

ಶೂನ್ಯ HelpPrinterWrapper::ಕ್ರಿಯಾಕಾರಕ=(ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
  ಆದರೆ (!ಮೌಲ್ಯ)
    ಹಿಂತಿರುಗಿ;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  ಆದರೆ (globalParser().RegisteredOptionCategories.ಗಾತ್ರ_3() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.ಗಣ_Hidden_Flag(NotHidden);

    CategorizedPrinter = ನಿಜ; // Invoke categorized printer
  } ಇಲ್ಲದಿದ್ದರೆ {
    UncategorizedPrinter = ನಿಜ; // Invoke uncategorized printer
  }
}

// Print the value of each option.
ಶೂನ್ಯ cl::Print_Option_ಮೌಲ್ಯಗಳು() { globalParser().print_Option_ಮೌಲ್ಯಗಳು(); }

ಶೂನ್ಯ Command_ಸಾಲು_Parser::print_Option_ಮೌಲ್ಯಗಳು() {
  ಆದರೆ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ಹಿಂತಿರುಗಿ;

  Small_ಸದಿಶ<std::pair<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, Option *>, 128> Opts;
  ವಿಂಗಡಿಸು_Opts(ಸಕ್ರಿಯ_Sub_Command->OptionsMap, Opts, /*ShowHidden*/ ನಿಜ);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.ಎರಡನೇ_2->get_Option_ಅಗಲ());

  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Opt : Opts)
    Opt.ಎರಡನೇ_2->print_Option_ಮೌಲ್ಯ(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
ಶೂನ್ಯ cl::Print_Help_ಸಂದೇಶ(ತಾರ್ಕಿಕ Hidden, ತಾರ್ಕಿಕ Categorized) {
  ಆದರೆ (!Hidden && !Categorized)
    CommonOptions->Uncategorized_ಸಾಮಾನ್ಯ_Printer.printHelp();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!Hidden && Categorized)
    CommonOptions->Categorized_ಸಾಮಾನ್ಯ_Printer.printHelp();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  ಇಲ್ಲದಿದ್ದರೆ
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#ಆದರೆ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
      "+assertions",
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EXPENSIVE_CHECKS
      "+expensive-checks",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(ವಿಳಾಸ_sanitizer)
      "+asan",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(dataflow_sanitizer)
      "+dfsan",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(ಸ್ಮರಣೆ_sanitizer)
      "+msan",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(thread_sanitizer)
      "+tsan",
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#ಆದರೆ_ಅಂತ್ಯ
  };
  ಹಿಂತಿರುಗಿ ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
ಶೂನ್ಯ cl::printBuildConfig(raw_ostream &OS) {
#ಆದರೆ LLVM_ಆವೃತ್ತಿ_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#ಆದರೆ_ಅಂತ್ಯ
}

/// Utility function for printing version number.
ಶೂನ್ಯ cl::Print_ಆವೃತ್ತಿ_ಸಂದೇಶ() {
  CommonOptions->ಆವೃತ್ತಿ_Printer_Instance.print(CommonOptions->Extra_ಆವೃತ್ತಿ_Printers);
}

ಶೂನ್ಯ cl::ಗಣ_ಆವೃತ್ತಿ_Printer(ಆವೃತ್ತಿ_Printer_Ty func) {
  CommonOptions->Override_ಆವೃತ್ತಿ_Printer = func;
}

ಶೂನ್ಯ cl::ಕೂಡಿಸು_Extra_ಆವೃತ್ತಿ_Printer(ಆವೃತ್ತಿ_Printer_Ty func) {
  CommonOptions->Extra_ಆವೃತ್ತಿ_Printers.ತಳ್ಳು_ಹಿಂಭಾಗ(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  ಸ್ವಯಂಚಾಲಿತ &Subs = globalParser().RegisteredSubCommands;
  (ಶೂನ್ಯ)Subs;
  ಪ್ರತಿಜ್ಞಾ(Subs.ಒಳಗೊಂಡಿದೆ(&Sub));
  ಹಿಂತಿರುಗಿ Sub.OptionsMap;
}

iterator_range<Small_Ptr_ಗಣ<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ಹಿಂತಿರುಗಿ globalParser().getRegisteredSubcommands();
}

ಶೂನ್ಯ cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : Sub.OptionsMap) {
    ತಾರ್ಕಿಕ Unrelated = ನಿಜ;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Cat : I.ಎರಡನೇ_2->Categories) {
      ಆದರೆ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = ಸುಳ್ಳು;
    }
    ಆದರೆ (Unrelated)
      I.ಎರಡನೇ_2->ಗಣ_Hidden_Flag(cl::ReallyHidden);
  }
}

ಶೂನ್ಯ cl::HideUnrelatedOptions(ArrayRef<ಸ್ಥಿರಾಂಕ cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : Sub.OptionsMap) {
    ತಾರ್ಕಿಕ Unrelated = ನಿಜ;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Cat : I.ಎರಡನೇ_2->Categories) {
      ಆದರೆ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = ಸುಳ್ಳು;
    }
    ಆದರೆ (Unrelated)
      I.ಎರಡನೇ_2->ಗಣ_Hidden_Flag(cl::ReallyHidden);
  }
}

ಶೂನ್ಯ cl::Reset_Command_ಸಾಲು_Parser() { globalParser().reset(); }

ಶೂನ್ಯ cl::ಕೂಡಿಸು_Library_Options(LibraryOptions &L) {
  globalParser().Libraries.ತಳ್ಳು_ಹಿಂಭಾಗ(&L);
}

ಶೂನ್ಯ LibraryOptionsContext::ನಿಯೋಜಿಸು(ಸ್ಥಿರಾಂಕ LibraryOptionsContext &Other) {
  ಪ್ರತಿಜ್ಞಾ(ಇದು != &Other);
  ಮೌಲ್ಯಗಳು.clear();
  Saved_ವಾದಗಳು.clear();
  ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು = Other.Saved_ವಾದಗಳು;
  ಗಾಗಿ (size_t I = 0, E = ವಾದಗಳು.ಗಾತ್ರ_3(); I != E;) {
    StringRef Name = ವಾದಗಳು[I];
    Name.consume_front("-");
    Name.consume_front("-");
    LibraryOptions *L =
        globalParser().lookupLibraryOption(Name.ವಿಭಜಿಸು('=').ಮೊದಲ_2);
    ಚಿಹ್ನೆರಹಿತ N;
    cantFail(parse(*L, ವಾದಗಳು.drop_front(I), N));
    I += N;
  }
}

Error LibraryOptionsContext::parse(LibraryOptions &L,
                                   ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು,
                                   ಚಿಹ್ನೆರಹಿತ &Consumed) {
  ಸ್ವಯಂಚಾಲಿತ [It, Inserted] = ಮೌಲ್ಯಗಳು.try_emplace(L.getDefault());
  ಆದರೆ (Inserted)
    It->ಎರಡನೇ_2 = L.copyDefault(Alloc);
  ಹಿಂತಿರುಗಿ parseLibraryOption(L, It->ಎರಡನೇ_2, ವಾದಗಳು, Consumed, Alloc, &Saved_ವಾದಗಳು);
}

ಶೂನ್ಯ cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

ಶೂನ್ಯ LLVMParse_Command_ಸಾಲು_Options(ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Overview) {
  llvm::cl::Parse_Command_ಸಾಲು_Options(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
