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

#include "llvm/Support/CommandLine.h"

#include "DebugOptions.h"

#include "llvm-c/Support.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/LibraryOptionsContext.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <optional>
#include <string>
using namespace llvm;
using namespace cl;

#define DEBUG_TYPE "commandline"

//===----------------------------------------------------------------------===//
// Template instantiations and anchors.
//
namespace llvm {
namespace cl {
template class LLVM_EXPORT_TEMPLATE basic_parser<bool>;
template class LLVM_EXPORT_TEMPLATE basic_parser<boolOrDefault>;
template class LLVM_EXPORT_TEMPLATE basic_parser<ကိန်းပြည့်>;
template class LLVM_EXPORT_TEMPLATE basic_parser<long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<dviguu_na>;
template class LLVM_EXPORT_TEMPLATE basic_parser<float>;
template class LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template class LLVM_EXPORT_TEMPLATE basic_parser<var_na>;
template class LLVM_EXPORT_TEMPLATE basic_parser<ElementCount>;

#အကယ်၍ !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template class LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template class LLVM_EXPORT_TEMPLATE opt<bool>;
template class LLVM_EXPORT_TEMPLATE opt<var_na>;
template class LLVM_EXPORT_TEMPLATE opt<ကိန်းပြည့်>;
template class LLVM_EXPORT_TEMPLATE opt<unsigned>;

} // namespace cl
} // namespace llvm

// Pin the vtables to this file.
void GenericOptionValue::anchor() {}
void OptionValue<boolOrDefault>::anchor() {}
void OptionValue<std::string>::anchor() {}
void Option::anchor() {}
void basic_parser_impl::anchor() {}
void parser<bool>::anchor() {}
void parser<boolOrDefault>::anchor() {}
void parser<ကိန်းပြည့်>::anchor() {}
void parser<long>::anchor() {}
void parser<long long>::anchor() {}
void parser<unsigned>::anchor() {}
void parser<unsigned long>::anchor() {}
void parser<unsigned long long>::anchor() {}
void parser<dviguu_na>::anchor() {}
void parser<float>::anchor() {}
void parser<std::string>::anchor() {}
void parser<std::optional<std::string>>::anchor() {}
void parser<var_na>::anchor() {}
void parser<ElementCount>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
void opt_bool_anchor() { opt<bool> anchor{""}; }
void opt_char_anchor() { opt<var_na> anchor{""}; }
void opt_int_anchor() { opt<ကိန်းပြည့်> anchor{""}; }
void opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

const static size_t DefaultPad = 2;

static StringRef ArgPrefix = "-";
static StringRef ArgPrefixLong = "--";
static StringRef ArgHelpPrefix = " - ";

static size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  အကယ်၍ (Len == 1)
    ပြန်ပေး Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  ပြန်ပေး Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

static SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  အတွက် (size_t I = 0; I < Pad; ++I) {
    Prefix.push_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  ပြန်ပေး Prefix;
}

using OptionsMapTy = DenseMap<StringRef, Option *>;

namespace {

class PrintArg {
  StringRef ArgName;
  size_t Pad;
public:
  PrintArg(StringRef ArgName, size_t Pad = DefaultPad) : ArgName(ArgName), Pad(Pad) {}
  friend raw_ostream &operator<<(raw_ostream &OS, const PrintArg &);
};

raw_ostream &operator<<(raw_ostream &OS, const PrintArg& Arg) {
  OS << argPrefix(Arg.ArgName, Arg.Pad) << Arg.ArgName;
  ပြန်ပေး OS;
}

class CommandLineParser {
public:
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
  size_t NumIndexedLibraries = 0;
  // Storage that library options refer to: copies of the arguments passed to
  // LibraryOptions::parse and what it allocates. Response file expansions do
  // not outlive parsing.
  BumpPtrAllocator LibraryArgAlloc;

  CommandLineParser() { registerSubCommand(&SubCommand::getTopLevel()); }

  void ResetAllOptionOccurrences();

