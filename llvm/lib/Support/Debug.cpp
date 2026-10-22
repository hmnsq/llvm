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

#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/circular_raw_ostream.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <utility>

#ضمن "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using نطاق llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
ساكن std::pair<std::string, std::optional<صحيح>>
parseDebugType(StringRef DbgType) {
  std::optional<صحيح> Level;
  size_t ColonPos = DbgType.find(':');
  ܐܢ (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    ܐܢ (LevelStr.empty())
      Level = 0;
    ܐܚܪܢܐ {
      صحيح parsedLevel;
      ܐܢ (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  ܦܢܐ std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
نطاق llvm {
/// Exported boolean set by the -debug option.
منطقي DebugFlag = ܕܓܠܐ;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
ساكن ManagedStatic<std::vector<std::pair<std::string, std::optional<صحيح>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
منطقي isCurrentDebugType(ثابت حرف *DebugType, صحيح Level) {
  ܐܢ (CurrentDebugType->empty())
    ܦܢܐ ܫܪܪܐ;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  منطقي HasEnabledDebugType = ܕܓܠܐ;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  لكل (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_value() || D.second.value() > 0);
    ܐܢ (D.first != DebugType)
      استمر;
    ܐܢ (!D.second.has_value())
      ܦܢܐ ܫܪܪܐ;
    ܦܢܐ D.second >= Level;
  }
  ܦܢܐ !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
فراغ setCurrentDebugTypes(ثابت حرف **Types, unsigned Count);

فراغ setCurrentDebugType(ثابت حرف *Type) {
  setCurrentDebugTypes(&Type, 1);
}

فراغ setCurrentDebugTypes(ثابت حرف **Types, unsigned Count) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(Count);
  لكل (ثابت حرف *Type : ArrayRef(Types, Count))
    CurrentDebugType->push_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

نطاق {
بنية CreateDebug {
  ساكن فراغ *call() {
    ܦܢܐ جديد cl::opt<منطقي, ܫܪܪܐ>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
بنية CreateDebugBufferSize {
  ساكن فراغ *call() {
    ܦܢܐ جديد cl::opt<unsigned>(
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
ساكن ManagedStatic<cl::opt<منطقي, ܫܪܪܐ>, CreateDebug> Debug;
ساكن ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

نطاق {

بنية DebugOnlyOpt {
  فراغ operator=(ثابت std::string &Val) ثابت {
    ܐܢ (Val.empty())
      ܦܢܐ;
    DebugFlag = ܫܪܪܐ;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, ܕܓܠܐ);
    لكل (auto DbgType : DbgTypes)
      CurrentDebugType->push_back(parseDebugType(DbgType));
  }
};
} // namespace

ساكن DebugOnlyOpt DebugOnlyOptLoc;

نطاق {
بنية CreateDebugOnly {
  ساكن فراغ *call() {
    ܦܢܐ جديد cl::opt<DebugOnlyOpt, ܫܪܪܐ, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::value_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::ValueRequired);
  }
};
} // namespace

ساكن ManagedStatic<cl::opt<DebugOnlyOpt, ܫܪܪܐ, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

فراغ llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

ساكن فراغ printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
ساكن فراغ debug_user_sig_handler(فراغ *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  ساكن بنية dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      ܐܢ (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  ܦܢܐ thestrm.strm;
}

فراغ llvm::printDebugLog() {
  ܐܢ (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#ܐܚܪܢܐ
// Avoid "has no symbols" warning.
نطاق llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    ܦܢܐ errs();
  }
}
فراغ llvm::initDebugOptions() {}

فراغ llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
منطقي llvm::EnableDebugBuffering = ܕܓܠܐ;
