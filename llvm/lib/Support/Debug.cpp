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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/circular_raw_ostream.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <utility>

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು isCurrentDebugType
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಗಣ_Current_Debug_Type
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಗಣ_Current_Debug_Types

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
ಸ್ಥಿರ std::pair<std::string, std::optional<ಪೂರ್ಣಾಂಕ>>
parseDebugType(StringRef DbgType) {
  std::optional<ಪೂರ್ಣಾಂಕ> Level;
  size_t ColonPos = DbgType.ಕಂಡುಹಿಡಿ(':');
  ಆದರೆ (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    ಆದರೆ (LevelStr.ಖಾಲಿಗಳು())
      Level = 0;
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ parsedLevel;
      ಆದರೆ (ಗಮ್ಯ_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  ಹಿಂತಿರುಗಿ std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
ನಾಮಾಕಾಶ llvm {
/// Exported boolean set by the -debug option.
ತಾರ್ಕಿಕ DebugFlag = ಸುಳ್ಳು;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
ಸ್ಥಿರ ManagedStatic<std::ಸದಿಶ<std::pair<std::string, std::optional<ಪೂರ್ಣಾಂಕ>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
ತಾರ್ಕಿಕ isCurrentDebugType(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *DebugType, ಪೂರ್ಣಾಂಕ Level) {
  ಆದರೆ (CurrentDebugType->ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  ತಾರ್ಕಿಕ HasEnabledDebugType = ಸುಳ್ಳು;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.ಎರಡನೇ_2.has_ಮೌಲ್ಯ_2() || D.ಎರಡನೇ_2.ಮೌಲ್ಯ_2() > 0);
    ಆದರೆ (D.ಮೊದಲ_2 != DebugType)
      ಮುಂದುವರಿ;
    ಆದರೆ (!D.ಎರಡನೇ_2.has_ಮೌಲ್ಯ_2())
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಹಿಂತಿರುಗಿ D.ಎರಡನೇ_2 >= Level;
  }
  ಹಿಂತಿರುಗಿ !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
ಶೂನ್ಯ ಗಣ_Current_Debug_Types(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **Types, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3);

ಶೂನ್ಯ ಗಣ_Current_Debug_Type(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Type) {
  ಗಣ_Current_Debug_Types(&Type, 1);
}

ಶೂನ್ಯ ಗಣ_Current_Debug_Types(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **Types, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(ಎಣಿಕೆ_3);
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Type : ArrayRef(Types, ಎಣಿಕೆ_3))
    CurrentDebugType->ತಳ್ಳು_ಹಿಂಭಾಗ(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG

ನಾಮಾಕಾಶ {
ರಚನೆ CreateDebug {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<ತಾರ್ಕಿಕ, ನಿಜ>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
ರಚನೆ Create_Debug_ಬಫರ್_ಗಾತ್ರ {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<ಚಿಹ್ನೆರಹಿತ>(
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
ಸ್ಥಿರ ManagedStatic<cl::opt<ತಾರ್ಕಿಕ, ನಿಜ>, CreateDebug> Debug;
ಸ್ಥಿರ ManagedStatic<cl::opt<ಚಿಹ್ನೆರಹಿತ>, Create_Debug_ಬಫರ್_ಗಾತ್ರ> Debug_ಬಫರ್_ಗಾತ್ರ;

ನಾಮಾಕಾಶ {

ರಚನೆ DebugOnlyOpt {
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ std::string &Val) ಸ್ಥಿರಾಂಕ {
    ಆದರೆ (Val.ಖಾಲಿಗಳು())
      ಹಿಂತಿರುಗಿ;
    DebugFlag = ನಿಜ;
    Small_ಸದಿಶ<StringRef, 8> DbgTypes;
    StringRef(Val).ವಿಭಜಿಸು(DbgTypes, ',', -1, ಸುಳ್ಳು);
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ DbgType : DbgTypes)
      CurrentDebugType->ತಳ್ಳು_ಹಿಂಭಾಗ(parseDebugType(DbgType));
  }
};
} // namespace

ಸ್ಥಿರ DebugOnlyOpt DebugOnlyOptLoc;

ನಾಮಾಕಾಶ {
ರಚನೆ CreateDebugOnly {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<DebugOnlyOpt, ನಿಜ, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::ಮೌಲ್ಯ_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::ಮೌಲ್ಯ_Required);
  }
};
} // namespace

ಸ್ಥಿರ ManagedStatic<cl::opt<DebugOnlyOpt, ನಿಜ, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

ಶೂನ್ಯ llvm::initDebugOptions() {
  *Debug;
  *Debug_ಬಫರ್_ಗಾತ್ರ;
  *DebugOnly;
}

ಸ್ಥಿರ ಶೂನ್ಯ printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flush_ಬಫರ್_With_Banner();
}

// Signal handlers - dump debug output on termination.
ಸ್ಥಿರ ಶೂನ್ಯ debug_user_sig_handler(ಶೂನ್ಯ *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  ಸ್ಥಿರ ರಚನೆ dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *Debug_ಬಫರ್_ಗಾತ್ರ) {
      ಆದರೆ (EnableDebugBuffering && DebugFlag && *Debug_ಬಫರ್_ಗಾತ್ರ != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::ಕೂಡಿಸು_Signal_Handler(&debug_user_sig_handler, ಶೂನ್ಯಸೂಚಕ);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  ಹಿಂತಿರುಗಿ thestrm.strm;
}

ಶೂನ್ಯ llvm::printDebugLog() {
  ಆದರೆ (EnableDebugBuffering && DebugFlag && *Debug_ಬಫರ್_ಗಾತ್ರ != 0)
    printDebugLogImpl();
}

#ಇಲ್ಲದಿದ್ದರೆ
// Avoid "has no symbols" warning.
ನಾಮಾಕಾಶ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    ಹಿಂತಿರುಗಿ errs();
  }
}
ಶೂನ್ಯ llvm::initDebugOptions() {}

ಶೂನ್ಯ llvm::printDebugLog() {}
#ಆದರೆ_ಅಂತ್ಯ

/// EnableDebugBuffering - Turn on signal handler installation.
///
ತಾರ್ಕಿಕ llvm::EnableDebugBuffering = ಸುಳ್ಳು;
