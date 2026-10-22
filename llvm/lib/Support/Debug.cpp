//===-- Debug.cpp - An easy way to add debug output to your code ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a handy way of adding debugging information to your
// code, without it being enabled all of the time, and without having to add
// command line options to enable it.
//
// In particular, just wrap your code with the LLVM_DEBUG() macro, and it will
// be enabled automatically if you specify '-debug' on the command-line.
// Alternatively, you can also use the SET_DEBUG_TYPE("foo") macro to specify
// that your debug code belongs to class "foo".  Then, on the command line, you
// can specify '-debug-only=foo' to enable JUST the debug information for the
// foo class.
//
// When compiling without assertions, the -debug-* options and all code in
// LLVM_DEBUG() statements disappears, so it does not affect the runtime of the
// code.
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/CommandLine.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/circular_raw_ostream.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <utility>

#അന്തര്ഭാവ "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using നാമാകാശ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
സ്ഥിതം std::pair<std::string, std::optional<പൂർണ്ണസംഖ്യ>>
parseDebugType(StringRef DbgType) {
  std::optional<പൂർണ്ണസംഖ്യ> Level;
  size_t ColonPos = DbgType.find(':');
  എങ്കിൽ (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    എങ്കിൽ (LevelStr.empty())
      Level = 0;
    അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ parsedLevel;
      എങ്കിൽ (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  തിരികെ std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
നാമാകാശ llvm {
/// Exported boolean set by the -debug option.
തർക്കം DebugFlag = അസത്യം;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
സ്ഥിതം ManagedStatic<std::vector<std::pair<std::string, std::optional<പൂർണ്ണസംഖ്യ>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
തർക്കം isCurrentDebugType(സ്ഥിരം അക്ഷരം *DebugType, പൂർണ്ണസംഖ്യ Level) {
  എങ്കിൽ (CurrentDebugType->empty())
    തിരികെ സത്യം;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  തർക്കം HasEnabledDebugType = അസത്യം;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  വേണ്ടി (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_മൂല്യം_2() || D.second.മൂല്യം_2() > 0);
    എങ്കിൽ (D.first != DebugType)
      തുടരുക;
    എങ്കിൽ (!D.second.has_മൂല്യം_2())
      തിരികെ സത്യം;
    തിരികെ D.second >= Level;
  }
  തിരികെ !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
ശൂന്യം setCurrentDebugTypes(സ്ഥിരം അക്ഷരം **Types, unsigned എണ്ണം_2);

ശൂന്യം setCurrentDebugType(സ്ഥിരം അക്ഷരം *Type) {
  setCurrentDebugTypes(&Type, 1);
}

ശൂന്യം setCurrentDebugTypes(സ്ഥിരം അക്ഷരം **Types, unsigned എണ്ണം_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(എണ്ണം_2);
  വേണ്ടി (സ്ഥിരം അക്ഷരം *Type : ArrayRef(Types, എണ്ണം_2))
    CurrentDebugType->തള്ളുക_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

നാമാകാശ {
ഘടന CreateDebug {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<തർക്കം, സത്യം>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
ഘടന CreateDebugBufferSize {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<unsigned>(
        "debug-buffer-size",
        cl::desc("Buffer the last N characters of debug output "
                 "until program termination. "
                 "[default 0 -- immediate print-out]"),
        cl::Hidden, cl::init(0));
  }
};
} // namespace

// -debug - Command line option to enable the DEBUG statements in the passes.
// This flag may only be enabled in debug builds.
സ്ഥിതം ManagedStatic<cl::opt<തർക്കം, സത്യം>, CreateDebug> Debug;
സ്ഥിതം ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

നാമാകാശ {

ഘടന DebugOnlyOpt {
  ശൂന്യം operator=(സ്ഥിരം std::string &Val) സ്ഥിരം {
    എങ്കിൽ (Val.empty())
      തിരികെ;
    DebugFlag = സത്യം;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, അസത്യം);
    വേണ്ടി (auto DbgType : DbgTypes)
      CurrentDebugType->തള്ളുക_back(parseDebugType(DbgType));
  }
};
} // namespace

സ്ഥിതം DebugOnlyOpt DebugOnlyOptLoc;

നാമാകാശ {
ഘടന CreateDebugOnly {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<DebugOnlyOpt, സത്യം, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::മൂല്യം_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::മൂല്യം_Required);
  }
};
} // namespace

സ്ഥിതം ManagedStatic<cl::opt<DebugOnlyOpt, സത്യം, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

ശൂന്യം llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

സ്ഥിതം ശൂന്യം printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
സ്ഥിതം ശൂന്യം debug_user_sig_handler(ശൂന്യം *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  സ്ഥിതം ഘടന dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      എങ്കിൽ (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  തിരികെ thestrm.strm;
}

ശൂന്യം llvm::printDebugLog() {
  എങ്കിൽ (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#അല്ലെങ്കിൽ
// Avoid "has no symbols" warning.
നാമാകാശ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    തിരികെ errs();
  }
}
ശൂന്യം llvm::initDebugOptions() {}

ശൂന്യം llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
തർക്കം llvm::EnableDebugBuffering = അസത്യം;