  bool ParseCommandLineOptions(ကိန်းပြည့် argc, const var_na *const *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  void forEachSubCommand(Option &Opt, function_ref<void(SubCommand &)> Action) {
    အကယ်၍ (Opt.Subs.empty()) {
      Action(SubCommand::getTopLevel());
      ပြန်ပေး;
    }
    အကယ်၍ (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      အတွက် (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      ပြန်ပေး;
    }
    အတွက် (auto *SC : Opt.Subs) {
      assert(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  void addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    အကယ်၍ (Opt.hasArgStr())
      ပြန်ပေး;
    အကယ်၍ (!SC->OptionsMap.insert(std::make_pair(Name, &Opt)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << Name
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
  }

  void addLiteralOption(Option &Opt, StringRef Name) {
    forEachSubCommand(
        Opt, [&](SubCommand &SC) { addLiteralOption(Opt, &SC, Name); });
  }

  void addOption(Option *O, SubCommand *SC) {
    bool HadErrors = false;
    အကယ်၍ (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      အကယ်၍ (!SC->OptionsMap.insert(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = true;
      }
    }

    // Remember information about positional options.
    အကယ်၍ (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.push_back(O);
    မဟုတ်ပါက အကယ်၍ (O->getNumOccurrencesFlag() == cl::ConsumeAfter) {
      အကယ်၍ (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = true;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    အကယ်၍ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  void addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  void removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    အကယ်၍ (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    SubCommand &Sub = *SC;
    အတွက် (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      အကယ်၍ (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    အကယ်၍ (O->getFormattingFlag() == cl::Positional)
      အတွက် (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        အကယ်၍ (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          bha_nga;
        }
      }
    မဟုတ်ပါက အကယ်၍ (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  void removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  bool hasOptions(const SubCommand &Sub) const {
    ပြန်ပေး (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  bool hasOptions() const {
    အတွက် (const auto *S : RegisteredSubCommands) {
      အကယ်၍ (hasOptions(*S))
        ပြန်ပေး true;
    }
    ပြန်ပေး !Libraries.empty();
  }

  bool hasNamedSubCommands() const {
    အတွက် (const auto *S : RegisteredSubCommands)
      အကယ်၍ (!S->getName().empty())
        ပြန်ပေး true;
    ပြန်ပေး false;
  }

  SubCommand *getActiveSubCommand() { ပြန်ပေး ActiveSubCommand; }

  void updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    အကယ်၍ (!Sub.OptionsMap.insert(std::make_pair(NewName, O)).second) {
      errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
             << "' registered more than once!\n";
      report_fatal_error("inconsistency in registered CommandLine options");
    }
    Sub.OptionsMap.erase(O->ArgStr);
  }

  void updateArgStr(Option *O, StringRef NewName) {
    forEachSubCommand(*O,
                      [&](SubCommand &SC) { updateArgStr(O, NewName, &SC); });
  }

  void printOptionValues();

  void indexLibraryOptions() {
    bool HadErrors = false;
    အတွက် (; NumIndexedLibraries != Libraries.size(); ++NumIndexedLibraries) {
      LibraryOptions *L = Libraries[NumIndexedLibraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        အကယ်၍ (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 ပြန်ပေး SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          ပြန်ပေး;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = true;
      });
    }
    အကယ်၍ (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    ပြန်ပေး LibraryIndex.lookup(Name);
  }

  void registerCategory(OptionCategory *cat) {
    assert(count_if(RegisteredOptionCategories,
                    [cat](const OptionCategory *Category) {
             ပြန်ပေး cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.insert(cat);
  }

  void registerSubCommand(SubCommand *sub) {
    assert(count_if(RegisteredSubCommands,
                    [sub](const SubCommand *Sub) {
                      ပြန်ပေး (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.insert(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    assert(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    အတွက် (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      အကယ်၍ (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      မဟုတ်ပါက
        addLiteralOption(*O, sub, E.first);
    }
  }

  void unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    ပြန်ပေး make_range(RegisteredSubCommands.begin(),
                      RegisteredSubCommands.end());
  }

  void reset() {
    ActiveSubCommand = nullptr;
    ProgramName.clear();
    ProgramOverview = StringRef();

    MoreHelp.clear();
    RegisteredOptionCategories.clear();

    ResetAllOptionOccurrences();
    RegisteredSubCommands.clear();
    Libraries.clear();
    LibraryIndex.clear();
    NumIndexedLibraries = 0;

    SubCommand::getTopLevel().reset();
    SubCommand::getAll().reset();
    registerSubCommand(&SubCommand::getTopLevel());
  }

private:
  SubCommand *ActiveSubCommand = nullptr;

  Option *LookupOption(SubCommand &Sub, StringRef &Arg, StringRef &Value);
  SubCommand *LookupSubCommand(StringRef Name, std::string &NearestString);
};

} // namespace

// The global parser is kept as a block-scope static so that option
// constructors running during dynamic initialization of other translation
// units never reference a namespace-scope global whose initialization order
// is unspecified. The ManagedStatic keeps construction lazy and destruction
// tied to llvm_shutdown().
static CommandLineParser &globalParser() {
  static ManagedStatic<CommandLineParser> GlobalParser;
  ပြန်ပေး *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
static Error
parseLibraryOption(LibraryOptions &L, void *Opts, ArrayRef<const var_na *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<const var_na *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  const var_na *Saved[2];
  unsigned NumArgs = std::min<size_t>(Args.size(), 2);
  အတွက် (unsigned J = 0; J != NumArgs; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  အကယ်၍ (Error E = L.parse(Opts, ArrayRef(Saved, NumArgs), Consumed, Alloc))
    ပြန်ပေး E;
  အကယ်၍ (Record)
    Record->append(Saved, Saved + Consumed);
  ပြန်ပေး Error::success();
}

template <typename T, T TrueVal, T FalseVal>
static bool parseBool(Option &O, StringRef ArgName, StringRef Arg, T &Value) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  အကယ်၍ (!Arg.data() || Arg == "true" || Arg == "1") {
    Value = TrueVal;
    ပြန်ပေး false;
  }

  အကယ်၍ (Arg == "false" || Arg == "0") {
    Value = FalseVal;
    ပြန်ပေး false;
  }
  ပြန်ပေး O.error("'" + Arg +
                 "' is invalid value for boolean argument! Try 0 or 1");
}

void cl::AddLiteralOption(Option &O, StringRef Name) {
  globalParser().addLiteralOption(O, Name);
}

extrahelp::extrahelp(StringRef Help) : morehelp(Help) {
  globalParser().MoreHelp.push_back(Help);
}

Option::Option(NumOccurrencesFlag OccurrencesFlag, OptionHidden Hidden)
    : NumOccurrences(0), Occurrences(OccurrencesFlag), Value(0),
      HiddenFlag(Hidden), Formatting(NormalFormatting), Misc(0),
      FullyInitialized(false), Position(0) {
  Categories.push_back(&getGeneralCategory());
}

void Option::addArgument() {
  globalParser().addOption(this);
  FullyInitialized = true;
}

void Option::removeArgument() { globalParser().removeOption(this); }

void Option::setArgStr(StringRef S) {
  အကယ်၍ (FullyInitialized)
    globalParser().updateArgStr(this, S);
  assert(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

void Option::addCategory(OptionCategory &C) {
  assert(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  အကယ်၍ (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  မဟုတ်ပါက အကယ်၍ (!is_contained(Categories, &C))
    Categories.push_back(&C);
}

void Option::reset() {
  NumOccurrences = 0;
  setDefault();
}

void OptionCategory::registerCategory() {
  globalParser().registerCategory(this);
}

// A special subcommand representing no subcommand. It is kept as a
// block-scope static because it is referenced from cl::opt constructors,
// which run dynamically in an arbitrary order across translation units;
// block-scope statics are initialized on first use and therefore have no
// initialization-order hazard.
SubCommand &SubCommand::getTopLevel() {
  static ManagedStatic<SubCommand> TopLevelSubCommand;
  ပြန်ပေး *TopLevelSubCommand;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  static ManagedStatic<SubCommand> AllSubCommands;
  ပြန်ပေး *AllSubCommands;
}

void SubCommand::registerSubCommand() {
  globalParser().registerSubCommand(this);
}

void SubCommand::unregisterSubCommand() {
  globalParser().unregisterSubCommand(this);
}

void SubCommand::reset() {
  PositionalOpts.clear();
  OptionsMap.clear();

  ConsumeAfterOpt = nullptr;
}

SubCommand::operator bool() const {
  ပြန်ပေး (globalParser().getActiveSubCommand() == this);
}

//===----------------------------------------------------------------------===//
// Basic, shared command line option processing machinery.
//

/// LookupOption - Lookup the option specified by the specified option on the
/// command line.  If there is a value specified (after an equal sign) return
/// that as well.  This assumes that leading dashes have already been stripped.
Option *CommandLineParser::LookupOption(SubCommand &Sub, StringRef &Arg,
                                        StringRef &Value) {
  // Reject all dashes.
  အကယ်၍ (Arg.empty())
    ပြန်ပေး nullptr;
  assert(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  အကယ်၍ (EqualPos == StringRef::npos) {
    // Look up the option.
    ပြန်ပေး Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  အကယ်၍ (I == Sub.OptionsMap.end())
    ပြန်ပေး nullptr;

  auto *O = I->second;
  အကယ်၍ (O->getFormattingFlag() == cl::AlwaysPrefix)
    ပြန်ပေး nullptr;

  Value = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  ပြန်ပေး I->second;
}

SubCommand *CommandLineParser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  အကယ်၍ (Name.empty())
    ပြန်ပေး &SubCommand::getTopLevel();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  အတွက် (auto *S : RegisteredSubCommands) {
    assert(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    အကယ်၍ (S->getName().empty())
      continue;

    အကယ်၍ (S->getName() == Name)
      ပြန်ပေး S;

    အကယ်၍ (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  အကယ်၍ (NearestMatch)
    NearestString = NearestMatch->getName();

  ပြန်ပေး &SubCommand::getTopLevel();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
static Option *LookupNearestOption(StringRef Arg,
                                   const OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  အကယ်၍ (Arg.empty())
    ပြန်ပေး nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  အတွက် (const auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    အကယ်၍ (O->getOptionHiddenFlag() == ReallyHidden)
      continue;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    အကယ်၍ (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    bool PermitValue = O->getValueExpectedFlag() != cl::ValueDisallowed;
    StringRef Flag = PermitValue ? LHS : Arg;
    အတွက် (const auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/true, /*MaxEditDistance=*/BestDistance);
      အကယ်၍ (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        အကယ်၍ (RHS.empty() || !PermitValue)
          NearestString = std::string(Name);
        မဟုတ်ပါက
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  ပြန်ပေး Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
static bool CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef Value) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  အကယ်၍ (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(Value);
    StringRef::size_type Pos = Val.find(',');

    နေစဉ် (Pos != StringRef::npos) {
      // Process the portion before the comma.
      အကယ်၍ (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        ပြန်ပေး true;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    Value = Val;
  }

  ပြန်ပေး Handler->addOccurrence(pos, ArgName, Value);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
static anta_hstha bool ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef Value, ကိန်းပြည့် argc,
                                 const var_na *const *argv, ကိန်းပြည့် &i) {
  // Enforce value requirements
  switch (Handler->getValueExpectedFlag()) {
  pak_sa ValueRequired:
    အကယ်၍ (!Value.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      အကယ်၍ (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        ပြန်ပေး Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      assert(argv && "null check");
      Value = StringRef(argv[++i]);
    }
    bha_nga;
  pak_sa ValueDisallowed:
    အကယ်၍ (Value.data())
      ပြန်ပေး Handler->error("does not allow a value! '" + Twine(Value) +
                            "' specified.");
    bha_nga;
  pak_sa ValueOptional:
    bha_nga;
  }

  ပြန်ပေး CommaSeparateAndAddOccurrence(Handler, i, ArgName, Value);
}

bool llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, ကိန်းပြည့် i) {
  ကိန်းပြည့် Dummy = i;
  ပြန်ပေး ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
static Option *findPrefixOption(StringRef Name, size_t &Length,
                                const OptionsMapTy &OptionsMap) {
  အတွက် (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    အကယ်၍ (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      ပြန်ပေး O;
    }
  }
  ပြန်ပေး nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
static Option *HandlePrefixedOption(StringRef &Arg, StringRef &Value,
                                    const OptionsMapTy &OptionsMap) {
  အကယ်၍ (Arg.size() == 1)
    ပြန်ပေး nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  အကယ်၍ (!POpt)
    ပြန်ပေး nullptr;

  StringRef MaybeValue =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  အကယ်၍ (POpt->getFormattingFlag() == cl::Prefix && MaybeValue.starts_with("="))
    MaybeValue = MaybeValue.drop_front();
  Value = MaybeValue;
  ပြန်ပေး POpt;
}

static bool RequiresValue(const Option *O) {
  ပြန်ပေး O->getNumOccurrencesFlag() == cl::Required ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

static bool EatsUnboundedNumberOfValues(const Option *O) {
  ပြန်ပေး O->getNumOccurrencesFlag() == cl::ZeroOrMore ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

static bool isWhitespace(var_na C) {
  ပြန်ပေး C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

static bool isWhitespaceOrNull(var_na C) {
  ပြန်ပေး isWhitespace(C) || C == '\0';
}

static bool isQuote(var_na C) { ပြန်ပေး C == '\"' || C == '\''; }

void cl::TokenizeGNUCommandLine(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<const var_na *> &NewArgv,
                                bool MarkEOLs) {
  SmallString<128> Token;
  bool InToken = false;
  အတွက် (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    အကယ်၍ (!InToken) {
      နေစဉ် (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        အကယ်၍ (MarkEOLs && Src[I] == '\n')
          NewArgv.push_back(nullptr);
        ++I;
      }
      အကယ်၍ (I == E)
        bha_nga;
      InToken = true;
    }

    var_na C = Src[I];

    // Backslash escapes the next character.
    အကယ်၍ (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.push_back(Src[I]);
      continue;
    }

    // Consume a quoted string.
    အကယ်၍ (isQuote(C)) {
      ++I;
      နေစဉ် (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        အကယ်၍ (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.push_back(Src[I]);
        ++I;
      }
      အကယ်၍ (I == E)
        bha_nga;
      continue;
    }

    // End the token if this is whitespace.
    အကယ်၍ (isWhitespace(C)) {
      NewArgv.push_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      အကယ်၍ (MarkEOLs && C == '\n')
        NewArgv.push_back(nullptr);
      Token.clear();
      InToken = false;
      continue;
    }

    // This is a normal character.  Append it.
    Token.push_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  အကယ်၍ (InToken)
    NewArgv.push_back(Saver.save(Token.str()).data());
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
static size_t parseBackslash(StringRef Src, size_t I, SmallString<128> &Token) {
  size_t E = Src.size();
  ကိန်းပြည့် BackslashCount = 0;
  // Skip the backslashes.
  do {
    ++I;
    ++BackslashCount;
  } နေစဉ် (I != E && Src[I] == '\\');

  bool FollowedByDoubleQuote = (I != E && Src[I] == '"');
  အကယ်၍ (FollowedByDoubleQuote) {
    Token.append(BackslashCount / 2, '\\');
    အကယ်၍ (BackslashCount % 2 == 0)
      ပြန်ပေး I - 1;
    Token.push_back('"');
    ပြန်ပေး I;
  }
  Token.append(BackslashCount, '\\');
  ပြန်ပေး I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
static bool isWindowsSpecialChar(var_na C) {
  ပြန်ပေး isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
static bool isWindowsSpecialCharInCommandName(var_na C) {
  ပြန်ပေး isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
static anta_hstha void tokenizeWindowsCommandLineImpl(
    StringRef Src, StringSaver &Saver, function_ref<void(StringRef)> AddToken,
    bool AlwaysCopy, function_ref<void()> MarkEOL, bool InitialCommandName) {
  SmallString<128> Token;

  // Sometimes, this function will be handling a full command line including an
  // executable pathname at the start. In that situation, the initial pathname
  // needs different handling from the following arguments, because when
  // CreateProcess or cmd.exe scans the pathname, it doesn't treat \ as
  // escaping the quote character, whereas when libc scans the rest of the
  // command line, it does.
  bool CommandName = InitialCommandName;

  // Try to do as much work inside the state machine as possible.
  ga_nanaa { INIT, UNQUOTED, QUOTED } State = INIT;

  အတွက် (size_t I = 0, E = Src.size(); I < E; ++I) {
    switch (State) {
    pak_sa INIT: {
      assert(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      နေစဉ် (I < E && isWhitespaceOrNull(Src[I])) {
        အကယ်၍ (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      အကယ်၍ (I >= E)
        bha_nga;
      size_t Start = I;
      အကယ်၍ (CommandName) {
        နေစဉ် (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } မဟုတ်ပါက {
        နေစဉ် (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      အကယ်၍ (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        အကယ်၍ (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } မဟုတ်ပါက {
          CommandName = false;
        }
      } မဟုတ်ပါက အကယ်၍ (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } မဟုတ်ပါက အကယ်၍ (Src[I] == '\\') {
        assert(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } မဟုတ်ပါက {
        llvm_unreachable("unexpected special character");
      }
      bha_nga;
    }

    pak_sa UNQUOTED:
      အကယ်၍ (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        အကယ်၍ (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } မဟုတ်ပါက {
          CommandName = false;
        }
        State = INIT;
      } မဟုတ်ပါက အကယ်၍ (Src[I] == '\"') {
        State = QUOTED;
      } မဟုတ်ပါက အကယ်၍ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } မဟုတ်ပါက {
        Token.push_back(Src[I]);
      }
      bha_nga;

    pak_sa QUOTED:
      အကယ်၍ (Src[I] == '\"') {
        အကယ်၍ (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.push_back('"');
          ++I;
        } မဟုတ်ပါက {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } မဟုတ်ပါက အကယ်၍ (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } မဟုတ်ပါက {
        Token.push_back(Src[I]);
      }
      bha_nga;
    }
  }

  အကယ်၍ (State != INIT)
    AddToken(Saver.save(Token.str()));
}

void cl::TokenizeWindowsCommandLine(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<const var_na *> &NewArgv,
                                    bool MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    အကယ်၍ (MarkEOLs)
      NewArgv.push_back(nullptr);
  };
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, false);
}

void cl::TokenizeWindowsCommandLineNoCopy(StringRef Src, StringSaver &Saver,
                                          SmallVectorImpl<StringRef> &NewArgv) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok); };
  auto OnEOL = []() {};
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken, /*AlwaysCopy=*/false,
                                 OnEOL, false);
}

void cl::TokenizeWindowsCommandLineFull(StringRef Src, StringSaver &Saver,
                                        SmallVectorImpl<const var_na *> &NewArgv,
                                        bool MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    အကယ်၍ (MarkEOLs)
      NewArgv.push_back(nullptr);
  };
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, true);
}

void cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<const var_na *> &NewArgv,
                            bool MarkEOLs) {
  အတွက် (const var_na *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> Line;
    // Check for comment line.
    အကယ်၍ (isWhitespace(*Cur)) {
      နေစဉ် (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      continue;
    }
    အကယ်၍ (*Cur == '#') {
      နေစဉ် (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      continue;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    const var_na *Start = Cur;
    const var_na *LineEnd = nullptr;
    var_na Quote = 0;
    bool AtTokenStart = true;
    အတွက် (const var_na *End = Source.end(); Cur != End; ++Cur) {
      var_na C = *Cur;
      အကယ်၍ (C == '\\') {
        အကယ်၍ (Cur + 1 != End) {
          ++Cur;
          အကယ်၍ (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            Line.append(Start, Cur - 1);
            အကယ်၍ (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            continue;
          }
        }
        AtTokenStart = false;
        continue;
      }
      အကယ်၍ (C == '\n')
        bha_nga;
      အကယ်၍ (Quote) {
        အကယ်၍ (C == Quote)
          Quote = 0;
      } မဟုတ်ပါက အကယ်၍ (isQuote(C)) {
        Quote = C;
      } မဟုတ်ပါက အကယ်၍ (C == '#' && AtTokenStart) {
        LineEnd = Cur;
        နေစဉ် (Cur != End && *Cur != '\n')
          ++Cur;
        bha_nga;
      }
      AtTokenStart = isWhitespace(C);
    }
    // Tokenize line.
    Line.append(Start, LineEnd ? LineEnd : Cur);
    cl::TokenizeGNUCommandLine(Line, Saver, NewArgv, MarkEOLs);
  }
}

// It is called byte order marker but the UTF-8 BOM is actually not affected
// by the host system's endianness.
static bool hasUTF8ByteOrderMark(ArrayRef<var_na> S) {
  ပြန်ပေး (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
static void ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            const var_na *&Arg) {
  assert(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  const StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  အတွက် (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    const StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    အကယ်၍ (ResponseFile.empty())
      ResponseFile = LHS;
    မဟုတ်ပါက
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  အကယ်၍ (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    const StringRef Remaining = ArgString.substr(StartPos);
    အကယ်၍ (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<const var_na *> &NewArgv) {
  assert(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  အကယ်၍ (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    ပြန်ပေး llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<var_na> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  အကယ်၍ (hasUTF16ByteOrderMark(BufRef)) {
    အကယ်၍ (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      ပြန်ပေး llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  မဟုတ်ပါက အကယ်၍ (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  အကယ်၍ (!RelativeNames && !InConfigFile)
    ပြန်ပေး Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  အတွက် (const var_na *&Arg : NewArgv) {
    အကယ်၍ (!Arg)
      continue;

    // Substitute <CFGDIR> with the file's base path.
    အကယ်၍ (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    bool ConfigInclusion = false;
    အကယ်၍ (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      အကယ်၍ (!llvm::sys::path::is_relative(FileName))
        continue;
    } မဟုတ်ပါက အကယ်၍ (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = true;
    } မဟုတ်ပါက {
      continue;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.push_back('@');
    အကယ်၍ (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      အကယ်၍ (!findConfigFile(FileName, FilePath))
        ပြန်ပေး createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } မဟုတ်ပါက {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  ပြန်ပေး Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<const var_na *> &Argv) {
  sa_nracanaa ResponseFileRecord {
    std::string File;
    size_t End;
  };

  // To detect recursive response files, we maintain a stack of files and the
  // position of the last argument in the file. This position is updated
  // dynamically as we recursively expand files.
  SmallVector<ResponseFileRecord, 3> FileStack;

  // Push a dummy entry that represents the initial command line, removing
  // the need to check for an empty list.
  FileStack.push_back({"", Argv.size()});

  // Don't cache Argv.size() because it can change.
  အတွက် (unsigned I = 0; I != Argv.size();) {
    နေစဉ် (I == FileStack.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      FileStack.pop_back();
    }

    const var_na *Arg = Argv[I];
    // Check if it is an EOL marker
    အကယ်၍ (Arg == nullptr) {
      ++I;
      continue;
    }

    အကယ်၍ (Arg[0] != '@') {
      ++I;
      continue;
    }

    const var_na *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    အကယ်၍ (llvm::sys::path::is_relative(FName)) {
      အကယ်၍ (CurrentDir.empty()) {
        အကယ်၍ (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } မဟုတ်ပါက {
          ပြန်ပေး createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } မဟုတ်ပါက {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    အကယ်၍ (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      အကယ်၍ (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        အကယ်၍ (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          continue;
        }
      }
      အကယ်၍ (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      ပြန်ပေး createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    const llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, this](const ResponseFileRecord &RFile) -> ErrorOr<bool> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      အကယ်၍ (!RHS)
        ပြန်ပေး RHS.getError();
      ပြန်ပေး FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    အတွက် (const auto &F : drop_begin(FileStack)) {
      အကယ်၍ (ErrorOr<bool> R = IsEquivalent(F)) {
        အကယ်၍ (R.get())
          ပြန်ပေး createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } မဟုတ်ပါက {
        ပြန်ပေး createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<const var_na *, 0> ExpandedArgv;
    အကယ်၍ (Error Err = expandResponseFile(FName, ExpandedArgv))
      ပြန်ပေး Err;

    အတွက် (ResponseFileRecord &Record : FileStack) {
      // Increase the end of all active records by the number of newly expanded
      // arguments, minus the response file itself.
      Record.End += ExpandedArgv.size() - 1;
    }

    FileStack.push_back({FName, I + ExpandedArgv.size()});
    Argv.erase(Argv.begin() + I);
    Argv.insert(Argv.begin() + I, ExpandedArgv.begin(), ExpandedArgv.end());
  }

  // If successful, the top of the file stack will mark the end of the Argv
  // stream. A failure here indicates a bug in the stack popping logic above.
  // Note that FileStack may have more than one element at this point because we
  // don't have a chance to pop the stack when encountering recursive files at
  // the end of the stream, so seeing that doesn't indicate a bug.
  assert(FileStack.size() > 0 && Argv.size() == FileStack.back().End);
  ပြန်ပေး Error::success();
}

bool cl::expandResponseFiles(ကိန်းပြည့် Argc, const var_na *const *Argv,
                             const var_na *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<const var_na *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#မဟုတ်ပါက
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  // The environment variable specifies initial options.
  အကယ်၍ (EnvVar)
    အကယ်၍ (std::optional<std::string> EnvValue = sys::Process::GetEnv(EnvVar))
      Tokenize(*EnvValue, Saver, NewArgv, /*MarkEOLs=*/false);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  အကယ်၍ (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    ပြန်ပေး false;
  }
  ပြန်ပေး true;
}

bool cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<const var_na *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  အကယ်၍ (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    ပြန်ပေး false;
  }
  ပြန်ပေး true;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

bool ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<var_na> &FilePath) {
  SmallString<128> CfgFilePath;
  const auto FileExists = [this](SmallString<128> Path) -> bool {
    auto Status = FS->status(Path);
    ပြန်ပေး Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  အကယ်၍ (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    အကယ်၍ (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      ပြန်ပေး false;
    အကယ်၍ (!FileExists(CfgFilePath))
      ပြန်ပေး false;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    ပြန်ပေး true;
  }

  // Look for the file in search directories.
  အတွက် (const StringRef &Dir : SearchDirs) {
    အကယ်၍ (Dir.empty())
      continue;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    အကယ်၍ (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      ပြန်ပေး true;
    }
  }

  ပြန်ပေး false;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<const var_na *> &Argv) {
  SmallString<128> AbsPath;
  အကယ်၍ (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    အကယ်၍ (std::error_code EC = FS->makeAbsolute(AbsPath))
      ပြန်ပေး make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = true;
  RelativeNames = true;
  အကယ်၍ (Error Err = expandResponseFile(CfgFile, Argv))
    ပြန်ပေး Err;
  ပြန်ပေး expandResponseFiles(Argv);
}

static void initCommonOptions();
static bool parseCommandLineOptionsImpl(LibraryOptionsContext *Context,
                                        ArrayRef<const var_na *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        const var_na *EnvVar) {
  initCommonOptions();
  SmallVector<const var_na *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.push_back(Args[0]);

  // Parse options from environment variable.
  အကယ်၍ (EnvVar) {
    အကယ်၍ (std::optional<std::string> EnvValue =
            sys::Process::GetEnv(StringRef(EnvVar)))
      TokenizeGNUCommandLine(*EnvValue, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  ကိန်းပြည့် NewArgc = static_cast<ကိန်းပြည့်>(NewArgv.size());

  // Parse all options.
  ပြန်ပေး globalParser().ParseCommandLineOptions(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

bool cl::ParseCommandLineOptions(ကိန်းပြည့် argc, const var_na *const *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, const var_na *EnvVar) {
  ပြန်ပေး parseCommandLineOptionsImpl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

bool cl::parseCommandLineOptions(LibraryOptionsContext &Context,
                                 ArrayRef<const var_na *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, const var_na *EnvVar) {
  ပြန်ပေး parseCommandLineOptionsImpl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
void CommandLineParser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  အတွက် (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    အတွက် (auto &O : SC->OptionsMap)
      Opts.push_back(O.second);
    အတွက် (Option *O : Opts)
      O->reset();
    အတွက် (Option *O : SC->PositionalOpts)
      O->reset();
    အကယ်၍ (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  အတွက် (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

bool CommandLineParser::ParseCommandLineOptions(
    ကိန်းပြည့် argc, const var_na *const *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  assert(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  bool IgnoreErrors = Errs;
  အကယ်၍ (!Errs)
    Errs = &errs();
  အကယ်၍ (!VFS)
    VFS = vfs::getRealFileSystem().get();
  bool ErrorParsing = false;

  // Expand response files.
  SmallVector<const var_na *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#မဟုတ်ပါက
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  အကယ်၍ (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    ပြန်ပေး false;
  }
  argv = &newArgv[0];
  argc = static_cast<ကိန်းပြည့်>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned NumPositionalRequired = 0;

  // Determine whether or not there are an unlimited number of positionals
  bool HasUnlimitedPositionals = false;

  ကိန်းပြည့် FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::getTopLevel();
  std::string NearestSubCommandString;
  bool MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  အကယ်၍ (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    အကယ်၍ (ChosenSubCommand != &SubCommand::getTopLevel())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  assert(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  အကယ်၍ (ConsumeAfterOpt) {
    assert(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  အကယ်၍ (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    bool UnboundedFound = false;
    အတွက် (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      အကယ်၍ (RequiresValue(Opt))
        ++NumPositionalRequired;
      မဟုတ်ပါက အကယ်၍ (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        အကယ်၍ (PositionalOpts.size() > 1) {
          အကယ်၍ (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = true;
        }
      } မဟုတ်ပါက အကယ်၍ (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        အကယ်၍ (!IgnoreErrors)
          Opt->error("error - option can never match, because "
                     "another positional argument will match an "
                     "unbounded number of values, and this option"
                     " does not require a value!");
        *Errs << ProgramName << ": CommandLine Error: Option '" << Opt->ArgStr
              << "' is all messed up!\n";
        *Errs << PositionalOpts.size();
        ErrorParsing = true;
      }
      UnboundedFound |= EatsUnboundedNumberOfValues(Opt);
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
  bool DashDashFound = false; // Have we read '--'?
  အတွက် (ကိန်းပြည့် i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef Value;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    အကယ်၍ (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      အကယ်၍ (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        continue; // We are done!
      }

      အကယ်၍ (!PositionalOpts.empty()) {
        PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        အကယ်၍ (PositionalVals.size() >= NumPositionalRequired && ConsumeAfterOpt) {
          အတွက် (++i; i < argc; ++i)
            PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));
          bha_nga; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        continue;
      }
    } မဟုတ်ပါက အကယ်၍ (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = true; // This is the mythical "--"?
      continue;             // Don't try to process it as an argument itself.
    } မဟုတ်ပါက အကယ်၍ (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);
      အကယ်၍ (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        continue; // We are done!
      }
    } မဟုတ်ပါက { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      အကယ်၍ (!Handler && ChosenSubCommand != &SubCommand::getTopLevel())
        Handler = LookupOption(SubCommand::getTopLevel(), ArgName, Value);

      အကယ်၍ (!Handler) {
        အကယ်၍ (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<const var_na *> Rest(argv + i, argc - i);
          unsigned N;
          အကယ်၍ (Error E = Context && L->isPerContext()
                            ? Context->parse(*L, Rest, N)
                            : parseLibraryOption(*L, L->global(), Rest, N,
                                                 LibraryArgAlloc)) {
            *Errs << ProgramName << ": " << toString(std::move(E)) << '\n';
            ErrorParsing = true;
          }
          i += N - 1;
          continue;
        }
      }

      // Check to see if this "option" is really a prefixed argument.
      အကယ်၍ (!Handler)
        Handler = HandlePrefixedOption(ArgName, Value, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      အကယ်၍ (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    အကယ်၍ (!Handler) {
      auto ReportUnknownArgument = [&](bool IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        အကယ်၍ (NearestArgumentName.empty())
          ပြန်ပေး;

        *Errs << ProgramName << ": Did you mean '";
        အကယ်၍ (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        မဟုတ်ပါက
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      အကယ်၍ (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/true, NearestHandlerString);
      မဟုတ်ပါက
        ReportUnknownArgument(/*IsArg=*/false, NearestSubCommandString);

      ErrorParsing = true;
      continue;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    အကယ်၍ (Handler->getFormattingFlag() == cl::Positional) {
      အကယ်၍ ((Handler->getMiscFlags() & PositionalEatsArgs) && !Value.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = true;
      }
      ActivePositionalArg = Handler;
    }
    မဟုတ်ပါက
      ErrorParsing |= ProvideOption(Handler, ArgName, Value, argc, argv, i);
  }

  // Check and handle positional arguments now...
  အကယ်၍ (NumPositionalRequired > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << NumPositionalRequired
             << " positional argument" << (NumPositionalRequired > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = true;
  } မဟုတ်ပါက အကယ်၍ (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = true;

  } မဟုတ်ပါက အကယ်၍ (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, NumVals = static_cast<unsigned>(PositionalVals.size());
    အတွက် (Option *Opt : PositionalOpts) {
      အကယ်၍ (RequiresValue(Opt)) {
        ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                PositionalVals[ValNo].second);
        ValNo++;
        --NumPositionalRequired; // We fulfilled our duty...
      }

      // If we _can_ give this option more arguments, do so now, as long as we
      // do not give it values that others need.  'Done' controls whether the
      // option even _WANTS_ any more.
      //
      bool Done = Opt->getNumOccurrencesFlag() == cl::Required;
      နေစဉ် (NumVals - ValNo > NumPositionalRequired && !Done) {
        switch (Opt->getNumOccurrencesFlag()) {
        pak_sa cl::Optional:
          Done = true; // Optional arguments want _at most_ one value
          [[fallthrough]];
        pak_sa cl::ZeroOrMore: // Zero or more will take all they can get...
        pak_sa cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          bha_nga;
        default:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } မဟုတ်ပါက {
    assert(ConsumeAfterOpt && NumPositionalRequired <= PositionalVals.size());
    unsigned ValNo = 0;
    အတွက် (Option *Opt : PositionalOpts)
      အကယ်၍ (RequiresValue(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    အကယ်၍ (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    အတွက် (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  အတွက် (const auto &Opt : OptionsMap) {
    switch (Opt.second->getNumOccurrencesFlag()) {
    pak_sa Required:
    pak_sa OneOrMore:
      အကယ်၍ (Opt.second->getNumOccurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = true;
      }
      [[fallthrough]];
    default:
      bha_nga;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             အတွက် (ကိန်းပြည့် i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  အကယ်၍ (ErrorParsing) {
    အကယ်၍ (!IgnoreErrors)
      exit(1);
    ပြန်ပေး false;
  }
  ပြန်ပေး true;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

bool Option::error(const Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  အကယ်၍ (!ArgName.data())
    ArgName = ArgStr;
  အကယ်၍ (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  မဟုတ်ပါက
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  ပြန်ပေး true;
}

bool Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef Value) {
  ++NumOccurrences;
  ပြန်ပေး handleOccurrence(pos, ArgName, Value);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
static StringRef getValueStr(const Option &O, StringRef DefaultMsg) {
  အကယ်၍ (O.ValueStr.empty())
    ပြန်ပေး DefaultMsg;
  ပြန်ပေး O.ValueStr;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() const {
  ပြန်ပေး argPlusPrefixesSize(ArgStr);
}

void Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t FirstLineIndentedBy) {
  assert(Indent >= FirstLineIndentedBy);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - FirstLineIndentedBy)
      << ArgHelpPrefix << Split.first << "\n";
  နေစဉ် (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(Indent) << Split.first << "\n";
  }
}

void Option::printEnumValHelpStr(StringRef HelpStr, size_t BaseIndent,
                                 size_t FirstLineIndentedBy) {
  const StringRef ValHelpPrefix = "  ";
  assert(BaseIndent >= FirstLineIndentedBy);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(BaseIndent - FirstLineIndentedBy)
      << ArgHelpPrefix << ValHelpPrefix << Split.first << "\n";
  နေစဉ် (!Split.second.empty()) {
    Split = Split.second.split('\n');
    outs().indent(BaseIndent + ValHelpPrefix.size()) << Split.first << "\n";
  }
}

// Print out the option for the alias.
void alias::printOptionInfo(size_t GlobalWidth) const {
  outs() << PrintArg(ArgStr);
  printHelpStr(HelpStr, GlobalWidth, argPlusPrefixesSize(ArgStr));
}

//===----------------------------------------------------------------------===//
// Parser Implementation code...
//

// basic_parser implementation
//

// Return the width of the option tag for printing...
size_t basic_parser_impl::getOptionWidth(const Option &O) const {
  size_t Len = argPlusPrefixesSize(O.ArgStr);
  auto ValName = getValueName();
  အကယ်၍ (!ValName.empty()) {
    size_t FormattingLen = 3;
    အကယ်၍ (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += getValueStr(O, ValName).size() + FormattingLen;
  }

  ပြန်ပေး Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
void basic_parser_impl::printOptionInfo(const Option &O,
                                        size_t GlobalWidth) const {
  outs() << PrintArg(O.ArgStr);

  auto ValName = getValueName();
  အကယ်၍ (!ValName.empty()) {
    အကယ်၍ (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << getValueStr(O, ValName) << ">...";
    } မဟုတ်ပါက အကယ်၍ (O.getValueExpectedFlag() == ValueOptional)
      outs() << "[=<" << getValueStr(O, ValName) << ">]";
    မဟုတ်ပါက {
      outs() << (O.ArgStr.size() == 1 ? " <" : "=<") << getValueStr(O, ValName)
             << '>';
    }
  }

  Option::printHelpStr(O.HelpStr, GlobalWidth, getOptionWidth(O));
}

void basic_parser_impl::printOptionName(const Option &O,
                                        size_t GlobalWidth) const {
  outs() << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());
}

// parser<bool> implementation
//
bool parser<bool>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         bool &Value) {
  ပြန်ပေး parseBool<bool, true, false>(O, ArgName, Arg, Value);
}

// parser<boolOrDefault> implementation
//
bool parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &Value) {
  ပြန်ပေး parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
                   boolOrDefault::BOU_FALSE>(O, ArgName, Arg, Value);
}

// parser<FixedOrScalableQuantity> implementation
//
template <typename FixedOrScalableQuantityT>
static bool parseFixedOrScalableQuantity(Option &O, StringRef Arg,
                                         StringRef ValueKind,
                                         FixedOrScalableQuantityT &Value) {
  using ScalarTy = typename FixedOrScalableQuantityT::ScalarTy;

  Arg = Arg.trim();

  ScalarTy MinValue;
  အကယ်၍ (!Arg.getAsInteger(0, MinValue)) {
    Value = FixedOrScalableQuantityT::getFixed(MinValue);
    ပြန်ပေး false;
  }

  StringRef Remainder = Arg;
  အကယ်၍ (!Remainder.consume_front("vscale"))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  အကယ်၍ (!Remainder.consume_front('x'))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  အကယ်၍ (Remainder.getAsInteger(0, MinValue))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Value = FixedOrScalableQuantityT::getScalable(MinValue);
  ပြန်ပေး false;
}

// parser<int> implementation
//
bool parser<ကိန်းပြည့်>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        ကိန်းပြည့် &Value) {
  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for integer argument!");
  ပြန်ပေး false;
}

// parser<long> implementation
//
bool parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &Value) {
  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for long argument!");
  ပြန်ပေး false;
}

// parser<long long> implementation
//
bool parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &Value) {
  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for llong argument!");
  ပြန်ပေး false;
}

// parser<unsigned> implementation
//
bool parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &Value) {

  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for uint argument!");
  ပြန်ပေး false;
}

// parser<unsigned long> implementation
//
bool parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &Value) {

  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for ulong argument!");
  ပြန်ပေး false;
}

// parser<unsigned long long> implementation
//
bool parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &Value) {

  အကယ်၍ (Arg.getAsInteger(0, Value))
    ပြန်ပေး O.error("'" + Arg + "' value invalid for ullong argument!");
  ပြန်ပေး false;
}

// parser<ElementCount> implementation
//
bool parser<ElementCount>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 ElementCount &Value) {
  ပြန်ပေး parseFixedOrScalableQuantity(O, Arg, getValueName(), Value);
}

// parser<double>/parser<float> implementation
//
static bool parseDouble(Option &O, StringRef Arg, dviguu_na &Value) {
  အကယ်၍ (to_float(Arg, Value))
    ပြန်ပေး false;
  ပြန်ပေး O.error("'" + Arg + "' value invalid for floating point argument!");
}

bool parser<dviguu_na>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           dviguu_na &Val) {
  ပြန်ပေး parseDouble(O, Arg, Val);
}

bool parser<float>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          float &Val) {
  dviguu_na dVal;
  အကယ်၍ (parseDouble(O, Arg, dVal))
    ပြန်ပေး true;
  Val = (float)dVal;
  ပြန်ပေး false;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = getNumOptions();

  အတွက် (unsigned i = 0; i != e; ++i) {
    အကယ်၍ (getOption(i) == Name)
      ပြန်ပေး i;
  }
  ပြန်ပေး e;
}

static StringRef EqValue = "=<value>";
static StringRef EmptyOption = "<empty>";
static StringRef OptionPrefix = "    =";
static size_t getOptionPrefixesSize() {
  ပြန်ပေး OptionPrefix.size() + ArgHelpPrefix.size();
}

static bool shouldPrintOption(StringRef Name, StringRef Description,
                              const Option &O) {
  ပြန်ပေး O.getValueExpectedFlag() != ValueOptional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(const Option &O) const {
  အကယ်၍ (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + EqValue.size();
    အတွက် (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef Name = getOption(i);
      အကယ်၍ (!shouldPrintOption(Name, getDescription(i), O))
        continue;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    ပြန်ပေး Size;
  } မဟုတ်ပါက {
    size_t BaseSize = 0;
    အတွက် (unsigned i = 0, e = getNumOptions(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    ပြန်ပေး BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
void generic_parser_base::printOptionInfo(const Option &O,
                                          size_t GlobalWidth) const {
  အကယ်၍ (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    အကယ်၍ (O.getValueExpectedFlag() == ValueOptional) {
      အတွက် (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
        အကယ်၍ (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          bha_nga;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << EqValue;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         EqValue.size() +
                             argPlusPrefixesSize(O.ArgStr));
    အတွက် (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      အကယ်၍ (!shouldPrintOption(OptionName, Description, O))
        continue;
      size_t FirstLineIndent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      အကယ်၍ (OptionName.empty()) {
        outs() << EmptyOption;
        assert(FirstLineIndent >= EmptyOption.size());
        FirstLineIndent += EmptyOption.size();
      }
      အကယ်၍ (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, FirstLineIndent);
      မဟုတ်ပါက
        outs() << '\n';
    }
  } မဟုတ်ပါက {
    အကယ်၍ (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    အတွက် (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef Option = getOption(i);
      outs() << "    " << PrintArg(Option);
      Option::printHelpStr(getDescription(i), GlobalWidth, Option.size() + 8);
    }
  }
}

static const size_t MaxOptWidth = 8; // arbitrary spacing for printOptionDiff

// printGenericOptionDiff - Print the value of this option and it's default.
//
// "Generic" options have each value mapped to a name.
void generic_parser_base::printGenericOptionDiff(
    const Option &O, const GenericOptionValue &Value,
    const GenericOptionValue &Default, size_t GlobalWidth) const {
  outs() << "  " << PrintArg(O.ArgStr);
  outs().indent(GlobalWidth - O.ArgStr.size());

  unsigned NumOpts = getNumOptions();
  အတွက် (unsigned i = 0; i != NumOpts; ++i) {
    အကယ်၍ (!Value.compare(getOptionValue(i)))
      continue;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t NumSpaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(NumSpaces) << " (default: ";
    အတွက် (unsigned j = 0; j != NumOpts; ++j) {
      အကယ်၍ (!Default.compare(getOptionValue(j)))
        continue;
      outs() << getOption(j);
      bha_nga;
    }
    outs() << ")\n";
    ပြန်ပေး;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
namespace llvm {
namespace cl {
static raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  ပြန်ပေး OS << static_cast<ကိန်းပြည့်>(V);
}
} // namespace cl
} // namespace llvm

#define PRINT_OPT_DIFF(T)                                                      \
  void parser<T>::printOptionDiff(const Option &O, T V, OptionValue<T> D,      \
                                  size_t GlobalWidth) const {                  \
    printOptionName(O, GlobalWidth);                                           \
    std::string Str;                                                           \
    {                                                                          \
      raw_string_ostream SS(Str);                                              \
      SS << V;                                                                 \
    }                                                                          \
    outs() << "= " << Str;                                                     \
    size_t NumSpaces =                                                         \
        MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;               \
    outs().indent(NumSpaces) << " (default: ";                                 \
    အကယ်၍ (D.hasValue())                                                          \
      outs() << D.getValue();                                                  \
    မဟုတ်ပါက                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(bool)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(ကိန်းပြည့်)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(dviguu_na)
PRINT_OPT_DIFF(float)
PRINT_OPT_DIFF(var_na)
PRINT_OPT_DIFF(ElementCount)

void parser<std::string>::printOptionDiff(const Option &O, StringRef V,
                                          const OptionValue<std::string> &D,
                                          size_t GlobalWidth) const {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t NumSpaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(NumSpaces) << " (default: ";
  အကယ်၍ (D.hasValue())
    outs() << D.getValue();
  မဟုတ်ပါက
    outs() << "*no default*";
  outs() << ")\n";
}

void parser<std::optional<std::string>>::printOptionDiff(
    const Option &O, std::optional<StringRef> V,
    const OptionValue<std::optional<std::string>> &D,
    size_t GlobalWidth) const {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t VSize = V.has_value() ? V.value().size() : 0;
  size_t NumSpaces = MaxOptWidth > VSize ? MaxOptWidth - VSize : 0;
  outs().indent(NumSpaces) << " (default: ";
  အကယ်၍ (D.hasValue() && D.getValue().has_value())
    outs() << D.getValue();
  မဟုတ်ပါက
    outs() << "*no value*";
  outs() << ")\n";
}

// Print a placeholder for options that don't yet support printOptionDiff().
void basic_parser_impl::printOptionNoValue(const Option &O,
                                           size_t GlobalWidth) const {
  printOptionName(O, GlobalWidth);
  outs() << "= *cannot print option value*\n";
}

//===----------------------------------------------------------------------===//
// -help and -help-hidden option implementation
//

static ကိန်းပြည့် OptNameCompare(const std::pair<const var_na *, Option *> *LHS,
                          const std::pair<const var_na *, Option *> *RHS) {
  ပြန်ပေး strcmp(LHS->first, RHS->first);
}

static ကိန်းပြည့် SubNameCompare(const std::pair<const var_na *, SubCommand *> *LHS,
                          const std::pair<const var_na *, SubCommand *> *RHS) {
  ပြန်ပေး strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
static void sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<const var_na *, Option *>> &Opts,
                     bool ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  အတွက် (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    အကယ်၍ (I->second->getOptionHiddenFlag() == ReallyHidden)
      continue;

    // Unless showhidden is set, ignore hidden flags.
    အကယ်၍ (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      continue;

    // If we've already seen this option, don't add it to the list again.
    အကယ်၍ (!OptionSet.insert(I->second).second)
      continue;

    Opts.push_back(
        std::pair<const var_na *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

static void
sortSubCommands(const SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<const var_na *, SubCommand *>> &Subs) {
  အတွက် (auto *S : SubMap) {
    အကယ်၍ (S->getName().empty())
      continue;
    Subs.push_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

namespace {

class HelpPrinter {
rak_sita:
  const bool ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<const var_na *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<const var_na *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  virtual void printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    အတွက် (const auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  void printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    အတွက် (const auto &S : Subs) {
      outs() << "  " << S.first;
      အကယ်၍ (!S.second->getDescription().empty()) {
        outs().indent(MaxSubLen - strlen(S.first));
        outs() << " - " << S.second->getDescription();
      }
      outs() << "\n";
    }
  }

public:
  explicit HelpPrinter(bool showHidden) : ShowHidden(showHidden) {}
  virtual ~HelpPrinter() = default;

  // Invoke the printer.
  void operator=(bool Value) {
    အကယ်၍ (!Value)
      ပြန်ပေး;
    printHelp();

    // Halt the program since help information was printed
    exit(0);
  }

  void printHelp() {
    SubCommand *Sub = globalParser().getActiveSubCommand();
    auto &OptionsMap = Sub->OptionsMap;
    auto &PositionalOpts = Sub->PositionalOpts;
    auto &ConsumeAfterOpt = Sub->ConsumeAfterOpt;

    StrOptionPairVector Opts;
    sortOpts(OptionsMap, Opts, ShowHidden);

    StrSubCommandPairVector Subs;
    sortSubCommands(globalParser().RegisteredSubCommands, Subs);

    အကယ်၍ (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    အကယ်၍ (Sub == &SubCommand::getTopLevel()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      အကယ်၍ (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } မဟုတ်ပါက {
      အကယ်၍ (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    အတွက် (auto *Opt : PositionalOpts) {
      အကယ်၍ (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    အကယ်၍ (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    အကယ်၍ (Sub == &SubCommand::getTopLevel() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      အတွက် (const auto &Sub : Subs)
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
    အတွက် (const auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    အကယ်၍ (ShowHidden)
      အတွက် (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              အကယ်၍ (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    အတွက် (const auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    အကယ်၍ (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    အတွက် (const auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    အတွက် (const auto &I : globalParser().MoreHelp)
      outs() << I;
    globalParser().MoreHelp.clear();
  }
};

class CategorizedHelpPrinter : public HelpPrinter {
public:
  explicit CategorizedHelpPrinter(bool showHidden) : HelpPrinter(showHidden) {}

  // Helper function for printOptions().
  // It shall return a negative value if A's name should be lexicographically
  // ordered before B's name. It returns a value greater than zero if B's name
  // should be ordered before A's name, and it returns 0 otherwise.
  static ကိန်းပြည့် OptionCategoryCompare(OptionCategory *const *A,
                                   OptionCategory *const *B) {
    ပြန်ပေး (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

rak_sita:
  void printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) override {
    std::vector<OptionCategory *> SortedCategories;
    DenseMap<OptionCategory *, std::vector<Option *>> CategorizedOptions;

    // Collect registered option categories into vector in preparation for
    // sorting.
    llvm::append_range(SortedCategories,
                       globalParser().RegisteredOptionCategories);

    // Sort the different option categories alphabetically.
    assert(SortedCategories.size() > 0 && "No option categories registered!");
    array_pod_sort(SortedCategories.begin(), SortedCategories.end(),
                   OptionCategoryCompare);

    // Walk through pre-sorted options and assign into categories.
    // Because the options are already alphabetically sorted the
    // options within categories will also be alphabetically sorted.
    အတွက် (const auto &I : Opts) {
      Option *Opt = I.second;
      အတွက် (OptionCategory *Cat : Opt->Categories) {
        assert(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].push_back(Opt);
      }
    }

    // Now do printing.
    အတွက် (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      const auto &CategoryOptions = CategorizedOptions[Category];
      အကယ်၍ (CategoryOptions.empty())
        continue;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      အကယ်၍ (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      မဟုတ်ပါက
        outs() << "\n";

      // Loop over the options in the category and print.
      အတွက် (const Option *Opt : CategoryOptions)
        Opt->printOptionInfo(MaxArgLen);
    }
  }
};

// This wraps the Uncategorizing and Categorizing printers and decides
// at run time which should be invoked.
class HelpPrinterWrapper {
private:
  HelpPrinter &UncategorizedPrinter;
  CategorizedHelpPrinter &CategorizedPrinter;

public:
  explicit HelpPrinterWrapper(HelpPrinter &UncategorizedPrinter,
                              CategorizedHelpPrinter &CategorizedPrinter)
      : UncategorizedPrinter(UncategorizedPrinter),
        CategorizedPrinter(CategorizedPrinter) {}

  // Invoke the printer.
  void operator=(bool Value);
};

} // End anonymous namespace

#အကယ်၍ defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# အကယ်၍ defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# မဟုတ်ပါက
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#elif defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# အကယ်၍ defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# မဟုတ်ပါက
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#မဟုတ်ပါက
// Otherwise, for an unknown compiler, assume this is an optimized build.
# define LLVM_IS_DEBUG_BUILD 0
#endif

namespace {
class VersionPrinter {
public:
  void print(const std::vector<VersionPrinterTy> &ExtraPrinters) {
    raw_ostream &OS = outs();
#ifdef PACKAGE_VENDOR
    OS << PACKAGE_VENDOR << " ";
#မဟုတ်ပါက
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#အကယ်၍ LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#မဟုတ်ပါက
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    အကယ်၍ (!ExtraPrinters.empty()) {
      အတွက် (const auto &I : ExtraPrinters)
        I(outs());
    }
  }
  void operator=(bool OptionWasSpecified);
};

sa_nracanaa CommandLineCommonOptions {
  // Declare the four HelpPrinter instances that are used to print out help, or
  // help-hidden as an uncategorized list or in categories.
  HelpPrinter UncategorizedNormalPrinter{false};
  HelpPrinter UncategorizedHiddenPrinter{true};
  CategorizedHelpPrinter CategorizedNormalPrinter{false};
  CategorizedHelpPrinter CategorizedHiddenPrinter{true};
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
  cl::opt<HelpPrinter, true, parser<bool>> HLOp{
      "help-list",
      cl::desc(
          "Display list of available options (--help-list-hidden for more)"),
      cl::location(UncategorizedNormalPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<HelpPrinter, true, parser<bool>> HLHOp{
      "help-list-hidden",
      cl::desc("Display list of all available options"),
      cl::location(UncategorizedHiddenPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  // Define uncategorized/categorized help printers. These printers change their
  // behaviour at runtime depending on whether one or more Option categories
  // have been declared.
  cl::opt<HelpPrinterWrapper, true, parser<bool>> HOp{
      "help",
      cl::desc("Display available options (--help-hidden for more)"),
      cl::location(WrappedNormalPrinter),
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::alias HOpA{"h", cl::desc("Alias for --help"), cl::aliasopt(HOp)};

  cl::opt<HelpPrinterWrapper, true, parser<bool>> HHOp{
      "help-hidden",
      cl::desc("Display all available options"),
      cl::location(WrappedHiddenPrinter),
      cl::Hidden,
      cl::ValueDisallowed,
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<bool> PrintOptions{
      "print-options",
      cl::desc("Print non-default options after command line parsing"),
      cl::Hidden,
      cl::init(false),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  cl::opt<bool> PrintAllOptions{
      "print-all-options",
      cl::desc("Print all option values after command line parsing"),
      cl::Hidden,
      cl::init(false),
      cl::cat(GenericCategory),
      cl::sub(SubCommand::getAll())};

  VersionPrinterTy OverrideVersionPrinter = nullptr;

  std::vector<VersionPrinterTy> ExtraVersionPrinters;

  // Define the --version option that prints out the LLVM version for the tool
  VersionPrinter VersionPrinterInstance;

  cl::opt<VersionPrinter, true, parser<bool>> VersOp{
      "version", cl::desc("Display the version of this program"),
      cl::location(VersionPrinterInstance), cl::ValueDisallowed,
      cl::cat(GenericCategory)};
};
} // End anonymous namespace

// Lazy-initialized global instance of options controlling the command-line
// parser and general handling.
static ManagedStatic<CommandLineCommonOptions> CommonOptions;

static void initCommonOptions() {
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
  static OptionCategory GeneralCategory{"General options"};
  ပြန်ပေး GeneralCategory;
}

void VersionPrinter::operator=(bool OptionWasSpecified) {
  အကယ်၍ (!OptionWasSpecified)
    ပြန်ပေး;

  အကယ်၍ (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

void HelpPrinterWrapper::operator=(bool Value) {
  အကယ်၍ (!Value)
    ပြန်ပေး;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  အကယ်၍ (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = true; // Invoke categorized printer
  } မဟုတ်ပါက {
    UncategorizedPrinter = true; // Invoke uncategorized printer
  }
}

// Print the value of each option.
void cl::PrintOptionValues() { globalParser().printOptionValues(); }

void CommandLineParser::printOptionValues() {
  အကယ်၍ (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    ပြန်ပေး;

  SmallVector<std::pair<const var_na *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ true);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  အတွက် (const auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  အတွက် (const auto &Opt : Opts)
    Opt.second->printOptionValue(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
void cl::PrintHelpMessage(bool Hidden, bool Categorized) {
  အကယ်၍ (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  မဟုတ်ပါက အကယ်၍ (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  မဟုတ်ပါက အကယ်၍ (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  မဟုတ်ပါက
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  static const StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#အကယ်၍ LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#အကယ်၍ __has_feature(address_sanitizer)
      "+asan",
#endif
#အကယ်၍ __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#အကယ်၍ __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#အကယ်၍ __has_feature(memory_sanitizer)
      "+msan",
#endif
#အကယ်၍ __has_feature(thread_sanitizer)
      "+tsan",
#endif
#အကယ်၍ __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  ပြန်ပေး ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
void cl::printBuildConfig(raw_ostream &OS) {
#အကယ်၍ LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
  OS << "Build config: ";
  llvm::interleaveComma(cl::getCompilerBuildConfig(), OS);
  OS << '\n';
#endif
}

/// Utility function for printing version number.
void cl::PrintVersionMessage() {
  CommonOptions->VersionPrinterInstance.print(CommonOptions->ExtraVersionPrinters);
}

void cl::SetVersionPrinter(VersionPrinterTy func) {
  CommonOptions->OverrideVersionPrinter = func;
}

void cl::AddExtraVersionPrinter(VersionPrinterTy func) {
  CommonOptions->ExtraVersionPrinters.push_back(func);
}

OptionsMapTy &cl::getRegisteredOptions(SubCommand &Sub) {
  initCommonOptions();
  auto &Subs = globalParser().RegisteredSubCommands;
  (void)Subs;
  assert(Subs.contains(&Sub));
  ပြန်ပေး Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  ပြန်ပေး globalParser().getRegisteredSubcommands();
}

void cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  အတွက် (auto &I : Sub.OptionsMap) {
    bool Unrelated = true;
    အတွက် (auto &Cat : I.second->Categories) {
      အကယ်၍ (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    အကယ်၍ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

void cl::HideUnrelatedOptions(ArrayRef<const cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  အတွက် (auto &I : Sub.OptionsMap) {
    bool Unrelated = true;
    အတွက် (auto &Cat : I.second->Categories) {
      အကယ်၍ (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    အကယ်၍ (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

void cl::ResetCommandLineParser() { globalParser().reset(); }

void cl::addLibraryOptions(LibraryOptions &L) {
  globalParser().Libraries.push_back(&L);
}

void LibraryOptionsContext::assign(const LibraryOptionsContext &Other) {
  assert(this != &Other);
  Values.clear();
  SavedArgs.clear();
  ArrayRef<const var_na *> Args = Other.SavedArgs;
  အတွက် (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<const var_na *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = Values.try_emplace(L.getDefault());
  အကယ်၍ (Inserted)
    It->second = L.copyDefault(Alloc);
  ပြန်ပေး parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

void cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

void LLVMParseCommandLineOptions(ကိန်းပြည့် argc, const var_na *const *argv,
                                 const var_na *Overview) {
  llvm::cl::ParseCommandLineOptions(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
