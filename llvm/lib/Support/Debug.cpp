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

#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/CommandLine.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/circular_raw_ostream.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <utility>

#அந்தர்பாவ "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using நாமாகாஶ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
நிலையான std::pair<std::string, std::optional<முழு>>
parseDebugType(StringRef DbgType) {
  std::optional<முழு> Level;
  size_t ColonPos = DbgType.find(':');
  எனில் (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    எனில் (LevelStr.empty())
      Level = 0;
    இல்லையெனில் {
      முழு parsedLevel;
      எனில் (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  திரும்பு std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
நாமாகாஶ llvm {
/// Exported boolean set by the -debug option.
தருக்க DebugFlag = பொய்;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
நிலையான ManagedStatic<std::vector<std::pair<std::string, std::optional<முழு>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
தருக்க isCurrentDebugType(மாறிலி எழுத்து *DebugType, முழு Level) {
  எனில் (CurrentDebugType->empty())
    திரும்பு உண்மை;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  தருக்க HasEnabledDebugType = பொய்;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  க்கு (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_மதிப்பு_2() || D.second.மதிப்பு_2() > 0);
    எனில் (D.first != DebugType)
      தொடர்;
    எனில் (!D.second.has_மதிப்பு_2())
      திரும்பு உண்மை;
    திரும்பு D.second >= Level;
  }
  திரும்பு !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
வெறுமை setCurrentDebugTypes(மாறிலி எழுத்து **Types, unsigned மொத்தம்_2);

வெறுமை setCurrentDebugType(மாறிலி எழுத்து *Type) {
  setCurrentDebugTypes(&Type, 1);
}

வெறுமை setCurrentDebugTypes(மாறிலி எழுத்து **Types, unsigned மொத்தம்_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(மொத்தம்_2);
  க்கு (மாறிலி எழுத்து *Type : ArrayRef(Types, மொத்தம்_2))
    CurrentDebugType->தள்ளு_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

நாமாகாஶ {
கட்டமைப்பு CreateDebug {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<தருக்க, உண்மை>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
கட்டமைப்பு CreateDebugBufferSize {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<unsigned>(
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
நிலையான ManagedStatic<cl::opt<தருக்க, உண்மை>, CreateDebug> Debug;
நிலையான ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

நாமாகாஶ {

கட்டமைப்பு DebugOnlyOpt {
  வெறுமை operator=(மாறிலி std::string &Val) மாறிலி {
    எனில் (Val.empty())
      திரும்பு;
    DebugFlag = உண்மை;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, பொய்);
    க்கு (auto DbgType : DbgTypes)
      CurrentDebugType->தள்ளு_back(parseDebugType(DbgType));
  }
};
} // namespace

நிலையான DebugOnlyOpt DebugOnlyOptLoc;

நாமாகாஶ {
கட்டமைப்பு CreateDebugOnly {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<DebugOnlyOpt, உண்மை, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::மதிப்பு_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::மதிப்பு_Required);
  }
};
} // namespace

நிலையான ManagedStatic<cl::opt<DebugOnlyOpt, உண்மை, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

வெறுமை llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

நிலையான வெறுமை printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
நிலையான வெறுமை debug_user_sig_handler(வெறுமை *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  நிலையான கட்டமைப்பு dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      எனில் (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  திரும்பு thestrm.strm;
}

வெறுமை llvm::printDebugLog() {
  எனில் (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#இல்லையெனில்
// Avoid "has no symbols" warning.
நாமாகாஶ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    திரும்பு errs();
  }
}
வெறுமை llvm::initDebugOptions() {}

வெறுமை llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
தருக்க llvm::EnableDebugBuffering = பொய்;
