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

#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/CommandLine.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/circular_raw_ostream.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <utility>

#અન્તર્ભાવ "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using નામાકાશ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
સ્થિર std::pair<std::string, std::optional<પૂર્ણાંક>>
parseDebugType(StringRef DbgType) {
  std::optional<પૂર્ણાંક> Level;
  size_t ColonPos = DbgType.find(':');
  જો (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    જો (LevelStr.empty())
      Level = 0;
    નહીંતર {
      પૂર્ણાંક parsedLevel;
      જો (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  પાછા std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
નામાકાશ llvm {
/// Exported boolean set by the -debug option.
તાર્કિક DebugFlag = ખોટું;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
સ્થિર ManagedStatic<std::vector<std::pair<std::string, std::optional<પૂર્ણાંક>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
તાર્કિક isCurrentDebugType(અચલ અક્ષર *DebugType, પૂર્ણાંક Level) {
  જો (CurrentDebugType->empty())
    પાછા સાચું;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  તાર્કિક HasEnabledDebugType = ખોટું;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  માટે (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_કિંમત_2() || D.second.કિંમત_2() > 0);
    જો (D.first != DebugType)
      ચાલુ;
    જો (!D.second.has_કિંમત_2())
      પાછા સાચું;
    પાછા D.second >= Level;
  }
  પાછા !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
શૂન્ય setCurrentDebugTypes(અચલ અક્ષર **Types, unsigned ગણતરી_2);

શૂન્ય setCurrentDebugType(અચલ અક્ષર *Type) {
  setCurrentDebugTypes(&Type, 1);
}

શૂન્ય setCurrentDebugTypes(અચલ અક્ષર **Types, unsigned ગણતરી_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(ગણતરી_2);
  માટે (અચલ અક્ષર *Type : ArrayRef(Types, ગણતરી_2))
    CurrentDebugType->ધકેલો_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

નામાકાશ {
સંરચના CreateDebug {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<તાર્કિક, સાચું>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
સંરચના CreateDebugBufferSize {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<unsigned>(
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
સ્થિર ManagedStatic<cl::opt<તાર્કિક, સાચું>, CreateDebug> Debug;
સ્થિર ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

નામાકાશ {

સંરચના DebugOnlyOpt {
  શૂન્ય operator=(અચલ std::string &Val) અચલ {
    જો (Val.empty())
      પાછા;
    DebugFlag = સાચું;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, ખોટું);
    માટે (auto DbgType : DbgTypes)
      CurrentDebugType->ધકેલો_back(parseDebugType(DbgType));
  }
};
} // namespace

સ્થિર DebugOnlyOpt DebugOnlyOptLoc;

નામાકાશ {
સંરચના CreateDebugOnly {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<DebugOnlyOpt, સાચું, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::કિંમત_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::કિંમત_Required);
  }
};
} // namespace

સ્થિર ManagedStatic<cl::opt<DebugOnlyOpt, સાચું, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

શૂન્ય llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

સ્થિર શૂન્ય printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
સ્થિર શૂન્ય debug_user_sig_handler(શૂન્ય *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  સ્થિર સંરચના dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      જો (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  પાછા thestrm.strm;
}

શૂન્ય llvm::printDebugLog() {
  જો (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#નહીંતર
// Avoid "has no symbols" warning.
નામાકાશ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    પાછા errs();
  }
}
શૂન્ય llvm::initDebugOptions() {}

શૂન્ય llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
તાર્કિક llvm::EnableDebugBuffering = ખોટું;
