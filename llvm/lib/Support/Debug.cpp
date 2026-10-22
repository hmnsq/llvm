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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/circular_raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <utility>

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ isCurrentDebugType
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸੈੱਟ_Current_Debug_Type
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸੈੱਟ_Current_Debug_Types

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
ਸਥਿਰ std::pair<std::string, std::optional<ਪੂਰਨ>>
parseDebugType(StringRef DbgType) {
  std::optional<ਪੂਰਨ> Level;
  size_t ColonPos = DbgType.ਲੱਭੋ(':');
  ਜੇ (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    ਜੇ (LevelStr.ਖਾਲੀ_ਥਾਵਾਂ())
      Level = 0;
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ parsedLevel;
      ਜੇ (ਮੰਜ਼ਿਲ_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  ਵਾਪਸ std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
ਨਾਮਾਕਾਸ਼ llvm {
/// Exported boolean set by the -debug option.
ਤਾਰਕਿਕ DebugFlag = ਝੂਠ;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
ਸਥਿਰ ManagedStatic<std::ਸਦਿਸ਼<std::pair<std::string, std::optional<ਪੂਰਨ>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
ਤਾਰਕਿਕ isCurrentDebugType(ਅਚਲ ਅੱਖਰ *DebugType, ਪੂਰਨ Level) {
  ਜੇ (CurrentDebugType->ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  ਤਾਰਕਿਕ HasEnabledDebugType = ਝੂਠ;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  ਲਈ (ਸਵੈਚਾਲਿਤ &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.ਦੂਜਾ_2.has_ਮੁੱਲ_2() || D.ਦੂਜਾ_2.ਮੁੱਲ_2() > 0);
    ਜੇ (D.ਪਹਿਲਾ_2 != DebugType)
      ਜਾਰੀ;
    ਜੇ (!D.ਦੂਜਾ_2.has_ਮੁੱਲ_2())
      ਵਾਪਸ ਸੱਚ;
    ਵਾਪਸ D.ਦੂਜਾ_2 >= Level;
  }
  ਵਾਪਸ !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
ਖਾਲੀ ਸੈੱਟ_Current_Debug_Types(ਅਚਲ ਅੱਖਰ **Types, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3);

ਖਾਲੀ ਸੈੱਟ_Current_Debug_Type(ਅਚਲ ਅੱਖਰ *Type) {
  ਸੈੱਟ_Current_Debug_Types(&Type, 1);
}

ਖਾਲੀ ਸੈੱਟ_Current_Debug_Types(ਅਚਲ ਅੱਖਰ **Types, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(ਗਿਣਤੀ_3);
  ਲਈ (ਅਚਲ ਅੱਖਰ *Type : ArrayRef(Types, ਗਿਣਤੀ_3))
    CurrentDebugType->ਧੱਕੋ_ਪਿੱਠ(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ CreateDebug {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<ਤਾਰਕਿਕ, ਸੱਚ>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
ਬਣਤਰ Create_Debug_ਬਫਰ_ਮਾਪ {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<ਬਿਨਾਨਿਸ਼ਾਨੀ>(
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
ਸਥਿਰ ManagedStatic<cl::opt<ਤਾਰਕਿਕ, ਸੱਚ>, CreateDebug> Debug;
ਸਥਿਰ ManagedStatic<cl::opt<ਬਿਨਾਨਿਸ਼ਾਨੀ>, Create_Debug_ਬਫਰ_ਮਾਪ> Debug_ਬਫਰ_ਮਾਪ;

ਨਾਮਾਕਾਸ਼ {

ਬਣਤਰ DebugOnlyOpt {
  ਖਾਲੀ ਸੰਚਾਲਕ=(ਅਚਲ std::string &Val) ਅਚਲ {
    ਜੇ (Val.ਖਾਲੀ_ਥਾਵਾਂ())
      ਵਾਪਸ;
    DebugFlag = ਸੱਚ;
    Small_ਸਦਿਸ਼<StringRef, 8> DbgTypes;
    StringRef(Val).ਵੰਡੋ(DbgTypes, ',', -1, ਝੂਠ);
    ਲਈ (ਸਵੈਚਾਲਿਤ DbgType : DbgTypes)
      CurrentDebugType->ਧੱਕੋ_ਪਿੱਠ(parseDebugType(DbgType));
  }
};
} // namespace

ਸਥਿਰ DebugOnlyOpt DebugOnlyOptLoc;

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ CreateDebugOnly {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<DebugOnlyOpt, ਸੱਚ, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::ਮੁੱਲ_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::ਮੁੱਲ_Required);
  }
};
} // namespace

ਸਥਿਰ ManagedStatic<cl::opt<DebugOnlyOpt, ਸੱਚ, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

ਖਾਲੀ llvm::initDebugOptions() {
  *Debug;
  *Debug_ਬਫਰ_ਮਾਪ;
  *DebugOnly;
}

ਸਥਿਰ ਖਾਲੀ printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flush_ਬਫਰ_With_Banner();
}

// Signal handlers - dump debug output on termination.
ਸਥਿਰ ਖਾਲੀ debug_user_sig_handler(ਖਾਲੀ *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  ਸਥਿਰ ਬਣਤਰ dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *Debug_ਬਫਰ_ਮਾਪ) {
      ਜੇ (EnableDebugBuffering && DebugFlag && *Debug_ਬਫਰ_ਮਾਪ != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::ਜੋੜੋ_Signal_Handler(&debug_user_sig_handler, ਖਾਲੀ_ਸੂਚਕ);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  ਵਾਪਸ thestrm.strm;
}

ਖਾਲੀ llvm::printDebugLog() {
  ਜੇ (EnableDebugBuffering && DebugFlag && *Debug_ਬਫਰ_ਮਾਪ != 0)
    printDebugLogImpl();
}

#ਨਹੀਂਤਾਂ
// Avoid "has no symbols" warning.
ਨਾਮਾਕਾਸ਼ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    ਵਾਪਸ errs();
  }
}
ਖਾਲੀ llvm::initDebugOptions() {}

ਖਾਲੀ llvm::printDebugLog() {}
#ਜੇ_ਅੰਤ

/// EnableDebugBuffering - Turn on signal handler installation.
///
ਤਾਰਕਿਕ llvm::EnableDebugBuffering = ਝੂਠ;
