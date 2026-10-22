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

#تعریف_منسوخ isCurrentDebugType
#تعریف_منسوخ سیٹ_Current_Debug_Type
#تعریف_منسوخ سیٹ_Current_Debug_Types

استعمال نامخلا llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
ساکن std::pair<std::string, std::optional<صحیح>>
parseDebugType(StringRef DbgType) {
  std::optional<صحیح> Level;
  size_t ColonPos = DbgType.ڈھونڈو(':');
  اگر (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    اگر (LevelStr.خالی_خانے())
      Level = 0;
    ورنہ {
      صحیح parsedLevel;
      اگر (مقصد_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  واپس std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
نامخلا llvm {
/// Exported boolean set by the -debug option.
منطقی DebugFlag = جھوٹ;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
ساکن ManagedStatic<std::سمتیہ<std::pair<std::string, std::optional<صحیح>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
منطقی isCurrentDebugType(ثابت حرف *DebugType, صحیح Level) {
  اگر (CurrentDebugType->خالی_خانے())
    واپس سچ;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  منطقی HasEnabledDebugType = جھوٹ;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  برائے (خودکار &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.دوسرا_2.has_قدر_2() || D.دوسرا_2.قدر_2() > 0);
    اگر (D.پہلا_2 != DebugType)
      جاری;
    اگر (!D.دوسرا_2.has_قدر_2())
      واپس سچ;
    واپس D.دوسرا_2 >= Level;
  }
  واپس !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
خالی سیٹ_Current_Debug_Types(ثابت حرف **Types, غیرعلامتی گنتی_2);

خالی سیٹ_Current_Debug_Type(ثابت حرف *Type) {
  سیٹ_Current_Debug_Types(&Type, 1);
}

خالی سیٹ_Current_Debug_Types(ثابت حرف **Types, غیرعلامتی گنتی_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(گنتی_2);
  برائے (ثابت حرف *Type : ArrayRef(Types, گنتی_2))
    CurrentDebugType->ڈالو_عقب(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#اگر_غیر_تعریف_شدہ NDEBUG

نامخلا {
ساخت CreateDebug {
  ساکن خالی *call() {
    واپس نیا cl::opt<منطقی, سچ>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
ساخت Create_Debug_بفر_سائز {
  ساکن خالی *call() {
    واپس نیا cl::opt<غیرعلامتی>(
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
ساکن ManagedStatic<cl::opt<منطقی, سچ>, CreateDebug> Debug;
ساکن ManagedStatic<cl::opt<غیرعلامتی>, Create_Debug_بفر_سائز> Debug_بفر_سائز;

نامخلا {

ساخت DebugOnlyOpt {
  خالی عامل=(ثابت std::string &Val) ثابت {
    اگر (Val.خالی_خانے())
      واپس;
    DebugFlag = سچ;
    Small_سمتیہ<StringRef, 8> DbgTypes;
    StringRef(Val).بانٹو(DbgTypes, ',', -1, جھوٹ);
    برائے (خودکار DbgType : DbgTypes)
      CurrentDebugType->ڈالو_عقب(parseDebugType(DbgType));
  }
};
} // namespace

ساکن DebugOnlyOpt DebugOnlyOptLoc;

نامخلا {
ساخت CreateDebugOnly {
  ساکن خالی *call() {
    واپس نیا cl::opt<DebugOnlyOpt, سچ, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::قدر_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::قدر_Required);
  }
};
} // namespace

ساکن ManagedStatic<cl::opt<DebugOnlyOpt, سچ, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

خالی llvm::initDebugOptions() {
  *Debug;
  *Debug_بفر_سائز;
  *DebugOnly;
}

ساکن خالی printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flush_بفر_With_Banner();
}

// Signal handlers - dump debug output on termination.
ساکن خالی debug_user_sig_handler(خالی *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  ساکن ساخت dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *Debug_بفر_سائز) {
      اگر (EnableDebugBuffering && DebugFlag && *Debug_بفر_سائز != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::جوڑو_Signal_Handler(&debug_user_sig_handler, خالی_اشاریہ);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  واپس thestrm.strm;
}

خالی llvm::printDebugLog() {
  اگر (EnableDebugBuffering && DebugFlag && *Debug_بفر_سائز != 0)
    printDebugLogImpl();
}

#ورنہ
// Avoid "has no symbols" warning.
نامخلا llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    واپس errs();
  }
}
خالی llvm::initDebugOptions() {}

خالی llvm::printDebugLog() {}
#اگر_ختم

/// EnableDebugBuffering - Turn on signal handler installation.
///
منطقی llvm::EnableDebugBuffering = جھوٹ;
