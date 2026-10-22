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

#شامل "llvm/Support/Debug.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/circular_raw_ostream.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <utility>

#شامل "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using فضاینام llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
ایستا std::pair<std::string, std::optional<صحیح>>
parseDebugType(StringRef DbgType) {
  std::optional<صحیح> Level;
  size_t ColonPos = DbgType.find(':');
  اگر (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    اگر (LevelStr.empty())
      Level = 0;
    وگرنه {
      صحیح parsedLevel;
      اگر (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  بازگشت std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
فضاینام llvm {
/// Exported boolean set by the -debug option.
منطقی DebugFlag = نادرست;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
ایستا ManagedStatic<std::vector<std::pair<std::string, std::optional<صحیح>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
منطقی isCurrentDebugType(ثابت نویسه *DebugType, صحیح Level) {
  اگر (CurrentDebugType->empty())
    بازگشت درست;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  منطقی HasEnabledDebugType = نادرست;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  برای (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_مقدار_2() || D.second.مقدار_2() > 0);
    اگر (D.first != DebugType)
      ادامه;
    اگر (!D.second.has_مقدار_2())
      بازگشت درست;
    بازگشت D.second >= Level;
  }
  بازگشت !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
تهی setCurrentDebugTypes(ثابت نویسه **Types, unsigned شمار_2);

تهی setCurrentDebugType(ثابت نویسه *Type) {
  setCurrentDebugTypes(&Type, 1);
}

تهی setCurrentDebugTypes(ثابت نویسه **Types, unsigned شمار_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(شمار_2);
  برای (ثابت نویسه *Type : ArrayRef(Types, شمار_2))
    CurrentDebugType->افزودن_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

فضاینام {
ساختار CreateDebug {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<منطقی, درست>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
ساختار CreateDebugBufferSize {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<unsigned>(
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
ایستا ManagedStatic<cl::opt<منطقی, درست>, CreateDebug> Debug;
ایستا ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

فضاینام {

ساختار DebugOnlyOpt {
  تهی operator=(ثابت std::string &Val) ثابت {
    اگر (Val.empty())
      بازگشت;
    DebugFlag = درست;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, نادرست);
    برای (auto DbgType : DbgTypes)
      CurrentDebugType->افزودن_back(parseDebugType(DbgType));
  }
};
} // namespace

ایستا DebugOnlyOpt DebugOnlyOptLoc;

فضاینام {
ساختار CreateDebugOnly {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<DebugOnlyOpt, درست, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::مقدار_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::مقدار_Required);
  }
};
} // namespace

ایستا ManagedStatic<cl::opt<DebugOnlyOpt, درست, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

تهی llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

ایستا تهی printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
ایستا تهی debug_user_sig_handler(تهی *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  ایستا ساختار dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      اگر (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  بازگشت thestrm.strm;
}

تهی llvm::printDebugLog() {
  اگر (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#وگرنه
// Avoid "has no symbols" warning.
فضاینام llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    بازگشت errs();
  }
}
تهی llvm::initDebugOptions() {}

تهی llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
منطقی llvm::EnableDebugBuffering = نادرست;
