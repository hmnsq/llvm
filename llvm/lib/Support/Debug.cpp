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

#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/CommandLine.h"
#অন্তর্ভাব "llvm/Support/ManagedStatic.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/circular_raw_ostream.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <utility>

#অন্তর্ভাব "DebugOptions.h"

#undef isCurrentDebugType
#undef setCurrentDebugType
#undef setCurrentDebugTypes

using নামাকাশ llvm;

/// Parse a debug type string into a pair of the debug type and the debug level.
/// The expected format is "type[:level]", where the level is an optional
/// integer.
স্থির std::pair<std::string, std::optional<পূর্ণাঙ্ক>>
parseDebugType(StringRef DbgType) {
  std::optional<পূর্ণাঙ্ক> Level;
  size_t ColonPos = DbgType.find(':');
  যদি (ColonPos != StringRef::npos) {
    StringRef LevelStr = DbgType.substr(ColonPos + 1);
    DbgType = DbgType.take_front(ColonPos);
    যদি (LevelStr.empty())
      Level = 0;
    অন্যথা {
      পূর্ণাঙ্ক parsedLevel;
      যদি (to_integer(LevelStr, parsedLevel, 10))
        Level = parsedLevel;
    }
  }
  প্রত্যাগম std::make_pair(DbgType.str(), Level);
}

// Even though LLVM might be built with NDEBUG, define symbols that the code
// built without NDEBUG can depend on via the llvm/Support/Debug.h header.
নামাকাশ llvm {
/// Exported boolean set by the -debug option.
তার্কিক DebugFlag = অসত্য;

/// The current debug type and an optional debug level.
/// The debug level is the verbosity of the debug output.
/// 0 is a special level that acts as an opt-out for this specific debug type.
/// If provided, the debug output is enabled only if the user specified a level
/// at least as high as the provided level.
স্থির ManagedStatic<std::vector<std::pair<std::string, std::optional<পূর্ণাঙ্ক>>>>
    CurrentDebugType;

/// Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
তার্কিক isCurrentDebugType(ধ্রুব বর্ণ *DebugType, পূর্ণাঙ্ক Level) {
  যদি (CurrentDebugType->empty())
    প্রত্যাগম সত্য;
  // Track if there is at least one debug type with a level, this is used
  // to allow to opt-out of some DebugType and leaving all the others enabled.
  তার্কিক HasEnabledDebugType = অসত্য;
  // See if DebugType is in list. Note: do not use find() as that forces us to
  // unnecessarily create an std::string instance.
  প্রতি (auto &D : *CurrentDebugType) {
    HasEnabledDebugType =
        HasEnabledDebugType || (!D.second.has_মান_2() || D.second.মান_2() > 0);
    যদি (D.first != DebugType)
      অনুবর্ত;
    যদি (!D.second.has_মান_2())
      প্রত্যাগম সত্য;
    প্রত্যাগম D.second >= Level;
  }
  প্রত্যাগম !HasEnabledDebugType;
}

/// Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
শূন্য setCurrentDebugTypes(ধ্রুব বর্ণ **Types, unsigned গুনতি_2);

শূন্য setCurrentDebugType(ধ্রুব বর্ণ *Type) {
  setCurrentDebugTypes(&Type, 1);
}

শূন্য setCurrentDebugTypes(ধ্রুব বর্ণ **Types, unsigned গুনতি_2) {
  CurrentDebugType->clear();
  CurrentDebugType->reserve(গুনতি_2);
  প্রতি (ধ্রুব বর্ণ *Type : ArrayRef(Types, গুনতি_2))
    CurrentDebugType->ঠেলো_back(parseDebugType(Type));
}

} // namespace llvm

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG

নামাকাশ {
সংরচনা CreateDebug {
  স্থির শূন্য *call() {
    প্রত্যাগম নূতন cl::opt<তার্কিক, সত্য>("debug", cl::desc("Enable debug output"),
                                   cl::Hidden, cl::location(DebugFlag));
  }
};

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
সংরচনা CreateDebugBufferSize {
  স্থির শূন্য *call() {
    প্রত্যাগম নূতন cl::opt<unsigned>(
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
স্থির ManagedStatic<cl::opt<তার্কিক, সত্য>, CreateDebug> Debug;
স্থির ManagedStatic<cl::opt<unsigned>, CreateDebugBufferSize> DebugBufferSize;

নামাকাশ {

সংরচনা DebugOnlyOpt {
  শূন্য operator=(ধ্রুব std::string &Val) ধ্রুব {
    যদি (Val.empty())
      প্রত্যাগম;
    DebugFlag = সত্য;
    SmallVector<StringRef, 8> DbgTypes;
    StringRef(Val).split(DbgTypes, ',', -1, অসত্য);
    প্রতি (auto DbgType : DbgTypes)
      CurrentDebugType->ঠেলো_back(parseDebugType(DbgType));
  }
};
} // namespace

স্থির DebugOnlyOpt DebugOnlyOptLoc;

নামাকাশ {
সংরচনা CreateDebugOnly {
  স্থির শূন্য *call() {
    প্রত্যাগম নূতন cl::opt<DebugOnlyOpt, সত্য, cl::parser<std::string>>(
        "debug-only",
        cl::desc(
            "Enable a specific type of debug output (comma separated list "
            "of types using the format \"type[:level]\", where the level "
            "is an optional integer. The level can be set to 1, 2, 3, etc. to "
            "control the verbosity of the output. Setting a debug-type level "
            "to zero acts as an opt-out for this specific debug-type without "
            "affecting the others."),
        cl::Hidden, cl::মান_desc("debug string"),
        cl::location(DebugOnlyOptLoc), cl::মান_Required);
  }
};
} // namespace

স্থির ManagedStatic<cl::opt<DebugOnlyOpt, সত্য, cl::parser<std::string>>,
                     CreateDebugOnly>
    DebugOnly;

শূন্য llvm::initDebugOptions() {
  *Debug;
  *DebugBufferSize;
  *DebugOnly;
}

স্থির শূন্য printDebugLogImpl() {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream &dbgout =
      static_cast<circular_raw_ostream &>(llvm::dbgs());
  dbgout.flushBufferWithBanner();
}

// Signal handlers - dump debug output on termination.
স্থির শূন্য debug_user_sig_handler(শূন্য *Cookie) { printDebugLogImpl(); }

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  স্থির সংরচনা dbgstream {
    circular_raw_ostream strm;

    dbgstream()
        : strm(errs(), "*** Debug Log Output ***\n",
               (!EnableDebugBuffering || !DebugFlag) ? 0 : *DebugBufferSize) {
      যদি (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, nullptr);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  প্রত্যাগম thestrm.strm;
}

শূন্য llvm::printDebugLog() {
  যদি (EnableDebugBuffering && DebugFlag && *DebugBufferSize != 0)
    printDebugLogImpl();
}

#অন্যথা
// Avoid "has no symbols" warning.
নামাকাশ llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    প্রত্যাগম errs();
  }
}
শূন্য llvm::initDebugOptions() {}

শূন্য llvm::printDebugLog() {}
#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
তার্কিক llvm::EnableDebugBuffering = অসত্য;
