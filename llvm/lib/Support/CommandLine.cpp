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
template class LLVM_EXPORT_TEMPLATE basic_parser<整数>;
template class LLVM_EXPORT_TEMPLATE basic_parser<long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<long long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<unsigned long long>;
template class LLVM_EXPORT_TEMPLATE basic_parser<double>;
template class LLVM_EXPORT_TEMPLATE basic_parser<float>;
template class LLVM_EXPORT_TEMPLATE basic_parser<std::string>;
template class LLVM_EXPORT_TEMPLATE basic_parser<char>;
template class LLVM_EXPORT_TEMPLATE basic_parser<ElementCount>;

#如果 !(defined(LLVM_ENABLE_LLVM_EXPORT_ANNOTATIONS) && defined(_MSC_VER))
// Only instantiate opt<std::string> when not building a Windows DLL. When
// exporting opt<std::string>, MSVC implicitly exports symbols for
// std::basic_string through transitive inheritance via std::string. These
// symbols may appear in clients, leading to duplicate symbol conflicts.
template class LLVM_EXPORT_TEMPLATE opt<std::string>;
#endif

template class LLVM_EXPORT_TEMPLATE opt<bool>;
template class LLVM_EXPORT_TEMPLATE opt<char>;
template class LLVM_EXPORT_TEMPLATE opt<整数>;
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
void parser<整数>::anchor() {}
void parser<long>::anchor() {}
void parser<long long>::anchor() {}
void parser<unsigned>::anchor() {}
void parser<unsigned long>::anchor() {}
void parser<unsigned long long>::anchor() {}
void parser<double>::anchor() {}
void parser<float>::anchor() {}
void parser<std::string>::anchor() {}
void parser<std::optional<std::string>>::anchor() {}
void parser<char>::anchor() {}
void parser<ElementCount>::anchor() {}

// These anchor functions instantiate opt<T> and reference its virtual
// destructor to ensure MSVC exports the corresponding vtable and typeinfo when
// building a Windows DLL. Without an explicit reference, MSVC may omit the
// instantiation at link time even if it is marked DLL-export.
void opt_bool_anchor() { opt<bool> anchor{""}; }
void opt_char_anchor() { opt<char> anchor{""}; }
void opt_int_anchor() { opt<整数> anchor{""}; }
void opt_unsigned_anchor() { opt<unsigned> anchor{""}; }

//===----------------------------------------------------------------------===//

const static size_t DefaultPad = 2;

static StringRef ArgPrefix = "-";
static StringRef ArgPrefixLong = "--";
static StringRef ArgHelpPrefix = " - ";

static size_t argPlusPrefixesSize(StringRef ArgName, size_t Pad = DefaultPad) {
  size_t Len = ArgName.size();
  如果 (Len == 1)
    返回 Len + Pad + ArgPrefix.size() + ArgHelpPrefix.size();
  返回 Len + Pad + ArgPrefixLong.size() + ArgHelpPrefix.size();
}

