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

#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/CommandLine.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/circular_raw_ostream.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <utility>

#समावेश "DebugOptions.h"

#अघोषित isCurrentDebugType
#अघोषित setCurrentDebugType
#अघोषित setCurrentDebugTypes

using namespace llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
अटल std::pair<std::string, std::optional<पूर्णांक>>
parseDebugType(StringRef DbgType) {
  std::optional<पूर्णांक> Level;
  माप_प्रकार ColonPos = DbgType.find(':');
  अगर (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    अगर (LevelStr.empty())
      Level = 0;
    अन्यथा {
      पूर्णांक parsedLevel;
      अगर (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  वापस std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
namespace llvm {
/// Exported boolean set by the -debug option.
bool DebugFlag = false;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
अटल ManagedStatic<std::vector<std::pair<std::string, std::optional<पूर्णांक>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
bool isCurrentDebugType(स्थिर अक्षर *DebugType, पूर्णांक Level) {
  अगर (CurrentDebugType->empty())
    वापस true;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  bool HasEnabledDebugType = false;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  क्रम (स्वतः &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_मूल्य_2() || D.second.मूल्य_2() > 0);
    अगर (D.first != DebugType)
      जारी;
    अगर (!D.second.has_मूल्य_2())
      वापस true;
    वापस D.second >= Level;
  }
  वापस !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
व्योम setCurrentDebugTypes(स्थिर अक्षर **Types, अचिन्हित गिनती_2);

व्योम setCurrentDebugType(स्थिर अक्षर *Type) {
  setCurrentDebugTypes(&Type, 1);
}

व्योम setCurrentDebugTypes(स्थिर अक्षर **Types, अचिन्हित गिनती_2) {
  CurrentDebugType->सफा();
  CurrentDebugType->reserve(गिनती_2);
  क्रम (स्थिर अक्षर *Type : ArrayRef(Types, गिनती_2))
    CurrentDebugType->धकेलो_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#अगर_अघोषित न_संशोधन

namespace {
काष्ठा CreateDebug {
  अटल व्योम *call() {
    वापस नया cl::opt<bool, true>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
काष्ठा Create_Debug_बफर_आकार {
  अटल व्योम *call() {
    वापस नया cl::opt<अचिन्हित>(
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
अटल ManagedStatic<cl::opt<bool, true>, CreateDebug> Debug;
अटल ManagedStatic<cl::opt<अचिन्हित>, Create_Debug_बफर_आकार> Debug_बफर_आकार;

namespace {

काष्ठा DebugOnlyOpt {
  व्योम चालक=(स्थिर std::string &Val) स्थिर {
    अगर (Val.empty())
      वापस;
    DebugFlag = true;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, false);
    क्रम (स्वतः DbgType : DbgTypes)
      CurrentDebugType->धकेलो_back(parseDebugType(DbgType));
  }
};
} // namespace

अटल DebugOnlyOpt DebugOnlyOptLoc;

namespace {
काष्ठा CreateDebugOnly {
  अटल व्योम *call() {
    वापस नया cl::opt<DebugOnlyOpt, true, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::मूल्य_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::मूल्य_Required);
  }
};
} // namespace

अटल ManagedStatic<cl::opt<DebugOnlyOpt, true, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

व्योम llvm::initDebugOptions() {
  *Debug;
  *Debug_बफर_आकार;
  *DebugOnly;
}

अटल व्योम printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flush_बफर_With_Banner();
}

// Signal handlers - dump debug output on termination.
अटल व्योम debug_user_sig_handler(व्योम *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  अटल काष्ठा dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *Debug_बफर_आकार) {
      अगर (EnableDebugBuffering && DebugFlag && *Debug_बफर_आकार != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  वापस thestrm.strm;
}

व्योम llvm::printDebugLog() {
  अगर (EnableDebugBuffering && DebugFlag && *Debug_बफर_आकार != 0)
    printDebugLogImpl();
}

#अन्यथा
// Avoid "has no symbols" warning.
namespace llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    वापस errs();
  }
}
व्योम llvm::initDebugOptions() {}

व्योम llvm::printDebugLog() {}
#पूर्ण_अगर

/// EnableDebugBuffering - Turn on signal handler installation.
///
bool llvm::EnableDebugBuffering = false;
