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

#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/circular_raw_ostream.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <utility>

#अन्तर्भाव "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using नामाकाश llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
स्थिर std::pair<std::string, std::optional<पूर्णाङ्क>>
parseDebugType(StringRef DbgType) {
  std::optional<पूर्णाङ्क> Level;
  माप_प्रकार ColonPos = DbgType.find(':');
  यदि (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    यदि (LevelStr.empty())
      Level = 0;
    अन्यथा {
      पूर्णाङ्क parsedLevel;
      यदि (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  प्रत्यागम std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
नामाकाश llvm {
/// Exported boolean set by the -debug option.
तार्किक DebugFlag = असत्य;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
स्थिर ManagedStatic<std::vector<std::pair<std::string, std::optional<पूर्णाङ्क>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
तार्किक isCurrentDebugType(ध्रुव वर्ण *DebugType, पूर्णाङ्क Level) {
  यदि (CurrentDebugType->empty())
    प्रत्यागम सत्य;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  तार्किक HasEnabledDebugType = असत्य;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  प्रति (स्वतः &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_मूल्य_2() || D.second.मूल्य_2() > 0);
    यदि (D.first != DebugType)
      अनुवर्त;
    यदि (!D.second.has_मूल्य_2())
      प्रत्यागम सत्य;
    प्रत्यागम D.second >= Level;
  }
  प्रत्यागम !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
शून्य setCurrentDebugTypes(ध्रुव वर्ण **Types, अचिन्हित गिनती_2);

शून्य setCurrentDebugType(ध्रुव वर्ण *Type) {
  setCurrentDebugTypes(&Type, 1);
}

शून्य setCurrentDebugTypes(ध्रुव वर्ण **Types, अचिन्हित गिनती_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(गिनती_2);
  प्रति (ध्रुव वर्ण *Type : ArrayRef(Types, गिनती_2))
    CurrentDebugType->धकेलो_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef न_संशोधन

नामाकाश {
संरचना CreateDebug {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<तार्किक, सत्य>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
संरचना CreateDebugBufferSize {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<अचिन्हित>(
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
स्थिर ManagedStatic<cl::opt<तार्किक, सत्य>, CreateDebug> Debug;
स्थिर ManagedStatic<cl::opt<अचिन्हित>, CreateDebugBufferSize> DebugBufferSize;

नामाकाश {

संरचना DebugOnlyOpt {
  शून्य चालक=(ध्रुव std::string &Val) ध्रुव {
    यदि (Val.empty())
      प्रत्यागम;
    DebugFlag = सत्य;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, असत्य);
    प्रति (स्वतः DbgType : DbgTypes)
      CurrentDebugType->धकेलो_back(parseDebugType(DbgType));
  }
};
} // namespace

स्थिर DebugOnlyOpt DebugOnlyOptLoc;

नामाकाश {
संरचना CreateDebugOnly {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<DebugOnlyOpt, सत्य, cl::parser<std::string>>(
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

स्थिर ManagedStatic<cl::opt<DebugOnlyOpt, सत्य, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

शून्य llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

स्थिर शून्य printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
स्थिर शून्य debug_user_sig_handler(शून्य *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  स्थिर संरचना dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      यदि (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  प्रत्यागम thestrm.strm;
}

शून्य llvm::printDebugLog() {
  यदि (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#अन्यथा
// Avoid "has no symbols" warning.
नामाकाश llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    प्रत्यागम errs();
  }
}
शून्य llvm::initDebugOptions() {}

शून्य llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
तार्किक llvm::EnableDebugBuffering = असत्य;