static SmallString<8> argPrefix(StringRef ArgName, size_t Pad = DefaultPad) {
  SmallString<8> Prefix;
  为 (size_t I = 0; I < Pad; ++I) {
    Prefix.push_back(' ');
  }
  Prefix.append(ArgName.size() > 1 ? ArgPrefixLong : ArgPrefix);
  返回 Prefix;
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
  返回 OS;
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

  bool ParseCommandLineOptions(整数 argc, const char *const *argv,
                               StringRef Overview, raw_ostream *Errs = nullptr,
                               vfs::FileSystem *VFS = nullptr,
                               LibraryOptionsContext *Context = nullptr);

  void forEachSubCommand(Option &Opt, function_ref<void(SubCommand &)> Action) {
    如果 (Opt.Subs.empty()) {
      Action(SubCommand::getTopLevel());
      返回;
    }
    如果 (Opt.Subs.size() == 1 && *Opt.Subs.begin() == &SubCommand::getAll()) {
      为 (auto *SC : RegisteredSubCommands)
        Action(*SC);
      Action(SubCommand::getAll());
      返回;
    }
    为 (auto *SC : Opt.Subs) {
      assert(SC != &SubCommand::getAll() &&
             "SubCommand::getAll() should not be used with other subcommands");
      Action(*SC);
    }
  }

  void addLiteralOption(Option &Opt, SubCommand *SC, StringRef Name) {
    如果 (Opt.hasArgStr())
      返回;
    如果 (!SC->OptionsMap.insert(std::make_pair(Name, &Opt)).second) {
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
    如果 (O->hasArgStr()) {
      // Add argument to the argument map!
      // An unregistered subcommand, such as MLIR's PassOptions, parses its
      // own arguments.
      如果 (!SC->OptionsMap.insert(std::make_pair(O->ArgStr, O)).second ||
          (RegisteredSubCommands.contains(SC) &&
           LibraryIndex.contains(O->ArgStr))) {
        errs() << ProgramName << ": CommandLine Error: Option '" << O->ArgStr
               << "' registered more than once!\n";
        HadErrors = true;
      }
    }

    // Remember information about positional options.
    如果 (O->getFormattingFlag() == cl::Positional)
      SC->PositionalOpts.push_back(O);
    否则 如果 (O->getNumOccurrencesFlag() == cl::ConsumeAfter) {
      如果 (SC->ConsumeAfterOpt) {
        O->error("Cannot specify more than one option with cl::ConsumeAfter!");
        HadErrors = true;
      }
      SC->ConsumeAfterOpt = O;
    }

    // Fail hard if there were errors. These are strictly unrecoverable and
    // indicate serious issues such as conflicting option names or an
    // incorrectly
    // linked LLVM distribution.
    如果 (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  void addOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { addOption(O, &SC); });
  }

  void removeOption(Option *O, SubCommand *SC) {
    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    如果 (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    SubCommand &Sub = *SC;
    为 (auto Name : OptionNames) {
      auto I = Sub.OptionsMap.find(Name);
      // Re-query end() each iteration: a prior erase invalidates iterators
      // (including a cached end()) under backward-shift deletion.
      如果 (I != Sub.OptionsMap.end() && I->second == O)
        Sub.OptionsMap.erase(I);
    }

    如果 (O->getFormattingFlag() == cl::Positional)
      为 (auto *Opt = Sub.PositionalOpts.begin();
           Opt != Sub.PositionalOpts.end(); ++Opt) {
        如果 (*Opt == O) {
          Sub.PositionalOpts.erase(Opt);
          break;
        }
      }
    否则 如果 (O == Sub.ConsumeAfterOpt)
      Sub.ConsumeAfterOpt = nullptr;
  }

  void removeOption(Option *O) {
    forEachSubCommand(*O, [&](SubCommand &SC) { removeOption(O, &SC); });
  }

  bool hasOptions(const SubCommand &Sub) const {
    返回 (!Sub.OptionsMap.empty() || !Sub.PositionalOpts.empty() ||
            nullptr != Sub.ConsumeAfterOpt);
  }

  bool hasOptions() const {
    为 (const auto *S : RegisteredSubCommands) {
      如果 (hasOptions(*S))
        返回 true;
    }
    返回 !Libraries.empty();
  }

  bool hasNamedSubCommands() const {
    为 (const auto *S : RegisteredSubCommands)
      如果 (!S->getName().empty())
        返回 true;
    返回 false;
  }

  SubCommand *getActiveSubCommand() { 返回 ActiveSubCommand; }

  void updateArgStr(Option *O, StringRef NewName, SubCommand *SC) {
    SubCommand &Sub = *SC;
    如果 (!Sub.OptionsMap.insert(std::make_pair(NewName, O)).second) {
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
    为 (; NumIndexedLibraries != Libraries.size(); ++NumIndexedLibraries) {
      LibraryOptions *L = Libraries[NumIndexedLibraries];
      L->forEachOption([&](StringRef Spelling, StringRef, StringRef) {
        StringRef Name = Spelling.rtrim('=');
        auto [It, Inserted] = LibraryIndex.try_emplace(Name, L);
        如果 (Inserted ? none_of(RegisteredSubCommands,
                               [&](SubCommand *SC) {
                                 返回 SC->OptionsMap.contains(Name);
                               })
                     : It->second == L)
          返回;
        errs() << ProgramName << ": CommandLine Error: Option '" << Name
               << "' registered more than once!\n";
        HadErrors = true;
      });
    }
    如果 (HadErrors)
      report_fatal_error("inconsistency in registered CommandLine options");
  }

  // A library loaded while parsing (e.g. a pass plugin) may define the name.
  LibraryOptions *lookupLibraryOption(StringRef Name) {
    indexLibraryOptions();
    返回 LibraryIndex.lookup(Name);
  }

  void registerCategory(OptionCategory *cat) {
    assert(count_if(RegisteredOptionCategories,
                    [cat](const OptionCategory *Category) {
             返回 cat->getName() == Category->getName();
           }) == 0 &&
           "Duplicate option categories");

    RegisteredOptionCategories.insert(cat);
  }

  void registerSubCommand(SubCommand *sub) {
    assert(count_if(RegisteredSubCommands,
                    [sub](const SubCommand *Sub) {
                      返回 (!sub->getName().empty()) &&
                             (Sub->getName() == sub->getName());
                    }) == 0 &&
           "Duplicate subcommands");
    RegisteredSubCommands.insert(sub);

    // For all options that have been registered for all subcommands, add the
    // option to this subcommand now.
    assert(sub != &SubCommand::getAll() &&
           "SubCommand::getAll() should not be registered");
    为 (auto &E : SubCommand::getAll().OptionsMap) {
      Option *O = E.second;
      如果 (O->isPositional() || O->isConsumeAfter() || O->hasArgStr())
        addOption(O, sub);
      否则
        addLiteralOption(*O, sub, E.first);
    }
  }

  void unregisterSubCommand(SubCommand *sub) {
    RegisteredSubCommands.erase(sub);
  }

  iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
  getRegisteredSubcommands() {
    返回 make_range(RegisteredSubCommands.begin(),
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
  返回 *GlobalParser;
}

// Parses the library option spelled by Args[0], which takes at most one
// separate value, into Opts. Opts may refer to the arguments, so they are
// copied into Alloc, and appended to Record if given.
static Error
parseLibraryOption(LibraryOptions &L, void *Opts, ArrayRef<const char *> Args,
                   unsigned &Consumed, BumpPtrAllocator &Alloc,
                   SmallVectorImpl<const char *> *Record = nullptr) {
  StringSaver Saver(Alloc);
  const char *Saved[2];
  unsigned NumArgs = std::min<size_t>(Args.size(), 2);
  为 (unsigned J = 0; J != NumArgs; ++J)
    Saved[J] = Saver.save(Args[J]).data();
  Consumed = 1;
  如果 (Error E = L.parse(Opts, ArrayRef(Saved, NumArgs), Consumed, Alloc))
    返回 E;
  如果 (Record)
    Record->append(Saved, Saved + Consumed);
  返回 Error::success();
}

template <typename T, T TrueVal, T FalseVal>
static bool parseBool(Option &O, StringRef ArgName, StringRef Arg, T &Value) {
  // ProvideOption passes a null Arg for a bare -flag (treated as true) and an
  // empty one for -flag= (treated as invalid).
  如果 (!Arg.data() || Arg == "true" || Arg == "1") {
    Value = TrueVal;
    返回 false;
  }

  如果 (Arg == "false" || Arg == "0") {
    Value = FalseVal;
    返回 false;
  }
  返回 O.error("'" + Arg +
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
  如果 (FullyInitialized)
    globalParser().updateArgStr(this, S);
  assert(!S.starts_with("-") && "Option can't start with '-");
  ArgStr = S;
}

void Option::addCategory(OptionCategory &C) {
  assert(!Categories.empty() && "Categories cannot be empty.");
  // Maintain backward compatibility by replacing the default GeneralCategory
  // if it's still set.  Otherwise, just add the new one.  The GeneralCategory
  // must be explicitly added if you want multiple categories that include it.
  如果 (&C != &getGeneralCategory() && Categories[0] == &getGeneralCategory())
    Categories[0] = &C;
  否则 如果 (!is_contained(Categories, &C))
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
  返回 *TopLevelSubCommand;
}

// A special subcommand that can be used to put an option into all subcommands.
SubCommand &SubCommand::getAll() {
  static ManagedStatic<SubCommand> AllSubCommands;
  返回 *AllSubCommands;
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
  返回 (globalParser().getActiveSubCommand() == this);
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
  如果 (Arg.empty())
    返回 nullptr;
  assert(&Sub != &SubCommand::getAll());

  size_t EqualPos = Arg.find('=');

  // If we have an equals sign, remember the value.
  如果 (EqualPos == StringRef::npos) {
    // Look up the option.
    返回 Sub.OptionsMap.lookup(Arg);
  }

  // If the argument before the = is a valid option name and the option allows
  // non-prefix form (ie is not AlwaysPrefix), we match.  If not, signal match
  // failure by returning nullptr.
  auto I = Sub.OptionsMap.find(Arg.substr(0, EqualPos));
  如果 (I == Sub.OptionsMap.end())
    返回 nullptr;

  auto *O = I->second;
  如果 (O->getFormattingFlag() == cl::AlwaysPrefix)
    返回 nullptr;

  Value = Arg.substr(EqualPos + 1);
  Arg = Arg.substr(0, EqualPos);
  返回 I->second;
}

SubCommand *CommandLineParser::LookupSubCommand(StringRef Name,
                                                std::string &NearestString) {
  如果 (Name.empty())
    返回 &SubCommand::getTopLevel();
  // Find a subcommand with the edit distance == 1.
  SubCommand *NearestMatch = nullptr;
  为 (auto *S : RegisteredSubCommands) {
    assert(S != &SubCommand::getAll() &&
           "SubCommand::getAll() is not expected in RegisteredSubCommands");
    如果 (S->getName().empty())
      continue;

    如果 (S->getName() == Name)
      返回 S;

    如果 (!NearestMatch && S->getName().edit_distance(Name) < 2)
      NearestMatch = S;
  }

  如果 (NearestMatch)
    NearestString = NearestMatch->getName();

  返回 &SubCommand::getTopLevel();
}

/// LookupNearestOption - Lookup the closest match to the option specified by
/// the specified option on the command line.  If there is a value specified
/// (after an equal sign) return that as well.  This assumes that leading dashes
/// have already been stripped.
static Option *LookupNearestOption(StringRef Arg,
                                   const OptionsMapTy &OptionsMap,
                                   std::string &NearestString) {
  // Reject all dashes.
  如果 (Arg.empty())
    返回 nullptr;

  // Split on any equal sign.
  std::pair<StringRef, StringRef> SplitArg = Arg.split('=');
  StringRef &LHS = SplitArg.first; // LHS == Arg when no '=' is present.
  StringRef &RHS = SplitArg.second;

  // Find the closest match.
  Option *Best = nullptr;
  unsigned BestDistance = 0;
  为 (const auto &[_, O] : OptionsMap) {
    // Do not suggest really hidden options (not shown in any help).
    如果 (O->getOptionHiddenFlag() == ReallyHidden)
      continue;

    SmallVector<StringRef, 16> OptionNames;
    O->getExtraOptionNames(OptionNames);
    如果 (O->hasArgStr())
      OptionNames.push_back(O->ArgStr);

    bool PermitValue = O->getValueExpectedFlag() != cl::ValueDisallowed;
    StringRef Flag = PermitValue ? LHS : Arg;
    为 (const auto &Name : OptionNames) {
      unsigned Distance = StringRef(Name).edit_distance(
          Flag, /*AllowReplacements=*/true, /*MaxEditDistance=*/BestDistance);
      如果 (!Best || Distance < BestDistance) {
        Best = O;
        BestDistance = Distance;
        如果 (RHS.empty() || !PermitValue)
          NearestString = std::string(Name);
        否则
          NearestString = (Twine(Name) + "=" + RHS).str();
      }
    }
  }

  返回 Best;
}

/// CommaSeparateAndAddOccurrence - A wrapper around Handler->addOccurrence()
/// that does special handling of cl::CommaSeparated options.
static bool CommaSeparateAndAddOccurrence(Option *Handler, unsigned pos,
                                          StringRef ArgName, StringRef Value) {
  // Check to see if this option accepts a comma separated list of values.  If
  // it does, we have to split up the value into multiple values.
  如果 (Handler->getMiscFlags() & CommaSeparated) {
    StringRef Val(Value);
    StringRef::size_type Pos = Val.find(',');

    当 (Pos != StringRef::npos) {
      // Process the portion before the comma.
      如果 (Handler->addOccurrence(pos, ArgName, Val.substr(0, Pos)))
        返回 true;
      // Erase the portion before the comma, AND the comma.
      Val = Val.substr(Pos + 1);
      // Check for another comma.
      Pos = Val.find(',');
    }

    Value = Val;
  }

  返回 Handler->addOccurrence(pos, ArgName, Value);
}

/// ProvideOption - For Value, this differentiates between an empty value ("")
/// and a null value (StringRef()).  The later is accepted for arguments that
/// don't allow a value (-foo) the former is rejected (-foo=).
static inline bool ProvideOption(Option *Handler, StringRef ArgName,
                                 StringRef Value, 整数 argc,
                                 const char *const *argv, 整数 &i) {
  // Enforce value requirements
  switch (Handler->getValueExpectedFlag()) {
  case ValueRequired:
    如果 (!Value.data()) { // No value specified?
      // If no other argument or the option only supports prefix form, we
      // cannot look at the next argument.
      如果 (i + 1 >= argc || Handler->getFormattingFlag() == cl::AlwaysPrefix)
        返回 Handler->error("requires a value!");
      // Steal the next argument, like for '-o filename'
      assert(argv && "null check");
      Value = StringRef(argv[++i]);
    }
    break;
  case ValueDisallowed:
    如果 (Value.data())
      返回 Handler->error("does not allow a value! '" + Twine(Value) +
                            "' specified.");
    break;
  case ValueOptional:
    break;
  }

  返回 CommaSeparateAndAddOccurrence(Handler, i, ArgName, Value);
}

bool llvm::cl::ProvidePositionalOption(Option *Handler, StringRef Arg, 整数 i) {
  整数 Dummy = i;
  返回 ProvideOption(Handler, Handler->ArgStr, Arg, 0, nullptr, Dummy);
}

// Find the cl::Prefix or cl::AlwaysPrefix option whose name is the longest
// prefix of Name.
static Option *findPrefixOption(StringRef Name, size_t &Length,
                                const OptionsMapTy &OptionsMap) {
  为 (; !Name.empty(); Name = Name.drop_back()) {
    Option *O = OptionsMap.lookup(Name);
    如果 (O && (O->getFormattingFlag() == cl::Prefix ||
              O->getFormattingFlag() == cl::AlwaysPrefix)) {
      Length = Name.size();
      返回 O;
    }
  }
  返回 nullptr;
}

/// The specified argument string (which started with at least one '-') does not
/// fully match an available option.  Check to see if this is a prefix option.
/// If so, split arg into output an Arg/Value pair and return the Option to
/// parse it with.
static Option *HandlePrefixedOption(StringRef &Arg, StringRef &Value,
                                    const OptionsMapTy &OptionsMap) {
  如果 (Arg.size() == 1)
    返回 nullptr;

  size_t Length = 0;
  Option *POpt = findPrefixOption(Arg, Length, OptionsMap);
  如果 (!POpt)
    返回 nullptr;

  StringRef MaybeValue =
      (Length < Arg.size()) ? Arg.substr(Length) : StringRef();
  Arg = Arg.substr(0, Length);

  // cl::Prefix options do not preserve '=' when used separately.
  如果 (POpt->getFormattingFlag() == cl::Prefix && MaybeValue.starts_with("="))
    MaybeValue = MaybeValue.drop_front();
  Value = MaybeValue;
  返回 POpt;
}

static bool RequiresValue(const Option *O) {
  返回 O->getNumOccurrencesFlag() == cl::Required ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

static bool EatsUnboundedNumberOfValues(const Option *O) {
  返回 O->getNumOccurrencesFlag() == cl::ZeroOrMore ||
         O->getNumOccurrencesFlag() == cl::OneOrMore;
}

static bool isWhitespace(char C) {
  返回 C == ' ' || C == '\t' || C == '\r' || C == '\n';
}

static bool isWhitespaceOrNull(char C) {
  返回 isWhitespace(C) || C == '\0';
}

static bool isQuote(char C) { 返回 C == '\"' || C == '\''; }

void cl::TokenizeGNUCommandLine(StringRef Src, StringSaver &Saver,
                                SmallVectorImpl<const char *> &NewArgv,
                                bool MarkEOLs) {
  SmallString<128> Token;
  bool InToken = false;
  为 (size_t I = 0, E = Src.size(); I != E; ++I) {
    // Consume runs of whitespace.
    如果 (!InToken) {
      当 (I != E && isWhitespace(Src[I])) {
        // Mark the end of lines in response files.
        如果 (MarkEOLs && Src[I] == '\n')
          NewArgv.push_back(nullptr);
        ++I;
      }
      如果 (I == E)
        break;
      InToken = true;
    }

    char C = Src[I];

    // Backslash escapes the next character.
    如果 (I + 1 < E && C == '\\') {
      ++I; // Skip the escape.
      Token.push_back(Src[I]);
      continue;
    }

    // Consume a quoted string.
    如果 (isQuote(C)) {
      ++I;
      当 (I != E && Src[I] != C) {
        // Backslash escapes the next character.
        如果 (Src[I] == '\\' && I + 1 != E)
          ++I;
        Token.push_back(Src[I]);
        ++I;
      }
      如果 (I == E)
        break;
      continue;
    }

    // End the token if this is whitespace.
    如果 (isWhitespace(C)) {
      NewArgv.push_back(Saver.save(Token.str()).data());
      // Mark the end of lines in response files.
      如果 (MarkEOLs && C == '\n')
        NewArgv.push_back(nullptr);
      Token.clear();
      InToken = false;
      continue;
    }

    // This is a normal character.  Append it.
    Token.push_back(C);
  }

  // Append the last token after hitting EOF with no whitespace.
  如果 (InToken)
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
  整数 BackslashCount = 0;
  // Skip the backslashes.
  do {
    ++I;
    ++BackslashCount;
  } 当 (I != E && Src[I] == '\\');

  bool FollowedByDoubleQuote = (I != E && Src[I] == '"');
  如果 (FollowedByDoubleQuote) {
    Token.append(BackslashCount / 2, '\\');
    如果 (BackslashCount % 2 == 0)
      返回 I - 1;
    Token.push_back('"');
    返回 I;
  }
  Token.append(BackslashCount, '\\');
  返回 I - 1;
}

// Windows treats whitespace, double quotes, and backslashes specially, except
// when parsing the first token of a full command line, in which case
// backslashes are not special.
static bool isWindowsSpecialChar(char C) {
  返回 isWhitespaceOrNull(C) || C == '\\' || C == '\"';
}
static bool isWindowsSpecialCharInCommandName(char C) {
  返回 isWhitespaceOrNull(C) || C == '\"';
}

// Windows tokenization implementation. The implementation is designed to be
// inlined and specialized for the two user entry points.
static inline void tokenizeWindowsCommandLineImpl(
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
  enum { INIT, UNQUOTED, QUOTED } State = INIT;

  为 (size_t I = 0, E = Src.size(); I < E; ++I) {
    switch (State) {
    case INIT: {
      assert(Token.empty() && "token should be empty in initial state");
      // Eat whitespace before a token.
      当 (I < E && isWhitespaceOrNull(Src[I])) {
        如果 (Src[I] == '\n')
          MarkEOL();
        ++I;
      }
      // Stop if this was trailing whitespace.
      如果 (I >= E)
        break;
      size_t Start = I;
      如果 (CommandName) {
        当 (I < E && !isWindowsSpecialCharInCommandName(Src[I]))
          ++I;
      } 否则 {
        当 (I < E && !isWindowsSpecialChar(Src[I]))
          ++I;
      }
      StringRef NormalChars = Src.slice(Start, I);
      如果 (I >= E || isWhitespaceOrNull(Src[I])) {
        // No special characters: slice out the substring and start the next
        // token. Copy the string if the caller asks us to.
        AddToken(AlwaysCopy ? Saver.save(NormalChars) : NormalChars);
        如果 (I < E && Src[I] == '\n') {
          MarkEOL();
          CommandName = InitialCommandName;
        } 否则 {
          CommandName = false;
        }
      } 否则 如果 (Src[I] == '\"') {
        Token += NormalChars;
        State = QUOTED;
      } 否则 如果 (Src[I] == '\\') {
        assert(!CommandName && "or else we'd have treated it as a normal char");
        Token += NormalChars;
        I = parseBackslash(Src, I, Token);
        State = UNQUOTED;
      } 否则 {
        llvm_unreachable("unexpected special character");
      }
      break;
    }

    case UNQUOTED:
      如果 (isWhitespaceOrNull(Src[I])) {
        // Whitespace means the end of the token. If we are in this state, the
        // token must have contained a special character, so we must copy the
        // token.
        AddToken(Saver.save(Token.str()));
        Token.clear();
        如果 (Src[I] == '\n') {
          CommandName = InitialCommandName;
          MarkEOL();
        } 否则 {
          CommandName = false;
        }
        State = INIT;
      } 否则 如果 (Src[I] == '\"') {
        State = QUOTED;
      } 否则 如果 (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } 否则 {
        Token.push_back(Src[I]);
      }
      break;

    case QUOTED:
      如果 (Src[I] == '\"') {
        如果 (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.push_back('"');
          ++I;
        } 否则 {
          // Otherwise, end the quoted portion and return to the unquoted state.
          State = UNQUOTED;
        }
      } 否则 如果 (Src[I] == '\\' && !CommandName) {
        I = parseBackslash(Src, I, Token);
      } 否则 {
        Token.push_back(Src[I]);
      }
      break;
    }
  }

  如果 (State != INIT)
    AddToken(Saver.save(Token.str()));
}

void cl::TokenizeWindowsCommandLine(StringRef Src, StringSaver &Saver,
                                    SmallVectorImpl<const char *> &NewArgv,
                                    bool MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    如果 (MarkEOLs)
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
                                        SmallVectorImpl<const char *> &NewArgv,
                                        bool MarkEOLs) {
  auto AddToken = [&](StringRef Tok) { NewArgv.push_back(Tok.data()); };
  auto OnEOL = [&]() {
    如果 (MarkEOLs)
      NewArgv.push_back(nullptr);
  };
  tokenizeWindowsCommandLineImpl(Src, Saver, AddToken,
                                 /*AlwaysCopy=*/true, OnEOL, true);
}

void cl::tokenizeConfigFile(StringRef Source, StringSaver &Saver,
                            SmallVectorImpl<const char *> &NewArgv,
                            bool MarkEOLs) {
  为 (const char *Cur = Source.begin(); Cur != Source.end();) {
    SmallString<128> Line;
    // Check for comment line.
    如果 (isWhitespace(*Cur)) {
      当 (Cur != Source.end() && isWhitespace(*Cur))
        ++Cur;
      continue;
    }
    如果 (*Cur == '#') {
      当 (Cur != Source.end() && *Cur != '\n')
        ++Cur;
      continue;
    }
    // Find end of the current line, splicing backslash-newline continuations.
    // A '#' that begins a new unquoted token starts a comment running to the
    // next literal newline; escapes and continuations are not honored inside
    // the comment.
    const char *Start = Cur;
    const char *LineEnd = nullptr;
    char Quote = 0;
    bool AtTokenStart = true;
    为 (const char *End = Source.end(); Cur != End; ++Cur) {
      char C = *Cur;
      如果 (C == '\\') {
        如果 (Cur + 1 != End) {
          ++Cur;
          如果 (*Cur == '\n' ||
              (*Cur == '\r' && (Cur + 1 != End) && Cur[1] == '\n')) {
            Line.append(Start, Cur - 1);
            如果 (*Cur == '\r')
              ++Cur;
            Start = Cur + 1;
            // Splicing does not introduce a token boundary.
            continue;
          }
        }
        AtTokenStart = false;
        continue;
      }
      如果 (C == '\n')
        break;
      如果 (Quote) {
        如果 (C == Quote)
          Quote = 0;
      } 否则 如果 (isQuote(C)) {
        Quote = C;
      } 否则 如果 (C == '#' && AtTokenStart) {
        LineEnd = Cur;
        当 (Cur != End && *Cur != '\n')
          ++Cur;
        break;
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
static bool hasUTF8ByteOrderMark(ArrayRef<char> S) {
  返回 (S.size() >= 3 && S[0] == '\xef' && S[1] == '\xbb' && S[2] == '\xbf');
}

// Substitute <CFGDIR> with the file's base path.
static void ExpandBasePaths(StringRef BasePath, StringSaver &Saver,
                            const char *&Arg) {
  assert(sys::path::is_absolute(BasePath));
  constexpr StringLiteral Token("<CFGDIR>");
  const StringRef ArgString(Arg);

  SmallString<128> ResponseFile;
  StringRef::size_type StartPos = 0;
  为 (StringRef::size_type TokenPos = ArgString.find(Token);
       TokenPos != StringRef::npos;
       TokenPos = ArgString.find(Token, StartPos)) {
    // Token may appear more than once per arg (e.g. comma-separated linker
    // args). Support by using path-append on any subsequent appearances.
    const StringRef LHS = ArgString.substr(StartPos, TokenPos - StartPos);
    如果 (ResponseFile.empty())
      ResponseFile = LHS;
    否则
      llvm::sys::path::append(ResponseFile, LHS);
    ResponseFile.append(BasePath);
    StartPos = TokenPos + Token.size();
  }

  如果 (!ResponseFile.empty()) {
    // Path-append the remaining arg substring if at least one token appeared.
    const StringRef Remaining = ArgString.substr(StartPos);
    如果 (!Remaining.empty())
      llvm::sys::path::append(ResponseFile, Remaining);
    Arg = Saver.save(ResponseFile.str()).data();
  }
}

// FName must be an absolute path.
Error ExpansionContext::expandResponseFile(
    StringRef FName, SmallVectorImpl<const char *> &NewArgv) {
  assert(sys::path::is_absolute(FName));
  llvm::ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      FS->getBufferForFile(FName);
  如果 (!MemBufOrErr) {
    std::error_code EC = MemBufOrErr.getError();
    返回 llvm::createStringError(EC, Twine("cannot not open file '") + FName +
                                           "': " + EC.message());
  }
  MemoryBuffer &MemBuf = *MemBufOrErr.get();
  StringRef Str(MemBuf.getBufferStart(), MemBuf.getBufferSize());

  // If we have a UTF-16 byte order mark, convert to UTF-8 for parsing.
  ArrayRef<char> BufRef(MemBuf.getBufferStart(), MemBuf.getBufferEnd());
  std::string UTF8Buf;
  如果 (hasUTF16ByteOrderMark(BufRef)) {
    如果 (!convertUTF16ToUTF8String(BufRef, UTF8Buf))
      返回 llvm::createStringError(std::errc::illegal_byte_sequence,
                                     "Could not convert UTF16 to UTF8");
    Str = StringRef(UTF8Buf);
  }
  // If we see UTF-8 BOM sequence at the beginning of a file, we shall remove
  // these bytes before parsing.
  // Reference: http://en.wikipedia.org/wiki/UTF-8#Byte_order_mark
  否则 如果 (hasUTF8ByteOrderMark(BufRef))
    Str = StringRef(BufRef.data() + 3, BufRef.size() - 3);

  // Tokenize the contents into NewArgv.
  Tokenizer(Str, Saver, NewArgv, MarkEOLs);

  // Expanded file content may require additional transformations, like using
  // absolute paths instead of relative in '@file' constructs or expanding
  // macros.
  如果 (!RelativeNames && !InConfigFile)
    返回 Error::success();

  StringRef BasePath = llvm::sys::path::parent_path(FName);
  为 (const char *&Arg : NewArgv) {
    如果 (!Arg)
      continue;

    // Substitute <CFGDIR> with the file's base path.
    如果 (InConfigFile)
      ExpandBasePaths(BasePath, Saver, Arg);

    // Discover the case, when argument should be transformed into '@file' and
    // evaluate 'file' for it.
    StringRef ArgStr(Arg);
    StringRef FileName;
    bool ConfigInclusion = false;
    如果 (ArgStr.consume_front("@")) {
      FileName = ArgStr;
      如果 (!llvm::sys::path::is_relative(FileName))
        continue;
    } 否则 如果 (ArgStr.consume_front("--config=")) {
      FileName = ArgStr;
      ConfigInclusion = true;
    } 否则 {
      continue;
    }

    // Update expansion construct.
    SmallString<128> ResponseFile;
    ResponseFile.push_back('@');
    如果 (ConfigInclusion && !llvm::sys::path::has_parent_path(FileName)) {
      SmallString<128> FilePath;
      如果 (!findConfigFile(FileName, FilePath))
        返回 createStringError(
            std::make_error_code(std::errc::no_such_file_or_directory),
            "cannot not find configuration file: " + FileName);
      ResponseFile.append(FilePath);
    } 否则 {
      ResponseFile.append(BasePath);
      llvm::sys::path::append(ResponseFile, FileName);
    }
    Arg = Saver.save(ResponseFile.str()).data();
  }
  返回 Error::success();
}

/// Expand response files on a command line recursively using the given
/// StringSaver and tokenization strategy.
Error ExpansionContext::expandResponseFiles(
    SmallVectorImpl<const char *> &Argv) {
  struct ResponseFileRecord {
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
  为 (unsigned I = 0; I != Argv.size();) {
    当 (I == FileStack.back().End) {
      // Passing the end of a file's argument list, so we can remove it from the
      // stack.
      FileStack.pop_back();
    }

    const char *Arg = Argv[I];
    // Check if it is an EOL marker
    如果 (Arg == nullptr) {
      ++I;
      continue;
    }

    如果 (Arg[0] != '@') {
      ++I;
      continue;
    }

    const char *FName = Arg + 1;
    // Note that CurrentDir is only used for top-level rsp files, the rest will
    // always have an absolute path deduced from the containing file.
    SmallString<128> CurrDir;
    如果 (llvm::sys::path::is_relative(FName)) {
      如果 (CurrentDir.empty()) {
        如果 (auto CWD = FS->getCurrentWorkingDirectory()) {
          CurrDir = *CWD;
        } 否则 {
          返回 createStringError(
              CWD.getError(), Twine("cannot get absolute path for: ") + FName);
        }
      } 否则 {
        CurrDir = CurrentDir;
      }
      llvm::sys::path::append(CurrDir, FName);
      FName = CurrDir.c_str();
    }

    ErrorOr<llvm::vfs::Status> Res = FS->status(FName);
    如果 (!Res || !Res->exists()) {
      std::error_code EC = Res.getError();
      如果 (!InConfigFile) {
        // If the specified file does not exist, leave '@file' unexpanded, as
        // libiberty does.
        如果 (!EC || EC == llvm::errc::no_such_file_or_directory) {
          ++I;
          continue;
        }
      }
      如果 (!EC)
        EC = llvm::errc::no_such_file_or_directory;
      返回 createStringError(EC, Twine("cannot not open file '") + FName +
                                       "': " + EC.message());
    }
    const llvm::vfs::Status &FileStatus = Res.get();

    auto IsEquivalent =
        [FileStatus, this](const ResponseFileRecord &RFile) -> ErrorOr<bool> {
      ErrorOr<llvm::vfs::Status> RHS = FS->status(RFile.File);
      如果 (!RHS)
        返回 RHS.getError();
      返回 FileStatus.equivalent(*RHS);
    };

    // Check for recursive response files.
    为 (const auto &F : drop_begin(FileStack)) {
      如果 (ErrorOr<bool> R = IsEquivalent(F)) {
        如果 (R.get())
          返回 createStringError(
              R.getError(), Twine("recursive expansion of: '") + F.File + "'");
      } 否则 {
        返回 createStringError(R.getError(),
                                 Twine("cannot open file: ") + F.File);
      }
    }

    // Replace this response file argument with the tokenization of its
    // contents.  Nested response files are expanded in subsequent iterations.
    SmallVector<const char *, 0> ExpandedArgv;
    如果 (Error Err = expandResponseFile(FName, ExpandedArgv))
      返回 Err;

    为 (ResponseFileRecord &Record : FileStack) {
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
  返回 Error::success();
}

bool cl::expandResponseFiles(整数 Argc, const char *const *Argv,
                             const char *EnvVar, StringSaver &Saver,
                             SmallVectorImpl<const char *> &NewArgv) {
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#否则
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  // The environment variable specifies initial options.
  如果 (EnvVar)
    如果 (std::optional<std::string> EnvValue = sys::Process::GetEnv(EnvVar))
      Tokenize(*EnvValue, Saver, NewArgv, /*MarkEOLs=*/false);

  // Command line options can override the environment variable.
  NewArgv.append(Argv + 1, Argv + Argc);
  ExpansionContext ECtx(Saver.getAllocator(), Tokenize);
  如果 (Error Err = ECtx.expandResponseFiles(NewArgv)) {
    errs() << toString(std::move(Err)) << '\n';
    返回 false;
  }
  返回 true;
}

bool cl::ExpandResponseFiles(StringSaver &Saver, TokenizerCallback Tokenizer,
                             SmallVectorImpl<const char *> &Argv) {
  ExpansionContext ECtx(Saver.getAllocator(), Tokenizer);
  如果 (Error Err = ECtx.expandResponseFiles(Argv)) {
    errs() << toString(std::move(Err)) << '\n';
    返回 false;
  }
  返回 true;
}

ExpansionContext::ExpansionContext(BumpPtrAllocator &A, TokenizerCallback T,
                                   vfs::FileSystem *FS)
    : Saver(A), Tokenizer(T), FS(FS ? FS : vfs::getRealFileSystem().get()) {}

bool ExpansionContext::findConfigFile(StringRef FileName,
                                      SmallVectorImpl<char> &FilePath) {
  SmallString<128> CfgFilePath;
  const auto FileExists = [this](SmallString<128> Path) -> bool {
    auto Status = FS->status(Path);
    返回 Status &&
           Status->getType() == llvm::sys::fs::file_type::regular_file;
  };

  // If file name contains directory separator, treat it as a path to
  // configuration file.
  如果 (llvm::sys::path::has_parent_path(FileName)) {
    CfgFilePath = FileName;
    如果 (llvm::sys::path::is_relative(FileName) && FS->makeAbsolute(CfgFilePath))
      返回 false;
    如果 (!FileExists(CfgFilePath))
      返回 false;
    FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
    返回 true;
  }

  // Look for the file in search directories.
  为 (const StringRef &Dir : SearchDirs) {
    如果 (Dir.empty())
      continue;
    CfgFilePath.assign(Dir);
    llvm::sys::path::append(CfgFilePath, FileName);
    llvm::sys::path::native(CfgFilePath);
    如果 (FileExists(CfgFilePath)) {
      FilePath.assign(CfgFilePath.begin(), CfgFilePath.end());
      返回 true;
    }
  }

  返回 false;
}

Error ExpansionContext::readConfigFile(StringRef CfgFile,
                                       SmallVectorImpl<const char *> &Argv) {
  SmallString<128> AbsPath;
  如果 (sys::path::is_relative(CfgFile)) {
    AbsPath.assign(CfgFile);
    如果 (std::error_code EC = FS->makeAbsolute(AbsPath))
      返回 make_error<StringError>(
          EC, Twine("cannot get absolute path for " + CfgFile));
    CfgFile = AbsPath.str();
  }
  InConfigFile = true;
  RelativeNames = true;
  如果 (Error Err = expandResponseFile(CfgFile, Argv))
    返回 Err;
  返回 expandResponseFiles(Argv);
}

static void initCommonOptions();
static bool parseCommandLineOptionsImpl(LibraryOptionsContext *Context,
                                        ArrayRef<const char *> Args,
                                        StringRef Overview, raw_ostream *Errs,
                                        vfs::FileSystem *VFS,
                                        const char *EnvVar) {
  initCommonOptions();
  SmallVector<const char *, 20> NewArgv;
  BumpPtrAllocator A;
  StringSaver Saver(A);
  NewArgv.push_back(Args[0]);

  // Parse options from environment variable.
  如果 (EnvVar) {
    如果 (std::optional<std::string> EnvValue =
            sys::Process::GetEnv(StringRef(EnvVar)))
      TokenizeGNUCommandLine(*EnvValue, Saver, NewArgv);
  }

  // Append options from command line.
  append_range(NewArgv, Args.drop_front());
  整数 NewArgc = static_cast<整数>(NewArgv.size());

  // Parse all options.
  返回 globalParser().ParseCommandLineOptions(NewArgc, &NewArgv[0], Overview,
                                                Errs, VFS, Context);
}

bool cl::ParseCommandLineOptions(整数 argc, const char *const *argv,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, const char *EnvVar) {
  返回 parseCommandLineOptionsImpl(nullptr, ArrayRef(argv, argc), Overview,
                                     Errs, VFS, EnvVar);
}

bool cl::parseCommandLineOptions(LibraryOptionsContext &Context,
                                 ArrayRef<const char *> Args,
                                 StringRef Overview, raw_ostream *Errs,
                                 vfs::FileSystem *VFS, const char *EnvVar) {
  返回 parseCommandLineOptionsImpl(&Context, Args, Overview, Errs, VFS,
                                     EnvVar);
}

/// Reset all options at least once, so that we can parse different options.
void CommandLineParser::ResetAllOptionOccurrences() {
  // Reset all option values to look like they have never been seen before.
  // Options might be reset twice (they can be reference in both OptionsMap
  // and one of the other members), but that does not harm.
  为 (auto *SC : RegisteredSubCommands) {
    // reset() removes default options from OptionsMap (via removeArgument), so
    // collect the options first to avoid invalidating the map iterator.
    SmallVector<Option *, 0> Opts;
    Opts.reserve(SC->OptionsMap.size());
    为 (auto &O : SC->OptionsMap)
      Opts.push_back(O.second);
    为 (Option *O : Opts)
      O->reset();
    为 (Option *O : SC->PositionalOpts)
      O->reset();
    如果 (SC->ConsumeAfterOpt)
      SC->ConsumeAfterOpt->reset();
  }
  为 (LibraryOptions *L : Libraries)
    L->reset();
  LibraryArgAlloc.Reset();
}

bool CommandLineParser::ParseCommandLineOptions(
    整数 argc, const char *const *argv, StringRef Overview, raw_ostream *Errs,
    vfs::FileSystem *VFS, LibraryOptionsContext *Context) {
  assert(hasOptions() && "No options specified!");

  ProgramOverview = Overview;
  bool IgnoreErrors = Errs;
  如果 (!Errs)
    Errs = &errs();
  如果 (!VFS)
    VFS = vfs::getRealFileSystem().get();
  bool ErrorParsing = false;

  // Expand response files.
  SmallVector<const char *, 20> newArgv(argv, argv + argc);
  BumpPtrAllocator A;
#ifdef _WIN32
  auto Tokenize = cl::TokenizeWindowsCommandLine;
#否则
  auto Tokenize = cl::TokenizeGNUCommandLine;
#endif
  ExpansionContext ECtx(A, Tokenize, VFS);
  如果 (Error Err = ECtx.expandResponseFiles(newArgv)) {
    *Errs << toString(std::move(Err)) << '\n';
    返回 false;
  }
  argv = &newArgv[0];
  argc = static_cast<整数>(newArgv.size());

  // Copy the program name into ProgName, making sure not to overflow it.
  ProgramName = std::string(sys::path::filename(StringRef(argv[0])));
  indexLibraryOptions();

  // Check out the positional arguments to collect information about them.
  unsigned NumPositionalRequired = 0;

  // Determine whether or not there are an unlimited number of positionals
  bool HasUnlimitedPositionals = false;

  整数 FirstArg = 1;
  SubCommand *ChosenSubCommand = &SubCommand::getTopLevel();
  std::string NearestSubCommandString;
  bool MaybeNamedSubCommand =
      argc >= 2 && argv[FirstArg][0] != '-' && hasNamedSubCommands();
  如果 (MaybeNamedSubCommand) {
    // If the first argument specifies a valid subcommand, start processing
    // options from the second argument.
    ChosenSubCommand =
        LookupSubCommand(StringRef(argv[FirstArg]), NearestSubCommandString);
    如果 (ChosenSubCommand != &SubCommand::getTopLevel())
      FirstArg = 2;
  }
  globalParser().ActiveSubCommand = ChosenSubCommand;

  assert(ChosenSubCommand);
  auto &ConsumeAfterOpt = ChosenSubCommand->ConsumeAfterOpt;
  auto &PositionalOpts = ChosenSubCommand->PositionalOpts;
  auto &OptionsMap = ChosenSubCommand->OptionsMap;

  如果 (ConsumeAfterOpt) {
    assert(PositionalOpts.size() > 0 &&
           "Cannot specify cl::ConsumeAfter without a positional argument!");
  }
  如果 (!PositionalOpts.empty()) {

    // Calculate how many positional values are _required_.
    bool UnboundedFound = false;
    为 (size_t i = 0, e = PositionalOpts.size(); i != e; ++i) {
      Option *Opt = PositionalOpts[i];
      如果 (RequiresValue(Opt))
        ++NumPositionalRequired;
      否则 如果 (ConsumeAfterOpt) {
        // ConsumeAfter cannot be combined with "optional" positional options
        // unless there is only one positional argument...
        如果 (PositionalOpts.size() > 1) {
          如果 (!IgnoreErrors)
            Opt->error("error - this positional option will never be matched, "
                       "because it does not Require a value, and a "
                       "cl::ConsumeAfter option is active!");
          ErrorParsing = true;
        }
      } 否则 如果 (UnboundedFound && !Opt->hasArgStr()) {
        // This option does not "require" a value...  Make sure this option is
        // not specified after an option that eats all extra arguments, or this
        // one will never get any!
        //
        如果 (!IgnoreErrors)
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
  为 (整数 i = FirstArg; i < argc; ++i) {
    Option *Handler = nullptr;
    std::string NearestHandlerString;
    StringRef Value;
    StringRef ArgName = "";

    // Check to see if this is a positional argument.  This argument is
    // considered to be positional if it doesn't start with '-', if it is "-"
    // itself, or if we have seen "--" already.
    //
    如果 (argv[i][0] != '-' || argv[i][1] == 0 || DashDashFound) {
      // Positional argument!
      如果 (ActivePositionalArg) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        continue; // We are done!
      }

      如果 (!PositionalOpts.empty()) {
        PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));

        // All of the positional arguments have been fulfulled, give the rest to
        // the consume after option... if it's specified...
        //
        如果 (PositionalVals.size() >= NumPositionalRequired && ConsumeAfterOpt) {
          为 (++i; i < argc; ++i)
            PositionalVals.push_back(std::make_pair(StringRef(argv[i]), i));
          break; // Handle outside of the argument processing loop...
        }

        // Delay processing positional arguments until the end...
        continue;
      }
    } 否则 如果 (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 &&
               !DashDashFound) {
      DashDashFound = true; // This is the mythical "--"?
      continue;             // Don't try to process it as an argument itself.
    } 否则 如果 (ActivePositionalArg &&
               (ActivePositionalArg->getMiscFlags() & PositionalEatsArgs)) {
      // If there is a positional argument eating options, check to see if this
      // option is another positional argument.  If so, treat it as an argument,
      // otherwise feed it to the eating positional.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);
      如果 (!Handler || Handler->getFormattingFlag() != cl::Positional) {
        ProvidePositionalOption(ActivePositionalArg, StringRef(argv[i]), i);
        continue; // We are done!
      }
    } 否则 { // We start with a '-', must be an argument.
      ArgName = StringRef(argv[i] + 1);
      // Eat second dash.
      ArgName.consume_front("-");

      Handler = LookupOption(*ChosenSubCommand, ArgName, Value);

      // If Handler is not found in a specialized subcommand, look up handler
      // in the top-level subcommand.
      // cl::opt without cl::sub belongs to top-level subcommand.
      如果 (!Handler && ChosenSubCommand != &SubCommand::getTopLevel())
        Handler = LookupOption(SubCommand::getTopLevel(), ArgName, Value);

      如果 (!Handler) {
        如果 (LibraryOptions *L = lookupLibraryOption(ArgName.split('=').first)) {
          ArrayRef<const char *> Rest(argv + i, argc - i);
          unsigned N;
          如果 (Error E = Context && L->isPerContext()
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
      如果 (!Handler)
        Handler = HandlePrefixedOption(ArgName, Value, OptionsMap);

      // Otherwise, look for the closest available option to report to the user
      // in the upcoming error.
      如果 (!Handler)
        LookupNearestOption(ArgName, OptionsMap, NearestHandlerString);
    }

    如果 (!Handler) {
      auto ReportUnknownArgument = [&](bool IsArg,
                                       StringRef NearestArgumentName) {
        *Errs << ProgramName << ": Unknown "
              << (IsArg ? "command line argument" : "subcommand") << " '"
              << argv[i] << "'.  Try: '" << argv[0] << " --help'\n";

        如果 (NearestArgumentName.empty())
          返回;

        *Errs << ProgramName << ": Did you mean '";
        如果 (IsArg)
          *Errs << PrintArg(NearestArgumentName, 0);
        否则
          *Errs << NearestArgumentName;
        *Errs << "'?\n";
      };

      如果 (i > 1 || !MaybeNamedSubCommand)
        ReportUnknownArgument(/*IsArg=*/true, NearestHandlerString);
      否则
        ReportUnknownArgument(/*IsArg=*/false, NearestSubCommandString);

      ErrorParsing = true;
      continue;
    }

    // If this is a named positional argument, just remember that it is the
    // active one...
    如果 (Handler->getFormattingFlag() == cl::Positional) {
      如果 ((Handler->getMiscFlags() & PositionalEatsArgs) && !Value.empty()) {
        Handler->error("This argument does not take a value.\n"
                       "\tInstead, it consumes any positional arguments until "
                       "the next recognized option.", *Errs);
        ErrorParsing = true;
      }
      ActivePositionalArg = Handler;
    }
    否则
      ErrorParsing |= ProvideOption(Handler, ArgName, Value, argc, argv, i);
  }

  // Check and handle positional arguments now...
  如果 (NumPositionalRequired > PositionalVals.size()) {
      *Errs << ProgramName
             << ": Not enough positional command line arguments specified!\n"
             << "Must specify at least " << NumPositionalRequired
             << " positional argument" << (NumPositionalRequired > 1 ? "s" : "")
             << ": See: " << argv[0] << " --help\n";

    ErrorParsing = true;
  } 否则 如果 (!HasUnlimitedPositionals &&
             PositionalVals.size() > PositionalOpts.size()) {
    *Errs << ProgramName << ": Too many positional arguments specified!\n"
          << "Can specify at most " << PositionalOpts.size()
          << " positional arguments: See: " << argv[0] << " --help\n";
    ErrorParsing = true;

  } 否则 如果 (!ConsumeAfterOpt) {
    // Positional args have already been handled if ConsumeAfter is specified.
    unsigned ValNo = 0, NumVals = static_cast<unsigned>(PositionalVals.size());
    为 (Option *Opt : PositionalOpts) {
      如果 (RequiresValue(Opt)) {
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
      当 (NumVals - ValNo > NumPositionalRequired && !Done) {
        switch (Opt->getNumOccurrencesFlag()) {
        case cl::Optional:
          Done = true; // Optional arguments want _at most_ one value
          [[fallthrough]];
        case cl::ZeroOrMore: // Zero or more will take all they can get...
        case cl::OneOrMore:  // One or more will take all they can get...
          ProvidePositionalOption(Opt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
          ValNo++;
          break;
        default:
          llvm_unreachable("Internal error, unexpected NumOccurrences flag in "
                           "positional argument processing!");
        }
      }
    }
  } 否则 {
    assert(ConsumeAfterOpt && NumPositionalRequired <= PositionalVals.size());
    unsigned ValNo = 0;
    为 (Option *Opt : PositionalOpts)
      如果 (RequiresValue(Opt)) {
        ErrorParsing |= ProvidePositionalOption(
            Opt, PositionalVals[ValNo].first, PositionalVals[ValNo].second);
        ValNo++;
      }

    // Handle the case where there is just one positional option, and it's
    // optional.  In this case, we want to give JUST THE FIRST option to the
    // positional option and keep the rest for the consume after.  The above
    // loop would have assigned no values to positional options in this case.
    //
    如果 (PositionalOpts.size() == 1 && ValNo == 0 && !PositionalVals.empty()) {
      ErrorParsing |= ProvidePositionalOption(PositionalOpts[0],
                                              PositionalVals[ValNo].first,
                                              PositionalVals[ValNo].second);
      ValNo++;
    }

    // Handle over all of the rest of the arguments to the
    // cl::ConsumeAfter command line option...
    为 (; ValNo != PositionalVals.size(); ++ValNo)
      ErrorParsing |=
          ProvidePositionalOption(ConsumeAfterOpt, PositionalVals[ValNo].first,
                                  PositionalVals[ValNo].second);
  }

  // Loop over args and make sure all required args are specified!
  为 (const auto &Opt : OptionsMap) {
    switch (Opt.second->getNumOccurrencesFlag()) {
    case Required:
    case OneOrMore:
      如果 (Opt.second->getNumOccurrences() == 0) {
        Opt.second->error("must be specified at least once!");
        ErrorParsing = true;
      }
      [[fallthrough]];
    default:
      break;
    }
  }

  // Now that we know if -debug is specified, we can use it.
  // Note that if ReadResponseFiles == true, this must be done before the
  // memory allocated for the expanded command line is free()d below.
  LLVM_DEBUG(dbgs() << "Args: ";
             为 (整数 i = 0; i < argc; ++i) dbgs() << argv[i] << ' ';
             dbgs() << '\n';);

  // Free all of the memory allocated to the map.  Command line options may only
  // be processed once!
  MoreHelp.clear();

  // If we had an error processing our arguments, don't let the program execute
  如果 (ErrorParsing) {
    如果 (!IgnoreErrors)
      exit(1);
    返回 false;
  }
  返回 true;
}

//===----------------------------------------------------------------------===//
// Option Base class implementation
//

bool Option::error(const Twine &Message, StringRef ArgName, raw_ostream &Errs) {
  如果 (!ArgName.data())
    ArgName = ArgStr;
  如果 (ArgName.empty())
    Errs << HelpStr; // Be nice for positional arguments
  否则
    Errs << globalParser().ProgramName << ": for the " << PrintArg(ArgName, 0);

  Errs << " option: " << Message << "\n";
  返回 true;
}

bool Option::addOccurrence(unsigned pos, StringRef ArgName, StringRef Value) {
  ++NumOccurrences;
  返回 handleOccurrence(pos, ArgName, Value);
}

// getValueStr - Get the value description string, using "DefaultMsg" if nothing
// has been specified yet.
//
static StringRef getValueStr(const Option &O, StringRef DefaultMsg) {
  如果 (O.ValueStr.empty())
    返回 DefaultMsg;
  返回 O.ValueStr;
}

//===----------------------------------------------------------------------===//
// cl::alias class implementation
//

// Return the width of the option tag for printing...
size_t alias::getOptionWidth() const {
  返回 argPlusPrefixesSize(ArgStr);
}

void Option::printHelpStr(StringRef HelpStr, size_t Indent,
                          size_t FirstLineIndentedBy) {
  assert(Indent >= FirstLineIndentedBy);
  std::pair<StringRef, StringRef> Split = HelpStr.split('\n');
  outs().indent(Indent - FirstLineIndentedBy)
      << ArgHelpPrefix << Split.first << "\n";
  当 (!Split.second.empty()) {
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
  当 (!Split.second.empty()) {
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
  如果 (!ValName.empty()) {
    size_t FormattingLen = 3;
    如果 (O.getMiscFlags() & PositionalEatsArgs)
      FormattingLen = 6;
    Len += getValueStr(O, ValName).size() + FormattingLen;
  }

  返回 Len;
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
void basic_parser_impl::printOptionInfo(const Option &O,
                                        size_t GlobalWidth) const {
  outs() << PrintArg(O.ArgStr);

  auto ValName = getValueName();
  如果 (!ValName.empty()) {
    如果 (O.getMiscFlags() & PositionalEatsArgs) {
      outs() << " <" << getValueStr(O, ValName) << ">...";
    } 否则 如果 (O.getValueExpectedFlag() == ValueOptional)
      outs() << "[=<" << getValueStr(O, ValName) << ">]";
    否则 {
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
  返回 parseBool<bool, true, false>(O, ArgName, Arg, Value);
}

// parser<boolOrDefault> implementation
//
bool parser<boolOrDefault>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  boolOrDefault &Value) {
  返回 parseBool<boolOrDefault, boolOrDefault::BOU_TRUE,
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
  如果 (!Arg.getAsInteger(0, MinValue)) {
    Value = FixedOrScalableQuantityT::getFixed(MinValue);
    返回 false;
  }

  StringRef Remainder = Arg;
  如果 (!Remainder.consume_front("vscale"))
    返回 O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  如果 (!Remainder.consume_front('x'))
    返回 O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Remainder = Remainder.ltrim();
  如果 (Remainder.getAsInteger(0, MinValue))
    返回 O.error("'" + Arg + "' value invalid for " + ValueKind +
                   " argument!");

  Value = FixedOrScalableQuantityT::getScalable(MinValue);
  返回 false;
}

// parser<int> implementation
//
bool parser<整数>::parse(Option &O, StringRef ArgName, StringRef Arg,
                        整数 &Value) {
  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for integer argument!");
  返回 false;
}

// parser<long> implementation
//
bool parser<long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                         long &Value) {
  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for long argument!");
  返回 false;
}

// parser<long long> implementation
//
bool parser<long long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                              long long &Value) {
  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for llong argument!");
  返回 false;
}

// parser<unsigned> implementation
//
bool parser<unsigned>::parse(Option &O, StringRef ArgName, StringRef Arg,
                             unsigned &Value) {

  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for uint argument!");
  返回 false;
}

// parser<unsigned long> implementation
//
bool parser<unsigned long>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                  unsigned long &Value) {

  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for ulong argument!");
  返回 false;
}

// parser<unsigned long long> implementation
//
bool parser<unsigned long long>::parse(Option &O, StringRef ArgName,
                                       StringRef Arg,
                                       unsigned long long &Value) {

  如果 (Arg.getAsInteger(0, Value))
    返回 O.error("'" + Arg + "' value invalid for ullong argument!");
  返回 false;
}

// parser<ElementCount> implementation
//
bool parser<ElementCount>::parse(Option &O, StringRef ArgName, StringRef Arg,
                                 ElementCount &Value) {
  返回 parseFixedOrScalableQuantity(O, Arg, getValueName(), Value);
}

// parser<double>/parser<float> implementation
//
static bool parseDouble(Option &O, StringRef Arg, double &Value) {
  如果 (to_float(Arg, Value))
    返回 false;
  返回 O.error("'" + Arg + "' value invalid for floating point argument!");
}

bool parser<double>::parse(Option &O, StringRef ArgName, StringRef Arg,
                           double &Val) {
  返回 parseDouble(O, Arg, Val);
}

bool parser<float>::parse(Option &O, StringRef ArgName, StringRef Arg,
                          float &Val) {
  double dVal;
  如果 (parseDouble(O, Arg, dVal))
    返回 true;
  Val = (float)dVal;
  返回 false;
}

// generic_parser_base implementation
//

// findOption - Return the option number corresponding to the specified
// argument string.  If the option is not found, getNumOptions() is returned.
//
unsigned generic_parser_base::findOption(StringRef Name) {
  unsigned e = getNumOptions();

  为 (unsigned i = 0; i != e; ++i) {
    如果 (getOption(i) == Name)
      返回 i;
  }
  返回 e;
}

static StringRef EqValue = "=<value>";
static StringRef EmptyOption = "<empty>";
static StringRef OptionPrefix = "    =";
static size_t getOptionPrefixesSize() {
  返回 OptionPrefix.size() + ArgHelpPrefix.size();
}

static bool shouldPrintOption(StringRef Name, StringRef Description,
                              const Option &O) {
  返回 O.getValueExpectedFlag() != ValueOptional || !Name.empty() ||
         !Description.empty();
}

// Return the width of the option tag for printing...
size_t generic_parser_base::getOptionWidth(const Option &O) const {
  如果 (O.hasArgStr()) {
    size_t Size =
        argPlusPrefixesSize(O.ArgStr) + EqValue.size();
    为 (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef Name = getOption(i);
      如果 (!shouldPrintOption(Name, getDescription(i), O))
        continue;
      size_t NameSize = Name.empty() ? EmptyOption.size() : Name.size();
      Size = std::max(Size, NameSize + getOptionPrefixesSize());
    }
    返回 Size;
  } 否则 {
    size_t BaseSize = 0;
    为 (unsigned i = 0, e = getNumOptions(); i != e; ++i)
      BaseSize = std::max(BaseSize, getOption(i).size() + 8);
    返回 BaseSize;
  }
}

// printOptionInfo - Print out information about this option.  The
// to-be-maintained width is specified.
//
void generic_parser_base::printOptionInfo(const Option &O,
                                          size_t GlobalWidth) const {
  如果 (O.hasArgStr()) {
    // When the value is optional, first print a line just describing the
    // option without values.
    如果 (O.getValueExpectedFlag() == ValueOptional) {
      为 (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
        如果 (getOption(i).empty()) {
          outs() << PrintArg(O.ArgStr);
          Option::printHelpStr(O.HelpStr, GlobalWidth,
                               argPlusPrefixesSize(O.ArgStr));
          break;
        }
      }
    }

    outs() << PrintArg(O.ArgStr) << EqValue;
    Option::printHelpStr(O.HelpStr, GlobalWidth,
                         EqValue.size() +
                             argPlusPrefixesSize(O.ArgStr));
    为 (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
      StringRef OptionName = getOption(i);
      StringRef Description = getDescription(i);
      如果 (!shouldPrintOption(OptionName, Description, O))
        continue;
      size_t FirstLineIndent = OptionName.size() + getOptionPrefixesSize();
      outs() << OptionPrefix << OptionName;
      如果 (OptionName.empty()) {
        outs() << EmptyOption;
        assert(FirstLineIndent >= EmptyOption.size());
        FirstLineIndent += EmptyOption.size();
      }
      如果 (!Description.empty())
        Option::printEnumValHelpStr(Description, GlobalWidth, FirstLineIndent);
      否则
        outs() << '\n';
    }
  } 否则 {
    如果 (!O.HelpStr.empty())
      outs() << "  " << O.HelpStr << '\n';
    为 (unsigned i = 0, e = getNumOptions(); i != e; ++i) {
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
  为 (unsigned i = 0; i != NumOpts; ++i) {
    如果 (!Value.compare(getOptionValue(i)))
      continue;

    outs() << "= " << getOption(i);
    size_t L = getOption(i).size();
    size_t NumSpaces = MaxOptWidth > L ? MaxOptWidth - L : 0;
    outs().indent(NumSpaces) << " (default: ";
    为 (unsigned j = 0; j != NumOpts; ++j) {
      如果 (!Default.compare(getOptionValue(j)))
        continue;
      outs() << getOption(j);
      break;
    }
    outs() << ")\n";
    返回;
  }
  outs() << "= *unknown option value*\n";
}

// printOptionDiff - Specializations for printing basic value types.
//
namespace llvm {
namespace cl {
static raw_ostream &operator<<(raw_ostream &OS, boolOrDefault V) {
  返回 OS << static_cast<整数>(V);
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
    如果 (D.hasValue())                                                          \
      outs() << D.getValue();                                                  \
    否则                                                                       \
      outs() << "*no default*";                                                \
    outs() << ")\n";                                                           \
  }

PRINT_OPT_DIFF(bool)
PRINT_OPT_DIFF(boolOrDefault)
PRINT_OPT_DIFF(整数)
PRINT_OPT_DIFF(long)
PRINT_OPT_DIFF(long long)
PRINT_OPT_DIFF(unsigned)
PRINT_OPT_DIFF(unsigned long)
PRINT_OPT_DIFF(unsigned long long)
PRINT_OPT_DIFF(double)
PRINT_OPT_DIFF(float)
PRINT_OPT_DIFF(char)
PRINT_OPT_DIFF(ElementCount)

void parser<std::string>::printOptionDiff(const Option &O, StringRef V,
                                          const OptionValue<std::string> &D,
                                          size_t GlobalWidth) const {
  printOptionName(O, GlobalWidth);
  outs() << "= " << V;
  size_t NumSpaces = MaxOptWidth > V.size() ? MaxOptWidth - V.size() : 0;
  outs().indent(NumSpaces) << " (default: ";
  如果 (D.hasValue())
    outs() << D.getValue();
  否则
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
  如果 (D.hasValue() && D.getValue().has_value())
    outs() << D.getValue();
  否则
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

static 整数 OptNameCompare(const std::pair<const char *, Option *> *LHS,
                          const std::pair<const char *, Option *> *RHS) {
  返回 strcmp(LHS->first, RHS->first);
}

static 整数 SubNameCompare(const std::pair<const char *, SubCommand *> *LHS,
                          const std::pair<const char *, SubCommand *> *RHS) {
  返回 strcmp(LHS->first, RHS->first);
}

// Copy Options into a vector so we can sort them as we like.
static void sortOpts(OptionsMapTy &OptMap,
                     SmallVectorImpl<std::pair<const char *, Option *>> &Opts,
                     bool ShowHidden) {
  SmallPtrSet<Option *, 32> OptionSet; // Duplicate option detection.

  为 (auto I = OptMap.begin(), E = OptMap.end(); I != E; ++I) {
    // Ignore really-hidden options.
    如果 (I->second->getOptionHiddenFlag() == ReallyHidden)
      continue;

    // Unless showhidden is set, ignore hidden flags.
    如果 (I->second->getOptionHiddenFlag() == Hidden && !ShowHidden)
      continue;

    // If we've already seen this option, don't add it to the list again.
    如果 (!OptionSet.insert(I->second).second)
      continue;

    Opts.push_back(
        std::pair<const char *, Option *>(I->first.data(), I->second));
  }

  // Sort the options list alphabetically.
  array_pod_sort(Opts.begin(), Opts.end(), OptNameCompare);
}

static void
sortSubCommands(const SmallPtrSetImpl<SubCommand *> &SubMap,
                SmallVectorImpl<std::pair<const char *, SubCommand *>> &Subs) {
  为 (auto *S : SubMap) {
    如果 (S->getName().empty())
      continue;
    Subs.push_back(std::make_pair(S->getName().data(), S));
  }
  array_pod_sort(Subs.begin(), Subs.end(), SubNameCompare);
}

namespace {

class HelpPrinter {
protected:
  const bool ShowHidden;
  using StrOptionPairVector =
      SmallVector<std::pair<const char *, Option *>, 128>;
  using StrSubCommandPairVector =
      SmallVector<std::pair<const char *, SubCommand *>, 128>;
  // Print the options. Opts is assumed to be alphabetically sorted.
  virtual void printOptions(StrOptionPairVector &Opts, size_t MaxArgLen) {
    为 (const auto &Opt : Opts)
      Opt.second->printOptionInfo(MaxArgLen);
  }

  void printSubCommands(StrSubCommandPairVector &Subs, size_t MaxSubLen) {
    为 (const auto &S : Subs) {
      outs() << "  " << S.first;
      如果 (!S.second->getDescription().empty()) {
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
    如果 (!Value)
      返回;
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

    如果 (!globalParser().ProgramOverview.empty())
      outs() << "OVERVIEW: " << globalParser().ProgramOverview << "\n";

    如果 (Sub == &SubCommand::getTopLevel()) {
      outs() << "USAGE: " << globalParser().ProgramName;
      如果 (!Subs.empty())
        outs() << " [subcommand]";
      outs() << " [options]";
    } 否则 {
      如果 (!Sub->getDescription().empty()) {
        outs() << "SUBCOMMAND '" << Sub->getName()
               << "': " << Sub->getDescription() << "\n\n";
      }
      outs() << "USAGE: " << globalParser().ProgramName << " " << Sub->getName()
             << " [options]";
    }

    为 (auto *Opt : PositionalOpts) {
      如果 (Opt->hasArgStr())
        outs() << " --" << Opt->ArgStr;
      outs() << " " << Opt->HelpStr;
    }

    // Print the consume after option info if it exists...
    如果 (ConsumeAfterOpt)
      outs() << " " << ConsumeAfterOpt->HelpStr;

    如果 (Sub == &SubCommand::getTopLevel() && !Subs.empty()) {
      // Compute the maximum subcommand length...
      size_t MaxSubLen = 0;
      为 (const auto &Sub : Subs)
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
    为 (const auto &Opt : Opts)
      MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

    SmallVector<std::pair<std::string, StringRef>, 0> LibraryOpts;
    如果 (ShowHidden)
      为 (LibraryOptions *L : globalParser().Libraries)
        L->forEachOption(
            [&](StringRef Spelling, StringRef MetaVar, StringRef Help) {
              如果 (!Help.empty())
                LibraryOpts.emplace_back((Spelling + MetaVar).str(), Help);
            });
    llvm::sort(LibraryOpts);
    为 (const auto &[Name, Help] : LibraryOpts)
      MaxArgLen = std::max(MaxArgLen, argPlusPrefixesSize(Name));

    outs() << "OPTIONS:\n";
    printOptions(Opts, MaxArgLen);

    如果 (!LibraryOpts.empty())
      outs() << "\nLibrary options:\n\n";
    为 (const auto &[Name, Help] : LibraryOpts) {
      outs() << PrintArg(Name);
      Option::printHelpStr(Help, MaxArgLen, argPlusPrefixesSize(Name));
    }

    // Print any extra help the user has declared.
    为 (const auto &I : globalParser().MoreHelp)
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
  static 整数 OptionCategoryCompare(OptionCategory *const *A,
                                   OptionCategory *const *B) {
    返回 (*A)->getName().compare((*B)->getName());
  }

  // Make sure we inherit our base class's operator=()
  using HelpPrinter::operator=;

protected:
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
    为 (const auto &I : Opts) {
      Option *Opt = I.second;
      为 (OptionCategory *Cat : Opt->Categories) {
        assert(llvm::is_contained(SortedCategories, Cat) &&
               "Option has an unregistered category");
        CategorizedOptions[Cat].push_back(Opt);
      }
    }

    // Now do printing.
    为 (OptionCategory *Category : SortedCategories) {
      // Hide empty categories for --help, but show for --help-hidden.
      const auto &CategoryOptions = CategorizedOptions[Category];
      如果 (CategoryOptions.empty())
        continue;

      // Print category information.
      outs() << "\n";
      outs() << Category->getName() << ":\n";

      // Check if description is set.
      如果 (!Category->getDescription().empty())
        outs() << Category->getDescription() << "\n\n";
      否则
        outs() << "\n";

      // Loop over the options in the category and print.
      为 (const Option *Opt : CategoryOptions)
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

#如果 defined(__GNUC__)
// GCC and GCC-compatible compilers define __OPTIMIZE__ when optimizations are
// enabled.
# 如果 defined(__OPTIMIZE__)
#  define LLVM_IS_DEBUG_BUILD 0
# 否则
#  define LLVM_IS_DEBUG_BUILD 1
# endif
#elif defined(_MSC_VER)
// MSVC doesn't have a predefined macro indicating if optimizations are enabled.
// Use _DEBUG instead. This macro actually corresponds to the choice between
// debug and release CRTs, but it is a reasonable proxy.
# 如果 defined(_DEBUG)
#  define LLVM_IS_DEBUG_BUILD 1
# 否则
#  define LLVM_IS_DEBUG_BUILD 0
# endif
#否则
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
#否则
    OS << "LLVM (http://llvm.org/):\n  ";
#endif
    OS << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n  ";
#如果 LLVM_IS_DEBUG_BUILD
    OS << "DEBUG build";
#否则
    OS << "Optimized build";
#endif
#ifndef NDEBUG
    OS << " with assertions";
#endif
    OS << ".\n";

    // Iterate over any registered extra printers and call them to add further
    // information.
    如果 (!ExtraPrinters.empty()) {
      为 (const auto &I : ExtraPrinters)
        I(outs());
    }
  }
  void operator=(bool OptionWasSpecified);
};

struct CommandLineCommonOptions {
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
  返回 GeneralCategory;
}

void VersionPrinter::operator=(bool OptionWasSpecified) {
  如果 (!OptionWasSpecified)
    返回;

  如果 (CommonOptions->OverrideVersionPrinter != nullptr) {
    CommonOptions->OverrideVersionPrinter(outs());
    exit(0);
  }
  print(CommonOptions->ExtraVersionPrinters);

  exit(0);
}

void HelpPrinterWrapper::operator=(bool Value) {
  如果 (!Value)
    返回;

  // Decide which printer to invoke. If more than one option category is
  // registered then it is useful to show the categorized help instead of
  // uncategorized help.
  如果 (globalParser().RegisteredOptionCategories.size() > 1) {
    // unhide --help-list option so user can have uncategorized output if they
    // want it.
    CommonOptions->HLOp.setHiddenFlag(NotHidden);

    CategorizedPrinter = true; // Invoke categorized printer
  } 否则 {
    UncategorizedPrinter = true; // Invoke uncategorized printer
  }
}

// Print the value of each option.
void cl::PrintOptionValues() { globalParser().printOptionValues(); }

void CommandLineParser::printOptionValues() {
  如果 (!CommonOptions->PrintOptions && !CommonOptions->PrintAllOptions)
    返回;

  SmallVector<std::pair<const char *, Option *>, 128> Opts;
  sortOpts(ActiveSubCommand->OptionsMap, Opts, /*ShowHidden*/ true);

  // Compute the maximum argument length...
  size_t MaxArgLen = 0;
  为 (const auto &Opt : Opts)
    MaxArgLen = std::max(MaxArgLen, Opt.second->getOptionWidth());

  为 (const auto &Opt : Opts)
    Opt.second->printOptionValue(MaxArgLen, CommonOptions->PrintAllOptions);
}

// Utility function for printing the help message.
void cl::PrintHelpMessage(bool Hidden, bool Categorized) {
  如果 (!Hidden && !Categorized)
    CommonOptions->UncategorizedNormalPrinter.printHelp();
  否则 如果 (!Hidden && Categorized)
    CommonOptions->CategorizedNormalPrinter.printHelp();
  否则 如果 (Hidden && !Categorized)
    CommonOptions->UncategorizedHiddenPrinter.printHelp();
  否则
    CommonOptions->CategorizedHiddenPrinter.printHelp();
}

ArrayRef<StringRef> cl::getCompilerBuildConfig() {
  static const StringRef Config[] = {
      // Placeholder to ensure the array always has elements, since it's an
      // error to have a zero-sized array. Slice this off before returning.
      "",
  // Actual compiler build config feature list:
#如果 LLVM_IS_DEBUG_BUILD
      "+unoptimized",
#endif
#ifndef NDEBUG
      "+assertions",
#endif
#ifdef EXPENSIVE_CHECKS
      "+expensive-checks",
#endif
#如果 __has_feature(address_sanitizer)
      "+asan",
#endif
#如果 __has_feature(dataflow_sanitizer)
      "+dfsan",
#endif
#如果 __has_feature(hwaddress_sanitizer)
      "+hwasan",
#endif
#如果 __has_feature(memory_sanitizer)
      "+msan",
#endif
#如果 __has_feature(thread_sanitizer)
      "+tsan",
#endif
#如果 __has_feature(undefined_behavior_sanitizer)
      "+ubsan",
#endif
#ifdef LLVM_INTEGRATED_CRT_ALLOC
      "+alloc:" LLVM_INTEGRATED_CRT_ALLOC,
#endif
  };
  返回 ArrayRef(Config).drop_front(1);
}

// Utility function for printing the build config.
void cl::printBuildConfig(raw_ostream &OS) {
#如果 LLVM_VERSION_PRINTER_SHOW_BUILD_CONFIG
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
  返回 Sub.OptionsMap;
}

iterator_range<SmallPtrSet<SubCommand *, 4>::iterator>
cl::getRegisteredSubcommands() {
  返回 globalParser().getRegisteredSubcommands();
}

void cl::HideUnrelatedOptions(cl::OptionCategory &Category, SubCommand &Sub) {
  initCommonOptions();
  为 (auto &I : Sub.OptionsMap) {
    bool Unrelated = true;
    为 (auto &Cat : I.second->Categories) {
      如果 (Cat == &Category || Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    如果 (Unrelated)
      I.second->setHiddenFlag(cl::ReallyHidden);
  }
}

void cl::HideUnrelatedOptions(ArrayRef<const cl::OptionCategory *> Categories,
                              SubCommand &Sub) {
  initCommonOptions();
  为 (auto &I : Sub.OptionsMap) {
    bool Unrelated = true;
    为 (auto &Cat : I.second->Categories) {
      如果 (is_contained(Categories, Cat) ||
          Cat == &CommonOptions->GenericCategory)
        Unrelated = false;
    }
    如果 (Unrelated)
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
  ArrayRef<const char *> Args = Other.SavedArgs;
  为 (size_t I = 0, E = Args.size(); I != E;) {
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
                                   ArrayRef<const char *> Args,
                                   unsigned &Consumed) {
  auto [It, Inserted] = Values.try_emplace(L.getDefault());
  如果 (Inserted)
    It->second = L.copyDefault(Alloc);
  返回 parseLibraryOption(L, It->second, Args, Consumed, Alloc, &SavedArgs);
}

void cl::ResetAllOptionOccurrences() {
  globalParser().ResetAllOptionOccurrences();
}

void LLVMParseCommandLineOptions(整数 argc, const char *const *argv,
                                 const char *Overview) {
  llvm::cl::ParseCommandLineOptions(argc, argv, StringRef(Overview),
                                    &llvm::nulls());
}
