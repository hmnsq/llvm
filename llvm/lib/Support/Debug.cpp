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

#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/circular_raw_ostream.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <utility>

#అన్తర్భావ "DebugOptions.h"

#నిర్వచనం_రద్దు isCurrentDebugType
#నిర్వచనం_రద్దు సమితి_Current_Debug_Type
#నిర్వచనం_రద్దు సమితి_Current_Debug_Types

ఉపయోగించి నామాకాశ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
స్థిర std::pair<std::string, std::optional<పూర్ణాంకం>>
parseDebugType(StringRef DbgType) {
  std::optional<పూర్ణాంకం> Level;
  size_t ColonPos = DbgType.కనుగొను(':');
  ఒకవేళ (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    ఒకవేళ (LevelStr.ఖాళీలు())
      Level = 0;
    లేకపోతే {
      పూర్ణాంకం parsedLevel;
      ఒకవేళ (గమ్యం_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  తిరిగి std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
నామాకాశ llvm {
/// Exported boolean set by the -debug option.
తార్కిక DebugFlag = అబద్ధం;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
స్థిర ManagedStatic<std::సదిశ<std::pair<std::string, std::optional<పూర్ణాంకం>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
తార్కిక isCurrentDebugType(స్థిరరాశి అక్షరం *DebugType, పూర్ణాంకం Level) {
  ఒకవేళ (CurrentDebugType->ఖాళీలు())
    తిరిగి నిజం;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  తార్కిక HasEnabledDebugType = అబద్ధం;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  కొరకు (స్వయంచాలక &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.రెండవ_2.has_విలువ_2() || D.రెండవ_2.విలువ_2() > 0);
    ఒకవేళ (D.మొదటి_2 != DebugType)
      కొనసాగు;
    ఒకవేళ (!D.రెండవ_2.has_విలువ_2())
      తిరిగి నిజం;
    తిరిగి D.రెండవ_2 >= Level;
  }
  తిరిగి !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
శూన్య సమితి_Current_Debug_Types(స్థిరరాశి అక్షరం **Types, గుర్తులేని లెక్క_2);

శూన్య సమితి_Current_Debug_Type(స్థిరరాశి అక్షరం *Type) {
  సమితి_Current_Debug_Types(&Type, 1);
}

శూన్య సమితి_Current_Debug_Types(స్థిరరాశి అక్షరం **Types, గుర్తులేని లెక్క_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(లెక్క_2);
  కొరకు (స్థిరరాశి అక్షరం *Type : ArrayRef(Types, లెక్క_2))
    CurrentDebugType->నెట్టు_వెనుక(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#నిర్వచితం_కాకపోతే NDEBUG

నామాకాశ {
నిర్మాణం CreateDebug {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<తార్కిక, నిజం>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
నిర్మాణం Create_Debug_బఫరు_కొలత {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<గుర్తులేని>(
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
స్థిర ManagedStatic<cl::opt<తార్కిక, నిజం>, CreateDebug> Debug;
స్థిర ManagedStatic<cl::opt<గుర్తులేని>, Create_Debug_బఫరు_కొలత> Debug_బఫరు_కొలత;

నామాకాశ {

నిర్మాణం DebugOnlyOpt {
  శూన్య క్రియాకారకం=(స్థిరరాశి std::string &Val) స్థిరరాశి {
    ఒకవేళ (Val.ఖాళీలు())
      తిరిగి;
    DebugFlag = నిజం;
    Small_సదిశ<StringRef, 8> DbgTypes;
    StringRef(Val).విభజించు(DbgTypes, ',', -1, అబద్ధం);
    కొరకు (స్వయంచాలక DbgType : DbgTypes)
      CurrentDebugType->నెట్టు_వెనుక(parseDebugType(DbgType));
  }
};
} // namespace

స్థిర DebugOnlyOpt DebugOnlyOptLoc;

నామాకాశ {
నిర్మాణం CreateDebugOnly {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<DebugOnlyOpt, నిజం, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::విలువ_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::విలువ_Required);
  }
};
} // namespace

స్థిర ManagedStatic<cl::opt<DebugOnlyOpt, నిజం, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

శూన్య llvm::initDebugOptions() {
  *Debug;
  *Debug_బఫరు_కొలత;
  *DebugOnly;
}

స్థిర శూన్య printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flush_బఫరు_With_Banner();
}

// Signal handlers - dump debug output on termination.
స్థిర శూన్య debug_user_sig_handler(శూన్య *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  స్థిర నిర్మాణం dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *Debug_బఫరు_కొలత) {
      ఒకవేళ (EnableDebugBuffering && DebugFlag && *Debug_బఫరు_కొలత != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::కూడు_Signal_Handler(&debug_user_sig_handler, శూన్యసూచిక);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  తిరిగి thestrm.strm;
}

శూన్య llvm::printDebugLog() {
  ఒకవేళ (EnableDebugBuffering && DebugFlag && *Debug_బఫరు_కొలత != 0)
    printDebugLogImpl();
}

#లేకపోతే
// Avoid "has no symbols" warning.
నామాకాశ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    తిరిగి errs();
  }
}
శూన్య llvm::initDebugOptions() {}

శూన్య llvm::printDebugLog() {}
#అయితే_ముగింపు

/// EnableDebugBuffering - Turn on signal handler installation.
///
తార్కిక llvm::EnableDebugBuffering = అబద్ధం;
