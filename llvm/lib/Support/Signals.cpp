//===- Signals.cpp - Signal Handling support --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines some helpful functions for dealing with the possibility of
// Unix signals occurring while your program is running.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileUtilities.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/StringSaver.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <array>
#ಅನ್ತರ್ಭಾವ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ಸ್ಥಿರ ತಾರ್ಕಿಕ DisableSymbolicationFlag = ಸುಳ್ಳು;
ಸ್ಥಿರ ManagedStatic<std::string> CrashDiagnosticsDirectory;
ನಾಮಾಕಾಶ {
ರಚನೆ CreateDisableSymbolication {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<ತಾರ್ಕಿಕ, ನಿಜ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ರಚನೆ CreateCrashDiagnosticsDir {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<std::string, ನಿಜ>(
        "crash-diagnostics-dir", cl::ಮೌಲ್ಯ_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
ಶೂನ್ಯ llvm::init_ಸಂಕೇತಗಳು_Options() {
  ಸ್ಥಿರ ManagedStatic<cl::opt<ತಾರ್ಕಿಕ, ನಿಜ>, CreateDisableSymbolication>
      DisableSymbolication;
  ಸ್ಥಿರ ManagedStatic<cl::opt<std::string, ನಿಜ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr ಅಕ್ಷರ DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr ಅಕ್ಷರ LLVMSymbolizer_ಮಾರ್ಗ_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr ಅಕ್ಷರ EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ರಚನೆ CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  ಶೂನ್ಯ *Cookie;
  ಎಣಿಕೆ ವರ್ಗ ಸ್ಥಿತಿ_2 { ಖಾಲಿಗಳು_2, Initializing, Initialized, Executing };
  std::atomic<ಸ್ಥಿತಿ_2> Flag;
};

ಸ್ಥಿರ constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ಸ್ಥಿರ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_ಗಮ್ಯ_ಸತತ() {
  ಸ್ಥಿರ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  ಹಿಂತಿರುಗಿ callbacks;
}

// Signal-safe.
ಶೂನ್ಯ sys::ಸತತ_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sandbox::scopedDisable();

  ಗಾಗಿ (CallbackAndCookie &ಸತತ_Me : Call_Backs_ಗಮ್ಯ_ಸತತ()) {
    ಸ್ವಯಂಚಾಲಿತ Expected = CallbackAndCookie::ಸ್ಥಿತಿ_2::Initialized;
    ಸ್ವಯಂಚಾಲಿತ Desired = CallbackAndCookie::ಸ್ಥಿತಿ_2::Executing;
    ಆದರೆ (!ಸತತ_Me.Flag.compare_exchange_strong(Expected, Desired))
      ಮುಂದುವರಿ;
    (*ಸತತ_Me.Callback)(ಸತತ_Me.Cookie);
    ಸತತ_Me.Callback = ಶೂನ್ಯಸೂಚಕ;
    ಸತತ_Me.Cookie = ಶೂನ್ಯಸೂಚಕ;
    ಸತತ_Me.Flag.store(CallbackAndCookie::ಸ್ಥಿತಿ_2::ಖಾಲಿಗಳು_2);
  }
}

// Signal-safe.
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                ಶೂನ್ಯ *Cookie) {
  ಗಾಗಿ (CallbackAndCookie &ಗಣ_Me : Call_Backs_ಗಮ್ಯ_ಸತತ()) {
    ಸ್ವಯಂಚಾಲಿತ Expected = CallbackAndCookie::ಸ್ಥಿತಿ_2::ಖಾಲಿಗಳು_2;
    ಸ್ವಯಂಚಾಲಿತ Desired = CallbackAndCookie::ಸ್ಥಿತಿ_2::Initializing;
    ಆದರೆ (!ಗಣ_Me.Flag.compare_exchange_strong(Expected, Desired))
      ಮುಂದುವರಿ;
    ಗಣ_Me.Callback = FnPtr;
    ಗಣ_Me.Cookie = Cookie;
    ಗಣ_Me.Flag.store(CallbackAndCookie::ಸ್ಥಿತಿ_2::Initialized);
    ಹಿಂತಿರುಗಿ;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ ಕಂಡುಹಿಡಿ_Modules_And_Offsets(ಶೂನ್ಯ **ರಾಶಿ_Trace, ಪೂರ್ಣಾಂಕ ಆಳ,
                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **Modules, intptr_t *Offsets,
                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ಸ್ಥಿರ Formatted_ಸಂಖ್ಯೆ format_ptr(ಶೂನ್ಯ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  ಚಿಹ್ನೆರಹಿತ Ptr_ಅಗಲ = 2 + 2 * ಗಾತ್ರ(ಶೂನ್ಯ *);
  ಹಿಂತಿರುಗಿ format_hex((uint64_t)PC, Ptr_ಅಗಲ);
}

/// Reads a file \p Filename written by llvm-symbolizer containing function
/// names and source locations for the addresses in \p AddressList and returns
/// the strings in a vector of pairs, where the first pair element is the index
/// of the corresponding entry in AddressList and the second is the symbolized
/// frame, in a format based on the sanitizer stack trace printer, with the
/// exception that it does not write out frame numbers (i.e. "#2 " for the
/// third address), as it is not assumed that \p AddressList corresponds to a
/// single stack trace.
/// There may be multiple returned entries for a single \p AddressList entry if
/// that frame address corresponds to one or more inlined frames; in this case,
/// all frames for an address will appear contiguously and in-order.
std::optional<Small_ಸದಿಶ<std::pair<ಚಿಹ್ನೆರಹಿತ, std::string>, 0>>
collect_ವಿಳಾಸ_Symbols(ಶೂನ್ಯ **ವಿಳಾಸ_List, ಚಿಹ್ನೆರಹಿತ ವಿಳಾಸ_ಎಣಿಕೆ,
                      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *MainExecutableName,
                      ಸ್ಥಿರಾಂಕ std::string &LLVMSymbolizer_ಮಾರ್ಗ) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_ಸದಿಶ<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, 0> Modules(ವಿಳಾಸ_ಎಣಿಕೆ, ಶೂನ್ಯಸೂಚಕ);
  Small_ಸದಿಶ<intptr_t, 0> Offsets(ವಿಳಾಸ_ಎಣಿಕೆ, 0);
  ಆದರೆ (!ಕಂಡುಹಿಡಿ_Modules_And_Offsets(ವಿಳಾಸ_List, ವಿಳಾಸ_ಎಣಿಕೆ, Modules.ದತ್ತಾಂಶ(),
                             Offsets.ದತ್ತಾಂಶ(), MainExecutableName, StrPool))
    ಹಿಂತಿರುಗಿ {};
  ಪೂರ್ಣಾಂಕ ಒಳಹರಿವು_FD;
  SmallString<32> ಒಳಹರಿವು_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ಒಳಹರಿವು_FD, ಒಳಹರಿವು_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ಒಳಹರಿವು_Remover(ಒಳಹರಿವು_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ಒಳಹರಿವು(ಒಳಹರಿವು_FD, ನಿಜ);
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ AddrIdx = 0; AddrIdx < ವಿಳಾಸ_ಎಣಿಕೆ; AddrIdx++) {
      ಆದರೆ (Modules[AddrIdx])
        ಒಳಹರಿವು << Modules[AddrIdx] << " " << (ಶೂನ್ಯ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ಒಳಹರಿವು_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef ವಾದಗಳು[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#ಆದರೆ_ಅಂತ್ಯ
                      "--demangle"};
  ಪೂರ್ಣಾಂಕ ಸತತ_ಫಲಿತಾಂಶ =
      sys::ExecuteAndWait(LLVMSymbolizer_ಮಾರ್ಗ, ವಾದಗಳು, std::nullopt, Redirects);
  ಆದರೆ (ಸತತ_ಫಲಿತಾಂಶ != 0)
    ಹಿಂತಿರುಗಿ {};

  Small_ಸದಿಶ<std::pair<ಚಿಹ್ನೆರಹಿತ, std::string>, 0> ಫಲಿತಾಂಶ;
  ಸ್ವಯಂಚಾಲಿತ OutputBuf = ಸ್ಮರಣೆ_ಬಫರ್::getFile(OutputFile.c_str());
  ಆದರೆ (!OutputBuf)
    ಹಿಂತಿರುಗಿ {};
  StringRef Output = OutputBuf.get()->get_ಬಫರ್();
  Small_ಸದಿಶ<StringRef, 32> ಸಾಲುಗಳು;
  Output.ವಿಭಜಿಸು(ಸಾಲುಗಳು, "\n");
  ಸ್ವಯಂಚಾಲಿತ *Cur_ಸಾಲು = ಸಾಲುಗಳು.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ AddrIdx = 0; AddrIdx < ವಿಳಾಸ_ಎಣಿಕೆ; AddrIdx++) {
    ಆದರೆ (!Modules[AddrIdx]) {
      ಸ್ವಯಂಚಾಲಿತ &Symbolized_ಚೌಕಟ್ಟು = ಫಲಿತಾಂಶ.emplace_ಹಿಂಭಾಗ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_ಚೌಕಟ್ಟು.ಎರಡನೇ_2);
      OS << format_ptr(ವಿಳಾಸ_List[AddrIdx]);
      ಮುಂದುವರಿ;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    ಗಾಗಿ (;;) {
      ಆದರೆ (Cur_ಸಾಲು == ಸಾಲುಗಳು.ಕೊನೆ())
        ಹಿಂತಿರುಗಿ {};
      StringRef FunctionName = *Cur_ಸಾಲು++;
      ಆದರೆ (FunctionName.ಖಾಲಿಗಳು())
        ನಿಲ್ಲಿಸು;
      ಸ್ವಯಂಚಾಲಿತ &Symbolized_ಚೌಕಟ್ಟು = ಫಲಿತಾಂಶ.emplace_ಹಿಂಭಾಗ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_ಚೌಕಟ್ಟು.ಎರಡನೇ_2);
      OS << format_ptr(ವಿಳಾಸ_List[AddrIdx]) << ' ';
      ಆದರೆ (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      ಆದರೆ (Cur_ಸಾಲು == ಸಾಲುಗಳು.ಕೊನೆ())
        ಹಿಂತಿರುಗಿ {};
      StringRef File_ಸಾಲು_Info = *Cur_ಸಾಲು++;
      ಆದರೆ (!File_ಸಾಲು_Info.starts_with("??")) {
        OS << File_ಸಾಲು_Info;
      } ಇಲ್ಲದಿದ್ದರೆ {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ErrorOr<std::string> get_LLVMSymbolizer_ಮಾರ್ಗ(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_ಮಾರ್ಗ_Or_Err = std::error_ಸಂಕೇತ();
  ಆದರೆ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ = getenv(LLVMSymbolizer_ಮಾರ್ಗ_Env)) {
    LLVMSymbolizer_ಮಾರ್ಗ_Or_Err = sys::ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ_By_Name(ಮಾರ್ಗ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!Argv0.ಖಾಲಿಗಳು()) {
    StringRef ಪೋಷಕ_2 = llvm::sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(Argv0);
    ಆದರೆ (!ಪೋಷಕ_2.ಖಾಲಿಗಳು())
      LLVMSymbolizer_ಮಾರ್ಗ_Or_Err =
          sys::ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ_By_Name("llvm-symbolizer", ಪೋಷಕ_2);
  }
  ಆದರೆ (!LLVMSymbolizer_ಮಾರ್ಗ_Or_Err)
    LLVMSymbolizer_ಮಾರ್ಗ_Or_Err = sys::ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ_By_Name("llvm-symbolizer");
  ಹಿಂತಿರುಗಿ LLVMSymbolizer_ಮಾರ್ಗ_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_ಬಳಸಿದ
ಸ್ಥಿರ ತಾರ್ಕಿಕ print_Symbolized_ರಾಶಿ_Trace(StringRef Argv0, ಶೂನ್ಯ **ರಾಶಿ_Trace,
                                      ಪೂರ್ಣಾಂಕ ಆಳ, llvm::raw_ostream &OS) {
  ಆದರೆ (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Don't recursively invoke the llvm-symbolizer binary.
  ಆದರೆ (Argv0.ಒಳಗೊಂಡಿದೆ("llvm-symbolizer"))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_ಮಾರ್ಗ_Or_Err = get_LLVMSymbolizer_ಮಾರ್ಗ(Argv0);
  ಆದರೆ (!LLVMSymbolizer_ಮಾರ್ಗ_Or_Err)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಸ್ಥಿರಾಂಕ std::string &LLVMSymbolizer_ಮಾರ್ಗ = *LLVMSymbolizer_ಮಾರ್ಗ_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ);

  ಸ್ವಯಂಚಾಲಿತ Symbolized_ವಿಳಾಸಗಳು_Opt = collect_ವಿಳಾಸ_Symbols(
      ರಾಶಿ_Trace, ಆಳ, MainExecutableName.c_str(), LLVMSymbolizer_ಮಾರ್ಗ);
  ಆದರೆ (!Symbolized_ವಿಳಾಸಗಳು_Opt)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಚೌಕಟ್ಟು_No = 0; ಚೌಕಟ್ಟು_No < Symbolized_ವಿಳಾಸಗಳು_Opt->ಗಾತ್ರ_3();
       ++ಚೌಕಟ್ಟು_No) {
    OS << ಬಲ_justify(formatv("#{0}", ಚೌಕಟ್ಟು_No).str(), std::log10(ಆಳ) + 2)
       << ' ' << (*Symbolized_ವಿಳಾಸಗಳು_Opt)[ಚೌಕಟ್ಟು_No].ಎರಡನೇ_2 << '\n';
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

#ಆದರೆ LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
ಶೂನ್ಯ sys::symbolize_ವಿಳಾಸಗಳು(ವಿಳಾಸ_ಗಣ &ವಿಳಾಸಗಳು,
                             Symbolized_ವಿಳಾಸ_Map &Symbolized_ವಿಳಾಸಗಳು) {
  ಪ್ರತಿಜ್ಞಾ(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_ಸದಿಶ<ಶೂನ್ಯ *, 0> ವಿಳಾಸ_List(ವಿಳಾಸಗಳು.begin(), ವಿಳಾಸಗಳು.ಕೊನೆ());
  ಆದರೆ (ವಿಳಾಸ_List.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;
  llvm::ವಿಂಗಡಿಸು(ವಿಳಾಸ_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_ಮಾರ್ಗ_Or_Err = get_LLVMSymbolizer_ಮಾರ್ಗ();
  ಆದರೆ (!LLVMSymbolizer_ಮಾರ್ಗ_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ಸ್ಥಿರಾಂಕ std::string &LLVMSymbolizer_ಮಾರ್ಗ = *LLVMSymbolizer_ಮಾರ್ಗ_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ);

  ಸ್ವಯಂಚಾಲಿತ Symbolized_ವಿಳಾಸಗಳು_Opt =
      collect_ವಿಳಾಸ_Symbols(ವಿಳಾಸ_List.begin(), ವಿಳಾಸ_List.ಗಾತ್ರ_3(),
                            MainExecutableName.c_str(), LLVMSymbolizer_ಮಾರ್ಗ);
  ಆದರೆ (!Symbolized_ವಿಳಾಸಗಳು_Opt)
    ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ Symbolized_ಚೌಕಟ್ಟು : *Symbolized_ವಿಳಾಸಗಳು_Opt) {
    Small_ಸದಿಶ<std::string, 0> &SymbolizedAddrs =
        Symbolized_ವಿಳಾಸಗಳು[ವಿಳಾಸ_List[Symbolized_ಚೌಕಟ್ಟು.ಮೊದಲ_2]];
    SymbolizedAddrs.ತಳ್ಳು_ಹಿಂಭಾಗ(Symbolized_ಚೌಕಟ್ಟು.ಎರಡನೇ_2);
  }
  ಹಿಂತಿರುಗಿ;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ತಾರ್ಕಿಕ printMarkupContext(raw_ostream &OS, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *MainExecutableName);

LLVM_ATTRIBUTE_ಬಳಸಿದ
ಸ್ಥಿರ ತಾರ್ಕಿಕ print_Markup_ರಾಶಿ_Trace(StringRef Argv0, ಶೂನ್ಯ **ರಾಶಿ_Trace, ಪೂರ್ಣಾಂಕ ಆಳ,
                                  raw_ostream &OS) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Env = getenv(EnableSymbolizerMarkupEnv);
  ಆದರೆ (!Env || !*Env)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ);
  ಆದರೆ (!printMarkupContext(OS, MainExecutableName.c_str()))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ I = 0; I < ಆಳ; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ರಾಶಿ_Trace[I]);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

// Include the platform-specific parts of this class.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Signals.inc"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Signals.inc"
#ಆದರೆ_ಅಂತ್ಯ
